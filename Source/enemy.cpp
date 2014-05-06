/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy class and enemy-related functions.
 */

#include "enemy.h"

/* ----------------------------------------------------------------------------
 * Creates an enemy.
 */
enemy::enemy(const float x, const float y, sector* s, enemy_type* type)
    : mob(x, y, s->floors[0].z, type, s) {
    
    ene_type = type;
    team = MOB_TEAM_ENEMIES; //ToDo removeish.
}