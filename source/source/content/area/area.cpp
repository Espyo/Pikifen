/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area class and related functions.
 */

#include <algorithm>
#include <vector>

#include "area.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


using std::size_t;
using std::vector;


namespace AREA {

//Default day time speed, in game-minutes per real-minutes.
const float DEF_DAY_TIME_SPEED = 120;

//Default day time at the start of gameplay, in minutes.
const size_t DEF_DAY_TIME_START = 7 * 60;

//Default difficulty.
const unsigned char DEF_DIFFICULTY = 0;

}


/**
 * @brief Checks to see if all indexes match their pointers,
 * for the various edges, vertexes, etc.
 *
 * This is merely a debugging tool. Aborts execution if any of the pointers
 * don't match.
 */
void Area::check_stability() {
    for(size_t v = 0; v < vertexes.size(); v++) {
        Vertex* v_ptr = vertexes[v];
        engine_assert(
            v_ptr->edges.size() == v_ptr->edge_idxs.size(),
            i2s(v_ptr->edges.size()) + " " + i2s(v_ptr->edge_idxs.size())
        );
        for(size_t e = 0; e < v_ptr->edges.size(); e++) {
            engine_assert(v_ptr->edges[e] == edges[v_ptr->edge_idxs[e]], "");
        }
    }
    
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* e_ptr = edges[e];
        for(size_t v = 0; v < 2; v++) {
            engine_assert(
                e_ptr->vertexes[v] == vertexes[e_ptr->vertex_idxs[v]], ""
            );
        }
        
        for(size_t s = 0; s < 2; s++) {
            Sector* s_ptr = e_ptr->sectors[s];
            if(
                s_ptr == nullptr &&
                e_ptr->sector_idxs[s] == INVALID
            ) {
                continue;
            }
            engine_assert(s_ptr == sectors[e_ptr->sector_idxs[s]], "");
        }
    }
    
    for(size_t s = 0; s < sectors.size(); s++) {
        Sector* s_ptr = sectors[s];
        engine_assert(
            s_ptr->edges.size() == s_ptr->edge_idxs.size(),
            i2s(s_ptr->edges.size()) + " " + i2s(s_ptr->edge_idxs.size())
        );
        for(size_t e = 0; e < s_ptr->edges.size(); e++) {
            engine_assert(s_ptr->edges[e] == edges[s_ptr->edge_idxs[e]], "");
        }
    }
}


/**
 * @brief Clears the info of an area map.
 */
void Area::clear() {
    for(size_t v = 0; v < vertexes.size(); v++) {
        delete vertexes[v];
    }
    for(size_t e = 0; e < edges.size(); e++) {
        delete edges[e];
    }
    for(size_t s = 0; s < sectors.size(); s++) {
        delete sectors[s];
    }
    for(size_t m = 0; m < mob_generators.size(); m++) {
        delete mob_generators[m];
    }
    for(size_t s = 0; s < path_stops.size(); s++) {
        delete path_stops[s];
    }
    for(size_t s = 0; s < tree_shadows.size(); s++) {
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
        game.content.bitmaps.list.free(bg_bmp);
        bg_bmp = nullptr;
    }
    if(thumbnail) {
        thumbnail = nullptr;
    }
    
    reset_metadata();
    manifest = nullptr;
    name.clear();
    type = AREA_TYPE_SIMPLE;
    subtitle.clear();
    difficulty = AREA::DEF_DIFFICULTY;
    spray_amounts.clear();
    song_name.clear();
    weather_name.clear();
    day_time_start = AREA::DEF_DAY_TIME_START;
    day_time_speed = AREA::DEF_DAY_TIME_SPEED;
    bg_bmp_name.clear();
    bg_color = COLOR_BLACK;
    bg_dist = 2.0f;
    bg_bmp_zoom = 1.0f;
    mission = MissionData();
    
    problems.non_simples.clear();
    problems.lone_edges.clear();
}


/**
 * @brief Cleans up redundant data and such.
 *
 * @param out_deleted_sectors If not nullptr, whether any sectors got deleted
 * is returned here.
 */
void Area::cleanup(bool* out_deleted_sectors) {
    //Get rid of unused sectors.
    bool deleted_sectors = false;
    for(size_t s = 0; s < sectors.size(); ) {
        if(sectors[s]->edges.empty()) {
            remove_sector(s);
            deleted_sectors = true;
        } else {
            s++;
        }
    }
    if(out_deleted_sectors) *out_deleted_sectors = deleted_sectors;
    
    //And some other cleanup.
    if(song_name == NONE_OPTION) {
        song_name.clear();
    }
    if(weather_name == NONE_OPTION) {
        weather_name.clear();
    }
    engine_version = get_engine_version_string();
}


/**
 * @brief Clones this area data into another Area object.
 *
 * @param other The area data object to clone to.
 */
void Area::clone(Area &other) {
    other.clear();
    
    if(!other.bg_bmp_name.empty() && other.bg_bmp) {
        game.content.bitmaps.list.free(other.bg_bmp_name);
    }
    other.bg_bmp_name = bg_bmp_name;
    if(other.bg_bmp_name.empty()) {
        other.bg_bmp = nullptr;
    } else {
        other.bg_bmp = game.content.bitmaps.list.get(bg_bmp_name, nullptr, false);
    }
    other.bg_bmp_zoom = bg_bmp_zoom;
    other.bg_color = bg_color;
    other.bg_dist = bg_dist;
    other.bmap = bmap;
    
    other.vertexes.reserve(vertexes.size());
    for(size_t v = 0; v < vertexes.size(); v++) {
        other.vertexes.push_back(new Vertex());
    }
    other.edges.reserve(edges.size());
    for(size_t e = 0; e < edges.size(); e++) {
        other.edges.push_back(new Edge());
    }
    other.sectors.reserve(sectors.size());
    for(size_t s = 0; s < sectors.size(); s++) {
        other.sectors.push_back(new Sector());
    }
    other.mob_generators.reserve(mob_generators.size());
    for(size_t m = 0; m < mob_generators.size(); m++) {
        other.mob_generators.push_back(new MobGen());
    }
    other.path_stops.reserve(path_stops.size());
    for(size_t s = 0; s < path_stops.size(); s++) {
        other.path_stops.push_back(new PathStop());
    }
    other.tree_shadows.reserve(tree_shadows.size());
    for(size_t t = 0; t < tree_shadows.size(); t++) {
        other.tree_shadows.push_back(new TreeShadow());
    }
    
    for(size_t v = 0; v < vertexes.size(); v++) {
        Vertex* v_ptr = vertexes[v];
        Vertex* ov_ptr = other.vertexes[v];
        ov_ptr->x = v_ptr->x;
        ov_ptr->y = v_ptr->y;
        ov_ptr->edges.reserve(v_ptr->edges.size());
        ov_ptr->edge_idxs.reserve(v_ptr->edge_idxs.size());
        for(size_t e = 0; e < v_ptr->edges.size(); e++) {
            size_t nr = v_ptr->edge_idxs[e];
            ov_ptr->edges.push_back(other.edges[nr]);
            ov_ptr->edge_idxs.push_back(nr);
        }
    }
    
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* e_ptr = edges[e];
        Edge* oe_ptr = other.edges[e];
        oe_ptr->vertexes[0] = other.vertexes[e_ptr->vertex_idxs[0]];
        oe_ptr->vertexes[1] = other.vertexes[e_ptr->vertex_idxs[1]];
        oe_ptr->vertex_idxs[0] = e_ptr->vertex_idxs[0];
        oe_ptr->vertex_idxs[1] = e_ptr->vertex_idxs[1];
        if(e_ptr->sector_idxs[0] == INVALID) {
            oe_ptr->sectors[0] = nullptr;
        } else {
            oe_ptr->sectors[0] = other.sectors[e_ptr->sector_idxs[0]];
        }
        if(e_ptr->sector_idxs[1] == INVALID) {
            oe_ptr->sectors[1] = nullptr;
        } else {
            oe_ptr->sectors[1] = other.sectors[e_ptr->sector_idxs[1]];
        }
        oe_ptr->sector_idxs[0] = e_ptr->sector_idxs[0];
        oe_ptr->sector_idxs[1] = e_ptr->sector_idxs[1];
        e_ptr->clone(oe_ptr);
    }
    
    for(size_t s = 0; s < sectors.size(); s++) {
        Sector* s_ptr = sectors[s];
        Sector* os_ptr = other.sectors[s];
        s_ptr->clone(os_ptr);
        os_ptr->texture_info.bmp_name = s_ptr->texture_info.bmp_name;
        os_ptr->texture_info.bitmap =
            game.content.bitmaps.list.get(s_ptr->texture_info.bmp_name, nullptr, false);
        os_ptr->edges.reserve(s_ptr->edges.size());
        os_ptr->edge_idxs.reserve(s_ptr->edge_idxs.size());
        for(size_t e = 0; e < s_ptr->edges.size(); e++) {
            size_t nr = s_ptr->edge_idxs[e];
            os_ptr->edges.push_back(other.edges[nr]);
            os_ptr->edge_idxs.push_back(nr);
        }
        os_ptr->triangles.reserve(s_ptr->triangles.size());
        for(size_t t = 0; t < s_ptr->triangles.size(); t++) {
            Triangle* t_ptr = &s_ptr->triangles[t];
            os_ptr->triangles.push_back(
                Triangle(
                    other.vertexes[find_vertex_idx(t_ptr->points[0])],
                    other.vertexes[find_vertex_idx(t_ptr->points[1])],
                    other.vertexes[find_vertex_idx(t_ptr->points[2])]
                )
            );
        }
        os_ptr->bbox[0] = s_ptr->bbox[0];
        os_ptr->bbox[1] = s_ptr->bbox[1];
    }
    
    for(size_t m = 0; m < mob_generators.size(); m++) {
        MobGen* m_ptr = mob_generators[m];
        MobGen* om_ptr = other.mob_generators[m];
        m_ptr->clone(om_ptr);
    }
    for(size_t m = 0; m < mob_generators.size(); m++) {
        MobGen* om_ptr = other.mob_generators[m];
        for(size_t l = 0; l < om_ptr->link_idxs.size(); l++) {
            om_ptr->links.push_back(
                other.mob_generators[om_ptr->link_idxs[l]]
            );
        }
    }
    
    for(size_t s = 0; s < path_stops.size(); s++) {
        PathStop* s_ptr = path_stops[s];
        PathStop* os_ptr = other.path_stops[s];
        os_ptr->pos = s_ptr->pos;
        s_ptr->clone(os_ptr);
        os_ptr->links.reserve(s_ptr->links.size());
        for(size_t l = 0; l < s_ptr->links.size(); l++) {
            PathLink* new_link =
                new PathLink(
                os_ptr,
                other.path_stops[s_ptr->links[l]->end_idx],
                s_ptr->links[l]->end_idx
            );
            s_ptr->links[l]->clone(new_link);
            new_link->distance = s_ptr->links[l]->distance;
            os_ptr->links.push_back(new_link);
        }
    }
    
    for(size_t t = 0; t < tree_shadows.size(); t++) {
        TreeShadow* t_ptr = tree_shadows[t];
        TreeShadow* ot_ptr = other.tree_shadows[t];
        ot_ptr->alpha = t_ptr->alpha;
        ot_ptr->angle = t_ptr->angle;
        ot_ptr->center = t_ptr->center;
        ot_ptr->bmp_name = t_ptr->bmp_name;
        ot_ptr->size = t_ptr->size;
        ot_ptr->sway = t_ptr->sway;
        ot_ptr->bitmap = game.content.bitmaps.list.get(t_ptr->bmp_name, nullptr, false);
    }
    
    other.manifest = manifest;
    other.type = type;
    other.name = name;
    other.subtitle = subtitle;
    other.description = description;
    other.tags = tags;
    other.difficulty = difficulty;
    other.maker = maker;
    other.version = version;
    other.maker_notes = maker_notes;
    other.spray_amounts = spray_amounts;
    other.song_name = song_name;
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
    other.mission.maker_record = mission.maker_record;
    other.mission.maker_record_date = mission.maker_record_date;
    
    other.problems.non_simples.clear();
    other.problems.lone_edges.clear();
    other.problems.lone_edges.reserve(problems.lone_edges.size());
    for(const auto &s : problems.non_simples) {
        size_t nr = find_sector_idx(s.first);
        other.problems.non_simples[other.sectors[nr]] = s.second;
    }
    for(const Edge* e : problems.lone_edges) {
        size_t nr = find_edge_idx(e);
        other.problems.lone_edges.insert(other.edges[nr]);
    }
}


/**
 * @brief Connects an edge to a sector.
 *
 * This adds the sector and its index to the edge's
 * lists, and adds the edge and its index to the sector's.
 *
 * @param e_ptr Edge to connect.
 * @param s_ptr Sector to connect.
 * @param side Which of the sides of the edge the sector goes to.
 */
void Area::connect_edge_to_sector(
    Edge* e_ptr, Sector* s_ptr, size_t side
) {
    if(e_ptr->sectors[side]) {
        e_ptr->sectors[side]->remove_edge(e_ptr);
    }
    e_ptr->sectors[side] = s_ptr;
    e_ptr->sector_idxs[side] = find_sector_idx(s_ptr);
    if(s_ptr) {
        s_ptr->add_edge(e_ptr, find_edge_idx(e_ptr));
    }
}


/**
 * @brief Connects an edge to a vertex.
 *
 * This adds the vertex and its index to the edge's
 * lists, and adds the edge and its index to the vertex's.
 *
 * @param e_ptr Edge to connect.
 * @param v_ptr Vertex to connect.
 * @param endpoint Which of the edge endpoints the vertex goes to.
 */
void Area::connect_edge_to_vertex(
    Edge* e_ptr, Vertex* v_ptr, size_t endpoint
) {
    if(e_ptr->vertexes[endpoint]) {
        e_ptr->vertexes[endpoint]->remove_edge(e_ptr);
    }
    e_ptr->vertexes[endpoint] = v_ptr;
    e_ptr->vertex_idxs[endpoint] = find_vertex_idx(v_ptr);
    v_ptr->add_edge(e_ptr, find_edge_idx(e_ptr));
}



/**
 * @brief Connects the edges of a sector that link to it into the
 * edge_idxs vector.
 *
 * @param s_ptr The sector.
 */
void Area::connect_sector_edges(Sector* s_ptr) {
    s_ptr->edge_idxs.clear();
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* e_ptr = edges[e];
        if(e_ptr->sectors[0] == s_ptr || e_ptr->sectors[1] == s_ptr) {
            s_ptr->edge_idxs.push_back(e);
        }
    }
    fix_sector_pointers(s_ptr);
}


/**
 * @brief Connects the edges that link to it into the edge_idxs vector.
 *
 * @param v_ptr The vertex.
 */
void Area::connect_vertex_edges(Vertex* v_ptr) {
    v_ptr->edge_idxs.clear();
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* e_ptr = edges[e];
        if(e_ptr->vertexes[0] == v_ptr || e_ptr->vertexes[1] == v_ptr) {
            v_ptr->edge_idxs.push_back(e);
        }
    }
    fix_vertex_pointers(v_ptr);
}


/**
 * @brief Scans the list of edges and retrieves the index of
 * the specified edge.
 *
 * @param e_ptr Edge to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::find_edge_idx(const Edge* e_ptr) const {
    for(size_t e = 0; e < edges.size(); e++) {
        if(edges[e] == e_ptr) return e;
    }
    return INVALID;
}


/**
 * @brief Scans the list of mob generators and retrieves the index of
 * the specified mob generator.
 *
 * @param m_ptr Mob to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::find_mob_gen_idx(const MobGen* m_ptr) const {
    for(size_t m = 0; m < mob_generators.size(); m++) {
        if(mob_generators[m] == m_ptr) return m;
    }
    return INVALID;
}


/**
 * @brief Scans the list of sectors and retrieves the index of
 * the specified sector.
 *
 * @param s_ptr Sector to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::find_sector_idx(const Sector* s_ptr) const {
    for(size_t s = 0; s < sectors.size(); s++) {
        if(sectors[s] == s_ptr) return s;
    }
    return INVALID;
}


/**
 * @brief Scans the list of vertexes and retrieves the index of
 * the specified vertex.
 *
 * @param v_ptr Vertex to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::find_vertex_idx(const Vertex* v_ptr) const {
    for(size_t v = 0; v < vertexes.size(); v++) {
        if(vertexes[v] == v_ptr) return v;
    }
    return INVALID;
}


/**
 * @brief Fixes the sector and vertex indexes in an edge,
 * making them match the correct sectors and vertexes,
 * based on the existing sector and vertex pointers.
 *
 * @param e_ptr Edge to fix the indexes of.
 */
void Area::fix_edge_idxs(Edge* e_ptr) {
    for(size_t s = 0; s < 2; s++) {
        if(!e_ptr->sectors[s]) {
            e_ptr->sector_idxs[s] = INVALID;
        } else {
            e_ptr->sector_idxs[s] = find_sector_idx(e_ptr->sectors[s]);
        }
    }
    
    for(size_t v = 0; v < 2; v++) {
        if(!e_ptr->vertexes[v]) {
            e_ptr->vertex_idxs[v] = INVALID;
        } else {
            e_ptr->vertex_idxs[v] = find_vertex_idx(e_ptr->vertexes[v]);
        }
    }
}


/**
 * @brief Fixes the sector and vertex pointers of an edge,
 * making them point to the correct sectors and vertexes,
 * based on the existing sector and vertex indexes.
 *
 * @param e_ptr Edge to fix the pointers of.
 */
void Area::fix_edge_pointers(Edge* e_ptr) {
    e_ptr->sectors[0] = nullptr;
    e_ptr->sectors[1] = nullptr;
    for(size_t s = 0; s < 2; s++) {
        size_t s_idx = e_ptr->sector_idxs[s];
        e_ptr->sectors[s] = (s_idx == INVALID ? nullptr : sectors[s_idx]);
    }
    
    e_ptr->vertexes[0] = nullptr;
    e_ptr->vertexes[1] = nullptr;
    for(size_t v = 0; v < 2; v++) {
        size_t v_idx = e_ptr->vertex_idxs[v];
        e_ptr->vertexes[v] = (v_idx == INVALID ? nullptr : vertexes[v_idx]);
    }
}


/**
 * @brief Fixes the path stop indexes in a path stop's links,
 * making them match the correct path stops,
 * based on the existing path stop pointers.
 *
 * @param s_ptr Path stop to fix the indexes of.
 */
void Area::fix_path_stop_idxs(PathStop* s_ptr) {
    for(size_t l = 0; l < s_ptr->links.size(); l++) {
        PathLink* l_ptr = s_ptr->links[l];
        l_ptr->end_idx = INVALID;
        
        if(!l_ptr->end_ptr) continue;
        
        for(size_t s = 0; s < path_stops.size(); s++) {
            if(l_ptr->end_ptr == path_stops[s]) {
                l_ptr->end_idx = s;
                break;
            }
        }
    }
}


/**
 * @brief Fixes the path stop pointers in a path stop's links,
 * making them point to the correct path stops,
 * based on the existing path stop indexes.
 *
 * @param s_ptr Path stop to fix the pointers of.
 */
void Area::fix_path_stop_pointers(PathStop* s_ptr) {
    for(size_t l = 0; l < s_ptr->links.size(); l++) {
        PathLink* l_ptr = s_ptr->links[l];
        l_ptr->end_ptr = nullptr;
        
        if(l_ptr->end_idx == INVALID) continue;
        if(l_ptr->end_idx >= path_stops.size()) continue;
        
        l_ptr->end_ptr = path_stops[l_ptr->end_idx];
    }
}


/**
 * @brief Fixes the edge indexes in a sector,
 * making them match the correct edges,
 * based on the existing edge pointers.
 *
 * @param s_ptr Sector to fix the indexes of.
 */
void Area::fix_sector_idxs(Sector* s_ptr) {
    s_ptr->edge_idxs.clear();
    for(size_t e = 0; e < s_ptr->edges.size(); e++) {
        s_ptr->edge_idxs.push_back(find_edge_idx(s_ptr->edges[e]));
    }
}


/**
 * @brief Fixes the edge pointers in a sector,
 * making them point to the correct edges,
 * based on the existing edge indexes.
 *
 * @param s_ptr Sector to fix the pointers of.
 */
void Area::fix_sector_pointers(Sector* s_ptr) {
    s_ptr->edges.clear();
    for(size_t e = 0; e < s_ptr->edge_idxs.size(); e++) {
        size_t e_idx = s_ptr->edge_idxs[e];
        s_ptr->edges.push_back(e_idx == INVALID ? nullptr : edges[e_idx]);
    }
}


/**
 * @brief Fixes the edge indexes in a vertex,
 * making them match the correct edges,
 * based on the existing edge pointers.
 *
 * @param v_ptr Vertex to fix the indexes of.
 */
void Area::fix_vertex_idxs(Vertex* v_ptr) {
    v_ptr->edge_idxs.clear();
    for(size_t e = 0; e < v_ptr->edges.size(); e++) {
        v_ptr->edge_idxs.push_back(find_edge_idx(v_ptr->edges[e]));
    }
}


/**
 * @brief Fixes the edge pointers in a vertex,
 * making them point to the correct edges,
 * based on the existing edge indexes.
 *
 * @param v_ptr Vertex to fix the pointers of.
 */
void Area::fix_vertex_pointers(Vertex* v_ptr) {
    v_ptr->edges.clear();
    for(size_t e = 0; e < v_ptr->edge_idxs.size(); e++) {
        size_t e_idx = v_ptr->edge_idxs[e];
        v_ptr->edges.push_back(e_idx == INVALID ? nullptr : edges[e_idx]);
    }
}


/**
 * @brief Generates the blockmap for the area, given the current info.
 */
void Area::generate_blockmap() {
    bmap.clear();
    
    if(vertexes.empty()) return;
    
    //First, get the starting point and size of the blockmap.
    Point min_coords = v2p(vertexes[0]);
    Point max_coords = min_coords;
    
    for(size_t v = 0; v < vertexes.size(); v++) {
        Vertex* v_ptr = vertexes[v];
        update_min_max_coords(min_coords, max_coords, v2p(v_ptr));
    }
    
    bmap.top_left_corner = min_coords;
    //Add one more to the cols/rows because, suppose there's an edge at y = 256.
    //The row would be 2. In reality, the row should be 3.
    bmap.n_cols =
        ceil((max_coords.x - min_coords.x) / GEOMETRY::BLOCKMAP_BLOCK_SIZE) + 1;
    bmap.n_rows =
        ceil((max_coords.y - min_coords.y) / GEOMETRY::BLOCKMAP_BLOCK_SIZE) + 1;
        
    bmap.edges.assign(
        bmap.n_cols, vector<vector<Edge*> >(bmap.n_rows, vector<Edge*>())
    );
    bmap.sectors.assign(
        bmap.n_cols, vector<unordered_set<Sector*> >(
            bmap.n_rows, unordered_set<Sector*>()
        )
    );
    
    
    //Now, add a list of edges to each block.
    generate_edges_blockmap(edges);
    
    
    /* If at this point, there's any block that's missing a sector,
     * that means we couldn't figure out the sectors due to the edges it has
     * alone. But the block still has a sector (or nullptr). So we need another
     * way to figure it out.
     * We know the following things that can speed up the process:
     * * The blocks at the edges of the blockmap have the nullptr sector as the
     *     only candidate.
     * * If a block's neighbor only has one sector, then this block has that
     *     same sector.
     * If we can't figure out the sector the easy way, then we have to use the
     * triangle method to get the sector. Using the center of the blockmap is
     * just as good a checking spot as any.
     */
    for(size_t bx = 0; bx < bmap.n_cols; bx++) {
        for(size_t by = 0; by < bmap.n_rows; by++) {
        
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
            
            Point corner = bmap.get_top_left_corner(bx, by);
            corner += GEOMETRY::BLOCKMAP_BLOCK_SIZE * 0.5;
            bmap.sectors[bx][by].insert(
                get_sector(corner, nullptr, false)
            );
        }
    }
}


/**
 * @brief Generates the blockmap for a set of edges.
 *
 * @param edge_list Edges to generate the blockmap around.
 */
void Area::generate_edges_blockmap(const vector<Edge*> &edge_list) {
    for(size_t e = 0; e < edge_list.size(); e++) {
    
        //Get which blocks this edge belongs to, via bounding-box,
        //and only then thoroughly test which it is inside of.
        
        Edge* e_ptr = edge_list[e];
        Point min_coords = v2p(e_ptr->vertexes[0]);
        Point max_coords = min_coords;
        update_min_max_coords(min_coords, max_coords, v2p(e_ptr->vertexes[1]));
        
        size_t b_min_x = bmap.get_col(min_coords.x);
        size_t b_max_x = bmap.get_col(max_coords.x);
        size_t b_min_y = bmap.get_row(min_coords.y);
        size_t b_max_y = bmap.get_row(max_coords.y);
        
        for(size_t bx = b_min_x; bx <= b_max_x; bx++) {
            for(size_t by = b_min_y; by <= b_max_y; by++) {
            
                //Get the block's coordinates.
                Point corner = bmap.get_top_left_corner(bx, by);
                
                //Check if the edge is inside this blockmap.
                if(
                    line_seg_intersects_rectangle(
                        corner,
                        corner + GEOMETRY::BLOCKMAP_BLOCK_SIZE,
                        v2p(e_ptr->vertexes[0]), v2p(e_ptr->vertexes[1])
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


/**
 * @brief Returns how many path links exist in the area.
 *
 * @return The number of path links.
 */
size_t Area::get_nr_path_links() {
    size_t one_ways_found = 0;
    size_t normals_found = 0;
    for(size_t s = 0; s < path_stops.size(); s++) {
        PathStop* s_ptr = path_stops[s];
        for(size_t l = 0; l < s_ptr->links.size(); l++) {
            PathLink* l_ptr = s_ptr->links[l];
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


/**
 * @brief Loads the area's main data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void Area::load_main_data_from_data_node(
    DataNode* node, CONTENT_LOAD_LEVEL level
) {
    //Content metadata.
    load_metadata_from_data_node(node);
    
    //Area configuration data.
    ReaderSetter rs(node);
    DataNode* weather_node = nullptr;
    DataNode* song_node = nullptr;
    
    rs.set("subtitle", subtitle);
    rs.set("difficulty", difficulty);
    rs.set("spray_amounts", spray_amounts);
    rs.set("song", song_name, &song_node);
    rs.set("weather", weather_name, &weather_node);
    rs.set("day_time_start", day_time_start);
    rs.set("day_time_speed", day_time_speed);
    rs.set("bg_bmp", bg_bmp_name);
    rs.set("bg_color", bg_color);
    rs.set("bg_dist", bg_dist);
    rs.set("bg_zoom", bg_bmp_zoom);
    
    //Weather.
    if(level > CONTENT_LOAD_LEVEL_BASIC) {
        if(weather_name.empty()) {
            weather_condition = Weather();
            
        } else if(
            game.content.weather_conditions.list.find(weather_name) ==
            game.content.weather_conditions.list.end()
        ) {
            game.errors.report(
                "Unknown weather condition \"" + weather_name + "\"!",
                weather_node
            );
            weather_condition = Weather();
            
        } else {
            weather_condition =
                game.content.weather_conditions.list[weather_name];
                
        }
        
        //Song.
        if(
            !song_name.empty() &&
            game.content.songs.list.find(song_name) ==
            game.content.songs.list.end()
        ) {
            game.errors.report(
                "Unknown song \"" + song_name + "\"!",
                song_node
            );
        }
    }
    
    if(level >= CONTENT_LOAD_LEVEL_FULL && !bg_bmp_name.empty()) {
        bg_bmp = game.content.bitmaps.list.get(bg_bmp_name, node);
    }
}


/**
 * @brief Loads the area's mission data from a data node.
 *
 * @param node Data node to load from.
 */
void Area::load_mission_data_from_data_node(DataNode* node) {
    mission.fail_hud_primary_cond = INVALID;
    mission.fail_hud_secondary_cond = INVALID;
    
    ReaderSetter rs(node);
    string goal_str;
    string required_mobs_str;
    int mission_grading_mode_int = MISSION_GRADING_MODE_GOAL;
    
    rs.set("mission_goal", goal_str);
    rs.set("mission_goal_amount", mission.goal_amount);
    rs.set("mission_goal_all_mobs", mission.goal_all_mobs);
    rs.set("mission_required_mobs", required_mobs_str);
    rs.set("mission_goal_exit_center", mission.goal_exit_center);
    rs.set("mission_goal_exit_size", mission.goal_exit_size);
    rs.set("mission_fail_conditions", mission.fail_conditions);
    rs.set("mission_fail_too_few_pik_amount", mission.fail_too_few_pik_amount);
    rs.set("mission_fail_too_many_pik_amount", mission.fail_too_many_pik_amount);
    rs.set("mission_fail_pik_killed", mission.fail_pik_killed);
    rs.set("mission_fail_leaders_kod", mission.fail_leaders_kod);
    rs.set("mission_fail_enemies_killed", mission.fail_enemies_killed);
    rs.set("mission_fail_time_limit", mission.fail_time_limit);
    rs.set("mission_fail_hud_primary_cond", mission.fail_hud_primary_cond);
    rs.set("mission_fail_hud_secondary_cond", mission.fail_hud_secondary_cond);
    rs.set("mission_grading_mode", mission_grading_mode_int);
    rs.set("mission_points_per_pikmin_born", mission.points_per_pikmin_born);
    rs.set("mission_points_per_pikmin_death", mission.points_per_pikmin_death);
    rs.set("mission_points_per_sec_left", mission.points_per_sec_left);
    rs.set("mission_points_per_sec_passed", mission.points_per_sec_passed);
    rs.set("mission_points_per_treasure_point", mission.points_per_treasure_point);
    rs.set("mission_points_per_enemy_point", mission.points_per_enemy_point);
    rs.set("mission_point_loss_data", mission.point_loss_data);
    rs.set("mission_point_hud_data", mission.point_hud_data);
    rs.set("mission_starting_points", mission.starting_points);
    rs.set("mission_bronze_req", mission.bronze_req);
    rs.set("mission_silver_req", mission.silver_req);
    rs.set("mission_gold_req", mission.gold_req);
    rs.set("mission_platinum_req", mission.platinum_req);
    rs.set("mission_maker_record", mission.maker_record);
    rs.set("mission_maker_record_date", mission.maker_record_date);
    
    mission.goal = MISSION_GOAL_END_MANUALLY;
    for(size_t g = 0; g < game.mission_goals.size(); g++) {
        if(game.mission_goals[g]->get_name() == goal_str) {
            mission.goal = (MISSION_GOAL) g;
            break;
        }
    }
    vector<string> mission_required_mobs_strs =
        semicolon_list_to_vector(required_mobs_str);
    mission.goal_mob_idxs.reserve(
        mission_required_mobs_strs.size()
    );
    for(size_t m = 0; m < mission_required_mobs_strs.size(); m++) {
        mission.goal_mob_idxs.insert(
            s2i(mission_required_mobs_strs[m])
        );
    }
    mission.grading_mode = (MISSION_GRADING_MODE) mission_grading_mode_int;
    
    //Automatically turn the pause menu fail condition on/off for convenience.
    if(mission.goal == MISSION_GOAL_END_MANUALLY) {
        disable_flag(
            mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
    } else {
        enable_flag(
            mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
    }
    
    //Automatically turn off the seconds left score criterion for convenience.
    if(
        !has_flag(
            mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        mission.points_per_sec_left = 0;
        disable_flag(
            mission.point_hud_data,
            get_idx_bitmask(MISSION_SCORE_CRITERIA_SEC_LEFT)
        );
        disable_flag(
            mission.point_loss_data,
            get_idx_bitmask(MISSION_SCORE_CRITERIA_SEC_LEFT)
        );
    }
}


/**
 * @brief Loads the area's geometry from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void Area::load_geometry_from_data_node(
    DataNode* node, CONTENT_LOAD_LEVEL level
) {
    //Vertexes.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Vertexes");
    }
    
    size_t n_vertexes =
        node->getChildByName(
            "vertexes"
        )->getNrOfChildrenByName("v");
    for(size_t v = 0; v < n_vertexes; v++) {
        DataNode* vertex_data =
            node->getChildByName(
                "vertexes"
            )->getChildByName("v", v);
        vector<string> words = split(vertex_data->value);
        if(words.size() == 2) {
            vertexes.push_back(
                new Vertex(s2f(words[0]), s2f(words[1]))
            );
        }
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Edges.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Edges");
    }
    
    size_t n_edges =
        node->getChildByName(
            "edges"
        )->getNrOfChildrenByName("e");
    for(size_t e = 0; e < n_edges; e++) {
        DataNode* edge_data =
            node->getChildByName(
                "edges"
            )->getChildByName("e", e);
        Edge* new_edge = new Edge();
        
        vector<string> s_idxs = split(edge_data->getChildByName("s")->value);
        if(s_idxs.size() < 2) s_idxs.insert(s_idxs.end(), 2, "-1");
        for(size_t s = 0; s < 2; s++) {
            if(s_idxs[s] == "-1") new_edge->sector_idxs[s] = INVALID;
            else new_edge->sector_idxs[s] = s2i(s_idxs[s]);
        }
        
        vector<string> v_idxs = split(edge_data->getChildByName("v")->value);
        if(v_idxs.size() < 2) v_idxs.insert(v_idxs.end(), 2, "0");
        
        new_edge->vertex_idxs[0] = s2i(v_idxs[0]);
        new_edge->vertex_idxs[1] = s2i(v_idxs[1]);
        
        DataNode* shadow_length =
            edge_data->getChildByName("shadow_length");
        if(!shadow_length->value.empty()) {
            new_edge->wall_shadow_length =
                s2f(shadow_length->value);
        }
        
        DataNode* shadow_color =
            edge_data->getChildByName("shadow_color");
        if(!shadow_color->value.empty()) {
            new_edge->wall_shadow_color = s2c(shadow_color->value);
        }
        
        DataNode* smoothing_length =
            edge_data->getChildByName("smoothing_length");
        if(!smoothing_length->value.empty()) {
            new_edge->ledge_smoothing_length =
                s2f(smoothing_length->value);
        }
        
        DataNode* smoothing_color =
            edge_data->getChildByName("smoothing_color");
        if(!smoothing_color->value.empty()) {
            new_edge->ledge_smoothing_color =
                s2c(smoothing_color->value);
        }
        
        edges.push_back(new_edge);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Sectors.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Sectors");
    }
    
    size_t n_sectors =
        node->getChildByName(
            "sectors"
        )->getNrOfChildrenByName("s");
    for(size_t s = 0; s < n_sectors; s++) {
        DataNode* sector_data =
            node->getChildByName(
                "sectors"
            )->getChildByName("s", s);
        Sector* new_sector = new Sector();
        
        size_t new_type =
            game.sector_types.get_idx(
                sector_data->getChildByName("type")->value
            );
        if(new_type == INVALID) {
            new_type = SECTOR_TYPE_NORMAL;
        }
        new_sector->type = (SECTOR_TYPE) new_type;
        new_sector->is_bottomless_pit =
            s2b(
                sector_data->getChildByName(
                    "is_bottomless_pit"
                )->getValueOrDefault("false")
            );
        new_sector->brightness =
            s2f(
                sector_data->getChildByName(
                    "brightness"
                )->getValueOrDefault(i2s(GEOMETRY::DEF_SECTOR_BRIGHTNESS))
            );
        new_sector->tag = sector_data->getChildByName("tag")->value;
        new_sector->z = s2f(sector_data->getChildByName("z")->value);
        new_sector->fade = s2b(sector_data->getChildByName("fade")->value);
        
        new_sector->texture_info.bmp_name =
            sector_data->getChildByName("texture")->value;
        new_sector->texture_info.rot =
            s2f(sector_data->getChildByName("texture_rotate")->value);
            
        vector<string> scales =
            split(sector_data->getChildByName("texture_scale")->value);
        if(scales.size() >= 2) {
            new_sector->texture_info.scale.x = s2f(scales[0]);
            new_sector->texture_info.scale.y = s2f(scales[1]);
        }
        vector<string> translations =
            split(sector_data->getChildByName("texture_trans")->value);
        if(translations.size() >= 2) {
            new_sector->texture_info.translation.x = s2f(translations[0]);
            new_sector->texture_info.translation.y = s2f(translations[1]);
        }
        new_sector->texture_info.tint =
            s2c(
                sector_data->getChildByName("texture_tint")->
                getValueOrDefault("255 255 255")
            );
            
        if(!new_sector->fade && !new_sector->is_bottomless_pit) {
            new_sector->texture_info.bitmap =
                game.content.bitmaps.list.get(new_sector->texture_info.bmp_name, nullptr);
        }
        
        DataNode* hazards_node = sector_data->getChildByName("hazards");
        vector<string> hazards_strs =
            semicolon_list_to_vector(hazards_node->value);
        for(size_t h = 0; h < hazards_strs.size(); h++) {
            string hazard_name = hazards_strs[h];
            if(game.content.hazards.list.find(hazard_name) == game.content.hazards.list.end()) {
                game.errors.report(
                    "Unknown hazard \"" + hazard_name +
                    "\"!", hazards_node
                );
            } else {
                new_sector->hazards.push_back(&(game.content.hazards.list[hazard_name]));
            }
        }
        new_sector->hazards_str = hazards_node->value;
        new_sector->hazard_floor =
            s2b(
                sector_data->getChildByName(
                    "hazards_floor"
                )->getValueOrDefault("true")
            );
            
        sectors.push_back(new_sector);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Mobs.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Object generators");
    }
    
    vector<std::pair<size_t, size_t> > mob_links_buffer;
    size_t n_mobs =
        node->getChildByName("mobs")->getNrOfChildren();
        
    for(size_t m = 0; m < n_mobs; m++) {
    
        DataNode* mob_node =
            node->getChildByName("mobs")->getChild(m);
            
        MobGen* mob_ptr = new MobGen();
        
        mob_ptr->pos = s2p(mob_node->getChildByName("p")->value);
        mob_ptr->angle =
            s2f(
                mob_node->getChildByName("angle")->getValueOrDefault("0")
            );
        mob_ptr->vars = mob_node->getChildByName("vars")->value;
        
        string category_name = mob_node->name;
        string type_name;
        MobCategory* category =
            game.mob_categories.get_from_internal_name(category_name);
        if(category) {
            type_name = mob_node->getChildByName("type")->value;
            mob_ptr->type = category->get_type(type_name);
        } else {
            category = game.mob_categories.get(MOB_CATEGORY_NONE);
            mob_ptr->type = nullptr;
        }
        
        vector<string> link_strs =
            split(mob_node->getChildByName("links")->value);
        for(size_t l = 0; l < link_strs.size(); l++) {
            mob_links_buffer.push_back(std::make_pair(m, s2i(link_strs[l])));
        }
        
        DataNode* stored_inside_node =
            mob_node->getChildByName("stored_inside");
        if(!stored_inside_node->value.empty()) {
            mob_ptr->stored_inside = s2i(stored_inside_node->value);
        }
        
        bool valid =
            category && category->id != MOB_CATEGORY_NONE &&
            mob_ptr->type;
            
        if(!valid) {
            //Error.
            mob_ptr->type = nullptr;
            if(level >= CONTENT_LOAD_LEVEL_FULL) {
                game.errors.report(
                    "Unknown mob type \"" + type_name + "\" of category \"" +
                    mob_node->name + "\"!",
                    mob_node
                );
            }
        }
        
        mob_generators.push_back(mob_ptr);
    }
    
    for(size_t l = 0; l < mob_links_buffer.size(); l++) {
        size_t f = mob_links_buffer[l].first;
        size_t s = mob_links_buffer[l].second;
        mob_generators[f]->links.push_back(
            mob_generators[s]
        );
        mob_generators[f]->link_idxs.push_back(s);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Paths.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Paths");
    }
    
    size_t n_stops =
        node->getChildByName("path_stops")->getNrOfChildren();
    for(size_t s = 0; s < n_stops; s++) {
    
        DataNode* path_stop_node =
            node->getChildByName("path_stops")->getChild(s);
            
        PathStop* s_ptr = new PathStop();
        
        s_ptr->pos = s2p(path_stop_node->getChildByName("pos")->value);
        s_ptr->radius = s2f(path_stop_node->getChildByName("radius")->value);
        s_ptr->flags = s2i(path_stop_node->getChildByName("flags")->value);
        s_ptr->label = path_stop_node->getChildByName("label")->value;
        DataNode* links_node = path_stop_node->getChildByName("links");
        size_t n_links = links_node->getNrOfChildren();
        
        for(size_t l = 0; l < n_links; l++) {
        
            string link_data = links_node->getChild(l)->value;
            vector<string> link_data_parts = split(link_data);
            
            PathLink* l_ptr =
                new PathLink(s_ptr, nullptr, s2i(link_data_parts[0]));
            if(link_data_parts.size() >= 2) {
                l_ptr->type = (PATH_LINK_TYPE) s2i(link_data_parts[1]);
            }
            
            s_ptr->links.push_back(l_ptr);
            
        }
        
        s_ptr->radius = std::max(s_ptr->radius, PATHS::MIN_STOP_RADIUS);
        
        path_stops.push_back(s_ptr);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Tree shadows.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Tree shadows");
    }
    
    size_t n_shadows =
        node->getChildByName("tree_shadows")->getNrOfChildren();
    for(size_t s = 0; s < n_shadows; s++) {
    
        DataNode* shadow_node =
            node->getChildByName("tree_shadows")->getChild(s);
            
        TreeShadow* s_ptr = new TreeShadow();
        
        vector<string> words =
            split(shadow_node->getChildByName("pos")->value);
        s_ptr->center.x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->center.y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        words = split(shadow_node->getChildByName("size")->value);
        s_ptr->size.x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->size.y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        s_ptr->angle =
            s2f(
                shadow_node->getChildByName(
                    "angle"
                )->getValueOrDefault("0")
            );
        s_ptr->alpha =
            s2i(
                shadow_node->getChildByName(
                    "alpha"
                )->getValueOrDefault("255")
            );
        s_ptr->bmp_name = shadow_node->getChildByName("file")->value;
        s_ptr->bitmap = game.content.bitmaps.list.get(s_ptr->bmp_name, nullptr);
        
        words = split(shadow_node->getChildByName("sway")->value);
        s_ptr->sway.x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->sway.y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        if(s_ptr->bitmap == game.bmp_error && level >= CONTENT_LOAD_LEVEL_FULL) {
            game.errors.report(
                "Unknown tree shadow texture \"" + s_ptr->bmp_name + "\"!",
                shadow_node
            );
        }
        
        tree_shadows.push_back(s_ptr);
        
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Set up stuff.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Geometry calculations");
    }
    
    for(size_t e = 0; e < edges.size(); e++) {
        fix_edge_pointers(
            edges[e]
        );
    }
    for(size_t s = 0; s < sectors.size(); s++) {
        connect_sector_edges(
            sectors[s]
        );
    }
    for(size_t v = 0; v < vertexes.size(); v++) {
        connect_vertex_edges(
            vertexes[v]
        );
    }
    for(size_t s = 0; s < path_stops.size(); s++) {
        fix_path_stop_pointers(
            path_stops[s]
        );
    }
    for(size_t s = 0; s < path_stops.size(); s++) {
        path_stops[s]->calculate_dists();
    }
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        //Fade sectors that also fade brightness should be
        //at midway between the two neighbors.
        for(size_t s = 0; s < sectors.size(); s++) {
            Sector* s_ptr = sectors[s];
            if(s_ptr->fade) {
                Sector* n1 = nullptr;
                Sector* n2 = nullptr;
                s_ptr->get_texture_merge_sectors(&n1, &n2);
                if(n1 && n2) {
                    s_ptr->brightness = (n1->brightness + n2->brightness) / 2;
                }
            }
        }
    }
    
    
    //Triangulate everything and save bounding boxes.
    set<Edge*> lone_edges;
    for(size_t s = 0; s < sectors.size(); s++) {
        Sector* s_ptr = sectors[s];
        s_ptr->triangles.clear();
        TRIANGULATION_ERROR res =
            triangulate_sector(s_ptr, &lone_edges, false);
            
        if(res != TRIANGULATION_ERROR_NONE && level == CONTENT_LOAD_LEVEL_EDITOR) {
            problems.non_simples[s_ptr] = res;
            problems.lone_edges.insert(
                lone_edges.begin(), lone_edges.end()
            );
        }
        
        s_ptr->calculate_bounding_box();
    }
    
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) generate_blockmap();
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads the thumbnail image from the disk and updates the
 * thumbnail class member.
 *
 * @param thumbnail_path Path to the bitmap.
 */
void Area::load_thumbnail(const string &thumbnail_path) {
    if(thumbnail) {
        thumbnail = nullptr;
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


/**
 * @brief Adds a new edge to the list.
 *
 * @return The new edge's pointer.
 */
Edge* Area::new_edge() {
    Edge* e_ptr = new Edge();
    edges.push_back(e_ptr);
    return e_ptr;
}


/**
 * @brief Adds a new sector to the list.
 *
 * @return The new sector's pointer.
 */
Sector* Area::new_sector() {
    Sector* s_ptr = new Sector();
    sectors.push_back(s_ptr);
    return s_ptr;
}


/**
 * @brief Adds a new vertex to the list.
 *
 * @return The new vertex's pointer.
 */
Vertex* Area::new_vertex() {
    Vertex* v_ptr = new Vertex();
    vertexes.push_back(v_ptr);
    return v_ptr;
}


/**
 * @brief Removes an edge from the list, and updates all indexes after it.
 *
 * @param e_idx Index number of the edge to remove.
 */
void Area::remove_edge(size_t e_idx) {
    edges.erase(edges.begin() + e_idx);
    for(size_t v = 0; v < vertexes.size(); v++) {
        Vertex* v_ptr = vertexes[v];
        for(size_t e = 0; e < v_ptr->edges.size(); e++) {
            if(
                v_ptr->edge_idxs[e] != INVALID &&
                v_ptr->edge_idxs[e] > e_idx
            ) {
                v_ptr->edge_idxs[e]--;
            } else {
                //This should never happen.
                engine_assert(
                    v_ptr->edge_idxs[e] != e_idx,
                    i2s(v_ptr->edge_idxs[e]) + " " + i2s(e_idx)
                );
            }
        }
    }
    for(size_t s = 0; s < sectors.size(); s++) {
        Sector* s_ptr = sectors[s];
        for(size_t e = 0; e < s_ptr->edges.size(); e++) {
            if(
                s_ptr->edge_idxs[e] != INVALID &&
                s_ptr->edge_idxs[e] > e_idx
            ) {
                s_ptr->edge_idxs[e]--;
            } else {
                //This should never happen.
                engine_assert(
                    s_ptr->edge_idxs[e] != e_idx,
                    i2s(s_ptr->edge_idxs[e]) + " " + i2s(e_idx)
                );
            }
        }
    }
}


/**
 * @brief Removes an edge from the list, and updates all indexes after it.
 *
 * @param e_ptr Pointer of the edge to remove.
 */
void Area::remove_edge(const Edge* e_ptr) {
    for(size_t e = 0; e < edges.size(); e++) {
        if(edges[e] == e_ptr) {
            remove_edge(e);
            return;
        }
    }
}


/**
 * @brief Removes a sector from the list, and updates all indexes after it.
 *
 * @param s_idx Index number of the sector to remove.
 */
void Area::remove_sector(size_t s_idx) {
    sectors.erase(sectors.begin() + s_idx);
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* e_ptr = edges[e];
        for(size_t s = 0; s < 2; s++) {
            if(
                e_ptr->sector_idxs[s] != INVALID &&
                e_ptr->sector_idxs[s] > s_idx
            ) {
                e_ptr->sector_idxs[s]--;
            } else {
                //This should never happen.
                engine_assert(
                    e_ptr->sector_idxs[s] != s_idx,
                    i2s(e_ptr->sector_idxs[s]) + " " + i2s(s_idx)
                );
            }
        }
    }
}


/**
 * @brief Removes a sector from the list, and updates all indexes after it.
 *
 * @param s_ptr Pointer of the sector to remove.
 */
void Area::remove_sector(const Sector* s_ptr) {
    for(size_t s = 0; s < sectors.size(); s++) {
        if(sectors[s] == s_ptr) {
            remove_sector(s);
            return;
        }
    }
}


/**
 * @brief Removes a vertex from the list, and updates all indexes after it.
 *
 * @param v_idx Index number of the vertex to remove.
 */
void Area::remove_vertex(size_t v_idx) {
    vertexes.erase(vertexes.begin() + v_idx);
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* e_ptr = edges[e];
        for(size_t v = 0; v < 2; v++) {
            if(
                e_ptr->vertex_idxs[v] != INVALID &&
                e_ptr->vertex_idxs[v] > v_idx
            ) {
                e_ptr->vertex_idxs[v]--;
            } else {
                //This should never happen.
                engine_assert(
                    e_ptr->vertex_idxs[v] != v_idx,
                    i2s(e_ptr->vertex_idxs[v]) + " " + i2s(v_idx)
                );
            }
        }
    }
}


/**
 * @brief Removes a vertex from the list, and updates all indexes after it.
 *
 * @param v_ptr Pointer of the vertex to remove.
 */
void Area::remove_vertex(const Vertex* v_ptr) {
    for(size_t v = 0; v < vertexes.size(); v++) {
        if(vertexes[v] == v_ptr) {
            remove_vertex(v);
            return;
        }
    }
}


/**
 * @brief Saves the area's geometry to a data node.
 *
 * @param node Data node to save to.
 */
void Area::save_geometry_to_data_node(DataNode* node) {
    //Vertexes.
    DataNode* vertexes_node = node->addNew("vertexes");
    for(size_t v = 0; v < vertexes.size(); v++) {

        //Vertex.
        Vertex* v_ptr = vertexes[v];
        vertexes_node->addNew("v", p2s(v2p(v_ptr)));
    }
    
    //Edges.
    DataNode* edges_node = node->addNew("edges");
    for(size_t e = 0; e < edges.size(); e++) {

        //Edge.
        Edge* e_ptr = edges[e];
        DataNode* edge_node = edges_node->addNew("e");
        GetterWriter egw(edge_node);

        string s_str;
        for(size_t s = 0; s < 2; s++) {
            if(e_ptr->sector_idxs[s] == INVALID) s_str += "-1";
            else s_str += i2s(e_ptr->sector_idxs[s]);
            s_str += " ";
        }
        s_str.erase(s_str.size() - 1);
        string v_str =
            i2s(e_ptr->vertex_idxs[0]) + " " + i2s(e_ptr->vertex_idxs[1]);

        egw.get("s", s_str);
        egw.get("v", v_str);
        
        if(e_ptr->wall_shadow_length != LARGE_FLOAT) {
            egw.get("shadow_length", e_ptr->wall_shadow_length);
        }
        
        if(e_ptr->wall_shadow_color != GEOMETRY::SHADOW_DEF_COLOR) {
            egw.get("shadow_color", e_ptr->wall_shadow_color);
        }
        
        if(e_ptr->ledge_smoothing_length != 0.0f) {
            egw.get("smoothing_length", e_ptr->ledge_smoothing_length);
        }
        
        if(e_ptr->ledge_smoothing_color != GEOMETRY::SMOOTHING_DEF_COLOR) {
            egw.get("smoothing_color", e_ptr->ledge_smoothing_color);
        }
    }
    
    //Sectors.
    DataNode* sectors_node = node->addNew("sectors");
    for(size_t s = 0; s < sectors.size(); s++) {

        //Sector.
        Sector* s_ptr = sectors[s];
        DataNode* sector_node = sectors_node->addNew("s");
        GetterWriter sgw(sector_node);
        
        if(s_ptr->type != SECTOR_TYPE_NORMAL) {
            sgw.get("type", game.sector_types.get_name(s_ptr->type));
        }
        if(s_ptr->is_bottomless_pit) {
            sgw.get("is_bottomless_pit", true);
        }
        sgw.get("z", s_ptr->z);
        if(s_ptr->brightness != GEOMETRY::DEF_SECTOR_BRIGHTNESS) {
            sgw.get("brightness", s_ptr->brightness);
        }
        if(!s_ptr->tag.empty()) {
            sgw.get("tag", s_ptr->tag);
        }
        if(s_ptr->fade) {
            sgw.get("fade", s_ptr->fade);
        }
        if(!s_ptr->hazards_str.empty()) {
            sgw.get("hazards", s_ptr->hazards_str);
            sgw.get("hazards_floor", s_ptr->hazard_floor);
        }
        
        if(!s_ptr->texture_info.bmp_name.empty()) {
            sgw.get("texture", s_ptr->texture_info.bmp_name);
        }
        
        if(s_ptr->texture_info.rot != 0) {
            sgw.get("texture_rotate", s_ptr->texture_info.rot);
        }
        if(
            s_ptr->texture_info.scale.x != 1 ||
            s_ptr->texture_info.scale.y != 1
        ) {
            sgw.get("texture_scale", s_ptr->texture_info.scale);
        }
        if(
            s_ptr->texture_info.translation.x != 0 ||
            s_ptr->texture_info.translation.y != 0
        ) {
            sgw.get("texture_trans", s_ptr->texture_info.translation);
        }
        if(
            s_ptr->texture_info.tint.r != 1.0 ||
            s_ptr->texture_info.tint.g != 1.0 ||
            s_ptr->texture_info.tint.b != 1.0 ||
            s_ptr->texture_info.tint.a != 1.0
        ) {
            sgw.get("texture_tint", s_ptr->texture_info.tint);
        }
        
    }
    
    //Mobs.
    DataNode* mobs_node = node->addNew("mobs");
    for(size_t m = 0; m < mob_generators.size(); m++) {

        //Mob.
        MobGen* m_ptr = mob_generators[m];
        string cat_name = "unknown";
        if(m_ptr->type && m_ptr->type->category) {
            cat_name = m_ptr->type->category->internal_name;
        }
        DataNode* mob_node = mobs_node->addNew(cat_name);
        GetterWriter mgw(mob_node);
        
        if(m_ptr->type) {
            mgw.get("type", m_ptr->type->manifest->internal_name);
        }
        mgw.get("p", m_ptr->pos);
        if(m_ptr->angle != 0) {
            mgw.get("angle", m_ptr->angle);
        }
        if(!m_ptr->vars.empty()) {
            mgw.get("vars", m_ptr->vars);
        }
        
        string links_str;
        for(size_t l = 0; l < m_ptr->link_idxs.size(); l++) {
            if(l > 0) links_str += " ";
            links_str += i2s(m_ptr->link_idxs[l]);
        }
        
        if(!links_str.empty()) {
            mgw.get("links", links_str);
        }
        
        if(m_ptr->stored_inside != INVALID) {
            mgw.get("stored_inside", m_ptr->stored_inside);
        }
    }
    
    //Path stops.
    DataNode* path_stops_node = node->addNew("path_stops");
    for(size_t s = 0; s < path_stops.size(); s++) {

        //Path stop.
        PathStop* s_ptr = path_stops[s];
        DataNode* path_stop_node = path_stops_node->addNew("s");
        GetterWriter sgw(path_stop_node);
        
        sgw.get("pos", s_ptr->pos);
        if(s_ptr->radius != PATHS::MIN_STOP_RADIUS) {
            sgw.get("radius", s_ptr->radius);
        }
        if(s_ptr->flags != 0) {
            sgw.get("flags", s_ptr->flags);
        }
        if(!s_ptr->label.empty()) {
            sgw.get("label", s_ptr->label);
        }
        
        DataNode* links_node = path_stop_node->addNew("links");
        for(size_t l = 0; l < s_ptr->links.size(); l++) {
            PathLink* l_ptr = s_ptr->links[l];
            string link_data = i2s(l_ptr->end_idx);
            if(l_ptr->type != PATH_LINK_TYPE_NORMAL) {
                link_data += " " + i2s(l_ptr->type);
            }
            links_node->addNew("l", link_data);
        }
        
    }
    
    //Tree shadows.
    DataNode* shadows_node = node->addNew("tree_shadows");
    for(size_t s = 0; s < tree_shadows.size(); s++) {

        //Tree shadow.
        TreeShadow* s_ptr = tree_shadows[s];
        DataNode* shadow_node = shadows_node->addNew("shadow");
        GetterWriter sgw(shadow_node);
        
        sgw.get("pos", s_ptr->center);
        sgw.get("size", s_ptr->size);
        sgw.get("file", s_ptr->bmp_name);
        sgw.get("sway", s_ptr->sway);
        if(s_ptr->angle != 0) {
            sgw.get("angle", s_ptr->angle);
        }
        if(s_ptr->alpha != 255) {
            sgw.get("alpha", s_ptr->alpha);
        }
        
    }
}


/**
 * @brief Saves the area's main data to a data node.
 *
 * @param node Data node to save to.
 */
void Area::save_main_data_to_data_node(DataNode* node) {
    //Content metadata.
    save_metadata_to_data_node(node);

    GetterWriter gw(node);
    
    //Main data.
    gw.get("subtitle", subtitle);
    gw.get("difficulty", difficulty);
    gw.get("bg_bmp", bg_bmp_name);
    gw.get("bg_color", bg_color);
    gw.get("bg_dist", bg_dist);
    gw.get("bg_zoom", bg_bmp_zoom);
    gw.get("song", song_name);
    gw.get("weather", weather_name);
    gw.get("day_time_start", day_time_start);
    gw.get("day_time_speed", day_time_speed);
    gw.get("spray_amounts", spray_amounts);
}


/**
 * @brief Saves the area's mission data to a data node.
 *
 * @param node Data node to save to.
 */
void Area::save_mission_data_to_data_node(DataNode* node) {
    GetterWriter gw(node);

    if(mission.goal != MISSION_GOAL_END_MANUALLY) {
        string goal_name = game.mission_goals[mission.goal]->get_name();
        gw.get("mission_goal", goal_name);
    }
    if(
        mission.goal == MISSION_GOAL_TIMED_SURVIVAL ||
        mission.goal == MISSION_GOAL_GROW_PIKMIN
    ) {
        gw.get("mission_goal_amount", mission.goal_amount);
    }
    if(
        mission.goal == MISSION_GOAL_COLLECT_TREASURE ||
        mission.goal == MISSION_GOAL_BATTLE_ENEMIES ||
        mission.goal == MISSION_GOAL_GET_TO_EXIT
    ) {
        gw.get("mission_goal_all_mobs", mission.goal_all_mobs);
        vector<string> mission_mob_idx_strs;
        for(auto m : mission.goal_mob_idxs) {
            mission_mob_idx_strs.push_back(i2s(m));
        }
        string mission_mob_idx_str = join(mission_mob_idx_strs, ";");
        if(!mission_mob_idx_str.empty()) {
            gw.get("mission_required_mobs", mission_mob_idx_str);
        }
    }
    if(mission.goal == MISSION_GOAL_GET_TO_EXIT) {
        gw.get("mission_goal_exit_center", mission.goal_exit_center);
        gw.get("mission_goal_exit_size", mission.goal_exit_size);
    }
    if(mission.fail_conditions > 0) {
        gw.get("mission_fail_conditions", mission.fail_conditions);
    }
    if(
        has_flag(
            mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_TOO_FEW_PIKMIN)
        )
    ) {
        gw.get("mission_fail_too_few_pik_amount", mission.fail_too_few_pik_amount);
    }
    if(
        has_flag(
            mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_TOO_MANY_PIKMIN)
        )
    ) {
        gw.get("mission_fail_too_many_pik_amount", mission.fail_too_many_pik_amount);
    }
    if(
        has_flag(
            mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_LOSE_PIKMIN)
        )
    ) {
        gw.get("mission_fail_pik_killed", mission.fail_pik_killed);
    }
    if(
        has_flag(
            mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_LOSE_LEADERS)
        )
    ) {
        gw.get("mission_fail_leaders_kod", mission.fail_leaders_kod);
    }
    if(
        has_flag(
            mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_KILL_ENEMIES)
        )
    ) {
        gw.get("mission_fail_enemies_killed", mission.fail_enemies_killed);
    }
    if(
        has_flag(
            mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        gw.get("mission_fail_time_limit", mission.fail_time_limit);
    }
    if(mission.fail_hud_primary_cond != INVALID) {
        gw.get("mission_fail_hud_primary_cond", mission.fail_hud_primary_cond);
    }
    if(mission.fail_hud_secondary_cond != INVALID) {
        gw.get("mission_fail_hud_secondary_cond", mission.fail_hud_secondary_cond);
    }
    gw.get("mission_grading_mode", mission.grading_mode);
    if(mission.grading_mode == MISSION_GRADING_MODE_POINTS) {
        if(mission.points_per_pikmin_born != 0) {
            gw.get("mission_points_per_pikmin_born", mission.points_per_pikmin_born);
        }
        if(mission.points_per_pikmin_death != 0) {
            gw.get("mission_points_per_pikmin_death", mission.points_per_pikmin_death);
        }
        if(mission.points_per_sec_left != 0) {
            gw.get("mission_points_per_sec_left", mission.points_per_sec_left);
        }
        if(mission.points_per_sec_passed != 0) {
            gw.get("mission_points_per_sec_passed", mission.points_per_sec_passed);
        }
        if(mission.points_per_treasure_point != 0) {
            gw.get("mission_points_per_treasure_point", mission.points_per_treasure_point);
        }
        if(mission.points_per_enemy_point != 0) {
            gw.get("mission_points_per_enemy_point", mission.points_per_enemy_point);
        }
        if(mission.point_loss_data > 0) {
            gw.get("mission_point_loss_data", mission.point_loss_data);
        }
        if(mission.point_hud_data != 255) {
            gw.get("mission_point_hud_data", mission.point_hud_data);
        }
        if(mission.starting_points != 0) {
            gw.get("mission_starting_points", mission.starting_points);
        }
        gw.get("mission_bronze_req", mission.bronze_req);
        gw.get("mission_silver_req", mission.silver_req);
        gw.get("mission_gold_req", mission.gold_req);
        gw.get("mission_platinum_req", mission.platinum_req);
        if(!mission.maker_record_date.empty()) {
            gw.get("mission_maker_record", mission.maker_record);
            gw.get("mission_maker_record_date", mission.maker_record_date);
        }
    }
}


/**
 * @brief Saves the area's thumbnail to the disk, or deletes it from the disk
 * if it's meant to not exist.
 *
 * @param to_backup Whether it's to save to the area backup.
 */
void Area::save_thumbnail(bool to_backup) {
    string thumb_path =
        (to_backup ? user_data_path : manifest->path) +
        "/" + FILE_NAMES::AREA_THUMBNAIL;
    if(thumbnail) {
        al_save_bitmap(thumb_path.c_str(), thumbnail.get());
    } else {
        al_remove_filename(thumb_path.c_str());
    }
}


/**
 * @brief Clears the info of the blockmap.
 */
void Blockmap::clear() {
    top_left_corner = Point();
    edges.clear();
    sectors.clear();
    n_cols = 0;
    n_rows = 0;
}


/**
 * @brief Returns the block column in which an X coordinate is contained.
 *
 * @param x X coordinate.
 * @return The column, or INVALID on error.
 */
size_t Blockmap::get_col(float x) const {
    if(x < top_left_corner.x) return INVALID;
    float final_x = (x - top_left_corner.x) / GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    if(final_x >= n_cols) return INVALID;
    return final_x;
}


/**
 * @brief Obtains a list of edges that are within the specified
 * rectangular region.
 *
 * @param tl Top-left coordinates of the region.
 * @param br Bottom-right coordinates of the region.
 * @param out_edges Set to fill the edges into.
 * @return Whether it succeeded.
 */
bool Blockmap::get_edges_in_region(
    const Point &tl, const Point &br, set<Edge*> &out_edges
) const {

    size_t bx1 = get_col(tl.x);
    size_t bx2 = get_col(br.x);
    size_t by1 = get_row(tl.y);
    size_t by2 = get_row(br.y);
    
    if(
        bx1 == INVALID || bx2 == INVALID ||
        by1 == INVALID || by2 == INVALID
    ) {
        //Out of bounds.
        return false;
    }
    
    for(size_t bx = bx1; bx <= bx2; bx++) {
        for(size_t by = by1; by <= by2; by++) {
        
            const vector<Edge*> &block_edges = edges[bx][by];
            
            for(size_t e = 0; e < block_edges.size(); e++) {
                out_edges.insert(block_edges[e]);
            }
        }
    }
    
    return true;
}


/**
 * @brief Returns the block row in which a Y coordinate is contained.
 *
 * @param y Y coordinate.
 * @return The row, or INVALID on error.
 */
size_t Blockmap::get_row(float y) const {
    if(y < top_left_corner.y) return INVALID;
    float final_y = (y - top_left_corner.y) / GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    if(final_y >= n_rows) return INVALID;
    return final_y;
}


/**
 * @brief Returns the top-left coordinates for the specified column and row.
 *
 * @param col Column to check.
 * @param row Row to check.
 * @return The top-left coordinates.
 */
Point Blockmap::get_top_left_corner(size_t col, size_t row) const {
    return
        Point(
            col * GEOMETRY::BLOCKMAP_BLOCK_SIZE + top_left_corner.x,
            row * GEOMETRY::BLOCKMAP_BLOCK_SIZE + top_left_corner.y
        );
}


/**
 * @brief Constructs a new mob generator object.
 *
 * @param pos Coordinates.
 * @param type The mob type.
 * @param angle Angle it is facing.
 * @param vars String representation of the script vars.
 */
MobGen::MobGen(
    const Point &pos, MobType* type, float angle, const string &vars
) :
    type(type),
    pos(pos),
    angle(angle),
    vars(vars) {
    
}


/**
 * @brief Clones the properties of this mob generator onto another
 * mob generator.
 *
 * @param destination Mob generator to clone the data into.
 * @param include_position If true, the position is included too.
 */
void MobGen::clone(MobGen* destination, bool include_position) const {
    destination->angle = angle;
    if(include_position) destination->pos = pos;
    destination->type = type;
    destination->vars = vars;
    destination->link_idxs = link_idxs;
    destination->stored_inside = stored_inside;
}


/**
 * @brief Constructs a new tree shadow object.
 *
 * @param center Center coordinates.
 * @param size Width and height.
 * @param angle Angle it is rotated by.
 * @param alpha How opaque it is [0-255].
 * @param bmp_name Internal name of the tree shadow texture's bitmap.
 * @param sway Multiply the sway distance by this much, horizontally and
 * vertically.
 */
TreeShadow::TreeShadow(
    const Point &center, const Point &size, float angle,
    unsigned char alpha, const string &bmp_name, const Point &sway
) :
    bmp_name(bmp_name),
    bitmap(nullptr),
    center(center),
    size(size),
    angle(angle),
    alpha(alpha),
    sway(sway) {
    
}


/**
 * @brief Destroys the tree shadow object.
 *
 */
TreeShadow::~TreeShadow() {
    game.content.bitmaps.list.free(bmp_name);
}
