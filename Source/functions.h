/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally used functions.
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#include "controls.h"
#include "data_file.h"
#include "leader.h"
#include "mob_event.h"
#include "spec_objs/onion.h"
#include "pikmin.h"
#include "sector.h"

//Returns the distance between two points.
#define dist(x1, y1, x2, y2) sqrt(((x1)-(x2)) * ((x1)-(x2)) + ((y1)-(y2)) * ((y1)-(y2)))
//Converts a float (or double) to a string.
#define ftos(n) to_string((long double) (n))
//Sets a lafi widget to invisible and disabled.
#define hide_widget(w) (w)->flags |= LAFI_FLAG_INVISIBLE | LAFI_FLAG_DISABLED;
//Converts an integer (or long) to a string.
#define itos(n) to_string((long long) (n))
//Returns a string with a number, adding a leading zero if it's less than 10.
#define leading_zero(n) (((n) < 10 ? "0" : (string) "") + itos((n)))
//Returns the modulus of a number, regardless of it being a float or negative.
#define mod(n, d) ((n) - (d) * floor((n) / (d)))
//Adds a new animation conversion on load_mob_types().
#define new_anim_conversion(id, name) anim_conversions.push_back(make_pair<size_t, string>((id), (name)))
//Normalizes an angle between the range [-M_PI, M_PI]
#define normalize_angle(a) (mod((a), M_PI*2) - M_PI)
//Sets a lafi widget to visible and enabled.
#define show_widget(w) (w)->flags &= ~(LAFI_FLAG_INVISIBLE | LAFI_FLAG_DISABLED);
//Returns the sign (1 or -1) of a number.
#define sign(n) (((n) >= 0) ? 1 : -1)
//Returns the task range for whether the Pikmin is idling or being C-sticked.
#define task_range ((pik_ptr->following_party == cur_leader_ptr && moving_group_intensity) ? 0 : PIKMIN_MIN_TASK_RANGE)

void               angle_to_coordinates(const float angle, const float magnitude, float* x_coord, float* y_coord);
ALLEGRO_COLOR      change_alpha(const ALLEGRO_COLOR c, const unsigned char a);
void               coordinates_to_angle(const float x_coord, const float y_coord, float* angle, float* magnitude);
void               error_log(string s, data_node* d = NULL);
bool               find_in_vector(const vector<string> v, const string s);
vector<string>     folder_to_vector(string folder_name, const bool folders);
void               generate_area_images();
ALLEGRO_COLOR      get_daylight_color();
ALLEGRO_TRANSFORM  get_world_to_screen_transform();
ALLEGRO_COLOR      interpolate_color(const float n, const float n1, const float n2, const ALLEGRO_COLOR c1, const ALLEGRO_COLOR c2);
void               load_area(const string name);
//ToDo try to figure out why in the world uncommenting this gives retarded errors. void               load_control(unsigned char action, unsigned char player, string name, data_node& file, string def)
ALLEGRO_BITMAP*    load_bmp(const string filename, data_node* node = NULL);
data_node          load_data_file(const string filename);
void               load_options();
sample_struct      load_sample(const string filename, ALLEGRO_MIXER* const mixer);
void               load_game_content();
void               move_point(const float x, const float y, const float tx, const float ty, const float speed, const float reach_radius, float* mx, float* my, float* angle, bool* reached);
float              randomf(float min, float max);
int                randomi(int min, int max);
void               rotate_point(const float x, const float y, const float angle, float* final_x, float* final_y);
void               save_options();
vector<string>     split(string text, const string del = " ", const bool inc_empty = false, const bool inc_del = false);
void               start_camera_pan(const int final_x, const int final_y);
void               start_camera_zoom(const float final_zoom_level);
void               start_message(string text, ALLEGRO_BITMAP* speaker_bmp);
string             str_to_lower(string s);
//bool               temp_point_inside_sector(float x, float y, vector<linedef> &linedefs);
void               use_spray(const size_t spray_nr);

void al_fwrite(ALLEGRO_FILE* f, string s);
string btos(bool b);
bool tob(string s);
ALLEGRO_COLOR toc(string s);
double tof(string s);
int toi(string s);

#endif //ifndef FUNCTIONS_H