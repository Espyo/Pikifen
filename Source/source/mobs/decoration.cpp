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


/* ----------------------------------------------------------------------------
 * Creates a decoration mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Decoration type this mob belongs to.
 * angle:
 *   Starting angle.
 */
decoration::decoration(
    const point &pos, decoration_type* type, const float angle
) :
    mob(pos, type, angle),
    dec_type(type),
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


/* ----------------------------------------------------------------------------
 * Draws a decorative object. This is responsible for randomly tinting it,
 * rotating it, etc.
 */
void decoration::draw_mob() {
    sprite* s_ptr = get_cur_sprite();
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(s_ptr, &eff, true, true, true);
    
    eff.tint_color.r *= individual_tint.r;
    eff.tint_color.g *= individual_tint.g;
    eff.tint_color.b *= individual_tint.b;
    eff.tint_color.a *= individual_tint.a;
    
    eff.scale *= individual_scale;
    eff.rotation += individual_rotation;
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
}
