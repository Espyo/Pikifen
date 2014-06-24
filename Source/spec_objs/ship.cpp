/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship class and ship-related functions.
 */

#include "ship.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a ship.
 */
ship::ship(float x, float y)
    : mob(x, y, special_mob_types["Ship"], 0, "") {
    
}