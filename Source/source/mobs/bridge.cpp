/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge class and bridge related functions.
 */

#include "bridge.h"
#include "../functions.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a bridge mob.
 */
bridge::bridge(
    const point &pos, bridge_type* bri_type,
    const float angle, const string &vars
) :
    mob(pos, bri_type, angle, vars),
    bri_type(bri_type) {
    
    //Search neighboring sectors.
    get_neighbor_bridge_sectors(get_sector(pos, NULL, true));
    team = MOB_TEAM_OBSTACLE;
    
}


/* ----------------------------------------------------------------------------
 * Populates this bridge's list of sectors by checking all
 * neighboring sectors recursively, until it can't find any more
 * sectors of the "bridge" or "bridge rail" type.
 */
void bridge::get_neighbor_bridge_sectors(sector* s_ptr) {

    if(!s_ptr) return;
    
    //If this sector is not a bridge sector, skip it.
    if(
        s_ptr->type != SECTOR_TYPE_BRIDGE &&
        s_ptr->type != SECTOR_TYPE_BRIDGE_RAIL
    ) return;
    
    //If this sector is already on the list, skip.
    for(size_t s = 0; s < secs.size(); ++s) {
        if(secs[s] == s_ptr) return;
    }
    
    secs.push_back(s_ptr);
    
    edge* e_ptr = NULL;
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        e_ptr = s_ptr->edges[e];
        get_neighbor_bridge_sectors(
            e_ptr->sectors[(e_ptr->sectors[0] == s_ptr ? 1 : 0)]
        );
    }
}
