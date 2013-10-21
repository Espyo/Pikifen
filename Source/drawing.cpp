#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"
#include "functions.h"
#include "vars.h"

void do_drawing(){

	/*  ***************************************
	  *** |  |                           |  | ***
	***** |__|          DRAWING          |__| *****
	  ***  \/                             \/  ***
		***************************************/

	if(!paused){

		size_t n_leaders =     leaders.size();
		size_t n_particles =   particles.size();
		size_t n_pikmin =      pikmin_list.size();
		size_t n_spray_types = spray_types.size();
		size_t n_treasures =   treasures.size();		

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

		//ToDo optimize
		size_t area_image_cols = area_images.size();
		for(size_t x=0; x < area_image_cols; x++){
			size_t area_image_rows = area_images[x].size();
			for(size_t y=0; y < area_image_rows; y++){
				al_draw_bitmap(area_images[x][y], x * AREA_IMAGE_SIZE + area_x1, y * AREA_IMAGE_SIZE + area_y1, 0);
			}
		}
		

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

		for(size_t l = 0; l < n_leaders; l++){
			draw_shadow(leaders[l]->x, leaders[l]->y, 32, leaders[l]->z - leaders[l]->sec->floors[0].z, shadow_stretch);
		}

		for(size_t p = 0; p < n_pikmin; p++){
			draw_shadow(pikmin_list[p]->x, pikmin_list[p]->y, 18, pikmin_list[p]->z - pikmin_list[p]->sec->floors[0].z, shadow_stretch);
		}


		/* Layer 3
		*********************
		*             .-.   *
		*   Cursor   ( = )> *
		*             `-´   *
		********************/

		size_t n_arrows = move_group_arrows.size();
		for(size_t a=0; a<n_arrows; a++){
			float x = cos(moving_group_angle) * move_group_arrows[a];
			float y = sin(moving_group_angle) * move_group_arrows[a];
			al_draw_scaled_rotated_bitmap(
				bmp_move_group_arrow,
				16, 26,
				leaders[current_leader]->x + x,
				leaders[current_leader]->y + y,
				0.5, 0.5,
				moving_group_angle,
				0);
		}

		size_t n_rings = whistle_rings.size();
		for(size_t r=0; r<n_rings; r++){
			float x = leaders[current_leader]->x + cos(leaders[current_leader]->angle) * whistle_rings[r];
			float y = leaders[current_leader]->y + sin(leaders[current_leader]->angle) * whistle_rings[r];
			unsigned char n = whistle_ring_colors[r];
			al_draw_circle(x, y, 12, al_map_rgb(WHISTLE_RING_COLORS[n][0], WHISTLE_RING_COLORS[n][1], WHISTLE_RING_COLORS[n][2]), 1);
		}

		if(whistle_radius > 0 || whistle_fade_time > 0){
			if(whistle_is_circle){
				unsigned char alpha = whistle_fade_time / WHISTLE_FADE_TIME * 255;
				float radius = whistle_fade_radius;
				if(whistle_radius > 0){
					alpha = 255;
					radius = whistle_radius;
				}
				al_draw_circle(cursor_x, cursor_y, radius, al_map_rgba(192, 192, 0, alpha), 2);
			}else{
				unsigned char n_dots = 16*6;
				for(unsigned char d=0; d<6; d++){
					for(unsigned char d2=0; d2<16; d2++){
						unsigned char current_dot = d2 * 6 + d;
						float angle = M_PI * 2 / n_dots * current_dot + whistle_dot_offset;

						float x = cursor_x + cos(angle) * whistle_dot_radius[d];
						float y = cursor_y + sin(angle) * whistle_dot_radius[d];

						ALLEGRO_COLOR c;
						float alpha_mult;
						if(whistle_fade_time > 0)
							alpha_mult = whistle_fade_time / WHISTLE_FADE_TIME;
						else
							alpha_mult = 1;

						if(d==0)        c=al_map_rgba(255, 0,   0,   255 * alpha_mult);
						else if(d == 1) c=al_map_rgba(255, 128, 0,   210 * alpha_mult);
						else if(d == 2) c=al_map_rgba(128, 255, 0,   165 * alpha_mult);
						else if(d == 3) c=al_map_rgba(0,   255, 255, 120 * alpha_mult);
						else if(d == 4) c=al_map_rgba(0,   0,   255, 75  * alpha_mult);
						else            c=al_map_rgba(128, 0,   255, 30  * alpha_mult);

						al_draw_filled_circle(x, y, 2, c);
					}
				}
			}
		}

		al_use_transform(&normal_transform);
		al_draw_scaled_rotated_bitmap(bmp_mouse_cursor, 48, 48, mouse_cursor_x, mouse_cursor_y, 0.5 * cam_zoom, 0.5 * cam_zoom, leaders[current_leader]->angle, 0);
		al_use_transform(&world_to_screen_transform);
		al_draw_scaled_rotated_bitmap(bmp_cursor, 48, 48, cursor_x, cursor_y, 0.5, 0.5, leaders[current_leader]->angle, 0);

			
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
				nectars[n]->x,
				nectars[n]->y,
				nectars[n]->size * (nectars[n]->amount_left + NECTAR_AMOUNT) / (NECTAR_AMOUNT * 2),
				al_map_rgb(255, 255, 0)
				);
		}

		//Treasures.
		for(size_t t=0; t<n_treasures; t++){
			al_draw_filled_circle(treasures[t]->x, treasures[t]->y, treasures[t]->size * 0.5, al_map_rgb(128, 255, 255));
		}

		//Pikmin
		n_pikmin = pikmin_list.size();
		for(size_t p = 0; p<n_pikmin; p++){
			ALLEGRO_BITMAP* bm = NULL;

			bool idling = !pikmin_list[p]->following_party && !pikmin_list[p]->carrying_treasure;

			if(pikmin_list[p]->type->name=="R"){
				if(pikmin_list[p]->burrowed) bm=bmp_red_burrowed[pikmin_list[p]->maturity];
				else if(idling) bm=bmp_red_idle[pikmin_list[p]->maturity];
				else bm=bmp_red[pikmin_list[p]->maturity];
			}else if(pikmin_list[p]->type->name=="Y"){
				if(pikmin_list[p]->burrowed) bm=bmp_yellow_burrowed[pikmin_list[p]->maturity];
				else if(idling) bm=bmp_yellow_idle[pikmin_list[p]->maturity];
				else bm=bmp_yellow[pikmin_list[p]->maturity];
			}if(pikmin_list[p]->type->name=="B"){
				if(pikmin_list[p]->burrowed) bm=bmp_blue_burrowed[pikmin_list[p]->maturity];
				else if(idling) bm=bmp_blue_idle[pikmin_list[p]->maturity];
				else bm=bmp_blue[pikmin_list[p]->maturity];
			}
			al_draw_scaled_rotated_bitmap(bm, 18, 18, pikmin_list[p]->x, pikmin_list[p]->y, 0.5, 0.5, pikmin_list[p]->angle, 0); //ToDo actual coordinates

			if(idling){
				al_draw_tinted_scaled_rotated_bitmap(
					bmp_idle_glow,
					change_alpha(pikmin_list[p]->main_color, 128),
					18,
					18,
					pikmin_list[p]->x,
					pikmin_list[p]->y,
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
			al_draw_scaled_rotated_bitmap(bm, 32, 32, leaders[l]->x, leaders[l]->y, 0.5, 0.5, leaders[l]->angle, 0);
		}

		//Onions.
		size_t n_onions = onions.size();
		for(size_t o=0; o<n_onions; o++){
			ALLEGRO_BITMAP* bm = NULL;
			if(onions[o]->type->name == "R") bm = bmp_red_onion;
			else if(onions[o]->type->name == "Y") bm = bmp_yellow_onion;
			if(onions[o]->type->name == "B") bm = bmp_blue_onion;

			int onion_bmp_w = 370, onion_bmp_h = 320;
			al_draw_tinted_scaled_bitmap(
				bm, al_map_rgba(255, 255, 255, 192),
				0, 0, onion_bmp_w, onion_bmp_h,
				onions[o]->x - onion_bmp_w / 4,
				onions[o]->y - onion_bmp_h / 4,
				onion_bmp_w / 2,
				onion_bmp_h / 2,
				0);
		}

		//Info spots.
		size_t n_info_spots = info_spots.size();
		for(size_t i=0; i<n_info_spots; i++){
			al_draw_filled_rectangle(
				info_spots[i]->x - info_spots[i]->size * 0.5,
				info_spots[i]->y - info_spots[i]->size * 0.5,
				info_spots[i]->x + info_spots[i]->size * 0.5,
				info_spots[i]->y + info_spots[i]->size * 0.5,
				al_map_rgb(192, 64, 192)
				);
		}

		//Ship(s).
		int ship_bmp_w = 274, ship_bmp_h = 225;
		size_t n_ships = ships.size();
		for(size_t s=0; s<n_ships; s++){
			al_draw_scaled_bitmap(
				bmp_ship,
				0, 0,
				ship_bmp_w, ship_bmp_h,
				ships[s]->x - ship_bmp_w / 4,
				ships[s]->y - ship_bmp_h / 4,
				ship_bmp_w / 2, ship_bmp_h / 2,
				0);
			al_draw_circle(ships[s]->x + ships[s]->size / 2 + SHIP_BEAM_RANGE, ships[s]->y, SHIP_BEAM_RANGE, al_map_rgb(ship_beam_ring_color[0], ship_beam_ring_color[1], ship_beam_ring_color[2]), 1);
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
			if(treasures[t]->carrier_info->current_n_carriers>0)
				draw_fraction(treasures[t]->x, treasures[t]->y, treasures[t]->carrier_info->current_n_carriers, treasures[t]->weight, al_map_rgb(192, 192, 192));
		}

		//Info spots.
		for(size_t i=0; i<n_info_spots; i++){
			if(dist(leaders[current_leader]->x, leaders[current_leader]->y, info_spots[i]->x, info_spots[i]->y) <= INFO_SPOT_TRIGGER_RANGE){
				string text;
				if(!info_spots[i]->fullscreen)
					text = info_spots[i]->text;
				else
					text = "(...)";
					
				al_draw_text(font, al_map_rgb(255, 255, 255), info_spots[i]->x, info_spots[i]->y - info_spots[i]->size * 0.5 - font_h * 2, ALLEGRO_ALIGN_CENTER, text.c_str());
				if(!info_spots[i]->fullscreen){
					int line_y = info_spots[i]->y - info_spots[i]->size * 0.5 - font_h * 0.75;
							
					al_draw_line(
						info_spots[i]->x - info_spots[i]->text_w * 0.5,
						line_y,
						info_spots[i]->x - 8,
						line_y,
						al_map_rgb(192, 192, 192), 1);
					al_draw_line(
						info_spots[i]->x + info_spots[i]->text_w * 0.5,
						line_y,
						info_spots[i]->x + 8,
						line_y,
						al_map_rgb(192, 192, 192), 1);
					al_draw_line(
						info_spots[i]->x - 8,
						line_y,
						info_spots[i]->x,
						info_spots[i]->y - info_spots[i]->size * 0.5 - font_h * 0.25,
						al_map_rgb(192, 192, 192), 1);
					al_draw_line(
						info_spots[i]->x + 8,
						line_y,
						info_spots[i]->x,
						info_spots[i]->y - info_spots[i]->size * 0.5 - font_h * 0.25,
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
				32 - icons_size * 0.5,
				scr_h - (32 + y_offset + icons_size * 0.5),
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
				leaders[l_nr]->health, leaders[l_nr]->max_health,
				icons_size * 0.5, true);
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
		size_t pikmin_in_party = leaders[current_leader]->party.size();
		for(size_t l=0; l<n_leaders; l++){
			//If this leader is following the current one, then he's not a Pikmin, subtract him from the party count total.
			if(leaders[l]->following_party == leaders[current_leader]) pikmin_in_party--;
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
				if(leader_ptr == leaders[0]) bm = bmp_olimar;
				else if(leader_ptr == leaders[1]) bm = bmp_louie;
				else if(leader_ptr == leaders[2]) bm = bmp_president;
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
		
	}else{ //Paused.
				
	}
			
	al_flip_display();
}
