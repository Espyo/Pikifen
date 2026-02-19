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


/**
 * @brief Constructs a new vertex object.
 *
 * @param x X coordinate.
 * @param y Y coordinate.
 */
Vertex::Vertex(float x, float y) :
    x(x),
    y(y) {
    
}


/**
 * @brief Adds an existing edge to the vertex's list of edges,
 * if it's not there already.
 *
 * @param ePtr Edge to add.
 * @param eIdx Index number of the edge to add.
 */
void Vertex::addEdge(Edge* ePtr, size_t eIdx) {
    for(size_t i = 0; i < edges.size(); i++) {
        if(edges[i] == ePtr) {
            return;
        }
    }
    edges.push_back(ePtr);
    edgeIdxs.push_back(eIdx);
}


/**
 * @brief Returns the edge that has the specified vertex as a neighbor
 * of this vertex.
 *
 * @param neighbor The neighbor vertex to check.
 * @return The edge, or nullptr if not found.
 */
Edge* Vertex::getEdgeByNeighbor(const Vertex* neighbor) const {
    for(size_t e = 0; e < edges.size(); e++) {
        if(edges[e]->getOtherVertex(this) == neighbor) {
            return edges[e];
        }
    }
    return nullptr;
}


/**
 * @brief Returns whether or not this vertex has the specified edge in its list.
 *
 * @param ePtr Edge to check.
 * @return Whether it has the edge.
 */
bool Vertex::hasEdge(const Edge* ePtr) const {
    for(size_t e = 0; e < edges.size(); e++) {
        if(edges[e] == ePtr) return true;
    }
    return false;
}


/**
 * @brief Returns whether or not this vertex is a second-degree neighbor to the
 * specified vertex. i.e. they have a shared neighbor between them.
 *
 * @param otherV The vertex to compare against.
 * @param firstNeighbor Return the common neighbor between them here,
 * if the result is true.
 * @return Whether it is a second-degree neighbor.
 */
bool Vertex::is2ndDegreeNeighbor(
    const Vertex* otherV, Vertex** firstNeighbor
) const {
    //Let's crawl forward through all edges and stop at the second level.
    //If other_v is at that distance, then we found it!
    
    for(size_t e1 = 0; e1 < edges.size(); e1++) {
        Vertex* nextV = edges[e1]->getOtherVertex(this);
        
        for(size_t e2 = 0; e2 < nextV->edges.size(); e2++) {
            if(nextV->edges[e2]->getOtherVertex(nextV) == otherV) {
                *firstNeighbor = nextV;
                return true;
            }
        }
        
    }
    return false;
}


/**
 * @brief Returns whether or not this vertex is a second-degree neighbor to the
 * specified edge. i.e. one of the vertex's neighbors is used by the edge.
 *
 * @param otherE The edge to compare against.
 * @param firstNeighbor Return the common neighbor between them here,
 * if the result is true.
 * @return Whether it is a second-degree neighbor.
 */
bool Vertex::is2ndDegreeNeighbor(
    const Edge* otherE, Vertex** firstNeighbor
) const {
    //Let's crawl forward through all edges and stop at the second level.
    //If otherE is at that distance, then we found it!
    
    for(size_t e1 = 0; e1 < edges.size(); e1++) {
        Vertex* nextV = edges[e1]->getOtherVertex(this);
        
        for(size_t e2 = 0; e2 < nextV->edges.size(); e2++) {
            if(nextV->edges[e2] == otherE) {
                *firstNeighbor = nextV;
                return true;
            }
        }
        
    }
    return false;
}


/**
 * @brief Returns whether or not this vertex is a neighbor to the
 * specified vertex. i.e. they have a shared edge between them.
 *
 * @param otherV The vertex to compare against.
 * @return Whether it is a neighbor.
 */
bool Vertex::isNeighbor(const Vertex* otherV) const {
    for(size_t e = 0; e < edges.size(); e++) {
        if(edges[e]->getOtherVertex(this) == otherV) {
            return true;
        }
    }
    return false;
}


/**
 * @brief Removes an edge from a vertex's list of edges, if it is there.
 *
 * @param ePtr Edge to remove.
 */
void Vertex::removeEdge(const Edge* ePtr) {
    size_t i = 0;
    for(; i < edges.size(); i++) {
        if(edges[i] == ePtr) {
            edges.erase(edges.begin() + i);
            edgeIdxs.erase(edgeIdxs.begin() + i);
            return;
        }
    }
}
