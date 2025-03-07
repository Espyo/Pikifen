/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bouncer class and bouncer-related functions.
 */

#pragma once

#include <vector>

#include "../mob_type/bouncer_type.h"
#include "mob.h"


/**
 * @brief An object that throws another mob, bouncing it away.
 */
class Bouncer : public Mob {

public:

    //--- Members ---
    
    //What type of bouncer it is.
    BouncerType* bou_type = nullptr;
    
    
    //--- Function declarations ---
    
    Bouncer(const Point &pos, BouncerType* type, float angle);
    
};
