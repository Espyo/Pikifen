/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Sector, edge, etc. classes, and related functions.
 */

#define _USE_MATH_DEFINES

#include <algorithm>
#include <cfloat>
#include <math.h>
#include <queue>
#include <unordered_set>

#include "area_editor.h"
#include "functions.h"
#include "sector.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates info on an area.
 */
area_data::area_data() :
    bg_bmp(nullptr),
    bg_bmp_zoom(1),
    bg_dist(2),
    bg_color(map_gray(0)) {

}


/* ----------------------------------------------------------------------------
 * Generates the blockmap for the area, given the current info.
 */
void area_data::generate_blockmap() {
    bmap.clear();

    if(vertexes.empty()) return;

    //First, get the starting point and size of the blockmap.
    float min_x, max_x, min_y, max_y;
    min_x = max_x = vertexes[0]->x;
    min_y = max_y = vertexes[0]->y;

    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        min_x = min(v_ptr->x, min_x);
        max_x = max(v_ptr->x, max_x);
        min_y = min(v_ptr->y, min_y);
        max_y = max(v_ptr->y, max_y);
    }

    bmap.x1 = min_x; bmap.y1 = min_y;
    //Add one more to the cols/rows because, suppose there's an edge at y = 256.
    //The row would be 2. In reality, the row should be 3.
    bmap.n_cols = ceil((max_x - min_x) / BLOCKMAP_BLOCK_SIZE) + 1;
    bmap.n_rows = ceil((max_y - min_y) / BLOCKMAP_BLOCK_SIZE) + 1;

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


    //If at this point, there's any block without a sector, that means
    //that the block has no edges. It has, however, a single sector,
    //so use the triangle method to get the sector. Checking the center is
    //just a good a spot as any.
    for(size_t bx = 0; bx < bmap.n_cols; ++bx) {
        for(size_t by = 0; by < bmap.n_rows; ++by) {

            if(bmap.sectors[bx][by].empty()) {

                bmap.sectors[bx][by].insert(
                    get_sector(
                        bmap.get_x1(bx) + BLOCKMAP_BLOCK_SIZE * 0.5,
                        bmap.get_y1(by) + BLOCKMAP_BLOCK_SIZE * 0.5,
                        NULL, false
                    )
                );
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Generates the blockmap for a set of edges.
 */
void area_data::generate_edges_blockmap(vector<edge*> &edges) {
    size_t b_min_x, b_max_x, b_min_y, b_max_y;

    for(size_t e = 0; e < edges.size(); ++e) {

        //Get which blocks this edge belongs to, via bounding-box,
        //and only then thoroughly test which it is inside of.

        edge* e_ptr = edges[e];

        b_min_x =
            bmap.get_col(min(e_ptr->vertexes[0]->x, e_ptr->vertexes[1]->x));
        b_max_x =
            bmap.get_col(max(e_ptr->vertexes[0]->x, e_ptr->vertexes[1]->x));
        b_min_y =
            bmap.get_row(min(e_ptr->vertexes[0]->y, e_ptr->vertexes[1]->y));
        b_max_y =
            bmap.get_row(max(e_ptr->vertexes[0]->y, e_ptr->vertexes[1]->y));

        for(size_t bx = b_min_x; bx <= b_max_x; ++bx) {
            for(size_t by = b_min_y; by <= b_max_y; ++by) {

                //Get the block's coordinates.
                float bx1 = bmap.get_x1(bx);
                float by1 = bmap.get_y1(by);

                //Check if the edge is inside this blockmap.
                if(
                    square_intersects_line(
                        bx1, by1,
                        bx1 + BLOCKMAP_BLOCK_SIZE, by1 + BLOCKMAP_BLOCK_SIZE,
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y,
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    )
                ) {

                    //If it is, add it and the sectors to the list.
                    bool add_edge = true;
                    if(e_ptr->sectors[0] && e_ptr->sectors[1]) {
                        //If there's no change in height, why bother?
                        if(
                            (e_ptr->sectors[0]->z == e_ptr->sectors[1]->z) &&
                            e_ptr->sectors[0]->type != SECTOR_TYPE_GATE &&
                            e_ptr->sectors[1]->type != SECTOR_TYPE_GATE &&
                            e_ptr->sectors[0]->type != SECTOR_TYPE_BLOCKING &&
                            e_ptr->sectors[1]->type != SECTOR_TYPE_BLOCKING
                        ) {
                            add_edge = false;
                        }
                    }

                    if(add_edge) bmap.edges[bx][by].push_back(e_ptr);

                    if(e_ptr->sectors[0]) {
                        bmap.sectors[bx][by].insert(e_ptr->sectors[0]);
                    }
                    if(e_ptr->sectors[1]) {
                        bmap.sectors[bx][by].insert(e_ptr->sectors[1]);
                    }
                }
            }
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

    if(!bg_bmp_file_name.empty()) {
        bitmaps.detach(bg_bmp_file_name);
    }
    bg_bmp_file_name.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a blockmap.
 */
blockmap::blockmap() :
    x1(0),
    y1(0),
    n_cols(0),
    n_rows(0) {

}


/* ----------------------------------------------------------------------------
 * Clears the info of the blockmap.
 */
void blockmap::clear() {
    x1 = y1 = 0;
    edges.clear();
    sectors.clear();
}


/* ----------------------------------------------------------------------------
 * Returns the block column in which an X coordinate is contained.
 * Returns INVALID on error.
 */
size_t blockmap::get_col(const float x) {
    if(x < x1) return INVALID;
    float final_x = (x - x1) / BLOCKMAP_BLOCK_SIZE;
    if(final_x >= n_cols) return INVALID;
    return final_x;
}


/* ----------------------------------------------------------------------------
 * Returns the block row in which a Y coordinate is contained.
 * Returns INVALID on error.
 */
size_t blockmap::get_row(const float y) {
    if(y < y1) return INVALID;
    float final_y = (y - y1) / BLOCKMAP_BLOCK_SIZE;
    if(final_y >= n_rows) return INVALID;
    return final_y;
}


/* ----------------------------------------------------------------------------
 * Returns the top-left X coordinate for the specified column.
 */
float blockmap::get_x1(const size_t col) {
    return col * BLOCKMAP_BLOCK_SIZE + x1;
}


/* ----------------------------------------------------------------------------
 * Returns the top-left Y coordinate for the specified row.
 */
float blockmap::get_y1(const size_t row) {
    return row * BLOCKMAP_BLOCK_SIZE + y1;
}


/* ----------------------------------------------------------------------------
 * Creates an edge.
 */
edge::edge(size_t v1, size_t v2) {
    vertexes[0] = vertexes[1] = NULL;
    sectors[0] = sectors[1] = NULL;
    sector_nrs[0] = sector_nrs[1] = INVALID;

    vertex_nrs[0] = v1; vertex_nrs[1] = v2;
}


/* ----------------------------------------------------------------------------
 * Fixes the pointers to point to the correct sectors and vertexes.
 */
void edge::fix_pointers(area_data &a) {
    sectors[0] = sectors[1] = NULL;
    for(size_t s = 0; s < 2; ++s) {
        size_t s_nr = sector_nrs[s];
        sectors[s] = (s_nr == INVALID ? NULL : a.sectors[s_nr]);
    }

    vertexes[0] = vertexes[1] = NULL;
    for(size_t v = 0; v < 2; ++v) {
        size_t v_nr = vertex_nrs[v];
        vertexes[v] = (v_nr == INVALID ? NULL : a.vertexes[v_nr]);
    }
}


/* ----------------------------------------------------------------------------
 * Removes the edge from its sectors, but doesn't mark
 * the sectors as "none".
 * Returns the edge number.
 */
size_t edge::remove_from_sectors() {
    size_t e_nr = INVALID;
    for(unsigned char s = 0; s < 2; ++s) {
        if(!sectors[s]) continue;
        for(size_t e = 0; e < sectors[s]->edges.size(); ++e) {
            edge* e_ptr = sectors[s]->edges[e];
            if(e_ptr == this) {
                sectors[s]->edges.erase(sectors[s]->edges.begin() + e);
                auto nr_it = sectors[s]->edge_nrs.begin() + e;
                e_nr = *nr_it;
                sectors[s]->edge_nrs.erase(nr_it);
                break;
            }
        }
    }
    return e_nr;
}


/* ----------------------------------------------------------------------------
 * Removes the edge from its vertexes, but doesn't mark
 * the vertexes as "none".
 * Returns the edge number.
 */
size_t edge::remove_from_vertexes() {
    size_t e_nr = INVALID;
    for(unsigned char v = 0; v < 2; ++v) {
        if(!vertexes[v]) continue;
        for(size_t e = 0; e < vertexes[v]->edges.size(); ++e) {
            edge* e_ptr = vertexes[v]->edges[e];
            if(e_ptr == this) {
                vertexes[v]->edges.erase(vertexes[v]->edges.begin() + e);
                auto nr_it = vertexes[v]->edge_nrs.begin() + e;
                e_nr = *nr_it;
                vertexes[v]->edge_nrs.erase(nr_it);
                break;
            }
        }
    }
    return e_nr;
}


/* ----------------------------------------------------------------------------
 * Creates a mob generation structure.
 */
mob_gen::mob_gen(
    float x, float y, unsigned char category,
    mob_type* type, float angle, string vars
) :
    category(category),
    type(type),
    x(x),
    y(y),
    angle(angle),
    vars(vars) {

}


/* ----------------------------------------------------------------------------
 * Creates a sector.
 */
sector::sector() :
    type(SECTOR_TYPE_NORMAL),
    z(0),
    hazard_floor(true),
    brightness(DEF_SECTOR_BRIGHTNESS),
    fade(false),
    always_cast_shadow(false) {

}


/* ----------------------------------------------------------------------------
 * Clones a sector's properties onto another,
 * not counting the list of edges or bitmap
 * (the file name is cloned too, though).
 */
void sector::clone(sector* new_sector) {
    new_sector->type = type;
    new_sector->z = z;
    new_sector->tag = tag;
    new_sector->brightness = brightness;
    new_sector->fade = fade;
    new_sector->texture_info.scale_x = texture_info.scale_x;
    new_sector->texture_info.scale_y = texture_info.scale_y;
    new_sector->texture_info.trans_x = texture_info.trans_y;
    new_sector->texture_info.rot = texture_info.rot;
    new_sector->texture_info.file_name = texture_info.file_name;
    //TODO hazards.
}


/* ----------------------------------------------------------------------------
 * Destroys a sector.
 */
sector::~sector() {
    for(size_t t = 0; t < 2; ++t) {
        if(texture_info.bitmap && texture_info.bitmap != bmp_error) {
            bitmaps.detach(texture_info.file_name);
        }
    }
}



/* ----------------------------------------------------------------------------
 * Creates a sector texture's info struct.
 */
sector_texture_info::sector_texture_info() :
    scale_x(1),
    scale_y(1),
    trans_x(0),
    trans_y(0),
    rot(0),
    bitmap(nullptr) {
}


/* ----------------------------------------------------------------------------
 * Creates an edge intersection info structure.
 */
edge_intersection::edge_intersection(edge* e1, edge* e2) :
    e1(e1),
    e2(e2) {

}


/* ----------------------------------------------------------------------------
 * Checks whether the edge intersection contains the specified edge.
 */
bool edge_intersection::contains(edge* e) {
    return e1 == e || e2 == e;
}



/* ----------------------------------------------------------------------------
 * Creates a new path stop.
 */
path_stop::path_stop(float x, float y, vector<path_link> links) :
    x(x),
    y(y),
    links(links) {

}


/* ----------------------------------------------------------------------------
 * Returns whether this stop links to another stop or not.
 * The link is one-way, meaning that if the only link is from the other stop
 * to this one, it will not count.
 */
bool path_stop::has_link(path_stop* other_stop) {
    for(size_t l = 0; l < links.size(); ++l) {
        if(links[l].end_ptr == other_stop) return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Calculates the distance between it and all neighbors, and does the same
 * for the other paths, if they link back.
 */
void path_stop::calculate_dists() {
    for(size_t l = 0; l < links.size(); ++l) {
        path_link* l_ptr = &links[l];
        l_ptr->calculate_dist(this);

        for(size_t l2 = 0; l2 < l_ptr->end_ptr->links.size(); ++l2) {
            path_link* l2_ptr = &l_ptr->end_ptr->links[l2];

            if(l2_ptr->end_ptr == this) {
                l2_ptr->calculate_dist(l_ptr->end_ptr);
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new stop link.
 */
path_link::path_link(path_stop* end_ptr, size_t end_nr) :
    end_ptr(end_ptr),
    end_nr(end_nr),
    distance(0) {

}


/* ----------------------------------------------------------------------------
 * Calculates and stores the distance between the two stops.
 * Because the link doesn't know about the starting stop,
 * you need to provide it as a parameter when calling the function.
 */
void path_link::calculate_dist(path_stop* start_ptr) {
    distance =
        dist(
            start_ptr->x, start_ptr->y,
            end_ptr->x, end_ptr->y
        ).to_float();
}


/* ----------------------------------------------------------------------------
 * Fixes the link pointers to point them to the correct stops.
 */
void path_stop::fix_pointers(area_data &a) {
    for(size_t l = 0; l < links.size(); ++l) {
        path_link* l_ptr = &links[l];
        l_ptr->end_ptr = NULL;

        if(l_ptr->end_nr == INVALID) continue;
        if(l_ptr->end_nr >= a.path_stops.size()) continue;

        l_ptr->end_ptr = a.path_stops[l_ptr->end_nr];
    }
}


/* ----------------------------------------------------------------------------
 * Fixes the link numbers to match them to the correct stop pointer.
 */
void path_stop::fix_nrs(area_data &a) {
    for(size_t l = 0; l < links.size(); ++l) {
        path_link* l_ptr = &links[l];
        l_ptr->end_nr = INVALID;

        if(!l_ptr->end_ptr) continue;

        for(size_t s = 0; s < a.path_stops.size(); ++s) {
            if(a.path_stops[s] == l_ptr->end_ptr) {
                l_ptr->end_nr = s;
                break;
            }
        }
    }
}



/* ----------------------------------------------------------------------------
 * Connects the edges that link to it into the edge_nrs vector.
 */
void sector::connect_edges(area_data &a, size_t s_nr) {
    edge_nrs.clear();
    for(size_t e = 0; e < a.edges.size(); ++e) {
        edge* e_ptr = a.edges[e];
        if(e_ptr->sector_nrs[0] == s_nr || e_ptr->sector_nrs[1] == s_nr) {
            edge_nrs.push_back(e);
        }
    }
    fix_pointers(a);
}


/* ----------------------------------------------------------------------------
 * Fixes the pointers to point them to the correct edges.
 */
void sector::fix_pointers(area_data &a) {
    edges.clear();
    for(size_t e = 0; e < edge_nrs.size(); ++e) {
        size_t e_nr = edge_nrs[e];
        edges.push_back(e_nr == INVALID ? NULL : a.edges[e_nr]);
    }
}


/* ----------------------------------------------------------------------------
 * Creates a sector correction specification.
 */
sector_correction::sector_correction(sector* sec) :
    sec(sec) {

}


/* ----------------------------------------------------------------------------
 * Creates a vertex.
 */
vertex::vertex(float x, float y) :
    x(x),
    y(y) {

}


/* ----------------------------------------------------------------------------
 * Creates a tree shadow.
 */
tree_shadow::tree_shadow(
    float x, float y, float w, float h,
    float an, unsigned char al, string f, float sx, float sy
) :
    x(x),
    y(y),
    w(w),
    h(h),
    angle(an),
    alpha(al),
    sway_x(sx),
    sway_y(sy),
    file_name(f),
    bitmap(nullptr) {

}


/* ----------------------------------------------------------------------------
 * Creates a triangle.
 */
triangle::triangle(vertex* v1, vertex* v2, vertex* v3) {
    points[0] = v1;
    points[1] = v2;
    points[2] = v3;
}


/* ----------------------------------------------------------------------------
 * Connects the edges that link to it into the edge_nrs vector.
 */
void vertex::connect_edges(area_data &a, size_t v_nr) {
    edge_nrs.clear();
    for(size_t e = 0; e < a.edges.size(); ++e) {
        edge* e_ptr = a.edges[e];
        if(e_ptr->vertex_nrs[0] == v_nr || e_ptr->vertex_nrs[1] == v_nr) {
            edge_nrs.push_back(e);
        }
    }
    fix_pointers(a);
}


/* ----------------------------------------------------------------------------
 * Fixes the pointers to point to the correct edges.
 */
void vertex::fix_pointers(area_data &a) {
    edges.clear();
    for(size_t e = 0; e < edge_nrs.size(); ++e) {
        size_t e_nr = edge_nrs[e];
        edges.push_back(e_nr == INVALID ? NULL : a.edges[e_nr]);
    }
}


/* ----------------------------------------------------------------------------
 * Returns the shortest available path between two points, following
 * the area's path graph.
 * start_*:        Start coordinates.
 * end_*:          End coordinates.
 * obstacle_found: If an obstacle was found in the only path, this points to it.
 * go_straight:    This is set according to whether it's better
   * to go straight to the end point.
 * total_dist:     If not NULL, place the total path distance here.
 */
vector<path_stop*> get_path(
    const float start_x, const float start_y,
    const float end_x, const float end_y,
    mob** obstacle_found, bool* go_straight,
    float* total_dist
) {

    vector<path_stop*> full_path;
    if(go_straight) *go_straight = false;

    if(cur_area_data.path_stops.empty()) return full_path;

    //Start by finding the closest stops to the start and finish.
    path_stop* closest_to_start = NULL;
    path_stop* closest_to_end = NULL;
    dist closest_to_start_dist;
    dist closest_to_end_dist;

    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];

        dist dist_to_start(start_x, start_y, s_ptr->x, s_ptr->y);
        dist dist_to_end(end_x, end_y, s_ptr->x, s_ptr->y);

        if(!closest_to_start || dist_to_start < closest_to_start_dist) {
            closest_to_start_dist = dist_to_start;
            closest_to_start = s_ptr;
        }
        if(!closest_to_end || dist_to_end < closest_to_end_dist) {
            closest_to_end_dist = dist_to_end;
            closest_to_end = s_ptr;
        }
    }

    //Let's just check something real quick:
    //if the destination is closer than any stop,
    //just go there right away!
    dist start_to_end_dist = dist(start_x, start_y, end_x, end_y);
    if(start_to_end_dist <= closest_to_start_dist) {
        if(go_straight) *go_straight = true;
        if(total_dist) {
            *total_dist = start_to_end_dist.to_float();
        }
        return full_path;
    }

    //If the start and destination share the same closest spot,
    //that means this is the only stop in the path.
    if(closest_to_start == closest_to_end) {
        full_path.push_back(closest_to_start);
        if(total_dist) {
            *total_dist = closest_to_start_dist.to_float();
            *total_dist += closest_to_end_dist.to_float();
        }
        return full_path;
    }


    //Calculate the path.
    full_path =
        dijkstra(
            closest_to_start, closest_to_end, obstacle_found, total_dist
        );

    if(total_dist && !full_path.empty()) {
        *total_dist +=
            dist(
                start_x, start_y,
                full_path[0]->x, full_path[0]->y
            ).to_float();
        *total_dist +=
            dist(
                full_path[full_path.size() - 1]->x,
                full_path[full_path.size() - 1]->y,
                end_x, end_y
            ).to_float();
    }

    return full_path;
}


/* ----------------------------------------------------------------------------
 * Returns what active obstacle stands in the way of these two stops, if any.
 */
mob* get_path_link_obstacle(path_stop* s1, path_stop* s2) {
    for(size_t m = 0; m < mobs.size(); ++m) {
        mob* m_ptr = mobs[m];
        if(!m_ptr->type->is_obstacle) continue;

        if(
            m_ptr->health != 0 &&
            circle_intersects_line(
                m_ptr->x, m_ptr->y,
                m_ptr->type->radius,
                s1->x, s1->y,
                s2->x, s2->y
            )
        ) {
            return m_ptr;
        }
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns a point's sign on a line segment,
 * used for detecting if it's inside a triangle.
 */
float get_point_sign(
    float x, float y, float lx1, float ly1, float lx2, float ly2
) {
    return (x - lx2) * (ly1 - ly2) - (lx1 - lx2) * (y - ly2);
}


/* ----------------------------------------------------------------------------
 * Returns the outer polygon and inner polygons of a sector,
 * with the vertexes ordered counter-clockwise for the outer,
 * and clockwise for the inner.
 */
void get_polys(sector* s_ptr, polygon* outer, vector<polygon>* inners) {
    if(!s_ptr || !outer || !inners) return;

    bool doing_outer = true;

    //First, compile a list of all edges related to this sector.
    map<edge*, bool> edges_done;

    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        edges_done[s_ptr->edges[e]] = false;
    }

    area_editor* ae = NULL;
    if(cur_game_state_nr == GAME_STATE_AREA_EDITOR) {
        ae = (area_editor*) game_states[cur_game_state_nr];
    }

    //Now travel along the edges, vertex by vertex, until we have no more left.
    while(!edges_done.empty()) {
        bool poly_done = false;

        //Start with the rightmost vertex.
        //If we still haven't closed the outer polygon, then this vertex
        //mandatorily belongs to it. Otherwise, it belongs to an inner.
        vertex* cur_vertex = get_rightmost_vertex(edges_done);
        vertex* next_vertex = NULL;
        vertex* prev_vertex = NULL;

        float prev_angle = M_PI; //At the start, assume the angle is 0 (right).

        if(!doing_outer) {
            inners->push_back(polygon());
        }

        while(!poly_done) {

            float base_angle = prev_angle - M_PI; //The angle we came from.

            //For every edge attached to this vertex, find the closest one
            //that hasn't been done, in the direction of travel.

            float best_angle_dif = 0;
            edge* best_edge = NULL;

            for(size_t e = 0; e < cur_vertex->edges.size(); ++e) {
                edge* e_ptr = cur_vertex->edges[e];
                auto it = edges_done.find(e_ptr);
                if(it == edges_done.end()) {
                    //We're not meant to check this edge.
                    continue;
                }

                vertex* other_vertex =
                    e_ptr->vertexes[0] == cur_vertex ?
                    e_ptr->vertexes[1] :
                    e_ptr->vertexes[0];

                if(other_vertex == prev_vertex) {
                    //This is where we came from.
                    continue;
                }

                //Find the angle between our vertex and this vertex.
                float angle =
                    atan2(
                        other_vertex->y - cur_vertex->y,
                        other_vertex->x - cur_vertex->x
                    );
                float angle_dif = get_angle_cw_dif(angle, base_angle);

                //For the outer poly, we're going counter-clockwise.
                //So the lowest angle difference is best.
                //For the inner ones, it's clockwise, so the highest.
                if(
                    !best_edge ||
                    (doing_outer  && angle_dif < best_angle_dif) ||
                    (!doing_outer && angle_dif > best_angle_dif)
                ) {
                    best_edge = e_ptr;
                    best_angle_dif = angle_dif;
                    prev_angle = angle;
                    next_vertex = other_vertex;
                }
            }

            if(!best_edge) {

                //If there is no edge to go to next, something went wrong.

                //If this polygon is only one vertex, though, then
                //that means it was a stray edge. Remove it.
                //Otherwise, something just went wrong, and this is
                //a non-simple sector.
                poly_done = true;
                if(!doing_outer && inners->back().size() == 1) {
                    if(ae) {
                        ae->lone_edges.insert(inners->back()[0]->edges[0]);
                    }
                    inners->erase(inners->begin() + inners->size() - 1);
                } else {
                    if(ae) {
                        ae->non_simples.insert(s_ptr);
                    }
                }

            } else if(edges_done[best_edge]) {

                //If we already did this edge, that's it, polygon closed.
                poly_done = true;

            } else {

                if(doing_outer) {
                    outer->push_back(cur_vertex);
                } else {
                    inners->back().push_back(cur_vertex);
                }

                //Continue onto the next edge.
                prev_vertex = cur_vertex;
                cur_vertex = next_vertex;
                edges_done[best_edge] = true;

            }
        }

        doing_outer = false;

        //Remove all edges that were done from the list.
        auto it = edges_done.begin();
        while(it != edges_done.end()) {
            if(it->second) {
                edges_done.erase(it++);
            } else {
                ++it;
            }
        }
    }

    //Before we quit, let's just check if the sector
    //uses a vertex more than twice.
    if(ae) {
        map<vertex*, size_t> vertex_count;
        edge* e_ptr = NULL;
        for(size_t e = 0; e < s_ptr->edges.size(); e++) {
            e_ptr = s_ptr->edges[e];
            vertex_count[e_ptr->vertexes[0]]++;
            vertex_count[e_ptr->vertexes[1]]++;
        }

        for(auto v = vertex_count.begin(); v != vertex_count.end(); v++) {
            if(v->second > 2) {
                //Unfortunately, it does...
                //That means it's a non-simple sector.
                //This likely caused an incorrect triangulation,
                //so let's report it.
                ae->non_simples.insert(s_ptr);
                break;
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Places the bounding box coordinates of a sector on the specified floats.
 */
void get_sector_bounding_box(
    sector* s_ptr, float* min_x, float* min_y, float* max_x, float* max_y
) {
    if(!min_x || !min_y || !max_x || !max_y) return;
    *min_x = s_ptr->edges[0]->vertexes[0]->x;
    *max_x = *min_x;
    *min_y = s_ptr->edges[0]->vertexes[0]->y;
    *max_y = *min_y;

    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        for(unsigned char v = 0; v < 2; ++v) {
            float x = s_ptr->edges[e]->vertexes[v]->x;
            float y = s_ptr->edges[e]->vertexes[v]->y;

            *min_x = min(*min_x, x);
            *max_x = max(*max_x, x);
            *min_y = min(*min_y, y);
            *max_y = max(*max_y, y);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns which sector the specified point belongs to.
 * x, y:         Coordinates of the point.
 * sector_nr:    If not NULL, the number of the sector
   * on the area map is placed here.
   * The number will not be set if the search is using the blockmap.
 * use_blockmap: If true, use the blockmap to search.
   * This provides faster results, but the blockmap must be built.
 */
sector* get_sector(
    const float x, const float y, size_t* sector_nr, const bool use_blockmap
) {

    if(use_blockmap) {

        size_t col = cur_area_data.bmap.get_col(x);
        size_t row = cur_area_data.bmap.get_row(y);
        if(col == INVALID || row == INVALID) return NULL;

        unordered_set<sector*>* sectors = &cur_area_data.bmap.sectors[col][row];

        if(sectors->size() == 1) return *sectors->begin();

        for(auto s = sectors->begin(); s != sectors->end(); ++s) {

            if(is_point_in_sector(x, y, *s)) {
                return *s;
            }
        }

        if(sector_nr) *sector_nr = INVALID;
        return NULL;

    } else {

        for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
            sector* s_ptr = cur_area_data.sectors[s];

            if(is_point_in_sector(x, y, s_ptr)) {
                if(sector_nr) *sector_nr = s;
                return s_ptr;
            }

        }

        if(sector_nr) *sector_nr = INVALID;
        return NULL;

    }
}


/* ----------------------------------------------------------------------------
 * Places the bounding box coordinates of a shadow on the specified floats.
 */
void get_shadow_bounding_box(
    tree_shadow* s_ptr, float* min_x, float* min_y, float* max_x, float* max_y
) {

    if(!min_x || !min_y || !max_x || !max_y) return;
    bool got_min_x = false;
    bool got_max_x = false;
    bool got_min_y = false;
    bool got_max_y = false;

    for(unsigned char p = 0; p < 4; ++p) {
        float x, y, final_x, final_y;

        if(p == 0 || p == 1) x = s_ptr->x - (s_ptr->w * 0.5);
        else                 x = s_ptr->x + (s_ptr->w * 0.5);
        if(p == 0 || p == 2) y = s_ptr->y - (s_ptr->h * 0.5);
        else                 y = s_ptr->y + (s_ptr->h * 0.5);

        x -= s_ptr->x;
        y -= s_ptr->y;
        rotate_point(x, y, s_ptr->angle, &final_x, &final_y);
        final_x += s_ptr->x;
        final_y += s_ptr->y;

        if(final_x < *min_x || !got_min_x) {
            *min_x = final_x;
            got_min_x = true;
        }
        if(final_y < *min_y || !got_min_y) {
            *min_y = final_y;
            got_min_y = true;
        }
        if(final_x > *max_x || !got_max_x) {
            *max_x = final_x;
            got_max_x = true;
        }
        if(final_y > *max_y || !got_max_y) {
            *max_y = final_y;
            got_max_y = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether a point is inside a sector by checking its triangles.
 */
bool is_point_in_sector(const float x, const float y, sector* s_ptr) {
    for(size_t t = 0; t < s_ptr->triangles.size(); ++t) {
        triangle* t_ptr = &s_ptr->triangles[t];
        if(
            is_point_in_triangle(
                x, y,
                t_ptr->points[0]->x, t_ptr->points[0]->y,
                t_ptr->points[1]->x, t_ptr->points[1]->y,
                t_ptr->points[2]->x, t_ptr->points[2]->y,
                false
            )
        ) {
            return true;
        }
    }

    return false;
}


/* ----------------------------------------------------------------------------
 * Returns whether a point is inside a triangle or not.
 * px, py: Coordinates of the point to check.
 * t**:    Coordinates of the triangle's points.
 * loq:    Less or equal.
   * Different code requires different precision for on-line cases.
   * Just...don't overthink this, I added this based on what worked and didn't.
 * Thanks go to
   * http://stackoverflow.com/questions/2049582/
   * how-to-determine-a-point-in-a-triangle
 */
bool is_point_in_triangle(
    float px, float py,
    float tx1, float ty1, float tx2, float ty2, float tx3, float ty3,
    bool loq
) {

    bool b1, b2, b3;

    float f1, f2, f3;

    f1 = get_point_sign(px, py, tx1, ty1, tx2, ty2);
    f2 = get_point_sign(px, py, tx2, ty2, tx3, ty3);
    f3 = get_point_sign(px, py, tx3, ty3, tx1, ty1);

    if(loq) {
        b1 = f1 <= 0.0f;
        b2 = f2 <= 0.0f;
        b3 = f3 <= 0.0f;
    } else {
        b1 = f1 < 0.0f;
        b2 = f2 < 0.0f;
        b3 = f3 < 0.0f;
    }

    return ((b1 == b2) && (b2 == b3));

    //Old code.
    /*float dx = px - tx1;
    float dy = py - ty1;

    bool s_ab = (tx2 - tx1) * dy - (ty2 - ty1) * dx > 0;

    if((tx3 - tx1) * dy - (ty3 - ty1) * dx > 0 == s_ab) return false;

    if((tx3 - tx2) * (py - ty2) - (ty3 - ty2) * (px - tx2) > 0 != s_ab) {
        return false;
    }

    return true;*/
}


/* ----------------------------------------------------------------------------
 * Returns whether this vertex is convex or not.
 */
bool is_vertex_convex(const vector<vertex*> &vec, const size_t nr) {
    const vertex* cur_v = vec[nr];
    const vertex* prev_v = get_prev_in_vector(vec, nr);
    const vertex* next_v = get_next_in_vector(vec, nr);
    float angle_prev = atan2(prev_v->y - cur_v->y, prev_v->x - cur_v->x);
    float angle_next = atan2(next_v->y - cur_v->y, next_v->x - cur_v->x);

    return get_angle_cw_dif(angle_prev, angle_next) < M_PI;
}


/* ----------------------------------------------------------------------------
 * Returns whether this vertex is an ear or not.
 */
bool is_vertex_ear(
    const vector<vertex*> &vec, const vector<size_t> &concaves, const size_t nr
) {
    //A vertex is an ear if the triangle of it, the previous, and next vertexes
    //does not contain any other vertex inside. Also, if it has vertexes inside,
    //they mandatorily are concave, so only check those.
    const vertex* v = vec[nr];
    const vertex* pv = get_prev_in_vector(vec, nr);
    const vertex* nv = get_next_in_vector(vec, nr);

    for(size_t c = 0; c < concaves.size(); ++c) {
        const vertex* v_to_check = vec[concaves[c]];
        if(v_to_check == v || v_to_check == pv || v_to_check == nv) continue;
        if(
            is_point_in_triangle(
                v_to_check->x, v_to_check->y,
                pv->x, pv->y,
                v->x, v->y,
                nv->x, nv->y,
                true
            )
        ) return false;
    }

    return true;
}


/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right.
 */
vertex* get_rightmost_vertex(map<edge*, bool> &edges) {
    vertex* rightmost = NULL;

    for(auto l = edges.begin(); l != edges.end(); ++l) {
        if(!rightmost) rightmost = l->first->vertexes[0];

        for(unsigned char v = 0; v < 2; ++v) {
            rightmost = get_rightmost_vertex(l->first->vertexes[v], rightmost);
        }
    }

    return rightmost;
}


/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right in a polygon.
 */
vertex* get_rightmost_vertex(polygon* p) {
    vertex* rightmost = NULL;

    for(size_t v = 0; v < p->size(); ++v) {
        vertex* v_ptr = p->at(v);
        if(!rightmost) rightmost = v_ptr;
        else {
            rightmost = get_rightmost_vertex(v_ptr, rightmost);
        }
    }

    return rightmost;
}


/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right between the two.
 * In the case of a tie, the highest one is returned.
 * This is necessary because at one point, the rightmost
 * vertex was being decided kinda randomly.
 */
vertex* get_rightmost_vertex(vertex* v1, vertex* v2) {
    if(v1->x > v2->x) return v1;
    if(v1->x == v2->x && v1->y < v2->y) return v1;
    return v2;
}


/* ----------------------------------------------------------------------------
 * Checks intersecting edges, and adds them to intersecting_edges;
 */
void check_edge_intersections(vertex* v) {

    area_editor* ae = NULL;
    if(cur_game_state_nr == GAME_STATE_AREA_EDITOR) {
        ae = (area_editor*) game_states[cur_game_state_nr];
    }

    for(size_t e = 0; e < v->edges.size(); ++e) {
        edge* e_ptr = v->edges[e];

        if(ae) {
            //Check if it's on the list of intersecting edges, and remove it,
            //so it can be recalculated now.
            for(size_t ie = 0; ie < ae->intersecting_edges.size();) {
                if(ae->intersecting_edges[ie].contains(e_ptr)) {
                    ae->intersecting_edges.erase(
                        ae->intersecting_edges.begin() + ie
                    );
                } else {
                    ++ie;
                }
            }
        }


        if(!e_ptr->vertexes[0]) continue; //It had been marked for deletion.

        //For every other edge in the map, check for intersections.
        for(size_t e2 = 0; e2 < cur_area_data.edges.size(); ++e2) {
            edge* e2_ptr = cur_area_data.edges[e2];
            if(!e2_ptr->vertexes[0]) {
                //It had been marked for deletion.
                continue;
            }

            //If the edge is actually on the same vertex, never mind.
            if(e_ptr->vertexes[0] == e2_ptr->vertexes[0]) continue;
            if(e_ptr->vertexes[0] == e2_ptr->vertexes[1]) continue;
            if(e_ptr->vertexes[1] == e2_ptr->vertexes[0]) continue;
            if(e_ptr->vertexes[1] == e2_ptr->vertexes[1]) continue;

            if(
                lines_intersect(
                    e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y,
                    e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y,
                    e2_ptr->vertexes[0]->x, e2_ptr->vertexes[0]->y,
                    e2_ptr->vertexes[1]->x, e2_ptr->vertexes[1]->y,
                    NULL, NULL)
            ) {
                if(ae) {
                    ae->intersecting_edges.push_back(
                        edge_intersection(e_ptr, e2_ptr)
                    );
                }
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Cleans a polygon's vertexes.
 * This deletes 0-length edges, and 180-degree vertexes.
 */
void clean_poly(polygon* p) {
    for(size_t v = 0; v < p->size();) {
        bool should_delete = false;
        vertex* prev_v = get_prev_in_vector((*p), v);
        vertex* cur_v =  p->at(v);
        vertex* next_v = get_next_in_vector((*p), v);

        //If the distance between both vertexes is so small
        //that it's basically 0, delete this vertex from the list.
        if(
            fabs(prev_v->x - cur_v->x) < 0.00001 &&
            fabs(prev_v->y - cur_v->y) < 0.00001
        ) {
            should_delete = true;
        }

        //If the angle between this vertex and the next is the same, then
        //this is just a redundant point in the edge prev - next. Delete it.
        if(
            fabs(
                atan2(prev_v->y - cur_v->y, prev_v->x - cur_v->x) -
                atan2(cur_v->y - next_v->y, cur_v->x - next_v->x)
            ) < 0.000001
        ) {
            should_delete = true;
        }

        if(should_delete) {
            p->erase(p->begin() + v);
        } else {
            ++v;
        }
    }
}


/* ----------------------------------------------------------------------------
 * When there are inner polygons, a cut must be made between it and the outer
 * polygon, as to make the outer holeless.
 */
void cut_poly(polygon* outer, vector<polygon>* inners) {
    bool outer_valid = true;
    if(!outer) {
        outer_valid = false;
    } else {
        if(outer->size() < 3) outer_valid = false;
    }

    if(!outer_valid) {
        //Some error happened.
        return;
    }

    vertex* outer_rightmost = get_rightmost_vertex(outer);

    for(size_t i = 0; i < inners->size(); ++i) {
        polygon* p = &inners->at(i);
        vertex* closest_edge_v1 = NULL;
        vertex* closest_edge_v2 = NULL;
        float closest_edge_ur = FLT_MAX;
        vertex* closest_vertex = NULL;
        float closest_vertex_ur = FLT_MAX;
        vertex* best_vertex = NULL;

        //Find the rightmost vertex on this inner.
        vertex* start = get_rightmost_vertex(p);

        if(!start) {
            //Some error occured.
            continue;
        }

        //Imagine a line from this vertex to the right.
        //If any edge of the outer polygon intersects it,
        //we just find the best vertex on that edge, and make the cut.
        //This line stretching right is known as a ray.
        float ray_width = outer_rightmost->x - start->x;

        //Let's also check the vertexes.
        //If the closest thing is a vertex, not an edge, then
        //we can skip a bunch of steps.
        vertex* v1 = NULL, *v2 = NULL;
        for(size_t v = 0; v < outer->size(); ++v) {
            v1 = outer->at(v);
            v2 = get_next_in_vector(*outer, v);
            if(
                (v1->x >= start->x ||
                 v2->x >= start->x) &&
                (v1->x <= outer_rightmost->x ||
                 v2->x <= outer_rightmost->x)
            ) {
                float ur;
                if(
                    lines_intersect(
                        v1->x, v1->y, v2->x, v2->y,
                        start->x, start->y, outer_rightmost->x, start->y,
                        &ur, NULL
                    )
                ) {
                    if(!closest_edge_v1 || ur < closest_edge_ur) {
                        closest_edge_v1 = v1;
                        closest_edge_v2 = v2;
                        closest_edge_ur = ur;
                    }
                }

                if(v1->y == start->y) {
                    float ur = (v1->x - start->x) / ray_width;
                    if(!closest_vertex || ur < closest_vertex_ur) {
                        closest_vertex = v1;
                        closest_vertex_ur = ur;
                    }
                }

            }
        }

        if(!closest_vertex && !closest_edge_v1) {
            //Some error occured.
            continue;
        }

        //Which is closest, a vertex or an edge?
        if(closest_vertex_ur <= closest_edge_ur) {
            //If it's a vertex, done.
            best_vertex = closest_vertex;
        } else {
            if(!closest_edge_v1) continue;

            //If it's an edge, some more complicated steps need to be done.

            //We're on the edge closest to the vertex.
            //Go to the rightmost vertex of this edge.
            vertex* vertex_to_compare =
                get_rightmost_vertex(closest_edge_v1, closest_edge_v2);

            //Now get a list of all vertexes inside the triangle
            //marked by the inner's vertex,
            //the point on the edge,
            //and the vertex we're comparing.
            vector<vertex*> inside_triangle;
            for(size_t v = 0; v < outer->size(); ++v) {
                vertex* v_ptr = outer->at(v);
                if(
                    is_point_in_triangle(
                        v_ptr->x, v_ptr->y,
                        start->x, start->y,
                        start->x + closest_edge_ur * ray_width, start->y,
                        vertex_to_compare->x, vertex_to_compare->y,
                        true) &&
                    v_ptr != vertex_to_compare
                ) {
                    inside_triangle.push_back(v_ptr);
                }
            }

            //Check which one makes the smallest angle compared to 0.
            best_vertex = vertex_to_compare;
            float closest_angle = FLT_MAX;

            for(size_t v = 0; v < inside_triangle.size(); ++v) {
                vertex* v_ptr = inside_triangle[v];
                float angle = atan2(v_ptr->y - start->y, v_ptr->x - start->x);
                if(fabs(angle) < closest_angle) {
                    closest_angle = fabs(angle);
                    best_vertex = v_ptr;
                }
            }
        }

        //This is the final vertex. Make a bridge
        //from the start vertex to this.
        //First, we must find whether the outer vertex
        //already has bridges or not.
        //If so, we place the new bridge before or after,
        //depending on the angle.
        //We know a bridge exists if the same vertex
        //appears twice.
        vector<size_t> bridges;
        for(size_t v = 0; v < outer->size(); ++v) {
            if(outer->at(v) == best_vertex) {
                bridges.push_back(v);
            }
        }

        //Insert the new bridge after this vertex.
        size_t insertion_vertex_nr;
        if(bridges.size() == 1) {
            //No bridges found, just use this vertex.
            insertion_vertex_nr = bridges[0];
        } else {
            //Find where to insert.
            insertion_vertex_nr = bridges.back();
            float new_bridge_angle =
                get_angle_cw_dif(
                    atan2(
                        start->y - best_vertex->y,
                        start->x - best_vertex->x
                    ), 0.0f
                );

            for(size_t v = 0; v < bridges.size(); ++v) {
                vertex* v_ptr = outer->at(bridges[v]);
                vertex* nv_ptr = get_next_in_vector(*outer, bridges[v]);
                float a =
                    get_angle_cw_dif(
                        atan2(nv_ptr->y - v_ptr->y, nv_ptr->x - v_ptr->x),
                        0.0f
                    );
                if(a < new_bridge_angle) {
                    insertion_vertex_nr = bridges[v];
                    break;
                }
            }
        }

        //Now, make the bridge.
        //On the outer vertex, change the next vertex
        //to be the start of the inner, then
        //circle the inner, and go back to the outer vertex.
        //Let's just find where the start vertex is...
        size_t iv = 0;
        for(; iv < p->size(); ++iv) {
            if(p->at(iv) == start) {
                break;
            }
        }

        auto it = p->begin() + iv;
        size_t n_after = p->size() - iv;
        //Finally, make the bridge.
        outer->insert(
            outer->begin() + insertion_vertex_nr + 1, it, p->end()
        );
        outer->insert(
            outer->begin() + insertion_vertex_nr + 1 + n_after, p->begin(), it
        );
        //This last one closes the inner polygon.
        outer->insert(
            outer->begin() + insertion_vertex_nr + 1 + n_after + iv, start
        );

        //Before we close the inner polygon, let's
        //check if the inner's rightmost and the outer best vertexes
        //are not the same.
        //This can happen if you have a square on the top-right
        //and one on the bottom-left, united by the central vertex.
        if(start != best_vertex) {
            outer->insert(
                outer->begin() + insertion_vertex_nr + 1 + n_after + iv + 1,
                best_vertex
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Traverses a graph using the depth first search algorithm.
 * nodes:   Vector of nodes.
 * visited: Set with the visited nodes.
 * start:   Starting node.
 */
void depth_first_search(
    vector<path_stop*> &nodes, unordered_set<path_stop*> &visited,
    path_stop* start
) {
    visited.insert(start);
    unordered_set<path_stop*> links;

    for(size_t l = 0; l < start->links.size(); ++l) {
        links.insert(start->links[l].end_ptr);
    }

    for(size_t n = 0; n < nodes.size(); ++n) {
        path_stop* n_ptr = nodes[n];
        if(n_ptr == start) continue;
        if(visited.find(n_ptr) != visited.end()) continue;
        if(n_ptr->has_link(start)) {
            links.insert(n_ptr);
        }
    }

    for(auto l = links.begin(); l != links.end(); ++l) {
        if(visited.find(*l) != visited.end()) continue;
        depth_first_search(nodes, visited, *l);
    }
}

/* ----------------------------------------------------------------------------
 * Uses Dijstra's algorithm to get the shortest path between two nodes.
 * https://en.wikipedia.org/wiki/Dijkstra's_algorithm
 * *node:          Start and end node.
 * obstacle_found: If the only path has an obstacle, this points to it.
 * total_dist:     If not NULL, place the total path distance here.
 */
vector<path_stop*> dijkstra(
    path_stop* start_node, path_stop* end_node,
    mob** obstacle_found, float* total_dist
) {

    unordered_set<path_stop*> unvisited;
    //Distance from starting node + previous stop on the best solution.
    map<path_stop*, pair<float, path_stop*> > data;
    //Data about any active obstacles we came across.
    vector<pair<path_stop*, mob*> > obstacles_found;
    //If we found an error, set this to true.
    bool got_error = false;

    //Initialize the algorithm.
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];
        unvisited.insert(s_ptr);
        data[s_ptr] = make_pair(FLT_MAX, (path_stop*) NULL);
    }

    //The distance between the start node and the start node is 0.
    data[start_node].first = 0;

    while(!unvisited.empty() && !got_error) {

        //Figure out what node to work on.
        path_stop* shortest_node = NULL;
        float shortest_node_dist = 0;
        pair<float, path_stop*> shortest_node_data;

        for(auto u = unvisited.begin(); u != unvisited.end(); ++u) {
            pair<float, path_stop*> d = data[*u];
            if(!shortest_node || d.first < shortest_node_dist) {
                shortest_node = *u;
                shortest_node_dist = d.first;
                shortest_node_data = d;
            }
        }

        //If we reached the end node, that's it, best path found!
        if(shortest_node == end_node) {

            vector<path_stop*> final_path;
            path_stop* next = data[end_node].second;
            final_path.push_back(end_node);
            float td = data[end_node].first;
            //Construct the path.
            while(next) {
                final_path.insert(final_path.begin(), next);
                next = data[next].second;
            }

            if(final_path.size() < 2) {
                //This can't be right... Something went wrong.
                got_error = true;
                break;
            } else {
                if(obstacle_found) *obstacle_found = NULL;
                if(total_dist) *total_dist = td;
                return final_path;
            }

        }

        //This node's been visited.
        unvisited.erase(unvisited.find(shortest_node));

        //Check the neighbors.
        for(size_t l = 0; l < shortest_node->links.size(); ++l) {
            path_link* l_ptr = &shortest_node->links[l];

            //If this neighbor's been visited, forget it.
            if(unvisited.find(l_ptr->end_ptr) == unvisited.end()) continue;

            //Is this link unobstructed?
            mob* obs = get_path_link_obstacle(shortest_node, l_ptr->end_ptr);
            if(obs) {
                obstacles_found.push_back(make_pair(shortest_node, obs));
                continue;
            }

            float total_dist = shortest_node_data.first + l_ptr->distance;
            auto d = &data[l_ptr->end_ptr];

            if(total_dist < d->first) {
                //Found a shorter path to this node.
                d->first = total_dist;
                d->second = shortest_node;
            }
        }
    }

    //If we got to this point, there means that there is no available path!
    if(!obstacles_found.empty()) {
        //Let's try making a path to the closest obstacle we found,
        //and stay there... The closest obstacle does not necessarily mean
        //the obstacle on the best path; we can't know the best path
        //because we can't reach the finish. By using the closest obstacle,
        //we just hope that that is also the best path.
        path_stop* closest_obstacle_node = NULL;
        mob* closest_obstacle_mob = NULL;
        dist closest_obstacle_d;
        for(size_t o = 0; o < obstacles_found.size(); ++o) {
            dist d(
                start_node->x, start_node->y,
                obstacles_found[o].first->x,
                obstacles_found[o].first->y
            );
            if(d < closest_obstacle_d || !closest_obstacle_node) {
                closest_obstacle_d = d;
                closest_obstacle_node = obstacles_found[o].first;
                closest_obstacle_mob = obstacles_found[o].second;
            }
        }

        vector<path_stop*> final_path;
        final_path.push_back(closest_obstacle_node);
        path_stop* next = data[closest_obstacle_node].second;
        float td = data[closest_obstacle_node].first;
        while(next) {
            final_path.insert(final_path.begin(), next);
            next = data[next].second;
        }

        if(obstacle_found) *obstacle_found = closest_obstacle_mob;
        if(total_dist) *total_dist = td;
        return final_path;

    } else {
        //No obstacle?... Something really went wrong. No path.
        if(total_dist) *total_dist = 0;
        return vector<path_stop*>();
    }
}


/* ----------------------------------------------------------------------------
 * Returns the clockwise distance between a1 and a2, in radians.
 */
float get_angle_cw_dif(float a1, float a2) {
    a1 = normalize_angle(a1);
    a2 = normalize_angle(a2);
    if(a1 > a2) a1 -= M_PI * 2;
    return a2 - a1;
}


/* ----------------------------------------------------------------------------
 * Returns the smallest distance between two angles.
 */
float get_angle_smallest_dif(float a1, float a2) {
    a1 = normalize_angle(a1);
    a2 = normalize_angle(a2);
    return M_PI - abs(abs(a1 - a2) - M_PI);
}


/* ----------------------------------------------------------------------------
 * Get the convex, concave and ear vertexes.
 */
void get_cce(
    vector<vertex*> &vertexes_left, vector<size_t> &ears,
    vector<size_t> &convex_vertexes, vector<size_t> &concave_vertexes
) {
    ears.clear();
    convex_vertexes.clear();
    concave_vertexes.clear();
    for(size_t v = 0; v < vertexes_left.size(); ++v) {
        bool is_convex = is_vertex_convex(vertexes_left, v);
        if(is_convex) {
            convex_vertexes.push_back(v);

        } else {
            concave_vertexes.push_back(v);
        }
    }

    for(size_t c = 0; c < convex_vertexes.size(); ++c) {
        if(is_vertex_ear(vertexes_left, concave_vertexes, convex_vertexes[c])) {
            ears.push_back(convex_vertexes[c]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether the two line segments intersect.
 * ur: Returns the distance from the start of line 2 in which
   * the intersection happens.
   * This is a ratio, so 0 is the start, 1 is the end of the line.
   * Oh, and the r stands for ray.
 * ul: Same as ur, but for line 1.
 */
bool lines_intersect(
    float l1x1, float l1y1, float l1x2, float l1y2,
    float l2x1, float l2y1, float l2x2, float l2y2,
    float* ur, float* ul
) {

    float div = (l2y2 - l2y1) * (l1x2 - l1x1) - (l2x2 - l2x1) * (l1y2 - l1y1);

    if(div != 0) {

        float local_ul, local_ur;

        //Calculate the intersection distance from the line.
        local_ul =
            ((l2x2 - l2x1) * (l1y1 - l2y1) - (l2y2 - l2y1) * (l1x1 - l2x1)) /
            div;
        if(ul) *ul = local_ul;

        //Calculate the intersection distance from the ray.
        local_ur =
            ((l1x2 - l1x1) * (l1y1 - l2y1) - (l1y2 - l1y1) * (l1x1 - l2x1)) /
            div;
        if(ur) *ur = local_ur;

        //Return whether they intersect.
        return
            (local_ur >= 0) && (local_ur <= 1) &&
            (local_ul >= 0) && (local_ul <= 1);

    } else {
        //No intersection.
        return false;
    }
}


/* ----------------------------------------------------------------------------
 * Triangulates (turns into triangles) a sector.
 * This is because drawing concave polygons is not possible.
 */
void triangulate(sector* s_ptr) {

    area_editor* ae = NULL;
    if(cur_game_state_nr == GAME_STATE_AREA_EDITOR) {
        ae = (area_editor*) game_states[cur_game_state_nr];
    }

    //We'll triangulate with the Triangulation by Ear Clipping algorithm.
    //http://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf

    polygon outer_poly;
    vector<polygon> inner_polys;

    //Before we start, let's just remove it from the
    //vector of non-simple sectors.
    if(ae) {
        auto it = ae->non_simples.find(s_ptr);
        if(it != ae->non_simples.end()) {
            ae->non_simples.erase(it);
        }
    }

    //And let's clear any "lone" edges here.
    if(ae) {
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            edge* e_ptr = s_ptr->edges[e];
            auto it = ae->lone_edges.find(e_ptr);
            if(it != ae->lone_edges.end()) {
                ae->lone_edges.erase(it);
            }
        }
    }

    //First, we need to know what vertexes mark the outermost polygon,
    //and what vertexes mark the inner ones.
    //There can be no islands or polygons of our sector inside the inner ones.
    //Example of a sector's polygons:
    /*
     * +-------+     +---+
     * | OUTER  \    |   |
     * |         +---+   |
     * |   +----+        |
     * |  /INNER|   +--+ |
     * | +------+   |  | |
     * +---+    +---+  | |
     *     |   /INNER  | |
     *     |  /        | |
     *     | +---------+ |
     *     +-------------+
     */
    get_polys(s_ptr, &outer_poly, &inner_polys);

    //Get rid of 0-length vertexes and 180-degree vertexes,
    //as they're redundant.
    clean_poly(&outer_poly);
    for(size_t i = 0; i < inner_polys.size(); ++i) clean_poly(&inner_polys[i]);

    //Make cuts on the outer polygon between where it and inner polygons exist,
    //as to make it holeless.
    cut_poly(&outer_poly, &inner_polys);

    s_ptr->triangles.clear();
    vector<vertex*> vertexes_left = outer_poly;
    vector<size_t> ears;
    vector<size_t> convex_vertexes;
    vector<size_t> concave_vertexes;

    //Begin by making a list of all concave, convex and ear vertexes.
    get_cce(vertexes_left, ears, convex_vertexes, concave_vertexes);

    //We do a triangulation until we're left
    //with three vertexes -- the final triangle.
    while(vertexes_left.size() > 3) {

        if(ears.empty()) {
            //Something went wrong, the polygon mightn't be simple.
            if(ae) {
                ae->non_simples.insert(s_ptr);
            }
            break;

        } else {
            //The ear, the previous and the next vertexes make a triangle.
            s_ptr->triangles.push_back(
                triangle(
                    vertexes_left[ears[0]],
                    get_prev_in_vector(vertexes_left, ears[0]),
                    get_next_in_vector(vertexes_left, ears[0])
                )
            );

            //Remove the ear.
            vertexes_left.erase(vertexes_left.begin() + ears[0]);

            //Recalculate the ears, concave and convex vertexes.
            get_cce(vertexes_left, ears, convex_vertexes, concave_vertexes);
        }
    }

    //Finally, add the final triangle.
    if(vertexes_left.size() == 3) {
        s_ptr->triangles.push_back(
            triangle(
                vertexes_left[1], vertexes_left[0], vertexes_left[2]
            )
        );
    }
}
