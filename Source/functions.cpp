#define _USE_MATH_DEFINES
#include <algorithm>
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

	make_uncarriable(new_member);
}

void angle_to_coordinates(float angle, float magnitude, float* x_coord, float* y_coord){
	*x_coord = cos(angle) * magnitude;
	*y_coord = sin(angle) * magnitude;
}

bool atob(string s){
	string ls = str_to_lower(s);
	if(ls == "yes" || ls == "true" || ls == "y" || ls == "t") return true;
	else return (atoi(ls) != 0);
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

void create_mob(mob* m){
	mobs.push_back(m);

	if(typeid(*m) == typeid(pikmin)){
		pikmin_list.push_back((pikmin*) m);

	}else if(typeid(*m) == typeid(leader)){
		leaders.push_back((leader*) m);

	}else if(typeid(*m) == typeid(onion)){
		onions.push_back((onion*) m);

	}else if(typeid(*m) == typeid(nectar)){
		nectars.push_back((nectar*) m);

	}else if(typeid(*m) == typeid(pellet)){
		pellets.push_back((pellet*) m);

	}else if(typeid(*m) == typeid(ship)){
		ships.push_back((ship*) m);

	}else if(typeid(*m) == typeid(treasure)){
		treasures.push_back((treasure*) m);

	}else if(typeid(*m) == typeid(info_spot)){
		info_spots.push_back((info_spot*) m);

	}
}

void delete_mob(mob* m){
	mobs.erase(find(mobs.begin(), mobs.end(), m));

	if(typeid(*m) == typeid(pikmin)){
		pikmin_list.erase(find(pikmin_list.begin(), pikmin_list.end(), (pikmin*) m));

	}else if(typeid(*m) == typeid(leader)){
		leaders.erase(find(leaders.begin(), leaders.end(), (leader*) m));

	}else if(typeid(*m) == typeid(onion)){
		onions.erase(find(onions.begin(), onions.end(), (onion*) m));

	}else if(typeid(*m) == typeid(nectar)){
		nectars.erase(find(nectars.begin(), nectars.end(), (nectar*) m));

	}else if(typeid(*m) == typeid(pellet)){
		pellets.erase(find(pellets.begin(), pellets.end(), (pellet*) m));

	}else if(typeid(*m) == typeid(ship)){
		ships.erase(find(ships.begin(), ships.end(), (ship*) m));

	}else if(typeid(*m) == typeid(treasure)){
		treasures.erase(find(treasures.begin(), treasures.end(), (treasure*) m));

	}else if(typeid(*m) == typeid(info_spot)){
		info_spots.erase(find(info_spots.begin(), info_spots.end(), (info_spot*) m));

	}
}

void dismiss(){
	leader* current_leader_ptr = leaders[current_leader];

	//ToDo what if there are a lot of Pikmin types?
	size_t n_party_members = current_leader_ptr->party.size();
	for(size_t m=0; m<n_party_members; m++){
		mob* member_ptr = current_leader_ptr->party[0];
		remove_from_party(member_ptr);

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
}

void draw_fraction(float cx, float cy, unsigned int current, unsigned int needed, ALLEGRO_COLOR color){
	float first_y = cy - (font_h * 3) / 2;
	al_draw_text(font, color, cx, first_y, ALLEGRO_ALIGN_CENTER, (to_string((long long) current).c_str()));
	al_draw_text(font, color, cx, first_y + font_h * 1.5, ALLEGRO_ALIGN_CENTER, (to_string((long long) needed).c_str()));

	ALLEGRO_TRANSFORM scale, old;
	al_copy_transform(&old, al_get_current_transform());
	
	al_identity_transform(&scale);
	al_scale_transform(&scale, 5, 1);
	al_translate_transform(&scale, cx, 0);
	al_compose_transform(&scale, &old);

	al_use_transform(&scale);{
		al_draw_text(font, color, 0, first_y + font_h * 0.75, ALLEGRO_ALIGN_CENTER, "-");
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
		shadow_x -= shadow_stretch * delta_z * SHADOW_Y_MULTIPLIER;
	}else{
		//Shadows point to the East.
		shadow_x = -(size * 0.5);
		shadow_x += shadow_stretch * delta_z * SHADOW_Y_MULTIPLIER;
	}
	
	
	draw_sprite(
		bmp_shadow,
		cx + shadow_x + shadow_w / 2, cy,
		shadow_w, size,
		0, al_map_rgba(255, 255, 255, 255*(1-shadow_stretch)));
}

void draw_sprite(ALLEGRO_BITMAP* bmp, float cx, float cy, float w, float h, float angle, ALLEGRO_COLOR tint){
	if(!bmp){
		bmp = bmp_error;
	}

	float bmp_w = al_get_bitmap_width(bmp);
	float bmp_h = al_get_bitmap_height(bmp);
	float x_scale = w / bmp_w;
	float y_scale = h / bmp_h;
	al_draw_tinted_scaled_rotated_bitmap(
		bmp,
		tint,
		bmp_w / 2, bmp_h / 2,
		cx, cy,
		x_scale, y_scale,
		angle,
		0);
}

void drop_mob(pikmin* p){
	if(!p->carrying_mob) return;

	//ToDo optimize this instead of running through the spot vector.
	if(p->carrying_mob){
		for(size_t s=0; s<p->carrying_mob->carrier_info->max_carriers; s++){
			if(p->carrying_mob->carrier_info->carrier_spots[s] == p){
				p->carrying_mob->carrier_info->carrier_spots[s] = NULL;
				p->carrying_mob->carrier_info->current_n_carriers--;
			}
		}
	}

	//Did this Pikmin leaving made the mob stop moving?
	if(p->carrying_mob->carrier_info->current_n_carriers < p->carrying_mob->weight){
		p->carrying_mob->remove_target(true);
	}else{
		start_carrying(p->carrying_mob, NULL, p); //Enter this code so that if this Pikmin leaving broke a tie, the Onion's picked correctly.
	}

	p->carrying_mob = NULL;
	p->remove_target(true);
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

void give_pikmin_to_onion(onion* o, unsigned amount){
	unsigned total_after = pikmin_list.size() + amount;
	unsigned pikmin_to_spit = amount;
	unsigned pikmin_to_keep = 0; //Pikmin to keep inside the Onion, without spitting.

	if(total_after > max_pikmin_in_field){
		pikmin_to_keep = total_after - max_pikmin_in_field;
		pikmin_to_spit = amount - pikmin_to_keep;
	}

	for(unsigned p=0; p<pikmin_to_spit; p++){
		float angle = random(0, M_PI*2);
		float x = o->x + cos(angle) * o->size * 2;
		float y = o->y + sin(angle) * o->size * 2;

		//ToDo throw them, don't teleport them.
		pikmin* new_pikmin = new pikmin(o->type, x, y, o->sec);
		new_pikmin->burrowed = true;
		create_mob(new_pikmin);
	}

	for(unsigned p=0; p<pikmin_to_keep; p++){
		pikmin_in_onions[o->type]++;
	}
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

void load_control(unsigned char action, unsigned char player, string name, data_node& file, string def){
	string s = file["p" + to_string((long long) (player+1)) + "_" + name].get_value((player == 0) ? def : "");
	vector<string> possible_controls = split(s, ",");
	size_t n_possible_controls = possible_controls.size();

	for(size_t c=0; c<n_possible_controls; c++){
		controls.push_back(control_info(action, player, possible_controls[c]));
	}
}

data_node load_data_file(string filename){
	data_node n = data_node("Game_data/" + filename);
	if(!n.file_was_opened){
		error_log("Could not open data file " + filename + "!");
	}

	return n;
}

void load_options(){
	data_node file = data_node("Options.txt");
	if(!file.file_was_opened) return;
	
	//Load joysticks.
	joystick_numbers.clear();
	int n_joysticks=al_get_num_joysticks();
	for(int j=0; j<n_joysticks; j++){
		joystick_numbers[al_get_joystick(j)] = j;
	}

	//Load controls.
	//Format of a control: "p<player number>_<action>=<possible control 1>,<possible control 2>,<...>"
	//Format of a possible control: "<input method>_<parameters, underscore separated>"
	//Input methods: "k" (keyboard key), "mb" (mouse button), "mwu" (mouse wheel up), "mwd" (down),
	//"mwl" (left), "mwr" (right), "jb" (joystick button), "jap" (joystick axis, positive), "jan" (joystick axis, negative).
	//The parameters are the key/button number, joystick number, joystick stick and axis, etc.
	//Check the constructor of control_info for more information.
	controls.clear();

	for(unsigned char p=0; p<4; p++){
		load_control(BUTTON_PUNCH,                p, "punch", file, "mb_1");
		load_control(BUTTON_WHISTLE,              p, "whistle", file, "mb_2");
		load_control(BUTTON_MOVE_RIGHT,           p, "move_right", file, "k_4");
		load_control(BUTTON_MOVE_UP,              p, "move_up", file, "k_23");
		load_control(BUTTON_MOVE_LEFT,            p, "move_left", file, "k_1");
		load_control(BUTTON_MOVE_DOWN,            p, "move_down", file, "k_19");
		load_control(BUTTON_MOVE_CURSOR_RIGHT,    p, "move_cursor_right", file, "");
		load_control(BUTTON_MOVE_CURSOR_UP,       p, "move_cursor_up", file, "");
		load_control(BUTTON_MOVE_CURSOR_LEFT,     p, "move_cursor_left", file, "");
		load_control(BUTTON_MOVE_CURSOR_DOWN,     p, "move_cursor_down", file, "");
		load_control(BUTTON_MOVE_GROUP_TO_CURSOR, p, "move_group_to_cursor", file, "k_75");
		load_control(BUTTON_MOVE_GROUP_RIGHT,     p, "move_group_right", file, "");
		load_control(BUTTON_MOVE_GROUP_UP,        p, "move_group_up", file, "");
		load_control(BUTTON_MOVE_GROUP_LEFT,      p, "move_group_left", file, "");
		load_control(BUTTON_MOVE_GROUP_DOWN,      p, "move_group_down", file, "");
		load_control(BUTTON_SWITCH_CAPTAIN_RIGHT, p, "switch_captain_right", file, "k_64");
		load_control(BUTTON_SWITCH_CAPTAIN_LEFT,  p, "switch_captain_left", file, "");
		load_control(BUTTON_DISMISS,              p, "dismiss", file, "k_217");
		load_control(BUTTON_USE_SPRAY_1,          p, "use_spray_1", file, "k_18");
		load_control(BUTTON_USE_SPRAY_2,          p, "use_spray_2", file, "k_6");
		load_control(BUTTON_USE_SPRAY,            p, "use_spray", file, "k_18");
		load_control(BUTTON_SWITCH_SPRAY_RIGHT,   p, "switch_spray_right", file, "k_5");
		load_control(BUTTON_SWITCH_SPRAY_LEFT,    p, "switch_spray_left", file, "k_17");
		load_control(BUTTON_SWITCH_ZOOM,          p, "switch_zoom", file, "k_3");
		load_control(BUTTON_ZOOM_IN,              p, "zoom_in", file, "mwu");
		load_control(BUTTON_ZOOM_OUT,             p, "zoom_out", file, "mwd");
		load_control(BUTTON_SWITCH_TYPE_RIGHT,    p, "switch_type_right", file, "");
		load_control(BUTTON_SWITCH_TYPE_LEFT,     p, "switch_type_left", file, "");
		load_control(BUTTON_SWITCH_MATURITY_UP,   p, "switch_maturity_up", file, "");
		load_control(BUTTON_SWITCH_MATURITY_DOWN, p, "switch_maturity_down", file, "");
		load_control(BUTTON_LIE_DOWN,             p, "lie_down", file, "k_26");
		load_control(BUTTON_PAUSE,                p, "pause", file, "k_59");
	}

	//Weed out controls that didn't parse correctly.
	size_t n_controls = controls.size();
	for(size_t c=0; c<n_controls; ){
		if(controls[c].action == BUTTON_NONE){
			controls.erase(controls.begin() + c);
		}else{
			c++;
		}
	}

	for(unsigned char p=0; p<4; p++){
		mouse_moves_cursor[p] = atob(file["p" + to_string((long long) (p+1)) + "_mouse_moves_cursor"].get_value((p==0) ? "true" : "false"));
	}

	//Other options.
	daylight_effect = atob(file["daylight_effect"].get_value("true"));
	game_fps = atoi(file["fps"].get_value("30"));
	scr_h = atoi(file["height"].get_value(to_string((long long) DEF_SCR_H)));
	particle_quality = atoi(file["particle_quality"].get_value("2"));
	pretty_whistle = atob(file["pretty_whistle"].get_value("true"));
	scr_w = atoi(file["width"].get_value(to_string((long long) DEF_SCR_W)));
	smooth_scaling = atob(file["smooth_scaling"].get_value("true"));
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

	pikmin_types.push_back(pikmin_type());
	pikmin_types.back().color = al_map_rgb(255, 255, 255);
	pikmin_types.back().name = "W";
	pikmin_types.back().max_move_speed = 100;
	pikmin_types.back().has_onion = false;

	statuses.push_back(status(0, 0, 1, 0, true, al_map_rgb(128, 0, 255), STATUS_AFFECTS_ENEMIES));
	statuses.push_back(status(1.5, 1.5, 1, 1, false, al_map_rgb(255, 64, 64), STATUS_AFFECTS_PIKMIN));

	spray_types.push_back(spray_type(&statuses[0], false, 10, al_map_rgb(128, 0, 255), NULL, NULL));
	spray_types.push_back(spray_type(&statuses[1], true, 40, al_map_rgb(255, 0, 0), NULL, NULL));

	pellet_types.push_back(pellet_type(32, 2, 1, 2, 1));
	pellet_types.push_back(pellet_type(64, 10, 5, 5, 3));
	pellet_types.push_back(pellet_type(96, 20, 10, 10, 5));
	pellet_types.push_back(pellet_type(128, 50, 20, 20, 10));
}

void make_uncarriable(mob* m){
	if(!m->carrier_info) return;

	delete m->carrier_info;
	m->carrier_info = NULL;
}

inline float random(float min, float max){
	return (float) rand() / ((float) RAND_MAX / (max - min)) + min;
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

void remove_from_party(mob* member){
	member->following_party->party.erase(find(
		member->following_party->party.begin(),
		member->following_party->party.end(),
		member));

	member->following_party = NULL;
	member->remove_target(false);
	member->uncallable_period = UNCALLABLE_PERIOD;
}

void save_options(){
	//ToDo make this prettier. Like a list of constants somewhere where it associates an action with the name on the text file.
	ALLEGRO_FILE* file=al_fopen("Options.txt", "w");

	if(!file) return;
	
	//First, group the controls by action and player.
	map<string, string> grouped_controls;

	//Tell the map what they are.
	for(unsigned char p=0; p<4; p++){
		string prefix="p" + to_string((long long) (p+1)) + "_";
		grouped_controls[prefix + "punch"] = "";
		grouped_controls[prefix + "whistle"] = "";
		grouped_controls[prefix + "move_right"] = "";
		grouped_controls[prefix + "move_up"] = "";
		grouped_controls[prefix + "move_left"] = "";
		grouped_controls[prefix + "move_down"] = "";
		grouped_controls[prefix + "move_cursor_right"] = "";
		grouped_controls[prefix + "move_cursor_up"] = "";
		grouped_controls[prefix + "move_cursor_left"] = "";
		grouped_controls[prefix + "move_cursor_down"] = "";
		grouped_controls[prefix + "move_group_right"] = "";
		grouped_controls[prefix + "move_group_up"] = "";
		grouped_controls[prefix + "move_group_left"] = "";
		grouped_controls[prefix + "move_group_down"] = "";
		grouped_controls[prefix + "move_group_to_cursor"] = "";
		grouped_controls[prefix + "switch_captain_right"] = "";
		grouped_controls[prefix + "switch_captain_left"] = "";
		grouped_controls[prefix + "dismiss"] = "";
		grouped_controls[prefix + "use_spray_1"] = "";
		grouped_controls[prefix + "use_spray_2"] = "";
		grouped_controls[prefix + "use_spray"] = "";
		grouped_controls[prefix + "switch_spray_right"] = "";
		grouped_controls[prefix + "switch_spray_left"] = "";
		grouped_controls[prefix + "switch_zoom"] = "";
		grouped_controls[prefix + "zoom_in"] = "";
		grouped_controls[prefix + "zoom_out"] = "";
		grouped_controls[prefix + "switch_type_right"] = "";
		grouped_controls[prefix + "switch_type_left"] = "";
		grouped_controls[prefix + "switch_maturity_up"] = "";
		grouped_controls[prefix + "switch_maturity_down"] = "";
		grouped_controls[prefix + "lie_down"] = "";
		grouped_controls[prefix + "pause"] = "";
	}

	size_t n_controls = controls.size();
	for(size_t c=0; c<n_controls; c++){
		string name="p" + to_string((long long) (controls[c].player + 1)) + "_";
		if(controls[c].action == BUTTON_PUNCH)                     name+="punch";
		else if(controls[c].action == BUTTON_WHISTLE)              name+="whistle";
		else if(controls[c].action == BUTTON_MOVE_RIGHT)           name+="move_right";
		else if(controls[c].action == BUTTON_MOVE_UP)              name+="move_up";
		else if(controls[c].action == BUTTON_MOVE_LEFT)            name+="move_left";
		else if(controls[c].action == BUTTON_MOVE_DOWN)            name+="move_down";
		else if(controls[c].action == BUTTON_MOVE_CURSOR_RIGHT)    name+="move_cursor_right";
		else if(controls[c].action == BUTTON_MOVE_CURSOR_UP)       name+="move_cursor_up";
		else if(controls[c].action == BUTTON_MOVE_CURSOR_LEFT)     name+="move_cursor_left";
		else if(controls[c].action == BUTTON_MOVE_CURSOR_DOWN)     name+="move_cursor_down";
		else if(controls[c].action == BUTTON_MOVE_GROUP_RIGHT)     name+="move_group_right";
		else if(controls[c].action == BUTTON_MOVE_GROUP_UP)        name+="move_group_up";
		else if(controls[c].action == BUTTON_MOVE_GROUP_LEFT)      name+="move_group_left";
		else if(controls[c].action == BUTTON_MOVE_GROUP_DOWN)      name+="move_group_down";
		else if(controls[c].action == BUTTON_MOVE_GROUP_TO_CURSOR) name+="move_group_to_cursor";
		else if(controls[c].action == BUTTON_SWITCH_CAPTAIN_RIGHT) name+="switch_captain_right";
		else if(controls[c].action == BUTTON_SWITCH_CAPTAIN_LEFT)  name+="switch_captain_left";
		else if(controls[c].action == BUTTON_DISMISS)              name+="dismiss";
		else if(controls[c].action == BUTTON_USE_SPRAY_1)          name+="use_spray_1";
		else if(controls[c].action == BUTTON_USE_SPRAY_2)          name+="use_spray_2";
		else if(controls[c].action == BUTTON_USE_SPRAY)            name+="use_spray";
		else if(controls[c].action == BUTTON_SWITCH_SPRAY_RIGHT)   name+="switch_spray_right";
		else if(controls[c].action == BUTTON_SWITCH_SPRAY_LEFT)    name+="switch_spray_left";
		else if(controls[c].action == BUTTON_SWITCH_ZOOM)          name+="switch_zoom";
		else if(controls[c].action == BUTTON_ZOOM_IN)              name+="zoom_in";
		else if(controls[c].action == BUTTON_ZOOM_OUT)             name+="zoom_out";
		else if(controls[c].action == BUTTON_SWITCH_TYPE_RIGHT)    name+="switch_type_right";
		else if(controls[c].action == BUTTON_SWITCH_TYPE_LEFT)     name+="switch_type_left";
		else if(controls[c].action == BUTTON_SWITCH_MATURITY_UP)   name+="switch_maturity_up";
		else if(controls[c].action == BUTTON_SWITCH_MATURITY_DOWN) name+="switch_maturity_down";
		else if(controls[c].action == BUTTON_LIE_DOWN)             name+="lie_down";
		else if(controls[c].action == BUTTON_PAUSE)                name+="pause";

		grouped_controls[name] += controls[c].stringify() + ",";
	}
	
	//Save controls.
	for(map<string, string>::iterator c=grouped_controls.begin(); c!=grouped_controls.end(); c++){
		if(c->second.size()) c->second.erase(c->second.size()-1); //Remove the final character, which is always an extra comma.

		al_fwrite(file, c->first + "=" + c->second + "\n");
	}
	
	for(unsigned char p=0; p<4; p++){
		al_fwrite(file, "p" + to_string((long long) (p+1)) + "_mouse_moves_cursor=" + btoa(mouse_moves_cursor[p]) + "\n");
	}

	//Other options.
	al_fwrite(file, "daylight_effect=" + btoa(daylight_effect) + "\n");
	al_fwrite(file, "fps=" + to_string((long long) game_fps) + "\n");
	al_fwrite(file, "height=" + to_string((long long) scr_h) + "\n");
	al_fwrite(file, "particle_quality=" + to_string((long long) particle_quality) + "\n");
	al_fwrite(file, "pretty_whistle=" + btoa(pretty_whistle) + "\n");
	al_fwrite(file, "width=" + to_string((long long) scr_w) + "\n");
	al_fwrite(file, "smooth_scaling=" + btoa(smooth_scaling) + "\n");

	al_fclose(file);
}

/* ----------------------------------------------------------------------------
 * Splits a string into several substrings, by the specified delimiter.
 * text:		The string to split.
 * del:			The delimiter. Default is space.
 * inc_empty:	If true, include empty substrings on the vector.
 ** i.e. if two delimiters come together in a row, keep an empty substring between.
 * inc_del:		If true, include the delimiters on the vector as a substring.
 */
vector<string> split(string text, string del, bool inc_empty, bool inc_del){
	vector<string> v;
	size_t pos;
	size_t del_size = del.size();
	
	do {
		pos = text.find(del);
		if (pos != string::npos) {	//If it DID find the delimiter.
			//Get the text between the start and the delimiter.
			string sub=text.substr(0, pos);	

			//Add the text before the delimiter to the vector.
			if(sub!="" || inc_empty)
				v.push_back(sub);

			//Add the delimiter to the vector, but only if requested.
			if(inc_del)
				v.push_back(del);
			
			text.erase(text.begin(), text.begin() + pos + del_size);	//Delete everything before the delimiter, including the delimiter itself, and search again.
		}
	} while (pos != string::npos);
	
	//Text after the final delimiter. (If there is one. If not, it's just the whole string.)

	if (text!="" || inc_empty)	//If it's a blank string, only add it if we want empty strings.
		v.push_back(text);
	
    return v;
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

//m: mob to start moving.
//np: new Pikmin; the Pikmin that justed joined. Used to detect ties and tie-breaking.
//lp: leaving Pikmin; the Pikmin that just left. Used to detect ties and tie-breaking.
void start_carrying(mob* m, pikmin* np, pikmin* lp){
	//ToDo what if an Onion hasn't been revelead yet?
	if(!m->carrier_info) return;

	if(m->carrier_info->carry_to_ship){
		
		m->set_target(
			ships[0]->x + ships[0]->size * 0.5 + m->size * 0.5 + 8,
			ships[0]->y,
			NULL,
			NULL,
			false);
		m->carrier_info->decided_type = NULL;

	}else{

		map<pikmin_type*, unsigned> type_quantity; //How many of each Pikmin type are carrying.
		vector<pikmin_type*> majority_types; //The Pikmin type with the most carriers.

		//First, count how many of each type there are.
		for(size_t p=0; p<m->carrier_info->max_carriers; p++){
			pikmin* pik_ptr = NULL;
		
			if(m->carrier_info->carrier_spots[p] == NULL) continue;
			if(typeid(*m->carrier_info->carrier_spots[p]) != typeid(pikmin)) continue;
			
			pik_ptr = (pikmin*) m->carrier_info->carrier_spots[p];

			if(!pik_ptr->type->has_onion) continue; //If it doesn't have an Onion, it won't even count. //ToDo what if it hasn't been discovered / Onion not on this area?

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

		//If we ended up with no candidates, pick a type at random, out of all possible types.
		if(majority_types.size() == 0){
			for(size_t t=0; t<pikmin_types.size(); t++){
				if(pikmin_types[t].has_onion){ //ToDo what if it hasn't been discovered / Onion not on this area?
					majority_types.push_back(&pikmin_types[t]);
				}
			}
		}

		//Now let's pick an Onion.
		if(majority_types.size() == 0){
			return; //ToDo warn that something went horribly wrong?

		}if(majority_types.size() == 1){
			//If there's only one possible type to pick, pick it.
			m->carrier_info->decided_type = majority_types[0];

		}else{
			//If there's a tie, let's take a careful look.
			bool new_tie = false;

			//Is the Pikmin that just joined part of the majority types?
			//If so, that means this Pikmin just created a NEW tie!
			//So let's pick a random Onion again.
			if(np){
				for(size_t mt=0; mt<majority_types.size(); mt++){
					if(np->type == majority_types[mt]){
						new_tie = true;
						break;
					}
				}
			}

			//If a Pikmin left, check if they are related to the majority types.
			//If not, then a new tie wasn't made, no worries.
			//If it was related, a new tie was created.
			if(lp){
				new_tie = false;
				for(size_t mt=0; mt<majority_types.size(); mt++){
					if(lp->type == majority_types[mt]){
						new_tie = true;
						break;
					}
				}
			}

			//Check if the previously decided type belongs to one of the majorities.
			//If so, it can be chosen again, but if not, it cannot.
			bool can_continue = false;
			for(size_t mt=0; mt<majority_types.size(); mt++){
				if(majority_types[mt] == m->carrier_info->decided_type){
					can_continue = true;
					break;
				}
			}
			if(!can_continue) m->carrier_info->decided_type = NULL;

			//If the Pikmin that just joined is not a part of the majorities,
			//then it had no impact on the existing ties.
			//Go with the Onion that had been decided before.
			if(new_tie || !m->carrier_info->decided_type){
				m->carrier_info->decided_type = majority_types[random(0, majority_types.size() - 1)];
			}
		}

	
		//Figure out where that type's Onion is.
		size_t onion_nr = 0;
		for(; onion_nr<onions.size(); onion_nr++){
			if(onions[onion_nr]->type == m->carrier_info->decided_type){
				break;
			}
		}

		//Finally, start moving the mob.
		m->set_target(onions[onion_nr]->x, onions[onion_nr]->y, NULL, NULL, false);
	}
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

/* ----------------------------------------------------------------------------
 * Converts an entire string into lowercase.
 */
string str_to_lower(string s){
	unsigned short n_characters=s.size();
	for(unsigned short c=0; c<n_characters; c++){
		s[c]=tolower(s[c]);
	}
	return s;
}


/*bool temp_point_inside_sector(float x, float y, vector<linedef> &linedefs){
	return true;
}*/

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

inline void al_fwrite(ALLEGRO_FILE* f, string s){ al_fwrite(f, s.c_str(), s.size()); }
inline double atof(string s){ return atof(s.c_str()); }
inline int atoi(string s){ return atof(s.c_str()); }
inline string btoa(bool b){ return b ? "true" : "false"; }