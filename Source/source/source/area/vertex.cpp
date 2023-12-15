/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Edge vertex class and related functions.
 */

#define _USE_MATH_DEFINES

#include "vertex.h"

#include "edge.h"


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
edge* vertex::get_edge_by_neighbor(const vertex* neighbor) const {
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
bool vertex::has_edge(const edge* e_ptr) const {
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
    const vertex* other_v, vertex** first_neighbor
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
    const edge* other_e, vertex** first_neighbor
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
bool vertex::is_neighbor(const vertex* other_v) const {
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
void vertex::remove_edge(const edge* e_ptr) {
    size_t i = 0;
    for(; i < edges.size(); ++i) {
        if(edges[i] == e_ptr) {
            edges.erase(edges.begin() + i);
            edge_nrs.erase(edge_nrs.begin() + i);
            return;
        }
    }
}
