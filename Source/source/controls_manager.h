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
};


/* ----------------------------------------------------------------------------
 * Contains information about the binding between a specific input,
 * and a player action type.
 */
struct control_binding {
    //Action type ID.
    int action_type_id;
    //Player number, starting at 0.
    int player_nr;
    //Type of input.
    INPUT_TYPES input_type;
    //Device number. i.e. the game controller number.
    int device_nr;
    //Button. Game controller button, keyboard key, mouse button, etc.
    int button_nr;
    //Game controller stick.
    int stick_nr;
    //Game controller axis.
    int axis_nr;
    
    control_binding();
};


/* ----------------------------------------------------------------------------
 * Defines an instance of a player action.
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
 * An input is data about some hardware signal. For instance, the fact
 * that a key was pressed along with its key code, or the fact that a game
 * controller's button was released, along with the button code and game
 * controller number.
 * The manager holds a list of control bindings, and when an input is received,
 * it scans through all bindings to figure out what actions should be
 * triggered.
 * It also has logic to do some cleanup like normalizing a game controller's
 * stick positions.
 */
struct controls_manager {
public:
    //Options.
    controls_manager_options options;
    
    void add_binding(const control_binding &binding);
    void clear_bindings();
    vector<control_binding> get_all_bindings() const;
    void handle_input(
        INPUT_TYPES type, float value,
        int device_nr = 0, int button_nr = 0, int stick_nr = 0, int axis_nr = 0
    );
    void new_frame();
    bool get_action(player_action &action);
    
private:
    //Queue of actions the game needs to handle.
    vector<player_action> action_queue;
    //Control bindings.
    vector<control_binding> bindings;
    //Each game action type's input state in the previous frame.
    map<int, float> old_action_type_states;
    //Each game action type's current input state.
    map<int, float> action_type_states;
    //Raw state of each game controller stick.
    map<int, map<int, map<int, float> > > raw_sticks;
    //Clean state of each game controller stick.
    map<int, map<int, map<int, float> > > clean_sticks;
    
    void clean_stick(int device_nr, int stick_nr);
    vector<int> get_action_types_from_input(
        INPUT_TYPES type,
        int device_nr = 0, int button_nr = 0, int stick_nr = 0, int axis_nr = 0
    );
};


#endif //ifndef CONTROL_MANAGER_INCLUDED
