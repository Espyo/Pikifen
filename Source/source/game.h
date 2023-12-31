/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the class that holds everything about the game.
 */

#ifndef GAME_INCLUDED
#define GAME_INCLUDED

#include <allegro5/allegro.h>

#include "game_states/game_state.h"

#include "audio.h"
#include "area/area.h"
#include "libs/controls_manager.h"
#include "game_config.h"
#include "game_states/animation_editor/editor.h"
#include "game_states/area_editor/editor.h"
#include "game_states/gameplay/gameplay.h"
#include "game_states/gui_editor/editor.h"
#include "game_states/menus.h"
#include "game_states/results.h"
#include "liquid.h"
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


/* ----------------------------------------------------------------------------
 * List of all game states.
 */
struct game_state_list {
    //Animation editor.
    animation_editor* animation_ed;
    //Area editor.
    area_editor* area_ed;
    //Area selection menu.
    area_menu_state* area_menu;
    //Controls menu.
    control_binds_menu_state* control_binds_menu;
    //Gameplay state.
    gameplay_state* gameplay;
    //GUI editor state.
    gui_editor* gui_ed;
    //Main menu.
    main_menu_state* main_menu;
    //Options menu.
    options_menu_state* options_menu;
    //Statistics menu.
    stats_menu_state* stats_menu;
    //Area results menu.
    results_state* results;
    
    void init();
    void destroy();
    
    game_state_list();
};


/* ----------------------------------------------------------------------------
 * Information about the whole game.
 */
class game_class {
public:
    //List of asset file names.
    asset_file_names_struct asset_file_names;
    //Audio.
    audio_manager audio;
    //Manager of all main bitmaps (not floor textures).
    bmp_manager bitmaps;
    //The error bitmap used to represent bitmaps that were not loaded.
    ALLEGRO_BITMAP* bmp_error;
    //Player 1's camera.
    camera_info cam;
    //Game's configuration.
    game_config config;
    //Player controls mediator.
    controls_mediator controls;
    //Total amount of time the current frame took to process, in seconds.
    double cur_frame_process_time;
    //Info about the maker tools.
    maker_tools_info maker_tools;
    //Data about the area that's currently being used.
    area_data cur_area_data;
    //Particle generators declared by the user.
    map<string, particle_generator> custom_particle_generators;
    //Time between the previous frame and the current.
    double delta_t;
    //Allegro display that represents the program window.
    ALLEGRO_DISPLAY* display;
    //A dummy mob state for mobs with no state to use.
    mob_state* dummy_mob_state;
    //Error manager.
    error_manager errors;
    //Manager for all full-screen fade-ins and fade-outs.
    fade_manager fade_mgr;
    //List of fonts.
    font_list fonts;
    //Duration of the last few frames.
    vector<double> framerate_history;
    //Last framerate average started at this point in the history.
    size_t framerate_last_avg_point;
    //List of hazards.
    map<string, hazard> hazards;
    //Identity matrix transformation. Cache for convenience.
    ALLEGRO_TRANSFORM identity_transform;
    //Default Dear ImGui style.
    ImVec4 imgui_default_style[ImGuiCol_COUNT];
    //Set to false to stop program execution next frame.
    bool is_game_running;
    //What Allegro joystick maps to what number.
    map<ALLEGRO_JOYSTICK*, int> controller_numbers;
    //Buffer with the liquid limit effect.
    ALLEGRO_BITMAP* liquid_limit_effect_buffer;
    //Info on every edge's liquid limit offset effect. Cache for performance.
    vector<edge_offset_cache> liquid_limit_effect_caches;
    //List of liquids.
    map<string, liquid*> liquids;
    //Loading screen subtext buffer.
    ALLEGRO_BITMAP* loading_subtext_bmp;
    //Loading screen main text buffer.
    ALLEGRO_BITMAP* loading_text_bmp;
    //List of mob actions.
    vector<mob_action> mob_actions;
    //List of mob categories.
    mob_category_manager mob_categories;
    //All mob types.
    mob_type_lists mob_types;
    //Mouse cursor information.
    mouse_cursor_struct mouse_cursor;
    //Database of all mission fail conditions.
    vector<mission_fail*> mission_fail_conds;
    //Database of all mission goals.
    vector<mission_goal*> mission_goals;
    //Database of all mission score criteria.
    vector<mission_score_criterion*> mission_score_criteria;
    //User options.
    options_struct options;
    //Performance monitor.
    performance_monitor_struct* perf_mon;
    //Database of all sector types and their names.
    enum_name_database sector_types;
    //Screen to world coordinate matrix. Cache for convenience.
    ALLEGRO_TRANSFORM screen_to_world_transform;
    //Should we be showing system info? (Framerate, version, etc.)
    bool show_system_info;
    //List of spike damage types.
    map<string, spike_damage_type> spike_damage_types;
    //List of spray types.
    vector<spray_type> spray_types;
    //List of game states.
    game_state_list states;
    //List of lifetime statistics.
    statistics_struct statistics;
    //List of status types.
    map<string, status_type*> status_types;
    //All system assets.
    system_asset_list sys_assets;
    //List of all mob team's internal names.
    string team_internal_names[N_MOB_TEAMS];
    //List of all mob team names, in proper English.
    string team_names[N_MOB_TEAMS];
    //Manager of all floor texture bitmaps.
    bmp_manager textures;
    //How much time has passed since the program booted.
    float time_passed;
    //Buffer with the wall shadows and ledge smoothings.
    ALLEGRO_BITMAP* wall_offset_effect_buffer;
    //Info on every edge's wall shadow offset effect. Cache for performance.
    vector<edge_offset_cache> wall_shadow_effect_caches;
    //Info on every edge's wall smoothing offset effect. Cache for performance.
    vector<edge_offset_cache> wall_smoothing_effect_caches;
    //List of weather conditions.
    map<string, weather> weather_conditions;
    //Current fullscreen state.
    bool win_fullscreen;
    //Current window height.
    unsigned int win_h;
    //Current window width.
    unsigned int win_w;
    //World to screen coordinate matrix. Cache for convenience.
    ALLEGRO_TRANSFORM world_to_screen_transform;
    
    //Change to a different state.
    void change_state(
        game_state* new_state,
        const bool unload_current = true, const bool load_new = true
    );
    //Get the name of the current state.
    string get_cur_state_name() const;
    //Unloads a state that never got unloaded.
    void unload_loaded_state(game_state* loaded_state);
    
    //Program execution.
    int start();
    void main_loop();
    void shutdown();
    
    game_class();
    
private:
    //Current game state: main menu, gameplay, etc.
    game_state* cur_state;
    //Queue of events.
    ALLEGRO_EVENT_QUEUE* logic_queue;
    //Timer for the main frame logic.
    ALLEGRO_TIMER* logic_timer;
    //Is delta_t meant to be reset for the next frame?
    bool reset_delta_t;
    
    void check_system_key_press(const ALLEGRO_EVENT &ev);
    void do_global_logic();
    void global_handle_allegro_event(const ALLEGRO_EVENT &ev);
    
};


extern game_class game;

#endif //ifndef GAME_INCLUDED
