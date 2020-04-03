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

#include "../data_file.h"
#include "../misc_structs.h"
#include "../spray_type.h"
#include "mob_type.h"

enum RESOURCE_ANIMATIONS {
    RESOURCE_ANIM_IDLING,
};

enum RESOURCE_DELIVERY_RESULTS {
    RESOURCE_DELIVERY_RESULT_DAMAGE_MOB,
    RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS,
    RESOURCE_DELIVERY_RESULT_ADD_POINTS,
};

enum RESOURCE_STATES {
    RESOURCE_STATE_IDLE_WAITING,
    RESOURCE_STATE_IDLE_MOVING,
    RESOURCE_STATE_IDLE_STUCK,
    RESOURCE_STATE_BEING_DELIVERED,
    
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
    size_t carrying_destination;
    size_t delivery_result;
    float damage_mob_amount;
    size_t spray_to_concoct;
    float point_amount;
    
    resource_type();
    void load_properties(data_node* file);
    anim_conversion_vector get_anim_conversions();
};

#endif //ifndef RESOURCE_TYPE_INCLUDED
