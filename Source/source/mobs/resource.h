/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the resource class and resource-related functions.
 */

#ifndef RESOURCE_INCLUDED
#define RESOURCE_INCLUDED

#include "../mob_types/resource_type.h"
#include "mob.h"
#include "pile.h"


/**
 * @brief A resource is any object that a single Pikmin can pick up, and deliver
 * somewhere else. It can optionally return to where the origin of the
 * resource came from.
 */
class resource : public mob {

public:
    
    //--- Members ---

    //What type of resource it is.
    resource_type* res_type;
    
    //Pile it belongs to, if any.
    pile* origin_pile;
    

    //--- Function declarations ---
    
    resource(const point &pos, resource_type* type, const float angle);
    
};


#endif //ifndef RESOURCE_INCLUDED
