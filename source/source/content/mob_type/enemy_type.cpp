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

#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"


/**
 * @brief Constructs a new enemy type object.
 */
EnemyType::EnemyType() :
    MobType(MOB_CATEGORY_ENEMIES) {
    
    targetType = MOB_TARGET_FLAG_ENEMY;
    huntableTargets =
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_PLAYER;
    hurtableTargets =
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_FRAGILE;
    startingTeam = MOB_TEAM_ENEMY_1;
    useDamageSquashAndStretch = true;
    
    createAndAddCarryingStates();
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void EnemyType::loadCatProperties(DataNode* file) {
    ReaderSetter eRS(file);
    
    eRS.set("allow_ground_attacks", allowGroundAttacks);
    eRS.set("revive_time", reviveTime);
    eRS.set("pikmin_seeds", pikminSeeds);
    eRS.set("points", points);
}
