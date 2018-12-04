/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the resource class and resource-related functions.
 */

#ifndef RESOURCE_INCLUDED
#define RESOURCE_INCLUDED

#include "mob.h"
#include "pile.h"
#include "../mob_types/resource_type.h"

enum RESOURCE_STATES {
    RESOURCE_STATE_IDLE_WAITING,
    RESOURCE_STATE_IDLE_MOVING,
    RESOURCE_STATE_BEING_DELIVERED,
    
    N_RESOURCE_STATES,
};


/* ----------------------------------------------------------------------------
 * A resource is any object that a single Pikmin can pick up, and deliver
 * somewhere else. It can optionally return to where the origin of the
 * resource came from.
 */
class resource : public mob {
public:

    resource_type* res_type;
    pile* origin_pile;
    
    resource(const point &pos, resource_type* type, const float angle);
    void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    void vanish();
};

#endif //ifndef RESOURCE_INCLUDED
