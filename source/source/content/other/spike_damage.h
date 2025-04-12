/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the spike damage class and spike damage-related functions.
 */

#pragma once

#include <vector>

#include <allegro5/allegro.h>

#include "../../util/geometry_utils.h"
#include "../content.h"
#include "particle.h"
#include "status.h"


using std::vector;


/**
 * @brief Type of spike damage.
 * When a mob is attacked, it can instantly deal some damage to the mob
 * that attacked it.
 */
class SpikeDamageType : public Content {

public:

    //--- Members ---
    
    //Amount of damage to cause, either in absolute HP or max HP ratio.
    float damage = 0.0f;
    
    //If true, damage is only dealt if the victim is eaten. e.g. White Pikmin.
    bool ingestionOnly = false;
    
    //If true, the damage var represents max HP ratio. If false, absolute HP.
    bool isDamageRatio = false;
    
    //Particle generator to use to generate particles, if any.
    ParticleGenerator* particleGen = nullptr;
    
    //Offset the particles by this much, horizontally.
    Point particleOffsetPos;
    
    //Offset the particles by this much, vertically.
    float particleOffsetZ = 0.0f;
    
    //Apply this status effect when the spike damage is applied.
    StatusType* statusToApply = nullptr;
    
    
    //--- Function declarations ---
    
    void loadFromDataNode(DataNode* node);
    
};
