/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
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
#include "mob_script.h"
#include "onion.h"
#include "pikmin.h"
#include "sector.h"

//Disables an enabled widget.
#define disable_widget(w) (w)->flags |= lafi::FLAG_DISABLED;

//Enables a disabled widget.
#define enable_widget(w) (w)->flags &= ~lafi::FLAG_DISABLED;

//Returns the previous element in a vector, but if it's the first, it retrieves the last.
#define get_prev_in_vector(v, nr) (v)[((nr) == 0 ? (v).size() - 1 : ((nr) - 1))]

//Returns the next element in a vector, but if it's the last, it retrieves the first.
#define get_next_in_vector(v, nr) (v)[((nr) == (v).size() - 1 ? 0 : ((nr) + 1))]

//Sets a lafi widget to invisible and disabled.
#define hide_widget(w) (w)->flags |= lafi::FLAG_INVISIBLE | lafi::FLAG_DISABLED;

//Converts an integer (or long) to a string.
#define i2s(n) to_string((long long) (n))

//Returns a string with a number, adding a leading zero if it's less than 10.
#define leading_zero(n) (((n) < 10 ? "0" : (string) "") + i2s((n)))

//Returns a white color with the specified alpha.
#define map_alpha(a) al_map_rgba(255, 255, 255, (a))

//Returns a gray with all indexes the same as specified value; it's fully opaque.
#define map_gray(g) al_map_rgb((g), (g), (g))

//Adds a new animation conversion on load_mob_types().
#define new_anim_conversion(id, name) anim_conversions.push_back(make_pair((id), (name)))

//Rounds a number. Ugh, why do I even have to create this.
#define round(n) (((n) > 0) ? floor((n) + 0.5) : ceil((n) - 0.5))

//Sets a lafi widget to visible and enabled.
#define show_widget(w) (w)->flags &= ~(lafi::FLAG_INVISIBLE | lafi::FLAG_DISABLED);

//Returns the sign (1 or -1) of a number.
#define sign(n) (((n) >= 0) ? 1 : -1)

//Returns the task range for whether the Pikmin is idling or being C-sticked.
#define task_range ((pik_ptr->following_party == cur_leader_ptr && group_move_intensity) ? 0 : PIKMIN_MIN_TASK_RANGE)



void               angle_to_coordinates(const float angle, const float magnitude, float* x_coord, float* y_coord);
ALLEGRO_COLOR      change_alpha(const ALLEGRO_COLOR &c, const unsigned char a);
bool               circle_intersects_line(const float cx, const float cy, const float cr, const float x1, const float y1, const float x2, const float y2, float* lix = NULL, float* liy = NULL);
void               clear_area_textures();
void               coordinates_to_angle(const float x_coord, const float y_coord, float* angle, float* magnitude);
void               error_log(string s, data_node* d = NULL);
bool               find_in_vector(const vector<string> &v, const string &s);
vector<string>     folder_to_vector(string folder_name, const bool folders, bool* folder_found = NULL);
void               generate_area_images();
ALLEGRO_COLOR      get_daylight_color();
float              get_sun_strength();
string             get_var_value(const string &vars_string, const string &var, const string &def);
ALLEGRO_TRANSFORM  get_world_to_screen_transform();
ALLEGRO_COLOR      interpolate_color(const float n, const float n1, const float n2, const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2);
float              interpolate_number(const float p, const float p1, const float p2, const float v1, const float v2);
void               load_area(const string &name, const bool load_for_editor);
void               load_area_textures();
void               load_control(unsigned char action, unsigned char player, const string &name, data_node &file, const string &def);
ALLEGRO_BITMAP*    load_bmp(const string &file_name, data_node* node = NULL, bool report_error = true);
data_node          load_data_file(const string &file_name);
void               load_options();
sample_struct      load_sample(const string &file_name, ALLEGRO_MIXER* const mixer);
void               load_game_content();
void               move_point(const float x, const float y, const float tx, const float ty, const float speed, const float reach_radius, float* mx, float* my, float* angle, bool* reached);
float              normalize_angle(float a);
float              randomf(float min, float max);
int                randomi(int min, int max);
string             replace_all(string s, string search, string replacement);
void               rotate_point(const float x, const float y, const float angle, float* final_x, float* final_y);
void               save_options();
vector<string>     split(string text, const string &del = " ", const bool inc_empty = false, const bool inc_del = false);
bool               square_intersects_line(const float sx1, const float sy1, const float sx2, const float sy2, const float lx1, const float ly1, const float lx2, const float ly2);
void               start_camera_pan(const int final_x, const int final_y);
void               start_camera_zoom(const float final_zoom_level);
void               start_message(string text, ALLEGRO_BITMAP* speaker_bmp);
string             str_to_lower(string s);
void               use_spray(const size_t spray_nr);


void al_fwrite(ALLEGRO_FILE* f, string s);
string b2s(const bool b);
string c2s(const ALLEGRO_COLOR &c);
string f2s(const float f);
bool s2b(const string &s);
ALLEGRO_COLOR s2c(const string &s);
double s2f(const string &s);
int s2i(const string &s);

#endif //ifndef FUNCTIONS_H
