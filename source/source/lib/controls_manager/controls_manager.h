/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Header for the controls manager class and related functions.
 *
 * This library manages the connections between inputs and player actions.
 */

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <vector>


using std::map;
using std::string;
using std::tuple;
using std::vector;


//Possible types of hardware sources for inputs.
enum INPUT_SOURCE_TYPE {

    //None.
    INPUT_SOURCE_TYPE_NONE,
    
    //Keyboard key.
    INPUT_SOURCE_TYPE_KEYBOARD_KEY,
    
    //Mouse button.
    INPUT_SOURCE_TYPE_MOUSE_BUTTON,
    
    //Mouse wheel scrolled up.
    INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP,
    
    //Mouse wheel scrolled down.
    INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN,
    
    //Mouse wheel scrolled left.
    INPUT_SOURCE_TYPE_MOUSE_WHEEL_LEFT,
    
    //Mouse wheel scrolled right.
    INPUT_SOURCE_TYPE_MOUSE_WHEEL_RIGHT,
    
    //Game controller button.
    INPUT_SOURCE_TYPE_CONTROLLER_BUTTON,
    
    //Game controller stick/D-pad axis tilted in a positive position.
    INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS,
    
    //Game controller stick/D-pad axis tilted in a negative position.
    INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG,
    
    //Some unknown type.
    INPUT_SOURCE_TYPE_UNKNOWN,
    
};


//Possible flags for emitted player actions.
enum PLAYER_ACTION_FLAG {

    //This action was issued as an auto-repeat.
    PLAYER_ACTION_FLAG_REPEAT = 1 << 0,
    
};


//Possible types of value a player action can have.
enum PLAYER_ACTION_VALUE_TYPE {

    //A float from 0 to 1.
    PLAYER_ACTION_VALUE_TYPE_ANALOG,

    //Either 0 or 1 (basically up or down).
    PLAYER_ACTION_VALUE_TYPE_BOOLEAN,

};



/**
 * @brief Defines a specific interactable thing in the player's hardware,
 * like a specific button on a specific game controller, a specific key on
 * the keyboard, etc.
 */
struct PlayerInputSource {

    //--- Members ---
    
    //Type of input source.
    INPUT_SOURCE_TYPE type = INPUT_SOURCE_TYPE_NONE;
    
    //Device number. i.e. the game controller number. 0 if N/A.
    int deviceNr = 0;
    
    //Game controller button, keyboard key, mouse button, etc. 0 if N/A.
    int buttonNr = 0;
    
    //Game controller stick. 0 if N/A.
    int stickNr = 0;
    
    //Game controller stick axis. 0 if N/A.
    int axisNr = 0;
    
    
    //--- Function declarations ---
    
    bool operator==(const PlayerInputSource& s2) const;
    
};


/**
 * @brief Defines an instance of a specific input: a specific gesture made
 * by a human on a specific source of a specific piece of hardware.
 */
struct PlayerInput {

    //--- Members ---
    
    //Hardware source.
    PlayerInputSource source;
    
    //Value associated, if applicable.
    float value = 0.0f;
    
};


/**
 * @brief Defines a bind between a specific input source and
 * a player action type. This is what's typically set in the game's options.
 */
struct ControlBind {

    //--- Members ---
    
    //Action type ID.
    int actionTypeId = 0;
    
    //Player number, starting at 1. 0 if N/A.
    int playerNr = 0;
    
    //Player input source bound.
    PlayerInputSource inputSource;
    
};


/**
 * @brief Represents one of the game's possible actions.
 */
struct PlayerActionType {

    //--- Members ---
    
    //Action type ID.
    int id = 0;

    //Type of value it can take.
    PLAYER_ACTION_VALUE_TYPE valueType = PLAYER_ACTION_VALUE_TYPE_ANALOG;
    
    //Auto-repeat. 0 if disabled, otherwise this indicates the threshold (0 - 1)
    //after which the input will start auto-repeating. The manager's
    //auto-repeating settings have to be configured for this to work.
    float autoRepeat = 0.0f;
    
};


/**
 * @brief Defines an instance of a specific player action. This is an abstract
 * gameplay activity, without any notion of hardware input. It's a pure
 * representation of what the player wants to do regardless of how they did it.
 */
struct PlayerAction {

    //--- Members ---
    
    //Action type ID.
    int actionTypeId = 0;
    
    //Player number, starting at 1. 0 if N/A.
    int playerNr = 0;
    
    //Value associated. 0 to 1.
    float value = 0.0f;
    
    //Flags. Use PLAYER_ACTION_FLAG.
    uint8_t flags = 0;
    
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
    
    //Interval between auto-repeat activations, at the slowest speed.
    float autoRepeatMaxInterval = 0.3f;
    
    //Interval between auto-repeat activations, at the fastest speed.
    float autoRepeatMinInterval = 0.05f;
    
    //How long it takes for the auto-repeat activations to reach max speed.
    float autoRepeatRampTime = 0.9f;
    
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

    private:
    
    //--- Structs ---
    
    /**
     * @brief Information about a player action type's current status.
     */
    struct ActionTypeStatus {
    
        //--- Members ---
        
        //Current value (0 - 1).
        float value = 0.0f;
        
        //Value in the previous frame.
        float oldValue = 0.0f;
        
        //How long it's been been active (!= 0) or inactive (== 0) for.
        float stateDuration = 0.0f;
        
        //How long until the next auto-repeat activation.
        float nextAutoRepeatActivation = 0.0f;
        
    };
    
    
public:

    //--- Members ---
    
    //Map of all registered player action types, using their IDs as the key.
    map<int, PlayerActionType> actionTypes;
    
    //All registered control binds.
    vector<ControlBind> binds;
    
    //Are we ignoring player actions right now?
    bool ignoringActions = false;
    
    //Options of the manager itself.
    ControlsManagerOptions options;
    
    
    //--- Function declarations ---
    
    float getValue(int playerActionTypeId) const;
    void handleInput(const PlayerInput& input);
    void startIgnoringInputSource(const PlayerInputSource& inputSource);
    vector<PlayerAction> newFrame(float deltaT);
    void setValue(int playerActionTypeId, float value);
    
    
private:

    //--- Members ---
    
    //Status of each player action type.
    map<int, ActionTypeStatus> actionTypeStatuses;
    
    //Queue of actions the game needs to handle this frame.
    vector<PlayerAction> actionQueue;
    
    //Raw state of each game controller stick.
    map<int, map<int, map<int, float> > > rawSticks;
    
    //Clean state of each game controller stick.
    map<int, map<int, map<int, float> > > cleanSticks;
    
    //Input sources currently being ignored.
    vector<PlayerInputSource> ignoredInputSources;
    
    
    //--- Function declarations ---
    
    void cleanStick(const PlayerInput& input);
    float convertActionValue(int actionTypeId, float value);
    vector<int> getActionTypesFromInput(
        const PlayerInput& input
    );
    void handleCleanInput(const PlayerInput& input, bool addDirectly);
    void processAutoRepeats(
        std::pair<const int, ActionTypeStatus>& it, float deltaT
    );
    bool processInputIgnoring(const PlayerInput& input);
    void processStateTimers(
        std::pair<const int, ActionTypeStatus>& it, float deltaT
    );
};
