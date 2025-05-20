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
 * @brief Adds a new player action to the list.
 *
 * @param id Its ID.
 * @param category Its category.
 * @param name Its name.
 * @param description Its descripton.
 * @param internalName The name of its property in the options file.
 * @param defaultBindStr String representing of this action's default
 * control bind.
 * @param autoRepeat Auto-repeat threshold.
 */
void ControlsMediator::addPlayerActionType(
    PLAYER_ACTION_TYPE id, PLAYER_ACTION_CAT category,
    const string &name, const string &description, const string &internalName,
    const string &defaultBindStr, float autoRepeat
) {
    PfePlayerActionType a;
    a.id = id;
    a.category = category;
    a.name = name;
    a.description = description;
    a.internalName = internalName;
    a.defaultBindStr = defaultBindStr;
    a.autoRepeat = autoRepeat;
    
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
PlayerInput ControlsMediator::allegroEventToInput(
    const ALLEGRO_EVENT &ev
) const {
    PlayerInput input;
    
    switch(ev.type) {
    case ALLEGRO_EVENT_KEY_DOWN:
    case ALLEGRO_EVENT_KEY_UP: {
        input.source.type = INPUT_SOURCE_TYPE_KEYBOARD_KEY;
        input.source.buttonNr = ev.keyboard.keycode;
        input.value = (ev.type == ALLEGRO_EVENT_KEY_DOWN) ? 1 : 0;
        break;
        
    } case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
    case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
        input.source.type = INPUT_SOURCE_TYPE_MOUSE_BUTTON;
        input.source.buttonNr = ev.mouse.button;
        input.value = (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;
        break;
        
    } case ALLEGRO_EVENT_MOUSE_AXES: {
        if(ev.mouse.dz > 0) {
            input.source.type = INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP;
            input.value = ev.mouse.dz;
        } else if(ev.mouse.dz < 0) {
            input.source.type = INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN;
            input.value = -ev.mouse.dz;
        } else if(ev.mouse.dw > 0) {
            input.source.type = INPUT_SOURCE_TYPE_MOUSE_WHEEL_RIGHT;
            input.value = ev.mouse.dw;
        } else if(ev.mouse.dw < 0) {
            input.source.type = INPUT_SOURCE_TYPE_MOUSE_WHEEL_LEFT;
            input.value = -ev.mouse.dw;
        }
        break;
        
    } case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
    case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP: {
        input.source.type = INPUT_SOURCE_TYPE_CONTROLLER_BUTTON;
        input.source.deviceNr = game.controllerNumbers[ev.joystick.id];
        input.source.buttonNr = ev.joystick.button;
        input.value = (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) ? 1 : 0;
        break;
        
    } case ALLEGRO_EVENT_JOYSTICK_AXIS: {
        if(ev.joystick.pos >= 0.0f) {
            input.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS;
            input.value = ev.joystick.pos;
        } else {
            input.source.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG;
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
vector<ControlBind> &ControlsMediator::binds() {
    return mgr.binds;
}


/**
 * @brief Finds a registered control bind for player 1 that matches
 * the requested action. Returns an empty bind if none is found.
 *
 * @param actionTypeId ID of the action type.
 * @return The bind.
 */
ControlBind ControlsMediator::findBind(
    const PLAYER_ACTION_TYPE actionTypeId
) const {
    for(size_t b = 0; b < mgr.binds.size(); b++) {
        if(mgr.binds[b].actionTypeId == actionTypeId) {
            return mgr.binds[b];
        }
    }
    return ControlBind();
}


/**
 * @brief Finds a registered control bind for player 1 that matches
 * the requested action. Returns an empty bind if none is found.
 *
 * @param actionName Name of the action.
 * @return The bind.
 */
ControlBind ControlsMediator::findBind(
    const string &actionName
) const {
    for(size_t b = 0; b < playerActionTypes.size(); b++) {
        if(playerActionTypes[b].internalName == actionName) {
            return findBind(playerActionTypes[b].id);
        }
    }
    return ControlBind();
}


/**
 * @brief Returns the current list of registered player action types.
 *
 * @return The types.
 */
const vector<PfePlayerActionType>
&ControlsMediator::getAllPlayerActionTypes() const {
    return playerActionTypes;
}


/**
 * @brief Returns a registered type, given its ID.
 *
 * @param actionId ID of the player action.
 * @return The type, or an empty type on failure.
 */
PfePlayerActionType ControlsMediator::getPlayerActionType(
    int actionId
) const {
    for(size_t b = 0; b < playerActionTypes.size(); b++) {
        if(playerActionTypes[b].id == actionId) {
            return playerActionTypes[b];
        }
    }
    return PfePlayerActionType();
}


/**
 * @brief Returns the internal name from an input ID,
 * used in the on_input_recieved event.
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
bool ControlsMediator::handleAllegroEvent(const ALLEGRO_EVENT &ev) {
    PlayerInput input = allegroEventToInput(ev);
    
    if(input.source.type != INPUT_SOURCE_TYPE_NONE) {
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
    const PlayerInputSource &s
) const {
    switch(s.type) {
    case INPUT_SOURCE_TYPE_KEYBOARD_KEY: {
        return "k_" + i2s(s.buttonNr);
    } case INPUT_SOURCE_TYPE_MOUSE_BUTTON: {
        return "mb_" + i2s(s.buttonNr);
    } case INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP: {
        return "mwu";
    } case INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN: {
        return "mwd";
    } case INPUT_SOURCE_TYPE_MOUSE_WHEEL_LEFT: {
        return "mwl";
    } case INPUT_SOURCE_TYPE_MOUSE_WHEEL_RIGHT: {
        return "mwr";
    } case INPUT_SOURCE_TYPE_CONTROLLER_BUTTON: {
        return "jb_" + i2s(s.deviceNr) + "_" + i2s(s.buttonNr);
    } case INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS: {
        return
            "jap_" + i2s(s.deviceNr) +
            "_" + i2s(s.stickNr) + "_" + i2s(s.axisNr);
    } case INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG: {
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
    const vector<PfePlayerActionType> &playerActionTypes =
        getAllPlayerActionTypes();
        
    for(size_t a = 0; a < playerActionTypes.size(); a++) {
        string actionTypeName = playerActionTypes[a].internalName;
        if(actionTypeName.empty()) continue;
        
        DataNode* bindNode = node->getChildByName(actionTypeName);
        vector<string> inputs = semicolonListToVector(bindNode->value);
        
        for(size_t c = 0; c < inputs.size(); c++) {
            PlayerInputSource inputSource = strToInputSource(inputs[c]);
            if(inputSource.type == INPUT_SOURCE_TYPE_NONE) continue;
            
            ControlBind newBind;
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
vector<PlayerAction> ControlsMediator::newFrame(float deltaT) {
    return mgr.newFrame(deltaT);
}


/**
 * @brief Releases all player inputs. Basically, set all of their values to 0.
 * Useful for when the game state is changed, or the window is out of focus.
 */
void ControlsMediator::releaseAll() {
    for(auto &a : mgr.actionTypes) {
        mgr.setValue(a.first, 0.0f);
    }
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
    const vector<PfePlayerActionType> &playerActionTypes =
        getAllPlayerActionTypes();
    const vector<ControlBind> &allBinds = binds();
    
    //Fill the defaults, which are all empty strings.
    for(size_t b = 0; b < playerActionTypes.size(); b++) {
        string actionTypeName = playerActionTypes[b].internalName;
        if(actionTypeName.empty()) continue;
        bindStrs[actionTypeName].clear();
    }
    
    //Fill their input strings.
    for(size_t b = 0; b < allBinds.size(); b++) {
        if(allBinds[b].playerNr != playerNr) continue;
        PfePlayerActionType actionType =
            getPlayerActionType(allBinds[b].actionTypeId);
        bindStrs[actionType.internalName] +=
            inputSourceToStr(allBinds[b].inputSource) + ";";
    }
    
    //Save them all.
    for(auto &c : bindStrs) {
        //Remove the final character, which is always an extra semicolon.
        if(c.second.size()) c.second.erase(c.second.size() - 1);
        
        node->addNew(c.first, c.second);
    }
}


/**
 * @brief Sets the options for the controls manager.
 *
 * @param options Options.
 */
void ControlsMediator::setOptions(const ControlsManagerOptions &options) {
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
 */
void ControlsMediator::startIgnoringInputSource(
    const PlayerInputSource &inputSource
) {
    mgr.startIgnoringInputSource(inputSource);
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
PlayerInputSource ControlsMediator::strToInputSource(
    const string &s
) const {
    PlayerInputSource inputSource;
    
    vector<string> parts = split(s, "_");
    size_t nParts = parts.size();
    
    if(nParts == 0) return inputSource;
    
    if(parts[0] == "k" && nParts >= 2) {
        //Keyboard.
        inputSource.type = INPUT_SOURCE_TYPE_KEYBOARD_KEY;
        inputSource.buttonNr = s2i(parts[1]);
        
    } else if(parts[0] == "mb" && nParts >= 2) {
        //Mouse button.
        inputSource.type = INPUT_SOURCE_TYPE_MOUSE_BUTTON;
        inputSource.buttonNr = s2i(parts[1]);
        
    } else if(parts[0] == "mwu") {
        //Mouse wheel up.
        inputSource.type = INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP;
        
    } else if(parts[0] == "mwd") {
        //Mouse wheel down.
        inputSource.type = INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN;
        
    } else if(parts[0] == "mwl") {
        //Mouse wheel left.
        inputSource.type = INPUT_SOURCE_TYPE_MOUSE_WHEEL_LEFT;
        
    } else if(parts[0] == "mwr") {
        //Mouse wheel right.
        inputSource.type = INPUT_SOURCE_TYPE_MOUSE_WHEEL_RIGHT;
        
    } else if(parts[0] == "jb" && nParts >= 3) {
        //Controller button.
        inputSource.type = INPUT_SOURCE_TYPE_CONTROLLER_BUTTON;
        inputSource.deviceNr = s2i(parts[1]);
        inputSource.buttonNr = s2i(parts[2]);
        
    } else if(parts[0] == "jap" && nParts >= 4) {
        //Controller stick axis, positive.
        inputSource.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS;
        inputSource.deviceNr = s2i(parts[1]);
        inputSource.stickNr = s2i(parts[2]);
        inputSource.axisNr = s2i(parts[3]);
        
    } else if(parts[0] == "jan" && nParts >= 4) {
        //Controller stick axis, negative.
        inputSource.type = INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG;
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
