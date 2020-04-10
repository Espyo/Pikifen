/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the class that holds everything about the game.
 */

#ifndef GAME_H
#define GAME_H

#include <allegro5/allegro.h>

#include "game_state.h"

#include "editors/animation_editor/editor.h"
#include "editors/area_editor/editor.h"
#include "gameplay.h"
#include "menus.h"


//Default target FPS.
const unsigned int DEF_TARGET_FPS = 60;

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
    //Time between the previous frame and the current.
    double delta_t;
    //Gameplay game state.
    gameplay* gameplay_state;
    //Set to false to stop program execution next frame.
    bool is_game_running;
    //Main menu game state.
    main_menu* main_menu_state;
    //Options game state.
    options_menu* options_menu_state;
    //Target framerate.
    int target_fps;
    
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

#endif //ifndef GAME_H
