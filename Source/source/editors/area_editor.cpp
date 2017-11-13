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
const float area_editor::DEBUG_TEXT_SCALE = 1.3f;
//Default grid interval.
const float area_editor::DEF_GRID_INTERVAL = 32.0f;
//Time until the next click is no longer considered a double-click.
const float area_editor::DOUBLE_CLICK_TIMEOUT = 0.5f;
//How much to zoom in/out with the keyboard keys.
const float area_editor::KEYBOARD_CAM_ZOOM = 0.25f;
//Maximum number of points that a circle sector can be created with.
const unsigned char area_editor::MAX_CIRCLE_SECTOR_POINTS = 32;
//Maximum number of texture suggestions.
const size_t area_editor::MAX_TEXTURE_SUGGESTIONS = 20;
//Minimum number of points that a circle sector can be created with.
const unsigned char area_editor::MIN_CIRCLE_SECTOR_POINTS = 3;
//If the mouse is dragged outside of this range, that's a real drag.
const float area_editor::MOUSE_DRAG_CONFIRM_RANGE = 4.0f;
//How long to tint the new sector's line(s) red for.
const float area_editor::NEW_SECTOR_ERROR_TINT_DURATION = 1.5f;
//Thickness to use when drawing a path link line.
const float area_editor::PATH_LINK_THICKNESS = 2.0f;
//Radius to use when drawing a path preview checkpoint.
const float area_editor::PATH_PREVIEW_CHECKPOINT_RADIUS = 8.0f;
//Only fetch the path these many seconds after the player stops the checkpoints.
const float area_editor::PATH_PREVIEW_TIMER_DUR = 0.1f;
//Radius to use when drawing a path stop circle.
const float area_editor::PATH_STOP_RADIUS = 16.0f;
//Scale the letters on the "points" of various features by this much.
const float area_editor::POINT_LETTER_TEXT_SCALE = 1.5f;
//Color of a selected element, or the selection box.
const unsigned char area_editor::SELECTION_COLOR[3] = {255, 215, 0};
//Speed at which the selection effect's "wheel" spins, in radians per second.
const float area_editor::SELECTION_EFFECT_SPEED = M_PI * 4;
//How long to override the status bar text for, for important messages.
const float area_editor::STATUS_OVERRIDE_IMPORTANT_DURATION = 6.0f;
//How long to override the status bar text for, for unimportant messages.
const float area_editor::STATUS_OVERRIDE_UNIMPORTANT_DURATION = 2.0f;
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
const string area_editor::ICON_SELECT_NONE =
    EDITOR_ICONS_FOLDER_NAME + "/Select_none.png";
const string area_editor::ICON_SELECT_EDGES =
    EDITOR_ICONS_FOLDER_NAME + "/Select_edges.png";
const string area_editor::ICON_SELECT_SECTORS =
    EDITOR_ICONS_FOLDER_NAME + "/Select_sectors.png";
const string area_editor::ICON_SELECT_VERTEXES =
    EDITOR_ICONS_FOLDER_NAME + "/Select_vertexes.png";


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
 * Creates a layout drawing node with no info.
 */
area_editor::layout_drawing_node::layout_drawing_node() :
    on_vertex(nullptr),
    on_vertex_nr(INVALID),
    on_edge(nullptr),
    on_edge_nr(INVALID),
    on_sector(nullptr),
    on_sector_nr(INVALID),
    is_new_vertex(false) {
    
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
    grid_interval(DEF_GRID_INTERVAL),
    is_ctrl_pressed(false),
    is_shift_pressed(false),
    is_gui_focused(false),
    last_mouse_click(INVALID),
    mouse_drag_confirmed(false),
    moving(false),
    moving_path_preview_checkpoint(-1),
    moving_cross_section_point(-1),
    new_sector_error_tint_timer(NEW_SECTOR_ERROR_TINT_DURATION),
    path_drawing_normals(true),
    path_preview_timer(0),
    selected_shadow(nullptr),
    selecting(false),
    selection_effect(0),
    selection_filter(SELECTION_FILTER_SECTORS),
    show_closest_stop(false),
    show_path_preview(false),
    status_override_timer(STATUS_OVERRIDE_IMPORTANT_DURATION),
    show_reference(false) {
    
    path_preview_timer =
    timer(PATH_PREVIEW_TIMER_DUR, [this] () {calculate_preview_path();});
}


/* ----------------------------------------------------------------------------
 * Checks whether it's possible to traverse from drawing node n1 to n2
 * with the existing edges and vertexes. In other words, if you draw a line
 * between n1 and n2, it will not go inside a sector.
 */
bool area_editor::are_nodes_traversable(
    const layout_drawing_node &n1, const layout_drawing_node &n2
) {
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
void area_editor::calculate_preview_path() {
    if(!show_path_preview) return;
    
    float d = 0;
    path_preview =
        get_path(
            path_preview_checkpoints[0],
            path_preview_checkpoints[1],
            NULL, NULL, &d
        );
        
    if(path_preview.empty() && d == 0) {
        d =
            dist(
                path_preview_checkpoints[0],
                path_preview_checkpoints[1]
            ).to_float();
    }
    
    ((lafi::label*) gui->widgets["frm_paths"]->widgets["lbl_path_dist"])->text =
        "  Total dist.: " + f2s(d);
}


/* ----------------------------------------------------------------------------
 * Cancels the circular sector creation operation and returns to normal.
 */
void area_editor::cancel_circle_sector() {
    clear_circle_sector();
    sub_state = EDITOR_SUB_STATE_NONE;
}


/* ----------------------------------------------------------------------------
 * Cancels the edge drawing operation and returns to normal.
 */
void area_editor::cancel_layout_drawing() {
    clear_layout_drawing();
    sub_state = EDITOR_SUB_STATE_NONE;
}


/* ----------------------------------------------------------------------------
 * Cancels the vertex moving operation.
 */
void area_editor::cancel_layout_moving() {
    for(auto v = selected_vertexes.begin(); v != selected_vertexes.end(); ++v) {
        (*v)->x = pre_move_vertex_coords[*v].x;
        (*v)->y = pre_move_vertex_coords[*v].y;
    }
    clear_layout_moving();
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
    
    zoom(z, false);
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
    
    bool prev_node_on_sector =
        (!prev_node->on_edge && !prev_node->on_vertex);
    bool tent_node_on_sector =
        (!tentative_node.on_edge && !tentative_node.on_vertex);
        
    if(
        !prev_node_on_sector && !tent_node_on_sector &&
        !are_nodes_traversable(*prev_node, tentative_node)
    ) {
        //Useful check if, for instance, you have a square in the middle
        //of your working sector, you draw a node to the left of the square,
        //a node on the square's left line, and then a node on the square's
        //right line. Technically, these last two nodes are related to the
        //outer sector, but shouldn't be allowed because the line between them
        //goes through a different sector.
        point center =
            (prev_node->snapped_spot + tentative_node.snapped_spot) / 2;
        sector* crossing_sector = get_sector(center, NULL, false);
        if(common_sectors.find(crossing_sector) == common_sectors.end()) {
            drawing_line_error = DRAWING_LINE_WAYWARD_SECTOR;
            return;
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
}


/* ----------------------------------------------------------------------------
 * Clears the data about the circular sector creation.
 */
void area_editor::clear_circle_sector() {
    new_circle_sector_step = 0;
    new_circle_sector_points.clear();
}


/* ----------------------------------------------------------------------------
 * Clears the currently loaded area data.
 */
void area_editor::clear_current_area() {
    clear_current_area_gui();
    
    //TODO
    /*
    change_reference("");
    reference_aspect_ratio = true;
    reference_size = point(1000, 1000);
    reference_a = 255;
    cur_sector = NULL;
    cur_mob = NULL;
    cur_shadow = NULL;
    */
    clear_area_textures();
    
    for(size_t s = 0; s < cur_area_data.tree_shadows.size(); ++s) {
        bitmaps.detach(
            TEXTURES_FOLDER_NAME + "/" +
            cur_area_data.tree_shadows[s]->file_name
        );
    }
    
    sector_to_gui();
    mob_to_gui();
    //TODO
    //reference_to_gui();
    
    cam_pos.x = cam_pos.y = 0;
    cam_zoom = 1;
    //TODO
    /*
    show_cross_section = false;
    show_cross_section_grid = false;
    show_path_preview = false;
    path_preview.clear();
    path_preview_checkpoints[0] = point(-DEF_GRID_INTERVAL, 0);
    path_preview_checkpoints[1] = point(DEF_GRID_INTERVAL, 0);
    cross_section_points[0] = point(-DEF_GRID_INTERVAL, 0);
    cross_section_points[1] = point(DEF_GRID_INTERVAL, 0);
    */
    
    clear_texture_suggestions();
    
    cur_area_data.clear();
    
    made_changes = false;
    backup_timer.start(editor_backup_interval);
    
    state = EDITOR_STATE_MAIN;
    change_to_right_frame();
}


/* ----------------------------------------------------------------------------
 * Clears the data about the layout drawing.
 */
void area_editor::clear_layout_drawing() {
    drawing_nodes.clear();
    drawing_line_error = DRAWING_LINE_NO_ERROR;
}


/* ----------------------------------------------------------------------------
 * Clears the data about the layout moving.
 */
void area_editor::clear_layout_moving() {
    pre_move_vertex_coords.clear();
    pre_move_area_data.clear();
    clear_selection();
    moving = false;
}


/* ----------------------------------------------------------------------------
 * Clears the data about the current selection.
 */
void area_editor::clear_selection() {
    selected_vertexes.clear();
    selected_edges.clear();
    selected_sectors.clear();
    selected_mobs.clear();
    selected_path_stops.clear();
    selected_path_links.clear();
    selection_homogenized = false;
    
    asa_to_gui();
    asb_to_gui();
    sector_to_gui();
    mob_to_gui();
    path_to_gui();
}


/* ----------------------------------------------------------------------------
 * Clears the list of texture suggestions. This frees up the bitmaps.
 */
void area_editor::clear_texture_suggestions() {
    for(size_t s = 0; s < texture_suggestions.size(); ++s) {
        texture_suggestions[s].destroy();
    }
    texture_suggestions.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a new item from the picker frame, given its name.
 */
void area_editor::create_new_from_picker(const string &name) {
    //TODO;
}


/* ----------------------------------------------------------------------------
 * Deletes the selected path links and/or stops.
 */
void area_editor::delete_selected_path_elements() {
    for(
        auto l = selected_path_links.begin();
        l != selected_path_links.end(); ++l
    ) {
        (*l).first->remove_link((*l).second);
    }
    selected_path_links.clear();
    
    for(
        auto s = selected_path_stops.begin();
        s != selected_path_stops.end(); ++s
    ) {
        //Check all links to this stop.
        for(size_t s2 = 0; s2 < cur_area_data.path_stops.size(); ++s2) {
            path_stop* s2_ptr = cur_area_data.path_stops[s2];
            for(size_t l = 0; l < s2_ptr->links.size(); ++l) {
                if(s2_ptr->links[l].end_ptr == *s) {
                    s2_ptr->links.erase(s2_ptr->links.begin() + l);
                    break;
                }
            }
        }
        
        //Finally, delete the stop.
        delete *s;
        for(size_t s2 = 0; s2 < cur_area_data.path_stops.size(); ++s2) {
            if(cur_area_data.path_stops[s2] == *s) {
                cur_area_data.path_stops.erase(
                    cur_area_data.path_stops.begin() + s2
                );
                break;
            }
        }
    }
    selected_path_stops.clear();
    
    path_preview.clear(); //Clear so it doesn't reference deleted stops.
    path_preview_timer.start(false);
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
    new_sector_error_tint_timer.tick(delta_t);
    status_override_timer.tick(delta_t);
    
    if(!cur_area_name.empty() && editor_backup_interval > 0) {
        backup_timer.tick(delta_t);
    }
    
    fade_mgr.tick(delta_t);
    
    selection_effect += SELECTION_EFFECT_SPEED * delta_t;
    
}


/* ----------------------------------------------------------------------------
 * Emits a message onto the status bar, and keeps it there for some seconds.
 * text:      Message text.
 * important: If true, the message stays for a few more seconds than normal.
 */
void area_editor::emit_status_bar_message(
    const string &text, const bool important
) {
    status_override_text = text;
    status_override_timer.start(
        important ?
        STATUS_OVERRIDE_IMPORTANT_DURATION :
        STATUS_OVERRIDE_UNIMPORTANT_DURATION
    );
    lbl_status_bar->text = status_override_text;
}


/* ----------------------------------------------------------------------------
 * Emits a message onto the status bar, based on the given triangulation error.
 */
void area_editor::emit_triangulation_error_status_bar_message(
    const TRIANGULATION_ERRORS error
) {
    if(error == TRIANGULATION_ERROR_LONE_EDGES) {
        emit_status_bar_message(
            "Some sectors ended up with lone edges!", true
        );
    } else if(error == TRIANGULATION_ERROR_NO_EARS) {
        emit_status_bar_message(
            "Some sectors could not be triangulated!", true
        );
    } else if(error == TRIANGULATION_ERROR_VERTEXES_REUSED) {
        emit_status_bar_message(
            "Some sectors reuse vertexes -- there are likely gaps!", true
        );
    }
}


/* ----------------------------------------------------------------------------
 * Finishes drawing a circular sector.
 */
void area_editor::finish_circle_sector() {
    clear_layout_drawing();
    for(size_t p = 0; p < new_circle_sector_points.size(); ++p) {
        layout_drawing_node n;
        n.raw_spot = new_circle_sector_points[p];
        n.snapped_spot = n.raw_spot;
        n.on_sector = get_sector(n.raw_spot, NULL, false);
        drawing_nodes.push_back(n);
    }
    finish_layout_drawing();
    
    clear_circle_sector();
    sub_state = EDITOR_SUB_STATE_NONE;
}


/* ----------------------------------------------------------------------------
 * Finishes the layout drawing operation, and tries to create whatever sectors.
 */
void area_editor::finish_layout_drawing() {
    if(drawing_nodes.size() < 3) {
        cancel_layout_drawing();
        return;
    }
    
    TRIANGULATION_ERRORS last_triangulation_error = TRIANGULATION_NO_ERROR;
    
    //This is the basic idea: create a new sector using the
    //vertexes provided by the user, as a "child" of an existing sector.
    
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
    
    bool is_clockwise = is_polygon_clockwise(drawing_vertexes);
    
    //Organize all edges such that their vertexes v1 and v2 are also in the same
    //order as the vertex order in the drawing.
    for(size_t e = 0; e < drawing_edges.size(); ++e) {
        if(drawing_edges[e]->vertexes[1] != drawing_vertexes[e]) {
            drawing_edges[e]->swap_vertexes();
        }
    }
    
    //Connect the edges to the sectors.
    unsigned char inner_sector_side = (is_clockwise ? 1 : 0);
    unsigned char outer_sector_side = (is_clockwise ? 0 : 1);
    
    map<edge*, sector*[2]> edge_sector_backups;
    
    for(size_t e = 0; e < drawing_edges.size(); ++e) {
        edge* e_ptr = drawing_edges[e];
        
        if(!e_ptr->sectors[0] && !e_ptr->sectors[1]) {
            //If it's a new edge, set it up properly.
            cur_area_data.connect_edge_to_sector(
                e_ptr, outer_sector, outer_sector_side
            );
            cur_area_data.connect_edge_to_sector(
                e_ptr, new_sector, inner_sector_side
            );
            
        } else {
            //If not, let's just add the info for the new sector,
            //and keep the information from the previous sector it was
            //pointing to. This will be cleaned up later on.
            edge_sector_backups[e_ptr][0] = e_ptr->sectors[0];
            edge_sector_backups[e_ptr][1] = e_ptr->sectors[1];
            
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
    set<edge*> triangulation_lone_edges;
    TRIANGULATION_ERRORS triangulation_error =
        triangulate(new_sector, &triangulation_lone_edges, true, false);
        
    if(triangulation_error == TRIANGULATION_NO_ERROR) {
        auto it = non_simples.find(new_sector);
        if(it != non_simples.end()) {
            non_simples.erase(it);
        }
    } else {
        non_simples[new_sector] = triangulation_error;
        last_triangulation_error = triangulation_error;
    }
    lone_edges.insert(
        triangulation_lone_edges.begin(),
        triangulation_lone_edges.end()
    );
    
    //All sectors inside the new one need to know that
    //their outer sector changed.
    unordered_set<edge*> inner_edges;
    for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
        vertex* v1_ptr = cur_area_data.edges[e]->vertexes[0];
        vertex* v2_ptr = cur_area_data.edges[e]->vertexes[1];
        if(
            is_point_in_sector(point(v1_ptr->x, v1_ptr->y), new_sector) &&
            is_point_in_sector(point(v2_ptr->x, v2_ptr->y), new_sector)
        ) {
            inner_edges.insert(cur_area_data.edges[e]);
        }
    }
    
    for(auto i = inner_edges.begin(); i != inner_edges.end(); ++i) {
        auto de_it = find(drawing_edges.begin(), drawing_edges.end(), *i);
        
        if(de_it != drawing_edges.end()) {
            //If this edge is a part of the drawing, then we know
            //that it's already set correctly from previous parts of
            //the algorithm. However, in the case where the new sector
            //is on the outside (i.e. this edge is both inside AND a neighbor)
            //then let's simplify the procedure and remove this edge from
            //the new sector, letting it keep its old data.
            //The new sector will still be closed using other edges; that's
            //guaranteed.
            if((*i)->sectors[outer_sector_side] == new_sector) {
                new_sector->remove_edge(*i);
                cur_area_data.connect_edge_to_sector(
                    *i, edge_sector_backups[*i][0], 0
                );
                cur_area_data.connect_edge_to_sector(
                    *i, edge_sector_backups[*i][1], 1
                );
                drawing_edges.erase(de_it);
            }
            
        } else {
            for(size_t s = 0; s < 2; ++s) {
                if((*i)->sectors[s] == outer_sector) {
                    cur_area_data.connect_edge_to_sector(*i, new_sector, s);
                }
            }
            
        }
    }
    
    //Final triangulations.
    triangulation_lone_edges.clear();
    triangulation_error =
        triangulate(new_sector, &triangulation_lone_edges, true, true);
        
    if(triangulation_error == TRIANGULATION_NO_ERROR) {
        auto it = non_simples.find(new_sector);
        if(it != non_simples.end()) {
            non_simples.erase(it);
        }
    } else {
        non_simples[new_sector] = triangulation_error;
        last_triangulation_error = triangulation_error;
    }
    lone_edges.insert(
        triangulation_lone_edges.begin(),
        triangulation_lone_edges.end()
    );
    
    if(outer_sector) {
        triangulation_error =
            triangulate(outer_sector, &triangulation_lone_edges, true, true);
            
        if(triangulation_error == TRIANGULATION_NO_ERROR) {
            auto it = non_simples.find(outer_sector);
            if(it != non_simples.end()) {
                non_simples.erase(it);
            }
        } else {
            non_simples[outer_sector] = triangulation_error;
            last_triangulation_error = triangulation_error;
        }
        lone_edges.insert(
            triangulation_lone_edges.begin(),
            triangulation_lone_edges.end()
        );
    }
    
    if(last_triangulation_error != TRIANGULATION_NO_ERROR) {
        emit_triangulation_error_status_bar_message(last_triangulation_error);
    }
    
    cur_area_data.check_matches(); //TODO
    
    //Select the new sector, making it ready for editing.
    clear_selection();
    select_sector(new_sector);
    sector_to_gui();
    
    clear_layout_drawing();
    sub_state = EDITOR_SUB_STATE_NONE;
}


/* ----------------------------------------------------------------------------
 * Finishes a vertex moving procedure.
 */
void area_editor::finish_layout_moving() {
    TRIANGULATION_ERRORS last_triangulation_error = TRIANGULATION_NO_ERROR;
    
    unordered_set<sector*> affected_sectors =
        get_affected_sectors(selected_vertexes);
    map<vertex*, vertex*> merges;
    map<vertex*, edge*> edges_to_split;
    unordered_set<sector*> merge_affected_sectors;
    
    //Find merge vertexes and edges to split, if any.
    for(auto v = selected_vertexes.begin(); v != selected_vertexes.end(); ++v) {
        point p((*v)->x, (*v)->y);
        vertex* merge_v =
            get_merge_vertex(
                p, cur_area_data.vertexes,
                cam_zoom / VERTEX_MERGE_RADIUS, NULL, *v
            );
            
        if(merge_v) {
            merges[*v] = merge_v;
            
        } else {
            edge* e_ptr = NULL;
            
            do {
                e_ptr = get_edge_under_point(p, e_ptr);
            } while(e_ptr != NULL && (*v)->has_edge(e_ptr));
            
            if(e_ptr) {
                edges_to_split[*v] = e_ptr;
            }
        }
    }
    
    set<edge*> moved_edges;
    for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
        edge* e_ptr = cur_area_data.edges[e];
        bool both_selected = true;
        for(size_t v = 0; v < 2; ++v) {
            if(
                selected_vertexes.find(e_ptr->vertexes[v]) ==
                selected_vertexes.end()
            ) {
                both_selected = false;
                break;
            }
        }
        if(both_selected) {
            moved_edges.insert(e_ptr);
        }
    }
    
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        point p(v_ptr->x, v_ptr->y);
        
        if(selected_vertexes.find(v_ptr) != selected_vertexes.end()) {
            continue;
        }
        bool is_merge_target = false;
        for(auto m = merges.begin(); m != merges.end(); ++m) {
            if(m->second == v_ptr) {
                //This vertex will have some other vertex merge into it; skip.
                is_merge_target = true;
                break;
            }
        }
        if(is_merge_target) continue;
        
        edge* e_ptr = NULL;
        do {
            e_ptr = get_edge_under_point(p, e_ptr);
        } while(
            e_ptr != NULL &&
            moved_edges.find(e_ptr) == moved_edges.end()
        );
        if(e_ptr) {
            edges_to_split[v_ptr] = e_ptr;
        }
    }
    
    //Before moving on and making changes, let's check for crossing edges,
    //but removing all of the ones that come from edge splits or vertex merges.
    vector<edge_intersection> intersections =
        get_intersecting_edges();
    for(auto m = merges.begin(); m != merges.end(); ++m) {
        for(size_t e1 = 0; e1 < m->first->edges.size(); ++e1) {
            for(size_t e2 = 0; e2 < m->second->edges.size(); ++e2) {
                for(size_t i = 0; i < intersections.size();) {
                    if(
                        intersections[i].contains(m->first->edges[e1]) &&
                        intersections[i].contains(m->second->edges[e2])
                    ) {
                        intersections.erase(intersections.begin() + i);
                    } else {
                        ++i;
                    }
                }
            }
        }
    }
    for(auto v = edges_to_split.begin(); v != edges_to_split.end(); ++v) {
        for(size_t e = 0; e < v->first->edges.size(); ++e) {
            for(size_t i = 0; i < intersections.size();) {
                if(
                    intersections[i].contains(v->first->edges[e]) &&
                    intersections[i].contains(v->second)
                ) {
                    intersections.erase(intersections.begin() + i);
                } else {
                    ++i;
                }
            }
        }
    }
    
    //If we ended up with any intersection still, abort!
    if(!intersections.empty()) {
        emit_status_bar_message(
            "That move would cause edges to intersect!", true
        );
        cancel_layout_moving();
        return;
    }
    
    //Merge vertexes and split edges now.
    for(auto v = edges_to_split.begin(); v != edges_to_split.end(); ++v) {
        merges[v->first] =
            split_edge(v->second, point((v->first)->x, v->first->y));
    }
    for(auto m = merges.begin(); m != merges.end(); ++m) {
        merge_vertex(m->second, m->first, &merge_affected_sectors);
    }
    
    affected_sectors.insert(
        merge_affected_sectors.begin(), merge_affected_sectors.end()
    );
    
    //Triangulate all affected sectors.
    for(auto s = affected_sectors.begin(); s != affected_sectors.end(); ++s) {
        if(!(*s)) continue;
        
        set<edge*> triangulation_lone_edges;
        TRIANGULATION_ERRORS triangulation_error =
            triangulate(*s, &triangulation_lone_edges, true, true);
        if(triangulation_error == TRIANGULATION_NO_ERROR) {
            auto it = non_simples.find(*s);
            if(it != non_simples.end()) {
                non_simples.erase(it);
            }
        } else {
            non_simples[*s] = triangulation_error;
            last_triangulation_error = triangulation_error;
        }
    }
    
    if(last_triangulation_error != TRIANGULATION_NO_ERROR) {
        emit_triangulation_error_status_bar_message(last_triangulation_error);
    }
    
    clear_layout_moving();
}


/* ----------------------------------------------------------------------------
 * Returns a sector common to all vertexes and edges.
 * A sector is considered this if a vertex has it as a sector of
 * a neighboring edge, or if a vertex is inside it.
 * Use the former for vertexes that will be merged, and the latter
 * for vertexes that won't.
 * vertexes: List of vertexes to check.
 * edges:    List of edges to check.
 * result:   Returns the common sector here.
 * Returns false if there is no common sector. True otherwise.
 */
bool area_editor::get_common_sector(
    vector<vertex*> &vertexes, vector<edge*> &edges, sector** result
) {
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
    for(auto s = sectors.begin(); s != sectors.end(); ++s) {
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
        layout_drawing_node* n1 = &drawing_nodes[n];
        layout_drawing_node* n2 = &(get_next_in_vector(drawing_nodes, n));
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
 * p:     The point.
 * after: Only check edges that come after this one.
 */
edge* area_editor::get_edge_under_point(const point &p, edge* after) {
    bool found_after = (!after ? true : false);
    
    for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
        edge* e_ptr = cur_area_data.edges[e];
        if(e_ptr == after) {
            found_after = true;
            continue;
        } else if(!found_after) {
            continue;
        }
        
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
 * Returns which edges are crossing against other edges, if any.
 */
vector<edge_intersection> area_editor::get_intersecting_edges() {
    vector<edge_intersection> intersections;
    
    for(size_t e1 = 0; e1 < cur_area_data.edges.size(); ++e1) {
        edge* e1_ptr = cur_area_data.edges[e1];
        for(size_t e2 = e1 + 1; e2 < cur_area_data.edges.size(); ++e2) {
            edge* e2_ptr = cur_area_data.edges[e2];
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
 * Returns all sectors affected by the specified vertexes.
 * This includes the NULL sector.
 */
unordered_set<sector*> area_editor::get_affected_sectors(
    set<vertex*> &vertexes
) {
    unordered_set<sector*> affected_sectors;
    for(auto v = vertexes.begin(); v != vertexes.end(); ++v) {
        for(size_t e = 0; e < (*v)->edges.size(); ++e) {
            affected_sectors.insert((*v)->edges[e]->sectors[0]);
            affected_sectors.insert((*v)->edges[e]->sectors[1]);
        }
    }
    return affected_sectors;
}


/* ----------------------------------------------------------------------------
 * Returns which layout element got clicked, if any.
 */
void area_editor::get_clicked_layout_element(
    vertex** clicked_vertex, edge** clicked_edge, sector** clicked_sector
) {
    *clicked_vertex = get_vertex_under_point(mouse_cursor_w);
    *clicked_edge = NULL;
    *clicked_sector = NULL;
    
    if(*clicked_vertex) return;
    
    if(selection_filter != SELECTION_FILTER_VERTEXES) {
        *clicked_edge = get_edge_under_point(mouse_cursor_w);
    }
    
    if(*clicked_edge) return;
    
    if(selection_filter == SELECTION_FILTER_SECTORS) {
        *clicked_sector = get_sector_under_point(mouse_cursor_w);
    }
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
 * Returns the path link currently under the specified point, or NULL if none.
 */
bool area_editor::get_path_link_under_point(
    const point &p,
    pair<path_stop*, path_stop*>* data1, pair<path_stop*, path_stop*>* data2
) {
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];
        for(size_t l = 0; l < s_ptr->links.size(); ++l) {
            path_stop* s2_ptr = s_ptr->links[l].end_ptr;
            if(
                circle_intersects_line(p, 8 / cam_zoom, s_ptr->pos, s2_ptr->pos)
            ) {
                *data1 = make_pair(s_ptr, s2_ptr);
                if(s2_ptr->has_link(s_ptr)) {
                    *data2 = make_pair(s2_ptr, s_ptr);
                } else {
                    *data2 = make_pair((path_stop*) NULL, (path_stop*) NULL);
                }
                return true;
            }
        }
    }
    
    return false;
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
 * Handles an error in the line the user is trying to draw.
 */
void area_editor::handle_line_error() {
    new_sector_error_tint_timer.start();
    if(drawing_line_error == DRAWING_LINE_CROSSES_DRAWING) {
        emit_status_bar_message(
            "That line crosses other lines in the drawing!", true
        );
    } else if(drawing_line_error == DRAWING_LINE_CROSSES_EDGES) {
        emit_status_bar_message(
            "That line crosses existing edges!", true
        );
    } else if(drawing_line_error == DRAWING_LINE_WAYWARD_SECTOR) {
        emit_status_bar_message(
            "That line goes out of the sector you're drawing on!", true
        );
    }
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
    
    emit_status_bar_message("Loaded successfully.", false);
}


/* ----------------------------------------------------------------------------
 * Merges vertex 1 into vertex 2.
 * v1:               Vertex that is being moved and will be merged.
 * v2:               Vertex that is going to absorb v1.
 * affected_sectors: List of sectors that will be affected by this merge.
 */
void area_editor::merge_vertex(
    vertex* v1, vertex* v2, unordered_set<sector*>* affected_sectors
) {
    vector<edge*> edges = v1->edges;
    //Find out what to do with every edge of the dragged vertex.
    for(size_t e = 0; e < edges.size(); ++e) {
    
        bool was_deleted = false;
        edge* e_ptr = edges[e];
        vertex* other_vertex = e_ptr->get_other_vertex(v1);
        
        if(other_vertex == v2) {
        
            //Squashed into non-existence.
            affected_sectors->insert(e_ptr->sectors[0]);
            affected_sectors->insert(e_ptr->sectors[1]);
            
            e_ptr->remove_from_vertexes();
            e_ptr->remove_from_sectors();
            
            //Delete it.
            cur_area_data.remove_edge(e_ptr);
            was_deleted = true;
            
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
                        cur_area_data.connect_edge_to_sector(
                            de_ptr, e_ptr->sectors[1], 0
                        );
                    } else if(e_ptr->sectors[0] == de_ptr->sectors[1]) {
                        cur_area_data.connect_edge_to_sector(
                            de_ptr, e_ptr->sectors[1], 1
                        );
                    } else if(e_ptr->sectors[1] == de_ptr->sectors[0]) {
                        cur_area_data.connect_edge_to_sector(
                            de_ptr, e_ptr->sectors[0], 0
                        );
                    } else if(e_ptr->sectors[1] == de_ptr->sectors[1]) {
                        cur_area_data.connect_edge_to_sector(
                            de_ptr, e_ptr->sectors[0], 1
                        );
                    }
                    
                    //Go to the edge's old vertexes and sextors
                    //and tell them that it no longer exists.
                    e_ptr->remove_from_vertexes();
                    e_ptr->remove_from_sectors();
                    
                    //Delete it.
                    cur_area_data.remove_edge(e_ptr);
                    was_deleted = true;
                    
                    break;
                }
            }
            
            //If it's matchless, that means it'll just be joined to
            //the group of edges on the destination vertex.
            if(!has_merged) {
                cur_area_data.connect_edge_to_vertex(
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
            ve_ptr->remove_from_sectors();
            ve_ptr->remove_from_vertexes();
            cur_area_data.remove_edge(ve_ptr);
        } else {
            ++ve;
        }
    }
    
    //Delete the old vertex.
    cur_area_data.remove_vertex(v1);
    
    //If any vertex or sector is out of edges, delete it.
    for(size_t v = 0; v < cur_area_data.vertexes.size();) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        if(v_ptr->edges.empty()) {
            cur_area_data.remove_vertex(v);
        } else {
            ++v;
        }
    }
    for(size_t s = 0; s < cur_area_data.sectors.size();) {
        sector* s_ptr = cur_area_data.sectors[s];
        if(s_ptr->edges.empty()) {
            cur_area_data.remove_sector(s);
        } else {
            ++s;
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Removes the selected sectors, if they are isolated.
 * Returns true on success.
 */
bool area_editor::remove_isolated_sectors() {
    map<sector*, sector*> alt_sectors;
    
    for(auto s = selected_sectors.begin(); s != selected_sectors.end(); ++s) {
        sector* s_ptr = *s;
        
        //If around the sector there are two different sectors, then
        //it's definitely connected.
        sector* alt_sector = NULL;
        bool got_an_alt_sector = false;
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            edge* e_ptr = s_ptr->edges[e];
            
            for(size_t s = 0; s < 2; ++s) {
                if(e_ptr->sectors[s] == s_ptr) {
                    //The main sector; never mind.
                    continue;
                }
                
                if(!got_an_alt_sector) {
                    alt_sector = e_ptr->sectors[s];
                    got_an_alt_sector = true;
                } else if(e_ptr->sectors[s] != alt_sector) {
                    //Different alternative sector found! No good.
                    return false;
                }
            }
        }
        
        alt_sectors[s_ptr] = alt_sector;
        
        //If any of the sector's vertexes have more than two edges, then
        //surely these vertexes are connected to other sectors.
        //Meaning our sector is not alone.
        for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
            edge* e_ptr = s_ptr->edges[e];
            for(size_t v = 0; v < 2; ++v) {
                if(e_ptr->vertexes[v]->edges.size() != 2) {
                    return false;
                }
            }
        }
    }
    
    TRIANGULATION_ERRORS last_triangulation_error = TRIANGULATION_NO_ERROR;
    
    //Remove the sectors now.
    for(auto s = selected_sectors.begin(); s != selected_sectors.end(); ++s) {
        sector* s_ptr = *s;
        
        vector<edge*> main_sector_edges = s_ptr->edges;
        unordered_set<vertex*> main_vertexes;
        for(size_t e = 0; e < main_sector_edges.size(); ++e) {
            edge* e_ptr = main_sector_edges[e];
            main_vertexes.insert(e_ptr->vertexes[0]);
            main_vertexes.insert(e_ptr->vertexes[1]);
            e_ptr->remove_from_sectors();
            e_ptr->remove_from_vertexes();
            cur_area_data.remove_edge(e_ptr);
        }
        
        for(auto v = main_vertexes.begin(); v != main_vertexes.end(); ++v) {
            cur_area_data.remove_vertex(*v);
        }
        
        cur_area_data.remove_sector(s_ptr);
        
        //Re-triangulate the outer sector.
        sector* alt_sector = alt_sectors[s_ptr];
        if(alt_sector) {
            set<edge*> triangulation_lone_edges;
            TRIANGULATION_ERRORS triangulation_error =
                triangulate(alt_sector, &triangulation_lone_edges, true, true);
                
            if(triangulation_error == TRIANGULATION_NO_ERROR) {
                auto it = non_simples.find(alt_sector);
                if(it != non_simples.end()) {
                    non_simples.erase(it);
                }
            } else {
                non_simples[alt_sector] = triangulation_error;
                last_triangulation_error = triangulation_error;
            }
            lone_edges.insert(
                triangulation_lone_edges.begin(),
                triangulation_lone_edges.end()
            );
        }
    }
    
    if(last_triangulation_error != TRIANGULATION_NO_ERROR) {
        emit_triangulation_error_status_bar_message(last_triangulation_error);
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Selects an edge and its vertexes.
 */
void area_editor::select_edge(edge* e) {
    if(selection_filter == SELECTION_FILTER_VERTEXES) return;
    selected_edges.insert(e);
    for(size_t v = 0; v < 2; ++v) {
        select_vertex(e->vertexes[v]);
    }
}


/* ----------------------------------------------------------------------------
 * Selects a sector and its edges and vertexes.
 */
void area_editor::select_sector(sector* s) {
    if(selection_filter != SELECTION_FILTER_SECTORS) return;
    selected_sectors.insert(s);
    for(size_t e = 0; e < s->edges.size(); ++e) {
        select_edge(s->edges[e]);
    }
}


/* ----------------------------------------------------------------------------
 * Selects a vertex.
 */
void area_editor::select_vertex(vertex* v) {
    selected_vertexes.insert(v);
}


/* ----------------------------------------------------------------------------
 * Sets the vector of points that make up a new circle sector.
 */
void area_editor::set_new_circle_sector_points() {
    float anchor_angle =
        get_angle(new_circle_sector_center, new_circle_sector_anchor);
    float cursor_angle =
        get_angle(new_circle_sector_center, mouse_cursor_w);
    float radius =
        dist(
            new_circle_sector_center, new_circle_sector_anchor
        ).to_float();
    float angle_dif =
        get_angle_smallest_dif(cursor_angle, anchor_angle);
        
    size_t n_points = MAX_CIRCLE_SECTOR_POINTS;
    if(angle_dif > 0) {
        n_points = round((M_PI * 2) / angle_dif);
    }
    n_points =
        clamp(n_points, MIN_CIRCLE_SECTOR_POINTS, MAX_CIRCLE_SECTOR_POINTS);
        
    new_circle_sector_points.clear();
    for(size_t p = 0; p < n_points; ++p) {
        float delta_a = ((M_PI * 2) / n_points) * p;
        new_circle_sector_points.push_back(
            point(
                new_circle_sector_center.x +
                radius * cos(anchor_angle + delta_a),
                new_circle_sector_center.y +
                radius * sin(anchor_angle + delta_a)
            )
        );
    }
    
    new_circle_sector_valid_edges.clear();
    for(size_t p = 0; p < n_points; ++p) {
        point next = get_next_in_vector(new_circle_sector_points, p);
        bool valid = true;
        
        for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
            edge* e_ptr = cur_area_data.edges[e];
            
            if(
                lines_intersect(
                    point(
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                    ),
                    point(
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    ),
                    new_circle_sector_points[p], next,
                    NULL, NULL
                )
            ) {
                valid = false;
                break;
            }
        }
        
        new_circle_sector_valid_edges.push_back(valid);
    }
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
 * Splits an edge into two, near the specified point, and returns the
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
 * Procedure to start moving the selected mobs.
 */
void area_editor::start_mob_move() {
    move_closest_mob = NULL;
    dist move_closest_mob_dist;
    for(auto m = selected_mobs.begin(); m != selected_mobs.end(); ++m) {
        pre_move_mob_coords[*m] = (*m)->pos;
        
        dist d(mouse_cursor_w, (*m)->pos);
        if(!move_closest_mob || d < move_closest_mob_dist) {
            move_closest_mob = *m;
            move_closest_mob_dist = d;
            move_closest_mob_start_pos = (*m)->pos;
        }
    }
    
    cur_area_data.clone(pre_move_area_data);
    
    move_mouse_start_pos = mouse_cursor_w;
    moving = true;
}


/* ----------------------------------------------------------------------------
 * Procedure to start moving the selected path stops.
 */
void area_editor::start_path_stop_move() {
    move_closest_stop = NULL;
    dist move_closest_stop_dist;
    for(
        auto s = selected_path_stops.begin();
        s != selected_path_stops.end(); ++s
    ) {
        pre_move_stop_coords[*s] = (*s)->pos;
        
        dist d(mouse_cursor_w, (*s)->pos);
        if(!move_closest_stop || d < move_closest_stop_dist) {
            move_closest_stop = *s;
            move_closest_stop_dist = d;
            move_closest_stop_start_pos = (*s)->pos;
        }
    }
    
    cur_area_data.clone(pre_move_area_data);
    
    move_mouse_start_pos = mouse_cursor_w;
    moving = true;
}


/* ----------------------------------------------------------------------------
 * Procedure to start moving the selected tree shadow.
 */
void area_editor::start_shadow_move() {
    cur_area_data.clone(pre_move_area_data);
    
    pre_move_shadow_coords = selected_shadow->center;
    
    move_mouse_start_pos = mouse_cursor_w;
    moving = true;
}


/* ----------------------------------------------------------------------------
 * Procedure to start moving the selected vertexes.
 */
void area_editor::start_vertex_move() {
    move_closest_vertex = NULL;
    dist move_closest_vertex_dist;
    for(auto v = selected_vertexes.begin(); v != selected_vertexes.end(); ++v) {
        point p((*v)->x, (*v)->y);
        pre_move_vertex_coords[*v] = p;
        
        dist d(mouse_cursor_w, p);
        if(!move_closest_vertex || d < move_closest_vertex_dist) {
            move_closest_vertex = *v;
            move_closest_vertex_dist = d;
            move_closest_vertex_start_pos = p;
        }
    }
    
    unordered_set<sector*> affected_sectors =
        get_affected_sectors(selected_vertexes);
        
    cur_area_data.clone(pre_move_area_data);
    
    move_mouse_start_pos = mouse_cursor_w;
    moving = true;
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
