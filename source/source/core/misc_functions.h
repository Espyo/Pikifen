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

#include "../core/game.h"
#include "../content/area/area.h"
#include "../content/area/sector.h"
#include "../content/mob/leader.h"
#include "../content/mob/onion.h"
#include "../content/mob/pikmin.h"
#include "../content/other/gui.h"
#include "../content/other/mob_script.h"
#include "../game_state/editor.h"
#include "../lib/data_file/data_file.h"
#include "controls.h"


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
typedef bool (*offset_effect_checker_t)(Edge*, Sector**, Sector**);

/**
 * @brief Function that returns an edge's edge offset effect color.
 *
 * The first parameter is the edge to check.
 * Returns the color.
 */
typedef ALLEGRO_COLOR (*offset_effect_color_getter_t)(Edge*);

/**
 * @brief Function that returns an edge's edge offset effect length.
 *
 * The first parameter is the edge to check.
 * Returns the length.
 */
typedef float (*offset_effect_length_getter_t)(Edge*);



bool are_walls_between(
    const Point &p1, const Point &p2,
    float ignore_walls_below_z = -FLT_MAX, bool* out_impassable_walls = nullptr
);
void clear_area_textures();
void crash(const string &reason, const string &info, int exit_status);
bool does_edge_have_ledge_smoothing(
    Edge* e_ptr, Sector** out_affected_sector, Sector** out_unaffected_sector
);
bool does_edge_have_liquid_limit(
    Edge* e_ptr, Sector** out_affected_sector, Sector** out_unaffected_sector
);
bool does_edge_have_wall_shadow(
    Edge* e_ptr, Sector** out_affected_sector, Sector** out_unaffected_sector
);
void draw_edge_offset_on_buffer(
    const vector<EdgeOffsetCache> &caches, size_t e_idx
);
Mob* get_closest_mob_to_cursor(bool must_have_health = false);
void get_edge_offset_edge_info(
    Edge* e_ptr, Vertex* end_vertex, unsigned char end_idx,
    float edge_process_angle,
    offset_effect_checker_t checker,
    offset_effect_length_getter_t length_getter,
    offset_effect_color_getter_t color_getter,
    float* out_angle, float* out_length, ALLEGRO_COLOR* out_color,
    float* out_elbow_angle, float* out_elbow_length
);
void get_edge_offset_intersection(
    const Edge* e1, const Edge* e2, const Vertex* common_vertex,
    float base_shadow_angle1, float base_shadow_angle2,
    float shadow_length,
    float* out_angle, float* out_length
);
ALLEGRO_COLOR get_ledge_smoothing_color(Edge* e_ptr);
ALLEGRO_COLOR get_liquid_limit_color(Edge* e_ptr);
float get_ledge_smoothing_length(Edge* e_ptr);
float get_liquid_limit_length(Edge* e_ptr);
string get_mission_record_entry_name(Area* area_ptr);
void get_next_edge(
    Vertex* v_ptr, float pivot_angle, bool clockwise,
    const Edge* ignore, Edge** out_edge, float* out_angle, float* out_diff
);
Mob* get_next_mob_near_cursor(
    Mob* pivot, bool must_have_health = false
);
void get_next_offset_effect_edge(
    Vertex* v_ptr, float pivot_angle, bool clockwise,
    const Edge* ignore, offset_effect_checker_t edge_checker,
    Edge** out_edge, float* out_angle, float* out_diff,
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
    const Point &leader_pos, const Point &cursor_pos,
    const ALLEGRO_COLOR &color,
    float u_offset, float u_scale,
    bool vary_thickness
);
map<string, string> get_var_map(const string &vars_string);
string get_engine_version_string();
ALLEGRO_COLOR get_wall_shadow_color(Edge* e_ptr);
float get_wall_shadow_length(Edge* e_ptr);
vector<std::pair<int, string> > get_weather_table(DataNode* node);
void gui_add_back_input_icon(
    GuiManager* gui, const string &item_name = "back_input"
);
bool mono_combo(
    const string &label, int* current_item, const vector<string> &items,
    int popup_max_height_in_items = -1
);
bool mono_combo(
    const string &label, string* current_item, const vector<string> &items,
    int popup_max_height_in_items = -1
);
bool mono_combo(
    const string &label, string* current_item,
    const vector<string> &item_internal_values,
    const vector<string> &item_display_names,
    int popup_max_height_in_items = -1
);
bool mono_button(
    const char* label, const ImVec2 &size = ImVec2(0, 0)
);
bool mono_input_text(
    const char* label, string* str, ImGuiInputTextFlags flags = 0,
    ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr
);
bool mono_input_text_with_hint(
    const char* label, const char* hint, string* str,
    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
    void* user_data = nullptr
);
bool mono_list_box(
    const string &label, int* current_item, const vector<string> &items,
    int height_in_items = -1
);
bool mono_selectable(
    const char* label, bool selected = false, ImGuiSelectableFlags flags = 0,
    const ImVec2 &size = ImVec2(0, 0)
);
bool mono_selectable(
    const char* label, bool* p_selected, ImGuiSelectableFlags flags = 0,
    const ImVec2 &size = ImVec2(0, 0)
);
bool open_manual(const string &page);
void print_info(
    const string &text,
    float total_duration = 5.0f,
    float fade_duration = 3.0f
);
void report_fatal_error(const string &s, const DataNode* dn = nullptr);
void save_maker_tools();
void save_options();
void save_screenshot();
void save_statistics();
void set_string_token_widths(
    vector<StringToken> &tokens,
    const ALLEGRO_FONT* text_font, const ALLEGRO_FONT* control_font,
    float max_control_bitmap_height = 0, bool control_condensed = false
);
void signal_handler(int signum);
void spew_pikmin_seed(
    const Point pos, float z, PikminType* pik_type,
    float angle, float horizontal_speed, float vertical_speed
);
vector<vector<StringToken> > split_long_string_with_tokens(
    const vector<StringToken> &tokens, int max_width
);
ParticleGenerator standard_particle_gen_setup(
    const string &internal_name, Mob* target_mob
);
void start_gameplay_message(const string &text, ALLEGRO_BITMAP* speaker_bmp);
vector<StringToken> tokenize_string(const string &s);
string unescape_string(const string &s);
void update_offset_effect_buffer(
    const Point &cam_tl, const Point &cam_br,
    const vector<EdgeOffsetCache> &caches, ALLEGRO_BITMAP* buffer,
    bool clear_first
);
void update_offset_effect_caches (
    vector<EdgeOffsetCache> &caches,
    const unordered_set<Vertex*> &vertexes_to_update,
    offset_effect_checker_t checker,
    offset_effect_length_getter_t length_getter,
    offset_effect_color_getter_t color_getter
);
Point v2p(const Vertex* v);



/**
 * @brief Goes through all keyframes in a keyframe interpolator, and lets you
 * adjust the value in each one, by running the "predicate" function for each.
 *
 * @tparam t Value type for the interpolator.
 * @param interpolator Interpolator to adjust.
 * @param predicate Function whose argument is the original value at that
 * keyframe, and whose return value is the new value.
 * @return Whether the operation succeeded.
 */
template<typename t>
bool adjust_keyframe_interpolator_values(
    KeyframeInterpolator<t> &interpolator,
    std::function<t(const t &)> predicate
) {
    bool result = false;
    size_t n_keyframes = interpolator.get_keyframe_count();
    for(size_t k = 0; k < n_keyframes; k++) {
        const auto &orig_keyframe = interpolator.get_keyframe(k);
        interpolator.set_keyframe_value(k, predicate(orig_keyframe.second));
        result = true;
    }
    return result;
}


/**
 * @brief Processes a Dear ImGui text widget, but sets the font to be
 * monospaced.
 *
 * @tparam args_t Function argument type.
 * @param args Function arguments to pass to ImGui::Text().
 */
template <typename ...args_t>
void mono_text(args_t && ...args) {
    ImGui::PushFont(game.sys_content.fnt_imgui_monospace);
    ImGui::Text(std::forward<args_t>(args)...);
    ImGui::PopFont();
}
