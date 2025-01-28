/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Controls manager class and controls manager related functions.
 */

#include <algorithm>
#include <cmath>

#include "controls_manager.h"

#include "../analog_stick_cleaner/analog_stick_cleaner.h"


/**
 * @brief When a game controller stick input is received, it should be checked
 * with the state of that entire stick to see if it needs to be normalized,
 * deadzones should be applied, etc.
 * The final cleaned stick positions can be found in the clean_sticks variable.
 *
 * @param input Input to clean.
 */
void controls_manager::clean_stick(const player_input &input) {
    raw_sticks[input.device_nr][input.stick_nr][input.axis_nr] =
        input.type == INPUT_TYPE_CONTROLLER_AXIS_POS ?
        input.value :
        -input.value;
        
    float coords[2];
    coords[0] = raw_sticks[input.device_nr][input.stick_nr][0];
    coords[1] = raw_sticks[input.device_nr][input.stick_nr][1];
    
    analog_stick_cleaner::settings_t cleanup_settings;
    cleanup_settings.deadzones.radial.inner = options.stick_min_deadzone;
    cleanup_settings.deadzones.radial.outer = options.stick_max_deadzone;
    analog_stick_cleaner::clean(coords, cleanup_settings);
    
    clean_sticks[input.device_nr][input.stick_nr][0] = coords[0];
    clean_sticks[input.device_nr][input.stick_nr][1] = coords[1];
}


/**
 * @brief Returns a list of action types that get triggered by the given input.
 *
 * @param input The input.
 * @return The action types.
 */
vector<int> controls_manager::get_action_types_from_input(
    const player_input &input
) {
    vector<int> action_types;
    
    for(size_t b = 0; b < binds.size(); b++) {
    
        const control_bind &bind = binds[b];
        
        if(bind.input.type != input.type) continue;
        
        switch(input.type) {
        case INPUT_TYPE_NONE:
        case INPUT_TYPE_UNKNOWN: {
            continue;
            break;
        } case INPUT_TYPE_KEYBOARD_KEY:
        case INPUT_TYPE_MOUSE_BUTTON: {
            if(
                bind.input.button_nr != input.button_nr
            ) {
                continue;
            }
            break;
        }
        case INPUT_TYPE_MOUSE_WHEEL_UP:
        case INPUT_TYPE_MOUSE_WHEEL_DOWN:
        case INPUT_TYPE_MOUSE_WHEEL_LEFT:
        case INPUT_TYPE_MOUSE_WHEEL_RIGHT: {
            break;
        }
        case INPUT_TYPE_CONTROLLER_BUTTON: {
            if(
                bind.input.device_nr != input.device_nr ||
                bind.input.button_nr != input.button_nr
            ) {
                continue;
            }
            break;
        }
        case INPUT_TYPE_CONTROLLER_AXIS_POS:
        case INPUT_TYPE_CONTROLLER_AXIS_NEG: {
            if(
                bind.input.device_nr != input.device_nr ||
                bind.input.stick_nr != input.stick_nr ||
                bind.input.axis_nr != input.axis_nr
            ) {
                continue;
            }
            break;
        }
        }
        
        action_types.push_back(bind.action_type_id);
        
    }
    
    return action_types;
}


/**
 * @brief Handles a final clean input.
 *
 * @param input Player input to process.
 * @param add_directly If true, the player actions bound to this input will
 * be added to the queue of actions directly.
 * If false, the manager will save the player actions' current state, and
 * only add the actions at the end of the frame, if their state is different
 * from the last frame's state.
 */
void controls_manager::handle_clean_input(
    const player_input &input, bool add_directly
) {
    //Find what game action types are bound to this input.
    vector<int> action_types = get_action_types_from_input(input);
    
    for(size_t a = 0; a < action_types.size(); a++) {
        if(add_directly) {
            //Add it to the action queue directly.
            player_action new_action;
            new_action.action_type_id = action_types[a];
            new_action.value = input.value;
            action_queue.push_back(new_action);
        } else {
            //Update each game action type's current input state,
            //so we can report them later.
            action_type_values[action_types[a]] = input.value;
        }
    }
}


/**
 * @brief Handles an input from hardware.
 *
 * @param input The input.
 */
void controls_manager::handle_input(
    const player_input &input
) {
    if(
        input.type == INPUT_TYPE_CONTROLLER_AXIS_POS ||
        input.type == INPUT_TYPE_CONTROLLER_AXIS_NEG
    ) {
        //Game controller stick inputs need to be cleaned up first,
        //by implementing deadzone logic.
        clean_stick(input);
        
        //We have to process both axes, so send two clean inputs.
        //But we also need to process imaginary tilts in the opposite direction.
        //If a player goes from walking left to walking right very quickly
        //in one frame, the "walking left" action may never receive a zero
        //value. So we should inject the zero manually with two more inputs.
        player_input x_pos_input = input;
        x_pos_input.type = INPUT_TYPE_CONTROLLER_AXIS_POS;
        x_pos_input.axis_nr = 0;
        x_pos_input.value =
            std::max(0.0f, clean_sticks[input.device_nr][input.stick_nr][0]);
        handle_clean_input(x_pos_input, false);
        
        player_input x_neg_input = input;
        x_neg_input.type = INPUT_TYPE_CONTROLLER_AXIS_NEG;
        x_neg_input.axis_nr = 0;
        x_neg_input.value =
            std::max(0.0f, -clean_sticks[input.device_nr][input.stick_nr][0]);
        handle_clean_input(x_neg_input, false);
        
        player_input y_pos_input = input;
        y_pos_input.type = INPUT_TYPE_CONTROLLER_AXIS_POS;
        y_pos_input.axis_nr = 1;
        y_pos_input.value =
            std::max(0.0f, clean_sticks[input.device_nr][input.stick_nr][1]);
        handle_clean_input(y_pos_input, false);
        
        player_input y_neg_input = input;
        y_neg_input.type = INPUT_TYPE_CONTROLLER_AXIS_NEG;
        y_neg_input.axis_nr = 1;
        y_neg_input.value =
            std::max(0.0f, -clean_sticks[input.device_nr][input.stick_nr][1]);
        handle_clean_input(y_neg_input, false);
        
    } else if(
        input.type == INPUT_TYPE_MOUSE_WHEEL_UP ||
        input.type == INPUT_TYPE_MOUSE_WHEEL_DOWN
    ) {
        //Mouse wheel inputs can have values over 1 to indicate the wheel
        //spun a lot. We should process each one as an individual input.
        //Plus, because mouse wheels have no physical state, the player
        //has no way of changing the value of a player action back to 0
        //using the mouse wheel. So whatever player actions we decide here
        //have to be added to this frame's action queue directly.
        for(unsigned int i = 0; i < input.value; i++) {
            player_input single_input = input;
            single_input.value = 1.0f;
            handle_clean_input(single_input, true);
        }
        
    } else {
        //Regular input.
        handle_clean_input(input, false);
        
    }
}


/**
 * @brief Returns the player actions that occurred during the last frame of
 * gameplay, and begins a new frame.
 *
 * @return The actions.
 */
vector<player_action> controls_manager::new_frame() {
    for(auto &a : action_type_values) {
        if(old_action_type_values[a.first] != a.second) {
            player_action new_action;
            new_action.action_type_id = a.first;
            new_action.value = a.second;
            action_queue.push_back(new_action);
        }
    }
    
    vector<player_action> result = action_queue;
    
    old_action_type_values = action_type_values;
    action_queue.clear();
    
    return result;
}
