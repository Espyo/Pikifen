/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gameplay-class utility classes and functions.
 */

#pragma once

#include "../../content/mob/mob_utils.h"
#include "../../content/other/gui.h"
#include "../../content/other/mob_script.h"
#include "../../util/geometry_utils.h"

class leader;
class pikmin_type;


/**
 * @brief Info about an event involving two mobs.
 *
 * When processing inter-mob events, we want the mob to follow them from the
 * closest mob to the one farthest away. As such, this struct saves data on
 * a viable mob, its distance, and the corresponding event.
 * We can then go through a vector of these pending intermob events in order.
 *
 */
struct pending_intermob_event {

    //--- Members ---
    
    //Distance between both mobs.
    dist d;
    
    //Pointer to the relevant event.
    mob_event* event_ptr = nullptr;
    
    //Mob who the event belongs to.
    mob* mob_ptr = nullptr;
    
    
    //--- Function definitions ---
    
    pending_intermob_event(
        const dist &d, mob_event* event_ptr, mob* mob_ptr
    ):
        d(d),
        event_ptr(event_ptr),
        mob_ptr(mob_ptr) {
        
    }
    
};
