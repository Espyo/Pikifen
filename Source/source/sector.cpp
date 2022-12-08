/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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

#include "sector.h"

#include "functions.h"
#include "game.h"
#include "utils/geometry_utils.h"
#include "utils/string_utils.h"


using std::unordered_set;
using std::set;


namespace GEOMETRY {
//Area blockmap blocks have this width and height.
const float BLOCKMAP_BLOCK_SIZE = 128;
//Default sector brightness.
const unsigned char DEF_SECTOR_BRIGHTNESS = 255;
//Liquids drain for this long.
const float LIQUID_DRAIN_DURATION = 2.0f;
//Auto wall shadow lengths are the sector height difference multiplied by this.
const float SHADOW_AUTO_LENGTH_MULT = 0.2f;
//Default color of wall shadows. This is the color at the edge.
const ALLEGRO_COLOR SHADOW_DEF_COLOR = {0.0f, 0.0f, 0.0f, 0.90f};
//Maximum length a wall shadow can be when the length is automatic.
const float SHADOW_MAX_AUTO_LENGTH = 50.0f;
//Maximum length a wall shadow can be.
const float SHADOW_MAX_LENGTH = 100.0f;
//Minimum length a wall shadow can be when the length is automatic.
const float SHADOW_MIN_AUTO_LENGTH = 8.0f;
//Minimum length a wall shadow can be.
const float SHADOW_MIN_LENGTH = 1.0f;
//Default color of the smoothing effect.
const ALLEGRO_COLOR SMOOTHING_DEF_COLOR = {0.0f, 0.0f, 0.0f, 0.70f};
//Maximum length the smoothing effect can be.
const float SMOOTHING_MAX_LENGTH = 100.0f;
//Mobs can walk up sectors that are, at the most,
//this high from the current one, as if climbing up steps.
const float STEP_HEIGHT = 50;
}


/* ----------------------------------------------------------------------------
 * Creates a blockmap.
 */
blockmap::blockmap() :
    n_cols(0),
    n_rows(0) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the info of the blockmap.
 */
void blockmap::clear() {
    top_left_corner = point();
    edges.clear();
    sectors.clear();
    n_cols = 0;
    n_rows = 0;
}


/* ----------------------------------------------------------------------------
 * Returns the block column in which an X coordinate is contained.
 * Returns INVALID on error.
 * x:
 *   X coordinate.
 */
size_t blockmap::get_col(const float x) const {
    if(x < top_left_corner.x) return INVALID;
    float final_x = (x - top_left_corner.x) / GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    if(final_x >= n_cols) return INVALID;
    return final_x;
}


/* ----------------------------------------------------------------------------
 * Obtains a list of edges that are within the specified rectangular region.
 * Returns true on success, false on error.
 * tl:
 *   Top-left coordinates of the region.
 * br:
 *   Bottom-right coordinates of the region.
 * edges:
 *   Set to fill the edges into.
 */
bool blockmap::get_edges_in_region(
    const point &tl, const point &br, set<edge*> &edges
) const {

    size_t bx1 = game.cur_area_data.bmap.get_col(tl.x);
    size_t bx2 = game.cur_area_data.bmap.get_col(br.x);
    size_t by1 = game.cur_area_data.bmap.get_row(tl.y);
    size_t by2 = game.cur_area_data.bmap.get_row(br.y);
    
    if(
        bx1 == INVALID || bx2 == INVALID ||
        by1 == INVALID || by2 == INVALID
    ) {
        //Out of bounds.
        return false;
    }
    
    for(size_t bx = bx1; bx <= bx2; ++bx) {
        for(size_t by = by1; by <= by2; ++by) {
        
            vector<edge*> &block_edges =
                game.cur_area_data.bmap.edges[bx][by];
                
            for(size_t e = 0; e < block_edges.size(); ++e) {
                edges.insert(block_edges[e]);
            }
        }
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns the block row in which a Y coordinate is contained.
 * Returns INVALID on error.
 * y:
 *   Y coordinate.
 */
size_t blockmap::get_row(const float y) const {
    if(y < top_left_corner.y) return INVALID;
    float final_y = (y - top_left_corner.y) / GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    if(final_y >= n_rows) return INVALID;
    return final_y;
}


/* ----------------------------------------------------------------------------
 * Returns the top-left coordinates for the specified column and row.
 * col:
 *   Column to check.
 * row:
 *   Row to check.
 */
point blockmap::get_top_left_corner(const size_t col, const size_t row) const {
    return
        point(
            col * GEOMETRY::BLOCKMAP_BLOCK_SIZE + top_left_corner.x,
            row * GEOMETRY::BLOCKMAP_BLOCK_SIZE + top_left_corner.y
        );
}


/* ----------------------------------------------------------------------------
 * Creates an edge.
 * v1:
 *   Its first vertex.
 * v2:
 *   Its second vertex.
 */
edge::edge(size_t v1, size_t v2) :
    wall_shadow_length(LARGE_FLOAT),
    wall_shadow_color(GEOMETRY::SHADOW_DEF_COLOR),
    ledge_smoothing_length(0),
    ledge_smoothing_color(GEOMETRY::SMOOTHING_DEF_COLOR) {
    
    vertexes[0] = vertexes[1] = NULL;
    sectors[0] = sectors[1] = NULL;
    sector_nrs[0] = sector_nrs[1] = INVALID;
    
    vertex_nrs[0] = v1; vertex_nrs[1] = v2;
}


/* ----------------------------------------------------------------------------
 * Clones an edge's properties onto another,
 * not counting the sectors or vertexes.
 * new_edge:
 *   Edge to clone the data into.
 */
void edge::clone(edge* new_edge) const {
    new_edge->wall_shadow_length = wall_shadow_length;
    new_edge->wall_shadow_color = wall_shadow_color;
    new_edge->ledge_smoothing_length = ledge_smoothing_length;
    new_edge->ledge_smoothing_color = ledge_smoothing_color;
}


/* ----------------------------------------------------------------------------
 * Returns the sector that ISN'T the specified one.
 * s_ptr:
 *   The sector that ISN'T the one to return.
 */
sector* edge::get_other_sector(const sector* s_ptr) const {
    if(sectors[0] == s_ptr) return sectors[1];
    return sectors[0];
}


/* ----------------------------------------------------------------------------
 * Returns the vertex that ISN'T the specified one.
 * v_ptr:
 *   The vertex that ISN'T the one to return.
 */
vertex* edge::get_other_vertex(const vertex* v_ptr) const {
    if(vertexes[0] == v_ptr) return vertexes[1];
    return vertexes[0];
}


/* ----------------------------------------------------------------------------
 * Returns which side has the specified sector, or INVALID if neither.
 * s_ptr:
 *   Sector to check.
 */
size_t edge::get_side_with_sector(sector* s_ptr) const {
    for(unsigned char s = 0; s < 2; ++s) {
        if(sectors[s] == s_ptr) return s;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * If the specified edge and this one are not neighbors, returns NULL.
 * Otherwise, returns the vertex that binds them.
 * other:
 *   Edge to check as a neighbor.
 */
vertex* edge::has_neighbor(edge* other) const {
    for(size_t v1 = 0; v1 < 2; ++v1) {
        for(size_t v2 = 0; v2 < 2; ++v2) {
            if(vertexes[v1] == other->vertexes[v2]) {
                return vertexes[v1];
            }
        }
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not an edge is valid.
 * An edge is valid if it has non-NULL vertexes.
 */
bool edge::is_valid() const {
    if(!vertexes[0]) return false;
    if(!vertexes[1]) return false;
    return true;
}


/* ----------------------------------------------------------------------------
 * Removes the edge from its sectors, but doesn't mark
 * the sectors as "none".
 * Returns the edge number.
 */
size_t edge::remove_from_sectors() {
    size_t e_nr = INVALID;
    for(unsigned char s = 0; s < 2; ++s) {
        sector* s_ptr = sectors[s];
        if(!s_ptr) continue;
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            edge* e_ptr = s_ptr->edges[e];
            if(e_ptr == this) {
                s_ptr->edges.erase(s_ptr->edges.begin() + e);
                auto nr_it = s_ptr->edge_nrs.begin() + e;
                e_nr = *nr_it;
                s_ptr->edge_nrs.erase(nr_it);
                break;
            }
        }
        sectors[s] = NULL;
        sector_nrs[s] = INVALID;
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
        vertex* v_ptr = vertexes[v];
        if(!v_ptr) continue;
        for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
            edge* e_ptr = v_ptr->edges[e];
            if(e_ptr == this) {
                v_ptr->edges.erase(v_ptr->edges.begin() + e);
                auto nr_it = v_ptr->edge_nrs.begin() + e;
                e_nr = *nr_it;
                v_ptr->edge_nrs.erase(nr_it);
                break;
            }
        }
        vertexes[v] = NULL;
        vertex_nrs[v] = INVALID;
    }
    return e_nr;
}


/* ----------------------------------------------------------------------------
 * Swaps the two vertexes of the edge around. It also swaps the sectors,
 * so that they still point in the right direction.
 */
void edge::swap_vertexes() {
    std::swap(vertexes[0], vertexes[1]);
    std::swap(vertex_nrs[0], vertex_nrs[1]);
    std::swap(sectors[0], sectors[1]);
    std::swap(sector_nrs[0], sector_nrs[1]);
}


/* ----------------------------------------------------------------------------
 * Transfers this edge from one sector to a different one.
 * from:
 *   Sector to transfer from.
 * to:
 *   Sector to transfer to.
 * to_nr:
 *   Number of the sector to transfer to.
 * edge_nr:
 *   Number of the current edge.
 */
void edge::transfer_sector(
    sector* from, sector* to, const size_t to_nr, const size_t edge_nr
) {
    size_t index = get_side_with_sector(from);
    engine_assert(
        index != INVALID,
        i2s(to_nr)
    );
    
    sectors[index] = to;
    sector_nrs[index] = to_nr;
    
    if(from) from->remove_edge(this);
    if(to) to->add_edge(this, edge_nr);
}


/* ----------------------------------------------------------------------------
 * Creates an edge intersection info structure.
 * e1:
 *   First edge in the intersection.
 * e2:
 *   Second edge in the intersection.
 */
edge_intersection::edge_intersection(edge* e1, edge* e2) :
    e1(e1),
    e2(e2) {
    
}


/* ----------------------------------------------------------------------------
 * Checks whether the edge intersection contains the specified edge.
 * e:
 *   Edge to check.
 */
bool edge_intersection::contains(edge* e) {
    return e1 == e || e2 == e;
}


/* ----------------------------------------------------------------------------
 * Creates a mob generation structure.
 * pos:
 *   Coordinates.
 * type:
 *   The mob type.
 * angle:
 *   Angle it is facing.
 * vars:
 *   String representation of the script vars.
 */
mob_gen::mob_gen(
    const point &pos, mob_type* type, const float angle, const string &vars
) :
    type(type),
    pos(pos),
    angle(angle),
    vars(vars) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an empty polygon.
 */
polygon::polygon() {
}


/* ----------------------------------------------------------------------------
 * Creates a polygon with the specified list of vertexes.
 * vertexes:
 *   Vertexes that make up the polygon.
 */
polygon::polygon(const vector<vertex*> &vertexes) :
    vertexes(vertexes) {
}


/* ----------------------------------------------------------------------------
 * Cleans a polygon's vertexes.
 * This deletes 0-length edges, and 180-degree vertexes.
 */
void polygon::clean() {
    for(size_t v = 0; v < vertexes.size();) {
        bool should_delete = false;
        vertex* prev_v = get_prev_in_vector(vertexes, v);
        vertex* cur_v = vertexes[v];
        vertex* next_v = get_next_in_vector(vertexes, v);
        
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
                get_angle(
                    point(cur_v->x, cur_v->y),
                    point(prev_v->x, prev_v->y)
                ) -
                get_angle(
                    point(next_v->x, next_v->y),
                    point(cur_v->x, cur_v->y)
                )
            ) < 0.000001
        ) {
            should_delete = true;
        }
        
        if(should_delete) {
            vertexes.erase(vertexes.begin() + v);
        } else {
            ++v;
        }
    }
}


/* ----------------------------------------------------------------------------
 * When this polygon has inner polygons, a cut must be made between it
 * and the inner polygons, as to make this one holeless.
 * inners:
 *   List of inner polygons.
 */
void polygon::cut(vector<polygon>* inners) {
    if(vertexes.size() < 3) {
        //Some error happened.
        return;
    }
    
    vertex* rightmost = get_rightmost_vertex();
    
    for(size_t i = 0; i < inners->size(); ++i) {
        polygon* inner_p = &(inners->at(i));
        vertex* closest_edge_v1 = NULL;
        vertex* closest_edge_v2 = NULL;
        float closest_edge_r = FLT_MAX;
        vertex* closest_vertex = NULL;
        float closest_vertex_r = FLT_MAX;
        vertex* best_vertex = NULL;
        
        //Find the rightmost vertex on this inner.
        vertex* start = inner_p->get_rightmost_vertex();
        
        if(!start) {
            //Some error occured.
            continue;
        }
        
        //Imagine a line from this vertex to the right.
        //If any edge of the outer polygon intersects it,
        //we just find the best vertex on that edge, and make the cut.
        //This line stretching right is known as a ray.
        float ray_width = rightmost->x - start->x;
        
        //Let's also check the vertexes.
        //If the closest thing is a vertex, not an edge, then
        //we can skip a bunch of steps.
        vertex* v1 = NULL, *v2 = NULL;
        for(size_t v = 0; v < vertexes.size(); ++v) {
            v1 = vertexes[v];
            v2 = get_next_in_vector(vertexes, v);
            if(
                (v1->x >= start->x ||
                 v2->x >= start->x) &&
                (v1->x <= rightmost->x ||
                 v2->x <= rightmost->x)
            ) {
                float r;
                if(
                    line_segs_intersect(
                        point(v1->x, v1->y), point(v2->x, v2->y),
                        point(start->x, start->y),
                        point(rightmost->x, start->y),
                        NULL, &r
                    )
                ) {
                    if(!closest_edge_v1 || r < closest_edge_r) {
                        closest_edge_v1 = v1;
                        closest_edge_v2 = v2;
                        closest_edge_r = r;
                    }
                }
                
                if(v1->y == start->y && v1->x >= start->x) {
                    r = (v1->x - start->x) / ray_width;
                    if(!closest_vertex || r < closest_vertex_r) {
                        closest_vertex = v1;
                        closest_vertex_r = r;
                    }
                }
                
            }
        }
        
        if(!closest_vertex && !closest_edge_v1) {
            //Some error occured.
            continue;
        }
        
        //Which is closest, a vertex or an edge?
        if(closest_vertex_r <= closest_edge_r) {
            //If it's a vertex, done.
            best_vertex = closest_vertex;
        } else {
            if(!closest_edge_v1) continue;
            
            //If it's an edge, some more complicated steps need to be done.
            
            //We're on the edge closest to the vertex.
            //Go to the rightmost vertex of this edge.
            vertex* vertex_to_compare =
                ::get_rightmost_vertex(closest_edge_v1, closest_edge_v2);
                
            //Now get a list of all vertexes inside the triangle
            //marked by the inner's vertex,
            //the point on the edge,
            //and the vertex we're comparing.
            vector<vertex*> inside_triangle;
            for(size_t v = 0; v < vertexes.size(); ++v) {
                vertex* v_ptr = vertexes[v];
                if(
                    is_point_in_triangle(
                        point(v_ptr->x, v_ptr->y),
                        point(start->x, start->y),
                        point(start->x + closest_edge_r * ray_width, start->y),
                        point(vertex_to_compare->x, vertex_to_compare->y),
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
                float angle =
                    get_angle(
                        point(start->x, start->y),
                        point(v_ptr->x, v_ptr->y)
                    );
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
        for(size_t v = 0; v < vertexes.size(); ++v) {
            if(vertexes[v] == best_vertex) {
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
                    get_angle(
                        point(best_vertex->x, best_vertex->y),
                        point(start->x, start->y)
                    ),
                    0.0f
                );
                
            for(size_t v = 0; v < bridges.size(); ++v) {
                vertex* v_ptr = vertexes[bridges[v]];
                vertex* nv_ptr = get_next_in_vector(vertexes, bridges[v]);
                float a =
                    get_angle_cw_dif(
                        get_angle(
                            point(v_ptr->x, v_ptr->y),
                            point(nv_ptr->x, nv_ptr->y)
                        ),
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
        for(; iv < inner_p->vertexes.size(); ++iv) {
            if(inner_p->vertexes[iv] == start) {
                break;
            }
        }
        
        auto it = inner_p->vertexes.begin() + iv;
        size_t n_after = inner_p->vertexes.size() - iv;
        //Finally, make the bridge.
        vertexes.insert(
            vertexes.begin() + insertion_vertex_nr + 1,
            it, inner_p->vertexes.end()
        );
        vertexes.insert(
            vertexes.begin() + insertion_vertex_nr + 1 + n_after,
            inner_p->vertexes.begin(), it
        );
        //This last one closes the inner polygon.
        vertexes.insert(
            vertexes.begin() + insertion_vertex_nr + 1 + n_after + iv,
            start
        );
        
        //Before we close the inner polygon, let's
        //check if the inner's rightmost and the outer best vertexes
        //are not the same.
        //This can happen if you have a square on the top-right
        //and one on the bottom-left, united by the central vertex.
        if(start != best_vertex) {
            vertexes.insert(
                vertexes.begin() + insertion_vertex_nr + 1 + n_after + iv + 1,
                best_vertex
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right in a polygon.
 */
vertex* polygon::get_rightmost_vertex() const {
    vertex* rightmost = NULL;
    
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        if(!rightmost) {
            rightmost = v_ptr;
        } else {
            rightmost = ::get_rightmost_vertex(v_ptr, rightmost);
        }
    }
    
    return rightmost;
}


/* ----------------------------------------------------------------------------
 * Creates a sector.
 */
sector::sector() :
    type(SECTOR_TYPE_NORMAL),
    is_bottomless_pit(false),
    z(0),
    brightness(GEOMETRY::DEF_SECTOR_BRIGHTNESS),
    fade(false),
    hazard_floor(true),
    liquid_drain_left(0),
    draining_liquid(false) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a sector.
 */
sector::~sector() {
    for(size_t t = 0; t < 2; ++t) {
        if(texture_info.bitmap && texture_info.bitmap != game.bmp_error) {
            game.bitmaps.detach(texture_info.file_name);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Adds an edge to the sector's list of edges, if it's not there already.
 * e_ptr:
 *   Edge to add.
 * e_nr:
 *   Index number of the edge to add.
 */
void sector::add_edge(edge* e_ptr, const size_t e_nr) {
    for(size_t i = 0; i < edges.size(); ++i) {
        if(edges[i] == e_ptr) {
            return;
        }
    }
    edges.push_back(e_ptr);
    edge_nrs.push_back(e_nr);
}


/* ----------------------------------------------------------------------------
 * Calculates the bounding box coordinates and saves them
 * in the object's bbox variables.
 */
void sector::calculate_bounding_box() {
    if(edges.empty()) {
        //Unused sector... This shouldn't exist.
        bbox[0] = point();
        bbox[1] = point();
        return;
    }
    
    bbox[0].x = edges[0]->vertexes[0]->x;
    bbox[1].x = bbox[0].x;
    bbox[0].y = edges[0]->vertexes[0]->y;
    bbox[1].y = bbox[0].y;
    
    for(size_t e = 0; e < edges.size(); ++e) {
        for(unsigned char v = 0; v < 2; ++v) {
            point coords(
                edges[e]->vertexes[v]->x,
                edges[e]->vertexes[v]->y
            );
            
            bbox[0].x = std::min(bbox[0].x, coords.x);
            bbox[1].x = std::max(bbox[1].x, coords.x);
            bbox[0].y = std::min(bbox[0].y, coords.y);
            bbox[1].y = std::max(bbox[1].y, coords.y);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Clones a sector's properties onto another,
 * not counting the list of edges, bounding box, or bitmap
 * (the file name is cloned too, though).
 * new_sector:
 *   Sector to clone the data into.
 */
void sector::clone(sector* new_sector) {
    new_sector->type = type;
    new_sector->is_bottomless_pit = is_bottomless_pit;
    new_sector->z = z;
    new_sector->tag = tag;
    new_sector->hazard_floor = hazard_floor;
    new_sector->hazards_str = hazards_str;
    new_sector->brightness = brightness;
    new_sector->texture_info.scale = texture_info.scale;
    new_sector->texture_info.translation = texture_info.translation;
    new_sector->texture_info.rot = texture_info.rot;
    new_sector->texture_info.tint = texture_info.tint;
    new_sector->fade = fade;
}


/* ----------------------------------------------------------------------------
 * Fills a vector with neighboring sectors, recursively, but only if they
 * meet certain criteria.
 * condition:
 *   Function that accepts a sector and checks its criteria. This
 *   function must return true if accepted, false if not.
 * sector_list:
 *   List of sectors to be filled. Also doubles as the list of visited sectors.
 */
void sector::get_neighbor_sectors_conditionally(
    const std::function<bool(sector* s_ptr)> &condition,
    vector<sector*> &sector_list
) {

    //If this sector is already on the list, skip.
    for(size_t s = 0; s < sector_list.size(); ++s) {
        if(sector_list[s] == this) return;
    }
    
    //If this sector is not eligible, return.
    if(!condition(this)) return;
    
    //This sector is valid!
    sector_list.push_back(this);
    
    //Now check its neighbors.
    edge* e_ptr = NULL;
    sector* other_s = NULL;
    for(size_t e = 0; e < edges.size(); ++e) {
        e_ptr = edges[e];
        other_s = e_ptr->get_other_sector(this);
        if(!other_s) continue;
        
        other_s->get_neighbor_sectors_conditionally(condition, sector_list);
    }
}


/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right in a sector.
 */
vertex* sector::get_rightmost_vertex() const {
    vertex* rightmost = NULL;
    
    for(size_t e = 0; e < edges.size(); ++e) {
        edge* e_ptr = edges[e];
        if(!rightmost) rightmost = e_ptr->vertexes[0];
        else {
            rightmost = ::get_rightmost_vertex(e_ptr->vertexes[0], rightmost);
            rightmost = ::get_rightmost_vertex(e_ptr->vertexes[1], rightmost);
        }
    }
    
    return rightmost;
}


/* ----------------------------------------------------------------------------
 * If texture merging is required, this returns what two neighboring sectors
 * will be used for it.
 * s1:
 *   Receives the first sector.
 * s2:
 *   Receives the second sector.
 */
void sector::get_texture_merge_sectors(sector** s1, sector** s2) const {
    //Check all edges to find which two textures need merging.
    edge* e_ptr = NULL;
    sector* neighbor = NULL;
    map<sector*, dist> neighbors;
    sector* texture_sector[2] = {NULL, NULL};
    
    //The two neighboring sectors with the lenghtiest edges are picked.
    //So save all sector/length pairs.
    //Sectors with different heights from the current one are also saved,
    //but they have lower priority compared to same-heigh sectors.
    for(size_t e = 0; e < edges.size(); ++e) {
        e_ptr = edges[e];
        bool valid = true;
        
        neighbor = e_ptr->get_other_sector(this);
        
        if(neighbor) {
            if(neighbor->fade) valid = false;
        }
        
        if(valid) {
            neighbors[neighbor] +=
                dist(
                    point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
                    point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y)
                );
        }
    }
    
    //Find the two lengthiest ones.
    vector<std::pair<dist, sector*> > neighbors_vec;
    for(auto &n : neighbors) {
        neighbors_vec.push_back(
            std::make_pair(
                //Yes, we do need these casts, for g++.
                (dist) (n.second), (sector*) (n.first)
            )
        );
    }
    sort(
        neighbors_vec.begin(), neighbors_vec.end(),
    [this] (std::pair<dist, sector*> p1, std::pair<dist, sector*> p2) -> bool {
        return p1.first < p2.first;
    }
    );
    if(neighbors_vec.size() >= 1) {
        texture_sector[0] = neighbors_vec.back().second;
    }
    if(neighbors_vec.size() >= 2) {
        texture_sector[1] = neighbors_vec[neighbors_vec.size() - 2].second;
    }
    
    if(!texture_sector[1] && texture_sector[0]) {
        //0 is always the bottom one. If we're fading into nothingness,
        //we should swap first.
        std::swap(texture_sector[0], texture_sector[1]);
    } else if(!texture_sector[1]) {
        //Nothing to draw.
        return;
    } else if(texture_sector[1]->is_bottomless_pit) {
        std::swap(texture_sector[0], texture_sector[1]);
    }
    
    *s1 = texture_sector[0];
    *s2 = texture_sector[1];
}


/* ----------------------------------------------------------------------------
 * Returns whether a sector's vertexes are ordered clockwise or not.
 */
bool sector::is_clockwise() const {
    vector<vertex*> vertexes;
    for(size_t e = 0; e < edges.size(); ++e) {
        vertexes.push_back(edges[e]->vertexes[0]);
    }
    return is_polygon_clockwise(vertexes);
}


/* ----------------------------------------------------------------------------
 * Returns whether a point is inside a sector by checking its triangles.
 * p:
 *   Point to check.
 */
bool sector::is_point_in_sector(const point &p) const {
    for(size_t t = 0; t < triangles.size(); ++t) {
        const triangle* t_ptr = &triangles[t];
        if(
            is_point_in_triangle(
                p,
                point(t_ptr->points[0]->x, t_ptr->points[0]->y),
                point(t_ptr->points[1]->x, t_ptr->points[1]->y),
                point(t_ptr->points[2]->x, t_ptr->points[2]->y),
                false
            )
        ) {
            return true;
        }
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Removes an edge from a sector's list of edges, if it is there.
 * e_ptr:
 *   Edge to remove.
 */
void sector::remove_edge(edge* e_ptr) {
    size_t i = 0;
    for(; i < edges.size(); ++i) {
        if(edges[i] == e_ptr) {
            edges.erase(edges.begin() + i);
            edge_nrs.erase(edge_nrs.begin() + i);
            return;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a sector texture's info struct.
 */
sector_texture_info::sector_texture_info() :
    scale(1.0, 1.0),
    rot(0),
    bitmap(nullptr),
    tint(COLOR_WHITE) {
}



/* ----------------------------------------------------------------------------
 * Creates a tree shadow.
 * center:
 *   Center coordinates.
 * size:
 *   Width and height.
 * angle:
 *   Angle it is rotated by.
 * alpha:
 *   How opaque it is [0-255].
 * file_name:
 *   Name of the file with the tree shadow's texture.
 * sway:
 *   Multiply the sway distance by this much, horizontally and vertically.
 */
tree_shadow::tree_shadow(
    const point &center, const point &size, const float angle,
    const unsigned char alpha, const string &file_name, const point &sway
) :
    file_name(file_name),
    bitmap(nullptr),
    center(center),
    size(size),
    angle(angle),
    alpha(alpha),
    sway(sway) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a tree shadow.
 */
tree_shadow::~tree_shadow() {
    game.textures.detach(file_name);
}


/* ----------------------------------------------------------------------------
 * Creates a triangle.
 * v1:
 *   First vertex.
 * v2:
 *   Second vertex.
 * v3:
 *   Third vertex.
 */
triangle::triangle(vertex* v1, vertex* v2, vertex* v3) {
    points[0] = v1;
    points[1] = v2;
    points[2] = v3;
}


/* ----------------------------------------------------------------------------
 * Creates a vertex.
 * x:
 *   X coordinate.
 * y:
 *   Y coordinate.
 */
vertex::vertex(float x, float y) :
    x(x),
    y(y) {
    
}


/* ----------------------------------------------------------------------------
 * Adds an edge to the vertex's list of edges, if it's not there already.
 * e_ptr:
 *   Edge to add.
 * e_nr:
 *   Index number of the edge to add.
 */
void vertex::add_edge(edge* e_ptr, const size_t e_nr) {
    for(size_t i = 0; i < edges.size(); ++i) {
        if(edges[i] == e_ptr) {
            return;
        }
    }
    edges.push_back(e_ptr);
    edge_nrs.push_back(e_nr);
}


/* ----------------------------------------------------------------------------
 * Returns the edge that has the specified vertex as a neighbor of this vertex.
 * Returns NULL if not found.
 * neighbor:
 *   The neighbor vertex to check.
 */
edge* vertex::get_edge_by_neighbor(vertex* neighbor) const {
    for(size_t e = 0; e < edges.size(); ++e) {
        if(edges[e]->get_other_vertex(this) == neighbor) {
            return edges[e];
        }
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not this vertex has the specified edge in its list.
 * e_ptr:
 *   Edge to check.
 */
bool vertex::has_edge(edge* e_ptr) const {
    for(size_t e = 0; e < edges.size(); ++e) {
        if(edges[e] == e_ptr) return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not this vertex is a second-degree neighbor to the
 * specified vertex. i.e. they have a shared neighbor between them.
 * other_v:
 *   The vertex to compare against.
 * first_neighbor:
 *   Return the common neighbor between them here, if the result is true.
 */
bool vertex::is_2nd_degree_neighbor(
    vertex* other_v, vertex** first_neighbor
) const {
    //Let's crawl forward through all edges and stop at the second level.
    //If other_v is at that distance, then we found it!
    
    for(size_t e1 = 0; e1 < edges.size(); ++e1) {
        vertex* next_v = edges[e1]->get_other_vertex(this);
        
        for(size_t e2 = 0; e2 < next_v->edges.size(); ++e2) {
            if(next_v->edges[e2]->get_other_vertex(next_v) == other_v) {
                *first_neighbor = next_v;
                return true;
            }
        }
        
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not this vertex is a second-degree neighbor to the
 * specified edge. i.e. one of the vertex's neighbors is used by the edge.
 * other_e:
 *   The edge to compare against.
 * first_neighbor:
 *   Return the common neighbor between them here, if the result is true.
 */
bool vertex::is_2nd_degree_neighbor(
    edge* other_e, vertex** first_neighbor
) const {
    //Let's crawl forward through all edges and stop at the second level.
    //If other_e is at that distance, then we found it!
    
    for(size_t e1 = 0; e1 < edges.size(); ++e1) {
        vertex* next_v = edges[e1]->get_other_vertex(this);
        
        for(size_t e2 = 0; e2 < next_v->edges.size(); ++e2) {
            if(next_v->edges[e2] == other_e) {
                *first_neighbor = next_v;
                return true;
            }
        }
        
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not this vertex is a neighbor to the
 * specified vertex. i.e. they have a shared edge between them.
 * other_v:
 *   The vertex to compare against.
 */
bool vertex::is_neighbor(vertex* other_v) const {
    for(size_t e = 0; e < edges.size(); ++e) {
        if(edges[e]->get_other_vertex(this) == other_v) {
            return true;
        }
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Removes an edge from a vertex's list of edges, if it is there.
 * e_ptr:
 *   Edge to remove.
 */
void vertex::remove_edge(edge* e_ptr) {
    size_t i = 0;
    for(; i < edges.size(); ++i) {
        if(edges[i] == e_ptr) {
            edges.erase(edges.begin() + i);
            edge_nrs.erase(edge_nrs.begin() + i);
            return;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Get the convex, concave and ear vertexes.
 * vertexes_left:
 *   List of vertexes left to be processed.
 * ears:
 *   List of ears found.
 * convex_vertexes:
 *   List of convex vertexes found.
 * concave_vertexes:
 *   List of concave vertexes found.
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
 * Returns all vertexes that are close enough to be merged with
 * the specified point, as well as their distances to said point.
 * pos:
 *   Coordinates of the point.
 * all_vertexes:
 *   Vector with all of the vertexes in the area.
 * merge_radius:
 *   Minimum radius to merge. This does not take the camera zoom
 *   level into account.
 */
vector<std::pair<dist, vertex*> > get_merge_vertexes(
    const point &pos, vector<vertex*> &all_vertexes, const float merge_radius
) {

    vector<std::pair<dist, vertex*> > result;
    for(size_t v = 0; v < all_vertexes.size(); ++v) {
        vertex* v_ptr = all_vertexes[v];
        
        dist d(pos, point(v_ptr->x, v_ptr->y));
        if(d <= merge_radius) {
        
            result.push_back(std::make_pair(d, v_ptr));
        }
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Returns the outer polygon and inner polygons of a sector,
 * with the vertexes ordered counter-clockwise for the outer,
 * and clockwise for the inner.
 * Returns a number based on what happened. See TRIANGULATION_ERRORS.
 * s_ptr:
 *   Pointer to the sector.
 * outer:
 *   Return the outer polygon here.
 * inners:
 *   Return the inner polygons here.
 * lone_edges:
 *   Return any lone edges found here.
 * check_vertex_reuse:
 *   True if the algorithm is meant to check for vertexes that get reused.
 */
TRIANGULATION_ERRORS get_polys(
    sector* s_ptr, polygon* outer, vector<polygon>* inners,
    set<edge*>* lone_edges, const bool check_vertex_reuse
) {
    if(!s_ptr || !outer || !inners) return TRIANGULATION_ERROR_INVALID_ARGS;
    TRIANGULATION_ERRORS result = TRIANGULATION_NO_ERROR;
    
    bool doing_outer = true;
    
    //First, compile a list of all edges related to this sector.
    map<edge*, bool> edges_done;
    
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        edges_done[s_ptr->edges[e]] = false;
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
        edge* prev_edge = NULL;
        
        //At the start, assume the angle is left.
        float prev_angle = TAU / 2;
        
        if(!doing_outer) {
            inners->push_back(polygon());
        }
        
        while(!poly_done) {
        
            float base_angle = prev_angle - TAU / 2; //The angle we came from.
            
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
                    get_angle(
                        point(cur_vertex->x, cur_vertex->y),
                        point(other_vertex->x, other_vertex->y)
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
                if(!doing_outer && inners->back().vertexes.size() == 1) {
                    if(lone_edges) {
                        lone_edges->insert(
                            inners->back().vertexes[0]->edges[0]
                        );
                    }
                    inners->erase(inners->begin() + inners->size() - 1);
                } else {
                    if(prev_edge) {
                        if(lone_edges) lone_edges->insert(prev_edge);
                    }
                    result = TRIANGULATION_ERROR_LONE_EDGES;
                }
                
            } else if(edges_done[best_edge]) {
            
                //If we already did this edge, that's it, polygon closed.
                poly_done = true;
                
            } else {
            
                if(doing_outer) {
                    outer->vertexes.push_back(cur_vertex);
                } else {
                    inners->back().vertexes.push_back(cur_vertex);
                }
                
                //Continue onto the next edge.
                prev_vertex = cur_vertex;
                cur_vertex = next_vertex;
                prev_edge = best_edge;
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
    if(check_vertex_reuse) {
        map<vertex*, size_t> vertex_count;
        edge* e_ptr = NULL;
        for(size_t e = 0; e < s_ptr->edges.size(); e++) {
            e_ptr = s_ptr->edges[e];
            vertex_count[e_ptr->vertexes[0]]++;
            vertex_count[e_ptr->vertexes[1]]++;
        }
        
        for(auto &v : vertex_count) {
            if(v.second > 2) {
                //Unfortunately, it does...
                //That means it's a non-simple sector.
                //This likely caused an incorrect triangulation,
                //so let's report it.
                result = TRIANGULATION_ERROR_VERTEXES_REUSED;
                break;
            }
        }
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right in a list of edges.
 * edges:
 *   Edges to check.
 */
vertex* get_rightmost_vertex(const map<edge*, bool> &edges) {
    vertex* rightmost = NULL;
    
    for(auto &e : edges) {
        if(!rightmost) rightmost = e.first->vertexes[0];
        
        for(unsigned char v = 0; v < 2; ++v) {
            rightmost = get_rightmost_vertex(e.first->vertexes[v], rightmost);
        }
    }
    
    return rightmost;
}


/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right between the two.
 * In the case of a tie, the highest one is returned.
 * This is necessary because at one point, the rightmost
 * vertex was being decided kinda randomly.
 * v1:
 *   First vertex to check.
 * v2:
 *   Second vertex to check.
 */
vertex* get_rightmost_vertex(vertex* v1, vertex* v2) {
    if(v1->x > v2->x) return v1;
    if(v1->x == v2->x && v1->y < v2->y) return v1;
    return v2;
}


/* ----------------------------------------------------------------------------
 * Returns which sector the specified point belongs to.
 * p:
 *   Coordinates of the point.
 * sector_nr:
 *   If not NULL, the number of the sector on the area map is placed here.
 *   The number will not be set if the search is using the blockmap.
 * use_blockmap:
 *   If true, use the blockmap to search.
 *   This provides faster results, but the blockmap must be built.
 */
sector* get_sector(
    const point &p, size_t* sector_nr, const bool use_blockmap
) {

    if(use_blockmap) {
    
        size_t col = game.cur_area_data.bmap.get_col(p.x);
        size_t row = game.cur_area_data.bmap.get_row(p.y);
        if(col == INVALID || row == INVALID) return NULL;
        
        unordered_set<sector*>* sectors =
            &game.cur_area_data.bmap.sectors[col][row];
            
        if(sectors->size() == 1) return *sectors->begin();
        
        for(auto s : (*sectors)) {
        
            if(!s) {
                continue;
            }
            if(s->is_point_in_sector(p)) {
                return s;
            }
        }
        
        return NULL;
        
    } else {
    
        for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
            sector* s_ptr = game.cur_area_data.sectors[s];
            
            if(
                p.x < s_ptr->bbox[0].x ||
                p.x > s_ptr->bbox[1].x ||
                p.y < s_ptr->bbox[0].y ||
                p.y > s_ptr->bbox[1].y
            ) {
                continue;
            }
            if(s_ptr->is_point_in_sector(p)) {
                if(sector_nr) *sector_nr = s;
                return s_ptr;
            }
            
        }
        
        if(sector_nr) *sector_nr = INVALID;
        return NULL;
        
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether a polygon was created clockwise or anti-clockwise,
 * given the order of its vertexes.
 * vertexes:
 *   Vertexes to check.
 */
bool is_polygon_clockwise(vector<vertex*> &vertexes) {
    //Solution by http://stackoverflow.com/a/1165943
    float sum = 0;
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        vertex* v2_ptr = get_next_in_vector(vertexes, v);
        sum += (v2_ptr->x - v_ptr->x) * (v2_ptr->y + v_ptr->y);
    }
    return sum < 0;
}


/* ----------------------------------------------------------------------------
 * Returns whether this vertex is convex or not.
 * vec:
 *   List of all vertexes.
 * nr:
 *   Index number of the vertex to check.
 */
bool is_vertex_convex(const vector<vertex*> &vec, const size_t nr) {
    const vertex* cur_v = vec[nr];
    const vertex* prev_v = get_prev_in_vector(vec, nr);
    const vertex* next_v = get_next_in_vector(vec, nr);
    float angle_prev =
        get_angle(
            point(cur_v->x, cur_v->y),
            point(prev_v->x, prev_v->y)
        );
    float angle_next =
        get_angle(
            point(cur_v->x, cur_v->y),
            point(next_v->x, next_v->y)
        );
        
    return get_angle_cw_dif(angle_prev, angle_next) < TAU / 2;
}


/* ----------------------------------------------------------------------------
 * Returns whether this vertex is an ear or not.
 * vec:
 *   List of all vertexes.
 * concaves:
 *   List of concave vertexes.
 * nr:
 *   Index number of the vertex to check.
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
                point(v_to_check->x, v_to_check->y),
                point(pv->x, pv->y),
                point(v->x, v->y),
                point(nv->x, nv->y),
                true
            )
        ) return false;
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Triangulates (turns into triangles) a sector.
 * This is because drawing concave polygons is not possible.
 * Returns a number based on what happened. See TRIANGULATION_ERRORS.
 * s_ptr:
 *   Pointer to the sector.
 * lone_edges:
 *   Return lone edges found here.
 * check_vertex_reuse:
 *   True if the algorithm is meant to check for vertexes that get reused.
 * clear_lone_edges:
 *   Clear this sector's edges from the list of lone edges, if they are there.
 */
TRIANGULATION_ERRORS triangulate(
    sector* s_ptr, set<edge*>* lone_edges, const bool check_vertex_reuse,
    const bool clear_lone_edges
) {

    //We'll triangulate with the Triangulation by Ear Clipping algorithm.
    //http://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
    
    polygon outer_poly;
    vector<polygon> inner_polys;
    
    //Let's clear any "lone" edges here.
    if(clear_lone_edges) {
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            auto it = lone_edges->find(s_ptr->edges[e]);
            if(it != lone_edges->end()) {
                lone_edges->erase(it);
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
    
    TRIANGULATION_ERRORS result =
        get_polys(
            s_ptr, &outer_poly, &inner_polys, lone_edges, check_vertex_reuse
        );
        
    //Get rid of 0-length edges and 180-degree vertexes,
    //as they're redundant.
    outer_poly.clean();
    for(size_t i = 0; i < inner_polys.size(); ++i) {
        inner_polys[i].clean();
    }
    
    //Make cuts on the outer polygon between where it and inner polygons exist,
    //as to make it holeless.
    outer_poly.cut(&inner_polys);
    
    vector<vertex*> vertexes_left = outer_poly.vertexes;
    vector<size_t> ears;
    vector<size_t> convex_vertexes;
    vector<size_t> concave_vertexes;
    s_ptr->triangles.clear();
    if(vertexes_left.size() > 3) {
        s_ptr->triangles.reserve(vertexes_left.size() - 2);
    }
    
    //Begin by making a list of all concave, convex and ear vertexes.
    get_cce(vertexes_left, ears, convex_vertexes, concave_vertexes);
    
    //We do a triangulation until we're left
    //with three vertexes -- the final triangle.
    while(vertexes_left.size() > 3) {
    
        if(ears.empty()) {
            //Something went wrong, the polygon mightn't be simple.
            result = TRIANGULATION_ERROR_NO_EARS;
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
    
    return result;
}
