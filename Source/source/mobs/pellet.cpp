/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet class and pellet-related functions.
 */

#include "pellet.h"

#include "../drawing.h"
#include "../functions.h"


/* ----------------------------------------------------------------------------
 * Creates a pellet mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Pellet type this mob belongs to.
 * angle:
 *   Starting angle.
 */
pellet::pellet(const point &pos, pellet_type* type, const float angle) :
    mob(pos, type, angle),
    pel_type(type) {
    
    become_carriable(CARRY_DESTINATION_ONION);
    
    set_animation(ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * Draws a pellet, with the number and all.
 */
void pellet::draw_mob() {
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    ALLEGRO_COLOR delivery_color = map_gray(0);
    float delivery_time_ratio_left = LARGE_FLOAT;
    
    if(fsm.cur_state->id == ENEMY_EXTRA_STATE_BEING_DELIVERED) {
        delivery_color = delivery_info->color;
        delivery_time_ratio_left = script_timer.get_ratio_left();
    }
    
    get_sprite_bitmap_effects(
        s_ptr, &eff, true, true,
        delivery_time_ratio_left, delivery_color
    );
    
    eff.scale.x *= type->radius * 2.0 / al_get_bitmap_width(s_ptr->bitmap);
    eff.scale.y *= type->radius * 2.0 / al_get_bitmap_height(s_ptr->bitmap);
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
    draw_bitmap_with_effects(pel_type->bmp_number, eff);
}
