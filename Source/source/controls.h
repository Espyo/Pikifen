/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the control-related classes and functions.
 * This refers to both hardware input receiving
 * and the corresponding in-game actions.
 */

#ifndef CONTROLS_INCLUDED
#define CONTROLS_INCLUDED

#include <functional>
#include <string>
#include <vector>

#include <allegro5/allegro.h>


using std::size_t;
using std::string;
using std::vector;


//List of "logical" buttons players can "press".
enum BUTTONS {
    //None.
    BUTTON_NONE,
    //Throw button.
    BUTTON_THROW,
    //Whistle button.
    BUTTON_WHISTLE,
    //Move right button.
    BUTTON_RIGHT,
    //Move up button.
    BUTTON_UP,
    //Move left button.
    BUTTON_LEFT,
    //Move down button.
    BUTTON_DOWN,
    //Move cursor right button.
    BUTTON_CURSOR_RIGHT,
    //Move cursor up button.
    BUTTON_CURSOR_UP,
    //Move cursor left button.
    BUTTON_CURSOR_LEFT,
    //Move cursor down button.
    BUTTON_CURSOR_DOWN,
    //Swarm group right button.
    BUTTON_GROUP_RIGHT,
    //Swarm group up button.
    BUTTON_GROUP_UP,
    //Swarm group left button.
    BUTTON_GROUP_LEFT,
    //Swarm group down button.
    BUTTON_GROUP_DOWN,
    //Swarm group towards cursor button.
    BUTTON_GROUP_CURSOR,
    //Swap to next leader button.
    BUTTON_NEXT_LEADER,
    //Swap to previous leader button.
    BUTTON_PREV_LEADER,
    //Dismiss button.
    BUTTON_DISMISS,
    //Use spray #1 button.
    BUTTON_USE_SPRAY_1,
    //Use spray #2 button.
    BUTTON_USE_SPRAY_2,
    //Use currently selected spray button.
    BUTTON_USE_SPRAY,
    //Swap to next spray button.
    BUTTON_NEXT_SPRAY,
    //Swap to previous spray button.
    BUTTON_PREV_SPRAY,
    //Change zoom level button.
    BUTTON_CHANGE_ZOOM,
    //Zoom in button.
    BUTTON_ZOOM_IN,
    //Zoom out button.
    BUTTON_ZOOM_OUT,
    //Swap to next standby type button.
    BUTTON_NEXT_TYPE,
    //Swap to previous standby type button.
    BUTTON_PREV_TYPE,
    //Swap to next standby type maturity button.
    BUTTON_NEXT_MATURITY,
    //Swap to previous standby type maturity button.
    BUTTON_PREV_MATURITY,
    //Lie down button.
    BUTTON_LIE_DOWN,
    //Pause button.
    BUTTON_PAUSE,
    //Menu navigation right button.
    BUTTON_MENU_RIGHT,
    //Menu navigation up button.
    BUTTON_MENU_UP,
    //Menu navigation left button.
    BUTTON_MENU_LEFT,
    //Menu navigation down button.
    BUTTON_MENU_DOWN,
    //Menu navigation OK button.
    BUTTON_MENU_OK,
    //Menu navigation back button.
    BUTTON_MENU_BACK,
    
    //Total amount of buttons.
    N_BUTTONS,
};


//Types of hardware controls.
enum CONTROL_TYPES {
    //None.
    CONTROL_TYPE_NONE,
    //Keyboard key.
    CONTROL_TYPE_KEYBOARD_KEY,
    //Mouse button.
    CONTROL_TYPE_MOUSE_BUTTON,
    //Mouse wheel scrolled up.
    CONTROL_TYPE_MOUSE_WHEEL_UP,
    //Mouse wheel scrolled down.
    CONTROL_TYPE_MOUSE_WHEEL_DOWN,
    //Mouse wheel scrolled left.
    CONTROL_TYPE_MOUSE_WHEEL_LEFT,
    //Mouse wheel scrolled right.
    CONTROL_TYPE_MOUSE_WHEEL_RIGHT,
    //Joystick button.
    CONTROL_TYPE_JOYSTICK_BUTTON,
    //Joystick axis tilted in a positive position.
    CONTROL_TYPE_JOYSTICK_AXIS_POS,
    //Joystick axis tilted in a negative position.
    CONTROL_TYPE_JOYSTICK_AXIS_NEG,
};



/* ----------------------------------------------------------------------------
 * This holds information over a user-specified
 * control. It has info over what hardware input
 * is required for this in-game control,
 * and what action it triggers.
 */
struct control_info {
    //Action number.
    BUTTONS action;
    //Type of control (hardware).
    CONTROL_TYPES type;
    //Device number. i.e. the gamepad number.
    int device_nr;
    //Button, whether the gamepad digital button, or the keyboard key.
    int button;
    //Stick on the gamepad.
    int stick;
    //Axis of the stick.
    int axis;
    
    control_info(BUTTONS action, const string &s);
    string stringify() const;
};


/* ----------------------------------------------------------------------------
 * Information about an action obtained from an Allegro event.
 */
struct action_from_event {
    //Button representing this action.
    BUTTONS button;
    //How far the button has been pressed, from 0 to 1.
    float pos;
    //Player that pressed this button.
    size_t player;
    
    action_from_event(
        const BUTTONS button, const float pos, const size_t player
    ) :
        button(button),
        pos(pos),
        player(player) { }
};


/* ----------------------------------------------------------------------------
 * Manager for the different gameplay "buttons", associated with the controls.
 */
struct button_manager {
    struct button {
        //ID of the button.
        BUTTONS id;
        //Name of the button.
        string name;
        //Its name in the options file.
        string internal_name;
        //String representing the default control to use for this button.
        string default_control_str;
    };
    
    //List of known buttons.
    vector<button> list;
    
    void add(
        const BUTTONS id, const string &name, const string &internal_name,
        const string &default_control_str
    );
};



control_info* find_control(const BUTTONS button_id);
control_info* find_control(const string &button_name);
vector<action_from_event> get_actions_from_event(const ALLEGRO_EVENT &ev);


#endif //ifndef CONTROLS_INCLUDED
