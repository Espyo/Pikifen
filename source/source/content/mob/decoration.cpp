/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Decoration class and decoration related functions.
 */

#include "decoration.h"

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"


/**
 * @brief Constructs a new decoration object.
 *
 * @param pos Starting coordinates.
 * @param type Decoration type this mob belongs to.
 * @param angle Starting angle.
 */
Decoration::Decoration(
    const Point &pos, DecorationType* type, float angle
) :
    Mob(pos, type, angle),
    decType(type) {
    
    float tint_interpol_ratio = game.rng.f(0.0f, 1.0f);
    ALLEGRO_COLOR tint_limit = decType->tintRandomMaximum;
    tint_limit.a = 1.0f;
    
    individualTint =
        interpolateColor(
            tint_interpol_ratio, 0.0, 1.0,
            tint_limit, al_map_rgba(255, 255, 255, 255)
        );
        
    float alpha_interpol_ratio = game.rng.f(0.0f, 1.0f);
    individualTint.a =
        interpolateNumber(
            alpha_interpol_ratio, 0.0f, 1.0f,
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
    
    eff.tintColor.r *= individualTint.r;
    eff.tintColor.g *= individualTint.g;
    eff.tintColor.b *= individualTint.b;
    eff.tintColor.a *= individualTint.a;
    
    eff.scale *= individualScale;
    eff.rotation += individualRotation;
    
    drawBitmapWithEffects(cur_s_ptr->bitmap, eff);
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Decoration::readScriptVars(const ScriptVarReader &svr) {
    Mob::readScriptVars(svr);
    
    bool random_animation_delay_var;
    bool random_tint_var;
    bool random_scale_var;
    bool random_rotation_var;
    
    if(svr.get("random_animation_delay", random_animation_delay_var)) {
        individualRandomAnimDelay = random_animation_delay_var;
    }
    if(svr.get("random_tint", random_tint_var)) {
        if(!random_tint_var) {
            individualTint = COLOR_WHITE;
        }
    }
    if(svr.get("random_scale", random_scale_var)) {
        if(!random_scale_var) {
            individualScale = 1.0f;
        }
    }
    if(svr.get("random_rotation", random_rotation_var)) {
        if(!random_rotation_var) {
            individualRotation = 0.0f;
        }
    }
}
