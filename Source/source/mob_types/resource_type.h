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

#include "../misc_structs.h"
#include "../spray_type.h"
#include "../libs/data_file.h"
#include "../mobs/mob_enums.h"
#include "../mob_types/mob_type.h"
#include "../utils/general_utils.h"


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
class resource_type : public mob_type {

public:

    //--- Members ---
    
    //Should it vanish when the Pikmin carrying it drops it?
    bool vanish_on_drop = false;
    
    //Should it return to the pile it came from when it vanishes?
    bool return_to_pile_on_vanish = false;
    
    //How long before it vanishes, after being dropped.
    float vanish_delay = 0.0f;
    
    //Carry destination.
    CARRY_DESTINATION carrying_destination = CARRY_DESTINATION_SHIP;
    
    //Result when successfully delivered.
    RESOURCE_DELIVERY_RESULT delivery_result = RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS;
    
    //If it damages the mob it was carried to, this is the damage amount.
    float damage_mob_amount = 1.0f;
    
    //If it concocts a spray when delivered, this is the spray type index.
    size_t spray_to_concoct = INVALID;
    
    //If it adds points when delivered, this is the amount.
    float point_amount = 1.0f;
    
    
    //--- Function declarations ---
    
    resource_type();
    void load_cat_properties(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
    
};
