/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Controls manager class and related functions.
 *
 * Please read the header file for more information.
 */

#include <algorithm>
#include <cmath>

#include "controls_manager.h"

#include "../analog_stick_cleaner/analog_stick_cleaner.h"


/**
 * @brief When a game controller stick input is received, it should be checked
 * with the state of that entire stick to see if it needs to be normalized,
 * deadzones should be applied, etc.
 * The final cleaned stick positions can be found in the cleanSticks variable.
 *
 * @param input Input to clean.
 */
void ControlsManager::cleanStick(const PlayerInput &input) {
    rawSticks[input.source.deviceNr][input.source.stickNr][input.source.axisNr] =
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
 * @brief Returns a list of action types that get triggered by the given input.
 *
 * @param input The input.
 * @return The action types.
 */
vector<int> ControlsManager::getActionTypesFromInput(
    const PlayerInput &input
) {
    vector<int> actionTypes;
    
    for(size_t b = 0; b < binds.size(); b++) {
        const ControlBind &bind = binds[b];
        if(bind.inputSource == input.source) {
            actionTypes.push_back(bind.actionTypeId);
        }
    }
    
    return actionTypes;
}


/**
 * @brief Handles a final clean input.
 *
 * @param input Player input to process.
 * @param addDirectly If true, the player actions bound to this input will
 * be added to the queue of actions directly.
 * If false, the manager will save the player actions' current state, and
 * only add the actions at the end of the frame, if their state is different
 * from the last frame's state.
 */
void ControlsManager::handleCleanInput(
    const PlayerInput &input, bool addDirectly
) {
    if(processInputIgnoring(input)) {
        //We have to ignore this one.
        return;
    }
    
    //Find what game action types are bound to this input.
    vector<int> actionTypes = getActionTypesFromInput(input);
    
    for(size_t a = 0; a < actionTypes.size(); a++) {
        if(addDirectly) {
            //Add it to the action queue directly.
            PlayerAction newAction;
            newAction.actionTypeId = actionTypes[a];
            newAction.value = input.value;
            actionQueue.push_back(newAction);
        } else {
            //Update each game action type's current input state,
            //so we can report them later.
            actionTypeStatuses[actionTypes[a]].value = input.value;
        }
    }
}


/**
 * @brief Returns the current value of a given player action type.
 *
 * @param playerActionTypeId ID of the player action type.
 * @return The value, or 0 on failure.
 */
float ControlsManager::getValue(int playerActionTypeId) const {
    auto it = actionTypeStatuses.find(playerActionTypeId);
    if(it == actionTypeStatuses.end()) return 0.0f;
    return it->second.value;
}


/**
 * @brief Handles an input from the player.
 *
 * @param input The input.
 */
void ControlsManager::handleInput(
    const PlayerInput &input
) {
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
        PlayerInput xPosInput = input;
        xPosInput.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS;
        xPosInput.source.axisNr = 0;
        xPosInput.value =
            std::max(0.0f, cleanSticks[input.source.deviceNr][input.source.stickNr][0]);
        handleCleanInput(xPosInput, false);
        
        PlayerInput xNegInput = input;
        xNegInput.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG;
        xNegInput.source.axisNr = 0;
        xNegInput.value =
            std::max(0.0f, -cleanSticks[input.source.deviceNr][input.source.stickNr][0]);
        handleCleanInput(xNegInput, false);
        
        PlayerInput yPosInput = input;
        yPosInput.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS;
        yPosInput.source.axisNr = 1;
        yPosInput.value =
            std::max(0.0f, cleanSticks[input.source.deviceNr][input.source.stickNr][1]);
        handleCleanInput(yPosInput, false);
        
        PlayerInput yNegInput = input;
        yNegInput.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG;
        yNegInput.source.axisNr = 1;
        yNegInput.value =
            std::max(0.0f, -cleanSticks[input.source.deviceNr][input.source.stickNr][1]);
        handleCleanInput(yNegInput, false);
        
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
            PlayerInput singleInput = input;
            singleInput.value = 1.0f;
            handleCleanInput(singleInput, true);
        }
        
    } else {
        //Regular input.
        handleCleanInput(input, false);
        
    }
}


/**
 * @brief Returns the player actions that occurred during the last frame of
 * gameplay, and begins a new frame.
 *
 * @param deltaT How much time has passed since the last frame.
 * @return The actions.
 */
vector<PlayerAction> ControlsManager::newFrame(float deltaT) {
    for(auto &a : actionTypeStatuses) {
        if(a.second.oldValue != a.second.value) {
            PlayerAction newAction;
            newAction.actionTypeId = a.first;
            newAction.value = a.second.value;
            actionQueue.push_back(newAction);
        }
    }
    
    for(auto &a : actionTypeStatuses) {
        processStateTimers(a, deltaT);
        processAutoRepeats(a, deltaT);
    }
    
    vector<PlayerAction> result;
    if(!ignoringActions) {
        result = actionQueue;
    }
    
    //Prepare things for the next frame.
    for(auto &a : actionTypeStatuses) {
        a.second.oldValue = a.second.value;
    }
    actionQueue.clear();
    
    return result;
}


/**
 * @brief Processes logic for auto-repeating player actions.
 *
 * @param it Iterator of the map of action type statuses.
 * @param deltaT How much time has passed since the last frame.
 */
void ControlsManager::processAutoRepeats(
    std::pair<const int, ActionTypeStatus> &it, float deltaT
) {
    float actionTypeAutoRepeat = actionTypes[it.first].autoRepeat;
    if(actionTypeAutoRepeat == 0.0f) return;
    float autoRepeatFactor =
        (it.second.value - actionTypeAutoRepeat) /
        (1.0f - actionTypeAutoRepeat);
    if(autoRepeatFactor <= 0.0f) return;
    if(it.second.value == 0.0f) return;
    if(it.second.stateDuration == 0.0f) return;
    float oldDuration = it.second.stateDuration - deltaT;
    if(oldDuration >= it.second.nextAutoRepeatActivation) return;
    
    while(it.second.stateDuration >= it.second.nextAutoRepeatActivation) {
        //Auto-repeat!
        PlayerAction newAction;
        newAction.actionTypeId = it.first;
        newAction.value = it.second.value;
        newAction.flags |= PLAYER_ACTION_FLAG_REPEAT;
        actionQueue.push_back(newAction);
        
        //Set the next activation.
        float currentFrequency =
            options.autoRepeatMaxInterval +
            (it.second.stateDuration / options.autoRepeatRampTime) *
            (options.autoRepeatMinInterval - options.autoRepeatMaxInterval);
        currentFrequency =
            std::max(options.autoRepeatMinInterval, currentFrequency);
        currentFrequency =
            std::min(currentFrequency, options.autoRepeatMaxInterval);
        it.second.nextAutoRepeatActivation += currentFrequency;
    }
}


/**
 * @brief Processes a received input, updates the list of ignored inputs if
 * necessary, and returns whether or not this one should be ignored.
 *
 * @param input Input to check.
 * @return Whether it should be ignored.
 */
bool ControlsManager::processInputIgnoring(
    const PlayerInput &input
) {
    for(size_t i = 0; i < ignoredInputSources.size(); i++) {
        if(ignoredInputSources[i] == input.source) {
            if(input.value != 0.0f) {
                //We just ignore it and keep it on the list.
                return true;
            } else {
                //Remove it from the list since it's finally at 0,
                //but still ignore it this time.
                ignoredInputSources.erase(ignoredInputSources.begin() + i);
                return true;
            }
        }
    }
    
    return false;
}


/**
 * @brief Processes the timers for action type states in a frame.
 *
 * @param it Iterator of the map of action type statuses.
 * @param deltaT How much time has passed since the last frame.
 */
void ControlsManager::processStateTimers(
    std::pair<const int, ActionTypeStatus> &it, float deltaT
) {
    bool isActive = it.second.value != 0.0f;
    bool wasActive = it.second.oldValue != 0.0f;
    if(isActive != wasActive) {
        //State changed. Reset the timer.
        it.second.stateDuration = 0.0f;
        it.second.nextAutoRepeatActivation = options.autoRepeatMaxInterval;
    } else {
        //Same state, increase the timer.
        it.second.stateDuration += deltaT;
    }
}


/**
 * @brief Sets the current value of a given player action type.
 *
 * @param playerActionTypeId ID of the player action type.
 * @param The value.
 */
void ControlsManager::setValue(int playerActionTypeId, float value) {
    value = std::min(value, 1.0f);
    value = std::max(0.0f, value);
    actionTypeStatuses[playerActionTypeId].value = value;
}


/**
 * @brief Ignores an input source from now on until the player performs the
 * input with value 0, at which point it becomes unignored.
 *
 * @param inputSource Input source to ignore.
 */
void ControlsManager::startIgnoringInputSource(
    const PlayerInputSource &inputSource
) {
    for(size_t i = 0; i < ignoredInputSources.size(); i++) {
        if(ignoredInputSources[i] == inputSource) {
            //Already ignored.
            return;
        }
    }
    ignoredInputSources.push_back(inputSource);
}


/**
 * @brief Returns whether two input sources are the same.
 *
 * @param s2 The other input source.
 * @return Whether they are the same.
 */
bool PlayerInputSource::operator==(const PlayerInputSource &s2) const {
    return
        type == s2.type &&
        deviceNr == s2.deviceNr &&
        buttonNr == s2.buttonNr &&
        stickNr == s2.stickNr &&
        axisNr == s2.axisNr;
}
