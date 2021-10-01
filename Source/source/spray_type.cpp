/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Spray type class and spray type-related functions.
 */

#include "spray_type.h"


/* ----------------------------------------------------------------------------
 * Creates a spray type.
 */
spray_type::spray_type() :
    group(true),
    group_pikmin_only(true),
    affects_user(true),
    angle(0),
    distance_range(0),
    angle_range(0),
    main_color(al_map_rgba(0, 0, 0, 0)),
    bmp_spray(nullptr),
    ingredients_needed(10),
    buries_pikmin(false) {
    
}
