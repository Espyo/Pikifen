/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area class and related functions.
 */

#include <algorithm>
#include <vector>

#include "area.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


using std::size_t;
using std::vector;


namespace AREA {

//Default day time speed, in game-minutes per real-minutes.
const float DEF_DAY_TIME_SPEED = 120;

//Default day time at the start of gameplay, in minutes.
const size_t DEF_DAY_TIME_START = 7 * 60;

//Default difficulty.
const unsigned char DEF_DIFFICULTY = 0;

}


#pragma region Area


/**
 * @brief Creates and adds a new edge to the list.
 *
 * @return The new edge's pointer.
 */
Edge* Area::addNewEdge() {
    Edge* ePtr = new Edge();
    edges.push_back(ePtr);
    return ePtr;
}


/**
 * @brief Creates and adds a new sector to the list.
 *
 * @return The new sector's pointer.
 */
Sector* Area::addNewSector() {
    Sector* sPtr = new Sector();
    sectors.push_back(sPtr);
    return sPtr;
}


/**
 * @brief Creates and adds a new vertex to the list.
 *
 * @return The new vertex's pointer.
 */
Vertex* Area::addNewVertex() {
    Vertex* vPtr = new Vertex();
    vertexes.push_back(vPtr);
    return vPtr;
}


/**
 * @brief Checks to see if all indexes match their pointers,
 * for the various edges, vertexes, etc.
 *
 * This is merely an engine debugging tool. Aborts execution if any of the
 * pointers don't match.
 */
void Area::checkStability() {
    forIdx(v, vertexes) {
        Vertex* vPtr = vertexes[v];
        engineAssert(
            vPtr->edges.size() == vPtr->edgeIdxs.size(),
            i2s(vPtr->edges.size()) + " " + i2s(vPtr->edgeIdxs.size())
        );
        forIdx(e, vPtr->edges) {
            engineAssert(vPtr->edges[e] == edges[vPtr->edgeIdxs[e]], "");
        }
    }
    
    forIdx(e, edges) {
        Edge* ePtr = edges[e];
        for(size_t v = 0; v < 2; v++) {
            engineAssert(
                ePtr->vertexes[v] == vertexes[ePtr->vertexIdxs[v]], ""
            );
        }
        
        for(size_t s = 0; s < 2; s++) {
            Sector* sPtr = ePtr->sectors[s];
            if(
                sPtr == nullptr &&
                ePtr->sectorIdxs[s] == INVALID
            ) {
                continue;
            }
            engineAssert(sPtr == sectors[ePtr->sectorIdxs[s]], "");
        }
    }
    
    forIdx(s, sectors) {
        Sector* sPtr = sectors[s];
        engineAssert(
            sPtr->edges.size() == sPtr->edgeIdxs.size(),
            i2s(sPtr->edges.size()) + " " + i2s(sPtr->edgeIdxs.size())
        );
        forIdx(e, sPtr->edges) {
            engineAssert(sPtr->edges[e] == edges[sPtr->edgeIdxs[e]], "");
        }
    }
}


/**
 * @brief Cleans up redundant data and such.
 *
 * @param outDeletedSectors If not nullptr, whether any sectors got deleted
 * is returned here.
 */
void Area::cleanup(bool* outDeletedSectors) {
    //Get rid of unused sectors.
    bool deletedSectors = false;
    for(size_t s = 0; s < sectors.size(); ) {
        if(sectors[s]->edges.empty()) {
            deleteSector(s);
            deletedSectors = true;
        } else {
            s++;
        }
    }
    if(outDeletedSectors) *outDeletedSectors = deletedSectors;
    
    //And some other cleanup.
    if(songName == NONE_OPTION) {
        songName.clear();
    }
    if(weatherName == NONE_OPTION) {
        weatherName.clear();
    }
    engineVersion = getEngineVersionString();
}


/**
 * @brief Clears the info of an area map.
 */
void Area::clear() {
    forIdx(v, vertexes) {
        delete vertexes[v];
    }
    forIdx(e, edges) {
        delete edges[e];
    }
    forIdx(s, sectors) {
        delete sectors[s];
    }
    forIdx(m, mobGenerators) {
        delete mobGenerators[m];
    }
    forIdx(s, pathStops) {
        delete pathStops[s];
    }
    forIdx(s, treeShadows) {
        delete treeShadows[s];
    }
    forIdx(r, regions) {
        delete regions[r];
    }
    
    vertexes.clear();
    edges.clear();
    sectors.clear();
    mobGenerators.clear();
    pathStops.clear();
    treeShadows.clear();
    regions.clear();
    bmap.clear();
    
    if(bgBmp) {
        game.content.bitmaps.list.free(bgBmp);
        bgBmp = nullptr;
    }
    if(thumbnail) {
        thumbnail = nullptr;
    }
    
    resetMetadata();
    manifest = nullptr;
    name.clear();
    type = AREA_TYPE_SIMPLE;
    subtitle.clear();
    difficulty = AREA::DEF_DIFFICULTY;
    sprayAmounts.clear();
    songName.clear();
    weatherName.clear();
    dayTimeStart = AREA::DEF_DAY_TIME_START;
    dayTimeSpeed = AREA::DEF_DAY_TIME_SPEED;
    bgBmpName.clear();
    bgColor = COLOR_BLACK;
    bgDist = 2.0f;
    bgBmpZoom = 1.0f;
    mission = MissionData();
    
    problems.nonSimples.clear();
    problems.loneEdges.clear();
}


/**
 * @brief Clones this area data into another Area object.
 *
 * @param other The area data object to clone to.
 */
void Area::clone(Area& other) {
    other.clear();
    
    if(!other.bgBmpName.empty() && other.bgBmp) {
        game.content.bitmaps.list.free(other.bgBmpName);
    }
    other.bgBmpName = bgBmpName;
    if(other.bgBmpName.empty()) {
        other.bgBmp = nullptr;
    } else {
        other.bgBmp = game.content.bitmaps.list.get(bgBmpName, nullptr, false);
    }
    other.bgBmpZoom = bgBmpZoom;
    other.bgColor = bgColor;
    other.bgDist = bgDist;
    other.bmap = bmap;
    
    other.vertexes.reserve(vertexes.size());
    forIdx(v, vertexes) {
        other.vertexes.push_back(new Vertex());
    }
    other.edges.reserve(edges.size());
    forIdx(e, edges) {
        other.edges.push_back(new Edge());
    }
    other.sectors.reserve(sectors.size());
    forIdx(s, sectors) {
        other.sectors.push_back(new Sector());
    }
    other.mobGenerators.reserve(mobGenerators.size());
    forIdx(m, mobGenerators) {
        other.mobGenerators.push_back(new MobGen());
    }
    other.pathStops.reserve(pathStops.size());
    forIdx(s, pathStops) {
        other.pathStops.push_back(new PathStop());
    }
    other.treeShadows.reserve(treeShadows.size());
    forIdx(t, treeShadows) {
        other.treeShadows.push_back(new TreeShadow());
    }
    other.regions.reserve(regions.size());
    forIdx(r, regions) {
        other.regions.push_back(new AreaRegion());
    }
    
    forIdx(v, vertexes) {
        Vertex* vPtr = vertexes[v];
        Vertex* ovPtr = other.vertexes[v];
        ovPtr->x = vPtr->x;
        ovPtr->y = vPtr->y;
        ovPtr->edges.reserve(vPtr->edges.size());
        ovPtr->edgeIdxs.reserve(vPtr->edgeIdxs.size());
        forIdx(e, vPtr->edges) {
            size_t nr = vPtr->edgeIdxs[e];
            ovPtr->edges.push_back(other.edges[nr]);
            ovPtr->edgeIdxs.push_back(nr);
        }
    }
    
    forIdx(e, edges) {
        Edge* ePtr = edges[e];
        Edge* oePtr = other.edges[e];
        oePtr->vertexes[0] = other.vertexes[ePtr->vertexIdxs[0]];
        oePtr->vertexes[1] = other.vertexes[ePtr->vertexIdxs[1]];
        oePtr->vertexIdxs[0] = ePtr->vertexIdxs[0];
        oePtr->vertexIdxs[1] = ePtr->vertexIdxs[1];
        if(ePtr->sectorIdxs[0] == INVALID) {
            oePtr->sectors[0] = nullptr;
        } else {
            oePtr->sectors[0] = other.sectors[ePtr->sectorIdxs[0]];
        }
        if(ePtr->sectorIdxs[1] == INVALID) {
            oePtr->sectors[1] = nullptr;
        } else {
            oePtr->sectors[1] = other.sectors[ePtr->sectorIdxs[1]];
        }
        oePtr->sectorIdxs[0] = ePtr->sectorIdxs[0];
        oePtr->sectorIdxs[1] = ePtr->sectorIdxs[1];
        ePtr->clone(oePtr);
    }
    
    forIdx(s, sectors) {
        Sector* sPtr = sectors[s];
        Sector* osPtr = other.sectors[s];
        sPtr->clone(osPtr);
        osPtr->textureInfo.bmpName = sPtr->textureInfo.bmpName;
        osPtr->textureInfo.bitmap =
            game.content.bitmaps.list.get(
                sPtr->textureInfo.bmpName, nullptr, false
            );
        osPtr->edges.reserve(sPtr->edges.size());
        osPtr->edgeIdxs.reserve(sPtr->edgeIdxs.size());
        forIdx(e, sPtr->edges) {
            size_t nr = sPtr->edgeIdxs[e];
            osPtr->edges.push_back(other.edges[nr]);
            osPtr->edgeIdxs.push_back(nr);
        }
        osPtr->triangles.reserve(sPtr->triangles.size());
        forIdx(t, sPtr->triangles) {
            Triangle* tPtr = &sPtr->triangles[t];
            osPtr->triangles.push_back(
                Triangle(
                    other.vertexes[findVertexIdx(tPtr->points[0])],
                    other.vertexes[findVertexIdx(tPtr->points[1])],
                    other.vertexes[findVertexIdx(tPtr->points[2])]
                )
            );
        }
        osPtr->bbox[0] = sPtr->bbox[0];
        osPtr->bbox[1] = sPtr->bbox[1];
    }
    
    forIdx(m, mobGenerators) {
        MobGen* mPtr = mobGenerators[m];
        MobGen* omPtr = other.mobGenerators[m];
        mPtr->clone(omPtr);
    }
    forIdx(m, mobGenerators) {
        MobGen* omPtr = other.mobGenerators[m];
        forIdx(l, omPtr->linkIdxs) {
            omPtr->links.push_back(
                other.mobGenerators[omPtr->linkIdxs[l]]
            );
        }
    }
    
    forIdx(s, pathStops) {
        PathStop* sPtr = pathStops[s];
        PathStop* osPtr = other.pathStops[s];
        osPtr->pos = sPtr->pos;
        sPtr->clone(osPtr);
        osPtr->links.reserve(sPtr->links.size());
        forIdx(l, sPtr->links) {
            PathLink* newLink =
                new PathLink(
                osPtr,
                other.pathStops[sPtr->links[l]->endIdx],
                sPtr->links[l]->endIdx
            );
            sPtr->links[l]->clone(newLink);
            newLink->distance = sPtr->links[l]->distance;
            osPtr->links.push_back(newLink);
        }
    }
    
    forIdx(t, treeShadows) {
        TreeShadow* tPtr = treeShadows[t];
        TreeShadow* otPtr = other.treeShadows[t];
        otPtr->alpha = tPtr->alpha;
        otPtr->pose.pos = tPtr->pose.pos;
        otPtr->pose.size = tPtr->pose.size;
        otPtr->pose.angle = tPtr->pose.angle;
        otPtr->bmpName = tPtr->bmpName;
        otPtr->sway = tPtr->sway;
        otPtr->bitmap =
            game.content.bitmaps.list.get(tPtr->bmpName, nullptr, false);
    }
    
    forIdx(r, regions) {
        AreaRegion* rPtr = regions[r];
        AreaRegion* orPtr = other.regions[r];
        orPtr->center = rPtr->center;
        orPtr->size = rPtr->size;
    }
    
    other.manifest = manifest;
    other.type = type;
    other.name = name;
    other.subtitle = subtitle;
    other.description = description;
    other.tags = tags;
    other.difficulty = difficulty;
    other.maker = maker;
    other.version = version;
    other.makerNotes = makerNotes;
    other.sprayAmounts = sprayAmounts;
    other.songName = songName;
    other.weatherName = weatherName;
    other.weatherCondition = weatherCondition;
    other.dayTimeStart = dayTimeStart;
    other.dayTimeSpeed = dayTimeSpeed;
    
    other.thumbnail = thumbnail;
    
    other.mission.preset = mission.preset;
    other.mission.endConds = mission.endConds;
    other.mission.mobGroups = mission.mobGroups;
    other.mission.hudItems = mission.hudItems;
    other.mission.timeLimit = mission.timeLimit;
    other.mission.medalAwardMode = mission.medalAwardMode;
    other.mission.startingPoints = mission.startingPoints;
    other.mission.bronzeReq = mission.bronzeReq;
    other.mission.silverReq = mission.silverReq;
    other.mission.goldReq = mission.goldReq;
    other.mission.platinumReq = mission.platinumReq;
    other.mission.makerRecord = mission.makerRecord;
    other.mission.makerRecordDate = mission.makerRecordDate;
    
    other.problems.nonSimples.clear();
    other.problems.loneEdges.clear();
    other.problems.loneEdges.reserve(problems.loneEdges.size());
    for(const auto& s : problems.nonSimples) {
        size_t nr = findSectorIdx(s.first);
        other.problems.nonSimples[other.sectors[nr]] = s.second;
    }
    for(const Edge* e : problems.loneEdges) {
        size_t nr = findEdgeIdx(e);
        other.problems.loneEdges.insert(other.edges[nr]);
    }
}


/**
 * @brief Connects an edge to a sector.
 *
 * This adds the sector and its index to the edge's
 * lists, and adds the edge and its index to the sector's.
 *
 * @param ePtr Edge to connect.
 * @param sPtr Sector to connect.
 * @param side Which of the sides of the edge the sector goes to.
 */
void Area::connectEdgeToSector(
    Edge* ePtr, Sector* sPtr, size_t side
) {
    if(ePtr->sectors[side]) {
        ePtr->sectors[side]->removeEdge(ePtr);
    }
    ePtr->sectors[side] = sPtr;
    ePtr->sectorIdxs[side] = findSectorIdx(sPtr);
    if(sPtr) {
        sPtr->addEdge(ePtr, findEdgeIdx(ePtr));
    }
}


/**
 * @brief Connects an edge to a vertex.
 *
 * This adds the vertex and its index to the edge's
 * lists, and adds the edge and its index to the vertex's.
 *
 * @param ePtr Edge to connect.
 * @param vPtr Vertex to connect.
 * @param endpoint Which of the edge endpoints the vertex goes to.
 */
void Area::connectEdgeToVertex(
    Edge* ePtr, Vertex* vPtr, size_t endpoint
) {
    if(ePtr->vertexes[endpoint]) {
        ePtr->vertexes[endpoint]->removeEdge(ePtr);
    }
    ePtr->vertexes[endpoint] = vPtr;
    ePtr->vertexIdxs[endpoint] = findVertexIdx(vPtr);
    vPtr->addEdge(ePtr, findEdgeIdx(ePtr));
}



/**
 * @brief Connects the edges of a sector that link to it into the
 * edgeIdxs vector.
 *
 * @param sPtr The sector.
 */
void Area::connectSectorEdges(Sector* sPtr) {
    sPtr->edgeIdxs.clear();
    forIdx(e, edges) {
        Edge* ePtr = edges[e];
        if(ePtr->sectors[0] == sPtr || ePtr->sectors[1] == sPtr) {
            sPtr->edgeIdxs.push_back(e);
        }
    }
    fixSectorPointers(sPtr);
}


/**
 * @brief Connects the edges that link to it into the edge_idxs vector.
 *
 * @param vPtr The vertex.
 */
void Area::connectVertexEdges(Vertex* vPtr) {
    vPtr->edgeIdxs.clear();
    forIdx(e, edges) {
        Edge* ePtr = edges[e];
        if(ePtr->vertexes[0] == vPtr || ePtr->vertexes[1] == vPtr) {
            vPtr->edgeIdxs.push_back(e);
        }
    }
    fixVertexPointers(vPtr);
}


/**
 * @brief Removes and deletes an edge, and updates all indexes after it.
 *
 * @param eIdx Index number of the edge to delete.
 */
void Area::deleteEdge(size_t eIdx) {
    delete edges[eIdx];
    edges.erase(edges.begin() + eIdx);
    
    forIdx(v, vertexes) {
        Vertex* vPtr = vertexes[v];
        forIdx(e, vPtr->edges) {
            if(vPtr->edgeIdxs[e] == INVALID) continue;
            adjustMisalignedIndex(
                vPtr->edgeIdxs[e], eIdx, false
            );
        }
    }
    
    forIdx(s, sectors) {
        Sector* sPtr = sectors[s];
        forIdx(e, sPtr->edges) {
            if(sPtr->edgeIdxs[e] == INVALID) continue;
            adjustMisalignedIndex(
                sPtr->edgeIdxs[e], eIdx, false
            );
        }
    }
}


/**
 * @brief Removes and deletes an edge, and updates all indexes after it.
 *
 * @param ePtr Pointer of the edge to delete.
 */
void Area::deleteEdge(const Edge* ePtr) {
    forIdx(e, edges) {
        if(edges[e] == ePtr) {
            deleteEdge(e);
            return;
        }
    }
}


/**
 * @brief Removes and deletes a sector, and updates all indexes after it.
 *
 * @param sIdx Index number of the sector to delete.
 */
void Area::deleteSector(size_t sIdx) {
    delete sectors[sIdx];
    sectors.erase(sectors.begin() + sIdx);
    
    forIdx(e, edges) {
        Edge* ePtr = edges[e];
        for(size_t s = 0; s < 2; s++) {
            if(ePtr->sectorIdxs[s] == INVALID) continue;
            adjustMisalignedIndex(
                ePtr->sectorIdxs[s], sIdx, false
            );
        }
    }
}


/**
 * @brief Removes and deletes a sector, and updates all indexes after it.
 *
 * @param sPtr Pointer of the sector to delete.
 */
void Area::deleteSector(const Sector* sPtr) {
    forIdx(s, sectors) {
        if(sectors[s] == sPtr) {
            deleteSector(s);
            return;
        }
    }
}


/**
 * @brief Removes and deletes a vertex, and updates all indexes after it.
 *
 * @param vIdx Index number of the vertex to delete.
 */
void Area::deleteVertex(size_t vIdx) {
    delete vertexes[vIdx];
    vertexes.erase(vertexes.begin() + vIdx);
    
    forIdx(e, edges) {
        Edge* ePtr = edges[e];
        for(size_t v = 0; v < 2; v++) {
            if(ePtr->vertexIdxs[v] == INVALID) continue;
            adjustMisalignedIndex(
                ePtr->vertexIdxs[v], vIdx, false
            );
        }
    }
}


/**
 * @brief Removes and deletes a vertex, and updates all indexes after it.
 *
 * @param vPtr Pointer of the vertex to delete.
 */
void Area::deleteVertex(const Vertex* vPtr) {
    forIdx(v, vertexes) {
        if(vertexes[v] == vPtr) {
            deleteVertex(v);
            return;
        }
    }
}


/**
 * @brief Scans the list of edges and retrieves the index of
 * the specified edge.
 *
 * @param ePtr Edge to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::findEdgeIdx(const Edge* ePtr) const {
    forIdx(e, edges) {
        if(edges[e] == ePtr) return e;
    }
    return INVALID;
}


/**
 * @brief Scans the list of mob generators and retrieves the index of
 * the specified mob generator.
 *
 * @param mPtr Mob to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::findMobGenIdx(const MobGen* mPtr) const {
    forIdx(m, mobGenerators) {
        if(mobGenerators[m] == mPtr) return m;
    }
    return INVALID;
}


/**
 * @brief Scans the list of sectors and retrieves the index of
 * the specified sector.
 *
 * @param sPtr Sector to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::findSectorIdx(const Sector* sPtr) const {
    forIdx(s, sectors) {
        if(sectors[s] == sPtr) return s;
    }
    return INVALID;
}


/**
 * @brief Scans the list of vertexes and retrieves the index of
 * the specified vertex.
 *
 * @param vPtr Vertex to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::findVertexIdx(const Vertex* vPtr) const {
    forIdx(v, vertexes) {
        if(vertexes[v] == vPtr) return v;
    }
    return INVALID;
}


/**
 * @brief Scans the list of tree shadows and retrieves the index of
 * the specified tree shadow.
 *
 * @param sPtr Shadow to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::findTreeShadowIdx(const TreeShadow* sPtr) const {
    forIdx(s, treeShadows) {
        if(treeShadows[s] == sPtr) return s;
    }
    return INVALID;
}


/**
 * @brief Fixes the sector and vertex indexes in an edge,
 * making them match the correct sectors and vertexes,
 * based on the existing sector and vertex pointers.
 *
 * @param ePtr Edge to fix the indexes of.
 */
void Area::fixEdgeIdxs(Edge* ePtr) {
    for(size_t s = 0; s < 2; s++) {
        if(!ePtr->sectors[s]) {
            ePtr->sectorIdxs[s] = INVALID;
        } else {
            ePtr->sectorIdxs[s] = findSectorIdx(ePtr->sectors[s]);
        }
    }
    
    for(size_t v = 0; v < 2; v++) {
        if(!ePtr->vertexes[v]) {
            ePtr->vertexIdxs[v] = INVALID;
        } else {
            ePtr->vertexIdxs[v] = findVertexIdx(ePtr->vertexes[v]);
        }
    }
}


/**
 * @brief Fixes the sector and vertex pointers of an edge,
 * making them point to the correct sectors and vertexes,
 * based on the existing sector and vertex indexes.
 *
 * @param ePtr Edge to fix the pointers of.
 */
void Area::fixEdgePointers(Edge* ePtr) {
    ePtr->sectors[0] = nullptr;
    ePtr->sectors[1] = nullptr;
    for(size_t s = 0; s < 2; s++) {
        size_t sIdx = ePtr->sectorIdxs[s];
        ePtr->sectors[s] = (sIdx == INVALID ? nullptr : sectors[sIdx]);
    }
    
    ePtr->vertexes[0] = nullptr;
    ePtr->vertexes[1] = nullptr;
    for(size_t v = 0; v < 2; v++) {
        size_t vIdx = ePtr->vertexIdxs[v];
        ePtr->vertexes[v] = (vIdx == INVALID ? nullptr : vertexes[vIdx]);
    }
}


/**
 * @brief Fixes the path stop indexes in a path stop's links,
 * making them match the correct path stops,
 * based on the existing path stop pointers.
 *
 * @param sPtr Path stop to fix the indexes of.
 */
void Area::fixPathStopIdxs(PathStop* sPtr) {
    forIdx(l, sPtr->links) {
        PathLink* lPtr = sPtr->links[l];
        lPtr->endIdx = INVALID;
        
        if(!lPtr->endPtr) continue;
        
        forIdx(s, pathStops) {
            if(lPtr->endPtr == pathStops[s]) {
                lPtr->endIdx = s;
                break;
            }
        }
    }
}


/**
 * @brief Fixes the path stop pointers in a path stop's links,
 * making them point to the correct path stops,
 * based on the existing path stop indexes.
 *
 * @param sPtr Path stop to fix the pointers of.
 */
void Area::fixPathStopPointers(PathStop* sPtr) {
    forIdx(l, sPtr->links) {
        PathLink* lPtr = sPtr->links[l];
        lPtr->endPtr = nullptr;
        
        if(lPtr->endIdx == INVALID) continue;
        if(lPtr->endIdx >= pathStops.size()) continue;
        
        lPtr->endPtr = pathStops[lPtr->endIdx];
    }
}


/**
 * @brief Fixes the edge indexes in a sector,
 * making them match the correct edges,
 * based on the existing edge pointers.
 *
 * @param sPtr Sector to fix the indexes of.
 */
void Area::fixSectorIdxs(Sector* sPtr) {
    sPtr->edgeIdxs.clear();
    forIdx(e, sPtr->edges) {
        sPtr->edgeIdxs.push_back(findEdgeIdx(sPtr->edges[e]));
    }
}


/**
 * @brief Fixes the edge pointers in a sector,
 * making them point to the correct edges,
 * based on the existing edge indexes.
 *
 * @param sPtr Sector to fix the pointers of.
 */
void Area::fixSectorPointers(Sector* sPtr) {
    sPtr->edges.clear();
    forIdx(e, sPtr->edgeIdxs) {
        size_t eIdx = sPtr->edgeIdxs[e];
        sPtr->edges.push_back(eIdx == INVALID ? nullptr : edges[eIdx]);
    }
}


/**
 * @brief Fixes the edge indexes in a vertex,
 * making them match the correct edges,
 * based on the existing edge pointers.
 *
 * @param vPtr Vertex to fix the indexes of.
 */
void Area::fixVertexIdxs(Vertex* vPtr) {
    vPtr->edgeIdxs.clear();
    forIdx(e, vPtr->edges) {
        vPtr->edgeIdxs.push_back(findEdgeIdx(vPtr->edges[e]));
    }
}


/**
 * @brief Fixes the edge pointers in a vertex,
 * making them point to the correct edges,
 * based on the existing edge indexes.
 *
 * @param vPtr Vertex to fix the pointers of.
 */
void Area::fixVertexPointers(Vertex* vPtr) {
    vPtr->edges.clear();
    forIdx(e, vPtr->edgeIdxs) {
        size_t eIdx = vPtr->edgeIdxs[e];
        vPtr->edges.push_back(eIdx == INVALID ? nullptr : edges[eIdx]);
    }
}


/**
 * @brief Generates the blockmap for the area, given the current info.
 */
void Area::generateBlockmap() {
    bmap.clear();
    
    if(vertexes.empty()) return;
    
    //First, get the starting point and size of the blockmap.
    Point minCoords = v2p(vertexes[0]);
    Point maxCoords = minCoords;
    
    forIdx(v, vertexes) {
        updateMinMaxCoords(minCoords, maxCoords, v2p(vertexes[v]));
    }
    
    bmap.topLeftCorner = minCoords;
    //Add one more to the cols/rows because, suppose there's an edge at y = 256.
    //The row would be 2. In reality, the row should be 3.
    bmap.nCols =
        ceil((maxCoords.x - minCoords.x) / GEOMETRY::BLOCKMAP_BLOCK_SIZE) + 1;
    bmap.nRows =
        ceil((maxCoords.y - minCoords.y) / GEOMETRY::BLOCKMAP_BLOCK_SIZE) + 1;
        
    bmap.edges.assign(
        bmap.nCols, vector<vector<Edge*> >(bmap.nRows, vector<Edge*>())
    );
    bmap.sectors.assign(
        bmap.nCols, vector<unordered_set<Sector*> >(
            bmap.nRows, unordered_set<Sector*>()
        )
    );
    
    
    //Now, add a list of edges to each block.
    generateEdgesBlockmap(edges);
    
    
    /* If at this point, there's any block that's missing a sector,
     * that means we couldn't figure out the sectors due to the edges it has
     * alone. But the block still has a sector (or nullptr). So we need another
     * way to figure it out.
     * We know the following things that can speed up the process:
     * * The blocks at the edges of the blockmap have the nullptr sector as the
     *     only candidate.
     * * If a block's neighbor only has one sector, then this block has that
     *     same sector.
     * If we can't figure out the sector the easy way, then we have to use the
     * triangle method to get the sector. Using the center of the blockmap is
     * just as good a checking spot as any.
     */
    for(size_t bx = 0; bx < bmap.nCols; bx++) {
        for(size_t by = 0; by < bmap.nRows; by++) {
        
            if(!bmap.sectors[bx][by].empty()) continue;
            
            if(
                bx == 0 || by == 0 ||
                bx == bmap.nCols - 1 || by == bmap.nRows - 1
            ) {
                bmap.sectors[bx][by].insert(nullptr);
                continue;
            }
            
            if(bmap.sectors[bx - 1][by].size() == 1) {
                bmap.sectors[bx][by].insert(*bmap.sectors[bx - 1][by].begin());
                continue;
            }
            if(bmap.sectors[bx + 1][by].size() == 1) {
                bmap.sectors[bx][by].insert(*bmap.sectors[bx + 1][by].begin());
                continue;
            }
            if(bmap.sectors[bx][by - 1].size() == 1) {
                bmap.sectors[bx][by].insert(*bmap.sectors[bx][by - 1].begin());
                continue;
            }
            if(bmap.sectors[bx][by + 1].size() == 1) {
                bmap.sectors[bx][by].insert(*bmap.sectors[bx][by + 1].begin());
                continue;
            }
            
            Point corner = bmap.getTopLeftCorner(bx, by);
            corner += GEOMETRY::BLOCKMAP_BLOCK_SIZE * 0.5;
            bmap.sectors[bx][by].insert(
                getSector(corner, nullptr, false)
            );
        }
    }
}


/**
 * @brief Generates the blockmap for a set of edges.
 *
 * @param edgeList Edges to generate the blockmap around.
 */
void Area::generateEdgesBlockmap(const vector<Edge*>& edgeList) {
    forIdx(e, edgeList) {
    
        //Get which blocks this edge belongs to, via bounding-box,
        //and only then thoroughly test which it is inside of.
        
        Edge* ePtr = edgeList[e];
        Point minCoords = v2p(ePtr->vertexes[0]);
        Point maxCoords = minCoords;
        updateMinMaxCoords(minCoords, maxCoords, v2p(ePtr->vertexes[1]));
        
        size_t bMinX = bmap.getCol(minCoords.x);
        size_t bMaxX = bmap.getCol(maxCoords.x);
        size_t bMinY = bmap.getRow(minCoords.y);
        size_t bMaxY = bmap.getRow(maxCoords.y);
        
        for(size_t bx = bMinX; bx <= bMaxX; bx++) {
            for(size_t by = bMinY; by <= bMaxY; by++) {
            
                //Get the block's coordinates.
                Point corner = bmap.getTopLeftCorner(bx, by);
                
                //Check if the edge is inside this blockmap.
                if(
                    lineSegIntersectsRectangle(
                        corner,
                        corner + GEOMETRY::BLOCKMAP_BLOCK_SIZE,
                        v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1])
                    )
                ) {
                
                    //If it is, add it and the sectors to the list.
                    bool addEdge = true;
                    if(ePtr->sectors[0] && ePtr->sectors[1]) {
                        //If there's no change in height, why bother?
                        if(
                            (ePtr->sectors[0]->z == ePtr->sectors[1]->z) &&
                            ePtr->sectors[0]->type != SECTOR_TYPE_BLOCKING &&
                            ePtr->sectors[1]->type != SECTOR_TYPE_BLOCKING
                        ) {
                            addEdge = false;
                        }
                    }
                    
                    if(addEdge) bmap.edges[bx][by].push_back(ePtr);
                    
                    if(ePtr->sectors[0] || ePtr->sectors[1]) {
                        bmap.sectors[bx][by].insert(ePtr->sectors[0]);
                        bmap.sectors[bx][by].insert(ePtr->sectors[1]);
                    }
                }
            }
        }
    }
}


/**
 * @brief Returns the maximum amount of Pikmin that can be out in the
 * field at once. Uses the game configuration's value, or the area's
 * override value.
 *
 * @return The limit.
 */
size_t Area::getMaxPikminInField() const {
    return
        maxPikminInField == INVALID ?
        game.config.rules.maxPikminInField :
        maxPikminInField;
}


/**
 * @brief Returns how many path links exist in the area.
 *
 * @return The number of path links.
 */
size_t Area::getNrPathLinks() {
    size_t oneWaysFound = 0;
    size_t normalsFound = 0;
    forIdx(s, pathStops) {
        PathStop* sPtr = pathStops[s];
        forIdx(l, sPtr->links) {
            PathLink* lPtr = sPtr->links[l];
            if(lPtr->endPtr->getLink(sPtr)) {
                //The other stop links to this one. So it's a two-way.
                normalsFound++;
            } else {
                oneWaysFound++;
            }
        }
    }
    return (normalsFound / 2.0f) + oneWaysFound;
}


/**
 * @brief Returns information about the enemy-related objects in the area.
 *
 * @param outAmount If not nullptr, the total amount of enemy objects
 * is returned here.
 * @param outPoints If not nullptr, the total amount of enemy points
 * is returned here.
 */
void Area::getTotalEnemyInfo(size_t* outAmount, size_t* outPoints) const {
    size_t amount = 0;
    size_t points = 0;
    
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        switch(mPtr->type->category->id) {
        case MOB_CATEGORY_ENEMIES: {
            EnemyType* eneType = (EnemyType*) mPtr->type;
            amount++;
            points += eneType->points;
            break;
            
        } default: {
            break;
        }
        }
    }
    
    if(outAmount) *outAmount = amount;
    if(outPoints) *outPoints = points;
}


/**
 * @brief Returns information about the treasure-related objects in the area.
 *
 * @param outAmount If not nullptr, the total amount of treasure objects
 * is returned here.
 * @param outPoints If not nullptr, the total amount of treasure points
 * is returned here.
 */
void Area::getTotalTreasureInfo(size_t* outAmount, size_t* outPoints) const {
    size_t amount = 0;
    size_t points = 0;
    
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        switch(mPtr->type->category->id) {
        case MOB_CATEGORY_TREASURES: {
            TreasureType* treType = (TreasureType*) mPtr->type;
            amount++;
            points += treType->points;
            break;
            
        } case MOB_CATEGORY_PILES: {
            PileType* pilType = (PileType*) mPtr->type;
            if(
                pilType->contents->deliveryResult !=
                RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
            ) {
                continue;
            }
            size_t amountInPile = pilType->maxAmount;
            map<string, string> varMap = getVarMap(mPtr->vars);
            ScriptVarReader sv(varMap);
            sv.get("amount", amountInPile);
            amountInPile =
                std::clamp(amountInPile, (size_t) 0, pilType->maxAmount);
            amount += amountInPile;
            points += amountInPile * pilType->contents->pointAmount;
            break;
            
        } case MOB_CATEGORY_RESOURCES: {
            ResourceType* resType = (ResourceType*) mPtr->type;
            if(
                resType->deliveryResult !=
                RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
            ) {
                continue;
            }
            amount++;
            points += resType->pointAmount;
            break;
            
        } default: {
            break;
        }
        }
    }
    
    if(outAmount) *outAmount = amount;
    if(outPoints) *outPoints = points;
}


/**
 * @brief Loads the area's geometry from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void Area::loadGeometryFromDataNode(
    DataNode* node, CONTENT_LOAD_LEVEL level
) {
    //Vertexes.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Area -- Vertexes");
    }
    
    DataNode* vertexesNode = node->getChildByName("vertexes");
    size_t nVertexes = vertexesNode->getNrOfChildren();
    for(size_t v = 0; v < nVertexes; v++) {
        DataNode* vertexNode = vertexesNode->getChild(v);
        Point coords = s2p(vertexNode->value);
        vertexes.push_back(new Vertex(coords.x, coords.y));
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Edges.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Area -- Edges");
    }
    
    DataNode* edgesNode = node->getChildByName("edges");
    size_t nEdges = edgesNode->getNrOfChildren();
    for(size_t e = 0; e < nEdges; e++) {
        DataNode* edgeNode = edgesNode->getChild(e);
        ReaderSetter eRS(edgeNode);
        Edge* newEdge = new Edge();
        
        string sectorIdxsStr;
        string vertexIdxsStr;
        
        eRS.set("s", sectorIdxsStr);
        eRS.set("v", vertexIdxsStr);
        eRS.set("shadow_length", newEdge->wallShadowLength);
        eRS.set("shadow_color", newEdge->wallShadowColor);
        eRS.set("smoothing_length", newEdge->ledgeSmoothingLength);
        eRS.set("smoothing_color", newEdge->ledgeSmoothingColor);
        
        vector<string> sIdxs = split(sectorIdxsStr);
        if(sIdxs.size() < 2) sIdxs.insert(sIdxs.end(), 2, "-1");
        for(size_t s = 0; s < 2; s++) {
            if(sIdxs[s] == "-1") newEdge->sectorIdxs[s] = INVALID;
            else newEdge->sectorIdxs[s] = s2i(sIdxs[s]);
        }
        
        vector<string> vIdxs = split(vertexIdxsStr);
        if(vIdxs.size() < 2) vIdxs.insert(vIdxs.end(), 2, "0");
        newEdge->vertexIdxs[0] = s2i(vIdxs[0]);
        newEdge->vertexIdxs[1] = s2i(vIdxs[1]);
        
        edges.push_back(newEdge);
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Sectors.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Area -- Sectors");
    }
    
    DataNode* sectorsNode = node->getChildByName("sectors");
    size_t nSectors = sectorsNode->getNrOfChildren();
    for(size_t s = 0; s < nSectors; s++) {
        DataNode* sectorNode = sectorsNode->getChild(s);
        ReaderSetter sRS(sectorNode);
        Sector* newSector = new Sector();
        
        string typeStr;
        string hazardStr;
        DataNode* hazardNode = nullptr;
        
        sRS.set("brightness", newSector->brightness);
        sRS.set("fade", newSector->fade);
        sRS.set("hazard", hazardStr, &hazardNode);
        sRS.set("hazards_floor", newSector->hazardFloor);
        sRS.set("is_bottomless_pit", newSector->isBottomlessPit);
        sRS.set("texture_rotate", newSector->textureInfo.tf.rot);
        sRS.set("texture_scale", newSector->textureInfo.tf.scale);
        sRS.set("texture_tint", newSector->textureInfo.tint);
        sRS.set("texture_trans", newSector->textureInfo.tf.trans);
        sRS.set("texture", newSector->textureInfo.bmpName);
        sRS.set("type", typeStr);
        sRS.set("vars", newSector->vars);
        sRS.set("z", newSector->z);
        
        bool sectorTypeFound;
        newSector->type =
            enumGetValue(sectorTypeINames, typeStr, &sectorTypeFound);
        if(!sectorTypeFound) {
            newSector->type = SECTOR_TYPE_NORMAL;
        }
        
        if(!newSector->fade && !newSector->isBottomlessPit) {
            newSector->textureInfo.bitmap =
                game.content.bitmaps.list.get(
                    newSector->textureInfo.bmpName, nullptr
                );
        }
        
        if(!hazardStr.empty()) {
            if(!isInMap(game.content.hazards.list, hazardStr)) {
                game.errors.report(
                    "Unknown hazard \"" + hazardStr + "\"!", hazardNode
                );
            } else {
                newSector->hazard = &(game.content.hazards.list[hazardStr]);
            }
        }
        
        sectors.push_back(newSector);
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Mobs.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Area -- Object generators");
    }
    
    DataNode* mobsNode = node->getChildByName("mobs");
    vector<std::pair<size_t, size_t> > mobLinksBuffer;
    size_t nMobs = mobsNode->getNrOfChildren();
    for(size_t m = 0; m < nMobs; m++) {
        DataNode* mobNode = mobsNode->getChild(m);
        ReaderSetter mRS(mobNode);
        MobGen* newMob = new MobGen();
        
        string typeStr;
        string linksStr;
        
        mRS.set("p", newMob->pos);
        mRS.set("angle", newMob->angle);
        mRS.set("boss", newMob->isBoss);
        mRS.set("vars", newMob->vars);
        mRS.set("type", typeStr);
        mRS.set("links", linksStr);
        mRS.set("stored_inside", newMob->storedInside);
        
        MobCategory* category =
            game.mobCategories.getFromInternalName(mobNode->name);
        if(category) {
            newMob->type = category->getType(typeStr);
        } else {
            category = game.mobCategories.get(MOB_CATEGORY_NONE);
        }
        
        vector<string> linkStrs = split(linksStr);
        forIdx(l, linkStrs) {
            mobLinksBuffer.push_back(std::make_pair(m, s2i(linkStrs[l])));
        }
        
        bool valid =
            category && category->id != MOB_CATEGORY_NONE && newMob->type;
            
        if(!valid) {
            //Error.
            if(level >= CONTENT_LOAD_LEVEL_FULL) {
                game.errors.report(
                    "Unknown mob type \"" + typeStr + "\" of category \"" +
                    mobNode->name + "\"!",
                    mobNode
                );
            }
        }
        
        mobGenerators.push_back(newMob);
    }
    
    forIdx(l, mobLinksBuffer) {
        size_t f = mobLinksBuffer[l].first;
        size_t s = mobLinksBuffer[l].second;
        mobGenerators[f]->links.push_back(
            mobGenerators[s]
        );
        mobGenerators[f]->linkIdxs.push_back(s);
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Paths.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Area -- Paths");
    }
    
    DataNode* stopsNode = node->getChildByName("path_stops");
    size_t nStops = stopsNode->getNrOfChildren();
    for(size_t s = 0; s < nStops; s++) {
        DataNode* stopNode = stopsNode->getChild(s);
        ReaderSetter sRS(stopNode);
        PathStop* newStop = new PathStop();
        
        sRS.set("pos", newStop->pos);
        sRS.set("radius", newStop->radius);
        sRS.set("flags", newStop->flags);
        sRS.set("label", newStop->label);
        
        DataNode* linksNode = stopNode->getChildByName("links");
        size_t nLinks = linksNode->getNrOfChildren();
        for(size_t l = 0; l < nLinks; l++) {
            string linkData = linksNode->getChild(l)->value;
            vector<string> linkDataParts = split(linkData);
            
            PathLink* newLink =
                new PathLink(newStop, nullptr, s2i(linkDataParts[0]));
            if(linkDataParts.size() >= 2) {
                newLink->type = (PATH_LINK_TYPE) s2i(linkDataParts[1]);
            }
            
            newStop->links.push_back(newLink);
        }
        
        newStop->radius = std::max(newStop->radius, PATHS::MIN_STOP_RADIUS);
        
        pathStops.push_back(newStop);
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Tree shadows.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Area -- Tree shadows");
    }
    
    DataNode* shadowsNode = node->getChildByName("tree_shadows");
    size_t nShadows = shadowsNode->getNrOfChildren();
    for(size_t s = 0; s < nShadows; s++) {
        DataNode* shadowNode = shadowsNode->getChild(s);
        ReaderSetter sRS(shadowNode);
        TreeShadow* newShadow = new TreeShadow();
        
        sRS.set("pos", newShadow->pose.pos);
        sRS.set("size", newShadow->pose.size);
        sRS.set("angle", newShadow->pose.angle);
        sRS.set("alpha", newShadow->alpha);
        sRS.set("file", newShadow->bmpName);
        sRS.set("sway", newShadow->sway);
        
        newShadow->bitmap =
            game.content.bitmaps.list.get(newShadow->bmpName, nullptr);
        if(
            newShadow->bitmap == game.bmpError &&
            level >= CONTENT_LOAD_LEVEL_FULL
        ) {
            game.errors.report(
                "Unknown tree shadow texture \"" + newShadow->bmpName + "\"!",
                shadowNode
            );
        }
        
        treeShadows.push_back(newShadow);
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Regions.
    
    DataNode* regionsNode = node->getChildByName("regions");
    size_t nRegions = regionsNode->getNrOfChildren();
    for(size_t r = 0; r < nRegions; r++) {
        DataNode* regionNode = regionsNode->getChild(r);
        ReaderSetter rRS(regionNode);
        AreaRegion* newRegion = new AreaRegion();
        
        rRS.set("center", newRegion->center);
        rRS.set("size", newRegion->size);
        
        regions.push_back(newRegion);
    }
    
    //Set up stuff.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Area -- Geometry calculations");
    }
    
    forIdx(e, edges) {
        fixEdgePointers(edges[e]);
    }
    forIdx(s, sectors) {
        connectSectorEdges(sectors[s]);
    }
    forIdx(v, vertexes) {
        connectVertexEdges(vertexes[v]);
    }
    forIdx(s, pathStops) {
        fixPathStopPointers(pathStops[s]);
    }
    forIdx(s, pathStops) {
        pathStops[s]->calculateDists();
    }
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        //Fade sectors that also fade brightness should be
        //at midway between the two neighbors.
        forIdx(s, sectors) {
            Sector* sPtr = sectors[s];
            if(sPtr->fade) {
                Sector* n1 = nullptr;
                Sector* n2 = nullptr;
                sPtr->getTextureMergeSectors(&n1, &n2);
                if(n1 && n2) {
                    sPtr->brightness = (n1->brightness + n2->brightness) / 2.0f;
                }
            }
        }
    }
    
    
    //Triangulate everything and save bounding boxes.
    set<Edge*> loneEdges;
    forIdx(s, sectors) {
        Sector* sPtr = sectors[s];
        sPtr->triangles.clear();
        TRIANGULATION_ERROR res =
            triangulateSector(sPtr, &loneEdges, false);
            
        if(
            res != TRIANGULATION_ERROR_NONE &&
            level == CONTENT_LOAD_LEVEL_EDITOR
        ) {
            problems.nonSimples[sPtr] = res;
            problems.loneEdges.insert(
                loneEdges.begin(), loneEdges.end()
            );
        }
        
        sPtr->calculateBoundingBox();
    }
    
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) generateBlockmap();
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
}


/**
 * @brief Loads the area's main data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void Area::loadMainDataFromDataNode(
    DataNode* node, CONTENT_LOAD_LEVEL level
) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Area configuration data.
    ReaderSetter aRS(node);
    
    DataNode* weatherNode = nullptr;
    DataNode* songNode = nullptr;
    
    aRS.set("subtitle", subtitle);
    aRS.set("difficulty", difficulty);
    aRS.set("spray_amounts", sprayAmounts);
    aRS.set("song", songName, &songNode);
    aRS.set("weather", weatherName, &weatherNode);
    aRS.set("day_time_start", dayTimeStart);
    aRS.set("day_time_speed", dayTimeSpeed);
    aRS.set("bg_bmp", bgBmpName);
    aRS.set("bg_color", bgColor);
    aRS.set("bg_dist", bgDist);
    aRS.set("bg_zoom", bgBmpZoom);
    aRS.set("max_pikmin_in_field", maxPikminInField);
    aRS.set("onions_auto_eject", onionsAutoEject);
    aRS.set("onions_eject_grown_pikmin", onionsEjectGrownPikmin);
    
    //Weather.
    if(level > CONTENT_LOAD_LEVEL_BASIC) {
        if(weatherName.empty()) {
            weatherCondition = Weather();
            
        } else if(!isInMap(game.content.weatherConditions.list, weatherName)) {
            game.errors.report(
                "Unknown weather condition \"" + weatherName + "\"!",
                weatherNode
            );
            weatherCondition = Weather();
            
        } else {
            weatherCondition =
                game.content.weatherConditions.list[weatherName];
                
        }
        
        //Song.
        if(!songName.empty() && !isInMap(game.content.songs.list, songName)) {
            game.errors.report(
                "Unknown song \"" + songName + "\"!",
                songNode
            );
        }
    }
    
    if(level >= CONTENT_LOAD_LEVEL_FULL && !bgBmpName.empty()) {
        bgBmp = game.content.bitmaps.list.get(bgBmpName, node);
    }
}


/**
 * @brief Loads the area's mission data from a data node.
 *
 * @param node Data node to load from.
 */
void Area::loadMissionDataFromDataNode(DataNode* node) {
    mission.hudItems.clear();
    mission.hudItems.insert(
        mission.hudItems.begin(),
        enumGetCount(missionHudItemIdNames), MissionHudItem()
    );
    
    ReaderSetter mRS(node);
    string presetStr;
    string medalAwardModeStr;
    DataNode* presetNode = nullptr;
    DataNode* medalAwardModeNode = nullptr;
    string briefingNotesStr;
    
    //DEPRECATED in 1.2.0 by the new mission system.
    if(node->getNrOfChildrenByName("mission_goal") > 0) {
        loadOldMissionSystem(node);
    }
    
    //General properties.
    mRS.set("mission_preset", presetStr, &presetNode);
    mRS.set("mission_time_limit", mission.timeLimit);
    mRS.set("mission_medal_award_mode", medalAwardModeStr, &medalAwardModeNode);
    mRS.set("mission_starting_points", mission.startingPoints);
    mRS.set("mission_bronze_req", mission.bronzeReq);
    mRS.set("mission_silver_req", mission.silverReq);
    mRS.set("mission_gold_req", mission.goldReq);
    mRS.set("mission_platinum_req", mission.platinumReq);
    mRS.set("mission_briefing_objective", mission.briefingObjective);
    mRS.set("mission_briefing_notes", briefingNotesStr);
    mRS.set("mission_maker_record", mission.makerRecord);
    mRS.set("mission_maker_record_date", mission.makerRecordDate);
    
    if(presetNode) {
        readEnumProp(
            missionPresetNames, presetStr,
            &mission.preset, "mission preset", presetNode
        );
    }
    if(medalAwardModeNode) {
        readEnumProp(
            missionMedalAwardModeNames, medalAwardModeStr,
            &mission.medalAwardMode, "mission medal award mode",
            medalAwardModeNode
        );
    }
    mission.briefingNotes = semicolonListToVector(briefingNotesStr);
    
    //End conditions.
    DataNode* condsNode = node->getChildByName("mission_end_conditions");
    size_t nConds = condsNode->getNrOfChildren();
    for(size_t c = 0; c < nConds; c++) {
        DataNode* condNode = condsNode->getChild(c);
        MissionEndCond newCond;
        
        ReaderSetter cRS(condNode);
        string typeStr;
        string metricTypeStr;
        int matchAmountInt = 0;
        DataNode* typeNode = nullptr;
        DataNode* metricTypeNode = nullptr;
        
        cRS.set("type", typeStr, &typeNode);
        cRS.set("metric_type", metricTypeStr, &metricTypeNode);
        cRS.set("index_param", newCond.idxParam);
        cRS.set("match_amount", matchAmountInt);
        cRS.set("clear", newCond.clear);
        cRS.set("zero_time_for_score", newCond.zeroTimeForScore);
        cRS.set("neutral_mood", newCond.neutralMood);
        cRS.set("reason", newCond.reason);
        
        readEnumProp(
            missionEndCondNames, typeStr, &newCond.type,
            "mission end condition type", typeNode
        );
        readEnumProp(
            missionMetricNames, metricTypeStr, &newCond.metricType,
            "mission end condition metric type", metricTypeNode
        );
        newCond.matchAmount = matchAmountInt == -1 ? INVALID : matchAmountInt;
        
        mission.endConds.push_back(newCond);
    }
    
    //Mob groups.
    DataNode* mobGroupsNode =
        node->getChildByName("mission_mob_groups");
    size_t nMobGroups = mobGroupsNode->getNrOfChildren();
    for(size_t g = 0; g < nMobGroups; g++) {
        DataNode* groupNode = mobGroupsNode->getChild(g);
        MissionMobGroup newGroup;
        
        ReaderSetter cRS(groupNode);
        string typeStr;
        string mobIdxsStr;
        DataNode* typeNode = nullptr;
        
        cRS.set("type", typeStr);
        cRS.set("enemies_need_collection", newGroup.enemiesNeedCollection);
        cRS.set("highlight_on_radar", newGroup.highlightOnRadar);
        cRS.set("mob_idxs", mobIdxsStr);
        
        readEnumProp(
            missionMobGroupTypeNames, typeStr, &newGroup.type,
            "mission mob group type", typeNode
        );
        
        vector<string> mobIdxsStrVec = semicolonListToVector(mobIdxsStr);
        newGroup.mobIdxs.reserve(mobIdxsStrVec.size());
        forIdx(m, mobIdxsStrVec) {
            newGroup.mobIdxs.push_back(s2i(mobIdxsStrVec[m]));
        }
        
        mission.mobGroups.push_back(newGroup);
    }
    
    //HUD items.
    DataNode* itemsNode = node->getChildByName("mission_hud_items");
    size_t nItems = itemsNode->getNrOfChildren();
    for(size_t i = 0; i < nItems; i++) {
        DataNode* itemNode = itemsNode->getChild(i);
        MissionHudItem newItem;
        
        ReaderSetter iRS(itemNode);
        string displayTypeStr;
        string metricTypeStr;
        int totalAmountInt = 0;
        DataNode* displayTypeNode = nullptr;
        DataNode* metricTypeNode = nullptr;
        
        iRS.set("enabled", newItem.enabled);
        iRS.set("display_type", displayTypeStr);
        iRS.set("metric_type", metricTypeStr);
        iRS.set("index_param", newItem.idxParam);
        iRS.set("text", newItem.text);
        iRS.set("total_amount", totalAmountInt);
        
        readEnumProp(
            missionHudItemDisplayTypeNames, displayTypeStr,
            &newItem.displayType, "mission HUD item display type",
            displayTypeNode
        );
        readEnumProp(
            missionMetricNames, metricTypeStr,
            &newItem.metricType, "mission HUD item metric type",
            metricTypeNode
        );
        newItem.totalAmount = totalAmountInt == -1 ? INVALID : totalAmountInt;
        
        mission.hudItems[i] = newItem;
    }
    
    //Score criteria.
    DataNode* scoreCriteriaNode =
        node->getChildByName("mission_score_criteria");
    size_t nScoreCriteria = scoreCriteriaNode->getNrOfChildren();
    for(size_t c = 0; c < nScoreCriteria; c++) {
        DataNode* criterionNode = scoreCriteriaNode->getChild(c);
        MissionScoreCriterion newCriterion;
        
        ReaderSetter cRS(criterionNode);
        string metricTypeStr;
        DataNode* metricTypeNode;
        
        cRS.set("type", metricTypeStr, &metricTypeNode);
        cRS.set("index_param", newCriterion.idxParam);
        cRS.set("points", newCriterion.points);
        cRS.set("affects_hud", newCriterion.affectsHud);
        
        readEnumProp(
            missionMetricNames, metricTypeStr,
            &newCriterion.metricType, "mission score criterion metric type",
            metricTypeNode
        );
        
        mission.scoreCriteria.push_back(newCriterion);
    }
}


/**
 * @brief Loads deprecated (pre-1.2.0) mission information.
 *
 * @param node Data node to load from.
 */
void Area::loadOldMissionSystem(DataNode* node) {
    ReaderSetter mRS(node);
    MISSION_GOAL_OLD goal = MISSION_GOAL_OLD_END_MANUALLY;
    string goalStr;
    string requiredMobsStr;
    unordered_set<size_t> goalMobIdxs;
    size_t goalAmount = 0;
    bool goalAllMobs = true;
    Point goalExitCenter;
    Point goalExitSize;
    Bitmask8 failConditions = 0;
    size_t failTooFewPikAmount = 0;
    size_t failTooManyPikAmount = 0;
    size_t failPikKilled = 0;
    size_t failLeadersKod = 0;
    size_t failEnemiesDefeated = 0;
    size_t failTimeLimit = 0;
    size_t failHudPrimaryCond = INVALID;
    size_t failHudSecondaryCond = INVALID;
    int pointsPerPikminBorn = 0;
    int pointsPerPikminDeath = 0;
    int pointsPerSecLeft = 0;
    int pointsPerSecPassed = 0;
    int pointsPerTreasurePoint = 0;
    int pointsPerEnemyPoint = 0;
    bool enemyPointsOnCollection = false;
    Bitmask8 pointLossData = 0;
    Bitmask8 pointHudData = 255;
    int medalAwardModeInt = 0;
    
    //General properties.
    mRS.set("mission_goal", goalStr);
    mRS.set("mission_goal_amount", goalAmount);
    mRS.set("mission_goal_all_mobs", goalAllMobs);
    mRS.set("mission_required_mobs", requiredMobsStr);
    mRS.set("mission_goal_exit_center", goalExitCenter);
    mRS.set("mission_goal_exit_size", goalExitSize);
    mRS.set("mission_fail_conditions", failConditions);
    mRS.set("mission_fail_too_few_pik_amount", failTooFewPikAmount);
    mRS.set("mission_fail_too_many_pik_amount", failTooManyPikAmount);
    mRS.set("mission_fail_pik_killed", failPikKilled);
    mRS.set("mission_fail_leaders_kod", failLeadersKod);
    mRS.set("mission_fail_enemies_defeated", failEnemiesDefeated);
    mRS.set("mission_fail_time_limit", failTimeLimit);
    mRS.set("mission_fail_hud_primary_cond", failHudPrimaryCond);
    mRS.set("mission_fail_hud_secondary_cond", failHudSecondaryCond);
    mRS.set("mission_points_per_pikmin_born", pointsPerPikminBorn);
    mRS.set("mission_points_per_pikmin_death", pointsPerPikminDeath);
    mRS.set("mission_points_per_sec_left", pointsPerSecLeft);
    mRS.set("mission_points_per_sec_passed", pointsPerSecPassed);
    mRS.set(
        "mission_points_per_treasure_point", pointsPerTreasurePoint
    );
    mRS.set("mission_points_per_enemy_point", pointsPerEnemyPoint);
    mRS.set("enemy_points_on_collection", enemyPointsOnCollection);
    mRS.set("mission_point_loss_data", pointLossData);
    mRS.set("mission_point_hud_data", pointHudData);
    mRS.set("mission_grading_mode", medalAwardModeInt);
    
    mission.medalAwardMode = (MISSION_MEDAL_AWARD_MODE) medalAwardModeInt;
    
    //Goal.
    goal = enumGetValue(missionGoalNames, goalStr);
    vector<string> missionRequiredMobsStr =
        semicolonListToVector(requiredMobsStr);
    goalMobIdxs.reserve(missionRequiredMobsStr.size());
    forIdx(m, missionRequiredMobsStr) {
        goalMobIdxs.insert(
            s2i(missionRequiredMobsStr[m])
        );
    }
    
    //Automatically turn the pause menu fail condition on/off for convenience.
    if(goal == MISSION_GOAL_OLD_END_MANUALLY) {
        disableFlag(
            failConditions,
            getIdxBitmask(MISSION_FAIL_COND_OLD_PAUSE_MENU)
        );
    } else {
        enableFlag(
            failConditions,
            getIdxBitmask(MISSION_FAIL_COND_OLD_PAUSE_MENU)
        );
    }
    
    //Automatically turn off the seconds left score criterion for convenience.
    if(
        !hasFlag(
            failConditions,
            getIdxBitmask(MISSION_FAIL_COND_OLD_TIME_LIMIT)
        )
    ) {
        pointsPerSecLeft = 0;
        disableFlag(
            pointHudData,
            getIdxBitmask(MISSION_SCORE_CRITERIA_OLD_SEC_LEFT)
        );
        disableFlag(
            pointLossData,
            getIdxBitmask(MISSION_SCORE_CRITERIA_OLD_SEC_LEFT)
        );
    }
    
    //Port the goal to end conditions + mob groups.
    size_t goalMobGroupIdx = 0;
    size_t exitRegionIdx = 0;
    
    switch(goal) {
    case MISSION_GOAL_OLD_END_MANUALLY: {
        break;
        
    } case MISSION_GOAL_OLD_COLLECT_TREASURE: {
        mission.briefingObjective =
            "Collect treasures!";
        mission.mobGroups.push_back(
        MissionMobGroup {
            .type = MISSION_MOB_GROUP_TREASURES,
            .highlightOnRadar = true,
            .mobIdxs =
            vector<size_t>(goalMobIdxs.begin(), goalMobIdxs.end()),
        }
        );
        goalMobGroupIdx = mission.mobGroups.size() - 1;
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = true,
            .zeroTimeForScore = true,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_MORE,
            .metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS,
            .idxParam = goalMobGroupIdx,
            .matchAmount = goalAllMobs ? INVALID : goalAmount,
            .clear = true,
            .neutralMood = false,
            .reason = "Got all treasures!",
        }
        );
        break;
        
    } case MISSION_GOAL_OLD_BATTLE_ENEMIES: {
        mission.briefingObjective =
            "Battle enemies!";
        mission.mobGroups.push_back(
        MissionMobGroup {
            .type = MISSION_MOB_GROUP_ENEMIES,
            .enemiesNeedCollection = enemyPointsOnCollection,
            .highlightOnRadar = true,
            .mobIdxs =
            vector<size_t>(goalMobIdxs.begin(), goalMobIdxs.end()),
        }
        );
        goalMobGroupIdx = mission.mobGroups.size() - 1;
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = true,
            .zeroTimeForScore = true,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_MORE,
            .metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS,
            .idxParam = goalMobGroupIdx,
            .matchAmount = goalAllMobs ? INVALID : goalAmount,
            .clear = true,
            .neutralMood = false,
            .reason = "Got all enemies!",
        }
        );
        break;
        
    } case MISSION_GOAL_OLD_TIMED_SURVIVAL: {
        mission.briefingObjective =
            "Survive until the time limit!";
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = false,
            .zeroTimeForScore = false,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_LESS,
            .metricType = MISSION_METRIC_SECS_LEFT,
            .matchAmount = 0,
            .clear = true,
            .neutralMood = false,
            .reason = "Survived!",
        }
        );
        mission.timeLimit = goalAmount;
        break;
        
    } case MISSION_GOAL_OLD_GET_TO_EXIT: {
        mission.briefingObjective =
            "Get the leaders to the exit!";
        regions.push_back(
        new AreaRegion {
            .center = goalExitCenter,
            .size = goalExitSize,
        }
        );
        exitRegionIdx = regions.size() - 1;
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = false,
            .zeroTimeForScore = true,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_MORE,
            .metricType = MISSION_METRIC_LEADERS_IN_REGION,
            .idxParam = exitRegionIdx,
            .matchAmount = goalAllMobs ? INVALID : goalMobIdxs.size(),
            .clear = true,
            .neutralMood = false,
            .reason = "Got to the exit!",
        }
        );
        break;
        
    } case MISSION_GOAL_OLD_GROW_PIKMIN: {
        mission.briefingObjective =
            "Grow " + i2s(goalAmount) + " Pikmin!";
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = false,
            .zeroTimeForScore = true,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_MORE,
            .metricType = MISSION_METRIC_LIVING_PIKMIN,
            .matchAmount = goalAmount,
            .clear = true,
            .neutralMood = false,
            .reason = "Grew " + i2s(goalAmount) + " Pikmin!",
        }
        );
        break;
    }
    }
    
    //Port the fail conditions to end conditions.
    if(
        hasFlag(
            failConditions, getIdxBitmask(MISSION_FAIL_COND_OLD_TIME_LIMIT)
        )
    ) {
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_LESS,
            .metricType = MISSION_METRIC_SECS_LEFT,
            .matchAmount = 0,
            .clear = false,
            .zeroTimeForScore =
            hasFlag(pointLossData, MISSION_FAIL_COND_OLD_TIME_LIMIT),
            .neutralMood = false,
            .reason = "Time's up!",
        }
        );
        mission.timeLimit = failTimeLimit;
    }
    if(
        hasFlag(
            failConditions, getIdxBitmask(MISSION_FAIL_COND_OLD_TOO_FEW_PIKMIN)
        )
    ) {
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_LESS,
            .metricType = MISSION_METRIC_LIVING_PIKMIN,
            .matchAmount = failTooFewPikAmount,
            .clear = false,
            .neutralMood = false,
            .reason = "Reached " + i2s(failTooFewPikAmount) + " Pikmin!",
        }
        );
    }
    if(
        hasFlag(
            failConditions, getIdxBitmask(MISSION_FAIL_COND_OLD_TOO_MANY_PIKMIN)
        )
    ) {
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_MORE,
            .metricType = MISSION_METRIC_LIVING_PIKMIN,
            .matchAmount = failTooManyPikAmount,
            .clear = false,
            .neutralMood = false,
            .reason = "Reached " + i2s(failTooManyPikAmount) + " Pikmin!",
        }
        );
    }
    if(
        hasFlag(
            failConditions, getIdxBitmask(MISSION_FAIL_COND_OLD_LOSE_PIKMIN)
        )
    ) {
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_MORE,
            .metricType = MISSION_METRIC_PIKMIN_LOST,
            .matchAmount = failPikKilled,
            .clear = false,
            .neutralMood = false,
            .reason = "Lost " + i2s(failPikKilled) + " Pikmin!",
        }
        );
    }
    if(
        hasFlag(
            failConditions, getIdxBitmask(MISSION_FAIL_COND_OLD_TAKE_DAMAGE)
        )
    ) {
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_TAKE_DAMAGE,
            .clear = false,
            .neutralMood = false,
            .reason = "Took damage!",
        }
        );
    }
    if(
        hasFlag(
            failConditions, getIdxBitmask(MISSION_FAIL_COND_OLD_LOSE_LEADERS)
        )
    ) {
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_MORE,
            .metricType = MISSION_METRIC_LEADERS_LOST,
            .matchAmount = failLeadersKod,
            .clear = false,
            .neutralMood = false,
            .reason = "Lost " + i2s(failLeadersKod) + " leaders!",
        }
        );
    }
    size_t enemyDefeatFailIdx = 0;
    if(
        hasFlag(
            failConditions, getIdxBitmask(MISSION_FAIL_COND_OLD_DEFEAT_ENEMIES)
        )
    ) {
        mission.mobGroups.push_back(
        MissionMobGroup {
            .type = MISSION_MOB_GROUP_ENEMIES,
        }
        );
        enemyDefeatFailIdx = mission.mobGroups.size() - 1;
        mission.endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_METRIC_OR_MORE,
            .metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS,
            .idxParam = enemyDefeatFailIdx,
            .matchAmount = failEnemiesDefeated,
            .clear = false,
            .neutralMood = false,
            .reason = "Defeated " + i2s(failEnemiesDefeated) + " enemies!",
        }
        );
    }
    if(
        hasFlag(
            failConditions, getIdxBitmask(MISSION_FAIL_COND_OLD_PAUSE_MENU)
        )
    ) {
        mission.endConds[0].clear = false;
    }
    
    //Port the goal HUD items.
    switch(goal) {
    case MISSION_GOAL_OLD_END_MANUALLY: {
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled = true;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
            MISSION_HUD_ITEM_DISPLAY_TEXT;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].text =
            "End whenever you want";
        break;
        
    } case MISSION_GOAL_OLD_COLLECT_TREASURE: {
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled = true;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
            MISSION_HUD_ITEM_DISPLAY_CUR_TOT;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].metricType =
            MISSION_METRIC_MOB_GROUP_CLEARED_MOBS;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].totalAmount =
            goalAllMobs ? INVALID : goalAmount;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].idxParam = goalMobGroupIdx;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].text = "Treasures:";
        break;
        
    } case MISSION_GOAL_OLD_BATTLE_ENEMIES: {
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled = true;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
            MISSION_HUD_ITEM_DISPLAY_CUR_TOT;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].metricType =
            MISSION_METRIC_MOB_GROUP_CLEARED_MOBS;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].totalAmount =
            goalAllMobs ? INVALID : goalAmount;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].idxParam = goalMobGroupIdx;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].text = "Enemies:";
        break;
        
    } case MISSION_GOAL_OLD_TIMED_SURVIVAL: {
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled = true;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
            MISSION_HUD_ITEM_DISPLAY_TEXT;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].text = "Survive";
        
        mission.hudItems[MISSION_HUD_ITEM_ID_CLOCK].enabled = true;
        mission.hudItems[MISSION_HUD_ITEM_ID_CLOCK].displayType =
            MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN;
        mission.hudItems[MISSION_HUD_ITEM_ID_CLOCK].text = "Time:";
        break;
        
    } case MISSION_GOAL_OLD_GET_TO_EXIT: {
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled = true;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
            MISSION_HUD_ITEM_DISPLAY_CUR_TOT;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].metricType =
            MISSION_METRIC_LEADERS_IN_REGION;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].totalAmount =
            goalAllMobs ? INVALID : goalMobIdxs.size();
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].idxParam = exitRegionIdx;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].text = "In exit:";
        break;
        
    } case MISSION_GOAL_OLD_GROW_PIKMIN: {
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled = true;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
            MISSION_HUD_ITEM_DISPLAY_CUR_TOT;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].metricType =
            MISSION_METRIC_LIVING_PIKMIN;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].totalAmount =
            goalAmount;
        mission.hudItems[MISSION_HUD_ITEM_ID_GOAL].text = "Pikmin:";
        break;
        
    }
    }
    
    if(
        ((MISSION_MEDAL_AWARD_MODE) medalAwardModeInt) ==
        MISSION_MEDAL_AWARD_MODE_POINTS &&
        pointHudData != 0
    ) {
        mission.hudItems[MISSION_HUD_ITEM_ID_SCORE].enabled = true;
        mission.hudItems[MISSION_HUD_ITEM_ID_SCORE].displayType =
            MISSION_HUD_ITEM_DISPLAY_SCORE;
    }
    
    //Port the failure HUD items.
    if(
        failHudPrimaryCond == MISSION_FAIL_COND_OLD_TIME_LIMIT ||
        failHudSecondaryCond == MISSION_FAIL_COND_OLD_TIME_LIMIT
    ) {
        MISSION_HUD_ITEM_ID id =
            failHudPrimaryCond == MISSION_FAIL_COND_OLD_TIME_LIMIT ?
            MISSION_HUD_ITEM_ID_CLOCK :
            MISSION_HUD_ITEM_ID_MISC;
        mission.hudItems[id].enabled = true;
        mission.hudItems[id].displayType = MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN;
        mission.hudItems[id].totalAmount = INVALID;
        mission.hudItems[id].text = "Time:";
    }
    if(
        failHudPrimaryCond == MISSION_FAIL_COND_OLD_TOO_FEW_PIKMIN ||
        failHudSecondaryCond == MISSION_FAIL_COND_OLD_TOO_FEW_PIKMIN
    ) {
        MISSION_HUD_ITEM_ID id =
            failHudPrimaryCond == MISSION_FAIL_COND_OLD_TOO_FEW_PIKMIN ?
            MISSION_HUD_ITEM_ID_CLOCK :
            MISSION_HUD_ITEM_ID_MISC;
        mission.hudItems[id].enabled = true;
        mission.hudItems[id].displayType = MISSION_HUD_ITEM_DISPLAY_CUR_TOT;
        mission.hudItems[id].metricType = MISSION_METRIC_LIVING_PIKMIN;
        mission.hudItems[id].totalAmount = failTooFewPikAmount;
        mission.hudItems[id].text = "Pikmin:";
    }
    if(
        failHudPrimaryCond == MISSION_FAIL_COND_OLD_TOO_MANY_PIKMIN ||
        failHudSecondaryCond == MISSION_FAIL_COND_OLD_TOO_MANY_PIKMIN
    ) {
        MISSION_HUD_ITEM_ID id =
            failHudPrimaryCond == MISSION_FAIL_COND_OLD_TOO_MANY_PIKMIN ?
            MISSION_HUD_ITEM_ID_CLOCK :
            MISSION_HUD_ITEM_ID_MISC;
        mission.hudItems[id].enabled = true;
        mission.hudItems[id].displayType = MISSION_HUD_ITEM_DISPLAY_CUR_TOT;
        mission.hudItems[id].metricType = MISSION_METRIC_LIVING_PIKMIN;
        mission.hudItems[id].totalAmount = failTooManyPikAmount;
        mission.hudItems[id].text = "Pikmin:";
    }
    if(
        failHudPrimaryCond == MISSION_FAIL_COND_OLD_LOSE_PIKMIN ||
        failHudSecondaryCond == MISSION_FAIL_COND_OLD_LOSE_PIKMIN
    ) {
        MISSION_HUD_ITEM_ID id =
            failHudPrimaryCond == MISSION_FAIL_COND_OLD_LOSE_PIKMIN ?
            MISSION_HUD_ITEM_ID_CLOCK :
            MISSION_HUD_ITEM_ID_MISC;
        mission.hudItems[id].enabled = true;
        mission.hudItems[id].displayType = MISSION_HUD_ITEM_DISPLAY_CUR_TOT;
        mission.hudItems[id].metricType = MISSION_METRIC_PIKMIN_LOST;
        mission.hudItems[id].totalAmount = failPikKilled;
        mission.hudItems[id].text = "Pikmin lost:";
    }
    if(
        failHudPrimaryCond == MISSION_FAIL_COND_OLD_LOSE_LEADERS ||
        failHudSecondaryCond == MISSION_FAIL_COND_OLD_LOSE_LEADERS
    ) {
        MISSION_HUD_ITEM_ID id =
            failHudPrimaryCond == MISSION_FAIL_COND_OLD_LOSE_LEADERS ?
            MISSION_HUD_ITEM_ID_CLOCK :
            MISSION_HUD_ITEM_ID_MISC;
        mission.hudItems[id].enabled = true;
        mission.hudItems[id].displayType = MISSION_HUD_ITEM_DISPLAY_CUR_TOT;
        mission.hudItems[id].metricType = MISSION_METRIC_LEADERS_LOST;
        mission.hudItems[id].totalAmount = failLeadersKod;
        mission.hudItems[id].text = "Leaders lost:";
    }
    if(
        failHudPrimaryCond == MISSION_FAIL_COND_OLD_DEFEAT_ENEMIES ||
        failHudSecondaryCond == MISSION_FAIL_COND_OLD_DEFEAT_ENEMIES
    ) {
        MISSION_HUD_ITEM_ID id =
            failHudPrimaryCond == MISSION_FAIL_COND_OLD_DEFEAT_ENEMIES ?
            MISSION_HUD_ITEM_ID_CLOCK :
            MISSION_HUD_ITEM_ID_MISC;
        mission.hudItems[id].enabled = true;
        mission.hudItems[id].displayType = MISSION_HUD_ITEM_DISPLAY_CUR_TOT;
        mission.hudItems[id].metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS;
        mission.hudItems[id].idxParam = enemyDefeatFailIdx;
        mission.hudItems[id].text = "Enemies:";
    }
    
    //Port the score criteria.
    if(pointsPerPikminBorn != 0) {
        mission.scoreCriteria.push_back(
        MissionScoreCriterion {
            .metricType = MISSION_METRIC_PIKMIN_BORN,
            .points = pointsPerPikminBorn,
            .affectsHud =
            hasFlag(
                pointHudData,
                getIdxBitmask(MISSION_SCORE_CRITERIA_OLD_PIKMIN_BORN)
            ),
        }
        );
    }
    if(pointsPerPikminDeath != 0) {
        mission.scoreCriteria.push_back(
        MissionScoreCriterion {
            .metricType = MISSION_METRIC_PIKMIN_LOST,
            .points = pointsPerPikminDeath,
            .affectsHud =
            hasFlag(
                pointHudData,
                getIdxBitmask(MISSION_SCORE_CRITERIA_OLD_PIKMIN_DEATH)
            ),
        }
        );
    }
    if(pointsPerSecLeft != 0) {
        mission.scoreCriteria.push_back(
        MissionScoreCriterion {
            .metricType = MISSION_METRIC_SECS_LEFT,
            .points = pointsPerSecLeft,
            .affectsHud =
            hasFlag(
                pointHudData,
                getIdxBitmask(MISSION_SCORE_CRITERIA_OLD_SEC_LEFT)
            ),
        }
        );
    }
    if(pointsPerSecPassed != 0) {
        mission.scoreCriteria.push_back(
        MissionScoreCriterion {
            .metricType = MISSION_METRIC_SECS_PASSED,
            .points = pointsPerSecPassed,
            .affectsHud =
            hasFlag(
                pointHudData,
                getIdxBitmask(MISSION_SCORE_CRITERIA_OLD_SEC_PASSED)
            ),
        }
        );
    }
    if(pointsPerTreasurePoint != 0) {
        mission.scoreCriteria.push_back(
        MissionScoreCriterion {
            .metricType = MISSION_METRIC_TREASURE_COLLECTION_PTS,
            .points = pointsPerTreasurePoint,
            .affectsHud =
            hasFlag(
                pointHudData,
                getIdxBitmask(MISSION_SCORE_CRITERIA_OLD_TREASURE_POINTS)
            ),
        }
        );
    }
    if(pointsPerEnemyPoint != 0) {
        mission.scoreCriteria.push_back(
        MissionScoreCriterion {
            .metricType =
            enemyPointsOnCollection ?
            MISSION_METRIC_ENEMY_COLLECTION_PTS :
            MISSION_METRIC_ENEMY_DEFEAT_PTS,
            .points = pointsPerEnemyPoint,
            .affectsHud =
            hasFlag(
                pointHudData,
                getIdxBitmask(MISSION_SCORE_CRITERIA_OLD_ENEMY_POINTS)
            ),
        }
        );
    }
    
    //Port the subtitle, since it's used in mission score records.
    if(subtitle.empty()) {
        subtitle = enumGetName(missionGoalNames, goal);
    }
}


/**
 * @brief Loads the area maker's reminders from a data node.
 *
 * @param node Data node to load from.
 */
void Area::loadRemindersFromDataNode(DataNode* node) {
    reminders.clear();
    size_t nReminders = node->getNrOfChildren();
    for(size_t r = 0; r < nReminders; r++) {
        DataNode* reminderNode = node->getChild(r);
        AreaMakerReminder newReminder;
        
        ReaderSetter rRS(reminderNode);
        
        rRS.set("pos", newReminder.pos);
        rRS.set("text", newReminder.text);
        
        reminders.push_back(newReminder);
    }
}


/**
 * @brief Loads the thumbnail image from the disk and updates the
 * thumbnail class member.
 *
 * @param thumbnailPath Path to the bitmap.
 */
void Area::loadThumbnail(const string& thumbnailPath) {
    if(thumbnail) {
        thumbnail = nullptr;
    }
    
    if(al_filename_exists(thumbnailPath.c_str())) {
        thumbnail =
            std::shared_ptr<ALLEGRO_BITMAP>(
                al_load_bitmap(thumbnailPath.c_str()),
        [](ALLEGRO_BITMAP * b) {
            al_destroy_bitmap(b);
        }
            );
    }
}


/**
 * @brief Saves the area's geometry to a data node.
 *
 * @param node Data node to save to.
 */
void Area::saveGeometryToDataNode(DataNode* node) {
    //Vertexes.
    DataNode* vertexesNode = node->addNew("vertexes");
    forIdx(v, vertexes) {
    
        //Vertex.
        Vertex* vPtr = vertexes[v];
        vertexesNode->addNew("v", p2s(v2p(vPtr)));
    }
    
    //Edges.
    DataNode* edgesNode = node->addNew("edges");
    forIdx(e, edges) {
    
        //Edge.
        Edge* ePtr = edges[e];
        DataNode* edgeNode = edgesNode->addNew("e");
        GetterWriter eGW(edgeNode);
        
        string sStr;
        for(size_t s = 0; s < 2; s++) {
            if(ePtr->sectorIdxs[s] == INVALID) sStr += "-1";
            else sStr += i2s(ePtr->sectorIdxs[s]);
            sStr += " ";
        }
        sStr.erase(sStr.size() - 1);
        string vStr =
            i2s(ePtr->vertexIdxs[0]) + " " + i2s(ePtr->vertexIdxs[1]);
            
        eGW.write("s", sStr);
        eGW.write("v", vStr);
        
        if(ePtr->wallShadowLength != LARGE_FLOAT) {
            eGW.write("shadow_length", ePtr->wallShadowLength);
        }
        
        if(ePtr->wallShadowColor != GEOMETRY::SHADOW_DEF_COLOR) {
            eGW.write("shadow_color", ePtr->wallShadowColor);
        }
        
        if(ePtr->ledgeSmoothingLength != 0.0f) {
            eGW.write("smoothing_length", ePtr->ledgeSmoothingLength);
        }
        
        if(ePtr->ledgeSmoothingColor != GEOMETRY::SMOOTHING_DEF_COLOR) {
            eGW.write("smoothing_color", ePtr->ledgeSmoothingColor);
        }
    }
    
    //Sectors.
    DataNode* sectorsNode = node->addNew("sectors");
    forIdx(s, sectors) {
    
        //Sector.
        Sector* sPtr = sectors[s];
        DataNode* sectorNode = sectorsNode->addNew("s");
        GetterWriter sGW(sectorNode);
        
        if(sPtr->type != SECTOR_TYPE_NORMAL) {
            sGW.write("type", enumGetName(sectorTypeINames, sPtr->type));
        }
        if(sPtr->isBottomlessPit) {
            sGW.write("is_bottomless_pit", true);
        }
        sGW.write("z", sPtr->z);
        if(sPtr->brightness != GEOMETRY::DEF_SECTOR_BRIGHTNESS) {
            sGW.write("brightness", sPtr->brightness);
        }
        if(!sPtr->vars.empty()) {
            sGW.write("vars", sPtr->vars);
        }
        if(sPtr->fade) {
            sGW.write("fade", sPtr->fade);
        }
        if(sPtr->hazard) {
            sGW.write("hazard", sPtr->hazard->manifest->internalName);
            sGW.write("hazards_floor", sPtr->hazardFloor);
        }
        
        if(!sPtr->textureInfo.bmpName.empty()) {
            sGW.write("texture", sPtr->textureInfo.bmpName);
        }
        
        if(sPtr->textureInfo.tf.rot != 0) {
            sGW.write("texture_rotate", sPtr->textureInfo.tf.rot);
        }
        if(
            sPtr->textureInfo.tf.scale.x != 1 ||
            sPtr->textureInfo.tf.scale.y != 1
        ) {
            sGW.write("texture_scale", sPtr->textureInfo.tf.scale);
        }
        if(
            sPtr->textureInfo.tf.trans.x != 0 ||
            sPtr->textureInfo.tf.trans.y != 0
        ) {
            sGW.write("texture_trans", sPtr->textureInfo.tf.trans);
        }
        if(
            sPtr->textureInfo.tint.r != 1.0 ||
            sPtr->textureInfo.tint.g != 1.0 ||
            sPtr->textureInfo.tint.b != 1.0 ||
            sPtr->textureInfo.tint.a != 1.0
        ) {
            sGW.write("texture_tint", sPtr->textureInfo.tint);
        }
        
    }
    
    //Mobs.
    DataNode* mobsNode = node->addNew("mobs");
    forIdx(m, mobGenerators) {
    
        //Mob.
        MobGen* mPtr = mobGenerators[m];
        string catName = "unknown";
        if(mPtr->type && mPtr->type->category) {
            catName = mPtr->type->category->internalName;
        }
        DataNode* mobNode = mobsNode->addNew(catName);
        GetterWriter mGW(mobNode);
        
        if(mPtr->type) {
            mGW.write("type", mPtr->type->manifest->internalName);
        }
        mGW.write("p", mPtr->pos);
        if(mPtr->angle != 0) {
            mGW.write("angle", mPtr->angle);
        }
        if(!mPtr->vars.empty()) {
            mGW.write("vars", mPtr->vars);
        }
        if(mPtr->isBoss) {
            mGW.write("boss", mPtr->isBoss);
        }
        
        string linksStr;
        forIdx(l, mPtr->linkIdxs) {
            if(l > 0) linksStr += " ";
            linksStr += i2s(mPtr->linkIdxs[l]);
        }
        
        if(!linksStr.empty()) {
            mGW.write("links", linksStr);
        }
        
        if(mPtr->storedInside != INVALID) {
            mGW.write("stored_inside", mPtr->storedInside);
        }
    }
    
    //Path stops.
    DataNode* pathStopsNode = node->addNew("path_stops");
    forIdx(s, pathStops) {
    
        //Path stop.
        PathStop* sPtr = pathStops[s];
        DataNode* pathStopNode = pathStopsNode->addNew("s");
        GetterWriter sGW(pathStopNode);
        
        sGW.write("pos", sPtr->pos);
        if(sPtr->radius != PATHS::MIN_STOP_RADIUS) {
            sGW.write("radius", sPtr->radius);
        }
        if(sPtr->flags != 0) {
            sGW.write("flags", sPtr->flags);
        }
        if(!sPtr->label.empty()) {
            sGW.write("label", sPtr->label);
        }
        
        DataNode* linksNode = pathStopNode->addNew("links");
        forIdx(l, sPtr->links) {
            PathLink* lPtr = sPtr->links[l];
            string linkData = i2s(lPtr->endIdx);
            if(lPtr->type != PATH_LINK_TYPE_NORMAL) {
                linkData += " " + i2s(lPtr->type);
            }
            linksNode->addNew("l", linkData);
        }
        
    }
    
    //Tree shadows.
    DataNode* shadowsNode = node->addNew("tree_shadows");
    forIdx(s, treeShadows) {
    
        //Tree shadow.
        TreeShadow* sPtr = treeShadows[s];
        DataNode* shadowNode = shadowsNode->addNew("shadow");
        GetterWriter sGW(shadowNode);
        
        sGW.write("pos", sPtr->pose.pos);
        sGW.write("size", sPtr->pose.size);
        sGW.write("file", sPtr->bmpName);
        sGW.write("sway", sPtr->sway);
        if(sPtr->pose.angle != 0) {
            sGW.write("angle", sPtr->pose.angle);
        }
        if(sPtr->alpha != 255) {
            sGW.write("alpha", sPtr->alpha);
        }
        
    }
    
    //Regions.
    DataNode* regionsNode = node->addNew("regions");
    forIdx(r, regions) {
    
        //Region.
        AreaRegion* rPtr = regions[r];
        DataNode* regionNode = regionsNode->addNew("region");
        GetterWriter rGW(regionNode);
        
        rGW.write("center", rPtr->center);
        rGW.write("size", rPtr->size);
        
    }
}


/**
 * @brief Saves the area's main data to a data node.
 *
 * @param node Data node to save to.
 */
void Area::saveMainDataToDataNode(DataNode* node) {
    //Content metadata.
    saveMetadataToDataNode(node);
    
    GetterWriter aGW(node);
    
    //Main data.
    aGW.write("subtitle", subtitle);
    aGW.write("difficulty", difficulty);
    aGW.write("bg_bmp", bgBmpName);
    aGW.write("bg_color", bgColor);
    aGW.write("bg_dist", bgDist);
    aGW.write("bg_zoom", bgBmpZoom);
    aGW.write("song", songName);
    aGW.write("weather", weatherName);
    aGW.write("day_time_start", dayTimeStart);
    aGW.write("day_time_speed", dayTimeSpeed);
    aGW.write("spray_amounts", sprayAmounts);
    
    if(maxPikminInField != INVALID) {
        aGW.write("max_pikmin_in_field", maxPikminInField);
    }
    if(onionsAutoEject) {
        aGW.write("onions_auto_eject", onionsAutoEject);
    }
    if(onionsEjectGrownPikmin) {
        aGW.write("onions_eject_grown_pikmin", onionsEjectGrownPikmin);
    }
}


/**
 * @brief Saves the area's mission data to a data node.
 *
 * @param node Data node to save to.
 */
void Area::saveMissionDataToDataNode(DataNode* node) {
    //General properties.
    GetterWriter mGW(node);
    
    string briefingNotesStr =
        join(mission.briefingNotes, ";");
    mGW.write(
        "mission_preset", enumGetName(missionPresetNames, mission.preset)
    );
    mGW.write("mission_time_limit", mission.timeLimit);
    mGW.write(
        "mission_medal_award_mode",
        enumGetName(missionMedalAwardModeNames, mission.medalAwardMode)
    );
    mGW.write("mission_starting_points", mission.startingPoints);
    mGW.write("mission_bronze_req", mission.bronzeReq);
    mGW.write("mission_silver_req", mission.silverReq);
    mGW.write("mission_gold_req", mission.goldReq);
    mGW.write("mission_platinum_req", mission.platinumReq);
    mGW.write("mission_briefing_objective", mission.briefingObjective);
    mGW.write("mission_briefing_notes", briefingNotesStr);
    mGW.write("mission_maker_record", mission.makerRecord);
    mGW.write("mission_maker_record_date", mission.makerRecordDate);
    
    //End conditions.
    DataNode* condsNode = node->addNew("mission_end_conditions");
    forIdx(c, mission.endConds) {
        DataNode* condNode = condsNode->addNew("end_condition");
        MissionEndCond* condPtr = &mission.endConds[c];
        
        GetterWriter cGW(condNode);
        int matchAmountInt =
            condPtr->matchAmount == INVALID ? -1 : condPtr->matchAmount;
            
        cGW.write(
            "type", enumGetName(missionEndCondNames, condPtr->type)
        );
        cGW.write(
            "metric_type", enumGetName(missionMetricNames, condPtr->metricType)
        );
        cGW.write("index_param", condPtr->idxParam);
        cGW.write("match_amount", matchAmountInt);
        cGW.write("clear", condPtr->clear);
        cGW.write("zero_time_for_score", condPtr->zeroTimeForScore);
        cGW.write("neutral_mood", condPtr->neutralMood);
        cGW.write("reason", condPtr->reason);
    }
    
    //Mob groups.
    DataNode* groupsNode = node->addNew("mission_mob_groups");
    forIdx(g, mission.mobGroups) {
        DataNode* groupNode = groupsNode->addNew("group");
        MissionMobGroup* groupPtr = &mission.mobGroups[g];
        
        GetterWriter cGW(groupNode);
        
        cGW.write(
            "type", enumGetName(missionMobGroupTypeNames, groupPtr->type)
        );
        cGW.write("enemies_need_collection", groupPtr->enemiesNeedCollection);
        cGW.write("highlight_on_radar", groupPtr->highlightOnRadar);
        
        vector<string> mobIdxsStrVec;
        for(auto m : groupPtr->mobIdxs) {
            mobIdxsStrVec.push_back(i2s(m));
        }
        string mobIdxsStr = join(mobIdxsStrVec, ";");
        if(!mobIdxsStr.empty()) {
            cGW.write("mob_idxs", mobIdxsStr);
        }
    }
    
    //HUD items.
    DataNode* itemsNode = node->addNew("mission_hud_items");
    forIdx(i, mission.hudItems) {
        DataNode* itemNode = itemsNode->addNew("item");
        MissionHudItem* itemPtr = &mission.hudItems[i];
        
        GetterWriter iGW(itemNode);
        int totalAmountInt =
            itemPtr->totalAmount == INVALID ? -1 : itemPtr->totalAmount;
            
        iGW.write("enabled", itemPtr->enabled);
        iGW.write(
            "display_type",
            enumGetName(missionHudItemDisplayTypeNames, itemPtr->displayType)
        );
        iGW.write(
            "metric_type", enumGetName(missionMetricNames, itemPtr->metricType)
        );
        iGW.write("index_param", itemPtr->idxParam);
        iGW.write("text", itemPtr->text);
        iGW.write("total_amount", totalAmountInt);
    }
    
    //Score criteria.
    DataNode* scoreCriteriaNode = node->addNew("mission_score_criteria");
    forIdx(c, mission.scoreCriteria) {
        DataNode* criterionNode = scoreCriteriaNode->addNew("criterion");
        MissionScoreCriterion* criterionPtr = &mission.scoreCriteria[c];
        
        GetterWriter cGW(criterionNode);
        
        cGW.write(
            "type", enumGetName(missionMetricNames, criterionPtr->metricType)
        );
        cGW.write("index_param", criterionPtr->idxParam);
        cGW.write("points", criterionPtr->points);
        cGW.write("affects_hud", criterionPtr->affectsHud);
    }
}


/**
 * @brief Saves the area maker's reminders to a data node.
 *
 * @param node Data node to save to.
 */
void Area::saveRemindersToDataNode(DataNode* node) {
    forIdx(r, reminders) {
        DataNode* reminderNode = node->addNew("reminder");
        AreaMakerReminder* reminderPtr = &reminders[r];
        
        GetterWriter rGW(reminderNode);
        
        rGW.write("pos", reminderPtr->pos);
        rGW.write("text", reminderPtr->text);
    }
}


/**
 * @brief Saves the area's thumbnail to the disk, or deletes it from the disk
 * if it's meant to not exist.
 *
 * @param toBackup Whether it's to save to the area backup.
 */
void Area::saveThumbnail(bool toBackup) {
    string thumbPath =
        (toBackup ? userDataPath : manifest->path) +
        "/" + FILE_NAMES::AREA_THUMBNAIL;
    if(thumbnail) {
        al_save_bitmap(thumbPath.c_str(), thumbnail.get());
    } else {
        al_remove_filename(thumbPath.c_str());
    }
}


#pragma endregion
#pragma region Blockmap


/**
 * @brief Clears the info of the blockmap.
 */
void Blockmap::clear() {
    topLeftCorner = Point();
    edges.clear();
    sectors.clear();
    nCols = 0;
    nRows = 0;
}


/**
 * @brief Returns the block column in which an X coordinate is contained.
 *
 * @param x X coordinate.
 * @return The column, or INVALID on error.
 */
size_t Blockmap::getCol(float x) const {
    if(x < topLeftCorner.x) return INVALID;
    float finalX = (x - topLeftCorner.x) / GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    if(finalX >= nCols) return INVALID;
    return finalX;
}


/**
 * @brief Obtains a list of edges that are within the specified
 * rectangular region.
 *
 * @param tl Top-left coordinates of the region.
 * @param br Bottom-right coordinates of the region.
 * @param outEdges Set to fill the edges into.
 * @return Whether it succeeded.
 */
bool Blockmap::getEdgesInRegion(
    const Point& tl, const Point& br, set<Edge*>& outEdges
) const {

    size_t bx1 = getCol(tl.x);
    size_t bx2 = getCol(br.x);
    size_t by1 = getRow(tl.y);
    size_t by2 = getRow(br.y);
    
    if(
        bx1 == INVALID || bx2 == INVALID ||
        by1 == INVALID || by2 == INVALID
    ) {
        //Out of bounds.
        return false;
    }
    
    for(size_t bx = bx1; bx <= bx2; bx++) {
        for(size_t by = by1; by <= by2; by++) {
        
            const vector<Edge*>& blockEdges = edges[bx][by];
            
            forIdx(e, blockEdges) {
                outEdges.insert(blockEdges[e]);
            }
        }
    }
    
    return true;
}


/**
 * @brief Returns the block row in which a Y coordinate is contained.
 *
 * @param y Y coordinate.
 * @return The row, or INVALID on error.
 */
size_t Blockmap::getRow(float y) const {
    if(y < topLeftCorner.y) return INVALID;
    float finalY = (y - topLeftCorner.y) / GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    if(finalY >= nRows) return INVALID;
    return finalY;
}


/**
 * @brief Returns the top-left coordinates for the specified column and row.
 *
 * @param col Column to check.
 * @param row Row to check.
 * @return The top-left coordinates.
 */
Point Blockmap::getTopLeftCorner(size_t col, size_t row) const {
    return
        Point(
            col * GEOMETRY::BLOCKMAP_BLOCK_SIZE + topLeftCorner.x,
            row * GEOMETRY::BLOCKMAP_BLOCK_SIZE + topLeftCorner.y
        );
}


#pragma endregion
#pragma region Others


/**
 * @brief Constructs a new mob generator object.
 *
 * @param pos Coordinates.
 * @param type The mob type.
 * @param angle Angle it is facing.
 * @param vars String representation of the script vars.
 * @param boss Whether it is a boss encounter.
 */
MobGen::MobGen(
    const Point& pos, MobType* type, float angle, const string& vars,
    bool boss
) :
    type(type),
    pos(pos),
    angle(angle),
    isBoss(boss),
    vars(vars) {
    
}


/**
 * @brief Clones the properties of this mob generator onto another
 * mob generator.
 *
 * @param destination Mob generator to clone the data into.
 * @param includePosition If true, the position is included too.
 */
void MobGen::clone(MobGen* destination, bool includePosition) const {
    destination->angle = angle;
    if(includePosition) destination->pos = pos;
    destination->type = type;
    destination->isBoss = isBoss;
    destination->vars = vars;
    destination->linkIdxs = linkIdxs;
    destination->storedInside = storedInside;
}


/**
 * @brief Constructs a new tree shadow object.
 *
 * @param center Center coordinates.
 * @param size Width and height.
 * @param angle Angle it is rotated by.
 * @param alpha How opaque it is [0 - 255].
 * @param bmpName Internal name of the tree shadow texture's bitmap.
 * @param sway Multiply the sway distance by this much, horizontally and
 * vertically.
 */
TreeShadow::TreeShadow(
    const Point& center, const Point& size, float angle,
    unsigned char alpha, const string& bmpName, const Point& sway
) :
    bmpName(bmpName),
    bitmap(nullptr),
    alpha(alpha),
    sway(sway) {
    
    pose.pos = center;
    pose.size = size;
    pose.angle = angle;
    
}


/**
 * @brief Destroys the tree shadow object.
 *
 */
TreeShadow::~TreeShadow() {
    game.content.bitmaps.list.free(bmpName);
}


#pragma endregion
