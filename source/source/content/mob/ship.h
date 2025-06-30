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
extern const float BEAM_EMIT_RATE;
extern const float BEAM_RING_ANIM_DUR;
extern const float CONTROL_POINT_ANIM_DUR;
extern const unsigned char CONTROL_POINT_RING_AMOUNT;
}


/**
 * @brief A ship is where "treasure" is delivered to.
 */
class Ship : public Mob {

public:

    //--- Members ---
    
    //What type of ship it is.
    ShipType* shiType = nullptr;
    
    //Nest data.
    PikminNest* nest = nullptr;
    
    //Time left until the next beam ring is spat out.
    Timer nextBeamRingTimer = Timer(SHIP::BEAM_EMIT_RATE);
    
    //Hue of each beam ring.
    vector<float> beamRingColors;
    
    //How long each beam ring has existed for.
    vector<float> beamRings;
    
    //How many objects are currently being beamed?
    size_t mobsBeingBeamed = 0;
    
    //ID of the beam sound, if any.
    size_t soundBeamId = 0;
    
    //The control point's absolute coordinates.
    Point controlPointFinalPos;
    
    //The receptacle's absolute coordinates.
    Point receptacleFinalPos;
    
    //Distance between control point and receptacle. Cache for convenience.
    float controlPointToReceptacleDist = 0.0f;
    
    
    //--- Function declarations ---
    
    Ship(const Point& pos, ShipType* type, float angle);
    ~Ship();
    void healLeader(Leader* l) const;
    bool isLeaderOnCp(const Leader* l) const;
    void drawMob() override;
    void readScriptVars(const ScriptVarReader& svr) override;
    void tickClassSpecifics(float deltaT) override;
    
};
