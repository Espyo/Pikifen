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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


using std::set;
using std::size_t;
using std::string;
using std::unordered_set;
using std::vector;


namespace AREA_EDITOR {

//Color for blocking sectors in the "show blocking sectors" mode.
const ALLEGRO_COLOR BLOCKING_COLOR = al_map_rgba(100, 32, 32, 192);

//A comfortable distance, useful for many scenarios.
const float COMFY_DIST = 32.0f;

//Radius to use when drawing a cross-section point.
const float CROSS_SECTION_POINT_RADIUS = 8.0f;

//The cursor snap for heavy modes updates these many times a second.
const float CURSOR_SNAP_UPDATE_INTERVAL = 0.05f;

//Scale the debug text by this much.
const float DEBUG_TEXT_SCALE = 1.3f;

//Default reference image opacity.
const unsigned char DEF_REFERENCE_ALPHA = 128;

//Amount to pan the camera by when using the keyboard.
const float KEYBOARD_PAN_AMOUNT = 32.0f;

//Maximum number of points that a circle sector can be created with.
const unsigned char MAX_CIRCLE_SECTOR_POINTS = 32;

//Maximum grid interval.
const float MAX_GRID_INTERVAL = 4096;

//Maximum number of texture suggestions.
const size_t MAX_TEXTURE_SUGGESTIONS = 20;

//Text color for various measurement labels in the canvas.
const ALLEGRO_COLOR MEASUREMENT_COLOR = al_map_rgb(64, 255, 64);

//Minimum number of points that a circle sector can be created with.
const unsigned char MIN_CIRCLE_SECTOR_POINTS = 3;

//Minimum grid interval.
const float MIN_GRID_INTERVAL = 2.0;

//Thickness to use when drawing a mob link line.
const float MOB_LINK_THICKNESS = 2.0f;

//How long to tint the new sector's line(s) red for.
const float NEW_SECTOR_ERROR_TINT_DURATION = 1.5f;

//Color for non-blocking sectors in the "show blocking sectors" mode.
const ALLEGRO_COLOR NON_BLOCKING_COLOR = al_map_rgba(64, 160, 64, 192);

//Thickness to use when drawing a path link line.
const float PATH_LINK_THICKNESS = 3.0f;

//Radius to use when drawing a path preview checkpoint.
const float PATH_PREVIEW_CHECKPOINT_RADIUS = 8.0f;

//Only fetch the path these many seconds after the player stops the checkpoints.
const float PATH_PREVIEW_TIMER_DUR = 0.1f;

//Scale the letters on the "points" of various features by this much.
const float POINT_LETTER_TEXT_SCALE = 1.5f;

//Quick previewing lasts this long in total, including the fade out.
const float QUICK_PREVIEW_DURATION = 4.0f;

//Minimum width or height that the reference image can have.
const float REFERENCE_MIN_SIZE = 5.0f;

//Color of a selected element, or the selection box.
const unsigned char SELECTION_COLOR[3] = {255, 255, 0};

//Speed at which the selection effect's "wheel" spins, in radians per second.
const float SELECTION_EFFECT_SPEED = TAU * 2;

//Padding for the transformation widget when manipulating the selection.
const float SELECTION_TW_PADDING = 8.0f;

//Wait this long before letting a new repeat undo operation be saved.
const float UNDO_SAVE_LOCK_DURATION = 1.0f;

//Minimum distance between two vertexes for them to merge.
const float VERTEX_MERGE_RADIUS = 10.0f;

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 8.0f;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.01f;

}


/**
 * @brief Constructs a new area editor object.
 */
AreaEditor::AreaEditor() :
    backup_timer(game.options.area_editor_backup_interval),
    load_dialog_picker(this) {
    
    enable_flag(path_preview_settings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES);
    path_preview_timer =
    Timer(AREA_EDITOR::PATH_PREVIEW_TIMER_DUR, [this] () {
        path_preview_dist = calculate_preview_path();
    });
    
    undo_save_lock_timer =
        Timer(
            AREA_EDITOR::UNDO_SAVE_LOCK_DURATION,
    [this] () {undo_save_lock_operation.clear();}
        );
        
    if(game.options.area_editor_backup_interval > 0) {
        backup_timer =
            Timer(
                game.options.area_editor_backup_interval,
        [this] () {save_backup();}
            );
    }
    
    zoom_max_level = AREA_EDITOR::ZOOM_MAX_LEVEL;
    zoom_min_level = AREA_EDITOR::ZOOM_MIN_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        Command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(&AreaEditor::circle_sector_cmd, "circle_sector");
    register_cmd(&AreaEditor::copy_properties_cmd, "copy_properties");
    register_cmd(&AreaEditor::delete_area_cmd, "delete_area");
    register_cmd(&AreaEditor::delete_cmd, "delete");
    register_cmd(&AreaEditor::delete_edge_cmd, "delete_edge");
    register_cmd(&AreaEditor::delete_tree_shadow_cmd, "delete_tree_shadow");
    register_cmd(&AreaEditor::duplicate_mobs_cmd, "duplicate_mobs");
    register_cmd(
        &AreaEditor::grid_interval_decrease_cmd, "grid_interval_decrease"
    );
    register_cmd(
        &AreaEditor::grid_interval_increase_cmd, "grid_interval_increase"
    );
    register_cmd(&AreaEditor::layout_drawing_cmd, "layout_drawing");
    register_cmd(&AreaEditor::load_cmd, "load");
    register_cmd(&AreaEditor::new_mob_cmd, "new_mob");
    register_cmd(&AreaEditor::new_path_cmd, "new_path");
    register_cmd(&AreaEditor::new_tree_shadow_cmd, "new_tree_shadow");
    register_cmd(&AreaEditor::paste_properties_cmd, "paste_properties");
    register_cmd(&AreaEditor::paste_texture_cmd, "paste_texture");
    register_cmd(&AreaEditor::quick_play_cmd, "quick_play");
    register_cmd(&AreaEditor::quit_cmd, "quit");
    register_cmd(&AreaEditor::redo_cmd, "redo");
    register_cmd(&AreaEditor::reference_toggle_cmd, "reference_toggle");
    register_cmd(&AreaEditor::reload_cmd, "reload");
    register_cmd(&AreaEditor::save_cmd, "save");
    register_cmd(&AreaEditor::select_all_cmd, "select_all");
    register_cmd(&AreaEditor::selection_filter_cmd, "selection_filter");
    register_cmd(&AreaEditor::snap_mode_cmd, "snap_mode");
    register_cmd(&AreaEditor::undo_cmd, "undo");
    register_cmd(&AreaEditor::zoom_and_pos_reset_cmd, "zoom_and_pos_reset");
    register_cmd(&AreaEditor::zoom_everything_cmd, "zoom_everything");
    register_cmd(&AreaEditor::zoom_in_cmd, "zoom_in");
    register_cmd(&AreaEditor::zoom_out_cmd, "zoom_out");
    
#undef register_cmd
}


/**
 * @brief Calculates what the day speed should be, taking into account
 * the specified start day time, end day time, and mission duration.
 *
 * @param day_start_min Day start time, in minutes.
 * @param day_end_min Day end time, in minutes.
 * @param mission_min Mission duration, in minutes.
 * @return The day speed.
 */
float AreaEditor::calculate_day_speed(
    float day_start_min, float day_end_min,
    float mission_min
) {
    if(mission_min == 0.0f) return 0.0f;
    float aux_day_end_min = day_end_min;
    if(day_end_min < day_start_min) {
        aux_day_end_min += 24 * 60;
    }
    return (aux_day_end_min - day_start_min) / mission_min;
}


/**
 * @brief Cancels the circular sector creation operation and returns to normal.
 */
void AreaEditor::cancel_circle_sector() {
    clear_circle_sector();
    sub_state = EDITOR_SUB_STATE_NONE;
    set_status();
}


/**
 * @brief Cancels the edge drawing operation and returns to normal.
 */
void AreaEditor::cancel_layout_drawing() {
    clear_layout_drawing();
    sub_state = EDITOR_SUB_STATE_NONE;
    set_status();
}


/**
 * @brief Cancels the vertex moving operation.
 */
void AreaEditor::cancel_layout_moving() {
    for(auto const &v : selected_vertexes) {
        v->x = pre_move_vertex_coords[v].x;
        v->y = pre_move_vertex_coords[v].y;
    }
    clear_layout_moving();
}


/**
 * @brief Changes to a new state, cleaning up whatever is needed.
 *
 * @param new_state The new state.
 */
void AreaEditor::change_state(const EDITOR_STATE new_state) {
    clear_selection();
    state = new_state;
    sub_state = EDITOR_SUB_STATE_NONE;
    set_status();
}


/**
 * @brief Clears the data about the circular sector creation.
 */
void AreaEditor::clear_circle_sector() {
    new_circle_sector_step = 0;
    new_circle_sector_points.clear();
}


/**
 * @brief Clears the currently loaded area data.
 */
void AreaEditor::clear_current_area() {
    reference_file_path.clear();
    update_reference();
    clear_selection();
    clear_circle_sector();
    clear_layout_drawing();
    clear_layout_moving();
    clear_problems();
    
    clear_area_textures();
    
    if(game.cur_area_data) {
        for(size_t s = 0; s < game.cur_area_data->tree_shadows.size(); s++) {
            game.content.bitmaps.list.free(game.cur_area_data->tree_shadows[s]->bmp_name);
        }
    }
    
    game.cam.set_pos(Point());
    game.cam.set_zoom(1.0f);
    show_cross_section = false;
    show_cross_section_grid = false;
    show_blocking_sectors = false;
    show_path_preview = false;
    path_preview.clear();
    //LARGE_FLOAT means they were never given a previous position.
    path_preview_checkpoints[0] = Point(LARGE_FLOAT);
    path_preview_checkpoints[1] = Point(LARGE_FLOAT);
    cross_section_checkpoints[0] = Point(LARGE_FLOAT);
    cross_section_checkpoints[1] = Point(LARGE_FLOAT);
    
    clear_texture_suggestions();
    
    game.content.unload_current_area(CONTENT_LOAD_LEVEL_EDITOR);
    
    changes_mgr.reset();
    backup_timer.start(game.options.area_editor_backup_interval);
    
    thumbnail_needs_saving = false;
    thumbnail_backup_needs_saving = false;
    
    sub_state = EDITOR_SUB_STATE_NONE;
    state = EDITOR_STATE_MAIN;
}


/**
 * @brief Clears the data about the layout drawing.
 */
void AreaEditor::clear_layout_drawing() {
    drawing_nodes.clear();
    drawing_line_result = DRAWING_LINE_RESULT_OK;
    sector_split_info.useless_split_part_2_checkpoint = INVALID;
}


/**
 * @brief Clears the data about the layout moving.
 */
void AreaEditor::clear_layout_moving() {
    if(pre_move_area_data) {
        forget_prepared_state(pre_move_area_data);
        pre_move_area_data = nullptr;
    }
    pre_move_vertex_coords.clear();
    clear_selection();
    moving = false;
}


/**
 * @brief Clears the data about the current problems, if any.
 */
void AreaEditor::clear_problems() {
    problem_type = EPT_NONE_YET;
    problem_title.clear();
    problem_description.clear();
    problem_edge_intersection.e1 = nullptr;
    problem_edge_intersection.e2 = nullptr;
    problem_mob_ptr = nullptr;
    problem_path_stop_ptr = nullptr;
    problem_sector_ptr = nullptr;
    problem_shadow_ptr = nullptr;
    problem_vertex_ptr = nullptr;
}


/**
 * @brief Clears the data about the current selection.
 */
void AreaEditor::clear_selection() {
    if(sub_state == EDITOR_SUB_STATE_OCTEE) {
        sub_state = EDITOR_SUB_STATE_NONE;
    }
    
    selected_vertexes.clear();
    selected_edges.clear();
    selected_sectors.clear();
    selected_mobs.clear();
    selected_path_stops.clear();
    selected_path_links.clear();
    selected_shadow = nullptr;
    selection_homogenized = false;
    set_selection_status_text();
}


/**
 * @brief Clears the list of texture suggestions. This frees up the bitmaps.
 */
void AreaEditor::clear_texture_suggestions() {
    for(size_t s = 0; s < texture_suggestions.size(); s++) {
        texture_suggestions[s].destroy();
    }
    texture_suggestions.clear();
}


/**
 * @brief Clears the undo history, deleting the memory allocated for them.
 */
void AreaEditor::clear_undo_history() {
    for(size_t h = 0; h < undo_history.size(); h++) {
        delete undo_history[h].first;
    }
    undo_history.clear();
    for(size_t h = 0; h < redo_history.size(); h++) {
        delete redo_history[h].first;
    }
    redo_history.clear();
}


/**
 * @brief Code to run when the area picker is closed.
 */
void AreaEditor::close_load_dialog() {
    if(manifest.internal_name.empty() && dialogs.size() == 1) {
        //If nothing got loaded, we can't return to the editor proper.
        //Quit out, since most of the time that's the user's intent. (e.g.
        //they entered the editor and want to leave without doing anything.)
        //Also make sure no other dialogs are trying to show up, like the load
        //failed dialog.
        leave();
    }
}


/**
 * @brief Code to run when the options dialog is closed.
 */
void AreaEditor::close_options_dialog() {
    save_options();
}


/**
 * @brief Creates a new area to work on.
 *
 * @param requested_area_path Path to the requested area's folder.
 */
void AreaEditor::create_area(const string &requested_area_path) {
    //Setup.
    setup_for_new_area_pre();
    changes_mgr.mark_as_non_existent();
    
    //Basic area data.
    game.cur_area_data = new Area();
    game.content.areas.path_to_manifest(
        requested_area_path, &manifest, &game.cur_area_data->type
    );
    game.cur_area_data->manifest = &manifest;
    game.cur_area_data->user_data_path =
        FOLDER_PATHS_FROM_ROOT::AREA_USER_DATA + "/" +
        manifest.pack + "/" +
        (
            game.cur_area_data->type == AREA_TYPE_SIMPLE ?
            FOLDER_NAMES::SIMPLE_AREAS :
            FOLDER_NAMES::MISSION_AREAS
        ) + "/" +
        manifest.internal_name;
        
    //Create a sector for it.
    clear_layout_drawing();
    float r = AREA_EDITOR::COMFY_DIST * 10;
    
    LayoutDrawingNode n;
    n.raw_spot = Point(-r, -r);
    n.snapped_spot = n.raw_spot;
    drawing_nodes.push_back(n);
    
    n.raw_spot = Point(r, -r);
    n.snapped_spot = n.raw_spot;
    drawing_nodes.push_back(n);
    
    n.raw_spot = Point(r, r);
    n.snapped_spot = n.raw_spot;
    drawing_nodes.push_back(n);
    
    n.raw_spot = Point(-r, r);
    n.snapped_spot = n.raw_spot;
    drawing_nodes.push_back(n);
    
    finish_new_sector_drawing();
    
    clear_selection();
    
    //Give a texture to give to this sector.
    string texture_to_use = find_good_first_texture();
    if(!texture_to_use.empty()) {
        update_sector_texture(game.cur_area_data->sectors[0], texture_to_use);
        update_texture_suggestions(texture_to_use);
    }
    
    //Now add a leader. The first available.
    game.cur_area_data->mob_generators.push_back(
        new MobGen(Point(), game.config.leader_order[0], 0, "")
    );
    
    //Finish up.
    setup_for_new_area_post();
    update_history(manifest, "");
    
    set_status(
        "Created area \"" + manifest.internal_name + "\" successfully."
    );
}


/**
 * @brief Creates vertexes based on the edge drawing the user has just made.
 *
 * Drawing nodes that are already on vertexes don't count, but the other ones
 * either create edge splits, or create simple vertexes inside a sector.
 */
void AreaEditor::create_drawing_vertexes() {
    for(size_t n = 0; n < drawing_nodes.size(); n++) {
        LayoutDrawingNode* n_ptr = &drawing_nodes[n];
        if(n_ptr->on_vertex) continue;
        Vertex* new_vertex = nullptr;
        
        if(n_ptr->on_edge) {
            new_vertex = split_edge(n_ptr->on_edge, n_ptr->snapped_spot);
            
            //The split created new edges, so let's check future nodes
            //and update them, since they could've landed on new edges.
            for(size_t n2 = n; n2 < drawing_nodes.size(); n2++) {
                if(drawing_nodes[n2].on_edge == n_ptr->on_edge) {
                    drawing_nodes[n2].on_edge =
                        get_edge_under_point(drawing_nodes[n2].snapped_spot);
                }
            }
        } else {
            new_vertex = game.cur_area_data->new_vertex();
            new_vertex->x = n_ptr->snapped_spot.x;
            new_vertex->y = n_ptr->snapped_spot.y;
            n_ptr->is_new_vertex = true;
        }
        
        n_ptr->on_vertex = new_vertex;
    }
}


/**
 * @brief Creates a new mob where the cursor is.
 */
void AreaEditor::create_mob_under_cursor() {
    register_change("object creation");
    sub_state = EDITOR_SUB_STATE_NONE;
    Point hotspot = snap_point(game.mouse_cursor.w_pos);
    
    if(last_mob_custom_cat_name.empty()) {
        last_mob_custom_cat_name =
            game.config.pikmin_order[0]->custom_category_name;
        last_mob_type =
            game.config.pikmin_order[0];
    }
    
    game.cur_area_data->mob_generators.push_back(
        new MobGen(hotspot, last_mob_type)
    );
    
    selected_mobs.insert(game.cur_area_data->mob_generators.back());
    
    set_status("Created object.");
}


/**
 * @brief Deletes the current area.
 */
void AreaEditor::delete_current_area() {
    string orig_internal_name = manifest.internal_name;
    bool go_to_load_dialog = true;
    bool success = false;
    string message_box_text;
    
    //Start by deleting the user data, if any.
    vector<string> non_important_files;
    non_important_files.push_back(FILE_NAMES::AREA_MAIN_DATA);
    non_important_files.push_back(FILE_NAMES::AREA_GEOMETRY);
    non_important_files.push_back(FILE_NAMES::AREA_REFERENCE_CONFIG);
    wipe_folder(
        game.cur_area_data->user_data_path,
        non_important_files
    );
    
    if(!changes_mgr.exists_on_disk()) {
        //If the area doesn't exist on disk, since it was never
        //saved, then there's nothing to delete.
        success = true;
        go_to_load_dialog = true;
        
    } else {
        //Delete the actual area data.
        non_important_files.clear();
        non_important_files.push_back(FILE_NAMES::AREA_MAIN_DATA);
        non_important_files.push_back(FILE_NAMES::AREA_GEOMETRY);
        FS_DELETE_RESULT result =
            wipe_folder(
                manifest.path,
                non_important_files
            );
            
        switch(result) {
        case FS_DELETE_RESULT_OK: {
            success = true;
            go_to_load_dialog = true;
            break;
        } case FS_DELETE_RESULT_NOT_FOUND: {
            success = false;
            message_box_text =
                "Area \"" + orig_internal_name +
                "\" deletion failed! The folder was not found!";
            go_to_load_dialog = false;
            break;
        } case FS_DELETE_RESULT_HAS_IMPORTANT: {
            success = true;
            message_box_text =
                "The area \"" + orig_internal_name + "\" was deleted "
                "successfully, but the folder still has user files, which "
                "have not been deleted.";
            go_to_load_dialog = true;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            message_box_text =
                "Area \"" + orig_internal_name +
                "\" deletion failed! Something went wrong. Please make sure "
                "there are enough permissions to delete the folder and "
                "try again.";
            go_to_load_dialog = false;
            break;
        }
        }
        
    }
    
    //This code will be run after everything is done, be it after the standard
    //procedure, or after the user hits OK on the message box.
    const auto finish_up = [this, go_to_load_dialog] () {
        if(go_to_load_dialog) {
            setup_for_new_area_pre();
            open_load_dialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        set_status(
            "Deleted area \"" + orig_internal_name + "\" successfully."
        );
    } else {
        set_status(
            "Area \"" + orig_internal_name + "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(message_box_text.empty()) {
        finish_up();
    } else {
        open_message_dialog(
            "Area deletion failed!",
            message_box_text,
            finish_up
        );
    }
}


/**
 * @brief Handles the logic part of the main loop of the area editor.
 *
 */
void AreaEditor::do_logic() {
    Editor::do_logic_pre();
    
    process_gui();
    
    cursor_snap_timer.tick(game.delta_t);
    path_preview_timer.tick(game.delta_t);
    quick_preview_timer.tick(game.delta_t);
    new_sector_error_tint_timer.tick(game.delta_t);
    undo_save_lock_timer.tick(game.delta_t);
    
    if(
        game.cur_area_data &&
        !manifest.internal_name.empty() &&
        game.options.area_editor_backup_interval > 0
    ) {
        backup_timer.tick(game.delta_t);
    }
    
    selection_effect += AREA_EDITOR::SELECTION_EFFECT_SPEED * game.delta_t;
    
    Editor::do_logic_post();
}


/**
 * @brief Splits the sector using the user's final drawing.
 */
void AreaEditor::do_sector_split() {
    //Create the drawing's new edges and connect them.
    vector<Edge*> drawing_edges;
    for(size_t n = 0; n < drawing_nodes.size() - 1; n++) {
        LayoutDrawingNode* n_ptr = &drawing_nodes[n];
        LayoutDrawingNode* next_node = &drawing_nodes[n + 1];
        
        Edge* new_node_edge = game.cur_area_data->new_edge();
        
        game.cur_area_data->connect_edge_to_vertex(
            new_node_edge, n_ptr->on_vertex, 0
        );
        game.cur_area_data->connect_edge_to_vertex(
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
    vector<Edge*> new_sector_edges = drawing_edges;
    vector<Vertex*> new_sector_vertexes;
    for(size_t d = 0; d < drawing_nodes.size(); d++) {
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
    for(
        size_t t = 0;
        t < sector_split_info.traversed_vertexes[0].size() - 1;
        t++
    ) {
        new_sector_vertexes.push_back(
            sector_split_info.traversed_vertexes[0][t]
        );
    }
    
    bool is_new_clockwise = is_polygon_clockwise(new_sector_vertexes);
    
    if(is_new_clockwise == sector_split_info.is_working_at_stage_1_left) {
        //Darn, the new sector goes clockwise, which means the new sector's to
        //the right of these edges, when traversing them in stage 1's order,
        //but we know from before that the working sector is to the left!
        //(Or vice-versa.) This means that the drawing is against an inner
        //sector (it's the only way this can happen), and that this selection
        //of vertexes would result in a sector that's going around that
        //inner sector. Let's swap to the traversal stage 2 data.
        
        new_sector_vertexes.clear();
        for(size_t d = 0; d < drawing_nodes.size(); d++) {
            new_sector_vertexes.push_back(drawing_nodes[d].on_vertex);
        }
        //Same as before, skip the last.
        for(
            size_t t = 0;
            t < sector_split_info.traversed_vertexes[1].size() - 1;
            t++
        ) {
            new_sector_vertexes.push_back(
                sector_split_info.traversed_vertexes[1][t]
            );
        }
        
        for(
            size_t t = 0;
            t < sector_split_info.traversed_edges[1].size();
            t++
        ) {
            new_sector_edges.push_back(
                sector_split_info.traversed_edges[1][t]
            );
        }
        
    } else {
        //We can use stage 1's data!
        //The vertexes are already in place, so let's fill in the edges.
        for(
            size_t t = 0;
            t < sector_split_info.traversed_edges[0].size();
            t++
        ) {
            new_sector_edges.push_back(
                sector_split_info.traversed_edges[0][t]
            );
        }
        
    }
    
    //Organize all edge vertexes such that they follow the same order.
    for(size_t e = 0; e < new_sector_edges.size(); e++) {
        if(new_sector_edges[e]->vertexes[0] != new_sector_vertexes[e]) {
            new_sector_edges[e]->swap_vertexes();
        }
    }
    
    //Create the new sector, empty.
    Sector* new_sector =
        create_sector_for_layout_drawing(sector_split_info.working_sector);
        
    //Connect the edges to the sectors.
    unsigned char new_sector_side = (is_new_clockwise ? 1 : 0);
    unsigned char working_sector_side = (is_new_clockwise ? 0 : 1);
    
    for(size_t e = 0; e < new_sector_edges.size(); e++) {
        Edge* e_ptr = new_sector_edges[e];
        
        if(!e_ptr->sectors[0] && !e_ptr->sectors[1]) {
            //If it's a new edge, set it up properly.
            game.cur_area_data->connect_edge_to_sector(
                e_ptr, sector_split_info.working_sector, working_sector_side
            );
            game.cur_area_data->connect_edge_to_sector(
                e_ptr, new_sector, new_sector_side
            );
            
        } else {
            //If not, let's transfer from the working sector to the new one.
            game.cur_area_data->connect_edge_to_sector(
                e_ptr, new_sector, new_sector_side
            );
            
        }
    }
    
    //The new sector is created, but only its outer edges exist.
    //Triangulate these so we can check what's inside.
    triangulate_sector(new_sector, nullptr, false);
    
    //All sectors inside the new one need to know that
    //their outer sector changed. Since we're only checking from the edges
    //that used to be long to the working sector, the edges that were created
    //with the drawing will not be included.
    update_inner_sectors_outer_sector(
        sector_split_info.working_sector_old_edges,
        sector_split_info.working_sector,
        new_sector
    );
    
    //Finally, update all affected sectors. Only the working sector and
    //the new sector have had their triangles changed, so work only on those.
    unordered_set<Sector*> affected_sectors;
    affected_sectors.insert(sector_split_info.working_sector);
    affected_sectors.insert(new_sector);
    update_affected_sectors(affected_sectors);
    
    //Select one of the two sectors, making it ready for editing.
    //We want to select the smallest of the two, because it's the "most new".
    //If you have a sector that's a really complex shape, and you split
    //such that one of the post-split sectors is a triangle, chances are you
    //had that complex shape, and you wanted to make a new triangle from it,
    //not that you had a "triangle" and wanted to make a complex shape.
    clear_selection();
    
    if(!sector_split_info.working_sector) {
        select_sector(new_sector);
    } else {
        float working_sector_area =
            (
                sector_split_info.working_sector->bbox[1].x -
                sector_split_info.working_sector->bbox[0].x
            ) * (
                sector_split_info.working_sector->bbox[1].y -
                sector_split_info.working_sector->bbox[0].y
            );
        float new_sector_area =
            (new_sector->bbox[1].x - new_sector->bbox[0].x) *
            (new_sector->bbox[1].y - new_sector->bbox[0].y);
            
        if(working_sector_area < new_sector_area) {
            select_sector(sector_split_info.working_sector);
        } else {
            select_sector(new_sector);
        }
    }
    
    clear_layout_drawing();
    sub_state = EDITOR_SUB_STATE_NONE;
    
    register_change("sector split", sector_split_info.pre_split_area_data);
    if(!sector_split_info.working_sector) {
        set_status(
            "Created sector with " +
            amount_str((int) new_sector->edges.size(), "edge") + "."
        );
    } else {
        set_status(
            "Split sector, creating one with " +
            amount_str((int) new_sector->edges.size(), "edge") + ", one with " +
            amount_str(
                (int) sector_split_info.working_sector->edges.size(),
                "edge"
            ) + "."
        );
    }
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void AreaEditor::draw_canvas_imgui_callback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.area_ed->draw_canvas();
}


/**
 * @brief Emits a message onto the status bar, based on the given
 * triangulation error.
 *
 * @param error The triangulation error.
 */
void AreaEditor::emit_triangulation_error_status_bar_message(
    const TRIANGULATION_ERROR error
) {
    switch(error) {
    case TRIANGULATION_ERROR_LONE_EDGES: {
        set_status(
            "Some sectors have lone edges!",
            true
        );
        break;
    } case TRIANGULATION_ERROR_NOT_CLOSED: {
        set_status(
            "Some sectors are not closed!",
            true
        );
        break;
    } case TRIANGULATION_ERROR_NO_EARS: {
        set_status(
            "Some sectors could not be triangulated!",
            true
        );
        break;
    } case TRIANGULATION_ERROR_INVALID_ARGS: {
        set_status(
            "An unknown error has occured with some sectors!",
            true
        );
        break;
    } case TRIANGULATION_ERROR_NONE: {
        break;
    }
    }
}


/**
 * @brief Finishes drawing a circular sector.
 */
void AreaEditor::finish_circle_sector() {
    clear_layout_drawing();
    for(size_t p = 0; p < new_circle_sector_points.size(); p++) {
        LayoutDrawingNode n;
        n.raw_spot = new_circle_sector_points[p];
        n.snapped_spot = n.raw_spot;
        n.on_sector = get_sector(n.raw_spot, nullptr, false);
        drawing_nodes.push_back(n);
    }
    finish_new_sector_drawing();
    
    clear_circle_sector();
    sub_state = EDITOR_SUB_STATE_NONE;
}


/**
 * @brief Finishes a vertex moving procedure.
 */
void AreaEditor::finish_layout_moving() {
    unordered_set<Sector*> affected_sectors;
    get_affected_sectors(selected_vertexes, affected_sectors);
    map<Vertex*, Vertex*> merges;
    map<Vertex*, Edge*> edges_to_split;
    unordered_set<Sector*> merge_affected_sectors;
    
    //Find merge vertexes and edges to split, if any.
    for(auto &v : selected_vertexes) {
        Point p(v->x, v->y);
        
        vector<std::pair<Distance, Vertex*> > merge_vertexes =
            get_merge_vertexes(
                p, game.cur_area_data->vertexes,
                AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
            );
            
        for(size_t mv = 0; mv < merge_vertexes.size(); ) {
            Vertex* mv_ptr = merge_vertexes[mv].second;
            if(
                mv_ptr == v ||
                selected_vertexes.find(mv_ptr) != selected_vertexes.end()
            ) {
                merge_vertexes.erase(merge_vertexes.begin() + mv);
            } else {
                mv++;
            }
        }
        
        sort(
            merge_vertexes.begin(), merge_vertexes.end(),
        [] (std::pair<Distance, Vertex*> v1, std::pair<Distance, Vertex*> v2) -> bool {
            return v1.first < v2.first;
        }
        );
        
        Vertex* merge_v = nullptr;
        if(!merge_vertexes.empty()) {
            merge_v = merge_vertexes[0].second;
        }
        
        if(merge_v) {
            merges[v] = merge_v;
            
        } else {
            Edge* e_ptr = nullptr;
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
                e_ptr != nullptr &&
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
    
    set<Edge*> moved_edges;
    for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
        Edge* e_ptr = game.cur_area_data->edges[e];
        bool both_selected = true;
        for(size_t v = 0; v < 2; v++) {
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
    for(size_t v = 0; v < game.cur_area_data->vertexes.size(); v++) {
        Vertex* v_ptr = game.cur_area_data->vertexes[v];
        Point p(v_ptr->x, v_ptr->y);
        
        if(selected_vertexes.find(v_ptr) != selected_vertexes.end()) {
            continue;
        }
        bool is_merge_target = false;
        for(auto const &m : merges) {
            if(m.second == v_ptr) {
                //This vertex will have some other vertex merge into it; skip.
                is_merge_target = true;
                break;
            }
        }
        if(is_merge_target) continue;
        
        Edge* e_ptr = nullptr;
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
    
    //Before moving on and making changes, check if the move causes problems.
    //Start by checking all crossing edges, but removing all of the ones that
    //come from edge splits or vertex merges.
    vector<EdgeIntersection> intersections =
        get_intersecting_edges();
    for(auto &m : merges) {
        for(size_t e1 = 0; e1 < m.first->edges.size(); e1++) {
            for(size_t e2 = 0; e2 < m.second->edges.size(); e2++) {
                for(size_t i = 0; i < intersections.size();) {
                    if(
                        intersections[i].contains(m.first->edges[e1]) &&
                        intersections[i].contains(m.second->edges[e2])
                    ) {
                        intersections.erase(intersections.begin() + i);
                    } else {
                        i++;
                    }
                }
            }
        }
    }
    for(auto &v : edges_to_split) {
        for(size_t e = 0; e < v.first->edges.size(); e++) {
            for(size_t i = 0; i < intersections.size();) {
                if(
                    intersections[i].contains(v.first->edges[e]) &&
                    intersections[i].contains(v.second)
                ) {
                    intersections.erase(intersections.begin() + i);
                } else {
                    i++;
                }
            }
        }
    }
    
    //If we ended up with any intersection still, abort!
    if(!intersections.empty()) {
        cancel_layout_moving();
        forget_prepared_state(pre_move_area_data);
        pre_move_area_data = nullptr;
        set_status("That move would cause edges to intersect!", true);
        return;
    }
    
    //If there's a vertex between any dragged vertex and its merge, and this
    //vertex was meant to be a merge destination itself, then don't do it.
    //When the first merge happens, this vertex will be gone, and we'll be
    //unable to use it for the second merge. There are no plans to support
    //this complex corner case, so abort!
    for(auto &m : merges) {
        Vertex* crushed_vertex = nullptr;
        if(m.first->is_2nd_degree_neighbor(m.second, &crushed_vertex)) {
        
            for(auto const &m2 : merges) {
                if(m2.second == crushed_vertex) {
                    cancel_layout_moving();
                    forget_prepared_state(pre_move_area_data);
                    pre_move_area_data = nullptr;
                    set_status(
                        "That move would crush an edge that's in the middle!",
                        true
                    );
                    return;
                }
            }
        }
    }
    
    //Merge vertexes and split edges now.
    for(auto v = edges_to_split.begin(); v != edges_to_split.end(); ++v) {
        merges[v->first] =
            split_edge(v->second, Point(v->first->x, v->first->y));
        //This split could've thrown off the edge pointer of a different
        //vertex to merge. Let's re-calculate.
        Edge* new_edge = game.cur_area_data->edges.back();
        auto v2 = v;
        ++v2;
        for(; v2 != edges_to_split.end(); ++v2) {
            if(v->second != v2->second) continue;
            v2->second =
                get_correct_post_split_edge(v2->first, v2->second, new_edge);
        }
    }
    for(auto const &m : merges) {
        merge_vertex(m.first, m.second, &merge_affected_sectors);
    }
    
    affected_sectors.insert(
        merge_affected_sectors.begin(), merge_affected_sectors.end()
    );
    
    //Update all affected sectors.
    update_affected_sectors(affected_sectors);
    
    register_change("vertex movement", pre_move_area_data);
    pre_move_area_data = nullptr;
    clear_layout_moving();
}


/**
 * @brief Finishes creating a new sector.
 */
void AreaEditor::finish_new_sector_drawing() {
    if(drawing_nodes.size() < 3) {
        cancel_layout_drawing();
        return;
    }
    
    //This is the basic idea: create a new sector using the
    //vertexes provided by the user, as a "child" of an existing sector.
    
    //Get the outer sector, so we can know where to start working in.
    Sector* outer_sector = nullptr;
    if(!get_drawing_outer_sector(&outer_sector)) {
        //Something went wrong. Abort.
        cancel_layout_drawing();
        set_status(
            "That sector wouldn't have a defined parent! Try again.",
            true
        );
        return;
    }
    
    vector<Edge*> outer_sector_old_edges;
    if(outer_sector) {
        outer_sector_old_edges = outer_sector->edges;
    } else {
        for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
            Edge* e_ptr = game.cur_area_data->edges[e];
            if(e_ptr->sectors[0] == nullptr || e_ptr->sectors[1] == nullptr) {
                outer_sector_old_edges.push_back(e_ptr);
            }
        }
    }
    
    register_change("sector creation");
    
    //First, create vertexes wherever necessary.
    create_drawing_vertexes();
    
    //Now that all nodes have a vertex, create the necessary edges.
    vector<Vertex*> drawing_vertexes;
    vector<Edge*> drawing_edges;
    for(size_t n = 0; n < drawing_nodes.size(); n++) {
        LayoutDrawingNode* n_ptr = &drawing_nodes[n];
        size_t prev_node_idx =
            sum_and_wrap((int) n, -1, (int) drawing_nodes.size());
        LayoutDrawingNode* prev_node = &drawing_nodes[prev_node_idx];
        
        drawing_vertexes.push_back(n_ptr->on_vertex);
        
        Edge* prev_node_edge =
            n_ptr->on_vertex->get_edge_by_neighbor(prev_node->on_vertex);
            
        if(!prev_node_edge) {
            prev_node_edge = game.cur_area_data->new_edge();
            
            game.cur_area_data->connect_edge_to_vertex(
                prev_node_edge, prev_node->on_vertex, 0
            );
            game.cur_area_data->connect_edge_to_vertex(
                prev_node_edge, n_ptr->on_vertex, 1
            );
        }
        
        drawing_edges.push_back(prev_node_edge);
    }
    
    //Create the new sector, empty.
    Sector* new_sector = create_sector_for_layout_drawing(outer_sector);
    
    //Connect the edges to the sectors.
    bool is_clockwise = is_polygon_clockwise(drawing_vertexes);
    unsigned char inner_sector_side = (is_clockwise ? 1 : 0);
    unsigned char outer_sector_side = (is_clockwise ? 0 : 1);
    
    for(size_t e = 0; e < drawing_edges.size(); e++) {
        Edge* e_ptr = drawing_edges[e];
        
        game.cur_area_data->connect_edge_to_sector(
            e_ptr, outer_sector, outer_sector_side
        );
        game.cur_area_data->connect_edge_to_sector(
            e_ptr, new_sector, inner_sector_side
        );
    }
    
    //The new sector is created, but only its outer edges exist.
    //Triangulate these so we can check what's inside.
    triangulate_sector(new_sector, nullptr, false);
    
    //All sectors inside the new one need to know that
    //their outer sector changed.
    update_inner_sectors_outer_sector(
        outer_sector_old_edges, outer_sector, new_sector
    );
    
    //Finally, update all affected sectors. Only the working sector and
    //the new sector have had their triangles changed, so work only on those.
    unordered_set<Sector*> affected_sectors;
    affected_sectors.insert(new_sector);
    affected_sectors.insert(outer_sector);
    update_affected_sectors(affected_sectors);
    
    //Select the new sector, making it ready for editing.
    clear_selection();
    select_sector(new_sector);
    
    clear_layout_drawing();
    sub_state = EDITOR_SUB_STATE_NONE;
    
    set_status(
        "Created sector with " +
        amount_str((int) new_sector->edges.size(), "edge") + ", " +
        amount_str((int) drawing_vertexes.size(), "vertex", "vertexes") + "."
    );
}


/**
 * @brief Forgets a pre-prepared area state that was almost ready to be added to
 * the undo history.
 *
 * @param prepared_state The prepared state to forget.
 */
void AreaEditor::forget_prepared_state(Area* prepared_state) {
    delete prepared_state;
}


/**
 * @brief Returns some tooltip text that represents an area folder's manifest.
 *
 * @param path Path to the folder.
 * @param user_data_path Path to the area's user data folder, if applicable.
 * @return The tooltip text.
 */
string AreaEditor::get_folder_tooltip(
    const string &path, const string &user_data_path
) const {
    ContentManifest temp_manif;
    AREA_TYPE type;
    game.content.areas.path_to_manifest(
        path, &temp_manif, &type
    );
    string result =
        "Internal name: " + temp_manif.internal_name + "\n"
        "Area type: " + (type == AREA_TYPE_SIMPLE ? "simple" : "mission") + "\n"
        "Folder path: " + path + "\n"
        "Pack: " + game.content.packs.list[temp_manif.pack].name;
    if(!user_data_path.empty()) {
        result +=
            "\nUser data folder path: " + user_data_path;
    }
    return result;
}


/**
 * @brief In the options data file, options pertaining to an editor's history
 * have a prefix. This function returns that prefix.
 *
 * @return The prefix.
 */
string AreaEditor::get_history_option_prefix() const {
    return "area_editor_history_";
}


/**
 * @brief Returns which layout element the mouse is over, if any.
 * It will only return one of them.
 *
 * @param hovered_vertex If a vertex is hovered, it is returned here.
 * @param hovered_edge If an edge is hovered, it is returned here.
 * @param hovered_sector If a sector is hovered, it is returned here.
 */
void AreaEditor::get_hovered_layout_element(
    Vertex** hovered_vertex, Edge** hovered_edge, Sector** hovered_sector
) const {
    *hovered_vertex = get_vertex_under_point(game.mouse_cursor.w_pos);
    *hovered_edge = nullptr;
    *hovered_sector = nullptr;
    
    if(*hovered_vertex) return;
    
    if(selection_filter != SELECTION_FILTER_VERTEXES) {
        *hovered_edge = get_edge_under_point(game.mouse_cursor.w_pos);
    }
    
    if(*hovered_edge) return;
    
    if(selection_filter == SELECTION_FILTER_SECTORS) {
        *hovered_sector = get_sector_under_point(game.mouse_cursor.w_pos);
    }
}


/**
 * @brief Returns the number of required mobs for this mission.
 *
 * @return The number.
 */
size_t AreaEditor::get_mission_required_mob_count() const {
    size_t total_required = 0;
    
    if(game.cur_area_data->mission.goal_all_mobs) {
        for(
            size_t m = 0;
            m < game.cur_area_data->mob_generators.size();
            m++
        ) {
            MobGen* g = game.cur_area_data->mob_generators[m];
            if(
                game.mission_goals[game.cur_area_data->mission.goal]->
                is_mob_applicable(g->type)
            ) {
                total_required++;
            }
        }
    } else {
        total_required =
            game.cur_area_data->mission.goal_mob_idxs.size();
    }
    
    return total_required;
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string AreaEditor::get_name() const {
    return "area editor";
}


/**
 * @brief Returns the path to the currently opened content,
 * or an empty string if none.
 *
 * @return The path.
 */
string AreaEditor::get_opened_content_path() const {
    if(
        game.cur_area_data &&
        game.cur_area_data->manifest &&
        !manifest.internal_name.empty()
    ) {
        return manifest.path;
    } else {
        return "";
    }
}


/**
 * @brief Returns the current height offset for the quick sector height set
 * mode.
 *
 * @return The offset.
 */
float AreaEditor::get_quick_height_set_offset() const {
    float offset = quick_height_set_start_pos.y - game.mouse_cursor.s_pos.y;
    offset = floor(offset / 2.0f);
    offset = floor(offset / 10.0f);
    offset *= 10.0f;
    return offset;
}


/**
 * @brief Evaluates the user's drawing to figure out how the split is
 * going to work.
 *
 * @return The evaluation result.
 */
AreaEditor::SECTOR_SPLIT_RESULT AreaEditor::get_sector_split_evaluation() {
    sector_split_info.traversed_edges[0].clear();
    sector_split_info.traversed_edges[1].clear();
    sector_split_info.traversed_vertexes[0].clear();
    sector_split_info.traversed_vertexes[1].clear();
    
    //Traverse the sector, starting on the last point of the drawing,
    //going edge by edge, until we hit that point again.
    //During traversal, collect a list of traversed edges and vertexes.
    //This traversal happens in two stages. In the first stage, collect them
    //into the first set of vectors. Once the traversal reaches the checkpoint,
    //it restarts and goes in the opposite direction, collecting edges and
    //vertexes into the second set of vectors from here on out. Normally,
    //we only need the data from stage 1 to create a sector, but as we'll see
    //later on, we may need to use data from stage 2 instead.
    traverse_sector_for_split(
        sector_split_info.working_sector,
        drawing_nodes.back().on_vertex,
        drawing_nodes[0].on_vertex,
        sector_split_info.traversed_edges,
        sector_split_info.traversed_vertexes,
        &sector_split_info.is_working_at_stage_1_left
    );
    
    if(sector_split_info.traversed_edges[0].empty()) {
        //Something went wrong.
        return SECTOR_SPLIT_RESULT_INVALID;
    }
    
    if(sector_split_info.traversed_edges[1].empty()) {
        //If the sector's neighboring edges were traversed entirely
        //without finding the drawing's last point, then that point is in a set
        //of edges different from the drawing's first point. This can happen
        //if the points are in different inner sectors, or if only
        //one of them is in an inner sector.
        //If the user were to split in this way, the sector would still be
        //in one piece, except with a disallowed gash. Cancel.
        return SECTOR_SPLIT_RESULT_USELESS;
    }
    
    return SECTOR_SPLIT_RESULT_OK;
}


/**
 * @brief Focuses the camera on the problem found, if any.
 */
void AreaEditor::goto_problem() {
    switch(problem_type) {
    case EPT_NONE:
    case EPT_NONE_YET: {
        return;
        
    } case EPT_INTERSECTING_EDGES: {

        if(
            !problem_edge_intersection.e1 || !problem_edge_intersection.e2
        ) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        Point min_coords = v2p(problem_edge_intersection.e1->vertexes[0]);
        Point max_coords = min_coords;
        
        update_min_max_coords(
            min_coords, max_coords,
            v2p(problem_edge_intersection.e1->vertexes[1])
        );
        update_min_max_coords(
            min_coords, max_coords,
            v2p(problem_edge_intersection.e2->vertexes[0])
        );
        update_min_max_coords(
            min_coords, max_coords,
            v2p(problem_edge_intersection.e2->vertexes[1])
        );
        
        change_state(EDITOR_STATE_LAYOUT);
        select_edge(problem_edge_intersection.e1);
        select_edge(problem_edge_intersection.e2);
        center_camera(min_coords, max_coords);
        
        break;
        
    } case EPT_BAD_SECTOR: {

        if(game.cur_area_data->problems.non_simples.empty()) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        change_state(EDITOR_STATE_LAYOUT);
        Sector* s_ptr = game.cur_area_data->problems.non_simples.begin()->first;
        select_sector(s_ptr);
        center_camera(s_ptr->bbox[0], s_ptr->bbox[1]);
        
        break;
        
    } case EPT_LONE_EDGE: {

        if(game.cur_area_data->problems.lone_edges.empty()) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        Edge* e_ptr = *game.cur_area_data->problems.lone_edges.begin();
        Point min_coords = v2p(e_ptr->vertexes[0]);
        Point max_coords = min_coords;
        update_min_max_coords(
            min_coords, max_coords, v2p(e_ptr->vertexes[1])
        );
        
        change_state(EDITOR_STATE_LAYOUT);
        select_edge(e_ptr);
        center_camera(min_coords, max_coords);
        
        break;
        
    } case EPT_OVERLAPPING_VERTEXES: {

        if(!problem_vertex_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        change_state(EDITOR_STATE_LAYOUT);
        select_vertex(problem_vertex_ptr);
        center_camera(
            Point(
                problem_vertex_ptr->x - 64,
                problem_vertex_ptr->y - 64
            ),
            Point(
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
        
        change_state(EDITOR_STATE_LAYOUT);
        select_sector(problem_sector_ptr);
        center_camera(problem_sector_ptr->bbox[0], problem_sector_ptr->bbox[1]);
        
        break;
        
    } case EPT_TYPELESS_MOB:
    case EPT_MOB_OOB:
    case EPT_MOB_IN_WALL:
    case EPT_MOB_LINKS_TO_SELF:
    case EPT_MOB_STORED_IN_LOOP:
    case EPT_SECTORLESS_BRIDGE:
    case EPT_PILE_BRIDGE_PATH: {

        if(!problem_mob_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        change_state(EDITOR_STATE_MOBS);
        selected_mobs.insert(problem_mob_ptr);
        center_camera(problem_mob_ptr->pos - 64, problem_mob_ptr->pos + 64);
        
        break;
        
    } case EPT_LONE_PATH_STOP:
    case EPT_PATH_STOPS_TOGETHER:
    case EPT_PATH_STOP_ON_LINK:
    case EPT_PATH_STOP_OOB: {

        if(!problem_path_stop_ptr) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        change_state(EDITOR_STATE_PATHS);
        selected_path_stops.insert(problem_path_stop_ptr);
        center_camera(
            problem_path_stop_ptr->pos - 64,
            problem_path_stop_ptr->pos + 64
        );
        
        break;
        
    } case EPT_UNKNOWN_SHADOW: {

        Point min_coords, max_coords;
        get_transformed_rectangle_bounding_box(
            problem_shadow_ptr->center, problem_shadow_ptr->size,
            problem_shadow_ptr->angle, &min_coords, &max_coords
        );
        
        change_state(EDITOR_STATE_DETAILS);
        select_tree_shadow(problem_shadow_ptr);
        center_camera(min_coords, max_coords);
        
        break;
        
    } default: {
        //Nowhere to go.
        break;
        
    }
    }
}


/**
 * @brief Handles an error in the line the user is trying to draw.
 */
void AreaEditor::handle_line_error() {
    new_sector_error_tint_timer.start();
    switch(drawing_line_result) {
    case DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX: {
        break;
    } case DRAWING_LINE_RESULT_ALONG_EDGE: {
        set_status(
            "That line is drawn on top of an edge!",
            true
        );
        break;
    } case DRAWING_LINE_RESULT_CROSSES_DRAWING: {
        set_status(
            "That line crosses other lines in the drawing!",
            true
        );
        break;
    } case DRAWING_LINE_RESULT_CROSSES_EDGES: {
        set_status(
            "That line crosses existing edges!",
            true
        );
        break;
    } case DRAWING_LINE_RESULT_WAYWARD_SECTOR: {
        set_status(
            "That line goes out of the sector you're drawing on!",
            true
        );
        break;
    } case DRAWING_LINE_RESULT_OK: {
        break;
    }
    }
}


/**
 * @brief Loads the area editor.
 */
void AreaEditor::load() {
    Editor::load();
    
    //Load necessary game content.
    game.content.reload_packs();
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_MOB_TYPE,
        CONTENT_TYPE_WEATHER_CONDITION,
        CONTENT_TYPE_AREA,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    load_custom_mob_cat_types(true);
    
    //Misc. setup.
    last_mob_custom_cat_name.clear();
    last_mob_type = nullptr;
    selected_shadow = nullptr;
    selection_effect = 0.0;
    selection_homogenized = false;
    show_closest_stop = false;
    show_path_preview = false;
    preview_mode = false;
    quick_preview_timer.stop();
    preview_song.clear();
    clear_problems();
    clear_selection();
    
    change_state(EDITOR_STATE_MAIN);
    game.audio.set_current_song(game.sys_content_names.sng_editors, false);
    
    //Automatically load a file if needed, or show the load dialog.
    if(!quick_play_area_path.empty()) {
        load_area_folder(quick_play_area_path, false, true);
        game.cam.set_pos(quick_play_cam_pos);
        game.cam.set_zoom(quick_play_cam_z);
        quick_play_area_path.clear();
        
    } else if(!auto_load_folder.empty()) {
        load_area_folder(auto_load_folder, false, true);
        
    } else {
        open_load_dialog();
        
    }
}


/**
 * @brief Load the area from the disk.
 *
 * @param requested_area_path Path to the requested area's folder.
 * @param from_backup If false, load it normally.
 * If true, load from a backup, if any.
 * @param should_update_history If true, this loading process should update
 * the user's folder open history.
 */
void AreaEditor::load_area_folder(
    const string &requested_area_path,
    bool from_backup, bool should_update_history
) {
    //Setup.
    setup_for_new_area_pre();
    changes_mgr.mark_as_non_existent();
    
    //Load.
    AREA_TYPE requested_area_type;
    game.content.areas.path_to_manifest(
        requested_area_path, &manifest, &requested_area_type
    );
    if(
        !game.content.load_area_as_current(
            requested_area_path, &manifest,
            CONTENT_LOAD_LEVEL_EDITOR, from_backup
        )
    ) {
        open_message_dialog(
            "Load failed!",
            "Failed to load the area folder \"" + manifest.path + "\"!",
        [this] () { open_load_dialog(); }
        );
        manifest.clear();
        return;
    }
    
    //Calculate texture suggestions.
    map<string, size_t> texture_uses_map;
    vector<std::pair<string, size_t> > texture_uses_vector;
    
    for(size_t s = 0; s < game.cur_area_data->sectors.size(); s++) {
        const string &n = game.cur_area_data->sectors[s]->texture_info.bmp_name;
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
        u < texture_uses_vector.size() && u <
        AREA_EDITOR::MAX_TEXTURE_SUGGESTIONS;
        u++
    ) {
        texture_suggestions.push_back(
            TextureSuggestion(texture_uses_vector[u].first)
        );
    }
    
    //Other tasks.
    load_reference();
    
    //Finish up.
    changes_mgr.reset();
    setup_for_new_area_post();
    if(should_update_history) {
        update_history(manifest, game.cur_area_data->name);
    }
    set_status(
        "Loaded area \"" + manifest.internal_name + "\" " +
        (from_backup ? "from a backup " : "") +
        "successfully."
    );
}


/**
 * @brief Loads a backup file.
 */
void AreaEditor::load_backup() {
    load_area_folder(
        manifest.path,
        true, false
    );
    backup_timer.start(game.options.area_editor_backup_interval);
    changes_mgr.mark_as_changed();
    
    //We don't know if the backup's thumbnail is different from the standard
    //copy's thumbnail. To be safe, just mark it as needing a save. Loading a
    //backup is such a rare operation that it's worth the effort of re-saving
    //the bitmap.
    thumbnail_needs_saving = true;
}


/**
 * @brief Loads the reference image data from the reference configuration file.
 */
void AreaEditor::load_reference() {
    DataNode file(game.cur_area_data->user_data_path + "/" + FILE_NAMES::AREA_REFERENCE_CONFIG);
    
    if(file.fileWasOpened) {
        reference_file_path = file.getChildByName("file")->value;
        reference_center = s2p(file.getChildByName("center")->value);
        reference_size = s2p(file.getChildByName("size")->value);
        reference_alpha =
            s2i(
                file.getChildByName(
                    "alpha"
                )->getValueOrDefault(i2s(AREA_EDITOR::DEF_REFERENCE_ALPHA))
            );
        show_reference = s2b(file.getChildByName("visible")->value);
        
    } else {
        reference_file_path.clear();
        reference_center = Point();
        reference_size = Point();
        reference_alpha = 0;
        show_reference = true;
    }
    
    update_reference();
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void AreaEditor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        Point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/**
 * @brief Callback for when the user picks an area from the picker.
 *
 * @param name Name of the area.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Pointer to the area's manifest.
 * @param is_new Is it a new area, or an existing one?
 */
void AreaEditor::pick_area_folder(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    ContentManifest* temp_manif = (ContentManifest*) info;
    
    auto really_load = [this, temp_manif] () {
        close_top_dialog();
        load_area_folder(temp_manif->path, false, true);
    };
    
    if(
        temp_manif->pack == FOLDER_NAMES::BASE_PACK &&
        !game.options.engine_developer
    ) {
        open_base_content_warning_dialog(really_load);
    } else {
        really_load();
    }
}


/**
 * @brief Callback for when the user picks a texture from the picker.
 *
 * @param name Name of the texture.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Unused.
 * @param is_new Unused.
 */
void AreaEditor::pick_texture(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    Sector* s_ptr = nullptr;
    if(selected_sectors.size() == 1 || selection_homogenized) {
        s_ptr = *selected_sectors.begin();
    }
    if(!s_ptr) return;
    
    if(name == "Choose another...") {
        open_bitmap_dialog(
        [this, s_ptr] (const string &bmp) {
            if(s_ptr->texture_info.bmp_name == bmp) return;
            register_change("sector texture change");
            update_texture_suggestions(bmp);
            update_sector_texture(s_ptr, bmp);
            homogenize_selected_sectors();
            set_status("Picked an image successfully.");
        },
        FOLDER_NAMES::TEXTURES
        );
    } else {
        if(s_ptr->texture_info.bmp_name == name) return;
        register_change("sector texture change");
        update_texture_suggestions(name);
        update_sector_texture(s_ptr, name);
        homogenize_selected_sectors();
    }
}


/**
 * @brief Prepares an area state to be delivered to register_change() later,
 * or forgotten altogether with forget_prepared_state().
 *
 * @return The prepared state.
 */
Area* AreaEditor::prepare_state() {
    Area* new_state = new Area();
    game.cur_area_data->clone(*new_state);
    return new_state;
}


/**
 * @brief Code to run for the circle sector command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::circle_sector_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(
        sub_state == EDITOR_SUB_STATE_DRAWING ||
        sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR
    ) {
        return;
    }
    
    if(
        !game.cur_area_data->problems.non_simples.empty() ||
        !game.cur_area_data->problems.lone_edges.empty()
    ) {
        set_status(
            "Please fix any broken sectors or edges before trying to make "
            "a new sector!",
            true
        );
        return;
    }
    
    clear_selection();
    clear_circle_sector();
    set_status("Use the canvas to place a circular sector.");
    sub_state = EDITOR_SUB_STATE_CIRCLE_SECTOR;
}


/**
 * @brief Code to run for the copy properties command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::copy_properties_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(!selected_sectors.empty()) {
            copy_sector_properties();
        } else {
            copy_edge_properties();
        }
        break;
    } case EDITOR_STATE_MOBS: {
        copy_mob_properties();
        break;
    } case EDITOR_STATE_PATHS: {
        copy_path_link_properties();
        break;
    }
    }
}


/**
 * @brief Code to run for the delete current area command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::delete_area_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    open_dialog(
        "Delete area?",
        std::bind(&AreaEditor::process_gui_delete_area_dialog, this)
    );
    dialogs.back()->custom_size = Point(600, 0);
}


/**
 * @brief Code to run for the delete command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::delete_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        delete_edge_cmd(1.0f);
        break;
    } case EDITOR_STATE_MOBS: {
        delete_mob_cmd(1.0f);
        break;
    } case EDITOR_STATE_PATHS: {
        delete_path_cmd(1.0f);
        break;
    } case EDITOR_STATE_DETAILS: {
        delete_tree_shadow_cmd(1.0f);
        break;
    }
    }
}


/**
 * @brief Code to run for the duplicate mobs command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::duplicate_mobs_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(
        sub_state == EDITOR_SUB_STATE_NEW_MOB ||
        sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB ||
        sub_state == EDITOR_SUB_STATE_STORE_MOB_INSIDE ||
        sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK ||
        sub_state == EDITOR_SUB_STATE_DEL_MOB_LINK
    ) {
        return;
    }
    
    if(selected_mobs.empty()) {
        set_status("You have to select mobs to duplicate!", true);
    } else {
        set_status("Use the canvas to place the duplicated objects.");
        sub_state = EDITOR_SUB_STATE_DUPLICATE_MOB;
    }
}


/**
 * @brief Code to run for the grid interval decrease command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::grid_interval_decrease_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.options.area_editor_grid_interval =
        std::max(
            game.options.area_editor_grid_interval * 0.5f,
            AREA_EDITOR::MIN_GRID_INTERVAL
        );
    set_status(
        "Decreased grid interval to " +
        i2s(game.options.area_editor_grid_interval) + "."
    );
}


/**
 * @brief Code to run for the grid interval increase command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::grid_interval_increase_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.options.area_editor_grid_interval =
        std::min(
            game.options.area_editor_grid_interval * 2.0f,
            AREA_EDITOR::MAX_GRID_INTERVAL
        );
    set_status(
        "Increased grid interval to " +
        i2s(game.options.area_editor_grid_interval) + "."
    );
}


/**
 * @brief Code to run for the layout drawing command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::layout_drawing_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(
        sub_state == EDITOR_SUB_STATE_DRAWING ||
        sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR
    ) {
        return;
    }
    
    if(
        !game.cur_area_data->problems.non_simples.empty() ||
        !game.cur_area_data->problems.lone_edges.empty()
    ) {
        set_status(
            "Please fix any broken sectors or edges before trying to make "
            "a new sector!",
            true
        );
        return;
    }
    
    clear_selection();
    clear_layout_drawing();
    update_layout_drawing_status_text();
    sub_state = EDITOR_SUB_STATE_DRAWING;
}


/**
 * @brief Code to run for the load area command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::load_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    changes_mgr.ask_if_unsaved(
        load_widget_pos,
        "loading an area", "load",
        std::bind(&AreaEditor::open_load_dialog, this),
    [this] () { return save_area(false); }
    );
}


/**
 * @brief Code to run for the new mob command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::new_mob_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(
        sub_state == EDITOR_SUB_STATE_NEW_MOB ||
        sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB ||
        sub_state == EDITOR_SUB_STATE_STORE_MOB_INSIDE ||
        sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK ||
        sub_state == EDITOR_SUB_STATE_DEL_MOB_LINK
    ) {
        return;
    }
    
    clear_selection();
    set_status("Use the canvas to place a new object.");
    sub_state = EDITOR_SUB_STATE_NEW_MOB;
}


/**
 * @brief Code to run for the new path command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::new_path_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
        return;
    }
    
    clear_selection();
    path_drawing_stop_1 = nullptr;
    set_status("Use the canvas to draw a path.");
    sub_state = EDITOR_SUB_STATE_PATH_DRAWING;
}


/**
 * @brief Code to run for the new tree shadow command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::new_tree_shadow_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(sub_state == EDITOR_SUB_STATE_NEW_SHADOW) {
        return;
    }
    
    clear_selection();
    set_status("Use the canvas to place a new tree shadow.");
    sub_state = EDITOR_SUB_STATE_NEW_SHADOW;
}


/**
 * @brief Code to run for the paste properties command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::paste_properties_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(sub_state != EDITOR_SUB_STATE_NONE) return;
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(!selected_sectors.empty()) {
            paste_sector_properties();
        } else {
            paste_edge_properties();
        }
        break;
    } case EDITOR_STATE_MOBS: {
        paste_mob_properties();
        break;
    } case EDITOR_STATE_PATHS: {
        paste_path_link_properties();
        break;
    }
    }
}


/**
 * @brief Code to run for the paste texture command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::paste_texture_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(state != EDITOR_STATE_LAYOUT) return;
    if(sub_state != EDITOR_SUB_STATE_NONE) return;
    paste_sector_texture();
}


/**
 * @brief Code to run for the quick play command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::quick_play_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!save_area(false)) return;
    quick_play_area_path = manifest.path;
    quick_play_cam_pos = game.cam.pos;
    quick_play_cam_z = game.cam.zoom;
    leave();
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::quit_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&AreaEditor::leave, this),
    [this] () { return save_area(false); }
    );
}


/**
 * @brief Code to run for the redo command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::redo_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(
        sub_state != EDITOR_SUB_STATE_NONE ||
        moving || selecting || cur_transformation_widget.is_moving_handle()
    ) {
        set_status("Can't redo in the middle of an operation!", true);
        return;
    }
    
    redo();
}


/**
 * @brief Code to run for the reference toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::reference_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    show_reference = !show_reference;
    string state_str = (show_reference ? "Enabled" : "Disabled");
    save_reference();
    set_status(state_str + " reference image visibility.");
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::reload_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!changes_mgr.exists_on_disk()) return;
    
    changes_mgr.ask_if_unsaved(
        reload_widget_pos,
        "reloading the current area", "reload",
    [this] () {
        load_area_folder(string(manifest.path), false, false);
    },
    [this] () { return save_area(false); }
    );
}


/**
 * @brief Code to run for the delete edge command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::delete_edge_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    //Check if the user can delete.
    if(moving || selecting) {
        return;
    }
    
    if(selected_edges.empty()) {
        set_status("You have to select edges to delete!", true);
        return;
    }
    
    //Prepare everything.
    register_change("edge deletion");
    size_t n_before = game.cur_area_data->edges.size();
    size_t n_selected = selected_edges.size();
    
    //Delete!
    bool success = delete_edges(selected_edges);
    
    //Cleanup.
    clear_selection();
    sub_state = EDITOR_SUB_STATE_NONE;
    
    //Report.
    if(success) {
        set_status(
            "Deleted " +
            amount_str(
                (int) (n_before - game.cur_area_data->edges.size()),
                "edge"
            ) +
            " (" + i2s(n_selected) + " were selected)."
        );
    }
}


/**
 * @brief Code to run for the delete mob command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::delete_mob_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    //Check if the user can delete.
    if(moving || selecting) {
        return;
    }
    
    if(selected_mobs.empty()) {
        set_status("You have to select mobs to delete!", true);
        return;
    }
    
    //Prepare everything.
    register_change("object deletion");
    size_t n_before = game.cur_area_data->mob_generators.size();
    
    //Delete!
    delete_mobs(selected_mobs);
    
    //Cleanup.
    clear_selection();
    sub_state = EDITOR_SUB_STATE_NONE;
    
    //Report.
    set_status(
        "Deleted " +
        amount_str(
            (int) (n_before - game.cur_area_data->mob_generators.size()),
            "object"
        ) +
        "."
    );
}


/**
 * @brief Code to run for the delete path command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::delete_path_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    //Check if the user can delete.
    if(moving || selecting) {
        return;
    }
    
    if(selected_path_links.empty() && selected_path_stops.empty()) {
        set_status("You have to select something to delete!", true);
        return;
    }
    
    //Prepare everything.
    register_change("path deletion");
    size_t n_stops_before = game.cur_area_data->path_stops.size();
    size_t n_links_before = game.cur_area_data->get_nr_path_links();
    
    //Delete!
    delete_path_links(selected_path_links);
    delete_path_stops(selected_path_stops);
    
    //Cleanup.
    clear_selection();
    sub_state = EDITOR_SUB_STATE_NONE;
    path_preview.clear(); //Clear so it doesn't reference deleted stops.
    path_preview_timer.start(false);
    
    //Report.
    set_status(
        "Deleted " +
        amount_str(
            (int) (n_stops_before - game.cur_area_data->path_stops.size()),
            "path stop"
        ) +
        ", " +
        amount_str(
            (int) (n_links_before - game.cur_area_data->get_nr_path_links()),
            "path link"
        ) +
        "."
    );
}


/**
 * @brief Code to run for the remove tree shadow command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::delete_tree_shadow_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(!selected_shadow) {
        set_status("You have to select a shadow to delete!", true);
    } else {
        register_change("tree shadow deletion");
        for(
            size_t s = 0;
            s < game.cur_area_data->tree_shadows.size();
            s++
        ) {
            if(
                game.cur_area_data->tree_shadows[s] ==
                selected_shadow
            ) {
                game.cur_area_data->tree_shadows.erase(
                    game.cur_area_data->tree_shadows.begin() + s
                );
                delete selected_shadow;
                selected_shadow = nullptr;
                break;
            }
        }
        set_status("Deleted tree shadow.");
    }
}


/**
 * @brief Code to run for the save button command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::save_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!save_area(false)) {
        return;
    }
}


/**
 * @brief Code to run for the select all command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::select_all_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(sub_state == EDITOR_SUB_STATE_NONE && !selecting && !moving) {
        if(state == EDITOR_STATE_LAYOUT) {
            selected_edges.insert(
                game.cur_area_data->edges.begin(),
                game.cur_area_data->edges.end()
            );
            selected_sectors.insert(
                game.cur_area_data->sectors.begin(),
                game.cur_area_data->sectors.end()
            );
            selected_vertexes.insert(
                game.cur_area_data->vertexes.begin(),
                game.cur_area_data->vertexes.end()
            );
            
        } else if(state == EDITOR_STATE_MOBS) {
            selected_mobs.insert(
                game.cur_area_data->mob_generators.begin(),
                game.cur_area_data->mob_generators.end()
            );
            
        } else if(state == EDITOR_STATE_PATHS) {
            selected_path_stops.insert(
                game.cur_area_data->path_stops.begin(),
                game.cur_area_data->path_stops.end()
            );
        }
        
        update_vertex_selection();
        set_selection_status_text();
        
    } else if(
        sub_state == EDITOR_SUB_STATE_MISSION_MOBS
    ) {
        register_change("mission object requirements change");
        for(
            size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++
        ) {
            MobGen* m_ptr = game.cur_area_data->mob_generators[m];
            if(
                game.mission_goals[game.cur_area_data->mission.goal]->
                is_mob_applicable(m_ptr->type)
            ) {
                game.cur_area_data->mission.goal_mob_idxs.insert(m);
            }
        }
    }
}


/**
 * @brief Code to run for the selection filter command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::selection_filter_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    clear_selection();
    if(!is_shift_pressed) {
        selection_filter =
            (SELECTION_FILTER)
            sum_and_wrap(selection_filter, 1, N_SELECTION_FILTERS);
    } else {
        selection_filter =
            (SELECTION_FILTER)
            sum_and_wrap(selection_filter, -1, N_SELECTION_FILTERS);
    }
    
    string final_status_text = "Set selection filter to ";
    switch(selection_filter) {
    case SELECTION_FILTER_SECTORS: {
        final_status_text += "sectors + edges + vertexes";
        break;
    } case SELECTION_FILTER_EDGES: {
        final_status_text += "edges + vertexes";
        break;
    } case SELECTION_FILTER_VERTEXES: {
        final_status_text += "vertexes";
        break;
    } case N_SELECTION_FILTERS: {
        break;
    }
    }
    final_status_text += ".";
    set_status(final_status_text);
}


/**
 * @brief Code to run for the snap mode command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::snap_mode_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!is_shift_pressed) {
        game.options.area_editor_snap_mode =
            (AreaEditor::SNAP_MODE)
            sum_and_wrap(game.options.area_editor_snap_mode, 1, N_SNAP_MODES);
    } else {
        game.options.area_editor_snap_mode =
            (AreaEditor::SNAP_MODE)
            sum_and_wrap(game.options.area_editor_snap_mode, -1, N_SNAP_MODES);
    }
    
    string final_status_text = "Set snap mode to ";
    switch(game.options.area_editor_snap_mode) {
    case SNAP_MODE_GRID: {
        final_status_text += "grid";
        break;
    } case SNAP_MODE_VERTEXES: {
        final_status_text += "vertexes";
        break;
    } case SNAP_MODE_EDGES: {
        final_status_text += "edges";
        break;
    } case SNAP_MODE_NOTHING: {
        final_status_text += "nothing";
        break;
    } case N_SNAP_MODES: {
        break;
    }
    }
    final_status_text += ".";
    set_status(final_status_text);
}


/**
 * @brief Code to run for the undo command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::undo_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(
        sub_state != EDITOR_SUB_STATE_NONE ||
        moving || selecting || cur_transformation_widget.is_moving_handle()
    ) {
        set_status("Can't undo in the middle of an operation!", true);
        return;
    }
    
    undo();
}


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::zoom_and_pos_reset_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(game.cam.target_zoom == 1.0f) {
        game.cam.target_pos = Point();
    } else {
        game.cam.target_zoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom everything command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::zoom_everything_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    bool got_something = false;
    Point min_coords, max_coords;
    
    for(size_t v = 0; v < game.cur_area_data->vertexes.size(); v++) {
        Vertex* v_ptr = game.cur_area_data->vertexes[v];
        if(v_ptr->x < min_coords.x || !got_something) {
            min_coords.x = v_ptr->x;
        }
        if(v_ptr->y < min_coords.y || !got_something) {
            min_coords.y = v_ptr->y;
        }
        if(v_ptr->x > max_coords.x || !got_something) {
            max_coords.x = v_ptr->x;
        }
        if(v_ptr->y > max_coords.y || !got_something) {
            max_coords.y = v_ptr->y;
        }
        got_something = true;
    }
    
    for(size_t m = 0; m < game.cur_area_data->mob_generators.size(); m++) {
        MobGen* m_ptr = game.cur_area_data->mob_generators[m];
        if(m_ptr->pos.x < min_coords.x || !got_something) {
            min_coords.x = m_ptr->pos.x;
        }
        if(m_ptr->pos.y < min_coords.y || !got_something) {
            min_coords.y = m_ptr->pos.y;
        }
        if(m_ptr->pos.x > max_coords.x || !got_something) {
            max_coords.x = m_ptr->pos.x;
        }
        if(m_ptr->pos.y > max_coords.y || !got_something) {
            max_coords.y = m_ptr->pos.y;
        }
        got_something = true;
    }
    
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        PathStop* s_ptr = game.cur_area_data->path_stops[s];
        if(s_ptr->pos.x < min_coords.x || !got_something) {
            min_coords.x = s_ptr->pos.x;
        }
        if(s_ptr->pos.y < min_coords.y || !got_something) {
            min_coords.y = s_ptr->pos.y;
        }
        if(s_ptr->pos.x > max_coords.x || !got_something) {
            max_coords.x = s_ptr->pos.x;
        }
        if(s_ptr->pos.y > max_coords.y || !got_something) {
            max_coords.y = s_ptr->pos.y;
        }
        got_something = true;
    }
    
    if(!got_something) return;
    
    center_camera(min_coords, max_coords);
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::zoom_in_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        clamp(
            game.cam.target_zoom +
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/**
 * @brief Code to run for the zoom out command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::zoom_out_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        clamp(
            game.cam.target_zoom -
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/**
 * @brief Recreates the current drawing's nodes. Useful if the data the nodes
 * were holding is stale, like if the area's state had to be reverted
 * mid-drawing.
 *
 */
void AreaEditor::recreate_drawing_nodes() {
    for(size_t n = 0; n < drawing_nodes.size(); n++) {
        drawing_nodes[n] = LayoutDrawingNode(this, drawing_nodes[n].raw_spot);
    }
}


/**
 * @brief Redoes the latest undone change to the area using the undo history,
 * if available.
 */
void AreaEditor::redo() {
    if(redo_history.empty()) {
        set_status("Nothing to redo.");
        return;
    }
    
    //Let's first save the state of things right now so we can feed it into
    //the undo history afterwards.
    Area* new_state = new Area();
    game.cur_area_data->clone(*new_state);
    string operation_name = redo_history.front().second;
    
    //Change the area state.
    set_state_from_undo_or_redo_history(redo_history.front().first);
    
    //Feed the previous state into the undo history.
    undo_history.push_front(make_pair(new_state, operation_name));
    delete redo_history.front().first;
    redo_history.pop_front();
    
    set_status("Redo successful: " + operation_name + ".");
}


/**
 * @brief Saves the state of the area in the undo history.
 *
 * When this happens, a timer is set. During this timer, if the next change's
 * operation is the same as the previous one's, then it is ignored.
 * This is useful to stop, for instance, a slider
 * drag from saving several dozen operations in the undo history.
 *
 * @param operation_name Name of the operation.
 * @param pre_prepared_state If you have the area state prepared from
 * elsewhere in the code, specify it here.
 * Otherwise, it uses the current area state.
 */
void AreaEditor::register_change(
    const string &operation_name, Area* pre_prepared_state
) {
    changes_mgr.mark_as_changed();
    
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
    
    Area* new_state = pre_prepared_state;
    if(!pre_prepared_state) {
        new_state = new Area();
        game.cur_area_data->clone(*new_state);
    }
    undo_history.push_front(make_pair(new_state, operation_name));
    
    for(size_t h = 0; h < redo_history.size(); h++) {
        delete redo_history[h].first;
    }
    redo_history.clear();
    
    undo_save_lock_operation = operation_name;
    undo_save_lock_timer.start();
    
    update_undo_history();
}


/**
 * @brief Reloads all loaded areas.
 */
void AreaEditor::reload_areas() {
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
    }
    );
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
}


/**
 * @brief Removes the current area thumbnail, if any.
 */
void AreaEditor::remove_thumbnail() {
    game.cur_area_data->thumbnail = nullptr;
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void AreaEditor::reset_cam_xy() {
    game.cam.target_pos = Point();
}


/**
 * @brief Resets the camera's zoom.
 */
void AreaEditor::reset_cam_zoom() {
    zoom_with_cursor(1.0f);
}


/**
 * @brief Returns to a previously prepared area state.
 *
 * @param prepared_state Prepared state to return to.
 */
void AreaEditor::rollback_to_prepared_state(Area* prepared_state) {
    prepared_state->clone(*(game.cur_area_data));
}


/**
 * @brief Saves the area onto the disk.
 *
 * @param to_backup If false, save normally.
 * If true, save to an auto-backup file.
 * @return Whether it succeded.
 */
bool AreaEditor::save_area(bool to_backup) {

    //First, some cleanup.
    bool deleted_sectors;
    game.cur_area_data->cleanup(&deleted_sectors);
    if(deleted_sectors && !selected_sectors.empty()) {
        clear_selection();
    }
    
    //Store everything into the relevant data nodes.
    DataNode geometry_file("", "");
    DataNode main_data_file("", "");
    game.cur_area_data->save_geometry_to_data_node(&geometry_file);
    game.cur_area_data->save_main_data_to_data_node(&main_data_file);
    if(game.cur_area_data->type == AREA_TYPE_MISSION) {
        game.cur_area_data->save_mission_data_to_data_node(&main_data_file);
    }
    
    //Save the thumbnail, or delete it if none.
    //al_save_bitmap is slow, so let's only write the thumbnail file
    //if there have been changes.
    if(
        (thumbnail_needs_saving && !to_backup) ||
        (thumbnail_backup_needs_saving && to_backup)
    ) {
        game.cur_area_data->save_thumbnail(to_backup);
        (to_backup ? thumbnail_backup_needs_saving : thumbnail_needs_saving) =
            false;
    }
    
    //Finally, actually save to disk.
    string base_folder_path =
        to_backup ? game.cur_area_data->user_data_path : manifest.path;
    string main_data_file_path =
        base_folder_path + "/" + FILE_NAMES::AREA_MAIN_DATA;
    string geometry_file_path =
        base_folder_path + "/" + FILE_NAMES::AREA_GEOMETRY;
        
    bool geo_save_ok = geometry_file.saveFile(geometry_file_path);
    bool main_data_save_ok = main_data_file.saveFile(main_data_file_path);
    
    if(!geo_save_ok || !main_data_save_ok) {
        show_system_message_box(
            nullptr, "Save failed!",
            "Could not save the area!",
            (
                "An error occured while saving the area to the folder \"" +
                base_folder_path + "\". "
                "Make sure that the folder exists and it is not read-only, "
                "and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        
        set_status("Could not save the area!", true);
        
    }
    
    //Set up some things post-save.
    backup_timer.start(game.options.area_editor_backup_interval);
    
    save_reference();
    
    bool save_successful = geo_save_ok && main_data_save_ok;
    if(save_successful && !to_backup) {
        //If this was a normal save, save the backup too, so that the
        //maker doesn't have an outdated backup.
        save_backup();
        
        changes_mgr.mark_as_saved();
        set_status("Saved area successfully.");
        
        update_history(manifest, game.cur_area_data->name);
    }
    
    return save_successful;
}


/**
 * @brief Saves the area onto a backup file.
 */
void AreaEditor::save_backup() {

    //Restart the timer.
    backup_timer.start(game.options.area_editor_backup_interval);
    
    save_area(true);
}


/**
 * @brief Saves the reference data to disk, in the area's reference config file.
 */
void AreaEditor::save_reference() {
    string file_path =
        game.cur_area_data->user_data_path + "/" + FILE_NAMES::AREA_REFERENCE_CONFIG;
        
    if(!reference_bitmap) {
        //The user doesn't want a reference any more.
        //Delete its config file.
        al_remove_filename(file_path.c_str());
        return;
    }
    
    DataNode reference_file("", "");
    GetterWriter gw(&reference_file);

    gw.get("file", reference_file_path);
    gw.get("center", reference_center);
    gw.get("size", reference_size);
    gw.get("alpha", reference_alpha);
    gw.get("visible", show_reference);
    
    reference_file.saveFile(file_path);
}


/**
 * @brief Selects an edge and its vertexes.
 *
 * @param e Edge to select.
 */
void AreaEditor::select_edge(Edge* e) {
    if(selection_filter == SELECTION_FILTER_VERTEXES) return;
    selected_edges.insert(e);
    for(size_t v = 0; v < 2; v++) {
        select_vertex(e->vertexes[v]);
    }
    set_selection_status_text();
}


/**
 * @brief Selects all path stops with the given label.
 *
 * @param label Label to search for.
 */
void AreaEditor::select_path_stops_with_label(const string &label) {
    clear_selection();
    for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
        PathStop* s_ptr = game.cur_area_data->path_stops[s];
        if(s_ptr->label == label) {
            selected_path_stops.insert(s_ptr);
        }
    }
    set_selection_status_text();
}


/**
 * @brief Selects a sector and its edges and vertexes.
 *
 * @param s Sector to select.
 */
void AreaEditor::select_sector(Sector* s) {
    if(selection_filter != SELECTION_FILTER_SECTORS) return;
    selected_sectors.insert(s);
    for(size_t e = 0; e < s->edges.size(); e++) {
        select_edge(s->edges[e]);
    }
    set_selection_status_text();
}


/**
 * @brief Selects a tree shadow.
 *
 * @param s_ptr Tree shadow to select.
 */
void AreaEditor::select_tree_shadow(TreeShadow* s_ptr) {
    selected_shadow = s_ptr;
    set_selection_status_text();
}


/**
 * @brief Selects a vertex.
 *
 * @param v Vertex to select.
 */
void AreaEditor::select_vertex(Vertex* v) {
    selected_vertexes.insert(v);
    set_selection_status_text();
    update_vertex_selection();
}


/**
 * @brief Sets the vector of points that make up a new circle sector.
 */
void AreaEditor::set_new_circle_sector_points() {
    float anchor_angle =
        get_angle(new_circle_sector_center, new_circle_sector_anchor);
    float cursor_angle =
        get_angle(new_circle_sector_center, game.mouse_cursor.w_pos);
    float radius =
        Distance(
            new_circle_sector_center, new_circle_sector_anchor
        ).to_float();
    float angle_dif =
        get_angle_smallest_dif(cursor_angle, anchor_angle);
        
    size_t n_points = AREA_EDITOR::MAX_CIRCLE_SECTOR_POINTS;
    if(angle_dif > 0) {
        n_points = round(TAU / angle_dif);
    }
    n_points =
        clamp(
            n_points,
            AREA_EDITOR::MIN_CIRCLE_SECTOR_POINTS,
            AREA_EDITOR::MAX_CIRCLE_SECTOR_POINTS
        );
        
    new_circle_sector_points.clear();
    for(size_t p = 0; p < n_points; p++) {
        float delta_a = (TAU / n_points) * p;
        new_circle_sector_points.push_back(
            Point(
                new_circle_sector_center.x +
                radius * cos(anchor_angle + delta_a),
                new_circle_sector_center.y +
                radius * sin(anchor_angle + delta_a)
            )
        );
    }
    
    new_circle_sector_valid_edges.clear();
    for(size_t p = 0; p < n_points; p++) {
        Point next = get_next_in_vector(new_circle_sector_points, p);
        bool valid = true;
        
        for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
            Edge* e_ptr = game.cur_area_data->edges[e];
            
            if(
                line_segs_intersect(
                    Point(
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                    ),
                    Point(
                        e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                    ),
                    new_circle_sector_points[p], next,
                    nullptr, nullptr
                )
            ) {
                valid = false;
                break;
            }
        }
        
        new_circle_sector_valid_edges.push_back(valid);
    }
}


/**
 * @brief Sets the status text based on how many things are selected.
 */
void AreaEditor::set_selection_status_text() {
    set_status();
    
    if(game.cur_area_data && !game.cur_area_data->problems.non_simples.empty()) {
        emit_triangulation_error_status_bar_message(
            game.cur_area_data->problems.non_simples.begin()->second
        );
    }
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(!selected_vertexes.empty()) {
            set_status(
                "Selected " +
                amount_str(
                    (int) selected_sectors.size(), "sector"
                ) +
                ", " +
                amount_str(
                    (int) selected_edges.size(), "edge"
                ) +
                ", " +
                amount_str(
                    (int) selected_vertexes.size(), "vertex", "vertexes"
                ) +
                "."
            );
        }
        break;
        
    } case EDITOR_STATE_MOBS: {
        if(!selected_mobs.empty()) {
            set_status(
                "Selected " +
                amount_str((int) selected_mobs.size(), "object") +
                "."
            );
        }
        break;
        
    } case EDITOR_STATE_PATHS: {
        if(!selected_path_links.empty() || !selected_path_stops.empty()) {
            size_t normals_found = 0;
            size_t one_ways_found = 0;
            for(const auto &l : selected_path_links) {
                if(l->end_ptr->get_link(l->start_ptr)) {
                    //They both link to each other. So it's a two-way.
                    normals_found++;
                } else {
                    one_ways_found++;
                }
            }
            set_status(
                "Selected " +
                amount_str((int) selected_path_stops.size(), "path stop") +
                ", " +
                amount_str(
                    (int) ((normals_found / 2.0f) + one_ways_found),
                    "path link"
                ) +
                "."
            );
        }
        break;
        
    } case EDITOR_STATE_DETAILS: {
        if(selected_shadow) {
            set_status("Selected a tree shadow.");
        }
        break;
        
    }
    }
}


/**
 * @brief Changes the state of the area using one of the saved states in the
 * undo history or redo history.
 *
 * @param state State to load.
 */
void AreaEditor::set_state_from_undo_or_redo_history(Area* state) {
    state->clone(*(game.cur_area_data));
    
    undo_save_lock_timer.stop();
    undo_save_lock_operation.clear();
    update_undo_history();
    
    clear_selection();
    clear_circle_sector();
    clear_layout_drawing();
    clear_layout_moving();
    clear_problems();
    
    update_all_edge_offset_caches();
    
    path_preview.clear(); //Clear so it doesn't reference deleted stops.
    path_preview_timer.start(false);
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Sets up the editor's logic to split a sector.
 */
void AreaEditor::setup_sector_split() {
    if(drawing_nodes.size() < 2) {
        cancel_layout_drawing();
        return;
    }
    
    sector_split_info.pre_split_area_data = prepare_state();
    
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
    sector_split_info.working_sector =
        get_sector_under_point(
            (drawing_nodes[0].snapped_spot + drawing_nodes[1].snapped_spot) /
            2.0f
        );
    sector_split_info.working_sector_old_edges.clear();
    if(sector_split_info.working_sector) {
        sector_split_info.working_sector_old_edges =
            sector_split_info.working_sector->edges;
    } else {
        for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
            Edge* e_ptr = game.cur_area_data->edges[e];
            if(e_ptr->sectors[0] == nullptr || e_ptr->sectors[1] == nullptr) {
                sector_split_info.working_sector_old_edges.push_back(e_ptr);
            }
        }
    }
    
    //First, create vertexes wherever necessary.
    create_drawing_vertexes();
}


/**
 * @brief Sets up the editor for a new area,
 * be it from an existing file or from scratch, after the actual creation/load
 * takes place.
 */
void AreaEditor::setup_for_new_area_post() {
    clear_undo_history();
    update_undo_history();
    update_all_edge_offset_caches();
}


/**
 * @brief Sets up the editor for a new area,
 * be it from an existing file or from scratch, before the actual creation/load
 * takes place.
 */
void AreaEditor::setup_for_new_area_pre() {
    clear_current_area();
    manifest.clear();
    
    game.cam.zoom = 1.0f;
    game.cam.pos = Point();
    
    state = EDITOR_STATE_MAIN;
    
    //At this point we'll have nearly unloaded some assets like the thumbnail.
    //Since Dear ImGui still hasn't rendered the current frame, which could
    //have had those assets on-screen, if it tries now it'll crash. So skip.
    game.skip_dear_imgui_frame = true;
}


/**
 * @brief Procedure to start moving the selected mobs.
 */
void AreaEditor::start_mob_move() {
    register_change("object movement");
    
    move_closest_mob = nullptr;
    Distance move_closest_mob_dist;
    for(auto const &m : selected_mobs) {
        pre_move_mob_coords[m] = m->pos;
        
        Distance d(game.mouse_cursor.w_pos, m->pos);
        if(!move_closest_mob || d < move_closest_mob_dist) {
            move_closest_mob = m;
            move_closest_mob_dist = d;
            move_start_pos = m->pos;
        }
    }
    
    move_mouse_start_pos = game.mouse_cursor.w_pos;
    moving = true;
}


/**
 * @brief Procedure to start moving the selected path stops.
 */
void AreaEditor::start_path_stop_move() {
    register_change("path stop movement");
    
    move_closest_stop = nullptr;
    Distance move_closest_stop_dist;
    for(
        auto s = selected_path_stops.begin();
        s != selected_path_stops.end(); ++s
    ) {
        pre_move_stop_coords[*s] = (*s)->pos;
        
        Distance d(game.mouse_cursor.w_pos, (*s)->pos);
        if(!move_closest_stop || d < move_closest_stop_dist) {
            move_closest_stop = *s;
            move_closest_stop_dist = d;
            move_start_pos = (*s)->pos;
        }
    }
    
    move_mouse_start_pos = game.mouse_cursor.w_pos;
    moving = true;
}


/**
 * @brief Procedure to start moving the selected vertexes.
 */
void AreaEditor::start_vertex_move() {
    pre_move_area_data = prepare_state();
    
    move_closest_vertex = nullptr;
    Distance move_closest_vertex_dist;
    for(auto const &v : selected_vertexes) {
        Point p(v->x, v->y);
        pre_move_vertex_coords[v] = p;
        
        Distance d(game.mouse_cursor.w_pos, p);
        if(!move_closest_vertex || d < move_closest_vertex_dist) {
            move_closest_vertex = v;
            move_closest_vertex_dist = d;
            move_start_pos = p;
        }
    }
    
    move_mouse_start_pos = game.mouse_cursor.w_pos;
    moving = true;
}


/**
 * @brief Traverses a sector's edges, in order, going from neighbor to neighbor.
 *
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
 *
 * @param s_ptr Sector to traverse.
 * @param begin Vertex to begin in.
 * @param checkpoint Vertex to switch stages at.
 * @param edges Pointer to an array of two vectors.
 * Edges encountered during each stage are inserted into either one
 * of these vectors.
 * @param vertexes Pointer to an array of two vectors.
 * Vertexes encountered during each stage are inserted into either one
 * of these vectors.
 * @param working_sector_left This bool will be set to true if,
 * during stage 1 traversal, the working sector is to the left,
 * and false if to the right.
 */
void AreaEditor::traverse_sector_for_split(
    const Sector* s_ptr, Vertex* begin, const Vertex* checkpoint,
    vector<Edge*>* edges, vector<Vertex*>* vertexes,
    bool* working_sector_left
) {
    Edge* first_e_ptr = nullptr;
    unsigned char first_edge_visits = 0;
    
    for(unsigned char s = 0; s < 2; s++) {
        Vertex* v_ptr = begin;
        Vertex* prev_v_ptr = nullptr;
        float prev_e_angle = TAU / 2.0f;
        
        while(true) {
            Edge* next_e_ptr = nullptr;
            float next_e_angle = 0.0f;
            Vertex* next_v_ptr = nullptr;
            
            find_trace_edge(
                v_ptr, prev_v_ptr, s_ptr, prev_e_angle, s == 0,
                &next_e_ptr, &next_e_angle, &next_v_ptr, nullptr
            );
            
            if(!next_e_ptr) {
                return;
            }
            
            if(!first_e_ptr) {
                first_e_ptr = next_e_ptr;
                //In stage 1, travelling in this direction, is the
                //working sector to the left or to the right?
                if(next_e_ptr->vertexes[0] == begin) {
                    //This edge travels in the same direction as us. Side 0 is
                    //to the left, side 1 is to the right, so just check if the
                    //working sector is to the left.
                    *working_sector_left = (next_e_ptr->sectors[0] == s_ptr);
                } else {
                    //This edge travels the opposite way. Same logic as above,
                    //but reversed.
                    *working_sector_left = (next_e_ptr->sectors[1] == s_ptr);
                }
            }
            
            prev_v_ptr = v_ptr;
            prev_e_angle = next_e_angle;
            v_ptr = next_v_ptr;
            
            edges[s].push_back(next_e_ptr);
            vertexes[s].push_back(next_v_ptr);
            
            if(next_v_ptr == checkpoint) {
                //Enter stage 2, or quit.
                break;
            }
            
            if(next_e_ptr == first_e_ptr) {
                first_edge_visits++;
                if(first_edge_visits == 2) {
                    //We retreaded old ground without finding the checkpoint?
                    //Finish the algorithm right now.
                    return;
                }
            }
        }
    }
}


/**
 * @brief Undoes the last change to the area using the undo history,
 * if available.
 */
void AreaEditor::undo() {
    if(undo_history.empty()) {
        set_status("Nothing to undo.");
        return;
    }
    
    //Let's first save the state of things right now so we can feed it into
    //the redo history afterwards.
    Area* new_state = new Area();
    game.cur_area_data->clone(*new_state);
    string operation_name = undo_history.front().second;
    
    //Change the area state.
    set_state_from_undo_or_redo_history(undo_history.front().first);
    
    //Feed the previous state into the redo history.
    redo_history.push_front(make_pair(new_state, operation_name));
    delete undo_history.front().first;
    undo_history.pop_front();
    
    set_status("Undo successful: " + operation_name + ".");
}


/**
 * @brief Undoes the last placed layout drawing node.
 */
void AreaEditor::undo_layout_drawing_node() {
    if(drawing_nodes.empty()) return;
    drawing_nodes.erase(
        drawing_nodes.begin() + drawing_nodes.size() - 1
    );
    if(
        sector_split_info.useless_split_part_2_checkpoint != INVALID &&
        drawing_nodes.size() < sector_split_info.useless_split_part_2_checkpoint
    ) {
        //Back to before useless split part 2. Remove the checkpoint.
        sector_split_info.useless_split_part_2_checkpoint = INVALID;
    }
    update_layout_drawing_status_text();
}


/**
 * @brief Unloads the editor from memory.
 */
void AreaEditor::unload() {
    Editor::unload();
    
    clear_undo_history();
    
    if(copy_buffer_sector) {
        delete copy_buffer_sector;
        copy_buffer_sector = nullptr;
    }
    if(copy_buffer_edge) {
        delete copy_buffer_edge;
        copy_buffer_edge = nullptr;
    }
    if(copy_buffer_mob) {
        delete copy_buffer_mob;
        copy_buffer_mob = nullptr;
    }
    if(copy_buffer_path_link) {
        delete copy_buffer_path_link;
        copy_buffer_path_link = nullptr;
    }
    
    clear_current_area();
    
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
        CONTENT_TYPE_WEATHER_CONDITION,
        CONTENT_TYPE_MOB_TYPE,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
}


/**
 * @brief Updates all edge offset caches relevant to the area editor.
 */
void AreaEditor::update_all_edge_offset_caches() {
    game.wall_smoothing_effect_caches.clear();
    game.wall_smoothing_effect_caches.insert(
        game.wall_smoothing_effect_caches.begin(),
        game.cur_area_data->edges.size(),
        EdgeOffsetCache()
    );
    update_offset_effect_caches(
        game.wall_smoothing_effect_caches,
        unordered_set<Vertex*>(
            game.cur_area_data->vertexes.begin(),
            game.cur_area_data->vertexes.end()
        ),
        does_edge_have_ledge_smoothing,
        get_ledge_smoothing_length,
        get_ledge_smoothing_color
    );
    game.wall_shadow_effect_caches.clear();
    game.wall_shadow_effect_caches.insert(
        game.wall_shadow_effect_caches.begin(),
        game.cur_area_data->edges.size(),
        EdgeOffsetCache()
    );
    update_offset_effect_caches(
        game.wall_shadow_effect_caches,
        unordered_set<Vertex*>(
            game.cur_area_data->vertexes.begin(),
            game.cur_area_data->vertexes.end()
        ),
        does_edge_have_wall_shadow,
        get_wall_shadow_length,
        get_wall_shadow_color
    );
    game.liquid_limit_effect_caches.clear();
    game.liquid_limit_effect_caches.insert(
        game.liquid_limit_effect_caches.begin(),
        game.cur_area_data->edges.size(),
        EdgeOffsetCache()
    );
    update_offset_effect_caches(
        game.liquid_limit_effect_caches,
        unordered_set<Vertex*>(
            game.cur_area_data->vertexes.begin(),
            game.cur_area_data->vertexes.end()
        ),
        does_edge_have_liquid_limit,
        get_liquid_limit_length,
        get_liquid_limit_color
    );
}


/**
 * @brief Updates the status text according to what's going on in the current
 * sector drawing.
 *
 */
void AreaEditor::update_layout_drawing_status_text() {
    bool useless_split_part_2 = false;
    if(
        sector_split_info.useless_split_part_2_checkpoint !=
        INVALID &&
        drawing_nodes.size() >=
        sector_split_info.useless_split_part_2_checkpoint
    ) {
        useless_split_part_2 = true;
    }
    
    if(useless_split_part_2) {
        set_status(
            "To split this sector, continue your "
            "drawing to make a new sector."
        );
    } else {
        set_status("Use the canvas to draw a sector.");
    }
}


/**
 * @brief Updates the reference image's bitmap, since its file name
 * just changed.
 */
void AreaEditor::update_reference() {
    if(reference_bitmap && reference_bitmap != game.bmp_error) {
        al_destroy_bitmap(reference_bitmap);
    }
    reference_bitmap = nullptr;
    
    if(!reference_file_path.empty()) {
        reference_bitmap =
            load_bmp(reference_file_path, nullptr, false, true, true);
            
        if(
            reference_size.x == 0 ||
            reference_size.y == 0
        ) {
            //Let's assume this is a new reference. Reset sizes and alpha.
            reference_size = get_bitmap_dimensions(reference_bitmap);
            reference_alpha = AREA_EDITOR::DEF_REFERENCE_ALPHA;
        }
    } else {
        reference_center = Point();
        reference_size = Point();
    }
}


/**
 * @brief Updates a sector's texture.
 *
 * @param s_ptr Sector to update.
 * @param internal_name Internal name of the new texture.
 */
void AreaEditor::update_sector_texture(
    Sector* s_ptr, const string &internal_name
) {
    game.content.bitmaps.list.free(s_ptr->texture_info.bmp_name);
    s_ptr->texture_info.bmp_name = internal_name;
    s_ptr->texture_info.bitmap = game.content.bitmaps.list.get(internal_name);
}


/**
 * @brief Updates the list of texture suggestions, adding a new one or
 * bumping it up.
 *
 * @param n Name of the chosen texture.
 */
void AreaEditor::update_texture_suggestions(const string &n) {
    //First, check if it exists.
    size_t pos = INVALID;
    
    for(size_t s = 0; s < texture_suggestions.size(); s++) {
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
            TextureSuggestion(n)
        );
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        TextureSuggestion s = texture_suggestions[pos];
        texture_suggestions.erase(texture_suggestions.begin() + pos);
        texture_suggestions.insert(texture_suggestions.begin(), s);
    }
    
    if(texture_suggestions.size() > AREA_EDITOR::MAX_TEXTURE_SUGGESTIONS) {
        texture_suggestions[texture_suggestions.size() - 1].destroy();
        texture_suggestions.erase(
            texture_suggestions.begin() + texture_suggestions.size() - 1
        );
    }
}


/**
 * @brief Updates the state and description of the undo button based on
 * the undo history.
 */
void AreaEditor::update_undo_history() {
    while(undo_history.size() > game.options.area_editor_undo_limit) {
        undo_history.pop_back();
    };
}


/**
 * @brief Updates the selection transformation widget's information, since
 * a new vertex was just selected.
 */
void AreaEditor::update_vertex_selection() {
    Point sel_tl(FLT_MAX, FLT_MAX);
    Point sel_br(-FLT_MAX, -FLT_MAX);
    for(Vertex* v : selected_vertexes) {
        update_min_max_coords(sel_tl, sel_br, v2p(v));
    }
    sel_tl.x -= AREA_EDITOR::SELECTION_TW_PADDING;
    sel_tl.y -= AREA_EDITOR::SELECTION_TW_PADDING;
    sel_br.x += AREA_EDITOR::SELECTION_TW_PADDING;
    sel_br.y += AREA_EDITOR::SELECTION_TW_PADDING;
    selection_center = (sel_br + sel_tl) / 2.0f;
    selection_size = sel_br - sel_tl;
    selection_angle = 0.0f;
    selection_orig_center = selection_center;
    selection_orig_size = selection_size;
    selection_orig_angle = selection_angle;
}


/**
 * @brief Constructs a new layout drawing node object.
 *
 * @param ae_ptr Pointer to the area editor instance in charge.
 * @param mouse_click Coordinates of the mouse click.
 */
AreaEditor::LayoutDrawingNode::LayoutDrawingNode(
    const AreaEditor* ae_ptr, const Point &mouse_click
) :
    raw_spot(mouse_click),
    snapped_spot(mouse_click) {
    
    vector<std::pair<Distance, Vertex*> > merge_vertexes =
        get_merge_vertexes(
            mouse_click, game.cur_area_data->vertexes,
            AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
        );
    if(!merge_vertexes.empty()) {
        sort(
            merge_vertexes.begin(), merge_vertexes.end(),
        [] (std::pair<Distance, Vertex*> v1, std::pair<Distance, Vertex*> v2) -> bool {
            return v1.first < v2.first;
        }
        );
        on_vertex = merge_vertexes[0].second;
        on_vertex_idx = game.cur_area_data->find_vertex_idx(on_vertex);
    }
    
    if(on_vertex) {
        snapped_spot.x = on_vertex->x;
        snapped_spot.y = on_vertex->y;
        
    } else {
        on_edge = ae_ptr->get_edge_under_point(mouse_click);
        
        if(on_edge) {
            on_edge_idx = game.cur_area_data->find_edge_idx(on_edge);
            snapped_spot =
                get_closest_point_in_line_seg(
                    Point(on_edge->vertexes[0]->x, on_edge->vertexes[0]->y),
                    Point(on_edge->vertexes[1]->x, on_edge->vertexes[1]->y),
                    mouse_click
                );
                
        } else {
            on_sector = get_sector(mouse_click, &on_sector_idx, false);
            
        }
    }
}


/**
 * @brief Constructs a new layout drawing node object.
 *
 */
AreaEditor::LayoutDrawingNode::LayoutDrawingNode() {
}


/**
 * @brief Constructs a new texture suggestion object.
 *
 * @param n File name of the texture.
 */
AreaEditor::TextureSuggestion::TextureSuggestion(
    const string &n
) :
    bmp(nullptr),
    name(n) {
    
    bmp = game.content.bitmaps.list.get(name, nullptr, false);
}


/**
 * @brief Destroys a texture suggestion.
 */
void AreaEditor::TextureSuggestion::destroy() {
    game.content.bitmaps.list.free(name);
}
