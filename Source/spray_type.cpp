/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Spray type class and spray type-related functions.
 */

#include "spray_type.h"

/* ----------------------------------------------------------------------------
 * Creates a spray type.
 */
spray_type::spray_type(status* effect, const bool burpable, const float duration, const ALLEGRO_COLOR main_color, ALLEGRO_BITMAP* bmp_spray, ALLEGRO_BITMAP* bmp_berry, const bool can_drop_blobs, const unsigned int berries_needed) {
    this->effect = effect;
    this->burpable = burpable;
    this->duration = duration;
    this->main_color = main_color;
    this->bmp_spray = bmp_spray;
    this->bmp_berry = bmp_berry;
    this->berries_needed = berries_needed;
    this->can_drop_blobs = can_drop_blobs;
}
