/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General area editor-related functions.
 */

#include <algorithm>

#include "editor.h"

#include "../../functions.h"
#include "../../game.h"
#include "../../imgui/imgui_impl_allegro5.h"
#include "../../load.h"
#include "../../utils/string_utils.h"


using std::set;
using std::size_t;
using std::string;
using std::unordered_set;
using std::vector;

//Radius to use when drawing a cross-section point.
const float area_editor::CROSS_SECTION_POINT_RADIUS = 8.0f;
//A comfortable distance, useful for many scenarios.
const float area_editor::COMFY_DIST = 32.0f;
//The cursor snap for heavy modes updates these many times a second.
const float area_editor::CURSOR_SNAP_UPDATE_INTERVAL = 0.05f;
//Scale the debug text by this much.
const float area_editor::DEBUG_TEXT_SCALE = 1.3f;
//Default reference image opacity.
const unsigned char area_editor::DEF_REFERENCE_ALPHA = 128;
//Amount to pan the camera by when using the keyboard.
const float area_editor::KEYBOARD_PAN_AMOUNT = 32.0f;
//Maximum number of points that a circle sector can be created with.
const unsigned char area_editor::MAX_CIRCLE_SECTOR_POINTS = 32;
//Maximum grid interval.
const float area_editor::MAX_GRID_INTERVAL = 4096;
//Maximum number of texture suggestions.
const size_t area_editor::MAX_TEXTURE_SUGGESTIONS = 20;
//Minimum number of points that a circle sector can be created with.
const unsigned char area_editor::MIN_CIRCLE_SECTOR_POINTS = 3;
//Minimum grid interval.
const float area_editor::MIN_GRID_INTERVAL = 2.0;
//Thickness to use when drawing a mob link line.
const float area_editor::MOB_LINK_THICKNESS = 2.0f;
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
const unsigned char area_editor::SELECTION_COLOR[3] = {255, 255, 0};
//Speed at which the selection effect's "wheel" spins, in radians per second.
const float area_editor::SELECTION_EFFECT_SPEED = TAU * 2;
//Wait this long before letting a new repeat undo operation be saved.
const float area_editor::UNDO_SAVE_LOCK_DURATION = 1.0f;
//Minimum distance between two vertexes for them to merge.
const float area_editor::VERTEX_MERGE_RADIUS = 10.0f;
//Maximum zoom level possible in the editor.
const float area_editor::ZOOM_MAX_LEVEL_EDITOR = 8.0f;
//Minimum zoom level possible in the editor.
const float area_editor::ZOOM_MIN_LEVEL_EDITOR = 0.01f;


/* ----------------------------------------------------------------------------
 * Initializes area editor class stuff.
 */
area_editor::area_editor() :
    backup_timer(game.options.area_editor_backup_interval),
    cursor_snap_timer(CURSOR_SNAP_UPDATE_INTERVAL),
    debug_edge_nrs(false),
    debug_sector_nrs(false),
    debug_path_nrs(false),
    debug_triangulation(false),
    debug_vertex_nrs(false),
    drawing_line_error(DRAWING_LINE_NO_ERROR),
    last_mob_category(nullptr),
    last_mob_type(nullptr),
    moving(false),
    moving_path_preview_checkpoint(-1),
    moving_cross_section_point(-1),
    new_sector_error_tint_timer(NEW_SECTOR_ERROR_TINT_DURATION),
    path_drawing_normals(true),
    pre_move_area_data(nullptr),
    problem_edge_intersection(NULL, NULL),
    reference_bitmap(nullptr),
    selected_shadow(nullptr),
    selecting(false),
    selection_effect(0),
    selection_filter(SELECTION_FILTER_SECTORS),
    show_closest_stop(false),
    show_path_preview(false),
    show_reference(true),
    stt_mode(0),
    stt_sector(nullptr),
    quick_play_cam_z(1.0f) {
    
    path_preview_timer =
    timer(PATH_PREVIEW_TIMER_DUR, [this] () {
        float d = calculate_preview_path();
        //TODO update distance in gui.
    });
    
    undo_save_lock_timer =
        timer(
            UNDO_SAVE_LOCK_DURATION,
    [this] () {undo_save_lock_operation.clear();}
        );
        
    if(game.options.area_editor_backup_interval > 0) {
        backup_timer =
            timer(
                game.options.area_editor_backup_interval,
        [this] () {save_backup();}
            );
    }
    
    selected_shadow_transformation.allow_rotation = true;
    
    zoom_max_level = ZOOM_MAX_LEVEL_EDITOR;
    zoom_min_level = ZOOM_MIN_LEVEL_EDITOR;
    
    use_imgui = true;
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
    for(auto v : selected_vertexes) {
        v->x = pre_move_vertex_coords[v].x;
        v->y = pre_move_vertex_coords[v].y;
    }
    clear_layout_moving();
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
    //TODO clear_current_area_gui();
    
    reference_transformation.keep_aspect_ratio = true;
    update_reference("");
    clear_selection();
    clear_circle_sector();
    clear_layout_drawing();
    clear_layout_moving();
    clear_problems();
    non_simples.clear();
    lone_edges.clear();
    
    clear_area_textures();
    
    for(size_t s = 0; s < game.cur_area_data.tree_shadows.size(); ++s) {
        game.textures.detach(game.cur_area_data.tree_shadows[s]->file_name);
    }
    
    //TODO sector_to_gui();
    //TODO mob_to_gui();
    //TODO tools_to_gui();
    
    game.cam.pos = point();
    game.cam.zoom = 1.0f;
    show_cross_section = false;
    show_cross_section_grid = false;
    show_path_preview = false;
    path_preview.clear();
    //LARGE_FLOAT means they were never given a previous position.
    path_preview_checkpoints[0] = point(LARGE_FLOAT, LARGE_FLOAT);
    path_preview_checkpoints[1] = point(LARGE_FLOAT, LARGE_FLOAT);
    cross_section_checkpoints[0] = point(LARGE_FLOAT, LARGE_FLOAT);
    cross_section_checkpoints[1] = point(LARGE_FLOAT, LARGE_FLOAT);
    
    clear_texture_suggestions();
    
    game.cur_area_data.clear();
    
    made_new_changes = false;
    backup_timer.start(game.options.area_editor_backup_interval);
    
    state = EDITOR_STATE_MAIN;
    //TODO change_to_right_frame();
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
    if(pre_move_area_data) {
        forget_prepared_state(pre_move_area_data);
        pre_move_area_data = NULL;
    }
    pre_move_vertex_coords.clear();
    clear_selection();
    moving = false;
}


/* ----------------------------------------------------------------------------
 * Clears the data about the current problems, if any.
 */
void area_editor::clear_problems() {
    problem_type = EPT_NONE_YET;
    problem_edge_intersection.e1 = NULL;
    problem_edge_intersection.e2 = NULL;
    problem_mob_ptr = NULL;
    problem_path_stop_ptr = NULL;
    problem_sector_ptr = NULL;
    problem_shadow_ptr = NULL;
    problem_vertex_ptr = NULL;
    problem_string.clear();
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
    selected_shadow = NULL;
    selection_homogenized = false;
    
    //TODO asa_to_gui();
    //TODO asb_to_gui();
    //TODO sector_to_gui();
    //TODO mob_to_gui();
    //TODO path_to_gui();
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
 * Clears the undo history, deleting the memory allocated for them.
 */
void area_editor::clear_undo_history() {
    for(size_t h = 0; h < undo_history.size(); ++h) {
        delete undo_history[h].first;
    }
    undo_history.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a new area to work on.
 */
void area_editor::create_area() {
    clear_current_area();
    //TODO disable_widget(frm_toolbar->widgets["but_reload"]);
    
    //Create a sector for it.
    clear_layout_drawing();
    float r = COMFY_DIST * 10;
    
    layout_drawing_node n;
    n.raw_spot = point(-r, -r);
    n.snapped_spot = n.raw_spot;
    drawing_nodes.push_back(n);
    
    n.raw_spot = point(r, -r);
    n.snapped_spot = n.raw_spot;
    drawing_nodes.push_back(n);
    
    n.raw_spot = point(r, r);
    n.snapped_spot = n.raw_spot;
    drawing_nodes.push_back(n);
    
    n.raw_spot = point(-r, r);
    n.snapped_spot = n.raw_spot;
    drawing_nodes.push_back(n);
    
    finish_layout_drawing();
    
    clear_selection();
    
    //Find a texture to give to this sector.
    vector<string> textures = folder_to_vector(TEXTURES_FOLDER_PATH, false);
    size_t texture_to_use = INVALID;
    //First, if there's any "grass" texture, use that.
    for(size_t t = 0; t < textures.size(); ++t) {
        string lc_name = str_to_lower(textures[t]);
        if(lc_name.find("grass") != string::npos) {
            texture_to_use = t;
            break;
        }
    }
    //No grass texture? Try one with "dirt".
    if(texture_to_use == INVALID) {
        for(size_t t = 0; t < textures.size(); ++t) {
            string lc_name = str_to_lower(textures[t]);
            if(lc_name.find("dirt") != string::npos) {
                texture_to_use = t;
                break;
            }
        }
    }
    //If there's no good texture, just pick the first one.
    if(texture_to_use == INVALID) {
        if(!textures.empty()) texture_to_use = 0;
    }
    //Apply the texture.
    if(texture_to_use != INVALID) {
        update_sector_texture(
            game.cur_area_data.sectors[0], textures[texture_to_use]
        );
        update_texture_suggestions(textures[texture_to_use]);
    }
    
    //Now add a leader. The first available.
    game.cur_area_data.mob_generators.push_back(
        new mob_gen(
            game.mob_categories.get(MOB_CATEGORY_LEADERS), point(),
            game.config.leader_order[0], 0, ""
        )
    );
    
    clear_undo_history();
    update_undo_history();
    //TODO update_toolbar();
}


/* ----------------------------------------------------------------------------
 * Deletes the selected mobs.
 */
void area_editor::delete_selected_mobs() {
    if(selected_mobs.empty()) {
        //TODO
        /*emit_status_bar_message(
            "You have to select mobs to delete!", false
        );*/
        return;
    }
    
    register_change("object deletion");
    
    delete_mobs(selected_mobs);
    
    clear_selection();
    sub_state = EDITOR_SUB_STATE_NONE;
}


/* ----------------------------------------------------------------------------
 * Deletes the selected path links and/or stops.
 */
void area_editor::delete_selected_path_elements() {
    if(selected_path_links.empty() && selected_path_stops.empty()) {
        //TODO
        /*emit_status_bar_message(
            "You have to select something to delete!", false
        );*/
        return;
    }
    
    register_change("path deletion");
    
    delete_path_links(selected_path_links);
    selected_path_links.clear();
    
    delete_path_stops(selected_path_stops);
    selected_path_stops.clear();
    
    path_preview.clear(); //Clear so it doesn't reference deleted stops.
    path_preview_timer.start(false);
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor.
 */
void area_editor::do_logic() {
    editor::do_logic_pre();
    
    process_gui();
    
    cursor_snap_timer.tick(game.delta_t);
    path_preview_timer.tick(game.delta_t);
    new_sector_error_tint_timer.tick(game.delta_t);
    undo_save_lock_timer.tick(game.delta_t);
    
    if(!cur_area_name.empty() && game.options.area_editor_backup_interval > 0) {
        backup_timer.tick(game.delta_t);
    }
    
    selection_effect += SELECTION_EFFECT_SPEED * game.delta_t;
    
    editor::do_logic_post();
}


/* ----------------------------------------------------------------------------
 * Dear ImGui callback for when the canvas needs to be drawn on-screen.
 */
void area_editor::draw_canvas_imgui_callback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.area_editor_st->draw_canvas();
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
        //TODO
        /*emit_status_bar_message(
            "That sector wouldn't have a defined parent! Try again.", true
        );*/
        return;
    }
    
    register_change("sector creation");
    
    //Start creating the new sector.
    sector* new_sector = game.cur_area_data.new_sector();
    
    if(outer_sector) {
        outer_sector->clone(new_sector);
        update_sector_texture(
            new_sector,
            outer_sector->texture_info.file_name
        );
    } else {
        if(!texture_suggestions.empty()) {
            update_sector_texture(new_sector, texture_suggestions[0].name);
        } else {
            update_sector_texture(new_sector, "");
        }
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
            new_vertex = game.cur_area_data.new_vertex();
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
            prev_node_edge = game.cur_area_data.new_edge();
            
            game.cur_area_data.connect_edge_to_vertex(
                prev_node_edge, prev_node->on_vertex, 0
            );
            game.cur_area_data.connect_edge_to_vertex(
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
    
    map<edge*, std::pair<sector*, sector*> > edge_sector_backups;
    
    for(size_t e = 0; e < drawing_edges.size(); ++e) {
        edge* e_ptr = drawing_edges[e];
        
        if(!e_ptr->sectors[0] && !e_ptr->sectors[1]) {
            //If it's a new edge, set it up properly.
            game.cur_area_data.connect_edge_to_sector(
                e_ptr, outer_sector, outer_sector_side
            );
            game.cur_area_data.connect_edge_to_sector(
                e_ptr, new_sector, inner_sector_side
            );
            
        } else {
            //If not, let's just add the info for the new sector,
            //and keep the information from the previous sector it was
            //pointing to. This will be cleaned up later on.
            edge_sector_backups[e_ptr].first = e_ptr->sectors[0];
            edge_sector_backups[e_ptr].second = e_ptr->sectors[1];
            
            if(e_ptr->sectors[0] == outer_sector) {
                game.cur_area_data.connect_edge_to_sector(
                    e_ptr, new_sector, 0
                );
            } else {
                game.cur_area_data.connect_edge_to_sector(
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
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        vertex* v1_ptr = game.cur_area_data.edges[e]->vertexes[0];
        vertex* v2_ptr = game.cur_area_data.edges[e]->vertexes[1];
        if(
            new_sector->is_point_in_sector(point(v1_ptr->x, v1_ptr->y)) &&
            new_sector->is_point_in_sector(point(v2_ptr->x, v2_ptr->y)) &&
            new_sector->is_point_in_sector(
                point(
                    (v1_ptr->x + v2_ptr->x) / 2.0,
                    (v1_ptr->y + v2_ptr->y) / 2.0
                )
            )
        ) {
            inner_edges.insert(game.cur_area_data.edges[e]);
        }
    }
    
    for(auto i : inner_edges) {
        auto de_it = find(drawing_edges.begin(), drawing_edges.end(), i);
        
        if(de_it != drawing_edges.end()) {
            //If this edge is a part of the drawing, then we know
            //that it's already set correctly from previous parts of
            //the algorithm. However, in the case where the new sector
            //is on the outside (i.e. this edge is both inside AND a neighbor)
            //then let's simplify the procedure and remove this edge from
            //the new sector, letting it keep its old data.
            //The new sector will still be closed using other edges; that's
            //guaranteed.
            if(i->sectors[outer_sector_side] == new_sector) {
                new_sector->remove_edge(i);
                game.cur_area_data.connect_edge_to_sector(
                    i, edge_sector_backups[i].first, 0
                );
                game.cur_area_data.connect_edge_to_sector(
                    i, edge_sector_backups[i].second, 1
                );
                drawing_edges.erase(de_it);
            }
            
        } else {
            for(size_t s = 0; s < 2; ++s) {
                if(i->sectors[s] == outer_sector) {
                    game.cur_area_data.connect_edge_to_sector(i, new_sector, s);
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
        //TODO emit_triangulation_error_status_bar_message(last_triangulation_error);
    }
    
    //Calculate the bounding box of this sector, now that it's finished.
    new_sector->get_bounding_box(
        &new_sector->bbox[0], &new_sector->bbox[1]
    );
    
    //Select the new sector, making it ready for editing.
    clear_selection();
    select_sector(new_sector);
    //TODO sector_to_gui();
    
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
    for(auto v : selected_vertexes) {
        point p(v->x, v->y);
        
        vector<std::pair<dist, vertex*> > merge_vertexes =
            get_merge_vertexes(
                p, game.cur_area_data.vertexes,
                VERTEX_MERGE_RADIUS / game.cam.zoom
            );
            
        for(size_t mv = 0; mv < merge_vertexes.size(); ) {
            vertex* mv_ptr = merge_vertexes[mv].second;
            if(
                mv_ptr == v ||
                selected_vertexes.find(mv_ptr) != selected_vertexes.end()
            ) {
                merge_vertexes.erase(merge_vertexes.begin() + mv);
            } else {
                ++mv;
            }
        }
        
        sort(
            merge_vertexes.begin(), merge_vertexes.end(),
        [] (std::pair<dist, vertex*> v1, std::pair<dist, vertex*> v2) -> bool {
            return v1.first < v2.first;
        }
        );
        
        vertex* merge_v = NULL;
        if(!merge_vertexes.empty()) {
            merge_v = merge_vertexes[0].second;
        }
        
        if(merge_v) {
            merges[v] = merge_v;
            
        } else {
            edge* e_ptr = NULL;
            bool e_ptr_v1_selected = false;
            bool e_ptr_v2_selected = false;
            
            do {
                e_ptr = get_edge_under_point(p, e_ptr);
                if(e_ptr) {
                    e_ptr_v1_selected =
                        selected_vertexes.find(e_ptr->vertexes[0]) !=
                        selected_vertexes.end();
                    e_ptr_v2_selected =
                        selected_vertexes.find(e_ptr->vertexes[1]) !=
                        selected_vertexes.end();
                }
            } while(
                e_ptr != NULL &&
                (
                    v->has_edge(e_ptr) ||
                    e_ptr_v1_selected || e_ptr_v2_selected
                )
            );
            
            if(e_ptr) {
                edges_to_split[v] = e_ptr;
            }
        }
    }
    
    set<edge*> moved_edges;
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        edge* e_ptr = game.cur_area_data.edges[e];
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
    
    //If an edge is moving into a stationary vertex, it needs to be split.
    //Let's find such edges.
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = game.cur_area_data.vertexes[v];
        point p(v_ptr->x, v_ptr->y);
        
        if(selected_vertexes.find(v_ptr) != selected_vertexes.end()) {
            continue;
        }
        bool is_merge_target = false;
        for(auto &m : merges) {
            if(m.second == v_ptr) {
                //This vertex will have some other vertex merge into it; skip.
                is_merge_target = true;
                break;
            }
        }
        if(is_merge_target) continue;
        
        edge* e_ptr = NULL;
        bool valid = true;
        do {
            e_ptr = get_edge_under_point(p, e_ptr);
            if(e_ptr) {
                if(v_ptr->has_edge(e_ptr)) {
                    valid = false;
                }
                if(moved_edges.find(e_ptr) == moved_edges.end()) {
                    valid = false;
                }
            }
        } while(e_ptr && !valid);
        if(e_ptr) {
            edges_to_split[v_ptr] = e_ptr;
        }
    }
    
    //Before moving on and making changes, let's check for crossing edges,
    //but removing all of the ones that come from edge splits or vertex merges.
    vector<edge_intersection> intersections =
        get_intersecting_edges();
    for(auto &m : merges) {
        for(size_t e1 = 0; e1 < m.first->edges.size(); ++e1) {
            for(size_t e2 = 0; e2 < m.second->edges.size(); ++e2) {
                for(size_t i = 0; i < intersections.size();) {
                    if(
                        intersections[i].contains(m.first->edges[e1]) &&
                        intersections[i].contains(m.second->edges[e2])
                    ) {
                        intersections.erase(intersections.begin() + i);
                    } else {
                        ++i;
                    }
                }
            }
        }
    }
    for(auto &v : edges_to_split) {
        for(size_t e = 0; e < v.first->edges.size(); ++e) {
            for(size_t i = 0; i < intersections.size();) {
                if(
                    intersections[i].contains(v.first->edges[e]) &&
                    intersections[i].contains(v.second)
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
        //TODO
        /*emit_status_bar_message(
            "That move would cause edges to intersect!", true
        );*/
        cancel_layout_moving();
        forget_prepared_state(pre_move_area_data);
        pre_move_area_data = NULL;
        return;
    }
    
    //If there's a vertex between any dragged vertex and its merge, and this
    //vertex was meant to be a merge destination itself, then don't do it.
    //When the first merge happens, this vertex will be gone, and we'll be
    //unable to use it for the second merge. There are no plans to support
    //this complex corner case, so abort!
    for(auto &m : merges) {
        vertex* crushed_vertex = NULL;
        if(m.first->is_2nd_degree_neighbor(m.second, &crushed_vertex)) {
        
            for(auto &m2 : merges) {
                if(m2.second == crushed_vertex) {
                    //TODO
                    /*emit_status_bar_message(
                        "That move would crush an edge that's in the middle!",
                        true
                    );*/
                    cancel_layout_moving();
                    forget_prepared_state(pre_move_area_data);
                    pre_move_area_data = NULL;
                    return;
                }
            }
        }
    }
    
    //Merge vertexes and split edges now.
    for(auto v = edges_to_split.begin(); v != edges_to_split.end(); ++v) {
        merges[v->first] =
            split_edge(v->second, point(v->first->x, v->first->y));
        //This split could've thrown off the edge pointer of a different
        //vertex to merge. Let's re-calculate.
        edge* new_edge = game.cur_area_data.edges.back();
        auto v2 = v;
        ++v2;
        for(; v2 != edges_to_split.end(); ++v2) {
            if(v->second != v2->second) continue;
            v2->second =
                get_correct_post_split_edge(v2->first, v2->second, new_edge);
        }
    }
    for(auto &m : merges) {
        merge_vertex(m.first, m.second, &merge_affected_sectors);
    }
    
    affected_sectors.insert(
        merge_affected_sectors.begin(), merge_affected_sectors.end()
    );
    
    //Triangulate all affected sectors.
    for(auto s : affected_sectors) {
        if(!s) continue;
        
        set<edge*> triangulation_lone_edges;
        TRIANGULATION_ERRORS triangulation_error =
            triangulate(s, &triangulation_lone_edges, true, true);
        if(triangulation_error == TRIANGULATION_NO_ERROR) {
            auto it = non_simples.find(s);
            if(it != non_simples.end()) {
                non_simples.erase(it);
            }
        } else {
            non_simples[s] = triangulation_error;
            last_triangulation_error = triangulation_error;
        }
        
        s->get_bounding_box(
            &(s->bbox[0]), &(s->bbox[1])
        );
    }
    
    if(last_triangulation_error != TRIANGULATION_NO_ERROR) {
        //TODO emit_triangulation_error_status_bar_message(last_triangulation_error);
    }
    
    register_change("vertex movement", pre_move_area_data);
    pre_move_area_data = NULL;
    clear_layout_moving();
}


/* ----------------------------------------------------------------------------
 * Forgets a pre-prepared area state that was almost ready to be added to
 * the undo history.
 */
void area_editor::forget_prepared_state(area_data* prepared_state) {
    delete prepared_state;
}


/* ----------------------------------------------------------------------------
 * Returns which layout element got clicked, if any.
 */
void area_editor::get_clicked_layout_element(
    vertex** clicked_vertex, edge** clicked_edge, sector** clicked_sector
) const {
    *clicked_vertex = get_vertex_under_point(game.mouse_cursor_w);
    *clicked_edge = NULL;
    *clicked_sector = NULL;
    
    if(*clicked_vertex) return;
    
    if(selection_filter != SELECTION_FILTER_VERTEXES) {
        *clicked_edge = get_edge_under_point(game.mouse_cursor_w);
    }
    
    if(*clicked_edge) return;
    
    if(selection_filter == SELECTION_FILTER_SECTORS) {
        *clicked_sector = get_sector_under_point(game.mouse_cursor_w);
    }
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string area_editor::get_name() const {
    return "area editor";
}


/* ----------------------------------------------------------------------------
 * Focuses the camera on the problem found, if any.
 */
void area_editor::goto_problem() {
    switch(problem_type) {
    case EPT_NONE:
    case EPT_NONE_YET: {
        return;
        break;
        
    } case EPT_INTERSECTING_EDGES: {

        if(
            !problem_edge_intersection.e1 || !problem_edge_intersection.e2
        ) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        point min_coords, max_coords;
        min_coords.x = problem_edge_intersection.e1->vertexes[0]->x;
        max_coords.x = min_coords.x;
        min_coords.y = problem_edge_intersection.e1->vertexes[0]->y;
        max_coords.y = min_coords.y;
        
        min_coords.x =
            std::min(
                min_coords.x, problem_edge_intersection.e1->vertexes[0]->x
            );
        min_coords.x =
            std::min(
                min_coords.x, problem_edge_intersection.e1->vertexes[1]->x
            );
        min_coords.x =
            std::min(
                min_coords.x, problem_edge_intersection.e2->vertexes[0]->x
            );
        min_coords.x =
            std::min(
                min_coords.x, problem_edge_intersection.e2->vertexes[1]->x
            );
        max_coords.x =
            std::max(
                max_coords.x, problem_edge_intersection.e1->vertexes[0]->x
            );
        max_coords.x =
            std::max(
                max_coords.x, problem_edge_intersection.e1->vertexes[1]->x
            );
        max_coords.x =
            std::max(
                max_coords.x, problem_edge_intersection.e2->vertexes[0]->x
            );
        max_coords.x =
            std::max(
                max_coords.x, problem_edge_intersection.e2->vertexes[1]->x
            );
        min_coords.y =
            std::min(
                min_coords.y, problem_edge_intersection.e1->vertexes[0]->y
            );
        min_coords.y =
            std::min(
                min_coords.y, problem_edge_intersection.e1->vertexes[1]->y
            );
        min_coords.y =
            std::min(
                min_coords.y, problem_edge_intersection.e2->vertexes[0]->y
            );
        min_coords.y =
            std::min(
                min_coords.y, problem_edge_intersection.e2->vertexes[1]->y
            );
        max_coords.y =
            std::max(
                max_coords.y, problem_edge_intersection.e1->vertexes[0]->y
            );
        max_coords.y =
            std::max(
                max_coords.y, problem_edge_intersection.e1->vertexes[1]->y
            );
        max_coords.y =
            std::max(
                max_coords.y, problem_edge_intersection.e2->vertexes[0]->y
            );
        max_coords.y =
            std::max(
                max_coords.y, problem_edge_intersection.e2->vertexes[1]->y
            );
            
        center_camera(min_coords, max_coords);
        
        break;
        
    } case EPT_BAD_SECTOR: {

        if(non_simples.empty()) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        sector* s_ptr = non_simples.begin()->first;
        point min_coords, max_coords;
        s_ptr->get_bounding_box(&min_coords, &max_coords);
        
        center_camera(min_coords, max_coords);
        
        break;
        
    } case EPT_LONE_EDGE: {

        if(lone_edges.empty()) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        edge* e_ptr = *lone_edges.begin();
        point min_coords, max_coords;
        min_coords.x = e_ptr->vertexes[0]->x;
        max_coords.x = min_coords.x;
        min_coords.y = e_ptr->vertexes[0]->y;
        max_coords.y = min_coords.y;
        
        min_coords.x = std::min(min_coords.x, e_ptr->vertexes[0]->x);
        min_coords.x = std::min(min_coords.x, e_ptr->vertexes[1]->x);
        max_coords.x = std::max(max_coords.x, e_ptr->vertexes[0]->x);
        max_coords.x = std::max(max_coords.x, e_ptr->vertexes[1]->x);
        min_coords.y = std::min(min_coords.y, e_ptr->vertexes[0]->y);
        min_coords.y = std::min(min_coords.y, e_ptr->vertexes[1]->y);
        max_coords.y = std::max(max_coords.y, e_ptr->vertexes[0]->y);
        max_coords.y = std::max(max_coords.y, e_ptr->vertexes[1]->y);
        
        center_camera(min_coords, max_coords);
        
        break;
        
    } case EPT_OVERLAPPING_VERTEXES: {

        if(!problem_vertex_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        center_camera(
            point(
                problem_vertex_ptr->x - 64,
                problem_vertex_ptr->y - 64
            ),
            point(
                problem_vertex_ptr->x + 64,
                problem_vertex_ptr->y + 64
            )
        );
        
        break;
        
    } case EPT_UNKNOWN_TEXTURE: {

        if(!problem_sector_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        point min_coords, max_coords;
        problem_sector_ptr->get_bounding_box(&min_coords, &max_coords);
        center_camera(min_coords, max_coords);
        
        break;
        
    } case EPT_TYPELESS_MOB:
    case EPT_MOB_OOB:
    case EPT_MOB_IN_WALL:
    case EPT_SECTORLESS_BRIDGE: {

        if(!problem_mob_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        center_camera(problem_mob_ptr->pos - 64, problem_mob_ptr->pos + 64);
        
        break;
        
    } case EPT_LONE_PATH_STOP:
    case EPT_PATH_STOPS_TOGETHER:
    case EPT_PATH_STOP_OOB: {

        if(!problem_path_stop_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        center_camera(
            problem_path_stop_ptr->pos - 64,
            problem_path_stop_ptr->pos + 64
        );
        
        break;
        
    } case EPT_INVALID_SHADOW: {

        point min_coords, max_coords;
        get_transformed_rectangle_bounding_box(
            problem_shadow_ptr->center, problem_shadow_ptr->size,
            problem_shadow_ptr->angle, &min_coords, &max_coords
        );
        center_camera(min_coords, max_coords);
        
        break;
    }
    }
}



/* ----------------------------------------------------------------------------
 * Handles an error in the line the user is trying to draw.
 */
void area_editor::handle_line_error() {
    //TODO
}


/* ----------------------------------------------------------------------------
 * Loads the area editor.
 */
void area_editor::load() {
    editor::load();
    
    //Reset some variables.
    cross_section_window_start = point(0.0f, 0.0f);
    cross_section_window_end = point(canvas_br.x * 0.5, canvas_br.y * 0.5);
    cross_section_z_window_start =
        point(cross_section_window_end.x, cross_section_window_start.y);
    cross_section_z_window_end =
        point(cross_section_window_end.x + 48, cross_section_window_end.y);
    is_ctrl_pressed = false;
    is_shift_pressed = false;
    is_gui_focused = false;
    last_mob_category = NULL;
    last_mob_type = NULL;
    loaded_content_yet = false;
    problem_type = EPT_NONE_YET;
    selected_shadow = NULL;
    selection_effect = 0.0;
    selection_homogenized = false;
    show_closest_stop = false;
    show_path_preview = false;
    snap_mode = SNAP_GRID;
    state = EDITOR_STATE_MAIN;
    
    //Reset some other states.
    clear_selection();
    //TODO gui->lose_focus();
    //change_to_right_frame();
    //update_status_bar();
    
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
    
    //Load necessary game content.
    load_custom_particle_generators(false);
    load_spike_damage_types();
    load_liquids(false);
    load_status_types(false);
    load_spray_types(false);
    load_hazards();
    load_mob_types(false);
    load_weather();
    
    //Set up stuff to show the player.
    //open_picker(PICKER_LOAD_AREA);
    
    if(!quick_play_area.empty()) {
        cur_area_name = quick_play_area;
        quick_play_area.clear();
        load_area(false);
        game.cam.pos = quick_play_cam_pos;
        game.cam.zoom = quick_play_cam_z;
        
    } else if(!auto_load_area.empty()) {
        cur_area_name = auto_load_area;
        load_area(false);
        
    }
}


/* ----------------------------------------------------------------------------
 * Load the area from the disk.
 * from_backup: If false, load it normally. If true, load from a backup, if any.
 */
void area_editor::load_area(const bool from_backup) {
    clear_current_area();
    
    ::load_area(cur_area_name, true, from_backup);
    
    //Calculate texture suggestions.
    map<string, size_t> texture_uses_map;
    vector<std::pair<string, size_t> > texture_uses_vector;
    
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        string n = game.cur_area_data.sectors[s]->texture_info.file_name;
        if(n.empty()) continue;
        texture_uses_map[n]++;
    }
    for(auto &u : texture_uses_map) {
        texture_uses_vector.push_back(make_pair(u.first, u.second));
    }
    sort(
        texture_uses_vector.begin(), texture_uses_vector.end(),
    [] (std::pair<string, size_t> u1, std::pair<string, size_t> u2) -> bool {
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
    
    load_reference();
    //TODO update_main_frame();
    
    made_new_changes = false;
    
    clear_undo_history();
    update_undo_history();
    //TODO update_toolbar();
    //TODO enable_widget(frm_toolbar->widgets["but_reload"]);
    
    game.cam.zoom = 1.0f;
    game.cam.pos = point();
    
    //TODO emit_status_bar_message("Loaded successfully.", false);
}


/* ----------------------------------------------------------------------------
 * Loads a backup file.
 */
void area_editor::load_backup() {
    if(!update_backup_status()) return;
    
    load_area(true);
    backup_timer.start(game.options.area_editor_backup_interval);
}


/* ----------------------------------------------------------------------------
 * Loads the reference image data from the reference configuration file.
 */
void area_editor::load_reference() {
    data_node file(
        USER_AREA_DATA_FOLDER_PATH + "/" + cur_area_name + "/Reference.txt"
    );
    
    string new_ref_file_name;
    if(file.file_was_opened) {
        new_ref_file_name = file.get_child_by_name("file")->value;
        reference_transformation.set_center(
            s2p(file.get_child_by_name("center")->value)
        );
        reference_transformation.set_size(
            s2p(file.get_child_by_name("size")->value)
        );
        reference_alpha =
            s2i(
                file.get_child_by_name(
                    "alpha"
                )->get_value_or_default(i2s(DEF_REFERENCE_ALPHA))
            );
            
    } else {
        new_ref_file_name.clear();
        reference_transformation.set_center(point());
        reference_transformation.set_size(point());
        reference_alpha = 0;
    }
    
    update_reference(new_ref_file_name);
}


/* ----------------------------------------------------------------------------
 * Pans the camera around.
 */
void area_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.pos.x -= ev.mouse.dx / game.cam.zoom;
    game.cam.pos.y -= ev.mouse.dy / game.cam.zoom;
}


/* ----------------------------------------------------------------------------
 * Prepares an area state to be delivered to register_change() later,
 * or forgotten altogether with forget_prepared_state().
 */
area_data* area_editor::prepare_state() {
    area_data* new_state = new area_data();
    game.cur_area_data.clone(*new_state);
    return new_state;
}


/* ----------------------------------------------------------------------------
 * Saves the state of the area in the undo history.
 * When this happens, a timer is set. During this timer, if the next change's
 * operation is the same as the previous one's, then it is ignored.
 * This is useful to stop, for instance, a slider
 * drag from saving several dozen operations in the undo history.
 * operation_name:     Name of the operation.
 * pre_prepared_state: If you have the area state prepared from elsewhere in
 *   the code, specify it here. Otherwise, it uses the current area state.
 */
void area_editor::register_change(
    const string &operation_name, area_data* pre_prepared_state
) {
    if(game.options.area_editor_undo_limit == 0) {
        if(pre_prepared_state) {
            forget_prepared_state(pre_prepared_state);
        }
        return;
    }
    
    if(!undo_save_lock_operation.empty()) {
        if(undo_save_lock_operation == operation_name) {
            undo_save_lock_timer.start();
            return;
        }
    }
    
    area_data* new_state = pre_prepared_state;
    if(!pre_prepared_state) {
        new_state = new area_data();
        game.cur_area_data.clone(*new_state);
    }
    undo_history.push_front(make_pair(new_state, operation_name));
    
    made_new_changes = true;
    undo_save_lock_operation = operation_name;
    undo_save_lock_timer.start();
    
    update_undo_history();
    //TODO update_toolbar();
}


/* ----------------------------------------------------------------------------
 * Resets the camera's X and Y coordinates.
 */
void area_editor::reset_cam_xy(const ALLEGRO_EVENT &ev) {
    game.cam.pos.x = 0;
    game.cam.pos.y = 0;
}


/* ----------------------------------------------------------------------------
 * Resets the camera's zoom.
 */
void area_editor::reset_cam_zoom(const ALLEGRO_EVENT &ev) {
    zoom(1.0f);
}


/* ----------------------------------------------------------------------------
 * Saves the area onto the disk.
 * to_backup: If false, save normally. If true, save to an auto-backup file.
 * Returns true on success, false on failure.
 */
bool area_editor::save_area(const bool to_backup) {

    //Before we start, let's get rid of unused sectors.
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ) {
        if(game.cur_area_data.sectors[s]->edges.empty()) {
            game.cur_area_data.remove_sector(s);
        } else {
            ++s;
        }
    }
    
    //First, the geometry file.
    data_node geometry_file("", "");
    
    //Vertexes.
    data_node* vertexes_node = new data_node("vertexes", "");
    geometry_file.add(vertexes_node);
    
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); ++v) {
        vertex* v_ptr = game.cur_area_data.vertexes[v];
        data_node* vertex_node =
            new data_node("v", f2s(v_ptr->x) + " " + f2s(v_ptr->y));
        vertexes_node->add(vertex_node);
    }
    
    //Edges.
    data_node* edges_node = new data_node("edges", "");
    geometry_file.add(edges_node);
    
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        edge* e_ptr = game.cur_area_data.edges[e];
        data_node* edge_node = new data_node("e", "");
        edges_node->add(edge_node);
        string s_str;
        for(size_t s = 0; s < 2; ++s) {
            if(e_ptr->sector_nrs[s] == INVALID) s_str += "-1";
            else s_str += i2s(e_ptr->sector_nrs[s]);
            s_str += " ";
        }
        s_str.erase(s_str.size() - 1);
        edge_node->add(new data_node("s", s_str));
        edge_node->add(
            new data_node(
                "v",
                i2s(e_ptr->vertex_nrs[0]) + " " + i2s(e_ptr->vertex_nrs[1])
            )
        );
    }
    
    //Sectors.
    data_node* sectors_node = new data_node("sectors", "");
    geometry_file.add(sectors_node);
    
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        data_node* sector_node = new data_node("s", "");
        sectors_node->add(sector_node);
        
        if(s_ptr->type != SECTOR_TYPE_NORMAL) {
            sector_node->add(
                new data_node("type", game.sector_types.get_name(s_ptr->type))
            );
        }
        if(s_ptr->is_bottomless_pit) {
            sector_node->add(
                new data_node("is_bottomless_pit", "true")
            );
        }
        sector_node->add(new data_node("z", f2s(s_ptr->z)));
        if(s_ptr->brightness != DEF_SECTOR_BRIGHTNESS) {
            sector_node->add(
                new data_node("brightness", i2s(s_ptr->brightness))
            );
        }
        if(!s_ptr->tag.empty()) {
            sector_node->add(new data_node("tag", s_ptr->tag));
        }
        if(s_ptr->fade) {
            sector_node->add(new data_node("fade", b2s(s_ptr->fade)));
        }
        if(s_ptr->always_cast_shadow) {
            sector_node->add(
                new data_node(
                    "always_cast_shadow",
                    b2s(s_ptr->always_cast_shadow)
                )
            );
        }
        if(!s_ptr->hazards_str.empty()) {
            sector_node->add(new data_node("hazards", s_ptr->hazards_str));
            sector_node->add(
                new data_node(
                    "hazards_floor",
                    b2s(s_ptr->hazard_floor)
                )
            );
        }
        
        if(!s_ptr->texture_info.file_name.empty()) {
            sector_node->add(
                new data_node(
                    "texture",
                    s_ptr->texture_info.file_name
                )
            );
        }
        
        if(s_ptr->texture_info.rot != 0) {
            sector_node->add(
                new data_node(
                    "texture_rotate",
                    f2s(s_ptr->texture_info.rot)
                )
            );
        }
        if(
            s_ptr->texture_info.scale.x != 1 ||
            s_ptr->texture_info.scale.y != 1
        ) {
            sector_node->add(
                new data_node(
                    "texture_scale",
                    f2s(s_ptr->texture_info.scale.x) + " " +
                    f2s(s_ptr->texture_info.scale.y)
                )
            );
        }
        if(
            s_ptr->texture_info.translation.x != 0 ||
            s_ptr->texture_info.translation.y != 0
        ) {
            sector_node->add(
                new data_node(
                    "texture_trans",
                    f2s(s_ptr->texture_info.translation.x) + " " +
                    f2s(s_ptr->texture_info.translation.y)
                )
            );
        }
        if(
            s_ptr->texture_info.tint.r != 1.0 ||
            s_ptr->texture_info.tint.g != 1.0 ||
            s_ptr->texture_info.tint.b != 1.0 ||
            s_ptr->texture_info.tint.a != 1.0
        ) {
            sector_node->add(
                new data_node("texture_tint", c2s(s_ptr->texture_info.tint))
            );
        }
        
    }
    
    //Mobs.
    data_node* mobs_node = new data_node("mobs", "");
    geometry_file.add(mobs_node);
    
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        data_node* mob_node =
            new data_node(m_ptr->category->name, "");
        mobs_node->add(mob_node);
        
        if(m_ptr->type) {
            mob_node->add(
                new data_node("type", m_ptr->type->name)
            );
        }
        mob_node->add(
            new data_node(
                "p",
                f2s(m_ptr->pos.x) + " " + f2s(m_ptr->pos.y)
            )
        );
        if(m_ptr->angle != 0) {
            mob_node->add(
                new data_node("angle", f2s(m_ptr->angle))
            );
        }
        if(m_ptr->vars.size()) {
            mob_node->add(
                new data_node("vars", m_ptr->vars)
            );
        }
        
        string links_str;
        for(size_t l = 0; l < m_ptr->link_nrs.size(); ++l) {
            if(l > 0) links_str += " ";
            links_str += i2s(m_ptr->link_nrs[l]);
        }
        
        if(!links_str.empty()) {
            mob_node->add(
                new data_node("links", links_str)
            );
        }
    }
    
    //Path stops.
    data_node* path_stops_node = new data_node("path_stops", "");
    geometry_file.add(path_stops_node);
    
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        data_node* path_stop_node = new data_node("s", "");
        path_stops_node->add(path_stop_node);
        
        path_stop_node->add(
            new data_node("pos", f2s(s_ptr->pos.x) + " " + f2s(s_ptr->pos.y))
        );
        
        data_node* links_node = new data_node("links", "");
        path_stop_node->add(links_node);
        
        for(size_t l = 0; l < s_ptr->links.size(); l++) {
            path_link* l_ptr = &s_ptr->links[l];
            data_node* link_node = new data_node("nr", i2s(l_ptr->end_nr));
            links_node->add(link_node);
        }
        
    }
    
    //Tree shadows.
    data_node* shadows_node = new data_node("tree_shadows", "");
    geometry_file.add(shadows_node);
    
    for(size_t s = 0; s < game.cur_area_data.tree_shadows.size(); ++s) {
        tree_shadow* s_ptr = game.cur_area_data.tree_shadows[s];
        data_node* shadow_node = new data_node("shadow", "");
        shadows_node->add(shadow_node);
        
        shadow_node->add(
            new data_node(
                "pos", f2s(s_ptr->center.x) + " " + f2s(s_ptr->center.y)
            )
        );
        shadow_node->add(
            new data_node(
                "size", f2s(s_ptr->size.x) + " " + f2s(s_ptr->size.y)
            )
        );
        if(s_ptr->angle != 0) {
            shadow_node->add(new data_node("angle", f2s(s_ptr->angle)));
        }
        if(s_ptr->alpha != 255) {
            shadow_node->add(new data_node("alpha", i2s(s_ptr->alpha)));
        }
        shadow_node->add(new data_node("file", s_ptr->file_name));
        shadow_node->add(
            new data_node("sway", f2s(s_ptr->sway.x) + " " + f2s(s_ptr->sway.y))
        );
        
    }
    
    //Now, the data file.
    data_node data_file("", "");
    
    data_file.add(
        new data_node("name", game.cur_area_data.name)
    );
    data_file.add(
        new data_node("subtitle", game.cur_area_data.subtitle)
    );
    data_file.add(
        new data_node("bg_bmp", game.cur_area_data.bg_bmp_file_name)
    );
    data_file.add(
        new data_node("bg_color", c2s(game.cur_area_data.bg_color))
    );
    data_file.add(
        new data_node("bg_dist", f2s(game.cur_area_data.bg_dist))
    );
    data_file.add(
        new data_node("bg_zoom", f2s(game.cur_area_data.bg_bmp_zoom))
    );
    data_file.add(
        new data_node("weather", game.cur_area_data.weather_name)
    );
    data_file.add(
        new data_node("creator", game.cur_area_data.creator)
    );
    data_file.add(
        new data_node("version", game.cur_area_data.version)
    );
    data_file.add(
        new data_node("notes", game.cur_area_data.notes)
    );
    data_file.add(
        new data_node("spray_amounts", game.cur_area_data.spray_amounts)
    );
    
    
    //Finally, save.
    string geometry_file_name;
    string data_file_name;
    if(to_backup) {
        geometry_file_name =
            USER_AREA_DATA_FOLDER_PATH + "/" + cur_area_name +
            "/Geometry_backup.txt";
        data_file_name =
            USER_AREA_DATA_FOLDER_PATH + "/" + cur_area_name +
            "/Data_backup.txt";
    } else {
        geometry_file_name =
            AREAS_FOLDER_PATH + "/" + cur_area_name +
            "/Geometry.txt";
        data_file_name =
            AREAS_FOLDER_PATH + "/" + cur_area_name +
            "/Data.txt";
    }
    bool geo_save_ok = geometry_file.save_file(geometry_file_name);
    bool data_save_ok = data_file.save_file(data_file_name);
    
    if(!geo_save_ok || !data_save_ok) {
        show_message_box(
            NULL, "Save failed!",
            "Could not save the area!",
            (
                "An error occured while saving the area to the folder \"" +
                AREAS_FOLDER_PATH + "/" + cur_area_name + "\". Make sure that "
                "the folder exists and it is not read-only, and try again."
            ).c_str(),
            NULL,
            ALLEGRO_MESSAGEBOX_WARN
        );
        
        //TODO emit_status_bar_message("Could not save the area!", true);
        
    } else {
        if(!to_backup) {
            //TODO emit_status_bar_message("Saved successfully.", false);
        }
    }
    
    backup_timer.start(game.options.area_editor_backup_interval);
    //TODO enable_widget(frm_toolbar->widgets["but_reload"]);
    
    save_reference();
    
    return geo_save_ok && data_save_ok;
    
}


/* ----------------------------------------------------------------------------
 * Saves the area onto a backup file.
 */
void area_editor::save_backup() {

    backup_timer.start(game.options.area_editor_backup_interval);
    
    //First, check if the folder even exists.
    //If not, chances are this is a new area.
    //We should probably create a backup anyway, but if the area is
    //just for testing, the backups are pointless.
    //Plus, creating the backup will create the area's folder on the disk,
    //which will basically mean the area exists, even though this might not be
    //what the user wants, since they haven't saved proper yet.
    
    ALLEGRO_FS_ENTRY* folder_fs_entry =
        al_create_fs_entry((AREAS_FOLDER_PATH + "/" + cur_area_name).c_str());
    bool folder_exists = al_open_directory(folder_fs_entry);
    al_close_directory(folder_fs_entry);
    al_destroy_fs_entry(folder_fs_entry);
    
    if(!folder_exists) return;
    
    save_area(true);
    update_backup_status();
}


/* ----------------------------------------------------------------------------
 * Saves the reference data to disk, in the area's reference config file.
 */
void area_editor::save_reference() {
    string file_name =
        USER_AREA_DATA_FOLDER_PATH + "/" + cur_area_name +
        "/Reference.txt";
        
    if(!reference_bitmap) {
        //The user doesn't want a reference more.
        //Delete its config file.
        al_remove_filename(file_name.c_str());
        return;
    }
    
    data_node reference_file("", "");
    reference_file.add(
        new data_node("file", reference_file_name)
    );
    reference_file.add(
        new data_node(
            "center",
            p2s(reference_transformation.get_center())
        )
    );
    reference_file.add(
        new data_node(
            "size",
            p2s(reference_transformation.get_size())
        )
    );
    reference_file.add(
        new data_node(
            "alpha",
            i2s(reference_alpha)
        )
    );
    
    reference_file.save_file(file_name);
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
 * Selects a tree shadow.
 */
void area_editor::select_tree_shadow(tree_shadow* s_ptr) {
    selected_shadow = s_ptr;
    selected_shadow_transformation.set_angle(s_ptr->angle);
    selected_shadow_transformation.set_center(s_ptr->center);
    selected_shadow_transformation.set_size(s_ptr->size);
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
        get_angle(new_circle_sector_center, game.mouse_cursor_w);
    float radius =
        dist(
            new_circle_sector_center, new_circle_sector_anchor
        ).to_float();
    float angle_dif =
        get_angle_smallest_dif(cursor_angle, anchor_angle);
        
    size_t n_points = MAX_CIRCLE_SECTOR_POINTS;
    if(angle_dif > 0) {
        n_points = round(TAU / angle_dif);
    }
    n_points =
        clamp(n_points, MIN_CIRCLE_SECTOR_POINTS, MAX_CIRCLE_SECTOR_POINTS);
        
    new_circle_sector_points.clear();
    for(size_t p = 0; p < n_points; ++p) {
        float delta_a = (TAU / n_points) * p;
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
        
        for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
            edge* e_ptr = game.cur_area_data.edges[e];
            
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
 * Procedure to start moving the selected mobs.
 */
void area_editor::start_mob_move() {
    register_change("object movement");
    
    move_closest_mob = NULL;
    dist move_closest_mob_dist;
    for(auto m : selected_mobs) {
        pre_move_mob_coords[m] = m->pos;
        
        dist d(game.mouse_cursor_w, m->pos);
        if(!move_closest_mob || d < move_closest_mob_dist) {
            move_closest_mob = m;
            move_closest_mob_dist = d;
            move_closest_mob_start_pos = m->pos;
        }
    }
    
    move_mouse_start_pos = game.mouse_cursor_w;
    moving = true;
}


/* ----------------------------------------------------------------------------
 * Procedure to start moving the selected path stops.
 */
void area_editor::start_path_stop_move() {
    register_change("path stop movement");
    
    move_closest_stop = NULL;
    dist move_closest_stop_dist;
    for(
        auto s = selected_path_stops.begin();
        s != selected_path_stops.end(); ++s
    ) {
        pre_move_stop_coords[*s] = (*s)->pos;
        
        dist d(game.mouse_cursor_w, (*s)->pos);
        if(!move_closest_stop || d < move_closest_stop_dist) {
            move_closest_stop = *s;
            move_closest_stop_dist = d;
            move_closest_stop_start_pos = (*s)->pos;
        }
    }
    
    move_mouse_start_pos = game.mouse_cursor_w;
    moving = true;
}


/* ----------------------------------------------------------------------------
 * Procedure to start moving the selected tree shadow.
 */
void area_editor::start_shadow_move() {
    pre_move_shadow_coords = selected_shadow->center;
    
    move_mouse_start_pos = game.mouse_cursor_w;
    moving = true;
}


/* ----------------------------------------------------------------------------
 * Procedure to start moving the selected vertexes.
 */
void area_editor::start_vertex_move() {
    pre_move_area_data = prepare_state();
    
    move_closest_vertex = NULL;
    dist move_closest_vertex_dist;
    for(auto v : selected_vertexes) {
        point p(v->x, v->y);
        pre_move_vertex_coords[v] = p;
        
        dist d(game.mouse_cursor_w, p);
        if(!move_closest_vertex || d < move_closest_vertex_dist) {
            move_closest_vertex = v;
            move_closest_vertex_dist = d;
            move_closest_vertex_start_pos = p;
        }
    }
    
    unordered_set<sector*> affected_sectors =
        get_affected_sectors(selected_vertexes);
        
    move_mouse_start_pos = game.mouse_cursor_w;
    moving = true;
}


/* ----------------------------------------------------------------------------
 * Undoes the last change to the area using the undo history, if available.
 */
void area_editor::undo() {
    if(undo_history.empty()) {
        //TODO emit_status_bar_message("Nothing to undo.", false);
        return;
    }
    if(
        sub_state != EDITOR_SUB_STATE_NONE || moving || selecting
    ) {
        //TODO
        /*emit_status_bar_message(
            "Can't undo in the middle of an operation.", false
        );*/
        return;
    }
    
    undo_history[0].first->clone(game.cur_area_data);
    delete undo_history[0].first;
    undo_history.pop_front();
    
    undo_save_lock_timer.stop();
    undo_save_lock_operation.clear();
    update_undo_history();
    //TODO update_toolbar();
    
    clear_selection();
    clear_circle_sector();
    clear_layout_drawing();
    clear_layout_moving();
    clear_problems();
    non_simples.clear();
    lone_edges.clear();
    //TODO change_to_right_frame();
    
    path_preview.clear(); //Clear so it doesn't reference deleted stops.
    path_preview_timer.start(false);
    
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Undoes the last placed layout drawing node.
 */
void area_editor::undo_layout_drawing_node() {
    drawing_nodes.erase(
        drawing_nodes.begin() + drawing_nodes.size() - 1
    );
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void area_editor::unload() {
    editor::unload();
    
    clear_current_area();
    cur_area_name.clear();
    
    unload_weather();
    unload_mob_types(false);
    unload_hazards();
    unload_spray_types();
    unload_status_types(false);
    unload_liquids();
    unload_spike_damage_types();
    unload_custom_particle_generators();
}


/* ----------------------------------------------------------------------------
 * Reads the area's backup file, and sets the "load backup" button's
 * availability accordingly.
 * Returns true if it exists, false if not.
 */
bool area_editor::update_backup_status() {
    //TODO disable_widget(frm_tools->widgets["but_backup"]);
    
    if(cur_area_name.empty()) return false;
    
    data_node file(
        USER_AREA_DATA_FOLDER_PATH + "/" +
        cur_area_name + "/Geometry_backup.txt"
    );
    if(!file.file_was_opened) return false;
    
    //TODO enable_widget(frm_tools->widgets["but_backup"]);
    return true;
}


/* ----------------------------------------------------------------------------
 * Updates the reference image's bitmap, given a new bitmap file name.
 */
void area_editor::update_reference(const string &new_file_name) {
    if(reference_file_name == new_file_name) {
        //Nothing to do.
        return;
    }
    
    reference_file_name = new_file_name;
    
    if(reference_bitmap && reference_bitmap != game.bmp_error) {
        al_destroy_bitmap(reference_bitmap);
    }
    reference_bitmap = NULL;
    
    if(!new_file_name.empty()) {
        reference_bitmap =
            load_bmp(new_file_name, NULL, false, true, true, true);
            
        if(
            reference_transformation.get_size().x == 0 ||
            reference_transformation.get_size().y == 0
        ) {
            //Let's assume this is a new reference. Reset sizes and alpha.
            reference_transformation.set_size(
                point(
                    al_get_bitmap_width(reference_bitmap),
                    al_get_bitmap_height(reference_bitmap)
                )
            );
            reference_alpha = DEF_REFERENCE_ALPHA;
        }
    } else {
        reference_transformation.set_center(point());
        reference_transformation.set_size(point());
    }
    
    //TODO tools_to_gui();
    //TODO update_toolbar();
}


/* ----------------------------------------------------------------------------
 * Updates a sector's texture.
 */
void area_editor::update_sector_texture(
    sector* s_ptr, const string &file_name
) {
    game.textures.detach(s_ptr->texture_info.file_name);
    s_ptr->texture_info.file_name = file_name;
    s_ptr->texture_info.bitmap = game.textures.get(file_name);
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
 * Updates the state and description of the undo button based on
 * the undo history.
 */
void area_editor::update_undo_history() {
    while(undo_history.size() > game.options.area_editor_undo_limit) {
        undo_history.pop_back();
    };
    
    //TODO update_toolbar();
}


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
    
    vector<std::pair<dist, vertex*> > merge_vertexes =
        get_merge_vertexes(
            mouse_click, game.cur_area_data.vertexes,
            VERTEX_MERGE_RADIUS / game.cam.zoom
        );
    if(!merge_vertexes.empty()) {
        sort(
            merge_vertexes.begin(), merge_vertexes.end(),
        [] (std::pair<dist, vertex*> v1, std::pair<dist, vertex*> v2) -> bool {
            return v1.first < v2.first;
        }
        );
        on_vertex = merge_vertexes[0].second;
        on_vertex_nr = game.cur_area_data.find_vertex_nr(on_vertex);
    }
    
    if(on_vertex) {
        snapped_spot.x = on_vertex->x;
        snapped_spot.y = on_vertex->y;
        
    } else {
        on_edge = ae_ptr->get_edge_under_point(mouse_click);
        
        if(on_edge) {
            on_edge_nr = game.cur_area_data.find_edge_nr(on_edge);
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
 * Creates a texture suggestion.
 */
area_editor::texture_suggestion::texture_suggestion(
    const string &n
) :
    bmp(NULL),
    name(n) {
    
    bmp = game.textures.get(name, NULL, false);
}


/* ----------------------------------------------------------------------------
 * Destroys a texture suggestion.
 */
void area_editor::texture_suggestion::destroy() {
    game.textures.detach(name);
}
