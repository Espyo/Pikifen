/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the resource type class and resource type-related functions.
 */

#ifndef RESOURCE_TYPE_INCLUDED
#define RESOURCE_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../misc_structs.h"
#include "../spray_type.h"
#include "../utils/data_file.h"
#include "../mobs/mob_enums.h"
#include "../mob_types/mob_type.h"


//Resource object animations.
enum RESOURCE_ANIMATIONS {
    //Idling.
    RESOURCE_ANIM_IDLING,
};


//Results when a resource is successfully delivered.
enum RESOURCE_DELIVERY_RESULTS {
    //Damage the mob it got delivered to.
    RESOURCE_DELIVERY_RESULT_DAMAGE_MOB,
    //Increase a spray type's ingredient count.
    RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS,
    //Add points to the player's score.
    RESOURCE_DELIVERY_RESULT_ADD_POINTS,
};


//Resource object states.
enum RESOURCE_STATES {
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
    
    //Total amount of resource object states.
    N_RESOURCE_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of resource (gold nugget, bridge fragment, spray ingredient, etc.).
 */
class resource_type : public mob_type {
public:
    bool vanish_on_drop;
    bool return_to_pile_on_vanish;
    float vanish_delay;
    CARRY_DESTINATIONS carrying_destination;
    RESOURCE_DELIVERY_RESULTS delivery_result;
    float damage_mob_amount;
    size_t spray_to_concoct;
    float point_amount;
    
    resource_type();
    void load_properties(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
};


#endif //ifndef RESOURCE_TYPE_INCLUDED
