/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop class and drop related functions.
 */

#include <algorithm>

#include "drop.h"

#include "../../core/drawing.h"
#include "../../core/game.h"


/**
 * @brief Constructs a new drop object.
 *
 * @param pos Starting coordinates.
 * @param type Drop type this mob belongs to.
 * @param angle Starting angle.
 */
Drop::Drop(const Point &pos, DropType* type, float angle) :
    Mob(pos, type, angle),
    droType(type),
    dosesLeft(droType->totalDoses) {
    
    
}


/**
 * @brief Draws a drop, but with its size reflecting the doses left or
 * the process of vanishing.
 */
void Drop::drawMob() {
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
        SPRITE_BMP_EFFECT_DELIVERY
    );
    
    eff.scale *= curScale;
    
    drawBitmapWithEffects(curSPtr->bitmap, eff);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Drop::tickClassSpecifics(float deltaT) {
    float intendedScale;
    
    if(dosesLeft == droType->totalDoses) {
        intendedScale = 1.0f;
    } else if(dosesLeft == 0) {
        intendedScale = 0.0f;
    } else {
        intendedScale =
            interpolateNumber(dosesLeft, 1, droType->totalDoses, 0.5, 1.0);
    }
    
    if(curScale > intendedScale) {
        curScale -= droType->shrinkSpeed * deltaT;
        curScale = std::max(intendedScale, curScale);
    }
    
    if(curScale == 0) {
        //Disappeared into nothingness. Time to delete...if it's not being used.
        
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            if(game.states.gameplay->mobs.all[m]->focusedMob == this) {
                return;
            }
        }
        
        toDelete = true;
    }
}
