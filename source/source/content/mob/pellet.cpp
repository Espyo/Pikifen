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
Pellet::Pellet(const Point& pos, PelletType* type, float angle) :
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
    Sprite* curSPtr;
    Sprite* nextSPtr;
    float interpolationFactor;
    getSpriteData(&curSPtr, &nextSPtr, &interpolationFactor);
    if(!curSPtr) return;
    
    BitmapEffect eff;
    getSpriteBitmapEffects(
        curSPtr, nextSPtr, interpolationFactor,
        &eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD |
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY |
        SPRITE_BMP_EFFECT_CARRY |
        (type->useDamageSquashAndStretch ? SPRITE_BMP_EFFECT_DAMAGE : 0)
    );
    
    Point bmpSize = getBitmapDimensions(curSPtr->bitmap);
    eff.tf.scale *= radius * 2.0f / bmpSize;
    
    drawBitmapWithEffects(curSPtr->bitmap, eff);

    if(pelType->drawNumber) {
        drawBitmapWithEffects(pelType->bmpNumber, eff);
    }
}
