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

#include "../drawing.h"
#include "../functions.h"


/**
 * @brief Constructs a new decoration object.
 *
 * @param pos Starting coordinates.
 * @param type Decoration type this mob belongs to.
 * @param angle Starting angle.
 */
decoration::decoration(
    const point &pos, decoration_type* type, const float angle
) :
    mob(pos, type, angle),
    dec_type(type),
    individual_random_anim_delay(true),
    individual_tint(COLOR_WHITE),
    individual_scale(1.0f),
    individual_rotation(0.0f) {
    
    float tint_interpol_ratio = randomf(0.0f, 1.0f);
    ALLEGRO_COLOR tint_limit = dec_type->tint_random_maximum;
    tint_limit.a = 1.0f;
    
    individual_tint =
        interpolate_color(
            tint_interpol_ratio, 0.0, 1.0,
            tint_limit, al_map_rgba(255, 255, 255, 255)
        );
        
    float alpha_interpol_ratio = randomf(0.0f, 1.0f);
    individual_tint.a =
        interpolate_number(
            alpha_interpol_ratio, 0.0f, 1.0f,
            dec_type->tint_random_maximum.a, 1.0f
        );
        
    individual_rotation +=
        randomf(
            -dec_type->rotation_random_variation,
            dec_type->rotation_random_variation
        );
        
    individual_scale +=
        randomf(
            -dec_type->scale_random_variation,
            dec_type->scale_random_variation
        );
}


/**
 * @brief Draws a decorative object. This is responsible for randomly
 * tinting it, rotating it, etc.
 */
void decoration::draw_mob() {
    sprite* s_ptr = get_cur_sprite();
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(
        s_ptr, &eff,
        SPRITE_BITMAP_EFFECT_STANDARD |
        SPRITE_BITMAP_EFFECT_STATUS |
        SPRITE_BITMAP_EFFECT_SECTOR_BRIGHTNESS |
        SPRITE_BITMAP_EFFECT_HEIGHT |
        SPRITE_BITMAP_EFFECT_DELIVERY
    );
    
    eff.tint_color.r *= individual_tint.r;
    eff.tint_color.g *= individual_tint.g;
    eff.tint_color.b *= individual_tint.b;
    eff.tint_color.a *= individual_tint.a;
    
    eff.scale *= individual_scale;
    eff.rotation += individual_rotation;
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void decoration::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    bool random_animation_delay_var;
    bool random_tint_var;
    bool random_scale_var;
    bool random_rotation_var;
    
    if(svr.get("random_animation_delay", random_animation_delay_var)) {
        individual_random_anim_delay = random_animation_delay_var;
    }
    if(svr.get("random_tint", random_tint_var)) {
        if(!random_tint_var) {
            individual_tint = COLOR_WHITE;
        }
    }
    if(svr.get("random_scale", random_scale_var)) {
        if(!random_scale_var) {
            individual_scale = 1.0f;
        }
    }
    if(svr.get("random_rotation", random_rotation_var)) {
        if(!random_rotation_var) {
            individual_rotation = 0.0f;
        }
    }
}
