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
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "controls.h"

#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "game_states/gameplay/gameplay.h"
#include "utils/general_utils.h"
#include "utils/string_utils.h"


/**
 * @brief Adds a new player action to the list.
 *
 * @param id Its ID.
 * @param category Its category.
 * @param name Its name.
 * @param description Its descripton.
 * @param internal_name The name of its property in the options file.
 * @param default_bind_str String representing of this action's default
 * control bind.
 */
void controls_mediator::add_player_action_type(
    const PLAYER_ACTION_TYPE id,
    const PLAYER_ACTION_CAT category,
    const string &name, const string &description, const string &internal_name,
    const string &default_bind_str
) {
    player_action_type a;
    a.id = id;
    a.category = category;
    a.name = name;
    a.description = description;
    a.internal_name = internal_name;
    a.default_bind_str = default_bind_str;
    
    player_action_types.push_back(a);
}


/**
 * @brief Returns the parsed input from an Allegro event.
 *
 * @param ev The Allegro event.
 * @return The input.
 * If this event does not pertain to any valid input, an input of type
 * INPUT_TYPE_NONE is returned.
 */
player_input controls_mediator::allegro_event_to_input(
    const ALLEGRO_EVENT &ev
) const {
    player_input input;
    
    switch(ev.type) {
    case ALLEGRO_EVENT_KEY_DOWN:
    case ALLEGRO_EVENT_KEY_UP: {
        input.type = INPUT_TYPE_KEYBOARD_KEY;
        input.button_nr = ev.keyboard.keycode;
        input.value = (ev.type == ALLEGRO_EVENT_KEY_DOWN) ? 1 : 0;
        break;
        
    } case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
    case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
        input.type = INPUT_TYPE_MOUSE_BUTTON;
        input.button_nr = ev.mouse.button;
        input.value = (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;
        break;
        
    } case ALLEGRO_EVENT_MOUSE_AXES: {
        if(ev.mouse.dz > 0) {
            input.type = INPUT_TYPE_MOUSE_WHEEL_UP;
            input.value = ev.mouse.dz;
        } else if(ev.mouse.dz < 0) {
            input.type = INPUT_TYPE_MOUSE_WHEEL_DOWN;
            input.value = -ev.mouse.dz;
        } else if(ev.mouse.dw > 0) {
            input.type = INPUT_TYPE_MOUSE_WHEEL_RIGHT;
            input.value = ev.mouse.dw;
        } else if(ev.mouse.dw < 0) {
            input.type = INPUT_TYPE_MOUSE_WHEEL_LEFT;
            input.value = -ev.mouse.dw;
        }
        break;
        
    } case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
    case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP: {
        input.type = INPUT_TYPE_CONTROLLER_BUTTON;
        input.device_nr = game.controller_numbers[ev.joystick.id];
        input.button_nr = ev.joystick.button;
        input.value = (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) ? 1 : 0;
        break;
        
    } case ALLEGRO_EVENT_JOYSTICK_AXIS: {
        if(ev.joystick.pos >= 0.0f) {
            input.type = INPUT_TYPE_CONTROLLER_AXIS_POS;
            input.value = ev.joystick.pos;
        } else {
            input.type = INPUT_TYPE_CONTROLLER_AXIS_NEG;
            input.value = -ev.joystick.pos;
        }
        input.device_nr = game.controller_numbers[ev.joystick.id];
        input.stick_nr = ev.joystick.stick;
        input.axis_nr = ev.joystick.axis;
        break;
    }
    }
    
    return input;
}


/**
 * @brief Returns the array of registered bind.
 *
 * @return The binds.
 */
vector<control_bind> &controls_mediator::binds() {
    return mgr.binds;
}


/**
 * @brief Finds a registered control bind for player 1 that matches
 * the requested action. Returns an empty bind if none is found.
 *
 * @param action_type_id ID of the action type.
 * @return The bind.
 */
control_bind controls_mediator::find_bind(
    const PLAYER_ACTION_TYPE action_type_id
) const {
    for(size_t b = 0; b < mgr.binds.size(); b++) {
        if(mgr.binds[b].action_type_id == action_type_id) {
            return mgr.binds[b];
        }
    }
    return control_bind();
}


/**
 * @brief Finds a registered control bind for player 1 that matches
 * the requested action. Returns an empty bind if none is found.
 *
 * @param action_name Name of the action.
 * @return The bind.
 */
control_bind controls_mediator::find_bind(
    const string &action_name
) const {
    for(size_t b = 0; b < player_action_types.size(); b++) {
        if(player_action_types[b].internal_name == action_name) {
            return find_bind(player_action_types[b].id);
        }
    }
    return control_bind();
}


/**
 * @brief Returns the current list of registered player action types.
 *
 * @return The types.
 */
const vector<player_action_type>
&controls_mediator::get_all_player_action_types() const {
    return player_action_types;
}


/**
 * @brief Returns a registered type, given its ID.
 *
 * @param action_id ID of the player action.
 * @return The type, or an empty type on failure.
 */
player_action_type controls_mediator::get_player_action_type(
    int action_id
) const {
    for(size_t b = 0; b < player_action_types.size(); b++) {
        if(player_action_types[b].id == action_id) {
            return player_action_types[b];
        }
    }
    return player_action_type();
}


/**
 * @brief Returns the internal name from an input ID,
 * used in the on_input_recieved event.
 *
 * @param action_id ID of the player action.
 * @return The name, or an empty string on failure.
 */
string controls_mediator::get_player_action_type_internal_name(
    int action_id
) {
    for(size_t b = 0; b < player_action_types.size(); b++) {
        if(player_action_types[b].id == action_id) {
            return player_action_types[b].internal_name;
        }
    }
    return "";
}


/**
 * @brief Returns the current input value of a given action type.
 *
 * @param player_action_type_id Action type to use.
 * @return The value.
 */
float controls_mediator::get_player_action_type_value(
    PLAYER_ACTION_TYPE player_action_type_id
) {
    return mgr.action_type_values[(int) player_action_type_id];
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The Allegro event.
 * @return Whether the event was handled.
 */
bool controls_mediator::handle_allegro_event(const ALLEGRO_EVENT &ev) {
    player_input input = allegro_event_to_input(ev);
    
    if(input.type != INPUT_TYPE_NONE) {
        mgr.handle_input(input);
        return true;
    } else {
        return false;
    }
}


/**
 * @brief Creates a string that represents an input.
 * Ignores the player number.
 *
 * @param i Input to read from.
 * @return The string, or an empty string on error.
 */
string controls_mediator::input_to_str(
    const player_input &i
) const {
    switch(i.type) {
    case INPUT_TYPE_KEYBOARD_KEY: {
        return "k_" + i2s(i.button_nr);
    } case INPUT_TYPE_MOUSE_BUTTON: {
        return "mb_" + i2s(i.button_nr);
    } case INPUT_TYPE_MOUSE_WHEEL_UP: {
        return "mwu";
    } case INPUT_TYPE_MOUSE_WHEEL_DOWN: {
        return "mwd";
    } case INPUT_TYPE_MOUSE_WHEEL_LEFT: {
        return "mwl";
    } case INPUT_TYPE_MOUSE_WHEEL_RIGHT: {
        return "mwr";
    } case INPUT_TYPE_CONTROLLER_BUTTON: {
        return "jb_" + i2s(i.device_nr) + "_" + i2s(i.button_nr);
    } case INPUT_TYPE_CONTROLLER_AXIS_POS: {
        return
            "jap_" + i2s(i.device_nr) +
            "_" + i2s(i.stick_nr) + "_" + i2s(i.axis_nr);
    } case INPUT_TYPE_CONTROLLER_AXIS_NEG: {
        return
            "jan_" + i2s(i.device_nr) +
            "_" + i2s(i.stick_nr) + "_" + i2s(i.axis_nr);
    } default: {
        return "";
    }
    }
}


/**
 * @brief Returns the player actions that occurred during the last frame
 * of gameplay, and begins a new frame.
 *
 * @return The player actions.
 */
vector<player_action> controls_mediator::new_frame() {
    return mgr.new_frame();
}


/**
 * @brief Releases all player inputs. Basically, set all of their values to 0.
 * Useful for when the game state is changed, or the window is out of focus.
 */
void controls_mediator::release_all() {
    for(auto &a : mgr.action_type_values) {
        a.second = 0.0f;
    }
}


/**
 * @brief Sets the options for the controls manager.
 *
 * @param options Options.
 */
void controls_mediator::set_options(const controls_manager_options &options) {
    mgr.options = options;
}


/**
 * @brief Creates an input from a string representation.
 * Ignores the player number.
 *
 * @param s String to read from.
 * @return The input, or a default input instance on error.
 */
player_input controls_mediator::str_to_input(
    const string &s
) const {
    player_input input;
    
    vector<string> parts = split(s, "_");
    size_t n_parts = parts.size();
    
    if(n_parts == 0) return input;
    
    if(parts[0] == "k" && n_parts >= 2) {
        //Keyboard.
        input.type = INPUT_TYPE_KEYBOARD_KEY;
        input.button_nr = s2i(parts[1]);
        
    } else if(parts[0] == "mb" && n_parts >= 2) {
        //Mouse button.
        input.type = INPUT_TYPE_MOUSE_BUTTON;
        input.button_nr = s2i(parts[1]);
        
    } else if(parts[0] == "mwu") {
        //Mouse wheel up.
        input.type = INPUT_TYPE_MOUSE_WHEEL_UP;
        
    } else if(parts[0] == "mwd") {
        //Mouse wheel down.
        input.type = INPUT_TYPE_MOUSE_WHEEL_DOWN;
        
    } else if(parts[0] == "mwl") {
        //Mouse wheel left.
        input.type = INPUT_TYPE_MOUSE_WHEEL_LEFT;
        
    } else if(parts[0] == "mwr") {
        //Mouse wheel right.
        input.type = INPUT_TYPE_MOUSE_WHEEL_RIGHT;
        
    } else if(parts[0] == "jb" && n_parts >= 3) {
        //Controller button.
        input.type = INPUT_TYPE_CONTROLLER_BUTTON;
        input.device_nr = s2i(parts[1]);
        input.button_nr = s2i(parts[2]);
        
    } else if(parts[0] == "jap" && n_parts >= 4) {
        //Controller stick axis, positive.
        input.type = INPUT_TYPE_CONTROLLER_AXIS_POS;
        input.device_nr = s2i(parts[1]);
        input.stick_nr = s2i(parts[2]);
        input.axis_nr = s2i(parts[3]);
        
    } else if(parts[0] == "jan" && n_parts >= 4) {
        //Controller stick axis, negative.
        input.type = INPUT_TYPE_CONTROLLER_AXIS_NEG;
        input.device_nr = s2i(parts[1]);
        input.stick_nr = s2i(parts[2]);
        input.axis_nr = s2i(parts[3]);
        
    } else {
        game.errors.report(
            "Unrecognized input \"" + s + "\"!"
        );
    }
    
    return input;
}


/**
 * @brief Processes a key press to check if it should do some "system" action,
 * like toggle the framerate, or activate a maker tool.
 *
 * @param keycode Allegro keycode of the pressed key.
 */
void gameplay_state::process_system_key_press(int keycode) {
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
        case MAKER_TOOL_TYPE_AREA_IMAGE: {
            ALLEGRO_BITMAP* bmp = draw_to_bitmap();
            string file_name =
                FOLDER_PATHS_FROM_ROOT::USER_DATA + "/Area_" +
                sanitize_file_name(game.cur_area_data->name) +
                "_" + get_current_time(true) + ".png";
                
            if(!al_save_bitmap(file_name.c_str(), bmp)) {
                game.errors.report(
                    "Could not save the area onto an image,"
                    " with the name \"" + file_name + "\"!"
                );
            }
            
            break;
            
        } case MAKER_TOOL_TYPE_CHANGE_SPEED: {
            game.maker_tools.change_speed =
                !game.maker_tools.change_speed;
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_TYPE_COLLISION: {
            game.maker_tools.collision =
                !game.maker_tools.collision;
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_TYPE_GEOMETRY_INFO: {
            game.maker_tools.geometry_info =
                !game.maker_tools.geometry_info;
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_TYPE_HITBOXES: {
            game.maker_tools.hitboxes =
                !game.maker_tools.hitboxes;
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_TYPE_HUD: {
            game.maker_tools.hud = !game.maker_tools.hud;
            break;
            
        } case MAKER_TOOL_TYPE_HURT_MOB: {
            mob* m = get_closest_mob_to_cursor();
            if(m) {
                m->set_health(
                    true, true, -game.maker_tools.mob_hurting_ratio
                );
            }
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_TYPE_MOB_INFO: {
            mob* m = get_closest_mob_to_cursor();
            mob* prev_lock_mob = game.maker_tools.info_lock;
            game.maker_tools.info_lock =
                (game.maker_tools.info_lock == m ? nullptr : m);
            if(prev_lock_mob != nullptr && game.maker_tools.info_lock == nullptr) {
                print_info("Mob: None.", 2.0f, 2.0f);
            }
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_TYPE_NEW_PIKMIN: {
            if(mobs.pikmin_list.size() < game.config.max_pikmin_in_field) {
                pikmin_type* new_pikmin_type =
                    game.content.mob_types.pikmin.begin()->second;
                    
                auto p = game.content.mob_types.pikmin.begin();
                for(; p != game.content.mob_types.pikmin.end(); ++p) {
                    if(p->second == game.maker_tools.last_pikmin_type) {
                        ++p;
                        if(p != game.content.mob_types.pikmin.end()) {
                            new_pikmin_type = p->second;
                        }
                        break;
                    }
                }
                game.maker_tools.last_pikmin_type = new_pikmin_type;
                
                create_mob(
                    game.mob_categories.get(MOB_CATEGORY_PIKMIN),
                    game.mouse_cursor.w_pos, new_pikmin_type, 0, "maturity=2"
                );
            }
            game.maker_tools.used_helping_tools = true;
            
            break;
            
        } case MAKER_TOOL_TYPE_PATH_INFO: {
            game.maker_tools.path_info = !game.maker_tools.path_info;
            game.maker_tools.used_helping_tools = true;
            break;
            
        } case MAKER_TOOL_TYPE_SET_SONG_POS_NEAR_LOOP: {
            game.audio.set_song_pos_near_loop();
            break;
            
        } case MAKER_TOOL_TYPE_TELEPORT: {
            sector* mouse_sector =
                get_sector(game.mouse_cursor.w_pos, nullptr, true);
            if(mouse_sector && cur_leader_ptr) {
                cur_leader_ptr->chase(
                    game.mouse_cursor.w_pos, mouse_sector->z,
                    CHASE_FLAG_TELEPORT
                );
                game.cam.set_pos(game.mouse_cursor.w_pos);
            }
            game.maker_tools.used_helping_tools = true;
            break;
            
        }
        }
        
    }
}
