/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship class and ship-related functions.
 */

#include "ship.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a ship.
 */
ship::ship(float x, float y, ship_type* type, float angle, const string &vars)
    : mob(x, y, type, angle, vars) {
    
    shi_type = type;
}
