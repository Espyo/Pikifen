/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion class and Onion-related functions.
 */

#include <algorithm>

#include "onion.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../utils/geometry_utils.h"
#include "../utils/string_utils.h"


using std::size_t;
using std::string;


namespace ONION {
//How quickly an Onion fades to and from see-through, in values per second.
const float FADE_SPEED = 255.0f;
//Delay before the Onion starts the seed spewing process.
const float FULL_SPEW_DELAY = 2.0f;
//Delay between each individual seed being spit.
const float NEXT_SPEW_DELAY = 0.10f;
//Onion opacity when it goes see-through.
const unsigned char SEETHROUGH_ALPHA = 128;
}


//An Onion-spat seed starts with this Z offset from the Onion.
const float onion::ONION_NEW_SEED_Z_OFFSET = 320.0f;
//After spitting a seed, the next seed's angle shifts by this much.
const float onion::ONION_SPEW_ANGLE_SHIFT = TAU * 0.12345;
//An Onion-spat seed is this quick, horizontally.
const float onion::ONION_SPEW_H_SPEED = 80.0f;
//Deviate the seed's horizontal speed by this much, more or less.
const float onion::ONION_SPEW_H_SPEED_DEVIATION = 10.0f;
//An Onion-spat seed is this quick, vertically.
const float onion::ONION_SPEW_V_SPEED = 600.0f;


/* ----------------------------------------------------------------------------
 * Creates an Onion mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Onion type this mob belongs to.
 * angle:
 *   Starting angle.
 */
onion::onion(const point &pos, onion_type* type, const float angle) :
    mob(pos, type, angle),
    oni_type(type),
    nest(nullptr),
    activated(true),
    full_spew_timer(ONION::FULL_SPEW_DELAY),
    next_spew_timer(ONION::NEXT_SPEW_DELAY),
    next_spew_angle(0),
    seethrough(255) {
    
    nest = new pikmin_nest_struct(this, oni_type->nest);
    
    //Increase its Z by one so that mobs that walk at
    //ground level next to it will appear under it.
    gravity_mult = 0.0f;
    z++;
    
    full_spew_timer.on_end =
    [this] () { next_spew_timer.start(); };
    next_spew_timer.on_end =
    [this] () {
        for(size_t t = 0; t < oni_type->nest->pik_types.size(); ++t) {
            if(spew_queue[t] > 0) {
                next_spew_timer.start();
                spew();
                return;
            }
        }
    };
    
    for(size_t t = 0; t < oni_type->nest->pik_types.size(); ++t) {
        spew_queue.push_back(0);
    }
    
    set_animation(
        MOB_TYPE::ANIM_IDLING, true,
        START_ANIMATION_RANDOM_TIME_ON_SPAWN
    );
}


/* ----------------------------------------------------------------------------
 * Destroys an Onion mob.
 */
onion::~onion() {
    delete nest;
}


/* ----------------------------------------------------------------------------
 * Draws an Onion.
 */
void onion::draw_mob() {
    sprite* s_ptr = get_cur_sprite();
    
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(
        s_ptr, &eff,
        SPRITE_BITMAP_EFFECT_STANDARD |
        SPRITE_BITMAP_EFFECT_STATUS |
        SPRITE_BITMAP_EFFECT_SECTOR_BRIGHTNESS |
        SPRITE_BITMAP_EFFECT_HEIGHT |
        SPRITE_BITMAP_EFFECT_DELIVERY
    );
    
    eff.tint_color.a *= (seethrough / 255.0f);
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 * svr:
 *   Script var reader to use.
 */
void onion::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    nest->read_script_vars(svr);
}


/* ----------------------------------------------------------------------------
 * Spew a Pikmin seed in the queue or add it to the Onion's storage.
 */
void onion::spew() {
    for(size_t t = 0; t < spew_queue.size(); ++t) {
        if(spew_queue[t] == 0) continue;
        
        spew_queue[t]--;
        
        game.statistics.pikmin_births++;
        game.states.gameplay->mission_info[0].pikmin_born++;
        game.states.gameplay->mission_info[0].last_pikmin_born_pos = pos;
        
        size_t total_after =
            game.states.gameplay->mobs.pikmin_list.size() + 1;
            
        if(total_after > game.config.max_pikmin_in_field) {
            nest->pikmin_inside[t][0]++;
            return;
        }
        
        float horizontal_strength =
            ONION_SPEW_H_SPEED +
            randomf(
                -ONION_SPEW_H_SPEED_DEVIATION,
                ONION_SPEW_H_SPEED_DEVIATION
            );
        spew_pikmin_seed(
            pos, z + ONION_NEW_SEED_Z_OFFSET, oni_type->nest->pik_types[t],
            next_spew_angle, horizontal_strength, ONION_SPEW_V_SPEED
        );
        
        next_spew_angle += ONION_SPEW_ANGLE_SHIFT;
        next_spew_angle = normalize_angle(next_spew_angle);
        
        return;
    }
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void onion::tick_class_specifics(const float delta_t) {
    bool needs_to_spew = false;
    for(size_t t = 0; t < oni_type->nest->pik_types.size(); ++t) {
        if(spew_queue[t] > 0) {
            needs_to_spew = true;
            break;
        }
    }
    
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); ++o) {
        onion* o_ptr = game.states.gameplay->mobs.onions[o];
        
        if(needs_to_spew) {
        
            o_ptr->full_spew_timer.tick(delta_t);
            o_ptr->next_spew_timer.tick(delta_t);
            
        }
        
        unsigned char final_alpha = 255;
        for(size_t p = 0; p < MAX_PLAYERS;++p){
        if(
            game.states.gameplay->player_info[p].cur_leader_ptr &&
            bbox_check(
                game.states.gameplay->player_info[p].cur_leader_ptr->pos, o_ptr->pos,
                game.states.gameplay->player_info[p].cur_leader_ptr->radius +
                o_ptr->radius * 3
            )
        ) {
            final_alpha = ONION::SEETHROUGH_ALPHA;
        }
        
        if(
            game.states.gameplay->player_info[p].cur_leader_ptr &&
            bbox_check(
                game.states.gameplay->player_info[p].leader_cursor_w, o_ptr->pos,
                game.states.gameplay->player_info[p].cur_leader_ptr->radius +
                o_ptr->radius * 3
            )
        ) {
            final_alpha = ONION::SEETHROUGH_ALPHA;
        }
        }
        if(o_ptr->seethrough != final_alpha) {
            if(final_alpha < o_ptr->seethrough) {
                o_ptr->seethrough =
                    std::max(
                        (double) final_alpha,
                        (double) o_ptr->seethrough - ONION::FADE_SPEED * delta_t
                    );
            } else {
                o_ptr->seethrough =
                    std::min(
                        (double) final_alpha,
                        (double) o_ptr->seethrough + ONION::FADE_SPEED * delta_t
                    );
            }
        }
    }
    
    nest->tick(delta_t);
}
