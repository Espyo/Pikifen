//ToDo there are a lot of times where I do, for instance, leaders[current_leader]->, replace this with a pointer for perfomance.

#pragma warning(disable : 4996)	//Disables warning about localtime being deprecated.

#include <fstream>
#include <math.h>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"
#include "controls.h"
#include "drawing.h"
#include "functions.h"
#include "logic.h"
#include "vars.h"

using namespace std;

int main(){
	//Install Allegro and initialize modules.
	al_init();
	al_install_mouse();
	al_install_keyboard();
	al_install_audio();
	al_install_joystick();
	al_init_image_addon();
	al_init_primitives_addon();
	al_init_acodec_addon();

	//Event stuff.
	ALLEGRO_DISPLAY* display = al_create_display(600, 400);
	ALLEGRO_TIMER* timer = al_create_timer(1.0/30.0);

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
	al_hide_mouse_cursor(display);
	if(smooth_scaling) al_set_new_bitmap_flags(ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR | ALLEGRO_MIPMAP);
	al_reserve_samples(16);
	srand(time(NULL));

	//Error bitmap.
	//ToDo move this somewhere else, maybe?
	int bmp_error_w, bmp_error_h;
	al_get_text_dimensions(al_create_builtin_font(), "ERROR", NULL, NULL, &bmp_error_w, &bmp_error_h);
	bmp_error = al_create_bitmap(bmp_error_w, bmp_error_h);
	al_set_target_bitmap(bmp_error);{
		al_draw_text(al_create_builtin_font(), al_map_rgb(255, 0, 0), 0, 0, 0, "ERROR");
	}al_set_target_backbuffer(display);

	//Graphics.
	bmp_olimar = load_bmp("Olimar.png");
	bmp_louie = load_bmp("Louie.png");
	bmp_president = load_bmp("President.png");
	for(unsigned char m=0; m<3; m++){
		string maturity_str;
		if(m==0) maturity_str = "leaf";
		else if(m==1) maturity_str = "bud";
		else maturity_str = "flower";
		bmp_red[m] = load_bmp(("Red_" + maturity_str + ".png").c_str());
		bmp_yellow[m] = load_bmp(("Yellow_" + maturity_str + ".png").c_str());
		bmp_blue[m] = load_bmp(("Blue_" + maturity_str + ".png").c_str());
		bmp_red_burrowed[m] = load_bmp(("Red_burrowed_" + maturity_str + ".png").c_str());
		bmp_yellow_burrowed[m] = load_bmp(("Yellow_burrowed_" + maturity_str + ".png").c_str());
		bmp_blue_burrowed[m] = load_bmp(("Blue_burrowed_" + maturity_str + ".png").c_str());
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
	bmp_idle_glow = load_bmp("Idle_glow.png");
	bmp_ub_spray = load_bmp("Ultra-bitter_spray.png");
	bmp_us_spray = load_bmp("Ultra-spicy_spray.png");
	bmp_move_group_arrow = load_bmp("Move_group_arrow.png");

	int font_ranges[] = {
		0x0020, 0x007F, //ASCII
	};

	ALLEGRO_BITMAP* temp_font_bitmap = load_bmp("Font.png");  //We can't load the font directly because we want to set the ranges.
	if(temp_font_bitmap) font=al_grab_font_from_bitmap(temp_font_bitmap, 1, font_ranges);
	temp_font_bitmap = load_bmp("Area_name_font.png");
	if(temp_font_bitmap) font_area_name=al_grab_font_from_bitmap(temp_font_bitmap, 1, font_ranges);
	al_destroy_bitmap(temp_font_bitmap);
	font_h = al_get_font_line_height(font);

	//Sound effects.
	sfx_pikmin_held=load_sample("Pikmin_held.ogg");
	sfx_pikmin_thrown=load_sample("Pikmin_thrown.ogg");
	sfx_pikmin_plucked=load_sample("Pikmin_plucked.ogg");
	sfx_pikmin_called=load_sample("Pikmin_called.ogg");
	sfx_dismiss=load_sample("Dismiss.ogg");
	sfx_olimar_whistle=load_sample("Olimar_whistle.ogg");
	sfx_louie_whistle=load_sample("Louie_whistle.ogg");
	sfx_president_whistle=load_sample("President_whistle.ogg");
	sfx_olimar_name_call=load_sample("Olimar_name_call.ogg");
	sfx_louie_name_call=load_sample("Louie_name_call.ogg");
	sfx_president_name_call=load_sample("President_name_call.ogg");
	sfx_throw=load_sample("Throw.ogg");
	sfx_camera=load_sample("Camera.ogg");

	//Game content.
	load_game_content();

	//Initializing game things.
	sprays.clear();
	size_t n_spray_types = spray_types.size();
	for(size_t s=0; s<n_spray_types; s++){ sprays.push_back(0); }
	pikmin_in_onions.clear();
	size_t n_total_onions = pikmin_types.size();
	for(size_t o=0; o<n_total_onions; o++){ pikmin_in_onions.push_back(0); }
	size_t n_treasures = treasures.size();
	
	//Some temp variables.
	sector s = sector();
	leaders.push_back(new leader(0, 0, &s));
	leaders.back()->main_color = al_map_rgb(255, 0, 0);
	leaders.back()->health = 10;
	leaders.back()->sfx_dismiss = sfx_dismiss;
	leaders.back()->sfx_whistle = sfx_olimar_whistle;
	leaders.back()->sfx_name_call = sfx_olimar_name_call;
	leaders.push_back(new leader(300, 250, &s));
	leaders.back()->main_color = al_map_rgb(0, 0, 255);
	leaders.back()->health = 8;
	leaders.back()->sfx_dismiss = sfx_dismiss;
	leaders.back()->sfx_whistle = sfx_louie_whistle;
	leaders.back()->sfx_name_call = sfx_louie_name_call;
	leaders.push_back(new leader(350, 200, &s));
	leaders.back()->main_color = al_map_rgb(0, 0, 255);
	leaders.back()->health = 6;
	leaders.back()->sfx_dismiss = sfx_dismiss;
	leaders.back()->sfx_whistle = sfx_president_whistle;
	leaders.back()->sfx_name_call = sfx_president_name_call;
	treasures.push_back(new treasure(300, 100, 40, &s, 30, 50));
	pikmin_list.push_back(new pikmin(&pikmin_types[0], 30, 30, &s));
	pikmin_list.back()->maturity = 1;
	pikmin_list.push_back(new pikmin(&pikmin_types[0], 40, 30, &s));
	pikmin_list.back()->maturity = 2;
	pikmin_list.push_back(new pikmin(&pikmin_types[1], 50, 30, &s));
	pikmin_list.back()->maturity = 1;
	pikmin_list.push_back(new pikmin(&pikmin_types[1], 60, 30, &s));
	pikmin_list.back()->maturity = 2;
	pikmin_list.push_back(new pikmin(&pikmin_types[2], 70, 30, &s));
	pikmin_list.back()->maturity = 1;
	pikmin_list.push_back(new pikmin(&pikmin_types[2], 80, 30, &s));
	pikmin_list.back()->maturity = 2;
	pikmin_list.push_back(new pikmin(&pikmin_types[0], 30, 200, &s));
	pikmin_list.back()->burrowed=true;
	pikmin_list.push_back(new pikmin(&pikmin_types[1], 50, 200, &s));
	pikmin_list.back()->burrowed=true;
	pikmin_list.push_back(new pikmin(&pikmin_types[2], 70, 200, &s));
	pikmin_list.back()->burrowed=true;
	for(unsigned char p=0; p<10; p++){
		for(unsigned char t=0; t<3; t++){
			pikmin_list.push_back(new pikmin(&pikmin_types[t], 100 + 10*p + 3*t, 30, &s));
		}
	}
	onions.push_back(new onion(400, 100, &s, &pikmin_types[0]));
	onions.push_back(new onion(400, 200, &s, &pikmin_types[1]));
	onions.push_back(new onion(400, 300, &s, &pikmin_types[2]));
	info_spots.push_back(new info_spot(300, 0, &s, "Treasure.", false, font));
	info_spots.push_back(new info_spot(400, 0, &s, "Onions.", false, font));
	nectars.push_back(new nectar(0, 400, &s));
	sprays[0] = sprays[1] = 10;
	spray_types[0].bmp_spray = bmp_ub_spray;
	spray_types[1].bmp_spray = bmp_us_spray;
	pikmin_in_onions[0] = 200;
	pikmin_in_onions[1] = 180;
	pikmin_in_onions[2] = 160;

	//Main loop.
	al_start_timer(timer);
	while(running){

		/*  ************************************************
		  *** | _ |                                  | _ | ***
		*****  \_/           EVENT HANDLING           \_/  *****
		  *** +---+                                  +---+ ***
		    ************************************************/

		al_wait_for_event(queue, &ev);

		handle_controls(ev);

		if(ev.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
			running=false;

		}else if(ev.type==ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(queue)){
			
			do_logic();
			do_drawing();
		}
	}

	//Quit.
	if(total_error_log.size()){
		time_t tt;
		time(&tt);
		struct tm t=*localtime(&tt);
		total_error_log =
			to_string((long long) t.tm_year + 1900) + "/" +
			to_string((long long) t.tm_mon + 1) + "/" +
			to_string((long long) t.tm_mday) + " " +
			to_string((long long) t.tm_hour) + ":" +
			to_string((long long) t.tm_min) + ":" +
			to_string((long long) t.tm_sec) +
			"\n" + total_error_log;
		
		string prev_error_log;
		string line;

		ifstream error_log_file_i("Error_log.txt");
		if(error_log_file_i.is_open()){
			while(!error_log_file_i.eof()){
				getline(error_log_file_i, line);
				prev_error_log += line + "\n";
			}
			prev_error_log.erase(prev_error_log.size()-1);
			error_log_file_i.close();
		}

		ofstream error_log_file_o("Error_log.txt");
		if(error_log_file_o.is_open()){
			error_log_file_o << total_error_log << "\n" << prev_error_log;
			error_log_file_o.close();
		}

	}
}
