/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
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

main_menu::main_menu() :
    game_state(),
    bmp_menu_bg(NULL),
    new_game_state(0),
    time_spent(0) {
    
}


void main_menu::load() {

    selected_widget = NULL;
    
    draw_loading_screen("", "", 1.0);
    al_flip_display();
    
    //Resources.
    bmp_menu_bg = load_bmp("Main_menu.jpg");
    
    data_node title_screen_logo_file(MISC_FOLDER + "/Title_screen_logo.txt");
    logo = load_animation_pool_from_file(&title_screen_logo_file);
    if(!logo.animations.empty()) {
        logo_anim = animation_instance(&logo);
        logo_anim.anim = logo.animations[0];
        logo_anim.start();
    }
    
    //Menu widgets.
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, scr_h * 0.55, scr_w * 0.8, scr_h * 0.08,
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_AREA_MENU);
        });
    }, "Play", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, scr_h * 0.63, scr_w * 0.8, scr_h * 0.08,
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_OPTIONS_MENU);
        });
    }, "Options", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, scr_h * 0.71, scr_w * 0.8, scr_h * 0.08,
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_ANIMATION_EDITOR);
        });
    }, "Animation editor", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, scr_h * 0.79, scr_w * 0.8, scr_h * 0.08,
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_AREA_EDITOR);
        });
    }, "Area editor", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, scr_h * 0.87, scr_w * 0.8, scr_h * 0.08,
    [] () {
        running = false;
    }, "Exit", font_area_name
        )
    );
    
    
    //Finishing touches.
    set_selected_widget(menu_widgets[0]);
    fade_mgr.start_fade(true, nullptr);
    
}


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


void main_menu::handle_controls(ALLEGRO_EVENT ev) {
    //TODO joystick navigation controls
    
    if(fade_mgr.is_fading()) return;
    
    handle_widget_events(ev);
    
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            running = false;
        }
    }
    
}


void main_menu::do_logic() {
    time_spent += delta_t;
    logo_anim.tick(delta_t);
    
    //Fade manager needs to come last, because if
    //the fade finishes and the state changes, and
    //after that we still attempt to do stuff in
    //this function, we're going to have a bad time.
    fade_mgr.tick(delta_t);
    
}


void main_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_sprite(
        bmp_menu_bg, scr_w * 0.5, scr_h * 0.5,
        scr_w, scr_h
    );
    
    frame* logo_anim_f = logo_anim.get_frame();
    if(logo_anim_f) {
        draw_sprite(
            logo_anim_f->bitmap,
            scr_w * 0.5,
            scr_h * 0.25,
            logo_anim_f->game_w,
            logo_anim_f->game_h
        );
    }
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    draw_scaled_text(
        font, al_map_rgb(255, 255, 255),
        8, scr_h  - 8,
        0.4, 0.4,
        ALLEGRO_ALIGN_LEFT, 2,
        "Pikmin (c) Nintendo"
    );
    draw_scaled_text(
        font, al_map_rgb(255, 255, 255),
        scr_w - 8, scr_h  - 8,
        0.4, 0.4,
        ALLEGRO_ALIGN_RIGHT, 2,
        game_name + " " + game_version +
        " is powered by PFE " +
        i2s(VERSION_MAJOR) + "." + i2s(VERSION_MINOR)  + "." + i2s(VERSION_REV)
    );
    
    fade_mgr.draw();
    
    al_flip_display();
}



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
            scr_w * 0.15, scr_h * 0.1, scr_w * 0.2, scr_h * 0.1,
            "Player:", font
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.3, scr_h * 0.1, scr_w * 0.15, scr_h * 0.1,
    [this] () {
        cur_page_nr = 0;
        cur_player_nr = cur_player_nr == 0 ? 3 : cur_player_nr - 1;
        cur_player_nr_widget->start_juicy_grow();
        update();
    },
    "<", font
        )
    );
    cur_player_nr_widget =
        new menu_text(
        scr_w * 0.4, scr_h * 0.1, scr_w * 0.1, scr_h * 0.1,
        "", font
    )
    ;
    menu_widgets.push_back(cur_player_nr_widget);
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, scr_h * 0.1, scr_w * 0.15, scr_h * 0.1,
    [this] () {
        cur_page_nr = 0;
        cur_player_nr = (cur_player_nr + 1) % 4;
        cur_player_nr_widget->start_juicy_grow();
        update();
    },
    ">", font
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.9, scr_h * 0.1, scr_w * 0.2, scr_h * 0.1,
    [this] () {
        leave();
    },
    "Exit", font
        )
    );
    
    for(size_t c = 0; c < 8; c++) {
        control_widgets.push_back(
            new menu_button(
                scr_w * 0.1, scr_h * (0.2 + 0.08 * c), scr_w * 0.15, scr_h * 0.1,
        [] () {
        
        },
        "-", font
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                scr_w * 0.2, scr_h * (0.2 + 0.08 * c), scr_w * 0.15, scr_h * 0.1,
        [] () {
        
        },
        "<", font
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_text(
                scr_w * 0.43, scr_h * (0.2 + 0.08 * c), scr_w * 0.50, scr_h * 0.1,
                "", font, al_map_rgb(255, 255, 255), ALLEGRO_ALIGN_LEFT
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                scr_w * 0.7, scr_h * (0.2 + 0.08 * c), scr_w * 0.15, scr_h * 0.1,
        [] () {
        
        },
        ">", font
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                scr_w * 0.85, scr_h * (0.2 + 0.08 * c), scr_w * 0.3, scr_h * 0.1,
        [] () {
        
        },
        "", font
            )
        );
        menu_widgets.push_back(control_widgets.back());
        
    }
    
    bottom_widgets.push_back(
        new menu_button(
            scr_w * 0.9, scr_h * 0.9, scr_w * 0.2, scr_h * 0.1,
    [this] () {
        if(controls[cur_player_nr].size()) {
            controls[cur_player_nr].push_back(control_info(controls[cur_player_nr].back()));
        } else {
            controls[cur_player_nr].push_back(control_info(BUTTON_NONE, ""));
        }
        //Go to the new control's page.
        cur_page_nr = controls[cur_player_nr].size() / 8.0f;
        this->control_widgets[((controls[cur_player_nr].size() - 1) % 8) * 5 + 2]->start_juicy_grow();
        update();
    },
    "New", font
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_text(
            scr_w * 0.15, scr_h * 0.9, scr_w * 0.2, scr_h * 0.1,
            "Page:", font
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_button(
            scr_w * 0.3, scr_h * 0.9, scr_w * 0.15, scr_h * 0.1,
    [this] () {
        if(cur_page_nr == 0) cur_page_nr = ceil(controls[cur_player_nr].size() / 8.0) - 1;
        else cur_page_nr--;
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    "<", font
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    cur_page_nr_widget =
        new menu_text(
        scr_w * 0.4, scr_h * 0.9, scr_w * 0.1, scr_h * 0.1,
        "", font
    )
    ;
    bottom_widgets.push_back(cur_page_nr_widget);
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_button(
            scr_w * 0.5, scr_h * 0.9, scr_w * 0.15, scr_h * 0.1,
    [this] () {
        cur_page_nr = (cur_page_nr + 1) % (size_t) ceil(controls[cur_player_nr].size() / 8.0);
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    ">", font
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    input_capture_msg_widget =
        new menu_text(
        scr_w * 0.5, scr_h * 0.9, scr_w, scr_h * 0.1,
        "Waiting for any input...", font
    )
    ;
    menu_widgets.push_back(input_capture_msg_widget);
    
    //Finishing touches.
    fade_mgr.start_fade(true, nullptr);
    set_selected_widget(menu_widgets[1]);
    update();
    
}


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


void options_menu::handle_controls(ALLEGRO_EVENT ev) {
    //TODO joystick navigation controls
    if(fade_mgr.is_fading()) return;
    
    if(capturing_input) {
    
        control_info* c_ptr = &controls[cur_player_nr][input_capture_control_nr];
        bool valid = true;
        
        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            c_ptr->type = CONTROL_TYPE_KEYBOARD_KEY;
            c_ptr->button = ev.keyboard.keycode;
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            c_ptr->type = CONTROL_TYPE_MOUSE_BUTTON;
            c_ptr->button = ev.mouse.button;
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(ev.mouse.dz > 0)      c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_UP;
            else if(ev.mouse.dz < 0) c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_DOWN;
            else if(ev.mouse.dw > 0) c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_RIGHT;
            else if(ev.mouse.dw < 0) c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_LEFT;
            else valid = false;
            
        } else if(ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
            c_ptr->type = CONTROL_TYPE_JOYSTICK_BUTTON;
            c_ptr->device_nr = joystick_numbers[ev.joystick.id];
            c_ptr->button = ev.joystick.button;
            
        } else if(ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
            c_ptr->type = (ev.joystick.pos > 0 ? CONTROL_TYPE_JOYSTICK_AXIS_POS : CONTROL_TYPE_JOYSTICK_AXIS_NEG);
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


void options_menu::do_logic() {
    fade_mgr.tick(delta_t);
    time_spent += delta_t;
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(delta_t);
    }
}


void options_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_sprite(
        bmp_menu_bg, scr_w * 0.5, scr_h * 0.5,
        scr_w, scr_h, 0, map_gray(128)
    );
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    size_t control_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(; list_nr < 8 && control_nr < controls[cur_player_nr].size(); ++control_nr, ++list_nr) {
        control_info* c_ptr = &controls[cur_player_nr][control_nr];
        
        draw_control(font, *c_ptr, scr_w * 0.85, scr_h * (0.2 + 0.08 * list_nr), scr_w * 0.2, scr_h * 0.1);
    }
    
    fade_mgr.draw();
    
    al_flip_display();
}


void options_menu::update() {
    cur_page_nr = min(cur_page_nr, (size_t) (ceil(controls[cur_player_nr].size() / 8.0) - 1));
    cur_player_nr_widget->text = i2s(cur_player_nr + 1);
    cur_page_nr_widget->text = i2s(cur_page_nr + 1);
    
    for(size_t cw = 0; cw < control_widgets.size(); ++cw) {
        control_widgets[cw]->enabled = false;
    }
    
    size_t control_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(; list_nr < 8 && control_nr < controls[cur_player_nr].size(); ++control_nr, ++list_nr) {
        control_info* c_ptr = &controls[cur_player_nr][control_nr];
        
        string action_name;
        if(c_ptr->action == BUTTON_NONE)                         action_name = "---";
        else if(c_ptr->action == BUTTON_THROW)                   action_name = "Throw";
        else if(c_ptr->action == BUTTON_WHISTLE)                 action_name = "Whistle";
        else if(c_ptr->action == BUTTON_MOVE_RIGHT)              action_name = "Right";
        else if(c_ptr->action == BUTTON_MOVE_UP)                 action_name = "Up";
        else if(c_ptr->action == BUTTON_MOVE_LEFT)               action_name = "Left";
        else if(c_ptr->action == BUTTON_MOVE_DOWN)               action_name = "Down";
        else if(c_ptr->action == BUTTON_MOVE_CURSOR_RIGHT)       action_name = "Cursor right";
        else if(c_ptr->action == BUTTON_MOVE_CURSOR_UP)          action_name = "Cursor up";
        else if(c_ptr->action == BUTTON_MOVE_CURSOR_LEFT)        action_name = "Cursor left";
        else if(c_ptr->action == BUTTON_MOVE_CURSOR_DOWN)        action_name = "Cursor down";
        else if(c_ptr->action == BUTTON_GROUP_MOVE_RIGHT)        action_name = "Group right";
        else if(c_ptr->action == BUTTON_GROUP_MOVE_UP)           action_name = "Group up";
        else if(c_ptr->action == BUTTON_GROUP_MOVE_LEFT)         action_name = "Group left";
        else if(c_ptr->action == BUTTON_GROUP_MOVE_DOWN)         action_name = "Group down";
        else if(c_ptr->action == BUTTON_GROUP_MOVE_GO_TO_CURSOR) action_name = "Group to cursor";
        else if(c_ptr->action == BUTTON_SWITCH_LEADER_RIGHT)     action_name = "Next leader";
        else if(c_ptr->action == BUTTON_SWITCH_LEADER_LEFT)      action_name = "Previous leader";
        else if(c_ptr->action == BUTTON_DISMISS)                 action_name = "Dismiss";
        else if(c_ptr->action == BUTTON_USE_SPRAY_1)             action_name = "Use spray 1";
        else if(c_ptr->action == BUTTON_USE_SPRAY_2)             action_name = "Use spray 2";
        else if(c_ptr->action == BUTTON_USE_SPRAY)               action_name = "Use spray";
        else if(c_ptr->action == BUTTON_SWITCH_SPRAY_RIGHT)      action_name = "Next spray";
        else if(c_ptr->action == BUTTON_SWITCH_SPRAY_LEFT)       action_name = "Previous spray";
        else if(c_ptr->action == BUTTON_SWITCH_ZOOM)             action_name = "Switch zoom";
        else if(c_ptr->action == BUTTON_ZOOM_IN)                 action_name = "Zoom in";
        else if(c_ptr->action == BUTTON_ZOOM_OUT)                action_name = "Zoom out";
        else if(c_ptr->action == BUTTON_SWITCH_TYPE_RIGHT)       action_name = "Next Pikmin";
        else if(c_ptr->action == BUTTON_SWITCH_TYPE_LEFT)        action_name = "Previous Pikmin";
        else if(c_ptr->action == BUTTON_SWITCH_MATURITY_UP)      action_name = "Next maturity";
        else if(c_ptr->action == BUTTON_SWITCH_MATURITY_DOWN)    action_name = "Prev. maturity";
        else if(c_ptr->action == BUTTON_LIE_DOWN)                action_name = "Lie down";
        else if(c_ptr->action == BUTTON_PAUSE)                   action_name = "Pause";
        
        for(size_t cw = 0; cw < 5; ++cw) {
            control_widgets[list_nr * 5 + cw]->enabled = true;
        }
        //Delete button.
        ((menu_button*) control_widgets[list_nr * 5 + 0])->click_handler = [this, control_nr] () {
            controls[cur_player_nr].erase(controls[cur_player_nr].begin() + control_nr);
            update();
        };
        //Previous action.
        ((menu_button*) control_widgets[list_nr * 5 + 1])->click_handler = [this, c_ptr, list_nr] () {
            c_ptr->action = c_ptr->action == 0 ? N_BUTTONS - 1 : c_ptr->action - 1;
            control_widgets[list_nr * 5 + 2]->start_juicy_grow();
            update();
        };
        //Action name.
        ((menu_text*) control_widgets[list_nr * 5 + 2])->text = action_name;
        //Next action.
        ((menu_button*) control_widgets[list_nr * 5 + 3])->click_handler = [this, c_ptr, list_nr] () {
            c_ptr->action = (c_ptr->action + 1) % N_BUTTONS;
            control_widgets[list_nr * 5 + 2]->start_juicy_grow();
            update();
        };
        //Set button.
        ((menu_button*) control_widgets[list_nr * 5 + 4])->click_handler = [this, control_nr] () {
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


void options_menu::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
    save_options();
}



area_menu::area_menu() :
    game_state(),
    time_spent(0),
    cur_page_nr(0),
    cur_page_nr_widget(NULL),
    bmp_menu_bg(NULL) {
    
}


void area_menu::load() {
    selected_widget = NULL;
    bmp_menu_bg = NULL;
    time_spent = 0;
    cur_page_nr = 0;
    
    //Areas.
    areas_to_pick = folder_to_vector(AREA_FOLDER, false);
    for(size_t a = 0; a < areas_to_pick.size();) {
        string n = areas_to_pick[a];
        bool valid = true;
        if(n.size() < 5) {
            valid = false;
        } else if(n.substr(n.size() - 4, 4) != ".txt") {
            valid = false;
        } else {
            areas_to_pick[a] = n.substr(0, n.size() - 4); //Remove the ".txt".
        }
        
        if(!valid) {
            areas_to_pick.erase(areas_to_pick.begin() + a);
        } else {
            ++a;
        }
    }
    
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
            scr_w * 0.3, scr_h * 0.1, scr_w * 0.5, scr_h * 0.1,
            "Pick an area:",
            font, al_map_rgb(255, 255, 255), ALLEGRO_ALIGN_LEFT
        )
    );
    
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.8, scr_h * 0.1, scr_w * 0.2, scr_h * 0.1,
    [] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_MAIN_MENU);
        });
    },
    "Back", font
        )
    );
    
    for(size_t a = 0; a < 8; ++a) {
        menu_widgets.push_back(
            new menu_button(
                scr_w * 0.5, scr_h * (0.2 + 0.08 * a), scr_w * 0.8, scr_h * 0.1,
        [] () {
        
        },
        "", font_area_name
            )
        );
        area_buttons.push_back(menu_widgets.back());
    }
    
    menu_widgets.push_back(
        new menu_text(
            scr_w * 0.15, scr_h * 0.9, scr_w * 0.2, scr_h * 0.1,
            "Page:", font
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.3, scr_h * 0.9, scr_w * 0.15, scr_h * 0.1,
    [this] () {
        if(cur_page_nr == 0) cur_page_nr = ceil(areas_to_pick.size() / 8.0) - 1;
        else cur_page_nr--;
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    "<", font
        )
    );
    cur_page_nr_widget =
        new menu_text(
        scr_w * 0.4, scr_h * 0.9, scr_w * 0.1, scr_h * 0.1,
        "", font
    )
    ;
    menu_widgets.push_back(cur_page_nr_widget);
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, scr_h * 0.9, scr_w * 0.15, scr_h * 0.1,
    [this] () {
        cur_page_nr = (cur_page_nr + 1) % (size_t) ceil(areas_to_pick.size() / 8.0);
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    ">", font
        )
    );
    
    //Finishing touches.
    fade_mgr.start_fade(true, nullptr);
    set_selected_widget(menu_widgets[0]);
    update();
    
}


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


void area_menu::handle_controls(ALLEGRO_EVENT ev) {
    //TODO joystick navigation controls
    if(fade_mgr.is_fading()) return;
    
    handle_widget_events(ev);
    
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            leave();
        }
    }
    
}


void area_menu::do_logic() {
    fade_mgr.tick(delta_t);
    time_spent += delta_t;
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(delta_t);
    }
}


void area_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_sprite(
        bmp_menu_bg, scr_w * 0.5, scr_h * 0.5,
        scr_w, scr_h, 0, map_gray(128)
    );
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    fade_mgr.draw();
    
    al_flip_display();
}


void area_menu::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
}


void area_menu::update() {
    cur_page_nr = min(cur_page_nr, (size_t) (ceil(areas_to_pick.size() / 8.0) - 1));
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
