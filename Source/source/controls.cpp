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
#include "game_states/gameplay/gameplay.h"
#include "utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Processes a key press to check if it should do some "system" action,
 * like toggle the framerate, or activate a maker tool.
 * keycode:
 *   Allegro keycode of the pressed key.
 */
void gameplay_state::process_system_key_press(const int keycode) {
    if(keycode == ALLEGRO_KEY_F1) {
    
        game.show_system_info = !game.show_system_info;
        
    } else if(
        game.maker_tools.enabled &&
        (
            (keycode >= ALLEGRO_KEY_F2 && keycode <= ALLEGRO_KEY_F11) ||
            (keycode >= ALLEGRO_KEY_0 && keycode <= ALLEGRO_KEY_9)
        )
    ) {
    
        unsigned char id;
        if(keycode >= ALLEGRO_KEY_F2 && keycode <= ALLEGRO_KEY_F11) {
            //The first ten indexes are the F2 - F11 keys.
            id = game.maker_tools.keys[keycode - ALLEGRO_KEY_F2];
        } else {
            //The second ten indexes are the 0 - 9 keys.
            id = game.maker_tools.keys[10 + (keycode - ALLEGRO_KEY_0)];
        }
        
        switch(id) {
        case MAKER_TOOL_AREA_IMAGE: {
            ALLEGRO_BITMAP* bmp = draw_to_bitmap();
            string file_name =
                USER_DATA_FOLDER_PATH + "/Area_" +
                sanitize_file_name(game.cur_area_data.name) +
                "_" + get_current_time(true) + ".png";
                
            if(!al_save_bitmap(file_name.c_str(), bmp)) {
                log_error(
                    "Could not save the area onto an image,"
                    " with the name \"" + file_name + "\"!"
                );
            }
            
            break;
            
        } case MAKER_TOOL_CHANGE_SPEED: {
            game.maker_tools.change_speed =
                !game.maker_tools.change_speed;
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_COLLISION: {
            game.maker_tools.collision =
                !game.maker_tools.collision;
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_GEOMETRY_INFO: {
            game.maker_tools.geometry_info =
                !game.maker_tools.geometry_info;
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_HITBOXES: {
            game.maker_tools.hitboxes =
                !game.maker_tools.hitboxes;
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_HUD: {
            game.maker_tools.hud = !game.maker_tools.hud;
            break;
            
        } case MAKER_TOOL_HURT_MOB: {
            mob* m = get_closest_mob_to_cursor();
            if(m) {
                m->set_health(
                    true, true, -game.maker_tools.mob_hurting_ratio
                );
            }
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_MOB_INFO: {
            mob* m = get_closest_mob_to_cursor();
            game.maker_tools.info_lock =
                (game.maker_tools.info_lock == m ? NULL : m);
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_NEW_PIKMIN: {
            if(mobs.pikmin_list.size() < game.config.max_pikmin_in_field) {
                pikmin_type* new_pikmin_type =
                    game.mob_types.pikmin.begin()->second;
                    
                auto p = game.mob_types.pikmin.begin();
                for(; p != game.mob_types.pikmin.end(); ++p) {
                    if(p->second == game.maker_tools.last_pikmin_type) {
                        ++p;
                        if(p != game.mob_types.pikmin.end()) {
                            new_pikmin_type = p->second;
                        }
                        break;
                    }
                }
                game.maker_tools.last_pikmin_type = new_pikmin_type;
                
                create_mob(
                    game.mob_categories.get(MOB_CATEGORY_PIKMIN),
                    game.mouse_cursor_w, new_pikmin_type, 0, "maturity=2"
                );
            }
            game.maker_tools.used_helping_tools = true;
            
            break;
            
        } case MAKER_TOOL_TELEPORT: {
            sector* mouse_sector =
                get_sector(game.mouse_cursor_w, NULL, true);
            if(mouse_sector && cur_leader_ptr) {
                cur_leader_ptr->chase(
                    game.mouse_cursor_w, mouse_sector->z,
                    CHASE_FLAG_TELEPORT
                );
                game.cam.set_pos(game.mouse_cursor_w);
            }
            game.maker_tools.used_helping_tools = true;
            break;
            
        }
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Constructs a new player action type.
 */
player_action_type::player_action_type() :
    id(PLAYER_ACTION_NONE) {
    
}


/* ----------------------------------------------------------------------------
 * Adds a new bind between player input and game action type.
 */
void controls_mediator::add_bind(const control_bind &bind) {
    mgr.add_bind(bind);
}


/* ----------------------------------------------------------------------------
 * Adds a new player action to the list.
 * id:
 *   Its ID.
 * name:
 *   Its name.
 * internal_name:
 *   The name of its property in the options file.
 * default_bind_str:
 *   String representing of this action's default control bind.
 */
void controls_mediator::add_player_action_type(
    const PLAYER_ACTION_TYPES id,
    const string &name, const string &internal_name,
    const string &default_bind_str
) {
    player_action_type a;
    a.id = id;
    a.name = name;
    a.internal_name = internal_name;
    a.default_bind_str = default_bind_str;
    
    player_action_types.push_back(a);
}


/* ----------------------------------------------------------------------------
 * Creates a string that represents a control bind.
 * Ignores the player number.
 * Returns an empty string on error.
 * b:
 *   Control bind to read from.
 */
string controls_mediator::bind_to_str(
    const control_bind &b
) const {
    switch(b.input.type) {
    case INPUT_TYPE_KEYBOARD_KEY: {
        return "k_" + i2s(b.input.button_nr);
    } case INPUT_TYPE_MOUSE_BUTTON: {
        return "mb_" + i2s(b.input.button_nr);
    } case INPUT_TYPE_MOUSE_WHEEL_UP: {
        return "mwu";
    } case INPUT_TYPE_MOUSE_WHEEL_DOWN: {
        return "mwd";
    } case INPUT_TYPE_MOUSE_WHEEL_LEFT: {
        return "mwl";
    } case INPUT_TYPE_MOUSE_WHEEL_RIGHT: {
        return "mwr";
    } case INPUT_TYPE_CONTROLLER_BUTTON: {
        return "jb_" + i2s(b.input.device_nr) + "_" + i2s(b.input.button_nr);
    } case INPUT_TYPE_CONTROLLER_AXIS_POS: {
        return
            "jap_" + i2s(b.input.device_nr) +
            "_" + i2s(b.input.stick_nr) + "_" + i2s(b.input.axis_nr);
    } case INPUT_TYPE_CONTROLLER_AXIS_NEG: {
        return
            "jan_" + i2s(b.input.device_nr) +
            "_" + i2s(b.input.stick_nr) + "_" + i2s(b.input.axis_nr);
    } default: {
        return "";
    }
    }
}


/* ----------------------------------------------------------------------------
 * Removes all registered control binds.
 */
void controls_mediator::clear_binds() {
    mgr.clear_bind();
}


/* ----------------------------------------------------------------------------
 * Removes all registered player action types.
 */
void controls_mediator::clear_player_action_types() {
    player_action_types.clear();
}


/* ----------------------------------------------------------------------------
 * Finds a registered control bind for player 1 that matches
 * the requested action. Returns an empty bind if none is found.
 * action_type_id:
 *   ID of the action type.
 */
control_bind controls_mediator::find_bind(
    const PLAYER_ACTION_TYPES action_type_id
) const {
    const vector<control_bind> &all_binds =
        mgr.get_all_binds();
    for(size_t b = 0; b < all_binds.size(); b++) {
        if(all_binds[b].action_type_id == action_type_id) {
            return all_binds[b];
        }
    }
    return control_bind();
}


/* ----------------------------------------------------------------------------
 * Finds a registered control bind for player 1 that matches
 * the requested action. Returns an empty bind if none is found.
 * action_name:
 *   Name of the action.
 */
control_bind controls_mediator::find_bind(
    const string &action_name
) const {
    for(size_t b = 0; b < player_action_types.size(); ++b) {
        if(player_action_types[b].internal_name == action_name) {
            return find_bind(player_action_types[b].id);
        }
    }
    return control_bind();
}


/* ----------------------------------------------------------------------------
 * Returns the current list of registered player action types.
 */
const vector<player_action_type>
&controls_mediator::get_all_player_action_types() const {
    return player_action_types;
}


/* ----------------------------------------------------------------------------
 * Returns all registered bind.
 */
const vector<control_bind> &controls_mediator::get_all_binds() const {
    return mgr.get_all_binds();
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event.
 * Returns true if the event was handled, false otherwise.
 */
bool controls_mediator::handle_allegro_event(const ALLEGRO_EVENT &ev) {
    switch(ev.type) {
    case ALLEGRO_EVENT_KEY_DOWN:
    case ALLEGRO_EVENT_KEY_UP: {
        mgr.handle_input(
            INPUT_TYPE_KEYBOARD_KEY,
            (ev.type == ALLEGRO_EVENT_KEY_DOWN) ? 1 : 0,
            0, ev.keyboard.keycode
        );
        return true;
        break;
    } case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
    case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
        mgr.handle_input(
            INPUT_TYPE_MOUSE_BUTTON,
            (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0,
            0, ev.mouse.button
        );
        return true;
        break;
    } case ALLEGRO_EVENT_MOUSE_AXES: {
        if(ev.mouse.dz > 0) {
            mgr.handle_input(
                INPUT_TYPE_MOUSE_WHEEL_UP,
                ev.mouse.dz
            );
        } else if(ev.mouse.dz < 0) {
            mgr.handle_input(
                INPUT_TYPE_MOUSE_WHEEL_DOWN,
                -ev.mouse.dz
            );
        } else if(ev.mouse.dw > 0) {
            mgr.handle_input(
                INPUT_TYPE_MOUSE_WHEEL_RIGHT,
                ev.mouse.dw
            );
        } else if(ev.mouse.dw < 0) {
            mgr.handle_input(
                INPUT_TYPE_MOUSE_WHEEL_LEFT,
                -ev.mouse.dw
            );
        }
        return true;
        break;
    } case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
    case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP: {
        mgr.handle_input(
            INPUT_TYPE_CONTROLLER_BUTTON,
            (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) ? 1 : 0,
            game.joystick_numbers[ev.joystick.id], ev.joystick.button
        );
        return true;
        break;
    } case ALLEGRO_EVENT_JOYSTICK_AXIS: {
        //Send inputs for both directions, so that, for instance, quickly
        //tilting the stick left will also send a "tilt right" input of 0.
        mgr.handle_input(
            INPUT_TYPE_CONTROLLER_AXIS_POS,
            std::max(ev.joystick.pos, 0.0f),
            game.joystick_numbers[ev.joystick.id],
            0,
            ev.joystick.stick,
            ev.joystick.axis
        );
        mgr.handle_input(
            INPUT_TYPE_CONTROLLER_AXIS_NEG,
            std::max(-ev.joystick.pos, 0.0f),
            game.joystick_numbers[ev.joystick.id],
            0,
            ev.joystick.stick,
            ev.joystick.axis
        );
        return true;
        break;
    }
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Begins a new frame of gameplay.
 */
void controls_mediator::new_frame() {
    mgr.new_frame();
}


/* ----------------------------------------------------------------------------
 * Returns the oldest action in the queue. Returns true if there is one,
 * false if not.
 * action:
 *   Action to fill.
 */
bool controls_mediator::read_action(player_action &action) {
    return mgr.read_action(action);
}


/* ----------------------------------------------------------------------------
 * Creates a control bind from a string representation.
 * Ignores the player number.
 * Returns a default control bind instance on error.
 * s:
 *   String to read from.
 */
control_bind controls_mediator::str_to_bind(
    const string &s
) const {
    control_bind bind;
    
    vector<string> parts = split(s, "_");
    size_t n_parts = parts.size();
    
    if(n_parts == 0) return bind;
    
    if(parts[0] == "k" && n_parts >= 2) {
        //Keyboard.
        bind.input.type = INPUT_TYPE_KEYBOARD_KEY;
        bind.input.button_nr = s2i(parts[1]);
        
    } else if(parts[0] == "mb" && n_parts >= 2) {
        //Mouse button.
        bind.input.type = INPUT_TYPE_MOUSE_BUTTON;
        bind.input.button_nr = s2i(parts[1]);
        
    } else if(parts[0] == "mwu") {
        //Mouse wheel up.
        bind.input.type = INPUT_TYPE_MOUSE_WHEEL_UP;
        
    } else if(parts[0] == "mwd") {
        //Mouse wheel down.
        bind.input.type = INPUT_TYPE_MOUSE_WHEEL_DOWN;
        
    } else if(parts[0] == "mwl") {
        //Mouse wheel left.
        bind.input.type = INPUT_TYPE_MOUSE_WHEEL_LEFT;
        
    } else if(parts[0] == "mwr") {
        //Mouse wheel right.
        bind.input.type = INPUT_TYPE_MOUSE_WHEEL_RIGHT;
        
    } else if(parts[0] == "jb" && n_parts >= 3) {
        //Controller button.
        bind.input.type = INPUT_TYPE_CONTROLLER_BUTTON;
        bind.input.device_nr = s2i(parts[1]);
        bind.input.button_nr = s2i(parts[2]);
        
    } else if(parts[0] == "jap" && n_parts >= 4) {
        //Controller stick axis, positive.
        bind.input.type = INPUT_TYPE_CONTROLLER_AXIS_POS;
        bind.input.device_nr = s2i(parts[1]);
        bind.input.stick_nr = s2i(parts[2]);
        bind.input.axis_nr = s2i(parts[3]);
        
    } else if(parts[0] == "jan" && n_parts >= 4) {
        //Controller stick axis, negative.
        bind.input.type = INPUT_TYPE_CONTROLLER_AXIS_NEG;
        bind.input.device_nr = s2i(parts[1]);
        bind.input.stick_nr = s2i(parts[2]);
        bind.input.axis_nr = s2i(parts[3]);
        
    } else {
        log_error(
            "Unrecognized control bind \"" + s + "\"!"
        );
    }
    
    return bind;
}
