/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship class and ship-related functions.
 */

#pragma once

#include "mob.h"

#include "../../util/general_utils.h"
#include "../mob_type/ship_type.h"
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
class Ship : public Mob {

public:

    //--- Members ---
    
    //What type of ship it is.
    ShipType* shi_type = nullptr;
    
    //Nest data.
    PikminNest* nest = nullptr;
    
    //Time left until the next tractor beam ring is spat out.
    Timer next_tractor_beam_ring_timer = Timer(SHIP::TRACTOR_BEAM_EMIT_RATE);
    
    //Hue of each tractor beam ring.
    vector<float> tractor_beam_ring_colors;
    
    //How long each tractor beam ring has existed for.
    vector<float> tractor_beam_rings;
    
    //How many objects are currently being beamed?
    size_t mobs_being_beamed = 0;
    
    //The control point's absolute coordinates.
    Point control_point_final_pos;
    
    //The receptacle's absolute coordinates.
    Point receptacle_final_pos;
    
    //Distance between control point and receptacle. Cache for convenience.
    float control_point_to_receptacle_dist = 0.0f;
    
    
    //--- Function declarations ---
    
    Ship(const Point &pos, ShipType* type, float angle);
    ~Ship();
    void heal_leader(Leader* l) const;
    bool is_leader_on_cp(const Leader* l) const;
    void draw_mob() override;
    void read_script_vars(const ScriptVarReader &svr) override;
    void tick_class_specifics(float delta_t) override;
    
};
