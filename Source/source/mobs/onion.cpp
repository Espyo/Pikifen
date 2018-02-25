/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion class and Onion-related functions.
 */

#include <algorithm>

#include "../drawing.h"
#include "../functions.h"
#include "../geometry_utils.h"
#include "onion.h"
#include "../vars.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Creates an Onion mob.
 */
onion::onion(
    const point &pos, onion_type* type, const float angle, const string &vars
) :
    mob(pos, type, angle, vars),
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
    
    set_animation(ANIM_IDLING);
}


//An Onion-spat seed is this quick, horizontally.
const float ONION_SPEW_H_SPEED = 80.0f;
//Deviate the seed's horizontal speed by this much, more or less.
const float ONION_SPEW_H_SPEED_DEVIATION = 10.0f;
//An Onion-spat seed is this quick, vertically.
const float ONION_SPEW_V_SPEED = 600.0f;
//An Onion-spat seed starts with this Z offset from the Onion.
const float NEW_SEED_Z_OFFSET = 320.0f;

/* ----------------------------------------------------------------------------
 * Spew a Pikmin seed in the queue or add it to the Onion's storage.
 */
void onion::spew() {
    if(spew_queue == 0) return;
    spew_queue--;
    
    unsigned total_after = pikmin_list.size() + 1;
    
    if(total_after > max_pikmin_in_field) {
        pikmin_in_onions[oni_type->pik_type]++;
        return;
    }
    
    pikmin* new_pikmin =
        (
            (pikmin*)
            create_mob(
                mob_categories.get(MOB_CATEGORY_PIKMIN),
                pos, oni_type->pik_type, next_spew_angle, ""
            )
        );
    float horizontal_strength =
        ONION_SPEW_H_SPEED +
        randomf(-ONION_SPEW_H_SPEED_DEVIATION, ONION_SPEW_H_SPEED_DEVIATION);
    new_pikmin->z = z + NEW_SEED_Z_OFFSET;
    new_pikmin->speed.x = cos(next_spew_angle) * horizontal_strength;
    new_pikmin->speed.y = sin(next_spew_angle) * horizontal_strength;
    new_pikmin->speed_z = ONION_SPEW_V_SPEED;
    new_pikmin->fsm.set_state(PIKMIN_STATE_SEED);
    new_pikmin->maturity = 0;
    
    next_spew_angle += ONION_SPEW_ANGLE_SHIFT;
    
}


/* ----------------------------------------------------------------------------
 * Ticks some logic specific to Onions.
 */
void onion::tick_class_specifics() {
    for(size_t o = 0; o < onions.size(); ++o) {
        onion* o_ptr = onions[o];
        
        if(o_ptr->spew_queue != 0) {
        
            o_ptr->full_spew_timer.tick(delta_t);
            o_ptr->next_spew_timer.tick(delta_t);
            
        }
        
        unsigned char final_alpha = 255;
        
        if(
            bbox_check(
                cur_leader_ptr->pos, o_ptr->pos,
                cur_leader_ptr->type->radius + o_ptr->type->radius * 3
            )
        ) {
            final_alpha = ONION_SEETHROUGH_ALPHA;
        }
        
        if(
            bbox_check(
                leader_cursor_w, o_ptr->pos,
                cur_leader_ptr->type->radius + o_ptr->type->radius * 3
            )
        ) {
            final_alpha = ONION_SEETHROUGH_ALPHA;
        }
        
        if(o_ptr->seethrough != final_alpha) {
            if(final_alpha < o_ptr->seethrough) {
                o_ptr->seethrough =
                    max(
                        (double) final_alpha,
                        o_ptr->seethrough - ONION_FADE_SPEED * delta_t
                    );
            } else {
                o_ptr->seethrough =
                    min(
                        (double) final_alpha,
                        o_ptr->seethrough + ONION_FADE_SPEED * delta_t
                    );
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Draws an Onion.
 */
void onion::draw(sprite_effect_manager* effect_manager) {
    sprite* s_ptr = anim.get_cur_sprite();
    
    if(!s_ptr) return;
    
    point draw_pos = get_sprite_center(s_ptr);
    point draw_size = get_sprite_dimensions(s_ptr);
    
    sprite_effect_manager effects;
    add_sector_brightness_sprite_effect(&effects);
    
    sprite_effect seethrough_effect;
    sprite_effect_props seethrough_effect_props;
    seethrough_effect_props.tint_color = al_map_rgba(255, 255, 255, seethrough);
    seethrough_effect.add_keyframe(0, seethrough_effect_props);
    effects.add_effect(seethrough_effect);
    
    draw_bitmap_with_effects(
        s_ptr->bitmap,
        draw_pos, draw_size,
        angle, &effects
    );
}
