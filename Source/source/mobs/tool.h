/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the tool class and tool-related functions.
 */

#ifndef TOOL_INCLUDED
#define TOOL_INCLUDED

#include <vector>

#include "../mob_types/tool_type.h"
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
class tool : public mob {

public:
    
    //--- Members ---

    //What type of tool it is.
    tool_type* too_type = nullptr;
    
    //Flags indicating if and how the mob can be held by other mobs.
    unsigned char holdability_flags = 0;
    
    //If a Pikmin is already reserved to get this tool, this points to it.
    pikmin* reserved = nullptr;
    
    
    //--- Function declarations ---
    
    tool(const point &pos, tool_type* type, const float angle);
    
};


#endif //ifndef TOOL_INCLUDED
