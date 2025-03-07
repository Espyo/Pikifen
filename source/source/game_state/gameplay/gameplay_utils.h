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

class Leader;
class PikminType;


/**
 * @brief Info about an event involving two mobs.
 *
 * When processing inter-mob events, we want the mob to follow them from the
 * closest mob to the one farthest away. As such, this struct saves data on
 * a viable mob, its distance, and the corresponding event.
 * We can then go through a vector of these pending intermob events in order.
 *
 */
struct PendingIntermobEvent {

    //--- Members ---
    
    //Distance between both mobs.
    Distance d;
    
    //Pointer to the relevant event.
    MobEvent* event_ptr = nullptr;
    
    //Mob who the event belongs to.
    Mob* mob_ptr = nullptr;
    
    
    //--- Function definitions ---
    
    PendingIntermobEvent(
        const Distance &d, MobEvent* event_ptr, Mob* mob_ptr
    ):
        d(d),
        event_ptr(event_ptr),
        mob_ptr(mob_ptr) {
        
    }
    
};
