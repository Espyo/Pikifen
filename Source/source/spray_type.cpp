/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Spray type class and spray type-related functions.
 */

#include "spray_type.h"

/* ----------------------------------------------------------------------------
 * Creates a spray type.
 */
spray_type::spray_type(status* effect, const bool burpable, const float duration, const ALLEGRO_COLOR &main_color, ALLEGRO_BITMAP* bmp_spray, ALLEGRO_BITMAP* bmp_berry, const bool can_drop_blobs, const unsigned int berries_needed) :
    effect(effect),
    burpable(burpable),
    duration(duration),
    main_color(main_color),
    bmp_spray(bmp_spray),
    bmp_berry(bmp_berry),
    berries_needed(berries_needed),
    can_drop_blobs(can_drop_blobs) {
    
}
