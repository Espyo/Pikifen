/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy class and enemy-related functions.
 */

#include <algorithm>
#include <unordered_set>

#include "enemy.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../mob_types/mob_type.h"
#include "../utils/math_utils.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates an enemy mob.
 */
enemy::enemy(const point &pos, enemy_type* type, const float angle) :
    mob(pos, type, angle),
    ene_type(type) {
    
}


/* ----------------------------------------------------------------------------
 * Returns whether or not an enemy can receive a given status effect.
 */
bool enemy::can_receive_status(status_type* s) const {
    return s->affects & STATUS_AFFECTS_ENEMIES;
}


/* ----------------------------------------------------------------------------
 * Draws an enemy, tinting it if necessary (for Onion delivery).
 */
void enemy::draw_mob() {
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    ALLEGRO_COLOR delivery_color = map_gray(0);
    float delivery_time_ratio_left = LARGE_FLOAT;
    
    if(fsm.cur_state->id == ENEMY_EXTRA_STATE_BEING_DELIVERED) {
        delivery_color = ((onion*) focused_mob)->oni_type->pik_type->main_color;
        delivery_time_ratio_left = script_timer.get_ratio_left();
    }
    
    get_sprite_bitmap_effects(
        s_ptr, &eff, true, true,
        delivery_time_ratio_left, delivery_color
    );
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
    
    draw_status_effect_bmp(this, eff);
}


//Normally, the spirit's diameter is the enemy's. Multiply the spirit by this.
const float ENEMY_SPIRIT_SIZE_MULT = 0.7;
//Maximum diameter an enemy's spirit can be.
const float ENEMY_SPIRIT_MAX_SIZE = 128;
//Minimum diameter an enemy's spirit can be.
const float ENEMY_SPIRIT_MIN_SIZE = 16;

/* ----------------------------------------------------------------------------
 * Logic specific to enemies for when they finish dying.
 */
void enemy::finish_dying_class_specifics() {
    if(ene_type->drops_corpse) {
        become_carriable(CARRY_DESTINATION_ONION);
        fsm.set_state(ENEMY_EXTRA_STATE_CARRIABLE_WAITING);
    }
    particle par(
        PARTICLE_TYPE_ENEMY_SPIRIT, pos, LARGE_FLOAT,
        clamp(
            type->radius * 2 * ENEMY_SPIRIT_SIZE_MULT,
            ENEMY_SPIRIT_MIN_SIZE, ENEMY_SPIRIT_MAX_SIZE
        ),
        2, PARTICLE_PRIORITY_MEDIUM
    );
    par.bitmap = game.sys_assets.bmp_enemy_spirit;
    par.speed.x = 0;
    par.speed.y = -50;
    par.friction = 0.5;
    par.gravity = 0;
    par.color = al_map_rgb(255, 192, 255);
    game.states.gameplay_st->particles.add(par);
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 */
void enemy::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    string spoils_var;
    string random_pellet_spoils_var;
    
    if(svr.get("spoils", spoils_var)) {
        vector<string> spoils_strs = semicolon_list_to_vector(spoils_var, ",");
        for(size_t s = 0; s < spoils_strs.size(); ++s) {
            mob_type* type_ptr =
                game.mob_categories.find_mob_type(spoils_strs[s]);
            if(!type_ptr) {
                log_error(
                    "A mob (" + get_error_message_mob_info(this) +
                    ") is set to have a spoil of type \"" + spoils_strs[s] +
                    "\", but no such mob type exists!"
                );
                continue;
            }
            specific_spoils.push_back(type_ptr);
        }
    }
    if(svr.get("random_pellet_spoils", random_pellet_spoils_var)) {
        vector<string> random_pellet_spoils_strs =
            semicolon_list_to_vector(random_pellet_spoils_var, ",");
        for(size_t s = 0; s < random_pellet_spoils_strs.size(); ++s) {
            random_pellet_spoils.push_back(s2i(random_pellet_spoils_strs[s]));
        }
    }
}


/* ----------------------------------------------------------------------------
 * Sets up stuff for the beginning of the enemy's death process.
 */
void enemy::start_dying_class_specifics() {
    vector<mob_type*> spoils_to_spawn = specific_spoils;
    
    //If there are random pellets to spawn, then prepare some data.
    if(!random_pellet_spoils.empty()) {
        vector<pikmin_type*> available_pik_types;
        
        //Start by obtaining a list of available Pikmin types, given the
        //Onions currently in the area.
        for(size_t o = 0; o < game.states.gameplay_st->mobs.onions.size(); ++o) {
            available_pik_types.push_back(game.states.gameplay_st->mobs.onions[o]->oni_type->pik_type);
        }
        
        //Remove duplicates from the list.
        sort(available_pik_types.begin(), available_pik_types.end());
        available_pik_types.erase(
            unique(available_pik_types.begin(), available_pik_types.end()),
            available_pik_types.end()
        );
        
        for(size_t s = 0; s < random_pellet_spoils.size(); ++s) {
            //For every pellet that we want to spawn...
            vector<pellet_type*> possible_pellets;
            
            //Check the pellet types that match that number and
            //also match the available Pikmin types.
            for(auto &p : game.mob_types.pellet) {
                bool pik_type_ok = false;
                for(size_t pt = 0; pt < available_pik_types.size(); ++pt) {
                    if(p.second->pik_type == available_pik_types[pt]) {
                        pik_type_ok = true;
                        break;
                    }
                }
                
                if(!pik_type_ok) {
                    //There is no Onion for this pellet type. Pass.
                    continue;
                }
                
                if(p.second->number == random_pellet_spoils[s]) {
                    possible_pellets.push_back(p.second);
                }
            }
            
            //And now, pick a random one out of the possible pellets.
            if(!possible_pellets.empty()) {
                spoils_to_spawn.push_back(
                    possible_pellets[randomi(0, possible_pellets.size() - 1)]
                );
            }
        }
    }
    
    for(size_t s = 0; s < spoils_to_spawn.size(); ++s) {
        mob_type::spawn_struct str;
        str.angle = 0;
        str.coords_z = 0;
        str.relative = true;
        str.momentum = 100;
        spawn(&str, spoils_to_spawn[s]);
    }
}
