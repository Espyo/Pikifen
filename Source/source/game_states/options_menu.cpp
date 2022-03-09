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


//Auto-throw preset values.
const AUTO_THROW_MODES options_menu_state::AUTO_THROW_PRESETS[] =
{AUTO_THROW_OFF, AUTO_THROW_HOLD, AUTO_THROW_TOGGLE};
//Auto-throw preset names.
const string options_menu_state::AUTO_THROW_PRESET_NAMES[] =
{"Off", "Hold button", "Button toggles"};
//Auto-throw preset amount.
const unsigned char options_menu_state::N_AUTO_THROW_PRESETS = 3;
//Cursor speed preset values.
const float options_menu_state::CURSOR_SPEED_PRESETS[] =
{250.0f, 350.0f, 500.0f, 700.0f, 1000.0f};
//Cursor speed preset names.
const string options_menu_state::CURSOR_SPEED_PRESET_NAMES[] =
{"Very slow", "Slow", "Medium", "Fast", "Very fast"};
//Cursor speed preset amount.
const unsigned char options_menu_state::N_CURSOR_SPEED_PRESETS = 5;
//Path to the GUI information file.
const string options_menu_state::GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Options_menu.txt";


/* ----------------------------------------------------------------------------
 * Creates an "options menu" state.
 */
options_menu_state::options_menu_state() :
    game_state() {
    
    //Let's fill in the list of preset resolutions. For that, we'll get
    //the display modes fetched by Allegro. These are usually nice round
    //resolutions, and they work on fullscreen mode.
    int n_modes = al_get_num_display_modes();
    for(int d = 0; d < n_modes; ++d) {
        ALLEGRO_DISPLAY_MODE d_info;
        if(!al_get_display_mode(d, &d_info)) continue;
        if(d_info.width < SMALLEST_WIN_W) continue;
        if(d_info.height < SMALLEST_WIN_H) continue;
        resolution_presets.push_back(
            std::make_pair(d_info.width, d_info.height)
        );
    }
    
    //In case things go wrong, at least add these presets.
    resolution_presets.push_back(
        std::make_pair(options_struct::DEF_WIN_W, options_struct::DEF_WIN_H)
    );
    resolution_presets.push_back(
        std::make_pair(SMALLEST_WIN_W, SMALLEST_WIN_H)
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
 * Changes to the next auto-throw mode preset in the list.
 * step:
 *   How much to move forward in the list.
 */
void options_menu_state::change_auto_throw(const signed int step) {
    size_t cur_auto_throw_idx = INVALID;
    
    for(size_t m = 0; m < N_AUTO_THROW_PRESETS; ++m) {
        if(game.options.auto_throw_mode == AUTO_THROW_PRESETS[m]) {
            cur_auto_throw_idx = m;
            break;
        }
    }
    
    if(cur_auto_throw_idx == INVALID) {
        cur_auto_throw_idx = 0;
    } else {
        cur_auto_throw_idx =
            sum_and_wrap(cur_auto_throw_idx, step, N_AUTO_THROW_PRESETS);
    }
    
    game.options.auto_throw_mode = AUTO_THROW_PRESETS[cur_auto_throw_idx];
    
    auto_throw_picker->start_juice_animation(
        gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    update();
}


/* ----------------------------------------------------------------------------
 * Changes to the next cursor speed preset in the list.
 * step:
 *   How much to move forward in the list.
 */
void options_menu_state::change_cursor_speed(const signed int step) {
    size_t cur_cursor_speed_idx = INVALID;
    
    for(size_t s = 0; s < N_CURSOR_SPEED_PRESETS; ++s) {
        if(game.options.cursor_speed == CURSOR_SPEED_PRESETS[s]) {
            cur_cursor_speed_idx = s;
            break;
        }
    }
    
    if(cur_cursor_speed_idx == INVALID) {
        cur_cursor_speed_idx = 0;
    } else {
        cur_cursor_speed_idx =
            sum_and_wrap(cur_cursor_speed_idx, step, N_CURSOR_SPEED_PRESETS);
    }
    
    game.options.cursor_speed = CURSOR_SPEED_PRESETS[cur_cursor_speed_idx];
    
    cursor_speed_picker->start_juice_animation(
        gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    update();
}


/* ----------------------------------------------------------------------------
 * Changes to the next resolution preset in the list.
 * step:
 *   How much to move forward in the list.
 */
void options_menu_state::change_resolution(const signed int step) {
    size_t cur_resolution_idx = INVALID;
    
    for(size_t r = 0; r < resolution_presets.size(); ++r) {
        if(
            game.options.intended_win_w == resolution_presets[r].first &&
            game.options.intended_win_h == resolution_presets[r].second
        ) {
            cur_resolution_idx = r;
            break;
        }
    }
    
    if(cur_resolution_idx == INVALID) {
        cur_resolution_idx = 0;
    } else {
        cur_resolution_idx =
            sum_and_wrap(cur_resolution_idx, step, resolution_presets.size());
    }
    
    game.options.intended_win_w = resolution_presets[cur_resolution_idx].first;
    game.options.intended_win_h = resolution_presets[cur_resolution_idx].second;
    
    trigger_restart_warning();
    resolution_picker->start_juice_animation(
        gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    update();
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
    game.fade_mgr.tick(game.delta_t);
    
    gui.tick(game.delta_t);
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
    gui.register_coords("back",              15, 10, 20,  6);
    gui.register_coords("fullscreen",        24, 20, 45,  8);
    gui.register_coords("resolution",        24, 30, 45,  8);
    gui.register_coords("cursor_speed",      24, 45, 45,  8);
    gui.register_coords("auto_throw",        24, 55, 45,  8);
    gui.register_coords("show_hud_controls", 24, 65, 45,  8);
    gui.register_coords("controls",          24, 75, 45,  8);
    gui.register_coords("tooltip",           50, 95, 95,  8);
    gui.register_coords("restart_warning",   60,  5, 70,  8);
    gui.read_coords(
        data_node(GUI_FILE_PATH).get_child_by_name("positions")
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
    [this] (const point &) {
        game.options.intended_win_fullscreen =
            !game.options.intended_win_fullscreen;
        trigger_restart_warning();
    };
    fullscreen_check->on_get_tooltip =
    [] () { return "Show the game in fullscreen, or in a window?"; };
    gui.add_item(fullscreen_check, "fullscreen");
    
    //Resolution picker.
    resolution_picker =
        new picker_gui_item("Resolution: ", "");
    resolution_picker->on_previous =
    [this] () {
        change_resolution(-1);
    };
    resolution_picker->on_next =
    [this] () {
        change_resolution(1);
    };
    resolution_picker->on_get_tooltip =
    [] () { return "The game's width and height."; };
    gui.add_item(resolution_picker, "resolution");
    
    //Cursor speed.
    cursor_speed_picker =
        new picker_gui_item("Cursor speed: ", "");
    cursor_speed_picker->on_previous =
    [this] () {
        change_cursor_speed(-1);
    };
    cursor_speed_picker->on_next =
    [this] () {
        change_cursor_speed(1);
    };
    cursor_speed_picker->on_get_tooltip =
    [] () { return "Cursor speed, when controlling without a mouse."; };
    gui.add_item(cursor_speed_picker, "cursor_speed");
    
    //Auto-throw mode.
    auto_throw_picker =
        new picker_gui_item("Auto-throw: ", "");
    auto_throw_picker->on_previous =
    [this] () {
        change_auto_throw(-1);
    };
    auto_throw_picker->on_next =
    [this] () {
        change_auto_throw(1);
    };
    auto_throw_picker->on_get_tooltip =
    [] () {
        switch(game.options.auto_throw_mode) {
        case AUTO_THROW_OFF: {
            return "Pikmin are only thrown when you release the button.";
        }
        case AUTO_THROW_HOLD: {
            return "Auto-throw Pikmin periodically as long as "
                   "the button is held.";
        }
        case AUTO_THROW_TOGGLE: {
            return "Press once to auto-throw Pikmin periodically, and again "
                   "to stop.";
        }
        default: {
            return "";
        }
        }
    };
    gui.add_item(auto_throw_picker, "auto_throw");
    
    //Show HUD controls checkbox.
    check_gui_item* show_hud_controls_check =
        new check_gui_item(
        &game.options.show_hud_controls,
        "Show controls on HUD", game.fonts.standard
    );
    show_hud_controls_check->on_get_tooltip =
    [] () { return "Show icons of the controls near relevant HUD items?"; };
    gui.add_item(show_hud_controls_check, "show_hud_controls");
    
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
    
    //Tooltip text.
    text_gui_item* tooltip_text =
        new text_gui_item("", game.fonts.standard);
    tooltip_text->on_draw =
        [this]
    (const point & center, const point & size) {
        draw_compressed_scaled_text(
            game.fonts.standard, COLOR_WHITE,
            center, point(0.7f, 0.7f),
            ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER, size, false,
            gui.get_current_tooltip()
        );
    };
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
    update();
    
}


/* ----------------------------------------------------------------------------
 * Triggers the restart warning at the bottom of the screen.
 */
void options_menu_state::trigger_restart_warning() {
    if(!warning_text->visible) {
        warning_text->visible = true;
        warning_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
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


/* ----------------------------------------------------------------------------
 * Updates the contents of the options menu.
 */
void options_menu_state::update() {
    //Resolution.
    size_t cur_resolution_idx = INVALID;
    
    for(size_t r = 0; r < resolution_presets.size(); ++r) {
        if(
            game.options.intended_win_w == resolution_presets[r].first &&
            game.options.intended_win_h == resolution_presets[r].second
        ) {
            cur_resolution_idx = r;
            break;
        }
    }
    
    resolution_picker->option =
        i2s(game.options.intended_win_w) + "x" +
        i2s(game.options.intended_win_h) +
        (cur_resolution_idx == INVALID ? " (custom)" : "");
        
    //Cursor speed.
    size_t cur_cursor_speed_idx = INVALID;
    
    for(size_t s = 0; s < N_CURSOR_SPEED_PRESETS; ++s) {
        if(game.options.cursor_speed == CURSOR_SPEED_PRESETS[s]) {
            cur_cursor_speed_idx = s;
            break;
        }
    }
    
    cursor_speed_picker->option =
        cur_cursor_speed_idx == INVALID ?
        i2s(game.options.cursor_speed) + " (custom)" :
        CURSOR_SPEED_PRESET_NAMES[cur_cursor_speed_idx];
        
    //Auto-throw.
    size_t cur_auto_throw_idx = INVALID;
    
    for(size_t m = 0; m < N_AUTO_THROW_PRESETS; ++m) {
        if(game.options.auto_throw_mode == AUTO_THROW_PRESETS[m]) {
            cur_auto_throw_idx = m;
            break;
        }
    }
    
    auto_throw_picker->option = AUTO_THROW_PRESET_NAMES[cur_auto_throw_idx];
}
