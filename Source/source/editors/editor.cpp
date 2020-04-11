/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Editor-related functions.
 */

#include "editor.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../LAFI/angle_picker.h"
#include "../LAFI/button.h"
#include "../LAFI/checkbox.h"
#include "../LAFI/radio_button.h"
#include "../LAFI/scrollbar.h"
#include "../LAFI/textbox.h"
#include "../load.h"
#include "../utils/math_utils.h"
#include "../utils/string_utils.h"
#include "../vars.h"


//Every icon in the icon bitmap file is these many pixels from the previous.
const int editor::EDITOR_ICON_BMP_PADDING = 1;
//Every icon in the icon bitmap file has this size.
const int editor::EDITOR_ICON_BMP_SIZE = 24;
//Time until the next click is no longer considered a double-click.
const float editor::DOUBLE_CLICK_TIMEOUT = 0.5f;
//How much to zoom in/out with the keyboard keys.
const float editor::KEYBOARD_CAM_ZOOM = 0.25f;
//How long to override the status bar text for, for important messages.
const float editor::STATUS_OVERRIDE_IMPORTANT_DURATION = 6.0f;
//How long to override the status bar text for, for unimportant messages.
const float editor::STATUS_OVERRIDE_UNIMPORTANT_DURATION = 1.5f;
//How long the unsaved changes warning stays on-screen for.
const float editor::UNSAVED_CHANGES_WARNING_DURATION = 3.0f;
//Height of the unsaved changes warning, sans spike.
const int editor::UNSAVED_CHANGES_WARNING_HEIGHT = 30;
//Width and height of the unsaved changes warning's spike.
const int editor::UNSAVED_CHANGES_WARNING_SPIKE_SIZE = 16;
//Width of the unsaved changes warning, sans spike.
const int editor::UNSAVED_CHANGES_WARNING_WIDTH = 150;

/* ----------------------------------------------------------------------------
 * Initializes editor class stuff.
 */
editor::editor() :
    cur_picker_id(0),
    double_click_time(0),
    frm_picker(nullptr),
    frm_toolbar(nullptr),
    gui(nullptr),
    holding_m1(false),
    holding_m2(false),
    holding_m3(false),
    is_ctrl_pressed(false),
    is_gui_focused(false),
    is_shift_pressed(false),
    last_mouse_click(INVALID),
    loaded_content_yet(false),
    made_new_changes(false),
    mouse_drag_confirmed(false),
    state(0),
    sub_state(0),
    unsaved_changes_warning_timer(UNSAVED_CHANGES_WARNING_DURATION),
    zoom_max_level(0),
    zoom_min_level(0) {
    
    editor_icons.reserve(N_EDITOR_ICONS);
    for(size_t i = 0; i < N_EDITOR_ICONS; ++i) {
        editor_icons.push_back(NULL);
    }
    
    status_override_timer =
    timer(STATUS_OVERRIDE_IMPORTANT_DURATION, [this] () {update_status_bar();});
    
}


/* ----------------------------------------------------------------------------
 * Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the screen,
 * where it's hard to see.
 */
void editor::center_camera(
    const point &min_coords, const point &max_coords
) {
    point min_c = min_coords;
    point max_c = max_coords;
    if(min_c == max_c) {
        min_c = min_c - 2.0;
        max_c = max_c + 2.0;
    }
    
    float width = max_c.x - min_c.x;
    float height = max_c.y - min_c.y;
    
    cam_pos.x = floor(min_c.x + width  / 2);
    cam_pos.y = floor(min_c.y + height / 2);
    
    float z;
    if(width > height) z = (canvas_br.x - canvas_tl.x) / width;
    else z = (canvas_br.y - canvas_tl.y) / height;
    z -= z * 0.1;
    
    zoom(z, false);
}


/* ----------------------------------------------------------------------------
 * Checks if there are any unsaved changes that have not been notified yet.
 * Returns true if there are, and also sets up the unsaved changes warning.
 * Returns false if everything is okay to continue.
 * caller_widget: Widget that summoned this warning.
 */
bool editor::check_new_unsaved_changes(lafi::widget* caller_widget) {
    unsaved_changes_warning_timer.stop();
    
    if(!made_new_changes) return false;
    made_new_changes = false;
    
    unsaved_changes_warning_pos =
        point(
            (caller_widget->x1 + caller_widget->x2) / 2.0,
            (caller_widget->y1 + caller_widget->y2) / 2.0
        );
    unsaved_changes_warning_timer.start();
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Creates a "picker" frame in the gui, used for picking objects
 * from a list.
 */
void editor::create_picker_frame() {

    frm_picker =
        new lafi::frame(canvas_br.x, 0, game.win_w, game.win_h);
    frm_picker->hide();
    gui->add("frm_picker", frm_picker);
    
    frm_picker->add(
        "but_back",
        new lafi::button(canvas_br.x + 8, 8, canvas_br.x + 96, 24, "Back")
    );
    frm_picker->add(
        "lbl_title",
        new lafi::label(canvas_br.x + 8, 32, game.win_w - 8, 44)
    );
    frm_picker->add(
        "txt_text",
        new lafi::textbox(canvas_br.x + 8, 52, game.win_w - 48, 68)
    );
    frm_picker->add(
        "but_new",
        new lafi::button(game.win_w - 40, 44, game.win_w - 8, 76, "+")
    );
    
    frm_picker->add(
        "frm_list",
        new lafi::frame(canvas_br.x + 8, 84, game.win_w - 32, game.win_h - 8)
    );
    frm_picker->add(
        "bar_scroll",
        new lafi::scrollbar(game.win_w - 24, 84, game.win_w - 8, game.win_h - 8)
    );
    
    
    frm_picker->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        this->frm_picker->hide();
        frm_toolbar->show();
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
        
        this->create_new_from_picker(cur_picker_id, name);
        
        made_new_changes = true;
        
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
 * Creates the status bar.
 */
void editor::create_status_bar() {
    lbl_status_bar =
        new lafi::label(0, canvas_br.y, canvas_br.x, game.win_h, "", 0, true);
    gui->add("lbl_status_bar", lbl_status_bar);
}


/* ----------------------------------------------------------------------------
 * Creates the toolbar frame.
 */
void editor::create_toolbar_frame() {
    frm_toolbar =
        new lafi::frame(0, 0, canvas_br.x, canvas_tl.y);
    gui->add("frm_toolbar", frm_toolbar);
    frm_toolbar->solid_color_only = true;
}


//Runs custom code when the user presses the "cancel" button on a picker.
void editor::custom_picker_cancel_action() { }


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor. This is meant to
 * be run after the editor's own logic code.
 */
void editor::do_logic_post() {
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor. This is meant to
 * be run before the editor's own logic code.
 */
void editor::do_logic_pre() {
    gui->tick(game.delta_t);
    
    update_transformations();
    
    if(double_click_time > 0) {
        double_click_time -= game.delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
    
    unsaved_changes_warning_timer.tick(game.delta_t);
    status_override_timer.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Draws the unsaved changes warning, if it is visible.
 */
void editor::draw_unsaved_changes_warning() {
    float r = unsaved_changes_warning_timer.get_ratio_left();
    if(r == 0) return;
    
    ALLEGRO_COLOR back_color = al_map_rgba(192, 192, 64, r * 255);
    ALLEGRO_COLOR outline_color = al_map_rgba(80, 80, 16, r * 255);
    ALLEGRO_COLOR text_color = al_map_rgba(0, 0, 0, r * 255);
    bool spike_up = unsaved_changes_warning_pos.y < game.win_h / 2.0;
    
    point box_center = unsaved_changes_warning_pos;
    if(unsaved_changes_warning_pos.x < UNSAVED_CHANGES_WARNING_WIDTH / 2.0) {
        box_center.x +=
            UNSAVED_CHANGES_WARNING_WIDTH / 2.0 - unsaved_changes_warning_pos.x;
    } else if(
        unsaved_changes_warning_pos.x >
        game.win_w - UNSAVED_CHANGES_WARNING_WIDTH / 2.0
    ) {
        box_center.x -=
            unsaved_changes_warning_pos.x -
            (game.win_w - UNSAVED_CHANGES_WARNING_WIDTH / 2.0);
    }
    if(spike_up) {
        box_center.y += UNSAVED_CHANGES_WARNING_HEIGHT / 2.0;
        box_center.y += UNSAVED_CHANGES_WARNING_SPIKE_SIZE;
    } else {
        box_center.y -= UNSAVED_CHANGES_WARNING_HEIGHT / 2.0;
        box_center.y -= UNSAVED_CHANGES_WARNING_SPIKE_SIZE;
    }
    
    point box_tl(
        box_center.x - UNSAVED_CHANGES_WARNING_WIDTH / 2.0,
        box_center.y - UNSAVED_CHANGES_WARNING_HEIGHT / 2.0
    );
    point box_br(
        box_center.x + UNSAVED_CHANGES_WARNING_WIDTH / 2.0,
        box_center.y + UNSAVED_CHANGES_WARNING_HEIGHT / 2.0
    );
    point spike_p1(
        unsaved_changes_warning_pos.x,
        unsaved_changes_warning_pos.y
    );
    point spike_p2(
        unsaved_changes_warning_pos.x -
        UNSAVED_CHANGES_WARNING_SPIKE_SIZE / 2.0,
        unsaved_changes_warning_pos.y +
        UNSAVED_CHANGES_WARNING_SPIKE_SIZE * (spike_up ? 1 : -1)
    );
    point spike_p3(
        unsaved_changes_warning_pos.x +
        UNSAVED_CHANGES_WARNING_SPIKE_SIZE / 2.0,
        unsaved_changes_warning_pos.y +
        UNSAVED_CHANGES_WARNING_SPIKE_SIZE * (spike_up ? 1 : -1)
    );
    
    al_draw_filled_rectangle(
        box_tl.x, box_tl.y, box_br.x, box_br.y, back_color
    );
    al_draw_filled_triangle(
        spike_p1.x, spike_p1.y, spike_p2.x, spike_p2.y, spike_p3.x, spike_p3.y,
        back_color
    );
    al_draw_rectangle(
        box_tl.x, box_tl.y, box_br.x, box_br.y, outline_color, 2
    );
    al_draw_line(
        spike_p2.x, spike_p2.y, spike_p3.x, spike_p3.y,
        back_color, 2
    );
    al_draw_line(
        spike_p1.x, spike_p1.y, spike_p2.x, spike_p2.y,
        outline_color, 2
    );
    al_draw_line(
        spike_p1.x, spike_p1.y, spike_p3.x, spike_p3.y,
        outline_color, 2
    );
    draw_text_lines(
        gui->style->text_font, text_color,
        box_center, ALLEGRO_ALIGN_CENTER, 1,
        "You have\nunsaved changes!"
    );
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
 * picker_id:    ID of the picker that is being generated.
 * elements:     List of elements to populate the picker with. This is a list
 *   of string+string pairs, where the first element is the
 *   category name (optional), and the second is the name
 *   of the element proper.
 * title:        Title of the picker, to place above the list. This is normally
 *   a request to the user, like "Pick an area.".
 * can_make_new: If true, the user can create a new element, by writing its
 *   name on the textbox, and pressing the "+" button.
 */
void editor::generate_and_open_picker(
    const size_t picker_id, const vector<std::pair<string, string> > &elements,
    const string &title, const bool can_make_new
) {

    cur_picker_id = picker_id;
    
    hide_all_frames();
    frm_picker->show();
    frm_toolbar->hide();
    
    set_label_text(frm_picker, "lbl_title", title);
    set_textbox_text(frm_picker, "txt_text", "");
    
    if(can_make_new) {
        ((lafi::textbox*) frm_picker->widgets["txt_text"])->placeholder =
            "(enter search term / new name)";
        enable_widget(frm_picker->widgets["but_new"]);
    } else {
        ((lafi::textbox*) frm_picker->widgets["txt_text"])->placeholder =
            "(enter search term)";
        disable_widget(frm_picker->widgets["but_new"]);
    }
    
    picker_elements = elements;
    populate_picker("");
}


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
bool editor::get_radio_selection(
    lafi::widget* parent, const string &radio_name
) {
    return
        ((lafi::radio_button*) parent->widgets[radio_name])->selected;
}
string editor::get_textbox_text(
    lafi::widget* parent, const string &textbox_name
) {
    return
        ((lafi::textbox*) parent->widgets[textbox_name])->text;
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event for control-related things.
 */
void editor::handle_controls(const ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
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
        !is_mouse_in_gui(game.mouse_cursor_s)
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
                handle_rmb_double_click(ev);
            } else if(ev.mouse.button == 3) {
                handle_mmb_double_click(ev);
            }
            
            double_click_time = 0;
            
        } else {
            if(ev.mouse.button == 1) {
                handle_lmb_down(ev);
            } else if(ev.mouse.button == 2) {
                handle_rmb_down(ev);
            } else if(ev.mouse.button == 3) {
                handle_mmb_down(ev);
            }
            
            last_mouse_click = ev.mouse.button;
            double_click_time = DOUBLE_CLICK_TIMEOUT;
        }
        
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        is_mouse_in_gui(game.mouse_cursor_s)
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
            handle_rmb_up(ev);
        } else if(ev.mouse.button == 3) {
            holding_m3 = false;
            handle_mmb_up(ev);
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED
    ) {
        if(
            fabs(ev.mouse.x - mouse_drag_start.x) >=
            editor_mouse_drag_threshold ||
            fabs(ev.mouse.y - mouse_drag_start.y) >=
            editor_mouse_drag_threshold
        ) {
            mouse_drag_confirmed = true;
        }
        
        if(mouse_drag_confirmed) {
            if(holding_m1) {
                handle_lmb_drag(ev);
            }
            if(holding_m2) {
                handle_rmb_drag(ev);
            }
            if(holding_m3) {
                handle_mmb_drag(ev);
            }
        }
        if(
            (ev.mouse.dz != 0 || ev.mouse.dw != 0) &&
            !is_mouse_in_gui(game.mouse_cursor_s)
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
        
        handle_key_down_anywhere(ev);
        if(!is_gui_focused) {
            handle_key_down_canvas(ev);
        }
        
        if(
            !(frm_picker->flags & lafi::FLAG_INVISIBLE) &&
            ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE
        ) {
            frm_picker->widgets["but_back"]->simulate_click();
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
        
        handle_key_up_anywhere(ev);
        if(!is_gui_focused) {
            handle_key_up_canvas(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        handle_key_char_anywhere(ev);
        if(!is_gui_focused) {
            handle_key_char_canvas(ev);
        }
        
    }
}


//Input handler functions.
void editor::handle_key_char_anywhere(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_up_anywhere(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_up_canvas(const ALLEGRO_EVENT &ev) {}
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


/* ----------------------------------------------------------------------------
 * Returns whether the mouse cursor is inside the gui or not.
 * The status bar counts as the gui.
 */
bool editor::is_mouse_in_gui(const point &mouse_coords) {
    return
        mouse_coords.x >= canvas_br.x || mouse_coords.y >= canvas_br.y ||
        mouse_coords.x <= canvas_tl.x || mouse_coords.y <= canvas_tl.y;
}


/* ----------------------------------------------------------------------------
 * Exits out of the editor, with a fade.
 */
void editor::leave() {
    game.fade_mgr.start_fade(false, [] () {
        if(game.area_editor_state->quick_play_area.empty()) {
            game.change_state(game.main_menu_state);
        } else {
            game.gameplay_state->area_to_load =
                game.area_editor_state->quick_play_area;
            game.change_state(game.gameplay_state);
        }
    });
}


/* ----------------------------------------------------------------------------
 * Loads content common for all editors.
 */
void editor::load() {
    bmp_editor_icons =
        load_bmp(asset_file_names.editor_icons, NULL, true, false);
    if(bmp_editor_icons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; ++i) {
            editor_icons[i] =
                al_create_sub_bitmap(
                    bmp_editor_icons,
                    EDITOR_ICON_BMP_SIZE * i + EDITOR_ICON_BMP_PADDING * i,
                    0, EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE
                );
        }
    }
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
            pick(cur_picker_id, name, category);
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
void editor::set_radio_selection(
    lafi::widget* parent, const string &radio_name, const bool selection
) {
    if(selection) {
        ((lafi::radio_button*) parent->widgets[radio_name])->select();
    } else {
        ((lafi::radio_button*) parent->widgets[radio_name])->unselect();
    }
}
void editor::set_textbox_text(
    lafi::widget* parent, const string &textbox_name, const string &text
) {
    ((lafi::textbox*) parent->widgets[textbox_name])->text = text;
}


/* ----------------------------------------------------------------------------
 * Unloads loaded editor-related content.
 */
void editor::unload() {
    if(bmp_editor_icons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; ++i) {
            al_destroy_bitmap(editor_icons[i]);
            editor_icons[i] = NULL;
        }
        al_destroy_bitmap(bmp_editor_icons);
        bmp_editor_icons = NULL;
    }
}


/* ----------------------------------------------------------------------------
 * Updates the variables that hold the canvas's coordinates.
 */
void editor::update_canvas_coordinates() {
    canvas_tl.x = 0;
    canvas_tl.y = 40;
    canvas_br.x = game.win_w * 0.675;
    canvas_br.y = game.win_h - 16;
}


/* ----------------------------------------------------------------------------
 * Updates the status bar.
 */
void editor::update_status_bar(
    const bool omit_coordinates, const bool reverse_y_coordinate
) {
    string new_text;
    if(status_override_timer.time_left > 0.0f) {
        new_text = status_override_text;
        
    } else {
        if(is_mouse_in_gui(game.mouse_cursor_s)) {
            lafi::widget* wum =
                gui->get_widget_under_mouse(
                    game.mouse_cursor_s.x, game.mouse_cursor_s.y
                );
            if(wum) {
                new_text = wum->description;
            }
        } else if(!loaded_content_yet) {
            new_text =
                "(Place the cursor on a widget "
                "to show information about it here!)";
        } else if(!omit_coordinates) {
            new_text =
                "(" + i2s(game.mouse_cursor_w.x) + "," +
                i2s(
                    reverse_y_coordinate ?
                    -game.mouse_cursor_w.y :
                    game.mouse_cursor_w.y
                ) + ")";
        }
    }
    
    lbl_status_bar->text = new_text;
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void editor::update_transformations() {
    //World coordinates to screen coordinates.
    point canvas_center(
        (canvas_tl.x + canvas_br.x) / 2.0,
        (canvas_tl.y + canvas_br.y) / 2.0
    );
    game.world_to_screen_transform = game.identity_transform;
    al_translate_transform(
        &game.world_to_screen_transform,
        -cam_pos.x + canvas_center.x / cam_zoom,
        -cam_pos.y + canvas_center.y / cam_zoom
    );
    al_scale_transform(&game.world_to_screen_transform, cam_zoom, cam_zoom);
    
    //Screen coordinates to world coordinates.
    game.screen_to_world_transform = game.world_to_screen_transform;
    al_invert_transform(&game.screen_to_world_transform);
}


/* ----------------------------------------------------------------------------
 * Zooms in or out to a specific amount, optionally keeping the mouse cursor
 * in the same spot.
 */
void editor::zoom(const float new_zoom, const bool anchor_cursor) {
    cam_zoom = clamp(new_zoom, zoom_min_level, zoom_max_level);
    
    if(anchor_cursor) {
        //Keep a backup of the old mouse coordinates.
        point old_mouse_pos = game.mouse_cursor_w;
        
        //Figure out where the mouse will be after the zoom.
        update_transformations();
        game.mouse_cursor_w = game.mouse_cursor_s;
        al_transform_coordinates(
            &game.screen_to_world_transform,
            &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
        );
        
        //Readjust the transformation by shifting the camera
        //so that the cursor ends up where it was before.
        cam_pos.x += (old_mouse_pos.x - game.mouse_cursor_w.x);
        cam_pos.y += (old_mouse_pos.y - game.mouse_cursor_w.y);
    }
    
    update_transformations();
}


/* ----------------------------------------------------------------------------
 * Returns true if all registered variables equal the given GUI values.
 */
bool editor::gui_to_var_helper::all_equal() {
    for(auto &b : bools) {
        if(*(b.first) != b.second) return false;
    }
    for(auto &i : ints) {
        if(*(i.first) != i.second) return false;
    }
    for(auto &f : floats) {
        if(*(f.first) != f.second) return false;
    }
    for(auto &c : uchars) {
        if(*(c.first) != c.second) return false;
    }
    for(auto &s : strings) {
        if(*(s.first) != s.second) return false;
    }
    for(auto &c : colors) {
        if((c.first)->r != c.second.r) return false;
        if((c.first)->g != c.second.g) return false;
        if((c.first)->b != c.second.b) return false;
        if((c.first)->a != c.second.a) return false;
    }
    for(auto &p : points) {
        if(*(p.first) != p.second) return false;
    }
    return true;
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
 * Adds a new color to the list.
 */
void editor::gui_to_var_helper::register_color(
    ALLEGRO_COLOR* var, const ALLEGRO_COLOR &gui_value
) {
    colors[var] = gui_value;
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
 * Adds a new int to the list.
 */
void editor::gui_to_var_helper::register_int(
    int* var, const int gui_value
) {
    ints[var] = gui_value;
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
 * Adds a new string to the list.
 */
void editor::gui_to_var_helper::register_string(
    string* var, const string &gui_value
) {
    strings[var] = gui_value;
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
 * Sets all variables to the given GUI values.
 */
void editor::gui_to_var_helper::set_all() {
    for(auto &b : bools) {
        *(b.first) = b.second;
    }
    for(auto &i : ints) {
        *(i.first) = i.second;
    }
    for(auto &f : floats) {
        *(f.first) = f.second;
    }
    for(auto &c : uchars) {
        *(c.first) = c.second;
    }
    for(auto &s : strings) {
        *(s.first) = s.second;
    }
    for(auto &c : colors) {
        *(c.first) = c.second;
    }
    for(auto &p : points) {
        *(p.first) = p.second;
    }
}


const float editor::transformation_controller::HANDLE_RADIUS = 6.0;
const float editor::transformation_controller::ROTATION_HANDLE_THICKNESS = 8.0;


/* ----------------------------------------------------------------------------
 * Creates a transformation controller.
 */
editor::transformation_controller::transformation_controller() :
    moving_handle(-1),
    size(point(16, 16)),
    angle(0),
    keep_aspect_ratio(true),
    allow_rotation(false) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the transformation (move, scale, rotate) handles.
 */
void editor::transformation_controller::draw_handles() {
    if(size.x == 0 || size.y == 0) return;
    
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
 * Returns the angle.
 */
float editor::transformation_controller::get_angle() {
    return angle;
}


/* ----------------------------------------------------------------------------
 * Returns the center.
 */
point editor::transformation_controller::get_center() {
    return center;
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
 * Returns the size.
 */
point editor::transformation_controller::get_size() {
    return size;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse press, allowing a handle to be grabbed.
 * Returns true if handled, false if nothing was done.
 */
bool editor::transformation_controller::handle_mouse_down(const point pos) {
    if(size.x == 0 || size.y == 0) return false;
    
    for(unsigned char h = 0; h < 9; ++h) {
        point handle_pos = get_handle_pos(h);
        if(dist(handle_pos, pos) <= HANDLE_RADIUS / cam_zoom) {
            moving_handle = h;
            pre_move_size = size;
            return true;
        }
    }
    
    if(allow_rotation) {
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
    }
    
    return false;
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
 * Handles a mouse release, allowing a handle to be released.
 */
void editor::transformation_controller::handle_mouse_up() {
    moving_handle = -1;
}


/* ----------------------------------------------------------------------------
 * Sets the angle.
 */
void editor::transformation_controller::set_angle(const float angle) {
    this->angle = angle;
    update();
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
