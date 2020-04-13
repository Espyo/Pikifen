/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Control-related functions.
 */

#include <algorithm>
#include <iostream>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "controls.h"

#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "gameplay.h"
#include "utils/string_utils.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates information about a control.
 * action: The action this control does in-game. Use BUTTON_*.
 * player: Player number.
 * s:      The textual code that represents the hardware inputs.
 */
control_info::control_info(unsigned char action, const string &s) :
    action(action),
    type(CONTROL_TYPE_NONE),
    device_nr(0),
    button(0),
    stick(0),
    axis(0) {
    vector<string> parts = split(s, "_");
    size_t n_parts = parts.size();
    
    if(n_parts == 0) return;
    if(parts[0] == "k") {   //Keyboard.
        if(n_parts > 1) {
            type = CONTROL_TYPE_KEYBOARD_KEY;
            button = s2i(parts[1]);
        }
        
    } else if(parts[0] == "mb") { //Mouse button.
        if(n_parts > 1) {
            type = CONTROL_TYPE_MOUSE_BUTTON;
            button = s2i(parts[1]);
        }
        
    } else if(parts[0] == "mwu") { //Mouse wheel up.
        type = CONTROL_TYPE_MOUSE_WHEEL_UP;
        
    } else if(parts[0] == "mwd") { //Mouse wheel down.
        type = CONTROL_TYPE_MOUSE_WHEEL_DOWN;
        
    } else if(parts[0] == "mwl") { //Mouse wheel left.
        type = CONTROL_TYPE_MOUSE_WHEEL_LEFT;
        
    } else if(parts[0] == "mwr") { //Mouse wheel right.
        type = CONTROL_TYPE_MOUSE_WHEEL_RIGHT;
        
    } else if(parts[0] == "jb") { //Joystick button.
        if(n_parts > 2) {
            type = CONTROL_TYPE_JOYSTICK_BUTTON;
            device_nr = s2i(parts[1]);
            button = s2i(parts[2]);
        }
        
    } else if(parts[0] == "jap") { //Joystick axis, positive.
        if(n_parts > 3) {
            type = CONTROL_TYPE_JOYSTICK_AXIS_POS;
            device_nr = s2i(parts[1]);
            stick = s2i(parts[2]);
            axis = s2i(parts[3]);
        }
    } else if(parts[0] == "jan") { //Joystick axis, negative.
        if(n_parts > 3) {
            type = CONTROL_TYPE_JOYSTICK_AXIS_NEG;
            device_nr = s2i(parts[1]);
            stick = s2i(parts[2]);
            axis = s2i(parts[3]);
        }
    } else {
        log_error(
            "Unrecognized control type \"" + parts[0] + "\""
            " (value=\"" + s + "\")!");
    }
}


/* ----------------------------------------------------------------------------
 * Converts a control info's hardware input data into a string,
 * used in the options file.
 */
string control_info::stringify() {
    if(type == CONTROL_TYPE_KEYBOARD_KEY) {
        return "k_" + i2s(button);
    } else if(type == CONTROL_TYPE_MOUSE_BUTTON) {
        return "mb_" + i2s(button);
    } else if(type == CONTROL_TYPE_MOUSE_WHEEL_UP) {
        return "mwu";
    } else if(type == CONTROL_TYPE_MOUSE_WHEEL_DOWN) {
        return "mwd";
    } else if(type == CONTROL_TYPE_MOUSE_WHEEL_LEFT) {
        return "mwl";
    } else if(type == CONTROL_TYPE_MOUSE_WHEEL_RIGHT) {
        return "mwr";
    } else if(type == CONTROL_TYPE_JOYSTICK_BUTTON) {
        return "jb_" + i2s(device_nr) + "_" + i2s(button);
    } else if(type == CONTROL_TYPE_JOYSTICK_AXIS_POS) {
        return "jap_" + i2s(device_nr) + "_" + i2s(stick) + "_" + i2s(axis);
    } else if(type == CONTROL_TYPE_JOYSTICK_AXIS_NEG) {
        return "jan_" + i2s(device_nr) + "_" + i2s(stick) + "_" + i2s(axis);
    }
    
    return "";
}


/* ----------------------------------------------------------------------------
 * Handles a button "press". Technically, it could also be a button release.
 * button: The button's ID. Use BUTTON_*.
 * pos:    The position of the button, i.e., how much it's "held".
 *   0 means it was released. 1 means it was fully pressed.
 *   For controls with more sensitivity, values between 0 and 1 are important.
 *   Like a 0.5 for swarming makes the group swarm at half distance.
 * player: Number of the player that pressed.
 */
void gameplay::handle_button(
    const size_t button, const float pos, const size_t player
) {

    if(!ready_for_input || !is_input_allowed) return;
    
    bool is_down = (pos >= 0.5);
    
    if(cur_message.empty()) {
    
        if(
            button == BUTTON_RIGHT ||
            button == BUTTON_UP ||
            button == BUTTON_LEFT ||
            button == BUTTON_DOWN
        ) {
        
            /*******************
            *               O_ *
            *   Move   --->/|  *
            *              V > *
            *******************/
            
            if(button == BUTTON_RIGHT) {
                leader_movement.right = pos;
            } else if(button == BUTTON_LEFT) {
                leader_movement.left = pos;
            } else if(button == BUTTON_UP) {
                leader_movement.up = pos;
            } else if(button == BUTTON_DOWN) {
                leader_movement.down = pos;
            }
            
        } else if(
            button == BUTTON_CURSOR_RIGHT ||
            button == BUTTON_CURSOR_UP ||
            button == BUTTON_CURSOR_LEFT ||
            button == BUTTON_CURSOR_DOWN
        ) {
            /********************
            *             .-.   *
            *   Cursor   ( = )> *
            *             '-'   *
            ********************/
            
            if(button == BUTTON_CURSOR_RIGHT) {
                cursor_movement.right = pos;
            } else if(button == BUTTON_CURSOR_LEFT) {
                cursor_movement.left = pos;
            } else if(button == BUTTON_CURSOR_UP) {
                cursor_movement.up = pos;
            } else if(button == BUTTON_CURSOR_DOWN) {
                cursor_movement.down = pos;
            }
            
        } else if(
            button == BUTTON_GROUP_RIGHT ||
            button == BUTTON_GROUP_UP ||
            button == BUTTON_GROUP_LEFT ||
            button == BUTTON_GROUP_DOWN
        ) {
            /******************
            *            ***  *
            *   Group   ****O *
            *            ***  *
            ******************/
            
            if(button == BUTTON_GROUP_RIGHT) {
                swarm_movement.right = pos;
            } else if(button == BUTTON_GROUP_LEFT) {
                swarm_movement.left = pos;
            } else if(button == BUTTON_GROUP_UP) {
                swarm_movement.up = pos;
            } else if(button == BUTTON_GROUP_DOWN) {
                swarm_movement.down = pos;
            }
            
        } else if(button == BUTTON_GROUP_CURSOR) {
        
            swarm_cursor = is_down;
            
        } else if(button == BUTTON_THROW) {
        
            /*******************
            *             .-.  *
            *   Throw    /   O *
            *           &      *
            *******************/
            
            if(is_down) { //Button press.
            
                bool done = false;
                
                //First check if the leader should heal themselves on the ship.
                if(close_to_ship_to_heal) {
                    close_to_ship_to_heal->heal_leader(cur_leader_ptr);
                    done = true;
                }
                
                //Now check if the leader should pluck a Pikmin.
                if(!done) {
                    if(close_to_pikmin_to_pluck) {
                        cur_leader_ptr->fsm.run_event(
                            LEADER_EV_GO_PLUCK,
                            (void*) close_to_pikmin_to_pluck
                        );
                        done = true;
                    }
                }
                
                //Now check if the leader should open an Onion's menu.
                if(!done) {
                    if(close_to_onion_to_open) {
                        close_to_onion_to_open->call_pikmin();
                        done = true;
                    }
                }
                
                //Now check if the leader should interact with an interactable.
                if(!done) {
                    if(close_to_interactable_to_use) {
                        string msg = "interact";
                        cur_leader_ptr->send_message(
                            close_to_interactable_to_use, msg
                        );
                        done = true;
                    }
                }
                
                //Now check if the leader should grab a Pikmin.
                if(!done) {
                    if(
                        cur_leader_ptr->holding.empty() &&
                        cur_leader_ptr->group->cur_standby_type &&
                        !closest_group_member_distant
                    ) {
                    
                        done = grab_closest_group_member();
                    }
                }
                
                //Now check if the leader should punch.
                if(!done) {
                    cur_leader_ptr->fsm.run_event(LEADER_EV_PUNCH);
                    done = true;
                }
                
            } else { //Button release.
                if(!cur_leader_ptr->holding.empty()) {
                    cur_leader_ptr->fsm.run_event(LEADER_EV_THROW);
                }
            }
            
        } else if(button == BUTTON_WHISTLE) {
        
            /********************
            *              .--= *
            *   Whistle   ( @ ) *
            *              '-'  *
            ********************/
            
            if(is_down) {
                //Button pressed.
                //Cancel auto-pluck, lying down, etc.
                cur_leader_ptr->fsm.run_event(LEADER_EV_CANCEL);
                
                if(close_to_onion_to_open) {
                    close_to_onion_to_open->stow_pikmin();
                    
                } else {
                    cur_leader_ptr->fsm.run_event(LEADER_EV_START_WHISTLE);
                    
                }
                
            } else {
                //Button released.
                cur_leader_ptr->fsm.run_event(LEADER_EV_STOP_WHISTLE);
                
            }
            
        } else if(
            button == BUTTON_NEXT_LEADER ||
            button == BUTTON_PREV_LEADER
        ) {
        
            /******************************
            *                    \O/  \O/ *
            *   Switch leader     | -> |  *
            *                    / \  / \ *
            ******************************/
            
            if(!is_down) return;
            
            change_to_next_leader(button == BUTTON_NEXT_LEADER, false);
            
        } else if(button == BUTTON_DISMISS) {
        
            /***********************
            *             \O/ / *  *
            *   Dismiss    |   - * *
            *             / \ \ *  *
            ***********************/
            
            if(!is_down) return;
            
            cur_leader_ptr->fsm.run_event(LEADER_EV_DISMISS);
            
        } else if(button == BUTTON_PAUSE) {
        
            /********************
            *           +-+ +-+ *
            *   Pause   | | | | *
            *           +-+ +-+ *
            ********************/
            
            if(!is_down) return;
            
            is_input_allowed = false;
            game.fade_mgr.start_fade(
                false,
            [this] () {
                this->leave();
            }
            );
            
            //paused = true;
            
        } else if(button == BUTTON_USE_SPRAY_1) {
        
            /*******************
            *             +=== *
            *   Sprays   (   ) *
            *             '-'  *
            *******************/
            
            if(!is_down) return;
            
            if(spray_types.size() == 1 || spray_types.size() == 2) {
                size_t spray_nr = 0;
                cur_leader_ptr->fsm.run_event(
                    LEADER_EV_SPRAY, (void*) &spray_nr
                );
            }
            
        } else if(button == BUTTON_USE_SPRAY_2) {
        
            if(!is_down) return;
            
            if(spray_types.size() == 2) {
                size_t spray_nr = 1;
                cur_leader_ptr->fsm.run_event(
                    LEADER_EV_SPRAY, (void*) &spray_nr
                );
            }
            
        } else if(
            button == BUTTON_NEXT_SPRAY ||
            button == BUTTON_PREV_SPRAY
        ) {
        
            if(!is_down) return;
            
            if(spray_types.size() > 2) {
                if(button == BUTTON_NEXT_SPRAY) {
                    selected_spray = (selected_spray + 1) % spray_types.size();
                } else {
                    if(selected_spray == 0) {
                        selected_spray = spray_types.size() - 1;
                    } else {
                        selected_spray--;
                    }
                }
            }
            
        } else if(button == BUTTON_USE_SPRAY) {
        
            if(!is_down) return;
            
            if(spray_types.size() > 2) {
                cur_leader_ptr->fsm.run_event(
                    LEADER_EV_SPRAY,
                    (void*) &selected_spray
                );
            }
            
        } else if(button == BUTTON_CHANGE_ZOOM) {
        
            /***************
            *           _  *
            *   Zoom   (_) *
            *          /   *
            ***************/
            
            if(!is_down) return;
            
            if(game.cam.target_zoom < game.options.zoom_mid_level) {
                game.cam.target_zoom = zoom_max_level;
            } else if(game.cam.target_zoom > game.options.zoom_mid_level) {
                game.cam.target_zoom = game.options.zoom_mid_level;
            } else {
                if(game.options.zoom_mid_level == zoom_min_level) {
                    game.cam.target_zoom = zoom_max_level;
                } else {
                    game.cam.target_zoom = zoom_min_level;
                }
            }
            
            sfx_camera.play(0, false);
            
        } else if(button == BUTTON_ZOOM_IN || button == BUTTON_ZOOM_OUT) {
        
            if(game.cam.target_zoom >= zoom_max_level && button == BUTTON_ZOOM_IN) {
                return;
            }
            
            if(game.cam.target_zoom <= zoom_min_level && button == BUTTON_ZOOM_OUT) {
                return;
            }
            
            float floored_pos = floor(pos);
            
            if(button == BUTTON_ZOOM_IN) {
                game.cam.target_zoom = game.cam.target_zoom + 0.1 * floored_pos;
            } else {
                game.cam.target_zoom = game.cam.target_zoom - 0.1 * floored_pos;
            }
            
            if(game.cam.target_zoom > zoom_max_level) game.cam.target_zoom = zoom_max_level;
            if(game.cam.target_zoom < zoom_min_level) game.cam.target_zoom = zoom_min_level;
            
            sfx_camera.play(-1, false);
            
        } else if(button == BUTTON_LIE_DOWN) {
        
            /**********************
            *                     *
            *   Lie down  -()/__/ *
            *                     *
            **********************/
            
            if(!is_down) return;
            
            cur_leader_ptr->fsm.run_event(LEADER_EV_LIE_DOWN);
            
            
        } else if(
            button == BUTTON_NEXT_TYPE ||
            button == BUTTON_PREV_TYPE
        ) {
        
            /****************************
            *                     -->   *
            *   Switch type   <( )> (o) *
            *                           *
            ****************************/
            
            if(!is_down) return;
            
            if(cur_leader_ptr->group->members.empty()) return;
            
            subgroup_type* starting_subgroup_type =
                cur_leader_ptr->group->cur_standby_type;
                
            bool switch_successful;
            
            if(cur_leader_ptr->holding.empty()) {
                //If the leader isn't holding anybody.
                switch_successful =
                    cur_leader_ptr->group->set_next_cur_standby_type(
                        button == BUTTON_PREV_TYPE
                    );
                    
            } else {
                //If the leader is holding a Pikmin, we can't let it
                //swap to a Pikmin that's far away.
                //So, every time that happens, skip that subgroup and
                //try the next. Also, make sure to cancel everything if
                //the loop already went through all types.
                
                bool finish = false;
                do {
                    switch_successful =
                        cur_leader_ptr->group->set_next_cur_standby_type(
                            button == BUTTON_PREV_TYPE
                        );
                        
                    if(
                        !switch_successful ||
                        cur_leader_ptr->group->cur_standby_type ==
                        starting_subgroup_type
                    ) {
                        //Reached around back to the first subgroup...
                        switch_successful = false;
                        finish = true;
                        
                    } else {
                        //Switched to a new subgroup.
                        update_closest_group_member();
                        if(!closest_group_member_distant) {
                            finish = true;
                        }
                        
                    }
                    
                } while(!finish);
                
                if(switch_successful) {
                    cur_leader_ptr->swap_held_pikmin(closest_group_member);
                }
            }
            
            if(switch_successful) {
                sfx_switch_pikmin.play(0, false);
            }
            
        } else if(
            button == BUTTON_NEXT_MATURITY ||
            button == BUTTON_PREV_MATURITY
        ) {
        
            /**********************************
            *                      V  -->  *  *
            *   Switch maturity    |       |  *
            *                     ( )     ( ) *
            **********************************/
            
            if(
                !is_down ||
                cur_leader_ptr->holding.empty() ||
                cur_leader_ptr->holding[0]->type->category->id !=
                MOB_CATEGORY_PIKMIN
            ) {
                return;
            }
            
            pikmin* held_p_ptr = (pikmin*) cur_leader_ptr->holding[0];
            
            pikmin* closest_members[N_MATURITIES];
            dist closest_dists[N_MATURITIES];
            for(size_t m = 0; m < N_MATURITIES; ++m) {
                closest_members[m] = NULL;
            }
            
            for(size_t m = 0; m < cur_leader_ptr->group->members.size(); ++m) {
                mob* m_ptr = cur_leader_ptr->group->members[m];
                if(m_ptr->type != held_p_ptr->type) continue;
                
                pikmin* p_ptr = (pikmin*) m_ptr;
                if(p_ptr->maturity == held_p_ptr->maturity) continue;
                
                dist d(cur_leader_ptr->pos, p_ptr->pos);
                if(
                    !closest_members[p_ptr->maturity] ||
                    d < closest_dists[p_ptr->maturity]
                ) {
                    closest_members[p_ptr->maturity] = p_ptr;
                    closest_dists[p_ptr->maturity] = d;
                }
                
            }
            
            size_t next_maturity = held_p_ptr->maturity;
            mob* new_pikmin = NULL;
            bool finished = false;
            do {
                next_maturity =
                    (size_t) sum_and_wrap(
                        next_maturity,
                        (button == BUTTON_NEXT_MATURITY ? 1 : -1),
                        N_MATURITIES
                    );
                    
                //Back to the start?
                if(next_maturity == held_p_ptr->maturity) break;
                
                if(!closest_members[next_maturity]) continue;
                
                new_pikmin = closest_members[next_maturity];
                finished = true;
                
            } while(!finished);
            
            if(new_pikmin) {
                cur_leader_ptr->swap_held_pikmin(new_pikmin);
            }
            
        }
        
    } else { //Displaying a message.
    
        if((button == BUTTON_THROW || button == BUTTON_PAUSE) && is_down) {
            size_t stopping_char =
                cur_message_stopping_chars[cur_message_section + 1];
            if(cur_message_char == stopping_char) {
                if(stopping_char == cur_message.size()) {
                    start_message("", NULL);
                } else {
                    cur_message_section++;
                }
            } else {
                cur_message_char = stopping_char;
            }
        }
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event related to hardware input,
 * and triggers the corresponding controls, if any.
 */
void gameplay::handle_controls(const ALLEGRO_EVENT &ev) {
    if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        if(ev.keyboard.keycode == ALLEGRO_KEY_T) {
        
            //Debug testing.
            //TODO remove any debug code that is in here before releasing.
            
            
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F1) {
        
            game.show_system_info = !game.show_system_info;
            
        } else if(
            creator_tools_enabled &&
            (
                (
                    ev.keyboard.keycode >= ALLEGRO_KEY_F2 &&
                    ev.keyboard.keycode <= ALLEGRO_KEY_F11
                ) || (
                    ev.keyboard.keycode >= ALLEGRO_KEY_0 &&
                    ev.keyboard.keycode <= ALLEGRO_KEY_9
                )
            )
        ) {
        
            unsigned char id;
            if(
                ev.keyboard.keycode >= ALLEGRO_KEY_F2 &&
                ev.keyboard.keycode <= ALLEGRO_KEY_F11
            ) {
                //The first ten indexes are the F2 - F11 keys.
                id =
                    creator_tool_keys[
                        ev.keyboard.keycode - ALLEGRO_KEY_F2
                ];
            } else {
                //The second ten indexes are the 0 - 9 keys.
                id =
                    creator_tool_keys[
                        10 + (ev.keyboard.keycode - ALLEGRO_KEY_0)
                ];
            }
            
            if(id == CREATOR_TOOL_AREA_IMAGE) {
                ALLEGRO_BITMAP* bmp = draw_to_bitmap();
                string file_name =
                    USER_DATA_FOLDER_PATH + "/Area_" + game.cur_area_data.name +
                    "_" + get_current_time(true) + ".png";
                    
                if(!al_save_bitmap(file_name.c_str(), bmp)) {
                    log_error(
                        "Could not save the area onto an image,"
                        " with the name \"" + file_name + "\"!"
                    );
                }
                
            } else if(id == CREATOR_TOOL_CHANGE_SPEED) {
                creator_tool_change_speed = !creator_tool_change_speed;
                
            } else if(id == CREATOR_TOOL_GEOMETRY_INFO) {
                creator_tool_geometry_info = !creator_tool_geometry_info;
                
            } else if(id == CREATOR_TOOL_HITBOXES) {
                creator_tool_hitboxes = !creator_tool_hitboxes;
                
            } else if(id == CREATOR_TOOL_HURT_MOB) {
                mob* m = get_closest_mob_to_cursor();
                if(m) {
                    m->set_health(true, true, -creator_tool_mob_hurting_ratio);
                }
                
            } else if(id == CREATOR_TOOL_MOB_INFO) {
                mob* m = get_closest_mob_to_cursor();
                creator_tool_info_lock =
                    (creator_tool_info_lock == m ? NULL : m);
                    
            } else if(id == CREATOR_TOOL_NEW_PIKMIN) {
                if(pikmin_list.size() < max_pikmin_in_field) {
                    pikmin_type* new_pikmin_type = pikmin_types.begin()->second;
                    
                    auto p = pikmin_types.begin();
                    for(; p != pikmin_types.end(); ++p) {
                        if(p->second == creator_tool_last_pikmin_type) {
                            ++p;
                            if(p != pikmin_types.end()) {
                                new_pikmin_type = p->second;
                            }
                            break;
                        }
                    }
                    creator_tool_last_pikmin_type = new_pikmin_type;
                    
                    create_mob(
                        mob_categories.get(MOB_CATEGORY_PIKMIN),
                        game.mouse_cursor_w, new_pikmin_type, 0, "maturity=2"
                    );
                }
                
            } else if(id == CREATOR_TOOL_TELEPORT) {
                cur_leader_ptr->chase(game.mouse_cursor_w, NULL, true);
                sector* mouse_sector =
                    get_sector(game.mouse_cursor_w, NULL, true);
                if(mouse_sector) {
                    cur_leader_ptr->z = mouse_sector->z;
                }
                
            }
            
        }
    }
    
    vector<action_from_event> actions = get_actions_from_event(ev);
    for(size_t a = 0; a < actions.size(); ++a) {
        handle_button(actions[a].button, actions[a].pos, actions[a].player);
    }
    
    for(size_t p = 0; p < MAX_PLAYERS; p++) {
        if(ev.type == ALLEGRO_EVENT_MOUSE_AXES && game.options.mouse_moves_cursor[p]) {
            game.mouse_cursor_s.x = ev.mouse.x;
            game.mouse_cursor_s.y = ev.mouse.y;
            game.mouse_cursor_w = game.mouse_cursor_s;
            
            al_transform_coordinates(
                &game.screen_to_world_transform,
                &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
            );
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Grabs an ALLEGRO_EVENT and checks all available controls.
 * For every control that matches, it adds its input information to a vector,
 * which it then returns.
 * ev:   Pointer to the event.
 */
vector<action_from_event> get_actions_from_event(const ALLEGRO_EVENT &ev) {

    vector<action_from_event> actions;
    
    for(size_t p = 0; p < MAX_PLAYERS; p++) {
        size_t n_controls = game.options.controls[p].size();
        for(size_t c = 0; c < n_controls; ++c) {
        
            control_info* con = &game.options.controls[p][c];
            
            if(
                con->type == CONTROL_TYPE_KEYBOARD_KEY &&
                (
                    ev.type == ALLEGRO_EVENT_KEY_DOWN ||
                    ev.type == ALLEGRO_EVENT_KEY_UP
                )
            ) {
                if(con->button == ev.keyboard.keycode) {
                    actions.push_back(
                        action_from_event(
                            con->action,
                            (ev.type == ALLEGRO_EVENT_KEY_DOWN) ? 1 : 0,
                            p
                        )
                    );
                }
                
            } else if(
                con->type == CONTROL_TYPE_MOUSE_BUTTON &&
                (
                    ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
                    ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
                )
            ) {
                if(con->button == (signed) ev.mouse.button) {
                    actions.push_back(
                        action_from_event(
                            con->action,
                            (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) ?
                            1 : 0,
                            p
                        )
                    );
                }
                
            } else if(
                con->type == CONTROL_TYPE_MOUSE_WHEEL_UP &&
                ev.type == ALLEGRO_EVENT_MOUSE_AXES
            ) {
                if(ev.mouse.dz > 0) {
                    actions.push_back(
                        action_from_event(con->action, ev.mouse.dz, p)
                    );
                }
                
            } else if(
                con->type == CONTROL_TYPE_MOUSE_WHEEL_DOWN &&
                ev.type == ALLEGRO_EVENT_MOUSE_AXES
            ) {
                if(ev.mouse.dz < 0) {
                    actions.push_back(
                        action_from_event(con->action, -ev.mouse.dz, p)
                    );
                }
                
            } else if(
                con->type == CONTROL_TYPE_MOUSE_WHEEL_LEFT &&
                ev.type == ALLEGRO_EVENT_MOUSE_AXES
            ) {
                if(ev.mouse.dw < 0) {
                    actions.push_back(
                        action_from_event(con->action, -ev.mouse.dw, p)
                    );
                }
                
            } else if(
                con->type == CONTROL_TYPE_MOUSE_WHEEL_RIGHT &&
                ev.type == ALLEGRO_EVENT_MOUSE_AXES
            ) {
                if(ev.mouse.dw > 0) {
                    actions.push_back(
                        action_from_event(con->action, ev.mouse.dw, p)
                    );
                }
                
            } else if(
                con->type == CONTROL_TYPE_JOYSTICK_BUTTON &&
                (
                    ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN ||
                    ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP
                )
            ) {
                if(
                    con->device_nr == game.joystick_numbers[ev.joystick.id] &&
                    (signed) con->button == ev.joystick.button
                ) {
                    actions.push_back(
                        action_from_event(
                            con->action,
                            (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) ?
                            1 : 0,
                            p
                        )
                    );
                }
                
            } else if(
                con->type == CONTROL_TYPE_JOYSTICK_AXIS_POS &&
                ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS
            ) {
                if(
                    con->device_nr == game.joystick_numbers[ev.joystick.id] &&
                    con->stick == ev.joystick.stick &&
                    con->axis == ev.joystick.axis
                ) {
                    if(ev.joystick.pos >= 0) {
                        actions.push_back(
                            action_from_event(con->action, ev.joystick.pos, p)
                        );
                    } else {
                        //Makes it so that, for instance, quickly tilting the
                        //stick left will also send a "tilt right" event of 0.
                        actions.push_back(
                            action_from_event(con->action, 0, p)
                        );
                    }
                }
                
            } else if(
                con->type == CONTROL_TYPE_JOYSTICK_AXIS_NEG &&
                ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS
            ) {
                if(
                    con->device_nr == game.joystick_numbers[ev.joystick.id] &&
                    con->stick == ev.joystick.stick &&
                    con->axis == ev.joystick.axis
                ) {
                    if(ev.joystick.pos <= 0) {
                        actions.push_back(
                            action_from_event(con->action, -ev.joystick.pos, p)
                        );
                    } else {
                        //Makes it so that, for instance, quickly tilting the
                        //stick left will also send a "tilt right" event of 0.
                        actions.push_back(
                            action_from_event(con->action, 0, p)
                        );
                    }
                }
            }
        }
        
    }
    
    return actions;
    
}
