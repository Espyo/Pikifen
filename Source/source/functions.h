/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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
#include "mobs/leader.h"
#include "mob_script.h"
#include "mobs/onion.h"
#include "mobs/pikmin.h"
#include "sector.h"

//Disables an enabled widget.
#define disable_widget(w) (w)->flags |= lafi::FLAG_DISABLED;

//Enables a disabled widget.
#define enable_widget(w) (w)->flags &= ~lafi::FLAG_DISABLED;

//Returns the previous element in a vector,
//but if it's the first, it retrieves the last.
#define get_prev_in_vector(v, nr) (v)[((nr) == 0 ? (v).size() - 1 : ((nr) - 1))]

//Returns the next element in a vector,
//but if it's the last, it retrieves the first.
#define get_next_in_vector(v, nr) (v)[((nr) == (v).size() - 1 ? 0 : ((nr) + 1))]

//Sets a lafi widget to invisible and disabled.
#define hide_widget(w) (w)->flags |= lafi::FLAG_INVISIBLE | lafi::FLAG_DISABLED;

//Converts an integer (or long) to a string.
#define i2s(n) to_string((long long) (n))

//Returns a string with a number, adding a leading zero if it's less than 10.
#define leading_zero(n) (((n) < 10 ? "0" : (string) "") + i2s((n)))

//Returns a white color with the specified alpha.
#define map_alpha(a) al_map_rgba(255, 255, 255, (a))

//Returns a gray with all indexes the same as specified value;
//it's fully opaque.
#define map_gray(g) al_map_rgb((g), (g), (g))

//Because we get events so many times per frame, it's faster
//to access them directly than to call a function.
#define q_get_event(m_ptr, ev_type) ((m_ptr)->fsm.cur_state->events[(ev_type)])

//Rounds a number. Ugh, why do I even have to create this.
#define round(n) (((n) > 0) ? floor((n) + 0.5) : ceil((n) - 0.5))

//Sets a lafi widget to visible and enabled.
#define show_widget(w) \
    (w)->flags &= ~(lafi::FLAG_INVISIBLE | lafi::FLAG_DISABLED);

//Returns the sign (1 or -1) of a number.
#define sign(n) (((n) >= 0) ? 1 : -1)

//Returns the task range for whether the Pikmin is idling or being C-sticked.
#define task_range(p) \
    (((p)->following_group == cur_leader_ptr && group_move_intensity) ? \
     group_move_task_range : idle_task_range)



string box_string(const string &s, const size_t size);
ALLEGRO_COLOR change_alpha(const ALLEGRO_COLOR &c, const unsigned char a);
ALLEGRO_COLOR change_color_lighting(const ALLEGRO_COLOR &c, const float l);
void change_game_state(unsigned int new_state);
void clear_area_textures();
void coordinates_to_angle(
    const point &coordinates, float* angle, float* magnitude
);
float deterministic_random(const unsigned int seed);
vector<string> folder_to_vector(
    string folder_name, const bool folders, bool* folder_found = NULL
);
unsigned char get_blackout_strength();
mob* get_closest_mob_to_cursor();
ALLEGRO_COLOR get_daylight_color();
float get_max_throw_height(const float throw_strength);
void get_multiline_text_dimensions(
    const ALLEGRO_FONT* const font, const string &text, int* ret_w, int* ret_h
);
float get_sun_strength();
float get_throw_z_speed(const float strength_multiplier);
string get_var_value(
    const string &vars_string, const string &var, const string &def
);
vector<pair<size_t, string> > get_weather_table(data_node* node);
ALLEGRO_COLOR interpolate_color(
    const float n, const float n1, const float n2,
    const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2
);
float interpolate_number(
    const float p, const float p1, const float p2,
    const float v1, const float v2
);
void load_area(
    const string &name, const bool load_for_editor, const bool from_backup
);
void load_area_textures();
void load_control(
    const unsigned char action, const unsigned char player,
    const string &name, data_node &file, const string &def = ""
);
ALLEGRO_BITMAP* load_bmp(
    const string &file_name, data_node* node = NULL, bool report_error = true
);
void load_custom_particle_generators(const bool load_resources);
data_node load_data_file(const string &file_name);
void load_hazards();
void load_liquids(const bool load_resources);
void load_options();
sample_struct load_sample(const string &file_name, ALLEGRO_MIXER* const mixer);
void load_game_config();
void load_spray_types();
void load_status_types(const bool load_resources);
void load_system_animations();
void log_error(string s, data_node* d = NULL);
void print_info(string t);
float randomf(float min, float max);
int randomi(int min, int max);
ALLEGRO_BITMAP* recreate_bitmap(ALLEGRO_BITMAP* b);
string replace_all(string s, string search, string replacement);
void save_options();
vector<string> semicolon_list_to_vector(const string &s);
vector<string> split(
    string text, const string &del = " ", const bool inc_empty = false,
    const bool inc_del = false
);
void start_message(string text, ALLEGRO_BITMAP* speaker_bmp);
string str_to_lower(string s);
string str_to_upper(string s);
int sum_and_wrap(const int nr, const int sum, const int wrap_limit);
void unload_hazards();
void unload_status_types();
void update_animation_editor_history(const string &n = "");


void al_fwrite(ALLEGRO_FILE* f, string s);
string b2s(const bool b);
string c2s(const ALLEGRO_COLOR &c);
string f2s(const float f);
string p2s(const point &p);
bool s2b(const string &s);
ALLEGRO_COLOR s2c(const string &s);
double s2f(const string &s);
int s2i(const string &s);
point s2p(const string &s);

#endif //ifndef FUNCTIONS_H
