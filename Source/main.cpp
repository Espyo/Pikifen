//ToDo there are a lot of times where I do, for instance, leaders[current_leader]., replace this with a pointer for perfomance.

#include <math.h>

#include <allegro5\allegro.h>
#include <allegro5\allegro_audio.h>
#include <allegro5\allegro_acodec.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_image.h>
#include <allegro5\allegro_primitives.h>

#include "const.h"
#include "controls.h"
#include "functions.h"
#include "vars.h"

int main(){
	//Configurations outside of Allegro.
	srand(time(NULL));

	//Install Allegro and initialize modules.
	al_init();
	al_install_mouse();
	al_install_keyboard();
	al_install_audio();
	al_init_image_addon();
	al_init_primitives_addon();
	al_init_acodec_addon();

	//Event stuff.
	ALLEGRO_DISPLAY* display = al_create_display(600, 400);
	ALLEGRO_TIMER* timer = al_create_timer(1.0/30.0);

	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));
	ALLEGRO_EVENT ev;

	//Other initial things.
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
	al_set_window_title(display, "Pikmin fangame engine");
	al_hide_mouse_cursor(display);
	if(smooth_scaling) al_set_new_bitmap_flags(ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR | ALLEGRO_MIPMAP);
	al_reserve_samples(16);

	//Graphics.
	bmp_olimar = al_load_bitmap("Olimar.png");
	bmp_louie = al_load_bitmap("Louie.png");
	bmp_president = al_load_bitmap("President.png");
	for(unsigned char m=0; m<3; m++){
		string maturity_str;
		if(m==0) maturity_str = "leaf";
		else if(m==1) maturity_str = "bud";
		else maturity_str = "flower";
		bmp_red[m] = al_load_bitmap(("Red_" + maturity_str + ".png").c_str());
		bmp_yellow[m] = al_load_bitmap(("Yellow_" + maturity_str + ".png").c_str());
		bmp_blue[m] = al_load_bitmap(("Blue_" + maturity_str + ".png").c_str());
		bmp_red_burrowed[m] = al_load_bitmap(("Red_burrowed_" + maturity_str + ".png").c_str());
		bmp_yellow_burrowed[m] = al_load_bitmap(("Yellow_burrowed_" + maturity_str + ".png").c_str());
		bmp_blue_burrowed[m] = al_load_bitmap(("Blue_burrowed_" + maturity_str + ".png").c_str());
		bmp_red_idle[m] = al_load_bitmap(("Red_idle_" + maturity_str + ".png").c_str());
		bmp_yellow_idle[m] = al_load_bitmap(("Yellow_idle_" + maturity_str + ".png").c_str());
		bmp_blue_idle[m] = al_load_bitmap(("Blue_idle_" + maturity_str + ".png").c_str());
	}
	bmp_red_onion = al_load_bitmap("Red_onion.png");
	bmp_yellow_onion = al_load_bitmap("Yellow_onion.png");
	bmp_blue_onion = al_load_bitmap("Blue_onion.png");
	bmp_cursor = al_load_bitmap("Cursor.png");
	bmp_mouse_cursor = al_load_bitmap("Mouse_cursor.png");
	bmp_background = al_load_bitmap("Background.png");
	bmp_bubble = al_load_bitmap("Bubble.png");
	bmp_day_bubble = al_load_bitmap("Day_bubble.png");
	bmp_health_bubble = al_load_bitmap("Health_bubble.png");
	bmp_sun = al_load_bitmap("Sun.png");
	bmp_shadow = al_load_bitmap("Shadow.png");
	bmp_idle_glow = al_load_bitmap("Idle_glow.png");
	bmp_ub_spray = al_load_bitmap("Ultra-bitter_spray.png");
	bmp_us_spray = al_load_bitmap("Ultra-spicy_spray.png");
	bmp_move_group_arrow = al_load_bitmap("Move_group_arrow.png");

	int font_ranges[] = {
		0x0020, 0x007F, //ASCII
		0x00A1, 0x00FF, //Latin 1
		0x0100, 0x017F, //Extended-A
		0x20AC, 0x20AC, //Euro
	};

	int font_area_name_ranges[] = {
		0x0020, 0x007F, //ASCII
	};

	ALLEGRO_BITMAP* temp_font_bitmap = al_load_bitmap("Font.png");  //We can't load the font directly because we want to set the ranges.
	if(temp_font_bitmap) font=al_grab_font_from_bitmap(temp_font_bitmap, 4, font_ranges);
	temp_font_bitmap = al_load_bitmap("Area_name_font.png");
	if(temp_font_bitmap) font_area_name=al_grab_font_from_bitmap(temp_font_bitmap, 1, font_area_name_ranges);
	al_destroy_bitmap(temp_font_bitmap);
	font_h = al_get_font_line_height(font);

	//Sound effects.
	sfx_pikmin_held.sample=al_load_sample("Pikmin_held.ogg");
	sfx_pikmin_thrown.sample=al_load_sample("Pikmin_thrown.ogg");
	sfx_pikmin_plucked.sample=al_load_sample("Pikmin_plucked.ogg");
	sfx_pikmin_called.sample = al_load_sample("Pikmin_called.ogg");
	sfx_dismiss.sample = al_load_sample("Dismiss.ogg");
	sfx_olimar_whistle.sample=al_load_sample("Olimar_whistle.ogg");
	sfx_louie_whistle.sample = al_load_sample("Louie_whistle.ogg");
	sfx_president_whistle.sample = al_load_sample("President_whistle.ogg");
	sfx_olimar_name_call.sample = al_load_sample("Olimar_name_call.ogg");
	sfx_louie_name_call.sample = al_load_sample("Louie_name_call.ogg");
	sfx_president_name_call.sample = al_load_sample("President_name_call.ogg");
	sfx_throw.sample = al_load_sample("Throw.ogg");
	sfx_camera.sample = al_load_sample("Camera.ogg");

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
	
	//Some variables.
	sector s = sector();
	leaders.push_back(leader(0, 0, &s));
	leaders.back().main_color = al_map_rgb(255, 0, 0);
	leaders.back().health = 10;
	leaders.back().sfx_dismiss = sfx_dismiss;
	leaders.back().sfx_whistle = sfx_olimar_whistle;
	leaders.back().sfx_name_call = sfx_olimar_name_call;
	leaders.push_back(leader(300, 250, &s));
	leaders.back().main_color = al_map_rgb(0, 0, 255);
	leaders.back().health = 8;
	leaders.back().sfx_dismiss = sfx_dismiss;
	leaders.back().sfx_whistle = sfx_louie_whistle;
	leaders.back().sfx_name_call = sfx_louie_name_call;
	leaders.push_back(leader(350, 200, &s));
	leaders.back().main_color = al_map_rgb(0, 0, 255);
	leaders.back().health = 6;
	leaders.back().sfx_dismiss = sfx_dismiss;
	leaders.back().sfx_whistle = sfx_president_whistle;
	leaders.back().sfx_name_call = sfx_president_name_call;
	treasures.push_back(treasure(300, 100, 0, 40, 10, &s, 30, 50));
	pikmin_list.push_back(pikmin(&pikmin_types[0], 30, 30, &s));
	pikmin_list.back().maturity = 1;
	pikmin_list.push_back(pikmin(&pikmin_types[0], 40, 30, &s));
	pikmin_list.back().maturity = 2;
	pikmin_list.push_back(pikmin(&pikmin_types[1], 50, 30, &s));
	pikmin_list.back().maturity = 1;
	pikmin_list.push_back(pikmin(&pikmin_types[1], 60, 30, &s));
	pikmin_list.back().maturity = 2;
	pikmin_list.push_back(pikmin(&pikmin_types[2], 70, 30, &s));
	pikmin_list.back().maturity = 1;
	pikmin_list.push_back(pikmin(&pikmin_types[2], 80, 30, &s));
	pikmin_list.back().maturity = 2;
	pikmin_list.push_back(pikmin(&pikmin_types[0], 30, 200, &s));
	pikmin_list.back().burrowed=true;
	pikmin_list.push_back(pikmin(&pikmin_types[1], 50, 200, &s));
	pikmin_list.back().burrowed=true;
	pikmin_list.push_back(pikmin(&pikmin_types[2], 70, 200, &s));
	pikmin_list.back().burrowed=true;
	for(unsigned char p=0; p<10; p++){
		for(unsigned char t=0; t<3; t++){
			pikmin_list.push_back(pikmin(&pikmin_types[t], 100 + 10*p + 3*t, 30, &s));
		}
	}
	onions.push_back(onion(400, 100, &s, &pikmin_types[0]));
	onions.push_back(onion(400, 200, &s, &pikmin_types[1]));
	onions.push_back(onion(400, 300, &s, &pikmin_types[2]));
	info_spots.push_back(info_spot(300, 0, &s, "Treasure.", false, font));
	info_spots.push_back(info_spot(400, 0, &s, "Onions.", false, font));
	nectars.push_back(nectar(0, 400, &s));
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
			
			/*  ********************************************
			  ***  .-.                                .-.  ***
			***** ( L )          MAIN LOGIC          ( L ) *****
			  ***  `-´                                `-´  ***
				********************************************/
			
			/*************************
			*                   .-.  *
			*   Timer things   ( L ) *
			*                   `-´  *
			*************************/

			day_minutes += (1.0f / game_fps) * day_minutes_per_irl_sec;
			if(day_minutes > 60 * 24) day_minutes -= 60 * 24;

			idle_glow_angle+=(1.0 / game_fps) * (IDLE_GLOW_SPIN_SPEED);

			if(cam_trans_pan_time_left > 0){
				cam_trans_pan_time_left -= 1.0/game_fps;
				if(cam_trans_pan_time_left < 0) cam_trans_pan_time_left = 0;

				float amount_left = cam_trans_pan_time_left / CAM_TRANSITION_DURATION;

				cam_x = cam_trans_pan_initi_x + (cam_trans_pan_final_x - cam_trans_pan_initi_x) * (1-amount_left);
				cam_y = cam_trans_pan_initi_y + (cam_trans_pan_final_y - cam_trans_pan_initi_y) * (1-amount_left);
			}

			if(cam_trans_zoom_time_left > 0){
				cam_trans_zoom_time_left -= 1.0/game_fps;
				if(cam_trans_zoom_time_left < 0) cam_trans_zoom_time_left = 0;

				float amount_left = cam_trans_zoom_time_left / CAM_TRANSITION_DURATION;

				cam_zoom = cam_trans_zoom_initi_level + (cam_trans_zoom_final_level - cam_trans_zoom_initi_level) * (1-amount_left);
			}

			//"Move group" arrows.
			if(moving_group){
				move_group_next_arrow_time-=1.0/game_fps;
				if(move_group_next_arrow_time<=0){
					move_group_next_arrow_time = MOVE_GROUP_ARROWS_INTERVAL;
					move_group_arrows.push_back(0);
				}
			}

			float d = dist(leaders[current_leader].x, leaders[current_leader].y, cursor_x, cursor_y) * 0.5;
			for(size_t a=0; a<move_group_arrows.size(); ){
				move_group_arrows[a]+=MOVE_GROUP_ARROW_SPEED * (1.0/game_fps);
				if(move_group_arrows[a] >= d){
					move_group_arrows.erase(move_group_arrows.begin() + a);
				}else{
					a++;
				}
			}

			//Whistle rings.
			if(whistling){
				whistle_next_ring_time-=1.0/game_fps;
				if(whistle_next_ring_time<=0){
					whistle_next_ring_time = WHISTLE_RINGS_INTERVAL;
					whistle_rings.push_back(0);
					whistle_ring_colors.push_back(whistle_ring_prev_color);
					whistle_ring_prev_color = (whistle_ring_prev_color + 1) % N_WHISTLE_RING_COLORS;
				}
			}

			d *= 2;
			for(size_t r=0; r<whistle_rings.size(); ){
				whistle_rings[r]+=WHISTLE_RING_SPEED * (1.0/game_fps);
				if(whistle_rings[r] >= d){
					whistle_rings.erase(whistle_rings.begin() + r);
					whistle_ring_colors.erase(whistle_ring_colors.begin() + r);
				}else{
					r++;
				}
			}
			
			/********************
			*              ***  *
			*   Whistle   * O * *
			*              ***  *
			********************/

			if(whistling && whistle_radius < MAX_WHISTLE_RADIUS){
				whistle_radius += (1.0 / game_fps) * WHISTLE_RADIUS_GROWTH_PS;
				if(whistle_radius > MAX_WHISTLE_RADIUS){
					whistle_radius = MAX_WHISTLE_RADIUS;
					whistle_max_hold = WHISTLE_MAX_HOLD_TIME;
				}
			}

			if(whistle_max_hold > 0){
				whistle_max_hold -= 1.0/game_fps;
				if(whistle_max_hold <= 0){
					stop_whistling();
				}
			}


			/******************
			*             /\  *
			*   Pikmin   (@:) *
			*             \/  *
			******************/

			size_t n_treasures = treasures.size();
			size_t n_pikmin = pikmin_list.size();
			for(size_t p=0; p<n_pikmin; p++){
				pikmin* pik_ptr = &pikmin_list[p];

				bool can_be_called = 
					!pik_ptr->following_party &&
					!pik_ptr->burrowed &&
					!pik_ptr->speed_z &&
					!pik_ptr->uncallable_period;
				bool whistled = (dist(pik_ptr->x, pik_ptr->y, cursor_x, cursor_y) <= whistle_radius && whistling);
				bool touched = dist(pik_ptr->x, pik_ptr->y, leaders[current_leader].x, leaders[current_leader].y) <= pik_ptr->size * 0.5 + leaders[current_leader].size * 0.5;
				bool is_busy = (pik_ptr->carrying_treasure || pik_ptr->enemy_attacking);

				if(can_be_called && (whistled || (touched && !is_busy))){

					//Pikmin got whistled or touched.
					add_to_party(&leaders[current_leader], pik_ptr);
					al_stop_sample(&sfx_pikmin_called.id);
					al_play_sample(sfx_pikmin_called.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &sfx_pikmin_called.id);

					pik_ptr->enemy_attacking = NULL;

					//ToDo optimize this instead of running through the spot vector.
					if(pik_ptr->carrying_treasure){
						for(size_t s=0; s<pik_ptr->carrying_treasure->max_carriers; s++){
							if(pik_ptr->carrying_treasure->carrier_info->carrier_spots[s] == pik_ptr){
								pik_ptr->carrying_treasure->carrier_info->carrier_spots[s] = NULL;
								pik_ptr->carrying_treasure->carrier_info->current_n_carriers--;
							}
						}
					}

					pik_ptr->carrying_treasure = NULL;
							
				}

				//Following party.
				if(pik_ptr->following_party){
					float move_x, move_y;
					angle_to_coordinates(group_move_angle, group_move_intensity * CURSOR_MAX_DIST * 0.5, &move_x, &move_y);

					pik_ptr->target_x = pik_ptr->following_party->x + random(0, 60) - 30 + move_x;
					pik_ptr->target_y = pik_ptr->following_party->y + random(0, 30) + move_y;
				}

				//Touching nectar.
				size_t n_nectars = nectars.size();
				if(
					!pik_ptr->carrying_treasure &&
					!pik_ptr->enemy_attacking &&
					!pik_ptr->burrowed &&
					!pik_ptr->speed_z &&
					pik_ptr->maturity != 2
					){
						for(size_t n=0; n<n_nectars; n++){
							if(dist(pik_ptr->x, pik_ptr->y, nectars[n].x, nectars[n].y) <= nectars[n].size + pik_ptr->size){
								if(nectars[n].amount_left > 0)
									nectars[n].amount_left--;

								pik_ptr->maturity = 2;
							}
						}
				}

				//Finding tasks.
				if(
					(!pik_ptr->following_party &&
					!pik_ptr->carrying_treasure &&
					!pik_ptr->enemy_attacking &&
					!pik_ptr->burrowed &&
					!pik_ptr->speed_z) ||
					(pik_ptr->following_party && moving_group)
					){
						for(size_t t=0; t<n_treasures; t++){
							if(dist(pik_ptr->x, pik_ptr->y, treasures[t].x, treasures[t].y)<=pik_ptr->size + treasures[t].size + MIN_PIKMIN_TASK_RANGE){
								//ToDo don't take the treasure if all spots are taken already.
								pik_ptr->carrying_treasure = &treasures[t];
								
								if(pik_ptr->following_party) remove_from_party(pik_ptr->following_party, pik_ptr);

								//ToDo remove this random cycle and replace with something more optimal.
								bool valid_spot = false;
								unsigned int spot = 0;
								while(!valid_spot){
									spot = random(0, treasures[t].max_carriers - 1);
									valid_spot = !treasures[t].carrier_info->carrier_spots[spot];
								}
								
								treasures[t].carrier_info->carrier_spots[spot] = pik_ptr;
								treasures[t].carrier_info->current_n_carriers++;

								pik_ptr->carrying_spot = spot;
								pik_ptr->go_to_target = true;

								break;
							}
						}
				}

				if(pik_ptr->carrying_treasure){
					pik_ptr->target_x = pik_ptr->carrying_treasure->x + pik_ptr->carrying_treasure->carrier_info->carrier_spots_x[pik_ptr->carrying_spot];
					pik_ptr->target_y = pik_ptr->carrying_treasure->y + pik_ptr->carrying_treasure->carrier_info->carrier_spots_y[pik_ptr->carrying_spot];
				}
				
				pik_ptr->tick();

			}


			/********************
			*              .-.  *
			*   Leaders   (*:O) *
			*              `-´  *
			********************/

			if(leaders[current_leader].holding_pikmin){
				leaders[current_leader].holding_pikmin->x = leaders[current_leader].x+8;
				leaders[current_leader].holding_pikmin->y = leaders[current_leader].y;
			}

			size_t n_leaders = leaders.size();
			for(size_t l=0; l<n_leaders; l++){
				if(whistling){
					if(l != current_leader){
						if(
							dist(leaders[l].x, leaders[l].y, cursor_x, cursor_y) <= whistle_radius &&
							!leaders[l].following_party &&
							!leaders[l].was_thrown){
							//Leader got whistled.
							add_to_party(&leaders[current_leader], &leaders[l]);

							size_t n_party_members = leaders[l].party.size();
							for(size_t m=0; m<n_party_members; m++){
								mob* member = leaders[l].party[0];
								remove_from_party(&leaders[l], member);
								add_to_party(&leaders[current_leader], member);
							}
						}
					}
				}

				if(leaders[l].following_party){
					leaders[l].target_x = leaders[l].following_party->x;
					leaders[l].target_y = leaders[l].following_party->y + 30;
				}
				
				leaders[l].tick();
			}

			if(cam_trans_pan_time_left > 0){
				cam_trans_pan_final_x = leaders[current_leader].x;
				cam_trans_pan_final_y = leaders[current_leader].y;
			}else{
				cam_x = leaders[current_leader].x;
				cam_y = leaders[current_leader].y;
			}

			/******************
			*            ***  *
			*   Group   ****O *
			*            ***  *
			******************/

			float closest_distance = 0;
			size_t n_members = leaders[current_leader].party.size();
			closest_party_member = leaders[current_leader].holding_pikmin;

			if(n_members > 0 && !closest_party_member){

				for(size_t m=0; m<n_members; m++){
					mob* ccc=leaders[current_leader].party[m];
					float xxx=leaders[current_leader].party[m]->x;
					float yyy=leaders[current_leader].party[m]->y;
					float d = dist(leaders[current_leader].x, leaders[current_leader].y, leaders[current_leader].party[m]->x, leaders[current_leader].party[m]->y);
					if(m==0 || d < closest_distance){
						closest_distance = d;
						closest_party_member = leaders[current_leader].party[m];
					}
				}

				if(closest_distance > MIN_PIKMIN_GRABBING_RANGE){
					closest_party_member = NULL;
				}
			}


			/********************
			*             .-.   *
			*   Cursor   ( = )> *
			*             `-´   *
			********************/

			float mcx = mouse_cursor_x, mcy = mouse_cursor_y;
			ALLEGRO_TRANSFORM screen_to_world_transform = get_world_to_screen_transform();
			al_invert_transform(&screen_to_world_transform);
			al_transform_coordinates(&screen_to_world_transform, &mcx, &mcy);
			cursor_x = mcx;
			cursor_y = mcy;

			leaders[current_leader].angle = atan2(cursor_y - leaders[current_leader].y, cursor_x - leaders[current_leader].x);
			d = dist(leaders[current_leader].x, leaders[current_leader].y, cursor_x, cursor_y);
			if(d > CURSOR_MAX_DIST){
				//Cursor goes beyond the range limit.
				cursor_x = leaders[current_leader].x + (cos(leaders[current_leader].angle) * CURSOR_MAX_DIST);
				cursor_y = leaders[current_leader].y + (sin(leaders[current_leader].angle) * CURSOR_MAX_DIST);
			}

			if(moving_group){
				group_move_angle = leaders[current_leader].angle;
				group_move_intensity = d / CURSOR_MAX_DIST;
				if(group_move_intensity > 1) group_move_intensity = 1;
			}

			/**********************
			*                 *   *
			*   Particles   *   * *
			*                ***  *
			**********************/

			//Tick all particles.
			size_t n_particles = particles.size();
			for(size_t p=0; p<n_particles; ){
				if(!particles[p].tick()){
					particles.erase(particles.begin() + p);
					n_particles--;
				}else{
					p++;
				}
			}

			//ToDo the particles can't be created once per frame! That's overkill! ...right?
			for(size_t l = 0; l < n_leaders; l++){
				if(leaders[l].was_thrown)
					random_particle_fire(leaders[l].x, leaders[l].y, 1, 1, 0.3, 0.5, 3, 4, change_alpha(leaders[l].main_color, 192));
			}

			for(size_t p = 0; p < n_pikmin; p++){
				if(pikmin_list[p].was_thrown)
					random_particle_fire(pikmin_list[p].x, pikmin_list[p].y, 1, 1, 0.3, 0.5, 3, 4, change_alpha(pikmin_list[p].main_color, 192));
			}




			/*  ***************************************
			  *** |  |                           |  | ***
			***** |__|          DRAWING          |__| *****
			  ***  \/                             \/  ***
				***************************************/

			al_clear_to_color(al_map_rgb(0,0,0));

			ALLEGRO_TRANSFORM normal_transform;
			al_identity_transform(&normal_transform);
			
			ALLEGRO_TRANSFORM world_to_screen_transform = get_world_to_screen_transform();
			al_use_transform(&world_to_screen_transform);

			/* Layer 1
			*************************
			*                ^^^^^^ *
			*   Background   ^^^^^^ *
			*                ^^^^^^ *
			************************/

			al_draw_scaled_bitmap(bmp_background, 0, 0, 512, 512, scr_w/2 - 512, scr_h/2 - 512, 512, 512, 0);
			al_draw_scaled_bitmap(bmp_background, 0, 0, 512, 512, scr_w/2 - 512, scr_h/2, 512, 512, 0);
			al_draw_scaled_bitmap(bmp_background, 0, 0, 512, 512, scr_w/2, scr_h/2 - 512, 512, 512, 0);
			al_draw_scaled_bitmap(bmp_background, 0, 0, 512, 512, scr_w/2, scr_h/2, 512, 512, 0);


			/* Layer 2
			************************
			*                  ##  *
			*   Mob shadows   #### *
			*                  ##  *
			***********************/

			float shadow_stretch = 0;
			
			if(day_minutes < 60*5 || day_minutes > 60*20){
				shadow_stretch = 1;
			}else if(day_minutes < 60*12){
				shadow_stretch = 1-((day_minutes - 60*5) / (60*12 - 60*5));
			}else{
				shadow_stretch = (day_minutes - 60*12) / (60*20 - 60*12);
			}

			n_leaders = leaders.size();
			for(size_t l = 0; l < n_leaders; l++){
				draw_shadow(leaders[l].x, leaders[l].y, 32, leaders[l].z - leaders[l].sec->floors[0].z, shadow_stretch);
			}

			n_pikmin = pikmin_list.size();
			for(size_t p = 0; p < n_pikmin; p++){
				draw_shadow(pikmin_list[p].x, pikmin_list[p].y, 18, pikmin_list[p].z - pikmin_list[p].sec->floors[0].z, shadow_stretch);
			}


			/* Layer 3
			*********************
			*             .-.   *
			*   Cursor   ( = )> *
			*             `-´   *
			********************/

			size_t n_arrows = move_group_arrows.size();
			for(size_t a=0; a<n_arrows; a++){
				float x = cos(leaders[current_leader].angle) * move_group_arrows[a];
				float y = sin(leaders[current_leader].angle) * move_group_arrows[a];
				al_draw_scaled_rotated_bitmap(
					bmp_move_group_arrow,
					16, 26,
					leaders[current_leader].x + x,
					leaders[current_leader].y + y,
					0.5, 0.5,
					leaders[current_leader].angle,
					0);
			}

			size_t n_rings = whistle_rings.size();
			for(size_t r=0; r<n_rings; r++){
				float x = leaders[current_leader].x + cos(leaders[current_leader].angle) * whistle_rings[r];
				float y = leaders[current_leader].y + sin(leaders[current_leader].angle) * whistle_rings[r];
				unsigned char n = whistle_ring_colors[r];
				al_draw_circle(x, y, 12, al_map_rgb(WHISTLE_RING_COLORS[n][1], WHISTLE_RING_COLORS[n][2], WHISTLE_RING_COLORS[n][3]), 1);
			}

			al_use_transform(&normal_transform);
			al_draw_scaled_rotated_bitmap(bmp_mouse_cursor, 48, 48, mouse_cursor_x, mouse_cursor_y, 0.5 * cam_zoom, 0.5 * cam_zoom, leaders[current_leader].angle, 0);
			al_use_transform(&world_to_screen_transform);
			al_draw_scaled_rotated_bitmap(bmp_cursor, 48, 48, cursor_x, cursor_y, 0.5, 0.5, leaders[current_leader].angle, 0);

			
			/* Layer 4
			****************
			*          \o/ *
			*   Mobs    |  *
			*          / \ *
			***************/

			//Nectar.
			size_t n_nectars = nectars.size();
			for(size_t n=0; n<n_nectars; n++){
				al_draw_filled_circle(
					nectars[n].x,
					nectars[n].y,
					nectars[n].size * (nectars[n].amount_left + NECTAR_AMOUNT) / (NECTAR_AMOUNT * 2),
					al_map_rgb(255, 255, 0)
					);
			}

			//Treasures.
			for(size_t t=0; t<n_treasures; t++){
				al_draw_filled_circle(treasures[t].x, treasures[t].y, treasures[t].size, al_map_rgb(128, 255, 255));
			}

			//Pikmin
			n_pikmin = pikmin_list.size();
			for(size_t p = 0; p<n_pikmin; p++){
				ALLEGRO_BITMAP* bm;

				bool idling = !pikmin_list[p].following_party && !pikmin_list[p].carrying_treasure;

				if(pikmin_list[p].type->name=="R"){
					if(pikmin_list[p].burrowed) bm=bmp_red_burrowed[pikmin_list[p].maturity];
					else if(idling) bm=bmp_red_idle[pikmin_list[p].maturity];
					else bm=bmp_red[pikmin_list[p].maturity];
				}else if(pikmin_list[p].type->name=="Y"){
					if(pikmin_list[p].burrowed) bm=bmp_yellow_burrowed[pikmin_list[p].maturity];
					else if(idling) bm=bmp_yellow_idle[pikmin_list[p].maturity];
					else bm=bmp_yellow[pikmin_list[p].maturity];
				}if(pikmin_list[p].type->name=="B"){
					if(pikmin_list[p].burrowed) bm=bmp_blue_burrowed[pikmin_list[p].maturity];
					else if(idling) bm=bmp_blue_idle[pikmin_list[p].maturity];
					else bm=bmp_blue[pikmin_list[p].maturity];
				}
				al_draw_scaled_rotated_bitmap(bm, 18, 18, pikmin_list[p].x, pikmin_list[p].y, 0.5, 0.5, pikmin_list[p].angle, 0); //ToDo actual coordinates

				if(!pikmin_list[p].following_party){
					al_draw_tinted_scaled_rotated_bitmap(
						bmp_idle_glow,
						change_alpha(pikmin_list[p].main_color, 128),
						18,
						18,
						pikmin_list[p].x,
						pikmin_list[p].y,
						0.5,
						0.5,
						idle_glow_angle,
						0
						);
					//ToDo actual coordinates.
				}
			}

			//Leaders.
			for(size_t l=0; l<n_leaders; l++){
				ALLEGRO_BITMAP* bm = (l==0) ? bmp_olimar : ((l==1) ? bmp_louie : bmp_president);
				al_draw_scaled_rotated_bitmap(bm, 32, 32, leaders[l].x, leaders[l].y, 0.5, 0.5, leaders[l].angle, 0);
			}

			if(whistle_radius > 0){
				al_draw_circle(cursor_x, cursor_y, whistle_radius, al_map_rgb(192, 192, 0), 2);
			}

			//Onions.
			size_t n_onions = onions.size();
			for(size_t o=0; o<n_onions; o++){
				ALLEGRO_BITMAP* bm;
				if(onions[o].type->name == "R") bm = bmp_red_onion;
				else if(onions[o].type->name == "Y") bm = bmp_yellow_onion;
				if(onions[o].type->name == "B") bm = bmp_blue_onion;

				int onion_bmp_w = 370, onion_bmp_h = 320;
				al_draw_tinted_scaled_bitmap(
					bm, al_map_rgba(255, 255, 255, 192),
					0, 0, onion_bmp_w, onion_bmp_h,
					onions[o].x - onion_bmp_w / 4,
					onions[o].y - onion_bmp_h / 4,
					onion_bmp_w / 2,
					onion_bmp_h / 2,
					0);
			}

			//Info spots.
			size_t n_info_spots = info_spots.size();
			for(size_t i=0; i<n_info_spots; i++){
				al_draw_filled_rectangle(
					info_spots[i].x - info_spots[i].size * 0.5,
					info_spots[i].y - info_spots[i].size * 0.5,
					info_spots[i].x + info_spots[i].size * 0.5,
					info_spots[i].y + info_spots[i].size * 0.5,
					al_map_rgb(192, 64, 192)
					);
			}


			/* Layer 5
			*****************************
			*                     Help  *
			*   On-screen text   --  -- *
			*                      \/   *
			****************************/
			
			//Fractions.
			for(size_t t=0; t<n_treasures; t++){
				//ToDo only show if it has at least one carrier.
				//ToDo fraction color.
				//ToDo it's not taking Pikmin strength into account.
				//ToDo can't this be more optimized than running through the vector of carrier spots?
				if(treasures[t].carrier_info->current_n_carriers>0)
					draw_fraction(treasures[t].x, treasures[t].y, treasures[t].carrier_info->current_n_carriers, treasures[t].weight, al_map_rgb(192, 192, 192));
			}

			//Info spots.
			for(size_t i=0; i<n_info_spots; i++){
				if(dist(leaders[current_leader].x, leaders[current_leader].y, info_spots[i].x, info_spots[i].y) <= INFO_SPOT_TRIGGER_RANGE){
					string text;
					if(!info_spots[i].fullscreen)
						text = info_spots[i].text;
					else
						text = "(...)";
					
					al_draw_text(font, al_map_rgb(255, 255, 255), info_spots[i].x, info_spots[i].y - info_spots[i].size * 0.5 - font_h * 2, ALLEGRO_ALIGN_CENTER, text.c_str());
					if(!info_spots[i].fullscreen){
						int line_y = info_spots[i].y - info_spots[i].size * 0.5 - font_h * 0.75;
							
						al_draw_line(
							info_spots[i].x - info_spots[i].text_w * 0.5,
							line_y,
							info_spots[i].x - 8,
							line_y,
							al_map_rgb(192, 192, 192), 1);
						al_draw_line(
							info_spots[i].x + info_spots[i].text_w * 0.5,
							line_y,
							info_spots[i].x + 8,
							line_y,
							al_map_rgb(192, 192, 192), 1);
						al_draw_line(
							info_spots[i].x - 8,
							line_y,
							info_spots[i].x,
							info_spots[i].y - info_spots[i].size * 0.5 - font_h * 0.25,
							al_map_rgb(192, 192, 192), 1);
						al_draw_line(
							info_spots[i].x + 8,
							line_y,
							info_spots[i].x,
							info_spots[i].y - info_spots[i].size * 0.5 - font_h * 0.25,
							al_map_rgb(192, 192, 192), 1);
					}
				}
			}


			/* Layer 6
			***********************
			*                 *   *
			*   Particles   *   * *
			*                ***  *
			**********************/

			if(particle_quality>0){
				n_particles = particles.size();
				for(size_t p=0; p<n_particles; p++){
					if(particle_quality==1){
						al_draw_filled_rectangle(
							particles[p].x - particles[p].size*0.5,
							particles[p].y - particles[p].size*0.5,
							particles[p].x + particles[p].size*0.5,
							particles[p].y + particles[p].size*0.5,
							change_alpha(particles[p].color, (particles[p].time / particles[p].starting_time) * particles[p].color.a * 255)
							);
					}else{
						al_draw_filled_circle(
							particles[p].x,
							particles[p].y,
							particles[p].size * 0.5,
							change_alpha(particles[p].color, (particles[p].time / particles[p].starting_time) * particles[p].color.a * 255)
							);
					}
				}
			}


			/* Layer 7
			***********************
			*              --==## *
			*   Daylight   --==## *
			*              --==## *
			**********************/

			al_use_transform(&normal_transform);

			if(daylight_effect){
				al_draw_filled_rectangle(0, 0, scr_w, scr_h, get_daylight_color());
			}


			/* Layer 8
			*****************
			*           (1) *
			*   HUD         *
			*         1/2/3 *
			****************/
			
			//ToDo actual image coordinates (remember the health bubble has a different image than the regular ones)
			int leader_img_size = 64;
			int bubble_img_size = 64;

			//Leader health.
			for(size_t l = 0; l < 3; l++){
				if(n_leaders < l + 1) continue;

				size_t l_nr = (current_leader+l)%n_leaders;
				ALLEGRO_BITMAP* bm = (l_nr==0) ? bmp_olimar : ((l_nr==1) ? bmp_louie : bmp_president);

				int icons_size;
				if(l == 0) icons_size = 32; else icons_size = 20;

				int y_offset;
				if(l == 0) y_offset = 0; else if(l == 1) y_offset = 44; else y_offset = 80;

				al_draw_scaled_bitmap(
					bm, 0, 0, leader_img_size, leader_img_size,
					32 - icons_size / 2,
					scr_h - (32 + y_offset + icons_size / 2),
					icons_size, icons_size, 0
					);
				al_draw_scaled_bitmap(bmp_bubble, 0, 0, bubble_img_size, bubble_img_size,
					32 - (icons_size * 1.6) / 2,
					scr_h - (32 + y_offset + (icons_size * 1.6) / 2),
					icons_size * 1.6, icons_size * 1.6, 0
					);

				draw_health(
					32 + icons_size * 1.5,
					scr_h - (32 + y_offset),
					leaders[l_nr].health, leaders[l_nr].max_health,
					icons_size / 2, true);
				al_draw_scaled_bitmap(
					bmp_health_bubble, 0, 0, bubble_img_size, bubble_img_size,
					(32 + icons_size * 1.5) - (icons_size * 1.2) / 2,
					scr_h - (32 + y_offset + (icons_size * 1.2) / 2),
					icons_size * 1.2, icons_size * 1.2, 0);
			}

			//Day hour.
			al_draw_text(font, al_map_rgb(255, 255, 255), 8, 8, 0,
				(to_string((long long) (day_minutes/60)) + ":" + to_string((long long) ((int) (day_minutes)%60))).c_str());

			//Sun Meter.
			unsigned char n_hours = (day_minutes_end - day_minutes_start) / 60;
			unsigned int sun_meter_span = (scr_w - 100) - 20;
			unsigned short interval = sun_meter_span / n_hours;
			for(unsigned char h = 0; h<n_hours + 1; h++){
				al_draw_scaled_bitmap(bmp_bubble, 0, 0, 64, 64, 20 + h*interval, 40, 10, 10, 0);
			}

			float day_passed_ratio = (float) (day_minutes - day_minutes_start) / (float) (day_minutes_end - day_minutes_start);
			al_draw_scaled_bitmap(bmp_sun, 0, 0, 64, 64, 10 + day_passed_ratio * sun_meter_span, 30, 30, 30, 0);

			//Pikmin count.
			//Count how many Pikmin only.
			n_leaders = leaders.size();
			size_t pikmin_in_party = leaders[current_leader].party.size();
			for(size_t l=0; l<n_leaders; l++){
				//If this leader is following the current one, then he's not a Pikmin, subtract him from the party count total.
				if(leaders[l].following_party == &leaders[current_leader]) pikmin_in_party--;
			}

			//Closest party member.
			if(closest_party_member){
				ALLEGRO_BITMAP* bm = NULL;
				unsigned int bm_w = 0;
				if(typeid(*closest_party_member) == typeid(pikmin)){
					pikmin* pikmin_ptr = dynamic_cast<pikmin*>(closest_party_member);
					if(pikmin_ptr->type->name=="R") bm=bmp_red[pikmin_ptr->maturity];
					else if(pikmin_ptr->type->name=="Y") bm=bmp_yellow[pikmin_ptr->maturity];
					else if(pikmin_ptr->type->name=="B") bm=bmp_blue[pikmin_ptr->maturity];
					bm_w = 36;
				}else if(typeid(*closest_party_member) == typeid(leader)){
					leader* leader_ptr = dynamic_cast<leader*>(closest_party_member);
					if(leader_ptr == &leaders[0]) bm = bmp_olimar;
					else if(leader_ptr == &leaders[1]) bm = bmp_louie;
					else if(leader_ptr == &leaders[2]) bm = bmp_president;
					bm_w = 64;
				}

				if(bm){
					al_draw_scaled_bitmap(bm, 0, 0, bm_w, bm_w, 245, scr_h - 25 - font_h, 32, 32, 0);
				}
			}

			al_draw_scaled_bitmap(bmp_bubble, 0, 0, 64, 64, 240, scr_h - 30 - font_h, 40, 40, 0);

			//Pikmin count numbers.
			al_draw_text(
				font, al_map_rgb(255, 255, 255), scr_w - 20, scr_h - 20 - font_h, ALLEGRO_ALIGN_RIGHT,
				(to_string((long long) pikmin_in_party) + "/" + to_string((long long) pikmin_list.size()) + "/1234").c_str()
				);

			//Day number.
			al_draw_text(
				font, al_map_rgb(255, 255, 255), scr_w - 50, 40, ALLEGRO_ALIGN_CENTER,
				(to_string((long long) day)).c_str());

			al_draw_scaled_bitmap(bmp_day_bubble, 0, 0, 128, 152, scr_w - 80, 10, 60, 70, 0);

			//Sprays.
			if(n_spray_types > 0){
				size_t top_spray_nr;
				if(n_spray_types < 3) top_spray_nr = 0; else top_spray_nr = selected_spray;

				al_draw_scaled_bitmap(spray_types[top_spray_nr].bmp_spray, 0, 0, 40, 48, 24, scr_h / 2 - 32, 20, 24, 0);
				al_draw_text(
					font, al_map_rgb(255, 255, 255), 48, scr_h / 2 - 32, 0,
					("x" + to_string((long long) sprays[top_spray_nr])).c_str());
				if(n_spray_types == 2){
					al_draw_scaled_bitmap(spray_types[1].bmp_spray, 0, 0, 40, 48, 24, scr_h / 2 + 8, 20, 24, 0);
					al_draw_text(
						font, al_map_rgb(255, 255, 255), 48, scr_h / 2 + 8, 0,
						("x" + to_string((long long) sprays[1])).c_str());
				}
			}

			//ToDo remove me al_draw_text(font_area_name, al_map_rgb(224, 232, 64), scr_w/2, scr_h/2, ALLEGRO_ALIGN_CENTER, "Green Hill Zone");
			
			al_flip_display();
		}
	}
}
