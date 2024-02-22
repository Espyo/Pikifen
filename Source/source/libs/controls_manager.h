/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the control manager class and control manager related functions.
 */

#ifndef CONTROL_MANAGER_INCLUDED
#define CONTROL_MANAGER_INCLUDED

#include <map>
#include <string>
#include <tuple>
#include <vector>

using std::map;
using std::string;
using std::tuple;
using std::vector;


//Possible types of inputs.
enum INPUT_TYPES {
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


/* ----------------------------------------------------------------------------
 * Defines an instance of a specific input.
 */
struct player_input {
    //Type of input.
    INPUT_TYPES type;
    //Device number. i.e. the game controller number.
    int device_nr;
    //Button. Game controller button, keyboard key, mouse button, etc.
    int button_nr;
    //Game controller stick, if any.
    int stick_nr;
    //Game controller axis, if any.
    int axis_nr;
    //Value associated, if applicable.
    float value;
    
    player_input();
};


/* ----------------------------------------------------------------------------
 * Contains information about the bind between a specific input,
 * and a player action type.
 */
struct control_bind {
    //Action type ID.
    int action_type_id;
    //Player number, starting at 0.
    int player_nr;
    //Player input bound.
    player_input input;
    
    control_bind();
};


/* ----------------------------------------------------------------------------
 * Defines an instance of a specific player action.
 */
struct player_action {
    //Action type ID.
    int action_type_id;
    //Value associated. 0 to 1.
    float value;
    
    player_action();
};


/* ----------------------------------------------------------------------------
 * Info about a control manager's options.
 */
struct controls_manager_options {
    //Minimum deadzone for sticks. 0 for none.
    float stick_min_deadzone;
    //Maximum deadzone for sticks. 1 for none.
    float stick_max_deadzone;
    
    controls_manager_options();
};


/* ----------------------------------------------------------------------------
 * Manages the connections between inputs and player actions.
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
struct controls_manager {
public:
    //Control binds.
    vector<control_bind> binds;
    //Each game action type's current input value.
    map<int, float> action_type_values;
    //Options.
    controls_manager_options options;
    
    void handle_input(const player_input &input);
    vector<player_action> new_frame();
    
private:
    //Queue of actions the game needs to handle this frame.
    vector<player_action> action_queue;
    //Each game action type's input values in the previous frame.
    map<int, float> old_action_type_values;
    //Raw state of each game controller stick.
    map<int, map<int, map<int, float> > > raw_sticks;
    //Clean state of each game controller stick.
    map<int, map<int, map<int, float> > > clean_sticks;
    
    void clean_stick(const player_input& input);
    vector<int> get_action_types_from_input(
        const player_input &input
    );
    void handle_clean_input(const player_input &input, bool add_directly);
};


#endif //ifndef CONTROL_MANAGER_INCLUDED
