/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
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

#include "editors/area_editor.h"
#include "functions.h"
#include "geometry_utils.h"
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
 * Connects an edge to a sector, adding the sector and its number to the edge's
 * lists, and adding the edge and its number to the sector's.
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
 */
size_t area_data::find_edge_nr(const edge* e_ptr) {
    for(size_t e = 0; e < edges.size(); ++e) {
        if(edges[e] == e_ptr) return e;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Scans the list of sectors and retrieves the number of the specified sector.
 * Returns INVALID if not found.
 */
size_t area_data::find_sector_nr(const sector* s_ptr) {
    for(size_t s = 0; s < sectors.size(); ++s) {
        if(sectors[s] == s_ptr) return s;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Scans the list of vertexes and retrieves the number of the specified vertex.
 * Returns INVALID if not found.
 */
size_t area_data::find_vertex_nr(const vertex* v_ptr) {
    for(size_t v = 0; v < vertexes.size(); ++v) {
        if(vertexes[v] == v_ptr) return v;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Fixes the sector and vertex numbers in an edge,
 * making them match the correct sectors and vertexes,
 * based on the existing sector and vertex pointers.
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
 */
void area_data::fix_path_stop_nrs(path_stop* s_ptr) {
    for(size_t l = 0; l < s_ptr->links.size(); ++l) {
        path_link* l_ptr = &s_ptr->links[l];
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
 */
void area_data::fix_path_stop_pointers(path_stop* s_ptr) {
    for(size_t l = 0; l < s_ptr->links.size(); ++l) {
        path_link* l_ptr = &s_ptr->links[l];
        l_ptr->end_ptr = NULL;
        
        if(l_ptr->end_nr == INVALID) continue;
        if(l_ptr->end_nr >= path_stops.size()) continue;
        
        l_ptr->end_ptr = path_stops[l_ptr->end_nr];
    }
}


/* ----------------------------------------------------------------------------
 * Fixes the edge numbers in a sector, making them match the correct edges,
 * based on the existing edge pointers.
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
        min_coords.x = min(v_ptr->x, min_coords.x);
        max_coords.x = max(v_ptr->x, max_coords.x);
        min_coords.y = min(v_ptr->y, min_coords.y);
        max_coords.y = max(v_ptr->y, max_coords.y);
    }
    
    bmap.top_left_corner = min_coords;
    //Add one more to the cols/rows because, suppose there's an edge at y = 256.
    //The row would be 2. In reality, the row should be 3.
    bmap.n_cols = ceil((max_coords.x - min_coords.x) / BLOCKMAP_BLOCK_SIZE) + 1;
    bmap.n_rows = ceil((max_coords.y - min_coords.y) / BLOCKMAP_BLOCK_SIZE) + 1;
    
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
    //that the block has no edges. It has, however, a single sector (or NULL),
    //so use the triangle method to get the sector. Checking the center is
    //just a good a spot as any.
    for(size_t bx = 0; bx < bmap.n_cols; ++bx) {
        for(size_t by = 0; by < bmap.n_rows; ++by) {
        
            if(bmap.sectors[bx][by].empty()) {
            
                point corner = bmap.get_top_left_corner(bx, by);
                corner += BLOCKMAP_BLOCK_SIZE * 0.5;
                bmap.sectors[bx][by].insert(
                    get_sector(corner, NULL, false)
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
                point corner = bmap.get_top_left_corner(bx, by);
                
                //Check if the edge is inside this blockmap.
                if(
                    rectangle_intersects_line(
                        corner,
                        corner + BLOCKMAP_BLOCK_SIZE,
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
 * Removes a vertex from the list, and updates all IDs referencing it.
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
                assert(e_ptr->vertex_nrs[v] != v_nr);
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Removes a vertex from the list, and updates all IDs referencing it.
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
 * Removes an edge from the list, and updates all IDs referencing it.
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
                assert(v_ptr->edge_nrs[e] != e_nr);
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
                assert(s_ptr->edge_nrs[e] != e_nr);
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Removes an edge from the list, and updates all IDs referencing it.
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
 */
void area_data::remove_sector(const size_t s_nr) {
    sectors.erase(sectors.begin() + s_nr);
    for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
        edge* e_ptr = cur_area_data.edges[e];
        for(size_t s = 0; s < 2; ++s) {
            if(
                e_ptr->sector_nrs[s] != INVALID &&
                e_ptr->sector_nrs[s] > s_nr
            ) {
                e_ptr->sector_nrs[s]--;
            } else {
                //This should never happen.
                assert(e_ptr->sector_nrs[s] != s_nr);
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Removes a sector from the list, and updates all IDs referencing it.
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
 * A debugging tool. This checks to see if all numbers match their pointers,
 * for the various edges, vertexes, etc. Aborts execution if any doesn't.
 */
void area_data::check_matches() {
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
            assert(v_ptr->edges[e] == edges[v_ptr->edge_nrs[e]]);
        }
    }
    
    for(size_t e = 0; e < edges.size(); ++e) {
        edge* e_ptr = edges[e];
        for(size_t v = 0; v < 2; ++v) {
            vertex* v_ptr = e_ptr->vertexes[v];
            assert(e_ptr->vertexes[v] == vertexes[e_ptr->vertex_nrs[v]]);
        }
        
        for(size_t s = 0; s < 2; ++s) {
            sector* s_ptr = e_ptr->sectors[s];
            if(
                e_ptr->sectors[s] == NULL &&
                e_ptr->sector_nrs[s] == INVALID
            ) {
                continue;
            }
            assert(e_ptr->sectors[s] == sectors[e_ptr->sector_nrs[s]]);
        }
    }
    
    for(size_t s = 0; s < sectors.size(); ++s) {
        sector* s_ptr = sectors[s];
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            assert(s_ptr->edges[e] == edges[s_ptr->edge_nrs[e]]);
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
    
    if(!bg_bmp_file_name.empty()) {
        bitmaps.detach(bg_bmp_file_name);
    }
    
    name.clear();
    subtitle.clear();
    weather_name.clear();
    bg_bmp_file_name.clear();
    bg_color = al_map_rgb(0, 0, 0);
    bg_dist = 2.0f;
    bg_bmp_zoom = 1.0f;
    
    reference_alpha = 255;
    reference_center = point();
    reference_size = point();
    reference_file_name.clear();
}


/* ----------------------------------------------------------------------------
 * Clones this area data into another area_data object.
 */
void area_data::clone(area_data &other) {
    other.clear();
    
    other.bg_bmp_file_name = bg_bmp_file_name;
    other.bg_bmp = bitmaps.get(bg_bmp_file_name, NULL, false);
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
    }
    
    for(size_t s = 0; s < sectors.size(); ++s) {
        sector* s_ptr = sectors[s];
        sector* os_ptr = other.sectors[s];
        s_ptr->clone(os_ptr);
        os_ptr->texture_info.file_name = s_ptr->texture_info.file_name;
        os_ptr->texture_info.bitmap =
            bitmaps.get(
                TEXTURES_FOLDER_NAME + "/" + s_ptr->texture_info.file_name,
                NULL, false
            );
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
    }
    
    for(size_t m = 0; m < mob_generators.size(); ++m) {
        mob_gen* m_ptr = mob_generators[m];
        mob_gen* om_ptr = other.mob_generators[m];
        om_ptr->angle = m_ptr->angle;
        om_ptr->category = m_ptr->category;
        om_ptr->pos = m_ptr->pos;
        om_ptr->type = m_ptr->type;
        om_ptr->vars = m_ptr->vars;
    }
    
    for(size_t s = 0; s < path_stops.size(); ++s) {
        path_stop* s_ptr = path_stops[s];
        path_stop* os_ptr = other.path_stops[s];
        os_ptr->links.reserve(s_ptr->links.size());
        for(size_t l = 0; l < s_ptr->links.size(); ++l) {
            os_ptr->links.push_back(
                path_link(
                    other.path_stops[s_ptr->links[l].end_nr],
                    s_ptr->links[l].end_nr
                )
            );
            os_ptr->links.back().distance = s_ptr->links[l].distance;
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
        ot_ptr->bitmap =
            bitmaps.get(
                TEXTURES_FOLDER_NAME + "/" + t_ptr->file_name, NULL, false
            );
    }
    
    other.name = name;
    other.subtitle = subtitle;
    other.weather_name = weather_name;
    other.weather_condition = weather_condition;
    
    other.reference_alpha = reference_alpha;
    other.reference_center = reference_center;
    other.reference_file_name = reference_file_name;
    other.reference_size = reference_size;
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
}


/* ----------------------------------------------------------------------------
 * Returns the block column in which an X coordinate is contained.
 * Returns INVALID on error.
 */
size_t blockmap::get_col(const float x) {
    if(x < top_left_corner.x) return INVALID;
    float final_x = (x - top_left_corner.x) / BLOCKMAP_BLOCK_SIZE;
    if(final_x >= n_cols) return INVALID;
    return final_x;
}


/* ----------------------------------------------------------------------------
 * Returns the block row in which a Y coordinate is contained.
 * Returns INVALID on error.
 */
size_t blockmap::get_row(const float y) {
    if(y < top_left_corner.y) return INVALID;
    float final_y = (y - top_left_corner.y) / BLOCKMAP_BLOCK_SIZE;
    if(final_y >= n_rows) return INVALID;
    return final_y;
}


/* ----------------------------------------------------------------------------
 * Returns the top-left coordinates for the specified column and row.
 */
point blockmap::get_top_left_corner(const size_t col, const size_t row) {
    return
        point(
            col * BLOCKMAP_BLOCK_SIZE + top_left_corner.x,
            row * BLOCKMAP_BLOCK_SIZE + top_left_corner.y
        );
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
 * Returns the vertex that ISN'T the specified one.
 */
vertex* edge::get_other_vertex(vertex* v_ptr) {
    if(vertexes[0] == v_ptr) return vertexes[1];
    return vertexes[0];
}


/* ----------------------------------------------------------------------------
 * Returns which side has the specified sector, or INVALID if neither.
 */
size_t edge::get_side_with_sector(sector* s_ptr) {
    for(unsigned char s = 0; s < 2; ++s) {
        if(sectors[s] == s_ptr) return s;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * If the specified edge and this one are not neighbors, returns NULL.
 * Otherwise, returns the vertex that binds them.
 */
vertex* edge::has_neighbor(edge* other) {
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
    }
    return e_nr;
}


/* ----------------------------------------------------------------------------
 * Swaps the two vertexes of the edge around. It also swaps the sectors,
 * so that they still point in the right direction.
 */
void edge::swap_vertexes() {
    swap(vertexes[0], vertexes[1]);
    swap(vertex_nrs[0], vertex_nrs[1]);
    swap(sectors[0], sectors[1]);
    swap(sector_nrs[0], sector_nrs[1]);
}


/* ----------------------------------------------------------------------------
 * Creates a mob generation structure.
 */
mob_gen::mob_gen(
    mob_category* category, const point &pos,
    mob_type* type, const float angle, const string &vars
) :
    category(category),
    type(type),
    pos(pos),
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
    always_cast_shadow(false),
    associated_liquid(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Adds an edge to the sector's list of edges, if it's not there already.
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
 * Clones a sector's properties onto another,
 * not counting the list of edges or bitmap
 * (the file name is cloned too, though).
 */
void sector::clone(sector* new_sector) {
    new_sector->type = type;
    new_sector->z = z;
    new_sector->tag = tag;
    new_sector->hazard_floor = hazard_floor;
    new_sector->hazards_str = hazards_str;
    new_sector->brightness = brightness;
    new_sector->texture_info.scale = texture_info.scale;
    new_sector->texture_info.translation = texture_info.translation;
    new_sector->texture_info.rot = texture_info.rot;
    new_sector->texture_info.tint = texture_info.tint;
    new_sector->always_cast_shadow = always_cast_shadow;
    new_sector->fade = fade;
}


/* ----------------------------------------------------------------------------
 * If texture merging is required, this returns what two neighboring sectors
 * will be used for it.
 */
void sector::get_texture_merge_sectors(sector** s1, sector** s2) {
    //Check all edges to find which two textures need merging.
    edge* e_ptr = NULL;
    sector* neighbor = NULL;
    bool valid = true;
    map<sector*, dist> neighbors;
    sector* texture_sector[2] = {NULL, NULL};
    
    //The two neighboring sectors with the lenghtiest edges are picked.
    //So save all sector/length pairs.
    //Sectors with different heights from the current one are also saved,
    //but they have lower priority compared to same-heigh sectors.
    for(size_t e = 0; e < edges.size(); ++e) {
        e_ptr = edges[e];
        valid = true;
        
        if(e_ptr->sectors[0] == this) neighbor = e_ptr->sectors[1];
        else neighbor = e_ptr->sectors[0];
        
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
    vector<pair<dist, sector*> > neighbors_vec;
    for(auto n = neighbors.begin(); n != neighbors.end(); ++n) {
        neighbors_vec.push_back(
            make_pair(
                //Yes, we do need these casts, for g++.
                (dist) (n->second), (sector*) (n->first)
            )
        );
    }
    sort(
        neighbors_vec.begin(), neighbors_vec.end(),
    [this] (pair<dist, sector*> p1, pair<dist, sector*> p2) -> bool {
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
        swap(texture_sector[0], texture_sector[1]);
    } else if(!texture_sector[1]) {
        //Nothing to draw.
        return;
    } else if(texture_sector[1]->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
        swap(texture_sector[0], texture_sector[1]);
    }
    
    *s1 = texture_sector[0];
    *s2 = texture_sector[1];
}


/* ----------------------------------------------------------------------------
 * Removes an edge from a sector's list of edges, if it is there.
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
    scale(1.0, 1.0),
    rot(0),
    bitmap(nullptr),
    tint(al_map_rgb(255, 255, 255)) {
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
path_stop::path_stop(const point &pos, vector<path_link> links) :
    pos(pos),
    links(links) {
    
}


/* ----------------------------------------------------------------------------
 * Adds a link between this stop and another, whether it's one-way or not.
 * Also adds the link to the other stop, if applicable.
 * If these two stops already had some link, it gets removed.
 * other_stop: Pointer to the other stop.
 * normal:     Normal link? False means one-way link.
 */
void path_stop::add_link(path_stop* other_stop, const bool normal) {
    remove_link(other_stop);
    if(other_stop->has_link(this)) {
        other_stop->remove_link(this);
    }
    
    links.push_back(path_link(other_stop, INVALID));
    if(normal) {
        other_stop->links.push_back(path_link(this, INVALID));
    }
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
 * Removes the link between this stop and the specified one.
 */
void path_stop::remove_link(path_stop* other_stop) {
    for(size_t l = 0; l < links.size(); ++l) {
        if(links[l].end_ptr == other_stop) {
            links.erase(links.begin() + l);
            return;
        }
    }
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
    distance = dist(start_ptr->pos, end_ptr->pos).to_float();
}


/* ----------------------------------------------------------------------------
 * Creates a tree shadow.
 */
tree_shadow::tree_shadow(
    const point &center, const point &size, const float angle,
    const unsigned char alpha, const string &file_name, const point &sway
) :
    center(center),
    size(size),
    angle(angle),
    alpha(alpha),
    sway(sway),
    file_name(file_name),
    bitmap(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a tree shadow.
 */
tree_shadow::~tree_shadow() {
    bitmaps.detach(TEXTURES_FOLDER_NAME + "/" + file_name);
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
 * Creates a vertex.
 */
vertex::vertex(float x, float y) :
    x(x),
    y(y) {
    
}


/* ----------------------------------------------------------------------------
 * Adds an edge to the vertex's list of edges, if it's not there already.
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
 */
edge* vertex::get_edge_by_neighbor(vertex* neighbor) {
    for(size_t e = 0; e < edges.size(); ++e) {
        if(edges[e]->get_other_vertex(this) == neighbor) {
            return edges[e];
        }
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not this vertex has the specified edge in its list.
 */
bool vertex::has_edge(edge* e_ptr) {
    for(size_t e = 0; e < edges.size(); ++e) {
        if(edges[e] == e_ptr) return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Removes an edge from a vertex's list of edges, if it is there.
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
 * Returns the closest vertex that can be merged with the specified point.
 * Returns NULL if there is no vertex close enough to merge.
 * point:        Coordinates of the point.
 * all_vertexes: Vector with all of the vertexes in the area.
 * merge_radius: Minimum radius to merge.
 * v_nr:         If not NULL, the vertex's number is returned here.
 * ignore:       Ignore this vertex when checking, if not NULL.
 */
vertex* get_merge_vertex(
    const point &pos, vector<vertex*> &all_vertexes,
    const float merge_radius, size_t* v_nr, vertex* ignore
) {
    dist closest_dist = 0;
    vertex* closest_v = NULL;
    size_t closest_nr = INVALID;
    
    for(size_t v = 0; v < all_vertexes.size(); ++v) {
        vertex* v_ptr = all_vertexes[v];
        if(v_ptr == ignore) continue;
        dist d(pos, point(v_ptr->x, v_ptr->y));
        if(
            d <= merge_radius &&
            (d < closest_dist || !closest_v)
        ) {
            closest_dist = d;
            closest_v = v_ptr;
            closest_nr = v;
        }
    }
    
    if(v_nr) *v_nr = closest_nr;
    return closest_v;
}


/* ----------------------------------------------------------------------------
 * Returns the shortest available path between two points, following
 * the area's path graph.
 * start:          Start coordinates.
 * end:            End coordinates.
 * obstacle_found: If an obstacle was found in the only path, this points to it.
 * go_straight:    This is set according to whether it's better
 *   to go straight to the end point.
 * total_dist:     If not NULL, place the total path distance here.
 */
vector<path_stop*> get_path(
    const point &start, const point &end,
    mob** obstacle_found, bool* go_straight,
    float* total_dist
) {

    vector<path_stop*> full_path;
    
    if(cur_area_data.path_stops.empty()) {
        if(go_straight) *go_straight = true;
        return full_path;
    } else {
        if(go_straight) *go_straight = false;
    }
    
    //Start by finding the closest stops to the start and finish.
    path_stop* closest_to_start = NULL;
    path_stop* closest_to_end = NULL;
    dist closest_to_start_dist;
    dist closest_to_end_dist;
    
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];
        
        dist dist_to_start(start, s_ptr->pos);
        dist dist_to_end(end, s_ptr->pos);
        
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
    dist start_to_end_dist = dist(start, end);
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
            dist(start, full_path[0]->pos).to_float();
        *total_dist +=
            dist(full_path[full_path.size() - 1]->pos, end).to_float();
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
                m_ptr->pos,
                m_ptr->type->radius,
                s1->pos, s2->pos
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
float get_point_sign(const point &p, const point &lp1, const point &lp2) {
    return (p.x - lp2.x) * (lp1.y - lp2.y) - (lp1.x - lp2.x) * (p.y - lp2.y);
}


/* ----------------------------------------------------------------------------
 * Returns the outer polygon and inner polygons of a sector,
 * with the vertexes ordered counter-clockwise for the outer,
 * and clockwise for the inner.
 * s_ptr:              Pointer to the sector.
 * outer:              Return the outer polygon here.
 * inners:             Return the inner polygons here.
 * lone_edges:         Return any lone edges found here.
 * check_vertex_reuse: True if the algorithm is meant to check for vertexes
 *   that get reused.
 * Returns a number based on what happened. See TRIANGULATION_ERRORS.
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
                if(!doing_outer && inners->back().size() == 1) {
                    if(lone_edges) {
                        lone_edges->insert(inners->back()[0]->edges[0]);
                    }
                    inners->erase(inners->begin() + inners->size() - 1);
                } else {
                    result = TRIANGULATION_ERROR_LONE_EDGES;
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
    if(check_vertex_reuse) {
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
                result = TRIANGULATION_ERROR_VERTEXES_REUSED;
                break;
            }
        }
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Places the bounding box coordinates of a sector on the specified floats.
 */
void get_sector_bounding_box(
    sector* s_ptr, point* min_coords, point* max_coords
) {
    if(!min_coords || !max_coords) return;
    min_coords->x = s_ptr->edges[0]->vertexes[0]->x;
    max_coords->x = min_coords->x;
    min_coords->y = s_ptr->edges[0]->vertexes[0]->y;
    max_coords->y = min_coords->y;
    
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        for(unsigned char v = 0; v < 2; ++v) {
            point coords(
                s_ptr->edges[e]->vertexes[v]->x,
                s_ptr->edges[e]->vertexes[v]->y
            );
            
            min_coords->x = min(min_coords->x, coords.x);
            max_coords->x = max(max_coords->x, coords.x);
            min_coords->y = min(min_coords->y, coords.y);
            max_coords->y = max(max_coords->y, coords.y);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns which sector the specified point belongs to.
 * p:            Coordinates of the point.
 * sector_nr:    If not NULL, the number of the sector
 *   on the area map is placed here.
 *   The number will not be set if the search is using the blockmap.
 * use_blockmap: If true, use the blockmap to search.
 *   This provides faster results, but the blockmap must be built.
 */
sector* get_sector(
    const point &p, size_t* sector_nr, const bool use_blockmap
) {

    if(use_blockmap) {
    
        size_t col = cur_area_data.bmap.get_col(p.x);
        size_t row = cur_area_data.bmap.get_row(p.y);
        if(col == INVALID || row == INVALID) return NULL;
        
        unordered_set<sector*>* sectors = &cur_area_data.bmap.sectors[col][row];
        
        if(sectors->size() == 1) return *sectors->begin();
        
        for(auto s = sectors->begin(); s != sectors->end(); ++s) {
        
            if(!(*s)) {
                continue;
            }
            if(is_point_in_sector(p, *s)) {
                return *s;
            }
        }
        
        return NULL;
        
    } else {
    
        for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
            sector* s_ptr = cur_area_data.sectors[s];
            
            if(is_point_in_sector(p, s_ptr)) {
                if(sector_nr) *sector_nr = s;
                return s_ptr;
            }
            
        }
        
        if(sector_nr) *sector_nr = INVALID;
        return NULL;
        
    }
}


/* ----------------------------------------------------------------------------
 * Places the bounding box coordinates of a shadow
 * on the specified point structs.
 */
void get_shadow_bounding_box(
    tree_shadow* s_ptr, point* min_coords, point* max_coords
) {

    if(!min_coords || !max_coords) return;
    bool got_min_x = false;
    bool got_max_x = false;
    bool got_min_y = false;
    bool got_max_y = false;
    
    for(unsigned char p = 0; p < 4; ++p) {
        point corner, final_corner;
        
        if(p == 0 || p == 1) corner.x = s_ptr->center.x - (s_ptr->size.x * 0.5);
        else                 corner.x = s_ptr->center.x + (s_ptr->size.x * 0.5);
        if(p == 0 || p == 2) corner.y = s_ptr->center.y - (s_ptr->size.y * 0.5);
        else                 corner.y = s_ptr->center.y + (s_ptr->size.y * 0.5);
        
        corner -= s_ptr->center;
        final_corner = rotate_point(corner, s_ptr->angle);
        final_corner += s_ptr->center;
        
        if(final_corner.x < min_coords->x || !got_min_x) {
            min_coords->x = final_corner.x;
            got_min_x = true;
        }
        if(final_corner.y < min_coords->y || !got_min_y) {
            min_coords->y = final_corner.y;
            got_min_y = true;
        }
        if(final_corner.x > max_coords->x || !got_max_x) {
            max_coords->x = final_corner.x;
            got_max_x = true;
        }
        if(final_corner.y > max_coords->y || !got_max_y) {
            max_coords->y = final_corner.y;
            got_max_y = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether or not an edge is valid.
 * An edge is valid if it has non-NULL vertexes.
 */
bool is_edge_valid(edge* l) {
    if(!l->vertexes[0]) return false;
    if(!l->vertexes[1]) return false;
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns whether a point is inside a sector by checking its triangles.
 */
bool is_point_in_sector(const point &p, sector* s_ptr) {
    for(size_t t = 0; t < s_ptr->triangles.size(); ++t) {
        triangle* t_ptr = &s_ptr->triangles[t];
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
 * Returns whether a point is inside a triangle or not.
 * p:      The point to check.
 * tp*:    Coordinates of the triangle's points.
 * loq:    Less or equal.
 *   Different code requires different precision for on-line cases.
 *   Just...don't overthink this, I added this based on what worked and didn't.
 * Thanks go to
 *   http://stackoverflow.com/questions/2049582/
 *   how-to-determine-a-point-in-a-triangle
 */
bool is_point_in_triangle(
    const point &p, const point &tp1, const point &tp2, const point &tp3,
    const bool loq
) {

    bool b1, b2, b3;
    
    float f1, f2, f3;
    
    f1 = get_point_sign(p, tp1, tp2);
    f2 = get_point_sign(p, tp2, tp3);
    f3 = get_point_sign(p, tp3, tp1);
    
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
}


/* ----------------------------------------------------------------------------
 * Returns whether a polygon was created clockwise or anti-clockwise,
 * given the order of its vertexes.
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
 * Returns whether a sector's vertexes are ordered clockwise or not.
 */
bool is_sector_clockwise(sector* s_ptr) {
    vector<vertex*> vertexes;
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        vertexes.push_back(s_ptr->edges[e]->vertexes[0]);
    }
    return is_polygon_clockwise(vertexes);
}


/* ----------------------------------------------------------------------------
 * Returns whether this vertex is convex or not.
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
 * Returns the vertex farthest to the right in a list of edges.
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
 * Returns the vertex farthest to the right in a sector.
 */
vertex* get_rightmost_vertex(sector* s) {
    vertex* rightmost = NULL;
    
    for(size_t e = 0; e < s->edges.size(); ++e) {
        edge* e_ptr = s->edges[e];
        if(!rightmost) rightmost = e_ptr->vertexes[0];
        else {
            rightmost = get_rightmost_vertex(e_ptr->vertexes[0], rightmost);
            rightmost = get_rightmost_vertex(e_ptr->vertexes[1], rightmost);
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
                        point(v1->x, v1->y), point(v2->x, v2->y),
                        point(start->x, start->y),
                        point(outer_rightmost->x, start->y),
                        &ur, NULL
                    )
                ) {
                    if(!closest_edge_v1 || ur < closest_edge_ur) {
                        closest_edge_v1 = v1;
                        closest_edge_v2 = v2;
                        closest_edge_ur = ur;
                    }
                }
                
                if(v1->y == start->y && v1->x >= start->x) {
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
                        point(v_ptr->x, v_ptr->y),
                        point(start->x, start->y),
                        point(start->x + closest_edge_ur * ray_width, start->y),
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
                    get_angle(
                        point(best_vertex->x, best_vertex->y),
                        point(start->x, start->y)
                    ),
                    0.0f
                );
                
            for(size_t v = 0; v < bridges.size(); ++v) {
                vertex* v_ptr = outer->at(bridges[v]);
                vertex* nv_ptr = get_next_in_vector(*outer, bridges[v]);
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
            dist d(start_node->pos, obstacles_found[o].first->pos);
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
 *   the intersection happens.
 *   This is a ratio, so 0 is the start, 1 is the end of the line.
 *   Oh, and the r stands for ray.
 * ul: Same as ur, but for line 1.
 */
bool lines_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    float* ur, float* ul
) {

    float div =
        (l2p2.y - l2p1.y) * (l1p2.x - l1p1.x) -
        (l2p2.x - l2p1.x) * (l1p2.y - l1p1.y);
        
    if(div != 0) {
    
        float local_ul, local_ur;
        
        //Calculate the intersection distance from the line.
        local_ul =
            (
                (l2p2.x - l2p1.x) * (l1p1.y - l2p1.y) -
                (l2p2.y - l2p1.y) * (l1p1.x - l2p1.x)
            ) / div;
        if(ul) *ul = local_ul;
        
        //Calculate the intersection distance from the ray.
        local_ur =
            (
                (l1p2.x - l1p1.x) * (l1p1.y - l2p1.y) -
                (l1p2.y - l1p1.y) * (l1p1.x - l2p1.x)
            ) / div;
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
 * Returns whether the two line segments intersect.
 * insersection: Return the intersection point here, if not NULL.
 */
bool lines_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    point* intersection
) {
    float ur;
    if(intersection) {
        intersection->x = 0.0f;
        intersection->y = 0.0f;
    }
    if(!lines_intersect(l1p1, l1p2, l2p1, l2p2, &ur, NULL)) return NULL;
    if(intersection) {
        intersection->x = l2p1.x + (l2p2.x - l2p1.x) * ur;
        intersection->y = l2p1.y + (l2p2.y - l2p1.y) * ur;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Triangulates (turns into triangles) a sector.
 * This is because drawing concave polygons is not possible.
 * s_ptr:              Pointer to the sector.
 * lone_edges:         Return lone edges found here.
 * check_vertex_reuse: True if the algorithm is meant to check for vertexes
 *   that get reused.
 * clear_lone_edges:   Clear this sector's edges from the list of lone edges,
 *   if they are there.
 * Returns a number based on what happened. See TRIANGULATION_ERRORS.
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
