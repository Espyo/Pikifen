/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the class that holds everything about the game.
 */

#pragma once

#include <allegro5/allegro.h>

#include "game_states/game_state.h"

#include "audio.h"
#include "area/area.h"
#include "content_manager.h"
#include "libs/controls_manager.h"
#include "game_config.h"
#include "game_states/animation_editor/editor.h"
#include "game_states/area_editor/editor.h"
#include "game_states/gameplay/gameplay.h"
#include "game_states/gui_editor/editor.h"
#include "game_states/menus.h"
#include "game_states/results.h"
#include "mob_script_action.h"
#include "misc_structs.h"
#include "options.h"


namespace GAME {
extern const ALLEGRO_COLOR CURSOR_STANDARD_COLOR;
extern const unsigned char CURSOR_TRAIL_MAX_ALPHA;
extern const float CURSOR_TRAIL_MAX_WIDTH;
extern const float CURSOR_TRAIL_MIN_SPOT_DIFF;
extern const float CURSOR_TRAIL_SAVE_INTERVAL;
extern const unsigned char CURSOR_TRAIL_SAVE_N_SPOTS;
extern const float FADE_DURATION;
extern const size_t FRAMERATE_AVG_SAMPLE_SIZE;
extern const size_t FRAMERATE_HISTORY_SIZE;
}


/**
 * @brief List of all game states.
 */
struct game_state_list {

    //--- Members ---
    
    //Animation editor.
    animation_editor* animation_ed = nullptr;
    
    //Area editor.
    area_editor* area_ed = nullptr;
    
    //Area selection menu.
    area_menu_state* area_menu = nullptr;
    
    //Controls menu.
    control_binds_menu_state* control_binds_menu = nullptr;
    
    //Gameplay state.
    gameplay_state* gameplay = nullptr;
    
    //GUI editor state.
    gui_editor* gui_ed = nullptr;
    
    //Main menu.
    main_menu_state* main_menu = nullptr;
    
    //Options menu.
    options_menu_state* options_menu = nullptr;
    
    //Statistics menu.
    stats_menu_state* stats_menu = nullptr;
    
    //Area results menu.
    results_state* results = nullptr;
    
    
    //--- Function declarations ---
    
    void init();
    void destroy();
    
};


/**
 * @brief Info about the whole game.
 */
class game_class {

public:

    //--- Members ---
    
    //List of asset file names.
    asset_file_names_t asset_file_names;
    
    //Audio.
    audio_manager audio;
    
    //The error bitmap used to represent bitmaps that were not loaded.
    ALLEGRO_BITMAP* bmp_error = nullptr;
    
    //Player 1's camera.
    camera_t cam;
    
    //Game's configuration.
    game_config config;
    
    //Player controls mediator.
    controls_mediator controls;
    
    //Total amount of time the current frame took to process, in seconds.
    double cur_frame_process_time = 0.0f;
    
    //Info about the maker tools.
    maker_tools_t maker_tools;
    
    //Game content.
    content_manager content;
    
    //Data about the area that's currently being used.
    area_data* cur_area_data = nullptr;
    
    //Time between the previous frame and the current.
    double delta_t = 0.0f;
    
    //Allegro display that represents the program window.
    ALLEGRO_DISPLAY* display = nullptr;
    
    //A dummy mob state for mobs with no state to use.
    mob_state* dummy_mob_state = nullptr;
    
    //Error manager.
    error_manager errors;
    
    //Manager for all full-screen fade-ins and fade-outs.
    fade_manager fade_mgr;
    
    //Duration of the last few frames.
    vector<double> framerate_history;
    
    //Last framerate average started at this point in the history.
    size_t framerate_last_avg_point = 0.0f;
    
    //Identity matrix transformation. Cache for convenience.
    ALLEGRO_TRANSFORM identity_transform;
    
    //Default Dear ImGui style.
    ImVec4 imgui_default_style[ImGuiCol_COUNT];
    
    //Set to false to stop program execution next frame.
    bool is_game_running = true;
    
    //What Allegro joystick maps to what number.
    map<ALLEGRO_JOYSTICK*, int> controller_numbers;
    
    //Buffer with the liquid limit effect.
    ALLEGRO_BITMAP* liquid_limit_effect_buffer = nullptr;
    
    //Info on every edge's liquid limit offset effect. Cache for performance.
    vector<edge_offset_cache> liquid_limit_effect_caches;
    
    //Loading screen subtext buffer.
    ALLEGRO_BITMAP* loading_subtext_bmp = nullptr;
    
    //Loading screen main text buffer.
    ALLEGRO_BITMAP* loading_text_bmp = nullptr;
    
    //List of mob actions.
    vector<mob_action> mob_actions;
    
    //List of mob categories.
    mob_category_manager mob_categories;
    
    //Mouse cursor information.
    mouse_cursor_t mouse_cursor;
    
    //Database of all mission fail conditions.
    vector<mission_fail*> mission_fail_conds;
    
    //Database of all mission goals.
    vector<mission_goal*> mission_goals;
    
    //Database of all mission score criteria.
    vector<mission_score_criterion*> mission_score_criteria;
    
    //User options.
    options_t options;
    
    //Performance monitor.
    performance_monitor_t* perf_mon = nullptr;
    
    //Database of all sector types and their names.
    enum_name_database sector_types;
    
    //Screen to world coordinate matrix. Cache for convenience.
    ALLEGRO_TRANSFORM screen_to_world_transform;
    
    //Should we be showing system info? (Framerate, version, etc.)
    bool show_system_info = false;
    
    //Skip rendering the scene with Dear ImGui for this frame.
    //It's a bit of a hack that fixes some corner cases.
    bool skip_dear_imgui_frame = false;
    
    //List of game states.
    game_state_list states;
    
    //List of lifetime statistics.
    statistics_t statistics;
    
    //All system assets.
    system_asset_list sys_assets;
    
    //List of all mob team's internal names.
    string team_internal_names[N_MOB_TEAMS];
    
    //List of all mob team names, in proper English.
    string team_names[N_MOB_TEAMS];
    
    //How much time has passed since the program booted.
    float time_passed = 0.0f;
    
    //Buffer with the wall shadows and ledge smoothings.
    ALLEGRO_BITMAP* wall_offset_effect_buffer = nullptr;
    
    //Info on every edge's wall shadow offset effect. Cache for performance.
    vector<edge_offset_cache> wall_shadow_effect_caches;
    
    //Info on every edge's wall smoothing offset effect. Cache for performance.
    vector<edge_offset_cache> wall_smoothing_effect_caches;
    
    //Current fullscreen state.
    bool win_fullscreen = OPTIONS::DEF_WIN_FULLSCREEN;
    
    //Current window height.
    unsigned int win_h = OPTIONS::DEF_WIN_H;
    
    //Current window width.
    unsigned int win_w = OPTIONS::DEF_WIN_W;
    
    //World to screen coordinate matrix. Cache for convenience.
    ALLEGRO_TRANSFORM world_to_screen_transform;
    
    
    //--- Function declarations ---
    
    game_class();
    void change_state(
        game_state* new_state,
        bool unload_current = true, bool load_new = true
    );
    string get_cur_state_name() const;
    void unload_loaded_state(game_state* loaded_state);
    void register_audio_stream_source(ALLEGRO_AUDIO_STREAM* stream);
    void unregister_audio_stream_source(ALLEGRO_AUDIO_STREAM* stream);
    int start();
    void main_loop();
    void shutdown();
    
private:

    //--- Members ---
    
    //Current game state: main menu, gameplay, etc.
    game_state* cur_state = nullptr;
    
    //Queue of Allegro events.
    ALLEGRO_EVENT_QUEUE* event_queue = nullptr;
    
    //Timer for the main frame logic.
    ALLEGRO_TIMER* main_timer = nullptr;
    
    //Is delta_t meant to be reset for the next frame?
    bool reset_delta_t = true;
    
    
    //--- Function declarations ---
    
    void check_system_key_press(const ALLEGRO_EVENT &ev);
    void do_global_drawing();
    void do_global_logic();
    void global_handle_allegro_event(const ALLEGRO_EVENT &ev);
    
};


extern game_class game;
