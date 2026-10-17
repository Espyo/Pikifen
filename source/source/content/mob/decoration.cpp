/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Decoration class and decoration related functions.
 */

#include "decoration.hpp"

#include "../../core/drawing.hpp"
#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../../util/allegro_utils.hpp"


/**
 * @brief Constructs a new decoration object.
 *
 * @param center Starting center coordinates.
 * @param type Decoration type this mob belongs to.
 * @param angle Starting angle.
 */
Decoration::Decoration(
    const Point& center, DecorationType* type, float angle
) :
    Mob(center, type, angle),
    decType(type) {
    
    float tintInterpolRatio = game.rng.f(0.0f, 1.0f);
    ALLEGRO_COLOR tintLimit = decType->tintRandomMaximum;
    tintLimit.a = 1.0f;
    
    individualTint =
        interpolateColor(
            tintInterpolRatio, 0.0, 1.0,
            tintLimit, COLOR_WHITE
        );
        
    float alphaInterpolRatio = game.rng.f(0.0f, 1.0f);
    individualTint.a =
        interpolateNumber(
            alphaInterpolRatio, 0.0f, 1.0f,
            decType->tintRandomMaximum.a, 1.0f
        );
        
    individualRotation +=
        game.rng.f(
            -decType->rotationRandomVariation,
            decType->rotationRandomVariation
        );
        
    individualScale +=
        game.rng.f(
            -decType->scaleRandomVariation,
            decType->scaleRandomVariation
        );
}


/**
 * @brief Draws a decorative object. This is responsible for randomly
 * tinting it, rotating it, etc.
 */
void Decoration::drawMob() {
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
        SPRITE_BMP_EFFECT_FLAG_DELIVERY |
        SPRITE_BMP_EFFECT_FLAG_PEEK_UNDERNEATH |
        (type->useDamageSquashAndStretch ? SPRITE_BMP_EFFECT_FLAG_DAMAGE : 0)
    );
    
    eff.tintColor.r *= individualTint.r;
    eff.tintColor.g *= individualTint.g;
    eff.tintColor.b *= individualTint.b;
    eff.tintColor.a *= individualTint.a;
    
    eff.tf.scale *= individualScale;
    
    drawBitmapWithEffects(curSPtr->bitmap, eff);
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param varsMgr Script var manager to use.
 */
void Decoration::readScriptVars(const ScriptVarManager& varsMgr) {
    Mob::readScriptVars(varsMgr);
    
    bool randomAnimationDelayVar;
    bool randomTintVar;
    bool randomScaleVar;
    bool randomRotationVar;
    
    if(varsMgr.getValue("random_animation_delay", randomAnimationDelayVar)) {
        individualRandomAnimDelay = randomAnimationDelayVar;
    }
    if(varsMgr.getValue("random_tint", randomTintVar)) {
        if(!randomTintVar) {
            individualTint = COLOR_WHITE;
        }
    }
    if(varsMgr.getValue("random_scale", randomScaleVar)) {
        if(!randomScaleVar) {
            individualScale = 1.0f;
        }
    }
    if(varsMgr.getValue("random_rotation", randomRotationVar)) {
        if(!randomRotationVar) {
            individualRotation = 0.0f;
        }
    }
}
