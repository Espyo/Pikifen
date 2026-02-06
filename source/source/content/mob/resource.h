/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the resource class and resource-related functions.
 */

#pragma once

#include "../mob_type/resource_type.h"
#include "mob.h"
#include "pile.h"


/**
 * @brief A resource is any object that a single Pikmin can pick up, and deliver
 * somewhere else. It can optionally return to where the origin of the
 * resource came from.
 */
class Resource : public Mob {

public:

    //--- Public members ---
    
    //What type of resource it is.
    ResourceType* resType = nullptr;
    
    //Pile it belongs to, if any.
    Pile* originPile = nullptr;
    
    
    //--- Public function declarations ---
    
    Resource(const Point& pos, ResourceType* type, float angle);
    int getMissionPoints(bool* applicableInThisMission) const override;
    
};
