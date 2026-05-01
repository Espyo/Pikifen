/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bouncer class and bouncer-related functions.
 */

#include "bouncer.h"


/**
 * @brief Constructs a new bouncer object.
 *
 * @param center Starting center coordinates.
 * @param type Bouncer type this mob belongs to.
 * @param angle Starting angle.
 */
Bouncer::Bouncer(
    const Point& center, BouncerType* type, float angle
):
    Mob(center, type, angle),
    bouType(type) {
    
    team = MOB_TEAM_NONE;
}
