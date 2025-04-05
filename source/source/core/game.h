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

#include "../game_state/game_state.h"

#include "../content/area/area.h"
#include "../content/content_manager.h"
#include "../content/other/mob_script_action.h"
#include "../game_state/animation_editor/editor.h"
#include "../game_state/annex_screen.h"
#include "../game_state/area_editor/editor.h"
#include "../game_state/gameplay/gameplay.h"
#include "../game_state/gui_editor/editor.h"
#include "../game_state/particle_editor/editor.h"
#include "../game_state/results.h"
#include "../game_state/title_screen.h"
#include "../lib/controls_manager/controls_manager.h"
#include "audio.h"
#include "game_config.h"
#include "misc_structs.h"
#include "options.h"
#include "shaders.h"


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
struct GameStateList {

    //--- Members ---
    
    //Animation editor.
    AnimationEditor* animation_ed = nullptr;
    
    //Area editor.
    AreaEditor* area_ed = nullptr;
    
    //Annex screen for misc. menus.
    AnnexScreen* annex_screen = nullptr;
    
    //Gameplay state.
    GameplayState* gameplay = nullptr;
    
    //GUI editor state.
    GuiEditor* gui_ed = nullptr;
    
    //GUI editor state.
    ParticleEditor* particle_ed = nullptr;
    
    //Title screen.
    TitleScreen* title_screen = nullptr;
    
    //Area results screen.
    Results* results = nullptr;
    
    
    //--- Function declarations ---
    
    void init();
    void destroy();
    
};


/**
 * @brief Info about the whole game.
 */
class Game {

public:

    //--- Members ---
    
    //Audio.
    AudioManager audio;
    
    //The error bitmap used to represent bitmaps that were not loaded.
    ALLEGRO_BITMAP* bmp_error = nullptr;
    
    //Player 1's camera.
    Camera cam;
    
    //Game's configuration.
    GameConfig config;
    
    //Player controls mediator.
    ControlsMediator controls;
    
    //Total amount of time the current frame took to process, in seconds.
    double cur_frame_process_time = 0.0f;
    
    //Info about the maker tools.
    MakerTools maker_tools;
    
    //Game content.
    ContentManager content;
    
    //Data about the area that's currently being used.
    Area* cur_area_data = nullptr;
    
    //Time between the previous frame and the current.
    double delta_t = 0.0f;
    
    //Allegro display that represents the program window.
    ALLEGRO_DISPLAY* display = nullptr;
    
    //A dummy mob state for mobs with no state to use.
    MobState* dummy_mob_state = nullptr;
    
    //Error manager.
    ErrorManager errors;
    
    //Shader manager.
    ShaderManager shaders;
    
    //Manager for all full-screen fade-ins and fade-outs.
    FadeManager fade_mgr;
    
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

    //Auto-repeater settings for leader auto-throws.
    AutoRepeaterSettings auto_throw_settings{
        LEADER::AUTO_THROW_SLOWEST_INTERVAL,
        LEADER::AUTO_THROW_FASTEST_INTERVAL,
        LEADER::AUTO_THROW_RAMP_TIME
    };
    
    //Buffer with the liquid limit effect.
    ALLEGRO_BITMAP* liquid_limit_effect_buffer = nullptr;
    
    //Info on every edge's liquid limit offset effect. Cache for performance.
    vector<EdgeOffsetCache> liquid_limit_effect_caches;
    
    //Loading screen subtext buffer.
    ALLEGRO_BITMAP* loading_subtext_bmp = nullptr;
    
    //Loading screen main text buffer.
    ALLEGRO_BITMAP* loading_text_bmp = nullptr;
    
    //List of mob actions.
    vector<MobAction> mob_actions;
    
    //List of mob categories.
    CategoryManager mob_categories;
    
    //Mouse cursor information.
    MouseCursor mouse_cursor;
    
    //Database of all mission fail conditions.
    vector<MissionFail*> mission_fail_conds;
    
    //Database of all mission goals.
    vector<MissionGoal*> mission_goals;
    
    //Database of all mission score criteria.
    vector<MissionScoreCriterion*> mission_score_criteria;
    
    //User options.
    Options options;
    
    //Performance monitor.
    PerformanceMonitor* perf_mon = nullptr;
    
    //Player actions in this frame.
    vector<PlayerAction> player_actions;
    
    //Randomness manager.
    RngManager rng;
    
    //Database of all sector types and their names.
    EnumNameDatabase sector_types;
    
    //Screen to world coordinate matrix. Cache for convenience.
    ALLEGRO_TRANSFORM screen_to_world_transform;
    
    //Should we be showing system info? (Framerate, version, etc.)
    bool show_system_info = false;
    
    //Skip rendering the scene with Dear ImGui for this frame.
    //It's a bit of a hack that fixes some corner cases.
    bool skip_dear_imgui_frame = false;
    
    //List of game states.
    GameStateList states;
    
    //List of lifetime statistics.
    Statistics statistics;
    
    //List of internal names of content that is needed by the system.
    SystemContentNames sys_content_names;
    
    //List of content that is needed system-wide.
    SystemContentList sys_content;
    
    //List of all mob team's internal names.
    string team_internal_names[N_MOB_TEAMS];
    
    //List of all mob team names, in proper English.
    string team_names[N_MOB_TEAMS];
    
    //How much time has passed since the program booted.
    float time_passed = 0.0f;
    
    //Buffer with the wall shadows and ledge smoothings.
    ALLEGRO_BITMAP* wall_offset_effect_buffer = nullptr;
    
    //Info on every edge's wall shadow offset effect. Cache for performance.
    vector<EdgeOffsetCache> wall_shadow_effect_caches;
    
    //Info on every edge's wall smoothing offset effect. Cache for performance.
    vector<EdgeOffsetCache> wall_smoothing_effect_caches;
    
    //Current fullscreen state.
    bool win_fullscreen = OPTIONS::GRAPHICS_D::WIN_FULLSCREEN;
    
    //Current window height.
    unsigned int win_h = OPTIONS::GRAPHICS_D::WIN_H;
    
    //Current window width.
    unsigned int win_w = OPTIONS::GRAPHICS_D::WIN_W;
    
    //World to screen coordinate matrix. Cache for convenience.
    ALLEGRO_TRANSFORM world_to_screen_transform;
    
    //Engine debugging tools.
    //Set them to true in the Game constructor as needed.
    struct {
    
        //--- Members ---
        
        //Show and operate on a Dear ImGui demo window.
        bool show_dear_imgui_demo = false;
        
    } debug;
    
    
    //--- Function declarations ---
    
    Game();
    void change_state(
        GameState* new_state,
        bool unload_current = true, bool load_new = true
    );
    string get_cur_state_name() const;
    void unload_loaded_state(GameState* loaded_state);
    void register_audio_stream_source(ALLEGRO_AUDIO_STREAM* stream);
    void unregister_audio_stream_source(ALLEGRO_AUDIO_STREAM* stream);
    int start();
    void main_loop();
    void shutdown();
    
private:

    //--- Members ---
    
    //Current game state: title screen, gameplay, etc.
    GameState* cur_state = nullptr;
    
    //Queue of Allegro events.
    ALLEGRO_EVENT_QUEUE* event_queue = nullptr;
    
    //Timer for the main frame logic.
    ALLEGRO_TIMER* main_timer = nullptr;
    
    //Is delta_t meant to be reset for the next frame?
    bool reset_delta_t = true;
    
    
    //--- Function declarations ---
    
    void global_drawing();
    void global_logic();
    void global_handle_allegro_event(const ALLEGRO_EVENT &ev);
    bool global_handle_system_player_action(const PlayerAction &action);
    
};


extern Game game;
