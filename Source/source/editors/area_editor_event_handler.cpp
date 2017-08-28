/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor event handler function.
 */

#include <allegro5/allegro.h>

#include "area_editor.h"
#include "../functions.h"
#include "../vars.h"

void area_editor::handle_controls(const ALLEGRO_EVENT &ev) {
    //TODO
    
    if(fade_mgr.is_fading()) return;
    
    gui->handle_event(ev);
    
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        handle_mouse_update(ev);
    }
}


void area_editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {
    mouse_cursor_s.x = ev.mouse.x;
    mouse_cursor_s.y = ev.mouse.y;
    mouse_cursor_w = mouse_cursor_s;
    al_transform_coordinates(
        &screen_to_world_transform,
        &mouse_cursor_w.x, &mouse_cursor_w.y
    );
    
    if(!is_mouse_in_gui(mouse_cursor_s)) {
        lbl_status_bar->text =
            "(" + i2s(mouse_cursor_w.x) + "," + i2s(mouse_cursor_w.y) + ")";
    } else {
        lbl_status_bar->text =
            gui->get_widget_under_mouse(
                mouse_cursor_s.x, mouse_cursor_s.y
            )->description;
    }
}
