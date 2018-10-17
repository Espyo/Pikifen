/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy type class and enemy type-related functions.
 */

#include "enemy_type.h"
#include "../functions.h"
#include "../utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Creates a type of enemy.
 */
enemy_type::enemy_type() :
    mob_type(MOB_CATEGORY_ENEMIES),
    pikmin_seeds(0),
    value(0),
    revive_speed(0),
    is_boss(false),
    drops_corpse(true),
    allow_ground_attacks(true) {
    
    add_carrying_states();
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void enemy_type::load_parameters(data_node* file) {
    drops_corpse =
        s2b(
            file->get_child_by_name("drops_corpse")->get_value_or_default("yes")
        );
    is_boss = s2b(file->get_child_by_name("is_boss")->value);
    pikmin_seeds = s2i(file->get_child_by_name("pikmin_seeds")->value);
    revive_speed = s2f(file->get_child_by_name("revive_speed")->value);
    value = s2f(file->get_child_by_name("value")->value);
    allow_ground_attacks =
        s2b(
            file->get_child_by_name("allow_ground_attacks")
            ->get_value_or_default("true")
        );
        
}


enemy_type::~enemy_type() { }
