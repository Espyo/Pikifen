/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Hardware-related classes and functions.
 */


#include <cmath>

#include "hardware_mediator.h"

#include "../util/string_utils.h"
#include "game.h"


using std::make_pair;


const map<DEVICE_BRAND, HardwareMediator::DeviceBrand>
HardwareMediator::deviceBrandDb {
    //Keyboard.
    {
        DEVICE_BRAND_KEYBOARD_ANY,
        {
            .buttonIcons {
                { ALLEGRO_KEY_ESCAPE, { "Esc" } },
                { ALLEGRO_KEY_INSERT, { "Ins" } },
                { ALLEGRO_KEY_DELETE, { "Del" } },
                { ALLEGRO_KEY_PGUP, { "PgUp" } },
                { ALLEGRO_KEY_PGDN, { "PgDn" } },
                { ALLEGRO_KEY_PAD_0, { "0 KP" } },
                { ALLEGRO_KEY_PAD_1, { "1 KP" } },
                { ALLEGRO_KEY_PAD_2, { "2 KP" } },
                { ALLEGRO_KEY_PAD_3, { "3 KP" } },
                { ALLEGRO_KEY_PAD_4, { "4 KP" } },
                { ALLEGRO_KEY_PAD_5, { "5 KP" } },
                { ALLEGRO_KEY_PAD_6, { "6 KP" } },
                { ALLEGRO_KEY_PAD_7, { "7 KP" } },
                { ALLEGRO_KEY_PAD_8, { "8 KP" } },
                { ALLEGRO_KEY_PAD_9, { "9 KP" } },
                { ALLEGRO_KEY_PAD_ASTERISK, { "* KP" } },
                { ALLEGRO_KEY_PAD_DELETE, { "Del KP" } },
                { ALLEGRO_KEY_PAD_ENTER, { "Enter KP" } },
                { ALLEGRO_KEY_PAD_EQUALS, { "= KP" } },
                { ALLEGRO_KEY_PAD_MINUS, { "- KP" } },
                { ALLEGRO_KEY_PAD_PLUS, { "+ KP" } },
                { ALLEGRO_KEY_PAD_SLASH, { "/ KP" } },
                {
                    ALLEGRO_KEY_LSHIFT,
                    { "Shift", PLAYER_INPUT_ICON_SPRITE_SHIFT }
                },
                {
                    ALLEGRO_KEY_RSHIFT,
                    { "Shift", PLAYER_INPUT_ICON_SPRITE_SHIFT }
                },
                { ALLEGRO_KEY_ALT, { "Alt" } },
                { ALLEGRO_KEY_ALTGR, { "AltGr" } },
                { ALLEGRO_KEY_LCTRL, { "Ctrl" } },
                { ALLEGRO_KEY_RCTRL, { "Ctrl" } },
                { ALLEGRO_KEY_BACKSLASH, { "\\" } },
                { ALLEGRO_KEY_BACKSLASH2, { "\\" } },
                {
                    ALLEGRO_KEY_BACKSPACE,
                    { "BkSpc", PLAYER_INPUT_ICON_SPRITE_BACKSPACE }
                },
                {
                    ALLEGRO_KEY_TAB,
                    { "Tab", PLAYER_INPUT_ICON_SPRITE_TAB }
                },
                {
                    ALLEGRO_KEY_ENTER,
                    { "Enter", PLAYER_INPUT_ICON_SPRITE_ENTER }
                },
                {
                    ALLEGRO_KEY_RIGHT,
                    { "Right", PLAYER_INPUT_ICON_SPRITE_RIGHT }
                },
                {
                    ALLEGRO_KEY_DOWN,
                    { "Down", PLAYER_INPUT_ICON_SPRITE_DOWN }
                },
                {
                    ALLEGRO_KEY_LEFT,
                    { "Left", PLAYER_INPUT_ICON_SPRITE_LEFT }
                },
                {
                    ALLEGRO_KEY_UP,
                    { "Up", PLAYER_INPUT_ICON_SPRITE_UP }
                },
            }
        }
    },
    //Mouse.
    {
        DEVICE_BRAND_MOUSE_ANY,
        {
            .buttonIcons {
                { 1, { "LMB", PLAYER_INPUT_ICON_SPRITE_LMB } },
                { 2, { "RMB", PLAYER_INPUT_ICON_SPRITE_RMB } },
                { 3, { "MMB", PLAYER_INPUT_ICON_SPRITE_MMB } },
            },
            .stickIcons {
                { 0, { "MWU", PLAYER_INPUT_ICON_SPRITE_MWU } },
                { 1, { "MWD", PLAYER_INPUT_ICON_SPRITE_MWD } },
            }
        }
    },
    //Nintendo Switch Pro Controller.
    {
        DEVICE_BRAND_CONTROLLER_SWITCH_PRO,
        {
            .buttonIcons {
                { 0, { "B" } },
                { 1, { "A" } },
                { 2, { "X" } },
                { 3, { "Y" } },
                { 4, { "Capture", PLAYER_INPUT_ICON_SWITCH_CAPTURE } },
                { 5, { "L" } },
                { 6, { "R" } },
                { 7, { "ZL" } },
                { 8, { "ZR" } },
                { 9, { "-" } },
                { 10, { "+" } },
                { 11, { "Home", PLAYER_INPUT_ICON_SWITCH_HOME } },
                { 12, { "L Stick", PLAYER_INPUT_ICON_SPRITE_L_STICK_CLICK } },
                { 13, { "R Stick", PLAYER_INPUT_ICON_SPRITE_R_STICK_CLICK } },
            },
            .stickIcons {
                { 0, { "L Stick", PLAYER_INPUT_ICON_SPRITE_L_STICK_RIGHT } },
                { 1, { "R Stick", PLAYER_INPUT_ICON_SPRITE_R_STICK_RIGHT } },
                { 2, { "D-pad", PLAYER_INPUT_ICON_SPRITE_D_PAD_RIGHT } },
            }
        }
    },
    //X-Box 360.
    {
        DEVICE_BRAND_CONTROLLER_XBOX_360,
        {
            .absurdityMap {
                { { false, 1, 1 }, { false, 1, 0 } }, //R stick horizontal.
                { { false, 2, 0 }, { false, 1, 1 } }, //R stick vertical.
                { { false, 1, 0 }, { true, 4, 0 } }, //LT.
                { { false, 2, 1 }, { true, 5, 0 } }, //RT.
            },
            .buttonIcons {
                { 0, { "A" } },
                { 1, { "B" } },
                { 2, { "X" } },
                { 3, { "Y" } },
                { 4, { "LB" } },
                { 5, { "RB" } },
                { 6, { "Back", PLAYER_INPUT_ICON_SPRITE_XBOX_BACK } },
                { 7, { "Start", PLAYER_INPUT_ICON_SPRITE_XBOX_START } },
                { 9, { "L Stick", PLAYER_INPUT_ICON_SPRITE_L_STICK_CLICK } },
                { 10, { "R Stick", PLAYER_INPUT_ICON_SPRITE_R_STICK_CLICK } },
            },
            .stickIcons {
                { 0, { "L Stick", PLAYER_INPUT_ICON_SPRITE_L_STICK_RIGHT }},
                { 1, { "L Stick", PLAYER_INPUT_ICON_SPRITE_R_STICK_RIGHT }},
                { 3, { "D-pad", PLAYER_INPUT_ICON_SPRITE_D_PAD_RIGHT }},
                { 4, { "LT" }},
                { 5, { "RT" }},
            }
        }
    }
};


/**
 * @brief Returns the controller number of the given Allegro joystick.
 * Returns INVALID if unknown.
 *
 * @param aJoyPtr The Allegro joystick.
 * @return The number.
 */
size_t HardwareMediator::getControllerNr(ALLEGRO_JOYSTICK* aJoyPtr) {
    for(size_t c = 0; c < controllers.size(); c++) {
        if(controllers[c].aJoyPtr == aJoyPtr) {
            return c;
        }
    }
    return INVALID;
}


/**
 * @brief Returns information about how a player input source's
 * icon should be drawn from the database.
 *
 * @param source Info on the input source.
 * @return The database entry, or nullptr if not found.
 */
const HardwareMediator::InputSourceIcon*
HardwareMediator::getIconDbEntry(
    const Inpution::InputSource& source
) const {
    const map<int, InputSourceIcon>* dbMap = nullptr;
    int dbMapKey = INVALID;
    
    const auto getControllerBrand =
    [source, this] (DEVICE_BRAND * result) {
        DEVICE_BRAND brand = DEVICE_BRAND_CONTROLLER_UNKOWN;
        if((size_t) source.deviceNr < controllers.size()) {
            brand = controllers[source.deviceNr].brand;
        }
        if(deviceBrandDb.find(brand) != deviceBrandDb.end()) {
            *result = brand;
            return true;
        }
        return false;
    };
    
    if(source.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_BUTTON) {
        dbMap = &deviceBrandDb.at(DEVICE_BRAND_MOUSE_ANY).buttonIcons;
        dbMapKey = source.buttonNr;
        
    } else if(source.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP) {
        dbMap = &deviceBrandDb.at(DEVICE_BRAND_MOUSE_ANY).stickIcons;
        dbMapKey = 0;
        
    } else if(source.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN) {
        dbMap = &deviceBrandDb.at(DEVICE_BRAND_MOUSE_ANY).stickIcons;
        dbMapKey = 1;
        
    } else if(source.type == Inpution::INPUT_SOURCE_TYPE_KEYBOARD_KEY) {
        dbMap = &deviceBrandDb.at(DEVICE_BRAND_KEYBOARD_ANY).buttonIcons;
        dbMapKey = source.buttonNr;
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_BUTTON
    ) {
        DEVICE_BRAND controllerBrand;
        if(getControllerBrand(&controllerBrand)) {
            dbMap = &deviceBrandDb.at(controllerBrand).buttonIcons;
        }
        dbMapKey = source.buttonNr;
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG ||
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS ||
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_ANALOG_BUTTON
    ) {
        DEVICE_BRAND controllerBrand;
        if(getControllerBrand(&controllerBrand)) {
            dbMap = &deviceBrandDb.at(controllerBrand).stickIcons;
        }
        dbMapKey = source.stickNr;
        
    }
    
    if(!dbMap) return nullptr;
    if(dbMap->find(dbMapKey) == dbMap->end()) return nullptr;
    
    return &dbMap->at(dbMapKey);
}


/**
 * @brief Returns information about how a player input source's icon should
 * be drawn from its database entry.
 *
 * @param dbEntry The database entry.
 * @param source Info on the input source.
 * If invalid, a "NONE" icon will be used.
 * @param outShape The shape is returned here.
 * @param outText The text to be written inside is returned here, or an
 * empty string is returned if there's nothing to write.
 * @param outBitmapSprite If it's one of the icons in the control bind
 * input icon spritesheet, the index of the sprite is returned here.
 */
void HardwareMediator::getIconInfoFromDbEntry(
    const HardwareMediator::InputSourceIcon* dbEntry,
    const Inpution::InputSource& source,
    PLAYER_INPUT_ICON_SHAPE* outShape, string* outText,
    PLAYER_INPUT_ICON_SPRITE* outBitmapSprite
) const {
    bool isStickRight =
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS &&
        source.axisNr == 0;
    bool isStickDown =
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS &&
        source.axisNr == 1;
    bool isStickLeft =
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG &&
        source.axisNr == 0;
    bool isStickUp =
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG &&
        source.axisNr == 1;
        
    *outText = dbEntry->name;
    
    if(dbEntry->spriteIdx != INVALID) {
        //Has icon.
        size_t iconIdxToUse = dbEntry->spriteIdx;
        
        if(isStickDown) {
            iconIdxToUse = dbEntry->spriteIdx + 1;
        } else if(isStickLeft) {
            iconIdxToUse = dbEntry->spriteIdx + 2;
        } else if(isStickUp) {
            iconIdxToUse = dbEntry->spriteIdx + 3;
        }
        
        *outShape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
        *outBitmapSprite = (PLAYER_INPUT_ICON_SPRITE) iconIdxToUse;
        
    } else {
        //Text-only.
        if(isStickRight) {
            *outText += " right";
        } else if(isStickDown) {
            *outText += " down";
        } else if(isStickLeft) {
            *outText += " left";
        } else if(isStickUp) {
            *outText += " up";
        }
    }
}


/**
 * @brief Returns information about how a player input source's icon should
 * be drawn, by making it up as best we can.
 *
 * @param source Info on the input source.
 * If invalid, a "NONE" icon will be used.
 * @param condensed If true, only the icon's fundamental information is
 * presented. If false, disambiguation information is included too.
 * For instance, keyboard keys that come in pairs specify whether they are
 * the left or right key, controller inputs specify what controller number
 * it is, etc.
 * @param outText The text to be written inside is returned here, or an
 * empty string is returned if there's nothing to write.
 * @param outExtra Any extra information worth writing before the icon is
 * returned here.
 */
void HardwareMediator::getIconInfoFromScratch(
    const Inpution::InputSource& source, bool condensed,
    string* outText, string* outExtra
) const {
    if(
        source.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_BUTTON
    ) {
        *outText = (condensed ? "M" : "Mouse ") + i2s(source.buttonNr);
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_UP
    ) {
        *outText = (condensed ? "MWU" : "Mouse wheel up");
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_DOWN
    ) {
        *outText = (condensed ? "MWD" : "Mouse wheel down");
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_LEFT
    ) {
        *outText = (condensed ? "MWL" : "Mouse wheel left");
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_MOUSE_WHEEL_RIGHT
    ) {
        *outText = (condensed ? "MWL" : "Mouse wheel right");
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_KEYBOARD_KEY
    ) {
        string name = al_keycode_to_name(source.buttonNr);
        for(size_t c = 0; c < name.size(); c++) {
            if(name[c] == '_') name[c] = ' ';
        }
        *outText = strToTitle(name);
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_BUTTON
    ) {
        *outText = (condensed ? "" : "Button ") + i2s(source.buttonNr);
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG
    ) {
        *outText = (condensed ? "S" : "Stick ") + i2s(source.stickNr + 1);
        if(source.axisNr == 0) {
            *outText += (condensed ? " U" : " up");
        } else if(source.axisNr == 1) {
            *outText += (condensed ? " L" : " left");
        } else {
            *outText +=
                (condensed ? " A" : " axis ") +
                i2s(source.axisNr + 1) + "-";
        }
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS
    ) {
        *outText = (condensed ? "S" : "Stick ") + i2s(source.stickNr + 1);
        if(source.axisNr == 0) {
            *outText += (condensed ? " D" : " down");
        } else if(source.axisNr == 1) {
            *outText += (condensed ? " R" : " right");
        } else {
            *outText +=
                (condensed ? " A" : " axis ") +
                i2s(source.axisNr + 1) + "+";
        }
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_ANALOG_BUTTON
    ) {
        *outText = (condensed ? "T" : "Trigger ") + i2s(source.stickNr + 1);
        *outText +=
            (condensed ? " A" : " axis ") +
            i2s(source.axisNr + 1);
            
    }
}


/**
 * @brief Returns misc. information about how a player
 * input source's icon should be drawn.
 *
 * @param source Info on the input source.
 * @param condensed If true, only the icon's fundamental information is
 * presented. If false, disambiguation information is included too.
 * For instance, keyboard keys that come in pairs specify whether they are
 * the left or right key, controller inputs specify what controller number
 * it is, etc.
 * @param outShape The shape is returned here.
 * @param outExtra Any extra information worth writing before the icon is
 * returned here.
 */
void HardwareMediator::getIconInfoMisc(
    const Inpution::InputSource& source, bool condensed,
    PLAYER_INPUT_ICON_SHAPE* outShape, string* outExtra
) const {
    //Assume rounded by default.
    *outShape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
    
    if(
        source.type == Inpution::INPUT_SOURCE_TYPE_KEYBOARD_KEY
    ) {
        *outShape = PLAYER_INPUT_ICON_SHAPE_RECTANGLE;
        if(!condensed) {
            if(source.buttonNr == ALLEGRO_KEY_LSHIFT) {
                *outExtra = "Left";
            } else if(source.buttonNr == ALLEGRO_KEY_RSHIFT) {
                *outExtra = "Right";
            } else if(source.buttonNr == ALLEGRO_KEY_LCTRL) {
                *outExtra = "Left";
            } else if(source.buttonNr == ALLEGRO_KEY_RCTRL) {
                *outExtra = "Right";
            }
        }
        
    } else if(
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG ||
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS ||
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_BUTTON ||
        source.type == Inpution::INPUT_SOURCE_TYPE_CONTROLLER_ANALOG_BUTTON
    ) {
        if(!condensed) *outExtra = "Pad " + i2s(source.deviceNr + 1);
        
    }
}


/**
 * @brief Returns information about how a player input source's icon should
 * be drawn.
 *
 * @param source Info on the input source.
 * If invalid, a "NONE" icon will be used.
 * @param condensed If true, only the icon's fundamental information is
 * presented. If false, disambiguation information is included too.
 * For instance, keyboard keys that come in pairs specify whether they are
 * the left or right key, controller inputs specify what controller number
 * it is, etc.
 * @param outShape The shape is returned here.
 * @param outText The text to be written inside is returned here, or an
 * empty string is returned if there's nothing to write.
 * @param outBitmapSprite If it's one of the icons in the control bind
 * input icon spritesheet, the index of the sprite is returned here.
 * @param outExtra Any extra information worth writing before the icon is
 * returned here.
 */
void HardwareMediator::getInputSourceIconInfo(
    const Inpution::InputSource& source, bool condensed,
    PLAYER_INPUT_ICON_SHAPE* outShape, string* outText,
    PLAYER_INPUT_ICON_SPRITE* outBitmapSprite,
    string* outExtra
) const {
    //Defaults and error case.
    *outShape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
    *outBitmapSprite = PLAYER_INPUT_ICON_SPRITE_LMB;
    *outText = "(NONE)";
    *outExtra = "";
    
    if(source.type == Inpution::INPUT_SOURCE_TYPE_NONE) return;
    
    //Get some misc. information first.
    getIconInfoMisc(
        source, condensed, outShape, outExtra
    );
    
    //Get database information, if it's in there.
    const InputSourceIcon* dbEntry = getIconDbEntry(source);
    
    if(dbEntry) {
        //Use the data from the database.
        getIconInfoFromDbEntry(
            dbEntry, source, outShape, outText, outBitmapSprite
        );
        
    } else {
        //Not in the database. Describe it as best we can.
        getIconInfoFromScratch(
            source, condensed, outText, outExtra
        );
    }
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The event.
 */
void HardwareMediator::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        //Mouse cursor movement.
        lastInputWasController = false;
        
    } else if(
        ev.type == ALLEGRO_EVENT_KEY_DOWN ||
        ev.type == ALLEGRO_EVENT_KEY_UP ||
        ev.type == ALLEGRO_EVENT_KEY_CHAR
    ) {
        //Keyboard input.
        lastInputWasController = false;
        
    } else if(ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
        //Game controller input.
        if(fabs(ev.joystick.pos) > 0.5f) {
            //Easy deadzone simulation.
            lastInputWasController = true;
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP
    ) {
        //Game controller input.
        lastInputWasController = true;
        
    } else if(
        ev.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION
    ) {
        //Game controller was connected or disconnected.
        updateControllers();
        
    }
}


/**
 * @brief Given an input source, it sanitizes it if necessary, by checking
 * the device brand database and converting absurd button, stick, and
 * axis numbers.
 *
 * @param source Source to sanitize.
 * @return The sanitized source, or the same source if nothing needed changing.
 */
Inpution::InputSource HardwareMediator::sanitizeStick(
    const Inpution::InputSource& source
) const {
    if((size_t) source.deviceNr > controllers.size()) {
        return source;
    }
    
    DEVICE_BRAND brand = controllers[source.deviceNr].brand;
    const auto dbEntryIt = deviceBrandDb.find(brand);
    if(dbEntryIt == deviceBrandDb.end()) return source;
    
    const DeviceBrand* dbEntry = &dbEntryIt->second;
    InputSourceMapEntry old {
        .isButton =
        source.type ==
        Inpution::INPUT_SOURCE_TYPE_CONTROLLER_ANALOG_BUTTON,
        .stickNr = source.stickNr,
        .axisNr = source.axisNr,
    };
    const auto stickMapEntryIt = dbEntry->absurdityMap.find(old);
    if(stickMapEntryIt == dbEntry->absurdityMap.end()) return source;
    
    Inpution::InputSource result = source;
    result.type =
        stickMapEntryIt->second.isButton ?
        Inpution::INPUT_SOURCE_TYPE_CONTROLLER_ANALOG_BUTTON :
        source.type;
    result.stickNr = stickMapEntryIt->second.stickNr;
    result.axisNr = stickMapEntryIt->second.axisNr;
    return result;
}


/**
 * @brief Polls Allegro for the connected game controllers and updates their
 * information.
 *
 * @param silent If true, no system notifications will appear for
 * connected or disconnected controllers.
 */
void HardwareMediator::updateControllers(bool silent) {
    int oldNControllers = (int) controllers.size();
    controllers.clear();
    al_reconfigure_joysticks();
    
    int nControllers = al_get_num_joysticks();
    for(int j = 0; j < nControllers; j++) {
        ALLEGRO_JOYSTICK* aJoyPtr = al_get_joystick(j);
        string name(al_get_joystick_name(aJoyPtr));
        DEVICE_BRAND brand = DEVICE_BRAND_CONTROLLER_UNKOWN;
        
        if(strStartsWith(name, "Nintendo Switch Pro Controller")) {
            brand = DEVICE_BRAND_CONTROLLER_SWITCH_PRO;
        } else if(strStartsWith(name, "Microsoft X-Box 360")) {
            brand = DEVICE_BRAND_CONTROLLER_XBOX_360;
        }
        
        controllers.push_back(
        Controller {
            .aJoyPtr = aJoyPtr,
            .brand = brand
        }
        );
    }
    
    if(!silent) {
        if(oldNControllers < nControllers) {
            game.systemNotifications.add(
                "Controller connected.", false, false
            );
        } else if(oldNControllers > nControllers) {
            game.systemNotifications.add(
                "Controller disconnected!", true, false
            );
            game.states.gameplay->tryPause();
            game.controls.releaseAll();
        }
    }
}
