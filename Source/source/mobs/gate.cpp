/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gate class and gate-related functions.
 */


#include "gate.h"

gate::gate(
    const float x, const float y, gate_type* type,
    const float angle, const string &vars
) :
    mob(x, y, type, angle, vars),
    gat_type(type),
    sec(get_sector(x, y, nullptr, true)) {

    sec->type = SECTOR_TYPE_BLOCKING;
    team = MOB_TEAM_OBSTACLE;
}
