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

#include "game_state.h"

#include "editors/animation_editor/editor.h"
#include "editors/area_editor/editor.h"
#include "gameplay.h"
#include "menus.h"
#include "options.h"


//When getting a framerate average, use a sample of this size.
const size_t FRAMERATE_AVG_SAMPLE_SIZE = 30;
//Only save the latest N FPS samples.
const size_t FRAMERATE_HISTORY_SIZE = 300;

class game_class {
public:

    //Animation editor game state.
    animation_editor* animation_editor_state;
    //Area editor game state.
    area_editor* area_editor_state;
    //Area menu game state.
    area_menu* area_menu_state;
    //Controls menu game state.
    controls_menu* controls_menu_state;
    //Data about the area that's currently being used.
    area_data cur_area_data;
    //Time between the previous frame and the current.
    double delta_t;
    //Allegro display that represents the program window.
    ALLEGRO_DISPLAY* display;
    //How many errors have been reported this application session.
    size_t errors_reported_so_far = 0;
    //Manager for all full-screen fade-ins and fade-outs.
    fade_manager fade_mgr;
    //Duration of the last few frames.
    vector<float> framerate_history;
    //Last framerate average started at this point in the history.
    size_t framerate_last_avg_point;
    //Gameplay game state.
    gameplay* gameplay_state;
    //Identity matrix transformation. Cache for convenience.
    ALLEGRO_TRANSFORM identity_transform;
    //Set to false to stop program execution next frame.
    bool is_game_running;
    //What Allegro joystick maps to what number.
    map<ALLEGRO_JOYSTICK*, int> joystick_numbers;
    //Standard leader order.
    vector<leader_type*> leader_order;
    //Loading screen subtext buffer.
    ALLEGRO_BITMAP* loading_subtext_bmp;
    //Loading screen main text buffer.
    ALLEGRO_BITMAP* loading_text_bmp;
    //Main menu game state.
    main_menu* main_menu_state;
    //OS mouse cursor position, in screen coordinates.
    point mouse_cursor_s;
    //OS mouse cursor position, in world coordinates.
    point mouse_cursor_w;
    //Global audio mixer.
    ALLEGRO_MIXER* mixer;
    //Name of the fan-game.
    string name;
    //User options.
    options_struct options;
    //Options game state.
    options_menu* options_menu_state;
    //Standard Pikmin order.
    vector<pikmin_type*> pikmin_order;
    //Screen to world coordinate matrix. Cache for convenience.
    ALLEGRO_TRANSFORM screen_to_world_transform;
    //Should we be showing system info? (Framerate, version, etc.)
    bool show_system_info;
    //Version of the fan-game.
    string version;
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
    void change_state(game_state* new_state);
    //Get the name of the current state.
    string get_cur_state_name();
    
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
