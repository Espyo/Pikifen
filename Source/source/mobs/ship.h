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


namespace SHIP {
extern const float CONTROL_POINT_ANIM_DUR;
extern const unsigned char CONTROL_POINT_RING_AMOUNT;
extern const float TRACTOR_BEAM_EMIT_RATE;
extern const float TRACTOR_BEAM_RING_ANIM_DUR;
}


/**
 * @brief A ship is where "treasure" is delivered to.
 */
class ship : public mob {

public:
    
    //--- Members ---

    //What type of ship it is.
    ship_type* shi_type;
    
    //Nest data.
    pikmin_nest_struct* nest;
    
    //Time left until the next tractor beam ring is spat out.
    timer next_tractor_beam_ring_timer;

    //Hue of each tractor beam ring.
    vector<float> tractor_beam_ring_colors;

    //How long each tractor beam ring has existed for.
    vector<float> tractor_beam_rings;

    //How many objects are currently being beamed?
    size_t mobs_being_beamed;
    
    //The control point's absolute coordinates.
    point control_point_final_pos;

    //The receptacle's absolute coordinates.
    point receptacle_final_pos;
    
    //Distance between control point and receptacle. Cache for convenience.
    float control_point_to_receptacle_dist;
    
    
    //--- Function declarations ---

    ship(const point &pos, ship_type* type, float angle);
    ~ship();
    void heal_leader(leader* l) const;
    bool is_leader_on_cp(const leader* l) const;
    void draw_mob() override;
    void read_script_vars(const script_var_reader &svr) override;
    void tick_class_specifics(const float delta_t) override;
    
};


#endif //ifndef SHIP_INCLUDED
