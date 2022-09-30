/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for Allegro-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#ifndef ALLEGRO_UTILS_INCLUDED
#define ALLEGRO_UTILS_INCLUDED

#include <allegro5/allegro.h>


bool operator==(const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2);
bool operator!=(const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2);

void set_combined_clipping_rectangles(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2
);

#endif //ifndef ALLEGRO_UTILS_INCLUDED
