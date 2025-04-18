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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


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
    const LayoutDrawingNode &n1, const LayoutDrawingNode &n2
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
 * Sets the line's status to drawing_line_result.
 *
 * @param pos Position the user is trying to finish the line on.
 */
void AreaEditor::checkDrawingLine(const Point &pos) {
    drawingLineResult = DRAWING_LINE_RESULT_OK;
    
    if(drawingNodes.empty()) {
        return;
    }
    
    LayoutDrawingNode* prev_node = &drawingNodes.back();
    LayoutDrawingNode tentative_node(this, pos);
    
    //Check if the user hits a vertex or an edge, but the drawing is
    //meant to be a new sector shape.
    if(
        (!drawingNodes[0].onEdge && !drawingNodes[0].onVertex) &&
        (tentative_node.onEdge || tentative_node.onVertex)
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX;
        return;
    }
    
    //Check if it's just hitting the same edge, or vertexes of the same edge.
    if(
        tentative_node.onEdge &&
        tentative_node.onEdge == prev_node->onEdge
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    if(
        tentative_node.onVertex &&
        tentative_node.onVertex->hasEdge(prev_node->onEdge)
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    if(
        prev_node->onVertex &&
        prev_node->onVertex->hasEdge(tentative_node.onEdge)
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    if(
        tentative_node.onVertex &&
        tentative_node.onVertex->isNeighbor(prev_node->onVertex)
    ) {
        drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    
    //Check for edge collisions in collinear lines.
    for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
        //We don't need to watch out for the edge of the current point
        //or the previous one, since this collinearity check doesn't
        //return true for line segments that touch in only one point.
        Edge* e_ptr = game.curAreaData->edges[e];
        Point ep1 = v2p(e_ptr->vertexes[0]);
        Point ep2 = v2p(e_ptr->vertexes[1]);
        
        if(
            lineSegsAreCollinear(prev_node->snappedSpot, pos, ep1, ep2)
        ) {
            if(
                collinearLineSegsIntersect(
                    prev_node->snappedSpot, pos, ep1, ep2
                )
            ) {
                drawingLineResult = DRAWING_LINE_RESULT_ALONG_EDGE;
                return;
            }
        }
    }
    
    //Check for edge collisions.
    for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
        Edge* e_ptr = game.curAreaData->edges[e];
        //If this edge is the same or a neighbor of the previous node,
        //then never mind.
        if(
            prev_node->onEdge == e_ptr ||
            tentative_node.onEdge == e_ptr
        ) {
            continue;
        }
        if(prev_node->onVertex) {
            if(
                e_ptr->vertexes[0] == prev_node->onVertex ||
                e_ptr->vertexes[1] == prev_node->onVertex
            ) {
                continue;
            }
        }
        if(tentative_node.onVertex) {
            if(
                e_ptr->vertexes[0] == tentative_node.onVertex ||
                e_ptr->vertexes[1] == tentative_node.onVertex
            ) {
                continue;
            }
        }
        
        if(
            lineSegsIntersect(
                prev_node->snappedSpot, pos,
                v2p(e_ptr->vertexes[0]), v2p(e_ptr->vertexes[1]),
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
            LayoutDrawingNode* n1_ptr = &drawingNodes[n];
            LayoutDrawingNode* n2_ptr = &drawingNodes[n + 1];
            Point intersection;
            if(
                lineSegsIntersect(
                    prev_node->snappedSpot, pos,
                    n1_ptr->snappedSpot, n2_ptr->snappedSpot,
                    &intersection
                )
            ) {
                if(
                    Distance(intersection, drawingNodes.begin()->snappedSpot) >
                    AREA_EDITOR::VERTEX_MERGE_RADIUS / game.view.cam.zoom
                ) {
                    //Only a problem if this isn't the user's drawing finish.
                    drawingLineResult = DRAWING_LINE_RESULT_CROSSES_DRAWING;
                    return;
                }
            }
        }
        
        if(
            circleIntersectsLineSeg(
                pos, 8.0 / game.view.cam.zoom,
                prev_node->snappedSpot,
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
        Point working_sector_point(
            (
                drawingNodes[0].snappedSpot.x +
                drawingNodes[1].snappedSpot.x
            ) / 2.0f,
            (
                drawingNodes[0].snappedSpot.y +
                drawingNodes[1].snappedSpot.y
            ) / 2.0f
        );
        Sector* working_sector = getSectorUnderPoint(working_sector_point);
        
        Point latest_sector_point(
            (
                drawingNodes.back().snappedSpot.x +
                pos.x
            ) / 2.0f,
            (
                drawingNodes.back().snappedSpot.y +
                pos.y
            ) / 2.0f
        );
        Sector* latest_sector = getSectorUnderPoint(latest_sector_point);
        
        if(latest_sector != working_sector) {
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
    if(selectedEdges.empty()) {
        setStatus(
            "To copy an edge's properties, you must first select an edge "
            "to copy from!",
            true
        );
        return;
    }
    
    if(selectedEdges.size() > 1) {
        setStatus(
            "To copy an edge's properties, you can only select 1 edge!",
            true
        );
        return;
    }
    
    Edge* source_edge = *selectedEdges.begin();
    if(!copyBufferEdge) {
        copyBufferEdge = new Edge();
    }
    source_edge->clone(copyBufferEdge);
    setStatus("Successfully copied the edge's properties.");
    return;
}


/**
 * @brief Copies the currently selected mob's properties onto the copy buffer,
 * so they can be then pasted onto another mob.
 */
void AreaEditor::copyMobProperties() {
    if(selectedMobs.empty()) {
        setStatus(
            "To copy an object's properties, you must first select an object "
            "to copy from!",
            true
        );
        return;
    }
    
    if(selectedMobs.size() > 1) {
        setStatus(
            "To copy an object's properties, you can only select 1 object!",
            true
        );
        return;
    }
    
    MobGen* source_mob = *selectedMobs.begin();
    if(!copyBufferMob) {
        copyBufferMob = new MobGen();
    }
    source_mob->clone(copyBufferMob);
    setStatus("Successfully copied the object's properties.");
    return;
}


/**
 * @brief Copies the currently selected path link's properties onto the
 * copy buffer, so they can be then pasted onto another path link.
 *
 */
void AreaEditor::copyPathLinkProperties() {
    if(selectedPathLinks.empty()) {
        setStatus(
            "To copy a path link's properties, you must first select a path "
            "link to copy from!",
            true
        );
        return;
    }
    
    size_t really_selected_nr = selectedPathLinks.size();
    if(really_selected_nr == 2) {
        //Check if these are just the two sides of the same two-way link.
        //If so then yeah, we basically only have one link really selected.
        PathLink* l_ptr = *selectedPathLinks.begin();
        if(!l_ptr->isOneWay()) {
            really_selected_nr = 1;
        }
    }
    
    if(really_selected_nr > 1) {
        setStatus(
            "To copy a path link's properties, you can only select 1 "
            "path link!",
            true
        );
        return;
    }
    
    PathLink* source_link = *selectedPathLinks.begin();
    if(!copyBufferPathLink) {
        copyBufferPathLink = new PathLink(nullptr, nullptr, INVALID);
    }
    source_link->clone(copyBufferPathLink);
    setStatus("Successfully copied the path link's properties.");
    return;
}


/**
 * @brief Copies the currently selected sector's properties onto the
 * copy buffer, so they can be then pasted onto another sector.
 *
 */
void AreaEditor::copySectorProperties() {
    if(selectedSectors.empty()) {
        setStatus(
            "To copy a sector's properties, you must first select a sector "
            "to copy from!",
            true
        );
        return;
    }
    
    if(selectedSectors.size() > 1) {
        setStatus(
            "To copy a sector's properties, you can only select 1 sector!",
            true
        );
        return;
    }
    
    Sector* source_sector = *selectedSectors.begin();
    if(!copyBufferSector) {
        copyBufferSector = new Sector();
    }
    source_sector->clone(copyBufferSector);
    copyBufferSector->textureInfo = source_sector->textureInfo;
    setStatus("Successfully copied the sector's properties.");
    return;
}


/**
 * @brief Creates a new sector for use in layout drawing operations.
 * This automatically clones it from another sector, if not nullptr, or gives it
 * a recommended texture if the other sector nullptr.
 *
 * @param copy_from Sector to copy from.
 * @return The created sector.
 */
Sector* AreaEditor::createSectorForLayoutDrawing(const Sector* copy_from) {
    Sector* new_sector = game.curAreaData->newSector();
    
    if(copy_from) {
        copy_from->clone(new_sector);
        updateSectorTexture(new_sector, copy_from->textureInfo.bmpName);
    } else {
        if(!textureSuggestions.empty()) {
            updateSectorTexture(new_sector, textureSuggestions[0].name);
        } else {
            updateSectorTexture(new_sector, "");
        }
    }
    
    return new_sector;
}


/**
 * @brief Deletes the specified edge, removing it from all sectors and
 * vertexes that use it, as well as removing any now-useless sectors
 * or vertexes.
 *
 * @param e_ptr Edge to delete.
 */
void AreaEditor::deleteEdge(Edge* e_ptr) {
    //Remove sectors first.
    Sector* sectors[2] = { e_ptr->sectors[0], e_ptr->sectors[1] };
    e_ptr->removeFromSectors();
    for(size_t s = 0; s < 2; s++) {
        if(!sectors[s]) continue;
        if(sectors[s]->edges.empty()) {
            game.curAreaData->removeSector(sectors[s]);
        }
    }
    
    //Now, remove vertexes.
    Vertex* vertexes[2] = { e_ptr->vertexes[0], e_ptr->vertexes[1] };
    e_ptr->removeFromVertexes();
    for(size_t v = 0; v < 2; v++) {
        if(vertexes[v]->edges.empty()) {
            game.curAreaData->removeVertex(vertexes[v]);
        }
    }
    
    //Finally, delete the edge proper.
    game.curAreaData->removeEdge(e_ptr);
}


/**
 * @brief Deletes the specified edges. The sectors on each side of the edge
 * are merged, so the smallest sector will be deleted. In addition,
 * this operation will delete any sectors that would end up incomplete.
 *
 * @param which Edges to delete.
 * @return Whether all edges were deleted successfully.
 */
bool AreaEditor::deleteEdges(const set<Edge*> &which) {
    bool ret = true;
    
    for(Edge* e_ptr : which) {
        if(!e_ptr->vertexes[0]) {
            //Huh, looks like one of the edge deletion procedures already
            //wiped this edge out. Skip it.
            continue;
        }
        if(!mergeSectors(e_ptr->sectors[0], e_ptr->sectors[1])) {
            ret = false;
        }
    }
    
    return ret;
}


/**
 * @brief Deletes the specified mobs.
 *
 * @param which Mobs to delete.
 */
void AreaEditor::deleteMobs(const set<MobGen*> &which) {
    for(auto const &sm : which) {
        //Get its index.
        size_t m_idx = 0;
        for(; m_idx < game.curAreaData->mobGenerators.size(); m_idx++) {
            if(game.curAreaData->mobGenerators[m_idx] == sm) break;
        }
        
        //Update links.
        for(
            size_t m2 = 0; m2 < game.curAreaData->mobGenerators.size(); m2++
        ) {
            MobGen* m2_ptr = game.curAreaData->mobGenerators[m2];
            
            for(size_t l = 0; l < m2_ptr->links.size(); l++) {
            
                if(m2_ptr->linkIdxs[l] > m_idx) {
                    m2_ptr->linkIdxs[l]--;
                }
                
                if(m2_ptr->links[l] == sm) {
                    m2_ptr->links.erase(m2_ptr->links.begin() + l);
                    m2_ptr->linkIdxs.erase(m2_ptr->linkIdxs.begin() + l);
                }
            }
            
            if(
                m2_ptr->storedInside != INVALID &&
                m2_ptr->storedInside > m_idx
            ) {
                m2_ptr->storedInside--;
            } else if(m2_ptr->storedInside == m_idx) {
                m2_ptr->storedInside = INVALID;
            }
        }
        
        //Check the list of mission requirement objects.
        unordered_set<size_t> new_mrmi;
        new_mrmi.reserve(game.curAreaData->mission.goalMobIdxs.size());
        for(size_t m2 : game.curAreaData->mission.goalMobIdxs) {
            if(m2 > m_idx) {
                new_mrmi.insert(m2 - 1);
            } else if(m2 != m_idx) {
                new_mrmi.insert(m2);
            }
        }
        game.curAreaData->mission.goalMobIdxs = new_mrmi;
        
        //Finally, delete it.
        game.curAreaData->mobGenerators.erase(
            game.curAreaData->mobGenerators.begin() + m_idx
        );
        delete sm;
    }
}


/**
 * @brief Deletes the specified path links.
 *
 * @param which Path links to delete.
 */
void AreaEditor::deletePathLinks(const set<PathLink*> &which) {
    for(auto &l : which) {
        l->startPtr->removeLink(l);
    }
}


/**
 * @brief Deletes the specified path stops.
 *
 * @param which Path stops to delete.
 */
void AreaEditor::deletePathStops(const set<PathStop*> &which) {
    for(auto &s : which) {
        //Check all links that end at this stop.
        for(size_t s2 = 0; s2 < game.curAreaData->pathStops.size(); s2++) {
            PathStop* s2_ptr = game.curAreaData->pathStops[s2];
            s2_ptr->removeLink(s);
        }
        
        //Finally, delete the stop.
        delete s;
        for(size_t s2 = 0; s2 < game.curAreaData->pathStops.size(); s2++) {
            if(game.curAreaData->pathStops[s2] == s) {
                game.curAreaData->pathStops.erase(
                    game.curAreaData->pathStops.begin() + s2
                );
                break;
            }
        }
    }
    
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        game.curAreaData->fixPathStopIdxs(game.curAreaData->pathStops[s]);
    }
}


/**
 * @brief Tries to find a good texture for the first sector in a
 * newly-created area.
 *
 * @return The texture's internal name, or empty if none was found.
 */
string AreaEditor::findGoodFirstTexture() {
    //First, if there's any "grass" texture, use that.
    for(const auto &g : game.content.bitmaps.manifests) {
        string lc_name = strToLower(g.first);
        if(
            lc_name.find("texture") != string::npos &&
            lc_name.find("grass") != string::npos
        ) {
            return g.first;
        }
    }
    
    //No grass texture? Try one with "dirt".
    for(const auto &g : game.content.bitmaps.manifests) {
        string lc_name = strToLower(g.first);
        if(
            lc_name.find("texture") != string::npos &&
            lc_name.find("dirt") != string::npos
        ) {
            return g.first;
        }
    }
    
    //If there's no good texture, just pick the first one.
    for(const auto &g : game.content.bitmaps.manifests) {
        string lc_name = strToLower(g.first);
        if(lc_name.find("texture") != string::npos) {
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
    
    findProblemsNoGoalMob();
    if(problemType != EPT_NONE_YET) return;
    
    findProblemsNoScoreCriteria();
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
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        if(!m_ptr->type) continue;
        if(m_ptr->type->category->id != MOB_CATEGORY_PILES) {
            continue;
        }
        
        for(size_t l = 0; l < m_ptr->links.size(); l++) {
            if(!m_ptr->links[l]->type) continue;
            if(m_ptr->links[l]->type->category->id != MOB_CATEGORY_BRIDGES) {
                continue;
            }
            
            PathFollowSettings settings;
            settings.flags =
                PATH_FOLLOW_FLAG_SCRIPT_USE |
                PATH_FOLLOW_FLAG_LIGHT_LOAD |
                PATH_FOLLOW_FLAG_AIRBORNE;
            vector<PathStop*> path;
            getPath(
                m_ptr->pos, m_ptr->links[l]->pos,
                settings, path,
                nullptr, nullptr, nullptr
            );
            
            for(size_t s = 1; s < path.size(); s++) {
                if(
                    circleIntersectsLineSeg(
                        m_ptr->links[l]->pos,
                        getMobGenRadius(m_ptr->links[l]),
                        path[s - 1]->pos,
                        path[s]->pos
                    )
                ) {
                    problemMobPtr = m_ptr->links[l];
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
 * @brief Checks for any intersecting edges in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsIntersectingEdge() {
    vector<EdgeIntersection> intersections = getIntersectingEdges();
    if(!intersections.empty()) {
        float r;
        EdgeIntersection* ei_ptr = &(*intersections.begin());
        lineSegsIntersect(
            v2p(ei_ptr->e1->vertexes[0]), v2p(ei_ptr->e1->vertexes[1]),
            v2p(ei_ptr->e2->vertexes[0]), v2p(ei_ptr->e2->vertexes[1]),
            &r, nullptr
        );
        
        float a =
            getAngle(
                v2p(ei_ptr->e1->vertexes[0]), v2p(ei_ptr->e1->vertexes[1])
            );
        Distance d(
            v2p(ei_ptr->e1->vertexes[0]), v2p(ei_ptr->e1->vertexes[1])
        );
        
        problemEdgeIntersection = *intersections.begin();
        problemType = EPT_INTERSECTING_EDGES;
        problemTitle = "Two edges cross each other!";
        problemDescription =
            "They cross at (" +
            f2s(
                floor(ei_ptr->e1->vertexes[0]->x + cos(a) * r *
                      d.toFloat())
            ) + "," + f2s(
                floor(ei_ptr->e1->vertexes[0]->y + sin(a) * r *
                      d.toFloat())
            ) + "). Edges should never cross each other.";
    }
}


/**
 * @brief Checks for any lone edges in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsLoneEdge() {
    if(!game.curAreaData->problems.loneEdges.empty()) {
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
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* s_ptr = game.curAreaData->pathStops[s];
        bool has_link = false;
        
        if(!s_ptr->links.empty()) continue; //Duh, this means it has links.
        
        for(size_t s2 = 0; s2 < game.curAreaData->pathStops.size(); s2++) {
            PathStop* s2_ptr = game.curAreaData->pathStops[s2];
            if(s2_ptr == s_ptr) continue;
            
            if(s2_ptr->get_link(s_ptr)) {
                has_link = true;
                break;
            }
            
            if(has_link) break;
        }
        
        if(!has_link) {
            problemPathStopPtr = s_ptr;
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
    bool has_leader = false;
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        if(
            game.curAreaData->mobGenerators[m]->type != nullptr &&
            game.curAreaData->mobGenerators[m]->type->category->id ==
            MOB_CATEGORY_LEADERS
        ) {
            has_leader = true;
            break;
        }
    }
    if(!has_leader) {
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
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* s_ptr = game.curAreaData->sectors[s];
        if(s_ptr->edges.empty()) continue;
        if(s_ptr->isBottomlessPit) continue;
        if(
            s_ptr->textureInfo.bmpName.empty() &&
            !s_ptr->isBottomlessPit && !s_ptr->fade
        ) {
            problemSectorPtr = s_ptr;
            problemType = EPT_UNKNOWN_TEXTURE;
            problemTitle = "Sector with missing texture!";
            problemDescription =
                "Give it a valid texture.";
            return;
        }
    }
}


/**
 * @brief Checks for any mobs are inside walls in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsMobInsideWalls() {
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        if(!m_ptr->type) continue;
        
        if(
            m_ptr->type->category->id == MOB_CATEGORY_BRIDGES ||
            m_ptr->type->category->id == MOB_CATEGORY_DECORATIONS
        ) {
            continue;
        }
        
        for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
            Edge* e_ptr = game.curAreaData->edges[e];
            if(!e_ptr->isValid()) continue;
            
            if(
                circleIntersectsLineSeg(
                    m_ptr->pos,
                    m_ptr->type->radius,
                    v2p(e_ptr->vertexes[0]), v2p(e_ptr->vertexes[1]),
                    nullptr, nullptr
                )
            ) {
            
                if(
                    e_ptr->sectors[0] && e_ptr->sectors[1] &&
                    e_ptr->sectors[0]->z == e_ptr->sectors[1]->z
                ) {
                    continue;
                }
                
                Sector* mob_sector = getSector(m_ptr->pos, nullptr, false);
                
                bool in_wall = false;
                
                if(
                    !e_ptr->sectors[0] || !e_ptr->sectors[1]
                ) {
                    //Either sector is the void, definitely stuck.
                    in_wall = true;
                    
                } else if(
                    e_ptr->sectors[0] != mob_sector &&
                    e_ptr->sectors[1] != mob_sector
                ) {
                    //It's intersecting with two sectors that aren't
                    //even the sector it's on? Definitely inside wall.
                    in_wall = true;
                    
                } else if(
                    e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING ||
                    e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
                ) {
                    //If either sector's of the blocking type, definitely stuck.
                    in_wall = true;
                    
                } else if(
                    e_ptr->sectors[0] == mob_sector &&
                    e_ptr->sectors[1]->z > mob_sector->z + GEOMETRY::STEP_HEIGHT
                ) {
                    in_wall = true;
                    
                } else if(
                    e_ptr->sectors[1] == mob_sector &&
                    e_ptr->sectors[0]->z > mob_sector->z + GEOMETRY::STEP_HEIGHT
                ) {
                    in_wall = true;
                    
                }
                
                if(in_wall) {
                    problemMobPtr = m_ptr;
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
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        for(size_t l = 0; l < m_ptr->links.size(); l++) {
            if(m_ptr->links[l] == m_ptr) {
                problemMobPtr = m_ptr;
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
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        if(m_ptr->storedInside == INVALID) continue;
        unordered_set<MobGen*> visited_mobs;
        visited_mobs.insert(m_ptr);
        size_t next_idx = m_ptr->storedInside;
        while(next_idx != INVALID) {
            MobGen* next_ptr = game.curAreaData->mobGenerators[next_idx];
            if(visited_mobs.find(next_ptr) != visited_mobs.end()) {
                problemMobPtr = next_ptr;
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
            visited_mobs.insert(next_ptr);
            next_idx = next_ptr->storedInside;
        }
    }
}


/**
 * @brief Checks for any missing mission goal mob in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsNoGoalMob() {
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        (
            game.curAreaData->mission.goal == MISSION_GOAL_COLLECT_TREASURE ||
            game.curAreaData->mission.goal == MISSION_GOAL_BATTLE_ENEMIES ||
            game.curAreaData->mission.goal == MISSION_GOAL_GET_TO_EXIT
        )
    ) {
        if(getMissionRequiredMobCount() == 0) {
            problemType = EPT_NO_GOAL_MOBS;
            problemTitle = "No mission goal mobs!";
            problemDescription =
                "This mission's goal requires some mobs, yet there are none.";
            return;
        }
    }
}


/**
 * @brief Checks for any missing mission score criterion in the area, and
 * fills the problem info if so.
 */
void AreaEditor::findProblemsNoScoreCriteria() {
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.gradingMode == MISSION_GRADING_MODE_POINTS
    ) {
        bool has_any_criterion = false;
        for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
            if(
                game.missionScoreCriteria[c]->getMultiplier(
                    &game.curAreaData->mission
                ) != 0
            ) {
                has_any_criterion = true;
                break;
            }
        }
        if(!has_any_criterion) {
            problemType = EPT_NO_SCORE_CRITERIA;
            problemTitle = "No active score criteria!";
            problemDescription =
                "In this mission, the player is graded according to their "
                "score. However, none of the score criteria are active, "
                "so the player's score will always be 0.";
            return;
        }
    }
}


/**
 * @brief Checks for any non-simple sectors in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsNonSimpleSector() {
    if(!game.curAreaData->problems.nonSimples.empty()) {
        problemType = EPT_BAD_SECTOR;
        problemTitle = "Non-simple sector!";
        switch(game.curAreaData->problems.nonSimples.begin()->second) {
        case TRIANGULATION_ERROR_LONE_EDGES: {
            problemDescription =
                "It contains lone edges. Try clearing them up.";
            break;
        } case TRIANGULATION_ERROR_NOT_CLOSED: {
            problemDescription =
                "It is not closed. Try closing it.";
            break;
        } case TRIANGULATION_ERROR_NO_EARS: {
            problemDescription =
                "There's been a triangulation error. Try undoing or "
                "deleting the sector, and then rebuild it. Make sure there "
                "are no gaps, and keep it simple.";
            break;
        } case TRIANGULATION_ERROR_INVALID_ARGS: {
            problemDescription =
                "An unknown error has occured with the sector.";
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
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        if(!getSector(m_ptr->pos, nullptr, false)) {
            problemMobPtr = m_ptr;
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
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* s_ptr = game.curAreaData->pathStops[s];
        if(!getSector(s_ptr->pos, nullptr, false)) {
            problemPathStopPtr = s_ptr;
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
    for(size_t v = 0; v < game.curAreaData->vertexes.size(); v++) {
        Vertex* v1_ptr = game.curAreaData->vertexes[v];
        
        for(size_t v2 = v + 1; v2 < game.curAreaData->vertexes.size(); v2++) {
            Vertex* v2_ptr = game.curAreaData->vertexes[v2];
            
            if(v1_ptr->x == v2_ptr->x && v1_ptr->y == v2_ptr->y) {
                problemVertexPtr = v1_ptr;
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
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* s_ptr = game.curAreaData->pathStops[s];
        for(size_t s2 = 0; s2 < game.curAreaData->pathStops.size(); s2++) {
            PathStop* link_start_ptr = game.curAreaData->pathStops[s2];
            if(link_start_ptr == s_ptr) continue;
            
            for(size_t l = 0; l < link_start_ptr->links.size(); l++) {
                PathStop* link_end_ptr = link_start_ptr->links[l]->endPtr;
                if(link_end_ptr == s_ptr) continue;
                
                if(
                    circleIntersectsLineSeg(
                        s_ptr->pos, s_ptr->radius,
                        link_start_ptr->pos, link_end_ptr->pos
                    )
                ) {
                    problemPathStopPtr = s_ptr;
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
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* s_ptr = game.curAreaData->pathStops[s];
        for(size_t s2 = 0; s2 < game.curAreaData->pathStops.size(); s2++) {
            PathStop* s2_ptr = game.curAreaData->pathStops[s2];
            if(s2_ptr == s_ptr) continue;
            
            if(Distance(s_ptr->pos, s2_ptr->pos) <= 3.0) {
                problemPathStopPtr = s_ptr;
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
    size_t n_pikmin_mobs = 0;
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        if(m_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
            n_pikmin_mobs++;
            if(n_pikmin_mobs > game.config.rules.maxPikminInField) {
                problemType = EPT_PIKMIN_OVER_LIMIT;
                problemTitle = "Over the Pikmin limit!";
                problemDescription =
                    "There are more Pikmin in the area than the limit allows. "
                    "This means some of them will not appear. Current limit: "
                    + i2s(game.config.rules.maxPikminInField) + ".";
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
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        if(!game.curAreaData->mobGenerators[m]->type) {
            problemMobPtr = game.curAreaData->mobGenerators[m];
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
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* s_ptr = game.curAreaData->sectors[s];
        if(s_ptr->edges.empty()) continue;
        if(s_ptr->isBottomlessPit) continue;
        
        if(s_ptr->textureInfo.bmpName.empty()) continue;
        
        const auto &texture_it =
            game.content.bitmaps.manifests.find(s_ptr->textureInfo.bmpName);
        if(texture_it == game.content.bitmaps.manifests.end()) {
            problemSectorPtr = s_ptr;
            problemType = EPT_UNKNOWN_TEXTURE;
            problemTitle = "Sector with unknown texture!";
            problemDescription =
                "Texture name: \"" + s_ptr->textureInfo.bmpName + "\".";
            return;
        }
    }
}


/**
 * @brief Checks for any unknown tree shadow texture in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsUnknownTreeShadow() {
    for(size_t s = 0; s < game.curAreaData->treeShadows.size(); s++) {
        if(game.curAreaData->treeShadows[s]->bitmap == game.bmpError) {
            problemShadowPtr = game.curAreaData->treeShadows[s];
            problemType = EPT_UNKNOWN_SHADOW;
            problemTitle = "Tree shadow with invalid texture!";
            problemDescription =
                "Texture name: \"" +
                game.curAreaData->treeShadows[s]->bmpName + "\".";
            return;
        }
    }
}


/**
 * @brief Adds to the list all sectors affected by the specified sector.
 * The list can include the nullptr sector, and will include the
 * provided sector too.
 *
 * @param s_ptr Sector that's affecting others.
 * @param list The list of affected sectors to fill out.
 */
void AreaEditor::getAffectedSectors(
    Sector* s_ptr, unordered_set<Sector*> &list
) const {
    for(size_t e = 0; e < s_ptr->edges.size(); e++) {
        list.insert(s_ptr->edges[e]->sectors[0]);
        list.insert(s_ptr->edges[e]->sectors[1]);
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
    const set<Sector*> &sectors, unordered_set<Sector*> &list
) const {
    for(auto &s : sectors) {
        getAffectedSectors(s, list);
    }
}


/**
 * @brief Adds to the list all sectors affected by the specified vertexes.
 * The list can include the nullptr sector.
 *
 * @param vertexes Vertexes that are affecting sectors.
 * @param list The list of affected sectors to fill out.
 */
void AreaEditor::getAffectedSectors(
    const set<Vertex*> &vertexes, unordered_set<Sector*> &list
) const {
    for(auto &v : vertexes) {
        for(size_t e = 0; e < v->edges.size(); e++) {
            list.insert(v->edges[e]->sectors[0]);
            list.insert(v->edges[e]->sectors[1]);
        }
    }
}


/**
 * @brief For a given vertex, returns the edge closest to the given angle,
 * in the given direction.
 *
 * @param v_ptr Pointer to the vertex.
 * @param angle Angle coming into the vertex.
 * @param clockwise Return the closest edge clockwise?
 * @param out_closest_edge_angle If not nullptr, the angle the edge makes
 * into its other vertex is returned here.
 * @return The closest edge.
 */
Edge* AreaEditor::getClosestEdgeToAngle(
    Vertex* v_ptr, float angle, bool clockwise,
    float* out_closest_edge_angle
) const {
    Edge* best_edge = nullptr;
    float best_angle_diff = 0;
    float best_edge_angle = 0;
    
    for(size_t e = 0; e < v_ptr->edges.size(); e++) {
        Edge* e_ptr = v_ptr->edges[e];
        Vertex* other_v_ptr = e_ptr->getOtherVertex(v_ptr);
        
        float a = getAngle(v2p(v_ptr), v2p(other_v_ptr));
        float diff = getAngleCwDiff(angle, a);
        
        if(
            !best_edge ||
            (clockwise && diff < best_angle_diff) ||
            (!clockwise && diff > best_angle_diff)
        ) {
            best_edge = e_ptr;
            best_angle_diff = diff;
            best_edge_angle = a;
        }
    }
    
    if(out_closest_edge_angle) {
        *out_closest_edge_angle = best_edge_angle;
    }
    return best_edge;
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
    vector<Vertex*> &vertexes, vector<Edge*> &edges, Sector** result
) const {
    unordered_set<Sector*> sectors;
    
    //First, populate the list of common sectors with a sample.
    //Let's use the first vertex or edge's sectors.
    if(!vertexes.empty()) {
        for(size_t e = 0; e < vertexes[0]->edges.size(); e++) {
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
    for(size_t v = 0; v < vertexes.size(); v++) {
        Vertex* v_ptr = vertexes[v];
        for(auto s = sectors.begin(); s != sectors.end();) {
            bool found_s = false;
            
            for(size_t e = 0; e < v_ptr->edges.size(); e++) {
                if(
                    v_ptr->edges[e]->sectors[0] == *s ||
                    v_ptr->edges[e]->sectors[1] == *s
                ) {
                    found_s = true;
                    break;
                }
            }
            
            if(!found_s) {
                sectors.erase(s++);
            } else {
                ++s;
            }
        }
    }
    
    //Now repeat for each edge.
    for(size_t e = 0; e < edges.size(); e++) {
        Edge* e_ptr = edges[e];
        for(auto s = sectors.begin(); s != sectors.end();) {
            if(e_ptr->sectors[0] != *s && e_ptr->sectors[1] != *s) {
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
    float best_rightmost_x = 0;
    Sector* best_rightmost_sector = nullptr;
    for(auto &s : sectors) {
        if(s == nullptr) continue;
        Vertex* v_ptr = s->getRightmostVertex();
        if(!best_rightmost_sector || v_ptr->x < best_rightmost_x) {
            best_rightmost_sector = s;
            best_rightmost_x = v_ptr->x;
        }
    }
    
    *result = best_rightmost_sector;
    return true;
}


/**
 * @brief After an edge split, some vertexes could've wanted to merge with the
 * original edge, but may now need to merge with the NEW edge.
 * This function can check which is the "correct" edge to point to, from
 * the two provided.
 *
 * @param v_ptr Vertex that caused a split.
 * @param e1_ptr First edge resulting of the split.
 * @param e2_ptr Second edge resulting of the split.
 * @return The correct edge.
 */
Edge* AreaEditor::getCorrectPostSplitEdge(
    const Vertex* v_ptr, Edge* e1_ptr, Edge* e2_ptr
) const {
    float score1 = 0;
    float score2 = 0;
    getClosestPointInLineSeg(
        v2p(e1_ptr->vertexes[0]), v2p(e1_ptr->vertexes[1]),
        v2p(v_ptr), &score1
    );
    getClosestPointInLineSeg(
        v2p(e2_ptr->vertexes[0]), v2p(e2_ptr->vertexes[1]),
        v2p(v_ptr), &score2
    );
    if(fabs(score1 - 0.5) < fabs(score2 - 0.5)) {
        return e1_ptr;
    } else {
        return e2_ptr;
    }
}


/**
 * @brief Returns true if the drawing has an outer sector it belongs to,
 * even if the sector is the void, or false if something's gone wrong.
 *
 * @param result The outer sector, if any, is returned here.
 * @return Whether it succeded.
 */
bool AreaEditor::getDrawingOuterSector(Sector** result) const {
    //Start by checking if there's a node on a sector. If so, that's it!
    for(size_t n = 0; n < drawingNodes.size(); n++) {
        if(!drawingNodes[n].onVertex && !drawingNodes[n].onEdge) {
            (*result) = drawingNodes[n].onSector;
            return true;
        }
    }
    
    //If none are on sectors, let's try the following:
    //Grab the first line that is not on top of an existing one,
    //and find the sector that line is on by checking its center.
    for(size_t n = 0; n < drawingNodes.size(); n++) {
        const LayoutDrawingNode* n1 = &drawingNodes[n];
        const LayoutDrawingNode* n2 = &(getNextInVector(drawingNodes, n));
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
    for(size_t n = 0; n < drawingNodes.size(); n++) {
        if(drawingNodes[n].onVertex) {
            v.push_back(drawingNodes[n].onVertex);
        } else if(drawingNodes[n].onEdge) {
            e.push_back(drawingNodes[n].onEdge);
        }
    }
    return getCommonSector(v, e, result);
}


/**
 * @brief Returns the edge currently under the specified point, or nullptr if none.
 *
 * @param p The point.
 * @param after Only check edges that come after this one.
 * @return The edge.
 */
Edge* AreaEditor::getEdgeUnderPoint(
    const Point &p, const Edge* after
) const {
    bool found_after = (!after ? true : false);
    
    for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
        Edge* e_ptr = game.curAreaData->edges[e];
        if(e_ptr == after) {
            found_after = true;
            continue;
        } else if(!found_after) {
            continue;
        }
        
        if(!e_ptr->isValid()) continue;
        
        if(
            circleIntersectsLineSeg(
                p, 8 / game.view.cam.zoom,
                v2p(e_ptr->vertexes[0]), v2p(e_ptr->vertexes[1])
            )
        ) {
            return e_ptr;
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
    
    for(size_t e1 = 0; e1 < game.curAreaData->edges.size(); e1++) {
        Edge* e1_ptr = game.curAreaData->edges[e1];
        for(size_t e2 = e1 + 1; e2 < game.curAreaData->edges.size(); e2++) {
            Edge* e2_ptr = game.curAreaData->edges[e2];
            if(e1_ptr->hasNeighbor(e2_ptr)) continue;
            if(
                lineSegsIntersect(
                    v2p(e1_ptr->vertexes[0]), v2p(e1_ptr->vertexes[1]),
                    v2p(e2_ptr->vertexes[0]), v2p(e2_ptr->vertexes[1]),
                    nullptr, nullptr
                )
            ) {
                intersections.push_back(EdgeIntersection(e1_ptr, e2_ptr));
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
    const Point &p,
    std::pair<MobGen*, MobGen*>* data1, std::pair<MobGen*, MobGen*>* data2
) const {
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        for(size_t l = 0; l < m_ptr->links.size(); l++) {
            MobGen* m2_ptr = m_ptr->links[l];
            if(
                circleIntersectsLineSeg(
                    p, 8 / game.view.cam.zoom, m_ptr->pos, m2_ptr->pos
                )
            ) {
                *data1 = std::make_pair(m_ptr, m2_ptr);
                *data2 = std::make_pair((MobGen*) nullptr, (MobGen*) nullptr);
                
                for(size_t l2 = 0; l2 < m2_ptr->links.size(); l2++) {
                    if(m2_ptr->links[l2] == m_ptr) {
                        *data2 = std::make_pair(m2_ptr, m_ptr);
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
 * @param out_idx If not nullptr, the mob index is returned here.
 * If no mob matches, INVALID is returned instead.
 * @return The mob.
 */
MobGen* AreaEditor::getMobUnderPoint(
    const Point &p, size_t* out_idx
) const {
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        
        if(
            Distance(m_ptr->pos, p) <= getMobGenRadius(m_ptr)
        ) {
            if(out_idx) *out_idx = m;
            return m_ptr;
        }
    }
    
    if(out_idx) *out_idx = INVALID;
    return nullptr;
}


/**
 * @brief Returns true if there are path links currently under the specified
 * point. link1 takes the info of the found link. If there's also a link in
 * the opposite direction, link2 gets that data, otherwise link2 receives nullptr.
 *
 * @param p The point to check against.
 * @param link1 If there is a path link under that point,
 * its pointer is returned here.
 * @param link2 If there is a path link under the point, but going in the
 * opposite direction, its pointer is returned here.
 * @return The link.
 */
bool AreaEditor::getPathLinkUnderPoint(
    const Point &p, PathLink** link1, PathLink** link2
) const {
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* s_ptr = game.curAreaData->pathStops[s];
        for(size_t l = 0; l < s_ptr->links.size(); l++) {
            PathStop* s2_ptr = s_ptr->links[l]->endPtr;
            if(
                circleIntersectsLineSeg(
                    p, 8 / game.view.cam.zoom, s_ptr->pos, s2_ptr->pos
                )
            ) {
                *link1 = s_ptr->links[l];
                *link2 = s2_ptr->get_link(s_ptr);
                return true;
            }
        }
    }
    
    return false;
}


/**
 * @brief Returns the path stop currently under the specified point,
 * or nullptr if none.
 *
 * @param p Point to check against.
 * @return The stop.
 */
PathStop* AreaEditor::getPathStopUnderPoint(const Point &p) const {
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* s_ptr = game.curAreaData->pathStops[s];
        
        if(Distance(s_ptr->pos, p) <= s_ptr->radius) {
            return s_ptr;
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
Sector* AreaEditor::getSectorUnderPoint(const Point &p) const {
    return getSector(p, nullptr, false);
}


/**
 * @brief Returns the vertex currently under the specified point,
 * or nullptr if none.
 *
 * @param p Point to check against.
 * @return The vertex.
 */
Vertex* AreaEditor::getVertexUnderPoint(const Point &p) const {
    for(size_t v = 0; v < game.curAreaData->vertexes.size(); v++) {
        Vertex* v_ptr = game.curAreaData->vertexes[v];
        
        if(
            rectanglesIntersect(
                p - (4 / game.view.cam.zoom),
                p + (4 / game.view.cam.zoom),
                Point(
                    v_ptr->x - (4 / game.view.cam.zoom),
                    v_ptr->y - (4 / game.view.cam.zoom)
                ),
                Point(
                    v_ptr->x + (4 / game.view.cam.zoom),
                    v_ptr->y + (4 / game.view.cam.zoom)
                )
            )
        ) {
            return v_ptr;
        }
    }
    
    return nullptr;
}


/**
 * @brief Homogenizes all selected edges,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedEdges() {
    if(selectedEdges.size() < 2) return;
    
    Edge* base = *selectedEdges.begin();
    for(auto e = selectedEdges.begin(); e != selectedEdges.end(); ++e) {
        if(e == selectedEdges.begin()) continue;
        base->clone(*e);
    }
}


/**
 * @brief Homogenizes all selected mobs,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedMobs() {
    if(selectedMobs.size() < 2) return;
    
    MobGen* base = *selectedMobs.begin();
    for(auto m = selectedMobs.begin(); m != selectedMobs.end(); ++m) {
        if(m == selectedMobs.begin()) continue;
        base->clone(*m, false);
    }
}


/**
 * @brief Homogenizes all selected path links,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedPathLinks() {
    if(selectedPathLinks.size() < 2) return;
    
    PathLink* base = *selectedPathLinks.begin();
    for(
        auto l = selectedPathLinks.begin();
        l != selectedPathLinks.end();
        ++l
    ) {
        if(l == selectedPathLinks.begin()) continue;
        
        base->clone(*l);
    }
}


/**
 * @brief Homogenizes all selected path stops,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedPathStops() {
    if(selectedPathStops.size() < 2) return;
    
    PathStop* base = *selectedPathStops.begin();
    for(
        auto s = selectedPathStops.begin();
        s != selectedPathStops.end();
        ++s
    ) {
        if(s == selectedPathStops.begin()) continue;
        
        base->clone(*s);
    }
}


/**
 * @brief Homogenizes all selected sectors,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedSectors() {
    if(selectedSectors.size() < 2) return;
    
    Sector* base = *selectedSectors.begin();
    for(auto s = selectedSectors.begin(); s != selectedSectors.end(); ++s) {
        if(s == selectedSectors.begin()) continue;
        base->clone(*s);
        updateSectorTexture(*s, base->textureInfo.bmpName);
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
    Sector* main_sector = s1;
    Sector* small_sector = s2;
    if(!s2) {
        main_sector = s2;
        small_sector = s1;
    } else if(s1) {
        float s1_area =
            (s1->bbox[1].x - s1->bbox[0].x) *
            (s1->bbox[1].y - s1->bbox[0].y);
        float s2_area =
            (s2->bbox[1].x - s2->bbox[0].x) *
            (s2->bbox[1].y - s2->bbox[0].y);
        if(s1_area < s2_area) {
            main_sector = s2;
            small_sector = s1;
        }
    }
    
    //For all of the smaller sector's edges, either mark them
    //as edges to transfer to the large sector, or edges
    //to delete (because they'd just end up having the larger sector on
    //both sides).
    unordered_set<Edge*> common_edges;
    unordered_set<Edge*> edges_to_transfer;
    
    for(size_t e = 0; e < small_sector->edges.size(); e++) {
        Edge* e_ptr = small_sector->edges[e];
        if(e_ptr->getOtherSector(small_sector) == main_sector) {
            common_edges.insert(e_ptr);
        } else {
            edges_to_transfer.insert(e_ptr);
        }
    }
    
    //However, if there are no common edges beween sectors,
    //this operation is invalid.
    if(common_edges.empty()) {
        setStatus("Those two sectors are not neighbors!", true);
        return false;
    }
    
    //Before doing anything, get the list of sectors that will be affected.
    unordered_set<Sector*> affected_sectors;
    getAffectedSectors(small_sector, affected_sectors);
    if(main_sector) getAffectedSectors(main_sector, affected_sectors);
    
    //Transfer edges that need transferal.
    for(Edge* e_ptr : edges_to_transfer) {
        e_ptr->transferSector(
            small_sector, main_sector,
            main_sector ?
            game.curAreaData->findSectorIdx(main_sector) :
            INVALID,
            game.curAreaData->findEdgeIdx(e_ptr)
        );
    }
    
    //Delete the other ones.
    for(Edge* e_ptr : common_edges) {
        deleteEdge(e_ptr);
    }
    
    //Delete the now-merged sector.
    game.curAreaData->removeSector(small_sector);
    
    //Update all affected sectors.
    affected_sectors.erase(small_sector);
    updateAffectedSectors(affected_sectors);
    
    return true;
}


/**
 * @brief Merges vertex 1 into vertex 2.
 *
 * @param v1 Vertex that is being moved and will be merged.
 * @param v2 Vertex that is going to absorb v1.
 * @param affected_sectors List of sectors that will be affected by this merge.
 */
void AreaEditor::mergeVertex(
    const Vertex* v1, Vertex* v2, unordered_set<Sector*>* affected_sectors
) {
    vector<Edge*> edges = v1->edges;
    //Find out what to do with every edge of the dragged vertex.
    for(size_t e = 0; e < edges.size(); e++) {
    
        Edge* e_ptr = edges[e];
        Vertex* other_vertex = e_ptr->getOtherVertex(v1);
        
        if(other_vertex == v2) {
        
            //Squashed into non-existence.
            affected_sectors->insert(e_ptr->sectors[0]);
            affected_sectors->insert(e_ptr->sectors[1]);
            
            //Delete it.
            deleteEdge(e_ptr);
            
        } else {
        
            bool has_merged = false;
            //Check if the edge will be merged with another one.
            //These are edges that share a common vertex,
            //plus the moved/destination vertex.
            for(size_t de = 0; de < v2->edges.size(); de++) {
            
                Edge* de_ptr = v2->edges[de];
                Vertex* d_other_vertex = de_ptr->getOtherVertex(v2);
                
                if(d_other_vertex == other_vertex) {
                    //The edge will be merged with this one.
                    has_merged = true;
                    affected_sectors->insert(e_ptr->sectors[0]);
                    affected_sectors->insert(e_ptr->sectors[1]);
                    affected_sectors->insert(de_ptr->sectors[0]);
                    affected_sectors->insert(de_ptr->sectors[1]);
                    
                    //Set the new sectors.
                    if(e_ptr->sectors[0] == de_ptr->sectors[0]) {
                        game.curAreaData->connectEdgeToSector(
                            de_ptr, e_ptr->sectors[1], 0
                        );
                    } else if(e_ptr->sectors[0] == de_ptr->sectors[1]) {
                        game.curAreaData->connectEdgeToSector(
                            de_ptr, e_ptr->sectors[1], 1
                        );
                    } else if(e_ptr->sectors[1] == de_ptr->sectors[0]) {
                        game.curAreaData->connectEdgeToSector(
                            de_ptr, e_ptr->sectors[0], 0
                        );
                    } else if(e_ptr->sectors[1] == de_ptr->sectors[1]) {
                        game.curAreaData->connectEdgeToSector(
                            de_ptr, e_ptr->sectors[0], 1
                        );
                    }
                    
                    //Delete it.
                    deleteEdge(e_ptr);
                    
                    break;
                }
            }
            
            //If it's matchless, that means it'll just be joined to
            //the group of edges on the destination vertex.
            if(!has_merged) {
                game.curAreaData->connectEdgeToVertex(
                    e_ptr, v2, (e_ptr->vertexes[0] == v1 ? 0 : 1)
                );
                for(size_t v2e = 0; v2e < v2->edges.size(); v2e++) {
                    affected_sectors->insert(v2->edges[v2e]->sectors[0]);
                    affected_sectors->insert(v2->edges[v2e]->sectors[1]);
                }
            }
        }
        
    }
    
    //Check if any of the final edges have the same sector
    //on both sides. If so, delete them.
    for(size_t ve = 0; ve < v2->edges.size(); ) {
        Edge* ve_ptr = v2->edges[ve];
        if(ve_ptr->sectors[0] == ve_ptr->sectors[1]) {
            deleteEdge(ve_ptr);
        } else {
            ve++;
        }
    }
    
    //Delete the old vertex.
    game.curAreaData->removeVertex(v1);
    
    //If any vertex or sector is out of edges, delete it.
    for(size_t v = 0; v < game.curAreaData->vertexes.size();) {
        Vertex* v_ptr = game.curAreaData->vertexes[v];
        if(v_ptr->edges.empty()) {
            game.curAreaData->removeVertex(v);
        } else {
            v++;
        }
    }
    for(size_t s = 0; s < game.curAreaData->sectors.size();) {
        Sector* s_ptr = game.curAreaData->sectors[s];
        if(s_ptr->edges.empty()) {
            game.curAreaData->removeSector(s);
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
    
    if(selectedEdges.empty()) {
        setStatus(
            "To paste edge properties, you must first select which edge "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("edge property paste");
    
    for(Edge* e : selectedEdges) {
        copyBufferEdge->clone(e);
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
    
    if(selectedMobs.empty()) {
        setStatus(
            "To paste object properties, you must first select which object "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("object property paste");
    
    for(MobGen* m : selectedMobs) {
        copyBufferMob->clone(m, false);
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
    
    if(selectedPathLinks.empty()) {
        setStatus(
            "To paste path link properties, you must first select which path "
            "link to paste to!",
            true
        );
        return;
    }
    
    registerChange("path link property paste");
    
    for(PathLink* l : selectedPathLinks) {
        copyBufferPathLink->clone(l);
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
    
    if(selectedSectors.empty()) {
        setStatus(
            "To paste sector properties, you must first select which sector "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("sector property paste");
    
    for(Sector* s : selectedSectors) {
        copyBufferSector->clone(s);
        updateSectorTexture(s, copyBufferSector->textureInfo.bmpName);
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
    
    if(selectedSectors.empty()) {
        setStatus(
            "To paste a sector texture, you must first select which sector "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("sector texture paste");
    
    for(Sector* s : selectedSectors) {
        updateSectorTexture(s, copyBufferSector->textureInfo.bmpName);
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
    for(size_t v = 0; v < game.curAreaData->vertexes.size(); v++) {
        Vertex* v_ptr = game.curAreaData->vertexes[v];
        v_ptr->x *= mults[0];
        v_ptr->y *= mults[1];
    }
    
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* s_ptr = game.curAreaData->sectors[s];
        s_ptr->textureInfo.scale.x *= mults[0];
        s_ptr->textureInfo.scale.y *= mults[1];
        triangulateSector(s_ptr, nullptr, false);
        s_ptr->calculateBoundingBox();
    }
    
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        m_ptr->pos.x *= mults[0];
        m_ptr->pos.y *= mults[1];
    }
    
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* s_ptr = game.curAreaData->pathStops[s];
        s_ptr->pos.x *= mults[0];
        s_ptr->pos.y *= mults[1];
    }
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        game.curAreaData->pathStops[s]->calculateDists();
    }
    
    for(size_t s = 0; s < game.curAreaData->treeShadows.size(); s++) {
        TreeShadow* s_ptr = game.curAreaData->treeShadows[s];
        s_ptr->center.x *= mults[0];
        s_ptr->center.y *= mults[1];
        s_ptr->size.x   *= mults[0];
        s_ptr->size.y   *= mults[1];
        s_ptr->sway.x   *= mults[0];
        s_ptr->sway.y   *= mults[1];
    }
    
    game.curAreaData->mission.goalExitCenter.x *= mults[0];
    game.curAreaData->mission.goalExitCenter.y *= mults[1];
    game.curAreaData->mission.goalExitSize.x *= mults[0];
    game.curAreaData->mission.goalExitSize.y *= mults[1];
}


/**
 * @brief Makes all currently selected mob generators (if any) rotate to
 * face where the the given point is.
 *
 * @param pos Point that the mobs must face.
 */
void AreaEditor::rotateMobGensToPoint(const Point &pos) {
    if(selectedMobs.empty()) {
        setStatus(
            "To rotate objects, you must first select some objects!",
            true
        );
        return;
    }
    
    registerChange("object rotation");
    selectionHomogenized = false;
    for(auto const &m : selectedMobs) {
        m->angle = getAngle(m->pos, pos);
    }
    setStatus("Rotated objects to face " + p2s(pos) + ".");
}


/**
 * @brief Snaps a point to the nearest available snapping space, based on the
 * current snap mode, Shift key state, and Ctrl key state.
 *
 * @param p Point to snap.
 * @param ignore_selected If true, ignore the selected vertexes or edges
 * when snapping to vertexes or edges.
 * @return The snapped point.
 */
Point AreaEditor::snapPoint(const Point &p, bool ignore_selected) {
    SNAP_MODE mode_to_use = game.options.areaEd.snapMode;
    Point final_point = p;
    
    if(isShiftPressed) {
        if(game.options.areaEd.snapMode == SNAP_MODE_NOTHING) {
            mode_to_use = SNAP_MODE_GRID;
        } else {
            mode_to_use = SNAP_MODE_NOTHING;
        }
    }
    
    if(isCtrlPressed) {
        if(curTransformationWidget.isMovingCenterHandle()) {
            final_point =
                snapPointToAxis(
                    final_point, curTransformationWidget.getOldCenter()
                );
        } else if(moving) {
            final_point =
                snapPointToAxis(final_point, moveStartPos);
        }
    }
    
    switch(mode_to_use) {
    case SNAP_MODE_GRID: {
        return
            snapPointToGrid(
                final_point,
                game.options.areaEd.gridInterval
            );
        break;
        
    } case SNAP_MODE_VERTEXES: {
        if(cursorSnapTimer.timeLeft > 0.0f) {
            return cursorSnapCache;
        }
        cursorSnapTimer.start();
        
        vector<Vertex*> vertexes_to_check = game.curAreaData->vertexes;
        if(ignore_selected) {
            for(const Vertex* v : selectedVertexes) {
                for(size_t v2 = 0; v2 < vertexes_to_check.size(); v2++) {
                    if(vertexes_to_check[v2] == v) {
                        vertexes_to_check.erase(vertexes_to_check.begin() + v2);
                        break;
                    }
                }
            }
        }
        vector<std::pair<Distance, Vertex*> > snappable_vertexes =
            getMergeVertexes(
                final_point, vertexes_to_check,
                game.options.areaEd.snapThreshold / game.view.cam.zoom
            );
        if(snappable_vertexes.empty()) {
            cursorSnapCache = final_point;
            return final_point;
        } else {
            sort(
                snappable_vertexes.begin(), snappable_vertexes.end(),
                [] (
                    std::pair<Distance, Vertex*> v1, std::pair<Distance, Vertex*> v2
            ) -> bool {
                return v1.first < v2.first;
            }
            );
            
            Point result(
                snappable_vertexes[0].second->x,
                snappable_vertexes[0].second->y
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
        
        Distance closest_dist;
        bool got_one = false;
        
        for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
            Edge* e_ptr = game.curAreaData->edges[e];
            float r;
            
            if(ignore_selected) {
                //Let's ignore not only the selected edge, but also
                //neighboring edges, because as we move an edge,
                //the neighboring edges stretch along with it.
                bool skip = false;
                for(Vertex* v : selectedVertexes) {
                    if(v->hasEdge(e_ptr)) {
                        skip = true;
                        break;
                    }
                }
                if(skip) continue;
            }
            
            Point edge_p =
                getClosestPointInLineSeg(
                    v2p(e_ptr->vertexes[0]), v2p(e_ptr->vertexes[1]),
                    final_point, &r
                );
                
            if(r < 0.0f) {
                edge_p = v2p(e_ptr->vertexes[0]);
            } else if(r > 1.0f) {
                edge_p = v2p(e_ptr->vertexes[1]);
            }
            
            Distance d(final_point, edge_p);
            if(d > game.options.areaEd.snapThreshold / game.view.cam.zoom) {
                continue;
            }
            
            if(!got_one || d < closest_dist) {
                got_one = true;
                closest_dist = d;
                final_point = edge_p;
            }
        }
        
        cursorSnapCache = final_point;
        return final_point;
        
        break;
        
    } case SNAP_MODE_NOTHING: {
    } case N_SNAP_MODES: {
        break;
        
    }
    }
    
    return final_point;
}


/**
 * @brief Splits an edge into two, near the specified point, and returns the
 * newly-created vertex. The new vertex gets added to the current area.
 *
 * @param e_ptr Edge to split.
 * @param where Point to split at.
 * @return The newly-created vertex.
 */
Vertex* AreaEditor::splitEdge(Edge* e_ptr, const Point &where) {
    Point new_v_pos =
        getClosestPointInLineSeg(
            v2p(e_ptr->vertexes[0]), v2p(e_ptr->vertexes[1]),
            where
        );
        
    //Create the new vertex and the new edge.
    Vertex* new_v_ptr = game.curAreaData->newVertex();
    new_v_ptr->x = new_v_pos.x;
    new_v_ptr->y = new_v_pos.y;
    Edge* new_e_ptr = game.curAreaData->newEdge();
    e_ptr->clone(new_e_ptr);
    
    //Connect the vertexes and edges.
    game.curAreaData->connectEdgeToVertex(new_e_ptr, new_v_ptr, 0);
    game.curAreaData->connectEdgeToVertex(new_e_ptr, e_ptr->vertexes[1], 1);
    game.curAreaData->connectEdgeToVertex(e_ptr, new_v_ptr, 1);
    
    //Connect the sectors and new edge.
    if(e_ptr->sectors[0]) {
        game.curAreaData->connectEdgeToSector(
            new_e_ptr, e_ptr->sectors[0], 0
        );
    }
    if(e_ptr->sectors[1]) {
        game.curAreaData->connectEdgeToSector(
            new_e_ptr, e_ptr->sectors[1], 1
        );
    }
    
    updateAllEdgeOffsetCaches();
    
    return new_v_ptr;
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
    PathLink* l1, PathLink* l2, const Point &where
) {
    bool normal_link = (l2 != nullptr);
    Point new_stop_pos =
        getClosestPointInLineSeg(
            l1->startPtr->pos, l1->endPtr->pos,
            where
        );
        
    //Create the new stop.
    PathStop* new_stop_ptr = new PathStop(new_stop_pos);
    game.curAreaData->pathStops.push_back(new_stop_ptr);
    
    //Delete the old links.
    PathStop* old_start_ptr = l1->startPtr;
    PathStop* old_end_ptr = l1->endPtr;
    PATH_LINK_TYPE old_link_type = l1->type;
    l1->startPtr->removeLink(l1->endPtr);
    if(normal_link) {
        l2->startPtr->removeLink(l2->endPtr);
    }
    
    //Create the new links.
    old_start_ptr->addLink(new_stop_ptr, normal_link);
    new_stop_ptr->addLink(old_end_ptr, normal_link);
    
    //Fix the dangling path stop numbers in the links, and other properties.
    game.curAreaData->fixPathStopIdxs(old_start_ptr);
    game.curAreaData->fixPathStopIdxs(old_end_ptr);
    game.curAreaData->fixPathStopIdxs(new_stop_ptr);
    
    old_start_ptr->get_link(new_stop_ptr)->type = old_link_type;
    new_stop_ptr->get_link(old_end_ptr)->type = old_link_type;
    if(normal_link) {
        new_stop_ptr->get_link(old_start_ptr)->type = old_link_type;
        old_end_ptr->get_link(new_stop_ptr)->type = old_link_type;
    }
    
    //Update the distances.
    new_stop_ptr->calculateDistsPlusNeighbors();
    
    return new_stop_ptr;
}


/**
 * @brief Updates the triangles and bounding box of the specified sectors, and
 * reports any errors found.
 *
 * @param affected_sectors The list of affected sectors.
 */
void AreaEditor::updateAffectedSectors(
    const unordered_set<Sector*> &affected_sectors
) {
    TRIANGULATION_ERROR last_triangulation_error = TRIANGULATION_ERROR_NONE;
    
    for(Sector* s_ptr : affected_sectors) {
        if(!s_ptr) continue;
        
        set<Edge*> triangulation_lone_edges;
        TRIANGULATION_ERROR triangulation_error =
            triangulateSector(s_ptr, &triangulation_lone_edges, true);
            
        if(triangulation_error == TRIANGULATION_ERROR_NONE) {
            auto it = game.curAreaData->problems.nonSimples.find(s_ptr);
            if(it != game.curAreaData->problems.nonSimples.end()) {
                game.curAreaData->problems.nonSimples.erase(it);
            }
        } else {
            game.curAreaData->problems.nonSimples[s_ptr] =
                triangulation_error;
            last_triangulation_error = triangulation_error;
        }
        game.curAreaData->problems.loneEdges.insert(
            triangulation_lone_edges.begin(),
            triangulation_lone_edges.end()
        );
        
        s_ptr->calculateBoundingBox();
    }
    
    if(last_triangulation_error != TRIANGULATION_ERROR_NONE) {
        emitTriangulationErrorStatusBarMessage(last_triangulation_error);
    }
    
    updateAllEdgeOffsetCaches();
}


/**
 * @brief When the user creates a new sector, which houses other sectors inside,
 * and these inner sectors need to know their outer sector changed.
 * This will go through a list of edges, check if they are inside
 * the new sector, and if so, update their outer sector.
 *
 * @param edges_to_check List of edges to check if they belong inside
 * the new sector or not.
 * @param old_outer What the old outer sector used to be.
 * @param new_outer What the new outer sector is.
 */
void AreaEditor::updateInnerSectorsOuterSector(
    const vector<Edge*> &edges_to_check,
    const Sector* old_outer, Sector* new_outer
) {
    for(size_t e = 0; e < edges_to_check.size(); e++) {
        Edge* e_ptr = edges_to_check[e];
        Vertex* v1_ptr = e_ptr->vertexes[0];
        Vertex* v2_ptr = e_ptr->vertexes[1];
        if(
            new_outer->isPointInSector(v2p(v1_ptr)) &&
            new_outer->isPointInSector(v2p(v2_ptr)) &&
            new_outer->isPointInSector(
                Point(
                    (v1_ptr->x + v2_ptr->x) / 2.0f,
                    (v1_ptr->y + v2_ptr->y) / 2.0f
                )
            )
        ) {
            for(size_t s = 0; s < 2; s++) {
                if(e_ptr->sectors[s] == old_outer) {
                    game.curAreaData->connectEdgeToSector(
                        e_ptr, new_outer, s
                    );
                    break;
                }
            }
        }
    }
}
