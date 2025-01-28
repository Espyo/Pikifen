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

#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "geometry.h"
#include "sector.h"


/**
 * @brief Constructs a new edge object.
 *
 * @param v1 Its first vertex.
 * @param v2 Its second vertex.
 */
edge::edge(size_t v1, size_t v2) {
    vertex_idxs[0] = v1; vertex_idxs[1] = v2;
}


/**
 * @brief Clones an edge's properties onto another,
 * not counting the sectors or vertexes.
 *
 * @param destination Edge to clone the data into.
 */
void edge::clone(edge* destination) const {
    destination->wall_shadow_length = wall_shadow_length;
    destination->wall_shadow_color = wall_shadow_color;
    destination->ledge_smoothing_length = ledge_smoothing_length;
    destination->ledge_smoothing_color = ledge_smoothing_color;
}


/**
 * @brief Returns the sector that ISN'T the specified one.
 *
 * @param s_ptr The sector that ISN'T the one to return.
 * @return The other sector.
 */
sector* edge::get_other_sector(const sector* s_ptr) const {
    if(sectors[0] == s_ptr) return sectors[1];
    return sectors[0];
}


/**
 * @brief Returns the vertex that ISN'T the specified one.
 *
 * @param v_ptr The vertex that ISN'T the one to return.
 * @return The other vertex.
 */
vertex* edge::get_other_vertex(const vertex* v_ptr) const {
    if(vertexes[0] == v_ptr) return vertexes[1];
    return vertexes[0];
}


/**
 * @brief Returns which side has the specified sector, if any.
 *
 * @param s_ptr Sector to check.
 * @return The side index, or INVALID if neither.
 */
size_t edge::get_side_with_sector(const sector* s_ptr) const {
    for(unsigned char s = 0; s < 2; s++) {
        if(sectors[s] == s_ptr) return s;
    }
    return INVALID;
}


/**
 * @brief If the specified edge and this one are not neighbors, returns nullptr.
 * Otherwise, returns the vertex that binds them.
 *
 * @param other Edge to check as a neighbor.
 * @return The binding vertex, or nullptr if they are not neighbors.
 */
vertex* edge::has_neighbor(const edge* other) const {
    for(size_t v1 = 0; v1 < 2; v1++) {
        for(size_t v2 = 0; v2 < 2; v2++) {
            if(vertexes[v1] == other->vertexes[v2]) {
                return vertexes[v1];
            }
        }
    }
    return nullptr;
}


/**
 * @brief Returns whether or not an edge is valid.
 * An edge is valid if it has non-nullptr vertexes.
 *
 * @return Whether it is valid.
 */
bool edge::is_valid() const {
    if(!vertexes[0]) return false;
    if(!vertexes[1]) return false;
    return true;
}


/**
 * @brief Removes the edge from its sectors, but doesn't mark
 * the sectors as "none".
 *
 * @return The edge index.
 */
size_t edge::remove_from_sectors() {
    size_t e_idx = INVALID;
    for(unsigned char s = 0; s < 2; s++) {
        sector* s_ptr = sectors[s];
        if(!s_ptr) continue;
        for(size_t e = 0; e < s_ptr->edges.size(); e++) {
            edge* e_ptr = s_ptr->edges[e];
            if(e_ptr == this) {
                s_ptr->edges.erase(s_ptr->edges.begin() + e);
                auto nr_it = s_ptr->edge_idxs.begin() + e;
                e_idx = *nr_it;
                s_ptr->edge_idxs.erase(nr_it);
                break;
            }
        }
        sectors[s] = nullptr;
        sector_idxs[s] = INVALID;
    }
    return e_idx;
}


/**
 * @brief Removes the edge from its vertexes, but doesn't mark
 * the vertexes as "none".
 *
 * @return The edge index.
 */
size_t edge::remove_from_vertexes() {
    size_t e_idx = INVALID;
    for(unsigned char v = 0; v < 2; v++) {
        vertex* v_ptr = vertexes[v];
        if(!v_ptr) continue;
        for(size_t e = 0; e < v_ptr->edges.size(); e++) {
            edge* e_ptr = v_ptr->edges[e];
            if(e_ptr == this) {
                v_ptr->edges.erase(v_ptr->edges.begin() + e);
                auto nr_it = v_ptr->edge_idxs.begin() + e;
                e_idx = *nr_it;
                v_ptr->edge_idxs.erase(nr_it);
                break;
            }
        }
        vertexes[v] = nullptr;
        vertex_idxs[v] = INVALID;
    }
    return e_idx;
}


/**
 * @brief Swaps the two vertexes of the edge around. It also swaps the sectors,
 * so that they still point in the right direction.
 */
void edge::swap_vertexes() {
    std::swap(vertexes[0], vertexes[1]);
    std::swap(vertex_idxs[0], vertex_idxs[1]);
    std::swap(sectors[0], sectors[1]);
    std::swap(sector_idxs[0], sector_idxs[1]);
}


/**
 * @brief Transfers this edge from one sector to a different one.
 *
 * @param from Sector to transfer from.
 * @param to Sector to transfer to.
 * @param to_idx Index of the sector to transfer to.
 * @param edge_idx Index of the current edge.
 */
void edge::transfer_sector(
    sector* from, sector* to, size_t to_idx, size_t edge_idx
) {
    size_t idx = get_side_with_sector(from);
    engine_assert(
        idx != INVALID,
        i2s(to_idx)
    );
    
    sectors[idx] = to;
    sector_idxs[idx] = to_idx;
    
    if(from) from->remove_edge(this);
    if(to) to->add_edge(this, edge_idx);
}


/**
 * @brief Constructs a new edge intersection object.
 *
 * @param e1 First edge in the intersection.
 * @param e2 Second edge in the intersection.
 */
edge_intersection::edge_intersection(edge* e1, edge* e2) :
    e1(e1),
    e2(e2) {
    
}


/**
 * @brief Checks whether the edge intersection contains the specified edge.
 *
 * @param e Edge to check.
 * @return Whether it contains the edge.
 */
bool edge_intersection::contains(const edge* e) {
    return e1 == e || e2 == e;
}
