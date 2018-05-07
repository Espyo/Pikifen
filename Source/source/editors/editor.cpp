/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Editor-related functions.
 */

#include "editor.h"
#include "../functions.h"
#include "../LAFI/angle_picker.h"
#include "../LAFI/button.h"
#include "../LAFI/checkbox.h"
#include "../LAFI/frame.h"
#include "../LAFI/radio_button.h"
#include "../LAFI/scrollbar.h"
#include "../LAFI/textbox.h"
#include "../vars.h"


//Time until the next click is no longer considered a double-click.
const float editor::DOUBLE_CLICK_TIMEOUT = 0.5f;
//Name of the folder in the graphics folder where the icons are found.
const string editor::EDITOR_ICONS_FOLDER_NAME = "Editor_icons";
//If the mouse is dragged outside of this range, that's a real drag.
const float editor::MOUSE_DRAG_CONFIRM_RANGE = 4.0f;
//How long to override the status bar text for, for important messages.
const float editor::STATUS_OVERRIDE_IMPORTANT_DURATION = 6.0f;
//How long to override the status bar text for, for unimportant messages.
const float editor::STATUS_OVERRIDE_UNIMPORTANT_DURATION = 1.5f;

/* ----------------------------------------------------------------------------
 * Initializes editor class stuff.
 */
editor::editor() :
    gui(nullptr),
    warning_style(nullptr),
    gui_x(0),
    double_click_time(0),
    holding_m1(false),
    holding_m2(false),
    holding_m3(false),
    icons(EDITOR_ICONS_FOLDER_NAME),
    is_ctrl_pressed(false),
    is_gui_focused(false),
    is_shift_pressed(false),
    last_mouse_click(INVALID),
    loaded_content_yet(false),
    made_changes(false),
    mouse_drag_confirmed(false),
    mode(0),
    sec_mode(0),
    status_bar_y(0),
    zoom_max_level(0),
    zoom_min_level(0) {
    
    warning_style = new lafi::style(
        al_map_rgb(224, 224, 64),
        al_map_rgb(0, 0, 0),
        al_map_rgb(96, 96, 96)
    );
    
    status_override_timer =
    timer(STATUS_OVERRIDE_IMPORTANT_DURATION, [this] () {update_status_bar();});
    
}


/* ----------------------------------------------------------------------------
 * Destroys an instance of the editor class.
 */
editor::~editor() {
    delete warning_style;
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
        new lafi::frame(gui_x, scr_h - 48, scr_w, scr_h, warning_style);
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
        custom_picker_cancel_action();
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
            get_textbox_text(this->frm_picker, "txt_text");
        if(name.empty()) return;
        
        this->create_new_from_picker(name);
        
        made_changes = true;
        
        set_textbox_text(this->frm_picker, "txt_text", "");
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
 * Handles the logic part of the main loop of the area editor.
 */
void editor::do_logic() {
    gui->tick(delta_t);
    
    update_transformations();
    
    if(double_click_time > 0) {
        double_click_time -= delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
    
    status_override_timer.tick(delta_t);
    fade_mgr.tick(delta_t);
}


/* ----------------------------------------------------------------------------
 * Emits a message onto the status bar, and keeps it there for some seconds.
 * text:      Message text.
 * important: If true, the message stays for a few more seconds than normal.
 */
void editor::emit_status_bar_message(
    const string &text, const bool important
) {
    status_override_text = text;
    status_override_timer.start(
        important ?
        STATUS_OVERRIDE_IMPORTANT_DURATION :
        STATUS_OVERRIDE_UNIMPORTANT_DURATION
    );
    lbl_status_bar->text = status_override_text;
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
    
    set_label_text(frm_picker, "lbl_title", title);
    set_textbox_text(frm_picker, "txt_text", "");
    
    if(can_make_new) {
        enable_widget(frm_picker->widgets["but_new"]);
    } else {
        disable_widget(frm_picker->widgets["but_new"]);
    }
    
    picker_elements = elements;
    populate_picker("");
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event for control-related things.
 */
void editor::handle_controls(const ALLEGRO_EVENT &ev) {
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
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        !is_mouse_in_gui(mouse_cursor_s)
    ) {
    
        if(ev.mouse.button == 1) {
            holding_m1 = true;
        } else if(ev.mouse.button == 2) {
            holding_m2 = true;
        } else if(ev.mouse.button == 3) {
            holding_m3 = true;
        }
        
        mouse_drag_start = point(ev.mouse.x, ev.mouse.y);
        mouse_drag_confirmed = false;
        
        gui->lose_focus();
        is_gui_focused = false;
        
        if(ev.mouse.button == last_mouse_click && double_click_time > 0) {
            if(ev.mouse.button == 1) {
                handle_lmb_double_click(ev);
            } else if(ev.mouse.button == 2) {
                if(area_editor_mmb_pan) {
                    handle_mmb_double_click(ev);
                } else {
                    handle_rmb_double_click(ev);
                }
            } else if(ev.mouse.button == 3) {
                if(area_editor_mmb_pan) {
                    handle_rmb_double_click(ev);
                } else {
                    handle_mmb_double_click(ev);
                }
            }
            
            double_click_time = 0;
            
        } else {
            if(ev.mouse.button == 1) {
                handle_lmb_down(ev);
            } else if(ev.mouse.button == 2) {
                if(area_editor_mmb_pan) {
                    handle_mmb_down(ev);
                } else {
                    handle_rmb_down(ev);
                }
            } else if(ev.mouse.button == 3) {
                if(area_editor_mmb_pan) {
                    handle_rmb_down(ev);
                } else {
                    handle_mmb_down(ev);
                }
            }
            
            last_mouse_click = ev.mouse.button;
            double_click_time = DOUBLE_CLICK_TIMEOUT;
        }
        
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        is_mouse_in_gui(mouse_cursor_s)
    ) {
        is_gui_focused = true;
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        if(ev.mouse.button == 1) {
            holding_m1 = false;
            handle_lmb_up(ev);
        } else if(ev.mouse.button == 2) {
            holding_m2 = false;
            if(area_editor_mmb_pan) {
                handle_mmb_up(ev);
            } else {
                handle_rmb_up(ev);
            }
        } else if(ev.mouse.button == 3) {
            holding_m3 = false;
            if(area_editor_mmb_pan) {
                handle_rmb_up(ev);
            } else {
                handle_mmb_up(ev);
            }
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED
    ) {
        if(
            fabs(ev.mouse.x - mouse_drag_start.x) >= MOUSE_DRAG_CONFIRM_RANGE ||
            fabs(ev.mouse.y - mouse_drag_start.y) >= MOUSE_DRAG_CONFIRM_RANGE
        ) {
            mouse_drag_confirmed = true;
        }
        
        if(mouse_drag_confirmed) {
            if(holding_m1) {
                handle_lmb_drag(ev);
            }
            if(holding_m2) {
                if(area_editor_mmb_pan) {
                    handle_mmb_drag(ev);
                } else {
                    handle_rmb_drag(ev);
                }
            }
            if(holding_m3) {
                if(area_editor_mmb_pan) {
                    handle_rmb_drag(ev);
                } else {
                    handle_mmb_drag(ev);
                }
            }
        }
        if(
            (ev.mouse.dz != 0 || ev.mouse.dw != 0) &&
            !is_mouse_in_gui(mouse_cursor_s)
        ) {
            handle_mouse_wheel(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            is_shift_pressed = true;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            is_ctrl_pressed = true;
            
        }
        
        if(!is_gui_focused) {
            handle_key_down(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            is_shift_pressed = false;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            is_ctrl_pressed = false;
            
        }
        
        if(!is_gui_focused) {
            handle_key_up(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        if(!is_gui_focused) {
            handle_key_char(ev);
        }
        
    }
}


//Input handler functions.
void editor::handle_key_char(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_up(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_up(const ALLEGRO_EVENT &ev) {}
void editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {}
void editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_up(const ALLEGRO_EVENT &ev) {}

//LAFI helper functions.
float editor::get_angle_picker_angle(
    lafi::widget* parent, const string &picker_name
) {
    return
        ((lafi::angle_picker*) parent->widgets[picker_name])->get_angle_rads();
}
string editor::get_button_text(
    lafi::widget* parent, const string &button_name
) {
    return
        ((lafi::button*) parent->widgets[button_name])->text;
}
bool editor::get_checkbox_check(
    lafi::widget* parent, const string &checkbox_name
) {
    return
        ((lafi::checkbox*) parent->widgets[checkbox_name])->checked;
}
string editor::get_label_text(
    lafi::widget* parent, const string &label_name
) {
    return
        ((lafi::label*) parent->widgets[label_name])->text;
}
string editor::get_textbox_text(
    lafi::widget* parent, const string &textbox_name
) {
    return
        ((lafi::textbox*) parent->widgets[textbox_name])->text;
}
bool editor::get_radio_selection(
    lafi::widget* parent, const string &radio_name
) {
    return
        ((lafi::radio_button*) parent->widgets[radio_name])->selected;
}
void editor::set_angle_picker_angle(
    lafi::widget* parent, const string &picker_name, const float angle
) {
    ((lafi::angle_picker*) parent->widgets[picker_name])->set_angle_rads(angle);
}
void editor::set_button_text(
    lafi::widget* parent, const string &button_name, const string &text
) {
    ((lafi::button*) parent->widgets[button_name])->text = text;
}
void editor::set_checkbox_check(
    lafi::widget* parent, const string &checkbox_name, const bool check
) {
    if(check) {
        ((lafi::checkbox*) parent->widgets[checkbox_name])->check();
    } else {
        ((lafi::checkbox*) parent->widgets[checkbox_name])->uncheck();
    }
}
void editor::set_label_text(
    lafi::widget* parent, const string &label_name, const string &text
) {
    ((lafi::label*) parent->widgets[label_name])->text = text;
}
void editor::set_textbox_text(
    lafi::widget* parent, const string &textbox_name, const string &text
) {
    ((lafi::textbox*) parent->widgets[textbox_name])->text = text;
}
void editor::set_radio_selection(
    lafi::widget* parent, const string &radio_name, const bool selection
) {
    if(selection) {
        ((lafi::radio_button*) parent->widgets[radio_name])->select();
    } else {
        ((lafi::radio_button*) parent->widgets[radio_name])->unselect();
    }
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
                new lafi::label(category + ":", ALLEGRO_ALIGN_LEFT);
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
        b->autoscroll = true;
        
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


/* ----------------------------------------------------------------------------
 * Updates the status bar.
 */
void editor::update_status_bar(const bool omit_coordinates) {
    string new_text;
    if(status_override_timer.time_left > 0.0f) {
        new_text = status_override_text;
        
    } else {
        if(is_mouse_in_gui(mouse_cursor_s)) {
            lafi::widget* wum =
                gui->get_widget_under_mouse(mouse_cursor_s.x, mouse_cursor_s.y);
            if(wum) {
                new_text = wum->description;
            }
        } else if(!loaded_content_yet) {
            new_text =
                "(Place the cursor on a widget "
                "to show information about it here!)";
        } else if(!omit_coordinates) {
            new_text =
                "(" + i2s(mouse_cursor_w.x) + "," + i2s(mouse_cursor_w.y) + ")";
        }
    }
    
    lbl_status_bar->text = new_text;
}


/* ----------------------------------------------------------------------------
 * Zooms in or out to a specific amount, optionally keeping the mouse cursor
 * in the same spot.
 */
void editor::zoom(const float new_zoom, const bool anchor_cursor) {
    cam_zoom = clamp(new_zoom, zoom_min_level, zoom_max_level);
    
    if(anchor_cursor) {
        //Keep a backup of the old mouse coordinates.
        point old_mouse_pos = mouse_cursor_w;
        
        //Figure out where the mouse will be after the zoom.
        update_transformations();
        mouse_cursor_w = mouse_cursor_s;
        al_transform_coordinates(
            &screen_to_world_transform,
            &mouse_cursor_w.x, &mouse_cursor_w.y
        );
        
        //Readjust the transformation by shifting the camera
        //so that the cursor ends up where it was before.
        cam_pos.x += (old_mouse_pos.x - mouse_cursor_w.x);
        cam_pos.y += (old_mouse_pos.y - mouse_cursor_w.y);
    }
    
    update_transformations();
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
    corners[0] = point(-size.x / 2.0, -size.y / 2.0);
    corners[1] = point(size.x / 2.0, -size.y / 2.0);
    corners[2] = point(size.x / 2.0, size.y / 2.0);
    corners[3] = point(-size.x / 2.0, size.y / 2.0);
    for(unsigned char c = 0; c < 4; ++c) {
        al_transform_coordinates(
            &disalign_transform, &corners[c].x, &corners[c].y
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


/* ----------------------------------------------------------------------------
 * Adds a new boolean to the list.
 */
void editor::gui_to_var_helper::register_bool(
    bool* var, const bool gui_value
) {
    bools[var] = gui_value;
}


/* ----------------------------------------------------------------------------
 * Adds a new int to the list.
 */
void editor::gui_to_var_helper::register_int(
    int* var, const int gui_value
) {
    ints[var] = gui_value;
}


/* ----------------------------------------------------------------------------
 * Adds a new float to the list.
 */
void editor::gui_to_var_helper::register_float(
    float* var, const float gui_value
) {
    floats[var] = gui_value;
}


/* ----------------------------------------------------------------------------
 * Adds a new unsigned char to the list.
 */
void editor::gui_to_var_helper::register_uchar(
    unsigned char* var, const unsigned char gui_value
) {
    uchars[var] = gui_value;
}


/* ----------------------------------------------------------------------------
 * Adds a new string to the list.
 */
void editor::gui_to_var_helper::register_string(
    string* var, const string &gui_value
) {
    strings[var] = gui_value;
}


/* ----------------------------------------------------------------------------
 * Adds a new color to the list.
 */
void editor::gui_to_var_helper::register_color(
    ALLEGRO_COLOR* var, const ALLEGRO_COLOR &gui_value
) {
    colors[var] = gui_value;
}


/* ----------------------------------------------------------------------------
 * Adds a new point to the list.
 */
void editor::gui_to_var_helper::register_point(
    point* var, const point &gui_value
) {
    points[var] = gui_value;
}


/* ----------------------------------------------------------------------------
 * Returns true if all registered variables equal the given GUI values.
 */
bool editor::gui_to_var_helper::all_equal() {
    for(auto b = bools.begin(); b != bools.end(); ++b) {
        if(*(b->first) != b->second) return false;
    }
    for(auto i = ints.begin(); i != ints.end(); ++i) {
        if(*(i->first) != i->second) return false;
    }
    for(auto f = floats.begin(); f != floats.end(); ++f) {
        if(*(f->first) != f->second) return false;
    }
    for(auto c = uchars.begin(); c != uchars.end(); ++c) {
        if(*(c->first) != c->second) return false;
    }
    for(auto s = strings.begin(); s != strings.end(); ++s) {
        if(*(s->first) != s->second) return false;
    }
    for(auto c = colors.begin(); c != colors.end(); ++c) {
        if((c->first)->r != c->second.r) return false;
        if((c->first)->g != c->second.g) return false;
        if((c->first)->b != c->second.b) return false;
        if((c->first)->a != c->second.a) return false;
    }
    for(auto p = points.begin(); p != points.end(); ++p) {
        if(*(p->first) != p->second) return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Sets all variables to the given GUI values.
 */
void editor::gui_to_var_helper::set_all() {
    for(auto b = bools.begin(); b != bools.end(); ++b) {
        *(b->first) = b->second;
    }
    for(auto i = ints.begin(); i != ints.end(); ++i) {
        *(i->first) = i->second;
    }
    for(auto f = floats.begin(); f != floats.end(); ++f) {
        *(f->first) = f->second;
    }
    for(auto c = uchars.begin(); c != uchars.end(); ++c) {
        *(c->first) = c->second;
    }
    for(auto s = strings.begin(); s != strings.end(); ++s) {
        *(s->first) = s->second;
    }
    for(auto c = colors.begin(); c != colors.end(); ++c) {
        *(c->first) = c->second;
    }
    for(auto p = points.begin(); p != points.end(); ++p) {
        *(p->first) = p->second;
    }
}


//Runs custom code when the user presses the "cancel" button on a picker.
void editor::custom_picker_cancel_action() { }
