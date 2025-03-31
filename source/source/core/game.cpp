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

#include "../lib/imgui/imgui_impl_allegro5.h"
#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "drawing.h"
#include "init.h"
#include "load.h"
#include "misc_functions.h"


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


/**
 * @brief Constructs a new game class object.
 */
Game::Game() {
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


/**
 * @brief Changes to a different game state.
 *
 * @param new_state State to change to.
 * @param unload_current If true, the current state is unloaded from memory.
 * @param load_new If true, the new state is loaded to memory.
 * If you try to change to that state when it is not loaded,
 * things will go wrong.
 */
void Game::change_state(
    GameState* new_state, bool unload_current, bool load_new
) {
    if(cur_state && unload_current) {
        cur_state->unload();
        cur_state->loaded = false;
    }
    
    cur_state = new_state;
    
    if(load_new) {
        cur_state->load();
        cur_state->loaded = true;
    }
    
    //Because during the loading screens there is no activity, on the
    //next frame, the game will assume the time between that and the last
    //non-loading frame is normal. This could be something like 2 seconds.
    //Let's reset the delta_t, then.
    reset_delta_t = true;
}


/**
 * @brief Returns the name of the current state.
 *
 * @return The name.
 */
string Game::get_cur_state_name() const {
    if(!cur_state) return "none";
    return cur_state->get_name();
}


/**
 * @brief Performs some global drawings to run every frame.
 */
void Game::global_drawing() {
    //Dear ImGui.
    if(debug.show_dear_imgui_demo) {
        ImGui::ShowDemoWindow();
    }
    ImGui::Render();
    if(!skip_dear_imgui_frame) {
        ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
    } else {
        skip_dear_imgui_frame = false;
    }
    
    //Fade manager.
    if(!debug.show_dear_imgui_demo) {
        game.fade_mgr.draw();
    }
}


/**
 * @brief Performs some global logic to run every frame.
 */
void Game::global_logic() {
    //Player action handling.
    for(size_t a = 0; a < player_actions.size();) {
        if(maker_tools.handle_global_player_action(player_actions[a])) {
            player_actions.erase(player_actions.begin() + a);
        } else if(global_handle_system_player_action(player_actions[a])) {
            player_actions.erase(player_actions.begin() + a);
        } else {
            a++;
        }
    }
    
    //Cursor trail.
    if(options.advanced.draw_cursor_trail) {
        mouse_cursor.save_timer.tick(delta_t);
    }
    
    //Audio.
    audio.tick(delta_t);
    
    //Dear ImGui.
    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 */
void Game::global_handle_allegro_event(const ALLEGRO_EVENT &ev) {
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        //Mouse cursor.
        mouse_cursor.update_pos(ev, screen_to_world_transform);
        
    } else if(ev.type == ALLEGRO_EVENT_AUDIO_STREAM_FINISHED) {
        //Audio stream finished.
        game.audio.handle_stream_finished(
            (ALLEGRO_AUDIO_STREAM*) (ev.any.source)
        );
        
    } else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        //Hitting the X on the game window.
        is_game_running = false;
        
    } else if(ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
        //On Windows, when you tab out then back in, sometimes you'd see
        //weird artifacts. This workaround fixes it.
        al_resize_display(display, win_w, win_h);
        
    }
    
    //Dear ImGui.
    ImGui_ImplAllegro5_ProcessEvent((ALLEGRO_EVENT*) &ev);
}


/**
 * @brief Handles a system player action, if possible.
 *
 * @param action The action.
 * @return Whether it got handled.
 */
bool Game::global_handle_system_player_action(const PlayerAction &action) {
    bool is_system_action =
        controls.get_player_action_type(action.actionTypeId).category ==
        PLAYER_ACTION_CAT_SYSTEM;
    if(!is_system_action) return false;
    if(action.value < 0.5f) return false;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_SYSTEM_INFO: {
        show_system_info = !show_system_info;
        break;
        
    } case PLAYER_ACTION_TYPE_SCREENSHOT: {
        save_screenshot();
        break;
    }
    }
    
    return true;
}


/**
 * @brief The main loop of the program. Processes events,
 * ticks frames of gameplay, etc.
 */
void Game::main_loop() {
    //Used to calculate the time difference between the current and last frames.
    double prev_frame_start_time = 0.0;
    ALLEGRO_EVENT ev;
    
    //Main loop.
    al_start_timer(main_timer);
    while(is_game_running) {
    
        /*  ************************************************
          *** | _ |                                  | _ | ***
        *****  \_/           EVENT HANDLING           \_/  *****
          *** +---+                                  +---+ ***
            ************************************************/
        
        al_wait_for_event(event_queue, &ev);
        
        global_handle_allegro_event(ev);
        cur_state->handle_allegro_event(ev);
        controls.handle_allegro_event(ev);
        
        switch(ev.type) {
        case ALLEGRO_EVENT_TIMER: {
            if(al_is_event_queue_empty(event_queue)) {
            
                double cur_frame_start_time = al_get_time();
                if(reset_delta_t) {
                    //Failsafe.
                    prev_frame_start_time =
                        cur_frame_start_time -
                        1.0f / options.advanced.target_fps;
                    reset_delta_t = false;
                }
                
                float real_delta_t =
                    cur_frame_start_time - prev_frame_start_time;
                statistics.runtime += real_delta_t;
                
                //Anti speed-burst cap.
                delta_t = std::min(real_delta_t, 0.2f);
                
                time_passed += delta_t;
                GameState* prev_state = cur_state;
                
                player_actions = controls.new_frame(delta_t);
                global_logic();
                cur_state->do_logic();
                
                if(cur_state == prev_state) {
                    //Only draw if we didn't change states in the meantime.
                    cur_state->do_drawing();
                    global_drawing();
                    al_flip_display();
                } else {
                    ImGui::EndFrame();
                }
                
                double cur_frame_end_time = al_get_time();
                cur_frame_process_time =
                    cur_frame_end_time - cur_frame_start_time;
                    
                prev_frame_start_time = cur_frame_start_time;
                
            }
            break;
            
        }
        }
    }
}


/**
 * @brief Shuts down the program, cleanly freeing everything.
 */
void Game::shutdown() {
    if(perf_mon) {
        perf_mon->save_log();
    }
    
    if(cur_state) {
        cur_state->unload();
    }
    
    content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MISC,
        CONTENT_TYPE_BITMAP,
        CONTENT_TYPE_SOUND,
        CONTENT_TYPE_SONG_TRACK,
        CONTENT_TYPE_SONG,
    }
    );
    
    delete dummy_mob_state;
    
    unload_misc_resources();
    destroy_mob_categories();
    states.destroy();
    destroy_misc();
    destroy_event_things(main_timer, event_queue);
    destroy_allegro();
}


/**
 * @brief Registers an Allegro audio stream's event source into the event
 * queue.
 *
 * @param stream The audio stream.
 */
void Game::register_audio_stream_source(ALLEGRO_AUDIO_STREAM* stream) {
    al_register_event_source(
        event_queue,
        al_get_audio_stream_event_source(stream)
    );
}


/**
 * @brief Starts up the program, setting up everything that's necessary.
 *
 * @return 0 if everything is okay, otherwise a return code to quit the
 * program with.
 */
int Game::start() {
    //Allegro initializations.
    init_allegro();
    
    //Panic check: is there a game_data folder?
    if(folder_to_vector(FOLDER_PATHS_FROM_ROOT::GAME_DATA, true).empty()) {
        show_system_message_box(
            nullptr, "game_data folder not found!",
            "game_data folder not found!",
            "Could not find the \"game_data\" folder! "
            "If you are running the engine from a zip file, "
            "you have to unpack it first.",
            nullptr,
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
    init_event_things(main_timer, event_queue);
    
    //Other fundamental initializations and loadings.
    init_misc();
    init_error_bitmap();
    content.reload_packs();
    content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MISC,
        CONTENT_TYPE_BITMAP,
        CONTENT_TYPE_SOUND,
        CONTENT_TYPE_SONG_TRACK,
        CONTENT_TYPE_SONG,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    load_fonts();
    load_misc_graphics();
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
    load_maker_tools();
    save_maker_tools();
    
    dummy_mob_state = new MobState("dummy");
    
    if(maker_tools.use_perf_mon) {
        perf_mon = new PerformanceMonitor();
    }
    
    //Auto-start in some state.
    if(
        maker_tools.enabled &&
        maker_tools.auto_start_state == "play" &&
        !maker_tools.auto_start_option.empty()
    ) {
        states.gameplay->path_of_area_to_load =
            maker_tools.auto_start_option;
        change_state(states.gameplay);
        
    } else if(
        maker_tools.enabled &&
        maker_tools.auto_start_state == "animation_editor"
    ) {
        states.animation_ed->auto_load_file =
            maker_tools.auto_start_option;
        change_state(states.animation_ed);
        
    } else if(
        maker_tools.enabled &&
        maker_tools.auto_start_state == "area_editor"
    ) {
        states.area_ed->auto_load_folder =
            maker_tools.auto_start_option;
        change_state(states.area_ed);
        
    } else if(
        maker_tools.enabled &&
        maker_tools.auto_start_state == "gui_editor"
    ) {
        states.gui_ed->auto_load_file =
            maker_tools.auto_start_option;
        change_state(states.gui_ed);
    } else if(
        maker_tools.enabled &&
        maker_tools.auto_start_state == "particle_editor"
    ) {
        states.particle_ed->auto_load_file =
            maker_tools.auto_start_option;
        change_state(states.particle_ed);
    } else {
        change_state(states.title_screen);
    }
    
    return 0;
}


/**
 * @brief Unloads a loaded state that never got to be unloaded. This should only
 * be the case if change_state was called with instructions to not
 * unload the previous one.
 *
 * @param loaded_state Loaded state to unload.
 */
void Game::unload_loaded_state(GameState* loaded_state) {
    loaded_state->unload();
}


/**
 * @brief Unregisters an Allegro audio stream's event source from the event
 * queue.
 *
 * @param stream The audio stream.
 */
void Game::unregister_audio_stream_source(ALLEGRO_AUDIO_STREAM* stream) {
    al_unregister_event_source(
        event_queue,
        al_get_audio_stream_event_source(stream)
    );
}


/**
 * @brief Destroys the states in the list.
 */
void GameStateList::destroy() {
    delete animation_ed;
    delete area_ed;
    delete annex_screen;
    delete gameplay;
    delete gui_ed;
    delete particle_ed;
    delete title_screen;
    delete results;
    
    animation_ed = nullptr;
    area_ed = nullptr;
    gameplay = nullptr;
    gui_ed = nullptr;
    particle_ed = nullptr;
    title_screen = nullptr;
    results = nullptr;
}


/**
 * @brief Initializes the states in the list.
 */
void GameStateList::init() {
    animation_ed = new AnimationEditor();
    area_ed = new AreaEditor();
    annex_screen = new AnnexScreen();
    gameplay = new GameplayState();
    gui_ed = new GuiEditor();
    particle_ed = new ParticleEditor();
    title_screen = new TitleScreen();
    results = new Results();
}


Game game;
