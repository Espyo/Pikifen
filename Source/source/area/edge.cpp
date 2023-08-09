/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Sector edge class and related functions.
 */

#define _USE_MATH_DEFINES

#include "edge.h"

#include "../functions.h"
#include "../utils/string_utils.h"
#include "geometry.h"
#include "sector.h"


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
 * destination:
 *   Edge to clone the data into.
 */
void edge::clone(edge* destination) const {
    destination->wall_shadow_length = wall_shadow_length;
    destination->wall_shadow_color = wall_shadow_color;
    destination->ledge_smoothing_length = ledge_smoothing_length;
    destination->ledge_smoothing_color = ledge_smoothing_color;
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
size_t edge::get_side_with_sector(const sector* s_ptr) const {
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
bool edge_intersection::contains(const edge* e) {
    return e1 == e || e2 == e;
}
