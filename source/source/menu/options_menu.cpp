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

#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../core/load.h"
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
 * @param action_type Action type.
 * @param bind_idx Index of that action type's bind.
 */
void OptionsMenu::chooseInput(
    const PLAYER_ACTION_TYPE action_type, size_t bind_idx
) {
    capturingInput = 1;
    capturingInputTimeout = OPTIONS_MENU::INPUT_CAPTURE_TIMEOUT_DURATION;
    game.controls.startIgnoringActions();
    
    const vector<ControlBind> &all_binds = game.controls.binds();
    size_t binds_counted = 0;
    curActionType = action_type;
    curBindIdx = all_binds.size();
    
    for(size_t b = 0; b < all_binds.size(); b++) {
        if(all_binds[b].actionTypeId != action_type) continue;
        if(binds_counted == bind_idx) {
            curBindIdx = b;
            break;
        } else {
            binds_counted++;
        }
    }
}


/**
 * @brief Deletes a bind from an action type.
 *
 * @param action_type Action type it belongs to.
 * @param bind_idx Index number of the control.
 */
void OptionsMenu::deleteBind(
    const PLAYER_ACTION_TYPE action_type, size_t bind_idx
) {
    vector<ControlBind> &all_binds = game.controls.binds();
    size_t binds_counted = 0;
    
    for(size_t b = 0; b < all_binds.size(); b++) {
        if(all_binds[b].actionTypeId != action_type) continue;
        if(binds_counted == bind_idx) {
            all_binds.erase(all_binds.begin() + b);
            break;
        } else {
            binds_counted++;
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
void OptionsMenu::handleAllegroEvent(const ALLEGRO_EVENT &ev) {
    if(!active) return;
    
    switch(capturingInput) {
    case 0: {
        //Not capturing.
        break;
    } case 1: {
        //Actively capturing.
        PlayerInput input = game.controls.allegroEventToInput(ev);
        if(input.value >= 0.5f) {
            vector<ControlBind> &all_binds = game.controls.binds();
            if(curBindIdx >= all_binds.size()) {
                ControlBind new_bind;
                new_bind.actionTypeId = curActionType;
                new_bind.playerNr = 0;
                new_bind.inputSource = input.source;
                all_binds.push_back(new_bind);
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
void OptionsMenu::handlePlayerAction(const PlayerAction &action) {
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
    [this] (const Point &) {
        audioGui.responsive = false;
        audioGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        topGui.responsive = true;
        topGui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    audioGui.backItem->onGetTooltip =
    [] () { return "Return to the top-level options menu."; };
    audioGui.addItem(audioGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&audioGui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "AUDIO OPTIONS",
        game.sysContent.fntAreaName, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    audioGui.addItem(header_text, "header");
    
    vector<float> preset_volume_values = {
        0.00f, 0.05f, 0.10f, 0.15f, 0.20f, 0.25f, 0.30f, 0.35f, 0.40f, 0.45f,
        0.50f, 0.55f, 0.60f, 0.65f, 0.70f, 0.75f, 0.80f, 0.85f, 0.90f, 0.95f,
        1.0f
    };
    vector<string> preset_volume_names = {
        "Off", "5%", "10%", "15%", "20%", "25%", "30%", "35%", "40%", "45%",
        "50%", "55%", "60%", "65%", "70%", "75%", "80%", "85%", "90%", "95%",
        "100%"
    };
    auto update_volumes = [this] () {
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
        preset_volume_values,
        preset_volume_names,
        "Volume of the final mix of all audio."
    );
    masterVolPicker->afterChange = update_volumes;
    masterVolPicker->init();
    audioGui.addItem(masterVolPicker, "master_volume");
    
    //Gameplay sound effects volume picker.
    gameplaySoundVolPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Gameplay sound effects volume: ",
        &game.options.audio.gameplaySoundVol,
        OPTIONS::AUDIO_D::GAMEPLAY_SOUND_VOL,
        preset_volume_values,
        preset_volume_names,
        "Volume for in-world gameplay sound effects specifically."
    );
    gameplaySoundVolPicker->afterChange = update_volumes;
    gameplaySoundVolPicker->init();
    audioGui.addItem(gameplaySoundVolPicker, "gameplay_sound_volume");
    
    //Music volume picker.
    musicVolPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Music volume: ",
        &game.options.audio.musicVol,
        OPTIONS::AUDIO_D::MUSIC_VOL,
        preset_volume_values,
        preset_volume_names,
        "Volume for music specifically."
    );
    musicVolPicker->afterChange = update_volumes;
    musicVolPicker->init();
    audioGui.addItem(musicVolPicker, "music_volume");
    
    //Ambiance sound volume picker.
    ambianceSoundVolPicker =
        new OptionsMenuPickerGuiItem<float>(
        "Ambiance sound effects volume: ",
        &game.options.audio.ambianceSoundVol,
        OPTIONS::AUDIO_D::AMBIANCE_SOUND_VOl,
        preset_volume_values,
        preset_volume_names,
        "Volume for in-world ambiance sound effects specifically."
    );
    ambianceSoundVolPicker->afterChange = update_volumes;
    ambianceSoundVolPicker->init();
    audioGui.addItem(ambianceSoundVolPicker, "ambiance_sound_volume");
    
    //UI sound effects volume picker.
    uiSoundVolPicker =
        new OptionsMenuPickerGuiItem<float>(
        "UI sound effects volume: ",
        &game.options.audio.uiSoundVol,
        OPTIONS::AUDIO_D::UI_SOUND_VOL,
        preset_volume_values,
        preset_volume_names,
        "Volume for interface sound effects specifically."
    );
    uiSoundVolPicker->afterChange = update_volumes;
    uiSoundVolPicker->init();
    audioGui.addItem(uiSoundVolPicker, "ui_sound_volume");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&audioGui);
    audioGui.addItem(tooltip_text, "tooltip");
    
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
    [this] (const Point &) {
        bindsGui.responsive = false;
        bindsGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        controlsGui.responsive = true;
        controlsGui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        saveOptions();
        saveMakerTools();
    };
    bindsGui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    bindsGui.addItem(bindsGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&bindsGui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "CONTROL BINDS",
        game.sysContent.fntAreaName, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    bindsGui.addItem(header_text, "header");
    
    //Controls list box.
    bindsListBox = new ListGuiItem();
    bindsGui.addItem(bindsListBox, "list");
    
    //Controls list scrollbar.
    ScrollGuiItem* list_scroll = new ScrollGuiItem();
    list_scroll->listItem = bindsListBox;
    bindsGui.addItem(list_scroll, "list_scroll");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&bindsGui);
    bindsGui.addItem(tooltip_text, "tooltip");
    
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
    [this] (const Point &) {
        controlsGui.responsive = false;
        controlsGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        topGui.responsive = true;
        topGui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    controlsGui.backItem->onGetTooltip =
    [] () { return "Return to the top-level options menu."; };
    controlsGui.addItem(controlsGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&controlsGui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "CONTROLS OPTIONS",
        game.sysContent.fntAreaName, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    controlsGui.addItem(header_text, "header");
    
    //Normal control binds button.
    ButtonGuiItem* normal_binds_button =
        new ButtonGuiItem("Normal control binds...", game.sysContent.fntStandard);
    normal_binds_button->onActivate =
    [this] (const Point &) {
        bindsMenuType = CONTROL_BINDS_MENU_NORMAL;
        controlsGui.responsive = false;
        controlsGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        bindsGui.responsive = true;
        bindsGui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        mustPopulateBinds = true;
    };
    normal_binds_button->onGetTooltip =
    [] () { return "Choose what buttons do what regular actions."; };
    controlsGui.addItem(normal_binds_button, "normal_binds");
    
    //Special control binds button.
    ButtonGuiItem* special_binds_button =
        new ButtonGuiItem("Special control binds...", game.sysContent.fntStandard);
    special_binds_button->onActivate =
    [this] (const Point &) {
        bindsMenuType = CONTROL_BINDS_MENU_SPECIAL;
        controlsGui.responsive = false;
        controlsGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        bindsGui.responsive = true;
        bindsGui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        mustPopulateBinds = true;
    };
    special_binds_button->onGetTooltip =
    [] () { return "Choose what buttons do what special features."; };
    controlsGui.addItem(special_binds_button, "special_binds");
    
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
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&controlsGui);
    controlsGui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    controlsGui.setSelectedItem(normal_binds_button, true);
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
    [this] (const Point &) {
        graphicsGui.responsive = false;
        graphicsGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        topGui.responsive = true;
        topGui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    graphicsGui.backItem->onGetTooltip =
    [] () { return "Return to the top-level options menu."; };
    graphicsGui.addItem(graphicsGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&graphicsGui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "GRAPHICS OPTIONS",
        game.sysContent.fntAreaName, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    graphicsGui.addItem(header_text, "header");
    
    //Fullscreen checkbox.
    CheckGuiItem* fullscreen_check =
        new CheckGuiItem(
        &game.options.graphics.intendedWinFullscreen,
        "Fullscreen", game.sysContent.fntStandard
    );
    fullscreen_check->onActivate =
    [this, fullscreen_check] (const Point &) {
        fullscreen_check->defActivateCode();
        triggerRestartWarning();
    };
    fullscreen_check->onGetTooltip =
    [] () {
        return
            "Show the game in fullscreen, or in a window? Default: " +
            b2s(OPTIONS::GRAPHICS_D::WIN_FULLSCREEN) + ".";
    };
    graphicsGui.addItem(fullscreen_check, "fullscreen");
    
    //Resolution picker.
    vector<string> resolution_preset_names;
    for(size_t p = 0; p < resolutionPresets.size(); p++) {
        resolution_preset_names.push_back(
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
        resolution_preset_names,
        "The game's width and height."
    );
    resolutionPicker->afterChange = [this] () {
        game.options.graphics.intendedWinW = curResolutionOption.first;
        game.options.graphics.intendedWinH = curResolutionOption.second;
        triggerRestartWarning();
    };
    resolutionPicker->valueToString = [] (const std::pair<int, int> &v) {
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
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&graphicsGui);
    graphicsGui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    graphicsGui.setSelectedItem(fullscreen_check, true);
    graphicsGui.responsive = false;
    graphicsGui.hideItems();
}


/**
 * @brief Initializes the misc. options menu GUI.
 */
void OptionsMenu::initGuiMiscPage() {
    //Menu items.
    miscGui.registerCoords("back",                 12,  5,   20,  6);
    miscGui.registerCoords("back_input",            3,  7,    4,  4);
    miscGui.registerCoords("header",               50, 10,   50,  6);
    miscGui.registerCoords("cursor_cam_weight",    50, 22.5, 70, 10);
    miscGui.registerCoords("show_hud_input_icons", 50, 40,   70, 10);
    miscGui.registerCoords("leaving_confirmation", 50, 57.5, 70, 10);
    miscGui.registerCoords("tooltip",              50, 96,   96,  4);
    miscGui.readCoords(
        game.content.guiDefs.list[OPTIONS_MENU::MISC_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    miscGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    miscGui.backItem->onActivate =
    [this] (const Point &) {
        miscGui.responsive = false;
        miscGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        topGui.responsive = true;
        topGui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    miscGui.backItem->onGetTooltip =
    [] () { return "Return to the top-level options menu."; };
    miscGui.addItem(miscGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&miscGui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "MISC. OPTIONS",
        game.sysContent.fntAreaName, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    miscGui.addItem(header_text, "header");
    
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
    
    //Show HUD player input icons checkbox.
    CheckGuiItem* show_hud_input_icons_check =
        new CheckGuiItem(
        &game.options.misc.showHudInputIcons,
        "Show input icons on HUD", game.sysContent.fntStandard
    );
    show_hud_input_icons_check->onGetTooltip =
    [] () {
        return
            "Show icons of the player inputs near relevant HUD items? "
            "Default: " + b2s(OPTIONS::MISC_D::SHOW_HUD_INPUT_ICONS) + ".";
    };
    miscGui.addItem(show_hud_input_icons_check, "show_hud_input_icons");
    
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
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&miscGui);
    miscGui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    miscGui.setSelectedItem(cursorCamWeightPicker, true);
    miscGui.responsive = false;
    miscGui.hideItems();
}


/**
 * @brief Initializes the top-level menu GUI.
 */
void OptionsMenu::initGuiTopPage() {
    DataNode* gui_file = &game.content.guiDefs.list[OPTIONS_MENU::TOP_GUI_FILE_NAME];
    
    //Button icon positions.
    DataNode* icons_node = gui_file->getChildByName("icons_to_the_left");
    
#define icon_left(name, def) s2b(icons_node->getChildByName(name)-> \
                                 getValueOrDefault(def))
    
    bool controls_icon_left = icon_left("controls", "true");
    bool graphics_icon_left = icon_left("graphics", "true");
    bool audio_icon_left = icon_left("audio", "true");
    bool packs_icon_left = icon_left("packs", "true");
    bool misc_icon_left = icon_left("misc", "true");
    
#undef icon_left
    
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
    topGui.readCoords(gui_file->getChildByName("positions"));
    
    //Back button.
    topGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    topGui.backItem->onActivate =
    [this] (const Point &) {
        saveOptions();
        leave();
    };
    topGui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    topGui.addItem(topGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&topGui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "OPTIONS",
        game.sysContent.fntAreaName, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    topGui.addItem(header_text, "header");
    
    //Controls options button.
    ButtonGuiItem* controls_button =
        new ButtonGuiItem("Controls", game.sysContent.fntStandard);
    controls_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_CONTROLS, draw.center, draw.size, controls_icon_left
        );
        drawButton(
            draw.center, draw.size,
            controls_button->text, controls_button->font,
            controls_button->color, controls_button->selected,
            controls_button->getJuiceValue()
        );
    };
    controls_button->onActivate =
    [this] (const Point &) {
        topGui.responsive = false;
        topGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        controlsGui.responsive = true;
        controlsGui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    controls_button->onGetTooltip =
    [] () { return "Change the way you control the game."; };
    topGui.addItem(controls_button, "controls");
    
    //Graphics options button.
    ButtonGuiItem* graphics_button =
        new ButtonGuiItem("Graphics", game.sysContent.fntStandard);
    graphics_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_GRAPHICS, draw.center, draw.size, graphics_icon_left
        );
        drawButton(
            draw.center, draw.size,
            graphics_button->text, graphics_button->font,
            graphics_button->color, graphics_button->selected,
            graphics_button->getJuiceValue()
        );
    };
    graphics_button->onActivate =
    [this] (const Point &) {
        topGui.responsive = false;
        topGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        graphicsGui.responsive = true;
        graphicsGui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    graphics_button->onGetTooltip =
    [] () { return "Change some options about how the game looks."; };
    topGui.addItem(graphics_button, "graphics");
    
    //Audio options button.
    ButtonGuiItem* audio_button =
        new ButtonGuiItem("Audio", game.sysContent.fntStandard);
    audio_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_AUDIO, draw.center, draw.size, audio_icon_left
        );
        drawButton(
            draw.center, draw.size,
            audio_button->text, audio_button->font,
            audio_button->color, audio_button->selected,
            audio_button->getJuiceValue()
        );
    };
    audio_button->onActivate =
    [this] (const Point &) {
        topGui.responsive = false;
        topGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        audioGui.responsive = true;
        audioGui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    audio_button->onGetTooltip =
    [] () { return "Change options about the way the game sounds."; };
    topGui.addItem(audio_button, "audio");
    
    //Packs options button.
    ButtonGuiItem* packs_button =
        new ButtonGuiItem("Packs", game.sysContent.fntStandard);
    packs_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_PACKS, draw.center, draw.size, packs_icon_left
        );
        drawButton(
            draw.center, draw.size,
            packs_button->text, packs_button->font,
            packs_button->color, packs_button->selected,
            packs_button->getJuiceValue()
        );
    };
    packs_button->onActivate =
    [this] (const Point &) {
        topGui.responsive = false;
        topGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        packsMenu = new PacksMenu();
        packsMenu->gui.responsive = true;
        packsMenu->gui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        packsMenu->leaveCallback = [ = ] () {
            packsMenu->unloadTimer = OPTIONS_MENU::HUD_MOVE_TIME;
            packsMenu->gui.responsive = false;
            packsMenu->gui.startAnimation(
                GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
                OPTIONS_MENU::HUD_MOVE_TIME
            );
            topGui.responsive = true;
            topGui.startAnimation(
                GUI_MANAGER_ANIM_LEFT_TO_CENTER,
                OPTIONS_MENU::HUD_MOVE_TIME
            );
        };
        packsMenu->load();
        packsMenu->enter();
    };
    packs_button->onGetTooltip =
    [] () { return "Manage any content packs you have installed."; };
    topGui.addItem(packs_button, "packs");
    
    //Misc. options button.
    ButtonGuiItem* misc_button =
        new ButtonGuiItem("Misc.", game.sysContent.fntStandard);
    misc_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_OPTIONS_MISC, draw.center, draw.size, misc_icon_left
        );
        drawButton(
            draw.center, draw.size,
            misc_button->text, misc_button->font,
            misc_button->color, misc_button->selected,
            misc_button->getJuiceValue()
        );
    };
    misc_button->onActivate =
    [this] (const Point &) {
        topGui.responsive = false;
        topGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        miscGui.responsive = true;
        miscGui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    misc_button->onGetTooltip =
    [] () { return "Change some miscellaneous gameplay and game options."; };
    topGui.addItem(misc_button, "misc");
    
    //Advanced bullet point.
    BulletGuiItem* advanced_bullet =
        new BulletGuiItem("Advanced...", game.sysContent.fntStandard);
    advanced_bullet->onActivate =
    [] (const Point &) {
        openManual("options.html");
    };
    advanced_bullet->onGetTooltip =
    [] () {
        return
            "Click to open the manual (in the game's folder) for info "
            "on advanced options.";
    };
    topGui.addItem(advanced_bullet, "advanced");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&topGui);
    topGui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    topGui.setSelectedItem(controls_button, true);
}


/**
 * @brief Loads the menu.
 */
void OptionsMenu::load() {
    //Let's fill in the list of preset resolutions. For that, we'll get
    //the display modes fetched by Allegro. These are usually nice round
    //resolutions, and they work on fullscreen mode.
    int n_modes = al_get_num_display_modes();
    for(int d = 0; d < n_modes; d++) {
        ALLEGRO_DISPLAY_MODE d_info;
        if(!al_get_display_mode(d, &d_info)) continue;
        if(d_info.width < SMALLEST_WIN_WIDTH) continue;
        if(d_info.height < SMALLEST_WIN_HEIGHT) continue;
        resolutionPresets.push_back(
            std::make_pair(d_info.width, d_info.height)
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
    GuiItem* item_to_select = nullptr;
    
    unordered_set<PLAYER_ACTION_CAT> allowed_categories;
    switch(bindsMenuType) {
    case CONTROL_BINDS_MENU_NORMAL: {
        allowed_categories = {
            PLAYER_ACTION_CAT_MAIN,
            PLAYER_ACTION_CAT_MENUS,
            PLAYER_ACTION_CAT_ADVANCED,
        };
        break;
    } case CONTROL_BINDS_MENU_SPECIAL: {
        allowed_categories = {
            PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS,
            PLAYER_ACTION_CAT_GLOBAL_MAKER_TOOLS,
            PLAYER_ACTION_CAT_SYSTEM,
        };
        break;
    }
    }
    
    bindsListBox->deleteAllChildren();
    
    const vector<PfePlayerActionType> &all_player_action_types =
        game.controls.getAllPlayerActionTypes();
    vector<ControlBind> &all_binds = game.controls.binds();
    
    bindsPerActionType.clear();
    bindsPerActionType.assign(all_player_action_types.size(), vector<ControlBind>());
    
    //Read all binds and sort them by player action type.
    for(size_t b = 0; b < all_binds.size(); b++) {
        const ControlBind &bind = all_binds[b];
        if(bind.playerNr != 0) continue;
        bindsPerActionType[bind.actionTypeId].push_back(bind);
    }
    
    PLAYER_ACTION_CAT last_cat = PLAYER_ACTION_CAT_NONE;
    
    for(size_t a = 0; a < all_player_action_types.size(); a++) {
        const PfePlayerActionType &action_type = all_player_action_types[a];
        
        if(action_type.internalName.empty()) continue;
        if(!isInContainer(allowed_categories, action_type.category)) continue;
        
        float action_y =
            bindsListBox->getChildBottom() +
            OPTIONS_MENU::BIND_BUTTON_PADDING;
            
        if(action_type.category != last_cat) {
        
            //Section header text.
            string section_name;
            switch(action_type.category) {
            case PLAYER_ACTION_CAT_NONE: {
                break;
            } case PLAYER_ACTION_CAT_MAIN: {
                section_name = "Main";
                break;
            } case PLAYER_ACTION_CAT_MENUS: {
                section_name = "Menus";
                break;
            } case PLAYER_ACTION_CAT_ADVANCED: {
                section_name = "Advanced";
                break;
            } case PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS: {
                section_name = "Gameplay maker tools";
                break;
            } case PLAYER_ACTION_CAT_GLOBAL_MAKER_TOOLS: {
                section_name = "Global maker tools";
                break;
            } case PLAYER_ACTION_CAT_SYSTEM: {
                section_name = "System";
                break;
            }
            }
            TextGuiItem* section_text =
                new TextGuiItem(section_name, game.sysContent.fntAreaName);
            section_text->ratioCenter =
                Point(
                    0.50f,
                    action_y + OPTIONS_MENU::BIND_BUTTON_HEIGHT / 2.0f
                );
            section_text->ratioSize =
                Point(0.50f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bindsListBox->addChild(section_text);
            bindsGui.addItem(section_text);
            
            action_y =
                bindsListBox->getChildBottom() +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
            last_cat = action_type.category;
            
        }
        
        float cur_y = action_y + OPTIONS_MENU::BIND_BUTTON_HEIGHT / 2.0f;
        
        //Action type name bullet.
        BulletGuiItem* name_bullet =
            new BulletGuiItem(action_type.name, game.sysContent.fntStandard);
        name_bullet->ratioCenter =
            Point(0.22f, cur_y);
        name_bullet->ratioSize =
            Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
        name_bullet->onGetTooltip =
        [action_type] () { return action_type.description; };
        bindsListBox->addChild(name_bullet);
        bindsGui.addItem(name_bullet);
        
        //More button.
        ButtonGuiItem* more_button =
            new ButtonGuiItem("...", game.sysContent.fntStandard);
        more_button->onActivate =
        [this, action_type] (const Point &) {
            if(showingBindsMore && action_type.id == curActionType) {
                showingBindsMore = false;
            } else {
                curActionType = action_type.id;
                showingBindsMore = true;
            }
            mustPopulateBinds = true;
        };
        more_button->ratioCenter =
            Point(0.92f, cur_y);
        more_button->ratioSize =
            Point(0.05f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
        string tooltip =
            (showingBindsMore && action_type.id == curActionType) ?
            "Hide options." :
            "Show information and options for this action.";
        more_button->onGetTooltip =
        [tooltip] () { return tooltip; };
        bindsListBox->addChild(more_button);
        bindsGui.addItem(more_button);
        if(action_type.id == curActionType) {
            item_to_select = more_button;
        }
        
        vector<ControlBind> a_binds = bindsPerActionType[action_type.id];
        for(size_t b = 0; b < a_binds.size(); b++) {
        
            //Change bind button.
            ButtonGuiItem* bind_button =
                new ButtonGuiItem("", game.sysContent.fntStandard);
            bind_button->onActivate =
            [this, action_type, b] (const Point &) {
                chooseInput(action_type.id, b);
            };
            bind_button->onDraw =
                [this, b, a_binds, bind_button]
            (const DrawInfo & draw) {
                drawPlayerInputSourceIcon(
                    game.sysContent.fntSlim, a_binds[b].inputSource, false,
                    draw.center, draw.size * 0.8f
                );
                
                drawButton(
                    draw.center, draw.size,
                    "", game.sysContent.fntStandard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->getJuiceValue()
                );
            };
            bind_button->ratioCenter =
                Point(0.63f, cur_y);
            bind_button->ratioSize =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bind_button->onGetTooltip =
            [] () { return "Change the input for this action."; };
            bindsListBox->addChild(bind_button);
            bindsGui.addItem(bind_button);
            
            if(showingBindsMore && action_type.id == curActionType) {
                //Remove bind button.
                ButtonGuiItem* remove_bind_button =
                    new ButtonGuiItem("", game.sysContent.fntStandard);
                remove_bind_button->onActivate =
                [this, action_type, b] (const Point &) {
                    deleteBind(action_type.id, b);
                };
                remove_bind_button->onDraw =
                    [this, remove_bind_button]
                (const DrawInfo & draw) {
                    drawButton(
                        draw.center, draw.size, "X", game.sysContent.fntStandard, COLOR_WHITE,
                        remove_bind_button->selected,
                        remove_bind_button->getJuiceValue()
                    );
                };
                remove_bind_button->ratioCenter =
                    Point(0.85f, cur_y);
                remove_bind_button->ratioSize =
                    Point(0.05f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
                remove_bind_button->onGetTooltip =
                [] () { return "Remove this input from this action."; };
                bindsListBox->addChild(remove_bind_button);
                bindsGui.addItem(remove_bind_button);
                remove_bind_button->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                );
            }
            
            if(action_type.id == curActionType) {
                bind_button->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
            }
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(a_binds.empty()) {
        
            //Add first bind button.
            ButtonGuiItem* bind_button =
                new ButtonGuiItem("", game.sysContent.fntStandard);
            bind_button->onActivate =
            [this, action_type] (const Point &) {
                chooseInput(action_type.id, 0);
            };
            bind_button->onDraw =
                [this, bind_button]
            (const DrawInfo & draw) {
                drawButton(
                    draw.center, draw.size, "", game.sysContent.fntStandard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->getJuiceValue()
                );
            };
            bind_button->ratioCenter =
                Point(0.63f, cur_y);
            bind_button->ratioSize =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bind_button->onGetTooltip =
            [] () { return "Choose an input for this action."; };
            bindsListBox->addChild(bind_button);
            bindsGui.addItem(bind_button);
            bind_button->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        } else if(showingBindsMore && action_type.id == curActionType) {
        
            //Add button.
            ButtonGuiItem* add_button =
                new ButtonGuiItem("Add...", game.sysContent.fntStandard);
            add_button->ratioCenter =
                Point(0.63f, cur_y);
            add_button->ratioSize =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            add_button->onActivate =
            [this, action_type, a_binds] (const Point &) {
                chooseInput(action_type.id, a_binds.size());
            };
            add_button->onGetTooltip =
            [] () { return "Add another input to this action."; };
            bindsListBox->addChild(add_button);
            bindsGui.addItem(add_button);
            add_button->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
            );
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(showingBindsMore && action_type.id == curActionType) {
        
            //Restore default button.
            ButtonGuiItem* restore_button =
                new ButtonGuiItem("Restore defaults", game.sysContent.fntStandard);
            restore_button->ratioCenter =
                Point(0.63f, cur_y);
            restore_button->ratioSize =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            restore_button->onActivate =
            [this, action_type] (const Point &) {
                restoreDefaultBinds(action_type.id);
            };
            restore_button->onGetTooltip =
            [] () { return "Restore this action's default inputs."; };
            bindsListBox->addChild(restore_button);
            bindsGui.addItem(restore_button);
            restore_button->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
            //Default label.
            TextGuiItem* default_label_text =
                new TextGuiItem(
                "Default:", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
            );
            default_label_text->ratioCenter =
                Point(0.63f, cur_y);
            default_label_text->ratioSize =
                Point(0.30f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bindsListBox->addChild(default_label_text);
            bindsGui.addItem(default_label_text);
            default_label_text->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            //Default icon.
            PlayerInputSource def_input_source =
                game.controls.strToInputSource(action_type.defaultBindStr);
            GuiItem* default_icon = new GuiItem();
            default_icon->ratioCenter =
                Point(0.68f, cur_y);
            default_icon->ratioSize =
                Point(0.17f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            default_icon->onDraw =
            [def_input_source] (const DrawInfo & draw) {
                drawPlayerInputSourceIcon(
                    game.sysContent.fntSlim, def_input_source, false, draw.center, draw.size
                );
            };
            bindsListBox->addChild(default_icon);
            bindsGui.addItem(default_icon);
            
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
    
    if(item_to_select) {
        bindsGui.setSelectedItem(item_to_select, true);
        //Try to center it.
        bindsListBox->onChildDirSelected(item_to_select);
    }
}


/**
 * @brief Restores the default binds for a given player action.
 *
 * @param action_type_id Action type ID of the action to restore.
 */
void OptionsMenu::restoreDefaultBinds(
    const PLAYER_ACTION_TYPE action_type_id
) {
    const PfePlayerActionType &action_type =
        game.controls.getPlayerActionType(action_type_id);
    vector<ControlBind> &all_binds =
        game.controls.binds();
        
    for(size_t b = 0; b < all_binds.size();) {
        if(
            all_binds[b].playerNr == 0 &&
            all_binds[b].actionTypeId == action_type_id
        ) {
            all_binds.erase(all_binds.begin() + b);
        } else {
            b++;
        }
    }
    
    PlayerInputSource def_input_source =
        game.controls.strToInputSource(action_type.defaultBindStr);
    ControlBind new_bind;
    
    if(def_input_source.type != INPUT_SOURCE_TYPE_NONE) {
        new_bind.actionTypeId = action_type_id;
        new_bind.playerNr = 0;
        new_bind.inputSource = def_input_source;
        all_binds.push_back(new_bind);
    }
    
    showingBindsMore = false;
    mustPopulateBinds = true;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void OptionsMenu::tick(float delta_t) {
    Menu::tick(delta_t);
    
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
