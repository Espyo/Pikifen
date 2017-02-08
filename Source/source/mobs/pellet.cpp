/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet class and pellet-related functions.
 */

#include "../drawing.h"
#include "../functions.h"
#include "pellet.h"

/* ----------------------------------------------------------------------------
 * Creates a pellet mob.
 */
pellet::pellet(
    float x, float y, pellet_type* type, const float angle, const string &vars
) :
    mob(x, y, type, angle, vars),
    pel_type(type) {
    
    become_carriable(false);
    
    set_animation(ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * Draws a pellet, with the number and all.
 */
void pellet::draw(sprite_effect_manager* effect_manager) {

    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    float draw_x, draw_y;
    float draw_w, draw_h, scale;
    get_sprite_center(this, s_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, s_ptr, &draw_w, &draw_h, &scale);
    
    sprite_effect_manager effects;
    add_brightness_sprite_effect(&effects);
    
    if(fsm.cur_state->id == PELLET_STATE_BEING_DELIVERED) {
        add_delivery_sprite_effect(
            &effects, script_timer.get_ratio_left(),
            ((onion*) carrying_target)->oni_type->pik_type->main_color
        );
    }
    
    draw_sprite_with_effects(
        s_ptr->bitmap,
        draw_x, draw_y,
        type->radius * 2.0, -1,
        angle, &effects
    );
    
    draw_sprite_with_effects(
        pel_type->bmp_number,
        draw_x, draw_y,
        type->radius, -1,
        0, &effects
    );
    
}
