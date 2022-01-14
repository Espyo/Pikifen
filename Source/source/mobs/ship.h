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
    //The amount of rings the ship's interaction point has.
    static const unsigned char SHIP_CONTROL_POINT_RING_AMOUNT;
    //Animate the interaction point's ring for this long.
    static const float SHIP_CONTROL_POINT_ANIM_DUR;
    
    //How often the tractor beam generates a ring.
    static const float SHIP_TRACTOR_BEAM_EMIT_RATE;
    //Animate the tractor's ring for this long.
    static const float SHIP_TRACTOR_RING_ANIM_DUR;

    //Time left until the next ring is spat out.
    timer next_tractor_ring_timer;
    //Hue of each ring.
    vector<float> tractor_beam_ring_colors;
    //How long each ring has existed for.
    vector<float> tractor_beam_rings;
    //Is the tractor beam active?
    bool tractor_beam_enabled;

    //What type of ship it is.
    ship_type* shi_type;
    
    //Nest data.
    pikmin_nest_struct* nest;
    
    //The interaction point's absolute coordinates.
    point control_point_final_pos;
    //The receptacle's absolute coordinates.
    point receptacle_final_pos;
    //Cached distance between the beam and the receptacle
    float control_point_to_receptacle_dist;
    
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
