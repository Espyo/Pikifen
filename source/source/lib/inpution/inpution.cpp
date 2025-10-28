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
        bool modIsDown = getInputSourceValue(m.second) > 0.5f;
        bool needsDown =
            std::find(
                bind.modifiers.begin(),
                bind.modifiers.end(),
                m.first
            ) != bind.modifiers.end();
        if(needsDown != modIsDown) {
            return false;
        }
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
            return value >= 0.5f ? 1.0f : 0.0f;
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
 *
 * @param source The source.
 * @return The value, or 0 if not found.
 */
float Manager::getInputSourceValue(
    const Inpution::InputSource& source
) const {
    const auto it = inputSourceValues.find(source);
    if(it == inputSourceValues.end()) return 0.0f;
    return inputSourceValues.at(source);
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
    if(processInputIgnoring(input)) {
        //We have to ignore this one.
        return;
    }
    
    if(!forceDirectEvent) {
        inputSourceValues[input.source] = input.value;
    }
    
    //Find what game action types are bound to this input.
    vector<int> actionTypesIds = getActionTypesFromInput(input);
    
    for(size_t a = 0; a < actionTypesIds.size(); a++) {
        if(
            !actionTypes[actionTypesIds[a]].directEvents &&
            !forceDirectEvent
        ) {
            continue;
        }
        //Add it to the action queue directly.
        Action newAction;
        newAction.actionTypeId = actionTypesIds[a];
        newAction.value = convertActionValue(actionTypesIds[a], input.value);
        newAction.reinsertionLifetime =
            actionTypes[actionTypesIds[a]].reinsertionTTL;
        actionQueue.push_back(newAction);
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
    
    for(auto& a : actionTypes) {
        actionTypeGlobalStatuses[a.first].value = getValue(a.first);
    }
    
    for(auto& a : actionTypeGlobalStatuses) {
        if(actionTypes[a.first].directEvents) {
            //Already added to the queue in handleCleanInput().
            continue;
        }
        float oldValue;
        if(actionTypes[a.first].freezable) {
            oldValue = curGameState.actionTypeStatuses[a.first].value;
        } else {
            oldValue = actionTypeGlobalStatuses[a.first].oldValue;
        }
        if(oldValue != a.second.value) {
            Action newAction;
            newAction.actionTypeId = a.first;
            newAction.value = a.second.value;
            newAction.reinsertionLifetime = actionTypes[a.first].reinsertionTTL;
            actionQueue.push_back(newAction);
        }
    }
    
    for(auto& a : curGameState.actionTypeStatuses) {
        processStateTimers(a, deltaT);
        processAutoRepeats(a, deltaT);
    }
    
    vector<Action> result;
    if(!ignoringActions) {
        result = actionQueue;
    }
    
    //Clear any ignore rules that were meant to apply now only, but their
    //input isn't > 0, so they no longer valid.
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
    for(auto& a : actionTypeGlobalStatuses) {
        curGameState.actionTypeStatuses[a.first].value = a.second.value;
    }
    for(auto& a : actionTypeGlobalStatuses) {
        a.second.oldValue = a.second.value;
    }
    actionQueue.clear();
    
    return result;
}


/**
 * @brief Processes logic for auto-repeating actions.
 *
 * @param it Iterator of the map of action type statuses.
 * @param deltaT How much time has passed since the last frame.
 */
void Manager::processAutoRepeats(
    std::pair<const int, ActionTypeGameStateStatus>& it, float deltaT
) {
    float actionTypeAutoRepeat = actionTypes[it.first].autoRepeat;
    if(actionTypeAutoRepeat == 0.0f) return;
    float autoRepeatFactor =
        (it.second.value - actionTypeAutoRepeat) /
        (1.0f - actionTypeAutoRepeat);
    if(autoRepeatFactor <= 0.0f) return;
    if(it.second.value == 0.0f) return;
    if(it.second.activationStateDuration == 0.0f) return;
    float oldDuration = it.second.activationStateDuration - deltaT;
    if(oldDuration >= it.second.nextAutoRepeatActivation) return;
    
    while(
        it.second.activationStateDuration >= it.second.nextAutoRepeatActivation
    ) {
        //Auto-repeat!
        Action newAction;
        newAction.actionTypeId = it.first;
        newAction.value = it.second.value;
        newAction.flags |= ACTION_FLAG_REPEAT;
        newAction.reinsertionLifetime = actionTypes[it.first].reinsertionTTL;
        actionQueue.push_back(newAction);
        
        //Set the next activation.
        float currentFrequency =
            options.autoRepeatMaxInterval +
            (it.second.activationStateDuration / options.autoRepeatRampTime) *
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
bool Manager::processInputIgnoring(const Input& input) {
    for(size_t i = 0; i < ignoredInputSources.size(); i++) {
        if(ignoredInputSources[i].source == input.source) {
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
void Manager::processStateTimers(
    std::pair<const int, ActionTypeGameStateStatus>& it, float deltaT
) {
    bool isActive = actionTypeGlobalStatuses[it.first].value != 0.0f;
    bool wasActive = it.second.value != 0.0f;
    if(isActive != wasActive) {
        //State changed. Reset the timer.
        it.second.activationStateDuration = 0.0f;
        it.second.nextAutoRepeatActivation = options.autoRepeatMaxInterval;
    } else {
        //Same state, increase the timer.
        it.second.activationStateDuration += deltaT;
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
 * @brief Ignores an input source from now on until the player performs the
 * input with value 0, at which point it becomes unignored.
 *
 * @param inputSource Input source to ignore.
 * @param nowOnly If true, only apply to inputs that are currently already
 * held down (> 0) this frame.
 * If false, leave the ignore rule until the next time it's pressed down.
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
