/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship class and ship-related functions.
 */

#ifndef SHIP_INCLUDED
#define SHIP_INCLUDED

#include "mob.h"
#include "ship_type.h"


enum SHIP_STATES {
    SHIP_STATE_IDLE,
    
    N_SHIP_STATES,
};


/* ----------------------------------------------------------------------------
 * A ship is where "treasure" is delivered to.
 */
class ship : public mob {
public:

    ship_type* shi_type;
    
    ship(float x, float y, ship_type* type, float angle, const string &vars);
    
    virtual void draw();
    
    static void fsm_receive_mob(mob* m, void* info1, void* info2);
};

#endif //ifndef SHIP_INCLUDED
