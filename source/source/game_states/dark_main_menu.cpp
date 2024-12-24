/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * "Dark", full-screen main menu state class and related functions.
 */

#include <algorithm>

#include "../game.h"
#include "../utils/allegro_utils.h"
#include "menus.h"


/**
 * @brief Draws the dark main menu.
 */
void dark_main_menu_state::do_drawing() {
    al_clear_to_color(COLOR_BLACK);
    
    draw_bitmap(
        bmp_menu_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h), 0, map_gray(64)
    );
    
    if(help_menu) help_menu->draw();
    
    draw_mouse_cursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Ticks one frame's worth of logic.
 */
void dark_main_menu_state::do_logic() {
    vector<player_action> player_actions = game.controls.new_frame();
    if(!game.fade_mgr.is_fading()) {
        for(size_t a = 0; a < player_actions.size(); a++) {
            if(help_menu) help_menu->handle_player_action(player_actions[a]);
        }
    }
    
    if(help_menu) {
        if(!help_menu->to_delete) {
            help_menu->tick(game.delta_t);
        } else {
            delete help_menu;
            help_menu = nullptr;
        }
    }
    
    game.fade_mgr.tick(game.delta_t);
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string dark_main_menu_state::get_name() const {
    return "dark main menu";
}


/**
 * @brief Handles Allegro events.
 *
 * @param ev Event to handle.
 */
void dark_main_menu_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    if(help_menu) help_menu->handle_event(ev);
}


/**
 * @brief Leaves the dark main menu and goes to the regular main menu.
 */
void dark_main_menu_state::leave() {
    game.fade_mgr.start_fade(false, [] () {
        game.change_state(game.states.main_menu);
    });
}


/**
 * @brief Loads the dark main menu into memory.
 */
void dark_main_menu_state::load() {
    //Resources.
    bmp_menu_bg = game.content.bitmaps.list.get(game.asset_file_names.bmp_main_menu);
    
    //Game content.
    game.content.reload_packs();
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    //Load the intended concrete menu.
    switch(menu_to_load) {
    case DARK_MAIN_MENU_MENU_HELP: {
        help_menu = new help_menu_t();
        help_menu->back_callback = [this] () { leave(); };
        break;
    }
    }
    menu_to_load = DARK_MAIN_MENU_MENU_HELP;
    
    //Finishing touches.
    game.audio.set_current_song(OPTIONS_MENU::SONG_NAME);
}


/**
 * @brief Unloads the dark main menu from memory.
 */
void dark_main_menu_state::unload() {
    //Resources.
    game.content.bitmaps.list.free(bmp_menu_bg);
    
    //Menus.
    if(help_menu) {
        delete help_menu;
        help_menu = nullptr;
    }
    
    //Game content.
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    }
    );
}
