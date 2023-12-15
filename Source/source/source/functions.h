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
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "area/area.h"
#include "controls.h"
#include "mob_script.h"
#include "mobs/leader.h"
#include "mobs/onion.h"
#include "mobs/pikmin.h"
#include "area/sector.h"
#include "libs/data_file.h"


//Possible results for the player interacting with a file dialog.
enum FILE_DIALOG_RESULTS {
    //Successful operation.
    FILE_DIALOG_RES_SUCCESS,
    //The option picked is not in the expected folder.
    FILE_DIALOG_RES_WRONG_FOLDER,
    //The player cancelled the dialog.
    FILE_DIALOG_RES_CANCELED,
};


//Possible results for a folder wipe operation.
enum WIPE_FOLDER_RESULTS {
    //Wipe successful.
    WIPE_FOLDER_RESULT_OK,
    //Folder not found.
    WIPE_FOLDER_RESULT_NOT_FOUND,
    //Folder has important files inside, or has folders inside.
    WIPE_FOLDER_RESULT_HAS_IMPORTANT,
    //An error occurred somewhere when deleting a file or folder.
    WIPE_FOLDER_RESULT_DELETE_ERROR,
};


//Turns a bit in a bitmask off.
#define disable_flag(flags, flag) (flags &= ~flag)

//Turns a bit in a bitmask on.
#define enable_flag(flags, flag) (flags |= flag)

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
        return; \
    }

//Returns the bitmask corresponding to a certain index. Useful for flags.
#define get_index_bitmask(i) (1 << i)

//Returns the previous element in a vector,
//but if it's the first, it retrieves the last.
#define get_prev_in_vector(v, nr) (v)[((nr) == 0 ? (v).size() - 1 : ((nr) - 1))]

//Returns the next element in a vector,
//but if it's the last, it retrieves the first.
#define get_next_in_vector(v, nr) (v)[((nr) == (v).size() - 1 ? 0 : ((nr) + 1))]

//Returns whether a bit is on or not in a bitmask.
#define has_flag(flags, flag) ((flags & flag) > 0)

//Returns a white color with the specified alpha.
#define map_alpha(a) al_map_rgba(255, 255, 255, (a))

//Returns a gray with all indexes the same as specified value;
//it's fully opaque.
#define map_gray(g) al_map_rgb((g), (g), (g))

//Returns the task range for whether the Pikmin is idling or being C-sticked.
#define task_range(p,player_id) \
    (((p)->following_group == player_info[player_id].cur_leader_ptr && player_info[player_id].swarm_magnitude) ? \
     game.config.swarm_task_range : game.config.idle_task_range)


//Function that checks if an edge should use a given edge offset effect.
typedef bool (*offset_effect_checker_ptr)(edge*, sector**, sector**);
//Function that returns an edge's edge offset effect color.
typedef ALLEGRO_COLOR (*offset_effect_color_getter_ptr)(edge*);
//Function that returns an edge's edge offset effect length.
typedef float (*offset_effect_length_getter_ptr)(edge*);


bool are_walls_between(
    const point &p1, const point &p2,
    float ignore_walls_below_z = -FLT_MAX, bool* impassable_walls = NULL
);
ALLEGRO_COLOR change_alpha(const ALLEGRO_COLOR &c, const unsigned char a);
ALLEGRO_COLOR change_color_lighting(const ALLEGRO_COLOR &c, const float l);
void clear_area_textures();
void crash(const string &reason, const string &info, const int exit_status);
bool does_edge_have_ledge_smoothing(
    edge* e_ptr, sector** affected_sector, sector** unaffected_sector
);
bool does_edge_have_liquid_limit(
    edge* e_ptr, sector** affected_sector, sector** unaffected_sector
);
bool does_edge_have_wall_shadow(
    edge* e_ptr, sector** affected_sector, sector** unaffected_sector
);
void draw_edge_offset_on_buffer(
    const vector<edge_offset_cache> &caches, size_t e_idx
);
vector<string> folder_to_vector(
    string folder_name, const bool folders, bool* folder_found = NULL
);
string get_current_time(const bool filename_friendly);
mob* get_closest_mob_to_cursor();
void get_edge_offset_edge_info(
    edge* e_ptr, vertex* end_vertex, const unsigned char end_idx,
    const float edge_process_angle,
    offset_effect_checker_ptr checker,
    offset_effect_length_getter_ptr length_getter,
    offset_effect_color_getter_ptr color_getter,
    float* final_angle, float* final_length, ALLEGRO_COLOR* final_color,
    float* final_elbow_angle, float* final_elbow_length
);
void get_edge_offset_intersection(
    edge* e1, edge* e2, vertex* common_vertex,
    const float base_shadow_angle1, const float base_shadow_angle2,
    const float shadow_length,
    float* final_angle, float* final_length
);
string get_key_name(const int keycode, const bool condensed);
ALLEGRO_COLOR get_ledge_smoothing_color(edge* e_ptr);
ALLEGRO_COLOR get_liquid_limit_color(edge* e_ptr);
float get_ledge_smoothing_length(edge* e_ptr);
float get_liquid_limit_length(edge* e_ptr);
void get_multiline_text_dimensions(
    const ALLEGRO_FONT* const font, const string &text, int* ret_w, int* ret_h
);
void get_next_edge(
    vertex* v_ptr, const float pivot_angle, const bool clockwise,
    const edge* ignore, edge** final_edge, float* final_angle, float* final_diff
);
void get_next_offset_effect_edge(
    vertex* v_ptr, const float pivot_angle, const bool clockwise,
    const edge* ignore, offset_effect_checker_ptr edge_checker,
    edge** final_edge, float* final_angle, float* final_diff,
    float* final_base_shadow_angle,
    bool* final_shadow_cw
);
string get_subtitle_or_mission_goal(
    const string &subtitle, const AREA_TYPES area_type,
    const MISSION_GOALS goal
);
unsigned char get_throw_preview_vertexes(
    ALLEGRO_VERTEX* vertexes,
    const float start, const float end,
    const point &leader_pos, const point &cursor_pos,
    const ALLEGRO_COLOR &color,
    const float u_offset, const float u_scale,
    const bool vary_thickness
);
map<string, string> get_var_map(const string &vars_string);
string get_engine_version_string();
ALLEGRO_COLOR get_wall_shadow_color(edge* e_ptr);
float get_wall_shadow_length(edge* e_ptr);
vector<std::pair<int, string> > get_weather_table(data_node* node);
ALLEGRO_COLOR interpolate_color(
    const float input, const float input_start, const float input_end,
    const ALLEGRO_COLOR &output_start, const ALLEGRO_COLOR &output_end
);
void log_error(const string &s, data_node* d = NULL);
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
string sanitize_file_name(const string &s);
void save_maker_tools();
void save_options();
void save_screenshot();
void save_statistics();
size_t select_next_item_directionally(
    const vector<point> &item_coordinates, const size_t selected_item,
    const float direction, const point &loop_region
);
vector<string> semicolon_list_to_vector(
    const string &s, const string &sep = ";"
);
void set_string_token_widths(
    vector<string_token> &tokens,
    const ALLEGRO_FONT* text_font, const ALLEGRO_FONT* control_font,
    const float max_control_bitmap_height = 0
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
vector<vector<string_token> > split_long_string_with_tokens(
    const vector<string_token> &tokens, const int max_width
);
string standardize_path(const string &path);
void start_message(const string &text, ALLEGRO_BITMAP* speaker_bmp,const int &player_id=0);
vector<string_token> tokenize_string(const string &s);
string unescape_string(const string &s);
void update_offset_effect_buffer(
    const point &cam_tl, const point &cam_br,
    const vector<edge_offset_cache> &caches, ALLEGRO_BITMAP* buffer,
    const bool clear_first
);
void update_offset_effect_caches (
    vector<edge_offset_cache> &caches,
    unordered_set<vertex*> vertexes_to_update,
    offset_effect_checker_ptr checker,
    offset_effect_length_getter_ptr length_getter,
    offset_effect_color_getter_ptr color_getter
);
string vector_tail_to_string(const vector<string> &v, const size_t pos);
WIPE_FOLDER_RESULTS wipe_folder(
    const string &folder_path, const vector<string> &non_important_files
);
string word_wrap(const string &s, const size_t n_chars_per_line);

void al_fwrite(ALLEGRO_FILE* f, const string &s);
string c2s(const ALLEGRO_COLOR &c);
ALLEGRO_COLOR s2c(const string &s);
string p2s(const point &p, float* z = NULL);
point s2p(const string &s, float* z = NULL);


/* ----------------------------------------------------------------------------
 * Returns whether or not the two vectors contain the same items,
 * regardless of order.
 * v1:
 *   First vector.
 * v2:
 *   Second vector.
 */
template<typename t>
bool vectors_contain_same(const vector<t> &v1, const vector<t> &v2) {
    if(v1.size() != v2.size()) {
        return false;
    }
    
    for(size_t i1 = 0; i1 < v1.size(); ++i1) {
        bool found = false;
        for(size_t i2 = 0; i2 < v2.size(); ++i2) {
            if(v1[i1] == v2[i2]) {
                found = true;
                break;
            }
        }
        if(!found) {
            return false;
        }
    }
    
    return true;
}


#if defined(_WIN32)
string strsignal(const int signum);
#endif //#if defined(_WIN32)

#endif //ifndef FUNCTIONS_INCLUDED
