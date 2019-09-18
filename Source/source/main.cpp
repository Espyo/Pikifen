/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program start and main loop.
 */

#include <algorithm>
#include <fstream>
#include <math.h>
#include <string>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"
#include "controls.h"
#include "drawing.h"
#include "editors/animation_editor/editor.h"
#include "editors/area_editor/editor.h"
#include "functions.h"
#include "init.h"
#include "load.h"
#include "menus.h"
#include "sector.h"
#include "vars.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Main function.
 * It begins by loading Allegro stuff,
 * the options, setting some settings,
 * and loading all of the game content.
 * Once that's done, it enters the main loop.
 */
int main(int argc, char** argv) {
    //Allegro initializations.
    init_allegro();
    
    //Panic check: is there a Game_data folder?
    if(folder_to_vector(GAME_DATA_FOLDER_PATH, true).empty()) {
        show_message_box(
            NULL, "Game_data folder not found!",
            "Game_data folder not found!",
            "Could not find the \"Game_data\" folder! "
            "If you are running the engine from a zip file, "
            "you have to unpack it first.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        return -1;
    }
    
    //Essentials.
    init_essentials();
    
    //Controls and options.
    init_controls();
    load_options();
    save_options();
    
    //Event stuff.
    ALLEGRO_TIMER* logic_timer;
    ALLEGRO_EVENT_QUEUE* logic_queue;
    ALLEGRO_EVENT ev;
    init_event_things(logic_timer, logic_queue);
    
    //Other fundamental initializations and loadings.
    init_asset_file_names();
    init_misc();
    init_game_states();
    init_error_bitmap();
    load_asset_file_names();
    load_fonts();
    load_misc_graphics();
    load_system_animations();
    load_misc_sounds();
    
    //Draw the basic loading screen.
    draw_loading_screen("", "", 1.0);
    al_flip_display();
    
    //Init and load some other things.
    init_mob_actions();
    init_mob_categories();
    init_sector_types();
    init_hud_items();
    load_game_config();
    load_creator_tools();
    save_creator_tools();
    
    if(
        creator_tools_enabled &&
        creator_tool_auto_start_mode == "play" &&
        !creator_tool_auto_start_option.empty()
    ) {
        area_to_load = creator_tool_auto_start_option;
        change_game_state(GAME_STATE_GAME);
    } else if(
        creator_tools_enabled &&
        creator_tool_auto_start_mode == "animation_editor"
    ) {
        (
            (animation_editor*)
            game_states[GAME_STATE_ANIMATION_EDITOR]
        )->auto_load_anim = creator_tool_auto_start_option;
        change_game_state(GAME_STATE_ANIMATION_EDITOR);
    } else if(
        creator_tools_enabled &&
        creator_tool_auto_start_mode == "area_editor"
    ) {
        (
            (area_editor*)
            game_states[GAME_STATE_AREA_EDITOR]
        )->auto_load_area = creator_tool_auto_start_option;
        change_game_state(GAME_STATE_AREA_EDITOR);
    } else {
        change_game_state(GAME_STATE_MAIN_MENU);
    }
    
    bool cursor_in_window = true;
    bool window_found = true;
    bool window_focused = true;
    
    //Main loop.
    al_start_timer(logic_timer);
    while(is_game_running) {
    
        /*  ************************************************
          *** | _ |                                  | _ | ***
        *****  \_/           EVENT HANDLING           \_/  *****
          *** +---+                                  +---+ ***
            ************************************************/
        
        al_wait_for_event(logic_queue, &ev);
        
        game_states[cur_game_state_nr]->handle_controls(ev);
        
        if(
            ev.type == ALLEGRO_EVENT_TIMER &&
            al_is_event_queue_empty(logic_queue)
        ) {
            double cur_time = al_get_time();
            if(reset_delta_t) {
                //Failsafe.
                prev_frame_time = cur_time - 1.0f / game_fps;
                reset_delta_t = false;
            }
            
            //Anti speed-burst cap.
            delta_t = min(cur_time - prev_frame_time, 0.2);
            
            size_t prev_game_state_nr = cur_game_state_nr;
            
            game_states[cur_game_state_nr]->do_logic();
            if(cur_game_state_nr == prev_game_state_nr) {
                //Only draw if we didn't change states in the meantime.
                game_states[cur_game_state_nr]->do_drawing();
            }
            
            prev_frame_time = cur_time;
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            is_game_running = false;
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
            //scr_w = ev.display.width;
            //scr_h = ev.display.height;
            
        } else if(
            ev.type == ALLEGRO_EVENT_KEY_DOWN &&
            ev.keyboard.keycode == ALLEGRO_KEY_F12
        ) {
            save_screenshot();
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_LOST) {
            window_found = false;
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_FOUND) {
            window_found = true;
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
            window_focused = false;
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
            window_focused = true;
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY) {
            cursor_in_window = false;
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY) {
            cursor_in_window = true;
            
        }
        
        cursor_ready = (window_found && window_focused && cursor_in_window);
    }
    
    if(cur_game_state_nr != INVALID) {
        game_states[cur_game_state_nr]->unload();
    }
    unload_misc_resources();
    destroy_mob_categories();
    destroy_game_states();
    destroy_misc();
    destroy_event_things(logic_timer, logic_queue);
    destroy_allegro();
    
}
