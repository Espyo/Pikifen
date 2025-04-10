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
    if(n1.on_sector || n2.on_sector) return false;
    
    if(n1.on_edge && n2.on_edge) {
        if(n1.on_edge != n2.on_edge) return false;
        
    } else if(n1.on_edge && n2.on_vertex) {
        if(
            n1.on_edge->vertexes[0] != n2.on_vertex &&
            n1.on_edge->vertexes[1] != n2.on_vertex
        ) {
            return false;
        }
        
    } else if(n1.on_vertex && n2.on_vertex) {
        if(!n1.on_vertex->getEdgeByNeighbor(n2.on_vertex)) {
            return false;
        }
        
    } else if(n1.on_vertex && n2.on_edge) {
        if(
            n2.on_edge->vertexes[0] != n1.on_vertex &&
            n2.on_edge->vertexes[1] != n1.on_vertex
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
    if(!show_path_preview) return 0;
    
    float d = 0;
    
    //We don't have a way to specify the invulnerabilities, since
    //hazards aren't saved to the sector data in the area editor.
    path_preview_result =
        getPath(
            path_preview_checkpoints[0],
            path_preview_checkpoints[1],
            path_preview_settings,
            path_preview, &d,
            &path_preview_closest[0], &path_preview_closest[1]
        );
        
    if(path_preview.empty() && d == 0) {
        d =
            Distance(
                path_preview_checkpoints[0],
                path_preview_checkpoints[1]
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
    drawing_line_result = DRAWING_LINE_RESULT_OK;
    
    if(drawing_nodes.empty()) {
        return;
    }
    
    LayoutDrawingNode* prev_node = &drawing_nodes.back();
    LayoutDrawingNode tentative_node(this, pos);
    
    //Check if the user hits a vertex or an edge, but the drawing is
    //meant to be a new sector shape.
    if(
        (!drawing_nodes[0].on_edge && !drawing_nodes[0].on_vertex) &&
        (tentative_node.on_edge || tentative_node.on_vertex)
    ) {
        drawing_line_result = DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX;
        return;
    }
    
    //Check if it's just hitting the same edge, or vertexes of the same edge.
    if(
        tentative_node.on_edge &&
        tentative_node.on_edge == prev_node->on_edge
    ) {
        drawing_line_result = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    if(
        tentative_node.on_vertex &&
        tentative_node.on_vertex->hasEdge(prev_node->on_edge)
    ) {
        drawing_line_result = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    if(
        prev_node->on_vertex &&
        prev_node->on_vertex->hasEdge(tentative_node.on_edge)
    ) {
        drawing_line_result = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    if(
        tentative_node.on_vertex &&
        tentative_node.on_vertex->isNeighbor(prev_node->on_vertex)
    ) {
        drawing_line_result = DRAWING_LINE_RESULT_ALONG_EDGE;
        return;
    }
    
    //Check for edge collisions in collinear lines.
    for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
        //We don't need to watch out for the edge of the current point
        //or the previous one, since this collinearity check doesn't
        //return true for line segments that touch in only one point.
        Edge* e_ptr = game.cur_area_data->edges[e];
        Point ep1 = v2p(e_ptr->vertexes[0]);
        Point ep2 = v2p(e_ptr->vertexes[1]);
        
        if(
            lineSegsAreCollinear(prev_node->snapped_spot, pos, ep1, ep2)
        ) {
            if(
                collinearLineSegsIntersect(
                    prev_node->snapped_spot, pos, ep1, ep2
                )
            ) {
                drawing_line_result = DRAWING_LINE_RESULT_ALONG_EDGE;
                return;
            }
        }
    }
    
    //Check for edge collisions.
    for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
        Edge* e_ptr = game.cur_area_data->edges[e];
        //If this edge is the same or a neighbor of the previous node,
        //then never mind.
        if(
            prev_node->on_edge == e_ptr ||
            tentative_node.on_edge == e_ptr
        ) {
            continue;
        }
        if(prev_node->on_vertex) {
            if(
                e_ptr->vertexes[0] == prev_node->on_vertex ||
                e_ptr->vertexes[1] == prev_node->on_vertex
            ) {
                continue;
            }
        }
        if(tentative_node.on_vertex) {
            if(
                e_ptr->vertexes[0] == tentative_node.on_vertex ||
                e_ptr->vertexes[1] == tentative_node.on_vertex
            ) {
                continue;
            }
        }
        
        if(
            lineSegsIntersect(
                prev_node->snapped_spot, pos,
                v2p(e_ptr->vertexes[0]), v2p(e_ptr->vertexes[1]),
                nullptr, nullptr
            )
        ) {
            drawing_line_result = DRAWING_LINE_RESULT_CROSSES_EDGES;
            return;
        }
    }
    
    //Check if the line intersects with the drawing's lines.
    if(drawing_nodes.size() >= 2) {
        for(size_t n = 0; n < drawing_nodes.size() - 2; n++) {
            LayoutDrawingNode* n1_ptr = &drawing_nodes[n];
            LayoutDrawingNode* n2_ptr = &drawing_nodes[n + 1];
            Point intersection;
            if(
                lineSegsIntersect(
                    prev_node->snapped_spot, pos,
                    n1_ptr->snapped_spot, n2_ptr->snapped_spot,
                    &intersection
                )
            ) {
                if(
                    Distance(intersection, drawing_nodes.begin()->snapped_spot) >
                    AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
                ) {
                    //Only a problem if this isn't the user's drawing finish.
                    drawing_line_result = DRAWING_LINE_RESULT_CROSSES_DRAWING;
                    return;
                }
            }
        }
        
        if(
            circleIntersectsLineSeg(
                pos, 8.0 / game.cam.zoom,
                prev_node->snapped_spot,
                drawing_nodes[drawing_nodes.size() - 2].snapped_spot
            )
        ) {
            drawing_line_result = DRAWING_LINE_RESULT_CROSSES_DRAWING;
            return;
        }
    }
    
    //Check if this line is entering a sector different from the one the
    //rest of the drawing is on.
    if(drawing_nodes.size() >= 2) {
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
                drawing_nodes[0].snapped_spot.x +
                drawing_nodes[1].snapped_spot.x
            ) / 2.0f,
            (
                drawing_nodes[0].snapped_spot.y +
                drawing_nodes[1].snapped_spot.y
            ) / 2.0f
        );
        Sector* working_sector = getSectorUnderPoint(working_sector_point);
        
        Point latest_sector_point(
            (
                drawing_nodes.back().snapped_spot.x +
                pos.x
            ) / 2.0f,
            (
                drawing_nodes.back().snapped_spot.y +
                pos.y
            ) / 2.0f
        );
        Sector* latest_sector = getSectorUnderPoint(latest_sector_point);
        
        if(latest_sector != working_sector) {
            drawing_line_result = DRAWING_LINE_RESULT_WAYWARD_SECTOR;
            return;
        }
    }
    
}


/**
 * @brief Copies the currently selected edge's properties onto the copy buffer,
 * so they can be then pasted onto another edge.
 */
void AreaEditor::copyEdgeProperties() {
    if(selected_edges.empty()) {
        setStatus(
            "To copy an edge's properties, you must first select an edge "
            "to copy from!",
            true
        );
        return;
    }
    
    if(selected_edges.size() > 1) {
        setStatus(
            "To copy an edge's properties, you can only select 1 edge!",
            true
        );
        return;
    }
    
    Edge* source_edge = *selected_edges.begin();
    if(!copy_buffer_edge) {
        copy_buffer_edge = new Edge();
    }
    source_edge->clone(copy_buffer_edge);
    setStatus("Successfully copied the edge's properties.");
    return;
}


/**
 * @brief Copies the currently selected mob's properties onto the copy buffer,
 * so they can be then pasted onto another mob.
 */
void AreaEditor::copyMobProperties() {
    if(selected_mobs.empty()) {
        setStatus(
            "To copy an object's properties, you must first select an object "
            "to copy from!",
            true
        );
        return;
    }
    
    if(selected_mobs.size() > 1) {
        setStatus(
            "To copy an object's properties, you can only select 1 object!",
            true
        );
        return;
    }
    
    MobGen* source_mob = *selected_mobs.begin();
    if(!copy_buffer_mob) {
        copy_buffer_mob = new MobGen();
    }
    source_mob->clone(copy_buffer_mob);
    setStatus("Successfully copied the object's properties.");
    return;
}


/**
 * @brief Copies the currently selected path link's properties onto the
 * copy buffer, so they can be then pasted onto another path link.
 *
 */
void AreaEditor::copyPathLinkProperties() {
    if(selected_path_links.empty()) {
        setStatus(
            "To copy a path link's properties, you must first select a path "
            "link to copy from!",
            true
        );
        return;
    }
    
    size_t really_selected_nr = selected_path_links.size();
    if(really_selected_nr == 2) {
        //Check if these are just the two sides of the same two-way link.
        //If so then yeah, we basically only have one link really selected.
        PathLink* l_ptr = *selected_path_links.begin();
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
    
    PathLink* source_link = *selected_path_links.begin();
    if(!copy_buffer_path_link) {
        copy_buffer_path_link = new PathLink(nullptr, nullptr, INVALID);
    }
    source_link->clone(copy_buffer_path_link);
    setStatus("Successfully copied the path link's properties.");
    return;
}


/**
 * @brief Copies the currently selected sector's properties onto the
 * copy buffer, so they can be then pasted onto another sector.
 *
 */
void AreaEditor::copySectorProperties() {
    if(selected_sectors.empty()) {
        setStatus(
            "To copy a sector's properties, you must first select a sector "
            "to copy from!",
            true
        );
        return;
    }
    
    if(selected_sectors.size() > 1) {
        setStatus(
            "To copy a sector's properties, you can only select 1 sector!",
            true
        );
        return;
    }
    
    Sector* source_sector = *selected_sectors.begin();
    if(!copy_buffer_sector) {
        copy_buffer_sector = new Sector();
    }
    source_sector->clone(copy_buffer_sector);
    copy_buffer_sector->texture_info = source_sector->texture_info;
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
    Sector* new_sector = game.cur_area_data->newSector();
    
    if(copy_from) {
        copy_from->clone(new_sector);
        updateSectorTexture(new_sector, copy_from->texture_info.bmp_name);
    } else {
        if(!texture_suggestions.empty()) {
            updateSectorTexture(new_sector, texture_suggestions[0].name);
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
            game.cur_area_data->removeSector(sectors[s]);
        }
    }
    
    //Now, remove vertexes.
    Vertex* vertexes[2] = { e_ptr->vertexes[0], e_ptr->vertexes[1] };
    e_ptr->removeFromVertexes();
    for(size_t v = 0; v < 2; v++) {
        if(vertexes[v]->edges.empty()) {
            game.cur_area_data->removeVertex(vertexes[v]);
        }
    }
    
    //Finally, delete the edge proper.
    game.cur_area_data->removeEdge(e_ptr);
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
        for(; m_idx < game.cur_area_data->mob_generators.size(); m_idx++) {
            if(game.cur_area_data->mob_generators[m_idx] == sm) break;
        }
        
        //Update links.
        for(
            size_t m2 = 0; m2 < game.cur_area_data->mob_generators.size(); m2++
        ) {
            MobGen* m2_ptr = game.cur_area_data->mob_generators[m2];
            
            for(size_t l = 0; l < m2_ptr->links.size(); l++) {
            
                if(m2_ptr->link_idxs[l] > m_idx) {
                    m2_ptr->link_idxs[l]--;
                }
                
                if(m2_ptr->links[l] == sm) {
                    m2_ptr->links.erase(m2_ptr->links.begin() + l);
                    m2_ptr->link_idxs.erase(m2_ptr->link_idxs.begin() + l);
                }
            }
            
            if(
                m2_ptr->stored_inside != INVALID &&
                m2_ptr->stored_inside > m_idx
            ) {
                m2_ptr->stored_inside--;
            } else if(m2_ptr->stored_inside == m_idx) {
                m2_ptr->stored_inside = INVALID;
            }
        }
        
        //Check the list of mission requirement objects.
        unordered_set<size_t> new_mrmi;
        new_mrmi.reserve(game.cur_area_data->mission.goal_mob_idxs.size());
        for(size_t m2 : game.cur_area_data->mission.goal_mob_idxs) {
            if(m2 > m_idx) {
                new_mrmi.insert(m2 - 1);
            } else if(m2 != m_idx) {
                new_mrmi.insert(m2);
            }
        }
        game.cur_area_data->mission.goal_mob_idxs = new_mrmi;
        
        //Finally, delete it.
        game.cur_area_data->mob_generators.erase(
            game.cur_area_data->mob_generators.begin() + m_idx
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
        l->start_ptr->removeLink(l);
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
        for(size_t s2 = 0; s2 < game.cur_area_data->path_stops.size(); s2++) {
            PathStop* s2_ptr = game.cur_area_data->path_stops[s2];
            s2_ptr->removeLink(s);
        }
        
        //Finally, delete the stop.
        delete s;
        for(size_t s2 = 0; s2 < game.cur_area_data->path_stops.size(); s2++) {
            if(game.cur_area_data->path_stops[s2] == s) {
                game.cur_area_data->path_stops.erase(
                    game.cur_area_data->path_stops.begin() + s2
                );
                break;
            }
        }
    }
    
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        game.cur_area_data->fixPathStopIdxs(game.cur_area_data->path_stops[s]);
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
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsOverlappingVertex();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsNonSimpleSector();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsLoneEdge();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsMissingLeader();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsTypelessMob();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsOobMob();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsMobInsideWalls();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsMobLinksToSelf();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsMobStoredInLoop();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsPikminOverLimit();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsBridgePath();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsOobPathStop();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsLonePathStop();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsPathStopOnLink();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsMissingTexture();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsUnknownTexture();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsPathStopsIntersecting();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsUnknownTreeShadow();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsNoGoalMob();
    if(problem_type != EPT_NONE_YET) return;
    
    findProblemsNoScoreCriteria();
    if(problem_type != EPT_NONE_YET) return;
    
    //All good!
    problem_type = EPT_NONE;
    problem_title = "None!";
    problem_description.clear();
}


/**
 * @brief Checks for any pile-to-bridge paths blocked by said bridge in the
 * area, and fills the problem info if so.
 */
void AreaEditor::findProblemsBridgePath() {
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
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
                    problem_mob_ptr = m_ptr->links[l];
                    problem_type = EPT_PILE_BRIDGE_PATH;
                    problem_title =
                        "Bridge is blocking the path to itself!";
                    problem_description =
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
        
        problem_edge_intersection = *intersections.begin();
        problem_type = EPT_INTERSECTING_EDGES;
        problem_title = "Two edges cross each other!";
        problem_description =
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
    if(!game.cur_area_data->problems.lone_edges.empty()) {
        problem_type = EPT_LONE_EDGE;
        problem_title = "Lone edge!";
        problem_description =
            "Likely leftover of something that went wrong. "
            "You probably want to drag one vertex into the other.";
    }
}


/**
 * @brief Checks for any lone path stops in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsLonePathStop() {
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        PathStop* s_ptr = game.cur_area_data->path_stops[s];
        bool has_link = false;
        
        if(!s_ptr->links.empty()) continue; //Duh, this means it has links.
        
        for(size_t s2 = 0; s2 < game.cur_area_data->path_stops.size(); s2++) {
            PathStop* s2_ptr = game.cur_area_data->path_stops[s2];
            if(s2_ptr == s_ptr) continue;
            
            if(s2_ptr->get_link(s_ptr)) {
                has_link = true;
                break;
            }
            
            if(has_link) break;
        }
        
        if(!has_link) {
            problem_path_stop_ptr = s_ptr;
            problem_type = EPT_LONE_PATH_STOP;
            problem_title = "Lone path stop!";
            problem_description =
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
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        if(
            game.cur_area_data->mob_generators[m]->type != nullptr &&
            game.cur_area_data->mob_generators[m]->type->category->id ==
            MOB_CATEGORY_LEADERS
        ) {
            has_leader = true;
            break;
        }
    }
    if(!has_leader) {
        problem_type = EPT_MISSING_LEADER;
        problem_title = "No leader!";
        problem_description =
            "You need at least one leader to actually play.";
    }
}


/**
 * @brief Checks for any missing texture in the area, and fills the problem
 * info if so.
 */
void AreaEditor::findProblemsMissingTexture() {
    for(size_t s = 0; s < game.cur_area_data->sectors.size(); s++) {
        Sector* s_ptr = game.cur_area_data->sectors[s];
        if(s_ptr->edges.empty()) continue;
        if(s_ptr->is_bottomless_pit) continue;
        if(
            s_ptr->texture_info.bmp_name.empty() &&
            !s_ptr->is_bottomless_pit && !s_ptr->fade
        ) {
            problem_sector_ptr = s_ptr;
            problem_type = EPT_UNKNOWN_TEXTURE;
            problem_title = "Sector with missing texture!";
            problem_description =
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
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
        if(!m_ptr->type) continue;
        
        if(
            m_ptr->type->category->id == MOB_CATEGORY_BRIDGES ||
            m_ptr->type->category->id == MOB_CATEGORY_DECORATIONS
        ) {
            continue;
        }
        
        for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
            Edge* e_ptr = game.cur_area_data->edges[e];
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
                    problem_mob_ptr = m_ptr;
                    problem_type = EPT_MOB_IN_WALL;
                    problem_title = "Mob stuck in wall!";
                    problem_description =
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
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
        for(size_t l = 0; l < m_ptr->links.size(); l++) {
            if(m_ptr->links[l] == m_ptr) {
                problem_mob_ptr = m_ptr;
                problem_type = EPT_MOB_LINKS_TO_SELF;
                problem_title = "Mob links to itself!";
                problem_description =
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
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
        if(m_ptr->stored_inside == INVALID) continue;
        unordered_set<MobGen*> visited_mobs;
        visited_mobs.insert(m_ptr);
        size_t next_idx = m_ptr->stored_inside;
        while(next_idx != INVALID) {
            MobGen* next_ptr = game.cur_area_data->mob_generators[next_idx];
            if(visited_mobs.find(next_ptr) != visited_mobs.end()) {
                problem_mob_ptr = next_ptr;
                problem_type = EPT_MOB_STORED_IN_LOOP;
                problem_title = "Mobs stored in a loop!";
                problem_description =
                    "This object is stored inside of another object, which "
                    "in turn is inside of another...and eventually, "
                    "one of the objects in this chain is stored inside of the "
                    "first one. This means none of these objects are "
                    "really out in the open, and so will never really be used "
                    "in the area. You probably want to unstore one of them.";
                return;
            }
            visited_mobs.insert(next_ptr);
            next_idx = next_ptr->stored_inside;
        }
    }
}


/**
 * @brief Checks for any missing mission goal mob in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsNoGoalMob() {
    if(
        game.cur_area_data->type == AREA_TYPE_MISSION &&
        (
            game.cur_area_data->mission.goal == MISSION_GOAL_COLLECT_TREASURE ||
            game.cur_area_data->mission.goal == MISSION_GOAL_BATTLE_ENEMIES ||
            game.cur_area_data->mission.goal == MISSION_GOAL_GET_TO_EXIT
        )
    ) {
        if(getMissionRequiredMobCount() == 0) {
            problem_type = EPT_NO_GOAL_MOBS;
            problem_title = "No mission goal mobs!";
            problem_description =
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
        game.cur_area_data->type == AREA_TYPE_MISSION &&
        game.cur_area_data->mission.grading_mode == MISSION_GRADING_MODE_POINTS
    ) {
        bool has_any_criterion = false;
        for(size_t c = 0; c < game.mission_score_criteria.size(); c++) {
            if(
                game.mission_score_criteria[c]->getMultiplier(
                    &game.cur_area_data->mission
                ) != 0
            ) {
                has_any_criterion = true;
                break;
            }
        }
        if(!has_any_criterion) {
            problem_type = EPT_NO_SCORE_CRITERIA;
            problem_title = "No active score criteria!";
            problem_description =
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
    if(!game.cur_area_data->problems.non_simples.empty()) {
        problem_type = EPT_BAD_SECTOR;
        problem_title = "Non-simple sector!";
        switch(game.cur_area_data->problems.non_simples.begin()->second) {
        case TRIANGULATION_ERROR_LONE_EDGES: {
            problem_description =
                "It contains lone edges. Try clearing them up.";
            break;
        } case TRIANGULATION_ERROR_NOT_CLOSED: {
            problem_description =
                "It is not closed. Try closing it.";
            break;
        } case TRIANGULATION_ERROR_NO_EARS: {
            problem_description =
                "There's been a triangulation error. Try undoing or "
                "deleting the sector, and then rebuild it. Make sure there "
                "are no gaps, and keep it simple.";
            break;
        } case TRIANGULATION_ERROR_INVALID_ARGS: {
            problem_description =
                "An unknown error has occured with the sector.";
            break;
        } case TRIANGULATION_ERROR_NONE: {
            problem_description.clear();
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
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
        if(!getSector(m_ptr->pos, nullptr, false)) {
            problem_mob_ptr = m_ptr;
            problem_type = EPT_MOB_OOB;
            problem_title = "Mob out of bounds!";
            problem_description =
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
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        PathStop* s_ptr = game.cur_area_data->path_stops[s];
        if(!getSector(s_ptr->pos, nullptr, false)) {
            problem_path_stop_ptr = s_ptr;
            problem_type = EPT_PATH_STOP_OOB;
            problem_title = "Path stop out of bounds!";
            problem_description =
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
    for(size_t v = 0; v < game.cur_area_data->vertexes.size(); v++) {
        Vertex* v1_ptr = game.cur_area_data->vertexes[v];
        
        for(size_t v2 = v + 1; v2 < game.cur_area_data->vertexes.size(); v2++) {
            Vertex* v2_ptr = game.cur_area_data->vertexes[v2];
            
            if(v1_ptr->x == v2_ptr->x && v1_ptr->y == v2_ptr->y) {
                problem_vertex_ptr = v1_ptr;
                problem_type = EPT_OVERLAPPING_VERTEXES;
                problem_title = "Overlapping vertexes!";
                problem_description =
                    "They are very close together at (" +
                    f2s(problem_vertex_ptr->x) + "," +
                    f2s(problem_vertex_ptr->y) + "), and should likely "
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
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        PathStop* s_ptr = game.cur_area_data->path_stops[s];
        for(size_t s2 = 0; s2 < game.cur_area_data->path_stops.size(); s2++) {
            PathStop* link_start_ptr = game.cur_area_data->path_stops[s2];
            if(link_start_ptr == s_ptr) continue;
            
            for(size_t l = 0; l < link_start_ptr->links.size(); l++) {
                PathStop* link_end_ptr = link_start_ptr->links[l]->end_ptr;
                if(link_end_ptr == s_ptr) continue;
                
                if(
                    circleIntersectsLineSeg(
                        s_ptr->pos, s_ptr->radius,
                        link_start_ptr->pos, link_end_ptr->pos
                    )
                ) {
                    problem_path_stop_ptr = s_ptr;
                    problem_type = EPT_PATH_STOP_ON_LINK;
                    problem_title = "Path stop on unrelated link!";
                    problem_description =
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
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        PathStop* s_ptr = game.cur_area_data->path_stops[s];
        for(size_t s2 = 0; s2 < game.cur_area_data->path_stops.size(); s2++) {
            PathStop* s2_ptr = game.cur_area_data->path_stops[s2];
            if(s2_ptr == s_ptr) continue;
            
            if(Distance(s_ptr->pos, s2_ptr->pos) <= 3.0) {
                problem_path_stop_ptr = s_ptr;
                problem_type = EPT_PATH_STOPS_TOGETHER;
                problem_title = "Two close path stops!";
                problem_description =
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
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
        if(m_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
            n_pikmin_mobs++;
            if(n_pikmin_mobs > game.config.rules.max_pikmin_in_field) {
                problem_type = EPT_PIKMIN_OVER_LIMIT;
                problem_title = "Over the Pikmin limit!";
                problem_description =
                    "There are more Pikmin in the area than the limit allows. "
                    "This means some of them will not appear. Current limit: "
                    + i2s(game.config.rules.max_pikmin_in_field) + ".";
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
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        if(!game.cur_area_data->mob_generators[m]->type) {
            problem_mob_ptr = game.cur_area_data->mob_generators[m];
            problem_type = EPT_TYPELESS_MOB;
            problem_title = "Mob with no type!";
            problem_description =
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
    for(size_t s = 0; s < game.cur_area_data->sectors.size(); s++) {
        Sector* s_ptr = game.cur_area_data->sectors[s];
        if(s_ptr->edges.empty()) continue;
        if(s_ptr->is_bottomless_pit) continue;
        
        if(s_ptr->texture_info.bmp_name.empty()) continue;
        
        const auto &texture_it =
            game.content.bitmaps.manifests.find(s_ptr->texture_info.bmp_name);
        if(texture_it == game.content.bitmaps.manifests.end()) {
            problem_sector_ptr = s_ptr;
            problem_type = EPT_UNKNOWN_TEXTURE;
            problem_title = "Sector with unknown texture!";
            problem_description =
                "Texture name: \"" + s_ptr->texture_info.bmp_name + "\".";
            return;
        }
    }
}


/**
 * @brief Checks for any unknown tree shadow texture in the area, and fills the
 * problem info if so.
 */
void AreaEditor::findProblemsUnknownTreeShadow() {
    for(size_t s = 0; s < game.cur_area_data->tree_shadows.size(); s++) {
        if(game.cur_area_data->tree_shadows[s]->bitmap == game.bmp_error) {
            problem_shadow_ptr = game.cur_area_data->tree_shadows[s];
            problem_type = EPT_UNKNOWN_SHADOW;
            problem_title = "Tree shadow with invalid texture!";
            problem_description =
                "Texture name: \"" +
                game.cur_area_data->tree_shadows[s]->bmp_name + "\".";
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
        v2p(e1_ptr->vertexes[0]),v2p(e1_ptr->vertexes[1]),
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
    for(size_t n = 0; n < drawing_nodes.size(); n++) {
        if(!drawing_nodes[n].on_vertex && !drawing_nodes[n].on_edge) {
            (*result) = drawing_nodes[n].on_sector;
            return true;
        }
    }
    
    //If none are on sectors, let's try the following:
    //Grab the first line that is not on top of an existing one,
    //and find the sector that line is on by checking its center.
    for(size_t n = 0; n < drawing_nodes.size(); n++) {
        const LayoutDrawingNode* n1 = &drawing_nodes[n];
        const LayoutDrawingNode* n2 = &(getNextInVector(drawing_nodes, n));
        if(!areNodesTraversable(*n1, *n2)) {
            *result =
                getSector(
                    (n1->snapped_spot + n2->snapped_spot) / 2,
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
    for(size_t n = 0; n < drawing_nodes.size(); n++) {
        if(drawing_nodes[n].on_vertex) {
            v.push_back(drawing_nodes[n].on_vertex);
        } else if(drawing_nodes[n].on_edge) {
            e.push_back(drawing_nodes[n].on_edge);
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
    
    for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
        Edge* e_ptr = game.cur_area_data->edges[e];
        if(e_ptr == after) {
            found_after = true;
            continue;
        } else if(!found_after) {
            continue;
        }
        
        if(!e_ptr->isValid()) continue;
        
        if(
            circleIntersectsLineSeg(
                p, 8 / game.cam.zoom,
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
    
    for(size_t e1 = 0; e1 < game.cur_area_data->edges.size(); e1++) {
        Edge* e1_ptr = game.cur_area_data->edges[e1];
        for(size_t e2 = e1 + 1; e2 < game.cur_area_data->edges.size(); e2++) {
            Edge* e2_ptr = game.cur_area_data->edges[e2];
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
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
        for(size_t l = 0; l < m_ptr->links.size(); l++) {
            MobGen* m2_ptr = m_ptr->links[l];
            if(
                circleIntersectsLineSeg(
                    p, 8 / game.cam.zoom, m_ptr->pos, m2_ptr->pos
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
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
        
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
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        PathStop* s_ptr = game.cur_area_data->path_stops[s];
        for(size_t l = 0; l < s_ptr->links.size(); l++) {
            PathStop* s2_ptr = s_ptr->links[l]->end_ptr;
            if(
                circleIntersectsLineSeg(
                    p, 8 / game.cam.zoom, s_ptr->pos, s2_ptr->pos
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
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        PathStop* s_ptr = game.cur_area_data->path_stops[s];
        
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
    for(size_t v = 0; v < game.cur_area_data->vertexes.size(); v++) {
        Vertex* v_ptr = game.cur_area_data->vertexes[v];
        
        if(
            rectanglesIntersect(
                p - (4 / game.cam.zoom),
                p + (4 / game.cam.zoom),
                Point(
                    v_ptr->x - (4 / game.cam.zoom),
                    v_ptr->y - (4 / game.cam.zoom)
                ),
                Point(
                    v_ptr->x + (4 / game.cam.zoom),
                    v_ptr->y + (4 / game.cam.zoom)
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
    if(selected_edges.size() < 2) return;
    
    Edge* base = *selected_edges.begin();
    for(auto e = selected_edges.begin(); e != selected_edges.end(); ++e) {
        if(e == selected_edges.begin()) continue;
        base->clone(*e);
    }
}


/**
 * @brief Homogenizes all selected mobs,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedMobs() {
    if(selected_mobs.size() < 2) return;
    
    MobGen* base = *selected_mobs.begin();
    for(auto m = selected_mobs.begin(); m != selected_mobs.end(); ++m) {
        if(m == selected_mobs.begin()) continue;
        base->clone(*m, false);
    }
}


/**
 * @brief Homogenizes all selected path links,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedPathLinks() {
    if(selected_path_links.size() < 2) return;
    
    PathLink* base = *selected_path_links.begin();
    for(
        auto l = selected_path_links.begin();
        l != selected_path_links.end();
        ++l
    ) {
        if(l == selected_path_links.begin()) continue;
        
        base->clone(*l);
    }
}


/**
 * @brief Homogenizes all selected path stops,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedPathStops() {
    if(selected_path_stops.size() < 2) return;
    
    PathStop* base = *selected_path_stops.begin();
    for(
        auto s = selected_path_stops.begin();
        s != selected_path_stops.end();
        ++s
    ) {
        if(s == selected_path_stops.begin()) continue;
        
        base->clone(*s);
    }
}


/**
 * @brief Homogenizes all selected sectors,
 * based on the one at the head of the selection.
 */
void AreaEditor::homogenizeSelectedSectors() {
    if(selected_sectors.size() < 2) return;
    
    Sector* base = *selected_sectors.begin();
    for(auto s = selected_sectors.begin(); s != selected_sectors.end(); ++s) {
        if(s == selected_sectors.begin()) continue;
        base->clone(*s);
        updateSectorTexture(*s, base->texture_info.bmp_name);
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
            game.cur_area_data->findSectorIdx(main_sector) :
            INVALID,
            game.cur_area_data->findEdgeIdx(e_ptr)
        );
    }
    
    //Delete the other ones.
    for(Edge* e_ptr : common_edges) {
        deleteEdge(e_ptr);
    }
    
    //Delete the now-merged sector.
    game.cur_area_data->removeSector(small_sector);
    
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
                        game.cur_area_data->connectEdgeToSector(
                            de_ptr, e_ptr->sectors[1], 0
                        );
                    } else if(e_ptr->sectors[0] == de_ptr->sectors[1]) {
                        game.cur_area_data->connectEdgeToSector(
                            de_ptr, e_ptr->sectors[1], 1
                        );
                    } else if(e_ptr->sectors[1] == de_ptr->sectors[0]) {
                        game.cur_area_data->connectEdgeToSector(
                            de_ptr, e_ptr->sectors[0], 0
                        );
                    } else if(e_ptr->sectors[1] == de_ptr->sectors[1]) {
                        game.cur_area_data->connectEdgeToSector(
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
                game.cur_area_data->connectEdgeToVertex(
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
    game.cur_area_data->removeVertex(v1);
    
    //If any vertex or sector is out of edges, delete it.
    for(size_t v = 0; v < game.cur_area_data->vertexes.size();) {
        Vertex* v_ptr = game.cur_area_data->vertexes[v];
        if(v_ptr->edges.empty()) {
            game.cur_area_data->removeVertex(v);
        } else {
            v++;
        }
    }
    for(size_t s = 0; s < game.cur_area_data->sectors.size();) {
        Sector* s_ptr = game.cur_area_data->sectors[s];
        if(s_ptr->edges.empty()) {
            game.cur_area_data->removeSector(s);
        } else {
            s++;
        }
    }
    
}


/**
 * @brief Pastes previously-copied edge properties onto the selected edges.
 */
void AreaEditor::pasteEdgeProperties() {
    if(!copy_buffer_edge) {
        setStatus(
            "To paste edge properties, you must first copy them "
            "from another one!",
            true
        );
        return;
    }
    
    if(selected_edges.empty()) {
        setStatus(
            "To paste edge properties, you must first select which edge "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("edge property paste");
    
    for(Edge* e : selected_edges) {
        copy_buffer_edge->clone(e);
    }
    
    updateAllEdgeOffsetCaches();
    
    setStatus("Successfully pasted edge properties.");
    return;
}


/**
 * @brief Pastes previously-copied mob properties onto the selected mobs.
 */
void AreaEditor::pasteMobProperties() {
    if(!copy_buffer_mob) {
        setStatus(
            "To paste object properties, you must first copy them "
            "from another one!",
            true
        );
        return;
    }
    
    if(selected_mobs.empty()) {
        setStatus(
            "To paste object properties, you must first select which object "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("object property paste");
    
    for(MobGen* m : selected_mobs) {
        copy_buffer_mob->clone(m, false);
    }
    
    setStatus("Successfully pasted object properties.");
    return;
}


/**
 * @brief Pastes previously-copied path link properties onto the selected
 * path links.
 */
void AreaEditor::pastePathLinkProperties() {
    if(!copy_buffer_path_link) {
        setStatus(
            "To paste path link properties, you must first copy them "
            "from another one!",
            true
        );
        return;
    }
    
    if(selected_path_links.empty()) {
        setStatus(
            "To paste path link properties, you must first select which path "
            "link to paste to!",
            true
        );
        return;
    }
    
    registerChange("path link property paste");
    
    for(PathLink* l : selected_path_links) {
        copy_buffer_path_link->clone(l);
    }
    
    setStatus("Successfully pasted path link properties.");
    return;
}


/**
 * @brief Pastes previously-copied sector properties onto the selected sectors.
 */
void AreaEditor::pasteSectorProperties() {
    if(!copy_buffer_sector) {
        setStatus(
            "To paste sector properties, you must first copy them "
            "from another one!",
            true
        );
        return;
    }
    
    if(selected_sectors.empty()) {
        setStatus(
            "To paste sector properties, you must first select which sector "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("sector property paste");
    
    for(Sector* s : selected_sectors) {
        copy_buffer_sector->clone(s);
        updateSectorTexture(s, copy_buffer_sector->texture_info.bmp_name);
    }
    
    updateAllEdgeOffsetCaches();
    
    setStatus("Successfully pasted sector properties.");
    return;
}


/**
 * @brief Pastes a previously-copied sector texture onto the selected sectors.
 */
void AreaEditor::pasteSectorTexture() {
    if(!copy_buffer_sector) {
        setStatus(
            "To paste a sector texture, you must first copy the properties "
            "from another one!",
            true
        );
        return;
    }
    
    if(selected_sectors.empty()) {
        setStatus(
            "To paste a sector texture, you must first select which sector "
            "to paste to!",
            true
        );
        return;
    }
    
    registerChange("sector texture paste");
    
    for(Sector* s : selected_sectors) {
        updateSectorTexture(s, copy_buffer_sector->texture_info.bmp_name);
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
    for(size_t v = 0; v < game.cur_area_data->vertexes.size(); v++) {
        Vertex* v_ptr = game.cur_area_data->vertexes[v];
        v_ptr->x *= mults[0];
        v_ptr->y *= mults[1];
    }
    
    for(size_t s = 0; s < game.cur_area_data->sectors.size(); s++) {
        Sector* s_ptr = game.cur_area_data->sectors[s];
        s_ptr->texture_info.scale.x *= mults[0];
        s_ptr->texture_info.scale.y *= mults[1];
        triangulateSector(s_ptr, nullptr, false);
        s_ptr->calculateBoundingBox();
    }
    
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
        m_ptr->pos.x *= mults[0];
        m_ptr->pos.y *= mults[1];
    }
    
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        PathStop* s_ptr = game.cur_area_data->path_stops[s];
        s_ptr->pos.x *= mults[0];
        s_ptr->pos.y *= mults[1];
    }
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        game.cur_area_data->path_stops[s]->calculateDists();
    }
    
    for(size_t s = 0; s < game.cur_area_data->tree_shadows.size(); s++) {
        TreeShadow* s_ptr = game.cur_area_data->tree_shadows[s];
        s_ptr->center.x *= mults[0];
        s_ptr->center.y *= mults[1];
        s_ptr->size.x   *= mults[0];
        s_ptr->size.y   *= mults[1];
        s_ptr->sway.x   *= mults[0];
        s_ptr->sway.y   *= mults[1];
    }
    
    game.cur_area_data->mission.goal_exit_center.x *= mults[0];
    game.cur_area_data->mission.goal_exit_center.y *= mults[1];
    game.cur_area_data->mission.goal_exit_size.x *= mults[0];
    game.cur_area_data->mission.goal_exit_size.y *= mults[1];
}


/**
 * @brief Makes all currently selected mob generators (if any) rotate to
 * face where the the given point is.
 *
 * @param pos Point that the mobs must face.
 */
void AreaEditor::rotateMobGensToPoint(const Point &pos) {
    if(selected_mobs.empty()) {
        setStatus(
            "To rotate objects, you must first select some objects!",
            true
        );
        return;
    }
    
    registerChange("object rotation");
    selection_homogenized = false;
    for(auto const &m : selected_mobs) {
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
    SNAP_MODE mode_to_use = game.options.area_editor.snap_mode;
    Point final_point = p;
    
    if(is_shift_pressed) {
        if(game.options.area_editor.snap_mode == SNAP_MODE_NOTHING) {
            mode_to_use = SNAP_MODE_GRID;
        } else {
            mode_to_use = SNAP_MODE_NOTHING;
        }
    }
    
    if(is_ctrl_pressed) {
        if(cur_transformation_widget.isMovingCenterHandle()) {
            final_point =
                snapPointToAxis(
                    final_point, cur_transformation_widget.getOldCenter()
                );
        } else if(moving) {
            final_point =
                snapPointToAxis(final_point, move_start_pos);
        }
    }
    
    switch(mode_to_use) {
    case SNAP_MODE_GRID: {
        return
            snapPointToGrid(
                final_point,
                game.options.area_editor.grid_interval
            );
        break;
        
    } case SNAP_MODE_VERTEXES: {
        if(cursor_snap_timer.time_left > 0.0f) {
            return cursor_snap_cache;
        }
        cursor_snap_timer.start();
        
        vector<Vertex*> vertexes_to_check = game.cur_area_data->vertexes;
        if(ignore_selected) {
            for(const Vertex* v : selected_vertexes) {
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
                game.options.area_editor.snap_threshold / game.cam.zoom
            );
        if(snappable_vertexes.empty()) {
            cursor_snap_cache = final_point;
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
            cursor_snap_cache = result;
            return result;
        }
        
        break;
        
    } case SNAP_MODE_EDGES: {
        if(cursor_snap_timer.time_left > 0.0f) {
            return cursor_snap_cache;
        }
        cursor_snap_timer.start();
        
        Distance closest_dist;
        bool got_one = false;
        
        for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
            Edge* e_ptr = game.cur_area_data->edges[e];
            float r;
            
            if(ignore_selected) {
                //Let's ignore not only the selected edge, but also
                //neighboring edges, because as we move an edge,
                //the neighboring edges stretch along with it.
                bool skip = false;
                for(Vertex* v : selected_vertexes) {
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
            if(d > game.options.area_editor.snap_threshold / game.cam.zoom) {
                continue;
            }
            
            if(!got_one || d < closest_dist) {
                got_one = true;
                closest_dist = d;
                final_point = edge_p;
            }
        }
        
        cursor_snap_cache = final_point;
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
    Vertex* new_v_ptr = game.cur_area_data->newVertex();
    new_v_ptr->x = new_v_pos.x;
    new_v_ptr->y = new_v_pos.y;
    Edge* new_e_ptr = game.cur_area_data->newEdge();
    e_ptr->clone(new_e_ptr);
    
    //Connect the vertexes and edges.
    game.cur_area_data->connectEdgeToVertex(new_e_ptr, new_v_ptr, 0);
    game.cur_area_data->connectEdgeToVertex(new_e_ptr, e_ptr->vertexes[1], 1);
    game.cur_area_data->connectEdgeToVertex(e_ptr, new_v_ptr, 1);
    
    //Connect the sectors and new edge.
    if(e_ptr->sectors[0]) {
        game.cur_area_data->connectEdgeToSector(
            new_e_ptr, e_ptr->sectors[0], 0
        );
    }
    if(e_ptr->sectors[1]) {
        game.cur_area_data->connectEdgeToSector(
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
            l1->start_ptr->pos, l1->end_ptr->pos,
            where
        );
        
    //Create the new stop.
    PathStop* new_stop_ptr = new PathStop(new_stop_pos);
    game.cur_area_data->path_stops.push_back(new_stop_ptr);
    
    //Delete the old links.
    PathStop* old_start_ptr = l1->start_ptr;
    PathStop* old_end_ptr = l1->end_ptr;
    PATH_LINK_TYPE old_link_type = l1->type;
    l1->start_ptr->removeLink(l1->end_ptr);
    if(normal_link) {
        l2->start_ptr->removeLink(l2->end_ptr);
    }
    
    //Create the new links.
    old_start_ptr->addLink(new_stop_ptr, normal_link);
    new_stop_ptr->addLink(old_end_ptr, normal_link);
    
    //Fix the dangling path stop numbers in the links, and other properties.
    game.cur_area_data->fixPathStopIdxs(old_start_ptr);
    game.cur_area_data->fixPathStopIdxs(old_end_ptr);
    game.cur_area_data->fixPathStopIdxs(new_stop_ptr);
    
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
            auto it = game.cur_area_data->problems.non_simples.find(s_ptr);
            if(it != game.cur_area_data->problems.non_simples.end()) {
                game.cur_area_data->problems.non_simples.erase(it);
            }
        } else {
            game.cur_area_data->problems.non_simples[s_ptr] =
                triangulation_error;
            last_triangulation_error = triangulation_error;
        }
        game.cur_area_data->problems.lone_edges.insert(
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
                    game.cur_area_data->connectEdgeToSector(
                        e_ptr, new_outer, s
                    );
                    break;
                }
            }
        }
    }
}
