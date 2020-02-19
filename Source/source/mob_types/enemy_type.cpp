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
    reader_setter rs(file);
    
    rs.set("allow_ground_attacks", allow_ground_attacks);
    rs.set("drops_corpse", drops_corpse);
    rs.set("pikmin_seeds", pikmin_seeds);
}


enemy_type::~enemy_type() { }
