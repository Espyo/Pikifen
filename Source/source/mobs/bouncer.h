/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bouncer class and bouncer-related functions.
 */

#ifndef BOUNCER_INCLUDED
#define BOUNCER_INCLUDED

#include <vector>

#include "../mob_types/bouncer_type.h"
#include "mob.h"


/**
 * @brief An object that throws another mob, bouncing it away.
 */
class bouncer : public mob {

public:
    
    //--- Members ---

    //What type of bouncer it is.
    bouncer_type* bou_type = nullptr;
    
    
    //--- Function declarations ---

    bouncer(const point &pos, bouncer_type* type, const float angle);
    
};


#endif //ifndef BOUNCER_INCLUDED
