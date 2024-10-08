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


/**
 * @brief Constructs a new pellet object.
 *
 * @param pos Starting coordinates.
 * @param type Pellet type this mob belongs to.
 * @param angle Starting angle.
 */
pellet::pellet(const point &pos, pellet_type* type, float angle) :
    mob(pos, type, angle),
    pel_type(type) {
    
    become_carriable(CARRY_DESTINATION_ONION);
    
    set_animation(
        MOB_TYPE::ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief Draws a pellet, with the number and all.
 */
void pellet::draw_mob() {
    sprite* cur_s_ptr;
    sprite* next_s_ptr;
    float interpolation_factor;
    get_sprite_data(&cur_s_ptr, &next_s_ptr, &interpolation_factor);
    if(!cur_s_ptr) return;
    
    bitmap_effect_t eff;
    get_sprite_bitmap_effects(
        cur_s_ptr, next_s_ptr, interpolation_factor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY |
        SPRITE_BMP_EFFECT_CARRY
    );
    
    eff.scale.x *= radius * 2.0 / al_get_bitmap_width(cur_s_ptr->bitmap);
    eff.scale.y *= radius * 2.0 / al_get_bitmap_height(cur_s_ptr->bitmap);
    
    draw_bitmap_with_effects(cur_s_ptr->bitmap, eff);
    draw_bitmap_with_effects(pel_type->bmp_number, eff);
}
