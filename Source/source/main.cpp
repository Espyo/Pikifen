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

#include <algorithm>
#include <fstream>
#include <math.h>
#include <string>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>

#include "animation_editor.h"
#include "area_editor.h"
#include "const.h"
#include "controls.h"
#include "drawing.h"
#include "functions.h"
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

/* ----------------------------------------------------------------------------
 * Main function.
 * It begins by loading Allegro stuff,
 * the options, setting some settings,
 * and loading all of the game content.
 * Once that's done, it enters the main loop.
 */
int main(int argc, char** argv) {
    //Panic check: is there a Game_data folder?
    if(folder_to_vector(GAME_DATA_FOLDER, true).empty()) {
        al_show_native_message_box(
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
    load_game_config();
    load_hud_coordinates();

    if(
        dev_tool_auto_start_mode == "play" &&
        !dev_tool_auto_start_option.empty()
    ) {
        area_to_load = dev_tool_auto_start_option;
        change_game_state(GAME_STATE_GAME);
    } else if(dev_tool_auto_start_mode == "animation_editor") {
        (
            (animation_editor*)
            game_states[GAME_STATE_ANIMATION_EDITOR]
        )->auto_load_anim = dev_tool_auto_start_option;
        change_game_state(GAME_STATE_ANIMATION_EDITOR);
    } else if(dev_tool_auto_start_mode == "area_editor") {
        (
            (area_editor*)
            game_states[GAME_STATE_AREA_EDITOR]
        )->auto_load_area = dev_tool_auto_start_option;
        change_game_state(GAME_STATE_AREA_EDITOR);
    } else {
        change_game_state(GAME_STATE_MAIN_MENU);
    }

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

        } else if(
            ev.type == ALLEGRO_EVENT_TIMER &&
            al_is_event_queue_empty(logic_queue)
        ) {
            double cur_time = al_get_time();
            if(reset_delta_t) {
                //Failsafe.
                prev_frame_time = cur_time - 1.0f / game_fps;
                reset_delta_t = false;
            }

            //Anti speed-burst cap.
            delta_t = min(cur_time - prev_frame_time, 0.2);

            game_states[cur_game_state_nr]->do_logic();
            game_states[cur_game_state_nr]->do_drawing();

            prev_frame_time = cur_time;

        } else if (
            ev.type == ALLEGRO_EVENT_KEY_DOWN &&
            ev.keyboard.keycode == ALLEGRO_KEY_F12
        ) {
            al_save_bitmap("Screenshot.png", al_get_backbuffer(display));

        }
    }

    if(cur_game_state_nr != INVALID) {
        game_states[cur_game_state_nr]->unload();
    }
    destroy_special_mob_types();
    destroy_game_states();
    destroy_resources();
    destroy_event_things(logic_timer, logic_queue);
    destroy_allegro();

}
