/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
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

gate::gate(const float x, const float y, gate_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    gat_type(type),
    sec(get_sector(x, y, nullptr, true)) {
    
    sec->type = SECTOR_TYPE_BLOCKING;
    team = MOB_TEAM_OBSTACLE;
}

void gate::open(mob* m, void* info1, void* info2) {
    gate* g_ptr = (gate*) m;
    g_ptr->sec->type = SECTOR_TYPE_NORMAL;
    m->set_animation(GATE_ANIM_NOTHING);
    m->start_dying();
    m->finish_dying();
    random_particle_explosion(
        PARTICLE_TYPE_BITMAP, bmp_smoke, g_ptr->x, g_ptr->y,
        60, 90, 10, 12, 2.5, 3, 64, 96, al_map_rgb(238, 204, 170)
    );
}

void gate::take_damage(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    float damage = calculate_damage(info->mob2, m, info->hi2, info->hi1);
    m->health -= damage;
}

void gate::set_anim(mob* m, void* info1, void* info2) {
    m->set_animation(GATE_ANIM_IDLE);
}