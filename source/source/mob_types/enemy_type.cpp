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


/**
 * @brief Constructs a new enemy type object.
 */
enemy_type::enemy_type() :
    mob_type(MOB_CATEGORY_ENEMIES) {
    
    target_type = MOB_TARGET_FLAG_ENEMY;
    huntable_targets =
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_PLAYER;
    hurtable_targets =
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_FRAGILE;
        
    starting_team = MOB_TEAM_ENEMY_1;
    
    add_carrying_states();
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void enemy_type::load_cat_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("allow_ground_attacks", allow_ground_attacks);
    rs.set("drops_corpse", drops_corpse);
    rs.set("pikmin_seeds", pikmin_seeds);
    rs.set("points", points);
    rs.set("is_boss", is_boss);
}
