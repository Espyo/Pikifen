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

#include "../../lib/data_file/data_file.h"
#include "../../util/general_utils.h"
#include "../mob/mob_utils.h"
#include "mob_type.h"
#include "pikmin_type.h"


//Onion object states.
enum ONION_STATE {

    //Idling.
    ONION_STATE_IDLING,
    
    //Generating Pikmin.
    ONION_STATE_GENERATING,
    
    //Stopped generating Pikmin.
    ONION_STATE_STOPPING_GENERATION,
    
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
    
};


/**
 * @brief An Onion type.
 * It's basically associated with one or more Pikmin types.
 */
class OnionType : public MobType {

public:

    //--- Members ---
    
    //Nest data.
    PikminNestType* nest = nullptr;
    
    //Sound data index for the pop sound. Cache for performance.
    size_t soundPopIdx = INVALID;
    
    
    //--- Function declarations ---
    
    OnionType();
    ~OnionType();
    void loadCatProperties(DataNode* file) override;
    void loadCatResources(DataNode* file) override;
    AnimConversionVector getAnimConversions() const override;
    
};
