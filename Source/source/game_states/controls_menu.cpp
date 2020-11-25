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


/* ----------------------------------------------------------------------------
 * Creates a "controls menu" state.
 */
controls_menu_state::controls_menu_state() :
    game_state(),
    bmp_menu_bg(NULL),
    time_spent(0),
    cur_player_nr(0),
    cur_page_nr(0),
    cur_player_nr_widget(NULL),
    cur_page_nr_widget(NULL),
    input_capture_msg_widget(NULL),
    capturing_input(false),
    input_capture_control_nr(0) {
    
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
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    size_t control_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(
        ;
        list_nr < 8 && control_nr < game.options.controls[cur_player_nr].size();
        ++control_nr, ++list_nr
    ) {
        control_info* c_ptr = &game.options.controls[cur_player_nr][control_nr];
        
        draw_control(
            game.fonts.main, *c_ptr,
            point(game.win_w * 0.83, game.win_h * (0.2 + 0.08 * list_nr)),
            point(game.win_w * 0.23, game.win_h * 0.07)
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
    time_spent += game.delta_t;
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(game.delta_t);
    }
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
            &game.options.controls[cur_player_nr][input_capture_control_nr];
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
            update();
        }
        
    } else {
    
        handle_widget_events(ev);
        
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
    selected_widget = NULL;
    bmp_menu_bg = NULL;
    cur_page_nr = 0;
    cur_player_nr = 0;
    capturing_input = false;
    
    //Resources.
    bmp_menu_bg = load_bmp(game.asset_file_names.main_menu);
    
    //Menu widgets.
    menu_widgets.push_back(
        new menu_text(
            point(game.win_w * 0.45, game.win_h * 0.10),
            point(game.win_w * 0.20, game.win_h * 0.08),
            "Player:", game.fonts.main
        )
    );
    
    menu_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.60, game.win_h * 0.10),
            point(game.win_w * 0.15, game.win_h * 0.08),
    [this] () {
        cur_page_nr = 0;
        cur_player_nr = sum_and_wrap(cur_player_nr, -1, MAX_PLAYERS);
        cur_player_nr_widget->start_juicy_grow();
        update();
    },
    "<", game.fonts.main
        )
    );
    
    cur_player_nr_widget =
        new menu_text(
        point(game.win_w * 0.70, game.win_h * 0.10),
        point(game.win_w * 0.10, game.win_h * 0.08),
        "", game.fonts.main
    );
    menu_widgets.push_back(cur_player_nr_widget);
    
    menu_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.80, game.win_h * 0.10),
            point(game.win_w * 0.15, game.win_h * 0.08),
    [this] () {
        cur_page_nr = 0;
        cur_player_nr = sum_and_wrap(cur_player_nr, 1, MAX_PLAYERS);
        cur_player_nr_widget->start_juicy_grow();
        update();
    },
    ">", game.fonts.main
        )
    );
    
    back_widget =
        new menu_button(
        point(game.win_w * 0.15, game.win_h * 0.10),
        point(game.win_w * 0.20, game.win_h * 0.08),
    [this] () {
        leave();
    },
    "Back", game.fonts.main
    );
    menu_widgets.push_back(back_widget);
    
    for(size_t c = 0; c < 8; c++) {
        control_widgets.push_back(
            new menu_button(
                point(game.win_w * 0.07, game.win_h * (0.20 + 0.08 * c)),
                point(game.win_w * 0.08, game.win_h * 0.07),
        [] () { }, "-", game.fonts.main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                point(game.win_w * 0.16, game.win_h * (0.20 + 0.08 * c)),
                point(game.win_w * 0.08, game.win_h * 0.07),
        [] () { }, "<", game.fonts.main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_text(
                point(game.win_w * 0.40, game.win_h * (0.20 + 0.08 * c)),
                point(game.win_w * 0.39, game.win_h * 0.07),
                "", game.fonts.main, al_map_rgb(255, 255, 255),
                ALLEGRO_ALIGN_LEFT
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                point(game.win_w * 0.65, game.win_h * (0.20 + 0.08 * c)),
                point(game.win_w * 0.08, game.win_h * 0.07),
        [] () { }, ">", game.fonts.main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                point(game.win_w * 0.83, game.win_h * (0.20 + 0.08 * c)),
                point(game.win_w * 0.26, game.win_h * 0.07),
        [] () { }, "", game.fonts.main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        
    }
    
    bottom_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.85, game.win_h * 0.90),
            point(game.win_w * 0.20, game.win_h * 0.07),
    [this] () {
        if(game.options.controls[cur_player_nr].size()) {
            size_t last_action =
                game.options.controls[cur_player_nr].back().action;
            game.options.controls[cur_player_nr].push_back(
                control_info(
                    last_action == N_BUTTONS - 1 ?
                    1 : //The "None" action is 0, so go to 1.
                    last_action + 1,
                    ""
                )
            );
        } else {
            game.options.controls[cur_player_nr].push_back(
                control_info(BUTTON_NONE, "")
            );
        }
        //Go to the new control's page.
        cur_page_nr = game.options.controls[cur_player_nr].size() / 8.0f;
        this->control_widgets[
        ((game.options.controls[cur_player_nr].size() - 1) % 8) * 5 + 2
        ]->start_juicy_grow();
        update();
    },
    "New", game.fonts.main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_text(
            point(game.win_w * 0.15, game.win_h * 0.90),
            point(game.win_w * 0.20, game.win_h * 0.08),
            "Page:", game.fonts.main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.30, game.win_h * 0.90),
            point(game.win_w * 0.15, game.win_h * 0.08),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, -1,
                ceil(game.options.controls[cur_player_nr].size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    "<", game.fonts.main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    cur_page_nr_widget =
        new menu_text(
        point(game.win_w * 0.40, game.win_h * 0.90),
        point(game.win_w * 0.10, game.win_h * 0.08),
        "", game.fonts.main
    );
    bottom_widgets.push_back(cur_page_nr_widget);
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.50, game.win_h * 0.90),
            point(game.win_w * 0.15, game.win_h * 0.08),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, 1,
                (size_t) ceil(game.options.controls[cur_player_nr].size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    ">", game.fonts.main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    input_capture_msg_widget =
        new menu_text(
        point(game.win_w * 0.50, game.win_h * 0.90),
        point(game.win_w * 1.00, game.win_h * 0.08),
        "Waiting for any input...", game.fonts.main
    );
    menu_widgets.push_back(input_capture_msg_widget);
    
    //Finishing touches.
    game.fade_mgr.start_fade(true, nullptr);
    set_selected_widget(menu_widgets[1]);
    update();
    
    al_reconfigure_joysticks();
    
}


/* ----------------------------------------------------------------------------
 * Unloads the controls menu from memory.
 */
void controls_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu widgets.
    set_selected_widget(NULL);
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    control_widgets.clear();
    bottom_widgets.clear();
    cur_player_nr_widget = NULL;
    cur_page_nr_widget = NULL;
    input_capture_msg_widget = NULL;
    
}


/* ----------------------------------------------------------------------------
 * Updates the contents of the controls menu.
 */
void controls_menu_state::update() {
    cur_page_nr =
        std::min(
            cur_page_nr,
            (size_t)
            (
                ceil(
                    game.options.controls[cur_player_nr].size() / 8.0
                ) - 1
            )
        );
    cur_player_nr_widget->text = i2s(cur_player_nr + 1);
    cur_page_nr_widget->text = i2s(cur_page_nr + 1);
    
    for(size_t cw = 0; cw < control_widgets.size(); ++cw) {
        control_widgets[cw]->enabled = false;
    }
    
    size_t control_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(
        ;
        list_nr < 8 && control_nr < game.options.controls[cur_player_nr].size();
        ++control_nr, ++list_nr
    ) {
        control_info* c_ptr = &game.options.controls[cur_player_nr][control_nr];
        
        string action_name;
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            if(c_ptr->action == game.buttons.list[b].id) {
                action_name = game.buttons.list[b].name;
                break;
            }
        }
        
        for(size_t cw = 0; cw < 5; ++cw) {
            control_widgets[list_nr * 5 + cw]->enabled = true;
        }
        //Delete button.
        ((menu_button*) control_widgets[list_nr * 5 + 0])->click_handler =
        [this, control_nr] () {
            game.options.controls[cur_player_nr].erase(
                game.options.controls[cur_player_nr].begin() + control_nr
            );
            update();
        };
        //Previous action.
        ((menu_button*) control_widgets[list_nr * 5 + 1])->click_handler =
        [this, c_ptr, list_nr] () {
            c_ptr->action = sum_and_wrap(c_ptr->action, -1, N_BUTTONS);
            control_widgets[list_nr * 5 + 2]->start_juicy_grow();
            update();
        };
        //Action name.
        ((menu_text*) control_widgets[list_nr * 5 + 2])->text = action_name;
        //Next action.
        ((menu_button*) control_widgets[list_nr * 5 + 3])->click_handler =
        [this, c_ptr, list_nr] () {
            c_ptr->action = sum_and_wrap(c_ptr->action, 1, N_BUTTONS);
            control_widgets[list_nr * 5 + 2]->start_juicy_grow();
            update();
        };
        //Set button.
        ((menu_button*) control_widgets[list_nr * 5 + 4])->click_handler =
        [this, control_nr] () {
            capturing_input = true;
            input_capture_control_nr = control_nr;
            update();
        };
        
        
    }
    
    //Show or hide the "please press something" message.
    if(capturing_input) {
        input_capture_msg_widget->enabled = true;
        for(size_t bw = 0; bw < bottom_widgets.size(); ++bw) {
            bottom_widgets[bw]->enabled = false;
        }
        
    } else {
        input_capture_msg_widget->enabled = false;
        for(size_t bw = 0; bw < bottom_widgets.size(); ++bw) {
            bottom_widgets[bw]->enabled = true;
        }
        
    }
}
