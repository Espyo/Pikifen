/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Global game-related functions.
 */

#include <allegro5/allegro_native_dialog.h>

#include "game.h"

#include "drawing.h"
#include "functions.h"
#include "init.h"
#include "load.h"


/* ----------------------------------------------------------------------------
 * Constructor for the game class.
 */
game_class::game_class() :
    animation_editor_state(nullptr),
    area_editor_state(nullptr),
    area_menu_state(nullptr),
    controls_menu_state(nullptr),
    delta_t(0.0),
    display(nullptr),
    errors_reported_so_far(0),
    framerate_last_avg_point(0),
    gameplay_state(nullptr),
    is_game_running(true),
    intended_win_fullscreen(DEF_WIN_FULLSCREEN),
    intended_win_h(DEF_WIN_H),
    intended_win_w(DEF_WIN_W),
    main_menu_state(nullptr),
    options_menu_state(nullptr),
    target_fps(DEF_TARGET_FPS),
    win_fullscreen(DEF_WIN_FULLSCREEN),
    win_h(DEF_WIN_H),
    win_w(DEF_WIN_W),
    cur_state(nullptr),
    reset_delta_t(true) {
    
}


/* ----------------------------------------------------------------------------
 * Changes to a different game state.
 */
void game_class::change_state(game_state* new_state) {

    if(cur_state) {
        cur_state->unload();
    }
    cur_state = new_state;
    cur_state->load();
    
    //Because during the loading screens, there is no activity, on the
    //next frame, the game will assume the time between that and the last
    //non-loading frame is normal. This could be something like 2 seconds.
    //Let's reset the delta_t, then.
    reset_delta_t = true;
}


/* ----------------------------------------------------------------------------
 * Returns the name of the current state.
 */
string game_class::get_cur_state_name() {
    if(cur_state) {
        return cur_state->get_name();
    }
    return "none";
}


/* ----------------------------------------------------------------------------
 * The main loop of the program. Processes events, ticks frames of gameplay,
 * etc.
 */
void game_class::main_loop() {
    bool cursor_in_window = true;
    //Used to calculate the time difference between the current and last frames.
    double prev_frame_time = 0.0;
    bool window_found = true;
    bool window_focused = true;
    ALLEGRO_EVENT ev;
    
    //Main loop.
    al_start_timer(logic_timer);
    while(is_game_running) {
    
        /*  ************************************************
          *** | _ |                                  | _ | ***
        *****  \_/           EVENT HANDLING           \_/  *****
          *** +---+                                  +---+ ***
            ************************************************/
        
        al_wait_for_event(logic_queue, &ev);
        
        cur_state->handle_controls(ev);
        
        if(
            ev.type == ALLEGRO_EVENT_TIMER &&
            al_is_event_queue_empty(logic_queue)
        ) {
            double cur_time = al_get_time();
            if(reset_delta_t) {
                //Failsafe.
                prev_frame_time = cur_time - 1.0f / target_fps;
                reset_delta_t = false;
            }
            
            //Anti speed-burst cap.
            delta_t = std::min(cur_time - prev_frame_time, 0.2);
            
            game_state* prev_state = cur_state;
            
            cur_state->do_logic();
            if(cur_state == prev_state) {
                //Only draw if we didn't change states in the meantime.
                cur_state->do_drawing();
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
    }
}


/* ----------------------------------------------------------------------------
 * Shuts down the program, cleanly freeing everything.
 */
void game_class::shutdown() {
    if(cur_state) {
        cur_state->unload();
    }
    unload_misc_resources();
    destroy_mob_categories();
    destroy_game_states();
    destroy_misc();
    destroy_event_things(logic_timer, logic_queue);
    destroy_allegro();
}


/* ----------------------------------------------------------------------------
 * Starts up the program, setting up everything that's necessary.
 * Returns 0 if everything is okay, otherwise a return code to quit the
 * program with.
 */
int game_class::start() {
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
    init_game_states();
    
    //Controls and options.
    init_controls();
    load_options();
    save_options();
    
    //Event stuff.
    init_event_things(logic_timer, logic_queue);
    
    //Other fundamental initializations and loadings.
    init_asset_file_names();
    init_misc();
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
        game.gameplay_state->area_to_load =
            creator_tool_auto_start_option;
        game.change_state(game.gameplay_state);
    } else if(
        creator_tools_enabled &&
        creator_tool_auto_start_mode == "animation_editor"
    ) {
        game.animation_editor_state->auto_load_anim =
            creator_tool_auto_start_option;
        game.change_state(game.animation_editor_state);
    } else if(
        creator_tools_enabled &&
        creator_tool_auto_start_mode == "area_editor"
    ) {
        game.area_editor_state->auto_load_area =
            creator_tool_auto_start_option;
        game.change_state(game.area_editor_state);
    } else {
        game.change_state(game.main_menu_state);
    }
    
    return 0;
}


game_class game = game_class();
