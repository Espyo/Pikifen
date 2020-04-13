/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally used functions.
 */

#ifndef FUNCTIONS_INCLUDED
#define FUNCTIONS_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>

#include "controls.h"
#include "mob_script.h"
#include "mobs/leader.h"
#include "mobs/onion.h"
#include "mobs/pikmin.h"
#include "sector.h"
#include "utils/data_file.h"
#include "vars.h"

enum FILE_DIALOG_RESULTS {
    FILE_DIALOG_RES_SUCCESS,
    FILE_DIALOG_RES_WRONG_FOLDER,
    FILE_DIALOG_RES_CANCELED,
};

//Disables an enabled widget.
#define disable_widget(w) (w)->flags |= lafi::FLAG_DISABLED;

//Enables a disabled widget.
#define enable_widget(w) (w)->flags &= ~lafi::FLAG_DISABLED;

//A custom-made assertion.
#define engine_assert(expr, message) \
    if(!(expr)) { \
        string info = "\"" #expr "\", in "; \
        info += __FUNCTION__; \
        info += " ("; \
        info += __FILE__; \
        info += ":"; \
        info += std::to_string((long long) (__LINE__)); \
        info += "). Extra info: "; \
        info += message; \
        crash("Assert", info, 1); \
    }

//Returns the previous element in a vector,
//but if it's the first, it retrieves the last.
#define get_prev_in_vector(v, nr) (v)[((nr) == 0 ? (v).size() - 1 : ((nr) - 1))]

//Returns the next element in a vector,
//but if it's the last, it retrieves the first.
#define get_next_in_vector(v, nr) (v)[((nr) == (v).size() - 1 ? 0 : ((nr) + 1))]

//Returns a white color with the specified alpha.
#define map_alpha(a) al_map_rgba(255, 255, 255, (a))

//Returns a gray with all indexes the same as specified value;
//it's fully opaque.
#define map_gray(g) al_map_rgb((g), (g), (g))

//Because we get events so many times per frame, it's faster
//to access them directly than to call a function.
#define q_get_event(m_ptr, ev_type) ((m_ptr)->fsm.cur_state->events[(ev_type)])

//Returns the task range for whether the Pikmin is idling or being C-sticked.
#define task_range(p) \
    (((p)->following_group == cur_leader_ptr && swarm_magnitude) ? \
     swarm_task_range : idle_task_range)



bool casts_shadow(sector* s1, sector* s2);
ALLEGRO_COLOR change_alpha(const ALLEGRO_COLOR &c, const unsigned char a);
ALLEGRO_COLOR change_color_lighting(const ALLEGRO_COLOR &c, const float l);
void change_game_state(unsigned int new_state);
void clear_area_textures();
void crash(const string &reason, const string &info, const int exit_status);
vector<string> folder_to_vector(
    string folder_name, const bool folders, bool* folder_found = NULL
);
unsigned char get_blackout_strength();
string get_current_time(const bool filename_friendly);
mob* get_closest_mob_to_cursor();
ALLEGRO_COLOR get_daylight_color();
ALLEGRO_COLOR get_fog_color();
void get_multiline_text_dimensions(
    const ALLEGRO_FONT* const font, const string &text, int* ret_w, int* ret_h
);
float get_sun_strength();
map<string, string> get_var_map(const string &vars_string);
float get_wall_shadow_length(const float height_difference);
vector<std::pair<size_t, string> > get_weather_table(data_node* node);
ALLEGRO_COLOR interpolate_color(
    const float n, const float n1, const float n2,
    const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2
);
void log_error(string s, data_node* d = NULL);
void print_info(
    const string &text,
    const float total_duration = 5.0f,
    const float fade_duration = 3.0f
);
vector<string> prompt_file_dialog(
    const string &initial_path, const string &title,
    const string &patterns, const int mode
);
vector<string> prompt_file_dialog_locked_to_folder(
    const string &folder, const string &title,
    const string &patterns, const int mode, FILE_DIALOG_RESULTS* result
);
ALLEGRO_BITMAP* recreate_bitmap(ALLEGRO_BITMAP* b);
void report_fatal_error(const string &s, data_node* dn = NULL);
void save_creator_tools();
void save_options();
void save_screenshot();
vector<string> semicolon_list_to_vector(
    const string &s, const string &sep = ";"
);
int show_message_box(
    ALLEGRO_DISPLAY* display, char const* title, char const* heading,
    char const* text, char const* buttons, int flags
);
void signal_handler(const int signum);
void spew_pikmin_seed(
    const point pos, const float z, pikmin_type* pik_type,
    const float angle, const float horizontal_speed, const float vertical_speed
);
string standardize_path(const string &path);
void start_message(string text, ALLEGRO_BITMAP* speaker_bmp);
string vector_tail_to_string(const vector<string> &v, const size_t pos);


void al_fwrite(ALLEGRO_FILE* f, string s);
string c2s(const ALLEGRO_COLOR &c);
ALLEGRO_COLOR s2c(const string &s);
string p2s(const point &p, float* z = NULL);
point s2p(const string &s, float* z = NULL);

#if defined(_WIN32)
string strsignal(const int signum);
#endif //#if defined(_WIN32)

#endif //ifndef FUNCTIONS_INCLUDED
