#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <allegro5/allegro.h>

#include "data_file.h"
#include "leader.h"
#include "pikmin.h"
#include "sector.h"

#define dist(x1, y1, x2, y2) sqrt(((x1)-(x2)) * ((x1)-(x2)) + ((y1)-(y2)) * ((y1)-(y2)))

#define random(min, max) (rand()%((max)-(min)+1))+(min)    //Returns a random number in the given interval, inclusive.

void              add_to_party(mob* party_leader, mob* new_member);
void              angle_to_coordinates(float angle, float magnitude, float* x_coord, float* y_coord);
ALLEGRO_COLOR     change_alpha(ALLEGRO_COLOR c, unsigned char a);
void              coordinates_to_angle(float x_coord, float y_coord, float* angle, float* magnitude);
void              draw_fraction(float cx, float cy, unsigned int current, unsigned int needed, ALLEGRO_COLOR color);
void              draw_health(float cx, float cy, unsigned int health, unsigned int max_health, float radius = 20, bool just_chart = false);
void              draw_sector(sector &s, float x, float y);
void              draw_shadow(float cx, float cy, float size, float delta_z, float shadow_stretch);
void              drop_treasure(pikmin* p);
void              error_log(string s);
void              generate_area_images();
ALLEGRO_COLOR     get_daylight_color();
ALLEGRO_TRANSFORM get_world_to_screen_transform();
ALLEGRO_COLOR     interpolate_color(float n, float n1, float n2, ALLEGRO_COLOR c1, ALLEGRO_COLOR c2);
void              load_area(string name);
ALLEGRO_BITMAP*   load_bmp(string filename);
data_node         load_data_file(string filename);
sample_struct     load_sample(string filename);
void              load_game_content();
void              random_particle_explosion(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void              random_particle_fire(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void              random_particle_splash(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void              random_particle_spray(float origin_x, float origin_y, float angle, ALLEGRO_COLOR color);
void              remove_from_party(mob* party_leader, mob* member_to_remove);
void              start_camera_pan(int final_x, int final_y);
void              start_camera_zoom(float final_zoom_level);
void              start_carrying(mob* m);
void              stop_whistling();
bool              temp_point_inside_sector(float x, float y, vector<linedef> &linedefs);
void              use_spray(size_t spray_nr);

double atof(string s);
int atoi(string s);

#endif //ifndef FUNCTIONS_H