/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet class and pellet-related functions.
 */

#include "pellet.h"

/* ----------------------------------------------------------------------------
 * Creates a pellet.
 */
pellet::pellet(float x, float y, sector* s, pellet_type* type)
    : mob(x, y, s->floors[0].z, type, s) {
    
    pel_type = type;
    this->carrier_info = new carrier_info_struct(this, type->max_carriers, false);
}