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

#include "../../functions.h"
#include "../../game.h"
#include "../../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Checks whether it's possible to traverse from drawing node n1 to n2
 * with the existing edges and vertexes. In other words, if you draw a line
 * between n1 and n2, it will not go inside a sector.
 * n1:
 *   Starting drawing node.
 * n2:
 *   Ending drawing node.
 */
bool area_editor::are_nodes_traversable(
    const layout_drawing_node &n1, const layout_drawing_node &n2
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
        if(!n1.on_vertex->get_edge_by_neighbor(n2.on_vertex)) {
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


/* ----------------------------------------------------------------------------
 * Calculates the preview path.
 */
float area_editor::calculate_preview_path() {
    if(!show_path_preview) return 0;
    
    float d = 0;
    path_preview =
        get_path(
            path_preview_checkpoints[0],
            path_preview_checkpoints[1],
            NULL, &d
        );
        
    if(path_preview.empty() && d == 0) {
        d =
            dist(
                path_preview_checkpoints[0],
                path_preview_checkpoints[1]
            ).to_float();
    }
    
    return d;
}


/* ----------------------------------------------------------------------------
 * Checks if the line the user is trying to draw is okay. Sets the line's status
 * to drawing_line_error.
 * pos:
 *   Position the user is trying to finish the line on.
 */
void area_editor::check_drawing_line(const point &pos) {
    drawing_line_error = DRAWING_LINE_NO_ERROR;
    
    if(drawing_nodes.empty()) {
        return;
    }
    
    layout_drawing_node* prev_node = &drawing_nodes.back();
    layout_drawing_node tentative_node(this, pos);
    
    //Check if the user is trying to close a loop, but the drawing is meant
    //to be a split between two sectors.
    if(
        (drawing_nodes[0].on_edge || drawing_nodes[0].on_vertex) &&
        dist(pos, drawing_nodes[0].snapped_spot) <=
        VERTEX_MERGE_RADIUS / game.cam.zoom
    ) {
        drawing_line_error = DRAWING_LINE_LOOPS_IN_SPLIT;
        return;
    }
    
    //Check if the user hits a vertex or an edge, but the drawing is
    //meant to be a new sector shape.
    if(
        (!drawing_nodes[0].on_edge && !drawing_nodes[0].on_vertex) &&
        (tentative_node.on_edge || tentative_node.on_vertex)
    ) {
        drawing_line_error = DRAWING_LINE_HIT_EDGE_OR_VERTEX;
        return;
    }
    
    //Check if it's just hitting the same edge, or vertexes of the same edge.
    if(
        tentative_node.on_edge &&
        tentative_node.on_edge == prev_node->on_edge
    ) {
        drawing_line_error = DRAWING_LINE_ALONG_EDGE;
        return;
    }
    if(
        tentative_node.on_vertex &&
        tentative_node.on_vertex->has_edge(prev_node->on_edge)
    ) {
        drawing_line_error = DRAWING_LINE_ALONG_EDGE;
        return;
    }
    if(
        prev_node->on_vertex &&
        prev_node->on_vertex->has_edge(tentative_node.on_edge)
    ) {
        drawing_line_error = DRAWING_LINE_ALONG_EDGE;
        return;
    }
    if(
        tentative_node.on_vertex &&
        tentative_node.on_vertex->is_neighbor(prev_node->on_vertex)
    ) {
        drawing_line_error = DRAWING_LINE_ALONG_EDGE;
        return;
    }
    
    //Check of edge collisions in collinear lines.
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        //We don't need to watch out for the edge of the current point
        //or the previous one, since this collinearity check doesn't
        //return true for line segments that touch in only one point.
        edge* e_ptr = game.cur_area_data.edges[e];
        point ep1(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y);
        point ep2(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y);
        
        if(lines_are_collinear(prev_node->snapped_spot, pos, ep1, ep2)) {
            if(
                collinear_lines_intersect(
                    prev_node->snapped_spot, pos, ep1, ep2
                )
            ) {
                drawing_line_error = DRAWING_LINE_ALONG_EDGE;
                return;
            }
        }
    }
    
    //Check for edge collisions.
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        edge* e_ptr = game.cur_area_data.edges[e];
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
            lines_intersect(
                prev_node->snapped_spot, pos,
                point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
                point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y),
                NULL, NULL
            )
        ) {
            drawing_line_error = DRAWING_LINE_CROSSES_EDGES;
            return;
        }
    }
    
    //Check if the line intersects with the drawing's lines.
    if(drawing_nodes.size() >= 2) {
        for(size_t n = 0; n < drawing_nodes.size() - 2; ++n) {
            layout_drawing_node* n1_ptr = &drawing_nodes[n];
            layout_drawing_node* n2_ptr = &drawing_nodes[n + 1];
            point intersection;
            if(
                lines_intersect(
                    prev_node->snapped_spot, pos,
                    n1_ptr->snapped_spot, n2_ptr->snapped_spot,
                    &intersection
                )
            ) {
                if(
                    dist(intersection, drawing_nodes.begin()->snapped_spot) >
                    VERTEX_MERGE_RADIUS / game.cam.zoom
                ) {
                    //Only a problem if this isn't the user's drawing finish.
                    drawing_line_error = DRAWING_LINE_CROSSES_DRAWING;
                    return;
                }
            }
        }
        
        if(
            circle_intersects_line(
                pos, 8.0 / game.cam.zoom,
                prev_node->snapped_spot,
                drawing_nodes[drawing_nodes.size() - 2].snapped_spot
            )
        ) {
            drawing_line_error = DRAWING_LINE_CROSSES_DRAWING;
            return;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new sector for use in layout drawing operations.
 * This automatically clones it from another sector, if not NULL, or gives it
 * a recommended texture if the other sector NULL.
 * copy_from:
 *   Sector to copy from.
 */
sector* area_editor::create_sector_for_layout_drawing(sector* copy_from) {
    sector* new_sector = game.cur_area_data.new_sector();
    
    if(copy_from) {
        copy_from->clone(new_sector);
        update_sector_texture(new_sector, copy_from->texture_info.file_name);
    } else {
        if(!texture_suggestions.empty()) {
            update_sector_texture(new_sector, texture_suggestions[0].name);
        } else {
            update_sector_texture(new_sector, "");
        }
    }
    
    return new_sector;
}


/* ----------------------------------------------------------------------------
 * Deletes the specified edge, removing it from all sectors and vertexes that
 * use it, as well as removing any now-useless sectors or vertexes.
 * e_ptr:
 *   Edge to delete.
 */
void area_editor::delete_edge(edge* e_ptr) {
    //Remove sectors first.
    sector* sectors[2] = { e_ptr->sectors[0], e_ptr->sectors[1] };
    e_ptr->remove_from_sectors();
    for(size_t s = 0; s < 2; ++s) {
        if(!sectors[s]) continue;
        if(sectors[s]->edges.empty()) {
            game.cur_area_data.remove_sector(sectors[s]);
        }
    }
    
    //Now, remove vertexes.
    vertex* vertexes[2] = { e_ptr->vertexes[0], e_ptr->vertexes[1] };
    e_ptr->remove_from_vertexes();
    for(size_t v = 0; v < 2; ++v) {
        if(vertexes[v]->edges.empty()) {
            game.cur_area_data.remove_vertex(vertexes[v]);
        }
    }
    
    //Finally, delete the edge proper.
    game.cur_area_data.remove_edge(e_ptr);
}


/* ----------------------------------------------------------------------------
 * Deletes the specified edges. The sectors on each side of the edge
 * are merged, so the smallest sector will be deleted. In addition,
 * this operation will delete any sectors that would end up incomplete.
 * Returns false if one of the edges couldn't be deleted.
 * which:
 *   Edges to delete.
 */
bool area_editor::delete_edges(const set<edge*> &which) {
    bool ret = true;
    
    for(edge* e_ptr : which) {
        if(!e_ptr->vertexes[0]) {
            //Huh, looks like one of the edge deletion procedures already
            //wiped this edge out. Skip it.
            continue;
        }
        if(!merge_sectors(e_ptr->sectors[0], e_ptr->sectors[1])) {
            ret = false;
        }
    }
    
    return ret;
}


/* ----------------------------------------------------------------------------
 * Deletes the specified mobs.
 * which:
 *   Mobs to delete.
 */
void area_editor::delete_mobs(const set<mob_gen*> &which) {
    for(auto sm : which) {
        size_t m_i = 0;
        for(; m_i < game.cur_area_data.mob_generators.size(); ++m_i) {
            if(game.cur_area_data.mob_generators[m_i] == sm) break;
        }
        
        //Check all links to this mob.
        for(
            size_t m2 = 0; m2 < game.cur_area_data.mob_generators.size(); ++m2
        ) {
            mob_gen* m2_ptr = game.cur_area_data.mob_generators[m2];
            for(size_t l = 0; l < m2_ptr->links.size(); ++l) {
            
                if(m2_ptr->link_nrs[l] > m_i) {
                    m2_ptr->link_nrs[l]--;
                }
                
                if(m2_ptr->links[l] == sm) {
                    m2_ptr->links.erase(m2_ptr->links.begin() + l);
                    m2_ptr->link_nrs.erase(m2_ptr->link_nrs.begin() + l);
                }
            }
        }
        
        game.cur_area_data.mob_generators.erase(
            game.cur_area_data.mob_generators.begin() + m_i
        );
        delete sm;
    }
}


/* ----------------------------------------------------------------------------
 * Deletes the specified path links.
 * which:
 *   Path links to delete.
 */
void area_editor::delete_path_links(
    const set<std::pair<path_stop*, path_stop*> > &which
) {
    for(auto l : which) {
        l.first->remove_link(l.second);
    }
}


/* ----------------------------------------------------------------------------
 * Deletes the specified path stops.
 * which:
 *   Path stops to delete.
 */
void area_editor::delete_path_stops(const set<path_stop*> &which) {
    for(auto s : which) {
        //Check all links to this stop.
        for(size_t s2 = 0; s2 < game.cur_area_data.path_stops.size(); ++s2) {
            path_stop* s2_ptr = game.cur_area_data.path_stops[s2];
            for(size_t l = 0; l < s2_ptr->links.size(); ++l) {
                if(s2_ptr->links[l].end_ptr == s) {
                    s2_ptr->links.erase(s2_ptr->links.begin() + l);
                    break;
                }
            }
        }
        
        //Finally, delete the stop.
        delete s;
        for(size_t s2 = 0; s2 < game.cur_area_data.path_stops.size(); ++s2) {
            if(game.cur_area_data.path_stops[s2] == s) {
                game.cur_area_data.path_stops.erase(
                    game.cur_area_data.path_stops.begin() + s2
                );
                break;
            }
        }
    }
    
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        game.cur_area_data.fix_path_stop_nrs(game.cur_area_data.path_stops[s]);
    }
}


/* ----------------------------------------------------------------------------
 * Tries to find problems with the area.
 * When it's done, sets the appropriate problem-related variables.
 */
void area_editor::find_problems() {
    problem_sector_ptr = NULL;
    problem_vertex_ptr = NULL;
    problem_shadow_ptr = NULL;
    
    //Check intersecting edges.
    vector<edge_intersection> intersections = get_intersecting_edges();
    if(!intersections.empty()) {
        float u;
        edge_intersection* ei_ptr = &(*intersections.begin());
        lines_intersect(
            point(ei_ptr->e1->vertexes[0]->x, ei_ptr->e1->vertexes[0]->y),
            point(ei_ptr->e1->vertexes[1]->x, ei_ptr->e1->vertexes[1]->y),
            point(ei_ptr->e2->vertexes[0]->x, ei_ptr->e2->vertexes[0]->y),
            point(ei_ptr->e2->vertexes[1]->x, ei_ptr->e2->vertexes[1]->y),
            NULL, &u
        );
        
        float a =
            get_angle(
                point(ei_ptr->e1->vertexes[0]->x, ei_ptr->e1->vertexes[0]->y),
                point(ei_ptr->e1->vertexes[1]->x, ei_ptr->e1->vertexes[1]->y)
            );
        dist d(
            point(ei_ptr->e1->vertexes[0]->x, ei_ptr->e1->vertexes[0]->y),
            point(ei_ptr->e1->vertexes[1]->x, ei_ptr->e1->vertexes[1]->y)
        );
        
        problem_edge_intersection = *intersections.begin();
        problem_type = EPT_INTERSECTING_EDGES;
        problem_title = "Two edges cross each other!";
        problem_description =
            "They cross at (" +
            f2s(
                floor(ei_ptr->e1->vertexes[0]->x + cos(a) * u *
                      d.to_float())
            ) + "," + f2s(
                floor(ei_ptr->e1->vertexes[0]->y + sin(a) * u *
                      d.to_float())
            ) + "). Edges should never cross each other.";
        return;
    }
    
    //Check overlapping vertexes.
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); ++v) {
        vertex* v1_ptr = game.cur_area_data.vertexes[v];
        
        for(size_t v2 = v + 1; v2 < game.cur_area_data.vertexes.size(); ++v2) {
            vertex* v2_ptr = game.cur_area_data.vertexes[v2];
            
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
    
    //Check non-simple sectors.
    if(!game.cur_area_data.problems.non_simples.empty()) {
        problem_type = EPT_BAD_SECTOR;
        problem_title = "Non-simple sector!";
        switch(game.cur_area_data.problems.non_simples.begin()->second) {
        case TRIANGULATION_ERROR_LONE_EDGES: {
            problem_description =
                "It contains lone edges. Try clearing them up.";
            break;
        } case TRIANGULATION_ERROR_NO_EARS: {
            problem_description =
                "There's been a triangulation error. Try undoing or "
                "deleting the sector, and then rebuild it. Make sure there "
                "are no gaps, and keep it simple.";
            break;
        } case TRIANGULATION_ERROR_VERTEXES_REUSED: {
            problem_description =
                "Some vertexes are re-used. Make sure the sector "
                "has no loops or that the same vertex is not re-used "
                "by multiple edges of the sector. Split popular vertexes "
                "if you must.";
            break;
        } case TRIANGULATION_ERROR_INVALID_ARGS: {
            problem_description =
                "An unknown error has occured with the sector.";
            break;
        } case TRIANGULATION_NO_ERROR: {
            problem_description.clear();
            break;
        }
        }
        return;
    }
    
    //Check lone edges.
    if(!game.cur_area_data.problems.lone_edges.empty()) {
        problem_type = EPT_LONE_EDGE;
        problem_title = "Lone edge!";
        problem_description =
            "Likely leftover of something that went wrong. "
            "You probably want to drag one vertex into the other.";
        return;
    }
    
    //Check for the existence of a leader object.
    bool has_leader = false;
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        if(
            game.cur_area_data.mob_generators[m]->category->id ==
            MOB_CATEGORY_LEADERS &&
            game.cur_area_data.mob_generators[m]->type != NULL
        ) {
            has_leader = true;
            break;
        }
    }
    if(!has_leader) {
        problem_type = EPT_MISSING_LEADER;
        problem_title = "No leader!";
        problem_description =
            "You need at least one leader to play.";
        return;
    }
    
    //Objects with no type.
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        if(!game.cur_area_data.mob_generators[m]->type) {
            problem_mob_ptr = game.cur_area_data.mob_generators[m];
            problem_type = EPT_TYPELESS_MOB;
            problem_title = "Mob with no type!";
            problem_description =
                "It has a category set, but no valid type set. "
                "Give it a type or delete it.";
            return;
        }
    }
    
    //Objects out of bounds.
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        if(!get_sector(m_ptr->pos, NULL, false)) {
            problem_mob_ptr = m_ptr;
            problem_type = EPT_MOB_OOB;
            problem_title = "Mob out of bounds!";
            problem_description =
                "Move it to somewhere inside the area's geometry.";
            return;
        }
    }
    
    //Objects inside walls.
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        
        if(
            m_ptr->category->id == MOB_CATEGORY_BRIDGES
        ) {
            continue;
        }
        
        for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
            edge* e_ptr = game.cur_area_data.edges[e];
            if(!e_ptr->is_valid()) continue;
            
            if(
                circle_intersects_line(
                    m_ptr->pos,
                    m_ptr->type->radius,
                    point(
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                    ),
                    point(
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    ),
                    NULL, NULL
                )
            ) {
            
                if(
                    e_ptr->sectors[0] && e_ptr->sectors[1] &&
                    e_ptr->sectors[0]->z == e_ptr->sectors[1]->z
                ) {
                    continue;
                }
                
                sector* mob_sector = get_sector(m_ptr->pos, NULL, false);
                
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
                    e_ptr->sectors[1]->z > mob_sector->z
                ) {
                    in_wall = true;
                    
                } else if(
                    e_ptr->sectors[1] == mob_sector &&
                    e_ptr->sectors[0]->z > mob_sector->z
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
    
    //Bridge mob that is not on a bridge sector.
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        if(m_ptr->category->id == MOB_CATEGORY_BRIDGES) {
            sector* s_ptr = get_sector(m_ptr->pos, NULL, false);
            if(s_ptr->type != SECTOR_TYPE_BRIDGE) {
                problem_mob_ptr = m_ptr;
                problem_type = EPT_SECTORLESS_BRIDGE;
                problem_title = "Bridge mob on wrong sector!";
                problem_description =
                    "This bridge object should be on a sector of the "
                    "\"Bridge\" type.";
                return;
            }
        }
    }
    
    //Path stops out of bounds.
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        if(!get_sector(s_ptr->pos, NULL, false)) {
            problem_path_stop_ptr = s_ptr;
            problem_type = EPT_PATH_STOP_OOB;
            problem_title = "Path stop out of bounds!";
            problem_description =
                "Move it to somewhere inside the area's geometry.";
            return;
        }
    }
    
    //Lone path stops.
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        bool has_link = false;
        
        if(!s_ptr->links.empty()) continue; //Duh, this means it has links.
        
        for(size_t s2 = 0; s2 < game.cur_area_data.path_stops.size(); ++s2) {
            path_stop* s2_ptr = game.cur_area_data.path_stops[s2];
            if(s2_ptr == s_ptr) continue;
            
            if(s2_ptr->get_link(s_ptr) != INVALID) {
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
    
    //Path graph is not connected.
    if(!game.cur_area_data.path_stops.empty()) {
        unordered_set<path_stop*> visited;
        depth_first_search(
            game.cur_area_data.path_stops,
            visited,
            game.cur_area_data.path_stops[0]
        );
        if(visited.size() != game.cur_area_data.path_stops.size()) {
            problem_type = EPT_PATHS_UNCONNECTED;
            problem_title = "Path split into multiple parts!";
            problem_description =
                "The path graph is split into two or more parts. Connect them.";
            return;
        }
    }
    
    //Check for missing textures.
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
    
        sector* s_ptr = game.cur_area_data.sectors[s];
        if(s_ptr->edges.empty()) continue;
        if(s_ptr->is_bottomless_pit) continue;
        if(
            s_ptr->texture_info.file_name.empty() &&
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
    
    //Check for unknown textures.
    vector<string> texture_file_names =
        folder_to_vector(TEXTURES_FOLDER_PATH, false);
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
    
        sector* s_ptr = game.cur_area_data.sectors[s];
        if(s_ptr->edges.empty()) continue;
        if(s_ptr->is_bottomless_pit) continue;
        
        if(s_ptr->texture_info.file_name.empty()) continue;
        
        if(
            std::find(
                texture_file_names.begin(), texture_file_names.end(),
                s_ptr->texture_info.file_name
            ) == texture_file_names.end()
        ) {
            problem_sector_ptr = s_ptr;
            problem_type = EPT_UNKNOWN_TEXTURE;
            problem_title = "Sector with unknown texture!";
            problem_description =
                "Texture name: \"" + s_ptr->texture_info.file_name + "\".";
            return;
        }
    }
    
    //Two stops intersecting.
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        for(size_t s2 = 0; s2 < game.cur_area_data.path_stops.size(); ++s2) {
            path_stop* s2_ptr = game.cur_area_data.path_stops[s2];
            if(s2_ptr == s_ptr) continue;
            
            if(dist(s_ptr->pos, s2_ptr->pos) <= 3.0) {
                problem_path_stop_ptr = s_ptr;
                problem_type = EPT_PATH_STOPS_TOGETHER;
                problem_title = "Two close path stops!";
                problem_description =
                    "These two are very close together. Separate them.";
                return;
            }
        }
    }
    
    //Check if there are tree shadows with invalid images.
    for(size_t s = 0; s < game.cur_area_data.tree_shadows.size(); ++s) {
        if(game.cur_area_data.tree_shadows[s]->bitmap == game.bmp_error) {
            problem_shadow_ptr = game.cur_area_data.tree_shadows[s];
            problem_type = EPT_UNKNOWN_SHADOW;
            problem_title = "Tree shadow with invalid texture!";
            problem_description =
                "Texture name: \"" +
                game.cur_area_data.tree_shadows[s]->file_name + "\".";
            return;
        }
    }
    
    //All good!
    problem_type = EPT_NONE;
    problem_title = "None!";
    problem_description.clear();
}


/* ----------------------------------------------------------------------------
 * Adds to the list all sectors affected by the specified sector.
 * The list can include the NULL sector, and will include the
 * provided sector too.
 * s_ptr:
 *   Sector that's affecting others.
 * list:
 *   The list of affected sectors to fill out.
 */
void area_editor::get_affected_sectors(
    sector* s_ptr, unordered_set<sector*> &list
) const {
    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
        list.insert(s_ptr->edges[e]->sectors[0]);
        list.insert(s_ptr->edges[e]->sectors[1]);
    }
}


/* ----------------------------------------------------------------------------
 * Adds to the list all sectors affected by the specified sectors.
 * The list can include the NULL sector, and will include the
 * provided sectors too.
 * sectors:
 *   Sectors that are affecting others.
 * list:
 *   The list of affected sectors to fill out.
 */
void area_editor::get_affected_sectors(
    set<sector*> &sectors, unordered_set<sector*> &list
) const {
    for(auto s : sectors) {
        get_affected_sectors(s, list);
    }
}


/* ----------------------------------------------------------------------------
 * Adds to the list all sectors affected by the specified vertexes.
 * The list can include the NULL sector.
 * vertexes:
 *   Vertexes that are affecting sectors.
 * list:
 *   The list of affected sectors to fill out.
 */
void area_editor::get_affected_sectors(
    set<vertex*> &vertexes, unordered_set<sector*> &list
) const {
    for(auto v : vertexes) {
        for(size_t e = 0; e < v->edges.size(); ++e) {
            list.insert(v->edges[e]->sectors[0]);
            list.insert(v->edges[e]->sectors[1]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * For a given vertex, returns the edge closest to the given angle, in the
 * given direction.
 * v_ptr:
 *   Pointer to the vertex.
 * angle:
 *   Angle coming into the vertex.
 * clockwise:
 *   Return the closest edge clockwise?
 * closest_edge_angle:
 *   If not NULL, the angle the edge makes into its
 *   other vertex is returned here.
 */
edge* area_editor::get_closest_edge_to_angle(
    vertex* v_ptr, const float angle, const bool clockwise,
    float* closest_edge_angle
) const {
    edge* best_edge = NULL;
    float best_angle_diff = 0;
    float best_edge_angle = 0;
    
    for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
        edge* e_ptr = v_ptr->edges[e];
        vertex* other_v_ptr = e_ptr->get_other_vertex(v_ptr);
        
        float a =
            get_angle(
                point(v_ptr->x, v_ptr->y),
                point(other_v_ptr->x, other_v_ptr->y)
            );
        float diff = get_angle_cw_dif(angle, a);
        
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
    
    if(closest_edge_angle) {
        *closest_edge_angle = best_edge_angle;
    }
    return best_edge;
}


/* ----------------------------------------------------------------------------
 * Returns a sector common to all vertexes and edges.
 * A sector is considered this if a vertex has it as a sector of
 * a neighboring edge, or if a vertex is inside it.
 * Use the former for vertexes that will be merged, and the latter
 * for vertexes that won't.
 * Returns false if there is no common sector. True otherwise.
 * vertexes:
 *   List of vertexes to check.
 * edges:
 *   List of edges to check.
 * result:
 *   Returns the common sector here.
 */
bool area_editor::get_common_sector(
    vector<vertex*> &vertexes, vector<edge*> &edges, sector** result
) const {
    unordered_set<sector*> sectors;
    
    //First, populate the list of common sectors with a sample.
    //Let's use the first vertex or edge's sectors.
    if(!vertexes.empty()) {
        for(size_t e = 0; e < vertexes[0]->edges.size(); ++e) {
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
    for(size_t v = 0; v < vertexes.size(); ++v) {
        vertex* v_ptr = vertexes[v];
        for(auto s = sectors.begin(); s != sectors.end();) {
            bool found_s = false;
            
            for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
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
    for(size_t e = 0; e < edges.size(); ++e) {
        edge* e_ptr = edges[e];
        for(auto s = sectors.begin(); s != sectors.end();) {
            if(e_ptr->sectors[0] != *s && e_ptr->sectors[1] != *s) {
                sectors.erase(s++);
            } else {
                ++s;
            }
        }
    }
    
    if(sectors.empty()) {
        *result = NULL;
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
    sector* best_rightmost_sector = NULL;
    for(auto s : sectors) {
        if(s == NULL) continue;
        vertex* v_ptr = s->get_rightmost_vertex();
        if(!best_rightmost_sector || v_ptr->x < best_rightmost_x) {
            best_rightmost_sector = s;
            best_rightmost_x = v_ptr->x;
        }
    }
    
    *result = best_rightmost_sector;
    return true;
}


/* ----------------------------------------------------------------------------
 * After an edge split, some vertexes could've wanted to merge with the
 * original edge, but may now need to merge with the NEW edge.
 * This function can check which is the "correct" edge to point to, from
 * the two provided.
 * v_ptr:
 *   Vertex that caused a split.
 * e1_ptr:
 *   First edge resulting of the split.
 * e2_ptr:
 *   Second edge resulting of the split.
 */
edge* area_editor::get_correct_post_split_edge(
    vertex* v_ptr, edge* e1_ptr, edge* e2_ptr
) const {
    float score1 = 0;
    float score2 = 0;
    get_closest_point_in_line(
        point(e1_ptr->vertexes[0]->x, e1_ptr->vertexes[0]->y),
        point(e1_ptr->vertexes[1]->x, e1_ptr->vertexes[1]->y),
        point(v_ptr->x, v_ptr->y),
        &score1
    );
    get_closest_point_in_line(
        point(e2_ptr->vertexes[0]->x, e2_ptr->vertexes[0]->y),
        point(e2_ptr->vertexes[1]->x, e2_ptr->vertexes[1]->y),
        point(v_ptr->x, v_ptr->y),
        &score2
    );
    if(fabs(score1 - 0.5) < fabs(score2 - 0.5)) {
        return e1_ptr;
    } else {
        return e2_ptr;
    }
}


/* ----------------------------------------------------------------------------
 * Returns true if the drawing has an outer sector it belongs to,
 * even if the sector is the void, or false if something's gone wrong.
 * result:
 *   The outer sector, if any, is returned here.
 */
bool area_editor::get_drawing_outer_sector(sector** result) const {
    //Start by checking if there's a node on a sector. If so, that's it!
    for(size_t n = 0; n < drawing_nodes.size(); ++n) {
        if(!drawing_nodes[n].on_vertex && !drawing_nodes[n].on_edge) {
            (*result) = drawing_nodes[n].on_sector;
            return true;
        }
    }
    
    //If none are on sectors, let's try the following:
    //Grab the first line that is not on top of an existing one,
    //and find the sector that line is on by checking its center.
    for(size_t n = 0; n < drawing_nodes.size(); ++n) {
        const layout_drawing_node* n1 = &drawing_nodes[n];
        const layout_drawing_node* n2 = &(get_next_in_vector(drawing_nodes, n));
        if(!are_nodes_traversable(*n1, *n2)) {
            *result =
                get_sector(
                    (n1->snapped_spot + n2->snapped_spot) / 2,
                    NULL, false
                );
            return true;
        }
    }
    
    //If we couldn't find the outer sector that easily,
    //let's try a different approach: check which sector is common
    //to all vertexes and edges.
    vector<vertex*> v;
    vector<edge*> e;
    for(size_t n = 0; n < drawing_nodes.size(); ++n) {
        if(drawing_nodes[n].on_vertex) {
            v.push_back(drawing_nodes[n].on_vertex);
        } else if(drawing_nodes[n].on_edge) {
            e.push_back(drawing_nodes[n].on_edge);
        }
    }
    return get_common_sector(v, e, result);
}


/* ----------------------------------------------------------------------------
 * Returns the edge currently under the specified point, or NULL if none.
 * p:
 *   The point.
 * after:
 *   Only check edges that come after this one.
 */
edge* area_editor::get_edge_under_point(const point &p, edge* after) const {
    bool found_after = (!after ? true : false);
    
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        edge* e_ptr = game.cur_area_data.edges[e];
        if(e_ptr == after) {
            found_after = true;
            continue;
        } else if(!found_after) {
            continue;
        }
        
        if(!e_ptr->is_valid()) continue;
        
        if(
            circle_intersects_line(
                p, 8 / game.cam.zoom,
                point(
                    e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                ),
                point(
                    e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                )
            )
        ) {
            return e_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns which edges are crossing against other edges, if any.
 */
vector<edge_intersection> area_editor::get_intersecting_edges() const {
    vector<edge_intersection> intersections;
    
    for(size_t e1 = 0; e1 < game.cur_area_data.edges.size(); ++e1) {
        edge* e1_ptr = game.cur_area_data.edges[e1];
        for(size_t e2 = e1 + 1; e2 < game.cur_area_data.edges.size(); ++e2) {
            edge* e2_ptr = game.cur_area_data.edges[e2];
            if(e1_ptr->has_neighbor(e2_ptr)) continue;
            if(
                lines_intersect(
                    point(e1_ptr->vertexes[0]->x, e1_ptr->vertexes[0]->y),
                    point(e1_ptr->vertexes[1]->x, e1_ptr->vertexes[1]->y),
                    point(e2_ptr->vertexes[0]->x, e2_ptr->vertexes[0]->y),
                    point(e2_ptr->vertexes[1]->x, e2_ptr->vertexes[1]->y),
                    NULL, NULL
                )
            ) {
                intersections.push_back(edge_intersection(e1_ptr, e2_ptr));
            }
        }
    }
    return intersections;
}


/* ----------------------------------------------------------------------------
 * Returns the radius of the specific mob generator. Normally, this returns the
 * type's radius, but if the type/radius is invalid, it returns a default.
 * m:
 *   The mob to get the radius of.
 */
float area_editor::get_mob_gen_radius(mob_gen* m) const {
    return m->type ? m->type->radius == 0 ? 16 : m->type->radius : 16;
}


/* ----------------------------------------------------------------------------
 * Returns true if there are path links currently under the specified point.
 * data1 takes the info of the found link. If there's also a link in
 * the opposite direction, data2 gets that data, otherwise data2 gets filled
 * with NULLs.
 * p:
 *   The point to check against.
 * data1:
 *   If there is a link under the point, its data is returned here.
 * data2:
 *   If there is a link under the point going in the opposite direction of
 *   the previous link, its data is returned here.
 */
bool area_editor::get_mob_link_under_point(
    const point &p,
    std::pair<mob_gen*, mob_gen*>* data1, std::pair<mob_gen*, mob_gen*>* data2
) const {
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        for(size_t l = 0; l < m_ptr->links.size(); ++l) {
            mob_gen* m2_ptr = m_ptr->links[l];
            if(
                circle_intersects_line(
                    p, 8 / game.cam.zoom, m_ptr->pos, m2_ptr->pos
                )
            ) {
                *data1 = std::make_pair(m_ptr, m2_ptr);
                *data2 = std::make_pair((mob_gen*) NULL, (mob_gen*) NULL);
                
                for(size_t l2 = 0; l2 < m2_ptr->links.size(); ++l2) {
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


/* ----------------------------------------------------------------------------
 * Returns the mob currently under the specified point, or NULL if none.
 * p:
 *   The point to check against.
 */
mob_gen* area_editor::get_mob_under_point(const point &p) const {
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        
        if(
            dist(m_ptr->pos, p) <= get_mob_gen_radius(m_ptr)
        ) {
            return m_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns true if there are path links currently under the specified point.
 * data1 takes the info of the found link. If there's also a link in
 * the opposite direction, data2 gets that data, otherwise data2 gets filled
 * with NULLs.
 * p:
 *   The point to check against.
 * data1:
 *   If there is a path link under that point, its data is returned here.
 * data2:
 *   If there is a path link under the point, but going in the opposite
 *   direction, its data is returned here.
 */
bool area_editor::get_path_link_under_point(
    const point &p,
    std::pair<path_stop*, path_stop*>* data1,
    std::pair<path_stop*, path_stop*>* data2
) const {
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        for(size_t l = 0; l < s_ptr->links.size(); ++l) {
            path_stop* s2_ptr = s_ptr->links[l].end_ptr;
            if(
                circle_intersects_line(
                    p, 8 / game.cam.zoom, s_ptr->pos, s2_ptr->pos
                )
            ) {
                *data1 = std::make_pair(s_ptr, s2_ptr);
                if(s2_ptr->get_link(s_ptr) != INVALID) {
                    *data2 = std::make_pair(s2_ptr, s_ptr);
                } else {
                    *data2 =
                        std::make_pair((path_stop*) NULL, (path_stop*) NULL);
                }
                return true;
            }
        }
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns the path stop currently under the specified point, or NULL if none.
 * p:
 *   Point to check against.
 */
path_stop* area_editor::get_path_stop_under_point(const point &p) const {
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        
        if(dist(s_ptr->pos, p) <= PATH_STOP_RADIUS) {
            return s_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns the sector currently under the specified point, or NULL if none.
 * p:
 *   Point to check against.
 */
sector* area_editor::get_sector_under_point(const point &p) const {
    return get_sector(p, NULL, false);
}


/* ----------------------------------------------------------------------------
 * Returns the vertex currently under the specified point, or NULL if none.
 * p:
 *   Point to check against.
 */
vertex* area_editor::get_vertex_under_point(const point &p) const {
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = game.cur_area_data.vertexes[v];
        
        if(
            rectangles_intersect(
                p - (4 / game.cam.zoom),
                p + (4 / game.cam.zoom),
                point(
                    v_ptr->x - (4 / game.cam.zoom),
                    v_ptr->y - (4 / game.cam.zoom)
                ),
                point(
                    v_ptr->x + (4 / game.cam.zoom),
                    v_ptr->y + (4 / game.cam.zoom)
                )
            )
        ) {
            return v_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Homogenizes all selected mobs,
 * based on the one at the head of the selection.
 */
void area_editor::homogenize_selected_mobs() {
    if(selected_mobs.size() < 2) return;
    
    mob_gen* base = *selected_mobs.begin();
    for(auto m = selected_mobs.begin(); m != selected_mobs.end(); ++m) {
        if(m == selected_mobs.begin()) continue;
        mob_gen* m_ptr = *m;
        m_ptr->category = base->category;
        m_ptr->type = base->type;
        m_ptr->angle = base->angle;
        m_ptr->vars = base->vars;
        m_ptr->links = base->links;
        m_ptr->link_nrs = base->link_nrs;
    }
    status_text =
        "Homogenized " + amount_str(selected_mobs.size(), "object") + ".";
}


/* ----------------------------------------------------------------------------
 * Homogenizes all selected sectors,
 * based on the one at the head of the selection.
 */
void area_editor::homogenize_selected_sectors() {
    if(selected_sectors.size() < 2) return;
    
    sector* base = *selected_sectors.begin();
    for(auto s = selected_sectors.begin(); s != selected_sectors.end(); ++s) {
        if(s == selected_sectors.begin()) continue;
        base->clone(*s);
        update_sector_texture(*s, base->texture_info.file_name);
    }
    status_text =
        "Homogenized " + amount_str(selected_sectors.size(), "sector") + ".";
}


/* ----------------------------------------------------------------------------
 * Merges two neighboring sectors into one. The final sector will
 * be the largest of the two.
 * s1:
 *   First sector to merge.
 * s2:
 *   Second sector to merge.
 */
bool area_editor::merge_sectors(sector* s1, sector* s2) {
    //Of the two sectors, figure out which is the largest.
    sector* main_sector = s1;
    sector* small_sector = s2;
    if(!s2) {
        main_sector = s2;
        small_sector = s1;
    } else if(s1 && s2) {
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
    unordered_set<edge*> common_edges;
    unordered_set<edge*> edges_to_transfer;
    
    for(size_t e = 0; e < small_sector->edges.size(); ++e) {
        edge* e_ptr = small_sector->edges[e];
        if(e_ptr->get_other_sector(small_sector) == main_sector) {
            common_edges.insert(e_ptr);
        } else {
            edges_to_transfer.insert(e_ptr);
        }
    }
    
    //However, if there are no common edges beween sectors,
    //this operation is invalid.
    if(common_edges.empty()) {
        status_text = "Those two sectors are not neighbors!";
        return false;
    }
    
    //Before doing anything, get the list of sectors that will be affected.
    unordered_set<sector*> affected_sectors;
    get_affected_sectors(small_sector, affected_sectors);
    if(main_sector) get_affected_sectors(main_sector, affected_sectors);
    
    //Transfer edges that need transferal.
    for(edge* e_ptr : edges_to_transfer) {
        e_ptr->transfer_sector(
            small_sector, main_sector,
            main_sector ?
            game.cur_area_data.find_sector_nr(main_sector) :
            INVALID,
            game.cur_area_data.find_edge_nr(e_ptr)
        );
    }
    
    //Delete the other ones.
    for(edge* e_ptr : common_edges) {
        delete_edge(e_ptr);
    }
    
    //Delete the now-merged sector.
    game.cur_area_data.remove_sector(small_sector);
    
    //Update all affected sectors.
    affected_sectors.erase(small_sector);
    update_affected_sectors(affected_sectors);
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Merges vertex 1 into vertex 2.
 * v1:
 *   Vertex that is being moved and will be merged.
 * v2:
 *   Vertex that is going to absorb v1.
 * affected_sectors:
 *   List of sectors that will be affected by this merge.
 */
void area_editor::merge_vertex(
    vertex* v1, vertex* v2, unordered_set<sector*>* affected_sectors
) {
    vector<edge*> edges = v1->edges;
    //Find out what to do with every edge of the dragged vertex.
    for(size_t e = 0; e < edges.size(); ++e) {
    
        edge* e_ptr = edges[e];
        vertex* other_vertex = e_ptr->get_other_vertex(v1);
        
        if(other_vertex == v2) {
        
            //Squashed into non-existence.
            affected_sectors->insert(e_ptr->sectors[0]);
            affected_sectors->insert(e_ptr->sectors[1]);
            
            //Delete it.
            delete_edge(e_ptr);
            
        } else {
        
            bool has_merged = false;
            //Check if the edge will be merged with another one.
            //These are edges that share a common vertex,
            //plus the moved/destination vertex.
            for(size_t de = 0; de < v2->edges.size(); ++de) {
            
                edge* de_ptr = v2->edges[de];
                vertex* d_other_vertex = de_ptr->get_other_vertex(v2);
                
                if(d_other_vertex == other_vertex) {
                    //The edge will be merged with this one.
                    has_merged = true;
                    affected_sectors->insert(e_ptr->sectors[0]);
                    affected_sectors->insert(e_ptr->sectors[1]);
                    affected_sectors->insert(de_ptr->sectors[0]);
                    affected_sectors->insert(de_ptr->sectors[1]);
                    
                    //Set the new sectors.
                    if(e_ptr->sectors[0] == de_ptr->sectors[0]) {
                        game.cur_area_data.connect_edge_to_sector(
                            de_ptr, e_ptr->sectors[1], 0
                        );
                    } else if(e_ptr->sectors[0] == de_ptr->sectors[1]) {
                        game.cur_area_data.connect_edge_to_sector(
                            de_ptr, e_ptr->sectors[1], 1
                        );
                    } else if(e_ptr->sectors[1] == de_ptr->sectors[0]) {
                        game.cur_area_data.connect_edge_to_sector(
                            de_ptr, e_ptr->sectors[0], 0
                        );
                    } else if(e_ptr->sectors[1] == de_ptr->sectors[1]) {
                        game.cur_area_data.connect_edge_to_sector(
                            de_ptr, e_ptr->sectors[0], 1
                        );
                    }
                    
                    //Delete it.
                    delete_edge(e_ptr);
                    
                    break;
                }
            }
            
            //If it's matchless, that means it'll just be joined to
            //the group of edges on the destination vertex.
            if(!has_merged) {
                game.cur_area_data.connect_edge_to_vertex(
                    e_ptr, v2, (e_ptr->vertexes[0] == v1 ? 0 : 1)
                );
                for(size_t v2e = 0; v2e < v2->edges.size(); ++v2e) {
                    affected_sectors->insert(v2->edges[v2e]->sectors[0]);
                    affected_sectors->insert(v2->edges[v2e]->sectors[1]);
                }
            }
        }
        
    }
    
    //Check if any of the final edges have the same sector
    //on both sides. If so, delete them.
    for(size_t ve = 0; ve < v2->edges.size(); ) {
        edge* ve_ptr = v2->edges[ve];
        if(ve_ptr->sectors[0] == ve_ptr->sectors[1]) {
            delete_edge(ve_ptr);
        } else {
            ++ve;
        }
    }
    
    //Delete the old vertex.
    game.cur_area_data.remove_vertex(v1);
    
    //If any vertex or sector is out of edges, delete it.
    for(size_t v = 0; v < game.cur_area_data.vertexes.size();) {
        vertex* v_ptr = game.cur_area_data.vertexes[v];
        if(v_ptr->edges.empty()) {
            game.cur_area_data.remove_vertex(v);
        } else {
            ++v;
        }
    }
    for(size_t s = 0; s < game.cur_area_data.sectors.size();) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        if(s_ptr->edges.empty()) {
            game.cur_area_data.remove_sector(s);
        } else {
            ++s;
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Resizes all X and Y coordinates by the specified multiplier.
 * mult:
 *   Multiply the coordinates by this value.
 */
void area_editor::resize_everything(const float mult) {
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = game.cur_area_data.vertexes[v];
        v_ptr->x *= mult;
        v_ptr->y *= mult;
    }
    
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        s_ptr->texture_info.scale *= mult;
        s_ptr->texture_info.translation *= mult;
        s_ptr->triangles.clear();
        triangulate(s_ptr, NULL, false, false);
    }
    
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        m_ptr->pos *= mult;
    }
    
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        s_ptr->pos *= mult;
    }
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        game.cur_area_data.path_stops[s]->calculate_dists();
    }
    
    for(size_t s = 0; s < game.cur_area_data.tree_shadows.size(); ++s) {
        tree_shadow* s_ptr = game.cur_area_data.tree_shadows[s];
        s_ptr->center *= mult;
        s_ptr->size   *= mult;
        s_ptr->sway   *= mult;
    }
}


/* ----------------------------------------------------------------------------
 * Makes all currently selected mob generators (if any) rotate to face where the
 * the given point is.
 * pos:
 *   Point that the mobs must face.
 */
void area_editor::rotate_mob_gens_to_point(const point &pos) {
    if(selected_mobs.empty()) return;
    
    register_change("object rotation");
    selection_homogenized = false;
    for(auto m : selected_mobs) {
        m->angle = get_angle(m->pos, pos);
    }
    status_text = "Rotated objects to face " + p2s(pos) + ".";
}


/* ----------------------------------------------------------------------------
 * Snaps a point to the nearest available snapping space, based on the
 * current snap mode.
 * p:
 *   Point to snap.
 * ignore_selected:
 *   If true, ignore the selected vertexes or edges when snapping to
 *   vertexes or edges.
 */
point area_editor::snap_point(const point &p, const bool ignore_selected) {
    if(is_shift_pressed) return p;
    
    switch(snap_mode) {
    case SNAP_GRID: {
        return
            point(
                round(p.x / game.options.area_editor_grid_interval) *
                game.options.area_editor_grid_interval,
                round(p.y / game.options.area_editor_grid_interval) *
                game.options.area_editor_grid_interval
            );
            
        break;
        
    } case SNAP_VERTEXES: {
        if(cursor_snap_timer.time_left > 0.0f) {
            return cursor_snap_cache;
        }
        cursor_snap_timer.start();
        
        vector<vertex*> vertexes_to_check = game.cur_area_data.vertexes;
        if(ignore_selected) {
            for(vertex* v : selected_vertexes) {
                for(size_t v2 = 0; v2 < vertexes_to_check.size(); ++v2) {
                    if(vertexes_to_check[v2] == v) {
                        vertexes_to_check.erase(vertexes_to_check.begin() + v2);
                        break;
                    }
                }
            }
        }
        vector<std::pair<dist, vertex*> > snappable_vertexes =
            get_merge_vertexes(
                p, vertexes_to_check,
                game.options.area_editor_snap_threshold / game.cam.zoom
            );
        if(snappable_vertexes.empty()) {
            cursor_snap_cache = p;
            return p;
        } else {
            sort(
                snappable_vertexes.begin(), snappable_vertexes.end(),
                [] (
                    std::pair<dist, vertex*> v1, std::pair<dist, vertex*> v2
            ) -> bool {
                return v1.first < v2.first;
            }
            );
            
            point result(
                snappable_vertexes[0].second->x,
                snappable_vertexes[0].second->y
            );
            cursor_snap_cache = result;
            return result;
        }
        
        break;
        
    } case SNAP_EDGES: {
        if(cursor_snap_timer.time_left > 0.0f) {
            return cursor_snap_cache;
        }
        cursor_snap_timer.start();
        
        dist closest_dist;
        point closest_point = p;
        bool got_one = false;
        
        for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
            edge* e_ptr = game.cur_area_data.edges[e];
            float r;
            
            if(ignore_selected) {
                //Let's ignore not only the selected edge, but also
                //neighboring edges, because as we move an edge,
                //the neighboring edges stretch along with it.
                bool skip = false;
                for(vertex* v : selected_vertexes) {
                    if(v->has_edge(e_ptr)) {
                        skip = true;
                        break;
                    }
                }
                if(skip) continue;
            }
            
            point edge_p =
                get_closest_point_in_line(
                    point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
                    point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y),
                    p, &r
                );
                
            if(r < 0.0f) {
                edge_p = point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y);
            } else if(r > 1.0f) {
                edge_p = point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y);
            }
            
            dist d(p, edge_p);
            if(d > game.options.area_editor_snap_threshold / game.cam.zoom) {
                continue;
            }
            
            if(!got_one || d < closest_dist) {
                got_one = true;
                closest_dist = d;
                closest_point = edge_p;
            }
        }
        
        cursor_snap_cache = closest_point;
        return closest_point;
        
        break;
        
    }
    }
    
    return p;
}


/* ----------------------------------------------------------------------------
 * Splits an edge into two, near the specified point, and returns the
 * newly-created vertex. The new vertex gets added to the current area.
 * e_ptr:
 *   Edge to split.
 * where:
 *   Point to split at.
 */
vertex* area_editor::split_edge(edge* e_ptr, const point &where) {
    point new_v_pos =
        get_closest_point_in_line(
            point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
            point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y),
            where
        );
        
    //Create the new vertex and the new edge.
    vertex* new_v_ptr = game.cur_area_data.new_vertex();
    new_v_ptr->x = new_v_pos.x;
    new_v_ptr->y = new_v_pos.y;
    edge* new_e_ptr = game.cur_area_data.new_edge();
    
    //Connect the vertexes and edges.
    game.cur_area_data.connect_edge_to_vertex(new_e_ptr, new_v_ptr, 0);
    game.cur_area_data.connect_edge_to_vertex(new_e_ptr, e_ptr->vertexes[1], 1);
    game.cur_area_data.connect_edge_to_vertex(e_ptr, new_v_ptr, 1);
    
    //Connect the sectors and new edge.
    if(e_ptr->sectors[0]) {
        game.cur_area_data.connect_edge_to_sector(
            new_e_ptr, e_ptr->sectors[0], 0
        );
    }
    if(e_ptr->sectors[1]) {
        game.cur_area_data.connect_edge_to_sector(
            new_e_ptr, e_ptr->sectors[1], 1
        );
    }
    
    return new_v_ptr;
}


/* ----------------------------------------------------------------------------
 * Splits a path link into two, near the specified point, and returns the
 * newly-created path stop. The new stop gets added to the current area.
 * l1:
 *   Path link to split.
 * l2:
 *   If there is also a path link going in the opposite direction between
 *   the two stops involved, this contains its data. Otherwise, it contains
 *   NULLs.
 * where:
 *   Where to make the split.
 */
path_stop* area_editor::split_path_link(
    const std::pair<path_stop*, path_stop*> &l1,
    const std::pair<path_stop*, path_stop*> &l2,
    const point &where
) {
    bool normal_link = (l2.first != NULL);
    point new_s_pos =
        get_closest_point_in_line(
            l1.first->pos, l1.second->pos,
            where
        );
        
    //Create the new stop.
    path_stop* new_s_ptr = new path_stop(new_s_pos);
    game.cur_area_data.path_stops.push_back(new_s_ptr);
    
    //Delete the old links.
    l1.first->remove_link(l1.second);
    if(normal_link) {
        l2.first->remove_link(l2.second);
    }
    
    //Create the new links.
    l1.first->add_link(new_s_ptr, normal_link);
    new_s_ptr->add_link(l1.second, normal_link);
    
    //Fix the dangling path stop numbers in the links.
    game.cur_area_data.fix_path_stop_nrs(l1.first);
    game.cur_area_data.fix_path_stop_nrs(l1.second);
    game.cur_area_data.fix_path_stop_nrs(new_s_ptr);
    
    //Update the distances.
    new_s_ptr->calculate_dists_plus_neighbors();
    
    return new_s_ptr;
}


/* ----------------------------------------------------------------------------
 * Updates the triangles and bounding box of the specified sectors, and
 * reports any errors found.
 * affected_sectors:
 *   The list of affected sectors.
 */
void area_editor::update_affected_sectors(
    const unordered_set<sector*> &affected_sectors
) {
    TRIANGULATION_ERRORS last_triangulation_error = TRIANGULATION_NO_ERROR;
    
    for(sector* s_ptr : affected_sectors) {
        if(!s_ptr) continue;
        
        set<edge*> triangulation_lone_edges;
        TRIANGULATION_ERRORS triangulation_error =
            triangulate(s_ptr, &triangulation_lone_edges, true, true);
            
        if(triangulation_error == TRIANGULATION_NO_ERROR) {
            auto it = game.cur_area_data.problems.non_simples.find(s_ptr);
            if(it != game.cur_area_data.problems.non_simples.end()) {
                game.cur_area_data.problems.non_simples.erase(it);
            }
        } else {
            game.cur_area_data.problems.non_simples[s_ptr] =
                triangulation_error;
            last_triangulation_error = triangulation_error;
        }
        game.cur_area_data.problems.lone_edges.insert(
            triangulation_lone_edges.begin(),
            triangulation_lone_edges.end()
        );
        
        s_ptr->calculate_bounding_box();
    }
    
    if(last_triangulation_error != TRIANGULATION_NO_ERROR) {
        emit_triangulation_error_status_bar_message(last_triangulation_error);
    }
}


/* ----------------------------------------------------------------------------
 * When the user creates a new sector, which houses other sectors inside,
 * and these inner sectors need to know their outer sector changed.
 * This will go through a list of edges, check if they are inside
 * the new sector, and if so, update their outer sector.
 * edges_to_check:
 *   List of edges to check if they belong inside the new sector or not.
 * old_outer:
 *   What the old outer sector used to be.
 * new_outer:
 *   What the new outer sector is.
 */
void area_editor::update_inner_sectors_outer_sector(
    const vector<edge*> &edges_to_check,
    sector* old_outer, sector* new_outer
) {
    for(size_t e = 0; e < edges_to_check.size(); ++e) {
        edge* e_ptr = edges_to_check[e];
        vertex* v1_ptr = e_ptr->vertexes[0];
        vertex* v2_ptr = e_ptr->vertexes[1];
        if(
            new_outer->is_point_in_sector(point(v1_ptr->x, v1_ptr->y)) &&
            new_outer->is_point_in_sector(point(v2_ptr->x, v2_ptr->y)) &&
            new_outer->is_point_in_sector(
                point(
                    (v1_ptr->x + v2_ptr->x) / 2.0f,
                    (v1_ptr->y + v2_ptr->y) / 2.0f
                )
            )
        ) {
            for(size_t s = 0; s < 2; ++s) {
                if(e_ptr->sectors[s] == old_outer) {
                    game.cur_area_data.connect_edge_to_sector(
                        e_ptr, new_outer, s
                    );
                    break;
                }
            }
        }
    }
}
