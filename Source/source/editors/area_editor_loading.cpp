/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor loading function.
 */

#include "area_editor.h"
#include "../LAFI/button.h"
#include "../LAFI/gui.h"
#include "../LAFI/label.h"
#include "../LAFI/style.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Loads the area editor.
 */
void area_editor::load() {

    update_gui_coordinates();
    
    lafi::style* s =
        new lafi::style(
        al_map_rgb(192, 192, 208),
        al_map_rgb(32, 32, 64),
        al_map_rgb(96, 128, 160),
        font_builtin
    );
    gui = new lafi::gui(scr_w, scr_h, s);
    
    
    //Main -- declarations.
    frm_main =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add(
        "lbl_area",
        new lafi::label("Area:"), 100, 16
    );
    frm_main->easy_row();
    frm_main->easy_add(
        "but_area",
        new lafi::button(), 100, 32
    );
    int y = frm_main->easy_row();
    
    frm_area =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_main->add("frm_area", frm_area);
    
    frm_area->easy_row();
    frm_area->easy_add(
        "but_layout",
        new lafi::button("Layout"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_objects",
        new lafi::button("Objects"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_paths",
        new lafi::button("Paths"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_details",
        new lafi::button("Details"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_review",
        new lafi::button("Review"), 100, 32
    );
    frm_area->easy_row();
    frm_area->easy_add(
        "but_tools",
        new lafi::button("Tools"), 100, 32
    );
    frm_area->easy_row();
    
    
    
    //Main -- properties.
    //frm_area->hide();
    
    
    
    create_changes_warning_frame();
    create_picker_frame(true);
    
    fade_mgr.start_fade(true, nullptr);
    
    state = EDITOR_STATE_MAIN;
    
    cam_zoom = 1.0;
    cam_pos.x = cam_pos.y = 0.0;
    
}
