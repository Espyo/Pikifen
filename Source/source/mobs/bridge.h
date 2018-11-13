/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge class and bridge-related functions.
 */

#ifndef BRIDGE_INCLUDED
#define BRIDGE_INCLUDED

#include "../mob_types/bridge_type.h"
#include "mob.h"

enum BRIDGE_STATES {
    BRIDGE_STATE_IDLING,
    BRIDGE_STATE_DESTROYED,
    
    N_BRIDGE_STATES,
};


/* ----------------------------------------------------------------------------
 * A bridge mob. Bridges on the engine are made up of two parts:
 * the mob itself, which Pikmin damage, and the sectors Pikmin can walk on.
 * The sectors initially start as something else (normally ground at a
 * lower level, or some water), and when the bridge opens, they change
 * into walkable wood.
 */
class bridge : public mob {
private:

    void get_neighbor_bridge_sectors(sector* s_ptr);
    
public:
    bridge_type* bri_type;
    vector<sector*> secs;
    
    bridge(const point &pos, bridge_type* bri_type, const float angle);
    
};

#endif //ifndef BRIDGE_INCLUDED
