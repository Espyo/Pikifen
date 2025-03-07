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

#include "../../core/game.h"


/**
 * @brief Constructs a new scale object.
 *
 * @param pos Starting coordinates.
 * @param type Scale type this mob belongs to.
 * @param angle Starting angle.
 */
Scale::Scale(const Point &pos, ScaleType* type, float angle) :
    Mob(pos, type, angle),
    sca_type(type),
    goal_number(type->goal_number) {
    
    
}


/**
 * @brief Calculates the total weight currently on top of the mob.
 *
 * @return The weight.
 */
float Scale::calculate_cur_weight() const {

    //Start by figuring out which mobs are applying weight.
    set<Mob*> weighing_mobs;
    
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* m_ptr = game.states.gameplay->mobs.all[m];
        
        if(m_ptr->standing_on_mob == this) {
            weighing_mobs.insert(m_ptr);
            for(size_t h = 0; h < m_ptr->holding.size(); h++) {
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


/**
 * @brief Returns information on how to show the fraction numbers.
 * This only keeps in mind things specific to this class, so it shouldn't
 * check for things like carrying, which is global to all mobs.
 *
 * @param fraction_value_nr The fraction's value (upper) number gets set here.
 * @param fraction_req_nr The fraction's required (lower) number gets set here.
 * @param fraction_color The fraction's color gets set here.
 * @return Whether the numbers should be shown.
 */
bool Scale::get_fraction_numbers_info(
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


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Scale::read_script_vars(const ScriptVarReader &svr) {
    Mob::read_script_vars(svr);
    
    svr.get("goal_number", goal_number);
}
