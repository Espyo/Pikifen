/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
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
    SHIP_STATE_IDLING,
    
    N_SHIP_STATES,
};


/* ----------------------------------------------------------------------------
 * A ship is where "treasure" is delivered to.
 */
class ship : public mob {
public:

    ship_type* shi_type;
    point beam_final_pos;
    
    ship(const point &pos, ship_type* type, float angle);
    
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    
    void heal_leader(leader* l);
    bool is_leader_under_ring(leader* l);
};

#endif //ifndef SHIP_INCLUDED
