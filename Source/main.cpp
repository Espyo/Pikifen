//ToDo limit whistle to the real cursor, not ghost.

#include <math.h>

#include <allegro5\allegro.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_image.h>
#include <allegro5\allegro_primitives.h>

#include "const.h"
#include "functions.h"
#include "vars.h"

int main(){
	bool running=true;
	
	//Configurations outside of Allegro.
	srand(time(NULL));

	//Install Allegro and initialize modules.
	al_init();
	al_install_mouse();
	al_install_keyboard();
	al_init_image_addon();
	al_init_primitives_addon();

	//Event stuff.
	ALLEGRO_DISPLAY* display = al_create_display(600, 400);
	ALLEGRO_TIMER* timer = al_create_timer(1.0/30.0);

	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));
	ALLEGRO_EVENT ev;

	//Graphics.
	ALLEGRO_BITMAP* olimar = al_load_bitmap("Olimar.png");
	ALLEGRO_BITMAP* louie = al_load_bitmap("Louie.png");
	ALLEGRO_BITMAP* red = al_load_bitmap("Red.png");
	ALLEGRO_BITMAP* yellow = al_load_bitmap("Yellow.png");
	ALLEGRO_BITMAP* blue = al_load_bitmap("Blue.png");
	ALLEGRO_BITMAP* red_burrowed = al_load_bitmap("Red_burrowed.png");
	ALLEGRO_BITMAP* yellow_burrowed = al_load_bitmap("Yellow_burrowed.png");
	ALLEGRO_BITMAP* blue_burrowed = al_load_bitmap("Blue_burrowed.png");
	ALLEGRO_BITMAP* cursor = al_load_bitmap("Cursor.png");
	ALLEGRO_BITMAP* mouse_cursor = al_load_bitmap("Mouse_cursor.png");
	ALLEGRO_BITMAP* background = al_load_bitmap("Background.png");
	ALLEGRO_BITMAP* bubble = al_load_bitmap("Bubble.png");
	ALLEGRO_BITMAP* day_bubble = al_load_bitmap("Day_bubble.png");
	ALLEGRO_BITMAP* health_bubble = al_load_bitmap("Health_bubble.png");
	ALLEGRO_BITMAP* sun = al_load_bitmap("Sun.png");

	int font_ranges[] = {
		0x0020, 0x007F,		/* ASCII */
		0x00A1, 0x00FF,		/* Latin 1 */
		0x0100, 0x017F,		/* Extended-A */
		0x20AC, 0x20AC		/* Euro */
	};

	ALLEGRO_BITMAP* font_bitmap = al_load_bitmap("Font.png");  //We can't load the font directly because we want to set the ranges.
	if(font_bitmap) font=al_grab_font_from_bitmap(font_bitmap, 4, font_ranges);
	al_destroy_bitmap(font_bitmap);
	font_h = al_get_font_line_height(font);

	//Configurations.
	al_set_window_title(display, "Pikmin fangame engine");
	al_hide_mouse_cursor(display);

	//Game content.
	load_game_content();

	//Some variables.
	sector s = sector();
	leaders.push_back(leader(200, 300, &s));
	leaders.push_back(leader(300, 250, &s));
	leaders.push_back(leader(350, 200, &s));
	pikmin_list.push_back(pikmin(&pikmin_types[0], 30, 30, &s));
	pikmin_list.push_back(pikmin(&pikmin_types[0], 40, 30, &s));
	pikmin_list.push_back(pikmin(&pikmin_types[1], 50, 30, &s));
	pikmin_list.push_back(pikmin(&pikmin_types[1], 60, 30, &s));
	pikmin_list.push_back(pikmin(&pikmin_types[2], 70, 30, &s));
	pikmin_list.push_back(pikmin(&pikmin_types[2], 80, 30, &s));
	pikmin_list.push_back(pikmin(&pikmin_types[0], 30, 200, &s));
	pikmin_list.back().burrowed=true;
	pikmin_list.push_back(pikmin(&pikmin_types[1], 50, 200, &s));
	pikmin_list.back().burrowed=true;
	pikmin_list.push_back(pikmin(&pikmin_types[2], 70, 200, &s));
	pikmin_list.back().burrowed=true;

	//Main loop.
	al_start_timer(timer);
	while(running){
		al_wait_for_event(queue, &ev);
		if(ev.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
			running=false;

		}else if(ev.type==ALLEGRO_EVENT_MOUSE_AXES){
			mouse_cursor_x = ev.mouse.x;
			mouse_cursor_y = ev.mouse.y;

		}else if(ev.type==ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
			if(ev.mouse.button == 1){
				bool punch = false;
				bool throw_pikmin = false;
				
				//First check if the leader should pluck a Pikmin.

				float closest_distance = 0;
				pikmin* closest_pikmin = NULL;
				size_t n_pikmin = pikmin_list.size();
				for(size_t p=0; p<n_pikmin; p++){
					if(!pikmin_list[p].burrowed) continue;
					float d = dist(leaders[current_leader].x, leaders[current_leader].y, pikmin_list[p].x, pikmin_list[p].y);
					if(closest_pikmin == NULL || d < closest_distance){
						closest_distance = d;
						closest_pikmin = &pikmin_list[p];
					}
				}

				if(closest_pikmin){
					if(closest_distance <= MIN_PLUCK_RANGE){
						//Pluck.
						closest_pikmin->burrowed = false;
						add_to_party(&leaders[current_leader], closest_pikmin);
					}else{
						throw_pikmin = true;
					}
				}else{
					throw_pikmin = true;
				}
				
				//Now check if the leader should throw a Pikmin.

				if(throw_pikmin){
					if(closest_party_member){
						leaders[current_leader].holding_pikmin = closest_party_member;
					}else{
						punch = true;
					}
				}

				//Now check if the leader should punch.

				if(punch){
					//ToDo
				}

			}else if(ev.mouse.button == 2){
				whistling = true;
			}

		}else if(ev.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
			if(ev.mouse.button == 1){
				if(leaders[current_leader].holding_pikmin){;

					leaders[current_leader].holding_pikmin->x = leaders[current_leader].x;
					leaders[current_leader].holding_pikmin->y = leaders[current_leader].y;

					float d = dist(leaders[current_leader].x, leaders[current_leader].y, cursor_x, cursor_y);
					leaders[current_leader].holding_pikmin->speed_x = cos(leaders[current_leader].angle) * d;
					leaders[current_leader].holding_pikmin->speed_y = sin(leaders[current_leader].angle) * d;
				
					remove_from_party(&leaders[current_leader], leaders[current_leader].holding_pikmin);
					leaders[current_leader].holding_pikmin = NULL;
				}

			}else if(ev.mouse.button == 2){
				stop_whistling();
			}

		}else if(ev.type==ALLEGRO_EVENT_KEY_DOWN){
			//ToDo change to angles.
			if(ev.keyboard.keycode == ALLEGRO_KEY_W){
				leaders[current_leader].speed_y = -LEADER_MOVE_SPEED;

			}else if(ev.keyboard.keycode == ALLEGRO_KEY_A){
				leaders[current_leader].speed_x = -LEADER_MOVE_SPEED;

			}else if(ev.keyboard.keycode == ALLEGRO_KEY_S){
				leaders[current_leader].speed_y = LEADER_MOVE_SPEED;

			}else if(ev.keyboard.keycode == ALLEGRO_KEY_D){
				leaders[current_leader].speed_x = LEADER_MOVE_SPEED;

			}else if(ev.keyboard.keycode == ALLEGRO_KEY_TAB){
				leaders[current_leader].speed_x = 0;
				leaders[current_leader].speed_y = 0;

				size_t new_leader = (current_leader + 1) % leaders.size();
				bool swap_leaders = false;

				//Transfer all members onto the new leader, if the new leader is in the current one's party.
				size_t n_party_members = leaders[current_leader].party.size();
				for(size_t m=0; m<n_party_members; m++){
					if(leaders[current_leader].party[m] == &leaders[new_leader]) swap_leaders=true;
				}

				if(swap_leaders){
					for(size_t m=0; m<n_party_members; m++){
						mob* member = leaders[current_leader].party[0];
						remove_from_party(&leaders[current_leader], member);
						if(member != &leaders[new_leader]){
							add_to_party(&leaders[new_leader], member);
						}
					}

					add_to_party(&leaders[new_leader], &leaders[current_leader]);
				}

				current_leader = new_leader;

			}else if(ev.keyboard.keycode == ALLEGRO_KEY_LCTRL){
				leader* current_leader_ptr = &leaders[current_leader];

				size_t n_party_members = current_leader_ptr->party.size();
				for(size_t m=0; m<n_party_members; m++){
					remove_from_party(current_leader_ptr, current_leader_ptr->party[0]);
				}
			}else if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE){
				running = false;

			}else if(ev.keyboard.keycode == ALLEGRO_KEY_T){
				//Debug testing.
				//ToDo remove.
				//leaders[current_leader].health--;
				day_minutes += 60;
			}

		}else if(ev.type==ALLEGRO_EVENT_KEY_UP){
			//ToDo change to angles.
			if(ev.keyboard.keycode == ALLEGRO_KEY_W || ev.keyboard.keycode==ALLEGRO_KEY_S){
				leaders[current_leader].speed_y = 0;
			}else if(ev.keyboard.keycode == ALLEGRO_KEY_A || ev.keyboard.keycode==ALLEGRO_KEY_D){
				leaders[current_leader].speed_x = 0;
			}

		}else if(ev.type==ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(queue)){
			
			//Logic.

			//Cursor.
			leaders[current_leader].angle = atan2(mouse_cursor_y - leaders[current_leader].y, mouse_cursor_x - leaders[current_leader].x);
			if(dist(leaders[current_leader].x, leaders[current_leader].y, mouse_cursor_x, mouse_cursor_y) > CURSOR_MAX_DIST){
				//Cursor goes beyond the range limit.
				cursor_x = leaders[current_leader].x + (cos(leaders[current_leader].angle) * CURSOR_MAX_DIST);
				cursor_y = leaders[current_leader].y + (sin(leaders[current_leader].angle) * CURSOR_MAX_DIST);
			}else{
				cursor_x = mouse_cursor_x;
				cursor_y = mouse_cursor_y;
			}

			//Whistling.
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

			//Pikmin.
			size_t n_pikmin = pikmin_list.size();
			for(size_t p=0; p<n_pikmin; p++){
				if(whistling){
					if(
						dist(pikmin_list[p].x, pikmin_list[p].y, cursor_x, cursor_y) <= whistle_radius &&
						pikmin_list[p].following_party==NULL &&
						!pikmin_list[p].burrowed){
							//Pikmin got whistled.
							add_to_party(&leaders[current_leader], &pikmin_list[p]);
					}
				}

				if(pikmin_list[p].following_party){
					pikmin_list[p].target_x = pikmin_list[p].following_party->x + random(0, 30);
					pikmin_list[p].target_y = pikmin_list[p].following_party->y + random(0, 30);
				}
				
				pikmin_list[p].tick();

			}

			//Leaders.
			if(leaders[current_leader].holding_pikmin){
				leaders[current_leader].holding_pikmin->x = leaders[current_leader].x+8;
				leaders[current_leader].holding_pikmin->y = leaders[current_leader].y;
			}

			size_t n_leaders = leaders.size();
			for(size_t l=0; l<n_leaders; l++){
				if(whistling){
					if(l != current_leader){
						if(dist(leaders[l].x, leaders[l].y, cursor_x, cursor_y) <= whistle_radius && leaders[l].following_party==NULL){
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

			//Party.
			float closest_distance = 0;
			size_t n_members = leaders[current_leader].party.size();
			closest_party_member = NULL;

			if(n_members > 0){

				for(size_t m=0; m<n_members; m++){
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

			//Time.
			day_minutes += (1.0f / game_fps) * day_minutes_per_irl_sec;
			if(day_minutes > 60 * 24) day_minutes -= 60 * 24;

			//Particles.
			size_t n_particles = particles.size();
			for(size_t p=0; p<n_particles; ){
				if(!particles[p].tick()){
					particles.erase(particles.begin() + p);
					n_particles--;
				}else{
					p++;
				}
			}





			//Draw.
			al_draw_bitmap(background, scr_w/2 - 512, scr_h/2 - 512, 0);
			al_draw_bitmap(background, scr_w/2 - 512, scr_h/2, 0);
			al_draw_bitmap(background, scr_w/2, scr_h/2 - 512, 0);
			al_draw_bitmap(background, scr_w/2, scr_h/2, 0);
			
			//Cursor.
			al_draw_rotated_bitmap(mouse_cursor, 24, 24, mouse_cursor_x, mouse_cursor_y, leaders[current_leader].angle, 0);
			al_draw_rotated_bitmap(cursor, 24, 24, cursor_x, cursor_y, leaders[current_leader].angle, 0);

			//Pikmin.
			n_pikmin = pikmin_list.size();
			for(size_t p = 0; p<n_pikmin; p++){
				ALLEGRO_BITMAP* bm;
				if(pikmin_list[p].type->name=="R"){
					if(pikmin_list[p].burrowed) bm=red_burrowed; else bm=red;
				}else if(pikmin_list[p].type->name=="Y"){
					if(pikmin_list[p].burrowed) bm=yellow_burrowed; else bm=yellow;
				}if(pikmin_list[p].type->name=="B"){
					if(pikmin_list[p].burrowed) bm=blue_burrowed; else bm=blue;
				}
				al_draw_rotated_bitmap(bm, 12, 12, pikmin_list[p].x, pikmin_list[p].y, pikmin_list[p].angle, 0);
			}

			//Leaders.
			for(size_t l=0; l<n_leaders; l++){
				ALLEGRO_BITMAP* bm = (l==0) ? olimar : louie;
				al_draw_rotated_bitmap(bm, 16, 16, leaders[l].x, leaders[l].y, leaders[l].angle, 0);
			}

			if(whistle_radius > 0){
				al_draw_circle(cursor_x, cursor_y, whistle_radius, al_map_rgb(192, 192, 0), 2);
			}

			n_particles = particles.size();
			for(size_t p=0; p<n_particles; p++){
				al_draw_filled_rectangle(
					particles[p].x - particles[p].size*0.5,
					particles[p].y - particles[p].size*0.5,
					particles[p].x + particles[p].size*0.5,
					particles[p].y + particles[p].size*0.5,
					particles[p].color);
			}

			//Daylight effect.
			if(daylight_effect){
				al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
				al_draw_filled_rectangle(0, 0, scr_w, scr_h, get_daylight_color());
			}

			//HUD
			//ToDo remove al_draw_line(0, scr_h - 40 - font_h, scr_w, scr_h - 40 - font_h, al_map_rgb(255, 255, 255), 1);

			ALLEGRO_BITMAP* bm = (current_leader==0) ? olimar : louie;
			al_draw_bitmap(bm, 20, scr_h - 45, 0);
			al_draw_scaled_bitmap(bubble, 0, 0, 64, 64, 10, scr_h - 55, 50, 50, 0);

			draw_health(85, scr_h - 30, leaders[current_leader].health, 10, true);
			al_draw_scaled_bitmap(health_bubble, 0, 0, 64, 64, 85 - HEALTH_CIRCLE_RADIUS * 1.2, scr_h - 30 - HEALTH_CIRCLE_RADIUS * 1.2, HEALTH_CIRCLE_RADIUS * 2.4, HEALTH_CIRCLE_RADIUS * 2.4, 0);

			al_draw_text(font, al_map_rgb(255, 255, 255), 8, 8, 0,
				(to_string((long long) (day_minutes/60)) + ":" + to_string((long long) ((int) (day_minutes)%60))).c_str());

			unsigned char n_hours = (day_minutes_end - day_minutes_start) / 60;
			unsigned int sun_meter_span = (scr_w - 100) - 20;
			unsigned short interval = sun_meter_span / n_hours;
			for(unsigned char h = 0; h<n_hours + 1; h++){
				al_draw_scaled_bitmap(bubble, 0, 0, 64, 64, 20 + h*interval, 40, 10, 10, 0);
			}

			float day_passed_ratio = (float) (day_minutes - day_minutes_start) / (float) (day_minutes_end - day_minutes_start);
			al_draw_scaled_bitmap(sun, 0, 0, 64, 64, 10 + day_passed_ratio * sun_meter_span, 30, 30, 30, 0);

			//Count how many Pikmin only.
			n_leaders = leaders.size();
			size_t pikmin_in_party = leaders[current_leader].party.size();
			for(size_t l=0; l<n_leaders; l++){
				//If this leader is following the current one, then he's not a Pikmin, subtract him from the party count total.
				if(leaders[l].following_party == &leaders[current_leader]) pikmin_in_party--;
			}

			if(closest_party_member){
				if(typeid(*closest_party_member) == typeid(pikmin)){
					pikmin* pikmin_ptr = dynamic_cast<pikmin*>(closest_party_member);
					ALLEGRO_BITMAP* bm;
					if(pikmin_ptr->type->name=="R") bm=red;
					else if(pikmin_ptr->type->name=="Y") bm=yellow;
					if(pikmin_ptr->type->name=="B") bm=blue;
					
					al_draw_bitmap(bm, 250, scr_h - 20 - font_h, 0);
				}
			}

			al_draw_scaled_bitmap(bubble, 0, 0, 64, 64, 240, scr_h - 30 - font_h, 40, 40, 0);

			al_draw_text(
				font, al_map_rgb(255, 255, 255), scr_w - 20, scr_h - 20 - font_h, ALLEGRO_ALIGN_RIGHT,
				(to_string((long long) pikmin_in_party) + "/" + to_string((long long) pikmin_list.size()) + "/1234").c_str()
				);

			al_draw_text(
				font, al_map_rgb(255, 255, 255), scr_w - 50, 40, ALLEGRO_ALIGN_CENTER,
				(to_string((long long) day)).c_str());

			al_draw_scaled_bitmap(day_bubble, 0, 0, 128, 152, scr_w - 80, 10, 60, 70, 0);
			
			al_flip_display();
		}
	}
}