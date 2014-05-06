/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Nectar class and nectar-related functions.
 */

#include "../const.h"
#include "nectar.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a nectar.
 */
nectar::nectar(float x, float y, sector* sec)
    : mob(x, y, sec->floors[0].z, nectar_mob_type, sec) {
    
    amount_left = NECTAR_AMOUNT;
}