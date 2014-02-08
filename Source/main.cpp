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
    //create_mob(new pikmin(&pikmin_types[3], -50, -50, &s));
    //create_mob(new pikmin(&pikmin_types[4], -50, -70, &s));
    create_mob(new info_spot(300, 0, &s, "Treasure.", false, font));
    create_mob(new info_spot(400, 0, &s, "Onions.", false, font));
    create_mob(new info_spot(-300, 0, &s, "http://www.pikminfanon.com/\nTopic:Pikmin_Engine_by_Espyo", false, font));
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
    
    cur_screen = SCREEN_GAME;
    enemies.back()->anim = animation(new vector<frame>(1, frame(bmp_cloaking_burrow_nit, 64, 64, 30, load_hitboxes())));
    
    editor_cur_bmp = bmp_cloaking_burrow_nit;
    editor_mode = EDITOR_MODE_NORMAL;
    editor_cur_hitboxes = load_hitboxes();
    
    if(cur_screen == SCREEN_GAME) {
        al_hide_mouse_cursor(display);
    } else {
        al_show_mouse_cursor(display);
        if(cur_screen == SCREEN_ANIMATION_EDITOR) {
            auto lambda_update_hitbox_gui = [] (lafi_widget*, int, int) { update_hitbox_gui(); };
            auto lambda_save_hitbox = [] (lafi_widget*) { save_hitbox(); };
            
            editor_gui = new lafi_gui(scr_w, scr_h);
            lafi_frame* right_frame = new lafi_frame(scr_w - 208, 0, scr_w, scr_h);
            right_frame->add("btn_normal_mode", new lafi_button(scr_w - 200, 8, scr_w - 168, 40, "N", "Normal mode"));
            right_frame->widgets["btn_normal_mode"]->left_mouse_click_handler = [](lafi_widget*, int, int) {
                editor_mode = EDITOR_MODE_NORMAL;
            };
            right_frame->add("btn_new_hitbox_mode", new lafi_button(scr_w - 160, 8, scr_w - 128, 40, "+", "New hitbox mode"));
            right_frame->widgets["btn_new_hitbox_mode"]->left_mouse_click_handler = [](lafi_widget*, int, int) {
                editor_mode = EDITOR_MODE_NEW_HITBOX;
            };
            right_frame->add("btn_delete_hitbox_mode", new lafi_button(scr_w - 120, 8, scr_w - 88,  40, "-", "Delete hitbox mode"));
            right_frame->widgets["btn_delete_hitbox_mode"]->left_mouse_click_handler = [](lafi_widget*, int, int) {
                editor_mode = EDITOR_MODE_DELETE_HITBOX;
            };
            right_frame->add("btn_load", new lafi_button(scr_w - 80,  8, scr_w - 48,  40, "L", "Load hitboxes from the game files"));
            right_frame->add("btn_save", new lafi_button(scr_w - 40,  8, scr_w - 8,   40, "S", "Save hitboxes to the game files"));
            
            right_frame->add("lbl_hitbox_name", new lafi_label(scr_w - 200, 48, scr_w - 136, 60, "Name:"));
            right_frame->add("txt_hitbox_name", new lafi_textbox(scr_w - 128, 48, scr_w - 8, 64));
            right_frame->widgets["txt_hitbox_name"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("lbl_hitbox_xy", new lafi_label(scr_w - 200, 72, scr_w - 120, 88, "X, Y:"));
            right_frame->add("txt_hitbox_x", new lafi_textbox(scr_w - 112, 72, scr_w - 64, 88));
            right_frame->widgets["txt_hitbox_x"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("txt_hitbox_y", new lafi_textbox(scr_w - 56, 72, scr_w - 8, 88));
            right_frame->widgets["txt_hitbox_y"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("lbl_hitbox_zr", new lafi_label(scr_w - 200, 96, scr_w - 120, 112, "Z, radius:"));
            right_frame->add("txt_hitbox_z", new lafi_textbox(scr_w - 112, 96, scr_w - 64, 112));
            right_frame->widgets["txt_hitbox_z"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("txt_hitbox_r", new lafi_textbox(scr_w - 56, 96, scr_w - 8, 112));
            right_frame->widgets["txt_hitbox_r"]->lose_focus_handler = lambda_save_hitbox;
            
            right_frame->add("rad_hitbox_normal", new lafi_radio_button(scr_w - 200, 120, scr_w - 136, 136, "Normal"));
            right_frame->widgets["rad_hitbox_normal"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("rad_hitbox_attack", new lafi_radio_button(scr_w - 128, 120, scr_w - 8, 136, "Attack"));
            right_frame->widgets["rad_hitbox_attack"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("rad_hitbox_shake", new lafi_radio_button(scr_w - 200, 144, scr_w - 136, 160, "Shake"));
            right_frame->widgets["rad_hitbox_shake"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("rad_hitbox_chomp", new lafi_radio_button(scr_w - 128, 144, scr_w - 8, 160, "Chomp"));
            right_frame->widgets["rad_hitbox_chomp"]->lose_focus_handler = lambda_save_hitbox;
            
            lafi_frame* normal_hitbox_frame = new lafi_frame(scr_w - 208, 168, scr_w, scr_h);
            normal_hitbox_frame->add("lbl_hitbox_defense", new lafi_label(scr_w - 200, 176, scr_w - 72, 192, "Defense mult.:"));
            normal_hitbox_frame->add("txt_hitbox_defense", new lafi_textbox(scr_w - 64,  176, scr_w - 8,  192));
            normal_hitbox_frame->widgets["txt_hitbox_defense"]->lose_focus_handler = lambda_save_hitbox;
            normal_hitbox_frame->add("chk_hitbox_latch", new lafi_checkbox(scr_w - 200, 200, scr_w - 8, 216, "Pikmin can latch"));
            normal_hitbox_frame->widgets["chk_hitbox_latch"]->lose_focus_handler = lambda_save_hitbox;
            normal_hitbox_frame->add("lbl_normal_hitbox_hazards", new lafi_label(scr_w - 200, 224, scr_w - 8, 240, "Hazards:"));
            normal_hitbox_frame->add("txt_normal_hitbox_hazards", new lafi_textbox(scr_w - 200, 240, scr_w - 8, 256));
            normal_hitbox_frame->widgets["txt_normal_hitbox_hazards"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("frm_normal_hitbox", normal_hitbox_frame);
            
            lafi_frame* attack_hitbox_frame = new lafi_frame(scr_w - 208, 168, scr_w, scr_h);
            attack_hitbox_frame->add("lbl_hitbox_attack", new lafi_label(scr_w - 200, 176, scr_w - 72, 192, "Attack mult.:"));
            attack_hitbox_frame->add("txt_hitbox_attack", new lafi_textbox(scr_w - 64, 176, scr_w - 8, 192));
            attack_hitbox_frame->widgets["txt_hitbox_attack"]->lose_focus_handler = lambda_save_hitbox;
            attack_hitbox_frame->add("lbl_attack_hitbox_hazards", new lafi_label(scr_w - 200, 200, scr_w - 8, 216, "Hazards:"));
            attack_hitbox_frame->add("txt_attack_hitbox_hazards", new lafi_textbox(scr_w - 200, 224, scr_w - 8, 240));
            attack_hitbox_frame->widgets["txt_attack_hitbox_hazards"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("frm_attack_hitbox", attack_hitbox_frame);
            
            lafi_frame* shake_hitbox_frame = new lafi_frame(scr_w - 208, 168, scr_w, scr_h);
            shake_hitbox_frame->add("lbl_hitbox_shake_angle", new lafi_label(scr_w - 200, 176, scr_w - 120, 192, "Angle:"));
            shake_hitbox_frame->add("txt_hitbox_shake_angle", new lafi_textbox(scr_w - 112, 176, scr_w - 8, 192));
            shake_hitbox_frame->widgets["txt_hitbox_shake_angle"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("frm_shake_hitbox", shake_hitbox_frame);
            
            lafi_frame* chomp_hitbox_frame = new lafi_frame(scr_w - 208, 168, scr_w, scr_h);
            chomp_hitbox_frame->add("chk_hitbox_swallow", new lafi_checkbox(scr_w - 200, 176, scr_w - 72, 192, "Swallowing"));
            chomp_hitbox_frame->widgets["chk_hitbox_swallow"]->lose_focus_handler = lambda_save_hitbox;
            right_frame->add("frm_chomp_hitbox", chomp_hitbox_frame);
            
            right_frame->widgets["rad_hitbox_normal"]->left_mouse_click_handler = lambda_update_hitbox_gui;
            right_frame->widgets["rad_hitbox_attack"]->left_mouse_click_handler = lambda_update_hitbox_gui;
            right_frame->widgets["rad_hitbox_shake"]->left_mouse_click_handler = lambda_update_hitbox_gui;
            right_frame->widgets["rad_hitbox_chomp"]->left_mouse_click_handler = lambda_update_hitbox_gui;
            
            right_frame->widgets["btn_save"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                data_node hitboxes_node = data_node("hitboxes", "");
                
                for(size_t h = 0; h < editor_cur_hitboxes.size(); h++) {
                    hitbox* cur_hitbox = &editor_cur_hitboxes[h];
                    data_node* hitbox_node = new data_node("hitbox", "");
                    hitboxes_node.add(hitbox_node);
                    
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
                
                hitboxes_node.save_file("Test.txt");
            };
            
            right_frame->widgets["btn_load"]->left_mouse_click_handler = [] (lafi_widget*, int, int) {
                editor_cur_hitboxes = load_hitboxes();
            };
            
            editor_gui->add("frm_right", right_frame);
            editor_gui_status_bar = new lafi_label(0, scr_h - 16, scr_w - 208, scr_h);
            editor_gui->add("lbl_status_bar", editor_gui_status_bar);
            
            update_hitbox_gui();
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