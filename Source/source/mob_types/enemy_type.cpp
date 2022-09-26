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
    allow_ground_attacks(true),
    points(10) {
    
    target_type = MOB_TARGET_TYPE_ENEMY;
    huntable_targets =
        MOB_TARGET_TYPE_ENEMY |
        MOB_TARGET_TYPE_PLAYER;
    hurtable_targets =
        MOB_TARGET_TYPE_ENEMY |
        MOB_TARGET_TYPE_PLAYER |
        MOB_TARGET_TYPE_FRAGILE;
        
    starting_team = MOB_TEAM_ENEMY_1;
    
    area_editor_prop_struct aep_spoils;
    aep_spoils.name = "Spoils";
    aep_spoils.var = "spoils";
    aep_spoils.type = AEMP_TEXT;
    aep_spoils.def_value = "";
    aep_spoils.tooltip =
        "What objects it drops upon defeat, separated by comma.";
    area_editor_props.push_back(aep_spoils);
    
    area_editor_prop_struct aep_pellets;
    aep_pellets.name = "Pellets";
    aep_pellets.var = "random_pellet_spoils";
    aep_pellets.type = AEMP_TEXT;
    aep_pellets.def_value = "";
    aep_pellets.tooltip =
        "What pellets it drops upon defeat, separated by comma. "
        "The color of each pellet is random (from the ones available in "
        "the area), but the number matches what you type. "
        "e.g.: \"1,1,5\" would spawn two 1 pellets and one 5 pellet.";
    area_editor_props.push_back(aep_pellets);
    
    add_carrying_states();
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void enemy_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("allow_ground_attacks", allow_ground_attacks);
    rs.set("drops_corpse", drops_corpse);
    rs.set("pikmin_seeds", pikmin_seeds);
    rs.set("points", points);
}
