/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion class and Onion-related functions.
 */

#ifndef ONION_INCLUDED
#define ONION_INCLUDED

#include "mob.h"
#include "pikmin_type.h"
#include "onion_type.h"

/*
 * An Onion is where Pikmin are stored.
 */
class onion : public mob {
public:
    onion_type* oni_type;
    
    onion(float x, float y, onion_type* type, const float angle, const string &vars);
};

#endif //ifndef ONION_INCLUDED