/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program start and main loop.
 */

//TODO check for ".c_str()" in the code, as apparently I have some atois and atof instead of toi and tof.

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
#include "init.h"
#include "LAFI/button.h"
#include "LAFI/checkbox.h"
#include "LAFI/frame.h"
#include "LAFI/label.h"
#include "LAFI/radio_button.h"
#include "LAFI/scrollbar.h"
#include "LAFI/textbox.h"
#include "logic.h"
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
int main(int argc, char**) {
    init_allegro();
    init_controls();
    
    load_options();
    save_options();
    
    //Event stuff.
    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_EVENT ev;
    init_event_things(timer, queue);
    init_misc();
    init_mob_categories();
    init_special_mob_types();
    init_sector_types();
    init_error_bitmap();
    init_fonts();
    
    draw_loading_screen("", "Loading...", 1.0);
    
    cur_screen = SCREEN_GAME;
    if(argc == 2) cur_screen = SCREEN_ANIMATION_EDITOR;
    else if(argc == 3) cur_screen = SCREEN_AREA_EDITOR;
    
    if(cur_screen == SCREEN_GAME) {
    
        //Graphics.
        bmp_red_onion = load_bmp("Red_onion.png");
        bmp_yellow_onion = load_bmp("Yellow_onion.png");
        bmp_blue_onion = load_bmp("Blue_onion.png");
        bmp_ship = load_bmp("Ship.png");
        
        bmp_bubble = load_bmp(          "Bubble.png");
        bmp_cursor = load_bmp(          "Cursor.png");
        bmp_day_bubble = load_bmp(      "Day_bubble.png");
        bmp_enemy_spirit = load_bmp(    "Enemy_spirit.png");
        bmp_hard_bubble = load_bmp(     "Hard_bubble.png");
        bmp_icon = load_bmp(            "Icon.png");
        bmp_idle_glow = load_bmp(       "Idle_glow.png");
        bmp_info_spot = load_bmp(       "Info_spot.png");
        bmp_message_box = load_bmp(     "Message_box.png");
        bmp_mouse_cursor = load_bmp(    "Mouse_cursor.png");
        bmp_group_move_arrow = load_bmp("Group_move_arrow.png");
        bmp_nectar = load_bmp(          "Nectar.png");
        bmp_no_pikmin = load_bmp(       "No_Pikmin.png");
        bmp_number_bubble = load_bmp(   "Number_bubble.png");
        bmp_pikmin_spirit = load_bmp(   "Pikmin_spirit.png");
        bmp_shadow = load_bmp(          "Shadow.png");
        bmp_smack = load_bmp(           "Smack.png");
        bmp_smoke = load_bmp(           "Smoke.png");
        bmp_sparkle = load_bmp(         "Sparkle.png");
        bmp_sun = load_bmp(             "Sun.png");
        bmp_sun_bubble = load_bmp(      "Sun_bubble.png");
        bmp_tp = load_bmp(              "TP.png");
        bmp_ub_spray = load_bmp(        "Ultra-bitter_spray.png");
        bmp_us_spray = load_bmp(        "Ultra-spicy_spray.png");
        
        bmp_test = load_bmp("Test.png");
        
        al_set_display_icon(display, bmp_icon);
        
        //Sound effects.
        voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16,   ALLEGRO_CHANNEL_CONF_2);
        mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
        al_attach_mixer_to_voice(mixer, voice);
        sfx_attack = load_sample(              "Attack.ogg",               mixer);
        sfx_pikmin_attack = load_sample(       "Pikmin_attack.ogg",        mixer);
        sfx_pikmin_carrying = load_sample(     "Pikmin_carrying.ogg",      mixer);
        sfx_pikmin_carrying_grab = load_sample("Pikmin_carrying_grab.ogg", mixer);
        sfx_pikmin_caught = load_sample(       "Pikmin_caught.ogg",        mixer);
        sfx_pikmin_dying = load_sample(        "Pikmin_dying.ogg",         mixer);
        sfx_pikmin_held = load_sample(         "Pikmin_held.ogg",          mixer);
        sfx_pikmin_idle = load_sample(         "Pikmin_idle.ogg",          mixer);
        sfx_pikmin_thrown = load_sample(       "Pikmin_thrown.ogg",        mixer);
        sfx_pikmin_pluck = load_sample(        "Pikmin_pluck.ogg",         mixer);
        sfx_pikmin_plucked = load_sample(      "Pikmin_plucked.ogg",       mixer);
        sfx_pikmin_called = load_sample(       "Pikmin_called.ogg",        mixer);
        sfx_dismiss = load_sample(             "Dismiss.ogg",              mixer);
        sfx_olimar_whistle = load_sample(      "Olimar_whistle.ogg",       mixer);
        sfx_louie_whistle = load_sample(       "Louie_whistle.ogg",        mixer);
        sfx_president_whistle = load_sample(   "President_whistle.ogg",    mixer);
        sfx_olimar_name_call = load_sample(    "Olimar_name_call.ogg",     mixer);
        sfx_louie_name_call = load_sample(     "Louie_name_call.ogg",      mixer);
        sfx_president_name_call = load_sample( "President_name_call.ogg",  mixer);
        sfx_throw = load_sample(               "Throw.ogg",                mixer);
        sfx_switch_pikmin = load_sample(       "Switch_Pikmin.ogg",        mixer);
        sfx_camera = load_sample(              "Camera.ogg",               mixer);
        
        //Game content.
        load_game_content();
        
        //Initializing game things.
        spray_amounts.clear();
        size_t n_spray_types = spray_types.size();
        for(size_t s = 0; s < n_spray_types; s++) { spray_amounts.push_back(0); }
        pikmin_in_onions.clear();
        for(auto o = pikmin_in_onions.begin(); o != pikmin_in_onions.end(); o++) { o->second = 0; }
        
        load_area("Play", false);
        load_area_textures();
        generate_area_images();
        
        for(size_t m = 0; m < cur_area_map.mob_generators.size(); m++) {
            mob_gen* m_ptr = cur_area_map.mob_generators[m];
            if(m_ptr->category == MOB_CATEGORY_ENEMIES) {
                create_mob(new enemy(m_ptr->x, m_ptr->y, (enemy_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
            } else if(m_ptr->category == MOB_CATEGORY_LEADERS) {
                create_mob(new leader(m_ptr->x, m_ptr->y, (leader_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
            } else if(m_ptr->category == MOB_CATEGORY_ONIONS) {
                create_mob(new onion(m_ptr->x, m_ptr->y, (onion_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
            } else if(m_ptr->category == MOB_CATEGORY_PELLETS) {
                create_mob(new pellet(m_ptr->x, m_ptr->y, (pellet_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
            } else if(m_ptr->category == MOB_CATEGORY_PIKMIN) {
                create_mob(new pikmin(m_ptr->x, m_ptr->y, (pikmin_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
            } else if(m_ptr->category == MOB_CATEGORY_SHIPS) {
                create_mob(new ship(m_ptr->x, m_ptr->y, (ship_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
            } else if(m_ptr->category == MOB_CATEGORY_SPECIAL) {
                m_ptr->type->create_mob(m_ptr->x, m_ptr->y, m_ptr->angle, m_ptr->vars);
            } else if(m_ptr->category == MOB_CATEGORY_TREASURES) {
                create_mob(new treasure(m_ptr->x, m_ptr->y, (treasure_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
            }
        }
        
        create_mob(new pikmin(-50, 200, pikmin_types["Red Pikmin"], 0, ""));
        pikmin_list.back()->maturity = 1;
        create_mob(new pikmin(-40, 200, pikmin_types["Red Pikmin"], 0, ""));
        pikmin_list.back()->maturity = 2;
        create_mob(new pikmin(-30, 200, pikmin_types["Red Pikmin"], 0, ""));
        pikmin_list.back()->maturity = 1;
        create_mob(new pikmin(-20, 200, pikmin_types["Yellow Pikmin"], 0, ""));
        pikmin_list.back()->maturity = 2;
        create_mob(new pikmin(-10, 200, pikmin_types["Yellow Pikmin"], 0, ""));
        pikmin_list.back()->maturity = 1;
        create_mob(new pikmin(0, 200, pikmin_types["Yellow Pikmin"], 0, ""));
        pikmin_list.back()->maturity = 2;
        create_mob(new pikmin(30, 150, pikmin_types["Blue Pikmin"], 0, ""));
        pikmin_list.back()->fsm.set_state(0);
        pikmin_list.back()->spawned = true;
        create_mob(new pikmin(50, 150, pikmin_types["Blue Pikmin"], 0, ""));
        pikmin_list.back()->fsm.set_state(0);
        pikmin_list.back()->spawned = true;
        create_mob(new pikmin(70, 150, pikmin_types["Blue Pikmin"], 0, ""));
        pikmin_list.back()->fsm.set_state(0);
        pikmin_list.back()->spawned = true;
        for(unsigned char p = 0; p < 10; p++) {
            for(auto t = pikmin_types.begin(); t != pikmin_types.end(); t++) {
                create_mob(new pikmin(20 + 10 * p + 3 * distance(pikmin_types.begin(), t), 200, t->second, 0, ""));
                pikmin_list.back()->maturity = 2;
            }
        }
        /*
        create_mob(new pellet(320, -100, pellet_types["Red 1"], 0, ""));
        create_mob(new pellet(250, -100, pellet_types["Red 5"], 0, ""));
        create_mob(new pellet(150, -100, pellet_types["Red 10"], 0, ""));
        create_mob(new pellet(0, -100, pellet_types["Red 20"], 0, ""));*/
        spray_amounts[0] = spray_amounts[1] = 10;
        spray_types[0].bmp_spray = bmp_ub_spray;
        spray_types[1].bmp_spray = bmp_us_spray;
        pikmin_in_onions[pikmin_types["Red Pikmin"]] = 200;
        pikmin_in_onions[pikmin_types["Yellow Pikmin"]] = 180;
        pikmin_in_onions[pikmin_types["Blue Pikmin"]] = 160;
        
        al_hide_mouse_cursor(display);
    } else {
        al_show_mouse_cursor(display);
        if(cur_screen == SCREEN_ANIMATION_EDITOR) {
            animation_editor::load();
        } else if(cur_screen == SCREEN_AREA_EDITOR) {
            area_editor::load();
        }
    }
    
    
    //Main loop.
    al_start_timer(timer);
    while(running) {
    
        /*  ************************************************
          *** | _ |                                  | _ | ***
        *****  \_/           EVENT HANDLING           \_/  *****
          *** +---+                                  +---+ ***
            ************************************************/
        
        al_wait_for_event(queue, &ev);
        
        if(cur_screen == SCREEN_GAME) {
            handle_game_controls(ev);
        } else if(cur_screen == SCREEN_AREA_EDITOR) {
            area_editor::handle_controls(ev);
        } else if(cur_screen == SCREEN_ANIMATION_EDITOR) {
            animation_editor::handle_controls(ev);
        }
        
        if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
            //scr_w = ev.display.width;
            //scr_h = ev.display.height;
            
        } else if(ev.type == ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(queue)) {
            double cur_time = al_get_time();
            if(prev_frame_time == 0) prev_frame_time = cur_time - 1.0f / game_fps; //Failsafe.
            delta_t = cur_time - prev_frame_time;
            
            if(cur_screen == SCREEN_GAME) {
                do_logic();
                do_drawing();
            } else if(cur_screen == SCREEN_AREA_EDITOR) {
                area_editor::do_logic();
            } else if(cur_screen == SCREEN_ANIMATION_EDITOR) {
                animation_editor::do_logic();
            }
            
            prev_frame_time = cur_time;
        }
    }
    
}
