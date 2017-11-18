/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Editor-related functions.
 */

#include "editor.h"
#include "../functions.h"
#include "../LAFI/button.h"
#include "../LAFI/frame.h"
#include "../LAFI/scrollbar.h"
#include "../LAFI/textbox.h"
#include "../vars.h"



/* ----------------------------------------------------------------------------
 * Initializes editor class stuff.
 */
editor::editor() :
    picker_allows_new(false),
    gui(nullptr),
    gui_x(0),
    holding_m1(false),
    holding_m2(false),
    holding_m3(false),
    made_changes(false),
    mode(0),
    sec_mode(0),
    status_bar_y(0) {
    
}


/* ----------------------------------------------------------------------------
 * Closes the change warning box.
 */
void editor::close_changes_warning() {
    gui->widgets["frm_changes"]->hide();
    show_bottom_frame();
}


/* ----------------------------------------------------------------------------
 * Creates a "you have unsaved changes!" warning frame in the gui.
 */
void editor::create_changes_warning_frame() {
    lafi::frame* frm_changes =
        new lafi::frame(gui_x, scr_h - 48, scr_w, scr_h);
    frm_changes->hide();
    gui->add("frm_changes", frm_changes);
    
    frm_changes->easy_row();
    frm_changes->easy_add(
        "lbl_text1",
        new lafi::label("Warning: you have", ALLEGRO_ALIGN_LEFT),
        80, 8
    );
    frm_changes->easy_row();
    frm_changes->easy_add(
        "lbl_text2",
        new lafi::label("unsaved changes!", ALLEGRO_ALIGN_LEFT),
        80, 8
    );
    frm_changes->easy_row();
    frm_changes->add(
        "but_ok",
        new lafi::button(scr_w - 40, scr_h - 40, scr_w - 8, scr_h - 8, "Ok")
    );
    
    frm_changes->widgets["but_ok"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) { close_changes_warning(); };
    
}


/* ----------------------------------------------------------------------------
 * Creates a "picker" frame in the gui, used for picking objects
 * from a list.
 */
void editor::create_picker_frame(const bool can_create_new) {

    picker_allows_new = can_create_new;
    
    lafi::frame* frm_picker =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_picker->hide();
    gui->add("frm_picker", frm_picker);
    
    frm_picker->add(
        "but_back",
        new lafi::button(gui_x + 8, 8, gui_x + 96, 24, "Back")
    );
    if(can_create_new) {
        frm_picker->add(
            "txt_new",
            new lafi::textbox(gui_x + 8, 40, scr_w - 48, 56)
        );
        frm_picker->add(
            "but_new",
            new lafi::button(scr_w - 40,  32, scr_w - 8,  64, "+")
        );
    }
    frm_picker->add(
        "frm_list",
        new lafi::frame(gui_x + 8, 72, scr_w - 32, scr_h - 56)
    );
    frm_picker->add(
        "bar_scroll",
        new lafi::scrollbar(scr_w - 24,  72, scr_w - 8,  scr_h - 56)
    );
    
    
    frm_picker->widgets["but_back"]->left_mouse_click_handler =
    [this, can_create_new] (lafi::widget*, int, int) {
        if(can_create_new) {
            (
                (lafi::textbox*)
                this->gui->widgets["frm_picker"]->widgets["txt_new"]
            )->text.clear();
        }
        
        this->gui->widgets["frm_picker"]->hide();
        show_bottom_frame();
        change_to_right_frame();
    };
    frm_picker->widgets["but_back"]->description =
        "Cancel.";
        
    if(can_create_new) {
        ((lafi::textbox*) frm_picker->widgets["txt_new"])->enter_key_widget =
            frm_picker->widgets["but_new"];
            
        frm_picker->widgets["but_new"]->left_mouse_click_handler =
        [this] (lafi::widget*, int, int) {
            string name =
                (
                    (lafi::textbox*)
                    this->gui->widgets["frm_picker"]->widgets["txt_new"]
                )->text;
            if(name.empty()) return;
            
            this->create_new_from_picker(name);
            
            made_changes = true;
            
            (
                (lafi::textbox*)
                this->gui->widgets["frm_picker"]->widgets["txt_new"]
            )->text.clear();
        };
        frm_picker->widgets["but_new"]->description =
            "Create a new one with the name on the textbox.";
    }
    
    frm_picker->widgets["frm_list"]->mouse_wheel_handler =
    [this] (lafi::widget*, int dy, int) {
        lafi::scrollbar* s =
            (lafi::scrollbar*)
            this->gui->widgets["frm_picker"]->widgets["bar_scroll"];
        if(s->widgets.find("but_bar") != s->widgets.end()) {
            s->move_button(
                0,
                (s->widgets["but_bar"]->y1 + s->widgets["but_bar"]->y2) /
                2 - 30 * dy
            );
        }
    };
    
}


/* ----------------------------------------------------------------------------
 * Populates and opens the frame where you pick from a list.
 */
void editor::generate_and_open_picker(
    const vector<string> &elements, const unsigned char type,
    const bool can_make_new
) {

    hide_all_frames();
    gui->widgets["frm_picker"]->show();
    hide_bottom_frame();
    
    if(picker_allows_new) {
        if(can_make_new) {
            enable_widget(gui->widgets["frm_picker"]->widgets["txt_new"]);
            enable_widget(gui->widgets["frm_picker"]->widgets["but_new"]);
        } else {
            disable_widget(gui->widgets["frm_picker"]->widgets["txt_new"]);
            disable_widget(gui->widgets["frm_picker"]->widgets["but_new"]);
        }
    }
    
    lafi::widget* f = gui->widgets["frm_picker"]->widgets["frm_list"];
    
    while(!f->widgets.empty()) {
        f->remove(f->widgets.begin()->first);
    }
    
    f->easy_reset();
    f->easy_row();
    for(size_t e = 0; e < elements.size(); ++e) {
        lafi::button* b = new lafi::button(0, 0, 0, 0, elements[e]);
        string name = elements[e];
        b->left_mouse_click_handler =
        [name, type, this] (lafi::widget*, int, int) {
            this->gui->widgets["frm_picker"]->hide();
            pick(name, type);
        };
        f->easy_add("but_" + i2s(e), b, 100, 24);
        f->easy_row(0);
    }
    
    (
        (lafi::scrollbar*) gui->widgets["frm_picker"]->widgets["bar_scroll"]
    )->make_widget_scroll(f);
}


/* ----------------------------------------------------------------------------
 * Hides the bottom tools frame.
 */
void editor::hide_bottom_frame() {
    gui->widgets["frm_bottom"]->hide();
}


/* ----------------------------------------------------------------------------
 * Returns whether the mouse cursor is inside the gui or not.
 * The status bar counts as the gui.
 */
bool editor::is_mouse_in_gui(const point &mouse_coords) {
    return mouse_coords.x >= gui_x || mouse_coords.y >= status_bar_y;
}


/* ----------------------------------------------------------------------------
 * Exits out of the editor, with a fade.
 */
void editor::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
}


/* ----------------------------------------------------------------------------
 * Shows the bottom tools frame.
 */
void editor::show_bottom_frame() {
    gui->widgets["frm_bottom"]->show();
}


/* ----------------------------------------------------------------------------
 * Shows the "unsaved changes" warning.
 */
void editor::show_changes_warning() {
    gui->widgets["frm_changes"]->show();
    hide_bottom_frame();
    
    made_changes = false;
}


/* ----------------------------------------------------------------------------
 * Updates the variables that hold the gui's coordinates.
 */
void editor::update_gui_coordinates() {
    gui_x = scr_w * 0.675;
    status_bar_y = scr_h - 16;
}


const float editor::transformation_controller::HANDLE_RADIUS = 6.0;


/* ----------------------------------------------------------------------------
 * Creates a transformation controller.
 */
editor::transformation_controller::transformation_controller() :
    moving_handle(-1),
    angle(0),
    keep_aspect_ratio(true),
    allow_angle_transformations(false) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the transformation (move, scale, rotate) handles.
 */
void editor::transformation_controller::draw_handles() {
    al_draw_rectangle(
        center.x - size.x / 2.0, center.y - size.y / 2.0,
        center.x + size.x / 2.0, center.y + size.y / 2.0,
        al_map_rgb(32, 32, 160), 2.0 / cam_zoom
    );
    
    for(unsigned char h = 0; h < 9; ++h) {
        point p = get_handle_pos(h);
        al_draw_filled_circle(
            p.x, p.y, HANDLE_RADIUS / cam_zoom, al_map_rgb(96, 96, 224)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Handles a mouse press, allowing a handle to be grabbed.
 */
void editor::transformation_controller::handle_mouse_down(const point pos) {
    for(unsigned char h = 0; h < 9; ++h) {
        point handle_pos = get_handle_pos(h);
        if(dist(handle_pos, pos) <= HANDLE_RADIUS / cam_zoom) {
            moving_handle = h;
            pre_move_size = size;
            break;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles a mouse release, allowing a handle to be released.
 */
void editor::transformation_controller::handle_mouse_up() {
    moving_handle = -1;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse move, allowing a handle to be moved.
 */
#include <iostream> //TODO
void editor::transformation_controller::handle_mouse_move(const point pos) {
    if(moving_handle == -1) {
        return;
    }
    
    if(moving_handle == 4) {
        center = pos;
        return;
    }
    
    point new_size = pre_move_size;
    point new_center = center;
    
    if(moving_handle == 0 || moving_handle == 3 || moving_handle == 6) {
        new_size.x = (center.x + size.x / 2.0) - pos.x;
    } else if(moving_handle == 2 || moving_handle == 5 || moving_handle == 8) {
        new_size.x = pos.x - (center.x - size.x / 2.0);
    }
    
    if(moving_handle == 0 || moving_handle == 1 || moving_handle == 2) {
        new_size.y = (center.y + size.y / 2.0) - pos.y;
    } else if(moving_handle == 6 || moving_handle == 7 || moving_handle == 8) {
        new_size.y = pos.y - (center.y - size.y / 2.0);
    }
    
    if(keep_aspect_ratio) {
        if(
            fabs(pre_move_size.x - new_size.x) >
            fabs(pre_move_size.y - new_size.y)
        ) {
            //Most significant change is width.
            if(pre_move_size.x != 0) {
                float ratio = pre_move_size.y / pre_move_size.x;
                new_size.y = new_size.x * ratio;
            }
            
        } else {
            //Most significant change is height.
            if(pre_move_size.y != 0) {
                float ratio = pre_move_size.x / pre_move_size.y;
                new_size.x = new_size.y * ratio;
            }
            
        }
    }
    
    if(moving_handle == 0 || moving_handle == 3 || moving_handle == 6) {
        new_center.x = (center.x + size.x / 2.0) - new_size.x / 2.0;
    } else if(moving_handle == 2 || moving_handle == 5 || moving_handle == 8) {
        new_center.x = (center.x - size.x / 2.0) + new_size.x / 2.0;
    }
    
    if(moving_handle == 0 || moving_handle == 1 || moving_handle == 2) {
        new_center.y = (center.y + size.y / 2.0) - new_size.y / 2.0;
    } else if(moving_handle == 6 || moving_handle == 7 || moving_handle == 8) {
        new_center.y = (center.y - size.y / 2.0) + new_size.y / 2.0;
    }
    
    center = new_center;
    size = new_size;
}


/* ----------------------------------------------------------------------------
 * Returns the position at which a handle is.
 */
point editor::transformation_controller::get_handle_pos(
    const unsigned char handle
) {
    point result = center;
    if(handle == 0 || handle == 3 || handle == 6) {
        result.x -= size.x / 2.0;
    } else if(handle == 2 || handle == 5 || handle == 8) {
        result.x += size.x / 2.0;
    }
    if(handle == 0 || handle == 1 || handle == 2) {
        result.y -= size.y / 2.0;
    } else if(handle == 6 || handle == 7 || handle == 8) {
        result.y += size.y / 2.0;
    }
    return result;
}
