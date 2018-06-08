/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
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
    const point &pos, pellet_type* type, const float angle, const string &vars
) :
    mob(pos, type, angle, vars),
    pel_type(type) {
    
    become_carriable(false);
    
    set_animation(ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * Draws a pellet, with the number and all.
 */
void pellet::draw(bitmap_effect_manager* effect_manager) {

    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    point draw_pos = get_sprite_center(s_ptr);
    
    bitmap_effect_manager effects;
    add_sector_brightness_bitmap_effect(&effects);
    
    if(fsm.cur_state->id == PELLET_STATE_BEING_DELIVERED) {
        add_delivery_bitmap_effect(
            &effects, script_timer.get_ratio_left(),
            ((onion*) carrying_target)->oni_type->pik_type->main_color
        );
    }
    
    draw_bitmap_with_effects(
        s_ptr->bitmap,
        draw_pos,
        point(type->radius * 2.0, -1),
        angle + s_ptr->angle, &effects
    );
    
    draw_bitmap_with_effects(
        pel_type->bmp_number,
        draw_pos,
        point(type->radius, -1),
        0, &effects
    );
    
}
