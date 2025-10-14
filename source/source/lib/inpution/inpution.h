/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Header for the Inpution middleware.
 * Please read the included readme file.
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


namespace Inpution {


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


//Possible flags for emitted actions.
enum ACTION_FLAG {

    //This action was issued as an auto-repeat.
    ACTION_FLAG_REPEAT = 1 << 0,
    
    //This action was reinserted into the queue.
    ACTION_FLAG_REINSERTED = 1 << 1,
    
};


//Possible types of value an action can have.
enum ACTION_VALUE_TYPE {

    //A float in the range [0 - 1].
    ACTION_VALUE_TYPE_ANALOG,
    
    //Either 0 or 1 (basically up or down).
    ACTION_VALUE_TYPE_DIGITAL,
    
};



/**
 * @brief Defines a specific interactable thing in the player's hardware,
 * like a specific button on a specific game controller, a specific key on
 * the keyboard, etc.
 */
struct InputSource {

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
    
    bool operator==(const InputSource& s2) const;
    bool operator<(const InputSource& s2) const;
    
};


/**
 * @brief Defines an instance of a specific input: a specific gesture made
 * by a human on a specific source of a specific piece of hardware.
 */
struct Input {

    //--- Members ---
    
    //Hardware source.
    InputSource source;
    
    //Value associated, if applicable.
    float value = 0.0f;
    
};


/**
 * @brief Defines a bind between a specific input source and
 * an action type. This is what's typically set in the game's options.
 */
struct Bind {

    //--- Members ---
    
    //Action type ID.
    int actionTypeId = 0;
    
    //Player number, starting at 1. 0 if N/A.
    int playerNr = 0;
    
    //Input source bound.
    InputSource inputSource;
    
};


/**
 * @brief Represents one of the game's possible actions.
 */
struct ActionType {

    //--- Members ---
    
    //Action type ID.
    int id = 0;
    
    //Type of value it can take. If digital but the input source is analog,
    //the value is rounded to 0 or 1 (i.e. depends on whether it's more than
    //or less than pressed halfway in).
    ACTION_VALUE_TYPE valueType = ACTION_VALUE_TYPE_ANALOG;
    
    //Auto-repeat. 0 if disabled, otherwise this indicates the threshold [0 - 1]
    //after which the input will start auto-repeating. The manager's
    //auto-repeating settings have to be configured for this to work.
    float autoRepeat = 0.0f;
    
    //If true, any input event that Inpution receives that's bound to this
    //action type will immediately add an event to the list that gets returned
    //by newFrame(). This can be useful, for instance, if you have two buttons
    //bound to the same action, and want either button's down press event
    //to trigger an action, like in a fast-paced rhythm game.
    //If false, input events feed an internal hardware state inside Inpution,
    //which is processed every frame to deliver the list in newFrame(). This
    //is useful for most cases, since if you have two inputs for the same
    //action, only the highest value of the two at any moment will be returned.
    //For example, if you have an analog stick and a D-pad bound to menu
    //focus movement.
    bool directEvents = false;
    
    //Actions can be told to return to the queue given by newFrame(). This is
    //useful as a buffer, where if the game can't handle that action, it can
    //throw it back into the queue to try to handle it in one of the next few
    //frames. For instance, the player pressed the kick button while mid-air,
    //close to the ground. The game can't perform a kick now, so it sends the
    //action back to the queue. In a few frames, the character lands, and the
    //action event can now be processed, even if it happened some frames after
    //the player really pressed the button.
    //This number controls the maximum time to live for a re-inserted action.
    //0 means it cannot be reinserted.
    float reinsertionTTL = 0.0f;
    
};


/**
 * @brief Defines an instance of a specific action. This is an abstract
 * gameplay activity, without any notion of hardware input. It's a pure
 * representation of what the player wants to do regardless of how they did it.
 */
struct Action {

    //--- Members ---
    
    //Action type ID.
    int actionTypeId = 0;
    
    //Player number, starting at 1. 0 if N/A.
    int playerNr = 0;
    
    //Value associated. [0 - 1].
    float value = 0.0f;
    
    //Flags. Use ACTION_FLAG.
    uint8_t flags = 0;
    
    //Queue reinsertion lifetime. See ActionType::insertionTTL.
    float reinsertionLifetime = 0.0f;
    
};


/**
 * @brief Info about a control manager's options.
 */
struct ManagerOptions {

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
 * @brief Manages the connections between inputs and actions.
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
 * analog stick positions.
 */
struct Manager {

    public:
    
    //--- Members ---
    
    //Map of all registered action types, using their IDs as the key.
    map<int, ActionType> actionTypes;
    
    //All registered control binds.
    vector<Bind> binds;
    
    //Are we ignoring actions right now?
    bool ignoringActions = false;
    
    //Options of the manager itself.
    ManagerOptions options;
    
    
    //--- Function declarations ---
    
    float getInputSourceValue(const Inpution::InputSource& source) const;
    float getValue(int actionTypeId) const;
    bool handleInput(const Input& input);
    vector<Action> newFrame(float deltaT);
    bool reinsertAction(const Action& action);
    bool releaseEverything();
    bool setGameState(const string& name = "");
    bool startIgnoringInputSource(const InputSource& inputSource, bool nowOnly);
    
    
    protected:
    
    //--- Structs ---
    
    /**
     * @brief Information about an action type's current status in a global way,
     * i.e. not in a specific game state.
     */
    struct ActionTypeGlobalStatus {
    
        //--- Members ---
        
        //Current value [0 - 1].
        float value = 0.0f;
        
    };
    
    
    /**
     * @brief Represents an action type's status in a given game state.
     */
    struct ActionTypeGameStateStatus {
    
        //Current value.
        float value = 0.0f;
        
        //How long it's been been active (!= 0) or inactive (== 0) for.
        float activationStateDuration = 0.0f;
        
        //How long until the next auto-repeat activation.
        float nextAutoRepeatActivation = 0.0f;
        
    };
    
    
    /**
     * @brief Represents one of the game's macro states.
     */
    struct GameState {
    
        //--- Members ---
        
        //Status of each action type in this game state.
        map<int, ActionTypeGameStateStatus> actionTypeStatuses;
        
    };
    
    
    /**
     * @brief Rule for ignoring a given input source.
     */
    struct IgnoreRule {
    
        //--- Members ---
        
        //The input source to ignore.
        InputSource source;
        
        //Whether it should only be ignored if it's active now, or if it can
        //wait until the next time it's active.
        bool nowOnly = true;
        
    };
    
    
    //--- Members ---
    
    //Global status of each action type.
    map<int, ActionTypeGlobalStatus> actionTypeGlobalStatuses;
    
    //Queue of actions the game needs to handle this frame.
    vector<Action> actionQueue;
    
    //Raw state of each game controller stick.
    map<int, map<int, map<int, float> > > rawSticks;
    
    //Clean state of each game controller stick.
    map<int, map<int, map<int, float> > > cleanSticks;
    
    //Values of each input source.
    map<InputSource, float> inputSourceValues;
    
    //Input sources currently being ignored.
    vector<IgnoreRule> ignoredInputSources;
    
    //Name of the current game state, or empty if none specified.
    string curGameStateName;
    
    //Game states.
    map<string, GameState> gameStates;
    
    //Last known time delta.
    float lastDeltaT = 0.0f;
    
    
    //--- Function declarations ---
    
    void cleanStick(const Input& input);
    float convertActionValue(int actionTypeId, float value) const;
    vector<int> getActionTypesFromInput(
        const Input& input
    );
    void handleCleanInput(const Input& input, bool forceDirectEvent);
    void processAutoRepeats(
        std::pair<const int, ActionTypeGameStateStatus>& it, float deltaT
    );
    bool processInputIgnoring(const Input& input);
    void processStateTimers(
        std::pair<const int, ActionTypeGameStateStatus>& it, float deltaT
    );
};


};
