#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <allegro5\allegro.h>

#include "leader.h"
#include "pikmin.h"

#define dist(x1, y1, x2, y2) sqrt(((x1)-(x2)) * ((x1)-(x2)) + ((y1)-(y2)) * ((y1)-(y2)))
#define random(min, max) (rand()%((max)-(min)+1))+(min)    //Returns a random number in the given interval, inclusive.

void add_to_party(mob* party_leader, mob* new_member);
void angle_to_coordinates(float angle, float magnitude, float* x_coord, float* y_coord);
void coordinates_to_angle(float x_coord, float y_coord, float* angle, float* magnitude);
void draw_health(float cx, float cy, unsigned int health, unsigned int max_health, bool just_chart = false);
void load_game_content();
void random_particle_explosion(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void random_particle_splash(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color);
void remove_from_party(mob* party_leader, mob* member_to_remove);
void stop_whistling();

#endif //ifndef FUNCTIONS_H