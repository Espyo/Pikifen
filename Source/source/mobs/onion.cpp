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
#include "../vars.h"

using std::size_t;
using std::string;

/* ----------------------------------------------------------------------------
 * Creates an Onion mob.
 */
onion::onion(const point &pos, onion_type* type, const float angle) :
    mob(pos, type, angle),
    oni_type(type),
    activated(true),
    spew_queue(0),
    full_spew_timer(ONION_FULL_SPEW_DELAY),
    next_spew_timer(ONION_NEXT_SPEW_DELAY),
    next_spew_angle(0),
    seethrough(255) {
    
    //Increase its Z by one so that mobs that walk at
    //ground level next to it will appear under it.
    gravity_mult = 0.0f;
    z++;
    
    full_spew_timer.on_end =
    [this] () { next_spew_timer.start(); };
    next_spew_timer.on_end =
    [this] () {
        if(spew_queue == 0) return; next_spew_timer.start(); spew();
    };
    
    for(size_t m = 0; m < N_MATURITIES; ++m) {
        pikmin_inside[m] = 0;
    }
    
    set_animation(ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * Temporary feature to allow Pikmin to be called from the Onion.
 * Calls out a Pikmin from inside the Onion, if possible.
 * Gives priority to the higher maturities.
 */
void onion::call_pikmin() {

    if(pikmin_list.size() >= max_pikmin_in_field) return;
    
    for(size_t m = 0; m < N_MATURITIES; ++m) {
        //Let's check the maturities in reverse order.
        size_t cur_m = N_MATURITIES - m - 1;
        
        if(pikmin_inside[cur_m] == 0) continue;
        
        pikmin_inside[cur_m]--;
        create_mob(
            mob_categories.get(MOB_CATEGORY_PIKMIN),
            pos, oni_type->pik_type, 0,
            "maturity=" + i2s(cur_m)
        );
        
        return;
    }
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
 * Reads the provided script variables, if any, and does stuff with them.
 */
void onion::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    string pikmin_inside_var;
    
    if(svr.get("pikmin_inside", pikmin_inside_var)) {
        vector<string> pikmin_inside_vars = split(pikmin_inside_var);
        
        for(
            size_t m = 0; m < pikmin_inside_vars.size() && m < N_MATURITIES;
            ++m
        ) {
            pikmin_inside[m] = s2i(pikmin_inside_vars[m]);
        }
    }
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
    if(spew_queue == 0) return;
    spew_queue--;
    
    unsigned total_after = pikmin_list.size() + 1;
    
    if(total_after > max_pikmin_in_field) {
        pikmin_inside[0]++;
        return;
    }
    
    float horizontal_strength =
        ONION_SPEW_H_SPEED +
        randomf(-ONION_SPEW_H_SPEED_DEVIATION, ONION_SPEW_H_SPEED_DEVIATION);
    spew_pikmin_seed(
        pos, z + ONION_NEW_SEED_Z_OFFSET, oni_type->pik_type,
        next_spew_angle, horizontal_strength, ONION_SPEW_V_SPEED
    );
    
    next_spew_angle += ONION_SPEW_ANGLE_SHIFT;
    next_spew_angle = normalize_angle(next_spew_angle);
}


/* ----------------------------------------------------------------------------
 * Temporary feature to allow Pikmin to be stowed in the Onion.
 * Stows away a Pikmin in the current leader's group, if possible.
 * Gives priority to the lower maturities.
 */
void onion::stow_pikmin() {
    //Find a Pikmin of that type, preferring lower maturities.
    pikmin* pik_to_stow = NULL;
    size_t maturity = 0;
    for(; maturity < N_MATURITIES; ++maturity) {
        for(
            size_t p = 0;
            p < game.gameplay_state->cur_leader_ptr->group->members.size();
            ++p
        ) {
            mob* mob_ptr =
                game.gameplay_state->cur_leader_ptr->group->members[p];
            if(mob_ptr->type->category->id != MOB_CATEGORY_PIKMIN) {
                continue;
            }
            
            pikmin* p_ptr = (pikmin*) mob_ptr;
            if(p_ptr->maturity != maturity) continue;
            if(p_ptr->pik_type != oni_type->pik_type) continue;
            
            pik_to_stow = p_ptr;
            break;
        }
        
        if(pik_to_stow) break;
    }
    
    if(!pik_to_stow) return;
    
    pik_to_stow->leave_group();
    pik_to_stow->to_delete = true;
    pikmin_inside[maturity]++;
}


/* ----------------------------------------------------------------------------
 * Ticks some logic specific to Onions.
 */
void onion::tick_class_specifics(const float delta_t) {
    for(size_t o = 0; o < onions.size(); ++o) {
        onion* o_ptr = onions[o];
        
        if(o_ptr->spew_queue != 0) {
        
            o_ptr->full_spew_timer.tick(delta_t);
            o_ptr->next_spew_timer.tick(delta_t);
            
        }
        
        unsigned char final_alpha = 255;
        
        if(
            bbox_check(
                game.gameplay_state->cur_leader_ptr->pos, o_ptr->pos,
                game.gameplay_state->cur_leader_ptr->type->radius +
                o_ptr->type->radius * 3
            )
        ) {
            final_alpha = ONION_SEETHROUGH_ALPHA;
        }
        
        if(
            bbox_check(
                game.gameplay_state->leader_cursor_w, o_ptr->pos,
                game.gameplay_state->cur_leader_ptr->type->radius +
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
}
