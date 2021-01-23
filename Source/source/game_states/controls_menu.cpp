/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Controls menu state class and controls menu state-related functions.
 */

#include <algorithm>

#include "menus.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../load.h"
#include "../utils/string_utils.h"


//Path to the GUI information file.
const string controls_menu_state::GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Controls_menu.txt";


/* ----------------------------------------------------------------------------
 * Creates a "controls menu" state.
 */
controls_menu_state::controls_menu_state() :
    game_state(),
    bmp_menu_bg(NULL),
    list_box(nullptr),
    capturing_input(false),
    input_capture_control_nr(0) {
    
}


/* ----------------------------------------------------------------------------
 * Adds a control to the player's controls.
 */
void controls_menu_state::add_control() {
    if(game.options.controls[0].size()) {
        size_t last_action =
            game.options.controls[0].back().action;
        game.options.controls[0].push_back(
            control_info(
                last_action == N_BUTTONS - 1 ?
                1 : //The "None" action is 0, so go to 1.
                last_action + 1,
                ""
            )
        );
    } else {
        game.options.controls[0].push_back(
            control_info(BUTTON_NONE, "")
        );
    }
}


/* ----------------------------------------------------------------------------
 * Adds GUI items about a control to the list.
 * index:
 *   Index number of the control.
 * focus:
 *   If true, focus on this new control.
 */
void controls_menu_state::add_control_gui_items(
    const size_t index, const bool focus
) {
    float items_y = 0.045f + index * 0.08f;
    
    //Delete button.
    button_gui_item* delete_button =
        new button_gui_item("-", game.fonts.main);
    delete_button->on_activate =
    [this, index] (const point &) {
        delete_control(index);
        delete_control_gui_items();
    };
    delete_button->center = point(0.07f, items_y);
    delete_button->size = point(0.08f, 0.07f);
    list_box->add_child(delete_button);
    gui.add_item(delete_button);
    
    //Previous action button.
    button_gui_item* prev_action_button =
        new button_gui_item("<", game.fonts.main);
    prev_action_button->on_activate =
    [this, index] (const point &) {
        choose_prev_action(index);
    };
    prev_action_button->center = point(0.16f, items_y);
    prev_action_button->size = point(0.08f, 0.07f);
    list_box->add_child(prev_action_button);
    gui.add_item(prev_action_button);
    
    //Action name.
    text_gui_item* action_name_text =
        new text_gui_item("", game.fonts.main);
    action_name_text->on_draw =
        [this, index, action_name_text]
    (const point & center, const point & size) {
        control_info* c_ptr = &game.options.controls[0][index];
        
        string action_name;
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            if(c_ptr->action == game.buttons.list[b].id) {
                action_name = game.buttons.list[b].name;
                break;
            }
        }
        
        float juicy_grow_amount = action_name_text->get_juicy_grow_amount();
        
        draw_compressed_scaled_text(
            game.fonts.main, map_gray(255),
            center,
            point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
            ALLEGRO_ALIGN_CENTER, 1, size,
            action_name
        );
    };
    action_name_text->center = point(0.40f, items_y);
    action_name_text->size = point(0.39f, 0.07f);
    list_box->add_child(action_name_text);
    gui.add_item(action_name_text);
    
    //Next action button.
    button_gui_item* next_action_button =
        new button_gui_item(">", game.fonts.main);
    next_action_button->on_activate =
    [this, index] (const point &) {
        choose_next_action(index);
    };
    next_action_button->center = point(0.65f, items_y);
    next_action_button->size = point(0.08f, 0.07f);
    list_box->add_child(next_action_button);
    gui.add_item(next_action_button);
    
    //Control button.
    button_gui_item* control_button =
        new button_gui_item("", game.fonts.main);
    control_button->on_activate =
    [this, index] (const point &) {
        choose_button(index);
    };
    control_button->on_draw =
        [this, index, control_button]
    (const point & center, const point & size) {
        control_info* c_ptr = &game.options.controls[0][index];
        
        draw_control(game.fonts.main, *c_ptr, center, size * 0.8f);
        
        draw_button(
            center, size, "", game.fonts.main, map_gray(255),
            control_button->selected,
            control_button->get_juicy_grow_amount()
        );
    };
    control_button->center = point(0.83f, items_y);
    control_button->size = point(0.26f, 0.07f);
    list_box->add_child(control_button);
    gui.add_item(control_button);
    
    //Focus, if requested.
    if(focus) {
        action_name_text->start_juicy_grow();
        float list_bottom = list_box->get_child_bottom();
        if(list_bottom > 1.0f) {
            list_box->target_offset = list_bottom - 1.0f;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Chooses the button for a given control.
 * index:
 *   Index number of the control.
 */
void controls_menu_state::choose_button(const size_t index) {
    capturing_input = true;
    input_capture_control_nr = index;
}


/* ----------------------------------------------------------------------------
 * Chooses the next action for a given control.
 * index:
 *   Index number of the control.
 */
void controls_menu_state::choose_next_action(const size_t index) {
    control_info* c_ptr = &game.options.controls[0][index];
    c_ptr->action = sum_and_wrap(c_ptr->action, 1, N_BUTTONS);
    gui_item* action_name_text = list_box->children[index * 5 + 2];
    ((text_gui_item*) action_name_text)->start_juicy_grow();
}


/* ----------------------------------------------------------------------------
 * Chooses the previous action for a given control.
 * index:
 *   Index number of the control.
 */
void controls_menu_state::choose_prev_action(const size_t index) {
    control_info* c_ptr = &game.options.controls[0][index];
    c_ptr->action = sum_and_wrap(c_ptr->action, -1, N_BUTTONS);
    gui_item* action_name_text = list_box->children[index * 5 + 2];
    ((text_gui_item*) action_name_text)->start_juicy_grow();
}


/* ----------------------------------------------------------------------------
 * Deletes a control from the player's controls.
 * index:
 *   Index number of the control.
 */
void controls_menu_state::delete_control(const size_t index) {
    game.options.controls[0].erase(
        game.options.controls[0].begin() + index
    );
}


/* ----------------------------------------------------------------------------
 * Deletes the GUI items about a control in the list.
 */
void controls_menu_state::delete_control_gui_items() {
    //We only need to delete the latest control GUI items.
    for(size_t i = 0; i < 5; ++i) {
        //Iterate through all five items.
        gui_item* i_ptr = list_box->children[list_box->children.size() - 1];
        list_box->remove_child(i_ptr);
        gui.remove_item(i_ptr);
        delete i_ptr;
    }
}


/* ----------------------------------------------------------------------------
 * Draws the controls menu.
 */
void controls_menu_state::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
    draw_bitmap(
        bmp_menu_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h), 0, map_gray(64)
    );
    
    gui.draw();
    
    if(capturing_input) {
        al_draw_filled_rectangle(
            0, 0, game.win_w, game.win_h,
            al_map_rgba(24, 24, 32, 192)
        );
        
        draw_text_lines(
            game.fonts.main,
            al_map_rgb(255, 255, 255),
            point(game.win_w / 2.0f, game.win_h / 2.0f),
            ALLEGRO_ALIGN_CENTER,
            1,
            "Waiting for any input..."
        );
    }
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void controls_menu_state::do_logic() {
    game.fade_mgr.tick(game.delta_t);
    
    gui.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string controls_menu_state::get_name() const {
    return "controls menu";
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 * ev:
 *   Event to handle.
 */
void controls_menu_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    if(capturing_input) {
    
        control_info* c_ptr =
            &game.options.controls[0][input_capture_control_nr];
        bool valid = true;
        
        switch(ev.type) {
        case ALLEGRO_EVENT_KEY_DOWN: {
            c_ptr->type = CONTROL_TYPE_KEYBOARD_KEY;
            c_ptr->button = ev.keyboard.keycode;
            break;
            
        } case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
            c_ptr->type = CONTROL_TYPE_MOUSE_BUTTON;
            c_ptr->button = ev.mouse.button;
            break;
            
        } case ALLEGRO_EVENT_MOUSE_AXES: {
            if(ev.mouse.dz > 0) {
                c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_UP;
            } else if(ev.mouse.dz < 0) {
                c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_DOWN;
            } else if(ev.mouse.dw > 0) {
                c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_RIGHT;
            } else if(ev.mouse.dw < 0) {
                c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_LEFT;
            } else {
                valid = false;
            }
            break;
            
        } case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN: {
            c_ptr->type = CONTROL_TYPE_JOYSTICK_BUTTON;
            c_ptr->device_nr = game.joystick_numbers[ev.joystick.id];
            c_ptr->button = ev.joystick.button;
            break;
            
        } case ALLEGRO_EVENT_JOYSTICK_AXIS: {
            c_ptr->type =
                (
                    ev.joystick.pos > 0 ?
                    CONTROL_TYPE_JOYSTICK_AXIS_POS :
                    CONTROL_TYPE_JOYSTICK_AXIS_NEG
                );
            c_ptr->device_nr = game.joystick_numbers[ev.joystick.id];
            c_ptr->stick = ev.joystick.stick;
            c_ptr->axis = ev.joystick.axis;
            break;
            
        } default: {
            valid = false;
            break;
            
        }
        }
        
        if(valid) {
            capturing_input = false;
        }
        
    } else {
    
        gui.handle_event(ev);
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Leaves the controls menu and goes to the options menu.
 */
void controls_menu_state::leave() {
    game.fade_mgr.start_fade(false, [] () {
        game.change_state(game.states.options_menu);
    });
    save_options();
}


/* ----------------------------------------------------------------------------
 * Loads the controls menu into memory.
 */
void controls_menu_state::load() {
    bmp_menu_bg = NULL;
    capturing_input = false;
    
    //Resources.
    bmp_menu_bg = load_bmp(game.asset_file_names.main_menu);
    
    //Menu items.
    gui.register_coords("back",        15, 10, 20,  6);
    gui.register_coords("new",         80, 10, 30,  7);
    gui.register_coords("list",        48, 55, 86, 80);
    gui.register_coords("list_scroll", 94, 55,  2, 80);
    gui.read_coords(
        data_node(GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    gui.back_item =
        new button_gui_item("Back", game.fonts.main);
    gui.back_item->on_activate =
    [this] (const point &) {
        leave();
    };
    gui.add_item(gui.back_item, "back");
    
    list_box = new list_gui_item();
    gui.add_item(list_box, "list");
    
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = list_box;
    list_box->scroll_item = list_scroll;
    gui.add_item(list_scroll, "list_scroll");
    
    button_gui_item* new_button =
        new button_gui_item("New...", game.fonts.main);
    new_button->on_activate =
    [this] (const point &) {
        add_control();
        add_control_gui_items(game.options.controls[0].size() - 1, true);
    };
    gui.add_item(new_button, "new");
    
    for(size_t c = 0; c < game.options.controls[0].size(); ++c) {
        add_control_gui_items(c, false);
    }
    
    //Finishing touches.
    game.fade_mgr.start_fade(true, nullptr);
    if(list_box->children.size()) {
        gui.set_selected_item(list_box->children[4]);
    } else {
        gui.set_selected_item(gui.back_item);
    }
    
    al_reconfigure_joysticks();
}


/* ----------------------------------------------------------------------------
 * Unloads the controls menu from memory.
 */
void controls_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu items.
    gui.destroy();
    
}
