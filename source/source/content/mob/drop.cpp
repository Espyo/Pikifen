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
        SPRITE_BMP_EFFECT_DELIVERY
    );
    
    eff.scale *= curScale;
    
    drawBitmapWithEffects(cur_s_ptr->bitmap, eff);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Drop::tickClassSpecifics(float delta_t) {
    float intended_scale;
    
    if(dosesLeft == droType->totalDoses) {
        intended_scale = 1.0f;
    } else if(dosesLeft == 0) {
        intended_scale = 0.0f;
    } else {
        intended_scale =
            interpolateNumber(dosesLeft, 1, droType->totalDoses, 0.5, 1.0);
    }
    
    if(curScale > intended_scale) {
        curScale -= droType->shrinkSpeed * delta_t;
        curScale = std::max(intended_scale, curScale);
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
