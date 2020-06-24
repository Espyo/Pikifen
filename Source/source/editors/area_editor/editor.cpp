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
//Quick previewing lasts this long in total, including the fade out.
const float area_editor::QUICK_PREVIEW_DURATION = 4.0f;
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
    can_load_backup(false),
    can_reload(false),
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
    octee_mode(OCTEE_MODE_OFFSET),
    path_drawing_normals(true),
    pre_move_area_data(nullptr),
    problem_edge_intersection(NULL, NULL),
    quick_preview_timer(QUICK_PREVIEW_DURATION),
    reference_bitmap(nullptr),
    selected_shadow(nullptr),
    selecting(false),
    selection_effect(0),
    selection_filter(SELECTION_FILTER_SECTORS),
    show_closest_stop(false),
    show_path_preview(false),
    show_reference(true),
    quick_play_cam_z(1.0f) {
    
    path_preview_timer =
    timer(PATH_PREVIEW_TIMER_DUR, [this] () {
        path_preview_dist = calculate_preview_path();
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
}


/* ----------------------------------------------------------------------------
 * Cancels the circular sector creation operation and returns to normal.
 */
void area_editor::cancel_circle_sector() {
    clear_circle_sector();
    sub_state = EDITOR_SUB_STATE_NONE;
    status_text.clear();
}


/* ----------------------------------------------------------------------------
 * Cancels the edge drawing operation and returns to normal.
 */
void area_editor::cancel_layout_drawing() {
    clear_layout_drawing();
    sub_state = EDITOR_SUB_STATE_NONE;
    status_text.clear();
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
 * Changes to a new state, cleaning up whatever is needed.
 */
void area_editor::change_state(const EDITOR_STATES new_state) {
    clear_selection();
    state = new_state;
    sub_state = EDITOR_SUB_STATE_NONE;
    status_text.clear();
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
    reference_transformation.keep_aspect_ratio = true;
    reference_file_name.clear();
    update_reference();
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
    
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
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
    problem_title.clear();
    problem_description.clear();
    problem_edge_intersection.e1 = NULL;
    problem_edge_intersection.e2 = NULL;
    problem_mob_ptr = NULL;
    problem_path_stop_ptr = NULL;
    problem_sector_ptr = NULL;
    problem_shadow_ptr = NULL;
    problem_vertex_ptr = NULL;
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
    set_selection_status_text();
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
 * Code to run when the area picker is closed.
 */
void area_editor::close_area_picker() {
    if(!loaded_content_yet && cur_area_name.empty()) {
        //The user cancelled the area selection picker
        //presented when you enter the area editor. Quit out.
        leave();
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new area to work on.
 */
void area_editor::create_area() {
    clear_current_area();
    
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
    
    finish_new_sector_drawing();
    
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
    can_reload = false;
}


/* ----------------------------------------------------------------------------
 * Creates vertexes based on the edge drawing the user has just made.
 * Drawing nodes that are already on vertexes don't count, but the other ones
 * either create edge splits, or create simple vertexes inside a sector.
 */
void area_editor::create_drawing_vertexes() {
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
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor.
 */
void area_editor::do_logic() {
    editor::do_logic_pre();
    
    process_gui();
    
    cursor_snap_timer.tick(game.delta_t);
    path_preview_timer.tick(game.delta_t);
    quick_preview_timer.tick(game.delta_t);
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
 * Emits a message onto the status bar, based on the given triangulation error.
 */
void area_editor::emit_triangulation_error_status_bar_message(
    const TRIANGULATION_ERRORS error
) {
    switch(error) {
    case TRIANGULATION_ERROR_LONE_EDGES: {
        status_text =
            "Some sectors have lone edges!";
        break;
    } case TRIANGULATION_ERROR_NO_EARS: {
        status_text =
            "Some sectors could not be triangulated!";
        break;
    } case TRIANGULATION_ERROR_VERTEXES_REUSED: {
        status_text =
            "Some sectors reuse vertexes -- there are likely gaps!";
        break;
    } case TRIANGULATION_ERROR_INVALID_ARGS: {
        status_text =
            "An unknown error has occured with some sectors!";
        break;
    } case TRIANGULATION_NO_ERROR: {
        break;
    }
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
    finish_new_sector_drawing();
    
    clear_circle_sector();
    sub_state = EDITOR_SUB_STATE_NONE;
}


/* ----------------------------------------------------------------------------
 * Finishes a vertex moving procedure.
 */
void area_editor::finish_layout_moving() {
    TRIANGULATION_ERRORS last_triangulation_error = TRIANGULATION_NO_ERROR;
    
    unordered_set<sector*> affected_sectors;
    get_affected_sectors(selected_vertexes, affected_sectors);
    map<vertex*, vertex*> merges;
    map<vertex*, edge*> edges_to_split;
    unordered_set<sector*> merge_affected_sectors;
    size_t vertex_amount = selected_vertexes.size();
    
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
        cancel_layout_moving();
        forget_prepared_state(pre_move_area_data);
        pre_move_area_data = NULL;
        status_text = "That move would cause edges to intersect!";
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
                    cancel_layout_moving();
                    forget_prepared_state(pre_move_area_data);
                    pre_move_area_data = NULL;
                    status_text =
                        "That move would crush an edge that's in the middle!";
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
    
    //Update all affected sectors.
    update_affected_sectors(affected_sectors);
    
    register_change("vertex movement", pre_move_area_data);
    pre_move_area_data = NULL;
    clear_layout_moving();
}


/* ----------------------------------------------------------------------------
 * Finishes creating a new sector.
 */
void area_editor::finish_new_sector_drawing() {
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
        status_text = "That sector wouldn't have a defined parent! Try again.";
        return;
    }
    
    vector<edge*> outer_sector_old_edges;
    if(outer_sector) {
        outer_sector_old_edges = outer_sector->edges;
    } else {
        for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
            edge* e_ptr = game.cur_area_data.edges[e];
            if(e_ptr->sectors[0] == NULL || e_ptr->sectors[1] == NULL) {
                outer_sector_old_edges.push_back(e_ptr);
            }
        }
    }
    
    register_change("sector creation");
    
    //First, create vertexes wherever necessary.
    create_drawing_vertexes();
    
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
    
    //Create the new sector, empty.
    sector* new_sector = create_sector_for_layout_drawing(outer_sector);
    
    //Connect the edges to the sectors.
    bool is_clockwise = is_polygon_clockwise(drawing_vertexes);
    unsigned char inner_sector_side = (is_clockwise ? 1 : 0);
    unsigned char outer_sector_side = (is_clockwise ? 0 : 1);
    
    for(size_t e = 0; e < drawing_edges.size(); ++e) {
        edge* e_ptr = drawing_edges[e];
        
        game.cur_area_data.connect_edge_to_sector(
            e_ptr, outer_sector, outer_sector_side
        );
        game.cur_area_data.connect_edge_to_sector(
            e_ptr, new_sector, inner_sector_side
        );
    }
    
    //The new sector is created, but only its outer edges exist.
    //Triangulate these so we can check what's inside.
    triangulate(new_sector, NULL, true, false);
    
    //All sectors inside the new one need to know that
    //their outer sector changed.
    update_inner_sectors_outer_sector(
        outer_sector_old_edges, outer_sector, new_sector
    );
    
    //Finally, update all affected sectors. Only the working sector and
    //the new sector have had their triangles changed, so work only on those.
    unordered_set<sector*> affected_sectors;
    affected_sectors.insert(new_sector);
    affected_sectors.insert(outer_sector);
    update_affected_sectors(affected_sectors);
    
    //Select the new sector, making it ready for editing.
    clear_selection();
    select_sector(new_sector);
    
    clear_layout_drawing();
    sub_state = EDITOR_SUB_STATE_NONE;
    
    status_text =
        "Created sector with " +
        amount_str(new_sector->edges.size(), "edge") + ", " +
        amount_str(drawing_vertexes.size(), "vertex", "vertexes") + ".";
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
        center_camera(s_ptr->bbox[0], s_ptr->bbox[1]);
        
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
        
        center_camera(problem_sector_ptr->bbox[0], problem_sector_ptr->bbox[1]);
        
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
        
    } case EPT_UNKNOWN_SHADOW: {

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
    new_sector_error_tint_timer.start();
    switch(drawing_line_error) {
    case DRAWING_LINE_LOOPS_IN_SPLIT: {
        status_text =
            "To split a sector, you can't end on the starting point!";
        break;
    } case DRAWING_LINE_HIT_EDGE_OR_VERTEX: {
        status_text =
            "To draw the shape of a sector, you can't hit an edge or vertex!";
        break;
    } case DRAWING_LINE_CROSSES_DRAWING: {
        status_text =
            "That line crosses other lines in the drawing!";
        break;
    } case DRAWING_LINE_CROSSES_EDGES: {
        status_text =
            "That line crosses existing edges!";
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the area editor.
 */
void area_editor::load() {
    editor::load();
    
    //Reset some variables.
    is_ctrl_pressed = false;
    is_shift_pressed = false;
    is_gui_focused = false;
    last_mob_category = NULL;
    last_mob_type = NULL;
    loaded_content_yet = false;
    selected_shadow = NULL;
    selection_effect = 0.0;
    selection_homogenized = false;
    show_closest_stop = false;
    show_path_preview = false;
    snap_mode = SNAP_GRID;
    state = EDITOR_STATE_MAIN;
    status_text.clear();
    
    //Reset some other states.
    clear_problems();
    clear_selection();
    
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
    
    if(!quick_play_area.empty()) {
        cur_area_name = quick_play_area;
        quick_play_area.clear();
        load_area(false);
        game.cam.set_pos(quick_play_cam_pos);
        game.cam.set_zoom(quick_play_cam_z);
        
    } else if(!auto_load_area.empty()) {
        cur_area_name = auto_load_area;
        load_area(false);
        
    } else {
        open_area_picker();
        
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
    
    made_new_changes = false;
    
    clear_undo_history();
    update_undo_history();
    can_reload = true;
    
    game.cam.zoom = 1.0f;
    game.cam.pos = point();
    
    status_text = "Loaded area \"" + cur_area_name + "\" successfully.";
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
    
    if(file.file_was_opened) {
        reference_file_name = file.get_child_by_name("file")->value;
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
        reference_file_name.clear();
        reference_transformation.set_center(point());
        reference_transformation.set_size(point());
        reference_alpha = 0;
    }
    
    update_reference();
}


/* ----------------------------------------------------------------------------
 * Callback for when the user picks an area from the picker.
 */
void area_editor::pick_area(const string &name, const bool is_new) {
    cur_area_name = name;
    
    if(is_new) {
        string new_area_path =
            AREAS_FOLDER_PATH + "/" + name;
        ALLEGRO_FS_ENTRY* new_area_folder_entry =
            al_create_fs_entry(new_area_path.c_str());
            
        if(al_fs_entry_exists(new_area_folder_entry)) {
            //Already exists, just load it.
            area_editor::load_area(false);
        } else {
            //Create a new area.
            create_area();
        }
        
        al_destroy_fs_entry(new_area_folder_entry);
        
        status_text = "Created area \"" + cur_area_name + "\" successfully.";
        
    } else {
        area_editor::load_area(false);
        
    }
    
    state = EDITOR_STATE_MAIN;
}


/* ----------------------------------------------------------------------------
 * Callback for when the user picks a texture from the picker.
 */
void area_editor::pick_texture(const string &name, const bool is_new) {
    sector* s_ptr = NULL;
    if(selected_sectors.size() == 1 || selection_homogenized) {
        s_ptr = *selected_sectors.begin();
    }
    
    if(!s_ptr) {
        return;
    }
    
    if(s_ptr->texture_info.file_name == name) {
        return;
    }
    
    register_change("sector texture change");
    
    update_texture_suggestions(name);
    
    update_sector_texture(s_ptr, name);
    
    homogenize_selected_sectors();
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
 * Code to run when the circle sector button widget is pressed.
 */
void area_editor::press_circle_sector_button() {
    if(moving || selecting) {
        return;
    }
    
    if(!non_simples.empty() || !lone_edges.empty()) {
        status_text =
            "Please fix any broken sectors or edges before trying to make "
            "a new sector!";
        return;
    }
    
    clear_circle_sector();
    if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        cancel_circle_sector();
    } else {
        status_text = "Use the canvas to place a circular sector.";
        sub_state = EDITOR_SUB_STATE_CIRCLE_SECTOR;
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the duplicate mobs button widget is pressed.
 */
void area_editor::press_duplicate_mobs_button() {
    if(selected_mobs.empty()) {
        status_text = "You have to select mobs to duplicate!";
    } else if(sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB) {
        status_text.clear();
        sub_state = EDITOR_SUB_STATE_NONE;
    } else {
        status_text = "Use the canvas to place the duplicated objects.";
        sub_state = EDITOR_SUB_STATE_DUPLICATE_MOB;
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the new mob button widget is pressed.
 */
void area_editor::press_new_mob_button() {
    if(moving || selecting) {
        return;
    }
    
    if(sub_state == EDITOR_SUB_STATE_NEW_MOB) {
        status_text.clear();
        sub_state = EDITOR_SUB_STATE_NONE;
    } else {
        clear_selection();
        status_text = "Use the canvas to place a new object.";
        sub_state = EDITOR_SUB_STATE_NEW_MOB;
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the new path button widget is pressed.
 */
void area_editor::press_new_path_button() {
    if(moving || selecting) {
        return;
    }
    
    if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
        status_text.clear();
        sub_state = EDITOR_SUB_STATE_NONE;
    } else {
        path_drawing_stop_1 = NULL;
        status_text = "Use the canvas to draw a path.";
        sub_state = EDITOR_SUB_STATE_PATH_DRAWING;
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the new sector button widget is pressed.
 */
void area_editor::press_new_sector_button() {
    if(moving || selecting) {
        return;
    }
    
    if(!non_simples.empty() || !lone_edges.empty()) {
        status_text =
            "Please fix any broken sectors or edges before trying to make "
            "a new sector!";
        return;
    }
    
    clear_layout_drawing();
    if(sub_state == EDITOR_SUB_STATE_DRAWING) {
        cancel_layout_drawing();
    } else {
        status_text = "Use the canvas to draw a sector.";
        sub_state = EDITOR_SUB_STATE_DRAWING;
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the new tree shadow button widget is pressed.
 */
void area_editor::press_new_tree_shadow_button() {
    if(moving || selecting) {
        return;
    }
    
    if(sub_state == EDITOR_SUB_STATE_NEW_SHADOW) {
        status_text.clear();
        sub_state = EDITOR_SUB_STATE_NONE;
    } else {
        status_text = "Use the canvas to place a new tree shadow.";
        sub_state = EDITOR_SUB_STATE_NEW_SHADOW;
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the quick play button widget is pressed.
 */
void area_editor::press_quick_play_button() {
    if(!save_area(false)) return;
    quick_play_area = cur_area_name;
    quick_play_cam_pos = game.cam.pos;
    quick_play_cam_z = game.cam.zoom;
    leave();
}


/* ----------------------------------------------------------------------------
 * Code to run when the quit button widget is pressed.
 */
void area_editor::press_quit_button() {
    if(!check_new_unsaved_changes(quit_widget_pos)) {
        status_text = "Bye!";
        leave();
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the toggle reference button widget is pressed.
 */
void area_editor::press_reference_button() {
    show_reference = !show_reference;
    string state_str = (show_reference ? "visible" : "invisible");
    status_text = "The reference image is now " + state_str + ".";
}


/* ----------------------------------------------------------------------------
 * Code to run when the reload button widget is pressed.
 */
void area_editor::press_reload_button() {
    if(!can_reload) {
        return;
    }
    if(!check_new_unsaved_changes(reload_widget_pos)) {
        load_area(false);
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the remove edge button widget is pressed.
 */
void area_editor::press_remove_edge_button() {
    //Check if the user can delete.
    if(moving || selecting) {
        return;
    }
    
    if(selected_edges.empty()) {
        status_text = "You have to select edges to delete!";
        return;
    }
    
    //Prepare everything.
    register_change("edge deletion");
    size_t n_before = game.cur_area_data.edges.size();
    size_t n_selected = selected_edges.size();
    
    //Delete!
    bool success = delete_edges(selected_edges);
    
    //Cleanup.
    clear_selection();
    sub_state = EDITOR_SUB_STATE_NONE;
    
    //Report.
    if(success) {
        status_text =
            "Deleted " +
            amount_str(n_before - game.cur_area_data.edges.size(), "edge") +
            " (" + i2s(n_selected) + " were selected).";
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the remove mob button widget is pressed.
 */
void area_editor::press_remove_mob_button() {
    //Check if the user can delete.
    if(moving || selecting) {
        return;
    }
    
    if(selected_mobs.empty()) {
        status_text = "You have to select mobs to delete!";
        return;
    }
    
    //Prepare everything.
    register_change("object deletion");
    size_t amount = selected_mobs.size();
    
    //Delete!
    delete_mobs(selected_mobs);
    
    //Cleanup.
    clear_selection();
    sub_state = EDITOR_SUB_STATE_NONE;
    
    //Report.
    status_text = "Deleted " + amount_str(amount, "object") + ".";
}


/* ----------------------------------------------------------------------------
 * Code to run when the remove path button widget is pressed.
 */
void area_editor::press_remove_path_button() {
    //Check if the user can delete.
    if(moving || selecting) {
        return;
    }
    
    if(selected_path_links.empty() && selected_path_stops.empty()) {
        status_text = "You have to select something to delete!";
        return;
    }
    
    //Prepare everything.
    register_change("path deletion");
    size_t path_link_amount = selected_path_links.size();
    size_t path_stop_amount = selected_path_stops.size();
    
    //Delete!
    delete_path_links(selected_path_links);
    delete_path_stops(selected_path_stops);
    
    //Cleanup.
    clear_selection();
    sub_state = EDITOR_SUB_STATE_NONE;
    path_preview.clear(); //Clear so it doesn't reference deleted stops.
    path_preview_timer.start(false);
    
    //Report.
    status_text =
        "Deleted " +
        amount_str(path_link_amount, "path link") + ", " +
        amount_str(path_stop_amount, "path stop") + ".";
}


/* ----------------------------------------------------------------------------
 * Code to run when the remove tree shadow button widget is pressed.
 */
void area_editor::press_remove_tree_shadow_button() {
    if(moving || selecting) {
        return;
    }
    
    if(!selected_shadow) {
        status_text = "You have to select a shadow to delete!";
    } else {
        register_change("tree shadow deletion");
        for(
            size_t s = 0;
            s < game.cur_area_data.tree_shadows.size();
            ++s
        ) {
            if(
                game.cur_area_data.tree_shadows[s] ==
                selected_shadow
            ) {
                game.cur_area_data.tree_shadows.erase(
                    game.cur_area_data.tree_shadows.begin() + s
                );
                delete selected_shadow;
                selected_shadow = NULL;
                break;
            }
        }
        status_text = "Deleted tree shadow.";
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the save button widget is pressed.
 */
void area_editor::press_save_button() {
    if(!save_area(false)) {
        return;
    }
    
    change_state(EDITOR_STATE_MAIN);
    made_new_changes = false;
    status_text = "Saved area successfully.";
}


/* ----------------------------------------------------------------------------
 * Code to run when the selection filter button widget is pressed.
 */
void area_editor::press_selection_filter_button() {
    clear_selection();
    if(!is_shift_pressed) {
        selection_filter =
            sum_and_wrap(selection_filter, 1, N_SELECTION_FILTERS);
    } else {
        selection_filter =
            sum_and_wrap(selection_filter, -1, N_SELECTION_FILTERS);
    }
    
    status_text = "Set selection filter to ";
    switch(selection_filter) {
    case SELECTION_FILTER_SECTORS: {
        status_text += "sectors + edges + vertexes";
        break;
    } case SELECTION_FILTER_EDGES: {
        status_text += "edges + vertexes";
        break;
    } case SELECTION_FILTER_VERTEXES: {
        status_text += "vertexes";
        break;
    }
    }
    status_text += ".";
}


/* ----------------------------------------------------------------------------
 * Code to run when the snap mode button widget is pressed.
 */
void area_editor::press_snap_mode_button() {
    if(!is_shift_pressed) {
        snap_mode = sum_and_wrap(snap_mode, 1, N_SNAP_MODES);
    } else {
        snap_mode = sum_and_wrap(snap_mode, -1, N_SNAP_MODES);
    }
    
    status_text = "Set snap mode to ";
    switch(snap_mode) {
    case SNAP_GRID: {
        status_text += "grid";
        break;
    } case SNAP_VERTEXES: {
        status_text += "vertexes";
        break;
    } case SNAP_EDGES: {
        status_text += "edges";
        break;
    } case SNAP_NOTHING: {
        status_text += "nothing";
        break;
    }
    }
    status_text += ".";
}


/* ----------------------------------------------------------------------------
 * Code to run when the undo button widget is pressed.
 */
void area_editor::press_undo_button() {
    if(
        sub_state != EDITOR_SUB_STATE_NONE || moving || selecting
    ) {
        status_text = "Can't undo in the middle of an operation.";
        return;
    }
    
    undo();
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
}


/* ----------------------------------------------------------------------------
 * Returns to a previously prepared area state.
 */
void area_editor::rollback_to_prepared_state(area_data* prepared_state) {
    prepared_state->clone(game.cur_area_data);
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
        
        status_text = "Could not save the area!";
        
    }
    
    backup_timer.start(game.options.area_editor_backup_interval);
    can_reload = true;
    
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
    set_selection_status_text();
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
    set_selection_status_text();
}


/* ----------------------------------------------------------------------------
 * Selects a tree shadow.
 */
void area_editor::select_tree_shadow(tree_shadow* s_ptr) {
    selected_shadow = s_ptr;
    selected_shadow_transformation.set_angle(s_ptr->angle);
    selected_shadow_transformation.set_center(s_ptr->center);
    selected_shadow_transformation.set_size(s_ptr->size);
    set_selection_status_text();
}


/* ----------------------------------------------------------------------------
 * Selects a vertex.
 */
void area_editor::select_vertex(vertex* v) {
    selected_vertexes.insert(v);
    set_selection_status_text();
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
 * Sets the status text based on how many things are selected.
 */
void area_editor::set_selection_status_text() {
    status_text.clear();
    
    if(!non_simples.empty()) {
        emit_triangulation_error_status_bar_message(
            non_simples.begin()->second
        );
    }
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(!selected_vertexes.empty()) {
            status_text =
                "Selected " +
                amount_str(selected_sectors.size(), "sector") +
                ", " +
                amount_str(selected_edges.size(), "edge") +
                ", " +
                amount_str(selected_vertexes.size(), "vertex", "vertexes") +
                ".";
        }
        break;
        
    } case EDITOR_STATE_MOBS: {
        if(!selected_mobs.empty()) {
            status_text =
                "Selected " +
                amount_str(selected_mobs.size(), "object") +
                ".";
        }
        break;
        
    } case EDITOR_STATE_PATHS: {
        if(!selected_path_links.empty() || !selected_path_stops.empty()) {
            status_text =
                "Selected " +
                amount_str(selected_path_links.size(), "path link") +
                ", " +
                amount_str(selected_path_stops.size(), "path stop") +
                ".";
        }
        break;
        
    } case EDITOR_STATE_DETAILS: {
        if(selected_shadow) {
            status_text = "Selected a tree shadow.";
        }
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Splits a sector into two sectors using the user's current drawing.
 */
void area_editor::split_sector_with_drawing() {
    if(drawing_nodes.size() < 2) {
        cancel_layout_drawing();
        return;
    }
    
    area_data* pre_split_area_data = prepare_state();
    
    //The idea is as follows: To split the working sector, we create a new
    //sector that takes up some of the same area as the working sector.
    //To do so, we traverse the sector's edges, from the last split point,
    //until we find the first split point. That path, plus the split, make up
    //the new sector.
    //Normally that's all, but if the cut is made against inner sectors of
    //the working sector, things get a bit trickier.
    //If the edges we traversed end up creating a sector that consumers that
    //inner sector, that won't do. Instead, the inner sector will have to be
    //created based on traversal in the opposite direction.
    //At the end, when the new sector is made, check its insides to see if
    //it must adopt some of the working sector's children sectors.
    
    //Figure out what the working sector is.
    //The middle point of two drawing nodes will always be in the working
    //sector, so it's a great place to check.
    sector* working_sector =
        get_sector_under_point(
            (drawing_nodes[0].snapped_spot + drawing_nodes[1].snapped_spot) /
            2.0f
        );
    vector<edge*> working_sector_old_edges;
    if(working_sector) {
        working_sector_old_edges = working_sector->edges;
    } else {
        for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
            edge* e_ptr = game.cur_area_data.edges[e];
            if(e_ptr->sectors[0] == NULL || e_ptr->sectors[1] == NULL) {
                working_sector_old_edges.push_back(e_ptr);
            }
        }
    }
    
    //First, create vertexes wherever necessary.
    create_drawing_vertexes();
    
    //Traverse the sector, starting on the last point of the drawing,
    //going edge by edge, until we hit that point again.
    //During traversal, collect a list of traversed edges and vertexes.
    //This traversal happens in two stages. In the first stage, collect them
    //into the first set of vectors. Once the traversal reaches the checkpoint,
    //it restarts and goes in the opposite direction, collecting edges and
    //vertexes into the second set of vectors from here on out. Normally,
    //we only need the data from stage 1 to create a sector, but as we'll see
    //later on, we may need to use data from stage 2 instead.
    vector<edge*> traversed_edges[2];
    vector<vertex*> traversed_vertexes[2];
    bool is_working_at_stage_1_left;
    traverse_sector_for_split(
        working_sector,
        drawing_nodes.back().on_vertex,
        drawing_nodes[0].on_vertex,
        traversed_edges,
        traversed_vertexes,
        &is_working_at_stage_1_left
    );
    
    if(traversed_edges[0].empty()) {
        //Something went wrong.
        rollback_to_prepared_state(pre_split_area_data);
        forget_prepared_state(pre_split_area_data);
        clear_selection();
        clear_layout_drawing();
        sub_state = EDITOR_SUB_STATE_NONE;
        status_text =
            "That's not a valid split!";
        return;
    }
    
    if(traversed_edges[1].empty()) {
        //If the sector's neighboring edges were traversed entirely
        //without finding the drawing's last point, then that point is in a set
        //of edges different from the drawing's first point. This can happen
        //if the points are in different inner sectors, or if only
        //one of them is in an inner sector.
        //If the user were to split in this way, the sector would still be
        //in one piece, except with a disallowed gash. Cancel.
        rollback_to_prepared_state(pre_split_area_data);
        forget_prepared_state(pre_split_area_data);
        clear_selection();
        clear_layout_drawing();
        sub_state = EDITOR_SUB_STATE_NONE;
        status_text =
            "That wouldn't split the sector in any useful way!";
        return;
    }
    
    //Create the drawing's new edges and connect them.
    vector<edge*> drawing_edges;
    for(size_t n = 0; n < drawing_nodes.size() - 1; ++n) {
        layout_drawing_node* n_ptr = &drawing_nodes[n];
        layout_drawing_node* next_node = &drawing_nodes[n + 1];
        
        edge* new_node_edge = game.cur_area_data.new_edge();
        
        game.cur_area_data.connect_edge_to_vertex(
            new_node_edge, n_ptr->on_vertex, 0
        );
        game.cur_area_data.connect_edge_to_vertex(
            new_node_edge, next_node->on_vertex, 1
        );
        
        drawing_edges.push_back(new_node_edge);
    }
    
    //Most of the time, the new sector can be made using the drawing edges
    //and the traversed edges from traversal stage 1. However, if the drawing
    //is made against an inner sector of our working sector, then there's a
    //50-50 chance that using the first set of traversed edges would result in
    //a sector that would engulf that inner sector. Instead, we'll have to use
    //the traversed edges from traversal stage 2.
    //Let's figure out which stage to use now.
    vector<edge*> new_sector_edges = drawing_edges;
    vector<vertex*> new_sector_vertexes;
    for(size_t d = 0; d < drawing_nodes.size(); ++d) {
        new_sector_vertexes.push_back(drawing_nodes[d].on_vertex);
    }
    
    //To figure it out, pretend we're using stage 1's data, and gather
    //the vertexes that would make the new sector. Then, check if
    //the result is clockwise or not.
    //Since the new sector is supposed to replace area from the working sector,
    //we can use that orientation and see if it matches with the sides that
    //the working sector belongs to. If not, we need the data from stage 2.
    //Oh, and in this loop, we can skip the last, since it's already
    //the same as the first drawing node.
    for(size_t t = 0; t < traversed_vertexes[0].size() - 1; ++t) {
        new_sector_vertexes.push_back(traversed_vertexes[0][t]);
    }
    
    bool is_new_clockwise = is_polygon_clockwise(new_sector_vertexes);
    
    if(is_new_clockwise == is_working_at_stage_1_left) {
        //Darn, the new sector goes clockwise, which means the new sector's to
        //the right of these edges, when traversing them in stage 1's order,
        //but we know from before that the working sector is to the left!
        //(Or vice-versa.) This means that the drawing is against an inner
        //sector (it's the only way this can happen), and that this selection
        //of vertexes would result in a sector that's going around that
        //inner sector. Let's swap to the traversal stage 2 data.
        
        new_sector_vertexes.clear();
        for(size_t d = 0; d < drawing_nodes.size(); ++d) {
            new_sector_vertexes.push_back(drawing_nodes[d].on_vertex);
        }
        //Same as before, skip the last.
        for(size_t t = 0; t < traversed_vertexes[1].size() - 1; ++t) {
            new_sector_vertexes.push_back(traversed_vertexes[1][t]);
        }
        for(size_t t = 0; t < traversed_edges[1].size(); ++t) {
            new_sector_edges.push_back(traversed_edges[1][t]);
        }
        
    } else {
        //We can use stage 1's data!
        //The vertexes are already in place, so let's fill in the edges.
        for(size_t t = 0; t < traversed_edges[0].size(); ++t) {
            new_sector_edges.push_back(traversed_edges[0][t]);
        }
        
    }
    
    //Organize all edge vertexes such that they follow the same order.
    for(size_t e = 0; e < new_sector_edges.size(); ++e) {
        if(new_sector_edges[e]->vertexes[0] != new_sector_vertexes[e]) {
            new_sector_edges[e]->swap_vertexes();
        }
    }
    
    //Create the new sector, empty.
    sector* new_sector = create_sector_for_layout_drawing(working_sector);
    size_t new_sector_nr = game.cur_area_data.sectors.size() - 1;
    
    //Connect the edges to the sectors.
    unsigned char new_sector_side = (is_new_clockwise ? 1 : 0);
    unsigned char working_sector_side = (is_new_clockwise ? 0 : 1);
    
    for(size_t e = 0; e < new_sector_edges.size(); ++e) {
        edge* e_ptr = new_sector_edges[e];
        
        if(!e_ptr->sectors[0] && !e_ptr->sectors[1]) {
            //If it's a new edge, set it up properly.
            game.cur_area_data.connect_edge_to_sector(
                e_ptr, working_sector, working_sector_side
            );
            game.cur_area_data.connect_edge_to_sector(
                e_ptr, new_sector, new_sector_side
            );
            
        } else {
            //If not, let's transfer from the working sector to the new one.
            game.cur_area_data.connect_edge_to_sector(
                e_ptr, new_sector, new_sector_side
            );
            
        }
    }
    
    //The new sector is created, but only its outer edges exist.
    //Triangulate these so we can check what's inside.
    triangulate(new_sector, NULL, true, false);
    
    //All sectors inside the new one need to know that
    //their outer sector changed. Since we're only checking from the edges
    //that used to be long to the working sector, the edges that were created
    //with the drawing will not be included.
    update_inner_sectors_outer_sector(
        working_sector_old_edges, working_sector, new_sector
    );
    
    //Finally, update all affected sectors. Only the working sector and
    //the new sector have had their triangles changed, so work only on those.
    unordered_set<sector*> affected_sectors;
    affected_sectors.insert(working_sector);
    affected_sectors.insert(new_sector);
    update_affected_sectors(affected_sectors);
    
    //Select one of the two sectors, making it ready for editing.
    //We want to select the smallest of the two, because it's the "most new".
    //If you have a sector that's a really complex shape, and you split
    //such that one of the post-split sectors is a triangle, chances are you
    //had that complex shape, and you wanted to make a new triangle from it,
    //not that you had a "triangle" and wanted to make a complex shape.
    clear_selection();
    
    if(!working_sector) {
        select_sector(new_sector);
    } else {
        float working_sector_area =
            (working_sector->bbox[1].x - working_sector->bbox[0].x) *
            (working_sector->bbox[1].y - working_sector->bbox[0].y);
        float new_sector_area =
            (new_sector->bbox[1].x - new_sector->bbox[0].x) *
            (new_sector->bbox[1].y - new_sector->bbox[0].y);
            
        if(working_sector_area < new_sector_area) {
            select_sector(working_sector);
        } else {
            select_sector(new_sector);
        }
    }
    
    clear_layout_drawing();
    sub_state = EDITOR_SUB_STATE_NONE;
    
    register_change("sector split", pre_split_area_data);
    status_text =
        "Split sector, creating one with " +
        amount_str(new_sector->edges.size(), "edge") + ".";
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
    
    move_mouse_start_pos = game.mouse_cursor_w;
    moving = true;
}


/* ----------------------------------------------------------------------------
 * Traverses a sector's edges, in order, going from neighbor to neighbor.
 * Traversal starts at a vertex, and during stage 1, the encountered
 * edges/vertexes are saved in the first set of vectors.
 * The direction of travel depends on whatever the first edge is in the
 * list of edges connected to the first vertex.
 * Eventually, we should find the checkpoint vertex during traversal;
 * at this point, the algorithm will switch to stage 2 and start over,
 * this time going in the opposite direction from before, and
 * saving encountered edges/vertexes in the second set of vectors.
 * Finally, the traversal should stop when the checkpoint vertex is hit again.
 * If the sector has inner sectors, not all edges will be encountered, since
 * this algorithm only goes neighbor by neighbor.
 * If the checkpoint vertex is never found, stage 2's data will be empty.
 * s_ptr:
 *   Sector to traverse.
 * begin:
 *   Vertex to begin in.
 * checkpoint:
 *   Vertex to switch stages at.
 * edges:
 *   Pointer to an array of two vectors. Edges encountered during each stage
 *   are inserted into either one of these vectors.
 * vertexes:
 *   Pointer to an array of two vectors. Vertexes encountered during each stage
 *   are inserted into either one of these vectors.
 * working_sector_left:
 *   This bool will be set to true if, during stage 1 traversal, the
 *   working sector is to the left, and false if to the right.
 */
void area_editor::traverse_sector_for_split(
    sector* s_ptr, vertex* begin, vertex* checkpoint,
    vector<edge*>* edges, vector<vertex*>* vertexes,
    bool* working_sector_left
) {
    edge* first_edge = NULL;
    
    for(unsigned char s = 0; s < 2; ++s) {
        vertex* v_ptr = begin;
        vertex* prev_vertex = NULL;
        
        while(true) {
            edge* next_edge = NULL;
            vertex* next_vertex = NULL;
            
            for(size_t e = 0; e < v_ptr->edges.size(); ++e) {
                edge* e_ptr = v_ptr->edges[e];
                if(e_ptr->sectors[0] != s_ptr && e_ptr->sectors[1] != s_ptr) {
                    //The working sector is not in this edge. This is some
                    //unrelated edge that won't help us.
                    continue;
                }
                if(e_ptr == first_edge) {
                    //This will only be true at the start of stage 2, when the
                    //algorithm tries to take the first edge's direction again.
                    continue;
                }
                next_vertex = e_ptr->get_other_vertex(v_ptr);
                if(next_vertex == prev_vertex) {
                    //This is the direction we came from.
                    continue;
                }
                
                next_edge = e_ptr;
                break;
            }
            
            if(!next_edge) {
                return;
            }
            
            if(!first_edge) {
                first_edge = next_edge;
                //In stage 1, travelling in this direction, is the
                //working sector to the left or to the right?
                if(next_edge->vertexes[0] == begin) {
                    //This edge travels in the same direction as us. Side 0 is
                    //to the left, side 1 is to the right, so just check if the
                    //working sector is to the left.
                    *working_sector_left = (next_edge->sectors[0] == s_ptr);
                } else {
                    //This edge travels the opposite way. Same logic as above,
                    //but reversed.
                    *working_sector_left = (next_edge->sectors[1] == s_ptr);
                }
            }
            
            prev_vertex = v_ptr;
            v_ptr = next_vertex;
            
            edges[s].push_back(next_edge);
            vertexes[s].push_back(next_vertex);
            
            if(next_vertex == checkpoint) {
                //Enter stage 2, or quit.
                break;
            }
            
            if(next_vertex == begin) {
                //We found the start again? Finish the algorithm right now.
                return;
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Undoes the last change to the area using the undo history, if available.
 */
void area_editor::undo() {
    if(undo_history.empty()) {
        status_text = "Nothing to undo.";
        return;
    }
    
    string operation_name = undo_history[0].second;
    
    undo_history[0].first->clone(game.cur_area_data);
    delete undo_history[0].first;
    undo_history.pop_front();
    
    undo_save_lock_timer.stop();
    undo_save_lock_operation.clear();
    update_undo_history();
    
    clear_selection();
    clear_circle_sector();
    clear_layout_drawing();
    clear_layout_moving();
    clear_problems();
    non_simples.clear();
    lone_edges.clear();
    
    path_preview.clear(); //Clear so it doesn't reference deleted stops.
    path_preview_timer.start(false);
    
    made_new_changes = true;
    status_text = "Undo successful: " + operation_name + ".";
}


/* ----------------------------------------------------------------------------
 * Undoes the last placed layout drawing node.
 */
void area_editor::undo_layout_drawing_node() {
    if(drawing_nodes.empty()) return;
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
    can_load_backup = false;
    
    if(cur_area_name.empty()) return false;
    
    data_node file(
        USER_AREA_DATA_FOLDER_PATH + "/" +
        cur_area_name + "/Geometry_backup.txt"
    );
    if(!file.file_was_opened) return false;
    
    can_load_backup = true;
    return true;
}


/* ----------------------------------------------------------------------------
 * Updates the reference image's bitmap, since its file name just changed.
 */
void area_editor::update_reference() {
    if(reference_bitmap && reference_bitmap != game.bmp_error) {
        al_destroy_bitmap(reference_bitmap);
    }
    reference_bitmap = NULL;
    
    if(!reference_file_name.empty()) {
        reference_bitmap =
            load_bmp(reference_file_name, NULL, false, true, true, true);
            
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
