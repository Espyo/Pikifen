/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship class and ship-related functions.
 */

#ifndef SHIP_INCLUDED
#define SHIP_INCLUDED

#include "../mob.h"

/*
 * A ship is where "treasure" is delivered to.
 */
class ship : public mob {
public:
    ship(float x, float y);
};

#endif //ifndef SHIP_INCLUDED