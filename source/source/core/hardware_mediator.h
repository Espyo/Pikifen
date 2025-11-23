/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the hardware-related classes and functions.
 * This is the mediator between Allegro hardware data and known real-world
 * hardware brands and types, in the context of controls.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>

#include "../lib/inpution/inpution.h"
#include "../util/general_utils.h"


using std::map;
using std::size_t;
using std::string;
using std::vector;


//Distinct brands of hardware device.
enum DEVICE_BRAND {

    //Any keyboard.
    DEVICE_BRAND_KEYBOARD_ANY,
    
    //Any mouse.
    DEVICE_BRAND_MOUSE_ANY,
    
    //Unknown controller.
    DEVICE_BRAND_CONTROLLER_UNKOWN,
    
    //Nintendo Switch Pro Controller.
    DEVICE_BRAND_CONTROLLER_SWITCH_PRO,
    
    //X-Box 360, or Steam Deck.
    DEVICE_BRAND_CONTROLLER_XBOX_360,
    
    //Total amount of device brands.
    DEVICE_BRAND_COUNT,
    
};


//Possible shapes for a player input icon.
enum PLAYER_INPUT_ICON_SHAPE {

    //Doesn't really have a shape, but instead draws a bitmap.
    PLAYER_INPUT_ICON_SHAPE_BITMAP,
    
    //Rectangle shape, representing keyboard keys.
    PLAYER_INPUT_ICON_SHAPE_RECTANGLE,
    
    //Circle/ellipse shape, representing buttons.
    PLAYER_INPUT_ICON_SHAPE_ROUNDED,
    
};


//Player input icon spritesheet sprites.
//The order matches what's in the spritesheet.
enum PLAYER_INPUT_ICON_SPRITE {

    //Left mouse button.
    PLAYER_INPUT_ICON_SPRITE_LMB,
    
    //Right mouse button.
    PLAYER_INPUT_ICON_SPRITE_RMB,
    
    //Middle mouse button.
    PLAYER_INPUT_ICON_SPRITE_MMB,
    
    //Mouse wheel up.
    PLAYER_INPUT_ICON_SPRITE_MWU,
    
    //Mouse wheel down.
    PLAYER_INPUT_ICON_SPRITE_MWD,
    
    //Right key.
    PLAYER_INPUT_ICON_SPRITE_RIGHT,
    
    //Down key.
    PLAYER_INPUT_ICON_SPRITE_DOWN,
    
    //Left key.
    PLAYER_INPUT_ICON_SPRITE_LEFT,
    
    //Up key.
    PLAYER_INPUT_ICON_SPRITE_UP,
    
    //Backspace key.
    PLAYER_INPUT_ICON_SPRITE_BACKSPACE,
    
    //Shift key.
    PLAYER_INPUT_ICON_SPRITE_SHIFT,
    
    //Tab key.
    PLAYER_INPUT_ICON_SPRITE_TAB,
    
    //Enter key.
    PLAYER_INPUT_ICON_SPRITE_ENTER,
    
    //Game controller left stick right.
    PLAYER_INPUT_ICON_SPRITE_L_STICK_RIGHT,
    
    //Game controller left stick down.
    PLAYER_INPUT_ICON_SPRITE_L_STICK_DOWN,
    
    //Game controller left stick left.
    PLAYER_INPUT_ICON_SPRITE_L_STICK_LEFT,
    
    //Game controller left stick up.
    PLAYER_INPUT_ICON_SPRITE_L_STICK_UP,
    
    //Game controller right stick right.
    PLAYER_INPUT_ICON_SPRITE_R_STICK_RIGHT,
    
    //Game controller right stick down.
    PLAYER_INPUT_ICON_SPRITE_R_STICK_DOWN,
    
    //Game controller right stick left.
    PLAYER_INPUT_ICON_SPRITE_R_STICK_LEFT,
    
    //Game controller right stick up.
    PLAYER_INPUT_ICON_SPRITE_R_STICK_UP,
    
    //Game controller D-pad right.
    PLAYER_INPUT_ICON_SPRITE_D_PAD_RIGHT,
    
    //Game controller D-pad down.
    PLAYER_INPUT_ICON_SPRITE_D_PAD_DOWN,
    
    //Game controller D-pad left.
    PLAYER_INPUT_ICON_SPRITE_D_PAD_LEFT,
    
    //Game controller D-pad up.
    PLAYER_INPUT_ICON_SPRITE_D_PAD_UP,
    
    //Game controller left stick click.
    PLAYER_INPUT_ICON_SPRITE_L_STICK_CLICK,
    
    //Game controller right stick click.
    PLAYER_INPUT_ICON_SPRITE_R_STICK_CLICK,
    
    //Nintendo Switch Home button.
    PLAYER_INPUT_ICON_SWITCH_HOME,
    
    //Nintendo Switch Capture button.
    PLAYER_INPUT_ICON_SWITCH_CAPTURE,
    
    //X-Box 360 Start button.
    PLAYER_INPUT_ICON_SPRITE_XBOX_START,
    
    //X-Box 360 Back button.
    PLAYER_INPUT_ICON_SPRITE_XBOX_BACK,
    
    //X-Box 360 Guide button.
    PLAYER_INPUT_ICON_SPRITE_XBOX_GUIDE,
    
};


/**
 * @brief Mediates everything related to hardware, in the context of controls.
 */
struct HardwareMediator {

    //--- Members ---
    
    //True if the last hardware input made from a game controller.
    //False if it was a keyboard, mouse, or other source.
    bool lastInputWasController = false;
    
    //--- Function declarations ---
    
    size_t getControllerNr(ALLEGRO_JOYSTICK* aJoyPtr);
    void getInputSourceIconInfo(
        const Inpution::InputSource& source, bool condensed,
        PLAYER_INPUT_ICON_SHAPE* outShape, string* outText,
        PLAYER_INPUT_ICON_SPRITE* outBitmapSprite,
        string* outExtra
    ) const;
    void handleAllegroEvent(const ALLEGRO_EVENT& ev);
    void updateControllers();
    
    
    private:
    
    //--- Misc. definitions ---
    
    struct Controller {
    
        //--- Members ---
        
        //Allegro joystick pointer.
        ALLEGRO_JOYSTICK* aJoyPtr = nullptr;
        
        //Brand.
        DEVICE_BRAND brand = DEVICE_BRAND_CONTROLLER_UNKOWN;
        
    };
    
    struct InputSource {
    
        //--- Members ---
        
        //Its name.
        string name;
        
        //Icon index in the bitmap, or INVALID for none.
        //For a stick, this is the first of the four directional icons.
        size_t iconIdx = INVALID;
        
    };
    
    struct DeviceBrand {
    
        //--- Members ---
        
        //Buttons.
        map<int, InputSource> buttons;
        
        //Sticks.
        map<int, InputSource> sticks;
        
    };
    
    
    //--- Constants ---
    
    //Database of specific ways to do some input icons.
    static const map<DEVICE_BRAND, DeviceBrand> deviceBrandIconDb;
    
    
    //--- Members ---
    
    //List of connected game controllers.
    vector<Controller> controllers;
    
    
    //--- Function declarations ---
    
    const HardwareMediator::InputSource* getIconDbEntry(
        const Inpution::InputSource& s
    ) const;
    void getIconInfoFromDbEntry(
        const HardwareMediator::InputSource* dbEntry,
        const Inpution::InputSource& source,
        PLAYER_INPUT_ICON_SHAPE* outShape, string* outText,
        PLAYER_INPUT_ICON_SPRITE* outBitmapSprite
    ) const;
    void getIconInfoFromScratch(
        const Inpution::InputSource& source, bool condensed,
        string* outText, string* outExtra
    ) const;
    void getIconInfoMisc(
        const Inpution::InputSource& source, bool condensed,
        PLAYER_INPUT_ICON_SHAPE* outShape, string* outExtra
    ) const;
    
};
