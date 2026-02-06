/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the tool class and tool-related functions.
 */

#pragma once

#include <vector>

#include "../mob_type/tool_type.h"
#include "mob.h"
#include "pikmin.h"


//Flags that control how it can be held.
enum HOLDABILITY_FLAG {

    //The mob can be held by Pikmin.
    HOLDABILITY_FLAG_PIKMIN = 1 << 0,
    
    //The mob can be held by enemies.
    HOLDABILITY_FLAG_ENEMIES = 1 << 1,
    
};


/**
 * @brief A tool for Pikmin.
 * This is anything that a Pikmin can carry to use at a later date.
 */
class Tool : public Mob {

public:

    //--- Public members ---
    
    //What type of tool it is.
    ToolType* tooType = nullptr;
    
    //Flags indicating if and how the mob can be held by other mobs.
    unsigned char holdabilityFlags = 0;
    
    //If a Pikmin is already reserved to get this tool, this points to it.
    Pikmin* reserved = nullptr;
    
    
    //--- Public function declarations ---
    
    Tool(const Point& pos, ToolType* type, float angle);
    
};
