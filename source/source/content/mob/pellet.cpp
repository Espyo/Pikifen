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

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"


/**
 * @brief Constructs a new pellet object.
 *
 * @param pos Starting coordinates.
 * @param type Pellet type this mob belongs to.
 * @param angle Starting angle.
 */
Pellet::Pellet(const Point &pos, PelletType* type, float angle) :
    Mob(pos, type, angle),
    pelType(type) {
    
    becomeCarriable(CARRY_DESTINATION_ONION);
    
    setAnimation(
        MOB_TYPE::ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief Draws a pellet, with the number and all.
 */
void Pellet::drawMob() {
    Sprite* cur_s_ptr;
    Sprite* next_s_ptr;
    float interpolation_factor;
    getSpriteData(&cur_s_ptr, &next_s_ptr, &interpolation_factor);
    if(!cur_s_ptr) return;
    
    BitmapEffect eff;
    getSpriteBitmapEffects(
        cur_s_ptr, next_s_ptr, interpolation_factor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY |
        SPRITE_BMP_EFFECT_CARRY
    );
    
    Point bmp_size = getBitmapDimensions(cur_s_ptr->bitmap);
    eff.scale *= radius * 2.0f / bmp_size;
    
    drawBitmapWithEffects(cur_s_ptr->bitmap, eff);
    drawBitmapWithEffects(pelType->bmpNumber, eff);
}
