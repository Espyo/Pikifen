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
    
    set_animation(
        MOB_TYPE::ANIM_IDLING, true, START_ANIMATION_RANDOM_TIME_ON_SPAWN
    );
}


/* ----------------------------------------------------------------------------
 * Draws a pellet, with the number and all.
 */
void pellet::draw_mob() {
    sprite* s_ptr = get_cur_sprite();
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(
        s_ptr, &eff,
        SPRITE_BITMAP_EFFECT_STANDARD |
        SPRITE_BITMAP_EFFECT_STATUS |
        SPRITE_BITMAP_EFFECT_SECTOR_BRIGHTNESS |
        SPRITE_BITMAP_EFFECT_HEIGHT |
        SPRITE_BITMAP_EFFECT_DELIVERY |
        SPRITE_BITMAP_EFFECT_CARRY
    );
    
    eff.scale.x *= radius * 2.0 / al_get_bitmap_width(s_ptr->bitmap);
    eff.scale.y *= radius * 2.0 / al_get_bitmap_height(s_ptr->bitmap);
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
    draw_bitmap_with_effects(pel_type->bmp_number, eff);
}
