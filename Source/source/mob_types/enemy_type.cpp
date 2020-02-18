/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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
    drops_corpse(true),
    allow_ground_attacks(true) {
    
    target_type = MOB_TARGET_TYPE_ENEMY;
    huntable_targets =
        MOB_TARGET_TYPE_ENEMY |
        MOB_TARGET_TYPE_PLAYER;
    hurtable_targets =
        MOB_TARGET_TYPE_ENEMY |
        MOB_TARGET_TYPE_PLAYER |
        MOB_TARGET_TYPE_FRAGILE;
        
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
    pikmin_seeds = s2i(file->get_child_by_name("pikmin_seeds")->value);
    allow_ground_attacks =
        s2b(
            file->get_child_by_name("allow_ground_attacks")
            ->get_value_or_default("true")
        );
        
}


enemy_type::~enemy_type() { }
