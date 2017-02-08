/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
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
    SHIP_STATE_IDLING,
    
    N_SHIP_STATES,
};


/* ----------------------------------------------------------------------------
 * A ship is where "treasure" is delivered to.
 */
class ship : public mob {
public:

    ship_type* shi_type;
    float beam_final_x;
    float beam_final_y;
    
    ship(float x, float y, ship_type* type, float angle, const string &vars);
    
    virtual void draw(sprite_effect_manager* effect_manager = NULL);
};

#endif //ifndef SHIP_INCLUDED
