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

void draw_fraction(float cx, float cy, unsigned int current, unsigned int needed, ALLEGRO_COLOR color){
	unsigned int first_y = cy - (font_h * 3) / 2;
	al_draw_text(font, color, cx, first_y, ALLEGRO_ALIGN_CENTER, (to_string((long long) current).c_str()));
	al_draw_text(font, color, cx, first_y + font_h * 2, ALLEGRO_ALIGN_CENTER, (to_string((long long) needed).c_str()));

	ALLEGRO_TRANSFORM scale, old;
	al_copy_transform(&old, al_get_current_transform());
	al_copy_transform(&scale, &old);
	al_translate_transform(&scale, cx / 5, 0);
	al_scale_transform(&scale, 5, 1);
	al_use_transform(&scale);{
		al_draw_text(font, color, 0, first_y + font_h, ALLEGRO_ALIGN_CENTER, "-");
	};al_use_transform(&old);
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
	static vector<pair<unsigned char, ALLEGRO_COLOR>> points;

	size_t n_points = points.size();
	if(n_points == 0){
		//This way, this vector is only created once.
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
	}

	for(size_t p = 0; p < n_points - 1; p++){
		if(day_minutes >= 60 * points[p].first && day_minutes < 60 * points[p+1].first){
			return interpolate_color(day_minutes, 60 * points[p].first, 60 * points[p+1].first, points[p].second, points[p+1].second);
		}
	}

	//If anything goes wrong, the player will see a strong red tint. This is a very obvious indicator of an error.
	return al_map_rgba(255, 0, 0, 128);
}

ALLEGRO_TRANSFORM get_world_to_screen_transform(){
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	al_translate_transform(
		&t,
		-cam_x + scr_w / 2 * 1/(cam_zoom),
		-cam_y + scr_h/2 * 1/(cam_zoom)
		);
	al_scale_transform(&t, cam_zoom, cam_zoom);
	return t;
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

	statuses.push_back(status(0, 0, 1, 0, true, al_map_rgb(128, 0, 255), STATUS_AFFECTS_ENEMIES));
	statuses.push_back(status(1.5, 1.5, 1, 1, false, al_map_rgb(255, 64, 64), STATUS_AFFECTS_PIKMIN));

	spray_types.push_back(spray_type(&statuses[0], false, 10, al_map_rgb(128, 0, 255), NULL, NULL));
	spray_types.push_back(spray_type(&statuses[1], true, 40, al_map_rgb(255, 0, 0), NULL, NULL));
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
			0,
			-1,
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

void random_particle_spray(float origin_x, float origin_y, float angle, ALLEGRO_COLOR color){
	unsigned char n_particles = random(25,30);

	for(unsigned char p=0; p<n_particles; p++){
		float angle_offset = ((random(0, (unsigned) (M_PI / 2 * 100))) / 100.0) - M_PI / 4;
		
		float power = random(30, 90);
		float speed_x = cos(angle+angle_offset) * power;
		float speed_y = sin(angle+angle_offset) * power;

		particles.push_back(particle(
			origin_x,
			origin_y,
			speed_x,
			speed_y,
			1,
			0,
			(random(30, 40)) / 10.0,
			(random(30, 40)) / 10.0,
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
	member_to_remove->uncallable_period = UNCALLABLE_PERIOD;
}

void start_camera_pan(int final_x, int final_y){
	cam_trans_pan_initi_x = cam_x;
	cam_trans_pan_initi_y = cam_y;
	cam_trans_pan_final_x = final_x;
	cam_trans_pan_final_y = final_y;
	cam_trans_pan_time_left = CAM_TRANSITION_DURATION;
}

void start_camera_zoom(float final_zoom_level){
	cam_trans_zoom_initi_level = cam_zoom;
	cam_trans_zoom_final_level = final_zoom_level;
	cam_trans_zoom_time_left = CAM_TRANSITION_DURATION;
}

void stop_whistling(){
	whistling = false;
	whistle_radius = 0;
	whistle_max_hold = 0;
}

void use_spray(size_t spray_nr){
	if(sprays[spray_nr]==0) return;

	random_particle_spray(
		leaders[current_leader].x,
		leaders[current_leader].y,
		leaders[current_leader].angle + ((spray_types[spray_nr].burpable) ? M_PI : 0),
		spray_types[spray_nr].main_color
		);

	sprays[spray_nr]--;
}