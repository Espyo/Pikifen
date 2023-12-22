/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Global game-related functions.
 */

#include <algorithm>

#include <allegro5/allegro_native_dialog.h>

#include "game.h"

#include "drawing.h"
#include "functions.h"
#include "init.h"
#include "libs/imgui/imgui_impl_allegro5.h"
#include "load.h"


namespace GAME {
//Standard color of the mouse cursor.
const ALLEGRO_COLOR CURSOR_STANDARD_COLOR = al_map_rgb(188, 230, 230);
//Maximum alpha of the cursor's trail -- the alpha value near the cursor.
const unsigned char CURSOR_TRAIL_MAX_ALPHA = 72;
//Maximum width of the cursor's trail -- the width value near the cursor.
const float CURSOR_TRAIL_MAX_WIDTH = 30.0f;
//How far the cursor must move from its current spot before the next spot.
const float CURSOR_TRAIL_MIN_SPOT_DIFF = 4.0f;
//Every X seconds, the cursor's position is saved, to create the trail effect.
const float CURSOR_TRAIL_SAVE_INTERVAL = 0.016f;
//Number of positions of the cursor to keep track of.
const unsigned char CURSOR_TRAIL_SAVE_N_SPOTS = 16;
//Duration of full-screen fades.
const float FADE_DURATION = 0.15f;
//When getting a framerate average, use a sample of this size.
const size_t FRAMERATE_AVG_SAMPLE_SIZE = 30;
//Only save the latest N FPS samples.
const size_t FRAMERATE_HISTORY_SIZE = 300;
}


/* ----------------------------------------------------------------------------
 * Constructor for the game class.
 */
game_class::game_class() :
    bitmaps(""),
    bmp_error(nullptr),
    delta_t(0.0),
    display(nullptr),
    dummy_mob_state(nullptr),
    framerate_last_avg_point(0),
    is_game_running(true),
    liquid_limit_effect_buffer(nullptr),
    loading_subtext_bmp(nullptr),
    loading_text_bmp(nullptr),
    perf_mon(nullptr),
    show_system_info(false),
    textures(TEXTURES_FOLDER_NAME),
    time_passed(0.0f),
    wall_offset_effect_buffer(nullptr),
    win_fullscreen(OPTIONS::DEF_WIN_FULLSCREEN),
    win_h(OPTIONS::DEF_WIN_H),
    win_w(OPTIONS::DEF_WIN_W),
    cur_state(nullptr),
    logic_queue(nullptr),
    logic_timer(nullptr),
    reset_delta_t(true) {
    
    team_internal_names[MOB_TEAM_NONE] = "none";
    team_internal_names[MOB_TEAM_PLAYER_1] = "player_1";
    team_internal_names[MOB_TEAM_PLAYER_2] = "player_2";
    team_internal_names[MOB_TEAM_PLAYER_3] = "player_3";
    team_internal_names[MOB_TEAM_PLAYER_4] = "player_4";
    team_internal_names[MOB_TEAM_ENEMY_1] = "enemy_1";
    team_internal_names[MOB_TEAM_ENEMY_2] = "enemy_2";
    team_internal_names[MOB_TEAM_ENEMY_3] = "enemy_3";
    team_internal_names[MOB_TEAM_ENEMY_4] = "enemy_4";
    team_internal_names[MOB_TEAM_OBSTACLE] = "obstacle";
    team_internal_names[MOB_TEAM_OTHER] = "other";
    
    team_names[MOB_TEAM_NONE] = "None";
    team_names[MOB_TEAM_PLAYER_1] = "Player 1";
    team_names[MOB_TEAM_PLAYER_2] = "Player 2";
    team_names[MOB_TEAM_PLAYER_3] = "Player 3";
    team_names[MOB_TEAM_PLAYER_4] = "Player 4";
    team_names[MOB_TEAM_ENEMY_1] = "Enemy 1";
    team_names[MOB_TEAM_ENEMY_2] = "Enemy 2";
    team_names[MOB_TEAM_ENEMY_3] = "Enemy 3";
    team_names[MOB_TEAM_ENEMY_4] = "Enemy 4";
    team_names[MOB_TEAM_OBSTACLE] = "Obstacle";
    team_names[MOB_TEAM_OTHER] = "Other";
}


/* ----------------------------------------------------------------------------
 * Changes to a different game state.
 * new_state:
 *   State to change to.
 * unload_current:
 *   If true, the current state is unloaded from memory.
 * load_new:
 *   If true, the new state is loaded to memory. If you try to change to
 *   that state when it is not loaded, things will go wrong.
 */
void game_class::change_state(
    game_state* new_state, const bool unload_current, const bool load_new
) {

    if(cur_state && unload_current) {
        cur_state->unload();
    }
    
    cur_state = new_state;
    
    if(load_new) {
        cur_state->load();
    }
    
    //Because during the loading screens, there is no activity, on the
    //next frame, the game will assume the time between that and the last
    //non-loading frame is normal. This could be something like 2 seconds.
    //Let's reset the delta_t, then.
    reset_delta_t = true;
}


/* ----------------------------------------------------------------------------
 * Checks whether the player has pressed some system-related key combination,
 * and acts accordingly.
 * ev:
 *   The Allegro event behind the key press.
 */
void game_class::check_system_key_press(const ALLEGRO_EVENT &ev) {
    switch(ev.keyboard.keycode) {
    case ALLEGRO_KEY_F12: {
        if(has_flag(ev.keyboard.modifiers, ALLEGRO_KEYMOD_CTRL)) {
            string cur_state_name = get_cur_state_name();
            if(cur_state_name == states.animation_ed->get_name()) {
                maker_tools.auto_start_mode = "animation_editor";
                maker_tools.auto_start_option =
                    states.animation_ed->get_opened_file_name();
            } else if(cur_state_name == states.area_ed->get_name()) {
                maker_tools.auto_start_mode = "area_editor";
                maker_tools.auto_start_option =
                    states.area_ed->get_opened_folder_path();
            } else if(cur_state_name == states.gui_ed->get_name()) {
                maker_tools.auto_start_mode = "gui_editor";
                maker_tools.auto_start_option =
                    states.gui_ed->get_opened_file_name();
            } else if(cur_state_name == states.gameplay->get_name()) {
                maker_tools.auto_start_mode = "play";
                maker_tools.auto_start_option =
                    states.gameplay->path_of_area_to_load;
            } else {
                maker_tools.auto_start_mode.clear();
                maker_tools.auto_start_option.clear();
            }
            save_maker_tools();
        } else {
            save_screenshot();
        }
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Performs some global logic to run every frame.
 */
void game_class::do_global_logic() {
    //Cursor trail.
    if(options.draw_cursor_trail) {
        mouse_cursor.save_timer.tick(delta_t);
    }
    
    //Audio.
    audio.tick(delta_t);
}


/* ----------------------------------------------------------------------------
 * Returns the name of the current state.
 */
string game_class::get_cur_state_name() const {
    if(cur_state) {
        return cur_state->get_name();
    }
    return "none";
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event.
 * ev:
 *   Event to handle.
 */
void game_class::global_handle_allegro_event(const ALLEGRO_EVENT &ev) {
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor.update_pos(ev, screen_to_world_transform);
    }
}


/* ----------------------------------------------------------------------------
 * The main loop of the program. Processes events, ticks frames of gameplay,
 * etc.
 */
void game_class::main_loop() {
    //Used to calculate the time difference between the current and last frames.
    double prev_frame_time = 0.0;
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
        
        global_handle_allegro_event(ev);
        cur_state->handle_allegro_event(ev);
        controls.handle_allegro_event(ev);
        
        switch(ev.type) {
        case ALLEGRO_EVENT_TIMER: {
            if(al_is_event_queue_empty(logic_queue)) {
                double cur_time = al_get_time();
                if(reset_delta_t) {
                    //Failsafe.
                    prev_frame_time = cur_time - 1.0f / options.target_fps;
                    reset_delta_t = false;
                }
                
                float real_delta_t = cur_time - prev_frame_time;
                statistics.runtime += real_delta_t;
                
                //Anti speed-burst cap.
                delta_t = std::min(real_delta_t, 0.2f);
                
                time_passed += delta_t;
                game_state* prev_state = cur_state;
                
                do_global_logic();
                cur_state->do_logic();
                if(cur_state == prev_state) {
                    //Only draw if we didn't change states in the meantime.
                    cur_state->do_drawing();
                }
                
                prev_frame_time = cur_time;
            }
            break;
            
        } case ALLEGRO_EVENT_DISPLAY_CLOSE: {
            is_game_running = false;
            break;
            
        } case ALLEGRO_EVENT_KEY_DOWN: {
            check_system_key_press(ev);
            break;
            
        }  case ALLEGRO_EVENT_DISPLAY_SWITCH_IN: {
            //On Windows, when you tab out then back in, sometimes you'd see
            //weird artifacts. This workaround fixes it.
            al_resize_display(display, win_w, win_h);
            break;
            
        }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Shuts down the program, cleanly freeing everything.
 */
void game_class::shutdown() {
    if(perf_mon) {
        perf_mon->save_log();
    }
    
    if(cur_state) {
        cur_state->unload();
    }
    unload_misc_resources();
    destroy_mob_categories();
    states.destroy();
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
    states.init();
    
    //Controls and options.
    init_controls();
    load_options();
    save_options();
    load_statistics();
    statistics.startups++;
    save_statistics();
    
    //Event stuff.
    init_event_things(logic_timer, logic_queue);
    
    //Other fundamental initializations and loadings.
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
    
    //Init Dear ImGui.
    init_dear_imgui();
    
    //Init and load some engine things.
    init_mob_actions();
    init_mob_categories();
    init_misc_databases();
    load_game_config();
    load_maker_tools();
    save_maker_tools();
    
    dummy_mob_state = new mob_state("dummy");
    
    if(maker_tools.use_perf_mon) {
        perf_mon = new performance_monitor_struct();
    }
    
    if(
        maker_tools.enabled &&
        maker_tools.auto_start_mode == "play" &&
        !maker_tools.auto_start_option.empty()
    ) {
        states.gameplay->path_of_area_to_load =
            maker_tools.auto_start_option;
        change_state(states.gameplay);
        
    } else if(
        maker_tools.enabled &&
        maker_tools.auto_start_mode == "animation_editor"
    ) {
        states.animation_ed->auto_load_anim =
            maker_tools.auto_start_option;
        change_state(states.animation_ed);
        
    } else if(
        maker_tools.enabled &&
        maker_tools.auto_start_mode == "area_editor"
    ) {
        states.area_ed->auto_load_area =
            maker_tools.auto_start_option;
        change_state(states.area_ed);
        
    } else if(
        maker_tools.enabled &&
        maker_tools.auto_start_mode == "gui_editor"
    ) {
        states.gui_ed->auto_load_file =
            maker_tools.auto_start_option;
        change_state(states.gui_ed);
        
    } else {
        change_state(states.main_menu);
    }
    
    return 0;
}


/* ----------------------------------------------------------------------------
 * Unloads a loaded state that never got to be unloaded. This should only
 * be the case if change_state was called with instructions to not
 * unload the previous one.
 * loaded_state:
 *   Loaded state to unload.
 */
void game_class::unload_loaded_state(game_state* loaded_state) {
    loaded_state->unload();
}


/* ----------------------------------------------------------------------------
 * Creates a game state list struct.
 */
game_state_list::game_state_list() :
    animation_ed(nullptr),
    area_ed(nullptr),
    area_menu(nullptr),
    control_binds_menu(nullptr),
    gameplay(nullptr),
    gui_ed(nullptr),
    main_menu(nullptr),
    options_menu(nullptr),
    stats_menu(nullptr),
    results(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys the states in the list.
 */
void game_state_list::destroy() {
    delete animation_ed;
    delete area_ed;
    delete area_menu;
    delete control_binds_menu;
    delete gameplay;
    delete gui_ed;
    delete main_menu;
    delete options_menu;
    delete stats_menu;
    delete results;
    
    animation_ed = NULL;
    area_ed = NULL;
    area_menu = NULL;
    control_binds_menu = NULL;
    gameplay = NULL;
    gui_ed = NULL;
    main_menu = NULL;
    options_menu = NULL;
    results = NULL;
}


/* ----------------------------------------------------------------------------
 * Initializes the states in the list.
 */
void game_state_list::init() {
    animation_ed = new animation_editor();
    area_ed = new area_editor();
    area_menu = new area_menu_state();
    control_binds_menu = new control_binds_menu_state();
    gameplay = new gameplay_state();
    gui_ed = new gui_editor();
    main_menu = new main_menu_state();
    options_menu = new options_menu_state();
    stats_menu = new stats_menu_state();
    results = new results_state();
}


game_class game;
