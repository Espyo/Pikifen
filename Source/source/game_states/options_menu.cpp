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
 * Changes to the next resolution preset in the list.
 * step:
 *   How much to move forward in the list.
 */
void options_menu_state::change_resolution(const signed int step) {
    size_t current_r_index = INVALID;
    
    for(size_t r = 0; r < resolution_presets.size(); ++r) {
        if(
            game.options.intended_win_w == resolution_presets[r].first &&
            game.options.intended_win_h == resolution_presets[r].second
        ) {
            current_r_index = r;
            break;
        }
    }
    
    if(current_r_index == INVALID) {
        current_r_index = 0;
    } else {
        current_r_index =
            sum_and_wrap(current_r_index, step, resolution_presets.size());
    }
    
    game.options.intended_win_w = resolution_presets[current_r_index].first;
    game.options.intended_win_h = resolution_presets[current_r_index].second;
    
    trigger_restart_warning();
    resolution_picker->start_juicy_grow();
    update();
}


/* ----------------------------------------------------------------------------
 * Draws the options menu.
 */
void options_menu_state::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
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
    gui.register_coords("back",            15, 10, 20,  6);
    gui.register_coords("fullscreen",      24, 20, 45,  8);
    gui.register_coords("resolution",      24, 30, 45,  8);
    gui.register_coords("controls",        24, 40, 45,  8);
    gui.register_coords("restart_warning", 50, 95, 95, 10);
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
    gui.add_item(resolution_picker, "resolution");
    
    //Controls button.
    button_gui_item* controls_button =
        new button_gui_item("Edit controls...", game.fonts.standard);
    controls_button->on_activate =
    [this] (const point &) {
        go_to_controls();
    };
    gui.add_item(controls_button, "controls");
    
    //Warning text.
    warning_text =
        new text_gui_item(
        "Please restart for the changes to take effect.",
        game.fonts.standard
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
        warning_text->start_juicy_grow();
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
    size_t current_r_index = INVALID;
    
    for(size_t r = 0; r < resolution_presets.size(); ++r) {
        if(
            game.options.intended_win_w == resolution_presets[r].first &&
            game.options.intended_win_h == resolution_presets[r].second
        ) {
            current_r_index = r;
            break;
        }
    }
    
    resolution_picker->option =
        i2s(game.options.intended_win_w) + "x" +
        i2s(game.options.intended_win_h) +
        (current_r_index == INVALID ? "(Custom)" : "");
}
