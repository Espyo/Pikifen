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
    bool ingestion_only = false;
    
    //If true, the damage var represents max HP ratio. If false, absolute HP.
    bool is_damage_ratio = false;
    
    //Particle generator to use to generate particles, if any.
    ParticleGenerator* particle_gen = nullptr;
    
    //Offset the particles by this much, horizontally.
    Point particle_offset_pos;
    
    //Offset the particles by this much, vertically.
    float particle_offset_z = 0.0f;
    
    //Apply this status effect when the spike damage is applied.
    StatusType* status_to_apply = nullptr;
    
    
    //--- Function declarations ---
    
    void load_from_data_node(DataNode* node);
    
};
