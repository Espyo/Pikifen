/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion type class and Onion type-related functions.
 */

#pragma once

#include "../../lib/data_file/data_file.hpp"
#include "../../util/general_utils.hpp"
#include "../mob/mob_utils.hpp"
#include "mob_type.hpp"
#include "pikmin_type.hpp"


//Onion object states.
enum ONION_STATE {

    //Idling.
    ONION_STATE_IDLING,
    
    //Generating Pikmin.
    ONION_STATE_GENERATING,
    
    //Stopped generating Pikmin.
    ONION_STATE_STOPPING_GENERATION,
    
    //Dead.
    ONION_STATE_DEAD,
    
    //Finished receiving a delivery.
    ONION_STATE_RECEIVED,
    
    //Total amount of Onion object states.
    N_ONION_STATES,
    
};


//Onion object animations.
enum ONION_ANIM {

    //Idling.
    ONION_ANIM_IDLING,
    
    //Generating.
    ONION_ANIM_GENERATING,
    
    //Winding down since it stopped generating.
    ONION_ANIM_STOPPING_GENERATION,
    
    //Finished receiving a delivery.
    ONION_ANIM_RECEIVED,
    
};


/**
 * @brief An Onion type.
 * It's basically associated with one or more Pikmin types.
 */
class OnionType : public MobType {

public:

    //--- Public members ---
    
    //Nest data.
    PikminNestType* nest = nullptr;
    
    //Whether it automatically ejects seeds or Pikmin if there's space.
    bool autoEject = false;
    
    //Whether it ejects seeds or fully-formed Pikmin.
    bool ejectGrownPikmin = false;
    
    //Families of nutrients it can receive. Sources from an unsupported family
    //cannot be accepted.
    vector<string> nutrientFamilies = { "pellet", "enemy" };
    
    //Number of nutrients required to form one Pikmin.
    size_t nutrientsPerSeed = 1;
    
    //Sound data index for the beam sound. Cache for performance.
    size_t soundBeamIdx = INVALID;
    
    //Sound data index for the pop sound. Cache for performance.
    size_t soundPopIdx = INVALID;
    
    //Sound data index for the object reception sound. Cache for performance.
    size_t soundReceptionIdx = INVALID;
    
    //Delivery animation to play for items that get delivered.
    DELIVERY_ANIM deliveryAnim = DELIVERY_ANIM_SUCK;
    
    //Radius of the area where a carried mob can be delivered.
    float deliveryAreaRadius = 0.0f;
    
    //If true, nutrients received are sent to the linked Onion and then ignored.
    bool sendNutrientsToLink = false;
    
    
    //--- Public function declarations ---
    
    OnionType();
    ~OnionType();
    void loadCatProperties(DataNode* file) override;
    void loadCatResources(DataNode* file) override;
    AnimConversionVector getAnimConversions() const override;
    
};
