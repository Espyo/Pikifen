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
Edge::Edge(size_t v1, size_t v2) {
    vertexIdxs[0] = v1; vertexIdxs[1] = v2;
}


/**
 * @brief Clones an edge's properties onto another,
 * not counting the sectors or vertexes.
 *
 * @param destination Edge to clone the data into.
 */
void Edge::clone(Edge* destination) const {
    destination->wallShadowLength = wallShadowLength;
    destination->wallShadowColor = wallShadowColor;
    destination->ledgeSmoothingLength = ledgeSmoothingLength;
    destination->ledgeSmoothingColor = ledgeSmoothingColor;
}


/**
 * @brief Returns the sector that ISN'T the specified one.
 *
 * @param sPtr The sector that ISN'T the one to return.
 * @return The other sector.
 */
Sector* Edge::getOtherSector(const Sector* sPtr) const {
    if(sectors[0] == sPtr) return sectors[1];
    return sectors[0];
}


/**
 * @brief Returns the vertex that ISN'T the specified one.
 *
 * @param vPtr The vertex that ISN'T the one to return.
 * @return The other vertex.
 */
Vertex* Edge::getOtherVertex(const Vertex* vPtr) const {
    if(vertexes[0] == vPtr) return vertexes[1];
    return vertexes[0];
}


/**
 * @brief Returns which side has the specified sector, if any.
 *
 * @param sPtr Sector to check.
 * @return The side index, or INVALID if neither.
 */
size_t Edge::getSideWithSector(const Sector* sPtr) const {
    for(unsigned char s = 0; s < 2; s++) {
        if(sectors[s] == sPtr) return s;
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
Vertex* Edge::hasNeighbor(const Edge* other) const {
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
bool Edge::isValid() const {
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
size_t Edge::removeFromSectors() {
    size_t eIdx = INVALID;
    for(unsigned char s = 0; s < 2; s++) {
        Sector* sPtr = sectors[s];
        if(!sPtr) continue;
        for(size_t e = 0; e < sPtr->edges.size(); e++) {
            Edge* ePtr = sPtr->edges[e];
            if(ePtr == this) {
                sPtr->edges.erase(sPtr->edges.begin() + e);
                auto nrIt = sPtr->edgeIdxs.begin() + e;
                eIdx = *nrIt;
                sPtr->edgeIdxs.erase(nrIt);
                break;
            }
        }
        sectors[s] = nullptr;
        sectorIdxs[s] = INVALID;
    }
    return eIdx;
}


/**
 * @brief Removes the edge from its vertexes, but doesn't mark
 * the vertexes as "none".
 *
 * @return The edge index.
 */
size_t Edge::removeFromVertexes() {
    size_t eIdx = INVALID;
    for(unsigned char v = 0; v < 2; v++) {
        Vertex* vPtr = vertexes[v];
        if(!vPtr) continue;
        for(size_t e = 0; e < vPtr->edges.size(); e++) {
            Edge* ePtr = vPtr->edges[e];
            if(ePtr == this) {
                vPtr->edges.erase(vPtr->edges.begin() + e);
                auto nrIt = vPtr->edgeIdxs.begin() + e;
                eIdx = *nrIt;
                vPtr->edgeIdxs.erase(nrIt);
                break;
            }
        }
        vertexes[v] = nullptr;
        vertexIdxs[v] = INVALID;
    }
    return eIdx;
}


/**
 * @brief Swaps the two vertexes of the edge around. It also swaps the sectors,
 * so that they still point in the right direction.
 */
void Edge::swapVertexes() {
    std::swap(vertexes[0], vertexes[1]);
    std::swap(vertexIdxs[0], vertexIdxs[1]);
    std::swap(sectors[0], sectors[1]);
    std::swap(sectorIdxs[0], sectorIdxs[1]);
}


/**
 * @brief Transfers this edge from one sector to a different one.
 *
 * @param from Sector to transfer from.
 * @param to Sector to transfer to.
 * @param toIdx Index of the sector to transfer to.
 * @param edgeIdx Index of the current edge.
 */
void Edge::transferSector(
    Sector* from, Sector* to, size_t toIdx, size_t edgeIdx
) {
    size_t idx = getSideWithSector(from);
    engineAssert(
        idx != INVALID,
        i2s(toIdx)
    );
    
    sectors[idx] = to;
    sectorIdxs[idx] = toIdx;
    
    if(from) from->removeEdge(this);
    if(to) to->addEdge(this, edgeIdx);
}


/**
 * @brief Constructs a new edge intersection object.
 *
 * @param e1 First edge in the intersection.
 * @param e2 Second edge in the intersection.
 */
EdgeIntersection::EdgeIntersection(Edge* e1, Edge* e2) :
    e1(e1),
    e2(e2) {
    
}


/**
 * @brief Checks whether the edge intersection contains the specified edge.
 *
 * @param e Edge to check.
 * @return Whether it contains the edge.
 */
bool EdgeIntersection::contains(const Edge* e) {
    return e1 == e || e2 == e;
}
