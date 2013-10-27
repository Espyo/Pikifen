#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "const.h"
#include "controls.h"
#include "functions.h"
#include "vars.h"

void handle_controls(ALLEGRO_EVENT ev){
	//Debugging.
	if(ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_T){
		//Debug testing.
		//ToDo remove.
		//leaders[current_leader]->health--;
		day_minutes += 60;
		day = sectors[0].floors[0].brightness;
	}


	if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
		if(ev.keyboard.keycode == ALLEGRO_KEY_W)           handle_button_down(BUTTON_MOVE_UP);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_A)      handle_button_down(BUTTON_MOVE_LEFT);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_S)      handle_button_down(BUTTON_MOVE_DOWN);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_D)      handle_button_down(BUTTON_MOVE_RIGHT);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_TAB)    handle_button_down(BUTTON_SWITCH_CAPTAIN_R);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_LCTRL)  handle_button_down(BUTTON_DISMISS);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_R)      {handle_button_down(BUTTON_USE_SPRAY_1); handle_button_down(BUTTON_USE_SPRAY);}
		else if(ev.keyboard.keycode == ALLEGRO_KEY_F)      handle_button_down(BUTTON_USE_SPRAY_2);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_Q)      handle_button_down(BUTTON_SWITCH_SPRAY_L);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_E)      handle_button_down(BUTTON_SWITCH_SPRAY_R);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_C)      handle_button_down(BUTTON_ZOOM_SWITCH);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_SPACE)  handle_button_down(BUTTON_MOVE_GROUP_TO_CURSOR);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) handle_button_down(BUTTON_PAUSE);

		else if(ev.keyboard.keycode == ALLEGRO_KEY_UP)     handle_button_down(BUTTON_MOVE_CURSOR_UP);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT)   handle_button_down(BUTTON_MOVE_CURSOR_LEFT);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN)   handle_button_down(BUTTON_MOVE_CURSOR_DOWN);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)  handle_button_down(BUTTON_MOVE_CURSOR_RIGHT);

	}else if(ev.type == ALLEGRO_EVENT_KEY_UP){
		if(ev.keyboard.keycode == ALLEGRO_KEY_W)           handle_button_up(BUTTON_MOVE_UP);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_A)      handle_button_up(BUTTON_MOVE_LEFT);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_S)      handle_button_up(BUTTON_MOVE_DOWN);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_D)      handle_button_up(BUTTON_MOVE_RIGHT);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_SPACE)  handle_button_up(BUTTON_MOVE_GROUP_TO_CURSOR);

		else if(ev.keyboard.keycode == ALLEGRO_KEY_UP)     handle_button_up(BUTTON_MOVE_CURSOR_UP);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT)   handle_button_up(BUTTON_MOVE_CURSOR_LEFT);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN)   handle_button_up(BUTTON_MOVE_CURSOR_DOWN);
		else if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)  handle_button_up(BUTTON_MOVE_CURSOR_RIGHT);

	}else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES){
		handle_mouse(AXIS_ACTION_MOVE_CURSOR, ev.mouse.x, ev.mouse.y);

		if(ev.mouse.dz > 0)      handle_button_down(BUTTON_ZOOM_IN, ev.mouse.dz);
		else if(ev.mouse.dz < 0) handle_button_down(BUTTON_ZOOM_OUT, -ev.mouse.dz);

	}else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
		if(ev.mouse.button == 1)      handle_button_down(BUTTON_PUNCH);
		else if(ev.mouse.button == 2) handle_button_down(BUTTON_WHISTLE);

	}else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){
		if(ev.mouse.button == 1)      handle_button_up(BUTTON_PUNCH);
		else if(ev.mouse.button == 2) handle_button_up(BUTTON_WHISTLE);

	}else if(ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS){
		if(ev.joystick.stick == 0 && ev.joystick.axis == 0){
			handle_analog(AXIS_ACTION_MOVE, ev.joystick.pos, true);
			handle_analog(AXIS_ACTION_MOVE_CURSOR, ev.joystick.pos, true);

		}else if(ev.joystick.stick == 0 && ev.joystick.axis == 1){
			handle_analog(AXIS_ACTION_MOVE, ev.joystick.pos, false);
			handle_analog(AXIS_ACTION_MOVE_CURSOR, ev.joystick.pos, false);

		}else if(ev.joystick.stick == 1 && ev.joystick.axis == 0){
			handle_analog(AXIS_ACTION_MOVE_GROUP, ev.joystick.pos, true);

		}else if(ev.joystick.stick == 0 && ev.joystick.axis == 2){
			handle_analog(AXIS_ACTION_MOVE_GROUP, ev.joystick.pos, false);

		}else if(ev.joystick.stick == 2 && ev.joystick.axis == 1){
			if(ev.joystick.pos == -1) handle_button_down(BUTTON_USE_SPRAY_1);
			else if(ev.joystick.pos == 1) handle_button_down(BUTTON_USE_SPRAY_2);
		}

	}else if(ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN){
		if(ev.joystick.button == 0) handle_button_down(BUTTON_DISMISS);
		if(ev.joystick.button == 1) handle_button_down(BUTTON_PUNCH);
		if(ev.joystick.button == 2) handle_button_down(BUTTON_WHISTLE);
		if(ev.joystick.button == 3) handle_button_down(BUTTON_SWITCH_CAPTAIN_R);
		if(ev.joystick.button == 5) handle_button_down(BUTTON_ZOOM_SWITCH);
		if(ev.joystick.button == 9) handle_button_down(BUTTON_PAUSE);

	}else if(ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP){
		if(ev.joystick.button == 0) handle_button_up(BUTTON_DISMISS);
		if(ev.joystick.button == 1) handle_button_up(BUTTON_PUNCH);
		if(ev.joystick.button == 2) handle_button_up(BUTTON_WHISTLE);
		if(ev.joystick.button == 3) handle_button_up(BUTTON_SWITCH_CAPTAIN_R);
		if(ev.joystick.button == 5) handle_button_up(BUTTON_ZOOM_SWITCH);

	}
}

void handle_button_down(unsigned int button, int aux){
	if(
		button == BUTTON_MOVE_RIGHT ||
		button == BUTTON_MOVE_UP ||
		button == BUTTON_MOVE_LEFT ||
		button == BUTTON_MOVE_DOWN
		){

			/*******************
			*              \O/ *
			*   Move   ---> |  *
			*              / \ *
			*******************/

			if(button == BUTTON_MOVE_RIGHT)     leaders[current_leader]->speed_x = LEADER_MOVE_SPEED;
			else if(button == BUTTON_MOVE_UP)   leaders[current_leader]->speed_y = -LEADER_MOVE_SPEED;
			else if(button == BUTTON_MOVE_LEFT) leaders[current_leader]->speed_x = -LEADER_MOVE_SPEED;
			else if(button == BUTTON_MOVE_DOWN) leaders[current_leader]->speed_y = LEADER_MOVE_SPEED;

	}else if(
		button == BUTTON_MOVE_CURSOR_RIGHT ||
		button == BUTTON_MOVE_CURSOR_UP ||
		button == BUTTON_MOVE_CURSOR_LEFT ||
		button == BUTTON_MOVE_CURSOR_DOWN
		){
			/********************
			*             .-.   *
			*   Cursor   ( = )> *
			*             `-´   *
			********************/

			if(button == BUTTON_MOVE_CURSOR_RIGHT)     mouse_cursor_speed_x = (1.0/game_fps) * MOUSE_CURSOR_MOVE_SPEED;
			else if(button == BUTTON_MOVE_CURSOR_UP)   mouse_cursor_speed_y = -(1.0/game_fps) * MOUSE_CURSOR_MOVE_SPEED;
			else if(button == BUTTON_MOVE_CURSOR_LEFT) mouse_cursor_speed_x = -(1.0/game_fps) * MOUSE_CURSOR_MOVE_SPEED;
			else if(button == BUTTON_MOVE_CURSOR_DOWN) mouse_cursor_speed_y = (1.0/game_fps) * MOUSE_CURSOR_MOVE_SPEED;

	}else if(button == BUTTON_PUNCH){

		/*******************
		*            .--._ *
		*   Punch   ( U  _ *
		*            `--´  *
		*******************/

		bool done = false;
				
		//First check if the leader should pluck a Pikmin.

		float closest_distance = 0;
		pikmin* closest_pikmin = NULL;
		size_t n_pikmin = pikmin_list.size();
		for(size_t p=0; p<n_pikmin; p++){
			if(!pikmin_list[p]->burrowed) continue;
			float d = dist(leaders[current_leader]->x, leaders[current_leader]->y, pikmin_list[p]->x, pikmin_list[p]->y);
			if(closest_pikmin == NULL || d < closest_distance){
				closest_distance = d;
				closest_pikmin = pikmin_list[p];
			}
		}

		if(closest_pikmin){
			if(closest_distance <= MIN_PLUCK_RANGE){
				//Pluck.
				closest_pikmin->burrowed = false;
				add_to_party(leaders[current_leader], closest_pikmin);
				al_play_sample(sfx_pikmin_plucked.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &sfx_pikmin_plucked.id);
				done = true;
			}
		}

		//Now check if the leader should open an onion's menu.

		if(!done){
			//ToDo
			size_t n_onions = onions.size();
			for(size_t o=0; o<n_onions; o++){
				if(dist(leaders[current_leader]->x, leaders[current_leader]->y, onions[o]->x, onions[o]->y) < MIN_ONION_CHECK_RANGE){
					//ToDo this is not how it works, there can be less onions on the field than the total number of Pikmin types.
					pikmin_in_onions[o]--;
					pikmin_list.push_back(new pikmin(onions[o]->type, onions[o]->x, onions[o]->y, onions[o]->sec));
					add_to_party(leaders[current_leader], pikmin_list[pikmin_list.size()-1]);
					done = true;
				}
			}
		}
		
		//Now check if the leader should heal themselves on the ship.
		if(!done){
			size_t n_ships = ships.size();
			for(size_t s=0; s<n_ships; s++){
				if(dist(leaders[current_leader]->x, leaders[current_leader]->y, ships[s]->x + ships[s]->size / 2 + SHIP_BEAM_RANGE, ships[s]->y) < SHIP_BEAM_RANGE){
					//ToDo make it prettier.
					leaders[current_leader]->health = leaders[current_leader]->max_health;
					done = true;
				}
			}
		}

		//Now check if the leader should grab a Pikmin.

		if(!done){
			if(closest_party_member){
				leaders[current_leader]->holding_pikmin = closest_party_member;
				al_play_sample(sfx_pikmin_held.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &sfx_pikmin_held.id);
				done = true;
			}
		}

		//Now check if the leader should punch.

		if(!done){
			//ToDo
		}

	}else if(button == BUTTON_WHISTLE){

		/********************
		*              .--= *
		*   Whistle   ( @ ) *
		*              `-´  *
		********************/

		whistling = true;
		al_play_sample(leaders[current_leader]->sfx_whistle.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &leaders[current_leader]->sfx_whistle.id);

		for(unsigned char d=0; d<6; d++) whistle_dot_radius[d] = -1;
		whistle_fade_time = 0;
		whistle_fade_radius = 0;

	}else if(
		button == BUTTON_SWITCH_CAPTAIN_R ||
		button == BUTTON_SWITCH_CAPTAIN_L
		){

			/******************************
			*                    \O/  \O/ *
			*   Switch captain    | -> |  *
			*                    / \  / \ *
			******************************/

			size_t new_leader = current_leader;
			if(button == BUTTON_SWITCH_CAPTAIN_R)
				new_leader = (current_leader + 1) % leaders.size();
			else if(button == BUTTON_SWITCH_CAPTAIN_L){
				if(current_leader == 0) new_leader = leaders.size() - 1;
				else new_leader = current_leader - 1;
			}

			mob* swap_leader = NULL;

			if(!leaders[current_leader]->speed_z){
				leaders[current_leader]->speed_x = 0;
				leaders[current_leader]->speed_y = 0;
			}
			if(!leaders[new_leader]->speed_z){
				leaders[new_leader]->speed_x = 0;
				leaders[new_leader]->speed_y = 0;
			}
			leaders[new_leader]->remove_target(true);

			//If the new leader is in another one's party, swap them.
			size_t n_leaders = leaders.size();
			for(size_t l=0; l<n_leaders; l++){
				if(l==new_leader) continue;
				size_t n_party_members = leaders[l]->party.size();
				for(size_t m=0; m<n_party_members; m++){
					if(leaders[l]->party[m] == leaders[new_leader]){
						swap_leader=leaders[l];
						break;
					}
				}
		}

		if(swap_leader){
			size_t n_party_members = swap_leader->party.size();
			for(size_t m=0; m<n_party_members; m++){
				mob* member = swap_leader->party[0];
				remove_from_party(swap_leader, member);
				if(member != leaders[new_leader]){
					add_to_party(leaders[new_leader], member);
				}
			}
					
			add_to_party(leaders[new_leader], swap_leader);
		}

		current_leader = new_leader;
		start_camera_pan(leaders[new_leader]->x, leaders[new_leader]->y);
		al_play_sample(leaders[new_leader]->sfx_name_call.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &leaders[new_leader]->sfx_name_call.id);

	}else if(button == BUTTON_DISMISS){

		/***********************
		*             \O/ / *  *
		*   Dismiss    |   - * *
		*             / \ \ *  *
		***********************/

		leader* current_leader_ptr = leaders[current_leader];

		//ToDo what if there are a lot of Pikmin types?
		size_t n_party_members = current_leader_ptr->party.size();
		for(size_t m=0; m<n_party_members; m++){
			mob* member_ptr = current_leader_ptr->party[0];
			remove_from_party(current_leader_ptr, member_ptr);

			float angle = 0;

			if(typeid(*member_ptr) == typeid(pikmin)){
				pikmin* pikmin_ptr = dynamic_cast<pikmin*>(member_ptr);
				if(pikmin_ptr->type->name == "R"){
					angle = M_PI * 1.25;
				}else if(pikmin_ptr->type->name == "Y"){
					angle = M_PI;
				}else{
					angle = M_PI * 0.75;
				}

				angle+=current_leader_ptr->angle;
				if(moving_group_intensity > 0) angle+=M_PI; //If the leader's moving the group, they should be dismissed towards the cursor.

				member_ptr->set_target(
					current_leader_ptr->x + cos(angle) * DISMISS_DISTANCE,
					current_leader_ptr->y + sin(angle) * DISMISS_DISTANCE,
					NULL,
					NULL,
					false);
			}					
		}

		al_play_sample(leaders[current_leader]->sfx_dismiss.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &leaders[current_leader]->sfx_dismiss.id);

	}else if(button == BUTTON_PAUSE){

		/********************
		*           +-+ +-+ *
		*   Pause   | | | | *
		*           +-+ +-+ *
		********************/

		running = false; //ToDo menu, not quit.
		//paused = true;

	}else if(button == BUTTON_USE_SPRAY_1){

		/*******************
		*             +=== *
		*   Sprays   (   ) *
		*             `-´  *
		*******************/

		if(spray_types.size() == 1 || spray_types.size() == 2){
			use_spray(0);
		}

	}else if(button == BUTTON_USE_SPRAY_2){
		if(spray_types.size() == 2){
			use_spray(1);
		}

	}else if(button == BUTTON_SWITCH_SPRAY_R || button == BUTTON_SWITCH_SPRAY_L){
		if(spray_types.size() > 2){
			if(button == BUTTON_SWITCH_SPRAY_R){
				selected_spray = (selected_spray + 1) % spray_types.size();
			}else{
				if(selected_spray == 0) selected_spray = spray_types.size() - 1;
				else selected_spray--;
			}
		}

	}else if(button == BUTTON_USE_SPRAY){
		if(spray_types.size() > 2){
			use_spray(selected_spray);
		}

	}else if(button == BUTTON_ZOOM_SWITCH){

		/***************
		*           _  *
		*   Zoom   (_) *
		*          /   *
		***************/

		float new_zoom;
		float zoom_to_compare;
		if(cam_trans_zoom_time_left > 0) zoom_to_compare = cam_trans_zoom_final_level; else zoom_to_compare = cam_zoom;

		if(zoom_to_compare < 1){
			new_zoom = MAX_ZOOM_LEVEL;
		}else if(zoom_to_compare > 1){
			new_zoom = 1;
		}else{
			new_zoom = MIN_ZOOM_LEVEL;
		}

		start_camera_zoom(new_zoom);

	}else if(button == BUTTON_ZOOM_IN || button == BUTTON_ZOOM_OUT){

		if(aux == 0) aux = 1;
		if((cam_zoom == MAX_ZOOM_LEVEL && button == BUTTON_ZOOM_IN) || (cam_zoom == MIN_ZOOM_LEVEL && button == BUTTON_ZOOM_OUT)) return;

		float new_zoom;
		float current_zoom;
		if(cam_trans_zoom_time_left) current_zoom = cam_trans_zoom_final_level; else current_zoom = cam_zoom;

		if(button == BUTTON_ZOOM_IN) new_zoom = current_zoom + 0.1 * aux; else new_zoom = current_zoom - 0.1 * aux;
		
		if(new_zoom > MAX_ZOOM_LEVEL) new_zoom = MAX_ZOOM_LEVEL;
		if(new_zoom < MIN_ZOOM_LEVEL) new_zoom = MIN_ZOOM_LEVEL;
		
		if(cam_trans_zoom_time_left){
			cam_trans_zoom_final_level = new_zoom;
		}else{
			start_camera_zoom(new_zoom);
		}

	}else if(button == BUTTON_MOVE_GROUP_TO_CURSOR){

		/******************
		*            ***  *
		*   Group   ****O *
		*            ***  *
		******************/

		moving_group_to_cursor = true;
		moving_group_intensity = 1;

	}
}

void handle_button_up(unsigned int button){
	//ToDo change to angles.
	if(button == BUTTON_MOVE_UP || button == BUTTON_MOVE_DOWN){

		/*******************
		*              \O/ *
		*   Move   ---> |  *
		*              / \ *
		*******************/

		leaders[current_leader]->speed_y = 0;

	}else if(button == BUTTON_MOVE_LEFT || button == BUTTON_MOVE_RIGHT){
		leaders[current_leader]->speed_x = 0;

	}else if(button == BUTTON_MOVE_CURSOR_UP || button == BUTTON_MOVE_CURSOR_DOWN){

		/********************
		*             .-.   *
		*   Cursor   ( = )> *
		*             `-´   *
		********************/

		mouse_cursor_speed_y = 0;

	}else if(button == BUTTON_MOVE_CURSOR_LEFT || button == BUTTON_MOVE_CURSOR_RIGHT){
		mouse_cursor_speed_x = 0;

	}else if(button == BUTTON_MOVE_GROUP_TO_CURSOR){

		/******************
		*            ***  *
		*   Group   ****O *
		*            ***  *
		******************/

		moving_group_to_cursor = false;
		moving_group_intensity = 0;

	}else if(button == BUTTON_PUNCH){

		/*******************
		*            .--._ *
		*   Punch   ( U  _ *
		*            `--´  *
		*******************/

		mob* holding_ptr = leaders[current_leader]->holding_pikmin;
		if(holding_ptr){

			holding_ptr->x = leaders[current_leader]->x;
			holding_ptr->y = leaders[current_leader]->y;
			holding_ptr->z = leaders[current_leader]->z;

			float d = dist(leaders[current_leader]->x, leaders[current_leader]->y, cursor_x, cursor_y);
			holding_ptr->speed_x = cos(leaders[current_leader]->angle) * d * THROW_DISTANCE_MULTIPLIER;
			holding_ptr->speed_y = sin(leaders[current_leader]->angle) * d * THROW_DISTANCE_MULTIPLIER;
			holding_ptr->speed_z = 2;

			holding_ptr->was_thrown = true;
				
			remove_from_party(leaders[current_leader], holding_ptr);
			leaders[current_leader]->holding_pikmin = NULL;

			al_stop_sample(&sfx_pikmin_held.id);
			al_stop_sample(&sfx_pikmin_thrown.id);
			al_stop_sample(&sfx_throw.id);
			al_play_sample(sfx_pikmin_thrown.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &sfx_pikmin_thrown.id);
			al_play_sample(sfx_throw.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &sfx_throw.id);
		}

	}else if(button == BUTTON_WHISTLE){

		/********************
		*              .--= *
		*   Whistle   ( @ ) *
		*              `-´  *
		********************/

		stop_whistling();

	}
}

void handle_analog(unsigned int action, float pos, bool x_axis){
	if(action == AXIS_ACTION_MOVE){
		/*******************
		*              \O/ *
		*   Move   ---> |  *
		*              / \ *
		*******************/

		if(fabs(pos) < 0.75) pos = 0;

		if(x_axis){
			leaders[current_leader]->speed_x = pos * LEADER_MOVE_SPEED;
		}else{
			leaders[current_leader]->speed_y = pos * LEADER_MOVE_SPEED;
		}

	}else if(action == AXIS_ACTION_MOVE_CURSOR){
		/********************
		*             .-.   *
		*   Cursor   ( = )> *
		*             `-´   *
		********************/

		if(x_axis){
			mouse_cursor_speed_x = pos * (1.0/game_fps) * MOUSE_CURSOR_MOVE_SPEED;
		}else{
			mouse_cursor_speed_y = pos * (1.0/game_fps) * MOUSE_CURSOR_MOVE_SPEED;
		}

	}else if(action == AXIS_ACTION_MOVE_GROUP){
		/******************
		*            ***  *
		*   Group   ****O *
		*            ***  *
		******************/

		if(x_axis){
			moving_group_pos_x = pos;
		}else{
			moving_group_pos_y = pos;
		}

	}
}

void handle_mouse(unsigned int action, float mx, float my){
	if(action == AXIS_ACTION_MOVE_CURSOR){

		/********************
		*             .-.   *
		*   Cursor   ( = )> *
		*             `-´   *
		********************/

		mouse_cursor_x = mx;
		mouse_cursor_y = my;

	}

}
