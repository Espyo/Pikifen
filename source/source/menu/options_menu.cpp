/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Options menu structs and functions.
 */

#include <algorithm>

#include "options_menu.h"

#include "../core/game.h"
#include "../core/load.h"
#include "../core/misc_functions.h"
#include "../util/string_utils.h"
#include "menu.h"


using DrawInfo = GuiItem::DrawInfo;


namespace OPTIONS_MENU {

//Name of the audio menu GUI information file.
const string AUDIO_GUI_FILE_NAME = "options_menu_audio";

//Height of each bind button.
const float BIND_BUTTON_HEIGHT = 0.07f;

//Padding between each bind button.
const float BIND_BUTTON_PADDING = 0.01f;

//Name of the GUI information file.
const string CONTROL_BINDS_GUI_FILE_NAME = "options_menu_control_binds";

//Name of the controls menu GUI information file.
const string CONTROLS_GUI_FILE_NAME = "options_menu_controls";

//Name of the graphics menu GUI information file.
const string GRAPHICS_GUI_FILE_NAME = "options_menu_graphics";

//How long the menu items take to move when switching pages.
const float HUD_MOVE_TIME = 0.5f;

//Timeout before the input capturing cancels.
const float INPUT_CAPTURE_TIMEOUT_DURATION = 5.0f;

//Name of the misc menu GUI information file.
const string MISC_GUI_FILE_NAME = "options_menu_misc";

//Name of the top-level menu GUI information file.
const string TOP_GUI_FILE_NAME = "options_menu_top";

}


/**
 * @brief Chooses the input for a given action type's bind.
 * If the bind index is greater than the number of existing binds for this
 * action type, then a new one gets added.
 *
 * @param actionType Action type.
 * @param bindIdx Index of that action type's bind.
 */
void OptionsMenu::chooseInput(
    const PLAYER_ACTION_TYPE actionType, size_t bindIdx
) {
    capturingInput = 1;
    capturingInputTimeout = OPTIONS_MENU::INPUT_CAPTURE_TIMEOUT_DURATION;
    game.controls.startIgnoringActions();
    
    const vector<ControlBind>& allBinds = game.controls.binds();
    size_t bindsCounted = 0;
    curActionType = actionType;
    curBindIdx = allBinds.size();
    
    for(size_t b = 0; b < allBinds.size(); b++) {
        if(allBinds[b].actionTypeId != actionType) continue;
        if(bindsCounted == bindIdx) {
            curBindIdx = b;
            break;
        } else {
            bindsCounted++;
        }
    }
}


/**
 * @brief Deletes a bind from an action type.
 *
 * @param actionType Action type it belongs to.
 * @param bindIdx Index number of the control.
 */
void OptionsMenu::deleteBind(
    const PLAYER_ACTION_TYPE actionType, size_t bindIdx
) {
    vector<ControlBind>& allBinds = game.controls.binds();
    size_t bindsCounted = 0;
    
    for(size_t b = 0; b < allBinds.size(); b++) {
        if(allBinds[b].actionTypeId != actionType) continue;
        if(bindsCounted == bindIdx) {
            allBinds.erase(allBinds.begin() + b);
            break;
        } else {
            bindsCounted++;
        }
    }
    
    mustPopulateBinds = true;
}


/**
 * @brief Draws the options menu.
 */
void OptionsMenu::draw() {
    Menu::draw();
    if(packsMenu) packsMenu->draw();
    
    if(capturingInput == 1) {
        al_draw_filled_rectangle(
            0, 0, game.winW, game.winH,
            al_map_rgba(24, 24, 32, 192)
        );
        
        drawTextLines(
            "Please perform the new input for \n" +
            game.controls.getPlayerActionType(curActionType).name + "\n"
            "\n"
            "(Or wait " + i2s(capturingInputTimeout + 1) + "s to cancel...)",
            game.sysContent.fntStandard,
            Point(game.winW / 2.0f, game.winH / 2.0f),
            Point(LARGE_FLOAT),
            COLOR_WHITE, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
            TEXT_SETTING_FLAG_CANT_GROW
        );
    }
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The event.
 */
void OptionsMenu::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    if(!active) return;
    
    switch(capturingInput) {
    case 0: {
        //Not capturing.
        break;
    } case 1: {
        //Actively capturing.
        PlayerInput input = game.controls.allegroEventToInput(ev);
        if(input.value >= 0.5f) {
            vector<ControlBind>& allBinds = game.controls.binds();
            if(curBindIdx >= allBinds.size()) {
                ControlBind newBind;
                newBind.actionTypeId = curActionType;
                newBind.playerNr = 0;
                newBind.inputSource = input.source;
                allBinds.push_back(newBind);
            } else {
                game.controls.binds()[curBindIdx].inputSource = input.source;
            }
            capturingInput = 2;
            game.controls.stopIgnoringActions();
            game.controls.startIgnoringInputSource(input.source);
            mustPopulateBinds = true;
        }
        return;
        break;
    } case 2: {
        //One frame of cooldown, so that we don't accidentally feed the
        //input meant for the capture to the GUI.
        return;
        break;
    }
    }
    
    Menu::handleAllegroEvent(ev);
    if(packsMenu) packsMenu->handleAllegroEvent(ev);
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void OptionsMenu::handlePlayerAction(const PlayerAction& action) {
    if(capturingInput != 0) return;
    Menu::handlePlayerAction(action);
    if(packsMenu) packsMenu->handlePlayerAction(action);
}


/**
 * @brief Initializes the audio options menu GUI.
 */
void OptionsMenu::initGuiAudioPage() {
    //Menu items.
    audioGui.registerCoords("back",                  12,  5,   20,  6);
    audioGui.registerCoords("back_input",             3,  7,    4,  4);
    audioGui.registerCoords("header",                50, 10,   50,  6);
    audioGui.registerCoords("master_volume",         50, 25,   70, 10);
    audioGui.registerCoords("gameplay_sound_volume", 50, 37.5, 65, 10);
    audioGui.registerCoords("music_volume",          50, 50,   65, 10);
    audioGui.registerCoords("ambiance_sound_volume", 50, 62.5, 65, 10);
    audioGui.registerCoords("ui_sound_volume",       50, 75,   65, 10);
    audioGui.registerCoords("tooltip",               50, 96,   96,  4);
    audioGui.readCoords(
        game.content.guiDefs.list[OPTIONS_MENU::AUDIO_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    audioGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    audioGui.backItem->onActivate =
    [this] (const Point&) {
        transitionGuis(
            audioGui, topGui, GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    audioGui.backItem->onGetTooltip =
    [] () { return "Return to the top-level options menu."; };
    audioGui.addItem(audioGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&audioGui);
    
    //Header text.
    TextGuiItem* headerText =
        new TextGuiItem(
        "AUDIO OPTIONS",
        game.sysContent.fntAreaName,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    audioGui.addItem(headerText, "header");
    
    vector<float> presetVolumeValues = {
        0.00f, 0.05f, 0.10f, 0.15f, 0.20f, 0.25f, 0.30f, 0.35f, 0.40f, 0.45f,
        0.50f, 0.55f, 0.60f, 0.65f, 0.70f, 0.75f, 0.80f, 0.85f, 0.90f, 0.95f,
        1.0f
    };
    vector<string> presetVolumeNames = {
        "Off", "5%", "10%", "15%", "20%", "25%", "30%", "35%", "40%", "45%",
        "50%", "55%", "60%", "65%", "70%", "75%", "80%", "85%", "90%", "95%",
        "100%"
    };
    auto updateVolumes = [this] () {
        game.audio.updateVolumes(
            game.options.audio.masterVol,
            game.options.audio.gameplaySoundVol,
            game.options.audio.musicVol,
            game.options.audio.ambianceSoundVol,
            game.options.audio.uiSoundVol
        );
    };
    
    //Master volume picker.
    masterVolPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Master volume: ",
        &game.options.audio.masterVol,
        OPTIONS::AUDIO_D::MASTER_VOL,
        presetVolumeValues,
        presetVolumeNames,
        "Volume of the final mix of all audio."
    );
    masterVolPicker->afterChange = updateVolumes;
    masterVolPicker->init();
    audioGui.addItem(masterVolPicker, "master_volume");
    
    //Gameplay sound effects volume picker.
    gameplaySoundVolPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Gameplay sound effects volume: ",
        &game.options.audio.gameplaySoundVol,
        OPTIONS::AUDIO_D::GAMEPLAY_SOUND_VOL,
        presetVolumeValues,
        presetVolumeNames,
        "Volume for in-world gameplay sound effects specifically."
    );
    gameplaySoundVolPicker->afterChange = updateVolumes;
    gameplaySoundVolPicker->init();
    audioGui.addItem(gameplaySoundVolPicker, "gameplay_sound_volume");
    
    //Music volume picker.
    musicVolPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Music volume: ",
        &game.options.audio.musicVol,
        OPTIONS::AUDIO_D::MUSIC_VOL,
        presetVolumeValues,
        presetVolumeNames,
        "Volume for music specifically."
    );
    musicVolPicker->afterChange = updateVolumes;
    musicVolPicker->init();
    audioGui.addItem(musicVolPicker, "music_volume");
    
    //Ambiance sound volume picker.
    ambianceSoundVolPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Ambiance sound effects volume: ",
        &game.options.audio.ambianceSoundVol,
        OPTIONS::AUDIO_D::AMBIANCE_SOUND_VOl,
        presetVolumeValues,
        presetVolumeNames,
        "Volume for in-world ambiance sound effects specifically."
    );
    ambianceSoundVolPicker->afterChange = updateVolumes;
    ambianceSoundVolPicker->init();
    audioGui.addItem(ambianceSoundVolPicker, "ambiance_sound_volume");
    
    //UI sound effects volume picker.
    uiSoundVolPicker =
        new OptionsMenuPickerGuiItem<float>(
        "UI sound effects volume: ",
        &game.options.audio.uiSoundVol,
        OPTIONS::AUDIO_D::UI_SOUND_VOL,
        presetVolumeValues,
        presetVolumeNames,
        "Volume for interface sound effects specifically."
    );
    uiSoundVolPicker->afterChange = updateVolumes;
    uiSoundVolPicker->init();
    audioGui.addItem(uiSoundVolPicker, "ui_sound_volume");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&audioGui);
    audioGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    audioGui.setSelectedItem(masterVolPicker, true);
    audioGui.responsive = false;
    audioGui.hideItems();
}


/**
 * @brief Initializes the control binds options menu GUI.
 */
void OptionsMenu::initGuiControlBindsPage() {
    //Menu items.
    bindsGui.registerCoords("back",        12,  5, 20,  6);
    bindsGui.registerCoords("back_input",   3,  7,  4,  4);
    bindsGui.registerCoords("header",      50,  5, 50,  6);
    bindsGui.registerCoords("list",        50, 51, 88, 82);
    bindsGui.registerCoords("list_scroll", 97, 51,  2, 82);
    bindsGui.registerCoords("tooltip",     50, 96, 96,  4);
    bindsGui.readCoords(
        game.content.guiDefs.list[OPTIONS_MENU::CONTROL_BINDS_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    bindsGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    bindsGui.backItem->onActivate =
    [this] (const Point&) {
        saveOptions();
        saveMakerTools();
        transitionGuis(
            bindsGui, controlsGui, GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    bindsGui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    bindsGui.addItem(bindsGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&bindsGui);
    
    //Header text.
    TextGuiItem* headerText =
        new TextGuiItem(
        "CONTROL BINDS",
        game.sysContent.fntAreaName,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    bindsGui.addItem(headerText, "header");
    
    //Controls list box.
    bindsListBox = new ListGuiItem();
    bindsGui.addItem(bindsListBox, "list");
    
    //Controls list scrollbar.
    ScrollGuiItem* listScroll = new ScrollGuiItem();
    listScroll->listItem = bindsListBox;
    bindsGui.addItem(listScroll, "list_scroll");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&bindsGui);
    bindsGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    bindsGui.setSelectedItem(bindsGui.backItem, true);
    bindsGui.responsive = false;
    bindsGui.hideItems();
    al_reconfigure_joysticks();
}


/**
 * @brief Initializes the controls options menu GUI.
 */
void OptionsMenu::initGuiControlsPage() {
    //Menu items.
    controlsGui.registerCoords("back",          12,    5, 20,  6);
    controlsGui.registerCoords("back_input",     3,    7,  4,  4);
    controlsGui.registerCoords("header",        50,   10, 50,  6);
    controlsGui.registerCoords("normal_binds",  50,   25, 70, 10);
    controlsGui.registerCoords("special_binds", 50, 36.5, 58,  9);
    controlsGui.registerCoords("cursor_speed",  50,   54, 70, 10);
    controlsGui.registerCoords("auto_throw",    50,   70, 70, 10);
    controlsGui.registerCoords("tooltip",       50,   96, 96,  4);
    controlsGui.readCoords(
        game.content.guiDefs.list[OPTIONS_MENU::CONTROLS_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    controlsGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    controlsGui.backItem->onActivate =
    [this] (const Point&) {
        transitionGuis(
            controlsGui, topGui, GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    controlsGui.backItem->onGetTooltip =
    [] () { return "Return to the top-level options menu."; };
    controlsGui.addItem(controlsGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&controlsGui);
    
    //Header text.
    TextGuiItem* headerText =
        new TextGuiItem(
        "CONTROLS OPTIONS",
        game.sysContent.fntAreaName,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    controlsGui.addItem(headerText, "header");
    
    //Normal control binds button.
    ButtonGuiItem* normalBindsButton =
        new ButtonGuiItem(
        "Normal control binds...", game.sysContent.fntStandard
    );
    normalBindsButton->onActivate =
    [this] (const Point&) {
        bindsMenuType = CONTROL_BINDS_MENU_NORMAL;
        mustPopulateBinds = true;
        transitionGuis(
            controlsGui, bindsGui, GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    normalBindsButton->onGetTooltip =
    [] () { return "Choose what buttons do what regular actions."; };
    controlsGui.addItem(normalBindsButton, "normal_binds");
    
    //Special control binds button.
    ButtonGuiItem* specialBindsButton =
        new ButtonGuiItem(
        "Special control binds...", game.sysContent.fntStandard
    );
    specialBindsButton->onActivate =
    [this] (const Point&) {
        bindsMenuType = CONTROL_BINDS_MENU_SPECIAL;
        mustPopulateBinds = true;
        transitionGuis(
            controlsGui, bindsGui, GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    specialBindsButton->onGetTooltip =
    [] () { return "Choose what buttons do what special features."; };
    controlsGui.addItem(specialBindsButton, "special_binds");
    
    //Cursor speed.
    cursorSpeedPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Cursor speed: ",
        &game.options.controls.cursorSpeed,
        OPTIONS::CONTROLS_D::CURSOR_SPEED,
    {250.0f, 350.0f, 500.0f, 700.0f, 1000.0f},
    {"Very slow", "Slow", "Medium", "Fast", "Very fast"},
    "Cursor speed, when controlling without a mouse."
    );
    cursorSpeedPicker->valueToString = [] (float v) {
        return f2s(v);
    };
    cursorSpeedPicker->init();
    controlsGui.addItem(cursorSpeedPicker, "cursor_speed");
    
    //Auto-throw mode.
    autoThrowPicker =
        new OptionsMenuPickerGuiItem<AUTO_THROW_MODE>(
        "Auto-throw: ",
        &game.options.controls.autoThrowMode,
        OPTIONS::CONTROLS_D::AUTO_THROW,
    {AUTO_THROW_MODE_OFF, AUTO_THROW_MODE_HOLD, AUTO_THROW_MODE_TOGGLE},
    {"Off", "Hold input", "Input toggles"}
    );
    autoThrowPicker->presetDescriptions = {
        "Pikmin are only thrown when you release the throw input.",
        "Auto-throw Pikmin periodically as long as the throw input is held.",
        "Do the throw input once to auto-throw periodically, and again to stop."
    };
    autoThrowPicker->init();
    controlsGui.addItem(autoThrowPicker, "auto_throw");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&controlsGui);
    controlsGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    controlsGui.setSelectedItem(normalBindsButton, true);
    controlsGui.responsive = false;
    controlsGui.hideItems();
}


/**
 * @brief Initializes the graphics options menu GUI.
 */
void OptionsMenu::initGuiGraphicsPage() {
    //Menu items.
    graphicsGui.registerCoords("back",            12,  5,   20,  6);
    graphicsGui.registerCoords("back_input",       3,  7,    4,  4);
    graphicsGui.registerCoords("header",          50, 10,   50,  6);
    graphicsGui.registerCoords("fullscreen",      50, 25,   70, 10);
    graphicsGui.registerCoords("resolution",      50, 42.5, 70, 10);
    graphicsGui.registerCoords("tooltip",         50, 96,   96,  4);
    graphicsGui.registerCoords("restart_warning", 50, 85,   70,  6);
    graphicsGui.readCoords(
        game.content.guiDefs.list[OPTIONS_MENU::GRAPHICS_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    graphicsGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    graphicsGui.backItem->onActivate =
    [this] (const Point&) {
        transitionGuis(
            graphicsGui, topGui, GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    graphicsGui.backItem->onGetTooltip =
    [] () { return "Return to the top-level options menu."; };
    graphicsGui.addItem(graphicsGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&graphicsGui);
    
    //Header text.
    TextGuiItem* headerText =
        new TextGuiItem(
        "GRAPHICS OPTIONS",
        game.sysContent.fntAreaName,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    graphicsGui.addItem(headerText, "header");
    
    //Fullscreen checkbox.
    CheckGuiItem* fullscreenCheck =
        new CheckGuiItem(
        &game.options.graphics.intendedWinFullscreen,
        "Fullscreen", game.sysContent.fntStandard
    );
    fullscreenCheck->onActivate =
    [this, fullscreenCheck] (const Point&) {
        fullscreenCheck->defActivateCode();
        triggerRestartWarning();
    };
    fullscreenCheck->onGetTooltip =
    [] () {
        return
            "Show the game in fullscreen, or in a window? Default: " +
            b2s(OPTIONS::GRAPHICS_D::WIN_FULLSCREEN) + ".";
    };
    graphicsGui.addItem(fullscreenCheck, "fullscreen");
    
    //Resolution picker.
    vector<string> resolutionPresetNames;
    for(size_t p = 0; p < resolutionPresets.size(); p++) {
        resolutionPresetNames.push_back(
            i2s(resolutionPresets[p].first) + "x" +
            i2s(resolutionPresets[p].second)
        );
    }
    curResolutionOption.first = game.options.graphics.intendedWinW;
    curResolutionOption.second = game.options.graphics.intendedWinH;
    resolutionPicker =
        new OptionsMenuPickerGuiItem<std::pair<int, int> >(
        "Resolution: ",
        &curResolutionOption,
        std::make_pair(OPTIONS::GRAPHICS_D::WIN_W, OPTIONS::GRAPHICS_D::WIN_H),
        resolutionPresets,
        resolutionPresetNames,
        "The game's width and height."
    );
    resolutionPicker->afterChange = [this] () {
        game.options.graphics.intendedWinW = curResolutionOption.first;
        game.options.graphics.intendedWinH = curResolutionOption.second;
        triggerRestartWarning();
    };
    resolutionPicker->valueToString = [] (const std::pair<int, int>& v) {
        return i2s(v.first) + "x" + i2s(v.second);
    };
    resolutionPicker->init();
    graphicsGui.addItem(resolutionPicker, "resolution");
    
    //Warning text.
    warningText =
        new TextGuiItem(
        "Please leave this menu and then restart for the "
        "changes to take effect.",
        game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_CENTER
    );
    warningText->visible = false;
    graphicsGui.addItem(warningText, "restart_warning");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&graphicsGui);
    graphicsGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    graphicsGui.setSelectedItem(fullscreenCheck, true);
    graphicsGui.responsive = false;
    graphicsGui.hideItems();
}


/**
 * @brief Initializes the misc. options menu GUI.
 */
void OptionsMenu::initGuiMiscPage() {
    //Menu items.
    miscGui.registerCoords("back",                   12,    5, 20,  6);
    miscGui.registerCoords("back_input",              3,    7,  4,  4);
    miscGui.registerCoords("header",                 50,   10, 50,  6);
    miscGui.registerCoords("pikmin_bump",            50,   25, 70, 10);
    miscGui.registerCoords("cursor_cam_weight",      50, 37.5, 70, 10);
    miscGui.registerCoords("show_counter_on_cursor", 50,   50, 70, 10);
    miscGui.registerCoords("show_hud_input_icons",   50, 62.5, 70, 10);
    miscGui.registerCoords("leaving_confirmation",   50,   75, 70, 10);
    miscGui.registerCoords("tooltip",                50,   96, 96,  4);
    miscGui.readCoords(
        game.content.guiDefs.list[OPTIONS_MENU::MISC_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    miscGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    miscGui.backItem->onActivate =
    [this] (const Point&) {
        transitionGuis(
            miscGui, topGui, GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    miscGui.backItem->onGetTooltip =
    [] () { return "Return to the top-level options menu."; };
    miscGui.addItem(miscGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&miscGui);
    
    //Header text.
    TextGuiItem* headerText =
        new TextGuiItem(
        "MISC. OPTIONS",
        game.sysContent.fntAreaName,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    miscGui.addItem(headerText, "header");
    
    //Cursor camera weight.
    cursorCamWeightPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Cursor cam weight: ",
        &game.options.misc.cursorCamWeight,
        OPTIONS::MISC_D::CURSOR_CAM_WEIGHT,
    {0.0f, 0.1f, 0.3f, 0.6f},
    {"None", "Small", "Medium", "Large"},
    "When you move the cursor, how much does it affect the camera?"
    );
    cursorCamWeightPicker->valueToString = [] (float v) {
        return f2s(v);
    };
    cursorCamWeightPicker->init();
    miscGui.addItem(cursorCamWeightPicker, "cursor_cam_weight");
    
    //Show counter on cursor checkbox.
    CheckGuiItem* showCounterOnCursorCheck =
        new CheckGuiItem(
        &game.options.misc.showCounterOnCursor,
        "Show counter on cursor", game.sysContent.fntStandard
    );
    showCounterOnCursorCheck->onGetTooltip =
    [] () {
        return
            "Show a standby type counter on the leader's cursor? "
            "Default: " + b2s(OPTIONS::MISC_D::SHOW_COUNTER_ON_CURSOR) + ".";
    };
    miscGui.addItem(showCounterOnCursorCheck, "show_counter_on_cursor");
    
    //Show HUD player input icons checkbox.
    CheckGuiItem* showHudinputIconsCheck =
        new CheckGuiItem(
        &game.options.misc.showHudInputIcons,
        "Show input icons on HUD", game.sysContent.fntStandard
    );
    showHudinputIconsCheck->onGetTooltip =
    [] () {
        return
            "Show icons of the player inputs near relevant HUD items? "
            "Default: " + b2s(OPTIONS::MISC_D::SHOW_HUD_INPUT_ICONS) + ".";
    };
    miscGui.addItem(showHudinputIconsCheck, "show_hud_input_icons");
    
    //Leaving confirmation mode.
    leavingConfirmationPicker =
        new OptionsMenuPickerGuiItem<LEAVING_CONF_MODE>(
        "Leave confirm: ",
        &game.options.misc.leavingConfMode,
    OPTIONS::MISC_D::LEAVING_CONF, {
        LEAVING_CONF_MODE_ALWAYS,
        LEAVING_CONF_MODE_1_MIN,
        LEAVING_CONF_MODE_NEVER
    },
    {"Always", "After 1min", "Never"}
    );
    leavingConfirmationPicker->presetDescriptions = {
        "When leaving from the pause menu, always ask to confirm.",
        "When leaving from the pause menu, only ask to confirm "
        "if one minute has passed.",
        "When leaving from the pause menu, never ask to confirm."
    };
    leavingConfirmationPicker->init();
    miscGui.addItem(leavingConfirmationPicker, "leaving_confirmation");
    
    //Pikmin bump mode.
    vector<float> presetPikminBumpValues = {
        0.0f, 25.0f, 50.0f
    };
    vector<string> presetPikminBumpNames = {
        "Touch", "Very close", "Nearby"
    };
    pikminBumpPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Pikmin bumping: ",
        &game.options.misc.pikminBumpDist,
        OPTIONS::MISC_D::PIKMIN_BUMP_DIST,
        presetPikminBumpValues, presetPikminBumpNames
    );
    pikminBumpPicker->presetDescriptions = {
        "Idle Pikmin only join the leader when they are touching.",
        "Idle Pikmin join the leader when they are very close.",
        "Idle Pikmin join the leader when they are somewhat nearby."
    };
    pikminBumpPicker->init();
    miscGui.addItem(pikminBumpPicker, "pikmin_bump");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&miscGui);
    miscGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    miscGui.setSelectedItem(cursorCamWeightPicker, true);
    miscGui.responsive = false;
    miscGui.hideItems();
}


/**
 * @brief Initializes the top-level menu GUI.
 */
void OptionsMenu::initGuiTopPage() {
    DataNode* guiFile =
        &game.content.guiDefs.list[OPTIONS_MENU::TOP_GUI_FILE_NAME];
        
    //Button icon positions.
    DataNode* iconsNode = guiFile->getChildByName("icons_to_the_left");
    
#define iconLeft(name, def) s2b(iconsNode->getChildByName(name)-> \
    getValueOrDefault(def))
    
    bool controlsIconLeft = iconLeft("controls", "true");
    bool graphicsIconLeft = iconLeft("graphics", "true");
    bool audioIconLeft = iconLeft("audio", "true");
    bool packsIconLeft = iconLeft("packs", "true");
    bool miscIconLeft = iconLeft("misc", "true");
    
#undef iconLeft
    
    //Menu items.
    topGui.registerCoords("back",       12,  5, 20,  6);
    topGui.registerCoords("back_input",  3,  7,  4,  4);
    topGui.registerCoords("header",     50, 10, 50,  6);
    topGui.registerCoords("controls",   50, 25, 65, 10);
    topGui.registerCoords("graphics",   50, 37, 65, 10);
    topGui.registerCoords("audio",      50, 49, 65, 10);
    topGui.registerCoords("packs",      50, 61, 65, 10);
    topGui.registerCoords("misc",       50, 73, 60, 10);
    topGui.registerCoords("advanced",   87, 86, 22,  8);
    topGui.registerCoords("tooltip",    50, 96, 96,  4);
    topGui.readCoords(guiFile->getChildByName("positions"));
    
    //Back button.
    topGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    topGui.backItem->onActivate =
    [this] (const Point&) {
        saveOptions();
        leave();
    };
    topGui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    topGui.addItem(topGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&topGui);
    
    //Header text.
    TextGuiItem* headerText =
        new TextGuiItem(
        "OPTIONS",
        game.sysContent.fntAreaName,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    topGui.addItem(headerText, "header");
    
    //Controls options button.
    ButtonGuiItem* controlsButton =
        new ButtonGuiItem("Controls", game.sysContent.fntStandard);
    controlsButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_CONTROLS, draw.center, draw.size, controlsIconLeft
        );
        drawButton(
            draw.center, draw.size,
            controlsButton->text, controlsButton->font,
            controlsButton->color, controlsButton->selected,
            controlsButton->getJuiceValue()
        );
    };
    controlsButton->onActivate =
    [this] (const Point&) {
        transitionGuis(
            topGui, controlsGui, GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    controlsButton->onGetTooltip =
    [] () { return "Change the way you control the game."; };
    topGui.addItem(controlsButton, "controls");
    
    //Graphics options button.
    ButtonGuiItem* graphicsButton =
        new ButtonGuiItem("Graphics", game.sysContent.fntStandard);
    graphicsButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_GRAPHICS, draw.center, draw.size, graphicsIconLeft
        );
        drawButton(
            draw.center, draw.size,
            graphicsButton->text, graphicsButton->font,
            graphicsButton->color, graphicsButton->selected,
            graphicsButton->getJuiceValue()
        );
    };
    graphicsButton->onActivate =
    [this] (const Point&) {
        transitionGuis(
            topGui, graphicsGui, GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    graphicsButton->onGetTooltip =
    [] () { return "Change some options about how the game looks."; };
    topGui.addItem(graphicsButton, "graphics");
    
    //Audio options button.
    ButtonGuiItem* audioButton =
        new ButtonGuiItem("Audio", game.sysContent.fntStandard);
    audioButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_AUDIO, draw.center, draw.size, audioIconLeft
        );
        drawButton(
            draw.center, draw.size,
            audioButton->text, audioButton->font,
            audioButton->color, audioButton->selected,
            audioButton->getJuiceValue()
        );
    };
    audioButton->onActivate =
    [this] (const Point&) {
        transitionGuis(
            topGui, audioGui, GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    audioButton->onGetTooltip =
    [] () { return "Change options about the way the game sounds."; };
    topGui.addItem(audioButton, "audio");
    
    //Packs options button.
    ButtonGuiItem* packsButton =
        new ButtonGuiItem("Packs", game.sysContent.fntStandard);
    packsButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_PACKS, draw.center, draw.size, packsIconLeft
        );
        drawButton(
            draw.center, draw.size,
            packsButton->text, packsButton->font,
            packsButton->color, packsButton->selected,
            packsButton->getJuiceValue()
        );
    };
    packsButton->onActivate =
    [this] (const Point&) {
        packsMenu = new PacksMenu();
        packsMenu->leaveCallback = [ = ] () {
            packsMenu->unloadTimer = OPTIONS_MENU::HUD_MOVE_TIME;
            transitionGuis(
                packsMenu->gui, topGui, GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
                OPTIONS_MENU::HUD_MOVE_TIME
            );
        };
        packsMenu->load();
        packsMenu->enter();
        transitionGuis(
            topGui, packsMenu->gui, GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    packsButton->onGetTooltip =
    [] () { return "Manage any content packs you have installed."; };
    topGui.addItem(packsButton, "packs");
    
    //Misc. options button.
    ButtonGuiItem* miscButton =
        new ButtonGuiItem("Misc.", game.sysContent.fntStandard);
    miscButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_OPTIONS_MISC, draw.center, draw.size, miscIconLeft
        );
        drawButton(
            draw.center, draw.size,
            miscButton->text, miscButton->font,
            miscButton->color, miscButton->selected,
            miscButton->getJuiceValue()
        );
    };
    miscButton->onActivate =
    [this] (const Point&) {
        transitionGuis(
            topGui, miscGui, GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    miscButton->onGetTooltip =
    [] () { return "Change some miscellaneous gameplay and game options."; };
    topGui.addItem(miscButton, "misc");
    
    //Advanced bullet point.
    BulletGuiItem* advancedBullet =
        new BulletGuiItem("Advanced...", game.sysContent.fntStandard);
    advancedBullet->onActivate =
    [] (const Point&) {
        openManual("options.html");
    };
    advancedBullet->onGetTooltip =
    [] () {
        return
            "Click to open the manual (in the game's folder) for info "
            "on advanced options.";
    };
    topGui.addItem(advancedBullet, "advanced");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&topGui);
    topGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    topGui.setSelectedItem(controlsButton, true);
}


/**
 * @brief Loads the menu.
 */
void OptionsMenu::load() {
    //Let's fill in the list of preset resolutions. For that, we'll get
    //the display modes fetched by Allegro. These are usually nice round
    //resolutions, and they work on fullscreen mode.
    int nModes = al_get_num_display_modes();
    for(int d = 0; d < nModes; d++) {
        ALLEGRO_DISPLAY_MODE dInfo;
        if(!al_get_display_mode(d, &dInfo)) continue;
        if(dInfo.width < SMALLEST_WIN_WIDTH) continue;
        if(dInfo.height < SMALLEST_WIN_HEIGHT) continue;
        resolutionPresets.push_back(
            std::make_pair(dInfo.width, dInfo.height)
        );
    }
    
    //In case things go wrong, at least add these presets.
    resolutionPresets.push_back(
        std::make_pair(OPTIONS::GRAPHICS_D::WIN_W, OPTIONS::GRAPHICS_D::WIN_H)
    );
    resolutionPresets.push_back(
        std::make_pair(SMALLEST_WIN_WIDTH, SMALLEST_WIN_HEIGHT)
    );
    
    //Sort the list.
    sort(
        resolutionPresets.begin(), resolutionPresets.end(),
    [] (std::pair<int, int> p1, std::pair<int, int> p2) -> bool {
        if(p1.first == p2.first) {
            return p1.second < p2.second;
        }
        return p1.first < p2.first;
    }
    );
    
    //Remove any duplicates.
    for(size_t p = 0; p < resolutionPresets.size() - 1;) {
        if(resolutionPresets[p] == resolutionPresets[p + 1]) {
            resolutionPresets.erase(resolutionPresets.begin() + (p + 1));
        } else {
            p++;
        }
    }
    
    //Init the GUIs.
    initGuiTopPage();
    initGuiControlsPage();
    initGuiControlBindsPage();
    initGuiGraphicsPage();
    initGuiAudioPage();
    initGuiMiscPage();
    
    //Finish the menu class setup.
    guis.push_back(&topGui);
    guis.push_back(&controlsGui);
    guis.push_back(&bindsGui);
    guis.push_back(&graphicsGui);
    guis.push_back(&audioGui);
    guis.push_back(&miscGui);
    Menu::load();
}


/**
 * @brief Populates the list of binds.
 */
void OptionsMenu::populateBinds() {
    GuiItem* itemToSelect = nullptr;
    
    unordered_set<PLAYER_ACTION_CAT> allowedCategories;
    switch(bindsMenuType) {
    case CONTROL_BINDS_MENU_NORMAL: {
        allowedCategories = {
            PLAYER_ACTION_CAT_MAIN,
            PLAYER_ACTION_CAT_MENUS,
            PLAYER_ACTION_CAT_ADVANCED,
        };
        break;
    } case CONTROL_BINDS_MENU_SPECIAL: {
        allowedCategories = {
            PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS,
            PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
            PLAYER_ACTION_CAT_SYSTEM,
        };
        break;
    }
    }
    
    bindsListBox->deleteAllChildren();
    
    const vector<PfePlayerActionType>& allPlayerActionTypes =
        game.controls.getAllPlayerActionTypes();
    vector<ControlBind>& allBinds = game.controls.binds();
    
    bindsPerActionType.clear();
    bindsPerActionType.assign(
        allPlayerActionTypes.size(), vector<ControlBind>()
    );
    
    //Read all binds and sort them by player action type.
    for(size_t b = 0; b < allBinds.size(); b++) {
        const ControlBind& bind = allBinds[b];
        if(bind.playerNr != 0) continue;
        bindsPerActionType[bind.actionTypeId].push_back(bind);
    }
    
    PLAYER_ACTION_CAT lastCat = PLAYER_ACTION_CAT_NONE;
    
    for(size_t a = 0; a < allPlayerActionTypes.size(); a++) {
        const PfePlayerActionType& actionType = allPlayerActionTypes[a];
        
        if(actionType.internalName.empty()) continue;
        if(!isInContainer(allowedCategories, actionType.category)) continue;
        
        float actionY =
            bindsListBox->getChildBottom() +
            OPTIONS_MENU::BIND_BUTTON_PADDING;
            
        if(actionType.category != lastCat) {
        
            //Section header text.
            string sectionName;
            switch(actionType.category) {
            case PLAYER_ACTION_CAT_NONE: {
                break;
            } case PLAYER_ACTION_CAT_MAIN: {
                sectionName = "Main";
                break;
            } case PLAYER_ACTION_CAT_MENUS: {
                sectionName = "Menus";
                break;
            } case PLAYER_ACTION_CAT_ADVANCED: {
                sectionName = "Advanced";
                break;
            } case PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS: {
                sectionName = "General maker tools";
                break;
            } case PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS: {
                sectionName = "Gameplay maker tools";
                break;
            } case PLAYER_ACTION_CAT_SYSTEM: {
                sectionName = "System";
                break;
            }
            }
            TextGuiItem* sectionText =
                new TextGuiItem(sectionName, game.sysContent.fntAreaName);
            sectionText->ratioCenter =
                Point(
                    0.50f,
                    actionY + OPTIONS_MENU::BIND_BUTTON_HEIGHT / 2.0f
                );
            sectionText->ratioSize =
                Point(0.50f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bindsListBox->addChild(sectionText);
            bindsGui.addItem(sectionText);
            
            actionY =
                bindsListBox->getChildBottom() +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
            lastCat = actionType.category;
            
        }
        
        float curY = actionY + OPTIONS_MENU::BIND_BUTTON_HEIGHT / 2.0f;
        
        //Action type name bullet.
        BulletGuiItem* nameBullet =
            new BulletGuiItem(actionType.name, game.sysContent.fntStandard);
        nameBullet->ratioCenter =
            Point(0.22f, curY);
        nameBullet->ratioSize =
            Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
        nameBullet->onGetTooltip =
        [actionType] () { return actionType.description; };
        bindsListBox->addChild(nameBullet);
        bindsGui.addItem(nameBullet);
        
        //More button.
        ButtonGuiItem* moreButton =
            new ButtonGuiItem("...", game.sysContent.fntStandard);
        moreButton->onActivate =
        [this, actionType] (const Point&) {
            if(showingBindsMore && actionType.id == curActionType) {
                showingBindsMore = false;
            } else {
                curActionType = actionType.id;
                showingBindsMore = true;
            }
            mustPopulateBinds = true;
        };
        moreButton->ratioCenter =
            Point(0.92f, curY);
        moreButton->ratioSize =
            Point(0.05f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
        string tooltip =
            (showingBindsMore && actionType.id == curActionType) ?
            "Hide options." :
            "Show information and options for this action.";
        moreButton->onGetTooltip =
        [tooltip] () { return tooltip; };
        bindsListBox->addChild(moreButton);
        bindsGui.addItem(moreButton);
        if(actionType.id == curActionType) {
            itemToSelect = moreButton;
        }
        
        vector<ControlBind> aBinds = bindsPerActionType[actionType.id];
        for(size_t b = 0; b < aBinds.size(); b++) {
        
            //Change bind button.
            ButtonGuiItem* bindButton =
                new ButtonGuiItem("", game.sysContent.fntStandard);
            bindButton->onActivate =
            [this, actionType, b] (const Point&) {
                chooseInput(actionType.id, b);
            };
            bindButton->onDraw =
                [this, b, aBinds, bindButton]
            (const DrawInfo & draw) {
                drawPlayerInputSourceIcon(
                    game.sysContent.fntSlim, aBinds[b].inputSource, false,
                    draw.center, draw.size * 0.8f
                );
                
                drawButton(
                    draw.center, draw.size,
                    "", game.sysContent.fntStandard, COLOR_WHITE,
                    bindButton->selected,
                    bindButton->getJuiceValue()
                );
            };
            bindButton->ratioCenter =
                Point(0.63f, curY);
            bindButton->ratioSize =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bindButton->onGetTooltip =
            [] () { return "Change the input for this action."; };
            bindsListBox->addChild(bindButton);
            bindsGui.addItem(bindButton);
            
            if(showingBindsMore && actionType.id == curActionType) {
                //Remove bind button.
                ButtonGuiItem* removeBindButton =
                    new ButtonGuiItem("", game.sysContent.fntStandard);
                removeBindButton->onActivate =
                [this, actionType, b] (const Point&) {
                    deleteBind(actionType.id, b);
                };
                removeBindButton->onDraw =
                    [this, removeBindButton]
                (const DrawInfo & draw) {
                    drawButton(
                        draw.center, draw.size, "X",
                        game.sysContent.fntStandard, COLOR_WHITE,
                        removeBindButton->selected,
                        removeBindButton->getJuiceValue()
                    );
                };
                removeBindButton->ratioCenter =
                    Point(0.85f, curY);
                removeBindButton->ratioSize =
                    Point(0.05f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
                removeBindButton->onGetTooltip =
                [] () { return "Remove this input from this action."; };
                bindsListBox->addChild(removeBindButton);
                bindsGui.addItem(removeBindButton);
                removeBindButton->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                );
            }
            
            if(actionType.id == curActionType) {
                bindButton->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
            }
            
            curY +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(aBinds.empty()) {
        
            //Add first bind button.
            ButtonGuiItem* bindButton =
                new ButtonGuiItem("", game.sysContent.fntStandard);
            bindButton->onActivate =
            [this, actionType] (const Point&) {
                chooseInput(actionType.id, 0);
            };
            bindButton->onDraw =
                [this, bindButton]
            (const DrawInfo & draw) {
                drawButton(
                    draw.center, draw.size, "",
                    game.sysContent.fntStandard, COLOR_WHITE,
                    bindButton->selected,
                    bindButton->getJuiceValue()
                );
            };
            bindButton->ratioCenter =
                Point(0.63f, curY);
            bindButton->ratioSize =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bindButton->onGetTooltip =
            [] () { return "Choose an input for this action."; };
            bindsListBox->addChild(bindButton);
            bindsGui.addItem(bindButton);
            bindButton->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            curY +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        } else if(showingBindsMore && actionType.id == curActionType) {
        
            //Add button.
            ButtonGuiItem* addButton =
                new ButtonGuiItem("Add...", game.sysContent.fntStandard);
            addButton->ratioCenter =
                Point(0.63f, curY);
            addButton->ratioSize =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            addButton->onActivate =
            [this, actionType, aBinds] (const Point&) {
                chooseInput(actionType.id, aBinds.size());
            };
            addButton->onGetTooltip =
            [] () { return "Add another input to this action."; };
            bindsListBox->addChild(addButton);
            bindsGui.addItem(addButton);
            addButton->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
            );
            
            curY +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(showingBindsMore && actionType.id == curActionType) {
        
            //Restore default button.
            ButtonGuiItem* restoreButton =
                new ButtonGuiItem(
                "Restore defaults", game.sysContent.fntStandard
            );
            restoreButton->ratioCenter =
                Point(0.63f, curY);
            restoreButton->ratioSize =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            restoreButton->onActivate =
            [this, actionType] (const Point&) {
                restoreDefaultBinds(actionType.id);
            };
            restoreButton->onGetTooltip =
            [] () { return "Restore this action's default inputs."; };
            bindsListBox->addChild(restoreButton);
            bindsGui.addItem(restoreButton);
            restoreButton->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            curY +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
            //Default label.
            TextGuiItem* defaultLabelText =
                new TextGuiItem(
                "Default:", game.sysContent.fntStandard,
                COLOR_WHITE, ALLEGRO_ALIGN_LEFT
            );
            defaultLabelText->ratioCenter =
                Point(0.63f, curY);
            defaultLabelText->ratioSize =
                Point(0.30f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bindsListBox->addChild(defaultLabelText);
            bindsGui.addItem(defaultLabelText);
            defaultLabelText->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            //Default icon.
            PlayerInputSource defInputSource =
                game.controls.strToInputSource(actionType.defaultBindStr);
            GuiItem* defaultIcon = new GuiItem();
            defaultIcon->ratioCenter =
                Point(0.68f, curY);
            defaultIcon->ratioSize =
                Point(0.17f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            defaultIcon->onDraw =
            [defInputSource] (const DrawInfo & draw) {
                drawPlayerInputSourceIcon(
                    game.sysContent.fntSlim, defInputSource,
                    false, draw.center, draw.size
                );
            };
            bindsListBox->addChild(defaultIcon);
            bindsGui.addItem(defaultIcon);
            
        }
        
        //Spacer line.
        GuiItem* line = new GuiItem();
        line->ratioCenter =
            Point(
                0.50f, bindsListBox->getChildBottom() + 0.02f
            );
        line->ratioSize = Point(0.90f, 0.02f);
        line->onDraw =
        [] (const DrawInfo & draw) {
            al_draw_line(
                draw.center.x - draw.size.x / 2.0f,
                draw.center.y,
                draw.center.x + draw.size.x / 2.0f,
                draw.center.y,
                COLOR_TRANSPARENT_WHITE,
                1.0f
            );
        };
        bindsListBox->addChild(line);
        bindsGui.addItem(line);
    }
    
    if(itemToSelect) {
        bindsGui.setSelectedItem(itemToSelect, true);
        //Try to center it.
        bindsListBox->onChildDirSelected(itemToSelect);
    }
}


/**
 * @brief Restores the default binds for a given player action.
 *
 * @param actionTypeId Action type ID of the action to restore.
 */
void OptionsMenu::restoreDefaultBinds(
    const PLAYER_ACTION_TYPE actionTypeId
) {
    const PfePlayerActionType& actionType =
        game.controls.getPlayerActionType(actionTypeId);
    vector<ControlBind>& allBinds =
        game.controls.binds();
        
    for(size_t b = 0; b < allBinds.size();) {
        if(
            allBinds[b].playerNr == 0 &&
            allBinds[b].actionTypeId == actionTypeId
        ) {
            allBinds.erase(allBinds.begin() + b);
        } else {
            b++;
        }
    }
    
    PlayerInputSource defInputSource =
        game.controls.strToInputSource(actionType.defaultBindStr);
    ControlBind newBind;
    
    if(defInputSource.type != INPUT_SOURCE_TYPE_NONE) {
        newBind.actionTypeId = actionTypeId;
        newBind.playerNr = 0;
        newBind.inputSource = defInputSource;
        allBinds.push_back(newBind);
    }
    
    showingBindsMore = false;
    mustPopulateBinds = true;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void OptionsMenu::tick(float deltaT) {
    Menu::tick(deltaT);
    
    if(mustPopulateBinds) {
        populateBinds();
        mustPopulateBinds = false;
    }
    
    //Tick the GUIs.
    if(packsMenu) {
        if(packsMenu->loaded) {
            packsMenu->tick(game.deltaT);
        }
        if(!packsMenu->loaded) {
            delete packsMenu;
            packsMenu = nullptr;
        }
    }
    
    //Input capturing logic.
    if(capturingInput == 1) {
        capturingInputTimeout -= game.deltaT;
        if(capturingInputTimeout <= 0.0f) {
            //Timed out. Cancel.
            capturingInput = 0;
            game.controls.stopIgnoringActions();
        }
    } else if(capturingInput == 2) {
        //A frame has passed in the post-capture cooldown. Finish the cooldown.
        capturingInput = 0;
    }
}


/**
 * @brief Triggers the restart warning.
 */
void OptionsMenu::triggerRestartWarning() {
    if(!warningText->visible) {
        warningText->visible = true;
        warningText->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
    }
}


/**
 * @brief Unloads the menu.
 */
void OptionsMenu::unload() {
    if(packsMenu) {
        packsMenu->unload();
        delete packsMenu;
        packsMenu = nullptr;
    }
    
    Menu::unload();
}
