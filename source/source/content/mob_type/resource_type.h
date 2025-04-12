/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the resource type class and resource type-related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include "../../core/misc_structs.h"
#include "../../lib/data_file/data_file.h"
#include "../../util/general_utils.h"
#include "../mob_type/mob_type.h"
#include "../mob/mob_enums.h"
#include "../other/spray_type.h"


//Resource object animations.
enum RESOURCE_ANIM {

    //Idling.
    RESOURCE_ANIM_IDLING,
    
};


//Results when a resource is successfully delivered.
enum RESOURCE_DELIVERY_RESULT {

    //Damage the mob it got delivered to.
    RESOURCE_DELIVERY_RESULT_DAMAGE_MOB,
    
    //Increase a spray type's ingredient count.
    RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS,
    
    //Add some treasure points.
    RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS,
    
    //The Pikmin stay on that spot.
    RESOURCE_DELIVERY_RESULT_STAY,
    
};


//Resource object states.
enum RESOURCE_STATE {

    //Waiting.
    RESOURCE_STATE_IDLE_WAITING,
    
    //Moving.
    RESOURCE_STATE_IDLE_MOVING,
    
    //Stuck.
    RESOURCE_STATE_IDLE_STUCK,
    
    //Thrown.
    RESOURCE_STATE_IDLE_THROWN,
    
    //Being delivered.
    RESOURCE_STATE_BEING_DELIVERED,
    
    //Staying in place after being delivered.
    RESOURCE_STATE_STAYING_AFTER_DELIVERY,
    
    //Total amount of resource object states.
    N_RESOURCE_STATES,
    
};


/**
 * @brief A type of resource (gold nugget, bridge fragment,
 * spray ingredient, etc.).
 */
class ResourceType : public MobType {

public:

    //--- Members ---
    
    //Should it vanish when the Pikmin carrying it drops it?
    bool vanishOnDrop = false;
    
    //Should it return to the pile it came from when it vanishes?
    bool returnToPileOnVanish = false;
    
    //How long before it vanishes, after being dropped.
    float vanishDelay = 0.0f;
    
    //Carry destination.
    CARRY_DESTINATION carryingDestination = CARRY_DESTINATION_SHIP;
    
    //Result when successfully delivered.
    RESOURCE_DELIVERY_RESULT deliveryResult = RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS;
    
    //If it damages the mob it was carried to, this is the damage amount.
    float damageMobAmount = 1.0f;
    
    //If it concocts a spray when delivered, this is the spray type index.
    size_t sprayToConcoct = INVALID;
    
    //If it adds points when delivered, this is the amount.
    float pointAmount = 1.0f;
    
    
    //--- Function declarations ---
    
    ResourceType();
    void loadCatProperties(DataNode* file) override;
    anim_conversion_vector getAnimConversions() const override;
    
};
