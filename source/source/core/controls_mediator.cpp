/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Control-related functions.
 */

#include <algorithm>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "controls_mediator.h"

#include "../game_state/gameplay/gameplay.h"
#include "../util/general_utils.h"
#include "../util/string_utils.h"
#include "const.h"
#include "drawing.h"
#include "game.h"
#include "misc_functions.h"


/**
 * @brief Returns whether the given action types have any binds where any of
 * them end up sharing an input source with another one.
 *
 * @param actionTypes List of action types to check.
 * @return Whether there's any sharing.
 */
bool ControlsMediator::actionTypesShareInputSource(
    const vector<PLAYER_ACTION_TYPE> actionTypes
) {
    set<Inpution::InputSource> sourcesUsed;
    const vector<Inpution::Bind>& allBinds = binds();
    for(size_t b = 0; b < allBinds.size(); b++) {
        if(!isInContainer(actionTypes, allBinds[b].actionTypeId)) continue;
        if(sourcesUsed.find(allBinds[b].inputSource) != sourcesUsed.end()) {
            return true;
        }
        sourcesUsed.insert(allBinds[b].inputSource);
    }
    return false;
}


/**
 * @brief Registers a new modifier, for any binds that want modifiers.
 *
 * @param id ID of the modifier.
 * @param source Input source of the modifier.
 */
void ControlsMediator::addModifier(
    int id, const Inpution::InputSource& source
) {
    mgr.modifiers[id] = source;
}


/**
 * @brief Adds a new player action to the list.
 *
 * @param id Its ID.
 * @param category Its category.
 * @param name Its name.
 * @param description Its description.
 * @param internalName The name of its property in the options file.
 * @param defaultBindStr String representing of this action's default
 * control bind.
 * @param valueType Type of value an action can have.
 * @param autoRepeat Auto-repeat threshold.
 * @param reinsertionTTL Time to live when reinserted into the queue.
 */
void ControlsMediator::addPlayerActionType(
    PLAYER_ACTION_TYPE id, PLAYER_ACTION_CAT category,
    const string& name, const string& description, const string& internalName,
    const string& defaultBindStr, Inpution::ACTION_VALUE_TYPE valueType,
    float autoRepeat, float reinsertionTTL
) {
    PlayerActionType a;
    a.id = id;
    a.category = category;
    a.name = name;
    a.description = description;
    a.internalName = internalName;
    a.defaultBindStr = defaultBindStr;
    a.valueType = valueType;
    a.autoRepeat = autoRepeat;
    a.reinsertionTTL = reinsertionTTL;
    
    playerActionTypes.push_back(a);
    mgr.actionTypes[id] = a;
}


/**
 * @brief Returns the parsed input from an Allegro event.
 *
 * @param ev The Allegro event.
 * @return The input.
 * If this event does not pertain to any valid input, an input of type
 * INPUT_SOURCE_TYPE_NONE is returned.
 */
Inpution::Input ControlsMediator::allegroEventToInput(
    const ALLEGRO_EVENT& ev
) const {
    Inpution::Input input;
    
    switch(ev.type) {
    case ALLEGRO_EVENT_KEY_DOWN:
    case ALLEGRO_EVENT_KEY_UP: {
        input.source.type = Inpution::INPUT_SOURCE_TYPE_KEYBOARD_KEY;
        input.source.buttonNr = ev.keyboard.keycode;
        input.value = (ev.type == ALLEGRO_EVENT_KEY_DOWN) ? 1 : 0;
        break;
        
    } case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
    case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
        input.source.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_BUTTON;
        input.source.buttonNr = ev.mouse.button;
        input.value = (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;
        break;
        
    } case ALLEGRO_EVENT_MOUSE_AXES: {
        if(ev.mouse.dz > 0) {
            input.source.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP;
            input.value = ev.mouse.dz;
        } else if(ev.mouse.dz < 0) {
            input.source.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN;
            input.value = -ev.mouse.dz;
        } else if(ev.mouse.dw > 0) {
            input.source.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_RIGHT;
            input.value = ev.mouse.dw;
        } else if(ev.mouse.dw < 0) {
            input.source.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_LEFT;
            input.value = -ev.mouse.dw;
        }
        break;
        
    } case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
    case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP: {
        input.source.type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_BUTTON;
        input.source.deviceNr = game.controllerNumbers[ev.joystick.id];
        input.source.buttonNr = ev.joystick.button;
        input.value = (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) ? 1 : 0;
        break;
        
    } case ALLEGRO_EVENT_JOYSTICK_AXIS: {
        if(ev.joystick.pos >= 0.0f) {
            input.source.type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS;
            input.value = ev.joystick.pos;
        } else {
            input.source.type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG;
            input.value = -ev.joystick.pos;
        }
        input.source.deviceNr = game.controllerNumbers[ev.joystick.id];
        input.source.stickNr = ev.joystick.stick;
        input.source.axisNr = ev.joystick.axis;
        break;
    }
    }
    
    return input;
}


/**
 * @brief Returns the array of registered bind.
 *
 * @return The binds.
 */
vector<Inpution::Bind>& ControlsMediator::binds() {
    return mgr.binds;
}


/**
 * @brief Finds a registered control bind for player 1 that matches
 * the requested action. Returns an empty bind if none is found.
 *
 * @param actionTypeId ID of the action type.
 * @return The bind.
 */
Inpution::Bind ControlsMediator::findBind(
    const PLAYER_ACTION_TYPE actionTypeId
) const {
    for(size_t b = 0; b < mgr.binds.size(); b++) {
        if(mgr.binds[b].actionTypeId == actionTypeId) {
            return mgr.binds[b];
        }
    }
    return Inpution::Bind();
}


/**
 * @brief Finds a registered control bind for player 1 that matches
 * the requested action. Returns an empty bind if none is found.
 *
 * @param actionName Name of the action.
 * @return The bind.
 */
Inpution::Bind ControlsMediator::findBind(
    const string& actionName
) const {
    for(size_t b = 0; b < playerActionTypes.size(); b++) {
        if(playerActionTypes[b].internalName == actionName) {
            return findBind(playerActionTypes[b].id);
        }
    }
    return Inpution::Bind();
}


/**
 * @brief Returns the current list of registered player action types.
 *
 * @return The types.
 */
const vector<PlayerActionType>
& ControlsMediator::getAllPlayerActionTypes() const {
    return playerActionTypes;
}


/**
 * @brief Returns the current value of an input source.
 *
 * @param source The source.
 * @return The value, or 0.0f if not found.
 */
float ControlsMediator::getInputSourceValue(
    const Inpution::InputSource& source
) const {
    return mgr.getInputSourceValue(source);
}


/**
 * @brief Returns a registered type, given its ID.
 *
 * @param actionId ID of the player action.
 * @return The type, or an empty type on failure.
 */
PlayerActionType ControlsMediator::getPlayerActionType(
    int actionId
) const {
    for(size_t b = 0; b < playerActionTypes.size(); b++) {
        if(playerActionTypes[b].id == actionId) {
            return playerActionTypes[b];
        }
    }
    return PlayerActionType();
}


/**
 * @brief Returns the internal name from an input ID,
 * used in the on_input_received event.
 *
 * @param actionId ID of the player action.
 * @return The name, or an empty string on failure.
 */
string ControlsMediator::getPlayerActionTypeInternalName(
    int actionId
) {
    for(size_t b = 0; b < playerActionTypes.size(); b++) {
        if(playerActionTypes[b].id == actionId) {
            return playerActionTypes[b].internalName;
        }
    }
    return "";
}


/**
 * @brief Returns the current input value of a given action type.
 *
 * @param playerActionTypeId Action type to use.
 * @return The value.
 */
float ControlsMediator::getPlayerActionTypeValue(
    PLAYER_ACTION_TYPE playerActionTypeId
) {
    return mgr.getValue((int) playerActionTypeId);
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The Allegro event.
 * @return Whether the event was handled.
 */
bool ControlsMediator::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    Inpution::Input input = allegroEventToInput(ev);
    
    if(input.source.type != Inpution::INPUT_SOURCE_TYPE_NONE) {
        mgr.handleInput(input);
        return true;
    } else {
        return false;
    }
}


/**
 * @brief Creates a string that represents an input.
 * Ignores the player number.
 *
 * @param s Input source to read from.
 * @return The string, or an empty string on error.
 */
string ControlsMediator::inputSourceToStr(
    const Inpution::InputSource& s
) const {
    switch(s.type) {
    case Inpution::INPUT_SOURCE_TYPE_KEYBOARD_KEY: {
        return "k_" + i2s(s.buttonNr);
    } case Inpution::INPUT_SOURCE_TYPE_MOUSE_BUTTON: {
        return "mb_" + i2s(s.buttonNr);
    } case Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP: {
        return "mwu";
    } case Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN: {
        return "mwd";
    } case Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_LEFT: {
        return "mwl";
    } case Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_RIGHT: {
        return "mwr";
    } case Inpution::INPUT_SOURCE_TYPE_CONTROLLER_BUTTON: {
        return "jb_" + i2s(s.deviceNr) + "_" + i2s(s.buttonNr);
    } case Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS: {
        return
            "jap_" + i2s(s.deviceNr) +
            "_" + i2s(s.stickNr) + "_" + i2s(s.axisNr);
    } case Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG: {
        return
            "jan_" + i2s(s.deviceNr) +
            "_" + i2s(s.stickNr) + "_" + i2s(s.axisNr);
    } default: {
        return "";
    }
    }
}


/**
 * @brief Loads a list of binds from a data node. Binds are formatted like so:
 * "<action type>=<input 1>;<input 2>;<...>"
 *
 * @param node The node.
 * @param playerNr Player number.
 */
void ControlsMediator::loadBindsFromDataNode(
    DataNode* node, unsigned char playerNr
) {
    const vector<PlayerActionType>& playerActionTypes =
        getAllPlayerActionTypes();
        
    for(size_t a = 0; a < playerActionTypes.size(); a++) {
        string actionTypeName = playerActionTypes[a].internalName;
        if(actionTypeName.empty()) continue;
        
        DataNode* bindNode = node->getChildByName(actionTypeName);
        vector<string> inputs = semicolonListToVector(bindNode->value);
        
        for(size_t c = 0; c < inputs.size(); c++) {
            Inpution::InputSource inputSource = strToInputSource(inputs[c]);
            if(inputSource.type == Inpution::INPUT_SOURCE_TYPE_NONE) continue;
            
            Inpution::Bind newBind;
            newBind.actionTypeId = playerActionTypes[a].id;
            newBind.playerNr = playerNr;
            newBind.inputSource = inputSource;
            binds().push_back(newBind);
        }
    }
}


/**
 * @brief Returns the player actions that occurred during the last frame
 * of gameplay, and begins a new frame.
 *
 * @param deltaT How much time has passed since the last frame.
 * @return The player actions.
 */
vector<Inpution::Action> ControlsMediator::newFrame(float deltaT) {
    return mgr.newFrame(deltaT);
}


/**
 * @brief Reinserts an action into the queue, decreasing its time-to-live.
 *
 * @param action The action.
 */
void ControlsMediator::reinsertAction(const Inpution::Action& action) {
    mgr.reinsertAction(action);
}


/**
 * @brief Releases all player inputs. Basically, set all of their values to 0.
 * Useful for when the game state is changed, or the window is out of focus.
 */
void ControlsMediator::releaseAll() {
    mgr.releaseEverything();
}


/**
 * @brief Loads the list of binds to a data node.
 *
 * @param node The node.
 * @param playerNr Player number.
 */
void ControlsMediator::saveBindsToDataNode(
    DataNode* node, unsigned char playerNr
) {
    map<string, string> bindStrs;
    const vector<PlayerActionType>& playerActionTypes =
        getAllPlayerActionTypes();
    const vector<Inpution::Bind>& allBinds = binds();
    
    //Fill the defaults, which are all empty strings.
    for(size_t b = 0; b < playerActionTypes.size(); b++) {
        string actionTypeName = playerActionTypes[b].internalName;
        if(actionTypeName.empty()) continue;
        bindStrs[actionTypeName].clear();
    }
    
    //Fill their input strings.
    for(size_t b = 0; b < allBinds.size(); b++) {
        if(allBinds[b].playerNr != playerNr) continue;
        PlayerActionType actionType =
            getPlayerActionType(allBinds[b].actionTypeId);
        bindStrs[actionType.internalName] +=
            inputSourceToStr(allBinds[b].inputSource) + ";";
    }
    
    //Save them all.
    for(auto& c : bindStrs) {
        //Remove the final character, which is always an extra semicolon.
        if(c.second.size()) c.second.erase(c.second.size() - 1);
        
        node->addNew(c.first, c.second);
    }
}


/**
 * @brief Sets the game state for the controls manager.
 *
 * @param state The state.
 */
void ControlsMediator::setGameState(CONTROLS_GAME_STATE state) {
    switch(state) {
    case CONTROLS_GAME_STATE_MENUS: {
        mgr.setGameState("menus");
        break;
    } case CONTROLS_GAME_STATE_INTERLUDE: {
        mgr.setGameState("interlude");
        break;
    } case CONTROLS_GAME_STATE_GAMEPLAY: {
        mgr.setGameState("gameplay");
        break;
    }
    }
}


/**
 * @brief Sets the options for the controls manager.
 *
 * @param options Options.
 */
void ControlsMediator::setOptions(const Inpution::ManagerOptions& options) {
    mgr.options = options;
}


/**
 * @brief Ignore player actions from here on.
 */
void ControlsMediator::startIgnoringActions() {
    mgr.ignoringActions = true;
}


/**
 * @brief Ignores an input source from now on until the player performs the
 * input with value 0, at which point it becomes unignored.
 *
 * @param inputSource Input source to ignore.
 * @param nowOnly If true, only apply to inputs that are currently held down.
 * If false, leave the ignoring until the next time it's pressed down.
 */
void ControlsMediator::startIgnoringInputSource(
    const Inpution::InputSource& inputSource, bool nowOnly
) {
    mgr.startIgnoringInputSource(inputSource, nowOnly);
}


/**
 * @brief No longer ignore player actions from here on.
 */
void ControlsMediator::stopIgnoringActions() {
    mgr.ignoringActions = false;
}


/**
 * @brief Creates an input from a string representation.
 * Ignores the player number. Input strings are formatted like so:
 * "<input type>_<parameters, underscore separated>"
 * Input types are:
 * "k" (keyboard key), "mb" (mouse button),
 * "mwu" (mouse wheel up), "mwd" (down),
 * "mwl" (left), "mwr" (right), "jb" (joystick button),
 * "jap" (joystick axis, positive), "jan" (joystick axis, negative).
 * The parameters are the key/button number, controller number,
 * controller stick and axis, etc.
 * @param s String to read from.
 * @return The input, or a default input instance on error.
 */
Inpution::InputSource ControlsMediator::strToInputSource(
    const string& s
) const {
    Inpution::InputSource inputSource;
    
    vector<string> parts = split(s, "_");
    size_t nParts = parts.size();
    
    if(nParts == 0) return inputSource;
    
    if(parts[0] == "k" && nParts >= 2) {
        //Keyboard.
        inputSource.type = Inpution::INPUT_SOURCE_TYPE_KEYBOARD_KEY;
        inputSource.buttonNr = s2i(parts[1]);
        
    } else if(parts[0] == "mb" && nParts >= 2) {
        //Mouse button.
        inputSource.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_BUTTON;
        inputSource.buttonNr = s2i(parts[1]);
        
    } else if(parts[0] == "mwu") {
        //Mouse wheel up.
        inputSource.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP;
        
    } else if(parts[0] == "mwd") {
        //Mouse wheel down.
        inputSource.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN;
        
    } else if(parts[0] == "mwl") {
        //Mouse wheel left.
        inputSource.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_LEFT;
        
    } else if(parts[0] == "mwr") {
        //Mouse wheel right.
        inputSource.type = Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_RIGHT;
        
    } else if(parts[0] == "jb" && nParts >= 3) {
        //Controller button.
        inputSource.type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_BUTTON;
        inputSource.deviceNr = s2i(parts[1]);
        inputSource.buttonNr = s2i(parts[2]);
        
    } else if(parts[0] == "jap" && nParts >= 4) {
        //Controller stick axis, positive.
        inputSource.type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS;
        inputSource.deviceNr = s2i(parts[1]);
        inputSource.stickNr = s2i(parts[2]);
        inputSource.axisNr = s2i(parts[3]);
        
    } else if(parts[0] == "jan" && nParts >= 4) {
        //Controller stick axis, negative.
        inputSource.type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG;
        inputSource.deviceNr = s2i(parts[1]);
        inputSource.stickNr = s2i(parts[2]);
        inputSource.axisNr = s2i(parts[3]);
        
    } else {
        game.errors.report(
            "Unrecognized input \"" + s + "\"!"
        );
    }
    
    return inputSource;
}
