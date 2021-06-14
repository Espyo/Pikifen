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

#include "game_config.h"
#include "game_states/animation_editor/editor.h"
#include "game_states/area_editor/editor.h"
#include "game_states/gameplay/gameplay.h"
#include "game_states/menus.h"
#include "game_states/results.h"
#include "liquid.h"
#include "mob_script_action.h"
#include "misc_structs.h"
#include "options.h"


//When getting a framerate average, use a sample of this size.
const size_t FRAMERATE_AVG_SAMPLE_SIZE = 30;
//Only save the latest N FPS samples.
const size_t FRAMERATE_HISTORY_SIZE = 300;


/* ----------------------------------------------------------------------------
 * List of all game states.
 */
struct game_state_list {
    animation_editor* animation_ed;
    area_editor* area_ed;
    area_menu_state* area_menu;
    controls_menu_state* controls_menu;
    gameplay_state* gameplay;
    main_menu_state* main_menu;
    options_menu_state* options_menu;
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
    //Manager of all main bitmaps (not floor textures).
    bmp_manager bitmaps;
    //The error bitmap used to represent bitmaps that were not loaded.
    ALLEGRO_BITMAP* bmp_error;
    //List of all virtual "buttons" that a player can press.
    button_manager buttons;
    //Player 1's camera.
    camera_info cam;
    //Game's configuration.
    game_config config;
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
    //How many errors have been reported this application session.
    size_t errors_reported_so_far = 0;
    //Manager for all full-screen fade-ins and fade-outs.
    fade_manager fade_mgr;
    //List of fonts.
    font_list fonts;
    //Duration of the last few frames.
    vector<float> framerate_history;
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
    map<ALLEGRO_JOYSTICK*, int> joystick_numbers;
    //Buffer with the liquid limit effect.
    ALLEGRO_BITMAP* liquid_limit_effect_buffer;
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
    //OS mouse cursor position, in screen coordinates.
    point mouse_cursor_s;
    //OS mouse cursor position, in world coordinates.
    point mouse_cursor_w;
    //Global audio mixer.
    ALLEGRO_MIXER* mixer;
    //User options.
    options_struct options;
    //Performance monitor.
    performance_monitor_struct* perf_mon;
    //Manager of all sector types.
    sector_types_manager sector_types;
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
    //Allegro voice from which the sound effects play.
    ALLEGRO_VOICE* voice;
    
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
    
};


extern game_class game;

#endif //ifndef GAME_INCLUDED
