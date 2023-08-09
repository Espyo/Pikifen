/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Sector class and related functions.
 */


#include <algorithm>

#include "sector.h"

#include "../game.h"
#include "geometry.h"


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
 * destination:
 *   Sector to clone the data into.
 */
void sector::clone(sector* destination) const {
    destination->type = type;
    destination->is_bottomless_pit = is_bottomless_pit;
    destination->z = z;
    destination->tag = tag;
    destination->hazards = hazards;
    destination->hazard_floor = hazard_floor;
    destination->hazards_str = hazards_str;
    destination->brightness = brightness;
    destination->texture_info.scale = texture_info.scale;
    destination->texture_info.translation = texture_info.translation;
    destination->texture_info.rot = texture_info.rot;
    destination->texture_info.tint = texture_info.tint;
    destination->fade = fade;
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
void sector::remove_edge(const edge* e_ptr) {
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
