/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the enemy type class and enemy type-related functions.
 */

#pragma once

#include <string>
#include <vector>

#include "../../lib/data_file/data_file.h"
#include "mob_type.h"


using std::size_t;


/**
 * @brief A type of enemy. A species, if you will.
 * Red Bulborb, Orange Bulborb, Cloaking Burrow-nit, etc.
 */
class EnemyType : public MobType {

public:

    //--- Members ---
    
    //How many Pikmin seeds are generated by delivering to an Onion?
    size_t pikminSeeds = 0;
    
    //How long after death until the enemy revives. 0 for no revival.
    float reviveTime = 0;
    
    //Can Pikmin perform grounded attacks on it?
    bool allowGroundAttacks = true;
    
    //Points worth for missions.
    size_t points = 10;
    
    
    //--- Function declarations ---
    
    EnemyType();
    void loadCatProperties(DataNode* file) override;
    
};
