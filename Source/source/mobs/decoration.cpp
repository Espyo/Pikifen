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
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a decoration mob.
 */
decoration::decoration(
    const point &pos, decoration_type* dec_type, const float angle
) :
    mob(pos, dec_type, angle),
    dec_type(dec_type),
    individual_scale(1.0f),
    individual_rotation(0.0f),
    has_done_first_animation(false) {
    
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
void decoration::draw_mob(bitmap_effect_manager* effect_manager) {

    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    point draw_pos = get_sprite_center(s_ptr);
    point draw_size = get_sprite_dimensions(s_ptr);
    
    bitmap_effect_manager effects;
    add_sector_brightness_bitmap_effect(&effects);
    
    bitmap_effect individual_randomness_effect;
    bitmap_effect_props props;
    props.tint_color = individual_tint;
    props.scale = point(individual_scale, individual_scale);
    props.rotation = individual_rotation;
    individual_randomness_effect.add_keyframe(0, props);
    effects.add_effect(individual_randomness_effect);
    
    draw_bitmap_with_effects(
        s_ptr->bitmap,
        draw_pos, draw_size,
        angle + s_ptr->angle, &effects
    );
    
}
