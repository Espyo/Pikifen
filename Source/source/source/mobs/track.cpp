/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Track class and track-related functions.
 */

#include "track.h"


/* ----------------------------------------------------------------------------
 * Creates a new track mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Track type this mob belongs to.
 * angle:
 *   Starting angle.
 */
track::track(
    const point &pos, track_type* type, const float angle
):
    mob(pos, type, angle),
    tra_type(type) {
    
    team = MOB_TEAM_NONE;
}
