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
    //Red color's index moves these many units per second.
    //(Green is faster and blue is faster still).
    static const unsigned int SHIP_BEAM_RING_COLOR_SPEED;
    
    //What type of ship it is.
    ship_type* shi_type;
    
    //Nest data.
    pikmin_nest_struct* nest;
    
    //The beam's absolute coordinates.
    point beam_final_pos;
    //Current color of the beam ring.
    unsigned char beam_ring_color[3];
    //Is the beam ring's color component going up or down?
    bool beam_ring_color_up[3];
    
    //Heal up a leader.
    void heal_leader(leader* l) const;
    //Checks if a leader is under the beam.
    bool is_leader_under_beam(leader* l) const;
    
    //Constructor.
    ship(const point &pos, ship_type* type, float angle);
    //Destructor.
    ~ship();
    
    //Mob drawing routine.
    virtual void draw_mob();
    //Read script variables from the area data.
    virtual void read_script_vars(const script_var_reader &svr);
    //Tick class-specific logic.
    virtual void tick_class_specifics(const float delta_t);
};


#endif //ifndef SHIP_INCLUDED
