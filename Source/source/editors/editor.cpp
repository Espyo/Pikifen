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
void editor::create_picker_frame() {

    frm_picker =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    frm_picker->hide();
    gui->add("frm_picker", frm_picker);
    
    frm_picker->add(
        "but_back",
        new lafi::button(gui_x + 8, 8, gui_x + 96, 24, "Back")
    );
    frm_picker->add(
        "lbl_title",
        new lafi::label(gui_x + 8, 32, scr_w - 8, 44)
    );
    frm_picker->add(
        "txt_text",
        new lafi::textbox(gui_x + 8, 52, scr_w - 48, 68)
    );
    frm_picker->add(
        "but_new",
        new lafi::button(scr_w - 40, 44, scr_w - 8, 76, "+")
    );
    
    frm_picker->add(
        "frm_list",
        new lafi::frame(gui_x + 8, 84, scr_w - 32, scr_h - 56)
    );
    frm_picker->add(
        "bar_scroll",
        new lafi::scrollbar(scr_w - 24, 84, scr_w - 8, scr_h - 56)
    );
    
    
    frm_picker->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        this->frm_picker->hide();
        show_bottom_frame();
        change_to_right_frame();
    };
    frm_picker->widgets["but_back"]->description =
        "Cancel.";
        
    ((lafi::textbox*) frm_picker->widgets["txt_text"])->enter_key_widget =
        frm_picker->widgets["but_new"];
    ((lafi::textbox*) frm_picker->widgets["txt_text"])->change_handler =
    [this] (lafi::widget * t) {
        populate_picker(((lafi::textbox*) t)->text);
    };
    frm_picker->widgets["txt_text"]->description =
        "Name of the element to create (if possible), or search filter.";
        
    frm_picker->widgets["but_new"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        string name =
            (
                (lafi::textbox*)
                this->frm_picker->widgets["txt_text"]
            )->text;
        if(name.empty()) return;
        
        this->create_new_from_picker(name);
        
        made_changes = true;
        
        (
            (lafi::textbox*)
            this->frm_picker->widgets["txt_text"]
        )->text.clear();
    };
    frm_picker->widgets["but_new"]->description =
        "Create a new one with the name on the textbox.";
        
    frm_picker->widgets["frm_list"]->mouse_wheel_handler =
    [this] (lafi::widget*, int dy, int) {
        lafi::scrollbar* s =
            (lafi::scrollbar*)
            this->frm_picker->widgets["bar_scroll"];
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
    const vector<pair<string, string> > &elements,
    const string &title, const bool can_make_new
) {

    hide_all_frames();
    frm_picker->show();
    hide_bottom_frame();
    
    ((lafi::label*) frm_picker->widgets["lbl_title"])->text = title;
    
    ((lafi::textbox*) frm_picker->widgets["txt_text"])->text.clear();
    
    if(can_make_new) {
        enable_widget(frm_picker->widgets["but_new"]);
    } else {
        disable_widget(frm_picker->widgets["but_new"]);
    }
    
    picker_elements = elements;
    populate_picker("");
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
 * Populates the picker frame with the elements of the list that match
 * the specified filter.
 */
void editor::populate_picker(const string &filter) {
    lafi::widget* f = frm_picker->widgets["frm_list"];
    string prev_category = "";
    string filter_lc = str_to_lower(filter);
    
    //Remove everything in the list so it can be re-populated.
    while(!f->widgets.empty()) {
        f->remove(f->widgets.begin()->first);
    }
    
    f->easy_reset();
    f->easy_row();
    
    for(size_t e = 0; e < picker_elements.size(); ++e) {
        string name = picker_elements[e].second;
        string lc_name = str_to_lower(name);
        string category = picker_elements[e].first;
        
        if(!filter.empty() && lc_name.find(filter_lc) == string::npos) {
            //Doesn't match the filter. Skip.
            continue;
        }
        
        if(category != prev_category) {
            //New category. Create its label.
            prev_category = category;
            lafi::label* c =
                new lafi::label("- " + category + " -", ALLEGRO_ALIGN_CENTER);
            f->easy_add("lbl_" + i2s(e), c, 100, 16);
            f->easy_row(0);
        }
        
        //Create the element's button.
        lafi::button* b = new lafi::button(name);
        
        b->left_mouse_click_handler =
        [name, category, this] (lafi::widget*, int, int) {
            this->frm_picker->hide();
            pick(name, category);
        };
        
        f->easy_add("but_" + i2s(e), b, 100, 24);
        f->easy_row(0);
    }
    
    //Make the scrollbar match the new list.
    (
        (lafi::scrollbar*) frm_picker->widgets["bar_scroll"]
    )->make_widget_scroll(f);
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
const float editor::transformation_controller::ROTATION_HANDLE_THICKNESS = 8.0;


/* ----------------------------------------------------------------------------
 * Creates a transformation controller.
 */
editor::transformation_controller::transformation_controller() :
    moving_handle(-1),
    angle(0),
    keep_aspect_ratio(true),
    allow_rotation(false) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the transformation (move, scale, rotate) handles.
 */
void editor::transformation_controller::draw_handles() {
    //Rotation handle.
    if(allow_rotation) {
        al_draw_circle(
            center.x, center.y, radius,
            al_map_rgb(64, 64, 192), ROTATION_HANDLE_THICKNESS / cam_zoom
        );
    }
    
    //Outline.
    point corners[4];
    corners[0] = point(-1, -1);
    corners[1] = point(1, -1);
    corners[2] = point(1, 1);
    corners[3] = point(-1, 1);
    for(unsigned char c = 0; c < 4; ++c) {
        al_transform_coordinates(
            &align_transform, &corners[c].x, &corners[c].y
        );
    }
    for(unsigned char c = 0; c < 4; ++c) {
        size_t c2 = sum_and_wrap(c, 1, 4);
        al_draw_line(
            corners[c].x, corners[c].y,
            corners[c2].x, corners[c2].y,
            al_map_rgb(32, 32, 160), 2.0 / cam_zoom
        );
    }
    
    //Translation and scale handles.
    for(unsigned char h = 0; h < 9; ++h) {
        point handle_pos = get_handle_pos(h);
        al_draw_filled_circle(
            handle_pos.x, handle_pos.y,
            HANDLE_RADIUS / cam_zoom, al_map_rgb(96, 96, 224)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Handles a mouse press, allowing a handle to be grabbed.
 * Returns true if handled, false if nothing was done.
 */
bool editor::transformation_controller::handle_mouse_down(const point pos) {
    for(unsigned char h = 0; h < 9; ++h) {
        point handle_pos = get_handle_pos(h);
        if(dist(handle_pos, pos) <= HANDLE_RADIUS / cam_zoom) {
            moving_handle = h;
            pre_move_size = size;
            return true;
        }
    }
    dist d(center, pos);
    if(
        d >= radius - ROTATION_HANDLE_THICKNESS / cam_zoom / 2.0 &&
        d <= radius + ROTATION_HANDLE_THICKNESS / cam_zoom / 2.0
    ) {
        moving_handle = 9;
        pre_rotation_angle = angle;
        pre_rotation_mouse_angle = ::get_angle(center, pos);
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse release, allowing a handle to be released.
 */
void editor::transformation_controller::handle_mouse_up() {
    moving_handle = -1;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse move, allowing a handle to be moved.
 * Returns true if handled, false if nothing was done.
 */
bool editor::transformation_controller::handle_mouse_move(const point pos) {
    if(moving_handle == -1) {
        return false;
    }
    
    if(moving_handle == 4) {
        set_center(pos);
        return true;
    }
    
    if(moving_handle == 9) {
        set_angle(
            pre_rotation_angle +
            (::get_angle(center, pos) - pre_rotation_mouse_angle)
        );
        return true;
    }
    
    point aligned_handle_pos = get_handle_pos(moving_handle);
    al_transform_coordinates(
        &align_transform,
        &aligned_handle_pos.x, &aligned_handle_pos.y
    );
    point aligned_cursor_pos = pos;
    al_transform_coordinates(
        &align_transform,
        &aligned_cursor_pos.x, &aligned_cursor_pos.y
    );
    point new_size = pre_move_size;
    point aligned_new_center = center;
    al_transform_coordinates(
        &align_transform,
        &aligned_new_center.x, &aligned_new_center.y
    );
    
    if(moving_handle == 0 || moving_handle == 3 || moving_handle == 6) {
        new_size.x = size.x / 2.0 - aligned_cursor_pos.x;
    } else if(moving_handle == 2 || moving_handle == 5 || moving_handle == 8) {
        new_size.x = aligned_cursor_pos.x - (-size.x / 2.0);
    }
    
    if(moving_handle == 0 || moving_handle == 1 || moving_handle == 2) {
        new_size.y = (size.y / 2.0) - aligned_cursor_pos.y;
    } else if(moving_handle == 6 || moving_handle == 7 || moving_handle == 8) {
        new_size.y = aligned_cursor_pos.y - (-size.y / 2.0);
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
        aligned_new_center.x = (size.x / 2.0) - new_size.x / 2.0;
    } else if(moving_handle == 2 || moving_handle == 5 || moving_handle == 8) {
        aligned_new_center.x = (-size.x / 2.0) + new_size.x / 2.0;
    }
    
    if(moving_handle == 0 || moving_handle == 1 || moving_handle == 2) {
        aligned_new_center.y = (size.y / 2.0) - new_size.y / 2.0;
    } else if(moving_handle == 6 || moving_handle == 7 || moving_handle == 8) {
        aligned_new_center.y = (-size.y / 2.0) + new_size.y / 2.0;
    }
    
    point new_center = aligned_new_center;
    al_transform_coordinates(
        &disalign_transform,
        &new_center.x, &new_center.y
    );
    
    set_center(new_center);
    set_size(new_size);
    
    //TODO old code.
    /*
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
    
    set_center(new_center);
    set_size(new_size);
    */
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns the center.
 */
point editor::transformation_controller::get_center() {
    return center;
}


/* ----------------------------------------------------------------------------
 * Returns the size.
 */
point editor::transformation_controller::get_size() {
    return size;
}


/* ----------------------------------------------------------------------------
 * Returns the angle.
 */
float editor::transformation_controller::get_angle() {
    return angle;
}


/* ----------------------------------------------------------------------------
 * Sets the center.
 */
void editor::transformation_controller::set_center(const point &center) {
    this->center = center;
    update();
}


/* ----------------------------------------------------------------------------
 * Sets the size.
 */
void editor::transformation_controller::set_size(const point &size) {
    this->size = size;
    update();
}


/* ----------------------------------------------------------------------------
 * Sets the angle.
 */
void editor::transformation_controller::set_angle(const float angle) {
    this->angle = angle;
    update();
}


/* ----------------------------------------------------------------------------
 * Returns the position at which a handle is.
 */
point editor::transformation_controller::get_handle_pos(
    const unsigned char handle
) {
    point result;
    if(handle == 0 || handle == 3 || handle == 6) {
        result.x = -size.x / 2.0;
    } else if(handle == 2 || handle == 5 || handle == 8) {
        result.x = size.x / 2.0;
    }
    if(handle == 0 || handle == 1 || handle == 2) {
        result.y = -size.y / 2.0;
    } else if(handle == 6 || handle == 7 || handle == 8) {
        result.y = size.y / 2.0;
    }
    al_transform_coordinates(&disalign_transform, &result.x, &result.y);
    return result;
}


/* ----------------------------------------------------------------------------
 * Updates the transformations to match the new data, as well as
 * some caches.
 */
void editor::transformation_controller::update() {
    al_identity_transform(&align_transform);
    al_translate_transform(&align_transform, -center.x, -center.y);
    al_rotate_transform(&align_transform, -angle);
    
    al_copy_transform(&disalign_transform, &align_transform);
    al_invert_transform(&disalign_transform);
    
    radius = dist(center, center + (size / 2)).to_float();
}
