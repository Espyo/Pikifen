/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship class and ship-related functions.
 */

#ifndef SHIP_INCLUDED
#define SHIP_INCLUDED

#include "mob.h"

#include "../mob_types/ship_type.h"
#include "leader.h"


/* ----------------------------------------------------------------------------
 * A ship is where "treasure" is delivered to.
 */
class ship : public mob {
public:
    //What type of ship it is.
    ship_type* shi_type;
    
    //The beam's absolute coordinates.
    point beam_final_pos;
    
    //Heal up a leader.
    void heal_leader(leader* l);
    //Checks if a leader is under the beam.
    bool is_leader_under_beam(leader* l);
    
    //Constructor.
    ship(const point &pos, ship_type* type, float angle);
    
    //Mob drawing routine.
    virtual void draw_mob();
};

#endif //ifndef SHIP_INCLUDED
