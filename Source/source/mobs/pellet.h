/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pellet class and pellet-related functions.
 */

#ifndef PELLET_INCLUDED
#define PELLET_INCLUDED

#include "mob.h"
#include "pellet_type.h"
#include "pikmin_type.h"

enum PELLET_STATES {
    PELLET_STATE_IDLE_WAITING,
    PELLET_STATE_IDLE_MOVING,
    PELLET_STATE_BEING_DELIVERED,
    
    N_PELLET_STATES,
};


/* ----------------------------------------------------------------------------
 * A pellet can be delivered to an Onion in
 * order to generate more Pikmin.
 * Delivering a pellet to the matching Onion
 * results in more Pikmin being created.
 */
class pellet : public mob {
public:
    pellet_type* pel_type;
    
    pellet(
        const point &pos, pellet_type* type,
        const float angle, const string &vars
    );
    virtual void draw(bitmap_effect_manager* effect_manager = NULL);
};

#endif //ifndef PELLET_INCLUDED
