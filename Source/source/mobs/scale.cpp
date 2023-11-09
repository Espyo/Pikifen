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
    sca_type(type),
    goal_number(type->goal_number) {
    
    
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
    for(auto &m : weighing_mobs) {
        w += m->type->weight;
    }
    
    return w;
}


/* ----------------------------------------------------------------------------
 * Returns information on how to show the fraction numbers.
 * Returns true if the fraction numbers should be shown, false if not.
 * This only keeps in mind things specific to this class, so it shouldn't
 * check for things like carrying, which is global to all mobs.
 * fraction_value_nr:
 *   The fraction's value (upper) number gets set here.
 * fraction_req_nr:
 *   The fraction's required (lower) number gets set here.
 * fraction_color:
 *   The fraction's color gets set here.
 */
bool scale::get_fraction_numbers_info(
    float* fraction_value_nr, float* fraction_req_nr,
    ALLEGRO_COLOR* fraction_color
) const {
    float weight = calculate_cur_weight();
    if(weight <= 0 || health <= 0) return false;
    *fraction_value_nr = weight;
    *fraction_req_nr = goal_number;
    *fraction_color = game.config.carrying_color_stop;
    return true;
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 * svr:
 *   Script var reader to use.
 */
void scale::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    svr.get("goal_number", goal_number);
}
