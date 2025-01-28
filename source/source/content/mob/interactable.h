/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the interactable class and interactable-related functions.
 */

#pragma once

#include <vector>

#include "../mob_type/interactable_type.h"
#include "mob.h"


/**
 * @brief A mob that the current leader can interact with.
 */
class interactable : public mob {

public:

    //--- Members ---
    
    //What type of interactable it is.
    interactable_type* int_type = nullptr;
    
    
    //--- Function declarations ---
    
    interactable(const point &pos, interactable_type* type, float angle);
    
};
