/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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
    main_color(COLOR_EMPTY) {
    surface_color[0] = COLOR_WHITE;
    surface_color[1] = COLOR_WHITE;
    surface_speed[0] = point(10,0);
    surface_speed[1] = point(-13, 0);
    surface_scale[0] = 1;
    surface_scale[1] = 1;
}
