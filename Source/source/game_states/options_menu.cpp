/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Options menu state class and options menu state-related functions.
 */

#include <algorithm>

#include "menus.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../load.h"
#include "../options.h"
#include "../utils/string_utils.h"


namespace OPTIONS_MENU {
//Path to the audio menu GUI information file.
const string AUDIO_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Options_menu_audio.txt";
//Path to the controls menu GUI information file.
const string CONTROLS_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Options_menu_controls.txt";
//Path to the graphics menu GUI information file.
const string GRAPHICS_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Options_menu_graphics.txt";
//How long the menu items take to move when switching pages.
const float HUD_MOVE_TIME = 0.5f;
//Path to the misc menu GUI information file.
const string MISC_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Options_menu_misc.txt";
//Path to the top-level menu GUI information file.
const string TOP_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Options_menu_top.txt";
}


/* ----------------------------------------------------------------------------
 * Creates an "options menu" state.
 */
options_menu_state::options_menu_state() :
    game_state(),
    bmp_menu_bg(nullptr),
    auto_throw_picker(nullptr),
    resolution_picker(nullptr),
    cursor_speed_picker(nullptr),
    cursor_cam_weight_picker(nullptr),
    leaving_confirmation_picker(nullptr),
    warning_text(nullptr) {
    
    //Let's fill in the list of preset resolutions. For that, we'll get
    //the display modes fetched by Allegro. These are usually nice round
    //resolutions, and they work on fullscreen mode.
    int n_modes = al_get_num_display_modes();
    for(int d = 0; d < n_modes; ++d) {
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
            ++p;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Draws the options menu.
 */
void options_menu_state::do_drawing() {
    al_clear_to_color(COLOR_BLACK);
    
    draw_bitmap(
        bmp_menu_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h), 0, map_gray(64)
    );
    
    top_gui.draw();
    controls_gui.draw();
    graphics_gui.draw();
    audio_gui.draw();
    misc_gui.draw();
    
    draw_mouse_cursor(GAME::CURSOR_STANDARD_COLOR);
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void options_menu_state::do_logic() {
    vector<player_action> player_actions = game.controls.new_frame();
    if(!game.fade_mgr.is_fading()) {
        for(size_t a = 0; a < player_actions.size(); ++a) {
            top_gui.handle_player_action(player_actions[a]);
            controls_gui.handle_player_action(player_actions[a]);
            graphics_gui.handle_player_action(player_actions[a]);
            audio_gui.handle_player_action(player_actions[a]);
            misc_gui.handle_player_action(player_actions[a]);
        }
    }
    
    top_gui.tick(game.delta_t);
    controls_gui.tick(game.delta_t);
    graphics_gui.tick(game.delta_t);
    audio_gui.tick(game.delta_t);
    misc_gui.tick(game.delta_t);
    
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string options_menu_state::get_name() const {
    return "options menu";
}


/* ----------------------------------------------------------------------------
 * Goes to the controls menu.
 */
void options_menu_state::go_to_control_binds() {
    game.fade_mgr.start_fade(false, [] () {
        game.change_state(game.states.control_binds_menu);
    });
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 * ev:
 *   Event to handle.
 */
void options_menu_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    top_gui.handle_event(ev);
    controls_gui.handle_event(ev);
    graphics_gui.handle_event(ev);
    audio_gui.handle_event(ev);
    misc_gui.handle_event(ev);
}


/* ----------------------------------------------------------------------------
 * Initializes the audio options menu GUI.
 */
void options_menu_state::init_gui_audio_page() {
    //Menu items.
    audio_gui.register_coords("back",    12,  5, 20, 6);
    audio_gui.register_coords("header",  50, 10, 50, 6);
    audio_gui.register_coords("tooltip", 50, 96, 96, 4);
    audio_gui.read_coords(
        data_node(OPTIONS_MENU::AUDIO_GUI_FILE_PATH).
        get_child_by_name("positions")
    );
    
    //Back button.
    audio_gui.back_item =
        new button_gui_item("Back", game.fonts.standard);
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
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "AUDIO OPTIONS",
        game.fonts.area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    audio_gui.add_item(header_text, "header");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&audio_gui);
    audio_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    //TODO audio_gui.set_selected_item();
    audio_gui.responsive = false;
    audio_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Initializes the controls options menu GUI.
 */
void options_menu_state::init_gui_controls_page() {
    //Menu items.
    controls_gui.register_coords("back",          12,  5,   20,  6);
    controls_gui.register_coords("header",        50, 10,   50,  6);
    controls_gui.register_coords("control_binds", 50, 27.5, 70, 15);
    controls_gui.register_coords("cursor_speed",  50, 50,   70, 15);
    controls_gui.register_coords("auto_throw",    50, 67.5, 70, 15);
    controls_gui.register_coords("tooltip",       50, 96,   96,  4);
    controls_gui.read_coords(
        data_node(OPTIONS_MENU::CONTROLS_GUI_FILE_PATH).
        get_child_by_name("positions")
    );
    
    //Back button.
    controls_gui.back_item =
        new button_gui_item("Back", game.fonts.standard);
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
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "CONTROLS OPTIONS",
        game.fonts.area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    controls_gui.add_item(header_text, "header");
    
    //Control binds button.
    button_gui_item* control_binds_button =
        new button_gui_item("Edit control binds...", game.fonts.standard);
    control_binds_button->on_activate =
    [this] (const point &) {
        go_to_control_binds();
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
    cursor_speed_picker->value_to_string = [] (const float v) {
        return f2s(v);
    };
    cursor_speed_picker->init();
    controls_gui.add_item(cursor_speed_picker, "cursor_speed");
    
    //Auto-throw mode.
    auto_throw_picker =
        new options_menu_picker_gui_item<AUTO_THROW_MODES>(
        "Auto-throw: ",
        &game.options.auto_throw_mode,
        OPTIONS::DEF_AUTO_THROW_MODE,
    {AUTO_THROW_OFF, AUTO_THROW_HOLD, AUTO_THROW_TOGGLE},
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
    controls_gui.set_selected_item(control_binds_button);
    controls_gui.responsive = false;
    controls_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Initializes the graphics options menu GUI.
 */
void options_menu_state::init_gui_graphics_page() {
    //Menu items.
    graphics_gui.register_coords("back",            12,  5,   20,  6);
    graphics_gui.register_coords("header",          50, 10,   50,  6);
    graphics_gui.register_coords("fullscreen",      50, 27.5, 70, 15);
    graphics_gui.register_coords("resolution",      50, 45,   70, 15);
    graphics_gui.register_coords("tooltip",         50, 96,   96,  4);
    graphics_gui.register_coords("restart_warning", 50, 85,   70,  6);
    graphics_gui.read_coords(
        data_node(OPTIONS_MENU::GRAPHICS_GUI_FILE_PATH).
        get_child_by_name("positions")
    );
    
    //Back button.
    graphics_gui.back_item =
        new button_gui_item("Back", game.fonts.standard);
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
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "GRAPHICS OPTIONS",
        game.fonts.area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    graphics_gui.add_item(header_text, "header");
    
    //Fullscreen checkbox.
    check_gui_item* fullscreen_check =
        new check_gui_item(
        &game.options.intended_win_fullscreen,
        "Fullscreen", game.fonts.standard
    );
    fullscreen_check->on_activate =
    [this, fullscreen_check] (const point &) {
        game.options.intended_win_fullscreen =
            !game.options.intended_win_fullscreen;
        fullscreen_check->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
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
    for(size_t p = 0; p < resolution_presets.size(); ++p) {
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
        game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_CENTER
    );
    warning_text->visible = false;
    graphics_gui.add_item(warning_text, "restart_warning");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&graphics_gui);
    graphics_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    graphics_gui.set_selected_item(fullscreen_check);
    graphics_gui.responsive = false;
    graphics_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Initializes the misc. options menu GUI.
 */
void options_menu_state::init_gui_misc_page() {
    //Menu items.
    misc_gui.register_coords("back",                 12,  5,   20,  6);
    misc_gui.register_coords("header",               50, 10,   50,  6);
    misc_gui.register_coords("cursor_cam_weight",    50, 25,   70, 15);
    misc_gui.register_coords("show_hud_input_icons", 50, 42.5, 70, 15);
    misc_gui.register_coords("leaving_confirmation", 50, 60,   70, 15);
    misc_gui.register_coords("tooltip",              50, 96,   96,  4);
    misc_gui.read_coords(
        data_node(OPTIONS_MENU::MISC_GUI_FILE_PATH).
        get_child_by_name("positions")
    );
    
    //Back button.
    misc_gui.back_item =
        new button_gui_item("Back", game.fonts.standard);
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
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "MISC. OPTIONS",
        game.fonts.area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
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
    cursor_cam_weight_picker->value_to_string = [] (const float v) {
        return f2s(v);
    };
    cursor_cam_weight_picker->init();
    misc_gui.add_item(cursor_cam_weight_picker, "cursor_cam_weight");
    
    //Show HUD player input icons checkbox.
    check_gui_item* show_hud_input_icons_check =
        new check_gui_item(
        &game.options.show_hud_input_icons,
        "Show input icons on HUD", game.fonts.standard
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
        new options_menu_picker_gui_item<LEAVING_CONFIRMATION_MODES>(
        "Leave confirm: ",
        &game.options.leaving_confirmation_mode,
    OPTIONS::DEF_LEAVING_CONFIRMATION_MODE, {
        LEAVING_CONFIRMATION_ALWAYS,
        LEAVING_CONFIRMATION_1_MIN,
        LEAVING_CONFIRMATION_NEVER
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
    misc_gui.set_selected_item(cursor_cam_weight_picker);
    misc_gui.responsive = false;
    misc_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Initializes the top-level menu GUI.
 */
void options_menu_state::init_gui_top_page() {
    //Menu items.
    top_gui.register_coords("back",     12,  5,   20,  6);
    top_gui.register_coords("header",   50, 10,   50,  6);
    top_gui.register_coords("controls", 50, 27.5, 65, 10);
    top_gui.register_coords("graphics", 50, 42.5, 65, 10);
    top_gui.register_coords("audio",    50, 57.5, 65, 10);
    top_gui.register_coords("misc",     50, 72.5, 50, 10);
    top_gui.register_coords("advanced", 87, 86,   22,  8);
    top_gui.register_coords("tooltip",  50, 96,   96,  4);
    top_gui.read_coords(
        data_node(OPTIONS_MENU::TOP_GUI_FILE_PATH).
        get_child_by_name("positions")
    );
    
    //Back button.
    top_gui.back_item =
        new button_gui_item("Back", game.fonts.standard);
    top_gui.back_item->on_activate =
    [this] (const point &) {
        leave();
    };
    top_gui.back_item->on_get_tooltip =
    [] () { return "Return to the main menu."; };
    top_gui.add_item(top_gui.back_item, "back");
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "OPTIONS",
        game.fonts.area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    top_gui.add_item(header_text, "header");
    
    //Controls options button.
    button_gui_item* controls_button =
        new button_gui_item("Controls", game.fonts.standard);
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
        new button_gui_item("Graphics", game.fonts.standard);
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
        new button_gui_item("Audio", game.fonts.standard);
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
    
    //Misc. options button.
    button_gui_item* misc_button =
        new button_gui_item("Misc.", game.fonts.standard);
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
    bullet_point_gui_item* advanced_bullet =
        new bullet_point_gui_item("Advanced...", game.fonts.standard);
    advanced_bullet->on_get_tooltip =
    [] () {
        return
            "For more advanced options, check out the "
            "manual in the game's folder.";
    };
    top_gui.add_item(advanced_bullet, "advanced");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&top_gui);
    top_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    game.fade_mgr.start_fade(true, nullptr);
    top_gui.set_selected_item(controls_button);
}


/* ----------------------------------------------------------------------------
 * Leaves the options menu and goes to the main menu.
 */
void options_menu_state::leave() {
    game.fade_mgr.start_fade(false, [] () {
        game.change_state(game.states.main_menu);
    });
    save_options();
}


/* ----------------------------------------------------------------------------
 * Loads the options menu into memory.
 */
void options_menu_state::load() {
    //Resources.
    bmp_menu_bg = load_bmp(game.asset_file_names.main_menu);
    
    init_gui_top_page();
    init_gui_controls_page();
    init_gui_graphics_page();
    init_gui_audio_page();
    init_gui_misc_page();
    
    switch(page_to_load) {
    case OPTIONS_MENU_PAGE_TOP: {
        top_gui.responsive = true;
        top_gui.show_items();
        break;
    } case OPTIONS_MENU_PAGE_CONTROLS: {
        controls_gui.responsive = true;
        controls_gui.show_items();
        break;
    }
    }
    page_to_load = OPTIONS_MENU_PAGE_TOP;
}


/* ----------------------------------------------------------------------------
 * Triggers the restart warning at the bottom of the screen.
 */
void options_menu_state::trigger_restart_warning() {
    if(!warning_text->visible) {
        warning_text->visible = true;
        warning_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
    }
}


/* ----------------------------------------------------------------------------
 * Unloads the options menu from memory.
 */
void options_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu items.
    top_gui.destroy();
    controls_gui.destroy();
    graphics_gui.destroy();
    audio_gui.destroy();
    misc_gui.destroy();
}
