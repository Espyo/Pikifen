/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area class, and related functions.
 */

#include <algorithm>

#include "area.h"

#include "functions.h"
#include "game.h"
#include "utils/string_utils.h"


namespace AREA {
//Default difficulty.
const unsigned char DEF_DIFFICULTY = 3;
//Default day time at the start of gameplay, in minutes.
const size_t DEF_DAY_TIME_START = 7 * 60;
//Default day time speed, in game-minutes per real-minutes.
const float DEF_DAY_TIME_SPEED = 120;
//Default mission bronze medal point requirement.
const int DEF_MISSION_MEDAL_BRONZE_REQ = 1000;
//Default mission silver medal point requirement.
const int DEF_MISSION_MEDAL_SILVER_REQ = 2000;
//Default mission gold medal point requirement.
const int DEF_MISSION_MEDAL_GOLD_REQ = 3000;
//Default mission platinum medal point requirement.
const int DEF_MISSION_MEDAL_PLATINUM_REQ = 4000;
//Default mission time limit duration, in seconds.
const size_t DEF_MISSION_TIME_LIMIT = 60;
};


/* ----------------------------------------------------------------------------
 * Creates info on an area.
 */
area_data::area_data() :
    type(AREA_TYPE_SIMPLE),
    bg_bmp(nullptr),
    bg_bmp_zoom(1),
    bg_dist(2),
    bg_color(map_gray(0)),
    thumbnail(nullptr),
    difficulty(AREA::DEF_DIFFICULTY),
    day_time_start(AREA::DEF_DAY_TIME_START),
    day_time_speed(AREA::DEF_DAY_TIME_SPEED) {
    
}


/* ----------------------------------------------------------------------------
 * A debugging tool. This checks to see if all numbers match their pointers,
 * for the various edges, vertexes, etc. Aborts execution if any doesn't.
 */
void area_data::check_stability() {
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        engine_assert(
            v_ptr->edges.size() == v_ptr->edge_nrs.size(),
            i2s(v_ptr->edges.size()) + " " + i2s(v_ptr->edge_nrs.size())
        );
        for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
            engine_assert(v_ptr->edges[e] == edges[v_ptr->edge_nrs[e]], "");
        }
    }
    
    for(size_t e = 0; e < edges.size(); ++e) {
        edge* e_ptr = edges[e];
        for(size_t v = 0; v < 2; ++v) {
            engine_assert(
                e_ptr->vertexes[v] == vertexes[e_ptr->vertex_nrs[v]], ""
            );
        }
        
        for(size_t s = 0; s < 2; ++s) {
            sector* s_ptr = e_ptr->sectors[s];
            if(
                s_ptr == NULL &&
                e_ptr->sector_nrs[s] == INVALID
            ) {
                continue;
            }
            engine_assert(s_ptr == sectors[e_ptr->sector_nrs[s]], "");
        }
    }
    
    for(size_t s = 0; s < sectors.size(); ++s) {
        sector* s_ptr = sectors[s];
        engine_assert(
            s_ptr->edges.size() == s_ptr->edge_nrs.size(),
            i2s(s_ptr->edges.size()) + " " + i2s(s_ptr->edge_nrs.size())
        );
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            engine_assert(s_ptr->edges[e] == edges[s_ptr->edge_nrs[e]], "");
        }
    }
}


/* ----------------------------------------------------------------------------
 * Clears the info of an area map.
 */
void area_data::clear() {
    for(size_t v = 0; v < vertexes.size(); ++v) {
        delete vertexes[v];
    }
    for(size_t e = 0; e < edges.size(); ++e) {
        delete edges[e];
    }
    for(size_t s = 0; s < sectors.size(); ++s) {
        delete sectors[s];
    }
    for(size_t m = 0; m < mob_generators.size(); ++m) {
        delete mob_generators[m];
    }
    for(size_t s = 0; s < path_stops.size(); ++s) {
        delete path_stops[s];
    }
    for(size_t s = 0; s < tree_shadows.size(); ++s) {
        delete tree_shadows[s];
    }
    
    vertexes.clear();
    edges.clear();
    sectors.clear();
    mob_generators.clear();
    path_stops.clear();
    tree_shadows.clear();
    bmap.clear();
    
    if(bg_bmp) {
        game.bitmaps.detach(bg_bmp);
        bg_bmp = NULL;
    }
    if(thumbnail) {
        thumbnail = NULL;
    }
    
    name.clear();
    folder_name.clear();
    type = AREA_TYPE_SIMPLE;
    subtitle.clear();
    description.clear();
    tags.clear();
    difficulty = AREA::DEF_DIFFICULTY;
    maker.clear();
    version.clear();
    notes.clear();
    engine_version.clear();
    spray_amounts.clear();
    weather_name.clear();
    day_time_start = AREA::DEF_DAY_TIME_START;
    day_time_speed = AREA::DEF_DAY_TIME_SPEED;
    bg_bmp_file_name.clear();
    bg_color = COLOR_BLACK;
    bg_dist = 2.0f;
    bg_bmp_zoom = 1.0f;
    mission.goal = MISSION_GOAL_END_MANUALLY;
    mission.goal_all_mobs = true;
    mission.goal_mob_idxs.clear();
    mission.goal_amount = 1;
    mission.goal_exit_center = point();
    mission.goal_exit_size =
        point(
            AREA_EDITOR::MISSION_EXIT_MIN_SIZE,
            AREA_EDITOR::MISSION_EXIT_MIN_SIZE
        );
    mission.fail_conditions = 0;
    mission.fail_too_few_pik_amount = 0;
    mission.fail_too_many_pik_amount = 1;
    mission.fail_pik_killed = 1;
    mission.fail_leaders_kod = 1;
    mission.fail_enemies_killed = 1;
    mission.fail_time_limit = AREA::DEF_MISSION_TIME_LIMIT;
    mission.grading_mode = MISSION_GRADING_GOAL;
    mission.points_per_pikmin_born = 0;
    mission.points_per_pikmin_death = 0;
    mission.points_per_sec_left = 0;
    mission.points_per_sec_passed = 0;
    mission.points_per_treasure_point = 0;
    mission.points_per_enemy_point = 0;
    mission.point_loss_data = 0;
    mission.point_hud_data = 255;
    mission.starting_points = 0;
    mission.bronze_req = AREA::DEF_MISSION_MEDAL_BRONZE_REQ;
    mission.silver_req = AREA::DEF_MISSION_MEDAL_SILVER_REQ;
    mission.gold_req = AREA::DEF_MISSION_MEDAL_GOLD_REQ;
    mission.platinum_req = AREA::DEF_MISSION_MEDAL_PLATINUM_REQ;
    
    problems.non_simples.clear();
    problems.lone_edges.clear();
}


/* ----------------------------------------------------------------------------
 * Clones this area data into another area_data object.
 * other:
 *   The area data object to clone to.
 */
void area_data::clone(area_data &other) {
    other.clear();
    
    if(!other.bg_bmp_file_name.empty() && other.bg_bmp) {
        game.bitmaps.detach(other.bg_bmp_file_name);
    }
    other.bg_bmp_file_name = bg_bmp_file_name;
    if(other.bg_bmp_file_name.empty()) {
        other.bg_bmp = NULL;
    } else {
        other.bg_bmp = game.bitmaps.get(bg_bmp_file_name, NULL, false);
    }
    other.bg_bmp_zoom = bg_bmp_zoom;
    other.bg_color = bg_color;
    other.bg_dist = bg_dist;
    other.bmap = bmap;
    
    other.vertexes.reserve(vertexes.size());
    for(size_t v = 0; v < vertexes.size(); ++v) {
        other.vertexes.push_back(new vertex());
    }
    other.edges.reserve(edges.size());
    for(size_t e = 0; e < edges.size(); ++e) {
        other.edges.push_back(new edge());
    }
    other.sectors.reserve(sectors.size());
    for(size_t s = 0; s < sectors.size(); ++s) {
        other.sectors.push_back(new sector());
    }
    other.mob_generators.reserve(mob_generators.size());
    for(size_t m = 0; m < mob_generators.size(); ++m) {
        other.mob_generators.push_back(new mob_gen());
    }
    other.path_stops.reserve(path_stops.size());
    for(size_t s = 0; s < path_stops.size(); ++s) {
        other.path_stops.push_back(new path_stop());
    }
    other.tree_shadows.reserve(tree_shadows.size());
    for(size_t t = 0; t < tree_shadows.size(); ++t) {
        other.tree_shadows.push_back(new tree_shadow());
    }
    
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        vertex* ov_ptr = other.vertexes[v];
        ov_ptr->x = v_ptr->x;
        ov_ptr->y = v_ptr->y;
        ov_ptr->edges.reserve(v_ptr->edges.size());
        ov_ptr->edge_nrs.reserve(v_ptr->edge_nrs.size());
        for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
            size_t nr = v_ptr->edge_nrs[e];
            ov_ptr->edges.push_back(other.edges[nr]);
            ov_ptr->edge_nrs.push_back(nr);
        }
    }
    
    for(size_t e = 0; e < edges.size(); ++e) {
        edge* e_ptr = edges[e];
        edge* oe_ptr = other.edges[e];
        oe_ptr->vertexes[0] = other.vertexes[e_ptr->vertex_nrs[0]];
        oe_ptr->vertexes[1] = other.vertexes[e_ptr->vertex_nrs[1]];
        oe_ptr->vertex_nrs[0] = e_ptr->vertex_nrs[0];
        oe_ptr->vertex_nrs[1] = e_ptr->vertex_nrs[1];
        if(e_ptr->sector_nrs[0] == INVALID) {
            oe_ptr->sectors[0] = NULL;
        } else {
            oe_ptr->sectors[0] = other.sectors[e_ptr->sector_nrs[0]];
        }
        if(e_ptr->sector_nrs[1] == INVALID) {
            oe_ptr->sectors[1] = NULL;
        } else {
            oe_ptr->sectors[1] = other.sectors[e_ptr->sector_nrs[1]];
        }
        oe_ptr->sector_nrs[0] = e_ptr->sector_nrs[0];
        oe_ptr->sector_nrs[1] = e_ptr->sector_nrs[1];
        e_ptr->clone(oe_ptr);
    }
    
    for(size_t s = 0; s < sectors.size(); ++s) {
        sector* s_ptr = sectors[s];
        sector* os_ptr = other.sectors[s];
        s_ptr->clone(os_ptr);
        os_ptr->texture_info.file_name = s_ptr->texture_info.file_name;
        os_ptr->texture_info.bitmap =
            game.textures.get(s_ptr->texture_info.file_name, NULL, false);
        os_ptr->edges.reserve(s_ptr->edges.size());
        os_ptr->edge_nrs.reserve(s_ptr->edge_nrs.size());
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            size_t nr = s_ptr->edge_nrs[e];
            os_ptr->edges.push_back(other.edges[nr]);
            os_ptr->edge_nrs.push_back(nr);
        }
        os_ptr->triangles.reserve(s_ptr->triangles.size());
        for(size_t t = 0; t < s_ptr->triangles.size(); ++t) {
            triangle* t_ptr = &s_ptr->triangles[t];
            os_ptr->triangles.push_back(
                triangle(
                    other.vertexes[find_vertex_nr(t_ptr->points[0])],
                    other.vertexes[find_vertex_nr(t_ptr->points[1])],
                    other.vertexes[find_vertex_nr(t_ptr->points[2])]
                )
            );
        }
        os_ptr->bbox[0] = s_ptr->bbox[0];
        os_ptr->bbox[1] = s_ptr->bbox[1];
    }
    
    for(size_t m = 0; m < mob_generators.size(); ++m) {
        mob_gen* m_ptr = mob_generators[m];
        mob_gen* om_ptr = other.mob_generators[m];
        om_ptr->angle = m_ptr->angle;
        om_ptr->pos = m_ptr->pos;
        om_ptr->type = m_ptr->type;
        om_ptr->vars = m_ptr->vars;
        om_ptr->link_nrs = m_ptr->link_nrs;
    }
    for(size_t m = 0; m < mob_generators.size(); ++m) {
        mob_gen* om_ptr = other.mob_generators[m];
        for(size_t l = 0; l < om_ptr->link_nrs.size(); ++l) {
            om_ptr->links.push_back(
                other.mob_generators[om_ptr->link_nrs[l]]
            );
        }
    }
    
    for(size_t s = 0; s < path_stops.size(); ++s) {
        path_stop* s_ptr = path_stops[s];
        path_stop* os_ptr = other.path_stops[s];
        os_ptr->pos = s_ptr->pos;
        os_ptr->links.reserve(s_ptr->links.size());
        for(size_t l = 0; l < s_ptr->links.size(); ++l) {
            path_link* new_link =
                new path_link(
                os_ptr,
                other.path_stops[s_ptr->links[l]->end_nr],
                s_ptr->links[l]->end_nr
            );
            new_link->distance = s_ptr->links[l]->distance;
            new_link->type = s_ptr->links[l]->type;
            new_link->label = s_ptr->links[l]->label;
            os_ptr->links.push_back(new_link);
        }
    }
    
    for(size_t t = 0; t < tree_shadows.size(); ++t) {
        tree_shadow* t_ptr = tree_shadows[t];
        tree_shadow* ot_ptr = other.tree_shadows[t];
        ot_ptr->alpha = t_ptr->alpha;
        ot_ptr->angle = t_ptr->angle;
        ot_ptr->center = t_ptr->center;
        ot_ptr->file_name = t_ptr->file_name;
        ot_ptr->size = t_ptr->size;
        ot_ptr->sway = t_ptr->sway;
        ot_ptr->bitmap = game.textures.get(t_ptr->file_name, NULL, false);
    }
    
    other.type = type;
    other.folder_name = folder_name;
    other.name = name;
    other.subtitle = subtitle;
    other.description = description;
    other.tags = tags;
    other.difficulty = difficulty;
    other.maker = maker;
    other.version = version;
    other.notes = notes;
    other.spray_amounts = spray_amounts;
    other.weather_name = weather_name;
    other.weather_condition = weather_condition;
    other.day_time_start = day_time_start;
    other.day_time_speed = day_time_speed;
    
    other.thumbnail = thumbnail;
    
    other.mission.goal = mission.goal;
    other.mission.goal_all_mobs = mission.goal_all_mobs;
    other.mission.goal_mob_idxs = mission.goal_mob_idxs;
    other.mission.goal_amount = mission.goal_amount;
    other.mission.goal_exit_center = mission.goal_exit_center;
    other.mission.goal_exit_size = mission.goal_exit_size;
    other.mission.fail_conditions = mission.fail_conditions;
    other.mission.fail_too_few_pik_amount = mission.fail_too_few_pik_amount;
    other.mission.fail_too_many_pik_amount = mission.fail_too_many_pik_amount;
    other.mission.fail_pik_killed = mission.fail_pik_killed;
    other.mission.fail_leaders_kod = mission.fail_leaders_kod;
    other.mission.fail_enemies_killed = mission.fail_enemies_killed;
    other.mission.fail_time_limit = mission.fail_time_limit;
    other.mission.grading_mode = mission.grading_mode;
    other.mission.points_per_pikmin_born = mission.points_per_pikmin_born;
    other.mission.points_per_pikmin_death = mission.points_per_pikmin_death;
    other.mission.points_per_sec_left = mission.points_per_sec_left;
    other.mission.points_per_sec_passed = mission.points_per_sec_passed;
    other.mission.points_per_treasure_point = mission.points_per_treasure_point;
    other.mission.points_per_enemy_point = mission.points_per_enemy_point;
    other.mission.point_loss_data = mission.point_loss_data;
    other.mission.point_hud_data = mission.point_hud_data;
    other.mission.starting_points = mission.starting_points;
    other.mission.bronze_req = mission.bronze_req;
    other.mission.silver_req = mission.silver_req;
    other.mission.gold_req = mission.gold_req;
    other.mission.platinum_req = mission.platinum_req;
    
    other.problems.non_simples.clear();
    other.problems.lone_edges.clear();
    other.problems.lone_edges.reserve(problems.lone_edges.size());
    for(const auto &s : problems.non_simples) {
        size_t nr = find_sector_nr(s.first);
        other.problems.non_simples[other.sectors[nr]] = s.second;
    }
    for(const edge* e : problems.lone_edges) {
        size_t nr = find_edge_nr(e);
        other.problems.lone_edges.insert(other.edges[nr]);
    }
}


/* ----------------------------------------------------------------------------
 * Connects an edge to a sector, adding the sector and its number to the edge's
 * lists, and adding the edge and its number to the sector's.
 * e_ptr:
 *   Edge to connect.
 * s_ptr:
 *   Sector to connect.
 * side:
 *   Which of the sides of the edge the sector goes to.
 */
void area_data::connect_edge_to_sector(
    edge* e_ptr, sector* s_ptr, size_t side
) {
    if(e_ptr->sectors[side]) {
        e_ptr->sectors[side]->remove_edge(e_ptr);
    }
    e_ptr->sectors[side] = s_ptr;
    e_ptr->sector_nrs[side] = find_sector_nr(s_ptr);
    if(s_ptr) {
        s_ptr->add_edge(e_ptr, find_edge_nr(e_ptr));
    }
}


/* ----------------------------------------------------------------------------
 * Connects an edge to a vertex, adding the vertex and its number to the edge's
 * lists, and adding the edge and its number to the vertex's.
 * e_ptr:
 *   Edge to connect.
 * v_ptr:
 *   Vertex to connect.
 * endpoint:
 *   Which of the edge endpoints the vertex goes to.
 */
void area_data::connect_edge_to_vertex(
    edge* e_ptr, vertex* v_ptr, size_t endpoint
) {
    if(e_ptr->vertexes[endpoint]) {
        e_ptr->vertexes[endpoint]->remove_edge(e_ptr);
    }
    e_ptr->vertexes[endpoint] = v_ptr;
    e_ptr->vertex_nrs[endpoint] = find_vertex_nr(v_ptr);
    v_ptr->add_edge(e_ptr, find_edge_nr(e_ptr));
}



/* ----------------------------------------------------------------------------
 * Connects the edges of a sector that link to it into the edge_nrs vector.
 * s_ptr:
 *   The sector.
 */
void area_data::connect_sector_edges(sector* s_ptr) {
    s_ptr->edge_nrs.clear();
    for(size_t e = 0; e < edges.size(); ++e) {
        edge* e_ptr = edges[e];
        if(e_ptr->sectors[0] == s_ptr || e_ptr->sectors[1] == s_ptr) {
            s_ptr->edge_nrs.push_back(e);
        }
    }
    fix_sector_pointers(s_ptr);
}


/* ----------------------------------------------------------------------------
 * Connects the edges that link to it into the edge_nrs vector.
 * v_ptr:
 *   The vertex.
 */
void area_data::connect_vertex_edges(vertex* v_ptr) {
    v_ptr->edge_nrs.clear();
    for(size_t e = 0; e < edges.size(); ++e) {
        edge* e_ptr = edges[e];
        if(e_ptr->vertexes[0] == v_ptr || e_ptr->vertexes[1] == v_ptr) {
            v_ptr->edge_nrs.push_back(e);
        }
    }
    fix_vertex_pointers(v_ptr);
}


/* ----------------------------------------------------------------------------
 * Scans the list of edges and retrieves the number of the specified edge.
 * Returns INVALID if not found.
 * e_ptr:
 *   Edge to find.
 */
size_t area_data::find_edge_nr(const edge* e_ptr) const {
    for(size_t e = 0; e < edges.size(); ++e) {
        if(edges[e] == e_ptr) return e;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Scans the list of mob generators and retrieves the number of
 * the specified mob generator. Returns INVALID if not found.
 * m_ptr:
 *   Mob to find.
 */
size_t area_data::find_mob_gen_nr(const mob_gen* m_ptr) const {
    for(size_t m = 0; m < mob_generators.size(); ++m) {
        if(mob_generators[m] == m_ptr) return m;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Scans the list of sectors and retrieves the number of the specified sector.
 * Returns INVALID if not found.
 * s_ptr:
 *   Sector to find.
 */
size_t area_data::find_sector_nr(const sector* s_ptr) const {
    for(size_t s = 0; s < sectors.size(); ++s) {
        if(sectors[s] == s_ptr) return s;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Scans the list of vertexes and retrieves the number of the specified vertex.
 * Returns INVALID if not found.
 * v_ptr:
 *   Vertex to find.
 */
size_t area_data::find_vertex_nr(const vertex* v_ptr) const {
    for(size_t v = 0; v < vertexes.size(); ++v) {
        if(vertexes[v] == v_ptr) return v;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Fixes the sector and vertex numbers in an edge,
 * making them match the correct sectors and vertexes,
 * based on the existing sector and vertex pointers.
 * e_ptr:
 *   Edge to fix the numbers of.
 */
void area_data::fix_edge_nrs(edge* e_ptr) {
    for(size_t s = 0; s < 2; ++s) {
        if(!e_ptr->sectors[s]) {
            e_ptr->sector_nrs[s] = INVALID;
        } else {
            e_ptr->sector_nrs[s] = find_sector_nr(e_ptr->sectors[s]);
        }
    }
    
    for(size_t v = 0; v < 2; ++v) {
        if(!e_ptr->vertexes[v]) {
            e_ptr->vertex_nrs[v] = INVALID;
        } else {
            e_ptr->vertex_nrs[v] = find_vertex_nr(e_ptr->vertexes[v]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Fixes the sector and vertex pointers of an edge,
 * making them point to the correct sectors and vertexes,
 * based on the existing sector and vertex numbers.
 * e_ptr:
 *   Edge to fix the pointers of.
 */
void area_data::fix_edge_pointers(edge* e_ptr) {
    e_ptr->sectors[0] = NULL;
    e_ptr->sectors[1] = NULL;
    for(size_t s = 0; s < 2; ++s) {
        size_t s_nr = e_ptr->sector_nrs[s];
        e_ptr->sectors[s] = (s_nr == INVALID ? NULL : sectors[s_nr]);
    }
    
    e_ptr->vertexes[0] = NULL;
    e_ptr->vertexes[1] = NULL;
    for(size_t v = 0; v < 2; ++v) {
        size_t v_nr = e_ptr->vertex_nrs[v];
        e_ptr->vertexes[v] = (v_nr == INVALID ? NULL : vertexes[v_nr]);
    }
}


/* ----------------------------------------------------------------------------
 * Fixes the path stop numbers in a path stop's links,
 * making them match the correct path stops,
 * based on the existing path stop pointers.
 * s_ptr:
 *   Path stop to fix the numbers of.
 */
void area_data::fix_path_stop_nrs(path_stop* s_ptr) {
    for(size_t l = 0; l < s_ptr->links.size(); ++l) {
        path_link* l_ptr = s_ptr->links[l];
        l_ptr->end_nr = INVALID;
        
        if(!l_ptr->end_ptr) continue;
        
        for(size_t s = 0; s < path_stops.size(); ++s) {
            if(l_ptr->end_ptr == path_stops[s]) {
                l_ptr->end_nr = s;
                break;
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Fixes the path stop pointers in a path stop's links,
 * making them point to the correct path stops,
 * based on the existing path stop numbers.
 * s_ptr:
 *   Path stop to fix the pointers of.
 */
void area_data::fix_path_stop_pointers(path_stop* s_ptr) {
    for(size_t l = 0; l < s_ptr->links.size(); ++l) {
        path_link* l_ptr = s_ptr->links[l];
        l_ptr->end_ptr = NULL;
        
        if(l_ptr->end_nr == INVALID) continue;
        if(l_ptr->end_nr >= path_stops.size()) continue;
        
        l_ptr->end_ptr = path_stops[l_ptr->end_nr];
    }
}


/* ----------------------------------------------------------------------------
 * Fixes the edge numbers in a sector, making them match the correct edges,
 * based on the existing edge pointers.
 * s_ptr:
 *   Sector to fix the numbers of.
 */
void area_data::fix_sector_nrs(sector* s_ptr) {
    s_ptr->edge_nrs.clear();
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        s_ptr->edge_nrs.push_back(find_edge_nr(s_ptr->edges[e]));
    }
}


/* ----------------------------------------------------------------------------
 * Fixes the edge pointers in a sector, making them point to the correct edges,
 * based on the existing edge numbers.
 * s_ptr:
 *   Sector to fix the pointers of.
 */
void area_data::fix_sector_pointers(sector* s_ptr) {
    s_ptr->edges.clear();
    for(size_t e = 0; e < s_ptr->edge_nrs.size(); ++e) {
        size_t e_nr = s_ptr->edge_nrs[e];
        s_ptr->edges.push_back(e_nr == INVALID ? NULL : edges[e_nr]);
    }
}


/* ----------------------------------------------------------------------------
 * Fixes the edge numbers in a vertex, making them match the correct edges,
 * based on the existing edge pointers.
 * v_ptr:
 *   Vertex to fix the numbers of.
 */
void area_data::fix_vertex_nrs(vertex* v_ptr) {
    v_ptr->edge_nrs.clear();
    for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
        v_ptr->edge_nrs.push_back(find_edge_nr(v_ptr->edges[e]));
    }
}


/* ----------------------------------------------------------------------------
 * Fixes the edge pointers in a vertex, making them point to the correct edges,
 * based on the existing edge numbers.
 * v_ptr:
 *   Vertex to fix the pointers of.
 */
void area_data::fix_vertex_pointers(vertex* v_ptr) {
    v_ptr->edges.clear();
    for(size_t e = 0; e < v_ptr->edge_nrs.size(); ++e) {
        size_t e_nr = v_ptr->edge_nrs[e];
        v_ptr->edges.push_back(e_nr == INVALID ? NULL : edges[e_nr]);
    }
}


/* ----------------------------------------------------------------------------
 * Generates the blockmap for the area, given the current info.
 */
void area_data::generate_blockmap() {
    bmap.clear();
    
    if(vertexes.empty()) return;
    
    //First, get the starting point and size of the blockmap.
    point min_coords, max_coords;
    min_coords.x = max_coords.x = vertexes[0]->x;
    min_coords.y = max_coords.y = vertexes[0]->y;
    
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        min_coords.x = std::min(v_ptr->x, min_coords.x);
        max_coords.x = std::max(v_ptr->x, max_coords.x);
        min_coords.y = std::min(v_ptr->y, min_coords.y);
        max_coords.y = std::max(v_ptr->y, max_coords.y);
    }
    
    bmap.top_left_corner = min_coords;
    //Add one more to the cols/rows because, suppose there's an edge at y = 256.
    //The row would be 2. In reality, the row should be 3.
    bmap.n_cols =
        ceil((max_coords.x - min_coords.x) / GEOMETRY::BLOCKMAP_BLOCK_SIZE) + 1;
    bmap.n_rows =
        ceil((max_coords.y - min_coords.y) / GEOMETRY::BLOCKMAP_BLOCK_SIZE) + 1;
        
    bmap.edges.assign(
        bmap.n_cols, vector<vector<edge*> >(bmap.n_rows, vector<edge*>())
    );
    bmap.sectors.assign(
        bmap.n_cols, vector<unordered_set<sector*> >(
            bmap.n_rows, unordered_set<sector*>()
        )
    );
    
    
    //Now, add a list of edges to each block.
    generate_edges_blockmap(edges);
    
    
    /* If at this point, there's any block that's missing a sector,
     * that means we couldn't figure out the sectors due to the edges it has
     * alone. But the block still has a sector (or NULL). So we need another
     * way to figure it out.
     * We know the following things that can speed up the process:
     * * The blocks at the edges of the blockmap have the NULL sector as the
     *     only candidate.
     * * If a block's neighbor only has one sector, then this block has that
     *     same sector.
     * If we can't figure out the sector the easy way, then we have to use the
     * triangle method to get the sector. Using the center of the blockmap is
     * just as good a checking spot as any.
     */
    for(size_t bx = 0; bx < bmap.n_cols; ++bx) {
        for(size_t by = 0; by < bmap.n_rows; ++by) {
        
            if(!bmap.sectors[bx][by].empty()) continue;
            
            if(
                bx == 0 || by == 0 ||
                bx == bmap.n_cols - 1 || by == bmap.n_rows - 1
            ) {
                bmap.sectors[bx][by].insert(nullptr);
                continue;
            }
            
            if(bmap.sectors[bx - 1][by].size() == 1) {
                bmap.sectors[bx][by].insert(*bmap.sectors[bx - 1][by].begin());
                continue;
            }
            if(bmap.sectors[bx + 1][by].size() == 1) {
                bmap.sectors[bx][by].insert(*bmap.sectors[bx + 1][by].begin());
                continue;
            }
            if(bmap.sectors[bx][by - 1].size() == 1) {
                bmap.sectors[bx][by].insert(*bmap.sectors[bx][by - 1].begin());
                continue;
            }
            if(bmap.sectors[bx][by + 1].size() == 1) {
                bmap.sectors[bx][by].insert(*bmap.sectors[bx][by + 1].begin());
                continue;
            }
            
            point corner = bmap.get_top_left_corner(bx, by);
            corner += GEOMETRY::BLOCKMAP_BLOCK_SIZE * 0.5;
            bmap.sectors[bx][by].insert(
                get_sector(corner, NULL, false)
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Generates the blockmap for a set of edges.
 * edges:
 *   Edges to generate the blockmap around.
 */
void area_data::generate_edges_blockmap(vector<edge*> &edges) {
    size_t b_min_x, b_max_x, b_min_y, b_max_y;
    
    for(size_t e = 0; e < edges.size(); ++e) {
    
        //Get which blocks this edge belongs to, via bounding-box,
        //and only then thoroughly test which it is inside of.
        
        edge* e_ptr = edges[e];
        
        b_min_x =
            bmap.get_col(
                std::min(e_ptr->vertexes[0]->x, e_ptr->vertexes[1]->x)
            );
        b_max_x =
            bmap.get_col(
                std::max(e_ptr->vertexes[0]->x, e_ptr->vertexes[1]->x)
            );
        b_min_y =
            bmap.get_row(
                std::min(e_ptr->vertexes[0]->y, e_ptr->vertexes[1]->y)
            );
        b_max_y =
            bmap.get_row(
                std::max(e_ptr->vertexes[0]->y, e_ptr->vertexes[1]->y)
            );
            
        for(size_t bx = b_min_x; bx <= b_max_x; ++bx) {
            for(size_t by = b_min_y; by <= b_max_y; ++by) {
            
                //Get the block's coordinates.
                point corner = bmap.get_top_left_corner(bx, by);
                
                //Check if the edge is inside this blockmap.
                if(
                    line_seg_intersects_rectangle(
                        corner,
                        corner + GEOMETRY::BLOCKMAP_BLOCK_SIZE,
                        point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
                        point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y)
                    )
                ) {
                
                    //If it is, add it and the sectors to the list.
                    bool add_edge = true;
                    if(e_ptr->sectors[0] && e_ptr->sectors[1]) {
                        //If there's no change in height, why bother?
                        if(
                            (e_ptr->sectors[0]->z == e_ptr->sectors[1]->z) &&
                            e_ptr->sectors[0]->type != SECTOR_TYPE_BLOCKING &&
                            e_ptr->sectors[1]->type != SECTOR_TYPE_BLOCKING
                        ) {
                            add_edge = false;
                        }
                    }
                    
                    if(add_edge) bmap.edges[bx][by].push_back(e_ptr);
                    
                    if(e_ptr->sectors[0] || e_ptr->sectors[1]) {
                        bmap.sectors[bx][by].insert(e_ptr->sectors[0]);
                        bmap.sectors[bx][by].insert(e_ptr->sectors[1]);
                    }
                }
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns how many path links exist in the area.
 */
size_t area_data::get_nr_path_links() {
    size_t one_ways_found = 0;
    size_t normals_found = 0;
    for(size_t s = 0; s < path_stops.size(); ++s) {
        path_stop* s_ptr = path_stops[s];
        for(size_t l = 0; l < s_ptr->links.size(); ++l) {
            path_link* l_ptr = s_ptr->links[l];
            if(l_ptr->end_ptr->get_link(s_ptr)) {
                //The other stop links to this one. So it's a two-way.
                normals_found++;
            } else {
                one_ways_found++;
            }
        }
    }
    return (normals_found / 2.0f) + one_ways_found;
}


/* ----------------------------------------------------------------------------
 * Loads the thumbnail image from the disk and updates the
 * thumbnail class member.
 * thumbnail_path:
 *   Path to the bitmap.
 */
void area_data::load_thumbnail(const string &thumbnail_path) {
    if(thumbnail) {
        thumbnail = NULL;
    }
    
    if(al_filename_exists(thumbnail_path.c_str())) {
        thumbnail =
            std::shared_ptr<ALLEGRO_BITMAP>(
                al_load_bitmap(thumbnail_path.c_str()),
        [](ALLEGRO_BITMAP * b) {
            al_destroy_bitmap(b);
        }
            );
    }
}


/* ----------------------------------------------------------------------------
 * Adds a new edge to the list and returns its pointer.
 */
edge* area_data::new_edge() {
    edge* e_ptr = new edge();
    edges.push_back(e_ptr);
    return e_ptr;
}


/* ----------------------------------------------------------------------------
 * Adds a new sector to the list and returns its pointer.
 */
sector* area_data::new_sector() {
    sector* s_ptr = new sector();
    sectors.push_back(s_ptr);
    return s_ptr;
}


/* ----------------------------------------------------------------------------
 * Adds a new vertex to the list and returns its pointer.
 */
vertex* area_data::new_vertex() {
    vertex* v_ptr = new vertex();
    vertexes.push_back(v_ptr);
    return v_ptr;
}


/* ----------------------------------------------------------------------------
 * Removes an edge from the list, and updates all IDs referencing it.
 * e_nr:
 *   Index number of the edge to remove.
 */
void area_data::remove_edge(const size_t e_nr) {
    edges.erase(edges.begin() + e_nr);
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
            if(
                v_ptr->edge_nrs[e] != INVALID &&
                v_ptr->edge_nrs[e] > e_nr
            ) {
                v_ptr->edge_nrs[e]--;
            } else {
                //This should never happen.
                engine_assert(
                    v_ptr->edge_nrs[e] != e_nr,
                    i2s(v_ptr->edge_nrs[e]) + " " + i2s(e_nr)
                );
            }
        }
    }
    for(size_t s = 0; s < sectors.size(); ++s) {
        sector* s_ptr = sectors[s];
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            if(
                s_ptr->edge_nrs[e] != INVALID &&
                s_ptr->edge_nrs[e] > e_nr
            ) {
                s_ptr->edge_nrs[e]--;
            } else {
                //This should never happen.
                engine_assert(
                    s_ptr->edge_nrs[e] != e_nr,
                    i2s(s_ptr->edge_nrs[e]) + " " + i2s(e_nr)
                );
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Removes an edge from the list, and updates all IDs referencing it.
 * e_ptr:
 *   Pointer of the edge to remove.
 */
void area_data::remove_edge(const edge* e_ptr) {
    for(size_t e = 0; e < edges.size(); ++e) {
        if(edges[e] == e_ptr) {
            remove_edge(e);
            return;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Removes a sector from the list, and updates all IDs referencing it.
 * s_nr:
 *   Index number of the sector to remove.
 */
void area_data::remove_sector(const size_t s_nr) {
    sectors.erase(sectors.begin() + s_nr);
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        edge* e_ptr = game.cur_area_data.edges[e];
        for(size_t s = 0; s < 2; ++s) {
            if(
                e_ptr->sector_nrs[s] != INVALID &&
                e_ptr->sector_nrs[s] > s_nr
            ) {
                e_ptr->sector_nrs[s]--;
            } else {
                //This should never happen.
                engine_assert(
                    e_ptr->sector_nrs[s] != s_nr,
                    i2s(e_ptr->sector_nrs[s]) + " " + i2s(s_nr)
                );
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Removes a sector from the list, and updates all IDs referencing it.
 * s_ptr:
 *   Pointer of the sector to remove.
 */
void area_data::remove_sector(const sector* s_ptr) {
    for(size_t s = 0; s < sectors.size(); ++s) {
        if(sectors[s] == s_ptr) {
            remove_sector(s);
            return;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Removes a vertex from the list, and updates all IDs referencing it.
 * v_nr:
 *   Index number of the vertex to remove.
 */
void area_data::remove_vertex(const size_t v_nr) {
    vertexes.erase(vertexes.begin() + v_nr);
    for(size_t e = 0; e < edges.size(); ++e) {
        edge* e_ptr = edges[e];
        for(size_t v = 0; v < 2; ++v) {
            if(
                e_ptr->vertex_nrs[v] != INVALID &&
                e_ptr->vertex_nrs[v] > v_nr
            ) {
                e_ptr->vertex_nrs[v]--;
            } else {
                //This should never happen.
                engine_assert(
                    e_ptr->vertex_nrs[v] != v_nr,
                    i2s(e_ptr->vertex_nrs[v]) + " " + i2s(v_nr)
                );
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Removes a vertex from the list, and updates all IDs referencing it.
 * v_ptr:
 *   Pointer of the vertex to remove.
 */
void area_data::remove_vertex(const vertex* v_ptr) {
    for(size_t v = 0; v < vertexes.size(); ++v) {
        if(vertexes[v] == v_ptr) {
            remove_vertex(v);
            return;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns the folder name and area type of an area on disk, given its path.
 * requested_area_path:
 *   Relative path to the requested area.
 * final_area_folder_name:
 *   The area's folder name is returned here, if not NULL.
 * final_area_type:
 *   The area's type is returned here, if not NULL.
 */
void get_area_info_from_path(
    const string &requested_area_path,
    string* final_area_folder_name,
    AREA_TYPES* final_area_type
) {
    if(final_area_folder_name) *final_area_folder_name = requested_area_path;
    if(final_area_type) *final_area_type = AREA_TYPE_SIMPLE;
    
    vector<string> parts = split(requested_area_path, "/");
    
    if(parts.size() <= 1) return;
    
    if(final_area_folder_name) *final_area_folder_name = parts.back();
    if(final_area_type) {
        if(parts[parts.size() - 2] == "Mission") {
            *final_area_type = AREA_TYPE_MISSION;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns the folder path where certain area folders are stored,
 * based on the type of area and whether it's to load from the game data folder
 * or the user data folder.
 * type:
 *   Type of area.
 * from_game_data:
 *   If true, get the folder in the game data folder.
 */
string get_base_area_folder_path(
    const AREA_TYPES type, const bool from_game_data
) {
    string result =
        from_game_data ?
        GAME_DATA_FOLDER_PATH :
        USER_DATA_FOLDER_PATH;
    result += "/";
    
    switch(type) {
    case AREA_TYPE_SIMPLE: {
        return result + SIMPLE_AREA_FOLDER_NAME;
    }
    case AREA_TYPE_MISSION: {
        return result + MISSION_AREA_FOLDER_NAME;
    }
    }
    return result;
}
