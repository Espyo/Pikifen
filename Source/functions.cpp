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

ALLEGRO_COLOR change_alpha(ALLEGRO_COLOR c, unsigned char a){
	ALLEGRO_COLOR c2;
	c2.r = c.r; c2.g = c.g; c2.b = c.b;
	c2.a = a / 255.0;
	return c2;
}

void coordinates_to_angle(float x_coord, float y_coord, float* angle, float* magnitude){
	*angle = atan2(y_coord, x_coord);
	*magnitude = dist(0, 0, x_coord, y_coord);
}

void draw_health(float cx, float cy, unsigned int health, unsigned int max_health, float radius, bool just_chart){
	float ratio = (float) health / (float) max_health;
	ALLEGRO_COLOR c;
	if(ratio >= 0.5){
		c=al_map_rgb_f(1-(ratio-0.5)*2, 1, 0);
	}else{
		c=al_map_rgb_f(1, (ratio*2), 0);
	}

	if(!just_chart) al_draw_filled_circle(cx, cy, radius, al_map_rgba(0, 0, 0, 128));
	al_draw_filled_pieslice(cx, cy, radius, -M_PI*0.5, -ratio*M_PI*2, c);
	if(!just_chart) al_draw_circle(cx, cy, radius + 1, al_map_rgb(0, 0, 0), 2);
}

void draw_shadow(float cx, float cy, float size, float delta_z, float shadow_stretch){
	if(shadow_stretch <= 0) return;

	int shadow_x = 0, shadow_w = size + (size * 3 * shadow_stretch);

	if(day_minutes < 60*12){
		//Shadows point to the West.
		shadow_x = -shadow_w + size / 2;
	}else{
		//Shadows point to the East.
		shadow_x = -(size / 2);
	}
	
	//ToDo shadow graphic dimensions.
	al_draw_tinted_scaled_bitmap(
		bmp_shadow,
		al_map_rgba(255, 255, 255, 255*(1-shadow_stretch)),
		0,
		0,
		64,
		64,
		cx + shadow_x,
		(cy - size / 2) + delta_z * SHADOW_Y_MULTIPLIER,
		shadow_w,
		size,
		0
		);
}

ALLEGRO_COLOR get_daylight_color(){
	//ToDo initialize this somewhere else.
	vector<pair<unsigned char, ALLEGRO_COLOR>> points;
	/*points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 0,  al_map_rgba(0,   0,   32,  192) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 3,  al_map_rgba(0,   0,   32,  192) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 4,  al_map_rgba(64,  64,  96,  128) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 7,  al_map_rgba(255, 128, 255, 24 ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 8,  al_map_rgba(255, 255, 255, 0  ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 18, al_map_rgba(255, 255, 255, 0  ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 17, al_map_rgba(255, 255, 0,   32 ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 18, al_map_rgba(255, 255, 128, 32 ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 19, al_map_rgba(128, 32,  0,   48 ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 20, al_map_rgba(32,  0,   0,   64 ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 21, al_map_rgba(0,   0,   32,  96 ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 23, al_map_rgba(0,   0,   32,  192) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 24, al_map_rgba(0,   0,   32,  192) ));*/

	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 0,  al_map_rgba(0,   0,   32,  192) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 5,  al_map_rgba(0,   0,   32,  192) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 6,  al_map_rgba(64,  64,  96,  128) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 7,  al_map_rgba(255, 128, 255, 24 ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 8,  al_map_rgba(255, 255, 255, 0  ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 17, al_map_rgba(255, 255, 255, 0  ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 18, al_map_rgba(255, 128, 0, 32 ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 19, al_map_rgba(0,   0,   32,  96 ) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 20, al_map_rgba(0,   0,   32,  192) ));
	points.push_back(make_pair<unsigned char, ALLEGRO_COLOR>( 24, al_map_rgba(0,   0,   32,  192) ));

	size_t n_points = points.size();
	for(size_t p = 0; p < n_points - 1; p++){
		if(day_minutes >= 60 * points[p].first && day_minutes < 60 * points[p+1].first){
			return interpolate_color(day_minutes, 60 * points[p].first, 60 * points[p+1].first, points[p].second, points[p+1].second);
		}
	}

	return al_map_rgba(255, 0, 0, 128);

	/*if(day_minutes < 60*3 || day_minutes > 60*23){
		return al_map_rgba(0, 0, 32, 192);

	}else if(day_minutes >= 60*3 && day_minutes < 60*4){
		return interpolate_color(day_minutes, 60*3, 60*4, al_map_rgba(0, 0, 32, 192), al_map_rgba(

	}else if(day_minutes >= 60*3 && day_minutes < 60*7){
		return interpolate_color(day_minutes, 60*3, 60*7, al_map_rgba(0, 0, 32, 192), al_map_rgba(255, 128, 255, 24));

	}else if(day_minutes >= 60*7 && day_minutes < 60*8){
		return interpolate_color(day_minutes, 60*7, 60*8, al_map_rgba(255, 128, 255, 24), al_map_rgba(255, 255, 255, 0));

	}else if(day_minutes >= 60*8 && day_minutes < 60*16){
		return al_map_rgba(255, 255, 255, 0);

	}else if(day_minutes >= 60*16 && day_minutes < 60*17){
		return interpolate_color(day_minutes, 60*16, 60*17, al_map_rgba(

	}*/
}

ALLEGRO_COLOR interpolate_color(float n, float n1, float n2, ALLEGRO_COLOR c1, ALLEGRO_COLOR c2){
	float progress = (float) (n - n1) / (float) (n2 - n1);
	return al_map_rgba_f(
		c1.r + progress * (c2.r - c1.r),
		c1.g + progress * (c2.g - c1.g),
		c1.b + progress * (c2.b - c1.b),
		c1.a + progress * (c2.a - c1.a)
		);
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

	spray_types.push_back(spray_type(true, al_map_rgb(255, 0, 0)));
	spray_types.push_back(spray_type(false, al_map_rgb(128, 0, 255)));
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

void random_particle_fire(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color){
	unsigned char n_particles = random(min, max);
	
	for(unsigned char p=0; p<n_particles; p++){
		particles.push_back(particle(
			center_x,
			center_y,
			(6-random(0, 12)),
			-(random(10, 20)),
			-1,
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