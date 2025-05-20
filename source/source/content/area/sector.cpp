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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "geometry.h"


/**
 * @brief Destroys the sector object.
 *
 */
Sector::~Sector() {
    for(size_t t = 0; t < 2; t++) {
        if(textureInfo.bitmap && textureInfo.bitmap != game.bmpError) {
            game.content.bitmaps.list.free(textureInfo.bmpName);
        }
    }
}


/**
 * @brief Adds an edge to the sector's list of edges, if it's not there already.
 *
 * @param ePtr Edge to add.
 * @param eIdx Index of the edge to add.
 */
void Sector::addEdge(Edge* ePtr, size_t eIdx) {
    for(size_t i = 0; i < edges.size(); i++) {
        if(edges[i] == ePtr) {
            return;
        }
    }
    edges.push_back(ePtr);
    edgeIdxs.push_back(eIdx);
}


/**
 * @brief Calculates the bounding box coordinates and saves them
 * in the object's bbox variables.
 */
void Sector::calculateBoundingBox() {
    if(edges.empty()) {
        //Unused sector... This shouldn't exist.
        bbox[0] = Point();
        bbox[1] = Point();
        return;
    }
    
    bbox[0] = v2p(edges[0]->vertexes[0]);
    bbox[1] = bbox[0];
    
    for(size_t e = 0; e < edges.size(); e++) {
        for(unsigned char v = 0; v < 2; v++) {
            Point p = v2p(edges[e]->vertexes[v]);
            updateMinMaxCoords(bbox[0], bbox[1], p);
        }
    }
}


/**
 * @brief Clones a sector's properties onto another,
 * not counting the list of edges, bounding box, or bitmap
 * (the file name is cloned too, though).
 *
 * @param destination Sector to clone the data into.
 */
void Sector::clone(Sector* destination) const {
    destination->type = type;
    destination->isBottomlessPit = isBottomlessPit;
    destination->z = z;
    destination->tag = tag;
    destination->hazard = hazard;
    destination->hazardFloor = hazardFloor;
    destination->brightness = brightness;
    destination->textureInfo.scale = textureInfo.scale;
    destination->textureInfo.translation = textureInfo.translation;
    destination->textureInfo.rot = textureInfo.rot;
    destination->textureInfo.tint = textureInfo.tint;
    destination->fade = fade;
}


/**
 * @brief Fills a vector with neighboring sectors, recursively, but only if they
 * meet certain criteria.
 *
 * @param condition Function that accepts a sector and checks its criteria.
 * This function must return true if accepted, false if not.
 * @param sectorList List of sectors to be filled.
 * Also doubles as the list of visited sectors.
 */
void Sector::getNeighborSectorsConditionally(
    const std::function<bool(Sector* s_ptr)> &condition,
    vector<Sector*> &sectorList
) {

    //If this sector is already on the list, skip.
    for(size_t s = 0; s < sectorList.size(); s++) {
        if(sectorList[s] == this) return;
    }
    
    //If this sector is not eligible, return.
    if(!condition(this)) return;
    
    //This sector is valid!
    sectorList.push_back(this);
    
    //Now check its neighbors.
    Edge* ePtr = nullptr;
    Sector* otherS = nullptr;
    for(size_t e = 0; e < edges.size(); e++) {
        ePtr = edges[e];
        otherS = ePtr->getOtherSector(this);
        if(!otherS) continue;
        
        otherS->getNeighborSectorsConditionally(condition, sectorList);
    }
}


/**
 * @brief Returns the vertex farthest to the right in a sector.
 *
 * @return The vertex.
 */
Vertex* Sector::getRightmostVertex() const {
    Vertex* rightmost = nullptr;
    
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* ePtr = edges[e];
        if(!rightmost) rightmost = ePtr->vertexes[0];
        else {
            rightmost = ::getRightmostVertex(ePtr->vertexes[0], rightmost);
            rightmost = ::getRightmostVertex(ePtr->vertexes[1], rightmost);
        }
    }
    
    return rightmost;
}


/**
 * @brief If texture merging is required, this returns what two
 * neighboring sectors will be used for it.
 *
 * @param s1 Receives the first sector.
 * @param s2 Receives the second sector.
 */
void Sector::getTextureMergeSectors(Sector** s1, Sector** s2) const {
    //Check all edges to find which two textures need merging.
    Edge* ePtr = nullptr;
    Sector* neighbor = nullptr;
    map<Sector*, Distance> neighbors;
    Sector* textureSector[2] = {nullptr, nullptr};
    
    //The two neighboring sectors with the lenghtiest edges are picked.
    //So save all sector/length pairs.
    //Sectors with different heights from the current one are also saved,
    //but they have lower priority compared to same-heigh sectors.
    for(size_t e = 0; e < edges.size(); e++) {
        ePtr = edges[e];
        bool valid = true;
        
        neighbor = ePtr->getOtherSector(this);
        
        if(neighbor) {
            if(neighbor->fade) valid = false;
        }
        
        if(valid) {
            neighbors[neighbor] +=
                Distance(v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]));
        }
    }
    
    //Find the two lengthiest ones.
    vector<std::pair<Distance, Sector*> > neighborsVec;
    for(auto &n : neighbors) {
        neighborsVec.push_back(
            std::make_pair(
                //Yes, we do need these casts, for g++.
                (Distance) (n.second), (Sector*) (n.first)
            )
        );
    }
    sort(
        neighborsVec.begin(), neighborsVec.end(),
    [this] (
        std::pair<Distance, Sector*> p1, std::pair<Distance, Sector*> p2
    ) -> bool {
        return p1.first < p2.first;
    }
    );
    if(neighborsVec.size() >= 1) {
        textureSector[0] = neighborsVec.back().second;
    }
    if(neighborsVec.size() >= 2) {
        textureSector[1] = neighborsVec[neighborsVec.size() - 2].second;
    }
    
    if(!textureSector[1] && textureSector[0]) {
        //0 is always the bottom one. If we're fading into nothingness,
        //we should swap first.
        std::swap(textureSector[0], textureSector[1]);
    } else if(!textureSector[1]) {
        //Nothing to draw.
        return;
    } else if(textureSector[1]->isBottomlessPit) {
        std::swap(textureSector[0], textureSector[1]);
    }
    
    *s1 = textureSector[0];
    *s2 = textureSector[1];
}


/**
 * @brief Returns whether a sector's vertexes are ordered clockwise or not.
 *
 * @return Whether it is clockwise.
 */
bool Sector::isClockwise() const {
    vector<Vertex*> vertexes;
    for(size_t e = 0; e < edges.size(); e++) {
        vertexes.push_back(edges[e]->vertexes[0]);
    }
    return isPolygonClockwise(vertexes);
}


/**
 * @brief Returns whether a point is inside a sector by checking its triangles.
 *
 * @param p Point to check.
 * @return Whether it is in the sector.
 */
bool Sector::isPointInSector(const Point &p) const {
    for(size_t t = 0; t < triangles.size(); t++) {
        const Triangle* tPtr = &triangles[t];
        if(
            isPointInTriangle(
                p,
                v2p(tPtr->points[0]),
                v2p(tPtr->points[1]),
                v2p(tPtr->points[2]),
                false
            )
        ) {
            return true;
        }
    }
    
    return false;
}


/**
 * @brief Removes an edge from a sector's list of edges, if it is there.
 *
 * @param ePtr Edge to remove.
 */
void Sector::removeEdge(const Edge* ePtr) {
    size_t i = 0;
    for(; i < edges.size(); i++) {
        if(edges[i] == ePtr) {
            edges.erase(edges.begin() + i);
            edgeIdxs.erase(edgeIdxs.begin() + i);
            return;
        }
    }
}


/**
 * @brief Returns which sector the specified point belongs to.
 *
 * @param p Coordinates of the point.
 * @param outSectorIdx If not nullptr, the index of the sector on the
 * area map is returned here. The index will not be set if the search
 * is using the blockmap.
 * @param useBlockmap If true, use the blockmap to search.
 * This provides faster results, but the blockmap must be built.
 * @return The sector.
 */
Sector* getSector(
    const Point &p, size_t* outSectorIdx, bool useBlockmap
) {

    if(useBlockmap) {
    
        size_t col = game.curAreaData->bmap.getCol(p.x);
        size_t row = game.curAreaData->bmap.getRow(p.y);
        if(col == INVALID || row == INVALID) return nullptr;
        
        unordered_set<Sector*>* sectors =
            &game.curAreaData->bmap.sectors[col][row];
            
        if(sectors->size() == 1) return *sectors->begin();
        
        for(auto &s : (*sectors)) {
        
            if(!s) {
                continue;
            }
            if(s->isPointInSector(p)) {
                return s;
            }
        }
        
        return nullptr;
        
    } else {
    
        for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
            Sector* sPtr = game.curAreaData->sectors[s];
            
            if(
                p.x < sPtr->bbox[0].x ||
                p.x > sPtr->bbox[1].x ||
                p.y < sPtr->bbox[0].y ||
                p.y > sPtr->bbox[1].y
            ) {
                continue;
            }
            if(sPtr->isPointInSector(p)) {
                if(outSectorIdx) *outSectorIdx = s;
                return sPtr;
            }
            
        }
        
        if(outSectorIdx) *outSectorIdx = INVALID;
        return nullptr;
        
    }
}
