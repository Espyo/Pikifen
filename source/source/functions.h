/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally used functions.
 */

#pragma once

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
        info += (message); \
        crash("Assert", info, 1); \
        return; \
    }

//Returns the task range for whether the Pikmin is idling or being C-sticked.
#define task_range(p) \
    (((p)->following_group == cur_leader_ptr && swarm_magnitude) ? \
     game.config.swarm_task_range : game.config.idle_task_range)


/**
 * @brief Function that checks if an edge should use a given edge offset effect.
 * 
 * The first parameter is the edge to check.
 * The second parameter is where the affected sector gets returned to.
 * The third parameter is where the unaffected sector gets returned to.
 * Returns whether it should receive the effect.
 */
typedef bool (*offset_effect_checker_t)(edge*, sector**, sector**);

/**
 * @brief Function that returns an edge's edge offset effect color.
 * 
 * The first parameter is the edge to check.
 * Returns the color.
 */
typedef ALLEGRO_COLOR (*offset_effect_color_getter_t)(edge*);

/**
 * @brief Function that returns an edge's edge offset effect length.
 * 
 * The first parameter is the edge to check.
 * Returns the length.
 */
typedef float (*offset_effect_length_getter_t)(edge*);


bool are_walls_between(
    const point &p1, const point &p2,
    float ignore_walls_below_z = -FLT_MAX, bool* out_impassable_walls = nullptr
);
void clear_area_textures();
void crash(const string &reason, const string &info, int exit_status);
bool does_edge_have_ledge_smoothing(
    edge* e_ptr, sector** out_affected_sector, sector** out_unaffected_sector
);
bool does_edge_have_liquid_limit(
    edge* e_ptr, sector** out_affected_sector, sector** out_unaffected_sector
);
bool does_edge_have_wall_shadow(
    edge* e_ptr, sector** out_affected_sector, sector** out_unaffected_sector
);
void draw_edge_offset_on_buffer(
    const vector<edge_offset_cache> &caches, size_t e_idx
);
mob* get_closest_mob_to_cursor();
void get_edge_offset_edge_info(
    edge* e_ptr, vertex* end_vertex, unsigned char end_idx,
    float edge_process_angle,
    offset_effect_checker_t checker,
    offset_effect_length_getter_t length_getter,
    offset_effect_color_getter_t color_getter,
    float* out_angle, float* out_length, ALLEGRO_COLOR* out_color,
    float* out_elbow_angle, float* out_elbow_length
);
void get_edge_offset_intersection(
    const edge* e1, const edge* e2, const vertex* common_vertex,
    float base_shadow_angle1, float base_shadow_angle2,
    float shadow_length,
    float* out_angle, float* out_length
);
ALLEGRO_COLOR get_ledge_smoothing_color(edge* e_ptr);
ALLEGRO_COLOR get_liquid_limit_color(edge* e_ptr);
float get_ledge_smoothing_length(edge* e_ptr);
float get_liquid_limit_length(edge* e_ptr);
void get_next_edge(
    vertex* v_ptr, float pivot_angle, bool clockwise,
    const edge* ignore, edge** out_edge, float* out_angle, float* out_diff
);
void get_next_offset_effect_edge(
    vertex* v_ptr, float pivot_angle, bool clockwise,
    const edge* ignore, offset_effect_checker_t edge_checker,
    edge** out_edge, float* out_angle, float* out_diff,
    float* out_base_shadow_angle,
    bool* out_shadow_cw
);
string get_subtitle_or_mission_goal(
    const string &subtitle, const AREA_TYPE area_type,
    const MISSION_GOAL goal
);
unsigned char get_throw_preview_vertexes(
    ALLEGRO_VERTEX* vertexes,
    float start, float end,
    const point &leader_pos, const point &cursor_pos,
    const ALLEGRO_COLOR &color,
    float u_offset, float u_scale,
    bool vary_thickness
);
map<string, string> get_var_map(const string &vars_string);
string get_engine_version_string();
ALLEGRO_COLOR get_wall_shadow_color(edge* e_ptr);
float get_wall_shadow_length(edge* e_ptr);
vector<std::pair<int, string> > get_weather_table(data_node* node);
void print_info(
    const string &text,
    float total_duration = 5.0f,
    float fade_duration = 3.0f
);
void report_fatal_error(const string &s, const data_node* dn = nullptr);
void save_maker_tools();
void save_options();
void save_screenshot();
void save_statistics();
void set_string_token_widths(
    vector<string_token> &tokens,
    const ALLEGRO_FONT* text_font, const ALLEGRO_FONT* control_font,
    float max_control_bitmap_height = 0, bool control_condensed = false
);
void signal_handler(int signum);
void spew_pikmin_seed(
    const point pos, float z, pikmin_type* pik_type,
    float angle, float horizontal_speed, float vertical_speed
);
vector<vector<string_token> > split_long_string_with_tokens(
    const vector<string_token> &tokens, int max_width
);
void start_message(const string &text, ALLEGRO_BITMAP* speaker_bmp);
vector<string_token> tokenize_string(const string &s);
string unescape_string(const string &s);
void update_offset_effect_buffer(
    const point &cam_tl, const point &cam_br,
    const vector<edge_offset_cache> &caches, ALLEGRO_BITMAP* buffer,
    bool clear_first
);
void update_offset_effect_caches (
    vector<edge_offset_cache> &caches,
    const unordered_set<vertex*>& vertexes_to_update,
    offset_effect_checker_t checker,
    offset_effect_length_getter_t length_getter,
    offset_effect_color_getter_t color_getter
);
