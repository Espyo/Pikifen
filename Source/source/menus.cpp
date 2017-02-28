/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Menus.
 */

#include <algorithm>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "drawing.h"
#include "functions.h"
#include "menus.h"
#include "vars.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Creates a "main menu" state.
 */
main_menu::main_menu() :
    game_state(),
    bmp_menu_bg(NULL),
    new_game_state(0),
    time_spent(0) {
    
}


/* ----------------------------------------------------------------------------
 * Loads the main menu into memory.
 */
void main_menu::load() {

    selected_widget = NULL;
    
    draw_loading_screen("", "", 1.0);
    al_flip_display();
    
    //Resources.
    bmp_menu_bg = load_bmp("Main_menu.jpg");
    
    data_node title_screen_logo_file(
        ANIMATIONS_FOLDER_PATH + "/Title_screen_logo.txt"
    );
    logo = load_animation_database_from_file(&title_screen_logo_file);
    if(!logo.animations.empty()) {
        logo_anim = animation_instance(&logo);
        logo_anim.cur_anim = logo.animations[0];
        logo_anim.start();
    }
    
    //Menu widgets.
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.55), point(scr_w * 0.8, scr_h * 0.08),
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_AREA_MENU);
        });
    }, "Play", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.63), point(scr_w * 0.8, scr_h * 0.08),
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_OPTIONS_MENU);
        });
    }, "Options", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.71), point(scr_w * 0.8, scr_h * 0.08),
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_ANIMATION_EDITOR);
        });
    }, "Animation editor", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.79), point(scr_w * 0.8, scr_h * 0.08),
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_AREA_EDITOR);
        });
    }, "Area editor", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.87), point(scr_w * 0.8, scr_h * 0.08),
    [] () {
        is_game_running = false;
    }, "Exit", font_area_name
        )
    );
    
    
    //Finishing touches.
    set_selected_widget(menu_widgets[0]);
    fade_mgr.start_fade(true, nullptr);
    
}


/* ----------------------------------------------------------------------------
 * Unloads the main menu from memory.
 */
void main_menu::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    logo.destroy();
    
    //Menu widgets.
    set_selected_widget(NULL);
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 */
void main_menu::handle_controls(const ALLEGRO_EVENT &ev) {
    //TODO joystick navigation controls
    
    if(fade_mgr.is_fading()) return;
    
    handle_widget_events(ev);
    
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            is_game_running = false;
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Ticks a frame's worth of logic.
 */
void main_menu::do_logic() {
    time_spent += delta_t;
    logo_anim.tick(delta_t);
    
    //Fade manager needs to come last, because if
    //the fade finishes and the state changes, and
    //after that we still attempt to do stuff in
    //this function, we're going to have a bad time.
    fade_mgr.tick(delta_t);
    
}


/* ----------------------------------------------------------------------------
 * Draws the main menu.
 */
void main_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_sprite(
        bmp_menu_bg, point(scr_w * 0.5, scr_h * 0.5),
        point(scr_w, scr_h)
    );
    
    sprite* logo_anim_s = logo_anim.get_cur_sprite();
    if(logo_anim_s) {
        draw_sprite(
            logo_anim_s->bitmap,
            point(scr_w * 0.5, scr_h * 0.25),
            logo_anim_s->game_size
        );
    }
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    draw_scaled_text(
        font_main, al_map_rgb(255, 255, 255),
        point(8, scr_h  - 8),
        point(0.4, 0.4),
        ALLEGRO_ALIGN_LEFT, 2,
        "Pikmin (c) Nintendo"
    );
    draw_scaled_text(
        font_main, al_map_rgb(255, 255, 255),
        point(scr_w - 8, scr_h  - 8),
        point(0.4, 0.4),
        ALLEGRO_ALIGN_RIGHT, 2,
        game_name + " " + game_version +
        ", powered by PFE " +
        i2s(VERSION_MAJOR) + "." + i2s(VERSION_MINOR)  + "." + i2s(VERSION_REV)
    );
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Creates an "options menu" state.
 */
options_menu::options_menu() :
    game_state(),
    bmp_menu_bg(NULL),
    cur_player_nr(0),
    cur_page_nr(0),
    cur_player_nr_widget(NULL),
    cur_page_nr_widget(NULL),
    input_capture_msg_widget(NULL),
    input_capture_control_nr(0),
    capturing_input(false),
    time_spent(0) {
    
}


/* ----------------------------------------------------------------------------
 * Loads the options menu into memory.
 */
void options_menu::load() {
    selected_widget = NULL;
    bmp_menu_bg = NULL;
    cur_page_nr = 0;
    cur_player_nr = 0;
    capturing_input = false;
    
    //Resources.
    bmp_menu_bg = load_bmp("Main_menu.jpg");
    
    //Menu widgets.
    menu_widgets.push_back(
        new menu_text(
            point(scr_w * 0.15, scr_h * 0.1), point(scr_w * 0.2, scr_h * 0.1),
            "Player:", font_main
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.3, scr_h * 0.1), point(scr_w * 0.15, scr_h * 0.1),
    [this] () {
        cur_page_nr = 0;
        cur_player_nr = sum_and_wrap(cur_player_nr, -1, MAX_PLAYERS);
        cur_player_nr_widget->start_juicy_grow();
        update();
    },
    "<", font_main
        )
    );
    cur_player_nr_widget =
        new menu_text(
        point(scr_w * 0.4, scr_h * 0.1), point(scr_w * 0.1, scr_h * 0.1),
        "", font_main
    )
    ;
    menu_widgets.push_back(cur_player_nr_widget);
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.1), point(scr_w * 0.15, scr_h * 0.1),
    [this] () {
        cur_page_nr = 0;
        cur_player_nr = sum_and_wrap(cur_player_nr, 1, MAX_PLAYERS);
        cur_player_nr_widget->start_juicy_grow();
        update();
    },
    ">", font_main
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.9, scr_h * 0.1), point(scr_w * 0.2, scr_h * 0.1),
    [this] () {
        leave();
    },
    "Exit", font_main
        )
    );
    
    for(size_t c = 0; c < 8; c++) {
        control_widgets.push_back(
            new menu_button(
                point(scr_w * 0.1, scr_h * (0.2 + 0.08 * c)),
                point(scr_w * 0.15, scr_h * 0.1),
        [] () { }, "-", font_main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                point(scr_w * 0.2, scr_h * (0.2 + 0.08 * c)),
                point(scr_w * 0.15, scr_h * 0.1),
        [] () { }, "<", font_main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_text(
                point(scr_w * 0.43, scr_h * (0.2 + 0.08 * c)),
                point(scr_w * 0.50, scr_h * 0.1),
                "", font_main, al_map_rgb(255, 255, 255),
                ALLEGRO_ALIGN_LEFT
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                point(scr_w * 0.7, scr_h * (0.2 + 0.08 * c)),
                point(scr_w * 0.15, scr_h * 0.1),
        [] () { }, ">", font_main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                point(scr_w * 0.85, scr_h * (0.2 + 0.08 * c)),
                point(scr_w * 0.3, scr_h * 0.1),
        [] () { }, "", font_main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        
    }
    
    bottom_widgets.push_back(
        new menu_button(
            point(scr_w * 0.9, scr_h * 0.9), point(scr_w * 0.2, scr_h * 0.1),
    [this] () {
        if(controls[cur_player_nr].size()) {
            controls[cur_player_nr].push_back(
                control_info(controls[cur_player_nr].back())
            );
        } else {
            controls[cur_player_nr].push_back(
                control_info(BUTTON_NONE, "")
            );
        }
        //Go to the new control's page.
        cur_page_nr = controls[cur_player_nr].size() / 8.0f;
        this->control_widgets[
            ((controls[cur_player_nr].size() - 1) % 8) * 5 + 2
        ]->start_juicy_grow();
        update();
    },
    "New", font_main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_text(
            point(scr_w * 0.15, scr_h * 0.9), point(scr_w * 0.2, scr_h * 0.1),
            "Page:", font_main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_button(
            point(scr_w * 0.3, scr_h * 0.9), point(scr_w * 0.15, scr_h * 0.1),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, -1, ceil(controls[cur_player_nr].size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    "<", font_main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    cur_page_nr_widget =
        new menu_text(
        point(scr_w * 0.4, scr_h * 0.9), point(scr_w * 0.1, scr_h * 0.1),
        "", font_main
    );
    bottom_widgets.push_back(cur_page_nr_widget);
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.9), point(scr_w * 0.15, scr_h * 0.1),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, 1,
                (size_t) ceil(controls[cur_player_nr].size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    ">", font_main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    input_capture_msg_widget =
        new menu_text(
        point(scr_w * 0.5, scr_h * 0.9), point(scr_w, scr_h * 0.1),
        "Waiting for any input...", font_main
    );
    menu_widgets.push_back(input_capture_msg_widget);
    
    //Finishing touches.
    fade_mgr.start_fade(true, nullptr);
    set_selected_widget(menu_widgets[1]);
    update();
    
}


/* ----------------------------------------------------------------------------
 * Unloads the options menu from memory.
 */
void options_menu::unload() {

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
 * Handles Allegro events.
 */
void options_menu::handle_controls(const ALLEGRO_EVENT &ev) {
    //TODO joystick navigation controls
    if(fade_mgr.is_fading()) return;
    
    if(capturing_input) {
    
        control_info* c_ptr =
            &controls[cur_player_nr][input_capture_control_nr];
        bool valid = true;
        
        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            c_ptr->type = CONTROL_TYPE_KEYBOARD_KEY;
            c_ptr->button = ev.keyboard.keycode;
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            c_ptr->type = CONTROL_TYPE_MOUSE_BUTTON;
            c_ptr->button = ev.mouse.button;
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
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
            
        } else if(ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
            c_ptr->type = CONTROL_TYPE_JOYSTICK_BUTTON;
            c_ptr->device_nr = joystick_numbers[ev.joystick.id];
            c_ptr->button = ev.joystick.button;
            
        } else if(ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
            c_ptr->type =
                (
                    ev.joystick.pos > 0 ?
                    CONTROL_TYPE_JOYSTICK_AXIS_POS :
                    CONTROL_TYPE_JOYSTICK_AXIS_NEG
                );
            c_ptr->device_nr = joystick_numbers[ev.joystick.id];
            c_ptr->stick = ev.joystick.stick;
            c_ptr->axis = ev.joystick.axis;
            
        } else {
            valid = false;
            
        }
        
        if(valid) {
            capturing_input = false;
            update();
        }
        
    } else {
    
        handle_widget_events(ev);
        
        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                leave();
            }
        }
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void options_menu::do_logic() {
    fade_mgr.tick(delta_t);
    time_spent += delta_t;
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(delta_t);
    }
}


/* ----------------------------------------------------------------------------
 * Draws the options menu.
 */
void options_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_sprite(
        bmp_menu_bg, point(scr_w * 0.5, scr_h * 0.5),
        point(scr_w, scr_h), 0, map_gray(128)
    );
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    size_t control_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(
        ;
        list_nr < 8 && control_nr < controls[cur_player_nr].size();
        ++control_nr, ++list_nr
    ) {
        control_info* c_ptr = &controls[cur_player_nr][control_nr];
        
        draw_control(
            font_main, *c_ptr,
            point(scr_w * 0.85, scr_h * (0.2 + 0.08 * list_nr)),
            point(scr_w * 0.2, scr_h * 0.1)
        );
    }
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Updates the contents of the options menu.
 */
void options_menu::update() {
    cur_page_nr =
        min(
            cur_page_nr,
            (size_t) (ceil(controls[cur_player_nr].size() / 8.0) - 1)
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
        list_nr < 8 && control_nr < controls[cur_player_nr].size();
        ++control_nr, ++list_nr
    ) {
        control_info* c_ptr = &controls[cur_player_nr][control_nr];
        
        string action_name;
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            if(c_ptr->action == buttons.list[b].id) {
                action_name = buttons.list[b].name;
                break;
            }
        }
        
        for(size_t cw = 0; cw < 5; ++cw) {
            control_widgets[list_nr * 5 + cw]->enabled = true;
        }
        //Delete button.
        ((menu_button*) control_widgets[list_nr * 5 + 0])->click_handler =
        [this, control_nr] () {
            controls[cur_player_nr].erase(
                controls[cur_player_nr].begin() + control_nr
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


/* ----------------------------------------------------------------------------
 * Leaves the options menu and goes to the main menu.
 */
void options_menu::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
    save_options();
}


/* ----------------------------------------------------------------------------
 * Creates an "area menu" state.
 */
area_menu::area_menu() :
    game_state(),
    time_spent(0),
    cur_page_nr(0),
    cur_page_nr_widget(NULL),
    bmp_menu_bg(NULL) {
    
}


/* ----------------------------------------------------------------------------
 * Loads the area menu into memory.
 */
void area_menu::load() {
    selected_widget = NULL;
    bmp_menu_bg = NULL;
    time_spent = 0;
    cur_page_nr = 0;
    
    //Areas.
    areas_to_pick = folder_to_vector(AREAS_FOLDER_PATH, true);
    
    //If there's only one area, go there right away.
    if(areas_to_pick.size() == 1) {
        area_to_load = areas_to_pick[0];
        change_game_state(GAME_STATE_GAME);
        return;
    }
    
    //Resources.
    bmp_menu_bg = load_bmp("Main_menu.jpg");
    
    //Menu widgets.
    menu_widgets.push_back(
        new menu_text(
            point(scr_w * 0.3, scr_h * 0.1), point(scr_w * 0.5, scr_h * 0.1),
            "Pick an area:",
            font_main, al_map_rgb(255, 255, 255), ALLEGRO_ALIGN_LEFT
        )
    );
    
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.8, scr_h * 0.1), point(scr_w * 0.2, scr_h * 0.1),
    [] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_MAIN_MENU);
        });
    },
    "Back", font_main
        )
    );
    
    for(size_t a = 0; a < 8; ++a) {
        menu_widgets.push_back(
            new menu_button(
                point(scr_w * 0.5, scr_h * (0.2 + 0.08 * a)),
                point(scr_w * 0.8, scr_h * 0.1),
        [] () {
        
        },
        "", font_area_name
            )
        );
        area_buttons.push_back(menu_widgets.back());
    }
    
    menu_widgets.push_back(
        new menu_text(
            point(scr_w * 0.15, scr_h * 0.9), point(scr_w * 0.2, scr_h * 0.1),
            "Page:", font_main
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.3, scr_h * 0.9), point(scr_w * 0.15, scr_h * 0.1),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, -1, ceil(areas_to_pick.size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    "<", font_main
        )
    );
    cur_page_nr_widget =
        new menu_text(
        point(scr_w * 0.4, scr_h * 0.9), point(scr_w * 0.1, scr_h * 0.1),
        "", font_main
    );
    menu_widgets.push_back(cur_page_nr_widget);
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.9), point(scr_w * 0.15, scr_h * 0.1),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, 1, ceil(areas_to_pick.size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    ">", font_main
        )
    );
    
    //Finishing touches.
    fade_mgr.start_fade(true, nullptr);
    set_selected_widget(menu_widgets[0]);
    update();
    
}


/* ----------------------------------------------------------------------------
 * Unloads the area menu from memory.
 */
void area_menu::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu widgets.
    set_selected_widget(NULL);
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    area_buttons.clear();
    areas_to_pick.clear();
    cur_page_nr_widget = NULL;
    
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 */
void area_menu::handle_controls(const ALLEGRO_EVENT &ev) {
    //TODO joystick navigation controls
    if(fade_mgr.is_fading()) return;
    
    handle_widget_events(ev);
    
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            leave();
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void area_menu::do_logic() {
    fade_mgr.tick(delta_t);
    time_spent += delta_t;
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(delta_t);
    }
}


/* ----------------------------------------------------------------------------
 * Draws the area menu.
 */
void area_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_sprite(
        bmp_menu_bg, point(scr_w * 0.5, scr_h * 0.5),
        point(scr_w, scr_h), 0, map_gray(128)
    );
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Leaves the area menu and goes into the main menu.
 */
void area_menu::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
}


/* ----------------------------------------------------------------------------
 * Updates the contents of the area menu.
 */
void area_menu::update() {
    cur_page_nr =
        min(cur_page_nr, (size_t) (ceil(areas_to_pick.size() / 8.0) - 1));
    cur_page_nr_widget->text = i2s(cur_page_nr + 1);
    
    for(size_t aw = 0; aw < area_buttons.size(); ++aw) {
        area_buttons[aw]->enabled = false;
    }
    
    size_t area_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(; list_nr < 8 && area_nr < areas_to_pick.size(); ++area_nr, ++list_nr) {
        string area_name = areas_to_pick[area_nr];
        
        ((menu_button*) area_buttons[list_nr])->click_handler = [area_name] () {
            area_to_load = area_name;
            fade_mgr.start_fade(false, [] () {
                change_game_state(GAME_STATE_GAME);
            });
        };
        ((menu_button*) area_buttons[list_nr])->text = area_name;
        area_buttons[list_nr]->enabled = true;
        
    }
}
