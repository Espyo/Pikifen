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
void options_menu_t::choose_input(
    const PLAYER_ACTION_TYPE action_type, size_t bind_idx
) {
    capturing_input = 1;
    capturing_input_timeout = OPTIONS_MENU::INPUT_CAPTURE_TIMEOUT_DURATION;
    
    const vector<control_bind> &all_binds = game.controls.binds();
    size_t binds_counted = 0;
    cur_action_type = action_type;
    cur_bind_idx = all_binds.size();
    
    for(size_t b = 0; b < all_binds.size(); b++) {
        if(all_binds[b].action_type_id != action_type) continue;
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
void options_menu_t::delete_bind(
    const PLAYER_ACTION_TYPE action_type, size_t bind_idx
) {
    vector<control_bind> &all_binds = game.controls.binds();
    size_t binds_counted = 0;
    
    for(size_t b = 0; b < all_binds.size(); b++) {
        if(all_binds[b].action_type_id != action_type) continue;
        if(binds_counted == bind_idx) {
            all_binds.erase(all_binds.begin() + b);
            break;
        } else {
            binds_counted++;
        }
    }
    
    populate_binds();
}


/**
 * @brief Draws the options menu.
 */
void options_menu_t::draw() {
    menu_t::draw();
    if(packs_menu) packs_menu->draw();
    
    if(capturing_input == 1) {
        al_draw_filled_rectangle(
            0, 0, game.win_w, game.win_h,
            al_map_rgba(24, 24, 32, 192)
        );
        
        draw_text_lines(
            "Please perform the new input for \n" +
            game.controls.get_player_action_type(cur_action_type).name + "\n"
            "\n"
            "(Or wait " + i2s(capturing_input_timeout + 1) + "s to cancel...)",
            game.sys_content.fnt_standard,
            point(game.win_w / 2.0f, game.win_h / 2.0f),
            point(LARGE_FLOAT),
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
void options_menu_t::handle_allegro_event(const ALLEGRO_EVENT &ev) {
    if(!active) return;
    
    switch(capturing_input) {
    case 0: {
        //Not capturing.
        break;
    } case 1: {
        //Actively capturing.
        player_input input = game.controls.allegro_event_to_input(ev);
        if(input.value >= 0.5f) {
            vector<control_bind> &all_binds = game.controls.binds();
            if(cur_bind_idx >= all_binds.size()) {
                control_bind new_bind;
                new_bind.action_type_id = cur_action_type;
                new_bind.player_nr = 0;
                new_bind.input = input;
                all_binds.push_back(new_bind);
            } else {
                game.controls.binds()[cur_bind_idx].input = input;
            }
            capturing_input = 2;
            populate_binds();
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
    
    menu_t::handle_allegro_event(ev);
    if(packs_menu) packs_menu->handle_allegro_event(ev);
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void options_menu_t::handle_player_action(const player_action &action) {
    if(capturing_input != 0) return;
    menu_t::handle_player_action(action);
    if(packs_menu) packs_menu->handle_player_action(action);
}


/**
 * @brief Initializes the audio options menu GUI.
 */
void options_menu_t::init_gui_audio_page() {
    //Menu items.
    audio_gui.register_coords("back",                  12,  5,   20,  6);
    audio_gui.register_coords("back_input",             3,  7,    4,  4);
    audio_gui.register_coords("header",                50, 10,   50,  6);
    audio_gui.register_coords("master_volume",         50, 25,   70, 10);
    audio_gui.register_coords("gameplay_sound_volume", 50, 37.5, 65, 10);
    audio_gui.register_coords("music_volume",          50, 50,   65, 10);
    audio_gui.register_coords("ambiance_sound_volume", 50, 62.5, 65, 10);
    audio_gui.register_coords("ui_sound_volume",       50, 75,   65, 10);
    audio_gui.register_coords("tooltip",               50, 96,   96,  4);
    audio_gui.read_coords(
        game.content.gui_defs.list[OPTIONS_MENU::AUDIO_GUI_FILE_NAME].
        get_child_by_name("positions")
    );
    
    //Back button.
    audio_gui.back_item =
        new button_gui_item("Back", game.sys_content.fnt_standard);
    audio_gui.back_item->on_activate =
    [this] (const point &) {
        audio_gui.responsive = false;
        audio_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        top_gui.responsive = true;
        top_gui.start_animation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    audio_gui.back_item->on_get_tooltip =
    [] () { return "Return to the top-level options menu."; };
    audio_gui.add_item(audio_gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&audio_gui);
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "AUDIO OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    audio_gui.add_item(header_text, "header");
    
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
        game.audio.update_volumes(
            game.options.master_volume,
            game.options.gameplay_sound_volume,
            game.options.music_volume,
            game.options.ambiance_sound_volume,
            game.options.ui_sound_volume
        );
    };
    
    //Master volume picker.
    master_vol_picker =
        new options_menu_picker_gui_item<float>(
        "Master volume: ",
        &game.options.master_volume,
        OPTIONS::DEF_MASTER_VOLUME,
        preset_volume_values,
        preset_volume_names,
        "Volume of the final mix of all audio."
    );
    master_vol_picker->after_change = update_volumes;
    master_vol_picker->init();
    audio_gui.add_item(master_vol_picker, "master_volume");
    
    //Gameplay sound effects volume picker.
    gameplay_sound_vol_picker =
        new options_menu_picker_gui_item<float>(
        "Gameplay sound effects volume: ",
        &game.options.gameplay_sound_volume,
        OPTIONS::DEF_GAMEPLAY_SOUND_VOLUME,
        preset_volume_values,
        preset_volume_names,
        "Volume for in-world gameplay sound effects specifically."
    );
    gameplay_sound_vol_picker->after_change = update_volumes;
    gameplay_sound_vol_picker->init();
    audio_gui.add_item(gameplay_sound_vol_picker, "gameplay_sound_volume");
    
    //Music volume picker.
    music_vol_picker =
        new options_menu_picker_gui_item<float>(
        "Music volume: ",
        &game.options.music_volume,
        OPTIONS::DEF_MUSIC_VOLUME,
        preset_volume_values,
        preset_volume_names,
        "Volume for music specifically."
    );
    music_vol_picker->after_change = update_volumes;
    music_vol_picker->init();
    audio_gui.add_item(music_vol_picker, "music_volume");
    
    //Ambiance sound volume picker.
    ambiance_sound_vol_picker =
        new options_menu_picker_gui_item<float>(
        "Ambiance sound effects volume: ",
        &game.options.ambiance_sound_volume,
        OPTIONS::DEF_AMBIANCE_SOUND_VOLUME,
        preset_volume_values,
        preset_volume_names,
        "Volume for in-world ambiance sound effects specifically."
    );
    ambiance_sound_vol_picker->after_change = update_volumes;
    ambiance_sound_vol_picker->init();
    audio_gui.add_item(ambiance_sound_vol_picker, "ambiance_sound_volume");
    
    //UI sound effects volume picker.
    ui_sound_vol_picker =
        new options_menu_picker_gui_item<float>(
        "UI sound effects volume: ",
        &game.options.ui_sound_volume,
        OPTIONS::DEF_UI_SOUND_VOLUME,
        preset_volume_values,
        preset_volume_names,
        "Volume for interface sound effects specifically."
    );
    ui_sound_vol_picker->after_change = update_volumes;
    ui_sound_vol_picker->init();
    audio_gui.add_item(ui_sound_vol_picker, "ui_sound_volume");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&audio_gui);
    audio_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    audio_gui.set_selected_item(master_vol_picker, true);
    audio_gui.responsive = false;
    audio_gui.hide_items();
}


/**
 * @brief Initializes the control binds options menu GUI.
 */
void options_menu_t::init_gui_control_binds_page() {
    //Menu items.
    binds_gui.register_coords("back",        12,  5, 20,  6);
    binds_gui.register_coords("back_input",   3,  7,  4,  4);
    binds_gui.register_coords("header",      50,  5, 50,  6);
    binds_gui.register_coords("list",        50, 51, 88, 82);
    binds_gui.register_coords("list_scroll", 97, 51,  2, 82);
    binds_gui.register_coords("tooltip",     50, 96, 96,  4);
    binds_gui.read_coords(
        game.content.gui_defs.list[OPTIONS_MENU::CONTROL_BINDS_GUI_FILE_NAME].
        get_child_by_name("positions")
    );
    
    //Back button.
    binds_gui.back_item =
        new button_gui_item("Back", game.sys_content.fnt_standard);
    binds_gui.back_item->on_activate =
    [this] (const point &) {
        binds_gui.responsive = false;
        binds_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        controls_gui.responsive = true;
        controls_gui.start_animation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        save_options();
    };
    binds_gui.back_item->on_get_tooltip =
    [] () { return "Return to the previous menu."; };
    binds_gui.add_item(binds_gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&binds_gui);
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "CONTROL BINDS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    binds_gui.add_item(header_text, "header");
    
    //Controls list box.
    binds_list_box = new list_gui_item();
    binds_gui.add_item(binds_list_box, "list");
    
    //Controls list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = binds_list_box;
    binds_gui.add_item(list_scroll, "list_scroll");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&binds_gui);
    binds_gui.add_item(tooltip_text, "tooltip");
    
    //Populate the list of binds.
    populate_binds();
    
    //Finishing touches.
    binds_gui.set_selected_item(binds_gui.back_item, true);
    binds_gui.responsive = false;
    binds_gui.hide_items();
    al_reconfigure_joysticks();
}


/**
 * @brief Initializes the controls options menu GUI.
 */
void options_menu_t::init_gui_controls_page() {
    //Menu items.
    controls_gui.register_coords("back",          12,  5,   20,  6);
    controls_gui.register_coords("back_input",     3,  7,    4,  4);
    controls_gui.register_coords("header",        50, 10,   50,  6);
    controls_gui.register_coords("control_binds", 50, 25,   70, 10);
    controls_gui.register_coords("cursor_speed",  50, 47.5, 70, 10);
    controls_gui.register_coords("auto_throw",    50, 65,   70, 10);
    controls_gui.register_coords("tooltip",       50, 96,   96,  4);
    controls_gui.read_coords(
        game.content.gui_defs.list[OPTIONS_MENU::CONTROLS_GUI_FILE_NAME].
        get_child_by_name("positions")
    );
    
    //Back button.
    controls_gui.back_item =
        new button_gui_item("Back", game.sys_content.fnt_standard);
    controls_gui.back_item->on_activate =
    [this] (const point &) {
        controls_gui.responsive = false;
        controls_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        top_gui.responsive = true;
        top_gui.start_animation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    controls_gui.back_item->on_get_tooltip =
    [] () { return "Return to the top-level options menu."; };
    controls_gui.add_item(controls_gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&controls_gui);
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "CONTROLS OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    controls_gui.add_item(header_text, "header");
    
    //Control binds button.
    button_gui_item* control_binds_button =
        new button_gui_item("Edit control binds...", game.sys_content.fnt_standard);
    control_binds_button->on_activate =
    [this] (const point &) {
        controls_gui.responsive = false;
        controls_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        binds_gui.responsive = true;
        binds_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    control_binds_button->on_get_tooltip =
    [] () { return "Choose what buttons do what."; };
    controls_gui.add_item(control_binds_button, "control_binds");
    
    //Cursor speed.
    cursor_speed_picker =
        new options_menu_picker_gui_item<float>(
        "Cursor speed: ",
        &game.options.cursor_speed,
        OPTIONS::DEF_CURSOR_SPEED,
    {250.0f, 350.0f, 500.0f, 700.0f, 1000.0f},
    {"Very slow", "Slow", "Medium", "Fast", "Very fast"},
    "Cursor speed, when controlling without a mouse."
    );
    cursor_speed_picker->value_to_string = [] (float v) {
        return f2s(v);
    };
    cursor_speed_picker->init();
    controls_gui.add_item(cursor_speed_picker, "cursor_speed");
    
    //Auto-throw mode.
    auto_throw_picker =
        new options_menu_picker_gui_item<AUTO_THROW_MODE>(
        "Auto-throw: ",
        &game.options.auto_throw_mode,
        OPTIONS::DEF_AUTO_THROW_MODE,
    {AUTO_THROW_MODE_OFF, AUTO_THROW_MODE_HOLD, AUTO_THROW_MODE_TOGGLE},
    {"Off", "Hold input", "Input toggles"}
    );
    auto_throw_picker->preset_descriptions = {
        "Pikmin are only thrown when you release the throw input.",
        "Auto-throw Pikmin periodically as long as the throw input is held.",
        "Do the throw input once to auto-throw periodically, and again to stop."
    };
    auto_throw_picker->init();
    controls_gui.add_item(auto_throw_picker, "auto_throw");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&controls_gui);
    controls_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    controls_gui.set_selected_item(control_binds_button, true);
    controls_gui.responsive = false;
    controls_gui.hide_items();
}


/**
 * @brief Initializes the graphics options menu GUI.
 */
void options_menu_t::init_gui_graphics_page() {
    //Menu items.
    graphics_gui.register_coords("back",            12,  5,   20,  6);
    graphics_gui.register_coords("back_input",       3,  7,    4,  4);
    graphics_gui.register_coords("header",          50, 10,   50,  6);
    graphics_gui.register_coords("fullscreen",      50, 25,   70, 10);
    graphics_gui.register_coords("resolution",      50, 42.5, 70, 10);
    graphics_gui.register_coords("tooltip",         50, 96,   96,  4);
    graphics_gui.register_coords("restart_warning", 50, 85,   70,  6);
    graphics_gui.read_coords(
        game.content.gui_defs.list[OPTIONS_MENU::GRAPHICS_GUI_FILE_NAME].
        get_child_by_name("positions")
    );
    
    //Back button.
    graphics_gui.back_item =
        new button_gui_item("Back", game.sys_content.fnt_standard);
    graphics_gui.back_item->on_activate =
    [this] (const point &) {
        graphics_gui.responsive = false;
        graphics_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        top_gui.responsive = true;
        top_gui.start_animation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    graphics_gui.back_item->on_get_tooltip =
    [] () { return "Return to the top-level options menu."; };
    graphics_gui.add_item(graphics_gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&graphics_gui);
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "GRAPHICS OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    graphics_gui.add_item(header_text, "header");
    
    //Fullscreen checkbox.
    check_gui_item* fullscreen_check =
        new check_gui_item(
        &game.options.intended_win_fullscreen,
        "Fullscreen", game.sys_content.fnt_standard
    );
    fullscreen_check->on_activate =
    [this, fullscreen_check] (const point &) {
        fullscreen_check->def_activate_code();
        trigger_restart_warning();
    };
    fullscreen_check->on_get_tooltip =
    [] () {
        return
            "Show the game in fullscreen, or in a window? Default: " +
            b2s(OPTIONS::DEF_WIN_FULLSCREEN) + ".";
    };
    graphics_gui.add_item(fullscreen_check, "fullscreen");
    
    //Resolution picker.
    vector<string> resolution_preset_names;
    for(size_t p = 0; p < resolution_presets.size(); p++) {
        resolution_preset_names.push_back(
            i2s(resolution_presets[p].first) + "x" +
            i2s(resolution_presets[p].second)
        );
    }
    cur_resolution_option.first = game.options.intended_win_w;
    cur_resolution_option.second = game.options.intended_win_h;
    resolution_picker =
        new options_menu_picker_gui_item<std::pair<int, int> >(
        "Resolution: ",
        &cur_resolution_option,
        std::make_pair(OPTIONS::DEF_WIN_W, OPTIONS::DEF_WIN_H),
        resolution_presets,
        resolution_preset_names,
        "The game's width and height."
    );
    resolution_picker->after_change = [this] () {
        game.options.intended_win_w = cur_resolution_option.first;
        game.options.intended_win_h = cur_resolution_option.second;
        trigger_restart_warning();
    };
    resolution_picker->value_to_string = [] (const std::pair<int, int> &v) {
        return i2s(v.first) + "x" + i2s(v.second);
    };
    resolution_picker->init();
    graphics_gui.add_item(resolution_picker, "resolution");
    
    //Warning text.
    warning_text =
        new text_gui_item(
        "Please restart for the changes to take effect.",
        game.sys_content.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_CENTER
    );
    warning_text->visible = false;
    graphics_gui.add_item(warning_text, "restart_warning");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&graphics_gui);
    graphics_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    graphics_gui.set_selected_item(fullscreen_check, true);
    graphics_gui.responsive = false;
    graphics_gui.hide_items();
}


/**
 * @brief Initializes the misc. options menu GUI.
 */
void options_menu_t::init_gui_misc_page() {
    //Menu items.
    misc_gui.register_coords("back",                 12,  5,   20,  6);
    misc_gui.register_coords("back_input",            3,  7,    4,  4);
    misc_gui.register_coords("header",               50, 10,   50,  6);
    misc_gui.register_coords("cursor_cam_weight",    50, 22.5, 70, 10);
    misc_gui.register_coords("show_hud_input_icons", 50, 40,   70, 10);
    misc_gui.register_coords("leaving_confirmation", 50, 57.5, 70, 10);
    misc_gui.register_coords("tooltip",              50, 96,   96,  4);
    misc_gui.read_coords(
        game.content.gui_defs.list[OPTIONS_MENU::MISC_GUI_FILE_NAME].
        get_child_by_name("positions")
    );
    
    //Back button.
    misc_gui.back_item =
        new button_gui_item("Back", game.sys_content.fnt_standard);
    misc_gui.back_item->on_activate =
    [this] (const point &) {
        misc_gui.responsive = false;
        misc_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        top_gui.responsive = true;
        top_gui.start_animation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    misc_gui.back_item->on_get_tooltip =
    [] () { return "Return to the top-level options menu."; };
    misc_gui.add_item(misc_gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&misc_gui);
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "MISC. OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    misc_gui.add_item(header_text, "header");
    
    //Cursor camera weight.
    cursor_cam_weight_picker =
        new options_menu_picker_gui_item<float>(
        "Cursor cam weight: ",
        &game.options.cursor_cam_weight,
        OPTIONS::DEF_CURSOR_CAM_WEIGHT,
    {0.0f, 0.1f, 0.3f, 0.6f},
    {"None", "Small", "Medium", "Large"},
    "When you move the cursor, how much does it affect the camera?"
    );
    cursor_cam_weight_picker->value_to_string = [] (float v) {
        return f2s(v);
    };
    cursor_cam_weight_picker->init();
    misc_gui.add_item(cursor_cam_weight_picker, "cursor_cam_weight");
    
    //Show HUD player input icons checkbox.
    check_gui_item* show_hud_input_icons_check =
        new check_gui_item(
        &game.options.show_hud_input_icons,
        "Show input icons on HUD", game.sys_content.fnt_standard
    );
    show_hud_input_icons_check->on_get_tooltip =
    [] () {
        return
            "Show icons of the player inputs near relevant HUD items? "
            "Default: " + b2s(OPTIONS::DEF_SHOW_HUD_INPUT_ICONS) + ".";
    };
    misc_gui.add_item(show_hud_input_icons_check, "show_hud_input_icons");
    
    //Leaving confirmation mode.
    leaving_confirmation_picker =
        new options_menu_picker_gui_item<LEAVING_CONFIRMATION_MODE>(
        "Leave confirm: ",
        &game.options.leaving_confirmation_mode,
    OPTIONS::DEF_LEAVING_CONFIRMATION_MODE, {
        LEAVING_CONFIRMATION_MODE_ALWAYS,
        LEAVING_CONFIRMATION_MODE_1_MIN,
        LEAVING_CONFIRMATION_MODE_NEVER
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
    misc_gui.add_item(leaving_confirmation_picker, "leaving_confirmation");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&misc_gui);
    misc_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    misc_gui.set_selected_item(cursor_cam_weight_picker, true);
    misc_gui.responsive = false;
    misc_gui.hide_items();
}


/**
 * @brief Initializes the top-level menu GUI.
 */
void options_menu_t::init_gui_top_page() {
    data_node* gui_file = &game.content.gui_defs.list[OPTIONS_MENU::TOP_GUI_FILE_NAME];
    
    //Button icon positions.
    data_node* icons_node = gui_file->get_child_by_name("icons_to_the_left");
    
#define icon_left(name, def) s2b(icons_node->get_child_by_name(name)-> \
                                 get_value_or_default(def))
    
    bool controls_icon_left = icon_left("controls", "true");
    bool graphics_icon_left = icon_left("graphics", "true");
    bool audio_icon_left = icon_left("audio", "true");
    bool packs_icon_left = icon_left("packs", "true");
    bool misc_icon_left = icon_left("misc", "true");
    
#undef icon_left
    
    //Menu items.
    top_gui.register_coords("back",       12,  5, 20,  6);
    top_gui.register_coords("back_input",  3,  7,  4,  4);
    top_gui.register_coords("header",     50, 10, 50,  6);
    top_gui.register_coords("controls",   50, 25, 65, 10);
    top_gui.register_coords("graphics",   50, 37, 65, 10);
    top_gui.register_coords("audio",      50, 49, 65, 10);
    top_gui.register_coords("packs",      50, 61, 65, 10);
    top_gui.register_coords("misc",       50, 73, 60, 10);
    top_gui.register_coords("advanced",   87, 86, 22,  8);
    top_gui.register_coords("tooltip",    50, 96, 96,  4);
    top_gui.read_coords(gui_file->get_child_by_name("positions"));
    
    //Back button.
    top_gui.back_item =
        new button_gui_item("Back", game.sys_content.fnt_standard);
    top_gui.back_item->on_activate =
    [this] (const point &) {
        save_options();
        leave();
    };
    top_gui.back_item->on_get_tooltip =
    [] () { return "Return to the previous menu."; };
    top_gui.add_item(top_gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&top_gui);
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "OPTIONS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    top_gui.add_item(header_text, "header");
    
    //Controls options button.
    button_gui_item* controls_button =
        new button_gui_item("Controls", game.sys_content.fnt_standard);
    controls_button->on_draw =
    [ = ] (const point & center, const point & size) {
        draw_menu_button_icon(
            MENU_ICON_CONTROLS, center, size, controls_icon_left
        );
        draw_button(
            center, size,
            controls_button->text, controls_button->font,
            controls_button->color, controls_button->selected,
            controls_button->get_juice_value()
        );
    };
    controls_button->on_activate =
    [this] (const point &) {
        top_gui.responsive = false;
        top_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        controls_gui.responsive = true;
        controls_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    controls_button->on_get_tooltip =
    [] () { return "Change the way you control the game."; };
    top_gui.add_item(controls_button, "controls");
    
    //Graphics options button.
    button_gui_item* graphics_button =
        new button_gui_item("Graphics", game.sys_content.fnt_standard);
    graphics_button->on_draw =
    [ = ] (const point & center, const point & size) {
        draw_menu_button_icon(
            MENU_ICON_GRAPHICS, center, size, graphics_icon_left
        );
        draw_button(
            center, size,
            graphics_button->text, graphics_button->font,
            graphics_button->color, graphics_button->selected,
            graphics_button->get_juice_value()
        );
    };
    graphics_button->on_activate =
    [this] (const point &) {
        top_gui.responsive = false;
        top_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        graphics_gui.responsive = true;
        graphics_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    graphics_button->on_get_tooltip =
    [] () { return "Change some options about how the game looks."; };
    top_gui.add_item(graphics_button, "graphics");
    
    //Audio options button.
    button_gui_item* audio_button =
        new button_gui_item("Audio", game.sys_content.fnt_standard);
    audio_button->on_draw =
    [ = ] (const point & center, const point & size) {
        draw_menu_button_icon(
            MENU_ICON_AUDIO, center, size, audio_icon_left
        );
        draw_button(
            center, size,
            audio_button->text, audio_button->font,
            audio_button->color, audio_button->selected,
            audio_button->get_juice_value()
        );
    };
    audio_button->on_activate =
    [this] (const point &) {
        top_gui.responsive = false;
        top_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        audio_gui.responsive = true;
        audio_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    audio_button->on_get_tooltip =
    [] () { return "Change options about the way the game sounds."; };
    top_gui.add_item(audio_button, "audio");
    
    //Packs options button.
    button_gui_item* packs_button =
        new button_gui_item("Packs", game.sys_content.fnt_standard);
    packs_button->on_draw =
    [ = ] (const point & center, const point & size) {
        draw_menu_button_icon(
            MENU_ICON_PACKS, center, size, packs_icon_left
        );
        draw_button(
            center, size,
            packs_button->text, packs_button->font,
            packs_button->color, packs_button->selected,
            packs_button->get_juice_value()
        );
    };
    packs_button->on_activate =
    [this] (const point &) {
        top_gui.responsive = false;
        top_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        packs_menu = new packs_menu_t();
        packs_menu->gui.responsive = true;
        packs_menu->gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        packs_menu->leave_callback = [ = ] () {
            packs_menu->unload_timer = OPTIONS_MENU::HUD_MOVE_TIME;
            packs_menu->gui.responsive = false;
            packs_menu->gui.start_animation(
                GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
                OPTIONS_MENU::HUD_MOVE_TIME
            );
            top_gui.responsive = true;
            top_gui.start_animation(
                GUI_MANAGER_ANIM_LEFT_TO_CENTER,
                OPTIONS_MENU::HUD_MOVE_TIME
            );
        };
        packs_menu->load();
        packs_menu->enter();
    };
    packs_button->on_get_tooltip =
    [] () { return "Manage any content packs you have installed."; };
    top_gui.add_item(packs_button, "packs");
    
    //Misc. options button.
    button_gui_item* misc_button =
        new button_gui_item("Misc.", game.sys_content.fnt_standard);
    misc_button->on_draw =
    [ = ] (const point & center, const point & size) {
        draw_menu_button_icon(
            MENU_ICON_OPTIONS_MISC, center, size, misc_icon_left
        );
        draw_button(
            center, size,
            misc_button->text, misc_button->font,
            misc_button->color, misc_button->selected,
            misc_button->get_juice_value()
        );
    };
    misc_button->on_activate =
    [this] (const point &) {
        top_gui.responsive = false;
        top_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
        misc_gui.responsive = true;
        misc_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            OPTIONS_MENU::HUD_MOVE_TIME
        );
    };
    misc_button->on_get_tooltip =
    [] () { return "Change some miscellaneous gameplay and game options."; };
    top_gui.add_item(misc_button, "misc");
    
    //Advanced bullet point.
    bullet_gui_item* advanced_bullet =
        new bullet_gui_item("Advanced...", game.sys_content.fnt_standard);
    advanced_bullet->on_activate =
    [] (const point &) {
        open_manual("options.html");
    };
    advanced_bullet->on_get_tooltip =
    [] () {
        return
            "Click to open the manual (in the game's folder) for info "
            "on advanced options.";
    };
    top_gui.add_item(advanced_bullet, "advanced");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&top_gui);
    top_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    top_gui.set_selected_item(controls_button, true);
}


/**
 * @brief Loads the menu.
 */
void options_menu_t::load() {
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
        std::make_pair(OPTIONS::DEF_WIN_W, OPTIONS::DEF_WIN_H)
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
    init_gui_top_page();
    init_gui_controls_page();
    init_gui_control_binds_page();
    init_gui_graphics_page();
    init_gui_audio_page();
    init_gui_misc_page();
    
    //Finish the menu class setup.
    guis.push_back(&top_gui);
    guis.push_back(&controls_gui);
    guis.push_back(&binds_gui);
    guis.push_back(&graphics_gui);
    guis.push_back(&audio_gui);
    guis.push_back(&misc_gui);
    menu_t::load();
}


/**
 * @brief Populates the list of binds.
 */
void options_menu_t::populate_binds() {
    binds_list_box->delete_all_children();
    
    const vector<player_action_type> &all_player_action_types =
        game.controls.get_all_player_action_types();
    vector<control_bind> &all_binds = game.controls.binds();
    
    binds_per_action_type.clear();
    binds_per_action_type.assign(all_player_action_types.size(), vector<control_bind>());
    
    //Read all binds and sort them by player action type.
    for(size_t b = 0; b < all_binds.size(); b++) {
        const control_bind &bind = all_binds[b];
        if(bind.player_nr != 0) continue;
        binds_per_action_type[bind.action_type_id].push_back(bind);
    }
    
    PLAYER_ACTION_CAT last_cat = PLAYER_ACTION_CAT_NONE;
    
    for(size_t a = 0; a < all_player_action_types.size(); a++) {
        const player_action_type &action_type = all_player_action_types[a];
        
        if(action_type.internal_name.empty()) continue;
        
        float action_y =
            binds_list_box->get_child_bottom() +
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
            }
            }
            text_gui_item* section_text =
                new text_gui_item(section_name, game.sys_content.fnt_area_name);
            section_text->center =
                point(
                    0.50f,
                    action_y + OPTIONS_MENU::BIND_BUTTON_HEIGHT / 2.0f
                );
            section_text->size =
                point(0.50f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            binds_list_box->add_child(section_text);
            binds_gui.add_item(section_text);
            
            action_y =
                binds_list_box->get_child_bottom() +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
            last_cat = action_type.category;
            
        }
        
        float cur_y = action_y + OPTIONS_MENU::BIND_BUTTON_HEIGHT / 2.0f;
        
        //Action type name bullet.
        bullet_gui_item* name_bullet =
            new bullet_gui_item(action_type.name, game.sys_content.fnt_standard);
        name_bullet->center =
            point(0.22f, cur_y);
        name_bullet->size =
            point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
        name_bullet->on_get_tooltip =
        [action_type] () { return action_type.description; };
        binds_list_box->add_child(name_bullet);
        binds_gui.add_item(name_bullet);
        
        //More button.
        button_gui_item* more_button =
            new button_gui_item("...", game.sys_content.fnt_standard);
        more_button->on_activate =
        [this, action_type] (const point &) {
            if(showing_binds_more && action_type.id == cur_action_type) {
                showing_binds_more = false;
            } else {
                cur_action_type = action_type.id;
                showing_binds_more = true;
            }
            populate_binds();
        };
        more_button->center =
            point(0.92f, cur_y);
        more_button->size =
            point(0.05f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
        string tooltip =
            (showing_binds_more && action_type.id == cur_action_type) ?
            "Hide options." :
            "Show information and options for this action.";
        more_button->on_get_tooltip =
        [tooltip] () { return tooltip; };
        binds_list_box->add_child(more_button);
        binds_gui.add_item(more_button);
        if(action_type.id == cur_action_type) {
            binds_gui.set_selected_item(more_button, true);
        }
        
        vector<control_bind> a_binds = binds_per_action_type[action_type.id];
        for(size_t b = 0; b < a_binds.size(); b++) {
        
            //Change bind button.
            button_gui_item* bind_button =
                new button_gui_item("", game.sys_content.fnt_standard);
            bind_button->on_activate =
            [this, action_type, b] (const point &) {
                choose_input(action_type.id, b);
            };
            bind_button->on_draw =
                [this, b, a_binds, bind_button]
            (const point & center, const point & size) {
                draw_player_input_icon(
                    game.sys_content.fnt_slim, a_binds[b].input, false,
                    center, size * 0.8f
                );
                
                draw_button(
                    center, size,
                    "", game.sys_content.fnt_standard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->get_juice_value()
                );
            };
            bind_button->center =
                point(0.63f, cur_y);
            bind_button->size =
                point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bind_button->on_get_tooltip =
            [] () { return "Change the input for this action."; };
            binds_list_box->add_child(bind_button);
            binds_gui.add_item(bind_button);
            
            if(showing_binds_more && action_type.id == cur_action_type) {
                //Remove bind button.
                button_gui_item* remove_bind_button =
                    new button_gui_item("", game.sys_content.fnt_standard);
                remove_bind_button->on_activate =
                [this, action_type, b] (const point &) {
                    delete_bind(action_type.id, b);
                };
                remove_bind_button->on_draw =
                    [this, remove_bind_button]
                (const point & center, const point & size) {
                    draw_button(
                        center, size, "X", game.sys_content.fnt_standard, COLOR_WHITE,
                        remove_bind_button->selected,
                        remove_bind_button->get_juice_value()
                    );
                };
                remove_bind_button->center =
                    point(0.85f, cur_y);
                remove_bind_button->size =
                    point(0.05f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
                remove_bind_button->on_get_tooltip =
                [] () { return "Remove this input from this action."; };
                binds_list_box->add_child(remove_bind_button);
                binds_gui.add_item(remove_bind_button);
                remove_bind_button->start_juice_animation(
                    gui_item::JUICE_TYPE_GROW_TEXT_HIGH
                );
            }
            
            if(action_type.id == cur_action_type) {
                bind_button->start_juice_animation(
                    gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
            }
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(a_binds.empty()) {
        
            //Add first bind button.
            button_gui_item* bind_button =
                new button_gui_item("", game.sys_content.fnt_standard);
            bind_button->on_activate =
            [this, action_type] (const point &) {
                choose_input(action_type.id, 0);
            };
            bind_button->on_draw =
                [this, bind_button]
            (const point & center, const point & size) {
                draw_button(
                    center, size, "", game.sys_content.fnt_standard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->get_juice_value()
                );
            };
            bind_button->center =
                point(0.63f, cur_y);
            bind_button->size =
                point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            bind_button->on_get_tooltip =
            [] () { return "Choose an input for this action."; };
            binds_list_box->add_child(bind_button);
            binds_gui.add_item(bind_button);
            bind_button->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        } else if(showing_binds_more && action_type.id == cur_action_type) {
        
            //Add button.
            button_gui_item* add_button =
                new button_gui_item("Add...", game.sys_content.fnt_standard);
            add_button->center =
                point(0.63f, cur_y);
            add_button->size =
                point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            add_button->on_activate =
            [this, action_type, a_binds] (const point &) {
                choose_input(action_type.id, a_binds.size());
            };
            add_button->on_get_tooltip =
            [] () { return "Add another input to this action."; };
            binds_list_box->add_child(add_button);
            binds_gui.add_item(add_button);
            add_button->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_HIGH
            );
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(showing_binds_more && action_type.id == cur_action_type) {
        
            //Restore default button.
            button_gui_item* restore_button =
                new button_gui_item("Restore defaults", game.sys_content.fnt_standard);
            restore_button->center =
                point(0.63f, cur_y);
            restore_button->size =
                point(0.34f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            restore_button->on_activate =
            [this, action_type] (const point &) {
                restore_default_binds(action_type.id);
            };
            restore_button->on_get_tooltip =
            [] () { return "Restore this action's default inputs."; };
            binds_list_box->add_child(restore_button);
            binds_gui.add_item(restore_button);
            restore_button->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                OPTIONS_MENU::BIND_BUTTON_HEIGHT +
                OPTIONS_MENU::BIND_BUTTON_PADDING;
                
            //Default label.
            text_gui_item* default_label_text =
                new text_gui_item(
                "Default:", game.sys_content.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
            );
            default_label_text->center =
                point(0.63f, cur_y);
            default_label_text->size =
                point(0.30f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            binds_list_box->add_child(default_label_text);
            binds_gui.add_item(default_label_text);
            default_label_text->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            //Default icon.
            player_input def_input =
                game.controls.str_to_input(action_type.default_bind_str);
            gui_item* default_icon = new gui_item();
            default_icon->center =
                point(0.68f, cur_y);
            default_icon->size =
                point(0.17f, OPTIONS_MENU::BIND_BUTTON_HEIGHT);
            default_icon->on_draw =
            [def_input] (const point & center, const point & size) {
                draw_player_input_icon(
                    game.sys_content.fnt_slim, def_input, false, center, size
                );
            };
            binds_list_box->add_child(default_icon);
            binds_gui.add_item(default_icon);
            
        }
        
        if(a < all_player_action_types.size() - 1) {
            //Spacer line.
            gui_item* line = new gui_item();
            line->center =
                point(
                    0.50f, binds_list_box->get_child_bottom() + 0.02f
                );
            line->size = point(0.90f, 0.02f);
            line->on_draw =
            [] (const point & center, const point & size) {
                al_draw_line(
                    center.x - size.x / 2.0f,
                    center.y,
                    center.x + size.x / 2.0f,
                    center.y,
                    COLOR_TRANSPARENT_WHITE,
                    1.0f
                );
            };
            binds_list_box->add_child(line);
            binds_gui.add_item(line);
        }
    }
}


/**
 * @brief Restores the default binds for a given player action.
 *
 * @param action_type_id Action type ID of the action to restore.
 */
void options_menu_t::restore_default_binds(
    const PLAYER_ACTION_TYPE action_type_id
) {
    const player_action_type &action_type =
        game.controls.get_player_action_type(action_type_id);
    vector<control_bind> &all_binds =
        game.controls.binds();
        
    for(size_t b = 0; b < all_binds.size();) {
        if(
            all_binds[b].player_nr == 0 &&
            all_binds[b].action_type_id == action_type_id
        ) {
            all_binds.erase(all_binds.begin() + b);
        } else {
            b++;
        }
    }
    
    player_input def_input =
        game.controls.str_to_input(action_type.default_bind_str);
    control_bind new_bind;
    
    if(def_input.type != INPUT_TYPE_NONE) {
        new_bind.action_type_id = action_type_id;
        new_bind.player_nr = 0;
        new_bind.input = def_input;
        all_binds.push_back(new_bind);
    }
    
    showing_binds_more = false;
    populate_binds();
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void options_menu_t::tick(float delta_t) {
    menu_t::tick(delta_t);
    
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
void options_menu_t::trigger_restart_warning() {
    if(!warning_text->visible) {
        warning_text->visible = true;
        warning_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
    }
}


/**
 * @brief Unloads the menu.
 */
void options_menu_t::unload() {
    if(packs_menu) {
        packs_menu->unload();
        delete packs_menu;
        packs_menu = nullptr;
    }
    
    menu_t::unload();
}
