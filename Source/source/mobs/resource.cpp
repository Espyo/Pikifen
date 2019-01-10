/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource class and resource-related functions.
 */

#include "../drawing.h"
#include "resource.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a resource.
 */
resource::resource(const point &pos, resource_type* type, const float angle) :
    mob(pos, type, angle),
    res_type(type),
    origin_pile(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Draws a resource.
 */
void resource::draw_mob(bitmap_effect_manager* effect_manager) {
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    point draw_pos = get_sprite_center(s_ptr);
    
    bitmap_effect_manager effects;
    add_sector_brightness_bitmap_effect(&effects);
    
    if(fsm.cur_state->id == RESOURCE_STATE_BEING_DELIVERED) {
        add_delivery_bitmap_effect(
            &effects, script_timer.get_ratio_left(),
            carrying_color_move
        );
    }
    
    draw_bitmap_with_effects(
        s_ptr->bitmap,
        draw_pos,
        point(type->radius * 2.0, -1),
        angle + s_ptr->angle, &effects
    );
}
