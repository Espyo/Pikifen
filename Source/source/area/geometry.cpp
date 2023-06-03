/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area geometry-related functions.
 */

#define _USE_MATH_DEFINES

#include <cmath>
#include <vector>

#include <allegro5/allegro_color.h>

#include "geometry.h"

#include "../functions.h"
#include "../utils/geometry_utils.h"
#include "vertex.h"


using std::vector;


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
    
    //Start with the rightmost vertex.
    vertex* rightmost = get_rightmost_vertex();
    
    //We have to make one cut for every inner.
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
                (v1->x >= start->x || v2->x >= start->x) &&
                (v1->x <= rightmost->x || v2->x <= rightmost->x)
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
 * Returns the next edge the trace algorithm should go to.
 * Because at each vertex there can be multiple edges, including multiple
 * edges that belong to the sector we are looking for, we should pick
 * the next edge carefully.
 * Based on information from the previous edge, we should continue travelling
 * via the edge with the smallest angle difference (depending on the rotation
 * direction we're heading.
 * v_ptr:
 *   Vertex to check.
 * prev_v_ptr:
 *   Vertex that we came from, if any. Used to ensure we don't go backwards.
 * s_ptr:
 *   Sector we are trying to trace.
 * prev_e_angle:
 *   Angle of the previous edge. This is the angle from the previous vertex to
 *   the current vertex, so it's sent here cached for performance.
 * best_is_closest_cw:
 *   True if we want the edge that is closest clockwise from the previous edge.
 *   False for the closest counter-clockwise.
 * next_e_ptr:
 *   The next edge is returned here. If there is none, NULL is returned.
 * next_e_angle:
 *   The next edge's angle is returned here. This is used to feed the next
 *   iteration of the algorithm so it doesn't need to re-calculate the angle.
 * next_v_ptr:
 *   Opposing vertex of the next edge.
 */
void find_trace_edge(
    vertex* v_ptr, vertex* prev_v_ptr, sector* s_ptr,
    float prev_e_angle, bool best_is_closest_cw,
    edge** next_e_ptr, float* next_e_angle, vertex** next_v_ptr
) {
    //Info about the best candidate edge, if any.
    edge* best_e_ptr = NULL;
    float best_e_angle = 0;
    float best_e_angle_cw_dif = 0;
    vertex* best_v_ptr = NULL;
    
    //Go through each edge to check for the best.
    for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
    
        edge* e_ptr = v_ptr->edges[e];
        
        if(e_ptr->sectors[0] != s_ptr && e_ptr->sectors[1] != s_ptr) {
            //This edge is not related to our sector.
            continue;
        }
        
        vertex* other_v_ptr = e_ptr->get_other_vertex(v_ptr);
        
        if(other_v_ptr == prev_v_ptr) {
            //This is where we came from.
            continue;
        }
        
        //Find this edge's angle,
        //between our vertex and this edge's other vertex.
        float e_angle =
            get_angle(
                point(v_ptr->x, v_ptr->y),
                point(other_v_ptr->x, other_v_ptr->y)
            );
            
        float angle_cw_dif = get_angle_cw_dif(prev_e_angle + TAU / 2.0f, e_angle);
        
        //Check if this is the best.
        if(
            !best_e_ptr ||
            (best_is_closest_cw && angle_cw_dif < best_e_angle_cw_dif) ||
            (!best_is_closest_cw && angle_cw_dif > best_e_angle_cw_dif)
        ) {
            best_e_ptr = e_ptr;
            best_e_angle_cw_dif = angle_cw_dif;
            best_e_angle = e_angle;
            best_v_ptr = other_v_ptr;
        }
    }
    
    //Return our result.
    *next_e_ptr = best_e_ptr;
    *next_e_angle = best_e_angle;
    *next_v_ptr = best_v_ptr;
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
    const vector<vertex*> &vertexes_left, vector<size_t> &ears,
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
    const point &pos, const vector<vertex*> &all_vertexes,
    const float merge_radius
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
 * Returns an error code.
 * s_ptr:
 *   Pointer to the sector.
 * outer:
 *   Return the outer polygon here.
 * inners:
 *   Return the inner polygons here.
 */
TRIANGULATION_ERRORS get_polys(
    sector* s_ptr, polygon* outer, vector<polygon>* inners
) {
    if(!s_ptr || !outer || !inners) return TRIANGULATION_ERROR_INVALID_ARGS;
    TRIANGULATION_ERRORS result = TRIANGULATION_NO_ERROR;
    
    bool doing_outer = true;
    
    //First, compile a list of all edges related to this sector.
    unordered_set<edge*> edges_left(s_ptr->edges.begin(), s_ptr->edges.end());
    
    //Now trace along the edges, vertex by vertex, until we have no more left.
    while(!edges_left.empty()) {
    
        //Start with the rightmost vertex.
        //If we still haven't closed the outer polygon, then this vertex
        //mandatorily belongs to it. Otherwise, it belongs to an inner.
        vertex* first_v_ptr = get_rightmost_vertex(edges_left);
        
        polygon* cur_poly_ptr = NULL;
        if(doing_outer) {
            cur_poly_ptr = outer;
        } else {
            inners->push_back(polygon());
            cur_poly_ptr = &(inners->back());
        }
        
        //Trace! For the outer poly, we're going counter-clockwise,
        //while for the inner ones, it's clockwise.
        trace_edges(
            first_v_ptr, s_ptr, !doing_outer,
            &cur_poly_ptr->vertexes, &edges_left
        );
        
        doing_outer = false;
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right in a list of edges.
 * edges:
 *   Edges to check.
 */
vertex* get_rightmost_vertex(const unordered_set<edge*> &edges) {
    vertex* rightmost = NULL;
    
    for(auto &e : edges) {
        if(!rightmost) rightmost = e->vertexes[0];
        
        for(unsigned char v = 0; v < 2; ++v) {
            rightmost = get_rightmost_vertex(e->vertexes[v], rightmost);
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
 * Traces edges until it returns to the start, at which point it
 * closes a polygon.
 * Returns an error code.
 * start_v_ptr:
 *   Vertex to start on.
 * s_ptr:
 *   Sector to trace around.
 * going_cw:
 *   True if the travel direction should be clockwise,
 *   false for counter-clockwise.
 * vertexes:
 *   The final list of vertexes is returned here.
 * unvisited_edges:
 *   List of edges that have not been visited, so the algorithm can
 *   remove them from the list as it visits them.
 */
TRIANGULATION_ERRORS trace_edges(
    vertex* start_v_ptr, sector* s_ptr, bool going_cw,
    vector<vertex*>* vertexes, unordered_set<edge*>* unvisited_edges
) {
    if(!start_v_ptr || !s_ptr) return TRIANGULATION_ERROR_INVALID_ARGS;
    
    vertex* v_ptr = start_v_ptr;
    
    //At the start, no need to check if we're going to the previous vertex.
    vertex* prev_v_ptr = NULL;
    //At the start, assume the angle is left.
    float prev_e_angle = TAU / 2.0f;
    
    edge* next_e_ptr = NULL;
    vertex* next_v_ptr = NULL;
    float next_e_angle = 0.0f;
    
    edge* first_e_ptr = NULL;
    
    TRIANGULATION_ERRORS result = TRIANGULATION_NO_ERROR;
    bool poly_done = false;
    
    //Trace around, vertex by vertex, until we're done.
    while(!poly_done) {
    
        //Find the next edge to go to.
        //For cases where the vertex only has two edges of our sector,
        //it's pretty obvious -- just go to the edge that isn't the one we
        //came from. But if the vertex has more edges, we need to pick based
        //on the angle and what we're trying to do. There are two approaches:
        //
        //            Turn inward           |           Turn outward           
        //----------------------------------+----------------------------------
        //You can think of it like you're   |Same, but the cane must stay on
        //walking on top of the lines, and  |the sector/s that are outside of
        //you have a cane dragging on the   |the trace direction.
        //floor to your left or right.      |
        //This cane must stay on the        |
        //sector/s that are inside of the   |
        //trace direction.                  |
        //----------------------------------+----------------------------------
        //With this you traverse the shape  |With this you traverse the shape
        //as deeply as possible, and enter  |as broadly as possible, and you
        //and close loops as soon as        |don't enter any loop.
        //possible.                         |
        //----------------------------------+----------------------------------
        //For outer polygons, this works    |For outer polygons, this will
        //fine and can even be used to      |skip out on geometry inside shared
        //detect islands.                   |vertexes (see fig. A).
        //----------------------------------+----------------------------------
        //For inner polygons, this works,   |For inner polygons the broad shape
        //but the other method is better.   |is more convenient.
        //----------------------------------+----------------------------------
        //So basically this is the best     |Basically best suited for inner
        //option for outer polygons. This   |polygons since when we land on
        //way when we land on a shared      |a reused vertex, we can skip over
        //vertex, we don't skip over parts  |loops and just get the final shape,
        //of the outer polygon's geometry.  |instead of obtaining multiple
        //e.g. see fig. A.                  |inner polygons.
        //                                  |e.g. see fig. B.
        //----------------------------------+----------------------------------
        //You want the edge closest to you  |You want the edge closest to you
        //in the opposite orientation to    |in the same orientation as
        //your direction of travel (from the|your direction of travel (from the
        //shared vertex). i.e. closest      |shared vertex). i.e. closest
        //clockwise if you're traversing the|clockwise if you're traversing the
        //edges counter-clockwise, closest  |edges clockwise, closest
        //counter-clockwise if you're       |counter-clockwise if you're
        //traversing clockwise.             |traversing counter-clockwise.
        //
        //Fig. A.
        //  +--------+
        //  |   +--+ |  1 = Sector 1
        //  |   |   \|  2 = Sector 2
        //  | 1 | 2  +
        //  |   |   /|
        //  |   +--+ |
        //  +--------+
        //
        //Fig. B.
        //  +---------+
        //  |    +--+ |  1 = Sector 1
        //  | 1  |2/  |  2 = Sector 2
        //  |    |/   |  3 = Sector 3
        //  | +--+    |
        //  |  \3|    |
        //  |   \|    |
        //  |    +    |
        //  +---------+
        //
        //With all of that said, the first iteration, where we find the first
        //edge, needs to be selected according to what sort of polygon we're
        //tracing. Counter-clockwise for outer, clockwise for inner. The edge
        //we pick from the starting vertex will dictate the direction of travel.
        //So for outer polygons, we want to start by picking the closest
        //counter-clockwise edge, so we can set the trace orientation to
        //counter-clockwise, and then swap over to picking the closest
        //clockwise so we can turn inward.
        //For inner polygons, start with the closest clockwise edge so we
        //trace clockwise, then continue that way so we turn outward.
        
        bool best_is_closest_cw = going_cw;
        if(prev_v_ptr != NULL) best_is_closest_cw = true;
        
        find_trace_edge(
            v_ptr, prev_v_ptr, s_ptr, prev_e_angle, best_is_closest_cw,
            &next_e_ptr, &next_e_angle, &next_v_ptr
        );
        
        //Now that we have the edge, what do we do?
        if(!next_e_ptr) {
            //If there is no edge to go to next, this sector is not closed.
            result = TRIANGULATION_ERROR_NOT_CLOSED;
            poly_done = true;
            
        } else if(next_e_ptr == first_e_ptr) {
            //If we already did this edge, that's it, polygon closed.
            poly_done = true;
            
        } else {
            //Part of the trace.
            vertexes->push_back(v_ptr);
            prev_e_angle = next_e_angle;
            prev_v_ptr = v_ptr;
            v_ptr = next_v_ptr;
            
        }
        
        //Finishing setup before the next iteration.
        if(!first_e_ptr) {
            first_e_ptr = next_e_ptr;
        }
        if(next_e_ptr) {
            unvisited_edges->erase(next_e_ptr);
        }
        
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Triangulates a polygon via the Triangulation by Ear Clipping algorithm.
 * http://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
 * Returns an error code.
 * poly:
 *   The polygon to triangulate.
 * triangles:
 *   The final list of triangles is returned here.
 */
TRIANGULATION_ERRORS triangulate_polygon(
    polygon* poly, vector<triangle>* triangles
) {

    TRIANGULATION_ERRORS result = TRIANGULATION_NO_ERROR;
    vector<vertex*> vertexes_left = poly->vertexes;
    vector<size_t> ears;
    vector<size_t> convex_vertexes;
    vector<size_t> concave_vertexes;
    
    if(vertexes_left.size() > 3) {
        triangles->reserve(vertexes_left.size() - 2);
    }
    
    //Begin by making a list of all concave, convex, and ear vertexes.
    get_cce(vertexes_left, ears, convex_vertexes, concave_vertexes);
    
    //We do the triangulation until we're left
    //with three vertexes -- the final triangle.
    while(vertexes_left.size() > 3) {
    
        if(ears.empty()) {
            //Something went wrong, the polygon mightn't be simple.
            result = TRIANGULATION_ERROR_NO_EARS;
            break;
            
        } else {
            //The ear, the previous, and the next vertexes make a triangle.
            triangles->push_back(
                triangle(
                    vertexes_left[ears[0]],
                    get_prev_in_vector(vertexes_left, ears[0]),
                    get_next_in_vector(vertexes_left, ears[0])
                )
            );
            
            //Remove the ear.
            vertexes_left.erase(vertexes_left.begin() + ears[0]);
            
            //Recalculate the ears, concave, and convex vertexes.
            get_cce(vertexes_left, ears, convex_vertexes, concave_vertexes);
        }
    }
    
    //Finally, add the final triangle.
    if(vertexes_left.size() == 3) {
        triangles->push_back(
            triangle(
                vertexes_left[1], vertexes_left[0], vertexes_left[2]
            )
        );
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Triangulates (turns into triangles) a sector.
 * This is because drawing concave polygons is not possible.
 * Returns an error code.
 * s_ptr:
 *   Pointer to the sector.
 * lone_edges:
 *   Return lone edges found here.
 * clear_lone_edges:
 *   Clear this sector's edges from the list of lone edges, if they are there.
 */
TRIANGULATION_ERRORS triangulate_sector(
    sector* s_ptr, set<edge*>* lone_edges, const bool clear_lone_edges
) {

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
    
    //Step 1. Get polygons.
    //We need to know what vertexes mark the outermost polygon,
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
    
    TRIANGULATION_ERRORS result = get_polys(s_ptr, &outer_poly, &inner_polys);
    if(result != TRIANGULATION_NO_ERROR) return result;
    
    //Get rid of 0-length edges and 180-degree vertexes,
    //as they're redundant.
    outer_poly.clean();
    for(size_t i = 0; i < inner_polys.size(); ++i) {
        inner_polys[i].clean();
    }
    
    //Step 2. Make cuts.
    //Make cuts on the outer polygon between where it and inner polygons exist,
    //as to make the outer polygon one big holeless polygon.
    outer_poly.cut(&inner_polys);
    
    //Step 3. Triangulate the polygon.
    //Transforming the polygon into triangles.
    s_ptr->triangles.clear();
    result = triangulate_polygon(&outer_poly, &s_ptr->triangles);
    
    //Done!
    return result;
}
