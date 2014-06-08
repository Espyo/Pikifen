/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program start and main loop.
 */

//ToDo check for ".c_str()" in the code, as apparently I have some atois and atof instead of toi and tof.

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
    //Install Allegro and initialize modules.
    al_init();
    al_install_mouse();
    al_install_keyboard();
    al_install_audio();
    al_install_joystick();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_acodec_addon();
    
    //Options and default controls.
    //ToDo create a manager for this, like the mob folder manager and whatnot.
    controls.push_back(control_info(BUTTON_PUNCH, 0, "mb_1"));
    controls.push_back(control_info(BUTTON_WHISTLE, 0, "mb_2"));
    controls.push_back(control_info(BUTTON_MOVE_RIGHT, 0, "k_4"));
    controls.push_back(control_info(BUTTON_MOVE_UP, 0, "k_23"));
    controls.push_back(control_info(BUTTON_MOVE_LEFT, 0, "k_1"));
    controls.push_back(control_info(BUTTON_MOVE_DOWN, 0, "k_19"));
    controls.push_back(control_info(BUTTON_MOVE_GROUP_TO_CURSOR, 0, "k_75"));
    controls.push_back(control_info(BUTTON_SWITCH_CAPTAIN_RIGHT, 0, "k_64"));
    controls.push_back(control_info(BUTTON_DISMISS, 0, "k_217"));
    controls.push_back(control_info(BUTTON_USE_SPRAY_1, 0, "k_18"));
    controls.push_back(control_info(BUTTON_USE_SPRAY_2, 0, "k_6"));
    controls.push_back(control_info(BUTTON_USE_SPRAY, 0, "k_18"));
    controls.push_back(control_info(BUTTON_SWITCH_SPRAY_RIGHT, 0, "k_5"));
    controls.push_back(control_info(BUTTON_SWITCH_SPRAY_LEFT, 0, "k_17"));
    controls.push_back(control_info(BUTTON_SWITCH_TYPE_RIGHT, 0, "mb_2"));
    controls.push_back(control_info(BUTTON_SWITCH_ZOOM, 0, "k_3"));
    controls.push_back(control_info(BUTTON_ZOOM_IN, 0, "mwu"));
    controls.push_back(control_info(BUTTON_ZOOM_OUT, 0, "mwd"));
    controls.push_back(control_info(BUTTON_LIE_DOWN, 0, "k_26"));
    controls.push_back(control_info(BUTTON_PAUSE, 0, "k_59"));
    load_options();
    save_options();
    
    //Event stuff.
    al_set_new_window_position(window_x, window_y);
    display = al_create_display(scr_w, scr_h);
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / game_fps);
    
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_joystick_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    ALLEGRO_EVENT ev;
    
    //Other initial things.
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    al_set_window_title(display, "Pikmin fangame engine");
    if(smooth_scaling) al_set_new_bitmap_flags(ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR | ALLEGRO_MIPMAP);
    al_reserve_samples(16);
    srand(time(NULL));
    
    sector_types.register_type(SECTOR_TYPE_NORMAL, "Normal");
    sector_types.register_type(SECTOR_TYPE_BOTTOMLESS_PIT, "Bottomless pit");
    sector_types.register_type(SECTOR_TYPE_LANDING_SITE, "Landing site");
    
    //Error bitmap.
    //ToDo move this somewhere else, maybe?
    int bmp_error_w, bmp_error_h;
    al_get_text_dimensions(al_create_builtin_font(), "ERROR", NULL, NULL, &bmp_error_w, &bmp_error_h);
    bmp_error = al_create_bitmap(bmp_error_w, bmp_error_h);
    al_set_target_bitmap(bmp_error); {
        al_draw_text(al_create_builtin_font(), al_map_rgb(255, 0, 0), 0, 0, 0, "ERROR");
    } al_set_target_backbuffer(display);
    
    
    int font_ranges[] = {
        0x0020, 0x007E, //ASCII
        0x00A0, 0x00A1, //Non-breaking space and inverted !
        0x00BF, 0x00FF, //Inverted ? and European vowels and such
    };
    int counter_font_ranges[] = {
        0x002D, 0x002D, //Dash
        0x002F, 0x0039, //Slash and numbers
        0x0078, 0x0078, //x
    };
    int value_font_ranges[] = {
        0x0024, 0x0024, //Dollar sign
        0x002D, 0x002D, //Dash
        0x0030, 0x0039, //Numbers
    };
    
    ALLEGRO_BITMAP* temp_font_bitmap = load_bmp("Font.png");  //We can't load the font directly because we want to set the ranges.
    if(temp_font_bitmap) font = al_grab_font_from_bitmap(temp_font_bitmap, 3, font_ranges);
    al_destroy_bitmap(temp_font_bitmap);
    
    temp_font_bitmap = load_bmp("Area_name_font.png");
    if(temp_font_bitmap) font_area_name = al_grab_font_from_bitmap(temp_font_bitmap, 3, font_ranges);
    al_destroy_bitmap(temp_font_bitmap);
    
    temp_font_bitmap = load_bmp("Counter_font.png");
    if(temp_font_bitmap) font_counter = al_grab_font_from_bitmap(temp_font_bitmap, 3, counter_font_ranges);
    al_destroy_bitmap(temp_font_bitmap);
    
    temp_font_bitmap = load_bmp("Value_font.png");
    if(temp_font_bitmap) font_value = al_grab_font_from_bitmap(temp_font_bitmap, 3, value_font_ranges);
    al_destroy_bitmap(temp_font_bitmap);
    
    if(font) font_h = al_get_font_line_height(font);
    if(font_counter) font_counter_h = al_get_font_line_height(font_counter);
    
    mob_folders.register_folder(MOB_FOLDER_NONE, "None", "None", [] (vector<string> &) { },
    [] (const string &) -> mob_type* { return NULL; });
    
    mob_folders.register_folder(MOB_FOLDER_ENEMIES, "Enemies", "Enemy", [] (vector<string> &li) {
        for(auto e = enemy_types.begin(); e != enemy_types.end(); e++) li.push_back(e->first);
    }, [] (const string & n) -> mob_type* {
        auto it = enemy_types.find(n); if(it == enemy_types.end()) return NULL; return it->second;
    });
    
    mob_folders.register_folder(MOB_FOLDER_LEADERS, "Leaders", "Leader", [] (vector<string> &li) {
        for(auto l = leader_types.begin(); l != leader_types.end(); l++) li.push_back(l->first);
    }, [] (const string & n) -> mob_type* {
        auto it = leader_types.find(n); if(it == leader_types.end()) return NULL; return it->second;
    });
    
    mob_folders.register_folder(MOB_FOLDER_ONIONS, "Onions", "Onion", [] (vector<string> &li) {
        for(auto o = onion_types.begin(); o != onion_types.end(); o++) li.push_back(o->first);
    }, [] (const string & n) -> mob_type* {
        auto it = onion_types.find(n); if(it == onion_types.end()) return NULL; return it->second;
    });
    
    mob_folders.register_folder(MOB_FOLDER_PELLETS, "Pellets", "Pellet", [] (vector<string> &li) {
        for(auto p = pellet_types.begin(); p != pellet_types.end(); p++) li.push_back(p->first);
    }, [] (const string & n) -> mob_type* {
        auto it = pellet_types.find(n); if(it == pellet_types.end()) return NULL; return it->second;
    });
    
    mob_folders.register_folder(MOB_FOLDER_PIKMIN, "Pikmin", "Pikmin", [] (vector<string> &li) {
        for(auto p = pikmin_types.begin(); p != pikmin_types.end(); p++) li.push_back(p->first);
    }, [] (const string & n) -> mob_type* {
        auto it = pikmin_types.find(n); if(it == pikmin_types.end()) return NULL; return it->second;
    });
    
    mob_folders.register_folder(MOB_FOLDER_SPECIAL, "Special", "Special", [] (vector<string> &li) {
        for(auto s = special_mob_types.begin(); s != special_mob_types.end(); s++) li.push_back(s->first);
    }, [] (const string & n) -> mob_type* {
        auto it = special_mob_types.find(n); if(it == special_mob_types.end()) return NULL; return it->second;
    });
    
    mob_folders.register_folder(MOB_FOLDER_TREASURES, "Treasures", "Treasure", [] (vector<string> &li) {
        for(auto t = treasure_types.begin(); t != treasure_types.end(); t++) li.push_back(t->first);
    }, [] (const string & n) -> mob_type* {
        auto it = treasure_types.find(n); if(it == treasure_types.end()) return NULL; return it->second;
    });
    
    
    
    
    mob_type* info_spot_mt = new mob_type();
    info_spot_mt->name = "Info spot";
    info_spot_mt->size = 32;
    special_mob_types["Info spot"] = info_spot_mt;
    
    mob_type* nectar_mt = new mob_type();
    nectar_mt->name = "Nectar";
    nectar_mt->always_active = true;
    nectar_mt->size = 16;
    special_mob_types["Nectar"] = nectar_mt;
    
    mob_type* ship_mt = new mob_type();
    ship_mt->name = "Ship";
    ship_mt->always_active = true;
    ship_mt->size = 140;
    special_mob_types["Ship"] = ship_mt;
    
    
    cur_screen = SCREEN_GAME;
    if(argc == 2) cur_screen = SCREEN_ANIMATION_EDITOR;
    else if(argc == 3) cur_screen = SCREEN_AREA_EDITOR;
    
    if(cur_screen == SCREEN_GAME) {
    
        //Graphics.
        bmp_red_onion = load_bmp("Red_onion.png");
        bmp_yellow_onion = load_bmp("Yellow_onion.png");
        bmp_blue_onion = load_bmp("Blue_onion.png");
        bmp_ship = load_bmp("Ship.png");
        bmp_red_pellet[0] = load_bmp("Red_1_pellet.png");
        bmp_red_pellet[1] = load_bmp("Red_5_pellet.png");
        bmp_red_pellet[2] = load_bmp("Red_10_pellet.png");
        bmp_red_pellet[3] = load_bmp("Red_20_pellet.png");
        
        bmp_bubble = load_bmp(              "Bubble.png");
        bmp_cursor = load_bmp(              "Cursor.png");
        bmp_day_bubble = load_bmp(          "Day_bubble.png");
        bmp_enemy_spirit = load_bmp(        "Enemy_spirit.png");
        bmp_hard_bubble = load_bmp(         "Hard_bubble.png");
        bmp_icon = load_bmp(                "Icon.png");
        bmp_idle_glow = load_bmp(           "Idle_glow.png");
        bmp_message_box = load_bmp(         "Message_box.png");
        bmp_mouse_cursor = load_bmp(        "Mouse_cursor.png");
        bmp_mouse_cursor_invalid = load_bmp("Mouse_cursor_invalid.png");
        bmp_move_group_arrow = load_bmp(    "Move_group_arrow.png");
        bmp_nectar = load_bmp(              "Nectar.png");
        bmp_number_bubble = load_bmp(       "Number_bubble.png");
        bmp_pikmin_spirit = load_bmp(       "Pikmin_spirit.png");
        bmp_shadow = load_bmp(              "Shadow.png");
        bmp_smack = load_bmp(               "Smack.png");
        bmp_smoke = load_bmp(               "Smoke.png");
        bmp_sparkle = load_bmp(             "Sparkle.png");
        bmp_sun = load_bmp(                 "Sun.png");
        bmp_sun_bubble = load_bmp(          "Sun_bubble.png");
        bmp_tp = load_bmp(                  "TP.png");
        bmp_ub_spray = load_bmp(            "Ultra-bitter_spray.png");
        bmp_us_spray = load_bmp(            "Ultra-spicy_spray.png");
        
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
        
        //ToDo
        //Some temp variables.
        sector s = sector();
        /*test_sector = sector();
        test_sector.z = 100;
        test_sector.floors[0].texture = bmp_test;
        
        test_linedefs.push_back(linedef(0, 0, 0, 0, 0, 0));
        test_linedefs.push_back(linedef(100, 0, 0, 0, 0, 0));
        test_linedefs.push_back(linedef(100, 100, 0, 0, 0, 0));
        test_linedefs.push_back(linedef(50, 150, 0, 0, 0, 0));
        test_linedefs.push_back(linedef(0, 100, 0, 0, 0, 0));*/
        
        load_area("Play", false);
        load_area_textures();
        generate_area_images();
        
        for(size_t m = 0; m < cur_area_map.mob_generators.size(); m++) {
            mob_gen* m_ptr = cur_area_map.mob_generators[m];
            if(m_ptr->folder == MOB_FOLDER_ENEMIES) {
                create_mob(new enemy(m_ptr->x, m_ptr->y, &s, (enemy_type*) m_ptr->type));
            } else if(m_ptr->folder == MOB_FOLDER_LEADERS) {
                create_mob(new leader(m_ptr->x, m_ptr->y, &s, (leader_type*) m_ptr->type));
            } else if(m_ptr->folder == MOB_FOLDER_ONIONS) {
                create_mob(new onion(m_ptr->x, m_ptr->y, &s, (onion_type*) m_ptr->type));
            } else if(m_ptr->folder == MOB_FOLDER_PELLETS) {
                create_mob(new pellet(m_ptr->x, m_ptr->y, &s, (pellet_type*) m_ptr->type));
            } else if(m_ptr->folder == MOB_FOLDER_PIKMIN) {
                create_mob(new pikmin(m_ptr->x, m_ptr->y, &s, (pikmin_type*) m_ptr->type));
            } else if(m_ptr->folder == MOB_FOLDER_SPECIAL) {
                create_mob(new mob(m_ptr->x, m_ptr->y, 0, m_ptr->type, &s));
            } else if(m_ptr->folder == MOB_FOLDER_TREASURES) {
                create_mob(new treasure(m_ptr->x, m_ptr->y, &s, (treasure_type*) m_ptr->type));
            }
        }
        
        create_mob(new pikmin(30, 30, &s, pikmin_types["Red Pikmin"]));
        pikmin_list.back()->maturity = 1;
        create_mob(new pikmin(40, 30, &s, pikmin_types["Red Pikmin"]));
        pikmin_list.back()->maturity = 2;
        create_mob(new pikmin(50, 30, &s, pikmin_types["Red Pikmin"]));
        pikmin_list.back()->maturity = 1;
        create_mob(new pikmin(60, 30, &s, pikmin_types["Yellow Pikmin"]));
        pikmin_list.back()->maturity = 2;
        create_mob(new pikmin(70, 30, &s, pikmin_types["Yellow Pikmin"]));
        pikmin_list.back()->maturity = 1;
        create_mob(new pikmin(80, 30, &s, pikmin_types["Yellow Pikmin"]));
        pikmin_list.back()->maturity = 2;
        create_mob(new pikmin(30, 200, &s, pikmin_types["Blue Pikmin"]));
        pikmin_list.back()->set_state(PIKMIN_STATE_BURIED);
        create_mob(new pikmin(50, 200, &s, pikmin_types["Blue Pikmin"]));
        pikmin_list.back()->set_state(PIKMIN_STATE_BURIED);
        create_mob(new pikmin(70, 200, &s, pikmin_types["Blue Pikmin"]));
        pikmin_list.back()->set_state(PIKMIN_STATE_BURIED);
        for(unsigned char p = 0; p < 10; p++) {
            for(auto t = pikmin_types.begin(); t != pikmin_types.end(); t++) {
                create_mob(new pikmin(100 + 10 * p + 3 * distance(pikmin_types.begin(), t), 30, &s, t->second));
                pikmin_list.back()->maturity = 2;
            }
        }
        create_mob(new info_spot(400, 0, &s, "Onions.", false));
        create_mob(new info_spot(-300, 0, &s, "http://www.pikminfanon.com/\nTopic:Pikmin_Engine_by_Espyo", false));
        create_mob(
            new info_spot(
                -300, -100, &s,
                "This is a test message.\n"
                "Second line.\n"
                "Third line, which is way too long to even be existing.\n"
                "Secret fourth line!\n"
                "Fifth line? Sure!\n"
                "6th incoming.",
                true
            )
        );
        create_mob(new nectar(0, 400, &s));
        create_mob(new pellet(320, -100, &s, pellet_types["Red 1"]));
        create_mob(new pellet(250, -100, &s, pellet_types["Red 5"]));
        create_mob(new pellet(150, -100, &s, pellet_types["Red 10"]));
        create_mob(new pellet(0, -100, &s, pellet_types["Red 20"]));
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