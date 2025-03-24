/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Header for the controls manager class and related functions.
 *
 * This library manages the connections between inputs and player actions.
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
    int deviceNr = 0;
    
    //Button. Game controller button, keyboard key, mouse button, etc.
    int buttonNr = 0;
    
    //Game controller stick, if any.
    int stickNr = 0;
    
    //Game controller axis, if any.
    int axisNr = 0;
    
    //Value associated, if applicable.
    float value = 0.0f;
    
    
    //--- Function declarations ---
    
    bool isSameHardwareSourceAs(const PlayerInput &i2);
    
};


/**
 * @brief Contains information about the bind between a specific input,
 * and a player action type.
 */
struct ControlBind {

    //--- Members ---
    
    //Action type ID.
    int actionTypeId = 0;
    
    //Player number, starting at 0.
    int playerNr = 0;
    
    //Player input bound.
    PlayerInput input;
    
};


/**
 * @brief Defines an instance of a specific player action.
 */
struct PlayerAction {

    //--- Members ---
    
    //Action type ID.
    int actionTypeId = 0;
    
    //Value associated. 0 to 1.
    float value = 0.0f;
    
};


/**
 * @brief Info about a control manager's options.
 */
struct ControlsManagerOptions {

    //--- Members ---
    
    //Minimum deadzone for sticks. 0 for none.
    float stickMinDeadzone = 0.0f;
    
    //Maximum deadzone for sticks. 1 for none.
    float stickMaxDeadzone = 1.0f;
    
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
    map<int, float> actionTypeValues;
    
    //Options.
    ControlsManagerOptions options;
    
    
    //--- Function declarations ---
    
    void handleInput(const PlayerInput &input);
    void startIgnoringInput(const PlayerInput &input);
    vector<PlayerAction> newFrame();
    
    
    private:
    
    //--- Members ---
    
    //Queue of actions the game needs to handle this frame.
    vector<PlayerAction> actionQueue;
    
    //Each game action type's input values in the previous frame.
    map<int, float> oldActionTypeValues;
    
    //Raw state of each game controller stick.
    map<int, map<int, map<int, float> > > rawSticks;
    
    //Clean state of each game controller stick.
    map<int, map<int, map<int, float> > > cleanSticks;
    
    //Inputs currently being ignored.
    vector<PlayerInput> ignoredInputs;
    
    
    //--- Function declarations ---
    
    void cleanStick(const PlayerInput &input);
    vector<int> getActionTypesFromInput(
        const PlayerInput &input
    );
    void handleCleanInput(const PlayerInput &input, bool addDirectly);
    bool processInputIgnoring(const PlayerInput &input);
    
};
