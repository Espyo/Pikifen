/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge class and bridge-related functions.
 */

#ifndef BRIDGE_INCLUDED
#define BRIDGE_INCLUDED

#include "../mob_types/bridge_type.h"
#include "mob.h"


/* ----------------------------------------------------------------------------
 * A bridge mob. Bridges on the engine are made up of two parts:
 * the mob itself, which Pikmin damage, and the sectors Pikmin can walk on.
 * The sectors initially start as something else (normally ground at a
 * lower level, or some water), and when the bridge opens, they change
 * into walkable wood.
 */
class bridge : public mob {
public:
    //What type of bridge it is.
    bridge_type* bri_type;
    
    //Sectors it will affect when it opens.
    vector<sector*> secs;
    
    //Constructor.
    bridge(const point &pos, bridge_type* bri_type, const float angle);
};


#endif //ifndef BRIDGE_INCLUDED
