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
    activated(true),
    calling_leader(nullptr),
    full_spew_timer(ONION_FULL_SPEW_DELAY),
    next_spew_timer(ONION_NEXT_SPEW_DELAY),
    next_spew_angle(0),
    next_call_time(0.0f),
    seethrough(255) {
    
    //Increase its Z by one so that mobs that walk at
    //ground level next to it will appear under it.
    gravity_mult = 0.0f;
    z++;
    
    full_spew_timer.on_end =
    [this] () { next_spew_timer.start(); };
    next_spew_timer.on_end =
    [this] () {
        for(size_t t = 0; t < oni_type->pik_types.size(); ++t) {
            if(spew_queue[t] > 0) {
                next_spew_timer.start();
                spew();
                return;
            }
        }
    };
    
    for(size_t t = 0; t < oni_type->pik_types.size(); ++t) {
        pikmin_inside.push_back(vector<size_t>(N_MATURITIES, 0));
        spew_queue.push_back(0);
        call_queue.push_back(0);
    }
    
    set_animation(ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * Temporary feature to allow Pikmin to be called from the Onion.
 * Calls out a Pikmin from inside the Onion, if possible.
 * Gives priority to the higher maturities.
 * Returns true if a Pikmin was spawned, false otherwise.
 * type_idx:
 *   Index of the Pikmin type, from the types this Onion manages.
 */
bool onion::call_pikmin(const size_t type_idx) {
    if(
        game.states.gameplay->mobs.pikmin_list.size() >=
        game.config.max_pikmin_in_field
    ) {
        return false;
    }
    
    for(size_t m = 0; m < N_MATURITIES; ++m) {
        //Let's check the maturities in reverse order.
        size_t cur_m = N_MATURITIES - m - 1;
        
        if(pikmin_inside[type_idx][cur_m] == 0) continue;
        
        //Spawn the Pikmin!
        //Update the Pikmin count.
        pikmin_inside[type_idx][cur_m]--;
        
        //Decide a leg to come out of.
        size_t leg_idx =
            randomi(0, (oni_type->leg_body_parts.size() / 2) - 1);
        size_t leg_hole_bp_idx =
            anim.anim_db->find_body_part(
                oni_type->leg_body_parts[leg_idx * 2]
            );
        size_t leg_foot_bp_idx =
            anim.anim_db->find_body_part(
                oni_type->leg_body_parts[leg_idx * 2 + 1]
            );
        point spawn_coords =
            get_hitbox(leg_hole_bp_idx)->get_cur_pos(pos, angle);
        float spawn_angle =
            get_angle(pos, spawn_coords);
            
        //Create the Pikmin.
        pikmin* new_pikmin =
            (pikmin*)
            create_mob(
                game.mob_categories.get(MOB_CATEGORY_PIKMIN),
                spawn_coords, oni_type->pik_types[type_idx], spawn_angle,
                "maturity=" + i2s(cur_m)
            );
            
        //Set its data to start sliding.
        new_pikmin->fsm.set_state(PIKMIN_STATE_LEAVING_ONION, (void*) this);
        vector<size_t> checkpoints;
        checkpoints.push_back(leg_hole_bp_idx);
        checkpoints.push_back(leg_foot_bp_idx);
        new_pikmin->track_info =
            new track_info_struct(
            this, checkpoints, oni_type->pikmin_exit_speed
        );
        new_pikmin->leader_to_return_to = calling_leader;
        
        return true;
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Draws an Onion.
 */
void onion::draw_mob() {
    sprite* s_ptr = anim.get_cur_sprite();
    
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(s_ptr, &eff, true, true);
    
    eff.tint_color.a *= (seethrough / 255.0f);
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
}


/* ----------------------------------------------------------------------------
 * Returns how many Pikmin of the given type exist inside.
 */
size_t onion::get_amount_by_type(pikmin_type* type) {
    size_t amount = 0;
    for(size_t t = 0; t < oni_type->pik_types.size(); ++t) {
        if(oni_type->pik_types[t] == type) {
            for(size_t m = 0; m < N_MATURITIES; ++m) {
                amount += pikmin_inside[t][m];
            }
            break;
        }
    }
    return amount;
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 * svr:
 *   Script var reader to use.
 */
void onion::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    string pikmin_inside_var;
    
    if(svr.get("pikmin_inside", pikmin_inside_var)) {
        vector<string> pikmin_inside_vars = split(pikmin_inside_var);
        size_t word = 0;
        
        for(size_t t = 0; t < oni_type->pik_types.size(); ++t) {
            for(size_t m = 0; m < N_MATURITIES; ++m) {
                if(word < pikmin_inside_vars.size()) {
                    pikmin_inside[t][m] = s2i(pikmin_inside_vars[word]);
                    word++;
                }
            }
        }
    }
}


//Wait these many seconds before allowing another Pikmin to be called out.
const float ONION_CALL_INTERVAL = 0.028f;

/* ----------------------------------------------------------------------------
 * Requests that Pikmin of the given type get called out.
 * type_idx:
 *   Index of the type of Pikmin to call out, from the Onion's types.
 * amount:
 *   How many to call out.
 * l_ptr:
 *   Leader responsible.
 */
void onion::request_pikmin(
    const size_t type_idx, const size_t amount, leader* l_ptr
) {
    call_queue[type_idx] += amount;
    next_call_time = ONION_CALL_INTERVAL;
    calling_leader = l_ptr;
}


//An Onion-spat seed starts with this Z offset from the Onion.
const float ONION_NEW_SEED_Z_OFFSET = 320.0f;
//After spitting a seed, the next seed's angle shifts by this much.
const float ONION_SPEW_ANGLE_SHIFT = TAU * 0.12345;
//An Onion-spat seed is this quick, horizontally.
const float ONION_SPEW_H_SPEED = 80.0f;
//Deviate the seed's horizontal speed by this much, more or less.
const float ONION_SPEW_H_SPEED_DEVIATION = 10.0f;
//An Onion-spat seed is this quick, vertically.
const float ONION_SPEW_V_SPEED = 600.0f;


/* ----------------------------------------------------------------------------
 * Spew a Pikmin seed in the queue or add it to the Onion's storage.
 */
void onion::spew() {
    for(size_t t = 0; t < spew_queue.size(); ++t) {
        if(spew_queue[t] == 0) continue;
        
        spew_queue[t]--;
        
        unsigned total_after =
            game.states.gameplay->mobs.pikmin_list.size() + 1;
            
        if(total_after > game.config.max_pikmin_in_field) {
            pikmin_inside[t][0]++;
            return;
        }
        
        float horizontal_strength =
            ONION_SPEW_H_SPEED +
            randomf(
                -ONION_SPEW_H_SPEED_DEVIATION,
                ONION_SPEW_H_SPEED_DEVIATION
            );
        spew_pikmin_seed(
            pos, z + ONION_NEW_SEED_Z_OFFSET, oni_type->pik_types[t],
            next_spew_angle, horizontal_strength, ONION_SPEW_V_SPEED
        );
        
        next_spew_angle += ONION_SPEW_ANGLE_SHIFT;
        next_spew_angle = normalize_angle(next_spew_angle);
        
        return;
    }
}


/* ----------------------------------------------------------------------------
 * Temporary feature to allow Pikmin to be stowed in the Onion.
 * Stows away a Pikmin in the current leader's group, if possible.
 * Gives priority to the lower maturities.
 */
void onion::stow_pikmin() {
    //TODO delete this when the Onion menu is done.
    //Find a Pikmin of that type, preferring lower maturities.
    pikmin* pik_to_stow = NULL;
    size_t maturity = 0;
    for(; maturity < N_MATURITIES; ++maturity) {
        for(
            size_t p = 0;
            p < game.states.gameplay->cur_leader_ptr->group->members.size();
            ++p
        ) {
            mob* mob_ptr =
                game.states.gameplay->cur_leader_ptr->group->members[p];
            if(mob_ptr->type->category->id != MOB_CATEGORY_PIKMIN) {
                continue;
            }
            
            pikmin* p_ptr = (pikmin*) mob_ptr;
            if(p_ptr->maturity != maturity) continue;
            if(p_ptr->pik_type != oni_type->pik_types[0]) continue;
            
            pik_to_stow = p_ptr;
            break;
        }
        
        if(pik_to_stow) break;
    }
    
    if(!pik_to_stow) return;
    
    pik_to_stow->leave_group();
    pik_to_stow->to_delete = true;
    pikmin_inside[0][maturity]++;
}


/* ----------------------------------------------------------------------------
 * Ticks some logic specific to Onions.
 * delta_t:
 *   How many seconds to tick by.
 */
void onion::tick_class_specifics(const float delta_t) {
    if(calling_leader && calling_leader->to_delete) {
        calling_leader = NULL;
    }
    
    bool needs_to_spew = false;
    for(size_t t = 0; t < oni_type->pik_types.size(); ++t) {
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
        
        if(
            bbox_check(
                game.states.gameplay->cur_leader_ptr->pos, o_ptr->pos,
                game.states.gameplay->cur_leader_ptr->type->radius +
                o_ptr->type->radius * 3
            )
        ) {
            final_alpha = ONION_SEETHROUGH_ALPHA;
        }
        
        if(
            bbox_check(
                game.states.gameplay->leader_cursor_w, o_ptr->pos,
                game.states.gameplay->cur_leader_ptr->type->radius +
                o_ptr->type->radius * 3
            )
        ) {
            final_alpha = ONION_SEETHROUGH_ALPHA;
        }
        
        if(o_ptr->seethrough != final_alpha) {
            if(final_alpha < o_ptr->seethrough) {
                o_ptr->seethrough =
                    std::max(
                        (double) final_alpha,
                        (double) o_ptr->seethrough - ONION_FADE_SPEED * delta_t
                    );
            } else {
                o_ptr->seethrough =
                    std::min(
                        (double) final_alpha,
                        (double) o_ptr->seethrough + ONION_FADE_SPEED * delta_t
                    );
            }
        }
    }
    
    //Call out Pikmin, if the timer agrees.
    if(next_call_time > 0.0f) {
        next_call_time -= delta_t;
    }
    
    while(next_call_time < 0.0f) {
        size_t best_type = INVALID;
        size_t best_type_amount = 0;
        
        for(size_t t = 0; t < oni_type->pik_types.size(); ++t) {
            if(call_queue[t] == 0) continue;
            if(call_queue[t] > best_type_amount) {
                best_type = t;
                best_type_amount = call_queue[t];
            }
        }
        
        if(best_type != INVALID) {
            if(call_pikmin(best_type)) {
                call_queue[best_type]--;
            }
        }
        
        next_call_time += ONION_CALL_INTERVAL;
    }
}
