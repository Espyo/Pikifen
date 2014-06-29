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
    for(size_t v = 0; v < vertices.size(); v++) {
        delete vertices[v];
    }
    for(size_t l = 0; l < linedefs.size(); l++) {
        delete linedefs[l];
    }
    for(size_t s = 0; s < sectors.size(); s++) {
        delete sectors[s];
    }
    for(size_t m = 0; m < mob_generators.size(); m++) {
        delete mob_generators[m];
    }
    for(size_t s = 0; s < tree_shadows.size(); s++) {
        delete tree_shadows[s];
    }
    
    vertices.clear();
    linedefs.clear();
    sectors.clear();
    mob_generators.clear();
    tree_shadows.clear();
}

/* ----------------------------------------------------------------------------
 * Creates a blockmap.
 */
blockmap::blockmap() {
    x1 = y1 = 0;
    n_cols = n_rows = 0;
}

/* ----------------------------------------------------------------------------
 * Creates a linedef.
 */
linedef::linedef(size_t v1, size_t v2) {
    vertices[0] = vertices[1] = NULL;
    sectors[0] = sectors[1] = NULL;
    sector_nrs[0] = sector_nrs[1] = string::npos;
    
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
 * Removes the linedef from its sectors, but doesn't mark
 * the sectors as "none".
 * Returns the linedef number.
 */
size_t linedef::remove_from_sectors() {
    size_t l_nr = string::npos;
    for(unsigned char s = 0; s < 2; s++) {
        if(!sectors[s]) continue;
        for(size_t l = 0; l < sectors[s]->linedefs.size(); l++) {
            linedef* l_ptr = sectors[s]->linedefs[l];
            if(l_ptr == this) {
                sectors[s]->linedefs.erase(sectors[s]->linedefs.begin() + l);
                auto nr_it = sectors[s]->linedef_nrs.begin() + l;
                l_nr = *nr_it;
                sectors[s]->linedef_nrs.erase(nr_it);
                break;
            }
        }
    }
    return l_nr;
}

/* ----------------------------------------------------------------------------
 * Removes the linedef from its vertices, but doesn't mark
 * the vertices as "none".
 * Returns the linedef number.
 */
size_t linedef::remove_from_vertices() {
    size_t l_nr = string::npos;
    for(unsigned char v = 0; v < 2; v++) {
        if(!vertices[v]) continue;
        for(size_t l = 0; l < vertices[v]->linedefs.size(); l++) {
            linedef* l_ptr = vertices[v]->linedefs[l];
            if(l_ptr == this) {
                vertices[v]->linedefs.erase(vertices[v]->linedefs.begin() + l);
                auto nr_it = vertices[v]->linedef_nrs.begin() + l;
                l_nr = *nr_it;
                vertices[v]->linedef_nrs.erase(nr_it);
                break;
            }
        }
    }
    return l_nr;
}

/* ----------------------------------------------------------------------------
 * Creates a mob generation structure.
 */
mob_gen::mob_gen(float x, float y, unsigned char folder, mob_type* type, float angle, string vars) {
    this->folder = folder;
    this->type = type;
    this->x = x; this->y = y;
    this->angle = angle;
    this->vars = vars;
}

/* ----------------------------------------------------------------------------
 * Creates a sector.
 */
sector::sector() {
    type = SECTOR_TYPE_NORMAL;
    z = 0;
    tag = 0;
    brightness = DEF_SECTOR_BRIGHTNESS;
    fade = false;
    scale_x = scale_y = 1;
    trans_x = trans_y = 0;
    rot = 0;
    bitmap = NULL;
}

/* ----------------------------------------------------------------------------
 * Clones a sector's properties onto another,
 * not counting the list of linedefs or bitmap
 * (the file name is cloned too, though).
 */
void sector::clone(sector* new_sector) {
    new_sector->type = type;
    new_sector->z = z;
    new_sector->tag = tag;
    new_sector->brightness = brightness;
    new_sector->fade = fade;
    new_sector->scale_x = scale_x;
    new_sector->scale_y = scale_y;
    new_sector->trans_x = trans_y;
    new_sector->rot = rot;
    new_sector->file_name = file_name;
    //ToDo hazards.
}

/* ----------------------------------------------------------------------------
 * Destroys a sector.
 */
sector::~sector() {
    for(size_t t = 0; t < 2; t++) {
        if(bitmap && bitmap != bmp_error) {
            bitmaps.detach(file_name);
        }
    }
}

/* ----------------------------------------------------------------------------
 * Creates a linedef intersection info structure.
 */
linedef_intersection::linedef_intersection(linedef* l1, linedef* l2) {
    this->l1 = l1; this->l2 = l2;
}

/* ----------------------------------------------------------------------------
 * Checks whether the linedef intersection contains the specified linedef.
 */
bool linedef_intersection::contains(linedef* l) {
    return l1 == l || l2 == l;
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
 * Creates a vertex.
 */
vertex::vertex(float x, float y) {
    this->x = x; this->y = y;
}

/* ----------------------------------------------------------------------------
 * Creates a tree shadow.
 */
tree_shadow::tree_shadow(float x, float y, float w, float h, float an, unsigned char al, string f, float sx, float sy) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->angle = an;
    this->alpha = al;
    this->sway_x = sx;
    this->sway_y = sy;
    this->file_name = f;
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
 * Returns a point's sign on a line, used for detecting if it's inside a triangle.
 */
float get_point_sign(float x, float y, float lx1, float ly1, float lx2, float ly2) {
    return (x - lx2) * (ly1 - ly2) - (lx1 - lx2) * (y - ly2);
}

/* ----------------------------------------------------------------------------
 * Returns the outer polygon and inner polygons of a sector,
 * with the vertices ordered counter-clockwise for the outer,
 * and clockwise for the inner.
 */
void get_polys(sector* s_ptr, polygon* outer, vector<polygon>* inners) {
    if(!s_ptr || !outer || !inners) return;
    
    bool doing_outer = true;
    
    //First, compile a list of all sidedefs related to this sector.
    map<linedef*, bool> lines_done;
    
    for(size_t l = 0; l < s_ptr->linedefs.size(); l++) {
        lines_done[s_ptr->linedefs[l]] = false;
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
        
        float prev_angle = M_PI; //At the start, assume the angle is 0 (right).
        
        if(!doing_outer) {
            inners->push_back(polygon());
        }
        
        while(!poly_done) {
        
            float base_angle = prev_angle - M_PI; //The angle we came from.
            
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
                float angle_dif = get_angle_dif(angle, base_angle);
                
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
                
                //If this polygon is only one vertex, though, then
                //that means it was a stray linedef. Remove it.
                //Otherwise, something just went wrong, and this is
                //a non-simple sector.
                poly_done = true;
                if(!doing_outer && inners->back().size() == 1) {
                    ed_lone_lines.insert(inners->back()[0]->linedefs[0]);
                    inners->erase(inners->begin() + inners->size() - 1);
                } else {
                    ed_non_simples.insert(s_ptr);
                }
                
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
 * Places the bounding box coordinates of a sector on the specified floats.
 */
void get_sector_bounding_box(sector* s_ptr, float* min_x, float* min_y, float* max_x, float* max_y) {
    if(!min_x || !min_y || !max_x || !max_y) return;
    *min_x = s_ptr->linedefs[0]->vertices[0]->x;
    *max_x = *min_x;
    *min_y = s_ptr->linedefs[0]->vertices[0]->y;
    *max_y = *min_y;
    
    for(size_t l = 0; l < s_ptr->linedefs.size(); l++) {
        for(unsigned char v = 0; v < 2; v++) {
            float x = s_ptr->linedefs[l]->vertices[v]->x;
            float y = s_ptr->linedefs[l]->vertices[v]->y;
            
            *min_x = min(*min_x, x);
            *max_x = max(*max_x, x);
            *min_y = min(*min_y, y);
            *max_y = max(*max_y, y);
        }
    }
}

/* ----------------------------------------------------------------------------
 * Returns which sector the specified point belongs to.
 */
sector* get_sector(float x, float y, size_t* sector_nr) {
    for(size_t s = 0; s < cur_area_map.sectors.size(); s++) {
        sector* s_ptr = cur_area_map.sectors[s];
        
        for(size_t t = 0; t < s_ptr->triangles.size(); t++) {
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
                if(sector_nr) *sector_nr = s;
                return s_ptr;
            }
        }
    }
    
    if(sector_nr) *sector_nr = string::npos;
    return NULL;
}

/* ----------------------------------------------------------------------------
 * Places the bounding box coordinates of a shadow on the specified floats.
 */
void get_shadow_bounding_box(tree_shadow* s_ptr, float* min_x, float* min_y, float* max_x, float* max_y) {
    if(!min_x || !min_y || !max_x || !max_y) return;
    bool got_min_x = false;
    bool got_max_x = false;
    bool got_min_y = false;
    bool got_max_y = false;
    
    for(unsigned char p = 0; p < 4; p++) {
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
 * Returns whether a point is inside a triangle or not.
 * px, py: Coordinates of the point to check.
 * t**:    Coordinates of the triangle's points.
 * loq:    Less or equal. Different code requires different precision for on-line cases.
   * Just... don't overthink this, I added this based on what worked and didn't.
 * Thanks go to http://stackoverflow.com/questions/2049582/how-to-determine-a-point-in-a-triangle
 */
bool is_point_in_triangle(float px, float py, float tx1, float ty1, float tx2, float ty2, float tx3, float ty3, bool loq) {
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
    
    /*float dx = px - tx1;
    float dy = py - ty1;
    
    bool s_ab = (tx2 - tx1) * dy - (ty2 - ty1) * dx > 0;
    
    if((tx3 - tx1) * dy - (ty3 - ty1) * dx > 0 == s_ab) return false;
    
    if((tx3 - tx2) * (py - ty2) - (ty3 - ty2) * (px - tx2) > 0 != s_ab) return false;
    
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
    
    return get_angle_dif(angle_prev, angle_next) < M_PI;
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
vertex* get_rightmost_vertex(map<linedef*, bool> &lines) {
    vertex* rightmost = NULL;
    
    for(auto l = lines.begin(); l != lines.end(); l++) {
        if(!rightmost) rightmost = l->first->vertices[0];
        
        for(unsigned char v = 0; v < 2; v++) {
            rightmost = get_rightmost_vertex(l->first->vertices[v], rightmost);
        }
    }
    
    return rightmost;
}

/* ----------------------------------------------------------------------------
 * Returns the vertex farthest to the right in a polygon.
 */
vertex* get_rightmost_vertex(polygon* p) {
    vertex* rightmost = NULL;
    
    for(size_t v = 0; v < p->size(); v++) {
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
 * Checks intersecting linedefs, and adds them to ed_intersecting_lines;
 */
void check_linedef_intersections(vertex* v) {
    for(size_t l = 0; l < v->linedefs.size(); l++) {
        linedef* l_ptr = v->linedefs[l];
        
        //Check if it's on the list of intersecting lines, and remove it,
        //so it can be recalculated now.
        for(size_t il = 0; il < ed_intersecting_lines.size();) {
            if(ed_intersecting_lines[il].contains(l_ptr)) {
                ed_intersecting_lines.erase(ed_intersecting_lines.begin() + il);
            } else {
                il++;
            }
        }
        
        
        if(!l_ptr->vertices[0]) continue; //It had been marked for deletion.
        
        //For every other linedef in the map, check for intersections.
        for(size_t l2 = 0; l2 < cur_area_map.linedefs.size(); l2++) {
            linedef* l2_ptr = cur_area_map.linedefs[l2];
            if(!l2_ptr->vertices[0]) continue; //It had been marked for deletion.
            
            //If the linedef is actually on the same vertex, never mind.
            if(l_ptr->vertices[0] == l2_ptr->vertices[0]) continue;
            if(l_ptr->vertices[0] == l2_ptr->vertices[1]) continue;
            if(l_ptr->vertices[1] == l2_ptr->vertices[0]) continue;
            if(l_ptr->vertices[1] == l2_ptr->vertices[1]) continue;
            
            if(
                lines_intersect(
                    l_ptr->vertices[0]->x, l_ptr->vertices[0]->y,
                    l_ptr->vertices[1]->x, l_ptr->vertices[1]->y,
                    l2_ptr->vertices[0]->x, l2_ptr->vertices[0]->y,
                    l2_ptr->vertices[1]->x, l2_ptr->vertices[1]->y,
                    NULL, NULL)
            ) {
                ed_intersecting_lines.push_back(linedef_intersection(l_ptr, l2_ptr));
            }
        }
    }
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
        if(fabs(atan2(prev_v->y - cur_v->y, prev_v->x - cur_v->x) - atan2(cur_v->y - next_v->y, cur_v->x - next_v->x)) < 0.000001) {
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
    
    for(size_t i = 0; i < inners->size(); i++) {
        polygon* p = &inners->at(i);
        vertex* closest_line_v1 = NULL;
        vertex* closest_line_v2 = NULL;
        float closest_line_ur = FLT_MAX;
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
        //If any line of the outer polygon intersects it,
        //we just find the best vertex on that line, and make the cut.
        //This line stretching right is known as a ray.
        float ray_width = outer_rightmost->x - start->x;
        
        //Let's also check the vertices.
        //If the closest thing is a vertex, not a line, then
        //we can skip a bunch of steps.
        vertex* v1 = NULL, *v2 = NULL;
        for(size_t v = 0; v < outer->size(); v++) {
            v1 = outer->at(v);
            v2 = get_next_in_vector(*outer, v);
            if(
                (v1->x >= start->x ||
                 v2->x >= start->x) &&
                (v1->x <= outer_rightmost->x ||
                 v2->x <= outer_rightmost->x)
            ) {
                float ur;
                if(lines_intersect(v1->x, v1->y, v2->x, v2->y, start->x, start->y, outer_rightmost->x, start->y, &ur, NULL)) {
                    if(!closest_line_v1 || ur < closest_line_ur) {
                        closest_line_v1 = v1;
                        closest_line_v2 = v2;
                        closest_line_ur = ur;
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
        
        if(!closest_vertex && !closest_line_v1) {
            //Some error occured.
            continue;
        }
        
        //Which is closest, a vertex or a line?
        if(closest_vertex_ur < closest_line_ur) {
            //If it's a vertex, done.
            best_vertex = closest_vertex;
        } else {
            if(!closest_line_v1) continue;
            
            //If it's a line, some more complicated steps need to be done.
            
            //We're on the line closest to the vertex.
            //Go to the rightmost vertex of this line.
            vertex* vertex_to_compare = get_rightmost_vertex(closest_line_v1, closest_line_v2);
            
            //Now get a list of all vertices inside the triangle
            //marked by the inner's vertex,
            //the point on the line,
            //and the vertex we're comparing.
            vector<vertex*> inside_triangle;
            for(size_t v = 0; v < outer->size(); v++) {
                vertex* v_ptr = outer->at(v);
                if(
                    is_point_in_triangle(
                        v_ptr->x, v_ptr->y,
                        start->x, start->y,
                        start->x + closest_line_ur * ray_width, start->y,
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
            
            for(size_t v = 0; v < inside_triangle.size(); v++) {
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
        for(size_t v = 0; v < outer->size(); v++) {
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
            float new_bridge_angle = get_angle_dif(atan2(start->y - best_vertex->y, start->x - best_vertex->x), 0);
            
            for(size_t v = 0; v < bridges.size(); v++) {
                vertex* v_ptr = outer->at(bridges[v]);
                vertex* nv_ptr = get_next_in_vector(*outer, bridges[v]);
                float a = get_angle_dif(atan2(nv_ptr->y - v_ptr->y, nv_ptr->x - v_ptr->x), 0);
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
        for(; iv < p->size(); iv++) {
            if(p->at(iv) == start) {
                break;
            }
        }
        
        auto it = p->begin() + iv;
        size_t n_after = p->size() - iv;
        //Finally, make the bridge.
        outer->insert(outer->begin() + insertion_vertex_nr + 1, it, p->end());
        outer->insert(outer->begin() + insertion_vertex_nr + 1 + n_after, p->begin(), it);
        outer->insert(outer->begin() + insertion_vertex_nr + 1 + n_after + iv, start); //Closes the inner polygon.
        
        //Before we close the inner polygon, let's
        //check if the inner's rightmost and the outer best vertices
        //are not the same.
        //This can happen if you have a square on the top-right
        //and one on the bottom-left, united by the central vertex.
        if(start != best_vertex) {
            outer->insert(outer->begin() + insertion_vertex_nr + 1 + n_after + iv + 1, best_vertex);
        }
    }
}

/* ----------------------------------------------------------------------------
 * Returns the clockwise distance between a1 and a2, in radians.
 */
float get_angle_dif(float a1, float a2) {
    a1 = normalize_angle(a1);
    a2 = normalize_angle(a2);
    if(a1 > a2) a1 -= M_PI * 2;
    return a2 - a1;
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
 * Returns whether the two lines intersect.
 * ur: Returns the distance from the start of line 2 in which the intersection happens.
   * This is in ratio, so 0 is the start, 1 is the end of the line. Oh, and the r stands for ray.
 * ul: Same as ur, but for line 1.
 */
bool lines_intersect(float l1x1, float l1y1, float l1x2, float l1y2, float l2x1, float l2y1, float l2x2, float l2y2, float* ur, float* ul) {
    float div = (l2y2 - l2y1) * (l1x2 - l1x1) - (l2x2 - l2x1) * (l1y2 - l1y1);
    
    float local_ul, local_ur;
    
    if(div != 0) {
    
        //Calculate the intersection distance from the line.
        local_ul = ((l2x2 - l2x1) * (l1y1 - l2y1) - (l2y2 - l2y1) * (l1x1 - l2x1)) / div;
        if(ul) *ul = local_ul;
        
        //Calculate the intersection distance from the ray.
        local_ur = ((l1x2 - l1x1) * (l1y1 - l2y1) - (l1y2 - l1y1) * (l1x1 - l2x1)) / div;
        if(ur) *ur = local_ur;
        
        //Return whether they intersect.
        return (local_ur >= 0) && (local_ur <= 1) && (local_ul > 0) && (local_ul < 1);
        
    } else {
        //No intersection.
        return false;
    }
}

/* ----------------------------------------------------------------------------
 * Triangulates (turns into triangles) a sector. This is because drawing concave polygons is not possible.
 */
void triangulate(sector* s_ptr) {

    //We'll triangulate with the Triangulation by Ear Clipping algorithm.
    //http://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
    
    polygon outer_poly;
    vector<polygon> inner_polys;
    
    //Before we start, let's just remove it from the
    //vector of non-simple sectors.
    auto it = ed_non_simples.find(s_ptr);
    if(it != ed_non_simples.end()) {
        ed_non_simples.erase(it);
    }
    
    //And let's clear any "lone" linedefs here.
    for(size_t l = 0; l < s_ptr->linedefs.size(); l++) {
        linedef* l_ptr = s_ptr->linedefs[l];
        auto it = ed_lone_lines.find(l_ptr);
        if(it != ed_lone_lines.end()) {
            ed_lone_lines.erase(it);
        }
    }
    
    //First, we need to know what vertices mark the outermost polygon,
    //and what vertices mark the inner ones.
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
    
    //Get rid of 0-length vertices and 180-degree vertices, as they're redundant.
    clean_poly(&outer_poly);
    for(size_t i = 0; i < inner_polys.size(); i++) clean_poly(&inner_polys[i]);
    
    //Make cuts on the outer polygon between where it and inner polygons exist, as to make it holeless.
    cut_poly(&outer_poly, &inner_polys);
    
    s_ptr->triangles.clear();
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
            ed_non_simples.insert(s_ptr);
            break;
            
        } else {
            //The ear, the previous and the next vertices make a triangle.
            s_ptr->triangles.push_back(
                triangle(
                    vertices_left[ears[0]],
                    get_prev_in_vector(vertices_left, ears[0]),
                    get_next_in_vector(vertices_left, ears[0])
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
        s_ptr->triangles.push_back(
            triangle(
                vertices_left[1], vertices_left[0], vertices_left[2]
            )
        );
    }
}
