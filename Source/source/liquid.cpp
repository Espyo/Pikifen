/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Liquid class and liquid-related functions.
 */

#include "liquid.h"

/* ----------------------------------------------------------------------------
 * Creates a liquid type.
 */
liquid::liquid() :
    main_color(al_map_rgba(0, 0, 0, 0)),
    surface_alpha(255) {
    
    surface_speed[0] = 10;
    surface_speed[1] = -13;
}
