/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pellet class and pellet-related functions.
 */

#pragma once

#include "../mob_types/pellet_type.h"
#include "../mob_types/pikmin_type.h"
#include "mob.h"


/**
 * @brief A pellet can be delivered to an Onion in
 * order to generate more Pikmin.
 * Delivering a pellet to the matching Onion
 * results in more Pikmin being created.
 */
class pellet : public mob {

public:
    
    //--- Members ---

    //What type of pellet it is.
    pellet_type* pel_type = nullptr;
    
    
    //--- Function declarations ---
    
    pellet(const point &pos, pellet_type* type, const float angle);
    void draw_mob() override;
    
};
