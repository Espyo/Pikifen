/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion class and Onion-related functions.
 */

#include "onion.h"

/* ----------------------------------------------------------------------------
 * Creates an onion.
 */
onion::onion(float x, float y, sector* sec, onion_type* type)
    : mob(x, y, sec->z, type, sec) {
    
    oni_type = type;
}