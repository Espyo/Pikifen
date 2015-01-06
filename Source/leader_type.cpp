/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader type class and leader type-related functions.
 */

#include "leader_type.h"
#include "const.h"

leader_type::leader_type() {
    whistle_range = DEF_WHISTLE_RANGE;
    punch_strength = DEF_PUNCH_STRENGTH;
    pluck_delay = 0.6;
    main_color = al_map_rgb(128, 128, 128);
    bmp_icon = NULL;
}
