#pragma warning(disable : 4996) //Disables warning about localtime being deprecated.

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
#include "vars.h"

using namespace std;

int main() {
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
    
    //Error bitmap.
    //ToDo move this somewhere else, maybe?
    int bmp_error_w, bmp_error_h;
    al_get_text_dimensions(al_create_builtin_font(), "ERROR", NULL, NULL, &bmp_error_w, &bmp_error_h);
    bmp_error = al_create_bitmap(bmp_error_w, bmp_error_h);
    al_set_target_bitmap(bmp_error); {
        al_draw_text(al_create_builtin_font(), al_map_rgb(255, 0, 0), 0, 0, 0, "ERROR");
    } al_set_target_backbuffer(display);
    
    //Graphics.
    bmp_olimar = load_bmp("Olimar.png");
    bmp_louie = load_bmp("Louie.png");
    bmp_president = load_bmp("President.png");
    for(unsigned char m = 0; m < 3; m++) {
        string maturity_str;
        if(m == 0) maturity_str = "leaf";
        else if(m == 1) maturity_str = "bud";
        else maturity_str = "flower";
        bmp_red[m] = load_bmp(("Red_" + maturity_str + ".png").c_str());
        bmp_yellow[m] = load_bmp(("Yellow_" + maturity_str + ".png").c_str());
        bmp_blue[m] = load_bmp(("Blue_" + maturity_str + ".png").c_str());
        bmp_red_buried[m] = load_bmp(("Red_buried_" + maturity_str + ".png").c_str());
        bmp_yellow_buried[m] = load_bmp(("Yellow_buried_" + maturity_str + ".png").c_str());
        bmp_blue_buried[m] = load_bmp(("Blue_buried_" + maturity_str + ".png").c_str());
        bmp_red_idle[m] = load_bmp(("Red_idle_" + maturity_str + ".png").c_str());
        bmp_yellow_idle[m] = load_bmp(("Yellow_idle_" + maturity_str + ".png").c_str());
        bmp_blue_idle[m] = load_bmp(("Blue_idle_" + maturity_str + ".png").c_str());
    }
    bmp_red_onion = load_bmp("Red_onion.png");
    bmp_yellow_onion = load_bmp("Yellow_onion.png");
    bmp_blue_onion = load_bmp("Blue_onion.png");
    bmp_cursor = load_bmp("Cursor.png");
    bmp_mouse_cursor = load_bmp("Mouse_cursor.png");
    bmp_background = load_bmp("Background.png");
    bmp_bubble = load_bmp("Bubble.png");
    bmp_day_bubble = load_bmp("Day_bubble.png");
    bmp_health_bubble = load_bmp("Health_bubble.png");
    bmp_sun = load_bmp("Sun.png");
    bmp_shadow = load_bmp("Shadow.png");
    bmp_ship = load_bmp("Ship.png");
    bmp_idle_glow = load_bmp("Idle_glow.png");
    bmp_ub_spray = load_bmp("Ultra-bitter_spray.png");
    bmp_us_spray = load_bmp("Ultra-spicy_spray.png");
    bmp_move_group_arrow = load_bmp("Move_group_arrow.png");
    bmp_test = load_bmp("Test.png");
    bmp_nectar = load_bmp("Nectar.png");
    bmp_icon = load_bmp("Icon.png");
    bmp_red_pellet[0] = load_bmp("Red_1_pellet.png");
    bmp_red_pellet[1] = load_bmp("Red_5_pellet.png");
    bmp_red_pellet[2] = load_bmp("Red_10_pellet.png");
    bmp_red_pellet[3] = load_bmp("Red_20_pellet.png");
    bmp_olimar_lying = load_bmp("Olimar_lying.png");
    bmp_louie_lying = load_bmp("Louie_lying.png");
    bmp_president_lying = load_bmp("President_lying.png");
    bmp_message_box = load_bmp("Message_box.png");
    bmp_cloaking_burrow_nit = load_bmp("Cloaking_Burrow-nit.png");
    
    int font_ranges[] = {
        0x0020, 0x007F, //ASCII
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
    if(temp_font_bitmap) font = al_grab_font_from_bitmap(temp_font_bitmap, 1, font_ranges);
    temp_font_bitmap = load_bmp("Area_name_font.png");
    if(temp_font_bitmap) font_area_name = al_grab_font_from_bitmap(temp_font_bitmap, 1, font_ranges);
    temp_font_bitmap = load_bmp("Counter_font.png");
    if(temp_font_bitmap) font_counter = al_grab_font_from_bitmap(temp_font_bitmap, 3, counter_font_ranges);
    temp_font_bitmap = load_bmp("Value_font.png");
    if(temp_font_bitmap) font_value = al_grab_font_from_bitmap(temp_font_bitmap, 3, value_font_ranges);
    al_destroy_bitmap(temp_font_bitmap);
    font_h = al_get_font_line_height(font);
    
    al_set_display_icon(display, bmp_icon);
    
    //Sound effects.
    sfx_pikmin_held = load_sample("Pikmin_held.ogg");
    sfx_pikmin_thrown = load_sample("Pikmin_thrown.ogg");
    sfx_pikmin_plucked = load_sample("Pikmin_plucked.ogg");
    sfx_pikmin_called = load_sample("Pikmin_called.ogg");
    sfx_dismiss = load_sample("Dismiss.ogg");
    sfx_olimar_whistle = load_sample("Olimar_whistle.ogg");
    sfx_louie_whistle = load_sample("Louie_whistle.ogg");
    sfx_president_whistle = load_sample("President_whistle.ogg");
    sfx_olimar_name_call = load_sample("Olimar_name_call.ogg");
    sfx_louie_name_call = load_sample("Louie_name_call.ogg");
    sfx_president_name_call = load_sample("President_name_call.ogg");
    sfx_throw = load_sample("Throw.ogg");
    sfx_switch_pikmin = load_sample("Switch_Pikmin.ogg");
    sfx_camera = load_sample("Camera.ogg");
    
    //Game content.
    load_game_content();
    
    //Initializing game things.
    spray_amounts.clear();
    size_t n_spray_types = spray_types.size();
    for(size_t s = 0; s < n_spray_types; s++) { spray_amounts.push_back(0); }
    pikmin_in_onions.clear();
    for(auto o = pikmin_in_onions.begin(); o != pikmin_in_onions.end(); o++) { o->second = 0; }
    
    //ToDo
    info_spot_mob_type = new mob_type();
    info_spot_mob_type->name = "Info spot";
    info_spot_mob_type->size = 32;
    info_spot_mob_type->sight_radius = 100; //ToDo testing only. Remove.
    info_spot_mob_type->near_radius = 30; //ToDo testing only. Remove.
    
    nectar_mob_type = new mob_type();
    nectar_mob_type->name = "Nectar";
    nectar_mob_type->always_active = true;
    nectar_mob_type->size = 16;
    
    ship_mob_type = new mob_type();
    ship_mob_type->name = "Ship";
    ship_mob_type->always_active = true;
    ship_mob_type->size = 140;
    
    //Some temp variables.
    sector s = sector();
    /*test_sector = sector();
    test_sector.floors[0].z = 100;
    test_sector.floors[0].texture = bmp_test;
    
    test_linedefs.push_back(linedef(0, 0, 0, 0, 0, 0));
    test_linedefs.push_back(linedef(100, 0, 0, 0, 0, 0));
    test_linedefs.push_back(linedef(100, 100, 0, 0, 0, 0));
    test_linedefs.push_back(linedef(50, 150, 0, 0, 0, 0));
    test_linedefs.push_back(linedef(0, 100, 0, 0, 0, 0));*/
    
    load_area("test");
    generate_area_images();
    
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
    pikmin_list.back()->buried = true;
    create_mob(new pikmin(50, 200, &s, pikmin_types["Blue Pikmin"]));
    pikmin_list.back()->buried = true;
    create_mob(new pikmin(70, 200, &s, pikmin_types["Blue Pikmin"]));
    pikmin_list.back()->buried = true;
    for(unsigned char p = 0; p < 10; p++) {
        for(auto t = pikmin_types.begin(); t != pikmin_types.end(); t++) {
            create_mob(new pikmin(100 + 10 * p + 3 * distance(pikmin_types.begin(), t), 30, &s, t->second));
        }
    }
    create_mob(new info_spot(300, 0, &s, "Treasure.", false));
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
    
    cur_screen = SCREEN_ANIMATION_EDITOR;
    animation a = animation(); //load_animations(data_node("Test.txt").get_child_by_name("animations")).begin()->second;
    enemies.back()->anim = animation_instance(&a);
    
    load_animations();
    ed_mode = EDITOR_MODE_NORMAL;
    
    if(cur_screen == SCREEN_GAME) {
        al_hide_mouse_cursor(display);
    } else {
        al_show_mouse_cursor(display);
        if(cur_screen == SCREEN_ANIMATION_EDITOR) {
            ed_gui = new lafi_gui(scr_w, scr_h);
            
            //Main frame.
            auto lambda_save_animation = [] (lafi_widget*) { save_animation(); };
            
            lafi_frame* frm_main = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
            
            frm_main->add("btn_object", new lafi_button(          scr_w - 200, 8,  scr_w - 8,  40, "", "Choose an object."));
            frm_main->add("btn_animation", new lafi_button(       scr_w - 200, 48, scr_w - 32, 80, "", "Choose an animation."));
            frm_main->add("btn_delete_animation", new lafi_button(scr_w - 24,  48, scr_w - 8,  80, "-", "Delete the current animation."));
            
            frm_main->widgets["btn_animation"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                ed_gui->widgets["frm_main"]->flags = LAFI_FLAG_INVISIBLE | LAFI_FLAG_DISABLED;
                ed_gui->widgets["frm_choose_animation"]->flags = 0;
            };
            frm_main->widgets["btn_delete_animation"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                ed_anims.erase(ed_anims.find(ed_anim_name));
                ed_anim_name = "";
                ed_anim = NULL;
                ed_cur_frame_nr = string::npos;
                fill_choose_animation_frame();
                ((lafi_button*) ed_gui->widgets["frm_main"]->widgets["btn_animation"])->text = "";
                ed_gui->widgets["frm_main"]->widgets["frm_animation"]->flags = LAFI_FLAG_INVISIBLE | LAFI_FLAG_DISABLED;
            };
            ed_gui->add("frm_main", frm_main);
            
            
            //Animation frame.
            lafi_frame* frm_animation = new lafi_frame(scr_w - 208, 88, scr_w, scr_h - 48);
            frm_animation->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
            
            frm_animation->add("btn_prev_frame", new lafi_button(  scr_w - 200, 96,  scr_w - 168, 128, "<",   "Go to the previous frame."));
            frm_animation->add("btn_play", new lafi_button(        scr_w - 160, 96,  scr_w - 128, 128, "P/P", "Play or pause the animation."));
            frm_animation->add("btn_next_frame", new lafi_button(  scr_w - 120, 96,  scr_w - 88,  128, ">",   "Go to the next frame."));
            frm_animation->add("btn_new_frame", new lafi_button(   scr_w - 80,  96,  scr_w - 48,  128, "+",   "Add a new frame after the current one."));
            frm_animation->add("btn_delete_frame", new lafi_button(scr_w - 40,  96,  scr_w - 8,   128, "-",   "Remove the current frame."));
            frm_animation->add("lbl_loop_frame", new lafi_label(   scr_w - 200, 136, scr_w - 96, 152, "Loop frame:"));
            frm_animation->add("txt_loop_frame", new lafi_textbox( scr_w - 88,  136, scr_w - 8,   152, ""));
            frm_animation->add("lbl_frame_info", new lafi_label(   scr_w - 200, 160, scr_w - 8,   176, ""));
            
            frm_animation->widgets["btn_prev_frame"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                ed_anim_playing = false;
                if(ed_anim->frames.size() > 0) {
                    if(ed_cur_frame_nr == string::npos) ed_cur_frame_nr = 0;
                    else if(ed_cur_frame_nr == 0) ed_cur_frame_nr = ed_anim->frames.size() - 1;
                    else ed_cur_frame_nr--;
                }
                load_animation_fields();
            };
            frm_animation->widgets["btn_next_frame"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                ed_anim_playing = false;
                if(ed_anim->frames.size() > 0) {
                    if(ed_cur_frame_nr == ed_anim->frames.size() - 1 || ed_cur_frame_nr == string::npos) ed_cur_frame_nr = 0;
                    else ed_cur_frame_nr++;
                }
                load_animation_fields();
            };
            frm_animation->widgets["btn_play"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                if(ed_anim->frames.size() < 2) {
                    ed_anim_playing = false;
                } else {
                    ed_anim_playing = !ed_anim_playing;
                    if(ed_anim->frames.size() > 0 && ed_cur_frame_nr == string::npos) ed_cur_frame_nr = 0;
                    ed_cur_frame_time = 0;
                }
            };
            frm_animation->widgets["btn_new_frame"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                ed_anim_playing = false;
                if(ed_cur_frame_nr != string::npos) {
                    frame* f = &ed_anim->frames[ed_cur_frame_nr];
                    
                    ed_anim->frames.insert(
                        ed_anim->frames.begin() + ed_cur_frame_nr,
                        (*f).clone()
                    );
                    ed_cur_frame_nr++;
                } else {
                    ed_anim->frames.push_back(frame());
                    ed_cur_frame_nr = 0;
                }
                load_animation_fields();
            };
            frm_animation->widgets["btn_delete_frame"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                ed_anim_playing = false;
                if(ed_cur_frame_nr != string::npos) {
                    ed_anim->frames.erase(ed_anim->frames.begin() + ed_cur_frame_nr);
                    if(ed_anim->frames.size() == 0) ed_cur_frame_nr = string::npos;
                    else if(ed_cur_frame_nr >= ed_anim->frames.size()) ed_cur_frame_nr = ed_anim->frames.size() - 1;
                }
                load_animation_fields();
            };
            frm_animation->widgets["lbl_loop_frame"]->description = "The animation loops back to this frame after the final one.";
            frm_animation->widgets["txt_loop_frame"]->lose_focus_handler = lambda_save_animation;
            
            frm_main->add("frm_animation", frm_animation);
            
            
            lafi_frame* frm_frame = new lafi_frame(scr_w - 208, 184, scr_w, scr_h - 48);
            frm_frame->flags = LAFI_FLAG_INVISIBLE | LAFI_FLAG_DISABLED;
            
            frm_frame->add("lbl_frame_file", new lafi_label(  scr_w - 200, 192, scr_w - 152, 208, "File:"));
            frm_frame->add("txt_frame_file", new lafi_textbox(scr_w - 152, 192, scr_w - 8,   208));
            frm_frame->add("lbl_frame_fxy",  new lafi_label(  scr_w - 200, 216, scr_w - 120, 232, "File X&Y:"));
            frm_frame->add("txt_frame_fx",   new lafi_textbox(scr_w - 112, 216, scr_w - 64,  232));
            frm_frame->add("txt_frame_fy",   new lafi_textbox(scr_w - 56,  216, scr_w - 8,   232));
            frm_frame->add("lbl_frame_fwh",  new lafi_label(  scr_w - 200, 240, scr_w - 120, 256, "File W&H:"));
            frm_frame->add("txt_frame_fw",   new lafi_textbox(scr_w - 112, 240, scr_w - 64,  256));
            frm_frame->add("txt_frame_fh",   new lafi_textbox(scr_w - 56,  240, scr_w - 8,   256));
            frm_frame->add("lbl_frame_gwh",  new lafi_label(  scr_w - 200, 264, scr_w - 120, 280, "Game W&H:"));
            frm_frame->add("txt_frame_gw",   new lafi_textbox(scr_w - 112, 264, scr_w - 64,  280));
            frm_frame->add("txt_frame_gh",   new lafi_textbox(scr_w - 56,  264, scr_w - 8,   280));
            frm_frame->add("lbl_frame_oxy",  new lafi_label(  scr_w - 200, 288, scr_w - 120, 304, "Offset X&Y:"));
            frm_frame->add("txt_frame_ox",   new lafi_textbox(scr_w - 112, 288, scr_w - 64,  304));
            frm_frame->add("txt_frame_oy",   new lafi_textbox(scr_w - 56,  288, scr_w - 8,   304));
            frm_frame->add("lbl_frame_d",    new lafi_label(  scr_w - 200, 312, scr_w - 120, 328, "Duration:"));
            frm_frame->add("txt_frame_d",    new lafi_textbox(scr_w - 112, 312, scr_w - 8,   328));
            frm_frame->add("btn_edit_hitboxes", new lafi_button(scr_w - 200, 336, scr_w - 8, 368, "Edit hitboxes"));
            
            frm_frame->widgets["lbl_frame_fxy"]->description = "Coordinates of the top-left corner of the sprite inside the image file.";
            frm_frame->widgets["lbl_frame_fwh"]->description = "Width and height of the sprite inside the image file.";
            frm_frame->widgets["lbl_frame_gwh"]->description = "Width and height of the sprite in-game.";
            frm_frame->widgets["lbl_frame_oxy"]->description = "Move the sprite with this. Use this for alignment.";
            frm_frame->widgets["btn_edit_hitboxes"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                ed_anim_playing = false;
                ed_mode = EDITOR_MODE_SELECT_HITBOX;
                ed_gui->widgets["frm_main"]->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
                ed_gui->widgets["frm_hitboxes"]->flags = 0;
                load_hitbox_fields();
            };
            frm_frame->widgets["txt_frame_file"]->lose_focus_handler = lambda_save_animation;
            frm_frame->widgets["txt_frame_fx"]->lose_focus_handler =   lambda_save_animation;
            frm_frame->widgets["txt_frame_fy"]->lose_focus_handler =   lambda_save_animation;
            frm_frame->widgets["txt_frame_fw"]->lose_focus_handler =   lambda_save_animation;
            frm_frame->widgets["txt_frame_fh"]->lose_focus_handler =   lambda_save_animation;
            frm_frame->widgets["txt_frame_gw"]->lose_focus_handler =   lambda_save_animation;
            frm_frame->widgets["txt_frame_gh"]->lose_focus_handler =   lambda_save_animation;
            frm_frame->widgets["txt_frame_ox"]->lose_focus_handler =   lambda_save_animation;
            frm_frame->widgets["txt_frame_oy"]->lose_focus_handler =   lambda_save_animation;
            frm_frame->widgets["txt_frame_d"]->lose_focus_handler =    lambda_save_animation;
            
            frm_animation->add("frm_frame", frm_frame);
            
            
            //Switch animation GUI.
            lafi_frame* frm_choose_animation = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
            frm_choose_animation->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
            
            frm_choose_animation->add("txt_new", new lafi_textbox(     scr_w - 200, 16, scr_w - 48, 32));
            frm_choose_animation->add("btn_new", new lafi_button(      scr_w - 40,  8,  scr_w - 8,  40, "+", "Create a new animation with the name on the textbox."));
            frm_choose_animation->add("frm_animations", new lafi_frame(scr_w - 200, 48, scr_w - 32, scr_h - 56));
            frm_choose_animation->add("bar_scroll", new lafi_scrollbar(scr_w - 24,  48, scr_w - 8,  scr_h - 56));
            ed_gui->add("frm_choose_animation", frm_choose_animation);
            
            frm_choose_animation->widgets["btn_new"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                string name = ((lafi_textbox*) ed_gui->widgets["frm_choose_animation"]->widgets["txt_new"])->text;
                if(name.size() == 0) return;
                if(ed_anims.find(name) != ed_anims.end()) return; //Already exists.
                ((lafi_textbox*) ed_gui->widgets["frm_choose_animation"]->widgets["txt_new"])->text = "";
                
                ed_anims[name] = animation();
                ed_gui->widgets["frm_choose_animation"]->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
                ed_gui->widgets["frm_main"]->flags = 0;
                load_animation(name);
                fill_choose_animation_frame();
            };
            
            
            //Hitbox GUI.
            //ToDo next/previous buttons.
            auto lambda_save_hitbox_click = [] (lafi_widget*, int, int) { save_hitbox(); };
            auto lambda_save_hitbox = [] (lafi_widget*) { save_hitbox(); };
            
            lafi_frame* frm_hitboxes = new lafi_frame(scr_w - 208, 0, scr_w, scr_h - 48);
            frm_hitboxes->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
            
            frm_hitboxes->add("btn_new_hitbox", new lafi_button(   scr_w - 200, 8, scr_w - 168, 40, "+", "Create a new hitbox."));
            frm_hitboxes->add("btn_delete_hitbox", new lafi_button(scr_w - 160, 8, scr_w - 128, 40, "-", "Delete the current hitbox."));
            frm_hitboxes->add("btn_prev_hitbox", new lafi_button(  scr_w - 120, 8, scr_w - 88,  40, "<", "Previous hitbox. Use this to find hitboxes off-camera."));
            frm_hitboxes->add("btn_next_hitbox", new lafi_button(  scr_w - 80,  8, scr_w - 48,  40, ">", "Next hitbox. Use this to find hitboxes off-camera."));
            frm_hitboxes->add("btn_back", new lafi_button(         scr_w - 40,  8, scr_w - 8,   40, "X", "Go back."));
            
            lafi_frame* frm_hitbox = new lafi_frame(scr_w - 208, 48, scr_w, scr_h - 48);
            frm_hitbox->add("lbl_hitbox_name", new lafi_label(  scr_w - 200, 56, scr_w - 136, 72, "Name:"));
            frm_hitbox->add("txt_hitbox_name", new lafi_textbox(scr_w - 128, 56, scr_w - 8,   72));
            frm_hitbox->add("lbl_hitbox_xy", new lafi_label(    scr_w - 200, 80, scr_w - 120, 96, "X, Y:"));
            frm_hitbox->add("txt_hitbox_x", new lafi_textbox(   scr_w - 112, 80, scr_w - 64,  96));
            frm_hitbox->add("txt_hitbox_y", new lafi_textbox(   scr_w - 56,  80, scr_w - 8,   96));
            frm_hitbox->add("lbl_hitbox_zr", new lafi_label(    scr_w - 200, 104, scr_w - 120, 120, "Z, radius:"));
            frm_hitbox->add("txt_hitbox_z", new lafi_textbox(   scr_w - 112, 104, scr_w - 64,  120));
            frm_hitbox->add("txt_hitbox_r", new lafi_textbox(   scr_w - 56,  104, scr_w - 8,   120));
            frm_hitbox->add("rad_hitbox_normal", new lafi_radio_button(scr_w - 200, 128, scr_w - 136, 144, "Normal"));
            frm_hitbox->add("rad_hitbox_attack", new lafi_radio_button(scr_w - 128, 128, scr_w - 8,   144, "Attack"));
            frm_hitbox->add("rad_hitbox_shake", new lafi_radio_button( scr_w - 200, 152, scr_w - 136, 168, "Shake"));
            frm_hitbox->add("rad_hitbox_chomp", new lafi_radio_button( scr_w - 128, 152, scr_w - 8,   168, "Chomp"));
            
            lafi_frame* frm_normal_hitbox = new lafi_frame(scr_w - 208, 176, scr_w, scr_h - 48);
            frm_normal_hitbox->add("lbl_hitbox_defense", new lafi_label(         scr_w - 200, 184, scr_w - 72, 200, "Defense mult.:"));
            frm_normal_hitbox->add("txt_hitbox_defense", new lafi_textbox(       scr_w - 64,  184, scr_w - 8,  200));
            frm_normal_hitbox->add("chk_hitbox_latch", new lafi_checkbox(        scr_w - 200, 208, scr_w - 8,  224, "Pikmin can latch"));
            frm_normal_hitbox->add("lbl_normal_hitbox_hazards", new lafi_label(  scr_w - 200, 232, scr_w - 8,  248, "Hazards:"));
            frm_normal_hitbox->add("txt_normal_hitbox_hazards", new lafi_textbox(scr_w - 200, 248, scr_w - 8,  264));
            frm_hitbox->add("frm_normal_hitbox", frm_normal_hitbox);
            
            lafi_frame* frm_attack_hitbox = new lafi_frame(scr_w - 208, 176, scr_w, scr_h - 48);
            frm_attack_hitbox->add("lbl_hitbox_attack", new lafi_label(          scr_w - 200, 184, scr_w - 72, 200, "Attack mult.:"));
            frm_attack_hitbox->add("txt_hitbox_attack", new lafi_textbox(        scr_w - 64,  184, scr_w - 8,  200));
            frm_attack_hitbox->add("lbl_attack_hitbox_hazards", new lafi_label(  scr_w - 200, 208, scr_w - 8,  224, "Hazards:"));
            frm_attack_hitbox->add("txt_attack_hitbox_hazards", new lafi_textbox(scr_w - 200, 232, scr_w - 8,  248));
            frm_hitbox->add("frm_attack_hitbox", frm_attack_hitbox);
            
            lafi_frame* frm_shake_hitbox = new lafi_frame(scr_w - 208, 176, scr_w, scr_h - 48);
            frm_shake_hitbox->add("lbl_hitbox_shake_angle", new lafi_label(  scr_w - 200, 184, scr_w - 120, 200, "Angle:"));
            frm_shake_hitbox->add("txt_hitbox_shake_angle", new lafi_textbox(scr_w - 112, 184, scr_w - 8,   200));
            frm_hitbox->add("frm_shake_hitbox", frm_shake_hitbox);
            
            lafi_frame* frm_chomp_hitbox = new lafi_frame(scr_w - 208, 176, scr_w, scr_h - 48);
            frm_chomp_hitbox->add("chk_hitbox_swallow", new lafi_checkbox(scr_w - 200, 184, scr_w - 72, 200, "Swallowing"));
            frm_hitbox->add("frm_chomp_hitbox", frm_chomp_hitbox);
            
            frm_hitboxes->widgets["btn_new_hitbox"]->left_mouse_click_handler = [](lafi_widget*, int, int) {
                ed_mode = EDITOR_MODE_NEW_HITBOX;
            };
            frm_hitboxes->widgets["btn_delete_hitbox"]->left_mouse_click_handler = [](lafi_widget*, int, int) {
                if(ed_cur_hitbox_nr != string::npos) {
                    ed_anim->frames[ed_cur_frame_nr].hitboxes.erase(ed_anim->frames[ed_cur_frame_nr].hitboxes.begin() + ed_cur_hitbox_nr);
                    if(ed_cur_hitbox_nr >= ed_anim->frames[ed_cur_frame_nr].hitboxes.size()) ed_cur_hitbox_nr--;
                    if(ed_anim->frames[ed_cur_frame_nr].hitboxes.size() == 0) ed_cur_hitbox_nr = string::npos;
                    load_hitbox_fields();
                }
            };
            frm_hitboxes->widgets["btn_prev_hitbox"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                if(ed_cur_hitbox_nr != string::npos) {
                    if(ed_cur_hitbox_nr == 0) ed_cur_hitbox_nr = ed_anim->frames[ed_cur_frame_nr].hitboxes.size() - 1;
                    else ed_cur_hitbox_nr--;
                    load_hitbox_fields();
                }
            };
            frm_hitboxes->widgets["btn_next_hitbox"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                if(ed_cur_hitbox_nr != string::npos) {
                    ed_cur_hitbox_nr = (ed_cur_hitbox_nr + 1) % ed_anim->frames[ed_cur_frame_nr].hitboxes.size();
                    load_hitbox_fields();
                }
            };
            frm_hitboxes->widgets["btn_back"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                ed_mode = EDITOR_MODE_NORMAL;
                ed_gui->widgets["frm_hitboxes"]->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
                ed_gui->widgets["frm_main"]->flags = 0;
                ed_cur_hitbox_nr = string::npos;
            };
            
            frm_hitbox->widgets["txt_hitbox_name"]->lose_focus_handler = lambda_save_hitbox;
            frm_hitbox->widgets["txt_hitbox_x"]->lose_focus_handler = lambda_save_hitbox;
            frm_hitbox->widgets["txt_hitbox_y"]->lose_focus_handler = lambda_save_hitbox;
            frm_hitbox->widgets["txt_hitbox_z"]->lose_focus_handler = lambda_save_hitbox;
            frm_hitbox->widgets["txt_hitbox_r"]->lose_focus_handler = lambda_save_hitbox;
            frm_hitbox->widgets["rad_hitbox_normal"]->lose_focus_handler = lambda_save_hitbox;
            frm_hitbox->widgets["rad_hitbox_attack"]->lose_focus_handler = lambda_save_hitbox;
            frm_hitbox->widgets["rad_hitbox_shake"]->lose_focus_handler = lambda_save_hitbox;
            frm_hitbox->widgets["rad_hitbox_chomp"]->lose_focus_handler = lambda_save_hitbox;
            frm_hitbox->widgets["rad_hitbox_normal"]->left_mouse_click_handler = lambda_save_hitbox_click;
            frm_hitbox->widgets["rad_hitbox_attack"]->left_mouse_click_handler = lambda_save_hitbox_click;
            frm_hitbox->widgets["rad_hitbox_shake"]->left_mouse_click_handler =  lambda_save_hitbox_click;
            frm_hitbox->widgets["rad_hitbox_chomp"]->left_mouse_click_handler =  lambda_save_hitbox_click;
            frm_normal_hitbox->widgets["txt_hitbox_defense"]->lose_focus_handler = lambda_save_hitbox;
            frm_normal_hitbox->widgets["chk_hitbox_latch"]->lose_focus_handler = lambda_save_hitbox;
            frm_normal_hitbox->widgets["txt_normal_hitbox_hazards"]->lose_focus_handler = lambda_save_hitbox;
            frm_attack_hitbox->widgets["txt_hitbox_attack"]->lose_focus_handler = lambda_save_hitbox;
            frm_attack_hitbox->widgets["txt_attack_hitbox_hazards"]->lose_focus_handler = lambda_save_hitbox;
            frm_shake_hitbox->widgets["txt_hitbox_shake_angle"]->lose_focus_handler = lambda_save_hitbox;
            frm_chomp_hitbox->widgets["chk_hitbox_swallow"]->lose_focus_handler = lambda_save_hitbox;
            
            frm_hitboxes->add("frm_hitbox", frm_hitbox);
            ed_gui->add("frm_hitboxes", frm_hitboxes);
            
            
            //Bottom bar.
            lafi_frame* frm_bottom = new lafi_frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
            frm_bottom->add("btn_toggle_hitboxes", new lafi_button(scr_w - 200, scr_h - 40, scr_w - 168, scr_h - 8, "Hit", "Toggle hitbox visibility."));
            frm_bottom->add("btn_load", new lafi_button(           scr_w - 120, scr_h - 40, scr_w - 88,  scr_h - 8, "Load", "Load the object."));
            frm_bottom->add("btn_save", new lafi_button(           scr_w - 80,  scr_h - 40, scr_w - 48,  scr_h - 8, "Save", "Save the object."));
            frm_bottom->add("btn_quit", new lafi_button(           scr_w - 40,  scr_h - 40, scr_w - 8,   scr_h - 8, "X", "Quit."));
            
            frm_bottom->widgets["btn_toggle_hitboxes"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                ed_hitboxes_visible = !ed_hitboxes_visible;
            };
            frm_bottom->widgets["btn_load"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                load_animations();
                load_animation_fields();
                fill_choose_animation_frame();
            };
            frm_bottom->widgets["btn_save"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                data_node animations_node = data_node("animations", "");
                
                for(auto a = ed_anims.begin(); a != ed_anims.end(); a++) {
                
                    data_node* anim_node = new data_node(a->first, "");
                    animations_node.add(anim_node);
                    
                    data_node* frames_node = new data_node("frames", "");
                    anim_node->add(frames_node);
                    
                    size_t n_frames = a->second.frames.size();
                    for(size_t f = 0; f < n_frames; f++) {
                        frame* cur_frame = &a->second.frames[f];
                        data_node* frame_node = new data_node("frame", "");
                        frames_node->add(frame_node);
                        
                        frame_node->add(new data_node("file", cur_frame->file));
                        frame_node->add(new data_node("file_x", to_string((long long)     cur_frame->file_x)));
                        frame_node->add(new data_node("file_y", to_string((long long)     cur_frame->file_y)));
                        frame_node->add(new data_node("file_w", to_string((long long)     cur_frame->file_w)));
                        frame_node->add(new data_node("file_h", to_string((long long)     cur_frame->file_h)));
                        frame_node->add(new data_node("game_w", to_string((long double)   cur_frame->game_w)));
                        frame_node->add(new data_node("game_h", to_string((long double)   cur_frame->game_h)));
                        frame_node->add(new data_node("offset_x", to_string((long double) cur_frame->offs_x)));
                        frame_node->add(new data_node("offset_y", to_string((long double) cur_frame->offs_y)));
                        frame_node->add(new data_node("duration", to_string((long double) cur_frame->duration)));
                        
                        data_node* hitboxes_node = new data_node("hitboxes", "");
                        frame_node->add(hitboxes_node);
                        
                        size_t n_hitboxes = cur_frame->hitboxes.size();
                        for(size_t h = 0; h < n_hitboxes; h++) {
                            hitbox* cur_hitbox = &cur_frame->hitboxes[h];
                            data_node* hitbox_node = new data_node("hitbox", "");
                            hitboxes_node->add(hitbox_node);
                            
                            data_node* name_node = new data_node("name", cur_hitbox->name);
                            hitbox_node->add(name_node);
                            
                            data_node* type_node = new data_node("type", to_string((long long) cur_hitbox->type));
                            hitbox_node->add(type_node);
                            
                            data_node* coords_node = new data_node(
                                "coords",
                                to_string((long double) cur_hitbox->x) + " " + to_string((long double) cur_hitbox->y) + " " + to_string((long double) cur_hitbox->z)
                            );
                            hitbox_node->add(coords_node);
                            
                            data_node* radius_node = new data_node(
                                "radius",
                                to_string((long double) cur_hitbox->radius)
                            );
                            hitbox_node->add(radius_node);
                            
                            //ToDo hazards.
                            
                            data_node* multiplier_node = new data_node(
                                "multiplier",
                                to_string((long double) cur_hitbox->multiplier)
                            );
                            hitbox_node->add(multiplier_node);
                            
                            data_node* shake_angle_node = new data_node(
                                "shake_angle",
                                to_string((long double) cur_hitbox->shake_angle)
                            );
                            hitbox_node->add(shake_angle_node);
                            
                            data_node* latch_node = new data_node("can_pikmin_latch", btos(cur_hitbox->can_pikmin_latch));
                            hitbox_node->add(latch_node);
                            
                            data_node* swallow_node = new data_node("swallow", btos(cur_hitbox->swallow));
                            hitbox_node->add(swallow_node);
                        }
                    }
                    
                    data_node* loop_frame_node = new data_node("loop_frame", to_string((long long) ed_anim->loop_frame));
                    anim_node->add(loop_frame_node);
                }
                
                animations_node.save_file("Test.txt");
            };
            //ToDo quit button.
            
            ed_gui->add("frm_bottom", frm_bottom);
            
            ed_gui_status_bar = new lafi_label(0, scr_h - 16, scr_w - 208, scr_h);
            ed_gui->add("lbl_status_bar", ed_gui_status_bar);
            
            load_animation_fields();
            fill_choose_animation_frame();
            if(ed_anims.size() > 0) load_animation(ed_anims.begin()->first);
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
            handle_area_editor_controls(ev);
        } else if(cur_screen == SCREEN_ANIMATION_EDITOR) {
            handle_animation_editor_controls(ev);
        }
        
        if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
            //scr_w = ev.display.width;
            //scr_h = ev.display.height;
            
        } else if(ev.type == ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(queue)) {
        
            if(cur_screen == SCREEN_GAME) {
                do_logic();
                do_drawing();
            } else if(cur_screen == SCREEN_AREA_EDITOR) {
                do_area_editor_logic();
            } else if(cur_screen == SCREEN_ANIMATION_EDITOR) {
                do_animation_editor_logic();
            }
        }
    }
    
    //Quit.
    if(total_error_log.size()) {
        time_t tt;
        time(&tt);
        struct tm t = *localtime(&tt);
        total_error_log =
            to_string((long long) t.tm_year + 1900) + "/" +
            leading_zero(t.tm_mon + 1) + "/" +
            leading_zero(t.tm_mday) + " " +
            leading_zero(t.tm_hour) + ":" +
            leading_zero(t.tm_min) + ":" +
            leading_zero(t.tm_sec) +
            "\n" + total_error_log;
            
        string prev_error_log;
        string line;
        
        ifstream error_log_file_i("Error_log.txt");
        if(error_log_file_i.is_open()) {
            while(!error_log_file_i.eof()) {
                getline(error_log_file_i, line);
                prev_error_log += line + "\n";
            }
            prev_error_log.erase(prev_error_log.size() - 1);
            error_log_file_i.close();
        }
        
        ofstream error_log_file_o("Error_log.txt");
        if(error_log_file_o.is_open()) {
            error_log_file_o << total_error_log << "\n" << prev_error_log;
            error_log_file_o.close();
        }
        
    }
    
}