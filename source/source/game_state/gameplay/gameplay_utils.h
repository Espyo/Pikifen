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
struct PendingInterMobEvent {

    //--- Public members ---
    
    //Distance between both mobs.
    Distance d;
    
    //Pointer to the relevant event.
    MobEvent* eventPtr = nullptr;
    
    //Mob who the event belongs to.
    Mob* mobPtr = nullptr;
    
    
    //--- Public function definitions ---
    
    PendingInterMobEvent(
        const Distance& d, MobEvent* eventPtr, Mob* mobPtr
    ):
        d(d),
        eventPtr(eventPtr),
        mobPtr(mobPtr) {
        
    }
    
};
