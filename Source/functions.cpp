#include <math.h>
#define _USE_MATH_DEFINES

#include <allegro5\allegro.h>
#include <allegro5\allegro_primitives.h>

#include "const.h"
#include "functions.h"
#include "vars.h"

void add_to_party(mob* party_leader, mob* new_member){
	if(new_member->following_party == party_leader) return;	//Already following, never mind.

	new_member->following_party = party_leader;
	new_member->go_to_target = true;
	party_leader->party.push_back(new_member);
}

void angle_to_coordinates(float angle, float magnitude, float* x_coord, float* y_coord){
	*x_coord = cos(angle) * magnitude;
	*y_coord = sin(angle) * magnitude;
}

void coordinates_to_angle(float x_coord, float y_coord, float* angle, float* magnitude){
	*angle = atan2(y_coord, x_coord);
	*magnitude = dist(0, 0, x_coord, y_coord);
}

void draw_health(float cx, float cy, unsigned int health, unsigned int max_health, bool just_chart){
	float ratio = (float) health / (float) max_health;
	ALLEGRO_COLOR c;
	if(ratio >= 0.5){
		c=al_map_rgb_f(1-(ratio-0.5)*2, 1, 0);
	}else{
		c=al_map_rgb_f(1, (ratio*2), 0);
	}

	if(!just_chart) al_draw_filled_circle(cx, cy, HEALTH_CIRCLE_RADIUS, al_map_rgba(0, 0, 0, 128));
	al_draw_filled_pieslice(cx, cy, HEALTH_CIRCLE_RADIUS, -M_PI*0.5, -ratio*M_PI*2, c);
	if(!just_chart) al_draw_circle(cx, cy, 11, al_map_rgb(0, 0, 0), 2);
}

void load_game_content(){
	//ToDo.
	pikmin_types.push_back(pikmin_type());
	pikmin_types.back().color = al_map_rgb(255, 0, 0);
	pikmin_types.back().name = "R";
	pikmin_types.back().max_move_speed = 50;

	pikmin_types.push_back(pikmin_type());
	pikmin_types.back().color = al_map_rgb(255, 255, 0);
	pikmin_types.back().name = "Y";
	pikmin_types.back().max_move_speed = 50;

	pikmin_types.push_back(pikmin_type());
	pikmin_types.back().color = al_map_rgb(0, 0, 255);
	pikmin_types.back().name = "B";
	pikmin_types.back().max_move_speed = 50;
}

void random_particle_explosion(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color){
	unsigned char n_particles = random(min, max);
	
	for(unsigned char p=0; p<n_particles; p++){
		float angle = (random(0, (unsigned) (M_PI * 2) * 100)) / 100.0;
		
		float speed_x = cos(angle) * 30;
		float speed_y = sin(angle) * 30;

		particles.push_back(particle(
			center_x,
			center_y,
			speed_x,
			speed_y,
			1,
			0,
			(random((unsigned) (time_min*100), (unsigned) (time_max*100)))/100.0,
			(random((unsigned) (size_min*100), (unsigned) (size_max*100)))/100.0,
			color
			));
	}
}

void random_particle_splash(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color){
	unsigned char n_particles = random(min, max);
	
	for(unsigned char p=0; p<n_particles; p++){
		particles.push_back(particle(
			center_x,
			center_y,
			(2-random(0,4)),
			-random(2, 4),
			0, 0.5,
			random(14, 20)/10,
			4,
			color
		));
	}
}

void remove_from_party(mob* party_leader, mob* member_to_remove){
	size_t n_pikmin = party_leader->party.size();
	for(size_t p=0; p<n_pikmin; p++){
		if(party_leader->party[p] == member_to_remove){
			party_leader->party.erase(party_leader->party.begin() + p);
			break;
		}
	}
	member_to_remove->following_party = NULL;
	member_to_remove->go_to_target = false;
}

void stop_whistling(){
	whistling = false;
	whistle_radius = 0;
	whistle_max_hold = 0;
}