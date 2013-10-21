#define _USE_MATH_DEFINES
#include <math.h>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"
#include "data_file.h"
#include "functions.h"
#include "vars.h"

void add_to_party(mob* party_leader, mob* new_member){
	if(new_member->following_party == party_leader) return;	//Already following, never mind.

	new_member->following_party = party_leader;
	//ToDo remove? new_member->go_to_target = true;
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
	float first_y = cy - (font_h * 3) / 2;
	al_draw_text(font, color, cx, first_y, ALLEGRO_ALIGN_CENTER, (to_string((long long) current).c_str()));
	al_draw_text(font, color, cx, first_y + font_h * 2, ALLEGRO_ALIGN_CENTER, (to_string((long long) needed).c_str()));

	ALLEGRO_TRANSFORM scale, old;
	al_copy_transform(&old, al_get_current_transform());
	
	al_identity_transform(&scale);
	al_scale_transform(&scale, 5, 1);
	al_translate_transform(&scale, cx, 0);
	al_compose_transform(&scale, &old);

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

void draw_sector(sector &s, float x, float y){
	ALLEGRO_VERTEX vs[200]; //ToDo 200?
	size_t n_linedefs = s.linedefs.size();
	unsigned char current_floor;
	unsigned char floors_to_draw;

	current_floor = (s.floors[0].z > s.floors[1].z) ? 1 : 0;
	floors_to_draw = (s.floors[0].z == s.floors[1].z) ? 1 : 2;	//ToDo remove this check?

	for(unsigned char f = 0; f < floors_to_draw; f++){

		for(size_t l=0; l<n_linedefs; l++){
			vs[l].x = s.linedefs[l]->x1 - x;
			vs[l].y = s.linedefs[l]->y1 - y;
			vs[l].u = s.linedefs[l]->x1;
			vs[l].v = s.linedefs[l]->y1;
			vs[l].z = 0;
			vs[l].color = al_map_rgba_f(s.floors[current_floor].brightness, s.floors[current_floor].brightness, s.floors[current_floor].brightness, 1);
		}

		al_draw_prim(vs, NULL, s.floors[current_floor].texture, 0, n_linedefs, ALLEGRO_PRIM_TRIANGLE_FAN);
		
		current_floor = (current_floor == 1) ? 0 : 1;

	}

}

void draw_shadow(float cx, float cy, float size, float delta_z, float shadow_stretch){
	if(shadow_stretch <= 0) return;

	float shadow_x = 0, shadow_w = size + (size * 3 * shadow_stretch);

	if(day_minutes < 60*12){
		//Shadows point to the West.
		shadow_x = -shadow_w + size * 0.5;
	}else{
		//Shadows point to the East.
		shadow_x = -(size * 0.5);
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
		(cy - size * 0.5) + delta_z * SHADOW_Y_MULTIPLIER,
		shadow_w,
		size,
		0
		);
}

void drop_treasure(pikmin* p){
	if(!p->carrying_treasure) return;

	//ToDo optimize this instead of running through the spot vector.
	if(p->carrying_treasure){
		for(size_t s=0; s<p->carrying_treasure->max_carriers; s++){
			if(p->carrying_treasure->carrier_info->carrier_spots[s] == p){
				p->carrying_treasure->carrier_info->carrier_spots[s] = NULL;
				p->carrying_treasure->carrier_info->current_n_carriers--;
			}
		}
	}

	//Did this Pikmin leaving made the treasure stop moving?
	if(p->carrying_treasure->carrier_info->current_n_carriers < p->carrying_treasure->weight){
		p->carrying_treasure->remove_target(true);
	}

	p->carrying_treasure = NULL;
}

void error_log(string s){
	//ToDo
	total_error_log += s + "\n";
}

void generate_area_images(){
	//ToDo if it aligns perfectly with AREA_IMAGE_SIZE, a glitch could happen. For instance, a sector that spans from 0 to 800, x and y

	//First, clear all existing area images.
	for(size_t x = 0; x < area_images.size(); x++){
		for(size_t y = 0; y < area_images[x].size(); y++){
			al_destroy_bitmap(area_images[x][y]);
		}
		area_images[x].clear();
	}
	area_images.clear();

	//Now, figure out how big our area is.
	size_t n_sectors = sectors.size();
	if(n_sectors == 0) return;
	if(sectors[0].linedefs.size() == 0) return;

	float min_x, max_x, min_y, max_y;
	min_x = max_x = sectors[0].linedefs[0]->x1;
	min_y = max_y = sectors[0].linedefs[0]->y1;

	for(size_t s = 0; s < n_sectors; s++){
		size_t n_linedefs = sectors[s].linedefs.size();
		for(size_t l = 0; l<n_linedefs; l++){
			float x = sectors[s].linedefs[l]->x1;
			float y = sectors[s].linedefs[l]->y1;

			min_x = min(x, min_x);
			max_x = max(x, max_x);
			min_y = min(y, min_y);
			max_y = max(y, max_y);
		}
	}

	area_x1 = min_x; area_y1 = min_y;

	//Create the new areas on the vectors.
	float area_width = max_x - min_x;
	float area_height = max_y - min_y;
	unsigned area_image_cols = ceil(area_width / AREA_IMAGE_SIZE);
	unsigned area_image_rows = ceil(area_height / AREA_IMAGE_SIZE);

	for(size_t x = 0; x < area_image_cols; x++){
		area_images.push_back(vector<ALLEGRO_BITMAP*>());

		for(size_t y = 0; y < area_image_rows; y++){
			area_images[x].push_back(al_create_bitmap(AREA_IMAGE_SIZE, AREA_IMAGE_SIZE));
		}
	}

	//For every sector, draw it on the area images it belongs on.
	for(size_t s = 0; s<n_sectors; s++){
		size_t n_linedefs = sectors[s].linedefs.size();
		if(n_linedefs == 0) continue;

		float s_min_x, s_max_x, s_min_y, s_max_y;
		unsigned sector_start_col, sector_end_col, sector_start_row, sector_end_row;
		s_min_x = s_max_x = sectors[s].linedefs[0]->x1;
		s_min_y = s_max_y = sectors[s].linedefs[0]->y1;

		for(size_t l = 1; l<n_linedefs; l++){	//Start at 1, because we already have the first linedef's values.
			float x = sectors[s].linedefs[l]->x1;
			float y = sectors[s].linedefs[l]->y1;

			s_min_x = min(x, s_min_x);
			s_max_x = max(x, s_max_x);
			s_min_y = min(y, s_min_y);
			s_max_y = max(y, s_max_y);
		}

		sector_start_col = (s_min_x - area_x1) / AREA_IMAGE_SIZE;
		sector_end_col =   (s_max_x - area_x1) / AREA_IMAGE_SIZE;
		sector_start_row = (s_min_y - area_y1) / AREA_IMAGE_SIZE;
		sector_end_row =   (s_max_y - area_y1) / AREA_IMAGE_SIZE;

		for(size_t x = sector_start_col; x<=sector_end_col; x++){
			for(size_t y = sector_start_row; y<=sector_end_row; y++){
				ALLEGRO_BITMAP* current_target_bmp = al_get_target_bitmap();
				al_set_target_bitmap(area_images[x][y]);{

					draw_sector(sectors[s], x * AREA_IMAGE_SIZE + area_x1, y * AREA_IMAGE_SIZE + area_y1);

				}al_set_target_bitmap(current_target_bmp);
			}
		}

	}

}

ALLEGRO_COLOR get_daylight_color(){
	//ToDo initialize this somewhere else?
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

void load_area(string name){
	sectors.clear();

	data_node file = load_data_file("Areas/" + name + ".txt");

	size_t n_sectors = file["sector"].size();
	for(size_t s = 0; s<n_sectors; s++){
		data_node sector_data = file["sector"][s];
		sector new_sector = sector();
		
		size_t n_floors = sector_data["floor"].size();
		if(n_floors > 2) n_floors = 2;
		for(size_t f = 0; f<n_floors; f++){		//ToDo this is not the way to do it.
			data_node floor_data = sector_data["floor"][f];
			floor_info new_floor = floor_info();

			new_floor.brightness = atof(floor_data["brightness"].get_value("1"));
			new_floor.rot = atof(floor_data["texture_rotate"].get_value());
			new_floor.scale = atof(floor_data["texture_scale"].get_value());
			new_floor.trans_x = atof(floor_data["texture_trans_x"].get_value());
			new_floor.trans_y = atof(floor_data["texture_trans_y"].get_value());
			new_floor.texture = load_bmp("Textures/" + floor_data["texture"].get_value());	//ToDo don't load it every time.
			new_floor.z = atof(floor_data["z"].get_value().c_str());
			//ToDo terrain sound.

			new_sector.floors[f] = new_floor;
		}

		size_t n_linedefs = sector_data["linedef"].size();
		for(size_t l = 0; l<n_linedefs; l++){
			data_node linedef_data = sector_data["linedef"][l];
			linedef* new_linedef = new linedef();

			new_linedef->x1 = atof(linedef_data["x"].get_value());
			new_linedef->y1 = atof(linedef_data["y"].get_value());

			//ToDo missing things.

			new_sector.linedefs.push_back(new_linedef);
		}

		//ToDo missing things.

		sectors.push_back(new_sector);
	}
}

ALLEGRO_BITMAP* load_bmp(string filename){
	ALLEGRO_BITMAP* b = NULL;
	b=al_load_bitmap(("Game_data/Graphics/" + filename).c_str());
	if(!b){
		error_log("Could not open image " + filename + "!");
		b = bmp_error;
	}

	return b;
}

data_node load_data_file(string filename){
	data_node n = data_node("Game_data/" + filename);
	if(!n.file_was_opened){
		error_log("Could not open data file " + filename + "!");
	}

	return n;
}

sample_struct load_sample(string filename){
	sample_struct s;
	s.sample = al_load_sample(("Game_data/Audio/" + filename).c_str());
	if(!s.sample){
		error_log("Could not open audio sample " + filename + "!");
	}

	return s;
}

void load_game_content(){
	//ToDo.
	pikmin_types.push_back(pikmin_type());
	pikmin_types.back().color = al_map_rgb(255, 0, 0);
	pikmin_types.back().name = "R";
	pikmin_types.back().max_move_speed = 80;

	pikmin_types.push_back(pikmin_type());
	pikmin_types.back().color = al_map_rgb(255, 255, 0);
	pikmin_types.back().name = "Y";
	pikmin_types.back().max_move_speed = 80;

	pikmin_types.push_back(pikmin_type());
	pikmin_types.back().color = al_map_rgb(0, 0, 255);
	pikmin_types.back().name = "B";
	pikmin_types.back().max_move_speed = 80;

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
			random((int)(time_min*10), (int)(time_max*10))/10,
			random((int)(size_min * 10), (int)(size_max * 10)) / 10,
			color
		));
	}
}

void random_particle_spray(float origin_x, float origin_y, float angle, ALLEGRO_COLOR color){
	unsigned char n_particles = random(35,40);

	for(unsigned char p=0; p<n_particles; p++){
		float angle_offset = ((random(0, (unsigned) (M_PI_2 * 100))) / 100.0) - M_PI_4;
		
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
			(random(60, 80)) / 10.0,
			color
			));
	}
}

void remove_from_party(mob* party_leader, mob* member_to_remove){
	size_t n_members = party_leader->party.size();
	for(size_t p=0; p<n_members; p++){
		if(party_leader->party[p] == member_to_remove){
			party_leader->party.erase(party_leader->party.begin() + p);
			break;
		}
	}
	member_to_remove->following_party = NULL;
	member_to_remove->remove_target(false);
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

	al_stop_sample(&sfx_camera.id);
	al_play_sample(sfx_camera.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &sfx_camera.id);
}

void start_carrying(mob* m){
	//ToDo what if an Onion hasn't been revelead yet?
	if(!m->carrier_info) return;

	float target_x, target_y;

	/*
	m->set_target(
		ships[0]->x + ships[0]->size * 0.5 + m->size * 0.5 + 8,
		ships[0]->y,
		NULL,
		NULL,
		false);
	*/

	map<pikmin_type*, unsigned> type_quantity; //How many of each Pikmin type are carrying.
	vector<pikmin_type*> majority_types; //The Pikmin type with the most carriers.

	//First, count how many of each type there are.
	for(size_t p=0; p<m->max_carriers; p++){
		pikmin* pik_ptr = NULL;
		
		if(m->carrier_info->carrier_spots[p] == NULL) continue;
		if(typeid(*m->carrier_info->carrier_spots[p]) == typeid(pikmin))
			pik_ptr = (pikmin*) m->carrier_info->carrier_spots[p];
		else continue;

		if(type_quantity.find(pik_ptr->type) == type_quantity.end()) type_quantity[pik_ptr->type]=0; //ToDo maps don't start the number at 0, so that's why I need this line, correct?
		type_quantity[pik_ptr->type]++;
	}

	//Then figure out what are the majority types.
	unsigned most = 0;
	for(map<pikmin_type*, unsigned>::iterator t = type_quantity.begin(); t!=type_quantity.end(); t++){
		if(t->second > most){
			most = t->second;
			majority_types.clear();
		}
		if(t->second == most) majority_types.push_back(t->first);
	}

	//Weed out the types that don't have Onions.
	for(size_t t=0; t<majority_types.size();){
		if(!majority_types[t]->has_onion){
			majority_types.erase(majority_types.begin() + t);
		}else{
			t++;
		}
	}

	//If we ended up with no candidates, pick a type at random, out of all possible types.
	if(majority_types.size() == 0){
		for(size_t t=0; t<pikmin_types.size(); t++){
			if(pikmin_types[t].has_onion){
				majority_types.push_back(&pikmin_types[t]);
			}
		}
	}

	//Of the candidate types, pick one at random (if there's only one, only that one'll be picked, so no worries).
	if(majority_types.size() == 0) return; //ToDo warn?
	size_t type_chosen = random(0, majority_types.size() - 1);
	
	//Figure out where that type's Onion is.
	size_t onion_nr = 0;
	for(; onion_nr<onions.size(); onion_nr++){
		if(onions[onion_nr]->type == majority_types[type_chosen]){
			break;
		}
	}

	//Calculate the delivery coordinates.
	target_x = onions[onion_nr]->x;
	target_y = onions[onion_nr]->y;

	//Finall, start moving the mob.
	m->set_target(target_x, target_y, NULL, NULL, false);
	m->carrier_info->carry_color = majority_types[type_chosen]->color;
}

void stop_whistling(){
	if(!whistling) return;

	whistle_fade_time = WHISTLE_FADE_TIME;
	whistle_fade_radius = whistle_radius;

	whistling = false;
	whistle_radius = 0;
	whistle_max_hold = 0;
	
	al_stop_sample(&leaders[current_leader]->sfx_whistle.id);
}

bool temp_point_inside_sector(float x, float y, vector<linedef> &linedefs){
	return true;
}

void use_spray(size_t spray_nr){
	if(sprays[spray_nr]==0) return;

	random_particle_spray(
		leaders[current_leader]->x,
		leaders[current_leader]->y,
		leaders[current_leader]->angle + ((spray_types[spray_nr].burpable) ? M_PI : 0),
		spray_types[spray_nr].main_color
		);

	sprays[spray_nr]--;
}

double atof(string s){ return atof(s.c_str()); }
int atoi(string s){ return atof(s.c_str()); }