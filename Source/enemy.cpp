/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy class and enemy-related functions.
 */

#include "enemy.h"
#include "functions.h"

/* ----------------------------------------------------------------------------
 * Creates an enemy.
 */
enemy::enemy(const float x, const float y, enemy_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    ene_type(type),
    spawn_delay(s2f(get_var_value(vars, "spawn_delay", "0"))),
    respawn_days_left(0),
    respawns_after_x_days(0),
    appears_after_day(0),
    appears_before_day(0),
    appears_every_x_days(0) {
    
    team = MOB_TEAM_ENEMY_1; //TODO removeish.
    
    //TODO day apperance interval
}
