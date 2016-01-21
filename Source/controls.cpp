/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
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

#include "const.h"
#include "controls.h"
#include "functions.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Handles an Allegro event related to hardware input,
 * and triggers the corresponding controls, if any.
 */
void handle_game_controls(const ALLEGRO_EVENT &ev) {
    //Debugging.
    if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        if(ev.keyboard.keycode == ALLEGRO_KEY_T) {
            //Debug testing.
            //TODO remove.
            dist closest_mob_to_cursor_dist = FLT_MAX;
            mob* closest_mob_to_cursor = NULL;
            float actual_cursor_x = mouse_cursor_x, actual_cursor_y = mouse_cursor_y;
            ALLEGRO_TRANSFORM t = get_world_to_screen_transform();
            al_invert_transform(&t);
            al_transform_coordinates(&t, &actual_cursor_x, &actual_cursor_y);
            for(size_t m = 0; m < mobs.size(); ++m) {
                dist d = dist(actual_cursor_x, actual_cursor_y, mobs[m]->x, mobs[m]->y);
                if(d < closest_mob_to_cursor_dist) {
                    closest_mob_to_cursor = mobs[m];
                    closest_mob_to_cursor_dist = d;
                }
            }
            if(closest_mob_to_cursor && closest_mob_to_cursor->fsm.cur_state) {
                string name = closest_mob_to_cursor->type->name;
                name += "        ";
                if(name.size() > 8) name = name.substr(0, 8);
                
                cout << "Mob: " << name << ". State: " << closest_mob_to_cursor->fsm.cur_state->name << "\n";
            }
            
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F1) {
            debug_show_framerate = !debug_show_framerate;
        } else if(ev.keyboard.keycode == ALLEGRO_KEY_F2) {
            if(!debug_last_axis.empty()) debug_last_axis.clear();
            else debug_last_axis = "Input a joystick axis control.";
        }
    }
    if(ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS && !debug_last_axis.empty()) {
        debug_last_axis =
            "Stick: " + i2s(ev.joystick.stick) + ". "
            "Axis: " + i2s(ev.joystick.axis) + ".";
    }
    
    
    for(size_t p = 0; p < 4; p++) {
        size_t n_controls = controls[p].size();
        for(size_t c = 0; c < n_controls; ++c) {
        
            control_info* con = &controls[p][c];
            
            if(con->type == CONTROL_TYPE_KEYBOARD_KEY && (ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_KEY_UP)) {
                if(con->button == ev.keyboard.keycode) {
                    handle_button(con->action, p, (ev.type == ALLEGRO_EVENT_KEY_DOWN) ? 1 : 0);
                }
            } else if(con->type == CONTROL_TYPE_MOUSE_BUTTON && (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)) {
                if(con->button == (signed) ev.mouse.button) {
                    handle_button(con->action, p, (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0);
                }
            } else if(con->type == CONTROL_TYPE_MOUSE_WHEEL_UP && ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
                if(ev.mouse.dz > 0) {
                    handle_button(con->action, p, ev.mouse.dz);
                }
            } else if(con->type == CONTROL_TYPE_MOUSE_WHEEL_DOWN && ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
                if(ev.mouse.dz < 0) {
                    handle_button(con->action, p, -ev.mouse.dz);
                }
            } else if(con->type == CONTROL_TYPE_MOUSE_WHEEL_LEFT && ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
                if(ev.mouse.dw < 0) {
                    handle_button(con->action, p, -ev.mouse.dw);
                }
            } else if(con->type == CONTROL_TYPE_MOUSE_WHEEL_RIGHT && ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
                if(ev.mouse.dw > 0) {
                    handle_button(con->action, p, ev.mouse.dw);
                }
            } else if(con->type == CONTROL_TYPE_JOYSTICK_BUTTON && (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP)) {
                if(con->device_nr == joystick_numbers[ev.joystick.id] && (signed) con->button == ev.joystick.button) {
                    handle_button(con->action, p, (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) ? 1 : 0);
                }
            } else if(con->type == CONTROL_TYPE_JOYSTICK_AXIS_POS && ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
                if(
                    con->device_nr == joystick_numbers[ev.joystick.id] && con->stick == ev.joystick.stick &&
                    con->axis == ev.joystick.axis && ev.joystick.pos >= 0) {
                    handle_button(con->action, p, ev.joystick.pos);
                }
            } else if(con->type == CONTROL_TYPE_JOYSTICK_AXIS_NEG && ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
                if(
                    con->device_nr == joystick_numbers[ev.joystick.id] && con->stick == ev.joystick.stick &&
                    con->axis == ev.joystick.axis && ev.joystick.pos <= 0) {
                    handle_button(con->action, p, -ev.joystick.pos);
                }
            }
        }
        
        if(ev.type == ALLEGRO_EVENT_MOUSE_AXES && mouse_moves_cursor[p]) {
            mouse_cursor_x = ev.mouse.x;
            mouse_cursor_y = ev.mouse.y;
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Handles a button "press". Technically, it could also be a button release.
 * button: The button's ID. Use BUTTON_*.
 * pos:    The position of the button, i.e., how much it's "held".
   * 0 means it was released. 1 means it was fully pressed.
   * For controls with more sensibility, values between 0 and 1 are important.
   * Like a 0.5 for the group movement makes it move at half distance.
 */
void handle_button(const unsigned int button, const unsigned char player, float pos) {

    if(cur_message.empty()) {
    
        if(
            button == BUTTON_MOVE_RIGHT ||
            button == BUTTON_MOVE_UP ||
            button == BUTTON_MOVE_LEFT ||
            button == BUTTON_MOVE_DOWN
        ) {
        
            /*******************
            *               O_ *
            *   Move   --->/|  *
            *              V > *
            *******************/
            
            if(pos != 0) active_control();
            
            if(     button == BUTTON_MOVE_RIGHT) leader_movement.right = pos;
            else if(button == BUTTON_MOVE_LEFT)  leader_movement.left =  pos;
            else if(button == BUTTON_MOVE_UP)    leader_movement.up =    pos;
            else if(button == BUTTON_MOVE_DOWN)  leader_movement.down =  pos;
            
        } else if(
            button == BUTTON_MOVE_CURSOR_RIGHT ||
            button == BUTTON_MOVE_CURSOR_UP ||
            button == BUTTON_MOVE_CURSOR_LEFT ||
            button == BUTTON_MOVE_CURSOR_DOWN
        ) {
            /********************
            *             .-.   *
            *   Cursor   ( = )> *
            *             '-'   *
            ********************/
            
            if(     button == BUTTON_MOVE_CURSOR_RIGHT) cursor_movement.right = pos;
            else if(button == BUTTON_MOVE_CURSOR_LEFT)  cursor_movement.left =  pos;
            else if(button == BUTTON_MOVE_CURSOR_UP)    cursor_movement.up =    pos;
            else if(button == BUTTON_MOVE_CURSOR_DOWN)  cursor_movement.down =  pos;
            
        } else if(
            button == BUTTON_GROUP_MOVE_RIGHT ||
            button == BUTTON_GROUP_MOVE_UP ||
            button == BUTTON_GROUP_MOVE_LEFT ||
            button == BUTTON_GROUP_MOVE_DOWN
        ) {
            /******************
            *            ***  *
            *   Group   ****O *
            *            ***  *
            ******************/
            
            active_control();
            
            if(     button == BUTTON_GROUP_MOVE_RIGHT) group_movement.right = pos;
            else if(button == BUTTON_GROUP_MOVE_LEFT)  group_movement.left =  pos;
            else if(button == BUTTON_GROUP_MOVE_UP)    group_movement.up =    pos;
            else if(button == BUTTON_GROUP_MOVE_DOWN)  group_movement.down =  pos;
            
            if(group_movement.get_intensity() != 0) {
                cur_leader_ptr->signal_group_move_start();
            } else {
                cur_leader_ptr->signal_group_move_end();
            }
            
        } else if(button == BUTTON_GROUP_MOVE_GO_TO_CURSOR) {
        
            active_control();
            
            if(pos > 0) {
                group_move_go_to_cursor = true;
                group_move_intensity = 1;
                cur_leader_ptr->signal_group_move_start();
            } else {
                group_move_go_to_cursor = false;
                group_move_intensity = 0;
                cur_leader_ptr->signal_group_move_end();
            }
            
        } else if(button == BUTTON_THROW) {
        
            /*******************
            *             .-.  *
            *   Throw    /   O *
            *           &      *
            *******************/
            
            if(pos > 0) { //Button press.
            
                active_control();
                
                bool done = false;
                
                //First check if the leader should pluck a Pikmin.
                dist d;
                pikmin* p = get_closest_buried_pikmin(cur_leader_ptr->x, cur_leader_ptr->y, &d, false);
                if(p && d <= MIN_PLUCK_RANGE) {
                    cur_leader_ptr->fsm.run_event(LEADER_EVENT_GO_PLUCK, (void*) p);
                    done = true;
                }
                
                //Now check if the leader should read an info spot.
                if(!done) {
                    size_t n_info_spots = info_spots.size();
                    for(size_t i = 0; i < n_info_spots; ++i) {
                        info_spot* i_ptr = info_spots[i];
                        if(i_ptr->opens_box) {
                            if(dist(cur_leader_ptr->x, cur_leader_ptr->y, i_ptr->x, i_ptr->y) <= INFO_SPOT_TRIGGER_RANGE) {
                                start_message(i_ptr->text, NULL);
                                done = true;
                                break;
                            }
                        }
                    }
                }
                
                //Now check if the leader should open an Onion's menu.
                if(!done) {
                    //TODO
                    size_t n_onions = onions.size();
                    for(size_t o = 0; o < n_onions; ++o) {
                        if(dist(cur_leader_ptr->x, cur_leader_ptr->y, onions[o]->x, onions[o]->y) <= MIN_ONION_CHECK_RANGE) {
                            if(pikmin_list.size() < max_pikmin_in_field) {
                                //TODO this is not how it works, there can be less Onions on the field than the total number of Pikmin types.
                                pikmin_in_onions[onions[o]->oni_type->pik_type]--;
                                create_mob(new pikmin(onions[o]->x, onions[o]->y, onions[o]->oni_type->pik_type, 0, ""));
                                add_to_party(cur_leader_ptr, pikmin_list[pikmin_list.size() - 1]);
                            }
                            done = true;
                        }
                    }
                }
                
                //Now check if the leader should heal themselves on the ship.
                if(!done) {
                    size_t n_ships = ships.size();
                    for(size_t s = 0; s < n_ships; ++s) {
                        if(dist(cur_leader_ptr->x, cur_leader_ptr->y, ships[s]->x + ships[s]->type->radius + SHIP_BEAM_RANGE, ships[s]->y) <= SHIP_BEAM_RANGE) {
                            if(ships[s]->shi_type->can_heal) {
                                //TODO make the whole process prettier.
                                cur_leader_ptr->health = cur_leader_ptr->type->max_health;
                                done = true;
                            }
                        }
                    }
                }
                
                //Now check if the leader should grab a Pikmin.
                if(!done) {
                    if(closest_party_member) {
                        mob_event* grabbed_ev = closest_party_member->fsm.get_event(MOB_EVENT_GRABBED_BY_FRIEND);
                        mob_event* grabber_ev = cur_leader_ptr->fsm.get_event(LEADER_EVENT_HOLDING);
                        if(grabber_ev && grabbed_ev) {
                            cur_leader_ptr->fsm.run_event(LEADER_EVENT_HOLDING, (void*) closest_party_member);
                            grabbed_ev->run(closest_party_member, (void*) closest_party_member);
                            done = true;
                        }
                    }
                }
                
                //Now check if the leader should punch.
                if(!done) {
                    //TODO
                }
                
            } else { //Button release.
                mob* holding_ptr = cur_leader_ptr->holding_pikmin;
                if(holding_ptr) {
                    cur_leader_ptr->fsm.run_event(LEADER_EVENT_THROW);
                }
            }
            
        } else if(button == BUTTON_WHISTLE) {
        
            /********************
            *              .--= *
            *   Whistle   ( @ ) *
            *              '-'  *
            ********************/
            
            active_control();
            
            if(pos > 0 && !cur_leader_ptr->holding_pikmin) {
                //Button pressed.
                cur_leader_ptr->fsm.run_event(LEADER_EVENT_START_WHISTLE);
                
            } else {
                //Button released.
                cur_leader_ptr->fsm.run_event(LEADER_EVENT_STOP_WHISTLE);
                
            }
            
        } else if(
            button == BUTTON_SWITCH_LEADER_RIGHT ||
            button == BUTTON_SWITCH_LEADER_LEFT
        ) {
        
            /******************************
            *                    \O/  \O/ *
            *   Switch leader     | -> |  *
            *                    / \  / \ *
            ******************************/
            
            if(pos == 0 || leaders.size() == 1) return;
            
            size_t new_leader_nr = cur_leader_nr;
            leader* new_leader_ptr = nullptr;
            bool search_new_leader = true;
            
            if(!cur_leader_ptr->fsm.get_event(LEADER_EVENT_UNFOCUSED)) {
                //This leader isn't ready to be switched out of. Forget it.
                return;
            }
            
            //We'll send the switch event to the next leader on the list.
            //If they accept, they run a function to change leaders.
            //If not, we try the next leader.
            //If we return to the current leader without anything being
            //changed, then stop trying; no leader can be switched to.
            
            size_t original_leader_nr = cur_leader_nr;
            
            while(search_new_leader) {
                if(button == BUTTON_SWITCH_LEADER_RIGHT)
                    new_leader_nr = (new_leader_nr + 1) % leaders.size();
                else if(button == BUTTON_SWITCH_LEADER_LEFT) {
                    if(new_leader_nr == 0) new_leader_nr = leaders.size() - 1;
                    else new_leader_nr = new_leader_nr - 1;
                }
                new_leader_ptr = leaders[new_leader_nr];
                
                if(new_leader_nr == original_leader_nr) {
                    //Back to the original; stop trying.
                    return;
                }
                
                new_leader_ptr->fsm.run_event(LEADER_EVENT_FOCUSED);
                
                //If after we called the event, the leader is the same,
                //then that means the leader can't be switched to.
                //Try a new one.
                if(cur_leader_nr != original_leader_nr) {
                    search_new_leader = false;
                }
            }
            
        } else if(button == BUTTON_DISMISS) {
        
            /***********************
            *             \O/ / *  *
            *   Dismiss    |   - * *
            *             / \ \ *  *
            ***********************/
            
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            cur_leader_ptr->fsm.run_event(LEADER_EVENT_DISMISS);
            
        } else if(button == BUTTON_PAUSE) {
        
            /********************
            *           +-+ +-+ *
            *   Pause   | | | | *
            *           +-+ +-+ *
            ********************/
            
            if(pos == 0) return;
            
            running = false; //TODO pause menu, not quit.
            //paused = true;
            
        } else if(button == BUTTON_USE_SPRAY_1) {
        
            /*******************
            *             +=== *
            *   Sprays   (   ) *
            *             '-'  *
            *******************/
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            if(spray_types.size() == 1 || spray_types.size() == 2) {
                size_t spray_nr = 0;
                cur_leader_ptr->fsm.run_event(LEADER_EVENT_SPRAY, (void*) &spray_nr);
            }
            
        } else if(button == BUTTON_USE_SPRAY_2) {
        
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            if(spray_types.size() == 2) {
                size_t spray_nr = 1;
                cur_leader_ptr->fsm.run_event(LEADER_EVENT_SPRAY, (void*) &spray_nr);
            }
            
        } else if(button == BUTTON_SWITCH_SPRAY_RIGHT || button == BUTTON_SWITCH_SPRAY_LEFT) {
        
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            if(spray_types.size() > 2) {
                if(button == BUTTON_SWITCH_SPRAY_RIGHT) {
                    selected_spray = (selected_spray + 1) % spray_types.size();
                } else {
                    if(selected_spray == 0) selected_spray = spray_types.size() - 1;
                    else selected_spray--;
                }
            }
            
        } else if(button == BUTTON_USE_SPRAY) {
        
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            if(spray_types.size() > 2) {
                cur_leader_ptr->fsm.run_event(LEADER_EVENT_SPRAY, (void*) &selected_spray);
            }
            
        } else if(button == BUTTON_SWITCH_ZOOM) {
        
            /***************
            *           _  *
            *   Zoom   (_) *
            *          /   *
            ***************/
            
            if(pos == 0) return;
            
            float new_zoom;
            float zoom_to_compare;
            if(cam_trans_zoom_timer.time_left > 0) zoom_to_compare = cam_trans_zoom_final_level; else zoom_to_compare = cam_zoom;
            
            if(zoom_to_compare < 1) {
                new_zoom = ZOOM_MAX_LEVEL;
            } else if(zoom_to_compare > 1) {
                new_zoom = 1;
            } else {
                new_zoom = ZOOM_MIN_LEVEL;
            }
            
            start_camera_zoom(new_zoom);
            
        } else if(button == BUTTON_ZOOM_IN || button == BUTTON_ZOOM_OUT) {
        
            if((cam_zoom == ZOOM_MAX_LEVEL && button == BUTTON_ZOOM_IN) || (cam_zoom == ZOOM_MIN_LEVEL && button == BUTTON_ZOOM_OUT)) return;
            
            float new_zoom;
            float current_zoom;
            if(cam_trans_zoom_timer.time_left > 0) current_zoom = cam_trans_zoom_final_level; else current_zoom = cam_zoom;
            
            pos = floor(pos);
            
            if(button == BUTTON_ZOOM_IN) new_zoom = current_zoom + 0.1 * pos; else new_zoom = current_zoom - 0.1 * pos;
            
            if(new_zoom > ZOOM_MAX_LEVEL) new_zoom = ZOOM_MAX_LEVEL;
            if(new_zoom < ZOOM_MIN_LEVEL) new_zoom = ZOOM_MIN_LEVEL;
            
            if(cam_trans_zoom_timer.time_left > 0) {
                cam_trans_zoom_final_level = new_zoom;
            } else {
                start_camera_zoom(new_zoom);
            }
            
        } else if(button == BUTTON_LIE_DOWN) {
        
            /**********************
            *                     *
            *   Lie down  -()/__/ *
            *                     *
            ***********************/
            
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            cur_leader_ptr->fsm.run_event(LEADER_EVENT_LIE_DOWN);
            
            
        } else if(button == BUTTON_SWITCH_TYPE_RIGHT || button == BUTTON_SWITCH_TYPE_LEFT) {
        
            /****************************
            *                     -->   *
            *   Switch type   <( )> (o) *
            *                           *
            *****************************/
            
            if(pos == 0 || !cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            vector<pikmin_type*> types_in_party;
            
            size_t n_members = cur_leader_ptr->party->members.size();
            //Get all Pikmin types in the group.
            for(size_t m = 0; m < n_members; ++m) {
                if(typeid(*cur_leader_ptr->party->members[m]) == typeid(pikmin)) {
                    pikmin* pikmin_ptr = dynamic_cast<pikmin*>(cur_leader_ptr->party->members[m]);
                    
                    if(find(types_in_party.begin(), types_in_party.end(), pikmin_ptr->type) == types_in_party.end()) {
                        types_in_party.push_back(pikmin_ptr->pik_type);
                    }
                } else if(typeid(*cur_leader_ptr->party->members[m]) == typeid(leader)) {
                
                    if(find(types_in_party.begin(), types_in_party.end(), (pikmin_type*) NULL) == types_in_party.end()) {
                        types_in_party.push_back(NULL); //NULL represents leaders.
                    }
                }
            }
            
            size_t n_types = types_in_party.size();
            if(n_types == 1) return;
            
            pikmin_type* current_type = NULL;
            pikmin_type* new_type = NULL;
            unsigned char current_maturity = 255;
            if(typeid(*cur_leader_ptr->holding_pikmin) == typeid(pikmin)) {
                pikmin* pikmin_ptr = dynamic_cast<pikmin*>(cur_leader_ptr->holding_pikmin);
                current_type = pikmin_ptr->pik_type;
                current_maturity = pikmin_ptr->maturity;
            }
            
            
            //Go one type adjacent to the current member being held.
            for(size_t t = 0; t < n_types; ++t) {
                if(current_type == types_in_party[t]) {
                    if(button == BUTTON_SWITCH_TYPE_RIGHT) {
                        new_type = types_in_party[(t + 1) % n_types];
                    } else {
                        new_type = types_in_party[((t - 1) + n_types) % n_types];
                    }
                }
            }
            
            size_t t_match_nr = n_members + 1; //Number of the member that matches the type we want.
            size_t tm_match_nr = n_members + 1; //Number of the member that matches the type and maturity we want.
            
            //Find a Pikmin of the new type.
            for(size_t m = 0; m < n_members; ++m) {
                if(typeid(*cur_leader_ptr->party->members[m]) == typeid(pikmin)) {
                
                    pikmin* pikmin_ptr = dynamic_cast<pikmin*>(cur_leader_ptr->party->members[m]);
                    if(pikmin_ptr->type == new_type) {
                        t_match_nr = m;
                        if(pikmin_ptr->maturity == current_maturity) {
                            tm_match_nr = m;
                            break;
                        }
                    }
                    
                } else if(typeid(*cur_leader_ptr->party->members[m]) == typeid(leader)) {
                
                    if(new_type == NULL) {
                        t_match_nr = m;
                        tm_match_nr = m;
                        break;
                    }
                }
            }
            
            //If no Pikmin matched the maturity, just use the one we found.
            if(tm_match_nr == n_members + 1) swap_pikmin(cur_leader_ptr->party->members[t_match_nr]);
            else swap_pikmin(cur_leader_ptr->party->members[tm_match_nr]);
            
        } else if(button == BUTTON_SWITCH_MATURITY_DOWN || button == BUTTON_SWITCH_MATURITY_UP) {
        
            if(pos == 0 || !cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            pikmin_type* current_type = NULL;
            unsigned char current_maturity = 255;
            unsigned char new_maturity = 255;
            pikmin* partners[3] = {NULL, NULL, NULL};
            if(typeid(*cur_leader_ptr->holding_pikmin) == typeid(pikmin)) {
                pikmin* pikmin_ptr = dynamic_cast<pikmin*>(cur_leader_ptr->holding_pikmin);
                current_type = pikmin_ptr->pik_type;
                current_maturity = pikmin_ptr->maturity;
            }
            
            size_t n_members = cur_leader_ptr->party->members.size();
            //Get Pikmin of the same type, one for each maturity.
            for(size_t m = 0; m < n_members; ++m) {
                if(typeid(*cur_leader_ptr->party->members[m]) == typeid(pikmin)) {
                    pikmin* pikmin_ptr = dynamic_cast<pikmin*>(cur_leader_ptr->party->members[m]);
                    
                    if(pikmin_ptr == cur_leader_ptr->holding_pikmin) continue;
                    
                    if(partners[pikmin_ptr->maturity] == NULL && pikmin_ptr->type == current_type) {
                        partners[pikmin_ptr->maturity] = pikmin_ptr;
                    }
                }
            }
            
            bool any_partners = false;
            for(unsigned char p = 0; p < 3; ++p) {
                if(partners[p]) any_partners = true;
            }
            
            if(!any_partners) return;
            
            new_maturity = current_maturity;
            do {
                if(button == BUTTON_SWITCH_MATURITY_DOWN) new_maturity = ((new_maturity - 1) + 3) % 3;
                else new_maturity = (new_maturity + 1) % 3;
            } while(!partners[new_maturity]);
            
            swap_pikmin(partners[new_maturity]);
            sfx_switch_pikmin.play(0, false);
            
        }
        
    } else { //Displaying a message.
    
        if((button == BUTTON_THROW || button == BUTTON_PAUSE) && pos == 1) {
            size_t stopping_char = cur_message_stopping_chars[cur_message_section + 1];
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
 * Call this whenever an "active" control is inputted. An "active" control is anything that moves the leader in some way.
 * This function makes the leader wake up from lying down, stop auto-plucking, etc.
 */
void active_control() {
    cur_leader_ptr->fsm.run_event(LEADER_EVENT_CANCEL);
}


/* ----------------------------------------------------------------------------
 * Creates information about a control.
 * action: The action this control does in-game. Use BUTTON_*.
 * player: Player number.
 * s:      The textual code that represents the hardware inputs.
 */
control_info::control_info(unsigned char action, string s) :
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
        error_log(
            "Unrecognized control type \"" + parts[0] + "\" (value=\"" + s + "\").");
    }
}


/* ----------------------------------------------------------------------------
 * Converts a control info's hardware input data into a string, used in the options file.
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
