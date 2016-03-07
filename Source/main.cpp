/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program start and main loop.
 */

#include <fstream>
#include <math.h>
#include <string>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "animation_editor.h"
#include "area_editor.h"
#include "const.h"
#include "controls.h"
#include "drawing.h"
#include "functions.h"
#include "gate.h"
#include "init.h"
#include "LAFI/button.h"
#include "LAFI/checkbox.h"
#include "LAFI/frame.h"
#include "LAFI/label.h"
#include "LAFI/radio_button.h"
#include "LAFI/scrollbar.h"
#include "LAFI/textbox.h"
#include "logic.h"
#include "menus.h"
#include "sector.h"
#include "vars.h"

using namespace std;

/*
 * Main function.
 * It begins by loading Allegro stuff,
 * the options, setting some settings,
 * and loading all of the game content.
 * Once that's done, it enters the main loop.
 */
int main(int argc, char** argv) {
    //Allegro initializations.
    init_allegro();
    
    //Controls and options.
    init_controls();
    load_options();
    save_options();
    
    //Event stuff.
    ALLEGRO_TIMER* logic_timer;
    ALLEGRO_EVENT_QUEUE* logic_queue;
    ALLEGRO_EVENT ev;
    init_event_things(logic_timer, logic_queue);
    
    //Other fundamental initializations.
    init_misc();
    init_game_states();
    init_error_bitmap();
    init_fonts();
    init_misc_graphics();
    init_misc_sounds();
    
    //The icon is used a lot, so load it here.
    bmp_icon = load_bmp("Icon.png");
    
    //Draw the basic loading screen.
    draw_loading_screen("", "", 1.0);
    al_flip_display();
    
    //Init some other things.
    init_mob_categories();
    init_special_mob_types();
    init_sector_types();
    init_dev_tools();
    init_hud_coordinates();
    read_game_config();
    load_hud_coordinates();
    
    unsigned int first_game_state = GAME_STATE_MAIN_MENU;
    if(argc >= 2) {
        string arg(argv[1]);
        if(arg == "play")
            first_game_state = GAME_STATE_GAME;
        else if(arg == "anim")
            first_game_state = GAME_STATE_ANIMATION_EDITOR;
        else if(arg == "area")
            first_game_state = GAME_STATE_AREA_EDITOR;
    }
    
    change_game_state(first_game_state);
    
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
        
        if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            is_game_running = false;
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
            //scr_w = ev.display.width;
            //scr_h = ev.display.height;
            
        } else if(ev.type == ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(logic_queue)) {
            double cur_time = al_get_time();
            if(reset_delta_t) {
                prev_frame_time = cur_time - 1.0f / game_fps; //Failsafe.
                reset_delta_t = false;
            }
            delta_t = cur_time - prev_frame_time;
            
            game_states[cur_game_state_nr]->do_logic();
            game_states[cur_game_state_nr]->do_drawing();
            
            prev_frame_time = cur_time;
            
        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_F12) {
            al_save_bitmap("Screenshot.png", al_get_backbuffer(display));
            
        }
    }
    
}
