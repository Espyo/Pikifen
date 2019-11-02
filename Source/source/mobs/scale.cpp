/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Scale class and scale-related functions.
 */

#include "scale.h"

#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a scale mob.
 */
scale::scale(const point &pos, scale_type* type, float angle) :
    mob(pos, type, angle),
    sca_type(type) {
    
    
}


/* ----------------------------------------------------------------------------
 * Calculates the total weight currently on top of the mob.
 */
float scale::calculate_cur_weight() {
    float w = 0;
    
    for(size_t m = 0; m < mobs.size(); ++m) {
        if(mobs[m]->standing_on_mob == this) {
            w += mobs[m]->type->weight;
        }
    }
    
    return w;
}
