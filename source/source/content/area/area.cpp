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


/**
 * @brief Checks to see if all indexes match their pointers,
 * for the various edges, vertexes, etc.
 *
 * This is merely a debugging tool. Aborts execution if any of the pointers
 * don't match.
 */
void Area::checkStability() {
    for(size_t v = 0; v < vertexes.size(); v++) {
        Vertex* vPtr = vertexes[v];
        engineAssert(
            vPtr->edges.size() == vPtr->edgeIdxs.size(),
            i2s(vPtr->edges.size()) + " " + i2s(vPtr->edgeIdxs.size())
        );
        for(size_t e = 0; e < vPtr->edges.size(); e++) {
            engineAssert(vPtr->edges[e] == edges[vPtr->edgeIdxs[e]], "");
        }
    }
    
    for(size_t e = 0; e < edges.size(); e++) {
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
    
    for(size_t s = 0; s < sectors.size(); s++) {
        Sector* sPtr = sectors[s];
        engineAssert(
            sPtr->edges.size() == sPtr->edgeIdxs.size(),
            i2s(sPtr->edges.size()) + " " + i2s(sPtr->edgeIdxs.size())
        );
        for(size_t e = 0; e < sPtr->edges.size(); e++) {
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
            removeSector(s);
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
    for(size_t v = 0; v < vertexes.size(); v++) {
        delete vertexes[v];
    }
    for(size_t e = 0; e < edges.size(); e++) {
        delete edges[e];
    }
    for(size_t s = 0; s < sectors.size(); s++) {
        delete sectors[s];
    }
    for(size_t m = 0; m < mobGenerators.size(); m++) {
        delete mobGenerators[m];
    }
    for(size_t s = 0; s < pathStops.size(); s++) {
        delete pathStops[s];
    }
    for(size_t s = 0; s < treeShadows.size(); s++) {
        delete treeShadows[s];
    }
    
    vertexes.clear();
    edges.clear();
    sectors.clear();
    mobGenerators.clear();
    pathStops.clear();
    treeShadows.clear();
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
void Area::clone(Area &other) {
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
    for(size_t v = 0; v < vertexes.size(); v++) {
        other.vertexes.push_back(new Vertex());
    }
    other.edges.reserve(edges.size());
    for(size_t e = 0; e < edges.size(); e++) {
        other.edges.push_back(new Edge());
    }
    other.sectors.reserve(sectors.size());
    for(size_t s = 0; s < sectors.size(); s++) {
        other.sectors.push_back(new Sector());
    }
    other.mobGenerators.reserve(mobGenerators.size());
    for(size_t m = 0; m < mobGenerators.size(); m++) {
        other.mobGenerators.push_back(new MobGen());
    }
    other.pathStops.reserve(pathStops.size());
    for(size_t s = 0; s < pathStops.size(); s++) {
        other.pathStops.push_back(new PathStop());
    }
    other.treeShadows.reserve(treeShadows.size());
    for(size_t t = 0; t < treeShadows.size(); t++) {
        other.treeShadows.push_back(new TreeShadow());
    }
    
    for(size_t v = 0; v < vertexes.size(); v++) {
        Vertex* vPtr = vertexes[v];
        Vertex* ovPtr = other.vertexes[v];
        ovPtr->x = vPtr->x;
        ovPtr->y = vPtr->y;
        ovPtr->edges.reserve(vPtr->edges.size());
        ovPtr->edgeIdxs.reserve(vPtr->edgeIdxs.size());
        for(size_t e = 0; e < vPtr->edges.size(); e++) {
            size_t nr = vPtr->edgeIdxs[e];
            ovPtr->edges.push_back(other.edges[nr]);
            ovPtr->edgeIdxs.push_back(nr);
        }
    }
    
    for(size_t e = 0; e < edges.size(); e++) {
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
    
    for(size_t s = 0; s < sectors.size(); s++) {
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
        for(size_t e = 0; e < sPtr->edges.size(); e++) {
            size_t nr = sPtr->edgeIdxs[e];
            osPtr->edges.push_back(other.edges[nr]);
            osPtr->edgeIdxs.push_back(nr);
        }
        osPtr->triangles.reserve(sPtr->triangles.size());
        for(size_t t = 0; t < sPtr->triangles.size(); t++) {
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
    
    for(size_t m = 0; m < mobGenerators.size(); m++) {
        MobGen* mPtr = mobGenerators[m];
        MobGen* omPtr = other.mobGenerators[m];
        mPtr->clone(omPtr);
    }
    for(size_t m = 0; m < mobGenerators.size(); m++) {
        MobGen* omPtr = other.mobGenerators[m];
        for(size_t l = 0; l < omPtr->linkIdxs.size(); l++) {
            omPtr->links.push_back(
                other.mobGenerators[omPtr->linkIdxs[l]]
            );
        }
    }
    
    for(size_t s = 0; s < pathStops.size(); s++) {
        PathStop* sPtr = pathStops[s];
        PathStop* osPtr = other.pathStops[s];
        osPtr->pos = sPtr->pos;
        sPtr->clone(osPtr);
        osPtr->links.reserve(sPtr->links.size());
        for(size_t l = 0; l < sPtr->links.size(); l++) {
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
    
    for(size_t t = 0; t < treeShadows.size(); t++) {
        TreeShadow* tPtr = treeShadows[t];
        TreeShadow* otPtr = other.treeShadows[t];
        otPtr->alpha = tPtr->alpha;
        otPtr->angle = tPtr->angle;
        otPtr->center = tPtr->center;
        otPtr->bmpName = tPtr->bmpName;
        otPtr->size = tPtr->size;
        otPtr->sway = tPtr->sway;
        otPtr->bitmap =
            game.content.bitmaps.list.get(tPtr->bmpName, nullptr, false);
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
    
    other.mission.goal = mission.goal;
    other.mission.goalAllMobs = mission.goalAllMobs;
    other.mission.goalMobIdxs = mission.goalMobIdxs;
    other.mission.goalAmount = mission.goalAmount;
    other.mission.goalExitCenter = mission.goalExitCenter;
    other.mission.goalExitSize = mission.goalExitSize;
    other.mission.failConditions = mission.failConditions;
    other.mission.failTooFewPikAmount = mission.failTooFewPikAmount;
    other.mission.failTooManyPikAmount = mission.failTooManyPikAmount;
    other.mission.failPikKilled = mission.failPikKilled;
    other.mission.failLeadersKod = mission.failLeadersKod;
    other.mission.failEnemiesDefeated = mission.failEnemiesDefeated;
    other.mission.failTimeLimit = mission.failTimeLimit;
    other.mission.gradingMode = mission.gradingMode;
    other.mission.pointsPerPikminBorn = mission.pointsPerPikminBorn;
    other.mission.pointsPerPikminDeath = mission.pointsPerPikminDeath;
    other.mission.pointsPerSecLeft = mission.pointsPerSecLeft;
    other.mission.pointsPerSecPassed = mission.pointsPerSecPassed;
    other.mission.pointsPerTreasurePoint = mission.pointsPerTreasurePoint;
    other.mission.pointsPerEnemyPoint = mission.pointsPerEnemyPoint;
    other.mission.enemyPointsOnCollection = mission.enemyPointsOnCollection;
    other.mission.pointLossData = mission.pointLossData;
    other.mission.pointHudData = mission.pointHudData;
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
    for(const auto &s : problems.nonSimples) {
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
    for(size_t e = 0; e < edges.size(); e++) {
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
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* ePtr = edges[e];
        if(ePtr->vertexes[0] == vPtr || ePtr->vertexes[1] == vPtr) {
            vPtr->edgeIdxs.push_back(e);
        }
    }
    fixVertexPointers(vPtr);
}


/**
 * @brief Scans the list of edges and retrieves the index of
 * the specified edge.
 *
 * @param ePtr Edge to find.
 * @return The index, or INVALID if not found.
 */
size_t Area::findEdgeIdx(const Edge* ePtr) const {
    for(size_t e = 0; e < edges.size(); e++) {
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
    for(size_t m = 0; m < mobGenerators.size(); m++) {
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
    for(size_t s = 0; s < sectors.size(); s++) {
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
    for(size_t v = 0; v < vertexes.size(); v++) {
        if(vertexes[v] == vPtr) return v;
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
    for(size_t l = 0; l < sPtr->links.size(); l++) {
        PathLink* lPtr = sPtr->links[l];
        lPtr->endIdx = INVALID;
        
        if(!lPtr->endPtr) continue;
        
        for(size_t s = 0; s < pathStops.size(); s++) {
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
    for(size_t l = 0; l < sPtr->links.size(); l++) {
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
    for(size_t e = 0; e < sPtr->edges.size(); e++) {
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
    for(size_t e = 0; e < sPtr->edgeIdxs.size(); e++) {
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
    for(size_t e = 0; e < vPtr->edges.size(); e++) {
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
    for(size_t e = 0; e < vPtr->edgeIdxs.size(); e++) {
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
    
    for(size_t v = 0; v < vertexes.size(); v++) {
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
void Area::generateEdgesBlockmap(const vector<Edge*> &edgeList) {
    for(size_t e = 0; e < edgeList.size(); e++) {
    
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
 * @brief Returns how many path links exist in the area.
 *
 * @return The number of path links.
 */
size_t Area::getNrPathLinks() {
    size_t oneWaysFound = 0;
    size_t normalsFound = 0;
    for(size_t s = 0; s < pathStops.size(); s++) {
        PathStop* sPtr = pathStops[s];
        for(size_t l = 0; l < sPtr->links.size(); l++) {
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
        sRS.set("tag", newSector->tag);
        sRS.set("texture_rotate", newSector->textureInfo.rot);
        sRS.set("texture_scale", newSector->textureInfo.scale);
        sRS.set("texture_tint", newSector->textureInfo.tint);
        sRS.set("texture_trans", newSector->textureInfo.translation);
        sRS.set("texture", newSector->textureInfo.bmpName);
        sRS.set("type", typeStr);
        sRS.set("z", newSector->z);
        
        size_t newType = game.sectorTypes.getIdx(typeStr);
        if(newType == INVALID) {
            newType = SECTOR_TYPE_NORMAL;
        }
        newSector->type = (SECTOR_TYPE) newType;
        
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
        for(size_t l = 0; l < linkStrs.size(); l++) {
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
    
    for(size_t l = 0; l < mobLinksBuffer.size(); l++) {
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
        
        sRS.set("pos", newShadow->center);
        sRS.set("size", newShadow->size);
        sRS.set("angle", newShadow->angle);
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
    
    //Set up stuff.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Area -- Geometry calculations");
    }
    
    for(size_t e = 0; e < edges.size(); e++) {
        fixEdgePointers(edges[e]);
    }
    for(size_t s = 0; s < sectors.size(); s++) {
        connectSectorEdges(sectors[s]);
    }
    for(size_t v = 0; v < vertexes.size(); v++) {
        connectVertexEdges(vertexes[v]);
    }
    for(size_t s = 0; s < pathStops.size(); s++) {
        fixPathStopPointers(pathStops[s]);
    }
    for(size_t s = 0; s < pathStops.size(); s++) {
        pathStops[s]->calculateDists();
    }
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        //Fade sectors that also fade brightness should be
        //at midway between the two neighbors.
        for(size_t s = 0; s < sectors.size(); s++) {
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
    for(size_t s = 0; s < sectors.size(); s++) {
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
    mission.failHudPrimaryCond = INVALID;
    mission.failHudSecondaryCond = INVALID;
    
    ReaderSetter mRS(node);
    
    string goalStr;
    string requiredMobsStr;
    int missionGradingModeInt = MISSION_GRADING_MODE_GOAL;
    
    mRS.set("mission_goal", goalStr);
    mRS.set("mission_goal_amount", mission.goalAmount);
    mRS.set("mission_goal_all_mobs", mission.goalAllMobs);
    mRS.set("mission_required_mobs", requiredMobsStr);
    mRS.set("mission_goal_exit_center", mission.goalExitCenter);
    mRS.set("mission_goal_exit_size", mission.goalExitSize);
    mRS.set("mission_fail_conditions", mission.failConditions);
    mRS.set("mission_fail_too_few_pik_amount", mission.failTooFewPikAmount);
    mRS.set("mission_fail_too_many_pik_amount", mission.failTooManyPikAmount);
    mRS.set("mission_fail_pik_killed", mission.failPikKilled);
    mRS.set("mission_fail_leaders_kod", mission.failLeadersKod);
    mRS.set("mission_fail_enemies_defeated", mission.failEnemiesDefeated);
    mRS.set("mission_fail_time_limit", mission.failTimeLimit);
    mRS.set("mission_fail_hud_primary_cond", mission.failHudPrimaryCond);
    mRS.set("mission_fail_hud_secondary_cond", mission.failHudSecondaryCond);
    mRS.set("mission_grading_mode", missionGradingModeInt);
    mRS.set("mission_points_per_pikmin_born", mission.pointsPerPikminBorn);
    mRS.set("mission_points_per_pikmin_death", mission.pointsPerPikminDeath);
    mRS.set("mission_points_per_sec_left", mission.pointsPerSecLeft);
    mRS.set("mission_points_per_sec_passed", mission.pointsPerSecPassed);
    mRS.set(
        "mission_points_per_treasure_point", mission.pointsPerTreasurePoint
    );
    mRS.set("mission_points_per_enemy_point", mission.pointsPerEnemyPoint);
    mRS.set("enemy_points_on_collection", mission.enemyPointsOnCollection);
    mRS.set("mission_point_loss_data", mission.pointLossData);
    mRS.set("mission_point_hud_data", mission.pointHudData);
    mRS.set("mission_starting_points", mission.startingPoints);
    mRS.set("mission_bronze_req", mission.bronzeReq);
    mRS.set("mission_silver_req", mission.silverReq);
    mRS.set("mission_gold_req", mission.goldReq);
    mRS.set("mission_platinum_req", mission.platinumReq);
    mRS.set("mission_maker_record", mission.makerRecord);
    mRS.set("mission_maker_record_date", mission.makerRecordDate);
    
    mission.goal = MISSION_GOAL_END_MANUALLY;
    for(size_t g = 0; g < game.missionGoals.size(); g++) {
        if(game.missionGoals[g]->getName() == goalStr) {
            mission.goal = (MISSION_GOAL) g;
            break;
        }
    }
    vector<string> missionRequiredMobsStr =
        semicolonListToVector(requiredMobsStr);
    mission.goalMobIdxs.reserve(
        missionRequiredMobsStr.size()
    );
    for(size_t m = 0; m < missionRequiredMobsStr.size(); m++) {
        mission.goalMobIdxs.insert(
            s2i(missionRequiredMobsStr[m])
        );
    }
    mission.gradingMode = (MISSION_GRADING_MODE) missionGradingModeInt;
    
    //Automatically turn the pause menu fail condition on/off for convenience.
    if(mission.goal == MISSION_GOAL_END_MANUALLY) {
        disableFlag(
            mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
    } else {
        enableFlag(
            mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
    }
    
    //Automatically turn off the seconds left score criterion for convenience.
    if(
        !hasFlag(
            mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        mission.pointsPerSecLeft = 0;
        disableFlag(
            mission.pointHudData,
            getIdxBitmask(MISSION_SCORE_CRITERIA_SEC_LEFT)
        );
        disableFlag(
            mission.pointLossData,
            getIdxBitmask(MISSION_SCORE_CRITERIA_SEC_LEFT)
        );
    }
}


/**
 * @brief Loads the thumbnail image from the disk and updates the
 * thumbnail class member.
 *
 * @param thumbnailPath Path to the bitmap.
 */
void Area::loadThumbnail(const string &thumbnailPath) {
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
 * @brief Adds a new edge to the list.
 *
 * @return The new edge's pointer.
 */
Edge* Area::newEdge() {
    Edge* ePtr = new Edge();
    edges.push_back(ePtr);
    return ePtr;
}


/**
 * @brief Adds a new sector to the list.
 *
 * @return The new sector's pointer.
 */
Sector* Area::newSector() {
    Sector* sPtr = new Sector();
    sectors.push_back(sPtr);
    return sPtr;
}


/**
 * @brief Adds a new vertex to the list.
 *
 * @return The new vertex's pointer.
 */
Vertex* Area::newVertex() {
    Vertex* vPtr = new Vertex();
    vertexes.push_back(vPtr);
    return vPtr;
}


/**
 * @brief Removes an edge from the list, and updates all indexes after it.
 *
 * @param eIdx Index number of the edge to remove.
 */
void Area::removeEdge(size_t eIdx) {
    edges.erase(edges.begin() + eIdx);
    for(size_t v = 0; v < vertexes.size(); v++) {
        Vertex* vPtr = vertexes[v];
        for(size_t e = 0; e < vPtr->edges.size(); e++) {
            if(
                vPtr->edgeIdxs[e] != INVALID &&
                vPtr->edgeIdxs[e] > eIdx
            ) {
                vPtr->edgeIdxs[e]--;
            } else {
                //This should never happen.
                engineAssert(
                    vPtr->edgeIdxs[e] != eIdx,
                    i2s(vPtr->edgeIdxs[e]) + " " + i2s(eIdx)
                );
            }
        }
    }
    for(size_t s = 0; s < sectors.size(); s++) {
        Sector* sPtr = sectors[s];
        for(size_t e = 0; e < sPtr->edges.size(); e++) {
            if(
                sPtr->edgeIdxs[e] != INVALID &&
                sPtr->edgeIdxs[e] > eIdx
            ) {
                sPtr->edgeIdxs[e]--;
            } else {
                //This should never happen.
                engineAssert(
                    sPtr->edgeIdxs[e] != eIdx,
                    i2s(sPtr->edgeIdxs[e]) + " " + i2s(eIdx)
                );
            }
        }
    }
}


/**
 * @brief Removes an edge from the list, and updates all indexes after it.
 *
 * @param ePtr Pointer of the edge to remove.
 */
void Area::removeEdge(const Edge* ePtr) {
    for(size_t e = 0; e < edges.size(); e++) {
        if(edges[e] == ePtr) {
            removeEdge(e);
            return;
        }
    }
}


/**
 * @brief Removes a sector from the list, and updates all indexes after it.
 *
 * @param sIdx Index number of the sector to remove.
 */
void Area::removeSector(size_t sIdx) {
    sectors.erase(sectors.begin() + sIdx);
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* ePtr = edges[e];
        for(size_t s = 0; s < 2; s++) {
            if(
                ePtr->sectorIdxs[s] != INVALID &&
                ePtr->sectorIdxs[s] > sIdx
            ) {
                ePtr->sectorIdxs[s]--;
            } else {
                //This should never happen.
                engineAssert(
                    ePtr->sectorIdxs[s] != sIdx,
                    i2s(ePtr->sectorIdxs[s]) + " " + i2s(sIdx)
                );
            }
        }
    }
}


/**
 * @brief Removes a sector from the list, and updates all indexes after it.
 *
 * @param sPtr Pointer of the sector to remove.
 */
void Area::removeSector(const Sector* sPtr) {
    for(size_t s = 0; s < sectors.size(); s++) {
        if(sectors[s] == sPtr) {
            removeSector(s);
            return;
        }
    }
}


/**
 * @brief Removes a vertex from the list, and updates all indexes after it.
 *
 * @param vIdx Index number of the vertex to remove.
 */
void Area::removeVertex(size_t vIdx) {
    vertexes.erase(vertexes.begin() + vIdx);
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* ePtr = edges[e];
        for(size_t v = 0; v < 2; v++) {
            if(
                ePtr->vertexIdxs[v] != INVALID &&
                ePtr->vertexIdxs[v] > vIdx
            ) {
                ePtr->vertexIdxs[v]--;
            } else {
                //This should never happen.
                engineAssert(
                    ePtr->vertexIdxs[v] != vIdx,
                    i2s(ePtr->vertexIdxs[v]) + " " + i2s(vIdx)
                );
            }
        }
    }
}


/**
 * @brief Removes a vertex from the list, and updates all indexes after it.
 *
 * @param vPtr Pointer of the vertex to remove.
 */
void Area::removeVertex(const Vertex* vPtr) {
    for(size_t v = 0; v < vertexes.size(); v++) {
        if(vertexes[v] == vPtr) {
            removeVertex(v);
            return;
        }
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
    for(size_t v = 0; v < vertexes.size(); v++) {
    
        //Vertex.
        Vertex* vPtr = vertexes[v];
        vertexesNode->addNew("v", p2s(v2p(vPtr)));
    }
    
    //Edges.
    DataNode* edgesNode = node->addNew("edges");
    for(size_t e = 0; e < edges.size(); e++) {
    
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
    for(size_t s = 0; s < sectors.size(); s++) {
    
        //Sector.
        Sector* sPtr = sectors[s];
        DataNode* sectorNode = sectorsNode->addNew("s");
        GetterWriter sGW(sectorNode);
        
        if(sPtr->type != SECTOR_TYPE_NORMAL) {
            sGW.write("type", game.sectorTypes.getName(sPtr->type));
        }
        if(sPtr->isBottomlessPit) {
            sGW.write("is_bottomless_pit", true);
        }
        sGW.write("z", sPtr->z);
        if(sPtr->brightness != GEOMETRY::DEF_SECTOR_BRIGHTNESS) {
            sGW.write("brightness", sPtr->brightness);
        }
        if(!sPtr->tag.empty()) {
            sGW.write("tag", sPtr->tag);
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
        
        if(sPtr->textureInfo.rot != 0) {
            sGW.write("texture_rotate", sPtr->textureInfo.rot);
        }
        if(
            sPtr->textureInfo.scale.x != 1 ||
            sPtr->textureInfo.scale.y != 1
        ) {
            sGW.write("texture_scale", sPtr->textureInfo.scale);
        }
        if(
            sPtr->textureInfo.translation.x != 0 ||
            sPtr->textureInfo.translation.y != 0
        ) {
            sGW.write("texture_trans", sPtr->textureInfo.translation);
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
    for(size_t m = 0; m < mobGenerators.size(); m++) {
    
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
        
        string linksStr;
        for(size_t l = 0; l < mPtr->linkIdxs.size(); l++) {
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
    for(size_t s = 0; s < pathStops.size(); s++) {
    
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
        for(size_t l = 0; l < sPtr->links.size(); l++) {
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
    for(size_t s = 0; s < treeShadows.size(); s++) {
    
        //Tree shadow.
        TreeShadow* sPtr = treeShadows[s];
        DataNode* shadowNode = shadowsNode->addNew("shadow");
        GetterWriter sGW(shadowNode);
        
        sGW.write("pos", sPtr->center);
        sGW.write("size", sPtr->size);
        sGW.write("file", sPtr->bmpName);
        sGW.write("sway", sPtr->sway);
        if(sPtr->angle != 0) {
            sGW.write("angle", sPtr->angle);
        }
        if(sPtr->alpha != 255) {
            sGW.write("alpha", sPtr->alpha);
        }
        
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
}


/**
 * @brief Saves the area's mission data to a data node.
 *
 * @param node Data node to save to.
 */
void Area::saveMissionDataToDataNode(DataNode* node) {
    GetterWriter mGW(node);
    
    if(mission.goal != MISSION_GOAL_END_MANUALLY) {
        string goalName = game.missionGoals[mission.goal]->getName();
        mGW.write("mission_goal", goalName);
    }
    if(
        mission.goal == MISSION_GOAL_TIMED_SURVIVAL ||
        mission.goal == MISSION_GOAL_GROW_PIKMIN
    ) {
        mGW.write("mission_goal_amount", mission.goalAmount);
    }
    if(
        mission.goal == MISSION_GOAL_COLLECT_TREASURE ||
        mission.goal == MISSION_GOAL_BATTLE_ENEMIES ||
        mission.goal == MISSION_GOAL_GET_TO_EXIT
    ) {
        mGW.write("mission_goal_all_mobs", mission.goalAllMobs);
        vector<string> missionMobIdxStrs;
        for(auto m : mission.goalMobIdxs) {
            missionMobIdxStrs.push_back(i2s(m));
        }
        string missionMobIdxStr = join(missionMobIdxStrs, ";");
        if(!missionMobIdxStr.empty()) {
            mGW.write("mission_required_mobs", missionMobIdxStr);
        }
    }
    if(mission.goal == MISSION_GOAL_GET_TO_EXIT) {
        mGW.write("mission_goal_exit_center", mission.goalExitCenter);
        mGW.write("mission_goal_exit_size", mission.goalExitSize);
    }
    if(mission.failConditions > 0) {
        mGW.write("mission_fail_conditions", mission.failConditions);
    }
    if(
        hasFlag(
            mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_TOO_FEW_PIKMIN)
        )
    ) {
        mGW.write(
            "mission_fail_too_few_pik_amount", mission.failTooFewPikAmount
        );
    }
    if(
        hasFlag(
            mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_TOO_MANY_PIKMIN)
        )
    ) {
        mGW.write(
            "mission_fail_too_many_pik_amount", mission.failTooManyPikAmount
        );
    }
    if(
        hasFlag(
            mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_LOSE_PIKMIN)
        )
    ) {
        mGW.write("mission_fail_pik_killed", mission.failPikKilled);
    }
    if(
        hasFlag(
            mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_LOSE_LEADERS)
        )
    ) {
        mGW.write("mission_fail_leaders_kod", mission.failLeadersKod);
    }
    if(
        hasFlag(
            mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_DEFEAT_ENEMIES)
        )
    ) {
        mGW.write("mission_fail_enemies_defeated", mission.failEnemiesDefeated);
    }
    if(
        hasFlag(
            mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        mGW.write("mission_fail_time_limit", mission.failTimeLimit);
    }
    if(mission.failHudPrimaryCond != INVALID) {
        mGW.write(
            "mission_fail_hud_primary_cond", mission.failHudPrimaryCond
        );
    }
    if(mission.failHudSecondaryCond != INVALID) {
        mGW.write(
            "mission_fail_hud_secondary_cond", mission.failHudSecondaryCond
        );
    }
    mGW.write("mission_grading_mode", mission.gradingMode);
    if(mission.gradingMode == MISSION_GRADING_MODE_POINTS) {
        if(mission.pointsPerPikminBorn != 0) {
            mGW.write(
                "mission_points_per_pikmin_born", mission.pointsPerPikminBorn
            );
        }
        if(mission.pointsPerPikminDeath != 0) {
            mGW.write(
                "mission_points_per_pikmin_death", mission.pointsPerPikminDeath
            );
        }
        if(mission.pointsPerSecLeft != 0) {
            mGW.write(
                "mission_points_per_sec_left", mission.pointsPerSecLeft
            );
        }
        if(mission.pointsPerSecPassed != 0) {
            mGW.write(
                "mission_points_per_sec_passed", mission.pointsPerSecPassed
            );
        }
        if(mission.pointsPerTreasurePoint != 0) {
            mGW.write(
                "mission_points_per_treasure_point",
                mission.pointsPerTreasurePoint
            );
        }
        if(mission.pointsPerEnemyPoint != 0) {
            mGW.write(
                "mission_points_per_enemy_point", mission.pointsPerEnemyPoint
            );
        }
        if(mission.enemyPointsOnCollection) {
            mGW.write(
                "enemy_points_on_collection", mission.enemyPointsOnCollection
            );
        }
        if(mission.pointLossData > 0) {
            mGW.write("mission_point_loss_data", mission.pointLossData);
        }
        if(mission.pointHudData != 255) {
            mGW.write("mission_point_hud_data", mission.pointHudData);
        }
        if(mission.startingPoints != 0) {
            mGW.write("mission_starting_points", mission.startingPoints);
        }
        mGW.write("mission_bronze_req", mission.bronzeReq);
        mGW.write("mission_silver_req", mission.silverReq);
        mGW.write("mission_gold_req", mission.goldReq);
        mGW.write("mission_platinum_req", mission.platinumReq);
        if(!mission.makerRecordDate.empty()) {
            mGW.write("mission_maker_record", mission.makerRecord);
            mGW.write("mission_maker_record_date", mission.makerRecordDate);
        }
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
    const Point &tl, const Point &br, set<Edge*> &outEdges
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
        
            const vector<Edge*> &blockEdges = edges[bx][by];
            
            for(size_t e = 0; e < blockEdges.size(); e++) {
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


/**
 * @brief Constructs a new mob generator object.
 *
 * @param pos Coordinates.
 * @param type The mob type.
 * @param angle Angle it is facing.
 * @param vars String representation of the script vars.
 */
MobGen::MobGen(
    const Point &pos, MobType* type, float angle, const string &vars
) :
    type(type),
    pos(pos),
    angle(angle),
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
 * @param alpha How opaque it is [0-255].
 * @param bmpName Internal name of the tree shadow texture's bitmap.
 * @param sway Multiply the sway distance by this much, horizontally and
 * vertically.
 */
TreeShadow::TreeShadow(
    const Point &center, const Point &size, float angle,
    unsigned char alpha, const string &bmpName, const Point &sway
) :
    bmpName(bmpName),
    bitmap(nullptr),
    center(center),
    size(size),
    angle(angle),
    alpha(alpha),
    sway(sway) {
    
}


/**
 * @brief Destroys the tree shadow object.
 *
 */
TreeShadow::~TreeShadow() {
    game.content.bitmaps.list.free(bmpName);
}
