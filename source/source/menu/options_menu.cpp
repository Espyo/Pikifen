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
    capturing_input = 1;
    capturing_input_timeout = OPTIONS_MENU::INPUT_CAPTURE_TIMEOUT_DURATION;
    game.controls.startIgnoringActions();
    
    const vector<ControlBind> &all_binds = game.controls.binds();
    size_t binds_counted = 0;
    cur_action_type = action_type;
    cur_bind_idx = all_binds.size();
    
    for(size_t b = 0; b < all_binds.size(); b++) {
        if(all_binds[b].actionTypeId != action_type) continue;
        if(binds_counted == bind_idx) {
            cur_bind_idx = b;
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
    
    must_populate_binds = true;
}


/**
 * @brief Draws the options menu.
 */
void OptionsMenu::draw() {
    Menu::draw();
    if(packs_menu) packs_menu->draw();
    
    if(capturing_input == 1) {
        al_draw_filled_rectangle(
            0, 0, game.win_w, game.win_h,
            al_map_rgba(24, 24, 32, 192)
        );
        
        drawTextLines(
            "Please perform the new input for \n" +
            game.controls.getPlayerActionType(cur_action_type).name + "\n"
            "\n"
            "(Or wait " + i2s(capturing_input_timeout + 1) + "s to cancel...)",
            game.sys_content.fnt_standard,
            Point(game.win_w / 2.0f, game.win_h / 2.0f),
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
    
    switch(capturing_input) {
    case 0: {
        //Not capturing.
        break;
    } case 1: {
        //Actively capturing.
        PlayerInput input = game.controls.allegroEventToInput(ev);
        if(input.value >= 0.5f) {
            vector<ControlBind> &all_binds = game.controls.binds();
            if(cur_bind_idx >= all_binds.size()) {
                ControlBind new_bind;
                new_bind.actionTypeId = cur_action_type;
                new_bind.playerNr = 0;
                new_bind.inputSource = input.source;
                all_binds.push_back(new_bind);
            } else {
                game.controls.binds()[cur_bind_idx].inputSource = input.source;
            }
            capturing_input = 2;
            game.controls.stopIgnoringActions();
            game.controls.startIgnoringInputSource(input.source);
            must_populate_binds = true;
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
    if(packs_menu) packs_menu->handleAllegroEvent(ev);
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void OptionsMenu::handlePlayerAction(const PlayerAction &action) {
    if(capturing_input != 0) return;
    Menu::handlePlayerAction(action);
    if(packs_menu) packs_menu->handlePlayerAction(action);
}


/**
 * @brief Initializes the audio options menu GUI.
 */
void OptionsMenu::initGuiAudioPage() {
    //Menu items.
    audio_gui.registerCoords("back",                  12,  5,   20,  6);
    audio_gui.registerCoords("back_input",             3,  7,    4,  4);
    audio_gui.registerCoords("header",                50, 10,   50,  6);
    audio_gui.registerCoords("master_volume",         50, 25,   70, 10);
    audio_gui.registerCoords("gameplay_sound_volume", 50, 37.5, 65, 10);
    audio_gui.registerCoords("music_volume",          50, 50,   65, 10);
    audio_gui.registerCoords("ambiance_sound_volume", 50, 62.5, 65, 10);
    audio_gui.registerCoords("ui_sound_volume",       50, 75,   65, 10);
    audio_gui.registerCoords("tooltip",               50, 96,   96,  4);
    audio_gui.readCoords(
        game.content.gui_defs.list[OPTIONS_MENU::AUDIO_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    audio_gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_standard);
    audio_gui.back_item->on_activate =
    [this] (const Point &) {
        audio_gui.responsive = false;
        audio_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        top_gui.responsive = true;
        top_gui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    audio_gui.back_item->on_get_tooltip =
    [] () { return "Return to the top-level options menu."; };
    audio_gui.addItem(audio_gui.back_item, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&audio_gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "AUDIO OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    audio_gui.addItem(header_text, "header");
    
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
            game.options.audio.master_vol,
            game.options.audio.gameplay_sound_vol,
            game.options.audio.music_vol,
            game.options.audio.ambiance_sound_vol,
            game.options.audio.ui_sound_vol
        );
    };
    
    //Master volume picker.
    master_vol_picker =
        new OptionsMenuPickerGuiItem<float>(
        "Master volume: ",
        &game.options.audio.master_vol,
        OPTIONS::AUDIO_D::MASTER_VOL,
        preset_volume_values,
        preset_volume_names,
        "Volume of the final mix of all audio."
    );
    master_vol_picker->after_change = update_volumes;
    master_vol_picker->init();
    audio_gui.addItem(master_vol_picker, "master_volume");
    
    //Gameplay sound effects volume picker.
    gameplay_sound_vol_picker =
        new OptionsMenuPickerGuiItem<float>(
        "Gameplay sound effects volume: ",
        &game.options.audio.gameplay_sound_vol,
        OPTIONS::AUDIO_D::GAMEPLAY_SOUND_VOL,
        preset_volume_values,
        preset_volume_names,
        "Volume for in-world gameplay sound effects specifically."
    );
    gameplay_sound_vol_picker->after_change = update_volumes;
    gameplay_sound_vol_picker->init();
    audio_gui.addItem(gameplay_sound_vol_picker, "gameplay_sound_volume");
    
    //Music volume picker.
    music_vol_picker =
        new OptionsMenuPickerGuiItem<float>(
        "Music volume: ",
        &game.options.audio.music_vol,
        OPTIONS::AUDIO_D::MUSIC_VOL,
        preset_volume_values,
        preset_volume_names,
        "Volume for music specifically."
    );
    music_vol_picker->after_change = update_volumes;
    music_vol_picker->init();
    audio_gui.addItem(music_vol_picker, "music_volume");
    
    //Ambiance sound volume picker.
    ambiance_sound_vol_picker =
        new OptionsMenuPickerGuiItem<float>(
        "Ambiance sound effects volume: ",
        &game.options.audio.ambiance_sound_vol,
        OPTIONS::AUDIO_D::AMBIANCE_SOUND_VOl,
        preset_volume_values,
        preset_volume_names,
        "Volume for in-world ambiance sound effects specifically."
    );
    ambiance_sound_vol_picker->after_change = update_volumes;
    ambiance_sound_vol_picker->init();
    audio_gui.addItem(ambiance_sound_vol_picker, "ambiance_sound_volume");
    
    //UI sound effects volume picker.
    ui_sound_vol_picker =
        new OptionsMenuPickerGuiItem<float>(
        "UI sound effects volume: ",
        &game.options.audio.ui_sound_vol,
        OPTIONS::AUDIO_D::UI_SOUND_VOL,
        preset_volume_values,
        preset_volume_names,
        "Volume for interface sound effects specifically."
    );
    ui_sound_vol_picker->after_change = update_volumes;
    ui_sound_vol_picker->init();
    audio_gui.addItem(ui_sound_vol_picker, "ui_sound_volume");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&audio_gui);
    audio_gui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    audio_gui.setSelectedItem(master_vol_picker, true);
    audio_gui.responsive = false;
    audio_gui.hideItems();
}


/**
 * @brief Initializes the control binds options menu GUI.
 */
void OptionsMenu::initGuiControlBindsPage() {
    //Menu items.
    binds_gui.registerCoords("back",        12,  5, 20,  6);
    binds_gui.registerCoords("back_input",   3,  7,  4,  4);
    binds_gui.registerCoords("header",      50,  5, 50,  6);
    binds_gui.registerCoords("list",        50, 51, 88, 82);
    binds_gui.registerCoords("list_scroll", 97, 51,  2, 82);
    binds_gui.registerCoords("tooltip",     50, 96, 96,  4);
    binds_gui.readCoords(
        game.content.gui_defs.list[OPTIONS_MENU::CONTROL_BINDS_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    binds_gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_standard);
    binds_gui.back_item->on_activate =
    [this] (const Point &) {
        binds_gui.responsive = false;
        binds_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        controls_gui.responsive = true;
        controls_gui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        saveOptions();
        saveMakerTools();
    };
    binds_gui.back_item->on_get_tooltip =
    [] () { return "Return to the previous menu."; };
    binds_gui.addItem(binds_gui.back_item, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&binds_gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "CONTROL BINDS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    binds_gui.addItem(header_text, "header");
    
    //Controls list box.
    binds_list_box = new ListGuiItem();
    binds_gui.addItem(binds_list_box, "list");
    
    //Controls list scrollbar.
    ScrollGuiItem* list_scroll = new ScrollGuiItem();
    list_scroll->list_item = binds_list_box;
    binds_gui.addItem(list_scroll, "list_scroll");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&binds_gui);
    binds_gui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    binds_gui.setSelectedItem(binds_gui.back_item, true);
    binds_gui.responsive = false;
    binds_gui.hideItems();
    al_reconfigure_joysticks();
}


/**
 * @brief Initializes the controls options menu GUI.
 */
void OptionsMenu::initGuiControlsPage() {
    //Menu items.
    controls_gui.registerCoords("back",          12,    5, 20,  6);
    controls_gui.registerCoords("back_input",     3,    7,  4,  4);
    controls_gui.registerCoords("header",        50,   10, 50,  6);
    controls_gui.registerCoords("normal_binds",  50,   25, 70, 10);
    controls_gui.registerCoords("special_binds", 50, 36.5, 58,  9);
    controls_gui.registerCoords("cursor_speed",  50,   54, 70, 10);
    controls_gui.registerCoords("auto_throw",    50,   70, 70, 10);
    controls_gui.registerCoords("tooltip",       50,   96, 96,  4);
    controls_gui.readCoords(
        game.content.gui_defs.list[OPTIONS_MENU::CONTROLS_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    controls_gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_standard);
    controls_gui.back_item->on_activate =
    [this] (const Point &) {
        controls_gui.responsive = false;
        controls_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        top_gui.responsive = true;
        top_gui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    controls_gui.back_item->on_get_tooltip =
    [] () { return "Return to the top-level options menu."; };
    controls_gui.addItem(controls_gui.back_item, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&controls_gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "CONTROLS OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    controls_gui.addItem(header_text, "header");
    
    //Normal control binds button.
    ButtonGuiItem* normal_binds_button =
        new ButtonGuiItem("Normal control binds...", game.sys_content.fnt_standard);
    normal_binds_button->on_activate =
    [this] (const Point &) {
        binds_menu_type = CONTROL_BINDS_MENU_NORMAL;
        controls_gui.responsive = false;
        controls_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        binds_gui.responsive = true;
        binds_gui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        must_populate_binds = true;
    };
    normal_binds_button->on_get_tooltip =
    [] () { return "Choose what buttons do what regular actions."; };
    controls_gui.addItem(normal_binds_button, "normal_binds");
    
    //Special control binds button.
    ButtonGuiItem* special_binds_button =
        new ButtonGuiItem("Special control binds...", game.sys_content.fnt_standard);
    special_binds_button->on_activate =
    [this] (const Point &) {
        binds_menu_type = CONTROL_BINDS_MENU_SPECIAL;
        controls_gui.responsive = false;
        controls_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        binds_gui.responsive = true;
        binds_gui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        must_populate_binds = true;
    };
    special_binds_button->on_get_tooltip =
    [] () { return "Choose what buttons do what special features."; };
    controls_gui.addItem(special_binds_button, "special_binds");
    
    //Cursor speed.
    cursor_speed_picker =
        new OptionsMenuPickerGuiItem<float>(
        "Cursor speed: ",
        &game.options.controls.cursor_speed,
        OPTIONS::CONTROLS_D::CURSOR_SPEED,
    {250.0f, 350.0f, 500.0f, 700.0f, 1000.0f},
    {"Very slow", "Slow", "Medium", "Fast", "Very fast"},
    "Cursor speed, when controlling without a mouse."
    );
    cursor_speed_picker->value_to_string = [] (float v) {
        return f2s(v);
    };
    cursor_speed_picker->init();
    controls_gui.addItem(cursor_speed_picker, "cursor_speed");
    
    //Auto-throw mode.
    auto_throw_picker =
        new OptionsMenuPickerGuiItem<AUTO_THROW_MODE>(
        "Auto-throw: ",
        &game.options.controls.auto_throw_mode,
        OPTIONS::CONTROLS_D::AUTO_THROW,
    {AUTO_THROW_MODE_OFF, AUTO_THROW_MODE_HOLD, AUTO_THROW_MODE_TOGGLE},
    {"Off", "Hold input", "Input toggles"}
    );
    auto_throw_picker->preset_descriptions = {
        "Pikmin are only thrown when you release the throw input.",
        "Auto-throw Pikmin periodically as long as the throw input is held.",
        "Do the throw input once to auto-throw periodically, and again to stop."
    };
    auto_throw_picker->init();
    controls_gui.addItem(auto_throw_picker, "auto_throw");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&controls_gui);
    controls_gui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    controls_gui.setSelectedItem(normal_binds_button, true);
    controls_gui.responsive = false;
    controls_gui.hideItems();
}


/**
 * @brief Initializes the graphics options menu GUI.
 */
void OptionsMenu::initGuiGraphicsPage() {
    //Menu items.
    graphics_gui.registerCoords("back",            12,  5,   20,  6);
    graphics_gui.registerCoords("back_input",       3,  7,    4,  4);
    graphics_gui.registerCoords("header",          50, 10,   50,  6);
    graphics_gui.registerCoords("fullscreen",      50, 25,   70, 10);
    graphics_gui.registerCoords("resolution",      50, 42.5, 70, 10);
    graphics_gui.registerCoords("tooltip",         50, 96,   96,  4);
    graphics_gui.registerCoords("restart_warning", 50, 85,   70,  6);
    graphics_gui.readCoords(
        game.content.gui_defs.list[OPTIONS_MENU::GRAPHICS_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    graphics_gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_standard);
    graphics_gui.back_item->on_activate =
    [this] (const Point &) {
        graphics_gui.responsive = false;
        graphics_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        top_gui.responsive = true;
        top_gui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    graphics_gui.back_item->on_get_tooltip =
    [] () { return "Return to the top-level options menu."; };
    graphics_gui.addItem(graphics_gui.back_item, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&graphics_gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "GRAPHICS OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    graphics_gui.addItem(header_text, "header");
    
    //Fullscreen checkbox.
    CheckGuiItem* fullscreen_check =
        new CheckGuiItem(
        &game.options.graphics.intended_win_fullscreen,
        "Fullscreen", game.sys_content.fnt_standard
    );
    fullscreen_check->on_activate =
    [this, fullscreen_check] (const Point &) {
        fullscreen_check->defActivateCode();
        triggerRestartWarning();
    };
    fullscreen_check->on_get_tooltip =
    [] () {
        return
            "Show the game in fullscreen, or in a window? Default: " +
            b2s(OPTIONS::GRAPHICS_D::WIN_FULLSCREEN) + ".";
    };
    graphics_gui.addItem(fullscreen_check, "fullscreen");
    
    //Resolution picker.
    vector<string> resolution_preset_names;
    for(size_t p = 0; p < resolution_presets.size(); p++) {
        resolution_preset_names.push_back(
            i2s(resolution_presets[p].first) + "x" +
            i2s(resolution_presets[p].second)
        );
    }
    cur_resolution_option.first = game.options.graphics.intended_win_w;
    cur_resolution_option.second = game.options.graphics.intended_win_h;
    resolution_picker =
        new OptionsMenuPickerGuiItem<std::pair<int, int> >(
        "Resolution: ",
        &cur_resolution_option,
        std::make_pair(OPTIONS::GRAPHICS_D::WIN_W, OPTIONS::GRAPHICS_D::WIN_H),
        resolution_presets,
        resolution_preset_names,
        "The game's width and height."
    );
    resolution_picker->after_change = [this] () {
        game.options.graphics.intended_win_w = cur_resolution_option.first;
        game.options.graphics.intended_win_h = cur_resolution_option.second;
        triggerRestartWarning();
    };
    resolution_picker->value_to_string = [] (const std::pair<int, int> &v) {
        return i2s(v.first) + "x" + i2s(v.second);
    };
    resolution_picker->init();
    graphics_gui.addItem(resolution_picker, "resolution");
    
    //Warning text.
    warning_text =
        new TextGuiItem(
        "Please restart for the changes to take effect.",
        game.sys_content.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_CENTER
    );
    warning_text->visible = false;
    graphics_gui.addItem(warning_text, "restart_warning");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&graphics_gui);
    graphics_gui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    graphics_gui.setSelectedItem(fullscreen_check, true);
    graphics_gui.responsive = false;
    graphics_gui.hideItems();
}


/**
 * @brief Initializes the misc. options menu GUI.
 */
void OptionsMenu::initGuiMiscPage() {
    //Menu items.
    misc_gui.registerCoords("back",                 12,  5,   20,  6);
    misc_gui.registerCoords("back_input",            3,  7,    4,  4);
    misc_gui.registerCoords("header",               50, 10,   50,  6);
    misc_gui.registerCoords("cursor_cam_weight",    50, 22.5, 70, 10);
    misc_gui.registerCoords("show_hud_input_icons", 50, 40,   70, 10);
    misc_gui.registerCoords("leaving_confirmation", 50, 57.5, 70, 10);
    misc_gui.registerCoords("tooltip",              50, 96,   96,  4);
    misc_gui.readCoords(
        game.content.gui_defs.list[OPTIONS_MENU::MISC_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    misc_gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_standard);
    misc_gui.back_item->on_activate =
    [this] (const Point &) {
        misc_gui.responsive = false;
        misc_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        top_gui.responsive = true;
        top_gui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    misc_gui.back_item->on_get_tooltip =
    [] () { return "Return to the top-level options menu."; };
    misc_gui.addItem(misc_gui.back_item, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&misc_gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "MISC. OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    misc_gui.addItem(header_text, "header");
    
    //Cursor camera weight.
    cursor_cam_weight_picker =
        new OptionsMenuPickerGuiItem<float>(
        "Cursor cam weight: ",
        &game.options.misc.cursor_cam_weight,
        OPTIONS::MISC_D::CURSOR_CAM_WEIGHT,
    {0.0f, 0.1f, 0.3f, 0.6f},
    {"None", "Small", "Medium", "Large"},
    "When you move the cursor, how much does it affect the camera?"
    );
    cursor_cam_weight_picker->value_to_string = [] (float v) {
        return f2s(v);
    };
    cursor_cam_weight_picker->init();
    misc_gui.addItem(cursor_cam_weight_picker, "cursor_cam_weight");
    
    //Show HUD player input icons checkbox.
    CheckGuiItem* show_hud_input_icons_check =
        new CheckGuiItem(
        &game.options.misc.show_hud_input_icons,
        "Show input icons on HUD", game.sys_content.fnt_standard
    );
    show_hud_input_icons_check->on_get_tooltip =
    [] () {
        return
            "Show icons of the player inputs near relevant HUD items? "
            "Default: " + b2s(OPTIONS::MISC_D::SHOW_HUD_INPUT_ICONS) + ".";
    };
    misc_gui.addItem(show_hud_input_icons_check, "show_hud_input_icons");
    
    //Leaving confirmation mode.
    leaving_confirmation_picker =
        new OptionsMenuPickerGuiItem<LEAVING_CONF_MODE>(
        "Leave confirm: ",
        &game.options.misc.leaving_conf_mode,
    OPTIONS::MISC_D::LEAVING_CONF, {
        LEAVING_CONF_MODE_ALWAYS,
        LEAVING_CONF_MODE_1_MIN,
        LEAVING_CONF_MODE_NEVER
    },
    {"Always", "After 1min", "Never"}
    );
    leaving_confirmation_picker->preset_descriptions = {
        "When leaving from the pause menu, always ask to confirm.",
        "When leaving from the pause menu, only ask to confirm "
        "if one minute has passed.",
        "When leaving from the pause menu, never ask to confirm."
    };
    leaving_confirmation_picker->init();
    misc_gui.addItem(leaving_confirmation_picker, "leaving_confirmation");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&misc_gui);
    misc_gui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    misc_gui.setSelectedItem(cursor_cam_weight_picker, true);
    misc_gui.responsive = false;
    misc_gui.hideItems();
}


/**
 * @brief Initializes the top-level menu GUI.
 */
void OptionsMenu::initGuiTopPage() {
    DataNode* gui_file = &game.content.gui_defs.list[OPTIONS_MENU::TOP_GUI_FILE_NAME];
    
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
    top_gui.registerCoords("back",       12,  5, 20,  6);
    top_gui.registerCoords("back_input",  3,  7,  4,  4);
    top_gui.registerCoords("header",     50, 10, 50,  6);
    top_gui.registerCoords("controls",   50, 25, 65, 10);
    top_gui.registerCoords("graphics",   50, 37, 65, 10);
    top_gui.registerCoords("audio",      50, 49, 65, 10);
    top_gui.registerCoords("packs",      50, 61, 65, 10);
    top_gui.registerCoords("misc",       50, 73, 60, 10);
    top_gui.registerCoords("advanced",   87, 86, 22,  8);
    top_gui.registerCoords("tooltip",    50, 96, 96,  4);
    top_gui.readCoords(gui_file->getChildByName("positions"));
    
    //Back button.
    top_gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_standard);
    top_gui.back_item->on_activate =
    [this] (const Point &) {
        saveOptions();
        leave();
    };
    top_gui.back_item->on_get_tooltip =
    [] () { return "Return to the previous menu."; };
    top_gui.addItem(top_gui.back_item, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&top_gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    top_gui.addItem(header_text, "header");
    
    //Controls options button.
    ButtonGuiItem* controls_button =
        new ButtonGuiItem("Controls", game.sys_content.fnt_standard);
    controls_button->on_draw =
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
    controls_button->on_activate =
    [this] (const Point &) {
        top_gui.responsive = false;
        top_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        controls_gui.responsive = true;
        controls_gui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    controls_button->on_get_tooltip =
    [] () { return "Change the way you control the game."; };
    top_gui.addItem(controls_button, "controls");
    
    //Graphics options button.
    ButtonGuiItem* graphics_button =
        new ButtonGuiItem("Graphics", game.sys_content.fnt_standard);
    graphics_button->on_draw =
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
    graphics_button->on_activate =
    [this] (const Point &) {
        top_gui.responsive = false;
        top_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        graphics_gui.responsive = true;
        graphics_gui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    graphics_button->on_get_tooltip =
    [] () { return "Change some options about how the game looks."; };
    top_gui.addItem(graphics_button, "graphics");
    
    //Audio options button.
    ButtonGuiItem* audio_button =
        new ButtonGuiItem("Audio", game.sys_content.fnt_standard);
    audio_button->on_draw =
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
    audio_button->on_activate =
    [this] (const Point &) {
        top_gui.responsive = false;
        top_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        audio_gui.responsive = true;
        audio_gui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    audio_button->on_get_tooltip =
    [] () { return "Change options about the way the game sounds."; };
    top_gui.addItem(audio_button, "audio");
    
    //Packs options button.
    ButtonGuiItem* packs_button =
        new ButtonGuiItem("Packs", game.sys_content.fnt_standard);
    packs_button->on_draw =
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
    packs_button->on_activate =
    [this] (const Point &) {
        top_gui.responsive = false;
        top_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        packs_menu = new PacksMenu();
        packs_menu->gui.responsive = true;
        packs_menu->gui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        packs_menu->leave_callback = [ = ] () {
            packs_menu->unload_timer = OPTIONS_MENU::HUD_MOVE_TIME;
            packs_menu->gui.responsive = false;
            packs_menu->gui.startAnimation(
                GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
                OPTIONS_MENU::HUD_MOVE_TIME
            );
            top_gui.responsive = true;
            top_gui.startAnimation(
                GUI_MANAGER_ANIM_LEFT_TO_CENTER,
                OPTIONS_MENU::HUD_MOVE_TIME
            );
        };
        packs_menu->load();
        packs_menu->enter();
    };
    packs_button->on_get_tooltip =
    [] () { return "Manage any content packs you have installed."; };
    top_gui.addItem(packs_button, "packs");
    
    //Misc. options button.
    ButtonGuiItem* misc_button =
        new ButtonGuiItem("Misc.", game.sys_content.fnt_standard);
    misc_button->on_draw =
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
    misc_button->on_activate =
    [this] (const Point &) {
        top_gui.responsive = false;
        top_gui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        misc_gui.responsive = true;
        misc_gui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    misc_button->on_get_tooltip =
    [] () { return "Change some miscellaneous gameplay and game options."; };
    top_gui.addItem(misc_button, "misc");
    
    //Advanced bullet point.
    BulletGuiItem* advanced_bullet =
        new BulletGuiItem("Advanced...", game.sys_content.fnt_standard);
    advanced_bullet->on_activate =
    [] (const Point &) {
        openManual("options.html");
    };
    advanced_bullet->on_get_tooltip =
    [] () {
        return
            "Click to open the manual (in the game's folder) for info "
            "on advanced options.";
    };
    top_gui.addItem(advanced_bullet, "advanced");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&top_gui);
    top_gui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    top_gui.setSelectedItem(controls_button, true);
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
        resolution_presets.push_back(
            std::make_pair(d_info.width, d_info.height)
        );
    }
    
    //In case things go wrong, at least add these presets.
    resolution_presets.push_back(
        std::make_pair(OPTIONS::GRAPHICS_D::WIN_W, OPTIONS::GRAPHICS_D::WIN_H)
    );
    resolution_presets.push_back(
        std::make_pair(SMALLEST_WIN_WIDTH, SMALLEST_WIN_HEIGHT)
    );
    
    //Sort the list.
    sort(
        resolution_presets.begin(), resolution_presets.end(),
    [] (std::pair<int, int> p1, std::pair<int, int> p2) -> bool {
        if(p1.first == p2.first) {
            return p1.second < p2.second;
        }
        return p1.first < p2.first;
    }
    );
    
    //Remove any duplicates.
    for(size_t p = 0; p < resolution_presets.size() - 1;) {
        if(resolution_presets[p] == resolution_presets[p + 1]) {
            resolution_presets.erase(resolution_presets.begin() + (p + 1));
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
    guis.push_back(&top_gui);
    guis.push_back(&controls_gui);
    guis.push_back(&binds_gui);
    guis.push_back(&graphics_gui);
    guis.push_back(&audio_gui);
    guis.push_back(&misc_gui);
    Menu::load();
}


/**
 * @brief Populates the list of binds.
 */
void OptionsMenu::populateBinds() {
    unordered_set<PLAYER_ACTION_CAT> allowed_categories;
    switch(binds_menu_type) {
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
    
    binds_list_box->deleteAllChildren();
    
    const vector<PfePlayerActionType> &all_player_action_types =
        game.controls.getAllPlayerActionTypes();
    vector<ControlBind> &all_binds = game.controls.binds();
    
    binds_per_action_type.clear();
    binds_per_action_type.assign(all_player_action_types.size(), vector<ControlBind>());
    
    //Read all binds and sort them by player action type.
    for(size_t b = 0; b < all_binds.size(); b++) {
        const ControlBind &bind = all_binds[b];
        if(bind.playerNr != 0) continue;
        binds_per_action_type[bind.actionTypeId].push_back(bind);
    }
    
    PLAYER_ACTION_CAT last_cat = PLAYER_ACTION_CAT_NONE;
    
    for(size_t a = 0; a < all_player_action_types.size(); a++) {
        const PfePlayerActionType &action_type = all_player_action_types[a];
        
        if(action_type.internal_name.empty()) continue;
        if(!isInContainer(allowed_categories, action_type.category)) continue;
        
        float action_y =
            binds_list_box->getChildBottom() +
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
                new TextGuiItem(section_name, game.sys_content.fnt_area_name);
            section_text->ratio_center =
                Point(
                    0.50f,
                    action_y + OPTIONS_MENU::BIND_BUTTON_HEIGHT / 2.0f
                );
            section_text->ratio_size =
                Point(0.50f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            binds_list_box->addChild(section_text);
            binds_gui.addItem(section_text);
            
            action_y =
                binds_list_box->getChildBottom() +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
            last_cat = action_type.category;
            
        }
        
        float cur_y = action_y + OPTIONS_MENU::BIND_BUTTON_HEIGHT / 2.0f;
        
        //Action type name bullet.
        BulletGuiItem* name_bullet =
            new BulletGuiItem(action_type.name, game.sys_content.fnt_standard);
        name_bullet->ratio_center =
            Point(0.22f, cur_y);
        name_bullet->ratio_size =
            Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
        name_bullet->on_get_tooltip =
        [action_type] () { return action_type.description; };
        binds_list_box->addChild(name_bullet);
        binds_gui.addItem(name_bullet);
        
        //More button.
        ButtonGuiItem* more_button =
            new ButtonGuiItem("...", game.sys_content.fnt_standard);
        more_button->on_activate =
        [this, action_type] (const Point &) {
            if(showing_binds_more && action_type.id == cur_action_type) {
                showing_binds_more = false;
            } else {
                cur_action_type = action_type.id;
                showing_binds_more = true;
            }
            must_populate_binds = true;
        };
        more_button->ratio_center =
            Point(0.92f, cur_y);
        more_button->ratio_size =
            Point(0.05f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
        string tooltip =
            (showing_binds_more && action_type.id == cur_action_type) ?
            "Hide options." :
            "Show information and options for this action.";
        more_button->on_get_tooltip =
        [tooltip] () { return tooltip; };
        binds_list_box->addChild(more_button);
        binds_gui.addItem(more_button);
        if(action_type.id == cur_action_type) {
            binds_gui.setSelectedItem(more_button, true);
        }
        
        vector<ControlBind> a_binds = binds_per_action_type[action_type.id];
        for(size_t b = 0; b < a_binds.size(); b++) {
        
            //Change bind button.
            ButtonGuiItem* bind_button =
                new ButtonGuiItem("", game.sys_content.fnt_standard);
            bind_button->on_activate =
            [this, action_type, b] (const Point &) {
                chooseInput(action_type.id, b);
            };
            bind_button->on_draw =
                [this, b, a_binds, bind_button]
            (const DrawInfo & draw) {
                drawPlayerInputSourceIcon(
                    game.sys_content.fnt_slim, a_binds[b].inputSource, false,
                    draw.center, draw.size * 0.8f
                );
                
                drawButton(
                    draw.center, draw.size,
                    "", game.sys_content.fnt_standard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->getJuiceValue()
                );
            };
            bind_button->ratio_center =
                Point(0.63f, cur_y);
            bind_button->ratio_size =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bind_button->on_get_tooltip =
            [] () { return "Change the input for this action."; };
            binds_list_box->addChild(bind_button);
            binds_gui.addItem(bind_button);
            
            if(showing_binds_more && action_type.id == cur_action_type) {
                //Remove bind button.
                ButtonGuiItem* remove_bind_button =
                    new ButtonGuiItem("", game.sys_content.fnt_standard);
                remove_bind_button->on_activate =
                [this, action_type, b] (const Point &) {
                    deleteBind(action_type.id, b);
                };
                remove_bind_button->on_draw =
                    [this, remove_bind_button]
                (const DrawInfo & draw) {
                    drawButton(
                        draw.center, draw.size, "X", game.sys_content.fnt_standard, COLOR_WHITE,
                        remove_bind_button->selected,
                        remove_bind_button->getJuiceValue()
                    );
                };
                remove_bind_button->ratio_center =
                    Point(0.85f, cur_y);
                remove_bind_button->ratio_size =
                    Point(0.05f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
                remove_bind_button->on_get_tooltip =
                [] () { return "Remove this input from this action."; };
                binds_list_box->addChild(remove_bind_button);
                binds_gui.addItem(remove_bind_button);
                remove_bind_button->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                );
            }
            
            if(action_type.id == cur_action_type) {
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
                new ButtonGuiItem("", game.sys_content.fnt_standard);
            bind_button->on_activate =
            [this, action_type] (const Point &) {
                chooseInput(action_type.id, 0);
            };
            bind_button->on_draw =
                [this, bind_button]
            (const DrawInfo & draw) {
                drawButton(
                    draw.center, draw.size, "", game.sys_content.fnt_standard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->getJuiceValue()
                );
            };
            bind_button->ratio_center =
                Point(0.63f, cur_y);
            bind_button->ratio_size =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bind_button->on_get_tooltip =
            [] () { return "Choose an input for this action."; };
            binds_list_box->addChild(bind_button);
            binds_gui.addItem(bind_button);
            bind_button->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        } else if(showing_binds_more && action_type.id == cur_action_type) {
        
            //Add button.
            ButtonGuiItem* add_button =
                new ButtonGuiItem("Add...", game.sys_content.fnt_standard);
            add_button->ratio_center =
                Point(0.63f, cur_y);
            add_button->ratio_size =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            add_button->on_activate =
            [this, action_type, a_binds] (const Point &) {
                chooseInput(action_type.id, a_binds.size());
            };
            add_button->on_get_tooltip =
            [] () { return "Add another input to this action."; };
            binds_list_box->addChild(add_button);
            binds_gui.addItem(add_button);
            add_button->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
            );
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(showing_binds_more && action_type.id == cur_action_type) {
        
            //Restore default button.
            ButtonGuiItem* restore_button =
                new ButtonGuiItem("Restore defaults", game.sys_content.fnt_standard);
            restore_button->ratio_center =
                Point(0.63f, cur_y);
            restore_button->ratio_size =
                Point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            restore_button->on_activate =
            [this, action_type] (const Point &) {
                restoreDefaultBinds(action_type.id);
            };
            restore_button->on_get_tooltip =
            [] () { return "Restore this action's default inputs."; };
            binds_list_box->addChild(restore_button);
            binds_gui.addItem(restore_button);
            restore_button->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
            //Default label.
            TextGuiItem* default_label_text =
                new TextGuiItem(
                "Default:", game.sys_content.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
            );
            default_label_text->ratio_center =
                Point(0.63f, cur_y);
            default_label_text->ratio_size =
                Point(0.30f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            binds_list_box->addChild(default_label_text);
            binds_gui.addItem(default_label_text);
            default_label_text->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            //Default icon.
            PlayerInputSource def_input_source =
                game.controls.strToInputSource(action_type.default_bind_str);
            GuiItem* default_icon = new GuiItem();
            default_icon->ratio_center =
                Point(0.68f, cur_y);
            default_icon->ratio_size =
                Point(0.17f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            default_icon->on_draw =
            [def_input_source] (const DrawInfo & draw) {
                drawPlayerInputSourceIcon(
                    game.sys_content.fnt_slim, def_input_source, false, draw.center, draw.size
                );
            };
            binds_list_box->addChild(default_icon);
            binds_gui.addItem(default_icon);
            
        }
        
        //Spacer line.
        GuiItem* line = new GuiItem();
        line->ratio_center =
            Point(
                0.50f, binds_list_box->getChildBottom() + 0.02f
            );
        line->ratio_size = Point(0.90f, 0.02f);
        line->on_draw =
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
        binds_list_box->addChild(line);
        binds_gui.addItem(line);
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
        game.controls.strToInputSource(action_type.default_bind_str);
    ControlBind new_bind;
    
    if(def_input_source.type != INPUT_SOURCE_TYPE_NONE) {
        new_bind.actionTypeId = action_type_id;
        new_bind.playerNr = 0;
        new_bind.inputSource = def_input_source;
        all_binds.push_back(new_bind);
    }
    
    showing_binds_more = false;
    must_populate_binds = true;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void OptionsMenu::tick(float delta_t) {
    Menu::tick(delta_t);
    
    if(must_populate_binds) {
        populateBinds();
        must_populate_binds = false;
    }
    
    //Tick the GUIs.
    if(packs_menu) {
        if(packs_menu->loaded) {
            packs_menu->tick(game.delta_t);
        }
        if(!packs_menu->loaded) {
            delete packs_menu;
            packs_menu = nullptr;
        }
    }
    
    //Input capturing logic.
    if(capturing_input == 1) {
        capturing_input_timeout -= game.delta_t;
        if(capturing_input_timeout <= 0.0f) {
            //Timed out. Cancel.
            capturing_input = 0;
        }
    } else if(capturing_input == 2) {
        //A frame has passed in the post-capture cooldown. Finish the cooldown.
        capturing_input = 0;
    }
}


/**
 * @brief Triggers the restart warning at the bottom of the screen.
 */
void OptionsMenu::triggerRestartWarning() {
    if(!warning_text->visible) {
        warning_text->visible = true;
        warning_text->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
    }
}


/**
 * @brief Unloads the menu.
 */
void OptionsMenu::unload() {
    if(packs_menu) {
        packs_menu->unload();
        delete packs_menu;
        packs_menu = nullptr;
    }
    
    Menu::unload();
}
