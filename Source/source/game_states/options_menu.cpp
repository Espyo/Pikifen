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
//Path to the GUI information file.
const string GUI_FILE_PATH = GUI_FOLDER_PATH + "/Options_menu.txt";
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
    
    gui.draw();
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void options_menu_state::do_logic() {
    vector<player_action> player_actions = game.controls.new_frame();
    for(size_t a = 0; a < player_actions.size(); ++a) {
        gui.handle_player_action(player_actions[a]);
    }
    
    gui.tick(game.delta_t);
    
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
void options_menu_state::go_to_controls() {
    game.fade_mgr.start_fade(false, [] () {
        game.change_state(game.states.controls_menu);
    });
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 * ev:
 *   Event to handle.
 */
void options_menu_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    gui.handle_event(ev);
    game.controls.handle_allegro_event(ev);
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
    
    //Menu items.
    gui.register_coords("back",                 12,  5, 20, 6);
    gui.register_coords("fullscreen",           24, 18, 40, 8);
    gui.register_coords("resolution",           76, 18, 40, 8);
    gui.register_coords("cursor_speed",         24, 32, 40, 8);
    gui.register_coords("cursor_cam_weight",    76, 32, 40, 8);
    gui.register_coords("auto_throw",           50, 44, 44, 8);
    gui.register_coords("show_hud_input_icons",    50, 54, 44, 8);
    gui.register_coords("leaving_confirmation", 50, 64, 44, 8);
    gui.register_coords("controls",             50, 74, 44, 8);
    gui.register_coords("advanced",             87, 86, 22, 8);
    gui.register_coords("tooltip",              50, 96, 96, 4);
    gui.register_coords("restart_warning",      63,  5, 70, 6);
    gui.read_coords(
        data_node(OPTIONS_MENU::GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    //Back button.
    gui.back_item =
        new button_gui_item("Back", game.fonts.standard);
    gui.back_item->on_activate =
    [this] (const point &) {
        leave();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Return to the main menu."; };
    gui.add_item(gui.back_item, "back");
    
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
    gui.add_item(fullscreen_check, "fullscreen");
    
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
    gui.add_item(resolution_picker, "resolution");
    
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
    gui.add_item(cursor_speed_picker, "cursor_speed");
    
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
    gui.add_item(cursor_cam_weight_picker, "cursor_cam_weight");
    
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
    gui.add_item(auto_throw_picker, "auto_throw");
    
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
    gui.add_item(show_hud_input_icons_check, "show_hud_input_icons");
    
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
    gui.add_item(leaving_confirmation_picker, "leaving_confirmation");
    
    //Controls button.
    button_gui_item* controls_button =
        new button_gui_item("Edit controls...", game.fonts.standard);
    controls_button->on_activate =
    [this] (const point &) {
        go_to_controls();
    };
    controls_button->on_get_tooltip =
    [] () { return "Choose what buttons do what."; };
    gui.add_item(controls_button, "controls");
    
    //Advanced bullet point.
    bullet_point_gui_item* advanced_bullet =
        new bullet_point_gui_item("Advanced...", game.fonts.standard);
    advanced_bullet->on_get_tooltip =
    [] () {
        return
            "For more advanced options, check out the "
            "manual in the game's folder.";
    };
    gui.add_item(advanced_bullet, "advanced");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&gui);
    gui.add_item(tooltip_text, "tooltip");
    
    //Warning text.
    warning_text =
        new text_gui_item(
        "Please restart for the changes to take effect.",
        game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    warning_text->visible = false;
    gui.add_item(warning_text, "restart_warning");
    
    //Finishing touches.
    game.fade_mgr.start_fade(true, nullptr);
    gui.set_selected_item(gui.back_item);
    
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
    gui.destroy();
}
