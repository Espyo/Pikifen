/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor drawing function.
 */

#include <algorithm>

#include "editor.h"


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the area editor.
 */
void area_editor::do_drawing() {
    //TODO
    al_clear_to_color(al_map_rgb(32, 48, 64));
    al_flip_display();
}
