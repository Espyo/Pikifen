/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion class and Onion-related functions.
 */

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "../../core/misc_structs.hpp"
#include "../../util/general_utils.hpp"
#include "../mob_type/onion_type.hpp"
#include "../mob_type/pikmin_type.hpp"
#include "mob.hpp"


namespace ONION {
extern const float GENERATION_DELAY;
extern const float NEW_SEED_Z_OFFSET;
extern const float NEXT_GENERATION_INTERVAL;
extern const Angle SPIT_ANGLE_SHIFT;
extern const float SPIT_H_SPEED;
extern const float SPIT_H_SPEED_DEVIATION;
extern const float SPIT_V_SPEED;
}


/**
 * @brief An Onion is where Pikmin are stored.
 */
class Onion : public Mob {

public:

    //--- Public members ---
    
    //What type of Onion it is.
    OnionType* oniType = nullptr;
    
    //Nest data.
    PikminNest* nest = nullptr;
    
    //Is this Onion currently activated?
    bool activated = true;
    
    //How many seeds are queued up to be spat, of each type.
    vector<size_t> generationQueue;
    
    //Time left until it starts the Pikmin generation process.
    Timer generationDelayTimer = Timer(ONION::GENERATION_DELAY);
    
    //Time left until it generates the next Pikmin in the queue.
    Timer nextGenerationTimer = Timer(ONION::NEXT_GENERATION_INTERVAL);
    
    //Nutrients currently inside, in queue for generating a seed.
    vector<size_t> nutrients;
    
    //Number of seeds it has spit so far.
    unsigned int nSpits = 0;
    
    //How many objects are currently being beamed?
    size_t mobsBeingBeamed = 0;
    
    //ID of the beam sound, if any.
    size_t soundBeamId = 0;
    
    
    //--- Public function declarations ---
    
    Onion(const Point& pos, OnionType* type, Angle angle);
    ~Onion();
    void generate();
    void readScriptVars(const ScriptVarManager& varsMgr) override;
    void receiveNutrients(PikminType* pikType, size_t nutrientAmount);
    void spit(size_t typeIdx);
    void startGenerating();
    void stopGenerating();
    
    
protected:

    //--- Protected function declarations ---
    
    void tickClassSpecifics(float deltaT) override;
    
};
