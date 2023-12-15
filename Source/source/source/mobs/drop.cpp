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

#include "../drawing.h"
#include "../game.h"


/* ----------------------------------------------------------------------------
 * Creates a drop mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Drop type this mob belongs to.
 * angle:
 *   Starting angle.
 */
drop::drop(const point &pos, drop_type* type, const float angle) :
    mob(pos, type, angle),
    dro_type(type),
    cur_scale(1.0),
    doses_left(dro_type->total_doses) {
    
    
}


/* ----------------------------------------------------------------------------
 * Draws a drop, but with its size reflecting the doses left or
 * the process of vanishing.
 */
void drop::draw_mob() {
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
    
    eff.scale *= cur_scale;
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void drop::tick_class_specifics(const float delta_t) {
    float intended_scale;
    
    if(doses_left == dro_type->total_doses) {
        intended_scale = 1.0f;
    } else if(doses_left == 0) {
        intended_scale = 0.0f;
    } else {
        intended_scale =
            interpolate_number(doses_left, 1, dro_type->total_doses, 0.5, 1.0);
    }
    
    if(cur_scale > intended_scale) {
        cur_scale -= dro_type->shrink_speed * delta_t;
        cur_scale = std::max(intended_scale, cur_scale);
    }
    
    if(cur_scale == 0) {
        //Disappeared into nothingness. Time to delete...if it's not being used.
        
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
            if(game.states.gameplay->mobs.all[m]->focused_mob == this) {
                return;
            }
        }
        
        to_delete = true;
    }
}
