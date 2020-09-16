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

#include "../game.h"


/* ----------------------------------------------------------------------------
 * Creates a scale mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Scale type this mob belongs to.
 * angle:
 *   Starting angle.
 */
scale::scale(const point &pos, scale_type* type, float angle) :
    mob(pos, type, angle),
    sca_type(type) {
    
    
}


/* ----------------------------------------------------------------------------
 * Calculates the total weight currently on top of the mob.
 */
float scale::calculate_cur_weight() const {

    //Start by figuring out which mobs are applying weight.
    set<mob*> weighing_mobs;
    
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
        mob* m_ptr = game.states.gameplay->mobs.all[m];
        
        if(m_ptr->standing_on_mob == this) {
            weighing_mobs.insert(m_ptr);
            for(size_t h = 0; h < m_ptr->holding.size(); ++h) {
                weighing_mobs.insert(m_ptr->holding[h]);
            }
        }
    }
    
    //Now, add up their weights.
    float w = 0;
    for(auto m : weighing_mobs) {
        w += m->type->weight;
    }
    
    return w;
}
