/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Control-related functions.
 */

#include <algorithm>
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
    if(ev.type == ALLEGRO_EVENT_KEY_CHAR && ev.keyboard.keycode == ALLEGRO_KEY_T) {
        //Debug testing.
        //ToDo remove.
        day_minutes += 30;
        day += 12;
        pikmin_list[0]->health -= 10;
    }
    
    size_t n_controls = controls.size();
    for(size_t c = 0; c < n_controls; c++) {
    
        if(controls[c].type == CONTROL_TYPE_KEYBOARD_KEY && (ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_KEY_UP)) {
            if(controls[c].button == ev.keyboard.keycode) {
                handle_button(controls[c].action, (ev.type == ALLEGRO_EVENT_KEY_DOWN) ? 1 : 0);
            }
        } else if(controls[c].type == CONTROL_TYPE_MOUSE_BUTTON && (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)) {
            if(controls[c].button == (signed) ev.mouse.button) {
                handle_button(controls[c].action, (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0);
            }
        } else if(controls[c].type == CONTROL_TYPE_MOUSE_WHEEL_UP && ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(ev.mouse.dz > 0) {
                handle_button(controls[c].action, ev.mouse.dz);
            }
        } else if(controls[c].type == CONTROL_TYPE_MOUSE_WHEEL_DOWN && ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(ev.mouse.dz < 0) {
                handle_button(controls[c].action, -ev.mouse.dz);
            }
        } else if(controls[c].type == CONTROL_TYPE_MOUSE_WHEEL_LEFT && ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(ev.mouse.dw < 0) {
                handle_button(controls[c].action, -ev.mouse.dw);
            }
        } else if(controls[c].type == CONTROL_TYPE_MOUSE_WHEEL_RIGHT && ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(ev.mouse.dw > 0) {
                handle_button(controls[c].action, ev.mouse.dz);
            }
        } else if(controls[c].type == CONTROL_TYPE_JOYSTICK_BUTTON && (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP)) {
            if(controls[c].device_nr == joystick_numbers[ev.joystick.id] && (signed) controls[c].button == ev.joystick.button) {
                handle_button(controls[c].action, (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) ? 1 : 0);
            }
        } else if(controls[c].type == CONTROL_TYPE_JOYSTICK_AXIS_POS && ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
            if(
                controls[c].device_nr == joystick_numbers[ev.joystick.id] && controls[c].stick == ev.joystick.stick &&
                controls[c].axis == ev.joystick.axis && ev.joystick.pos >= 0) {
                handle_button(controls[c].action, ev.joystick.pos);
            }
        } else if(controls[c].type == CONTROL_TYPE_JOYSTICK_AXIS_NEG && ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
            if(
                controls[c].device_nr == joystick_numbers[ev.joystick.id] && controls[c].stick == ev.joystick.stick &&
                controls[c].axis == ev.joystick.axis && ev.joystick.pos <= 0) {
                handle_button(controls[c].action, -ev.joystick.pos);
            }
        }
    }
    
    for(unsigned char p = 0; p < 4; p++) {
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
void handle_button(const unsigned int button, float pos) {

    leader* cur_leader_ptr = leaders[cur_leader_nr];
    
    if(cur_message.size() == 0) {
    
        if(
            button == BUTTON_MOVE_RIGHT ||
            button == BUTTON_MOVE_UP ||
            button == BUTTON_MOVE_LEFT ||
            button == BUTTON_MOVE_DOWN
        ) {
        
            /*******************
            *              \O/ *
            *   Move   ---> |  *
            *              / \ *
            *******************/
            
            if(pos != 0) active_control();
            
            if(button == BUTTON_MOVE_RIGHT || button == BUTTON_MOVE_LEFT) leader_move_x =
                    ((button == BUTTON_MOVE_LEFT) ? -pos : pos);
            if(button == BUTTON_MOVE_DOWN || button == BUTTON_MOVE_UP) leader_move_y =
                    ((button == BUTTON_MOVE_UP) ? -pos : pos);
                    
        } else if(
            button == BUTTON_MOVE_CURSOR_RIGHT ||
            button == BUTTON_MOVE_CURSOR_UP ||
            button == BUTTON_MOVE_CURSOR_LEFT ||
            button == BUTTON_MOVE_CURSOR_DOWN
        ) {
            /********************
            *             .-.   *
            *   Cursor   ( = )> *
            *             `-´   *
            ********************/
            
            if(button == BUTTON_MOVE_CURSOR_RIGHT)     mouse_cursor_speed_x = delta_t* MOUSE_CURSOR_MOVE_SPEED * pos;
            else if(button == BUTTON_MOVE_CURSOR_UP)   mouse_cursor_speed_y = -delta_t* MOUSE_CURSOR_MOVE_SPEED * pos;
            else if(button == BUTTON_MOVE_CURSOR_LEFT) mouse_cursor_speed_x = -delta_t* MOUSE_CURSOR_MOVE_SPEED * pos;
            else if(button == BUTTON_MOVE_CURSOR_DOWN) mouse_cursor_speed_y = delta_t* MOUSE_CURSOR_MOVE_SPEED * pos;
            
        } else if(
            button == BUTTON_MOVE_GROUP_RIGHT ||
            button == BUTTON_MOVE_GROUP_UP ||
            button == BUTTON_MOVE_GROUP_LEFT ||
            button == BUTTON_MOVE_GROUP_DOWN
        ) {
            /******************
            *            ***  *
            *   Group   ****O *
            *            ***  *
            ******************/
            
            active_control();
            
            if(button == BUTTON_MOVE_GROUP_RIGHT)     moving_group_pos_x = pos;
            else if(button == BUTTON_MOVE_GROUP_UP)   moving_group_pos_y = -pos;
            else if(button == BUTTON_MOVE_GROUP_LEFT) moving_group_pos_x = -pos;
            else if(button == BUTTON_MOVE_GROUP_DOWN) moving_group_pos_y = pos;
            
        } else if(button == BUTTON_MOVE_GROUP_TO_CURSOR) {
        
            active_control();
            
            if(pos > 0) {
                moving_group_to_cursor = true;
                moving_group_intensity = 1;
            } else {
                moving_group_to_cursor = false;
                moving_group_intensity = 0;
            }
            
        } else if(button == BUTTON_THROW) {
        
            /*******************
            *             ,-.  *
            *   Throw    /   O *
            *           &      *
            *******************/
            
            if(pos > 0) { //Button press.
            
                if(auto_pluck_input_time > 0) {
                    cur_leader_ptr->auto_pluck_mode = true;
                    
                    size_t n_leaders = leaders.size();
                    for(size_t l = 0; l < n_leaders; l++) {
                        if(leaders[l]->following_party == cur_leader_ptr) {
                            leaders[l]->auto_pluck_mode = true;
                        }
                    }
                    return;
                } else {
                    active_control();
                }
                
                bool done = false;
                
                //First check if the leader should pluck a Pikmin.
                float d;
                pikmin* p = get_closest_buried_pikmin(cur_leader_ptr->x, cur_leader_ptr->y, &d, false);
                if(p && d <= MIN_PLUCK_RANGE) {
                    go_pluck(cur_leader_ptr, p);
                    auto_pluck_input_time = AUTO_PLUCK_INPUT_INTERVAL;
                    done = true;
                }
                
                //Now check if the leader should read an info spot.
                if(!done) {
                    size_t n_info_spots = info_spots.size();
                    for(size_t i = 0; i < n_info_spots; i++) {
                        info_spot* i_ptr = info_spots[i];
                        if(i_ptr->opens_box) {
                            if(check_dist(cur_leader_ptr->x, cur_leader_ptr->y, i_ptr->x, i_ptr->y, INFO_SPOT_TRIGGER_RANGE)) {
                                start_message(i_ptr->text, NULL);
                                done = true;
                                break;
                            }
                        }
                    }
                }
                
                //Now check if the leader should open an onion's menu.
                if(!done) {
                    //ToDo
                    size_t n_onions = onions.size();
                    for(size_t o = 0; o < n_onions; o++) {
                        if(check_dist(cur_leader_ptr->x, cur_leader_ptr->y, onions[o]->x, onions[o]->y, MIN_ONION_CHECK_RANGE)) {
                            if(pikmin_list.size() < max_pikmin_in_field) {
                                //ToDo this is not how it works, there can be less onions on the field than the total number of Pikmin types.
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
                    for(size_t s = 0; s < n_ships; s++) {
                        if(check_dist(cur_leader_ptr->x, cur_leader_ptr->y, ships[s]->x + ships[s]->type->radius + SHIP_BEAM_RANGE, ships[s]->y, SHIP_BEAM_RANGE)) {
                            if(ships[s]->shi_type->can_heal) {
                                //ToDo make it prettier.
                                cur_leader_ptr->health = cur_leader_ptr->type->max_health;
                                done = true;
                            }
                        }
                    }
                }
                
                //Now check if the leader should grab a Pikmin.
                
                if(!done) {
                    if(closest_party_member && !cur_leader_ptr->holding_pikmin) {
                        cur_leader_ptr->holding_pikmin = closest_party_member;
                        sfx_pikmin_held.play(0, false);
                        done = true;
                    }
                }
                
                //Now check if the leader should punch.
                
                if(!done) {
                    //ToDo
                }
                
            } else { //Button release.
                mob* holding_ptr = cur_leader_ptr->holding_pikmin;
                if(holding_ptr) {
                
                    holding_ptr->x = cur_leader_ptr->x;
                    holding_ptr->y = cur_leader_ptr->y;
                    holding_ptr->z = cur_leader_ptr->z;
                    
                    float angle, d;
                    coordinates_to_angle(cursor_x - cur_leader_ptr->x, cursor_y - cur_leader_ptr->y, &angle, &d);
                    
                    float throw_height_mult = 1.0;
                    if(typeid(*holding_ptr) == typeid(pikmin)) {
                        throw_height_mult = ((pikmin*) holding_ptr)->pik_type->throw_height_mult;
                    }
                    
                    //This results in a 1.3 second throw, just like in Pikmin 2. Regular Pikmin are thrown about 288.88 units high.
                    holding_ptr->speed_x =
                        cos(angle) * d * THROW_DISTANCE_MULTIPLIER * (1.0 / (THROW_STRENGTH_MULTIPLIER * throw_height_mult));
                    holding_ptr->speed_y =
                        sin(angle) * d * THROW_DISTANCE_MULTIPLIER * (1.0 / (THROW_STRENGTH_MULTIPLIER * throw_height_mult));
                    holding_ptr->speed_z =
                        -(GRAVITY_ADDER) * (THROW_STRENGTH_MULTIPLIER * throw_height_mult);
                        
                    holding_ptr->angle = angle;
                    holding_ptr->face(angle);
                    
                    holding_ptr->was_thrown = true;
                    
                    remove_from_party(holding_ptr);
                    cur_leader_ptr->holding_pikmin = NULL;
                    
                    sfx_pikmin_held.stop();
                    sfx_pikmin_thrown.stop();
                    sfx_throw.stop();
                    sfx_pikmin_thrown.play(0, false);
                    sfx_throw.play(0, false);
                    cur_leader_ptr->anim.change(LEADER_ANIM_THROW, true, false, false);
                    holding_ptr->anim.change(PIKMIN_ANIM_THROWN, true, false, false);
                }
            }
            
        } else if(button == BUTTON_WHISTLE) {
        
            /********************
            *              .--= *
            *   Whistle   ( @ ) *
            *              `-´  *
            ********************/
            
            active_control();
            
            if(pos > 0 && !cur_leader_ptr->holding_pikmin) { //Button pressed.
                whistling = true;
                cur_leader_ptr->lea_type->sfx_whistle.play(0, false);
                
                for(unsigned char d = 0; d < 6; d++) whistle_dot_radius[d] = -1;
                whistle_fade_time = 0;
                whistle_fade_radius = 0;
                cur_leader_ptr->anim.change(LEADER_ANIM_WHISTLING, true, true, false);
                
            } else { //Button released.
                stop_whistling();
            }
            
        } else if(
            button == BUTTON_SWITCH_CAPTAIN_RIGHT ||
            button == BUTTON_SWITCH_CAPTAIN_LEFT
        ) {
        
            /******************************
            *                    \O/  \O/ *
            *   Switch captain    | -> |  *
            *                    / \  / \ *
            ******************************/
            
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            size_t new_leader_nr = cur_leader_nr;
            if(button == BUTTON_SWITCH_CAPTAIN_RIGHT)
                new_leader_nr = (cur_leader_nr + 1) % leaders.size();
            else if(button == BUTTON_SWITCH_CAPTAIN_LEFT) {
                if(cur_leader_nr == 0) new_leader_nr = leaders.size() - 1;
                else new_leader_nr = cur_leader_nr - 1;
            }
            
            if(new_leader_nr == cur_leader_nr) return;
            
            mob* swap_leader = NULL;
            
            if(!cur_leader_ptr->speed_z) {
                cur_leader_ptr->speed_x = 0;
                cur_leader_ptr->speed_y = 0;
            }
            if(!leaders[new_leader_nr]->speed_z) {
                leaders[new_leader_nr]->speed_x = 0;
                leaders[new_leader_nr]->speed_y = 0;
            }
            leaders[new_leader_nr]->remove_target(true);
            
            //If the new leader is in another one's party, swap them.
            size_t n_leaders = leaders.size();
            for(size_t l = 0; l < n_leaders; l++) {
                if(l == new_leader_nr) continue;
                size_t n_party_members = leaders[l]->party->members.size();
                for(size_t m = 0; m < n_party_members; m++) {
                    if(leaders[l]->party->members[m] == leaders[new_leader_nr]) {
                        swap_leader = leaders[l];
                        break;
                    }
                }
            }
            
            if(swap_leader) {
                size_t n_party_members = swap_leader->party->members.size();
                for(size_t m = 0; m < n_party_members; m++) {
                    mob* member = swap_leader->party->members[0];
                    remove_from_party(member);
                    if(member != leaders[new_leader_nr]) {
                        add_to_party(leaders[new_leader_nr], member);
                    }
                }
                
                add_to_party(leaders[new_leader_nr], swap_leader);
            }
            
            cur_leader_nr = new_leader_nr;
            start_camera_pan(leaders[new_leader_nr]->x, leaders[new_leader_nr]->y);
            leaders[new_leader_nr]->lea_type->sfx_name_call.play(0, false);
            
        } else if(button == BUTTON_DISMISS) {
        
            /***********************
            *             \O/ / *  *
            *   Dismiss    |   - * *
            *             / \ \ *  *
            ***********************/
            
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            dismiss();
            
        } else if(button == BUTTON_PAUSE) {
        
            /********************
            *           +-+ +-+ *
            *   Pause   | | | | *
            *           +-+ +-+ *
            ********************/
            
            if(pos == 0) return;
            
            running = false; //ToDo menu, not quit.
            //paused = true;
            
        } else if(button == BUTTON_USE_SPRAY_1) {
        
            /*******************
            *             +=== *
            *   Sprays   (   ) *
            *             `-´  *
            *******************/
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            if(spray_types.size() == 1 || spray_types.size() == 2) {
                use_spray(0);
            }
            
        } else if(button == BUTTON_USE_SPRAY_2) {
        
            if(pos == 0 || cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            if(spray_types.size() == 2) {
                use_spray(1);
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
                use_spray(selected_spray);
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
            if(cam_trans_zoom_time_left > 0) zoom_to_compare = cam_trans_zoom_final_level; else zoom_to_compare = cam_zoom;
            
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
            if(cam_trans_zoom_time_left) current_zoom = cam_trans_zoom_final_level; else current_zoom = cam_zoom;
            
            pos = floor(pos);
            
            if(button == BUTTON_ZOOM_IN) new_zoom = current_zoom + 0.1 * pos; else new_zoom = current_zoom - 0.1 * pos;
            
            if(new_zoom > ZOOM_MAX_LEVEL) new_zoom = ZOOM_MAX_LEVEL;
            if(new_zoom < ZOOM_MIN_LEVEL) new_zoom = ZOOM_MIN_LEVEL;
            
            if(cam_trans_zoom_time_left) {
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
            
            if(cur_leader_ptr->carrier_info) {
                active_control();
            } else {
            
                dismiss();
                
                cur_leader_ptr->carrier_info = new carrier_info_struct(
                    cur_leader_ptr,
                    3, //ToDo
                    false);
                    
                cur_leader_ptr->anim.change(LEADER_ANIM_LIE, true, false, false);
            }
            
        } else if(button == BUTTON_SWITCH_TYPE_RIGHT || button == BUTTON_SWITCH_TYPE_LEFT) {
        
            /****************************
            *                     -->   *
            *   Switch type   <(¨)> (ö) *
            *                           *
            *****************************/
            
            if(pos == 0 || !cur_leader_ptr->holding_pikmin) return;
            
            active_control();
            
            vector<pikmin_type*> types_in_party;
            
            size_t n_members = cur_leader_ptr->party->members.size();
            //Get all Pikmin types in the group.
            for(size_t m = 0; m < n_members; m++) {
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
            for(size_t t = 0; t < n_types; t++) {
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
            for(size_t m = 0; m < n_members; m++) {
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
            if(tm_match_nr == n_members + 1) cur_leader_ptr->holding_pikmin = cur_leader_ptr->party->members[t_match_nr];
            else cur_leader_ptr->holding_pikmin = cur_leader_ptr->party->members[tm_match_nr];
            sfx_switch_pikmin.play(0, false);
            
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
            for(size_t m = 0; m < n_members; m++) {
                if(typeid(*cur_leader_ptr->party->members[m]) == typeid(pikmin)) {
                    pikmin* pikmin_ptr = dynamic_cast<pikmin*>(cur_leader_ptr->party->members[m]);
                    
                    if(pikmin_ptr == cur_leader_ptr->holding_pikmin) continue;
                    
                    if(partners[pikmin_ptr->maturity] == NULL && pikmin_ptr->type == current_type) {
                        partners[pikmin_ptr->maturity] = pikmin_ptr;
                    }
                }
            }
            
            bool any_partners = false;
            for(unsigned char p = 0; p < 3; p++) {
                if(partners[p]) any_partners = true;
            }
            
            if(!any_partners) return;
            
            new_maturity = current_maturity;
            do {
                if(button == BUTTON_SWITCH_MATURITY_DOWN) new_maturity = ((new_maturity - 1) + 3) % 3;
                else new_maturity = (new_maturity + 1) % 3;
            } while(!partners[new_maturity]);
            
            cur_leader_ptr->holding_pikmin = partners[new_maturity];
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
 * Call this whenever an "active" control is inputted. An "active" control is anything that moves the captain in some way.
 * This function makes the captain wake up from lying down, stop auto-plucking, etc.
 */
void active_control() {
    if(leaders[cur_leader_nr]->carrier_info) {
        //Getting up.
        leaders[cur_leader_nr]->anim.change(LEADER_ANIM_GET_UP, true, false, false);
    }
    make_uncarriable(leaders[cur_leader_nr]);
    stop_auto_pluck(leaders[cur_leader_nr]);
}


/* ----------------------------------------------------------------------------
 * Creates information about a control.
 * action: The action this control does in-game. Use BUTTON_*.
 * player: Player number.
 * s:      The textual code that represents the hardware inputs.
 */
control_info::control_info(unsigned char action, unsigned char player, string s) {
    this->action = action;
    this->player = player;
    type = CONTROL_TYPE_NONE;
    
    device_nr = 0;
    button = 0;
    stick = 0;
    axis = 0;
    
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
            "Unrecognized control type \"" + parts[0] + "\" for player " +
            i2s((player + 1)) + " (value=\"" + s + "\").");
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
