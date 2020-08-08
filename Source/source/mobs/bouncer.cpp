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


/* ----------------------------------------------------------------------------
 * Creates a new bouncer mob.
 */
bouncer::bouncer(
    const point &pos, bouncer_type* type, const float angle
):
    mob(pos, type, angle),
    bou_type(type) {
    
    team = MOB_TEAM_NONE;
}
