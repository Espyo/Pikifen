/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Allegro-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#include "allegro_utils.h"


/* ----------------------------------------------------------------------------
 * Checks if two colors are the same.
 * c1:
 *   First color.
 * c2:
 *   Second color.
 */
bool operator==(const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2) {
    if(c1.r != c2.r) return false;
    if(c1.g != c2.g) return false;
    if(c1.b != c2.b) return false;
    return c1.a == c2.a;
}


/* ----------------------------------------------------------------------------
 * Checks if two colors are different.
 * c1:
 *   First color.
 * c2:
 *   Second color.
 */
bool operator!=(const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2) {
    return !operator==(c1, c2);
}
