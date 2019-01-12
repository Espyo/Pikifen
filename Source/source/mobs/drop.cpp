/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop class and drop related functions.
 */

#include "drop.h"

#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a drop mob.
 */
drop::drop(const point &pos, drop_type* dro_type, const float angle) :
    mob(pos, dro_type, angle),
    dro_type(dro_type),
    cur_scale(1.0),
    doses_left(dro_type->total_doses) {
    
    
}


/* ----------------------------------------------------------------------------
 * Draws a drop, but with its size reflecting the doses left or
 * the process of vanishing.
 */
void drop::draw_mob(bitmap_effect_manager* effect_manager) {
    bitmap_effect_manager internal_manager;
    if(!effect_manager) {
        effect_manager = &internal_manager;
    }
    
    bitmap_effect_props shrink_effect_props;
    shrink_effect_props.scale.x = cur_scale;
    shrink_effect_props.scale.y = cur_scale;
    
    bitmap_effect shrink_effect;
    shrink_effect.add_keyframe(0, shrink_effect_props);
    
    effect_manager->add_effect(shrink_effect);
    
    mob::draw_mob(effect_manager);
}


/* ----------------------------------------------------------------------------
 * Ticks some logic specific to drops.
 */
void drop::tick_class_specifics() {
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
        cur_scale = max(intended_scale, cur_scale);
    }
    
    if(cur_scale == 0) {
        //Disappeared into nothingness. Time to delete...if it's not being used.
        
        for(size_t m = 0; m < mobs.size(); ++m) {
            if(mobs[m]->focused_mob == this) {
                return;
            }
        }
        
        to_delete = true;
    }
}
