/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
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


/* ----------------------------------------------------------------------------
 * A pellet can be delivered to an Onion in
 * order to generate more Pikmin.
 * Delivering a pellet to the matching Onion
 * results in more Pikmin being created.
 */
class pellet : public mob {
public:
    pellet_type* pel_type;
    
    pellet(float x, float y, pellet_type* type, const float angle, const string &vars);
    virtual void draw();
};

#endif //ifndef PELLET_INCLUDED
