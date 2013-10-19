#include "const.h"
#include "functions.h"
#include "vars.h"

void do_logic(){

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
	if(moving_group_intensity){
		move_group_next_arrow_time-=1.0/game_fps;
		if(move_group_next_arrow_time<=0){
			move_group_next_arrow_time = MOVE_GROUP_ARROWS_INTERVAL;
			move_group_arrows.push_back(0);
		}
	}

	float dis = dist(leaders[current_leader]->x, leaders[current_leader]->y, cursor_x, cursor_y) * moving_group_intensity;
	for(size_t a=0; a<move_group_arrows.size(); ){
		move_group_arrows[a]+=MOVE_GROUP_ARROW_SPEED * (1.0/game_fps);
		if(move_group_arrows[a] >= dis){
			move_group_arrows.erase(move_group_arrows.begin() + a);
		}else{
			a++;
		}
	}

	//Whistle animations.
	whistle_dot_offset-=WHISTLE_DOT_SPIN_SPEED * (1.0/game_fps);

	if(whistle_fade_time > 0){	
		whistle_fade_time-=1.0/game_fps;
		if(whistle_fade_time<0) whistle_fade_time = 0;
	}

	if(whistling){
		//Create rings.
		whistle_next_ring_time-=1.0/game_fps;
		if(whistle_next_ring_time<=0){
			whistle_next_ring_time = WHISTLE_RINGS_INTERVAL;
			whistle_rings.push_back(0);
			whistle_ring_colors.push_back(whistle_ring_prev_color);
			whistle_ring_prev_color = (whistle_ring_prev_color + 1) % N_WHISTLE_RING_COLORS;
		}

		whistle_next_dot_time-=1.0/game_fps;
		if(whistle_next_dot_time<=0){
			whistle_next_dot_time=WHISTLE_DOT_INTERVAL;
			unsigned char dot=255;
			for(unsigned char d=0; d<6; d++){ //Find WHAT dot to add.
				if(whistle_dot_radius[d]==-1){ dot=d; break;}
			}

			if(dot!=255) whistle_dot_radius[dot]=0;
		}
	}

	for(unsigned char d=0; d<6; d++){
		if(whistle_dot_radius[d]==-1) continue;

		whistle_dot_radius[d] += (1.0 / game_fps) * WHISTLE_RADIUS_GROWTH_PS;
		if(whistle_radius > 0 && whistle_dot_radius[d] > MAX_WHISTLE_RADIUS){
			whistle_dot_radius[d] = MAX_WHISTLE_RADIUS;
		}else if(whistle_fade_radius > 0 && whistle_dot_radius[d] > whistle_fade_radius){
			whistle_dot_radius[d] = whistle_fade_radius;
		}
	}


	dis = dist(leaders[current_leader]->x, leaders[current_leader]->y, cursor_x, cursor_y);
	for(size_t r=0; r<whistle_rings.size(); ){
		//Erase rings that go beyond the cursor.
		whistle_rings[r]+=WHISTLE_RING_SPEED * (1.0/game_fps);
		if(whistle_rings[r] >= dis){
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
		pikmin* pik_ptr = pikmin_list[p];

		bool can_be_called = 
			!pik_ptr->following_party &&
			!pik_ptr->burrowed &&
			!pik_ptr->speed_z &&
			!pik_ptr->uncallable_period;
		bool whistled = (dist(pik_ptr->x, pik_ptr->y, cursor_x, cursor_y) <= whistle_radius && whistling);
		bool touched = dist(pik_ptr->x, pik_ptr->y, leaders[current_leader]->x, leaders[current_leader]->y) <= pik_ptr->size * 0.5 + leaders[current_leader]->size * 0.5;
		bool is_busy = (pik_ptr->carrying_treasure || pik_ptr->enemy_attacking);

		if(can_be_called && (whistled || (touched && !is_busy))){

			//Pikmin got whistled or touched.
			add_to_party(leaders[current_leader], pik_ptr);
			al_stop_sample(&sfx_pikmin_called.id);
			al_play_sample(sfx_pikmin_called.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &sfx_pikmin_called.id);

			pik_ptr->enemy_attacking = NULL;

			drop_treasure(pik_ptr);
							
		}

		//Following party.
		if(pik_ptr->following_party){
			float move_x, move_y;
			angle_to_coordinates(moving_group_angle, moving_group_intensity * CURSOR_MAX_DIST * 0.5, &move_x, &move_y);

			pik_ptr->set_target(
				random(0, 60) - 30 + move_x,
				random(0, 30) + move_y,
				&pik_ptr->following_party->x,
				&pik_ptr->following_party->y,
				false);
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
					if(dist(pik_ptr->x, pik_ptr->y, nectars[n]->x, nectars[n]->y) <= nectars[n]->size + pik_ptr->size){
						if(nectars[n]->amount_left > 0)
							nectars[n]->amount_left--;

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
			(pik_ptr->following_party && moving_group_intensity)
			){
				for(size_t t=0; t<n_treasures; t++){
					if(dist(pik_ptr->x, pik_ptr->y, treasures[t]->x, treasures[t]->y)<=pik_ptr->size + treasures[t]->size + MIN_PIKMIN_TASK_RANGE){
						//ToDo don't take the treasure if all spots are taken already.
						pik_ptr->carrying_treasure = treasures[t];
								
						if(pik_ptr->following_party) remove_from_party(pik_ptr->following_party, pik_ptr);

						//ToDo remove this random cycle and replace with something more optimal.
						bool valid_spot = false;
						unsigned int spot = 0;
						while(!valid_spot){
							spot = random(0, treasures[t]->max_carriers - 1);
							valid_spot = !treasures[t]->carrier_info->carrier_spots[spot];
						}
								
						treasures[t]->carrier_info->carrier_spots[spot] = pik_ptr;
						treasures[t]->carrier_info->current_n_carriers++;

						pik_ptr->carrying_spot = spot;
						pik_ptr->set_target(
							treasures[t]->carrier_info->carrier_spots_x[spot],
							treasures[t]->carrier_info->carrier_spots_y[spot],
							&treasures[t]->x,
							&treasures[t]->y,
							true);

						if(treasures[t]->carrier_info->current_n_carriers >= treasures[t]->weight){
							//Start moving the treasure.
							treasures[t]->set_target(
								0,
								0,
								NULL,
								NULL,
								false);
						}

						break;
					}
				}
		}

		pik_ptr->tick();

	}


	/********************
	*              .-.  *
	*   Leaders   (*:O) *
	*              `-´  *
	********************/

	if(leaders[current_leader]->holding_pikmin){
		leaders[current_leader]->holding_pikmin->x = leaders[current_leader]->x+8;
		leaders[current_leader]->holding_pikmin->y = leaders[current_leader]->y;
	}

	size_t n_leaders = leaders.size();
	for(size_t l=0; l<n_leaders; l++){
		if(whistling){
			if(l != current_leader){
				if(
					dist(leaders[l]->x, leaders[l]->y, cursor_x, cursor_y) <= whistle_radius &&
					!leaders[l]->following_party &&
					!leaders[l]->was_thrown){
					//Leader got whistled.
					add_to_party(leaders[current_leader], leaders[l]);

					size_t n_party_members = leaders[l]->party.size();
					for(size_t m=0; m<n_party_members; m++){
						mob* member = leaders[l]->party[0];
						remove_from_party(leaders[l], member);
						add_to_party(leaders[current_leader], member);
					}
				}
			}
		}

		if(leaders[l]->following_party){
			leaders[l]->set_target(
				0,
				30,
				&leaders[l]->following_party->x,
				&leaders[l]->following_party->y,
				false);
		}
				
		leaders[l]->tick();
	}

	if(cam_trans_pan_time_left > 0){
		cam_trans_pan_final_x = leaders[current_leader]->x;
		cam_trans_pan_final_y = leaders[current_leader]->y;
	}else{
		cam_x = leaders[current_leader]->x;
		cam_y = leaders[current_leader]->y;
	}

	/******************
	*            ***  *
	*   Group   ****O *
	*            ***  *
	******************/

	float closest_distance = 0;
	size_t n_members = leaders[current_leader]->party.size();
	closest_party_member = leaders[current_leader]->holding_pikmin;

	if(n_members > 0 && !closest_party_member){

		for(size_t m=0; m<n_members; m++){
			float d = dist(leaders[current_leader]->x, leaders[current_leader]->y, leaders[current_leader]->party[m]->x, leaders[current_leader]->party[m]->y);
			if(m==0 || d < closest_distance){
				closest_distance = d;
				closest_party_member = leaders[current_leader]->party[m];
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

	mouse_cursor_x += mouse_cursor_speed_x;
	mouse_cursor_y += mouse_cursor_speed_y;

	float mcx = mouse_cursor_x, mcy = mouse_cursor_y;
	ALLEGRO_TRANSFORM world_to_screen_transform = get_world_to_screen_transform();
	ALLEGRO_TRANSFORM screen_to_world_transform = world_to_screen_transform;
	al_invert_transform(&screen_to_world_transform);
	al_transform_coordinates(&screen_to_world_transform, &mcx, &mcy);
	cursor_x = mcx;
	cursor_y = mcy;

	leaders[current_leader]->angle = atan2(cursor_y - leaders[current_leader]->y, cursor_x - leaders[current_leader]->x);
	dis = dist(leaders[current_leader]->x, leaders[current_leader]->y, cursor_x, cursor_y);
	if(dis > CURSOR_MAX_DIST){
		//Cursor goes beyond the range limit.
		cursor_x = leaders[current_leader]->x + (cos(leaders[current_leader]->angle) * CURSOR_MAX_DIST);
		cursor_y = leaders[current_leader]->y + (sin(leaders[current_leader]->angle) * CURSOR_MAX_DIST);

		if(mouse_cursor_speed_x != 0 || mouse_cursor_speed_y != 0){
			//If we're speeding the mouse cursor (via analog stick), don't let it go beyond the edges.
			mouse_cursor_x = cursor_x;
			mouse_cursor_y = cursor_y;
			al_transform_coordinates(&world_to_screen_transform, &mouse_cursor_x, &mouse_cursor_y);
		}
	}

	if(moving_group_to_cursor){
		moving_group_angle = leaders[current_leader]->angle;
		moving_group_intensity = dis / CURSOR_MAX_DIST;
		if(moving_group_intensity > 1) moving_group_intensity = 1;
	}


	/**********************
	*                .-.  *
	*   Treasures   ($$$) *
	*                `-´  *
	**********************/

	for(size_t t=0; t<n_treasures; t++){
		treasures[t]->tick();
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
		if(leaders[l]->was_thrown)
			random_particle_fire(leaders[l]->x, leaders[l]->y, 1, 1, 0.3, 0.5, 3, 4, change_alpha(leaders[l]->main_color, 192));
	}

	for(size_t p = 0; p < n_pikmin; p++){
		if(pikmin_list[p]->was_thrown)
			random_particle_fire(pikmin_list[p]->x, pikmin_list[p]->y, 1, 1, 0.3, 0.5, 3, 4, change_alpha(pikmin_list[p]->main_color, 192));
	}
}