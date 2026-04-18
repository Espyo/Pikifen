/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor functions related to raw geometry editing logic, with
 * no dependencies on GUI and canvas implementations.
 */

#include <algorithm>

#include "editor.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/container_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Creates a new sector for use in layout drawing operations
 * and adds it to the area.
 * This automatically clones it from another sector, if not nullptr, or gives it
 * a recommended texture if the other sector nullptr.
 *
 * @param copyFrom Sector to copy from.
 * @return The created sector.
 */
Sector* AreaEditor::addNewSectorForLayoutDrawing(const Sector* copyFrom) {
    Sector* newSector = game.curArea->addNewSector();
    
    if(copyFrom) {
        copyFrom->clone(newSector);
        updateSectorTexture(newSector, copyFrom->textureInfo.bmpName);
    } else {
        if(!textureSuggestions.empty()) {
            updateSectorTexture(newSector, textureSuggestions[0].name);
        } else {
            updateSectorTexture(newSector, "");
        }
    }
    
    return newSector;
}


/**
 * @brief Checks whether it's possible to traverse from drawing node n1 to n2
 * with the existing edges and vertexes. In other words, if you draw a line
 * between n1 and n2, it will not go inside a sector.
 *
 * @param n1 Starting drawing node.
 * @param n2 Ending drawing node.
 * @return Whether they are traversable.
 */
bool AreaEditor::areNodesTraversable(
    const LayoutDrawingNode& n1, const LayoutDrawingNode& n2
) const {
    if(n1.onSector || n2.onSector) return false;
    
    if(n1.onEdge && n2.onEdge) {
        if(n1.onEdge != n2.onEdge) return false;
        
    } else if(n1.onEdge && n2.onVertex) {
        if(
            n1.onEdge->vertexes[0] != n2.onVertex &&
            n1.onEdge->vertexes[1] != n2.onVertex
        ) {
            return false;
        }
        
    } else if(n1.onVertex && n2.onVertex) {
        if(!n1.onVertex->getEdgeByNeighbor(n2.onVertex)) {
            return false;
        }
        
    } else if(n1.onVertex && n2.onEdge) {
        if(
            n2.onEdge->vertexes[0] != n1.onVertex &&
            n2.onEdge->vertexes[1] != n1.onVertex
        ) {
            return false;
        }
    }
    return true;
}


/**
 * @brief Calculates the preview path.
 *
 * @return The final distance.
 */
float AreaEditor::calculatePreviewPath() {
    if(!showPathPreview) return 0;
    
    float d = 0;
    
    //We don't have a way to specify the invulnerabilities, since
    //hazards aren't saved to the sector data in the area editor.
    pathPreviewResult =
        getPath(
            pathPreviewCheckpoints[0],
            pathPreviewCheckpoints[1],
            pathPreviewSettings,
            pathPreview, &d,
            &pathPreviewClosest[0], &pathPreviewClosest[1]
        );
        
    if(pathPreview.empty() && d == 0) {
        d =
            Distance(
                pathPreviewCheckpoints[0],
                pathPreviewCheckpoints[1]
            ).toFloat();
    }
    
    return d;
}


/**
 * @brief Checks if the line the user is trying to draw is okay.
 * Sets the line's status to drawingDineResult.
 *
 * @param pos Position the user is trying to finish the line on.
 */
void AreaEditor::checkDrawingLine(const Point& pos) {
    drawingLineResult = DRAWING_LINE_RESULT_OK;
    
    if(drawingNodes.empty()) {
        return;
    }
    
    LayoutDrawingNode* prevNode = &drawingNodes.back();
    LayoutDrawingNode tentativeNode(this, pos);
    
    //Check if the user hits a vertex or an edge, but the drawing is
    //meant to be a new sector shape.
    if(
        (!drawingNodes[0].onEdge && !drawingNodes[0].onVertex) &&
        (tentativeNode.onEdge || tentativeNode.onVertex)
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX;
        return;
    }
    
    //Check if it's just hitting the same edge, or vertexes of the same edge.
    if(
        tentativeNode.onEdge &&
        tentativeNode.onEdge == prevNode->onEdge
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    if(
        tentativeNode.onVertex &&
        tentativeNode.onVertex->hasEdge(prevNode->onEdge)
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    if(
        prevNode->onVertex &&
        prevNode->onVertex->hasEdge(tentativeNode.onEdge)
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    if(
        tentativeNode.onVertex &&
        tentativeNode.onVertex->isNeighbor(prevNode->onVertex)
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    
    //Check for edge collisions in collinear lines.
    forIdx(e, game.curArea->edges) {
        //We don't need to watch out for the edge of the current point
        //or the previous one, since this collinearity check doesn't
        //return true for line segments that touch in only one point.
        Edge* ePtr = game.curArea->edges[e];
        Point ep1 = v2p(ePtr->vertexes[0]);
        Point ep2 = v2p(ePtr->vertexes[1]);
        
        if(
            lineSegsAreCollinear(prevNode->snappedSpot, pos, ep1, ep2)
        ) {
            if(
                collinearLineSegsIntersect(
                    prevNode->snappedSpot, pos, ep1, ep2
                )
            ) {
                drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
                return;
            }
        }
    }
    
    //Check for edge collisions.
    forIdx(e, game.curArea->edges) {
        Edge* ePtr = game.curArea->edges[e];
        //If this edge is the same or a neighbor of the previous node,
        //then never mind.
        if(
            prevNode->onEdge == ePtr ||
            tentativeNode.onEdge == ePtr
        ) {
            continue;
        }
        if(prevNode->onVertex) {
            if(
                ePtr->vertexes[0] == prevNode->onVertex ||
                ePtr->vertexes[1] == prevNode->onVertex
            ) {
                continue;
            }
        }
        if(tentativeNode.onVertex) {
            if(
                ePtr->vertexes[0] == tentativeNode.onVertex ||
                ePtr->vertexes[1] == tentativeNode.onVertex
            ) {
                continue;
            }
        }
        
        if(
            lineSegsIntersect(
                prevNode->snappedSpot, pos,
                v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]),
                nullptr, nullptr
            )
        ) {
            drawingLineResult = DRAWING_LINE_RESULT_CROSSES_EDGES;
            return;
        }
    }
    
    //Check if the line intersects with the drawing's lines.
    if(drawingNodes.size() >= 2) {
        for(size_t n = 0; n < drawingNodes.size() - 2; n++) {
            LayoutDrawingNode* n1Ptr = &drawingNodes[n];
            LayoutDrawingNode* n2Ptr = &drawingNodes[n + 1];
            Point intersection;
            if(
                lineSegsIntersect(
                    prevNode->snappedSpot, pos,
                    n1Ptr->snappedSpot, n2Ptr->snappedSpot,
                    &intersection
                )
            ) {
                if(
                    Distance(intersection, drawingNodes.begin()->snappedSpot) >
                    AREA_EDITOR::VERTEX_MERGE_RADIUS / game.editorsView.cam.zoom
                ) {
                    //Only a problem if this isn't the user's drawing finish.
                    drawingLineResult = DRAWING_LINE_RESULT_CROSSES_DRAWING;
                    return;
                }
            }
        }
        
        if(
            circleIntersectsLineSeg(
                pos, 8.0 / game.editorsView.cam.zoom,
                prevNode->snappedSpot,
                drawingNodes[drawingNodes.size() - 2].snappedSpot
            )
        ) {
            drawingLineResult = DRAWING_LINE_RESULT_CROSSES_DRAWING;
            return;
        }
    }
    
    //Check if this line is entering a sector different from the one the
    //rest of the drawing is on.
    if(drawingNodes.size() >= 2) {
        //This check only makes sense from the third node onward.
        //Since both the first and the second node can't be on edges or
        //vertexes, and no node can cross edges or vertexes,
        //this means we can grab the midpoint of the first
        //and second nodes to get the sector the second node is on, or the
        //sector the second node is passing through. Basically,
        //the working sector.
        //This check is useful when the player tries to split a sector with
        //a useless split, and is tasked with continuing the drawing.
        Point workingSectorPoint(
            (
                drawingNodes[0].snappedSpot.x +
                drawingNodes[1].snappedSpot.x
            ) / 2.0f,
            (
                drawingNodes[0].snappedSpot.y +
                drawingNodes[1].snappedSpot.y
            ) / 2.0f
        );
        Sector* workingSector = getSectorUnderPoint(workingSectorPoint);
        
        Point latestSectorPoint(
            (
                drawingNodes.back().snappedSpot.x +
                pos.x
            ) / 2.0f,
            (
                drawingNodes.back().snappedSpot.y +
                pos.y
            ) / 2.0f
        );
        Sector* latestSector = getSectorUnderPoint(latestSectorPoint);
        
        if(latestSector != workingSector) {
            drawingLineResult = DRAWING_LINE_RESULT_WAYWARD_SECTOR;
            return;
        }
    }
    
}


/**
 * @brief Copies the currently selected edge's properties onto the copy buffer,
 * so they can be then pasted onto another edge.
 */
void AreaEditor::copyEdgeProperties() {
    if(!edgeSelection.hasAny()) {
        setStatus(
            "To copy an edge's properties, you must first select an edge "
            "to copy from!",
            true
        );
        return;
    }
    
    if(edgeSelection.getCount() > 1) {
        setStatus(
            "To copy an edge's properties, you can only select 1 edge!",
            true
        );
        return;
    }
    
    Edge* sourceEdge =
        game.curArea->edges[edgeSelection.getSingleItemIdx()];
    if(!copyBufferEdge) {
        copyBufferEdge = new Edge();
    }
    sourceEdge->clone(copyBufferEdge);
    setStatus("Successfully copied the edge's properties.");
}


/**
 * @brief Copies the currently selected mob's properties onto the copy buffer,
 * so they can be then pasted onto another mob.
 */
void AreaEditor::copyMobProperties() {
    if(!mobSelection.hasAny()) {
        setStatus(
            "To copy an object's properties, you must first select an object "
            "to copy from!",
            true
        );
        return;
    }
    
    if(!mobSelection.hasOne()) {
        setStatus(
            "To copy an object's properties, you can only select 1 object!",
            true
        );
        return;
    }
    
    MobGen* sourceMob =
        game.curArea->mobGenerators[mobSelection.getSingleItemIdx()];
    if(!copyBufferMob) {
        copyBufferMob = new MobGen();
    }
    sourceMob->clone(copyBufferMob);
    setStatus("Successfully copied the object's properties.");
}


/**
 * @brief Copies the currently selected path link's properties onto the
 * copy buffer, so they can be then pasted onto another path link.
 *
 */
void AreaEditor::copyPathLinkProperties() {
    if(!pathLinkSelection.hasAny()) {
        setStatus(
            "To copy a path link's properties, you must first select a path "
            "link to copy from!",
            true
        );
        return;
    }
    
    if(pathLinkSelection.getCount() > 1) {
        setStatus(
            "To copy a path link's properties, you can only select 1 "
            "path link!",
            true
        );
        return;
    }
    
    EditorPathLink* sourceELink =
        &game.curArea->editorPathLinks[pathLinkSelection.getFirstItemIdx()];
    if(!copyBufferPathLink) {
        copyBufferPathLink = new PathLink(nullptr, nullptr, INVALID);
    }
    sourceELink->link1->clone(copyBufferPathLink);
    setStatus("Successfully copied the path link's properties.");
}


/**
 * @brief Copies the currently selected sector's properties onto the
 * copy buffer, so they can be then pasted onto another sector.
 *
 */
void AreaEditor::copySectorProperties() {
    if(!sectorSelection.hasAny()) {
        setStatus(
            "To copy a sector's properties, you must first select a sector "
            "to copy from!",
            true
        );
        return;
    }
    
    if(sectorSelection.getCount() > 1) {
        setStatus(
            "To copy a sector's properties, you can only select 1 sector!",
            true
        );
        return;
    }
    
    Sector* sourceSector =
        game.curArea->sectors[sectorSelection.getFirstItemIdx()];
    if(!copyBufferSector) {
        copyBufferSector = new Sector();
    }
    sourceSector->clone(copyBufferSector);
    copyBufferSector->textureInfo = sourceSector->textureInfo;
    setStatus("Successfully copied the sector's properties.");
}


/**
 * @brief Removes and deletes the specified edge, removing it from all
 * sectors and vertexes that use it, as well as removing and deleting any
 * now-useless sectors or vertexes.
 *
 * @param ePtr Edge to delete.
 */
void AreaEditor::deleteEdge(Edge* ePtr) {
    //Delete sectors first.
    Sector* sectors[2] = { ePtr->sectors[0], ePtr->sectors[1] };
    ePtr->removeFromSectors();
    for(size_t s = 0; s < 2; s++) {
        if(!sectors[s]) continue;
        if(sectors[s]->edges.empty()) {
            game.curArea->deleteSector(sectors[s]);
        }
    }
    
    //Now, delete vertexes.
    Vertex* vertexes[2] = { ePtr->vertexes[0], ePtr->vertexes[1] };
    ePtr->removeFromVertexes();
    for(size_t v = 0; v < 2; v++) {
        if(vertexes[v]->edges.empty()) {
            game.curArea->deleteVertex(vertexes[v]);
        }
    }
    
    //Finally, delete the edge proper.
    game.curArea->deleteEdge(ePtr);
}


/**
 * @brief Removes and deletes the selected edges. The sectors on each side
 * of the edge are merged, so the smallest sector will be deleted. In addition,
 * this operation will remove and delete any sectors that would
 * end up incomplete.
 *
 * @return Whether all edges were deleted successfully.
 */
bool AreaEditor::deleteSelectedEdges() {
    bool success = true;
    
    const set<size_t>& selectedEdges = edgeSelection.getItemIdxs();
    
    for(size_t eIdx : selectedEdges) {
        Edge* ePtr = game.curArea->edges[eIdx];
        
        if(!ePtr->vertexes[0]) {
            //Huh, looks like one of the edge deletion procedures already
            //wiped this edge out. Skip it.
            continue;
        }
        
        success &= mergeSectors(ePtr->sectors[0], ePtr->sectors[1]);
    }
    
    return success;
}


/**
 * @brief Removes and deletes the selected mobs.
 */
void AreaEditor::deleteSelectedMobs() {
    const set<size_t>& selectedMobs = mobSelection.getItemIdxs();
    
    for(size_t mIdx : selectedMobs) {
        //Update links.
        forIdx(m2, game.curArea->mobGenerators) {
            MobGen* m2Ptr = game.curArea->mobGenerators[m2];
            forIdx(l, m2Ptr->links) {
                if(m2Ptr->linkIdxs[l] == mIdx) {
                    m2Ptr->links.erase(m2Ptr->links.begin() + l);
                    m2Ptr->linkIdxs.erase(m2Ptr->linkIdxs.begin() + l);
                } else {
                    adjustMisalignedIndex(m2Ptr->linkIdxs[l], mIdx, false);
                }
            }
            
            if(m2Ptr->storedInside != INVALID) {
                if(m2Ptr->storedInside == mIdx) {
                    m2Ptr->storedInside = INVALID;
                } else {
                    adjustMisalignedIndex(m2Ptr->storedInside, mIdx, false);
                }
            }
        }
        
        //Update mob groups.
        forIdx(c, game.curArea->mission.mobGroups) {
            MissionMobGroup* cPtr = &game.curArea->mission.mobGroups[c];
            for(size_t m = 0; m < cPtr->mobIdxs.size();) {
                if(cPtr->mobIdxs[m] == mIdx) {
                    cPtr->mobIdxs.erase(cPtr->mobIdxs.begin() + m);
                } else {
                    adjustMisalignedIndex(cPtr->mobIdxs[m], mIdx, false);
                    m++;
                }
            }
        }
        
        //Delete it.
        delete game.curArea->mobGenerators[mIdx];
    }
    
    //Finally, erase them from the vector.
    eraseIndexesInVector(selectedMobs, game.curArea->mobGenerators);
}


/**
 * @brief Removes and deletes the selected path links.
 */
void AreaEditor::deleteSelectedPathLinks() {
    const set<size_t>& selectedLinks = pathLinkSelection.getItemIdxs();
    
    for(size_t lIdx : selectedLinks) {
        EditorPathLink* elPtr = &game.curArea->editorPathLinks[lIdx];
        //Delete it from the start path stop.
        elPtr->link1->startPtr->deleteLink(elPtr->link1);
        
        //Delete it from the end path stop.
        if(elPtr->link2) elPtr->link2->startPtr->deleteLink(elPtr->link2);
    }
    
    //Finally, erase them from the vector.
    eraseIndexesInVector(selectedLinks, game.curArea->editorPathLinks);
}


/**
 * @brief Removes and deletes the selected path stops.
 */
void AreaEditor::deleteSelectedPathStops() {
    const set<size_t>& selectedStops = pathStopSelection.getItemIdxs();
    
    for(auto const& sIdx : selectedStops) {
        //Check all links that end at this stop.
        forIdx(s2, game.curArea->pathStops) {
            PathStop* s2Ptr = game.curArea->pathStops[s2];
            s2Ptr->deleteLink(game.curArea->pathStops[sIdx]);
        }
        
        //Delete it.
        delete game.curArea->pathStops[sIdx];
    }
    
    //Finally, erase them from the vector.
    eraseIndexesInVector(selectedStops, game.curArea->pathStops);
}


/**
 * @brief Removes and deletes the selected regions.
 */
void AreaEditor::deleteSelectedRegions() {
    const set<size_t>& selectedRegions = regionSelection.getItemIdxs();
    
    for(size_t rIdx : selectedRegions) {
        //Update end conditions.
        forIdx(e, game.curArea->mission.endConds) {
            MissionEndCond* ePtr = &game.curArea->mission.endConds[e];
            if(!ePtr->usesMetric()) continue;
            if(ePtr->metricType != MISSION_METRIC_LEADERS_IN_REGION) {
                continue;
            }
            if(ePtr->idxParam == 0) continue;
            adjustMisalignedIndex(
                ePtr->idxParam, rIdx, false
            );
        }
        
        //Delete it.
        delete game.curArea->regions[rIdx];
    }
    
    //Finally, erase them from the vector.
    eraseIndexesInVector(selectedRegions, game.curArea->regions);
}


/**
 * @brief Removes and deletes the selected tree shadows.
 */
void AreaEditor::deleteSelectedTreeShadows() {
    const set<size_t>& selectedShadows = shadowSelection.getItemIdxs();
    
    for(size_t sIdx : selectedShadows) {
        //Delete it.
        delete game.curArea->treeShadows[sIdx];
    }
    
    //Finally, erase them from the vector.
    eraseIndexesInVector(selectedShadows, game.curArea->treeShadows);
}


/**
 * @brief Tries to find a good texture for the first sector in a
 * newly-created area.
 *
 * @return The texture's internal name, or empty if none was found.
 */
string AreaEditor::findGoodFirstTexture() {
    //First, if there's any "grass" texture, use that.
    for(const auto& g : game.content.bitmaps.manifests) {
        string lcName = strToLower(g.first);
        if(
            lcName.find("texture") != string::npos &&
            lcName.find("grass") != string::npos
        ) {
            return g.first;
        }
    }
    
    //No grass texture? Try one with "dirt".
    for(const auto& g : game.content.bitmaps.manifests) {
        string lcName = strToLower(g.first);
        if(
            lcName.find("texture") != string::npos &&
            lcName.find("dirt") != string::npos
        ) {
            return g.first;
        }
    }
    
    //If there's no good texture, just pick the first one.
    for(const auto& g : game.content.bitmaps.manifests) {
        string lcName = strToLower(g.first);
        if(lcName.find("texture") != string::npos) {
            return g.first;
        }
    }
    
    //Still no good? Give up.
    return "";
}


/**
 * @brief Tries to find problems with the area.
 * When it's done, sets the appropriate problem-related variables.
 */
void AreaEditor::findProblems() {
    //First, clear any problem info.
    clearProblems();
    
    //Now, check for each of the clauses.
    findProblemsIntersectingEdge();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsOverlappingVertex();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsNonSimpleSector();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsLoneEdge();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsMissingLeader();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsTypelessMob();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsOobMob();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsMobInsideWalls();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsMobLinksToSelf();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsMobStoredInLoop();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsPikminOverLimit();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsBridgePath();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsOobPathStop();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsLonePathStop();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsPathStopOnLink();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsMissingTexture();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsUnknownTexture();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsPathStopsIntersecting();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsUnknownTreeShadow();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsEmptyMissionMobGroup();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsMissionNoEnd();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsMissionMultiplePauseEnds();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsInvalidIdxParamMissionEndCond();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsInvalidIdxParamMissionHudItem();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsInvalidIdxParamMissionScoreCri();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsNoTimeLimitMissionEndCond();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsNoTimeLimitMissionHudItem();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsNoTimeLimitMissionScoreCri();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsZeroPointMissionScoreCri();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsNoMissionScoreCri();
    if(problemType != EPT_NONE_YET) return;
    
    //All good!
    problemType = EPT_NONE;
    problemTitle = "None!";
    problemDescription.clear();
}


/**
 * @brief Checks for any pile-to-bridge paths blocked by said bridge in the
 * area, and fills the problem info if so.
 */
void AreaEditor::findProblemsBridgePath() {
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        if(!mPtr->type) continue;
        if(mPtr->type->category->id != MOB_CATEGORY_PILES) {
            continue;
        }
        
        forIdx(l, mPtr->links) {
            if(!mPtr->links[l]->type) continue;
            if(mPtr->links[l]->type->category->id != MOB_CATEGORY_BRIDGES) {
                continue;
            }
            
            PathFollowSettings settings;
            settings.flags =
                PATH_FOLLOW_FLAG_SCRIPT_USE |
                PATH_FOLLOW_FLAG_LIGHT_LOAD |
                PATH_FOLLOW_FLAG_AIRBORNE;
            vector<PathStop*> path;
            getPath(
                mPtr->pos, mPtr->links[l]->pos,
                settings, path,
                nullptr, nullptr, nullptr
            );
            
            for(size_t s = 1; s < path.size(); s++) {
                if(
                    circleIntersectsLineSeg(
                        mPtr->links[l]->pos,
                        getMobGenRadius(mPtr->links[l]),
                        path[s - 1]->pos,
                        path[s]->pos
                    )
                ) {
                    problemMobPtr = mPtr->links[l];
                    problemType = EPT_PILE_BRIDGE_PATH;
                    problemTitle =
                        "Bridge is blocking the path to itself!";
                    problemDescription =
                        "The path Pikmin must take from a pile to this "
                        "bridge is blocked by the unbuilt bridge object "
                        "itself. Move the path stop to some place a bit "
                        "before the bridge object.";
                    return;
                }
            }
        }
    }
}


/**
 * @brief Checks for any empty mission mob groups,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsEmptyMissionMobGroup() {
    forIdx(g, game.curArea->mission.mobGroups) {
        MissionMobGroup* gPtr = &game.curArea->mission.mobGroups[g];
        if(gPtr->calculateList().empty()) {
            problemType = EPT_EMPTY_MISSION_MOB_GROUP;
            problemTitle =
                "Empty mission mob group!";
            problemDescription =
                "Mission mob group #" + i2s(g + 1) + " has no "
                "mobs, making it pointless. Either assign some "
                "or delete the group.";
            return;
        }
    }
}


/**
 * @brief Checks for any intersecting edges in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsIntersectingEdge() {
    vector<EdgeIntersection> intersections = getIntersectingEdges();
    if(!intersections.empty()) {
        float r;
        EdgeIntersection* eiPtr = &(*intersections.begin());
        lineSegsIntersect(
            v2p(eiPtr->e1->vertexes[0]), v2p(eiPtr->e1->vertexes[1]),
            v2p(eiPtr->e2->vertexes[0]), v2p(eiPtr->e2->vertexes[1]),
            &r, nullptr
        );
        
        float a =
            getAngle(
                v2p(eiPtr->e1->vertexes[0]), v2p(eiPtr->e1->vertexes[1])
            );
        Distance d(
            v2p(eiPtr->e1->vertexes[0]), v2p(eiPtr->e1->vertexes[1])
        );
        
        problemEdgeIntersection = *intersections.begin();
        problemType = EPT_INTERSECTING_EDGES;
        problemTitle = "Two edges cross each other!";
        problemDescription =
            "They cross at (" +
            f2s(
                floor(eiPtr->e1->vertexes[0]->x + cos(a) * r *
                      d.toFloat())
            ) + "," + f2s(
                floor(eiPtr->e1->vertexes[0]->y + sin(a) * r *
                      d.toFloat())
            ) + "). Edges should never cross each other.";
    }
}


/**
 * @brief Checks for invalid index parameters in mission end conditions,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsInvalidIdxParamMissionEndCond() {
    forIdx(c, game.curArea->mission.endConds) {
        MissionEndCond* cPtr = &game.curArea->mission.endConds[c];
        if(!cPtr->usesMetric()) continue;
        MissionMetricType* metricTypePtr =
            game.missionMetricTypes[cPtr->metricType];
        if(metricTypePtr->getInfo().idxParamName.empty()) continue;
        
        bool idxInBounds = true;
        switch(cPtr->metricType) {
        case MISSION_METRIC_LEADERS_IN_REGION: {
            idxInBounds =
                cPtr->idxParam < game.curArea->regions.size();
            break;
        } default: {
            idxInBounds =
                cPtr->idxParam < game.curArea->mission.mobGroups.size();
            break;
        }
        }
        if(!idxInBounds) {
            problemType = EPT_INVALID_IDX_PARAM_MISSION_END_COND;
            problemTitle =
                "Invalid number in mission end condition!";
            problemDescription =
                "Mission end condition #" + i2s(c + 1) + " has a parameter "
                "that refers to an area region or a mob group, but that "
                "number is invalid. Please correct it.";
            return;
        }
    }
}


/**
 * @brief Checks for invalid index parameters in mission HUD items,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsInvalidIdxParamMissionHudItem() {
    forIdx(i, game.curArea->mission.hudItems) {
        MissionHudItem* iPtr = &game.curArea->mission.hudItems[i];
        if(!iPtr->enabled) continue;
        
        MissionMetricType* metricTypePtr =
            game.missionMetricTypes[iPtr->metricType];
        if(metricTypePtr->getInfo().idxParamName.empty()) continue;
        
        bool idxInBounds = true;
        switch(iPtr->metricType) {
        case MISSION_METRIC_LEADERS_IN_REGION: {
            idxInBounds =
                iPtr->idxParam < game.curArea->regions.size();
            break;
        } default: {
            idxInBounds =
                iPtr->idxParam < game.curArea->mission.mobGroups.size();
            break;
        }
        }
        if(!idxInBounds) {
            problemType = EPT_INVALID_IDX_PARAM_MISSION_HUD_ITEM;
            problemTitle =
                "Invalid number in mission HUD item!";
            problemDescription =
                "Mission HUD item #" + i2s(i + 1) + " has a parameter "
                "that refers to an area region or a mob group, but that "
                "number is invalid. Please correct it.";
            return;
        }
    }
}


/**
 * @brief Checks for invalid index parameters in mission score criteria,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsInvalidIdxParamMissionScoreCri() {
    forIdx(c, game.curArea->mission.scoreCriteria) {
        MissionScoreCriterion* cPtr = &game.curArea->mission.scoreCriteria[c];
        
        bool idxInBounds = true;
        switch(cPtr->metricType) {
        case MISSION_METRIC_LEADERS_IN_REGION: {
            idxInBounds =
                cPtr->idxParam < game.curArea->regions.size();
            break;
        } default: {
            idxInBounds =
                cPtr->idxParam < game.curArea->mission.mobGroups.size();
            break;
        }
        }
        
        if(!idxInBounds) {
            problemType = EPT_INVALID_IDX_PARAM_MISSION_SCORE_CRI;
            problemTitle =
                "Invalid number in mission score criterion!";
            problemDescription =
                "Mission score criterion #" + i2s(c + 1) + " has a parameter "
                "that refers to an area region or a mob group, but that "
                "number is invalid. Please correct it.";
            return;
        }
    }
}


/**
 * @brief Checks for any lone edges in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsLoneEdge() {
    if(!game.curArea->problems.loneEdges.empty()) {
        problemType = EPT_LONE_EDGE;
        problemTitle = "Lone edge!";
        problemDescription =
            "Likely leftover of something that went wrong. "
            "You probably want to drag one vertex into the other.";
    }
}


/**
 * @brief Checks for any lone path stops in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsLonePathStop() {
    forIdx(s, game.curArea->pathStops) {
        PathStop* sPtr = game.curArea->pathStops[s];
        bool hasLink = false;
        
        if(!sPtr->links.empty()) continue; //Duh, this means it has links.
        
        forIdx(s2, game.curArea->pathStops) {
            PathStop* s2Ptr = game.curArea->pathStops[s2];
            if(s2Ptr == sPtr) continue;
            
            if(s2Ptr->getLink(sPtr)) {
                hasLink = true;
                break;
            }
            
            if(hasLink) break;
        }
        
        if(!hasLink) {
            problemPathStopPtr = sPtr;
            problemType = EPT_LONE_PATH_STOP;
            problemTitle = "Lone path stop!";
            problemDescription =
                "Either connect it to another stop, or delete it.";
            return;
        }
    }
}


/**
 * @brief Checks for any missing leaders in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsMissingLeader() {
    bool hasLeader = false;
    forIdx(m, game.curArea->mobGenerators) {
        if(
            game.curArea->mobGenerators[m]->type != nullptr &&
            game.curArea->mobGenerators[m]->type->category->id ==
            MOB_CATEGORY_LEADERS
        ) {
            hasLeader = true;
            break;
        }
    }
    if(!hasLeader) {
        problemType = EPT_MISSING_LEADER;
        problemTitle = "No leader!";
        problemDescription =
            "You need at least one leader to actually play.";
    }
}


/**
 * @brief Checks for any missing texture in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsMissingTexture() {
    forIdx(s, game.curArea->sectors) {
        Sector* sPtr = game.curArea->sectors[s];
        if(sPtr->edges.empty()) continue;
        if(sPtr->isBottomlessPit) continue;
        if(
            sPtr->textureInfo.bmpName.empty() &&
            !sPtr->isBottomlessPit && !sPtr->fade
        ) {
            problemSectorPtr = sPtr;
            problemType = EPT_UNKNOWN_TEXTURE;
            problemTitle = "Sector with missing texture!";
            problemDescription =
                "Give it a valid texture.";
            return;
        }
    }
}


/**
 * @brief Checks for the existence of multiple pause menu mission end
 * conditions, and fills the problem info if so.
 */
void AreaEditor::findProblemsMissionMultiplePauseEnds() {
    bool foundOne = false;
    forIdx(c, game.curArea->mission.endConds) {
        MissionEndCond* cPtr = &game.curArea->mission.endConds[c];
        if(cPtr->type != MISSION_END_COND_PAUSE_MENU) continue;
        if(!foundOne) {
            foundOne = true;
        } else {
            problemType = EPT_MISSION_MULTIPLE_PAUSE_ENDS;
            problemTitle =
                "Multiple pause menu mission end conditions!";
            problemDescription =
                "There are multiple mission end conditions of the "
                "\"pause menu end\" type. Only the first one will "
                "be triggered, making any others pointless.";
            return;
        }
    }
}


/**
 * @brief Checks for the mission having no end conditions,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsMissionNoEnd() {
    if(game.curArea->type != AREA_TYPE_MISSION) return;
    if(game.curArea->mission.endConds.empty()) {
        problemType = EPT_MISSION_NO_END;
        problemTitle =
            "Mission has no end conditions!";
        problemDescription =
            "This mission does not have any end condition. "
            "This means the only thing the player can do is end "
            "the mission early from the pause menu as a fail. If that "
            "is what you intend, please add an end condition for that.";
        return;
    }
}


/**
 * @brief Checks for any mobs are inside walls in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsMobInsideWalls() {
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        if(!mPtr->type) continue;
        
        if(
            mPtr->type->category->id == MOB_CATEGORY_BRIDGES ||
            mPtr->type->category->id == MOB_CATEGORY_DECORATIONS
        ) {
            continue;
        }
        
        forIdx(e, game.curArea->edges) {
            Edge* ePtr = game.curArea->edges[e];
            if(!ePtr->isValid()) continue;
            
            if(
                circleIntersectsLineSeg(
                    mPtr->pos,
                    mPtr->type->radius,
                    v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]),
                    nullptr, nullptr
                )
            ) {
            
                if(
                    ePtr->sectors[0] && ePtr->sectors[1] &&
                    ePtr->sectors[0]->z == ePtr->sectors[1]->z
                ) {
                    continue;
                }
                
                Sector* mobSector = getSector(mPtr->pos, nullptr, false);
                
                bool inWall = false;
                
                if(
                    !ePtr->sectors[0] || !ePtr->sectors[1]
                ) {
                    //Either sector is the void, definitely stuck.
                    inWall = true;
                    
                } else if(
                    ePtr->sectors[0] != mobSector &&
                    ePtr->sectors[1] != mobSector
                ) {
                    //It's intersecting with two sectors that aren't
                    //even the sector it's on? Definitely inside wall.
                    inWall = true;
                    
                } else if(
                    ePtr->sectors[0]->type == SECTOR_TYPE_BLOCKING ||
                    ePtr->sectors[1]->type == SECTOR_TYPE_BLOCKING
                ) {
                    //If either sector's of the blocking type, definitely stuck.
                    inWall = true;
                    
                } else if(
                    ePtr->sectors[0] == mobSector &&
                    ePtr->sectors[1]->z > mobSector->z + GEOMETRY::STEP_HEIGHT
                ) {
                    inWall = true;
                    
                } else if(
                    ePtr->sectors[1] == mobSector &&
                    ePtr->sectors[0]->z > mobSector->z + GEOMETRY::STEP_HEIGHT
                ) {
                    inWall = true;
                    
                }
                
                if(inWall) {
                    problemMobPtr = mPtr;
                    problemType = EPT_MOB_IN_WALL;
                    problemTitle = "Mob stuck in wall!";
                    problemDescription =
                        "This object should not be stuck inside of a wall. "
                        "Move it to somewhere where it has more space.";
                    return;
                }
            }
        }
    }
}


/**
 * @brief Checks for any mob that links to itself in the area, and fills
 * the problem info if so.
 */
void AreaEditor::findProblemsMobLinksToSelf() {
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        forIdx(l, mPtr->links) {
            if(mPtr->links[l] == mPtr) {
                problemMobPtr = mPtr;
                problemType = EPT_MOB_LINKS_TO_SELF;
                problemTitle = "Mob links to itself!";
                problemDescription =
                    "This object has a link to itself. This will likely "
                    "cause unexpected behaviors, so you should delete "
                    "the link.";
                return;
            }
        }
    }
}


/**
 * @brief Checks for any mobs stored in other mobs in a loop in the area,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsMobStoredInLoop() {
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        if(mPtr->storedInside == INVALID) continue;
        unordered_set<MobGen*> visitedMobs;
        visitedMobs.insert(mPtr);
        size_t nextIdx = mPtr->storedInside;
        while(nextIdx != INVALID) {
            MobGen* nextPtr = game.curArea->mobGenerators[nextIdx];
            if(isInContainer(visitedMobs, nextPtr)) {
                problemMobPtr = nextPtr;
                problemType = EPT_MOB_STORED_IN_LOOP;
                problemTitle = "Mobs stored in a loop!";
                problemDescription =
                    "This object is stored inside of another object, which "
                    "in turn is inside of another...and eventually, "
                    "one of the objects in this chain is stored inside of the "
                    "first one. This means none of these objects are "
                    "really out in the open, and so will never really be used "
                    "in the area. You probably want to unstore one of them.";
                return;
            }
            visitedMobs.insert(nextPtr);
            nextIdx = nextPtr->storedInside;
        }
    }
}


/**
 * @brief Checks for the mission having no scoring criteria when it should,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsNoMissionScoreCri() {
    if(
        game.curArea->type != AREA_TYPE_MISSION ||
        game.curArea->mission.medalAwardMode != MISSION_MEDAL_AWARD_MODE_POINTS
    ) {
        return;
    }
    if(game.curArea->mission.scoreCriteria.empty()) {
        problemType = EPT_NO_MISSION_SCORE_CRI;
        problemTitle =
            "Mission has no score criteria!";
        problemDescription =
            "This mission award a medal based on points, but has "
            "no scoring criteria, meaning the player can't get points. "
            "Please add some scoring criteria.";
        return;
    }
}


/**
 * @brief Checks for mission end conditions that use a time limit with none set,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsNoTimeLimitMissionEndCond() {
    forIdx(c, game.curArea->mission.endConds) {
        MissionEndCond* cPtr = &game.curArea->mission.endConds[c];
        if(
            cPtr->usesMetric() &&
            cPtr->metricType == MISSION_METRIC_SECS_LEFT &&
            game.curArea->mission.timeLimit == 0
        ) {
            problemType = EPT_NO_TIME_LIMIT_MISSION_END_COND;
            problemTitle =
                "Mission end condition needs a time limit that isn't set!";
            problemDescription =
                "Mission end condition #" + i2s(c + 1) + " makes use of "
                "the mission time limit, but this mission does not have "
                "a time limit set. Either set it or remove the end condition.";
            return;
        }
    }
}


/**
 * @brief Checks for mission HUD items that use a time limit with none set,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsNoTimeLimitMissionHudItem() {
    forIdx(i, game.curArea->mission.hudItems) {
        MissionHudItem* iPtr = &game.curArea->mission.hudItems[i];
        if(
            iPtr->enabled &&
            iPtr->displayType == MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN &&
            game.curArea->mission.timeLimit == 0
        ) {
            problemType = EPT_NO_TIME_LIMIT_MISSION_HUD_ITEM;
            problemTitle =
                "Mission HUD item needs a time limit that isn't set!";
            problemDescription =
                "Mission HUD item #" + i2s(i + 1) + " makes use of "
                "the mission time limit, but this mission does not have "
                "a time limit set. Either set it or remove the HUD item.";
            return;
        }
    }
}


/**
 * @brief Checks for mission score criteria that use a time limit with none set,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsNoTimeLimitMissionScoreCri() {
    forIdx(c, game.curArea->mission.scoreCriteria) {
        MissionScoreCriterion* cPtr = &game.curArea->mission.scoreCriteria[c];
        if(
            cPtr->metricType == MISSION_METRIC_SECS_LEFT &&
            game.curArea->mission.timeLimit == 0
        ) {
            problemType = EPT_NO_TIME_LIMIT_MISSION_SCORE_CRI;
            problemTitle =
                "Mission score criterion needs a time limit that isn't set!";
            problemDescription =
                "Mission score criterion #" + i2s(c + 1) + " makes use of "
                "the mission time limit, but this mission does not have "
                "a time limit set. Either set it or remove the criterion.";
            return;
        }
    }
}


/**
 * @brief Checks for any non-simple sectors in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsNonSimpleSector() {
    if(!game.curArea->problems.nonSimples.empty()) {
        problemType = EPT_BAD_SECTOR;
        problemTitle = "Non-simple sector!";
        switch(game.curArea->problems.nonSimples.begin()->second) {
        case TRIANGULATION_ERROR_LONE_EDGES: {
            problemDescription =
                "It contains lone edges. Try clearing them up."
                "Check the included manual if you can't find a way to fix it.";
            break;
        } case TRIANGULATION_ERROR_NOT_CLOSED: {
            problemDescription =
                "It is not closed. Try closing it."
                "Check the included manual if you can't find a way to fix it.";
            break;
        } case TRIANGULATION_ERROR_NO_EARS: {
            problemDescription =
                "There's been a triangulation error."
                "Check the included manual if you can't find a way to fix it.";
            break;
        } case TRIANGULATION_ERROR_INVALID_ARGS: {
            problemDescription =
                "An unknown error has occurred with the sector."
                "Check the included manual if you can't find a way to fix it.";
            break;
        } case TRIANGULATION_ERROR_NONE: {
            problemDescription.clear();
            break;
        }
        }
    }
}


/**
 * @brief Checks for any objects out of bounds in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsOobMob() {
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        if(!getSector(mPtr->pos, nullptr, false)) {
            problemMobPtr = mPtr;
            problemType = EPT_MOB_OOB;
            problemTitle = "Mob out of bounds!";
            problemDescription =
                "Move it to somewhere inside the area's geometry.";
            return;
        }
    }
}


/**
 * @brief Checks for any out of bounds path stops in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsOobPathStop() {
    forIdx(s, game.curArea->pathStops) {
        PathStop* sPtr = game.curArea->pathStops[s];
        if(!getSector(sPtr->pos, nullptr, false)) {
            problemPathStopPtr = sPtr;
            problemType = EPT_PATH_STOP_OOB;
            problemTitle = "Path stop out of bounds!";
            problemDescription =
                "Move it to somewhere inside the area's geometry.";
            return;
        }
    }
}


/**
 * @brief Checks for any overlapping vertexes in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsOverlappingVertex() {
    forIdx(v, game.curArea->vertexes) {
        Vertex* v1Ptr = game.curArea->vertexes[v];
        
        for(size_t v2 = v + 1; v2 < game.curArea->vertexes.size(); v2++) {
            Vertex* v2Ptr = game.curArea->vertexes[v2];
            
            if(v1Ptr->x == v2Ptr->x && v1Ptr->y == v2Ptr->y) {
                problemVertexPtr = v1Ptr;
                problemType = EPT_OVERLAPPING_VERTEXES;
                problemTitle = "Overlapping vertexes!";
                problemDescription =
                    "They are very close together at (" +
                    f2s(problemVertexPtr->x) + "," +
                    f2s(problemVertexPtr->y) + "), and should likely "
                    "be merged together.";
                return;
            }
        }
    }
}


/**
 * @brief Checks for any path stop on top of an unrelated link in the area, and
 * fills the problem info if so.
 */
void AreaEditor::findProblemsPathStopOnLink() {
    forIdx(s, game.curArea->pathStops) {
        PathStop* sPtr = game.curArea->pathStops[s];
        forIdx(s2, game.curArea->pathStops) {
            PathStop* linkStartPtr = game.curArea->pathStops[s2];
            if(linkStartPtr == sPtr) continue;
            
            forIdx(l, linkStartPtr->links) {
                PathStop* linkEndPtr = linkStartPtr->links[l]->endPtr;
                if(linkEndPtr == sPtr) continue;
                
                if(
                    circleIntersectsLineSeg(
                        sPtr->pos, sPtr->radius,
                        linkStartPtr->pos, linkEndPtr->pos
                    )
                ) {
                    problemPathStopPtr = sPtr;
                    problemType = EPT_PATH_STOP_ON_LINK;
                    problemTitle = "Path stop on unrelated link!";
                    problemDescription =
                        "This path stop is on top of a link that has nothing "
                        "to do with it. If you meant to connect the two, do "
                        "so now. Otherwise, move the path stop a bit away from "
                        "the link so that they're not so deceptively close.";
                    return;
                }
            }
        }
    }
}


/**
 * @brief Checks for any path stops intersecting in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsPathStopsIntersecting() {
    forIdx(s, game.curArea->pathStops) {
        PathStop* sPtr = game.curArea->pathStops[s];
        forIdx(s2, game.curArea->pathStops) {
            PathStop* s2Ptr = game.curArea->pathStops[s2];
            if(s2Ptr == sPtr) continue;
            
            if(Distance(sPtr->pos, s2Ptr->pos) <= 3.0) {
                problemPathStopPtr = sPtr;
                problemType = EPT_PATH_STOPS_TOGETHER;
                problemTitle = "Two close path stops!";
                problemDescription =
                    "These two are very close together. Separate them.";
                return;
            }
        }
    }
}


/**
 * @brief Checks for any Pikmin over the limit in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsPikminOverLimit() {
    size_t nPikminMobs = 0;
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        if(mPtr->type->category->id == MOB_CATEGORY_PIKMIN) {
            nPikminMobs++;
            if(nPikminMobs > game.curArea->getMaxPikminInField()) {
                problemType = EPT_PIKMIN_OVER_LIMIT;
                problemTitle = "Over the Pikmin limit!";
                problemDescription =
                    "There are more Pikmin in the area than the limit allows. "
                    "This means some of them will not appear. Current limit: "
                    + i2s(game.curArea->getMaxPikminInField()) + ".";
                return;
            }
        }
    }
}


/**
 * @brief Checks for any mobs without a type in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsTypelessMob() {
    forIdx(m, game.curArea->mobGenerators) {
        if(!game.curArea->mobGenerators[m]->type) {
            problemMobPtr = game.curArea->mobGenerators[m];
            problemType = EPT_TYPELESS_MOB;
            problemTitle = "Mob with no type!";
            problemDescription =
                "It has an invalid category or type set. "
                "Give it a proper type or delete it.";
            return;
        }
    }
}


/**
 * @brief Checks for any unknown texture in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsUnknownTexture() {
    forIdx(s, game.curArea->sectors) {
        Sector* sPtr = game.curArea->sectors[s];
        if(sPtr->edges.empty()) continue;
        if(sPtr->isBottomlessPit) continue;
        
        if(sPtr->textureInfo.bmpName.empty()) continue;
        
        if(
            !isInMap(game.content.bitmaps.manifests, sPtr->textureInfo.bmpName)
        ) {
            problemSectorPtr = sPtr;
            problemType = EPT_UNKNOWN_TEXTURE;
            problemTitle = "Sector with unknown texture!";
            problemDescription =
                "Texture name: \"" + sPtr->textureInfo.bmpName + "\".";
            return;
        }
    }
}


/**
 * @brief Checks for any unknown tree shadow texture in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsUnknownTreeShadow() {
    forIdx(s, game.curArea->treeShadows) {
        if(game.curArea->treeShadows[s]->bitmap == game.bmpError) {
            problemShadowPtr = game.curArea->treeShadows[s];
            problemType = EPT_UNKNOWN_SHADOW;
            problemTitle = "Tree shadow with invalid texture!";
            problemDescription =
                "Texture name: \"" +
                game.curArea->treeShadows[s]->bmpName + "\".";
            return;
        }
    }
}


/**
 * @brief Checks for mission score criteria that have a zero point multipler,
 * and fills the problem info if so.
 */
void AreaEditor::findProblemsZeroPointMissionScoreCri() {
    forIdx(c, game.curArea->mission.scoreCriteria) {
        MissionScoreCriterion* cPtr = &game.curArea->mission.scoreCriteria[c];
        if(cPtr->points == 0) {
            problemType = EPT_ZERO_POINT_MISSION_SCORE_CRI;
            problemTitle =
                "Mission score criterion with zero points!";
            problemDescription =
                "Mission score criterion #" + i2s(c + 1) + " gives 0 "
                "points, making it useless. Either make it give the player "
                "some points or remove it.";
            return;
        }
    }
}


/**
 * @brief Adds to the list all sectors affected by the specified sector.
 * The list can include the nullptr sector, and will include the
 * provided sector too.
 *
 * @param sPtr Sector that's affecting others.
 * @param list The list of affected sectors to fill out.
 */
void AreaEditor::getAffectedSectors(
    Sector* sPtr, unordered_set<Sector*>& list
) const {
    forIdx(e, sPtr->edges) {
        list.insert(sPtr->edges[e]->sectors[0]);
        list.insert(sPtr->edges[e]->sectors[1]);
    }
}


/**
 * @brief Adds to the list all sectors affected by the specified sectors.
 * The list can include the nullptr sector, and will include the
 * provided sectors too.
 *
 * @param sectors Sectors that are affecting others.
 * @param list The list of affected sectors to fill out.
 */
void AreaEditor::getAffectedSectors(
    const set<Sector*>& sectors, unordered_set<Sector*>& list
) const {
    for(auto& s : sectors) {
        getAffectedSectors(s, list);
    }
}


/**
 * @brief Adds to the list all sectors affected by the specified vertexes.
 * The list can include the nullptr sector.
 *
 * @param vertexIdxs Indexes of the vertexes that are affecting sectors.
 * @param list The list of affected sectors to fill out.
 */
void AreaEditor::getAffectedSectors(
    const set<size_t>& vertexIdxs, unordered_set<Sector*>& list
) const {
    for(size_t vIdx : vertexIdxs) {
        Vertex* vPtr = game.curArea->vertexes[vIdx];
        forIdx(e, vPtr->edges) {
            list.insert(vPtr->edges[e]->sectors[0]);
            list.insert(vPtr->edges[e]->sectors[1]);
        }
    }
}


/**
 * @brief For a given vertex, returns the edge closest to the given angle,
 * in the given direction.
 *
 * @param vPtr Pointer to the vertex.
 * @param angle Angle coming into the vertex.
 * @param clockwise Return the closest edge clockwise?
 * @param outClosestEdgeAngle If not nullptr, the angle the edge makes
 * into its other vertex is returned here.
 * @return The closest edge.
 */
Edge* AreaEditor::getClosestEdgeToAngle(
    Vertex* vPtr, float angle, bool clockwise,
    float* outClosestEdgeAngle
) const {
    Edge* bestEdge = nullptr;
    float bestAngleDiff = 0;
    float bestEdgeAngle = 0;
    
    forIdx(e, vPtr->edges) {
        Edge* ePtr = vPtr->edges[e];
        Vertex* otherVPtr = ePtr->getOtherVertex(vPtr);
        
        float a = getAngle(v2p(vPtr), v2p(otherVPtr));
        float diff = getAngleCwDiff(angle, a);
        
        if(
            !bestEdge ||
            (clockwise && diff < bestAngleDiff) ||
            (!clockwise && diff > bestAngleDiff)
        ) {
            bestEdge = ePtr;
            bestAngleDiff = diff;
            bestEdgeAngle = a;
        }
    }
    
    if(outClosestEdgeAngle) {
        *outClosestEdgeAngle = bestEdgeAngle;
    }
    return bestEdge;
}


/**
 * @brief Returns a sector common to all vertexes and edges.
 * A sector is considered this if a vertex has it as a sector of
 * a neighboring edge, or if a vertex is inside it.
 * Use the former for vertexes that will be merged, and the latter
 * for vertexes that won't.
 *
 * @param vertexes List of vertexes to check.
 * @param edges List of edges to check.
 * @param result Returns the common sector here.
 * @return Whether there is a common sector.
 */
bool AreaEditor::getCommonSector(
    vector<Vertex*>& vertexes, vector<Edge*>& edges, Sector** result
) const {
    unordered_set<Sector*> sectors;
    
    //First, populate the list of common sectors with a sample.
    //Let's use the first vertex or edge's sectors.
    if(!vertexes.empty()) {
        forIdx(e, vertexes[0]->edges) {
            sectors.insert(vertexes[0]->edges[e]->sectors[0]);
            sectors.insert(vertexes[0]->edges[e]->sectors[1]);
        }
    } else {
        sectors.insert(edges[0]->sectors[0]);
        sectors.insert(edges[0]->sectors[1]);
    }
    
    //Then, check each vertex, and if a sector isn't present in that
    //vertex's list, then it's not a common one, so delete the sector
    //from the list of commons.
    forIdx(v, vertexes) {
        Vertex* vPtr = vertexes[v];
        for(auto s = sectors.begin(); s != sectors.end();) {
            bool foundS = false;
            
            forIdx(e, vPtr->edges) {
                if(
                    vPtr->edges[e]->sectors[0] == *s ||
                    vPtr->edges[e]->sectors[1] == *s
                ) {
                    foundS = true;
                    break;
                }
            }
            
            if(!foundS) {
                sectors.erase(s++);
            } else {
                ++s;
            }
        }
    }
    
    //Now repeat for each edge.
    forIdx(e, edges) {
        Edge* ePtr = edges[e];
        for(auto s = sectors.begin(); s != sectors.end();) {
            if(ePtr->sectors[0] != *s && ePtr->sectors[1] != *s) {
                sectors.erase(s++);
            } else {
                ++s;
            }
        }
    }
    
    if(sectors.empty()) {
        *result = nullptr;
        return false;
    } else if(sectors.size() == 1) {
        *result = *sectors.begin();
        return true;
    }
    
    //Uh-oh...there's no clear answer. We'll have to decide between the
    //involved sectors. Get the rightmost vertexes of all involved sectors.
    //The one most to the left wins.
    //Why? Imagine you're making a triangle inside a square, which is in turn
    //inside another square. The triangle's points share both the inner and
    //outer square sectors. The triangle "belongs" to the inner sector,
    //and we can easily find out which is the inner one with this method.
    float bestRightmostX = 0;
    Sector* bestRightmostSector = nullptr;
    for(auto& s : sectors) {
        if(s == nullptr) continue;
        Vertex* vPtr = s->getRightmostVertex();
        if(!bestRightmostSector || vPtr->x < bestRightmostX) {
            bestRightmostSector = s;
            bestRightmostX = vPtr->x;
        }
    }
    
    *result = bestRightmostSector;
    return true;
}


/**
 * @brief After an edge split, some vertexes could've wanted to merge with the
 * original edge, but may now need to merge with the NEW edge.
 * This function can check which is the "correct" edge to point to, from
 * the two provided.
 *
 * @param vPtr Vertex that caused a split.
 * @param e1Ptr First edge resulting of the split.
 * @param e2Ptr Second edge resulting of the split.
 * @return The correct edge.
 */
Edge* AreaEditor::getCorrectPostSplitEdge(
    const Vertex* vPtr, Edge* e1Ptr, Edge* e2Ptr
) const {
    float score1 = 0;
    float score2 = 0;
    getClosestPointInLineSeg(
        v2p(e1Ptr->vertexes[0]), v2p(e1Ptr->vertexes[1]),
        v2p(vPtr), &score1
    );
    getClosestPointInLineSeg(
        v2p(e2Ptr->vertexes[0]), v2p(e2Ptr->vertexes[1]),
        v2p(vPtr), &score2
    );
    if(fabs(score1 - 0.5) < fabs(score2 - 0.5)) {
        return e1Ptr;
    } else {
        return e2Ptr;
    }
}


/**
 * @brief Returns true if the drawing has an outer sector it belongs to,
 * even if the sector is the void, or false if something's gone wrong.
 *
 * @param result The outer sector, if any, is returned here.
 * @return Whether it succeeded.
 */
bool AreaEditor::getDrawingOuterSector(Sector** result) const {
    //Start by checking if there's a node on a sector. If so, that's it!
    forIdx(n, drawingNodes) {
        if(!drawingNodes[n].onVertex && !drawingNodes[n].onEdge) {
            (*result) = drawingNodes[n].onSector;
            return true;
        }
    }
    
    //If none are on sectors, let's try the following:
    //Grab the first line that is not on top of an existing one,
    //and find the sector that line is on by checking its center.
    forIdx(n, drawingNodes) {
        const LayoutDrawingNode* n1 =
            &drawingNodes[n];
        const LayoutDrawingNode* n2 =
            &(drawingNodes[(n + 1) % drawingNodes.size()]);
        if(!areNodesTraversable(*n1, *n2)) {
            *result =
                getSector(
                    (n1->snappedSpot + n2->snappedSpot) / 2,
                    nullptr, false
                );
            return true;
        }
    }
    
    //If we couldn't find the outer sector that easily,
    //let's try a different approach: check which sector is common
    //to all vertexes and edges.
    vector<Vertex*> v;
    vector<Edge*> e;
    forIdx(n, drawingNodes) {
        if(drawingNodes[n].onVertex) {
            v.push_back(drawingNodes[n].onVertex);
        } else if(drawingNodes[n].onEdge) {
            e.push_back(drawingNodes[n].onEdge);
        }
    }
    return getCommonSector(v, e, result);
}


/**
 * @brief Returns the edge currently under the specified point,
 * or nullptr if none.
 *
 * @param p The point.
 * @param after Only check edges that come after this one.
 * @return The edge.
 */
Edge* AreaEditor::getEdgeUnderPoint(
    const Point& p, const Edge* after
) const {
    bool foundAfter = (!after ? true : false);
    
    forIdx(e, game.curArea->edges) {
        Edge* ePtr = game.curArea->edges[e];
        if(ePtr == after) {
            foundAfter = true;
            continue;
        } else if(!foundAfter) {
            continue;
        }
        
        if(!ePtr->isValid()) continue;
        
        if(
            circleIntersectsLineSeg(
                p, 8 / game.editorsView.cam.zoom,
                v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1])
            )
        ) {
            return ePtr;
        }
    }
    
    return nullptr;
}


/**
 * @brief Returns which edges are crossing against other edges, if any.
 *
 * @return The edges.
 */
vector<EdgeIntersection> AreaEditor::getIntersectingEdges() const {
    vector<EdgeIntersection> intersections;
    
    forIdx(e1, game.curArea->edges) {
        Edge* e1Ptr = game.curArea->edges[e1];
        for(size_t e2 = e1 + 1; e2 < game.curArea->edges.size(); e2++) {
            Edge* e2Ptr = game.curArea->edges[e2];
            if(e1Ptr->hasNeighbor(e2Ptr)) continue;
            if(
                lineSegsIntersect(
                    v2p(e1Ptr->vertexes[0]), v2p(e1Ptr->vertexes[1]),
                    v2p(e2Ptr->vertexes[0]), v2p(e2Ptr->vertexes[1]),
                    nullptr, nullptr
                )
            ) {
                intersections.push_back(EdgeIntersection(e1Ptr, e2Ptr));
            }
        }
    }
    return intersections;
}


/**
 * @brief Returns the radius of the specific mob generator.
 * Normally, this returns the type's radius, but if the type/radius is invalid,
 * it returns a default.
 *
 * @param m The mob to get the radius of.
 * @return The radius or the default.
 */
float AreaEditor::getMobGenRadius(MobGen* m) const {
    return m->type ? m->type->radius == 0 ? 16 : m->type->radius : 16;
}


/**
 * @brief Returns true if there are path links currently under the specified
 * point. data1 takes the info of the found link. If there's also a link in
 * the opposite direction, data2 gets that data, otherwise data2 gets filled
 * with nullptrs.
 *
 * @param p The point to check against.
 * @param data1 If there is a link under the point, its data is returned here.
 * @param data2 If there is a link under the point going in the opposite
 * direction of the previous link, its data is returned here.
 * @return Whether there are links under the point.
 */
bool AreaEditor::getMobLinkUnderPoint(
    const Point& p,
    std::pair<MobGen*, MobGen*>* data1, std::pair<MobGen*, MobGen*>* data2
) const {
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        forIdx(l, mPtr->links) {
            MobGen* m2Ptr = mPtr->links[l];
            if(
                circleIntersectsLineSeg(
                    p, 8 / game.editorsView.cam.zoom, mPtr->pos, m2Ptr->pos
                )
            ) {
                *data1 = std::make_pair(mPtr, m2Ptr);
                *data2 = std::make_pair((MobGen*) nullptr, (MobGen*) nullptr);
                
                forIdx(l2, m2Ptr->links) {
                    if(m2Ptr->links[l2] == mPtr) {
                        *data2 = std::make_pair(m2Ptr, mPtr);
                        break;
                    }
                }
                return true;
            }
        }
    }
    
    return false;
}


/**
 * @brief Returns the mob currently under the specified point,
 * or nullptr if none.
 *
 * @param p The point to check against.
 * @param outIdx If not nullptr, the mob index is returned here.
 * If no mob matches, INVALID is returned instead.
 * @return The mob.
 */
MobGen* AreaEditor::getMobUnderPoint(const Point& p, size_t* outIdx) const {
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        
        if(
            Distance(mPtr->pos, p) <= getMobGenRadius(mPtr)
        ) {
            if(outIdx) *outIdx = m;
            return mPtr;
        }
    }
    
    if(outIdx) *outIdx = INVALID;
    return nullptr;
}


/**
 * @brief Returns the editor path link currently under a point.
 *
 * @param p The point to check against.
 * @return The link.
 */
EditorPathLink* AreaEditor::getEditorPathLinkUnderPoint(const Point& p) const {
    forIdx(l, game.curArea->editorPathLinks) {
        EditorPathLink* elPtr = &game.curArea->editorPathLinks[l];
        PathStop* s1Ptr = elPtr->link1->startPtr;
        PathStop* s2Ptr = elPtr->link1->endPtr;
        if(
            circleIntersectsLineSeg(
                p, 8.0f / game.editorsView.cam.zoom, s1Ptr->pos, s2Ptr->pos
            )
        ) {
            return elPtr;
        }
    }
    
    return nullptr;
}


/**
 * @brief Returns the path stop currently under the specified point,
 * or nullptr if none.
 *
 * @param p Point to check against.
 * @return The stop.
 */
PathStop* AreaEditor::getPathStopUnderPoint(const Point& p) const {
    forIdx(s, game.curArea->pathStops) {
        PathStop* sPtr = game.curArea->pathStops[s];
        
        if(Distance(sPtr->pos, p) <= sPtr->radius) {
            return sPtr;
        }
    }
    
    return nullptr;
}


/**
 * @brief Returns the sector currently under the specified point,
 * or nullptr if none.
 *
 * @param p Point to check against.
 * @return The sector.
 */
Sector* AreaEditor::getSectorUnderPoint(const Point& p) const {
    return getSector(p, nullptr, false);
}


/**
 * @brief Returns the vertex currently under the specified point,
 * or nullptr if none.
 *
 * @param p Point to check against.
 * @return The vertex.
 */
Vertex* AreaEditor::getVertexUnderPoint(const Point& p) const {
    forIdx(v, game.curArea->vertexes) {
        Vertex* vPtr = game.curArea->vertexes[v];
        
        if(
            rectanglesIntersect(
                p - (4 / game.editorsView.cam.zoom),
                p + (4 / game.editorsView.cam.zoom),
                Point(
                    vPtr->x - (4 / game.editorsView.cam.zoom),
                    vPtr->y - (4 / game.editorsView.cam.zoom)
                ),
                Point(
                    vPtr->x + (4 / game.editorsView.cam.zoom),
                    vPtr->y + (4 / game.editorsView.cam.zoom)
                )
            )
        ) {
            return vPtr;
        }
    }
    
    return nullptr;
}


/**
 * @brief Homogenizes all selected edges,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedEdges() {
    if(edgeSelection.getCount() < 2) return;
    
    const set<size_t>& selectedEdges = edgeSelection.getItemIdxs();
    Edge* base = game.curArea->edges[*selectedEdges.begin()];
    for(size_t eIdx : selectedEdges) {
        Edge* ePtr = game.curArea->edges[eIdx];
        if(ePtr == base) continue;
        base->clone(ePtr);
    }
}


/**
 * @brief Homogenizes all selected mobs,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedMobs() {
    if(mobSelection.getCount() < 2) return;
    
    const set<size_t>& selectedMobs = mobSelection.getItemIdxs();
    MobGen* base = game.curArea->mobGenerators[*selectedMobs.begin()];
    for(size_t mIdx : selectedMobs) {
        MobGen* mPtr = game.curArea->mobGenerators[mIdx];
        if(mPtr == base) continue;
        base->clone(mPtr, false);
    }
}


/**
 * @brief Homogenizes all selected path links,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedPathLinks() {
    if(pathLinkSelection.getCount() < 2) return;
    
    const set<size_t>& selectedLinks = pathLinkSelection.getItemIdxs();
    EditorPathLink* base =
        &game.curArea->editorPathLinks[*selectedLinks.begin()];
    for(size_t elIdx : selectedLinks) {
        EditorPathLink* elPtr = &game.curArea->editorPathLinks[elIdx];
        if(elPtr == base) continue;
        base->link1->clone(elPtr->link1);
        if(elPtr->link2) base->link1->clone(elPtr->link2);
    }
}


/**
 * @brief Homogenizes all selected path stops,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedPathStops() {
    if(pathStopSelection.getCount() < 2) return;
    
    const set<size_t>& selectedStops = pathStopSelection.getItemIdxs();
    PathStop* base = game.curArea->pathStops[*selectedStops.begin()];
    for(size_t sIdx : selectedStops) {
        PathStop* sPtr = game.curArea->pathStops[sIdx];
        if(sPtr == base) continue;
        base->clone(sPtr);
    }
}


/**
 * @brief Homogenizes all selected sectors,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedSectors() {
    if(sectorSelection.getCount() < 2) return;
    
    const set<size_t>& selectedSectors = sectorSelection.getItemIdxs();
    Sector* base = game.curArea->sectors[*selectedSectors.begin()];
    for(size_t sIdx : selectedSectors) {
        Sector* sPtr = game.curArea->sectors[sIdx];
        if(sPtr == base) continue;
        base->clone(sPtr);
        updateSectorTexture(sPtr, base->textureInfo.bmpName);
    }
}


/**
 * @brief Merges two neighboring sectors into one. The final sector will
 * be the largest of the two.
 *
 * @param s1 First sector to merge.
 * @param s2 Second sector to merge.
 * @return Whether it was successful.
 */
bool AreaEditor::mergeSectors(Sector* s1, Sector* s2) {
    //Of the two sectors, figure out which is the largest.
    Sector* mainSector = s1;
    Sector* smallSector = s2;
    if(!s2) {
        mainSector = s2;
        smallSector = s1;
    } else if(s1) {
        float s1Area =
            (s1->bbox[1].x - s1->bbox[0].x) *
            (s1->bbox[1].y - s1->bbox[0].y);
        float s2Area =
            (s2->bbox[1].x - s2->bbox[0].x) *
            (s2->bbox[1].y - s2->bbox[0].y);
        if(s1Area < s2Area) {
            mainSector = s2;
            smallSector = s1;
        }
    }
    
    //For all of the smaller sector's edges, either mark them
    //as edges to transfer to the large sector, or edges
    //to delete (because they'd just end up having the larger sector on
    //both sides).
    unordered_set<Edge*> commonEdges;
    unordered_set<Edge*> edgesToTransfer;
    
    forIdx(e, smallSector->edges) {
        Edge* ePtr = smallSector->edges[e];
        if(ePtr->getOtherSector(smallSector) == mainSector) {
            commonEdges.insert(ePtr);
        } else {
            edgesToTransfer.insert(ePtr);
        }
    }
    
    //However, if there are no common edges between sectors,
    //this operation is invalid.
    if(commonEdges.empty()) {
        setStatus("Those two sectors are not neighbors!", true);
        return false;
    }
    
    //Before doing anything, get the list of sectors that will be affected.
    unordered_set<Sector*> affectedSectors;
    getAffectedSectors(smallSector, affectedSectors);
    if(mainSector) getAffectedSectors(mainSector, affectedSectors);
    
    //Transfer edges that need transferal.
    for(Edge* ePtr : edgesToTransfer) {
        ePtr->transferSector(
            smallSector, mainSector,
            mainSector ?
            game.curArea->findSectorIdx(mainSector) :
            INVALID,
            game.curArea->findEdgeIdx(ePtr)
        );
    }
    
    //Delete the other ones.
    for(Edge* ePtr : commonEdges) {
        deleteEdge(ePtr);
    }
    
    //Delete the now-merged sector.
    game.curArea->deleteSector(smallSector);
    
    //Update all affected sectors.
    affectedSectors.erase(smallSector);
    updateAffectedSectors(affectedSectors);
    
    return true;
}


/**
 * @brief Merges vertex 1 into vertex 2.
 *
 * @param v1 Vertex that is being moved and will be merged.
 * @param v2 Vertex that is going to absorb v1.
 * @param affectedSectors List of sectors that will be affected by this merge.
 */
void AreaEditor::mergeVertex(
    const Vertex* v1, Vertex* v2, unordered_set<Sector*>* affectedSectors
) {
    vector<Edge*> edges = v1->edges;
    //Find out what to do with every edge of the dragged vertex.
    forIdx(e, edges) {
    
        Edge* ePtr = edges[e];
        Vertex* otherVertex = ePtr->getOtherVertex(v1);
        
        if(otherVertex == v2) {
        
            //Squashed into non-existence.
            affectedSectors->insert(ePtr->sectors[0]);
            affectedSectors->insert(ePtr->sectors[1]);
            
            //Delete it.
            deleteEdge(ePtr);
            
        } else {
        
            bool hasMerged = false;
            //Check if the edge will be merged with another one.
            //These are edges that share a common vertex,
            //plus the moved/destination vertex.
            forIdx(de, v2->edges) {
            
                Edge* dePtr = v2->edges[de];
                Vertex* dOtherVertex = dePtr->getOtherVertex(v2);
                
                if(dOtherVertex == otherVertex) {
                    //The edge will be merged with this one.
                    hasMerged = true;
                    affectedSectors->insert(ePtr->sectors[0]);
                    affectedSectors->insert(ePtr->sectors[1]);
                    affectedSectors->insert(dePtr->sectors[0]);
                    affectedSectors->insert(dePtr->sectors[1]);
                    
                    //Set the new sectors.
                    if(ePtr->sectors[0] == dePtr->sectors[0]) {
                        game.curArea->connectEdgeToSector(
                            dePtr, ePtr->sectors[1], 0
                        );
                    } else if(ePtr->sectors[0] == dePtr->sectors[1]) {
                        game.curArea->connectEdgeToSector(
                            dePtr, ePtr->sectors[1], 1
                        );
                    } else if(ePtr->sectors[1] == dePtr->sectors[0]) {
                        game.curArea->connectEdgeToSector(
                            dePtr, ePtr->sectors[0], 0
                        );
                    } else if(ePtr->sectors[1] == dePtr->sectors[1]) {
                        game.curArea->connectEdgeToSector(
                            dePtr, ePtr->sectors[0], 1
                        );
                    }
                    
                    //Delete it.
                    deleteEdge(ePtr);
                    
                    break;
                }
            }
            
            //If it's matchless, that means it'll just be joined to
            //the group of edges on the destination vertex.
            if(!hasMerged) {
                game.curArea->connectEdgeToVertex(
                    ePtr, v2, (ePtr->vertexes[0] == v1 ? 0 : 1)
                );
                forIdx(v2e, v2->edges) {
                    affectedSectors->insert(v2->edges[v2e]->sectors[0]);
                    affectedSectors->insert(v2->edges[v2e]->sectors[1]);
                }
            }
        }
        
    }
    
    //Check if any of the final edges have the same sector
    //on both sides. If so, delete them.
    for(size_t ve = 0; ve < v2->edges.size(); ) {
        Edge* vePtr = v2->edges[ve];
        if(vePtr->sectors[0] == vePtr->sectors[1]) {
            deleteEdge(vePtr);
        } else {
            ve++;
        }
    }
    
    //Delete the old vertex.
    game.curArea->deleteVertex(v1);
    
    //If any vertex or sector is out of edges, delete it.
    for(size_t v = 0; v < game.curArea->vertexes.size();) {
        Vertex* vPtr = game.curArea->vertexes[v];
        if(vPtr->edges.empty()) {
            game.curArea->deleteVertex(v);
        } else {
            v++;
        }
    }
    for(size_t s = 0; s < game.curArea->sectors.size();) {
        Sector* sPtr = game.curArea->sectors[s];
        if(sPtr->edges.empty()) {
            game.curArea->deleteSector(s);
        } else {
            s++;
        }
    }
    
}


/**
 * @brief Pastes previously-copied edge properties onto the selected edges.
 */
void AreaEditor::pasteEdgeProperties() {
    if(!copyBufferEdge) {
        setStatus(
            "To paste edge properties, you must first copy them "
            "from another one!",
            true
        );
        return;
    }
    
    if(!edgeSelection.hasAny()) {
        setStatus(
            "To paste edge properties, you must first select which edge "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("edge property paste");
    
    for(size_t eIdx : edgeSelection.getItemIdxs()) {
        copyBufferEdge->clone(game.curArea->edges[eIdx]);
    }
    
    updateAllEdgeOffsetCaches();
    
    setStatus("Successfully pasted edge properties.");
    return;
}


/**
 * @brief Pastes previously-copied mob properties onto the selected mobs.
 */
void AreaEditor::pasteMobProperties() {
    if(!copyBufferMob) {
        setStatus(
            "To paste object properties, you must first copy them "
            "from another one!",
            true
        );
        return;
    }
    
    if(!mobSelection.hasAny()) {
        setStatus(
            "To paste object properties, you must first select which object "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("object property paste");
    
    for(size_t mIdx : mobSelection.getItemIdxs()) {
        copyBufferMob->clone(game.curArea->mobGenerators[mIdx], false);
    }
    
    setStatus("Successfully pasted object properties.");
    return;
}


/**
 * @brief Pastes previously-copied path link properties onto the selected
 * path links.
 */
void AreaEditor::pastePathLinkProperties() {
    if(!copyBufferPathLink) {
        setStatus(
            "To paste path link properties, you must first copy them "
            "from another one!",
            true
        );
        return;
    }
    
    if(!pathLinkSelection.hasAny()) {
        setStatus(
            "To paste path link properties, you must first select which path "
            "link to paste to!",
            true
        );
        return;
    }
    
    registerChange("path link property paste");
    
    for(size_t lIdx : pathLinkSelection.getItemIdxs()) {
        EditorPathLink* lPtr = &game.curArea->editorPathLinks[lIdx];
        copyBufferPathLink->clone(lPtr->link1);
        if(lPtr->link2) copyBufferPathLink->clone(lPtr->link2);
    }
    
    setStatus("Successfully pasted path link properties.");
    return;
}


/**
 * @brief Pastes previously-copied sector properties onto the selected sectors.
 */
void AreaEditor::pasteSectorProperties() {
    if(!copyBufferSector) {
        setStatus(
            "To paste sector properties, you must first copy them "
            "from another one!",
            true
        );
        return;
    }
    
    if(!sectorSelection.hasAny()) {
        setStatus(
            "To paste sector properties, you must first select which sector "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("sector property paste");
    
    for(size_t sIdx : sectorSelection.getItemIdxs()) {
        Sector* sPtr = game.curArea->sectors[sIdx];
        copyBufferSector->clone(sPtr);
        updateSectorTexture(sPtr, copyBufferSector->textureInfo.bmpName);
    }
    
    updateAllEdgeOffsetCaches();
    
    setStatus("Successfully pasted sector properties.");
    return;
}


/**
 * @brief Pastes a previously-copied sector texture onto the selected sectors.
 */
void AreaEditor::pasteSectorTexture() {
    if(!copyBufferSector) {
        setStatus(
            "To paste a sector texture, you must first copy the properties "
            "from another one!",
            true
        );
        return;
    }
    
    if(!sectorSelection.hasAny()) {
        setStatus(
            "To paste a sector texture, you must first select which sector "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("sector texture paste");
    
    for(size_t sIdx : sectorSelection.getItemIdxs()) {
        Sector* sPtr = game.curArea->sectors[sIdx];
        updateSectorTexture(sPtr, copyBufferSector->textureInfo.bmpName);
    }
    
    setStatus("Successfully pasted sector texture.");
    return;
}


/**
 * @brief Resizes all X and Y coordinates by the specified multiplier.
 *
 * @param mults Multiply the coordinates horizontally and vertically by
 * these values.
 */
void AreaEditor::resizeEverything(float mults[2]) {
    forIdx(v, game.curArea->vertexes) {
        Vertex* vPtr = game.curArea->vertexes[v];
        vPtr->x *= mults[0];
        vPtr->y *= mults[1];
    }
    
    forIdx(s, game.curArea->sectors) {
        Sector* sPtr = game.curArea->sectors[s];
        sPtr->textureInfo.tf.scale.x *= mults[0];
        sPtr->textureInfo.tf.scale.y *= mults[1];
        triangulateSector(sPtr, nullptr, false);
        sPtr->calculateBoundingBox();
    }
    
    forIdx(m, game.curArea->mobGenerators) {
        MobGen* mPtr = game.curArea->mobGenerators[m];
        mPtr->pos.x *= mults[0];
        mPtr->pos.y *= mults[1];
    }
    
    forIdx(s, game.curArea->pathStops) {
        PathStop* sPtr = game.curArea->pathStops[s];
        sPtr->pos.x *= mults[0];
        sPtr->pos.y *= mults[1];
    }
    forIdx(s, game.curArea->pathStops) {
        game.curArea->pathStops[s]->calculateDists();
    }
    
    forIdx(s, game.curArea->treeShadows) {
        TreeShadow* sPtr = game.curArea->treeShadows[s];
        sPtr->pose.pos.x *= mults[0];
        sPtr->pose.pos.y *= mults[1];
        sPtr->pose.size.x *= mults[0];
        sPtr->pose.size.y *= mults[1];
        sPtr->sway.x *= mults[0];
        sPtr->sway.y *= mults[1];
    }
    
    forIdx(r, game.curArea->regions) {
        AreaRegion* rPtr = game.curArea->regions[r];
        rPtr->pose.pos.x *= mults[0];
        rPtr->pose.pos.y *= mults[1];
        rPtr->pose.size.x *= mults[0];
        rPtr->pose.size.y *= mults[1];
    }
}


/**
 * @brief Makes all currently selected mob generators (if any) rotate to
 * face where the the given point is.
 *
 * @param pos Point that the mobs must face.
 */
void AreaEditor::rotateMobGensToPoint(const Point& pos) {
    if(!mobSelection.hasAny()) {
        setStatus(
            "To rotate objects, you must first select some objects!",
            true
        );
        return;
    }
    
    registerChange("object rotation");
    mobSelection.setHomogenized(false);
    for(size_t mobIdx : mobSelection.getItemIdxs()) {
        MobGen* mPtr = game.curArea->mobGenerators[mobIdx];
        mPtr->angle = getAngle(mPtr->pos, pos);
    }
    setStatus("Rotated objects to face " + p2s(pos) + ".");
}


/**
 * @brief Snaps a point to the nearest available snapping space, based on the
 * current snap mode, Shift key state, and Ctrl key state.
 *
 * @param p Point to snap.
 * @param ignoreSelected If true, ignore the selected vertexes or edges
 * when snapping to vertexes or edges.
 * @return The snapped point.
 */
Point AreaEditor::snapPoint(const Point& p, bool ignoreSelected) {
    SNAP_MODE modeToUse = game.options.areaEd.snapMode;
    Point finalPoint = p;
    
    if(isShiftPressed) {
        if(game.options.areaEd.snapMode == SNAP_MODE_NOTHING) {
            modeToUse = SNAP_MODE_GRID;
        } else {
            modeToUse = SNAP_MODE_NOTHING;
        }
    }
    
    if(isCtrlPressed) {
        if(curTransformationWidget.isMovingCenterHandle()) {
            finalPoint =
                snapPointToAxis(
                    finalPoint, curTransformationWidget.getOldCenter()
                );
        } else if(moving) {
            finalPoint =
                snapPointToAxis(finalPoint, moveStartPos);
        }
    }
    
    switch(modeToUse) {
    case SNAP_MODE_GRID: {
        return
            snapPointToGrid(
                finalPoint,
                game.options.areaEd.gridInterval
            );
        break;
        
    } case SNAP_MODE_VERTEXES: {
        if(cursorSnapTimer.timeLeft > 0.0f) {
            return cursorSnapCache;
        }
        cursorSnapTimer.start();
        
        vector<Vertex*> vertexesToCheck = game.curArea->vertexes;
        if(ignoreSelected) {
            const set<size_t>& selectedVertexes = vertexSelection.getItemIdxs();
            for(size_t vIdx : selectedVertexes) {
                forIdx(v2, vertexesToCheck) {
                    if(vertexesToCheck[v2] == game.curArea->vertexes[vIdx]) {
                        vertexesToCheck.erase(vertexesToCheck.begin() + v2);
                        break;
                    }
                }
            }
        }
        vector<std::pair<Distance, Vertex*> > snappableVertexes =
            getMergeVertexes(
                finalPoint, vertexesToCheck,
                game.options.areaEd.snapThreshold / game.editorsView.cam.zoom
            );
        if(snappableVertexes.empty()) {
            cursorSnapCache = finalPoint;
            return finalPoint;
        } else {
            sort(
                snappableVertexes.begin(), snappableVertexes.end(),
                [] (
                    std::pair<Distance, Vertex*> v1,
                    std::pair<Distance, Vertex*> v2
            ) -> bool {
                return v1.first < v2.first;
            }
            );
            
            Point result(
                snappableVertexes[0].second->x,
                snappableVertexes[0].second->y
            );
            cursorSnapCache = result;
            return result;
        }
        
        break;
        
    } case SNAP_MODE_EDGES: {
        if(cursorSnapTimer.timeLeft > 0.0f) {
            return cursorSnapCache;
        }
        cursorSnapTimer.start();
        
        Distance closestDist;
        bool gotOne = false;
        
        forIdx(e, game.curArea->edges) {
            Edge* ePtr = game.curArea->edges[e];
            float r;
            
            if(ignoreSelected) {
                //Let's ignore not only the selected edge, but also
                //neighboring edges, because as we move an edge,
                //the neighboring edges stretch along with it.
                bool skip = false;
                const set<size_t>& selectedVertexes =
                    vertexSelection.getItemIdxs();
                for(size_t vIdx : selectedVertexes) {
                    if(game.curArea->vertexes[vIdx]->hasEdge(ePtr)) {
                        skip = true;
                        break;
                    }
                }
                if(skip) continue;
            }
            
            Point edgeP =
                getClosestPointInLineSeg(
                    v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]),
                    finalPoint, &r
                );
                
            if(r < 0.0f) {
                edgeP = v2p(ePtr->vertexes[0]);
            } else if(r > 1.0f) {
                edgeP = v2p(ePtr->vertexes[1]);
            }
            
            Distance d(finalPoint, edgeP);
            if(
                d >
                game.options.areaEd.snapThreshold / game.editorsView.cam.zoom
            ) {
                continue;
            }
            
            if(!gotOne || d < closestDist) {
                gotOne = true;
                closestDist = d;
                finalPoint = edgeP;
            }
        }
        
        cursorSnapCache = finalPoint;
        return finalPoint;
        
        break;
        
    } case SNAP_MODE_NOTHING: {
    } case N_SNAP_MODES: {
        break;
        
    }
    }
    
    return finalPoint;
}


/**
 * @brief Splits an edge into two, near the specified point, and returns the
 * newly-created vertex. The new vertex gets added to the current area.
 *
 * @param ePtr Edge to split.
 * @param where Point to split at.
 * @return The newly-created vertex.
 */
Vertex* AreaEditor::splitEdge(Edge* ePtr, const Point& where) {
    Point newVPos =
        getClosestPointInLineSeg(
            v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]),
            where
        );
        
    //Create the new vertex and the new edge.
    Vertex* newVPtr = game.curArea->addNewVertex();
    newVPtr->x = newVPos.x;
    newVPtr->y = newVPos.y;
    Edge* newEPtr = game.curArea->addNewEdge();
    ePtr->clone(newEPtr);
    
    //Connect the vertexes and edges.
    game.curArea->connectEdgeToVertex(newEPtr, newVPtr, 0);
    game.curArea->connectEdgeToVertex(newEPtr, ePtr->vertexes[1], 1);
    game.curArea->connectEdgeToVertex(ePtr, newVPtr, 1);
    
    //Connect the sectors and new edge.
    if(ePtr->sectors[0]) {
        game.curArea->connectEdgeToSector(
            newEPtr, ePtr->sectors[0], 0
        );
    }
    if(ePtr->sectors[1]) {
        game.curArea->connectEdgeToSector(
            newEPtr, ePtr->sectors[1], 1
        );
    }
    
    updateAllEdgeOffsetCaches();
    
    return newVPtr;
}


/**
 * @brief Splits a path link into two, near the specified point, and returns the
 * newly-created path stop. The new stop gets added to the current area.
 *
 * @param l1 Path link to split.
 * @param l2 If there is also a path link going in the opposite direction
 * between the two stops involved, this contains its data.
 * Otherwise, it contains nullptrs.
 * @param where Where to make the split.
 * @return The newly-created stop.
 */
PathStop* AreaEditor::splitPathLink(
    PathLink* l1, PathLink* l2, const Point& where
) {
    bool normalLink = (l2 != nullptr);
    Point newStopPos =
        getClosestPointInLineSeg(
            l1->startPtr->pos, l1->endPtr->pos,
            where
        );
        
    //Create the new stop.
    PathStop* newStopPtr = new PathStop(newStopPos);
    game.curArea->pathStops.push_back(newStopPtr);
    
    //Delete the old links.
    PathStop* oldStartPtr = l1->startPtr;
    PathStop* oldEndPtr = l1->endPtr;
    PATH_LINK_TYPE oldLinkType = l1->type;
    l1->startPtr->deleteLink(l1->endPtr);
    if(normalLink) {
        l2->startPtr->deleteLink(l2->endPtr);
    }
    
    //Create the new links.
    oldStartPtr->addNewLink(newStopPtr, normalLink);
    newStopPtr->addNewLink(oldEndPtr, normalLink);
    
    //Fix the dangling path stop numbers in the links, and other properties.
    game.curArea->fixPathStopIdxs(oldStartPtr);
    game.curArea->fixPathStopIdxs(oldEndPtr);
    game.curArea->fixPathStopIdxs(newStopPtr);
    
    oldStartPtr->getLink(newStopPtr)->type = oldLinkType;
    newStopPtr->getLink(oldEndPtr)->type = oldLinkType;
    if(normalLink) {
        newStopPtr->getLink(oldStartPtr)->type = oldLinkType;
        oldEndPtr->getLink(newStopPtr)->type = oldLinkType;
    }
    
    //Update the distances and editor links.
    newStopPtr->calculateDistsPlusNeighbors();
    game.curArea->setupEditorPathLinks();
    
    return newStopPtr;
}


/**
 * @brief Updates the triangles and bounding box of the specified sectors, and
 * reports any errors found.
 *
 * @param affectedSectors The list of affected sectors.
 */
void AreaEditor::updateAffectedSectors(
    const unordered_set<Sector*>& affectedSectors
) {
    TRIANGULATION_ERROR lastTriangulationError = TRIANGULATION_ERROR_NONE;
    
    for(Sector* sPtr : affectedSectors) {
        if(!sPtr) continue;
        
        set<Edge*> triangulationLoneEdges;
        TRIANGULATION_ERROR triangulationError =
            triangulateSector(sPtr, &triangulationLoneEdges, true);
            
        if(triangulationError == TRIANGULATION_ERROR_NONE) {
            auto it = game.curArea->problems.nonSimples.find(sPtr);
            if(it != game.curArea->problems.nonSimples.end()) {
                game.curArea->problems.nonSimples.erase(it);
            }
        } else {
            game.curArea->problems.nonSimples[sPtr] =
                triangulationError;
            lastTriangulationError = triangulationError;
        }
        game.curArea->problems.loneEdges.insert(
            triangulationLoneEdges.begin(),
            triangulationLoneEdges.end()
        );
        
        sPtr->calculateBoundingBox();
    }
    
    if(lastTriangulationError != TRIANGULATION_ERROR_NONE) {
        emitTriangulationErrorStatusBarMessage(lastTriangulationError);
    }
    
    updateAllEdgeOffsetCaches();
}


/**
 * @brief When the user creates a new sector, which houses other sectors inside,
 * and these inner sectors need to know their outer sector changed.
 * This will go through a list of edges, check if they are inside
 * the new sector, and if so, update their outer sector.
 *
 * @param edgesToCheck List of edges to check if they belong inside
 * the new sector or not.
 * @param oldOuter What the old outer sector used to be.
 * @param newOuter What the new outer sector is.
 */
void AreaEditor::updateInnerSectorsOuterSector(
    const vector<Edge*>& edgesToCheck,
    const Sector* oldOuter, Sector* newOuter
) {
    forIdx(e, edgesToCheck) {
        Edge* ePtr = edgesToCheck[e];
        Vertex* v1Ptr = ePtr->vertexes[0];
        Vertex* v2Ptr = ePtr->vertexes[1];
        if(
            newOuter->isPointInSector(v2p(v1Ptr)) &&
            newOuter->isPointInSector(v2p(v2Ptr)) &&
            newOuter->isPointInSector(
                Point(
                    (v1Ptr->x + v2Ptr->x) / 2.0f,
                    (v1Ptr->y + v2Ptr->y) / 2.0f
                )
            )
        ) {
            for(size_t s = 0; s < 2; s++) {
                if(ePtr->sectors[s] == oldOuter) {
                    game.curArea->connectEdgeToSector(
                        ePtr, newOuter, s
                    );
                    break;
                }
            }
        }
    }
}
