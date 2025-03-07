/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Annex screen state class and related functions.
 */

#include <algorithm>

#include "annex_screen.h"

#include "../core/game.h"
#include "../util/allegro_utils.h"


/**
 * @brief Draws the annex screen state.
 */
void annex_screen_state::do_drawing() {
    al_clear_to_color(COLOR_BLACK);
    
    draw_bitmap(
        bmp_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h), 0, map_gray(64)
    );
    
    if(cur_menu) cur_menu->draw();
    
    draw_mouse_cursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Ticks one frame's worth of logic.
 */
void annex_screen_state::do_logic() {
    vector<player_action> player_actions = game.controls.new_frame();
    
    if(!game.fade_mgr.is_fading()) {
        for(size_t a = 0; a < player_actions.size(); a++) {
            if(cur_menu) cur_menu->handle_player_action(player_actions[a]);
        }
    }
    
    if(cur_menu) {
        if(cur_menu->loaded) {
            cur_menu->tick(game.delta_t);
        }
        if(!cur_menu->loaded) {
            delete cur_menu;
            cur_menu = nullptr;
        }
    }
    
    game.fade_mgr.tick(game.delta_t);
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string annex_screen_state::get_name() const {
    return "annex screen";
}


/**
 * @brief Handles Allegro events.
 *
 * @param ev Event to handle.
 */
void annex_screen_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    if(cur_menu) cur_menu->handle_allegro_event(ev);
}


/**
 * @brief Leaves the annex screen state and goes to the title screen.
 */
void annex_screen_state::leave() {
    game.fade_mgr.start_fade(false, [] () {
        game.change_state(game.states.title_screen);
    });
}


/**
 * @brief Loads the annex screen state into memory.
 */
void annex_screen_state::load() {
    //Resources.
    bmp_bg =
        game.content.bitmaps.list.get(
            game.sys_content_names.bmp_title_screen_bg
        );
        
    //Game content.
    game.content.reload_packs();
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
        CONTENT_TYPE_AREA,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    //Load the intended concrete menu.
    switch(menu_to_load) {
    case ANNEX_SCREEN_MENU_AREA_SELECTION: {
        area_menu_t* area_menu = new area_menu_t();
        area_menu->area_type = area_menu_area_type;
        area_menu->leave_callback =
        [this] () {
            game.states.title_screen->page_to_load = MAIN_MENU_PAGE_PLAY;
            leave();
        };
        cur_menu = area_menu;
        break;
        
    } case ANNEX_SCREEN_MENU_HELP: {
        game.content.load_all(
        vector<CONTENT_TYPE> {
            CONTENT_TYPE_PARTICLE_GEN,
            CONTENT_TYPE_GLOBAL_ANIMATION,
            CONTENT_TYPE_LIQUID,
            CONTENT_TYPE_STATUS_TYPE,
            CONTENT_TYPE_SPRAY_TYPE,
            CONTENT_TYPE_HAZARD,
            CONTENT_TYPE_WEATHER_CONDITION,
            CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        },
        CONTENT_LOAD_LEVEL_BASIC
        );
        game.content.load_all(
        vector<CONTENT_TYPE> {
            CONTENT_TYPE_MOB_ANIMATION,
            CONTENT_TYPE_MOB_TYPE,
        },
        CONTENT_LOAD_LEVEL_FULL
        );
        help_menu_t* help_menu = new help_menu_t();
        help_menu->unload_callback =
        [this] () {
            game.content.unload_all(
            vector<CONTENT_TYPE> {
                CONTENT_TYPE_MOB_ANIMATION,
                CONTENT_TYPE_MOB_TYPE,
                CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
                CONTENT_TYPE_WEATHER_CONDITION,
                CONTENT_TYPE_HAZARD,
                CONTENT_TYPE_SPRAY_TYPE,
                CONTENT_TYPE_STATUS_TYPE,
                CONTENT_TYPE_LIQUID,
                CONTENT_TYPE_GLOBAL_ANIMATION,
                CONTENT_TYPE_PARTICLE_GEN,
            }
            );
        };
        help_menu->leave_callback =
        [this] () {
            game.states.title_screen->page_to_load = MAIN_MENU_PAGE_MAIN;
            leave();
        };
        cur_menu = help_menu;
        break;
        
    } case ANNEX_SCREEN_MENU_OPTIONS: {
        options_menu_t* options_menu = new options_menu_t();
        options_menu->leave_callback = [this] () {
            game.states.title_screen->page_to_load = MAIN_MENU_PAGE_MAIN;
            leave();
        };
        cur_menu = options_menu;
        break;
        
    } case ANNEX_SCREEN_MENU_STATS: {
        stats_menu_t* stats_menu = new stats_menu_t();
        stats_menu->leave_callback = [this] () {
            game.states.title_screen->page_to_load = MAIN_MENU_PAGE_MAIN;
            leave();
        };
        cur_menu = stats_menu;
        break;
        
    }
    }
    
    cur_menu->load();
    cur_menu->enter();
    menu_to_load = ANNEX_SCREEN_MENU_HELP;
    
    //Finishing touches.
    game.audio.set_current_song(game.sys_content_names.sng_menus);
    game.fade_mgr.start_fade(true, nullptr);
}


/**
 * @brief Unloads the annex screen state from memory.
 */
void annex_screen_state::unload() {
    //Resources.
    game.content.bitmaps.list.free(bmp_bg);
    
    //Menus.
    if(cur_menu) {
        cur_menu->unload();
        delete cur_menu;
        cur_menu = nullptr;
    }
    
    //Game content.
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
        CONTENT_TYPE_GUI,
    }
    );
}
