/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor drawing function.
 */

#include "area_editor.h"
#include "../vars.h"

void area_editor::do_drawing() {
    //TODO
    
    gui->draw();
    
    al_use_transform(&world_to_screen_transform);
    al_set_clipping_rectangle(0, 0, gui_x, status_bar_y);
    
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
    al_reset_clipping_rectangle();
    al_use_transform(&identity_transform);
    
    fade_mgr.draw();
    
    al_flip_display();
}
