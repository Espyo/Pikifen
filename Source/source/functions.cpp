/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally accessible functions.
 */

#define _USE_MATH_DEFINES
#undef _CMATH_

// Visual Studio warnings.
#ifdef _MSC_VER
// Disable warning about localtime being deprecated.
#pragma warning(disable : 4996)
#endif

#include <algorithm>
#include <iostream>
#include <stdlib.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "functions.h"

#include "const.h"
#include "drawing.h"
#include "game.h"
#include "init.h"
#include "utils/allegro_utils.h"
#include "utils/backtrace.h"
#include "utils/general_utils.h"
#include "utils/string_utils.h"


/**
 * @brief Checks if there are any walls between two points.
 * i.e. any edges that a mob can't simply step up to.
 *
 * @param p1 First point.
 * @param p2 Second point.
 * @param ignore_walls_below_z Any walls whose sector Zs are below
 * this value get ignored. Use -FLT_MAX to not ignore any wall.
 * @param out_impassable_walls If not nullptr, true will be returned here if
 * any of the walls are impassable, i.e. the void or "blocking"-type
 * sectors. False otherwise.
 * @return Whether there are walls between.
 */
bool are_walls_between(
    const point &p1, const point &p2,
    float ignore_walls_below_z, bool* out_impassable_walls
) {
    point bb_tl(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
    point bb_br(std::max(p1.x, p1.x), std::max(p2.y, p2.y));
    
    set<edge*> candidate_edges;
    if(
        !game.cur_area_data.bmap.get_edges_in_region(
            bb_tl, bb_br,
            candidate_edges
        )
    ) {
        //Somehow out of bounds.
        if(out_impassable_walls) *out_impassable_walls = true;
        return true;
    }
    
    for(auto const &e_ptr : candidate_edges) {
        if(
            !line_segs_intersect(
                p1, p2,
                point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
                point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y),
                nullptr
            )
        ) {
            continue;
        }
        for(size_t s = 0; s < 2; ++s) {
            if(!e_ptr->sectors[s]) {
                //No sectors means there's out-of-bounds geometry in the way.
                if(out_impassable_walls) *out_impassable_walls = true;
                return true;
            }
            if(e_ptr->sectors[s]->type == SECTOR_TYPE_BLOCKING) {
                //If a blocking sector is in the way, no clear line.
                if(out_impassable_walls) *out_impassable_walls = true;
                return true;
            }
        }
        if(
            e_ptr->sectors[0]->z < ignore_walls_below_z &&
            e_ptr->sectors[1]->z < ignore_walls_below_z
        ) {
            //This wall was chosen to be ignored.
            continue;
        }
        if(
            fabs(e_ptr->sectors[0]->z - e_ptr->sectors[1]->z) >
            GEOMETRY::STEP_HEIGHT
        ) {
            //The walls are more than stepping height in difference.
            //So it's a genuine wall in the way.
            if(out_impassable_walls) *out_impassable_walls = false;
            return true;
        }
    }
    
    if(out_impassable_walls) *out_impassable_walls = false;
    return false;
}


/**
 * @brief Clears the textures of the area's sectors from memory.
 */
void clear_area_textures() {
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        if(
            s_ptr->texture_info.bitmap &&
            s_ptr->texture_info.bitmap != game.bmp_error
        ) {
            game.textures.free(s_ptr->texture_info.file_name);
            s_ptr->texture_info.bitmap = nullptr;
        }
    }
}


/**
 * @brief Purposely crashes the engine, reporting as much information
 * as possible to the logs. Used when a fatal problem occurs.
 *
 * @param reason Explanation of the type of crash (assert, SIGSEGV, etc.).
 * @param info Any extra information to report to the logs.
 * @param exit_status Program exit status.
 */
void crash(const string &reason, const string &info, const int exit_status) {

    if(game.display) {
        ALLEGRO_BITMAP* backbuffer = al_get_backbuffer(game.display);
        if(backbuffer) {
            al_save_bitmap(
                (
                    USER_DATA_FOLDER_PATH + "/" +
                    "Crash_" + get_current_time(true) + ".png"
                ).c_str(),
                backbuffer
            );
        }
    }
    
    string error_str = "Program crash!\n";
    error_str +=
        "  Reason: " + reason + ".\n"
        "  Info: " + info + "\n"
        "  Time: " + get_current_time(false) + ".\n";
    if(game.errors.session_has_errors()) {
        error_str += "  Error log has messages!\n";
    }
    error_str +=
        "  Game state: " + game.get_cur_state_name() + ". delta_t: " +
        (
            game.delta_t == 0.0f ? "0" :
            f2s(game.delta_t) + " (" + f2s(1.0f / game.delta_t) + " FPS)"
        ) + ".\n"
        "  Mob count: " +
        i2s(game.states.gameplay->mobs.all.size()) + ". Particle count: " +
        i2s(game.states.gameplay->particles.get_count()) + ".\n" +
        "  Bitmaps loaded: " + i2s(game.bitmaps.get_list_size()) + " (" +
        i2s(game.bitmaps.get_total_uses()) + " total uses).\n" +
        "  Current area: ";
        
    if(!game.cur_area_data.name.empty()) {
        error_str +=
            game.cur_area_data.name + ", version " +
            game.cur_area_data.version + ".\n";
    } else {
        error_str += "none.\n";
    }
    
    error_str += "  Current leader: ";
    
    if(game.states.gameplay->cur_leader_ptr) {
        error_str +=
            game.states.gameplay->cur_leader_ptr->type->name + ", at " +
            p2s(game.states.gameplay->cur_leader_ptr->pos) +
            ", state history: " +
            game.states.gameplay->cur_leader_ptr->fsm.cur_state->name;
        for(size_t h = 0; h < STATE_HISTORY_SIZE; ++h) {
            error_str +=
                " " +
                game.states.gameplay->cur_leader_ptr->
                fsm.prev_state_names[h];
        }
        error_str += "\n  10 closest Pikmin to that leader:\n";
        
        vector<pikmin*> closest_pikmin =
            game.states.gameplay->mobs.pikmin_list;
        sort(
            closest_pikmin.begin(), closest_pikmin.end(),
        [] (const pikmin * p1, const pikmin * p2) -> bool {
            return
            dist(
                game.states.gameplay->cur_leader_ptr->pos,
                p1->pos
            ).to_float() <
            dist(
                game.states.gameplay->cur_leader_ptr->pos,
                p2->pos
            ).to_float();
        }
        );
        
        size_t closest_p_amount = std::min(closest_pikmin.size(), (size_t) 10);
        for(size_t p = 0; p < closest_p_amount; ++p) {
            error_str +=
                "    " + closest_pikmin[p]->type->name + ", at " +
                p2s(closest_pikmin[p]->pos) + ", history: " +
                closest_pikmin[p]->fsm.cur_state->name;
            for(size_t h = 0; h < STATE_HISTORY_SIZE; ++h) {
                error_str += " " + closest_pikmin[p]->fsm.prev_state_names[h];
            }
            error_str += "\n";
        }
    } else {
        error_str += "none.";
    }
    
    game.errors.report(error_str);
    
    show_message_box(
        nullptr, "Program crash!",
        "Pikifen has crashed!",
        "Sorry about that! To help fix this problem, please read the "
        "troubleshooting section of the included manual. Thanks!",
        nullptr,
        ALLEGRO_MESSAGEBOX_ERROR
    );
    
    exit(exit_status);
}


/**
 * @brief Checks whether a given edge should get a ledge smoothing
 * edge offset effect or not.
 *
 * @param e_ptr Edge to check.
 * @param out_affected_sector If there should be an effect, the affected sector,
 * i.e. the one getting the smoothing, is returned here.
 * @param out_unaffected_sector If there should be an effect, the
 * unaffected sector, i.e. the lower one, is returned here.
 * @return Whether it has ledge smoothing.
 */
bool does_edge_have_ledge_smoothing(
    edge* e_ptr, sector** out_affected_sector, sector** out_unaffected_sector
) {
    //Never-smooth walls don't have the effect.
    if(e_ptr->ledge_smoothing_length <= 0.0f) return false;
    
    if(
        (e_ptr->sectors[0] && !e_ptr->sectors[1]) ||
        e_ptr->sectors[1]->is_bottomless_pit
    ) {
        //If 0 exists but 1 doesn't.
        *out_affected_sector = e_ptr->sectors[0];
        *out_unaffected_sector = e_ptr->sectors[1];
        return true;
        
    } else if(
        (!e_ptr->sectors[0] && e_ptr->sectors[1]) ||
        e_ptr->sectors[0]->is_bottomless_pit
    ) {
        //If 1 exists but 0 doesn't.
        *out_affected_sector = e_ptr->sectors[1];
        *out_unaffected_sector = e_ptr->sectors[0];
        return true;
        
    } else {
        //Return whichever one is the tallest.
        if(e_ptr->sectors[0]->z > e_ptr->sectors[1]->z) {
            *out_affected_sector = e_ptr->sectors[0];
            *out_unaffected_sector = e_ptr->sectors[1];
            return true;
        } else if(e_ptr->sectors[1]->z > e_ptr->sectors[0]->z) {
            *out_affected_sector = e_ptr->sectors[1];
            *out_unaffected_sector = e_ptr->sectors[0];
            return true;
        } else {
            return false;
        }
        
    }
}


/**
 * @brief Checks whether a given edge should get a liquid limit
 * edge offset effect or not.
 *
 * @param e_ptr Edge to check.
 * @param out_affected_sector If there should be an effect, the affected sector,
 * i.e. the one with the liquid, is returned here.
 * @param out_unaffected_sector If there should be an effect, the
 * unaffected sector, i.e. the one without the liquid, is returned here.
 * @return Whether it has a liquid limit.
 */
bool does_edge_have_liquid_limit(
    edge* e_ptr, sector** out_affected_sector, sector** out_unaffected_sector
) {
    //Check if the sectors exist.
    if(!e_ptr->sectors[0] || !e_ptr->sectors[1]) return false;
    
    //Check which ones have liquid.
    bool has_liquid[2] = {false, false};
    for(unsigned char s = 0; s < 2; ++s) {
        for(size_t h = 0; h < e_ptr->sectors[s]->hazards.size(); ++h) {
            if(e_ptr->sectors[s]->hazards[h]->associated_liquid) {
                has_liquid[s] = true;
            }
        }
    }
    
    //Return edges with liquid on one side only.
    if(has_liquid[0] && !has_liquid[1]) {
        *out_affected_sector = e_ptr->sectors[0];
        *out_unaffected_sector = e_ptr->sectors[1];
        return true;
    } else if(has_liquid[1] && !has_liquid[0]) {
        *out_affected_sector = e_ptr->sectors[1];
        *out_unaffected_sector = e_ptr->sectors[0];
        return true;
    } else {
        return false;
    }
}


/**
 * @brief Checks whether a given edge should get a wall shadow
 * edge offset effect or not.
 *
 * @param e_ptr Edge to check.
 * @param out_affected_sector If there should be an effect, the affected sector,
 * i.e. the one getting shaded, is returned here.
 * @param out_unaffected_sector If there should be an effect, the
 * unaffected sector, i.e. the one casting the shadow, is returned here.
 * @return Whether it has a wall shadow.
 */
bool does_edge_have_wall_shadow(
    edge* e_ptr, sector** out_affected_sector, sector** out_unaffected_sector
) {
    //Never-cast walls don't cast.
    if(e_ptr->wall_shadow_length <= 0.0f) return false;
    
    //Invalid sectors don't cast.
    if(!e_ptr->sectors[0] || !e_ptr->sectors[1]) return false;
    if(e_ptr->sectors[0]->is_bottomless_pit) return false;
    if(e_ptr->sectors[1]->is_bottomless_pit) return false;
    
    //Same-height sectors can't cast.
    if(e_ptr->sectors[0]->z == e_ptr->sectors[1]->z) return false;
    
    //We can already save which one is highest.
    if(e_ptr->sectors[0]->z > e_ptr->sectors[1]->z) {
        *out_unaffected_sector = e_ptr->sectors[0];
        *out_affected_sector = e_ptr->sectors[1];
    } else {
        *out_unaffected_sector = e_ptr->sectors[1];
        *out_affected_sector = e_ptr->sectors[0];
    }
    
    if(e_ptr->wall_shadow_length != LARGE_FLOAT) {
        //Fixed shadow length.
        return true;
    } else {
        //Auto shadow length.
        return
            (*out_unaffected_sector)->z >
            (*out_affected_sector)->z + GEOMETRY::STEP_HEIGHT;
    }
}


/**
 * @brief Returns the mob that is closest to the mouse cursor.
 *
 * @return The mob.
 */
mob* get_closest_mob_to_cursor() {
    dist closest_mob_to_cursor_dist;
    mob* closest_mob_to_cursor = nullptr;
    
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
        mob* m_ptr = game.states.gameplay->mobs.all[m];
        
        if(!m_ptr->fsm.cur_state) continue;
        if(m_ptr->is_stored_inside_mob()) continue;
        
        dist d = dist(game.mouse_cursor.w_pos, m_ptr->pos);
        if(!closest_mob_to_cursor || d < closest_mob_to_cursor_dist) {
            closest_mob_to_cursor = m_ptr;
            closest_mob_to_cursor_dist = d;
        }
    }
    
    return closest_mob_to_cursor;
}


/**
 * @brief Returns the engine's version as a string.
 *
 * @return The string.
 */
string get_engine_version_string() {
    return
        i2s(VERSION_MAJOR) + "." +
        i2s(VERSION_MINOR) + "." +
        i2s(VERSION_REV);
}


/**
 * @brief Returns the color a ledge's smoothing should be.
 *
 * @param e_ptr Edge with the ledge.
 * @return The color.
 */
ALLEGRO_COLOR get_ledge_smoothing_color(edge* e_ptr) {
    return e_ptr->ledge_smoothing_color;
}


/**
 * @brief Returns the length a ledge's smoothing should be.
 *
 * @param e_ptr Edge with the ledge.
 * @return The length.
 */
float get_ledge_smoothing_length(edge* e_ptr) {
    return e_ptr->ledge_smoothing_length;
}


/**
 * @brief Returns the color a liquid limit's effect should be.
 *
 * @param e_ptr Edge with the liquid limit.
 * @return The color.
 */
ALLEGRO_COLOR get_liquid_limit_color(edge* e_ptr) {
    return {1.0f, 1.0f, 1.0f, 0.75f};
}


/**
 * @brief Returns the length a liquid's limit effect.
 *
 * @param e_ptr Edge with the liquid limit.
 * @return The length.
 */
float get_liquid_limit_length(edge* e_ptr) {
    //Let's vary the length randomly by the topleftmost edge coordinates.
    //It's better to use this than using just the first edge, for instance,
    //because that would result in many cases of edges that share a first
    //vertex. So it wouldn't look as random.
    //It is much more rare for two edges to share a topleftmost vertex.
    float min_x = std::min(e_ptr->vertexes[0]->x, e_ptr->vertexes[1]->x);
    float min_y = std::min(e_ptr->vertexes[0]->y, e_ptr->vertexes[1]->y);
    float r = (hash_nr2(min_x, min_y) / (float) UINT32_MAX) * 5.0f;
    return
        15.0f +
        12.0f * sin((game.states.gameplay->area_time_passed * 2.0f) + r);
}


/**
 * @brief Returns the width and height of a block of multi-line text.
 *
 * Lines are split by a single "\n" character.
 * These are the dimensions of a bitmap
 * that would hold a drawing by draw_text_lines().
 *
 * @param font The text's font.
 * @param text The text.
 * @param out_ret_w If not nullptr, the width is returned here.
 * @param out_ret_h If not nullptr, the height is returned here.
 */
void get_multiline_text_dimensions(
    const ALLEGRO_FONT* const font, const string &text,
    int* out_ret_w, int* out_ret_h
) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(font);
    size_t n_lines = lines.size();
    
    if(out_ret_h) *out_ret_h = std::max(0, (int) ((fh + 1) * n_lines) - 1);
    
    if(out_ret_w) {
        int largest_w = 0;
        for(size_t l = 0; l < lines.size(); ++l) {
            largest_w =
                std::max(
                    largest_w, al_get_text_width(font, lines[l].c_str())
                );
        }
        
        *out_ret_w = largest_w;
    }
}


/**
 * @brief Returns an area's subtitle or, if none is specified,
 * the mission's goal.
 *
 * @param subtitle Area subtitle.
 * @param area_type Type of area.
 * @param goal Mission goal.
 * @return The subtitle or goal.
 */
string get_subtitle_or_mission_goal(
    const string &subtitle, const AREA_TYPE area_type,
    const MISSION_GOAL goal
) {
    if(subtitle.empty() && area_type == AREA_TYPE_MISSION) {
        return game.mission_goals[goal]->get_name();
    }
    
    return subtitle;
}


/**
 * @brief Calculates the vertex info necessary to draw the throw preview line,
 * from a given start point to a given end point.
 *
 * The vertexes returned always come in groups of four, and each group
 * must be drawn individually with the ALLEGRO_PRIM_TRIANGLE_FAN type.
 *
 * @param vertexes The array of vertexes to fill.
 * Must have room for at least 16.
 * @param start Start the line at this point.
 * This is a ratio from the leader (0) to the cursor (1).
 * @param end Same as start, but for the end point.
 * @param leader_pos Position of the leader.
 * @param cursor_pos Position of the cursor.
 * @param color Color of the line.
 * @param u_offset Offset the texture u by this much.
 * @param u_scale Scale the texture u by this much.
 * @param vary_thickness If true, thickness varies as the line goes
 * forward. False makes it use the same thickness (the minimal one) throughout.
 * @return The amount of vertexes needed.
 */
unsigned char get_throw_preview_vertexes(
    ALLEGRO_VERTEX* vertexes,
    const float start, const float end,
    const point &leader_pos, const point &cursor_pos,
    const ALLEGRO_COLOR &color,
    const float u_offset, const float u_scale,
    const bool vary_thickness
) {
    const float segment_points[] = {
        0.0f, LEADER::THROW_PREVIEW_FADE_IN_RATIO,
        0.5f, LEADER::THROW_PREVIEW_FADE_OUT_RATIO,
        1.0f
    };
    
    float max_thickness =
        vary_thickness ?
        LEADER::THROW_PREVIEW_DEF_MAX_THICKNESS :
        LEADER::THROW_PREVIEW_MIN_THICKNESS;
        
    float leader_to_cursor_dist = dist(leader_pos, cursor_pos).to_float();
    unsigned char cur_v = 0;
    
    auto get_thickness =
    [max_thickness] (float n) -> float {
        if(n >= 0.5f) {
            n = 1 - n;
        }
        return
        interpolate_number(
            n, 0.0f, 0.5f, LEADER::THROW_PREVIEW_MIN_THICKNESS, max_thickness
        );
    };
    auto get_color =
    [&color] (float n) -> ALLEGRO_COLOR {
        if(n >= 0.5f) {
            n = 1 - n;
        }
        if(n < LEADER::THROW_PREVIEW_FADE_IN_RATIO) {
            return
            interpolate_color(
                n, 0.0f, LEADER::THROW_PREVIEW_FADE_IN_RATIO,
                change_alpha(color, 0),
                color
            );
        } else {
            return color;
        }
    };
    
    //Get the vertexes of each necessary segment.
    for(unsigned char segment = 0; segment < 4; ++segment) {
        float segment_start = std::max(segment_points[segment], start);
        float segment_end = std::min(segment_points[segment + 1], end);
        
        if(
            segment_start > segment_points[segment + 1] ||
            segment_end < segment_points[segment]
        ) {
            continue;
        }
        
        vertexes[cur_v].x = leader_to_cursor_dist * segment_start;
        vertexes[cur_v].y = -get_thickness(segment_start) / 2.0f;
        vertexes[cur_v].color = get_color(segment_start);
        cur_v++;
        
        vertexes[cur_v] = vertexes[cur_v - 1];
        vertexes[cur_v].y = -vertexes[cur_v].y;
        cur_v++;
        
        vertexes[cur_v].x = leader_to_cursor_dist * segment_end;
        vertexes[cur_v].y = get_thickness(segment_end) / 2.0f;
        vertexes[cur_v].color = get_color(segment_end);
        cur_v++;
        
        vertexes[cur_v] = vertexes[cur_v - 1];
        vertexes[cur_v].y = -vertexes[cur_v].y;
        cur_v++;
    }
    
    //Final setup on all points.
    for(unsigned char v = 0; v < cur_v; ++v) {
        point p(vertexes[v].x, vertexes[v].y);
        
        //Apply the texture UVs.
        vertexes[v].u = vertexes[v].x / u_scale - u_offset;
        vertexes[v].v = vertexes[v].y;
        
        //Rotate and move all points. For the sake of simplicity, up until now,
        //they were assuming the throw is perfectly to the right (0 degrees),
        //and that it starts on the world origin.
        p = rotate_point(p, get_angle(leader_pos, cursor_pos));
        p += leader_pos;
        vertexes[v].x = p.x;
        vertexes[v].y = p.y;
        
        //Give Z a value.
        vertexes[v].z = 0.0f;
    }
    
    return cur_v;
}


/**
 * @brief Given a string representation of mob script variables,
 * returns a map, where every key is a variable, and every value is the
 * variable's value.
 *
 * @param vars_string String with the variables.
 * @return The map.
 */
map<string, string> get_var_map(const string &vars_string) {
    map<string, string> final_map;
    vector<string> raw_vars = semicolon_list_to_vector(vars_string);
    
    for(size_t v = 0; v < raw_vars.size(); ++v) {
        vector<string> raw_parts = split(raw_vars[v], "=");
        if(raw_parts.size() < 2) {
            continue;
        }
        final_map[trim_spaces(raw_parts[0])] = trim_spaces(raw_parts[1]);
    }
    return final_map;
}


/**
 * @brief Returns the color a wall's shadow should be.
 *
 * @param e_ptr Edge with the wall.
 * @return The color.
 */
ALLEGRO_COLOR get_wall_shadow_color(edge* e_ptr) {
    return e_ptr->wall_shadow_color;
}


/**
 * @brief Returns the length a wall's shadow should be.
 *
 * @param e_ptr Edge with the wall.
 * @return The length.
 */
float get_wall_shadow_length(edge* e_ptr) {
    if(e_ptr->wall_shadow_length != LARGE_FLOAT) {
        return e_ptr->wall_shadow_length;
    }
    
    float height_difference =
        fabs(e_ptr->sectors[0]->z - e_ptr->sectors[1]->z);
    return
        clamp(
            height_difference * GEOMETRY::SHADOW_AUTO_LENGTH_MULT,
            GEOMETRY::SHADOW_MIN_AUTO_LENGTH,
            GEOMETRY::SHADOW_MAX_AUTO_LENGTH
        );
}


/**
 * @brief Auxiliary function that returns a table used in the weather configs.
 *
 * @param node Data node with the weather table.
 * @return The table.
 */
vector<std::pair<int, string> > get_weather_table(data_node* node) {
    vector<std::pair<int, string> > table;
    size_t n_points = node->get_nr_of_children();
    
    for(size_t p = 0; p < n_points; ++p) {
        data_node* point_node = node->get_child(p);
        
        int point_time = s2i(point_node->name);
        string point_value = point_node->value;
        
        table.push_back(make_pair(point_time, point_value));
    }
    
    sort(
        table.begin(), table.end(),
        [] (
            std::pair<int, string> p1,
            std::pair<int, string> p2
    ) -> bool {
        return p1.first < p2.first;
    }
    );
    
    if(!table.empty()) {
        auto first = table.front();
        auto last = table.back();
        if(first.first > 0) {
            //If there is no data for midnight (0),
            //use the data from the last point
            //(this is because the day loops after 24:00;
            //needed for interpolation).
            table.insert(
                table.begin(),
                make_pair(last.first - 24 * 60, last.second)
            );
        }
        if(last.first < 24 * 60) {
            //If there is no data for midnight (24),
            //use the data from the first point
            //(this is because the day loops after 24:00;
            //needed for interpolation).
            table.push_back(
                make_pair(first.first + 24 * 60, first.second)
            );
        }
    }
    
    return table;
}


/**
 * @brief Prints a bit of info onto the screen, for some seconds.
 *
 * @param text Text to print. Can use line breaks.
 * @param total_duration Total amount of time in which the text is present.
 * @param fade_duration When closing, fade out in the last N seconds.
 */
void print_info(
    const string &text, const float total_duration, const float fade_duration
) {
    game.maker_tools.info_print_text = text;
    game.maker_tools.info_print_duration = total_duration;
    game.maker_tools.info_print_fade_duration = fade_duration;
    game.maker_tools.info_print_timer.start(total_duration);
}


/**
 * @brief Reports a fatal error to the user and shuts down the program.
 *
 * @param s String explaining the error.
 * @param dn File to log the error into, if any.
 */
void report_fatal_error(const string &s, const data_node* dn) {
    game.errors.report(s, dn);
    
    show_message_box(
        nullptr, "Fatal error!",
        "Pikifen has encountered a fatal error!",
        s.c_str(),
        nullptr,
        ALLEGRO_MESSAGEBOX_ERROR
    );
    
    exit(-1);
    
}


/**
 * @brief Saves the maker tools settings.
 */
void save_maker_tools() {
    data_node file("", "");
    
    file.add(
        new data_node("enabled", b2s(game.maker_tools.enabled))
    );
    
    for(unsigned char k = 0; k < 20; k++) {
        string tool_key;
        if(k < 10) {
            //The first ten indexes are the F2 - F11 keys.
            tool_key = "f" + i2s(k + 2);
        } else {
            //The second ten indexes are the 0 - 9 keys.
            tool_key = i2s(k - 10);
        }
        string tool_name = MAKER_TOOLS::NAMES[game.maker_tools.keys[k]];
        
        file.add(new data_node(tool_key, tool_name));
    }
    
    file.add(
        new data_node(
            "area_image_mobs", b2s(game.maker_tools.area_image_mobs)
        )
    );
    file.add(
        new data_node(
            "area_image_padding", f2s(game.maker_tools.area_image_padding)
        )
    );
    file.add(
        new data_node(
            "area_image_shadows", b2s(game.maker_tools.area_image_shadows)
        )
    );
    file.add(
        new data_node(
            "area_image_size", i2s(game.maker_tools.area_image_size)
        )
    );
    file.add(
        new data_node(
            "change_speed_multiplier", f2s(game.maker_tools.change_speed_mult)
        )
    );
    file.add(
        new data_node(
            "mob_hurting_percentage",
            f2s(game.maker_tools.mob_hurting_ratio * 100)
        )
    );
    
    file.add(
        new data_node(
            "auto_start_option", game.maker_tools.auto_start_option
        )
    );
    file.add(
        new data_node(
            "auto_start_mode", game.maker_tools.auto_start_mode
        )
    );
    file.add(
        new data_node(
            "performance_monitor", b2s(game.maker_tools.use_perf_mon)
        )
    );
    
    file.save_file(MAKER_TOOLS_FILE_PATH, true, true);
}


/**
 * @brief Saves the player's options.
 */
void save_options() {
    data_node file("", "");
    
    //Save the standard options.
    game.options.save(&file);
    
    //Also add the editor histories.
    for(
        size_t h = 0; h < game.states.animation_ed->history.size(); ++h
    ) {
        file.add(
            new data_node(
                game.states.animation_ed->get_history_option_prefix() +
                i2s(h + 1),
                game.states.animation_ed->history[h]
            )
        );
    }
    
    for(
        size_t h = 0; h < game.states.area_ed->history.size(); ++h
    ) {
        file.add(
            new data_node(
                game.states.area_ed->get_history_option_prefix() +
                i2s(h + 1),
                game.states.area_ed->history[h]
            )
        );
    }
    
    for(
        size_t h = 0; h < game.states.gui_ed->history.size(); ++h
    ) {
        file.add(
            new data_node(
                game.states.gui_ed->get_history_option_prefix() +
                i2s(h + 1),
                game.states.gui_ed->history[h]
            )
        );
    }
    
    //Finally, save.
    file.save_file(OPTIONS_FILE_PATH, true, true);
}


/**
 * @brief Saves the current backbuffer onto a file.
 * In other words, dumps a screenshot.
 */
void save_screenshot() {
    string base_file_name = "Screenshot_" + get_current_time(true);
    
    //Check if a file with this name already exists.
    vector<string> files = folder_to_vector(USER_DATA_FOLDER_PATH, false);
    size_t variant_nr = 1;
    string final_file_name = base_file_name;
    bool valid_name = false;
    
    do {
    
        if(
            find(files.begin(), files.end(), final_file_name + ".png")
            == files.end()
        ) {
            //File name not found.
            //Go ahead and create a screenshot with this name.
            valid_name = true;
        } else {
            variant_nr++;
            final_file_name = base_file_name + " " + i2s(variant_nr);
        }
        
    } while(!valid_name);
    
    //Before saving, let's set every pixel's alpha to 255.
    //This is because alpha operations on the backbuffer behave weirdly.
    //On some machines, when saving to a bitmap, it will use those weird
    //alpha values, which may be harmless on the backbuffer, but not so much
    //on a saved PNG file.
    ALLEGRO_BITMAP* screenshot =
        al_clone_bitmap(al_get_backbuffer(game.display));
    ALLEGRO_LOCKED_REGION* region =
        al_lock_bitmap(
            screenshot,
            ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_READWRITE
        );
        
    unsigned char* row = (unsigned char*) region->data;
    int bmp_w = ceil(al_get_bitmap_width(screenshot));
    int bmp_h = ceil(al_get_bitmap_height(screenshot));
    for(int y = 0; y < bmp_h; ++y) {
        for(int x = 0; x < bmp_w; ++x) {
            row[(x) * 4 + 3] = 255;
        }
        row += region->pitch;
    }
    
    al_unlock_bitmap(screenshot);
    
    al_save_bitmap(
        (USER_DATA_FOLDER_PATH + "/" + final_file_name + ".png").c_str(),
        screenshot
    );
    
    al_destroy_bitmap(screenshot);
}


/**
 * @brief Saves the engine's lifetime statistics.
 */
void save_statistics() {
    data_node stats_file("", "");
    const statistics_t &s = game.statistics;
    
#define save(n, v) stats_file.add(new data_node(n, v))
    
    save("startups", i2s(s.startups));
    save("runtime", f2s(s.runtime));
    save("gameplay_time", f2s(s.gameplay_time));
    save("area_entries", i2s(s.area_entries));
    save("pikmin_births", i2s(s.pikmin_births));
    save("pikmin_deaths", i2s(s.pikmin_deaths));
    save("pikmin_eaten", i2s(s.pikmin_eaten));
    save("pikmin_hazard_deaths", i2s(s.pikmin_hazard_deaths));
    save("pikmin_blooms", i2s(s.pikmin_blooms));
    save("pikmin_saved", i2s(s.pikmin_saved));
    save("enemy_deaths", i2s(s.enemy_deaths));
    save("pikmin_thrown", i2s(s.pikmin_thrown));
    save("whistle_uses", i2s(s.whistle_uses));
    save("distance_walked", f2s(s.distance_walked));
    save("leader_damage_suffered", f2s(s.leader_damage_suffered));
    save("punch_damage_caused", f2s(s.punch_damage_caused));
    save("leader_kos", i2s(s.leader_kos));
    save("sprays_used", i2s(s.sprays_used));
    
#undef save
    
    stats_file.save_file(STATISTICS_FILE_PATH, true, true, true);
}


/**
 * @brief Sets the width of all string tokens in a vector of tokens.
 *
 * @param tokens Vector of tokens to set the widths of.
 * @param text_font Text font.
 * @param control_font Font for control bind icons.
 * @param max_control_bitmap_height If bitmap icons need to be condensed
 * vertically to fit a certain space, then their width will be affected too.
 * Specify the maximum height here. Use 0 to indicate no maximum height.
 * @param control_condensed If true, control bind player icons are condensed.
 */
void set_string_token_widths(
    vector<string_token> &tokens,
    const ALLEGRO_FONT* text_font, const ALLEGRO_FONT* control_font,
    const float max_control_bitmap_height, bool control_condensed
) {
    for(size_t t = 0; t < tokens.size(); ++t) {
        switch(tokens[t].type) {
        case STRING_TOKEN_CHAR: {
            tokens[t].width =
                al_get_text_width(text_font, tokens[t].content.c_str());
            break;
        } case STRING_TOKEN_CONTROL_BIND: {
            tokens[t].content = trim_spaces(tokens[t].content);
            tokens[t].width =
                get_player_input_icon_width(
                    control_font,
                    game.controls.find_bind(tokens[t].content).input,
                    control_condensed,
                    max_control_bitmap_height
                );
        }
        default: {
            break;
        }
        }
    }
}


/**
 * @brief Handles a system signal.
 *
 * @param signum Signal number.
 */
void signal_handler(const int signum) {
    volatile static bool already_handling_signal = false;
    
    if(already_handling_signal) {
        //This stops an infinite loop if there's a signal raise
        //inside this function. It shouldn't happen, but better be safe.
        exit(signum);
    }
    already_handling_signal = true;
    
    string bt_str = "Backtrace:\n";
    vector<string> bt = get_backtrace();
    for(size_t s = 0; s < bt.size(); ++s) {
        bt_str += "    " + bt[s] + "\n";
    }
    if(bt_str.back() == '\n') {
        bt_str.pop_back();
    }
    string signal_name(strsignal(signum));
    string type_str = "Signal " + i2s(signum) + " (" + signal_name + ")";
    
    crash(type_str, bt_str, signum);
}


/**
 * @brief Spews out a Pikmin from a given point. Used by Onions and converters.
 *
 * @param pos Point of origin.
 * @param z Z of the point of origin.
 * @param pik_type Type of the Pikmin to spew out.
 * @param angle Direction in which to spew.
 * @param horizontal_speed Horizontal speed in which to spew.
 * @param vertical_speed Vertical speed in which to spew.
 */
void spew_pikmin_seed(
    const point pos, const float z, pikmin_type* pik_type,
    const float angle, const float horizontal_speed, const float vertical_speed
) {
    pikmin* new_pikmin =
        (
            (pikmin*)
            create_mob(
                game.mob_categories.get(MOB_CATEGORY_PIKMIN),
                pos, pik_type, angle, "", nullptr, PIKMIN_STATE_SEED
            )
        );
    new_pikmin->z = z;
    new_pikmin->speed.x = cos(angle) * horizontal_speed;
    new_pikmin->speed.y = sin(angle) * horizontal_speed;
    new_pikmin->speed_z = vertical_speed;
    new_pikmin->maturity = 0;
}


/**
 * @brief Splits a long string, composed of string tokens,
 * into different line breaks, such that no line goes over the limit
 * unless necessary.
 *
 * @param tokens Tokens that make up the string.
 * @param max_width Maximum width of each line.
 * @return The lines.
 */
vector<vector<string_token> > split_long_string_with_tokens(
    const vector<string_token> &tokens, const int max_width
) {
    vector<vector<string_token> > tokens_per_line;
    if(tokens.empty()) return tokens_per_line;
    
    tokens_per_line.push_back(vector<string_token>());
    size_t cur_line_idx = 0;
    unsigned int caret = 0;
    vector<string_token> word_buffer;
    unsigned int word_buffer_width = 0;
    
    for(size_t t = 0; t < tokens.size() + 1; ++t) {
    
        bool token_is_space =
            t != tokens.size() &&
            tokens[t].type == STRING_TOKEN_CHAR && tokens[t].content == " ";
        bool token_is_line_break =
            t != tokens.size() &&
            tokens[t].type == STRING_TOKEN_LINE_BREAK;
            
        if(t == tokens.size() || token_is_space || token_is_line_break) {
            //Found a point where we can end a word.
            
            int caret_after_word = caret + word_buffer_width;
            bool line_will_be_too_long =
                caret > 0 && caret_after_word > max_width;
                
            if(line_will_be_too_long) {
                //Break to a new line before comitting the word.
                tokens_per_line.push_back(vector<string_token>());
                caret = 0;
                cur_line_idx++;
                
                //Remove the previous line's trailing space, if any.
                string_token &prev_tail =
                    tokens_per_line[cur_line_idx - 1].back();
                if(
                    prev_tail.type == STRING_TOKEN_CHAR &&
                    prev_tail.content == " "
                ) {
                    tokens_per_line[cur_line_idx - 1].pop_back();
                }
            }
            
            //Add the word to the current line.
            if(t < tokens.size()) {
                word_buffer.push_back(tokens[t]);
                word_buffer_width += tokens[t].width;
            }
            tokens_per_line[cur_line_idx].insert(
                tokens_per_line[cur_line_idx].end(),
                word_buffer.begin(), word_buffer.end()
            );
            caret += word_buffer_width;
            word_buffer.clear();
            word_buffer_width = 0;
            
            if(token_is_line_break) {
                //Break the line after comitting the word.
                tokens_per_line.push_back(vector<string_token>());
                caret = 0;
                cur_line_idx++;
            }
            
            continue;
            
        }
        
        //Add the token to the word buffer.
        word_buffer.push_back(tokens[t]);
        word_buffer_width += tokens[t].width;
    }
    
    return tokens_per_line;
}


/**
 * @brief Starts the display of a text message.
 *
 * If the text is empty, it closes the message box.
 * Any newline characters or slashes followed by n ("\n") will be used to
 * separate the message into lines.
 *
 * @param text Text to display.
 * @param speaker_bmp Bitmap representing the speaker.
 */
void start_message(const string &text, ALLEGRO_BITMAP* speaker_bmp) {
    if(!text.empty()) {
        string final_text = unescape_string(text);
        game.states.gameplay->msg_box =
            new msg_box_t(final_text, speaker_bmp);
        game.states.gameplay->hud->gui.start_animation(
            GUI_MANAGER_ANIM_IN_TO_OUT,
            GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
        );
    } else {
        delete game.states.gameplay->msg_box;
        game.states.gameplay->msg_box = nullptr;
        game.states.gameplay->hud->gui.start_animation(
            GUI_MANAGER_ANIM_OUT_TO_IN,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    }
}


/**
 * @brief Returns the tokens that make up a string.
 * This does not set the tokens's width.
 *
 * @param s String to tokenize.
 * @return The tokens.
 */
vector<string_token> tokenize_string(const string &s) {
    vector<string_token> tokens;
    string_token cur_token;
    cur_token.type = STRING_TOKEN_CHAR;
    
    for(size_t c = 0; c < s.size(); ++c) {
        if(str_peek(s, c, "\\\\")) {
            cur_token.content.push_back('\\');
            if(cur_token.type == STRING_TOKEN_CHAR) {
                tokens.push_back(cur_token);
                cur_token.content.clear();
            }
            c++;
            
        } else if(str_peek(s, c, "\\k")) {
            if(!cur_token.content.empty()) tokens.push_back(cur_token);
            cur_token.content.clear();
            if(cur_token.type != STRING_TOKEN_CONTROL_BIND) {
                cur_token.type = STRING_TOKEN_CONTROL_BIND;
            } else {
                cur_token.type = STRING_TOKEN_CHAR;
            }
            c++;
            
        } else if(s[c] == '\n' || str_peek(s, c, "\\n")) {
            if(!cur_token.content.empty()) tokens.push_back(cur_token);
            cur_token.content.clear();
            cur_token.type = STRING_TOKEN_LINE_BREAK;
            tokens.push_back(cur_token);
            cur_token.type = STRING_TOKEN_CHAR;
            if(s[c] != '\n') c++;
            
        } else {
            cur_token.content.push_back(s[c]);
            if(cur_token.type == STRING_TOKEN_CHAR) {
                tokens.push_back(cur_token);
                cur_token.content.clear();
            }
            
        }
    }
    if(!cur_token.content.empty()) tokens.push_back(cur_token);
    
    return tokens;
}


/**
 * @brief Unescapes a user string. This converts two backslashes into one, and
 * converts backslash n into a newline character.
 *
 * @param s String to unescape.
 * @return The unescaped string.
 */
string unescape_string(const string &s) {
    if(s.empty()) return s;
    
    string ret;
    ret.reserve(s.size());
    for(size_t c = 0; c < s.size() - 1;) {
        if(s[c] == '\\') {
            switch(s[c + 1]) {
            case 'n': {
                ret.push_back('\n');
                c += 2;
                break;
            } case '\\': {
                ret.push_back('\\');
                c += 2;
                break;
            } default: {
                ret.push_back('\\');
                ++c;
                break;
            }
            }
        } else {
            ret.push_back(s[c]);
            ++c;
        }
    }
    ret.push_back(s.back());
    return ret;
}
