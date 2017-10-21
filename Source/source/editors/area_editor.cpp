/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General area editor-related functions.
 */

#include <algorithm>

#include "area_editor.h"
#include "../functions.h"
#include "../load.h"
#include "../vars.h"

using namespace std;

//Scale the debug text by this much.
const float area_editor::DEBUG_TEXT_SCALE = 1.5f;
//Default grid interval.
const float area_editor::DEF_GRID_INTERVAL = 32.0f;
//Time until the next click is no longer considered a double-click.
const float area_editor::DOUBLE_CLICK_TIMEOUT = 0.5f;
//How long to tint the drawing line red for.
const float area_editor::DRAWING_LINE_ERROR_TINT_DURATION = 1.5f;
//How much to zoom in/out with the keyboard keys.
const float area_editor::KEYBOARD_CAM_ZOOM = 0.25f;
//Maximum number of texture suggestions.
const size_t area_editor::MAX_TEXTURE_SUGGESTIONS = 20;
//If the mouse is dragged outside of this range, that's a real drag.
const float area_editor::MOUSE_DRAG_CONFIRM_RANGE = 4.0f;
//Thickness to use when drawing a path link line.
const float area_editor::PATH_LINK_THICKNESS = 2.0f;
//Radius to use when drawing a path stop circle.
const float area_editor::PATH_STOP_RADIUS = 16.0f;
//Color of a selected element, or the selection box.
const unsigned char area_editor::SELECTION_COLOR[3] = {255, 215, 0};
//Speed at which the selection effect's "wheel" spins, in radians per second.
const float area_editor::SELECTION_EFFECT_SPEED = M_PI * 4;
//Minimum distance between two vertexes for them to merge.
const float area_editor::VERTEX_MERGE_RADIUS = 10.0f;
//Maximum zoom level possible in the editor.
const float area_editor::ZOOM_MAX_LEVEL_EDITOR = 8.0f;
//Minimum zoom level possible in the editor.
const float area_editor::ZOOM_MIN_LEVEL_EDITOR = 0.01f;

const string area_editor::EDITOR_ICONS_FOLDER_NAME = "Editor_icons";
const string area_editor::ICON_DELETE =
    EDITOR_ICONS_FOLDER_NAME + "/Delete.png";
const string area_editor::ICON_DELETE_LINK =
    EDITOR_ICONS_FOLDER_NAME + "/Delete_link.png";
const string area_editor::ICON_DELETE_STOP =
    EDITOR_ICONS_FOLDER_NAME + "/Delete_stop.png";
const string area_editor::ICON_DUPLICATE =
    EDITOR_ICONS_FOLDER_NAME + "/Duplicate.png";
const string area_editor::ICON_EXIT =
    EDITOR_ICONS_FOLDER_NAME + "/Exit.png";
const string area_editor::ICON_NEW =
    EDITOR_ICONS_FOLDER_NAME + "/New.png";
const string area_editor::ICON_NEW_1WAY_LINK =
    EDITOR_ICONS_FOLDER_NAME + "/New_1wlink.png";
const string area_editor::ICON_NEW_CIRCLE_SECTOR =
    EDITOR_ICONS_FOLDER_NAME + "/New_circle_sector.png";
const string area_editor::ICON_NEW_LINK =
    EDITOR_ICONS_FOLDER_NAME + "/New_link.png";
const string area_editor::ICON_NEW_STOP =
    EDITOR_ICONS_FOLDER_NAME + "/New_stop.png";
const string area_editor::ICON_NEXT =
    EDITOR_ICONS_FOLDER_NAME + "/Next.png";
const string area_editor::ICON_OPTIONS =
    EDITOR_ICONS_FOLDER_NAME + "/Options.png";
const string area_editor::ICON_PREVIOUS =
    EDITOR_ICONS_FOLDER_NAME + "/Previous.png";
const string area_editor::ICON_REFERENCE =
    EDITOR_ICONS_FOLDER_NAME + "/Reference.png";
const string area_editor::ICON_SAVE =
    EDITOR_ICONS_FOLDER_NAME + "/Save.png";


/* ----------------------------------------------------------------------------
 * Creates a layout drawing node based on the mouse's click position.
 */
area_editor::layout_drawing_node::layout_drawing_node(
    area_editor* ae_ptr, const point &mouse_click
) :
    raw_spot(mouse_click),
    snapped_spot(mouse_click),
    on_vertex(nullptr),
    on_vertex_nr(INVALID),
    on_edge(nullptr),
    on_edge_nr(INVALID),
    on_sector(nullptr),
    on_sector_nr(INVALID),
    is_new_vertex(false) {
    
    on_vertex =
        get_merge_vertex(
            mouse_click, cur_area_data.vertexes,
            VERTEX_MERGE_RADIUS / cam_zoom, &on_vertex_nr
        );
        
    if(on_vertex) {
        snapped_spot.x = on_vertex->x;
        snapped_spot.y = on_vertex->y;
        
    } else {
        on_edge = ae_ptr->get_edge_under_point(mouse_click);
        
        if(on_edge) {
            on_edge_nr = cur_area_data.find_edge_nr(on_edge);
            snapped_spot =
                get_closest_point_in_line(
                    point(on_edge->vertexes[0]->x, on_edge->vertexes[0]->y),
                    point(on_edge->vertexes[1]->x, on_edge->vertexes[1]->y),
                    mouse_click
                );
                
        } else {
            on_sector = get_sector(mouse_click, &on_sector_nr, false);
            
        }
    }
}


/* ----------------------------------------------------------------------------
 * Initializes area editor class stuff.
 */
area_editor::area_editor() :
    state(EDITOR_STATE_MAIN),
    backup_timer(editor_backup_interval),
    debug_edge_nrs(false),
    debug_sector_nrs(false),
    debug_triangulation(false),
    debug_vertex_nrs(false),
    double_click_time(0),
    drawing_line_error(DRAWING_LINE_NO_ERROR),
    drawing_line_error_tint_timer(DRAWING_LINE_ERROR_TINT_DURATION),
    grid_interval(DEF_GRID_INTERVAL),
    is_ctrl_pressed(false),
    is_shift_pressed(false),
    is_gui_focused(false),
    last_mouse_click(INVALID),
    mouse_drag_confirmed(false),
    path_preview_timer(0),
    selecting(false),
    selection_effect(0),
    show_reference(false) {
    
}


/* ----------------------------------------------------------------------------
 * Cancels the edge drawing operation.
 */
void area_editor::cancel_layout_drawing() {
    sub_state = EDITOR_SUB_STATE_NONE;
    drawing_nodes.clear();
    drawing_line_error = DRAWING_LINE_NO_ERROR;
    
    //TODO
    /*
    new_circle_sector_step = 0;
    new_circle_sector_points.clear();
    new_circle_sector_valid_edges.clear();
    */
}


/* ----------------------------------------------------------------------------
 * Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the screen,
 * where it's hard to see.
 */
void area_editor::center_camera(
    const point &min_coords, const point &max_coords
) {
    float width = max_coords.x - min_coords.x;
    float height = max_coords.y - min_coords.y;
    
    cam_pos.x = floor(min_coords.x + width  / 2);
    cam_pos.y = floor(min_coords.y + height / 2);
    
    float z;
    if(width > height) z = gui_x / width;
    else z = status_bar_y / height;
    
    z -= z * 0.1;
    
    zoom(z);
    
}


/* ----------------------------------------------------------------------------
 * Checks if the line the user is trying to draw is okay. Sets the line's status
 * to drawing_line_error.
 */
void area_editor::check_drawing_line(const point &pos) {
    drawing_line_error = DRAWING_LINE_NO_ERROR;
    
    if(drawing_nodes.empty()) {
        return;
    }
    
    layout_drawing_node* prev_node = &drawing_nodes.back();
    layout_drawing_node tentative_node(this, pos);
    
    //Check for edge collisions.
    if(!tentative_node.on_vertex) {
        for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
            //If this edge is the same or a neighbor of the previous node,
            //then never mind.
            edge* e_ptr = cur_area_data.edges[e];
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
    }
    
    //Check if the line intersects with the drawing's lines.
    if(drawing_nodes.size() >= 2) {
        for(size_t n = 0; n < drawing_nodes.size() - 2; ++n) {
            layout_drawing_node* n1_ptr = &drawing_nodes[n];
            layout_drawing_node* n2_ptr = &drawing_nodes[n + 1];
            if(
                lines_intersect(
                    prev_node->snapped_spot, pos,
                    n1_ptr->snapped_spot, n2_ptr->snapped_spot,
                    NULL, NULL
                )
            ) {
                drawing_line_error = DRAWING_LINE_CROSSES_DRAWING;
                return;
            }
        }
        
        if(
            circle_intersects_line(
                pos, 8.0 / cam_zoom,
                prev_node->snapped_spot,
                drawing_nodes[drawing_nodes.size() - 2].snapped_spot
            )
        ) {
            drawing_line_error = DRAWING_LINE_CROSSES_DRAWING;
            return;
        }
    }
    
    //Check if this line is entering a sector different from the one the
    //rest of the drawing is on.
    unordered_set<sector*> common_sectors;
    if(drawing_nodes[0].on_edge) {
        common_sectors.insert(drawing_nodes[0].on_edge->sectors[0]);
        common_sectors.insert(drawing_nodes[0].on_edge->sectors[1]);
    } else if(drawing_nodes[0].on_vertex) {
        for(size_t e = 0; e < drawing_nodes[0].on_vertex->edges.size(); ++e) {
            edge* e_ptr = drawing_nodes[0].on_vertex->edges[e];
            common_sectors.insert(e_ptr->sectors[0]);
            common_sectors.insert(e_ptr->sectors[1]);
        }
    } else {
        //It's all right if this includes the "NULL" sector.
        common_sectors.insert(drawing_nodes[0].on_sector);
    }
    for(size_t n = 1; n < drawing_nodes.size(); ++n) {
        layout_drawing_node* n_ptr = &drawing_nodes[n];
        unordered_set<sector*> node_sectors;
        if(n_ptr->on_edge) {
            node_sectors.insert(n_ptr->on_edge->sectors[0]);
            node_sectors.insert(n_ptr->on_edge->sectors[1]);
        } else if(n_ptr->on_vertex) {
            for(size_t e = 0; e < n_ptr->on_vertex->edges.size(); ++e) {
                edge* e_ptr = n_ptr->on_vertex->edges[e];
                node_sectors.insert(e_ptr->sectors[0]);
                node_sectors.insert(e_ptr->sectors[1]);
            }
        } else {
            //Again, it's all right if this includes the "NULL" sector.
            node_sectors.insert(n_ptr->on_sector);
        }
        
        for(auto s = common_sectors.begin(); s != common_sectors.end();) {
            if(node_sectors.find(*s) == node_sectors.end()) {
                common_sectors.erase(s++);
            } else {
                ++s;
            }
        }
    }
    
    if(tentative_node.on_edge) {
        if(
            common_sectors.find(tentative_node.on_edge->sectors[0]) ==
            common_sectors.end() &&
            common_sectors.find(tentative_node.on_edge->sectors[1]) ==
            common_sectors.end()
        ) {
            drawing_line_error = DRAWING_LINE_WAYWARD_SECTOR;
            return;
        }
    } else if(tentative_node.on_vertex) {
        bool vertex_ok = false;
        for(size_t e = 0; e < tentative_node.on_vertex->edges.size(); ++e) {
            edge* e_ptr = tentative_node.on_vertex->edges[e];
            if(
                common_sectors.find(e_ptr->sectors[0]) !=
                common_sectors.end() ||
                common_sectors.find(e_ptr->sectors[1]) !=
                common_sectors.end()
            ) {
                vertex_ok = true;
                break;
            }
        }
        if(!vertex_ok) {
            drawing_line_error = DRAWING_LINE_WAYWARD_SECTOR;
            return;
        }
    } else {
        if(
            common_sectors.find(tentative_node.on_sector) ==
            common_sectors.end()
        ) {
            drawing_line_error = DRAWING_LINE_WAYWARD_SECTOR;
            return;
        }
    }
    
    //Check if this drawing would leave any gaps.
    //TODO
    
}


/* ----------------------------------------------------------------------------
 * Clears the currently loaded area data.
 */
void area_editor::clear_current_area() {
    //TODO
}


/* ----------------------------------------------------------------------------
 *
 */
void area_editor::clear_selection() {
    selected_vertexes.clear();
    selected_edges.clear();
    selected_sectors.clear();
    selected_mobs.clear();
    selected_path_stops.clear();
    selection_homogenized = false;
}


/* ----------------------------------------------------------------------------
 * Creates a new item from the picker frame, given its name.
 */
void area_editor::create_new_from_picker(const string &name) {
    //TODO;
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor.
 */
void area_editor::do_logic() {

    gui->tick(delta_t);
    
    update_transformations();
    
    if(double_click_time > 0) {
        double_click_time -= delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
    
    path_preview_timer.tick(delta_t);
    drawing_line_error_tint_timer.tick(delta_t);
    
    if(!cur_area_name.empty() && editor_backup_interval > 0) {
        backup_timer.tick(delta_t);
    }
    
    fade_mgr.tick(delta_t);
    
    selection_effect += SELECTION_EFFECT_SPEED * delta_t;
    
}


/* ----------------------------------------------------------------------------
 * Checks if the drawing would create a neighbor-child hybrid sector inside.
 */
bool area_editor::drawing_creates_neighbor_child_hybrid() {
    unordered_set<sector*> neighbor_sectors;
    point drawing_tl = drawing_nodes[0].snapped_spot;
    point drawing_br = drawing_nodes[0].snapped_spot;
    for(size_t n = 0; n < drawing_nodes.size(); ++n) {
        layout_drawing_node* n_ptr = &drawing_nodes[n];
        if(n_ptr->on_edge) {
            neighbor_sectors.insert(n_ptr->on_edge->sectors[0]);
            neighbor_sectors.insert(n_ptr->on_edge->sectors[1]);
        }
        if(n_ptr->snapped_spot.x < drawing_tl.x) {
            drawing_tl.x = n_ptr->snapped_spot.x;
        }
        if(n_ptr->snapped_spot.x > drawing_br.x) {
            drawing_br.x = n_ptr->snapped_spot.x;
        }
        if(n_ptr->snapped_spot.y < drawing_tl.y) {
            drawing_tl.y = n_ptr->snapped_spot.y;
        }
        if(n_ptr->snapped_spot.y > drawing_br.y) {
            drawing_br.y = n_ptr->snapped_spot.y;
        }
    }
    
    if(neighbor_sectors.empty()) return false;
    
    for(auto s = neighbor_sectors.begin(); s != neighbor_sectors.end(); ++s) {
        for(size_t e = 0; e < (*s)->edges.size(); ++e) {
            edge* e_ptr = (*s)->edges[e];
            for(size_t v = 0; v < 2; ++v) {
                if(
                    e_ptr->vertexes[v]->x < drawing_tl.x ||
                    e_ptr->vertexes[v]->x > drawing_br.x ||
                    e_ptr->vertexes[v]->y < drawing_tl.y ||
                    e_ptr->vertexes[v]->y > drawing_br.y
                ) {
                    return false;
                }
            }
        }
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Finishes the layout drawing operation, and tries to create whatever sectors.
 */
void area_editor::finish_layout_drawing() {
    if(drawing_nodes.size() < 3) {
        cancel_layout_drawing();
        return;
    }
    
    //This is the basic idea: create a new sector using the
    //vertexes provided by the user, as a "child" of an existing sector.
    
    //Let's just save this, since we'll need it later.
    size_t orig_n_vertexes = cur_area_data.vertexes.size();
    
    //Quick check -- if all of a neighbor's vertexes are inside the drawing
    //then that would create a neighbor-child hybrid! Can't have that! Abort.
    if(drawing_creates_neighbor_child_hybrid()) {
        cancel_layout_drawing();
        return;
    }
    
    //Get the outer sector, so we can know where to start working in.
    sector* outer_sector = NULL;
    if(!get_drawing_outer_sector(&outer_sector)) {
        //Something went wrong. Abort.
        cancel_layout_drawing();
        return;
    }
    
    //Start creating the new sector.
    sector* new_sector = cur_area_data.new_sector();
    
    if(outer_sector) {
        outer_sector->clone(new_sector);
        update_sector_texture(
            new_sector,
            outer_sector->texture_info.file_name
        );
    } else {
        update_sector_texture(new_sector, "");
    }
    
    //First, create vertexes wherever necessary.
    for(size_t n = 0; n < drawing_nodes.size(); ++n) {
        layout_drawing_node* n_ptr = &drawing_nodes[n];
        if(n_ptr->on_vertex) continue;
        vertex* new_vertex = NULL;
        
        if(n_ptr->on_edge) {
            new_vertex = split_edge(n_ptr->on_edge, n_ptr->snapped_spot);
            
            //The split created new edges, so let's check future nodes
            //and update them, since they could've landed on new edges.
            for(size_t n2 = n; n2 < drawing_nodes.size(); ++n2) {
                if(drawing_nodes[n2].on_edge == n_ptr->on_edge) {
                    drawing_nodes[n2].on_edge =
                        get_edge_under_point(drawing_nodes[n2].snapped_spot);
                }
            }
        } else {
            new_vertex = cur_area_data.new_vertex();
            new_vertex->x = n_ptr->snapped_spot.x;
            new_vertex->y = n_ptr->snapped_spot.y;
            n_ptr->is_new_vertex = true;
        }
        
        n_ptr->on_vertex = new_vertex;
    }
    
    //Now that all nodes have a vertex, create the necessary edges.
    vector<vertex*> drawing_vertexes;
    vector<edge*> drawing_edges;
    for(size_t n = 0; n < drawing_nodes.size(); ++n) {
        layout_drawing_node* n_ptr = &drawing_nodes[n];
        layout_drawing_node* prev_node =
            &drawing_nodes[sum_and_wrap(n, -1, drawing_nodes.size())];
            
        drawing_vertexes.push_back(n_ptr->on_vertex);
        
        edge* prev_node_edge =
            n_ptr->on_vertex->get_edge_by_neighbor(prev_node->on_vertex);
            
        if(!prev_node_edge) {
            prev_node_edge = cur_area_data.new_edge();
            
            cur_area_data.connect_edge_to_vertex(
                prev_node_edge, prev_node->on_vertex, 0
            );
            cur_area_data.connect_edge_to_vertex(
                prev_node_edge, n_ptr->on_vertex, 1
            );
        }
        
        drawing_edges.push_back(prev_node_edge);
    }
    
    //Connect the edges to the sectors.
    bool is_clockwise = is_polygon_clockwise(drawing_vertexes);
    for(size_t e = 0; e < drawing_edges.size(); ++e) {
        edge* e_ptr = drawing_edges[e];
        if(!e_ptr->sectors[0] && !e_ptr->sectors[1]) {
            if(is_clockwise) {
                cur_area_data.connect_edge_to_sector(
                    e_ptr, outer_sector, 0
                );
                cur_area_data.connect_edge_to_sector(
                    e_ptr, new_sector, 1
                );
            } else {
                cur_area_data.connect_edge_to_sector(
                    e_ptr, new_sector, 0
                );
                cur_area_data.connect_edge_to_sector(
                    e_ptr, outer_sector, 1
                );
            }
            
        } else {
            if(e_ptr->sectors[0] == outer_sector) {
                cur_area_data.connect_edge_to_sector(
                    e_ptr, new_sector, 0
                );
            } else {
                cur_area_data.connect_edge_to_sector(
                    e_ptr, new_sector, 1
                );
            }
        }
    }
    
    //Triangulate new sector so we can check what's inside.
    triangulate(new_sector);
    
    //All sectors inside the new one need to know that
    //their outer sector changed.
    unordered_set<edge*> inner_edges;
    for(size_t v = 0; v < orig_n_vertexes; ++v) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        
        if(
            find(drawing_vertexes.begin(), drawing_vertexes.end(), v_ptr) !=
            drawing_vertexes.end()
        ) {
            //This vertex is part of the drawing; nothing to do here.
            continue;
        }
        
        if(is_point_in_sector(point(v_ptr->x, v_ptr->y), new_sector)) {
            inner_edges.insert(v_ptr->edges.begin(), v_ptr->edges.end());
        }
    }
    
    for(auto i = inner_edges.begin(); i != inner_edges.end(); ++i) {
        for(size_t s = 0; s < 2; ++s) {
            if((*i)->sectors[s] == outer_sector) {
                cur_area_data.connect_edge_to_sector(*i, new_sector, s);
            }
        }
    }
    
    //Final triangulations.
    triangulate(new_sector);
    if(outer_sector) triangulate(outer_sector);
    
    cur_area_data.check_matches(); //TODO
    
    //Select the new sector, ready for editing.
    clear_selection();
    selected_sectors.insert(new_sector);
    sector_to_gui();
    
    cancel_layout_drawing();
}


/* ----------------------------------------------------------------------------
 * Returns a sector common to all vertexes.
 * A sector is considered this if a vertex has it as a sector of
 * a neighboring edge, or if a vertex is inside it.
 * Use the former for vertexes that will be merged, and the latter
 * for vertexes that won't.
 * vertexes: List of vertexes to check.
 * result:   Returns the common sector here.
 * Returns false if there is no common sector. True otherwise.
 */
bool area_editor::get_common_sector(
    vector<vertex*> &vertexes, sector** result
) {
    unordered_set<sector*> sectors;
    
    //First, populate the list of common sectors with a sample.
    //Let's use the first vertex's sectors.
    for(size_t e = 0; e < vertexes[0]->edges.size(); ++e) {
        sectors.insert(vertexes[0]->edges[e]->sectors[0]);
        sectors.insert(vertexes[0]->edges[e]->sectors[1]);
    }
    
    //Then, check each vertex, and if a sector isn't present in that
    //vertex's list, then it's not a common one, so delete the sector
    //from the list of commons.
    for(size_t v = 1; v < vertexes.size(); ++v) {
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
    for(
        auto s = sectors.begin(); s != sectors.end(); ++s
    ) {
        if(*s == NULL) continue;
        vertex* v_ptr = get_rightmost_vertex(*s);
        if(!best_rightmost_sector || v_ptr->x < best_rightmost_x) {
            best_rightmost_sector = *s;
            best_rightmost_x = v_ptr->x;
        }
    }
    
    *result = best_rightmost_sector;
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns true if the drawing has an outer sector it belongs to,
 * even if the sector is the void, or false if something's gone wrong.
 * The outer sector is returned to result.
 */
bool area_editor::get_drawing_outer_sector(sector** result) {
    for(size_t n = 0; n < drawing_nodes.size(); ++n) {
        if(!drawing_nodes[n].on_vertex && !drawing_nodes[n].on_edge) {
            (*result) = drawing_nodes[n].on_sector;
            return true;
        }
    }
    
    //If we couldn't find the outer sector that easily,
    //let's try a different approach: check which sector is common
    //to all vertexes.
    vector<vertex*> v;
    for(size_t n = 0; n < drawing_nodes.size(); ++n) {
        if(!drawing_nodes[n].on_vertex) continue;
        v.push_back(drawing_nodes[n].on_vertex);
    }
    return get_common_sector(v, result);
}


/* ----------------------------------------------------------------------------
 * Returns the edge currently under the specified point, or NULL if none.
 */
edge* area_editor::get_edge_under_point(const point &p) {
    for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
        edge* e_ptr = cur_area_data.edges[e];
        
        if(!is_edge_valid(e_ptr)) continue;
        
        if(
            circle_intersects_line(
                p, 8 / cam_zoom,
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
 * Returns the radius of the specific mob generator. Normally, this returns the
 * type's radius, but if the type/radius is invalid, it returns a default.
 */
float area_editor::get_mob_gen_radius(mob_gen* m) {
    return m->type ? m->type->radius == 0 ? 16 : m->type->radius : 16;
}


/* ----------------------------------------------------------------------------
 * Returns the mob currently under the specified point, or NULL if none.
 */
mob_gen* area_editor::get_mob_under_point(const point &p) {
    for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_data.mob_generators[m];
        
        if(
            dist(m_ptr->pos, p) <= get_mob_gen_radius(m_ptr)
        ) {
            return m_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * For a given vertex, returns the edge closest to the given angle, in the
 * given direction.
 * v_ptr:           Pointer to the vertex.
 * angle:           Angle coming into the vertex.
 * clockwise:       Return the closest edge clockwise?
 * closest_edge_angle: If not NULL, the angle the edge makes into its
 *   other vertex is returned here.
 */
edge* area_editor::get_closest_edge_to_angle(
    vertex* v_ptr, const float angle, const bool clockwise,
    float* closest_edge_angle
) {
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
 * Returns the path stop currently under the specified point, or NULL if none.
 */
path_stop* area_editor::get_path_stop_under_point(const point &p) {
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];
        
        if(dist(s_ptr->pos, p) <= PATH_STOP_RADIUS) {
            return s_ptr;
        }
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns the sector currently under the specified point, or NULL if none.
 */
sector* area_editor::get_sector_under_point(const point &p) {
    return get_sector(p, NULL, false);
}


/* ----------------------------------------------------------------------------
 * Returns the vertex currently under the specified point, or NULL if none.
 */
vertex* area_editor::get_vertex_under_point(const point &p) {
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        
        if(
            rectangles_intersect(
                p - (4 / cam_zoom),
                p + (4 / cam_zoom),
                point(
                    v_ptr->x - (4 / cam_zoom),
                    v_ptr->y - (4 / cam_zoom)
                ),
                point(
                    v_ptr->x + (4 / cam_zoom),
                    v_ptr->y + (4 / cam_zoom)
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
    mob_gen* base = *selected_mobs.begin();
    for(auto m = selected_mobs.begin(); m != selected_mobs.end(); ++m) {
        if(m == selected_mobs.begin()) continue;
        mob_gen* m_ptr = *m;
        m_ptr->category = base->category;
        m_ptr->type = base->type;
        m_ptr->angle = base->angle;
        m_ptr->vars = base->vars;
    }
}


/* ----------------------------------------------------------------------------
 * Homogenizes all selected sectors,
 * based on the one at the head of the selection.
 */
void area_editor::homogenize_selected_sectors() {
    sector* base = *selected_sectors.begin();
    for(auto s = selected_sectors.begin(); s != selected_sectors.end(); ++s) {
        if(s == selected_sectors.begin()) continue;
        base->clone(*s);
        update_sector_texture(*s, base->texture_info.file_name);
    }
}


/* ----------------------------------------------------------------------------
 * Load the area from the disk.
 * from_backup: If false, load it normally. If true, load from a backup, if any.
 */
void area_editor::load_area(const bool from_backup) {
    clear_current_area();
    
    ::load_area(cur_area_name, true, from_backup);
    
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        check_edge_intersections(cur_area_data.vertexes[v]);
    }
    
    //Calculate texture suggestions.
    map<string, size_t> texture_uses_map;
    vector<pair<string, size_t> > texture_uses_vector;
    
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        string n = cur_area_data.sectors[s]->texture_info.file_name;
        if(n.empty()) continue;
        texture_uses_map[n]++;
    }
    for(auto u = texture_uses_map.begin(); u != texture_uses_map.end(); ++u) {
        texture_uses_vector.push_back(make_pair(u->first, u->second));
    }
    sort(
        texture_uses_vector.begin(), texture_uses_vector.end(),
    [] (pair<string, size_t> u1, pair<string, size_t> u2) -> bool {
        return u1.second > u2.second;
    }
    );
    
    for(
        size_t u = 0;
        u < texture_uses_vector.size() && u < MAX_TEXTURE_SUGGESTIONS;
        ++u
    ) {
        texture_suggestions.push_back(
            texture_suggestion(texture_uses_vector[u].first)
        );
    }
    
    //TODO change_reference(reference_file_name);
    
    enable_widget(gui->widgets["frm_options"]->widgets["but_load"]);
    made_changes = false;
    
    cam_zoom = 1.0f;
    cam_pos = point();
}


/* ----------------------------------------------------------------------------
 * Snaps a point to the nearest grid space.
 */
point area_editor::snap_to_grid(const point &p) {
    if(is_shift_pressed) return p;
    return
        point(
            round(p.x / grid_interval) * grid_interval,
            round(p.y / grid_interval) * grid_interval
        );
}


/* ----------------------------------------------------------------------------
 * Splits an edge into two, at the specified point, and returns the
 * newly-created vertex. The new vertex gets added to the current area.
 */
vertex* area_editor::split_edge(edge* e_ptr, const point &where) {
    point new_v_pos =
        get_closest_point_in_line(
            point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
            point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y),
            where
        );
        
    //Create the new vertex and the new edge.
    vertex* new_v_ptr = cur_area_data.new_vertex();
    new_v_ptr->x = new_v_pos.x;
    new_v_ptr->y = new_v_pos.y;
    edge* new_e_ptr = cur_area_data.new_edge();
    
    //Connect the vertexes and edges.
    cur_area_data.connect_edge_to_vertex(new_e_ptr, new_v_ptr, 0);
    cur_area_data.connect_edge_to_vertex(new_e_ptr, e_ptr->vertexes[1], 1);
    cur_area_data.connect_edge_to_vertex(e_ptr, new_v_ptr, 1);
    
    //Connect the sectors and new edge.
    if(e_ptr->sectors[0]) {
        cur_area_data.connect_edge_to_sector(new_e_ptr, e_ptr->sectors[0], 0);
    }
    if(e_ptr->sectors[1]) {
        cur_area_data.connect_edge_to_sector(new_e_ptr, e_ptr->sectors[1], 1);
    }
    
    return new_v_ptr;
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void area_editor::unload() {
    //TODO
    clear_current_area();
    
    delete(gui->style);
    delete(gui);
    
    unload_hazards();
    unload_mob_types(false);
    unload_status_types(false);
    
    icons.clear();
}


/* ----------------------------------------------------------------------------
 * Updates a sector's texture.
 */
void area_editor::update_sector_texture(sector* s_ptr, const string file_name) {
    bitmaps.detach(
        TEXTURES_FOLDER_NAME + "/" + s_ptr->texture_info.file_name
    );
    s_ptr->texture_info.file_name = file_name;
    s_ptr->texture_info.bitmap =
        bitmaps.get(TEXTURES_FOLDER_NAME + "/" + file_name);
}


/* ----------------------------------------------------------------------------
 * Updates the list of texture suggestions, adding a new one or bumping it up.
 */
void area_editor::update_texture_suggestions(const string &n) {
    //First, check if it exists.
    size_t pos = INVALID;
    
    for(size_t s = 0; s < texture_suggestions.size(); ++s) {
        if(texture_suggestions[s].name == n) {
            pos = s;
            break;
        }
    }
    
    if(pos == 0) {
        //Already #1? Never mind.
        return;
    } else if(pos == INVALID) {
        //If it doesn't exist, create it and add it to the top.
        texture_suggestions.insert(
            texture_suggestions.begin(),
            texture_suggestion(n)
        );
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        texture_suggestion s = texture_suggestions[pos];
        texture_suggestions.erase(texture_suggestions.begin() + pos);
        texture_suggestions.insert(texture_suggestions.begin(), s);
    }
    
    if(texture_suggestions.size() > MAX_TEXTURE_SUGGESTIONS) {
        texture_suggestions[texture_suggestions.size() - 1].destroy();
        texture_suggestions.erase(
            texture_suggestions.begin() + texture_suggestions.size() - 1
        );
    }
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void area_editor::update_transformations() {
    //World coordinates to screen coordinates.
    world_to_screen_transform = identity_transform;
    al_translate_transform(
        &world_to_screen_transform,
        -cam_pos.x + gui_x / 2.0 / cam_zoom,
        -cam_pos.y + status_bar_y / 2.0 / cam_zoom
    );
    al_scale_transform(&world_to_screen_transform, cam_zoom, cam_zoom);
    
    //Screen coordinates to world coordinates.
    screen_to_world_transform = world_to_screen_transform;
    al_invert_transform(&screen_to_world_transform);
}


/* ----------------------------------------------------------------------------
 * Zooms in or out to a specific amount, optionally keeping the mouse cursor
 * in the same spot.
 */
void area_editor::zoom(const float new_zoom, const bool anchor_cursor) {
    cam_zoom =
        clamp(new_zoom, ZOOM_MIN_LEVEL_EDITOR, ZOOM_MAX_LEVEL_EDITOR);
        
    if(anchor_cursor) {
        //Keep a backup of the old mouse coordinates.
        point old_mouse_pos = mouse_cursor_w;
        
        //Figure out where the mouse will be after the zoom.
        update_transformations();
        mouse_cursor_w = mouse_cursor_s;
        al_transform_coordinates(
            &screen_to_world_transform,
            &mouse_cursor_w.x, &mouse_cursor_w.y
        );
        
        //Readjust the transformation by shifting the camera
        //so that the cursor ends up where it was before.
        cam_pos.x += (old_mouse_pos.x - mouse_cursor_w.x);
        cam_pos.y += (old_mouse_pos.y - mouse_cursor_w.y);
    }
    
    update_transformations();
}


/* ----------------------------------------------------------------------------
 * Creates a texture suggestion.
 */
area_editor::texture_suggestion::texture_suggestion(const string &n) :
    bmp(NULL),
    name(n) {
    
    bmp = bitmaps.get(TEXTURES_FOLDER_NAME + "/" + name, NULL, false);
}


/* ----------------------------------------------------------------------------
 * Destroys a texture suggestion.
 */
void area_editor::texture_suggestion::destroy() {
    bitmaps.detach(TEXTURES_FOLDER_NAME + "/" + name);
}
