/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gate class and gate-related functions.
 */


#include "gate.h"

/* ----------------------------------------------------------------------------
 * Creates a gate mob.
 */
gate::gate(
    const point pos, gate_type* type,
    const float angle, const string &vars
) :
    mob(pos, type, angle, vars),
    gat_type(type),
    sec(get_sector(pos, nullptr, true)) {
    
    team = MOB_TEAM_OBSTACLE;
}
