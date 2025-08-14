/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Paths, path-finding, and related functions.
 */

#include <algorithm>

#include "pathing.h"

#include "../content/mob/mob_utils.h"
#include "../util/general_utils.h"
#include "game.h"
#include "misc_functions.h"


using std::map;
using std::string;
using std::unordered_set;
using std::vector;


namespace PATHS {

//Default distance at which the mob considers the chase finished.
const float DEF_CHASE_TARGET_DISTANCE = 3.0f;

//Minimum radius of a path stop.
const float MIN_STOP_RADIUS = 16.0f;

}


/**
 * @brief Constructs a new path link object.
 *
 * @param startPtr The path stop at the start of this link.
 * @param endPtr The path stop at the end of this link.
 * @param endIdx Index number of the path stop at the end of this link.
 */
PathLink::PathLink(PathStop* startPtr, PathStop* endPtr, size_t endIdx) :
    startPtr(startPtr),
    endPtr(endPtr),
    endIdx(endIdx) {
    
}


/**
 * @brief Calculates and stores the distance between the two stops.
 * Because the link doesn't know about the starting stop,
 * you need to provide it as a parameter when calling the function.
 *
 * @param startPtr The path stop at the start of this link.
 */
void PathLink::calculateDist(const PathStop* startPtr) {
    distance = Distance(startPtr->pos, endPtr->pos).toFloat();
}


/**
 * @brief Clones a path link's properties onto another,
 * not counting the path stops.
 *
 * @param destination Path link to clone the data into.
 */
void PathLink::clone(PathLink* destination) const {
    destination->type = type;
}


/**
 * @brief Checks if a path link is a plain one-way link,
 * or if it's actually one part of a normal, two-way link.
 *
 * @return Whether it's one-way.
 */
bool PathLink::isOneWay() const {
    return endPtr->getLink(startPtr) == nullptr;
}


/**
 * @brief Clears all info.
 */
void PathManager::clear() {
    if(!game.curAreaData) return;
    
    obstructions.clear();
    
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* sPtr = game.curAreaData->pathStops[s];
        for(size_t l = 0; l < sPtr->links.size(); l++) {
            game.curAreaData->pathStops[s]->links[l]->blockedByObstacle =
                false;
        }
    }
}


/**
 * @brief Handles the area having been loaded. It checks all path stops
 * and saves any sector hazards found.
 */
void PathManager::handleAreaLoad() {
    //Go through all path stops and check if they're on hazardous sectors.
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* sPtr = game.curAreaData->pathStops[s];
        if(!sPtr->sectorPtr) continue;
        if(!sPtr->sectorPtr->hazard) continue;
        hazardousStops.insert(sPtr);
    }
}


/**
 * @brief Handles an obstacle having been placed. This way, any link with that
 * obstruction can get updated.
 *
 * @param m Pointer to the obstacle mob that got added.
 */
void PathManager::handleObstacleAdd(Mob* m) {
    //Add the obstacle to our list, if needed.
    bool pathsChanged = false;
    
    //Go through all path links and check if they have obstacles.
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* sPtr = game.curAreaData->pathStops[s];
        
        for(size_t l = 0; l < sPtr->links.size(); l++) {
            PathLink* lPtr = game.curAreaData->pathStops[s]->links[l];
            
            if(
                circleIntersectsLineSeg(
                    m->pos, m->radius,
                    sPtr->pos, lPtr->endPtr->pos
                )
            ) {
                obstructions[lPtr].insert(m);
                lPtr->blockedByObstacle = true;
                pathsChanged = true;
            }
        }
    }
    
    if(pathsChanged) {
        //Re-calculate the paths of mobs taking paths.
        for(size_t m2 = 0; m2 < game.states.gameplay->mobs.all.size(); m2++) {
            Mob* m2Ptr = game.states.gameplay->mobs.all[m2];
            if(!m2Ptr->pathInfo) continue;
            
            m2Ptr->fsm.runEvent(MOB_EV_PATHS_CHANGED);
        }
    }
}


/**
 * @brief Handles an obstacle having been cleared. This way, any link with that
 * obstruction can get updated.
 *
 * @param m Pointer to the obstacle mob that got cleared.
 */
void PathManager::handleObstacleRemove(Mob* m) {
    //Remove the obstacle from our list, if it's there.
    bool pathsChanged = false;
    
    for(auto o = obstructions.begin(); o != obstructions.end();) {
        bool toDelete = false;
        if(o->second.erase(m) > 0) {
            if(o->second.empty()) {
                o->first->blockedByObstacle = false;
                toDelete = true;
                pathsChanged = true;
            }
        }
        if(toDelete) {
            o = obstructions.erase(o);
        } else {
            ++o;
        }
    }
    
    if(pathsChanged) {
        //Re-calculate the paths of mobs taking paths.
        for(size_t m2 = 0; m2 < game.states.gameplay->mobs.all.size(); m2++) {
            Mob* m2Ptr = game.states.gameplay->mobs.all[m2];
            if(!m2Ptr->pathInfo) continue;
            
            m2Ptr->fsm.runEvent(MOB_EV_PATHS_CHANGED);
        }
    }
}


/**
 * @brief Handles a sector having changed its hazards.
 * This way, any stop on that sector can be updated.
 *
 * @param sectorPtr Pointer to the sector whose hazards got updated.
 */
void PathManager::handleSectorHazardChange(Sector* sectorPtr) {
    //Remove relevant stops from our list.
    bool pathsChanged = false;
    
    for(auto s = hazardousStops.begin(); s != hazardousStops.end();) {
        bool toDelete = false;
        if((*s)->sectorPtr == sectorPtr) {
            pathsChanged = true;
            if(!sectorPtr->hazard) {
                //We only want to delete it if it became hazardless.
                toDelete = true;
            }
        }
        
        if(toDelete) {
            s = hazardousStops.erase(s);
        } else {
            ++s;
        }
    }
    
    if(pathsChanged) {
        //Re-calculate the paths of mobs taking paths.
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            Mob* mPtr = game.states.gameplay->mobs.all[m];
            if(!mPtr->pathInfo) continue;
            
            mPtr->fsm.runEvent(MOB_EV_PATHS_CHANGED);
        }
    }
}


/**
 * @brief Constructs a new path stop object.
 *
 * @param pos Its coordinates.
 * @param links List of path links, linking it to other stops.
 */
PathStop::PathStop(const Point& pos, const vector<PathLink*>& links) :
    pos(pos),
    links(links) {
    
}


/**
 * @brief Destroys the path stop object.
 */
PathStop::~PathStop() {
    while(!links.empty()) {
        delete *(links.begin());
        links.erase(links.begin());
    }
}


/**
 * @brief Adds a link between this stop and another, whether it's
 * one-way or not. Also adds the link to the other stop, if applicable.
 * If these two stops already had some link, it gets removed.
 *
 * @param otherStop Pointer to the other stop.
 * @param normal Normal link? False means one-way link.
 */
void PathStop::addLink(PathStop* otherStop, bool normal) {
    PATH_LINK_TYPE linkType = PATH_LINK_TYPE_NORMAL;
    
    PathLink* oldLinkData = getLink(otherStop);
    if(!oldLinkData) {
        oldLinkData = otherStop->getLink(this);
    }
    if(oldLinkData) {
        linkType = oldLinkData->type;
    }
    
    removeLink(oldLinkData);
    otherStop->removeLink(this);
    
    PathLink* newLink = new PathLink(this, otherStop, INVALID);
    newLink->type = linkType;
    links.push_back(newLink);
    
    if(normal) {
        newLink = new PathLink(otherStop, this, INVALID);
        newLink->type = linkType;
        otherStop->links.push_back(newLink);
    }
}


/**
 * @brief Calculates the distance between it and all neighbors.
 */
void PathStop::calculateDists() {
    for(size_t l = 0; l < links.size(); l++) {
        links[l]->calculateDist(this);
    }
}


/**
 * @brief Calculates the distance between it and all neighbors, and then
 * goes through the neighbors and updates their distance back to this stop,
 * if that neighbor links back.
 */
void PathStop::calculateDistsPlusNeighbors() {
    for(size_t l = 0; l < links.size(); l++) {
        PathLink* lPtr = links[l];
        lPtr->calculateDist(this);
    }
    
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* sPtr = game.curAreaData->pathStops[s];
        PathLink* lPtr = sPtr->getLink(this);
        if(lPtr) {
            lPtr->calculateDist(sPtr);
        }
    }
}


/**
 * @brief Clones a path stop's properties onto another, not counting the links.
 *
 * @param destination Path stop to clone the data into.
 */
void PathStop::clone(PathStop* destination) const {
    destination->radius = radius;
    destination->flags = flags;
    destination->label = label;
}


/**
 * @brief Returns the pointer of the link between this stop and another.
 * The links in memory are one-way, meaning that if the only link
 * is from the other stop to this one, it will not count.
 *
 * @param otherStop Path stop to check against.
 * @return The link, or nullptr if it does not link to that stop.
 */
PathLink* PathStop::getLink(const PathStop* otherStop) const {
    for(size_t l = 0; l < links.size(); l++) {
        if(links[l]->endPtr == otherStop) return links[l];
    }
    return nullptr;
}


/**
 * @brief Removes the specified link.
 * Does nothing if there is no such link.
 *
 * @param linkPtr Pointer to the link to remove.
 */
void PathStop::removeLink(const PathLink* linkPtr) {
    for(size_t l = 0; l < links.size(); l++) {
        if(links[l] == linkPtr) {
            delete links[l];
            links.erase(links.begin() + l);
            return;
        }
    }
}


/**
 * @brief Removes the link between this stop and the specified one.
 * Does nothing if there is no such link.
 *
 * @param otherStop Path stop to remove the link from.
 */
void PathStop::removeLink(const PathStop* otherStop) {
    for(size_t l = 0; l < links.size(); l++) {
        if(links[l]->endPtr == otherStop) {
            delete links[l];
            links.erase(links.begin() + l);
            return;
        }
    }
}


/**
 * @brief Uses A* to get the shortest path between two nodes.
 *
 * @param outPath The stops to visit, in order, are returned here.
 * @param startNode Start node.
 * @param endNode End node.
 * @param settings Settings about how the path should be followed.
 * @param outTotalDist If not nullptr, the total path distance is
 * returned here.
 * @return The operation's result.
 */
PATH_RESULT aStar(
    vector<PathStop*>& outPath,
    PathStop* startNode, PathStop* endNode,
    const PathFollowSettings& settings,
    float* outTotalDist
) {
    //https://en.wikipedia.org/wiki/A*_search_algorithm
    
    /**
     * @brief Represents a node's data in the algorithm.
     */
    struct Node {
        //In the best known path to this node, this is the known
        //distance from the start node to this one.
        float sinceStart = FLT_MAX;
        
        //In the best known path to this node, this is the node that
        //came before this one.
        PathStop* prev = nullptr;
        
        //Estimated distance if the final path takes this node.
        float estimated = FLT_MAX;
    };
    
    //All nodes that we want to visit.
    unordered_set<PathStop*> toVisit;
    //Data for all the nodes.
    map<PathStop*, Node> data;
    
    //Part 1: Initialize the algorithm.
    toVisit.insert(startNode);
    data[startNode].sinceStart = 0.0f;
    data[startNode].estimated = 0.0f;
    
    //Start iterating.
    while(!toVisit.empty()) {
    
        //Part 2: Figure out what node to work on in this iteration.
        PathStop* curNode = nullptr;
        float curNodeDist = 0.0f;
        Node curNodeData;
        
        for(auto u = toVisit.begin(); u != toVisit.end(); u++) {
            Node n = data[*u];
            float estDist = n.estimated;
            
            if(!curNode || estDist < curNodeDist) {
                curNode = *u;
                curNodeDist = estDist;
                curNodeData = n;
            }
        }
        
        //Part 3: If the node we're processing is the end node, then
        //that's it, best path found!
        if(curNode == endNode) {
        
            //Construct the path.
            float td = data[endNode].sinceStart;
            outPath.clear();
            outPath.push_back(endNode);
            PathStop* next = data[endNode].prev;
            while(next) {
                outPath.insert(outPath.begin(), next);
                next = data[next].prev;
            }
            
            if(outTotalDist) *outTotalDist = td;
            return PATH_RESULT_NORMAL_PATH;
            
        }
        
        //This node's been visited.
        toVisit.erase(curNode);
        
        //Part 4: Check the neighbors.
        for(size_t l = 0; l < curNode->links.size(); l++) {
            PathLink* lPtr = curNode->links[l];
            PathStop* neighbor = lPtr->endPtr;
            
            //Can this link be traversed?
            if(!canTraversePathLink(lPtr, settings)) {
                continue;
            }
            
            float tentativeScore =
                data[curNode].sinceStart + lPtr->distance;
                
            if(tentativeScore < data[neighbor].sinceStart) {
                //Found a better path from the start to this neighbor.
                data[neighbor].sinceStart = tentativeScore;
                data[neighbor].prev = curNode;
                data[neighbor].estimated =
                    tentativeScore +
                    Distance(neighbor->pos, endNode->pos).toFloat();
                toVisit.insert(neighbor);
            }
        }
    }
    
    //If we got to this point, there means that there is no available path!
    
    if(!hasFlag(settings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES)) {
        //Let's try again, this time ignoring obstacles.
        PathFollowSettings newSettings = settings;
        enableFlag(newSettings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES);
        PATH_RESULT newResult =
            aStar(
                outPath,
                startNode, endNode,
                newSettings,
                outTotalDist
            );
        if(newResult == PATH_RESULT_NORMAL_PATH) {
            //If we only managed to succeed with this ignore-obstacle attempt,
            //then that means a path exists, but there are obstacles.
            return PATH_RESULT_PATH_WITH_OBSTACLES;
        } else {
            return newResult;
        }
    }
    
    //Nothing that can be done. No path.
    outPath.clear();
    if(outTotalDist) *outTotalDist = 0;
    return PATH_RESULT_END_STOP_UNREACHABLE;
}


/**
 * @brief Checks if a path stop can be taken given some constraints.
 *
 * @param stopPtr Stop to check.
 * @param settings Settings about how the path should be followed.
 * @param outReason If not nullptr, the reason is returned here.
 * @return Whether it can be taken.
 */
bool canTakePathStop(
    PathStop* stopPtr, const PathFollowSettings& settings,
    PATH_BLOCK_REASON* outReason
) {
    Sector* sectorPtr = stopPtr->sectorPtr;
    if(!sectorPtr) {
        //We're probably in the area editor, where things change too often
        //for us to cache the sector pointer and access said cache.
        //Let's calculate now real quick.
        sectorPtr = getSector(stopPtr->pos, nullptr, false);
        if(!sectorPtr) {
            //It's really the void. Nothing that can be done here then.
            if(outReason) *outReason = PATH_BLOCK_REASON_STOP_IN_VOID;
            return false;
        }
    }
    
    return canTakePathStop(stopPtr, settings, sectorPtr, outReason);
}


/**
 * @brief Checks if a path stop can be taken given some constraints.
 *
 * @param stopPtr Stop to check.
 * @param settings Settings about how the path should be followed.
 * @param sectorPtr Pointer to the sector this stop is on.
 * @param outReason If not nullptr, the reason is returned here.
 * @return Whether it can take it.
 */
bool canTakePathStop(
    const PathStop* stopPtr, const PathFollowSettings& settings,
    Sector* sectorPtr, PATH_BLOCK_REASON* outReason
) {
    //Check if the end stop has limitations based on the stop flags.
    if(
        hasFlag(stopPtr->flags, PATH_STOP_FLAG_SCRIPT_ONLY) &&
        !hasFlag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE)
    ) {
        if(outReason) *outReason = PATH_BLOCK_REASON_NOT_IN_SCRIPT;
        return false;
    }
    if(
        hasFlag(stopPtr->flags, PATH_STOP_FLAG_LIGHT_LOAD_ONLY) &&
        !hasFlag(settings.flags, PATH_FOLLOW_FLAG_LIGHT_LOAD)
    ) {
        if(outReason) *outReason = PATH_BLOCK_REASON_NOT_LIGHT_LOAD;
        return false;
    }
    if(
        hasFlag(stopPtr->flags, PATH_STOP_FLAG_AIRBORNE_ONLY) &&
        !hasFlag(settings.flags, PATH_FOLLOW_FLAG_AIRBORNE)
    ) {
        if(outReason) *outReason = PATH_BLOCK_REASON_NOT_AIRBORNE;
        return false;
    }
    
    //Check if the travel is limited to stops with a certain label.
    if(!settings.label.empty() && stopPtr->label != settings.label) {
        if(outReason) *outReason = PATH_BLOCK_REASON_NOT_RIGHT_LABEL;
        return false;
    }
    
    //Check if the end stop is hazardous, by checking its sector.
    bool touchingHazard =
        sectorPtr && (
            !sectorPtr->hazardFloor ||
            !hasFlag(settings.flags, PATH_FOLLOW_FLAG_AIRBORNE)
        );
        
    if(
        !hasFlag(settings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES) &&
        touchingHazard &&
        sectorPtr &&
        sectorPtr->hazard &&
        sectorPtr->hazard->blocksPaths
    ) {
        //Check if this hazard doesn't cause Pikmin to try and avoid it.
        bool invulnerable = false;
        for(size_t ih = 0; ih < settings.invulnerabilities.size(); ih++) {
            if(
                settings.invulnerabilities[ih] ==
                sectorPtr->hazard
            ) {
                invulnerable = true;
                break;
            }
        }
        if(!invulnerable) {
            if(outReason) *outReason = PATH_BLOCK_REASON_HAZARDOUS_STOP;
            return false;
        }
    }
    
    //All good!
    return true;
}


/**
 * @brief Checks if a link can be traversed given some constraints.
 *
 * @param linkPtr Link to check.
 * @param settings Settings about how the path should be followed.
 * @param outReason If not nullptr, the reason is returned here.
 * @return Whether it can traverse it.
 */
bool canTraversePathLink(
    PathLink* linkPtr, const PathFollowSettings& settings,
    PATH_BLOCK_REASON* outReason
) {
    if(outReason) *outReason = PATH_BLOCK_REASON_NONE;
    
    //Check if there's an obstacle in the way.
    if(
        !hasFlag(settings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES) &&
        linkPtr->blockedByObstacle
    ) {
        if(outReason) *outReason = PATH_BLOCK_REASON_OBSTACLE;
        return false;
    }
    
    //Get the start and end sectors.
    Sector* startSector = linkPtr->startPtr->sectorPtr;
    if(!startSector) {
        //We're probably in the area editor, where things change too often
        //for us to cache the sector pointer and access said cache.
        //Let's calculate now real quick.
        startSector = getSector(linkPtr->startPtr->pos, nullptr, false);
        if(!startSector) {
            //It's really the void. Nothing that can be done here then.
            if(outReason) *outReason = PATH_BLOCK_REASON_STOP_IN_VOID;
            return false;
        }
    }
    Sector* endSector = linkPtr->endPtr->sectorPtr;
    if(!endSector) {
        //Same as above.
        endSector = getSector(linkPtr->endPtr->pos, nullptr, false);
        if(!endSector) {
            if(outReason) *outReason = PATH_BLOCK_REASON_STOP_IN_VOID;
            return false;
        }
    }
    
    //Check if the link has limitations based on link type.
    switch(linkPtr->type) {
    case PATH_LINK_TYPE_NORMAL: {
        break;
    } case PATH_LINK_TYPE_LEDGE: {
        if(
            !hasFlag(settings.flags, PATH_FOLLOW_FLAG_AIRBORNE) &&
            (endSector->z - startSector->z) > GEOMETRY::STEP_HEIGHT
        ) {
            if(outReason) *outReason = PATH_BLOCK_REASON_UP_LEDGE;
            return false;
        }
        break;
    }
    }
    
    //Check if there's any problem with the stop.
    if(!canTakePathStop(linkPtr->endPtr, settings, endSector, outReason)) {
        return false;
    }
    
    //All good!
    return true;
}


/**
 * @brief Traverses a graph using the depth first search algorithm.
 *
 * @param nodes Vector of nodes.
 * @param visited Set with the visited nodes.
 * @param start Starting node.
 */
void depthFirstSearch(
    vector<PathStop*>& nodes, unordered_set<PathStop*>& visited,
    PathStop* start
) {
    visited.insert(start);
    unordered_set<PathStop*> links;
    
    for(size_t l = 0; l < start->links.size(); l++) {
        links.insert(start->links[l]->endPtr);
    }
    
    for(size_t n = 0; n < nodes.size(); n++) {
        PathStop* nPtr = nodes[n];
        if(nPtr == start) continue;
        if(isInContainer(visited, nPtr)) continue;
        if(nPtr->getLink(start)) {
            links.insert(nPtr);
        }
    }
    
    for(auto& l : links) {
        if(isInContainer(visited, l)) continue;
        depthFirstSearch(nodes, visited, l);
    }
}


/**
 * @brief Gets the shortest available path between two points, following
 * the area's path graph.
 *
 * @param start Start coordinates.
 * @param end End coordinates.
 * @param settings Settings about how the path should be followed.
 * @param fullPath The stops to visit, in order, are returned here, if any.
 * @param outTotalDist If not nullptr, the total path distance is
 * returned here.
 * @param outStartStop If not nullptr, the closest stop to the start is
 * returned here.
 * @param outEndStop If not nullptr, the closest stop to the end is
 * returned here.
 * @return The operation's result.
 */
PATH_RESULT getPath(
    const Point& start, const Point& end,
    const PathFollowSettings& settings,
    vector<PathStop*>& fullPath, float* outTotalDist,
    PathStop** outStartStop, PathStop** outEndStop
) {

    fullPath.clear();
    
    if(game.curAreaData->pathStops.empty()) {
        if(outTotalDist) *outTotalDist = 0.0f;
        return PATH_RESULT_DIRECT_NO_STOPS;
    }
    
    Point startToUse =
        hasFlag(settings.flags, PATH_FOLLOW_FLAG_FAKED_START) ?
        settings.fakedStart :
        start;
        
    Point endToUse =
        hasFlag(settings.flags, PATH_FOLLOW_FLAG_FAKED_END) ?
        settings.fakedEnd :
        end;
        
    //Start by finding the closest stops to the start and finish.
    PathStop* closestToStart = nullptr;
    PathStop* closestToEnd = nullptr;
    float closestToStartDist = 0.0f;
    float closestToEndDist = 0.0f;
    
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* sPtr = game.curAreaData->pathStops[s];
        
        float distToStart =
            Distance(startToUse, sPtr->pos).toFloat() - sPtr->radius;
        float distToEnd =
            Distance(endToUse, sPtr->pos).toFloat() - sPtr->radius;
        distToStart = std::max(0.0f, distToStart);
        distToEnd = std::max(0.0f, distToEnd);
        
        bool isNewStart =
            !closestToStart || distToStart < closestToStartDist;
        bool isNewEnd =
            !closestToEnd || distToEnd < closestToEndDist;
            
        if(isNewStart || isNewEnd) {
            //We actually want this stop. Check now if it can be used.
            //We're not checking this earlier due to performance.
            if(!canTakePathStop(sPtr, settings)) {
                //Can't be taken. Skip.
                continue;
            }
        } else {
            //Not the closest so far. Skip.
            continue;
        }
        
        if(isNewStart) {
            closestToStartDist = distToStart;
            closestToStart = sPtr;
        }
        if(isNewEnd) {
            closestToEndDist = distToEnd;
            closestToEnd = sPtr;
        }
    }
    
    if(outStartStop) *outStartStop = closestToStart;
    if(outEndStop) *outEndStop = closestToEnd;
    
    //Let's just check something real quick:
    //if the destination is closer than any stop,
    //just go there right away!
    Distance startToEndDist(startToUse, endToUse);
    if(startToEndDist <= closestToStartDist) {
        if(outTotalDist) {
            *outTotalDist = startToEndDist.toFloat();
        }
        return PATH_RESULT_DIRECT;
    }
    
    //If the start and destination share the same closest spot,
    //that means this is the only stop in the path.
    if(closestToStart == closestToEnd) {
        fullPath.push_back(closestToStart);
        if(outTotalDist) {
            *outTotalDist = closestToStartDist;
            *outTotalDist += closestToEndDist;
        }
        return PATH_RESULT_PATH_WITH_SINGLE_STOP;
    }
    
    //Potential optimization: instead of calculating with this graph, consult
    //a different one where nodes that only have two links are removed.
    //e.g. A -> B -> C becomes A -> C.
    //This means traversing fewer nodes when figuring out the shortest path.
    
    //Calculate the path.
    PATH_RESULT result =
        aStar(
            fullPath,
            closestToStart, closestToEnd,
            settings, outTotalDist
        );
        
    if(outTotalDist && !fullPath.empty()) {
        *outTotalDist +=
            Distance(startToUse, fullPath[0]->pos).toFloat();
        *outTotalDist +=
            Distance(fullPath[fullPath.size() - 1]->pos, endToUse).toFloat();
    }
    
    return result;
}


/**
 * @brief Returns a string representation of a path block reason.
 *
 * @param reason Reason to convert.
 * @return The string.
 */
string pathBlockReasonToString(PATH_BLOCK_REASON reason) {
    switch(reason) {
    case PATH_BLOCK_REASON_NONE: {
        return "None";
        break;
    } case PATH_BLOCK_REASON_NO_PATH: {
        return "Invalid path";
        break;
    } case PATH_BLOCK_REASON_OBSTACLE: {
        return "Obstacle mob in the way";
        break;
    } case PATH_BLOCK_REASON_NOT_IN_SCRIPT: {
        return "Mob path should be from script";
        break;
    } case PATH_BLOCK_REASON_NOT_LIGHT_LOAD: {
        return "Mob should be light load";
        break;
    } case PATH_BLOCK_REASON_NOT_AIRBORNE: {
        return "Mob should be airborne";
        break;
    } case PATH_BLOCK_REASON_UP_LEDGE: {
        return "Mob cannot go up ledge";
        break;
    } case PATH_BLOCK_REASON_NOT_RIGHT_LABEL: {
        return "Mob's following links with a different label";
        break;
    } case PATH_BLOCK_REASON_STOP_IN_VOID: {
        return "Next path stop is in the void";
        break;
    } case PATH_BLOCK_REASON_HAZARDOUS_STOP: {
        return "Next stop is in hazardous sector";
        break;
    }
    }
    return "";
}



/**
 * @brief Returns a string representation of a path result.
 *
 * @param result Result to convert.
 * @return The string.
 */
string pathResultToString(PATH_RESULT result) {
    switch(result) {
    case PATH_RESULT_NORMAL_PATH: {
        return "Normal open path";
        break;
    } case PATH_RESULT_PATH_WITH_OBSTACLES: {
        return "Path exists, but with obstacles";
        break;
    } case PATH_RESULT_PATH_WITH_SINGLE_STOP: {
        return "Only a single stop is visited";
        break;
    } case PATH_RESULT_DIRECT: {
        return "Go directly";
        break;
    } case PATH_RESULT_DIRECT_NO_STOPS: {
        return "No stops, so go directly";
        break;
    } case PATH_RESULT_END_STOP_UNREACHABLE: {
        return "Final stop cannot be reached from first stop";
        break;
    } case PATH_RESULT_NO_DESTINATION: {
        return "Destination was never set";
        break;
    } case PATH_RESULT_ERROR: {
        return "Could not calculate a path";
        break;
    } case PATH_RESULT_NOT_CALCULATED: {
        return "Not calculated yet";
        break;
    }
    }
    return "";
}
