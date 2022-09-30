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

#include <algorithm>

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


/* ----------------------------------------------------------------------------
 * Calls al_set_clipping_rectangle, but makes sure that the new clipping
 * rectangle is inside of an older one, as to not suddenly start drawing
 * in places that the older rectangle said not to.
 * The order doesn't really matter.
 * x1:
 *   First rectangle's top-left X coordinate.
 * y1:
 *   First rectangle's top-left Y coordinate.
 * w1:
 *   First rectangle's width.
 * h1:
 *   First rectangle's width.
 * x2:
 *   Second rectangle's top-left X coordinate.
 * y2:
 *   Second rectangle's top-left Y coordinate.
 * w2:
 *   Second rectangle's width.
 * h2:
 *   Second rectangle's width.
 */
void set_combined_clipping_rectangles(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2
) {
    float best_left = std::max(x1, x2);
    float best_top = std::max(y1, y2);
    float best_right = std::min(x1 + w1, x2 + w2);
    float best_bottom = std::min(y1 + h1, y2 + h2);
    al_set_clipping_rectangle(
        best_left, best_top,
        best_right - best_left, best_bottom - best_top
    );
}
