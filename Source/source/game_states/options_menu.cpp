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
    
    if(!warning_widget->enabled) {
        warning_widget->enabled = true;
        warning_widget->start_juicy_grow();
    }
    resolution_widget->start_juicy_grow();
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
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void options_menu_state::do_logic() {
    time_spent += game.delta_t;
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(game.delta_t);
    }
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
    
    handle_widget_events(ev);
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
    
    //Menu widgets.
    back_widget =
        new menu_button(
        point(game.win_w * 0.15, game.win_h * 0.10),
        point(game.win_w * 0.20, game.win_h * 0.06),
    [this] () {
        leave();
    },
    "Back", game.fonts.main
    );
    menu_widgets.push_back(back_widget);
    
    fullscreen_widget =
        new menu_checkbox(
        point(game.win_w * 0.25, game.win_h * 0.20),
        point(game.win_w * 0.45, game.win_h * 0.08),
    [this] () {
        game.options.intended_win_fullscreen = this->fullscreen_widget->checked;
        warning_widget->enabled = true;
        update();
    },
    "Fullscreen", game.fonts.main
    );
    menu_widgets.push_back(fullscreen_widget);
    
    menu_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.05, game.win_h * 0.30),
            point(game.win_w * 0.05, game.win_h * 0.08),
    [this] () {
        change_resolution(-1);
    },
    "<", game.fonts.main
        )
    );
    
    resolution_widget =
        new menu_text(
        point(game.win_w * 0.26, game.win_h * 0.30),
        point(game.win_w * 0.35, game.win_h * 0.08),
        "Resolution: ", game.fonts.main,
        al_map_rgb(255, 255, 255), ALLEGRO_ALIGN_LEFT
    );
    menu_widgets.push_back(resolution_widget);
    
    menu_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.45, game.win_h * 0.30),
            point(game.win_w * 0.05, game.win_h * 0.08),
    [this] () {
        change_resolution(1);
    },
    ">", game.fonts.main
        )
    );
    
    menu_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.25, game.win_h * 0.40),
            point(game.win_w * 0.45, game.win_h * 0.08),
    [this] () {
        go_to_controls();
    },
    "Edit controls...", game.fonts.main,
    al_map_rgb(255, 255, 255), ALLEGRO_ALIGN_LEFT
        )
    );
    
    warning_widget =
        new menu_text(
        point(game.win_w * 0.50, game.win_h * 0.95),
        point(game.win_w * 0.95, game.win_h * 0.10),
        "Please restart for the changes to take effect.", game.fonts.main
    );
    warning_widget->enabled = false;
    menu_widgets.push_back(warning_widget);
    
    //Finishing touches.
    game.fade_mgr.start_fade(true, nullptr);
    set_selected_widget(menu_widgets[0]);
    update();
    
}


/* ----------------------------------------------------------------------------
 * Unloads the options menu from memory.
 */
void options_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu widgets.
    set_selected_widget(NULL);
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    
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
    
    resolution_widget->text =
        "Resolution: " +
        i2s(game.options.intended_win_w) + "x" +
        i2s(game.options.intended_win_h) +
        (current_r_index == INVALID ? " (Custom)" : "");
        
    fullscreen_widget->checked = game.options.intended_win_fullscreen;
}
