/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Sector, linedef, etc. classes, and related functions.
 */

#define _USE_MATH_DEFINES

#include <algorithm>
#include <cfloat>
#include <math.h>
#include <queue>
#include <unordered_set>

#include "functions.h"
#include "sector.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Clears the info on an area map.
 */
void area_map::clear() {
    vertices.clear();
    linedefs.clear();
    sectors.clear();
}

/* ----------------------------------------------------------------------------
 * Creates a linedef.
 */
linedef::linedef(size_t v1, size_t v2) {
    vertices[0] = vertices[1] = NULL;
    sectors[0] = sectors[1] = NULL;
    sector_nrs[0] = string::npos; sector_nrs[1] = string::npos;
    
    vertex_nrs[0] = v1; vertex_nrs[1] = v2;
}

/* ----------------------------------------------------------------------------
 * Fixes the pointers to point to the correct sectors and vertices.
 */
void linedef::fix_pointers(area_map &a) {
    sectors[0] = sectors[1] = NULL;
    for(size_t s = 0; s < 2; s++) {
        size_t s_nr = sector_nrs[s];
        sectors[s] = (s_nr == string::npos ? NULL : a.sectors[s_nr]);
    }
    
    vertices[0] = vertices[1] = NULL;
    for(size_t v = 0; v < 2; v++) {
        size_t v_nr = vertex_nrs[v];
        vertices[v] = (v_nr == string::npos ? NULL : a.vertices[v_nr]);
    }
}

/* ----------------------------------------------------------------------------
 * Creates a sector.
 */
sector::sector() {
    type = SECTOR_TYPE_NORMAL;
    tag = 0;
}

/* ----------------------------------------------------------------------------
 * Connects the linedefs that link to it into the linedef_nrs vector.
 */
void sector::connect_linedefs(area_map &a, size_t s_nr) {
    linedef_nrs.clear();
    for(size_t l = 0; l < a.linedefs.size(); l++) {
        linedef* l_ptr = a.linedefs[l];
        if(l_ptr->sector_nrs[0] == s_nr || l_ptr->sector_nrs[1] == s_nr) {
            linedef_nrs.push_back(l);
        }
    }
    fix_pointers(a);
}

/* ----------------------------------------------------------------------------
 * Fixes the pointers to point them to the correct linedefs.
 */
void sector::fix_pointers(area_map &a) {
    linedefs.clear();
    for(size_t l = 0; l < linedef_nrs.size(); l++) {
        size_t l_nr = linedef_nrs[l];
        linedefs.push_back(l_nr == string::npos ? NULL : a.linedefs[l_nr]);
    }
}

/* ----------------------------------------------------------------------------
 * Creates a structure with floor information.
 */
sector_texture::sector_texture() {
    scale = 0;
    trans_x = trans_y = 0;
    rot = 0;
    bitmap = NULL;
}

/* ----------------------------------------------------------------------------
 * Creates a vertex.
 */
vertex::vertex(float x, float y) {
    this->x = x; this->y = y;
}

/* ----------------------------------------------------------------------------
 * Creates a triangle.
 */
triangle::triangle(vertex* v1, vertex* v2, vertex* v3, sector* s_ptr, size_t s_nr) {
    points[0] = v1;
    points[1] = v2;
    points[2] = v3;
    this->s_ptr = s_ptr;
    this->s_nr = s_nr;
}

/* ----------------------------------------------------------------------------
 * Connects the linedefs that link to it into the linedef_nrs vector.
 */
void vertex::connect_linedefs(area_map &a, size_t v_nr) {
    linedef_nrs.clear();
    for(size_t l = 0; l < a.linedefs.size(); l++) {
        linedef* l_ptr = a.linedefs[l];
        if(l_ptr->vertex_nrs[0] == v_nr || l_ptr->vertex_nrs[1] == v_nr) {
            linedef_nrs.push_back(l);
        }
    }
    fix_pointers(a);
}

/* ----------------------------------------------------------------------------
 * Fixes the pointers to point to the correct linedefs.
 */
void vertex::fix_pointers(area_map &a) {
    linedefs.clear();
    for(size_t l = 0; l < linedef_nrs.size(); l++) {
        size_t l_nr = linedef_nrs[l];
        linedefs.push_back(l_nr == string::npos ? NULL : a.linedefs[l_nr]);
    }
}

/* ----------------------------------------------------------------------------
 * Returns the outer polygon and inner polygons of a sector,
 * with the vertices ordered counter-clockwise for the outer,
 * and clockwise for the inner.
 */
void get_polys(sector* s, polygon* outer, vector<polygon>* inners) {
    if(!s || !outer || !inners) return;
    
    bool doing_outer = true;
    
    //First, compile a list of all sidedefs related to this sector.
    map<linedef*, bool> lines_done;
    
    for(size_t l = 0; l < s->linedefs.size(); l++) {
        lines_done[s->linedefs[l]] = false;
    }
    
    //Now travel along the lines, vertex by vertex, until we have no more left.
    while(lines_done.size() > 0) {
        bool poly_done = false;
        
        //Start with the rightmost vertex.
        //If we still haven't closed the outer polygon, then this vertex
        //mandatorily belongs to it. Otherwise, it belongs to an inner.
        vertex* cur_vertex = get_rightmost_vertex(lines_done);
        vertex* next_vertex = NULL;
        vertex* prev_vertex = NULL;
        
        float prev_angle = 0; //At the start, assume the angle is 0 (right).
        
        if(!doing_outer) {
            inners->push_back(polygon());
        }
        
        while(!poly_done) {
        
            float base_angle = prev_angle; //The angle we came from.
            
            //For every linedef attached to this vertex, find the closest one
            //that hasn't been done, in the direction of travel.
            
            float best_angle_dif = 0;
            linedef* best_line = NULL;
            
            for(size_t l = 0; l < cur_vertex->linedefs.size(); l++) {
                linedef* l_ptr = cur_vertex->linedefs[l];
                auto it = lines_done.find(l_ptr);
                if(it == lines_done.end()) continue; //We're not meant to check this line.
                
                vertex* other_vertex = l_ptr->vertices[0] == cur_vertex ? l_ptr->vertices[1] : l_ptr->vertices[0];
                
                if(other_vertex == prev_vertex) continue; //This is where we came from.
                
                //Find the angle between our vertex and this vertex.
                float angle = atan2(other_vertex->y - cur_vertex->y, other_vertex->x - cur_vertex->x);
                float angle_dif = get_angle_dif(base_angle, angle);
                
                //For the outer poly, we're going counter-clockwise. So the lowest angle difference is best.
                //For the inner ones, it's clockwise, so the highest.
                if(
                    !best_line ||
                    (doing_outer  && angle_dif < best_angle_dif) ||
                    (!doing_outer && angle_dif > best_angle_dif)
                ) {
                    best_line = l_ptr;
                    best_angle_dif = angle_dif;
                    prev_angle = angle;
                    next_vertex = other_vertex;
                }
            }
            
            if(!best_line) {
            
                //If there is no line to go to next, something went wrong.
                //ToDo report error?
                poly_done = true;
                
            } else if(lines_done[best_line]) {
            
                //If we already did this line, that's it, polygon closed.
                poly_done = true;
                
            } else {
            
                if(doing_outer) {
                    outer->push_back(cur_vertex);
                } else {
                    inners->back().push_back(cur_vertex);
                }
                
                //Continue onto the next line.
                prev_vertex = cur_vertex;
                cur_vertex = next_vertex;
                lines_done[best_line] = true;
                
            }
        }
        
        doing_outer = false;
        
        //Remove all lines that were done from the list.
        auto it = lines_done.begin();
        while(it != lines_done.end()) {
            if(it->second) {
                lines_done.erase(it++);
            } else {
                ++it;
            }
        }
    }
}

/* ----------------------------------------------------------------------------
 * Returns which sector the specified point belongs to.
 */
sector* get_sector(float x, float y, size_t* sector_nr) {
    for(size_t t = 0; t < cur_area_map.triangles.size(); t++) {
        triangle* t_ptr = &cur_area_map.triangles[t];
        if(
            is_point_in_triangle(
                x, y,
                t_ptr->points[0]->x, t_ptr->points[0]->y,
                t_ptr->points[1]->x, t_ptr->points[1]->y,
                t_ptr->points[2]->x, t_ptr->points[2]->y
            )
        ) {
            if(sector_nr) *sector_nr = t_ptr->s_nr;
            return t_ptr->s_ptr;
        }
    }
    
    if(sector_nr) *sector_nr = string::npos;
    return NULL;
}

/* ----------------------------------------------------------------------------
 * Returns whether a point is inside a triangle or not.
 * px, py: Coordinates of the point to check.
 * t**:    Coordinates of the triangle's points.
 * Thanks go to http://stackoverflow.com/questions/2049582/how-to-determine-a-point-in-a-triangle
 */
bool is_point_in_triangle(float px, float py, float tx1, float ty1, float tx2, float ty2, float tx3, float ty3) {
    float s = ty1 * tx3 - tx1 * ty3 + (ty3 - ty1) * px + (tx1 - tx3) * py;
    float t = tx1 * ty2 - ty1 * tx2 + (ty1 - ty2) * px + (tx2 - tx1) * py;
    
    if ((s < 0) != (t < 0)) return false;
    
    float a = -ty2 * tx3 + ty1 * (tx3 - tx2) + tx1 * (ty2 - ty3) + tx2 * ty3;
    if (a < 0.0) {
        s = -s;
        t = -t;
        a = -a;
    }
    return s > 0 && t > 0 && (s + t) < a;
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
    
    return get_angle_dif(angle_next, angle_prev) <= M_PI;
}

/* ----------------------------------------------------------------------------
 * Returns whether this vertex is an ear or not.
 */
bool is_vertex_ear(const vector<vertex*> &vec, const vector<size_t> &concaves, const size_t nr) {
    //A vertex is an ear if the triangle of it, the previous and next vertices
    //does not contain any other vertex inside. Also, if it has vertices inside,
    //they mandatorily are concave, so only check those.
    const vertex* v = vec[nr];
    const vertex* pv = get_prev_in_vector(vec, nr);
    const vertex* nv = get_next_in_vector(vec, nr);
    
    for(size_t c = 0; c < concaves.size(); c++) {
        const vertex* v_to_check = vec[concaves[c]];
        if(
            is_point_in_triangle(
                v_to_check->x, v_to_check->y,
                pv->x, pv->y,
                v->x, v->y,
                nv->x, nv->y
            )
        ) return false;
    }
    
    return true;
}

/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right.
 */
vertex* get_rightmost_vertex(map<linedef*, bool> &lines) {
    vertex* rightmost = NULL;
    
    for(auto l = lines.begin(); l != lines.end(); l++) {
        if(!rightmost) rightmost = l->first->vertices[0];
        
        for(unsigned char v = 0; v < 2; v++) {
            if(l->first->vertices[v]->x > rightmost->x) rightmost = l->first->vertices[v];
        }
    }
    
    return rightmost;
}

/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right.
 */
vertex* get_rightmost_vertex(polygon* p) {
    vertex* rightmost = NULL;
    
    for(size_t v = 0; v < p->size(); v++) {
        if(!rightmost) rightmost = p->at(v);
        
        if(p->at(v)->x > rightmost->x) rightmost = p->at(v);
    }
    
    return rightmost;
}

/* ----------------------------------------------------------------------------
 * Cleans a polygon's vertices.
 * This deletes 0-lenght lines, and 180-degree vertices.
 */
void clean_poly(polygon* p) {
    for(size_t v = 0; v < p->size();) {
        bool should_delete = false;
        vertex* prev_v = get_prev_in_vector((*p), v);
        vertex* cur_v =  p->at(v);
        vertex* next_v = get_next_in_vector((*p), v);
        
        //If the distance between both vertices is so small that it's basically 0,
        //delete this vertex from the list.
        if(fabs(prev_v->x - cur_v->x) < 0.00001 && fabs(prev_v->y - cur_v->y) < 0.00001) {
            should_delete = true;
        }
        
        //If the angle between this vertex and the next is the same,
        //then this is just a redundant point in the line prev - next. Delete it.
        //ToDo is this working?
        if(fabs(atan2(prev_v->y - cur_v->y, prev_v->x - cur_v->x) - atan2(cur_v->y - next_v->y, cur_v->x - next_v->x)) < 0.00001) {
            should_delete = true;
        }
        
        if(should_delete) {
            p->erase(p->begin() + v);
        } else {
            v++;
        }
    }
}

/* ----------------------------------------------------------------------------
 * When there are inner polygons, a cut must be made between it and the outer
 * polygon, as to make the outer holeless.
 */
void cut_poly(polygon* outer, vector<polygon>* inners) {
    //ToDo what if both vertices of the intersecting line are on the same x-axis as the ray?
    vertex* outer_rightmost = get_rightmost_vertex(outer);
    
    for(size_t i = 0; i < inners->size(); i++) {
        polygon* p = &inners->at(i);
        vertex* closest_line_v1 = NULL;
        vertex* closest_line_v2 = NULL;
        float closest_line_u = FLT_MAX;
        
        //Find the rightmost vertex on this inner.
        vertex* start = get_rightmost_vertex(p);
        
        //Imagine a line from this vertex to the right.
        //If any line of the outer polygon intersects it,
        //we just find the vertex there, and make the cut.
        //For each line on the outer polygon, check if it intersects.
        vertex* v1 = NULL, *v2 = NULL;
        for(size_t v = 0; v < outer->size(); v++) {
            v1 = outer->at(v);
            v2 = get_next_in_vector(*outer, v);
            if(
                (v1->x >= start->x ||
                 v2->x >= start->x) &&
                (v1->x <= outer_rightmost->x + 10 || //+10 is just to make sure
                 v2->x <= outer_rightmost->x + 10)
            ) {
                float u, ul;
                if(lines_intersect(v1->x, v1->y, v2->x, v2->y, start->x, start->y, outer_rightmost->x, start->y, &u, &ul)) {
                    if(!closest_line_v1 || !closest_line_v2) {
                        closest_line_v1 = v1;
                        closest_line_v2 = v2;
                        closest_line_u = u;
                    }
                    
                    if(u < closest_line_u) {
                        closest_line_v1 = v1;
                        closest_line_v2 = v2;
                        closest_line_u = u;
                    }
                }
            }
        }
        
        //ToDo what if it doesn't find any? Should it try left?
        //ToDo and what if this inner polygon can only see another inner polygon?
        
        //We're on the line closest to the vertex.
        //Go to the rightmost vertex of this line.
        vertex* vertex_to_compare = (v1->x > v2->x ? v1 : v2);
        
        //Now get a list of all vertices inside the triangle
        //marked by the inner's vertex,
        //the point on the line,
        //and the vertex we're comparing.
        vector<vertex*> inside_triangle;
        for(size_t v = 0; v < outer->size(); v++) {
            vertex* v_ptr = outer->at(v);
            if(is_point_in_triangle(v_ptr->x, v_ptr->y, start->x, start->y, start->x + closest_line_u, start->y, vertex_to_compare->x, vertex_to_compare->y) && v_ptr != vertex_to_compare) {
                inside_triangle.push_back(v_ptr);
            }
        }
        
        //Check which one makes the smallest angle compared to 0.
        float closest_angle = FLT_MAX;
        vertex* best_vertex = vertex_to_compare;
        
        for(size_t v = 0; v < inside_triangle.size(); v++) {
            vertex* v_ptr = inside_triangle[v];
            float angle = atan2(start->y - v_ptr->y, start->x - v_ptr->x);
            if(fabs(angle) < closest_angle) {
                closest_angle = angle;
                best_vertex = v_ptr;
            }
        }
        
        //This is the final vertex. Make a segment
        //from the start vertex to this.
        //Find the vertex on the outer polygon,
        //and add the bridge to the inner polygon,
        //as well as the entire inner polygon.
        //Then go back to the vertex we were at.
        for(size_t v = 0; v < outer->size(); v++) {
            vertex* v_ptr = outer->at(v);
            if(v_ptr == best_vertex) {
            
                //We found the vertex of the outer to make the split at.
                //Now find which vertex of the inner we make the
                //other point of the split at.
                size_t iv = 0;
                for(; iv < p->size(); iv++) {
                    if(p->at(iv) == start) {
                        break;
                    }
                }
                
                auto it = p->begin() + iv;
                size_t n_after = p->size() - iv;
                //Finally, make the bridge.
                outer->insert(outer->begin() + v + 1, it, p->end());
                outer->insert(outer->begin() + v + 1 + n_after, p->begin(), it); //ToDo is it included?
                outer->insert(outer->begin() + v + 1 + n_after + iv, start); //Closes the inner polygon.
                outer->insert(outer->begin() + v + 1 + n_after + iv + 1, best_vertex); //Closes the outer polygon.
            }
            break;
        }
        
    }
}

/* ----------------------------------------------------------------------------
 * Returns the difference between two angles, in radians, but clockwise.
 */
float get_angle_dif(float a1, float a2) {
    if(a2 > a1) a2 -= M_PI * 2;
    return a1 - a2;
}

/* ----------------------------------------------------------------------------
 * Get the convex, concave and ear vertices.
 */
void get_cce(vector<vertex*> &vertices_left, vector<size_t> &ears, vector<size_t> &convex_vertices, vector<size_t> &concave_vertices) {
    ears.clear();
    convex_vertices.clear();
    concave_vertices.clear();
    for(size_t v = 0; v < vertices_left.size(); v++) {
        bool is_convex = is_vertex_convex(vertices_left, v);
        if(is_convex) {
            convex_vertices.push_back(v);
            
        } else {
            concave_vertices.push_back(v);
        }
    }
    
    for(size_t c = 0; c < convex_vertices.size(); c++) {
        if(is_vertex_ear(vertices_left, concave_vertices, convex_vertices[c])) {
            ears.push_back(convex_vertices[c]);
        }
    }
}

/* ----------------------------------------------------------------------------
 *
 */
bool lines_intersect(float l1x1, float l1y1, float l1x2, float l1y2, float l2x1, float l2y1, float l2x2, float l2y2, float* ur, float* ul) {
    if(!ur || !ul) return false;
    
    float div = (l2y2 - l2y1) * (l1x2 - l1x1) - (l2x2 - l2x1) * (l1y2 - l1y1);
    
    if(div != 0) {
    
        //Calculate the intersection distance from the line.
        *ul = ((l2x2 - l2x1) * (l1y1 - l2y1) - (l2y2 - l2y1) * (l1x1 - l2x1)) / div;
        
        //Calculate the intersection distance from the ray.
        *ur = ((l1x2 - l1x1) * (l1y1 - l2y1) - (l1y2 - l1y1) * (l1x1 - l2x1)) / div;
        
        //Return whether they intersect.
        return (*ur >= 0) && (*ur <= 1) && (*ul > 0) && (*ul < 1);
        
    } else {
        //No intersection.
        return false;
    }
}

/* ----------------------------------------------------------------------------
 * Triangulates (turns into triangles) a sector. This is because drawing concave polygons is not possible.
 */
vector<triangle> triangulate(sector* s, size_t s_nr) {

    //We'll triangulate with the Triangulation by Ear Clipping algorithm.
    //http://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
    
    polygon outer_poly;
    vector<polygon> inner_polys;
    
    //First, we need to know what vertices mark the outermost polygon,
    //and what vertices mark the inner ones.
    //There can be no islands or polygons of our sector inside the inner ones.
    //Example of a sector:
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
    get_polys(s, &outer_poly, &inner_polys);
    
    //Get rid of 0-length vertices and 180-degree vertices, as they're redundant.
    clean_poly(&outer_poly);
    for(size_t i = 0; i < inner_polys.size(); i++) clean_poly(&inner_polys[i]);
    
    //Make cuts on the outer polygon between where it and inner polygons exist, as to make it holeless.
    cut_poly(&outer_poly, &inner_polys);
    
    vector<triangle> triangles;
    vector<vertex*> vertices_left = outer_poly;
    vector<size_t> ears;
    vector<size_t> convex_vertices;
    vector<size_t> concave_vertices;
    
    //Begin by making a list of all concave, convex and ear vertices.
    get_cce(vertices_left, ears, convex_vertices, concave_vertices);
    
    //We do a triangulation until we're left with three vertices -- the final triangle.
    while(vertices_left.size() > 3) {
        if(ears.size() == 0) {
            //Something went wrong, the polygon mightn't be simple.
            //ToDo warn.
            break;
        } else {
            //The ear, the previous and the next vertices make a triangle.
            triangles.push_back(
                triangle(
                    vertices_left[ears[0]],
                    get_prev_in_vector(vertices_left, ears[0]),
                    get_next_in_vector(vertices_left, ears[0]),
                    s, s_nr
                )
            );
            
            //Remove the ear.
            vertices_left.erase(vertices_left.begin() + ears[0]);
            
            //Recalculate the ears, concave and convex vertices.
            get_cce(vertices_left, ears, convex_vertices, concave_vertices);
        }
    }
    
    //Finally, add the final triangle.
    if(vertices_left.size() == 3) {
        triangles.push_back(
            triangle(
                vertices_left[1], vertices_left[0], vertices_left[2], s, s_nr
            )
        );
    }
    
    return triangles;
}
