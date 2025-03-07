/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the control manager class and control manager related functions.
 */

#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

using std::map;
using std::string;
using std::tuple;
using std::vector;


//Possible types of inputs.
enum INPUT_TYPE {

    //None.
    INPUT_TYPE_NONE,
    
    //Keyboard key.
    INPUT_TYPE_KEYBOARD_KEY,
    
    //Mouse button.
    INPUT_TYPE_MOUSE_BUTTON,
    
    //Mouse wheel scrolled up.
    INPUT_TYPE_MOUSE_WHEEL_UP,
    
    //Mouse wheel scrolled down.
    INPUT_TYPE_MOUSE_WHEEL_DOWN,
    
    //Mouse wheel scrolled left.
    INPUT_TYPE_MOUSE_WHEEL_LEFT,
    
    //Mouse wheel scrolled right.
    INPUT_TYPE_MOUSE_WHEEL_RIGHT,
    
    //Game controller button.
    INPUT_TYPE_CONTROLLER_BUTTON,
    
    //Game controller stick/D-pad axis tilted in a positive position.
    INPUT_TYPE_CONTROLLER_AXIS_POS,
    
    //Game controller stick/D-pad axis tilted in a negative position.
    INPUT_TYPE_CONTROLLER_AXIS_NEG,
    
    //Some unknown type.
    INPUT_TYPE_UNKNOWN,
    
};


/**
 * @brief Defines an instance of a specific input.
 */
struct PlayerInput {

    //--- Members ---
    
    //Type of input.
    INPUT_TYPE type = INPUT_TYPE_NONE;
    
    //Device number. i.e. the game controller number.
    int device_nr = 0;
    
    //Button. Game controller button, keyboard key, mouse button, etc.
    int button_nr = 0;
    
    //Game controller stick, if any.
    int stick_nr = 0;
    
    //Game controller axis, if any.
    int axis_nr = 0;
    
    //Value associated, if applicable.
    float value = 0.0f;
    
};


/**
 * @brief Contains information about the bind between a specific input,
 * and a player action type.
 */
struct ControlBind {

    //--- Members ---
    
    //Action type ID.
    int action_type_id = 0;
    
    //Player number, starting at 0.
    int player_nr = 0;
    
    //Player input bound.
    PlayerInput input;
    
};


/**
 * @brief Defines an instance of a specific player action.
 */
struct PlayerAction {

    //--- Members ---
    
    //Action type ID.
    int action_type_id = 0;
    
    //Value associated. 0 to 1.
    float value = 0.0f;
    
};


/**
 * @brief Info about a control manager's options.
 */
struct ControlsManagerOptions {

    //--- Members ---
    
    //Minimum deadzone for sticks. 0 for none.
    float stick_min_deadzone = 0.0f;
    
    //Maximum deadzone for sticks. 1 for none.
    float stick_max_deadzone = 1.0f;
    
};


/**
 * @brief Manages the connections between inputs and player actions.
 *
 * The idea of this class is to be game-agnostic.
 * An input is data about some hardware signal. For instance, the fact
 * that a key was pressed along with its key code, or the fact that a game
 * controller's button was released, along with the button code and game
 * controller number.
 * The manager holds a list of control binds, and when an input is received,
 * it scans through all binds to figure out what actions should be
 * triggered.
 * It also has logic to do some cleanup like normalizing a game controller's
 * stick positions.
 */
struct ControlsManager {

    public:
    
    //--- Members ---
    
    //Control binds.
    vector<ControlBind> binds;
    
    //Each game action type's current input value.
    map<int, float> action_type_values;
    
    //Options.
    ControlsManagerOptions options;
    
    
    //--- Function declarations ---
    
    void handle_input(const PlayerInput &input);
    vector<PlayerAction> new_frame();
    
    private:
    
    //--- Members ---
    
    //Queue of actions the game needs to handle this frame.
    vector<PlayerAction> action_queue;
    
    //Each game action type's input values in the previous frame.
    map<int, float> old_action_type_values;
    
    //Raw state of each game controller stick.
    map<int, map<int, map<int, float> > > raw_sticks;
    
    //Clean state of each game controller stick.
    map<int, map<int, map<int, float> > > clean_sticks;
    
    
    //--- Function declarations ---
    
    void clean_stick(const PlayerInput &input);
    vector<int> get_action_types_from_input(
        const PlayerInput &input
    );
    void handle_clean_input(const PlayerInput &input, bool add_directly);
    
};
