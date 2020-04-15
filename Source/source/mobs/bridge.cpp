/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge class and bridge related functions.
 */

#include "bridge.h"

#include "../functions.h"

/* ----------------------------------------------------------------------------
 * Creates a bridge mob.
 */
bridge::bridge(const point &pos, bridge_type* bri_type, const float angle) :
    mob(pos, bri_type, angle),
    bri_type(bri_type) {
    
    //Search neighboring sectors.
    get_sector(pos, NULL, true)->get_neighbor_sectors_conditionally(
    [] (sector * s) -> bool {
        return
        s->type == SECTOR_TYPE_BRIDGE ||
        s->type == SECTOR_TYPE_BRIDGE_RAIL;
    },
    secs
    );
    
    team = MOB_TEAM_OBSTACLE;
    
}
