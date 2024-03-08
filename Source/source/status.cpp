/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Status effect classes and status effect-related functions.
 */

#include <algorithm>

#include "status.h"


/**
 * @brief Constructs a new status object.
 *
 * @param type Its type.
 */
status::status(status_type* type) :
    type(type) {
    
    time_left = type->auto_remove_time;
}


/**
 * @brief Ticks a status effect instance's time by one frame of logic,
 * but does not tick its effects logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void status::tick(const float delta_t) {
    if(type->auto_remove_time > 0.0f) {
        time_left -= delta_t;
        if(time_left <= 0.0f) {
            to_delete = true;
        }
    }
}
