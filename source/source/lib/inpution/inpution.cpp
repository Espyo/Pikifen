/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Source code for the Inpution middleware.
 * Please read the included readme file.
 */

#include <algorithm>
#include <cmath>

#include "inpution.h"

#include "../analog_stick_cleaner/analog_stick_cleaner.h"


namespace Inpution {


/**
 * @brief Returns whether a bind's requirement modifiers are being met.
 *
 * @param bind Bind to use.
 * @return Whether they are met.
 */
bool Manager::areBindRequirementsMet(const Bind& bind) const {
    if(!bind.requireModifiers) return true;
    
    for(const auto& m : modifiers) {
        bool modIsDown =
            getInputSourceValue(m.second) >= options.digitalThreshold;
        bool needsDown =
            std::find(
                bind.modifiers.begin(),
                bind.modifiers.end(),
                m.first
            ) != bind.modifiers.end();
            
        if(needsDown != modIsDown) return false;
    }
    
    return true;
}


/**
 * @brief When a game controller stick input is received, it should be checked
 * with the state of that entire stick to see if it needs to be normalized,
 * deadzones should be applied, etc.
 * The final cleaned stick positions can be found in the cleanSticks variable.
 *
 * @param input Input to clean.
 */
void Manager::cleanStick(const Input& input) {
    rawSticks[input.source.deviceNr]
    [input.source.stickNr][input.source.axisNr] =
        input.source.type == INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS ?
        input.value :
        -input.value;
        
    float coords[2];
    coords[0] = rawSticks[input.source.deviceNr][input.source.stickNr][0];
    coords[1] = rawSticks[input.source.deviceNr][input.source.stickNr][1];
    
    AnalogStickCleaner::Settings cleanupSettings;
    cleanupSettings.deadzones.radial.inner = options.stickMinDeadzone;
    cleanupSettings.deadzones.radial.outer = options.stickMaxDeadzone;
    AnalogStickCleaner::clean(coords, cleanupSettings);
    
    cleanSticks[input.source.deviceNr][input.source.stickNr][0] = coords[0];
    cleanSticks[input.source.deviceNr][input.source.stickNr][1] = coords[1];
}


/**
 * @brief Given an input value, converts it to an analog or boolean value,
 * according to the action type.
 *
 * @param actionTypeId ID of the action type.
 * @param value Value to convert.
 * @return The converted value.
 */
float Manager::convertActionValue(int actionTypeId, float value) const {
    auto it = actionTypes.find(actionTypeId);
    if(it != actionTypes.end()) {
        switch(it->second.valueType) {
        case ACTION_VALUE_TYPE_ANALOG: {
            return value;
            break;
        } case ACTION_VALUE_TYPE_DIGITAL: {
            return value >= options.digitalThreshold ? 1.0f : 0.0f;
            break;
        } case ACTION_VALUE_TYPE_1_ONLY: {
            return 1.0f;
            break;
        }
        }
    }
    
    return value;
}


/**
 * @brief Returns a list of action types that get triggered by the given input.
 *
 * @param input The input.
 * @return The action types.
 */
vector<int> Manager::getActionTypesFromInput(const Input& input) {
    vector<int> actionTypes;
    
    for(size_t b = 0; b < binds.size(); b++) {
        const Bind& bind = binds[b];
        if(!(bind.inputSource == input.source)) continue;
        if(!areBindRequirementsMet(bind)) continue;
        actionTypes.push_back(bind.actionTypeId);
    }
    
    return actionTypes;
}


/**
 * @brief Returns the current value of an input source.
 * Ignored input sources return 0.
 *
 * @param source The source.
 * @return The value, or 0 if not found.
 */
float Manager::getInputSourceValue(
    const Inpution::InputSource& source
) const {
    const auto it = inputSourceValues.find(source);
    if(it == inputSourceValues.end()) return 0.0f;
    for(size_t i = 0; i < ignoredInputSources.size(); i++) {
        if(ignoredInputSources[i].source == source) {
            //It's currently ignored.
            return 0.0f;
        }
    }
    return it->second;
}


/**
 * @brief Returns the current value of a given action type.
 *
 * @param actionTypeId ID of the action type.
 * @return The value, or 0 on failure.
 */
float Manager::getValue(int actionTypeId) const {
    float highestValue = 0.0f;
    for(const auto& bind : binds) {
        if(bind.actionTypeId != actionTypeId) continue;
        if(!areBindRequirementsMet(bind)) continue;
        float value = getInputSourceValue(bind.inputSource);
        highestValue = std::max(highestValue, value);
    }
    return convertActionValue(actionTypeId, highestValue);
}


/**
 * @brief Handles a final clean input.
 *
 * @param input Input to process.
 * @param forceDirectEvent If true, the actions bound to this input will
 * forcefully be added to the queue of actions directly.
 * If false, use the action type's discretion.
 */
void Manager::handleCleanInput(
    const Input& input, bool forceDirectEvent
) {
    if(!forceDirectEvent) {
        inputSourceValues[input.source] = input.value;
        
        if(processInputIgnoring(input)) {
            //We have to ignore this one.
            return;
        }
    }
    
    //Find what game action types are bound to this input.
    vector<int> actionTypesIds = getActionTypesFromInput(input);
    
    for(size_t a = 0; a < actionTypesIds.size(); a++) {
        const ActionType& actionType = actionTypes[actionTypesIds[a]];
        bool mustAddDirectly =
            forceDirectEvent ||
            actionType.directEvents ||
            actionType.valueType == ACTION_VALUE_TYPE_1_ONLY;
            
        if(mustAddDirectly) {
            //Add it to the action queue directly.
            Action newAction;
            newAction.actionTypeId = actionTypesIds[a];
            newAction.value =
                convertActionValue(actionTypesIds[a], input.value);
            newAction.flags |= ACTION_FLAG_DIRECT;
            newAction.reinsertionLifetime =
                actionTypes[actionTypesIds[a]].reinsertionTTL;
            actionQueue.push_back(newAction);
        }
    }
}


/**
 * @brief Handles a hardware input from the player.
 *
 * @param input The input.
 * @return Whether it succeeded.
 */
bool Manager::handleInput(const Input& input) {
    if(
        input.source.type == INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS ||
        input.source.type == INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG
    ) {
        //Game controller stick inputs need to be cleaned up first,
        //by implementing deadzone logic.
        cleanStick(input);
        
        //We have to process both axes, so send two clean inputs.
        //But we also need to process imaginary tilts in the opposite direction.
        //If a player goes from walking left to walking right very quickly
        //in one frame, the "walking left" action may never receive a zero
        //value. So we should inject the zero manually with two more inputs.
        Input xPosInput = input;
        xPosInput.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS;
        xPosInput.source.axisNr = 0;
        xPosInput.value =
            std::max(
                0.0f,
                cleanSticks[input.source.deviceNr][input.source.stickNr][0]
            );
        handleCleanInput(xPosInput, false);
        
        Input xNegInput = input;
        xNegInput.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG;
        xNegInput.source.axisNr = 0;
        xNegInput.value =
            std::max(
                0.0f,
                -cleanSticks[input.source.deviceNr][input.source.stickNr][0]
            );
        handleCleanInput(xNegInput, false);
        
        Input yPosInput = input;
        yPosInput.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS;
        yPosInput.source.axisNr = 1;
        yPosInput.value =
            std::max(
                0.0f,
                cleanSticks[input.source.deviceNr][input.source.stickNr][1]
            );
        handleCleanInput(yPosInput, false);
        
        Input yNegInput = input;
        yNegInput.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG;
        yNegInput.source.axisNr = 1;
        yNegInput.value =
            std::max(
                0.0f,
                -cleanSticks[input.source.deviceNr][input.source.stickNr][1]
            );
        handleCleanInput(yNegInput, false);
        
    } else if(
        input.source.type == INPUT_SOURCE_TYPE_CONTROLLER_ANALOG_BUTTON
    ) {
        //Game controller analog buttons have a value ranging from -1 to 1.
        //Let's normalize it and apply deadzone logic.
        Input cleanInput = input;
        cleanInput.value = (cleanInput.value + 1.0f) / 2.0f;
        AnalogStickCleaner::Settings cleanupSettings;
        cleanupSettings.deadzones.button.pressed =
            options.analogButtonMinDeadzone;
        cleanupSettings.deadzones.button.released =
            options.analogButtonMaxDeadzone;
        AnalogStickCleaner::cleanButton(&cleanInput.value, cleanupSettings);
        
        handleCleanInput(cleanInput, false);
        
    } else if(
        input.source.type == INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP ||
        input.source.type == INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN
    ) {
        //Mouse wheel inputs can have values over 1 to indicate the wheel
        //spun a lot. We should process each one as an individual input.
        //Plus, because mouse wheels have no physical state, the player
        //has no way of changing the value of a player action back to 0
        //using the mouse wheel. So whatever player actions we decide here
        //have to be added to this frame's action queue directly.
        for(unsigned int i = 0; i < input.value; i++) {
            Input singleInput = input;
            singleInput.value = 1.0f;
            handleCleanInput(singleInput, true);
        }
        
    } else if(
        input.source.type == INPUT_SOURCE_TYPE_KEYBOARD_CHAR
    ) {
        //Written characters are stateless. For instance, the user can insert
        //the a-with-a-tilde character, but can't "release" that character.
        handleCleanInput(input, true);
        
    } else {
        //Regular input.
        handleCleanInput(input, false);
        
    }
    
    return true;
}


/**
 * @brief Returns the actions that occurred during the last frame of
 * gameplay, and begins a new frame.
 *
 * @param deltaT How much time has passed since the last frame.
 * @return The actions.
 */
vector<Action> Manager::newFrame(float deltaT) {
    lastDeltaT = deltaT;
    auto& curGameState = gameStates[curGameStateName];
    
    //Get the current value of each action type.
    for(auto& a : actionTypes) {
        GameState& state = a.second.freezable ? curGameState : globalState;
        state.actionTypeStatuses[a.first].value = getValue(a.first);
    }
    
    //Add actions to the queue as needed.
    for(auto& a : actionTypes) {
        if(actionTypes[a.first].directEvents) {
            //Already added to the queue in handleCleanInput().
            continue;
        }
        GameState& state = a.second.freezable ? curGameState : globalState;
        if(
            state.actionTypeStatuses[a.first].oldValue !=
            state.actionTypeStatuses[a.first].value
        ) {
            Action newAction;
            newAction.actionTypeId = a.first;
            newAction.value = state.actionTypeStatuses[a.first].value;
            newAction.reinsertionLifetime = actionTypes[a.first].reinsertionTTL;
            actionQueue.push_back(newAction);
        }
    }
    
    //Process timers and auto-repeats.
    for(auto& a : actionTypes) {
        GameState& state = a.second.freezable ? curGameState : globalState;
        processTimers(
            state.actionTypeStatuses[a.first], a.second, deltaT
        );
        processAutoRepeats(
            state.actionTypeStatuses[a.first], a.first, a.second, deltaT
        );
    }
    
    vector<Action> result;
    if(!ignoringActions) {
        result = actionQueue;
    }
    
    //Clear any ignore rules that were meant to apply now only, but their
    //input isn't > 0, so they are no longer valid.
    for(size_t i = 0; i < ignoredInputSources.size();) {
        if(
            ignoredInputSources[i].nowOnly &&
            inputSourceValues[ignoredInputSources[i].source] == 0.0f
        ) {
            ignoredInputSources.erase(ignoredInputSources.begin() + i);
        } else {
            i++;
        }
    }
    
    //Prepare things for the next frame.
    for(auto& a : actionTypes) {
        GameState& state = a.second.freezable ? curGameState : globalState;
        state.actionTypeStatuses[a.first].oldValue =
            state.actionTypeStatuses[a.first].value;
    }
    actionQueue.clear();
    
    //Return the final list of actions.
    return result;
}


/**
 * @brief Processes logic for auto-repeating actions.
 *
 * @param status Status object of the action type.
 * @param actionTypeId ID of the action type.
 * @param actionType Data about the action type.
 * @param deltaT How much time has passed since the last frame.
 */
void Manager::processAutoRepeats(
    ActionTypeStatus& status, int actionTypeId,
    const ActionType& actionType, float deltaT
) {
    float actionTypeAutoRepeat = actionType.autoRepeat;
    if(actionTypeAutoRepeat == 0.0f) return;
    
    float autoRepeatFactor =
        (status.value - actionTypeAutoRepeat) /
        (1.0f - actionTypeAutoRepeat);
    if(autoRepeatFactor <= 0.0f) return;
    if(status.value == 0.0f) return;
    if(status.activationTimer == 0.0f) return;
    float oldDuration = status.activationTimer - deltaT;
    if(oldDuration >= status.nextAutoRepeatTimer) return;
    
    while(
        status.activationTimer >= status.nextAutoRepeatTimer
    ) {
        //Auto-repeat!
        Action newAction;
        newAction.actionTypeId = actionTypeId;
        newAction.value = status.value;
        newAction.flags |= ACTION_FLAG_REPEAT;
        newAction.reinsertionLifetime = actionType.reinsertionTTL;
        actionQueue.push_back(newAction);
        
        //Set the next activation.
        float currentFrequency =
            options.autoRepeatMaxInterval +
            (status.activationTimer / options.autoRepeatRampTime) *
            (options.autoRepeatMinInterval - options.autoRepeatMaxInterval);
        currentFrequency =
            std::max(options.autoRepeatMinInterval, currentFrequency);
        currentFrequency =
            std::min(currentFrequency, options.autoRepeatMaxInterval);
        status.nextAutoRepeatTimer += currentFrequency;
    }
}


/**
 * @brief Processes a received input, updates the list of ignored inputs if
 * necessary, and returns whether or not this one should be ignored.
 *
 * @param input Input to check.
 * @return Whether it should be ignored.
 */
bool Manager::processInputIgnoring(const Input& input) {
    for(size_t i = 0; i < ignoredInputSources.size(); i++) {
        if(ignoredInputSources[i].source == input.source) {
            if(input.value != 0.0f) {
                //We just ignore it and keep it on the list.
                return true;
            } else {
                //Remove it from the list since it's finally at 0,
                //and let the 0 go through.
                ignoredInputSources.erase(ignoredInputSources.begin() + i);
                return false;
            }
        }
    }
    
    return false;
}


/**
 * @brief Processes the timers for action types in a frame.
 *
 * @param status Status object of the action type.
 * @param actionType Data about the action type.
 * @param deltaT How much time has passed since the last frame.
 */
void Manager::processTimers(
    ActionTypeStatus& status, const ActionType& actionType, float deltaT
) {
    bool isActive = status.value != 0.0f;
    bool wasActive = status.oldValue != 0.0f;
    if(isActive != wasActive) {
        //Activation changed. Reset the timer.
        status.activationTimer = 0.0f;
        status.nextAutoRepeatTimer = options.autoRepeatMaxInterval;
    } else {
        //Same activation, increase the timer.
        status.activationTimer += deltaT;
    }
}


/**
 * @brief Reinserts an action event into the queue, so it can have a chance
 * at being processed again at a later frame. See ActionType::reinsertionTTL
 * for more information.
 *
 * @param action Action to reinsert.
 * @return Whether it succeeded.
 */
bool Manager::reinsertAction(const Action& action) {
    if(actionTypes[action.actionTypeId].reinsertionTTL <= 0.0f) return false;
    if(action.reinsertionLifetime <= 0.0f) return false;
    if(lastDeltaT == 0.0f) return false;
    
    Action newAction = action;
    newAction.reinsertionLifetime -= lastDeltaT;
    newAction.flags |= ACTION_FLAG_REINSERTED;
    actionQueue.push_back(newAction);
    return true;
}


/**
 * @brief Acts as if all buttons, keys, analog sticks, etc. have been released.
 *
 * @return Whether it succeeded.
 */
bool Manager::releaseEverything() {
    inputSourceValues.clear();
    return true;
}


/**
 * @brief Sets which game state to use from here on out, given its name. An
 * empty string is the default game state name when no game state is specified.
 * Changing to a different game state is useful when you want the previous
 * game state to not be aware of any action changes that are happening. A good
 * example is when you open the pause menu mid-gameplay. If the player was
 * holding B to charge the character's special move, paused, let go off B for
 * a second, held B again, and unpaused, tou probably don't want the regular
 * gameplay state to be aware of the special move 0 and special move 1
 * actions.
 * For menus, ideally you'd have some interval after the menu is closed in which
 * no actions are processed, so that the "menu close 1" input doesn't get
 * immediately consumed by normal gameplay.
 * Only action types that have the freezable property set to true will
 * be affected.
 *
 * @param name Name of the game state.
 * @return Whether it succeeded.
 */
bool Manager::setGameState(const string& name) {
    curGameStateName = name;
    return true;
}


/**
 * @brief Same as Manager::startIgnoringInputSource(), but applies to
 * all input sources of a given action.
 *
 * @param actionType Action type whose input sources to ignore.
 * @param nowOnly If true, only apply to inputs that are currently already
 * held down (> 0) this frame.
 * If false, keep the ignore rule up to the next time it's pressed down.
 * @return Whether it succeeded.
 */
bool Manager::startIgnoringActionInputSources(
    int actionType, bool nowOnly
) {
    bool result = false;
    for(size_t b = 0; b < binds.size(); b++) {
        Bind* bPtr = &binds[b];
        if(bPtr->actionTypeId != actionType) continue;
        result |= startIgnoringInputSource(bPtr->inputSource, nowOnly);
    }
    return result;
}


/**
 * @brief Ignores an input source from now on until the player performs the
 * input with value 0, at which point it becomes unignored.
 *
 * This is useful, for instance, when you want to detect the player's input
 * in the options menu's control binds screen, so you can assign the bind
 * but not have Inpution immediately right afterwards perform the action.
 * Also useful if you, for instance, have "unpause" and "attack" bound to the
 * same input, the player unpauses and immediately attacks.
 *
 * See also Manager::startIgnoringActionInputSources().
 *
 * @param inputSource Input source to ignore.
 * @param nowOnly If true, only apply to inputs that are currently already
 * held down (> 0) this frame.
 * If false, keep the ignore rule up to the next time it's pressed down.
 * @return Whether it succeeded.
 */
bool Manager::startIgnoringInputSource(
    const InputSource& inputSource, bool nowOnly
) {
    for(size_t i = 0; i < ignoredInputSources.size(); i++) {
        if(ignoredInputSources[i].source == inputSource) {
            //Already ignored.
            return false;
        }
    }
    ignoredInputSources.push_back({ .source = inputSource, .nowOnly = nowOnly});
    return true;
}


/**
 * @brief Returns whether two input sources are the same.
 *
 * @param s2 The other input source.
 * @return Whether they are the same.
 */
bool InputSource::operator==(const InputSource& s2) const {
    return
        type == s2.type &&
        deviceNr == s2.deviceNr &&
        buttonNr == s2.buttonNr &&
        stickNr == s2.stickNr &&
        axisNr == s2.axisNr;
}


/**
 * @brief Returns which input source should come first when sorting.
 *
 * @param s2 The other input source.
 * @return Whether the current should come before s2.
 */
bool InputSource::operator<(const InputSource& s2) const {
    if(type != s2.type) {
        return type < s2.type;
    }
    
    if(deviceNr != s2.deviceNr) {
        return deviceNr < s2.deviceNr;
    }
    
    if(buttonNr != s2.buttonNr) {
        return buttonNr < s2.buttonNr;
    }
    
    if(stickNr != s2.stickNr) {
        return stickNr < s2.stickNr;
    }
    
    return axisNr < s2.axisNr;
    
}


};
