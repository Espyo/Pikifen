/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gate class and gate-related functions.
 */

#include "functions.h"
#include "gate.h"
#include "vars.h"
#include <iostream>
gate::gate(const float x, const float y, gate_type* type, const string &vars) :
    mob(x, y, type, 0, vars),
    gat_type(type),
    sec(get_sector(x, y, nullptr, true)) {
    
    sec->z += 200; //TODO magic number.
    team = MOB_TEAM_OBSTACLE;
}

void gate::open(mob* m, void* info1, void* info2) {
    gate* g_ptr = (gate*) m;
    g_ptr->sec->z -= 200; //TODO magic number.
    m->set_animation(GATE_ANIM_NOTHING);
    random_particle_explosion(
        PARTICLE_TYPE_BITMAP, bmp_smoke, g_ptr->x, g_ptr->y,
        60, 90, 10, 12, 2.5, 3, 64, 96, al_map_rgb(238, 204, 170)
    );
}

void gate::set_anim(mob* m, void* info1, void* info2) {
    m->set_animation(GATE_ANIM_IDLE);
}