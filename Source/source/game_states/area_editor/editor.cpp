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
#include "../../libs/imgui/imgui_impl_allegro5.h"
#include "../../load.h"
#include "../../utils/allegro_utils.h"
#include "../../utils/general_utils.h"
#include "../../utils/string_utils.h"


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

//Width of the text widget that shows the mouse cursor coordinates.
const float MOUSE_COORDS_TEXT_WIDTH = 150.0f;

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

//Name of the song to play in this state.
const string SONG_NAME = "editors";

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
area_editor::area_editor() :
    backup_timer(game.options.area_editor_backup_interval),
    load_dialog_picker(this) {
    
    enable_flag(path_preview_settings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES);
    path_preview_timer =
    timer(AREA_EDITOR::PATH_PREVIEW_TIMER_DUR, [this] () {
        path_preview_dist = calculate_preview_path();
    });
    
    undo_save_lock_timer =
        timer(
            AREA_EDITOR::UNDO_SAVE_LOCK_DURATION,
    [this] () {undo_save_lock_operation.clear();}
        );
        
    if(game.options.area_editor_backup_interval > 0) {
        backup_timer =
            timer(
                game.options.area_editor_backup_interval,
        [this] () {save_backup();}
            );
    }
    
    zoom_max_level = AREA_EDITOR::ZOOM_MAX_LEVEL;
    zoom_min_level = AREA_EDITOR::ZOOM_MIN_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(&area_editor::circle_sector_cmd, "circle_sector");
    register_cmd(&area_editor::copy_properties_cmd, "copy_properties");
    register_cmd(&area_editor::delete_area_cmd, "delete_area");
    register_cmd(&area_editor::delete_cmd, "delete");
    register_cmd(&area_editor::delete_edge_cmd, "delete_edge");
    register_cmd(&area_editor::delete_tree_shadow_cmd, "delete_tree_shadow");
    register_cmd(&area_editor::duplicate_mobs_cmd, "duplicate_mobs");
    register_cmd(
        &area_editor::grid_interval_decrease_cmd, "grid_interval_decrease"
    );
    register_cmd(
        &area_editor::grid_interval_increase_cmd, "grid_interval_increase"
    );
    register_cmd(&area_editor::layout_drawing_cmd, "layout_drawing");
    register_cmd(&area_editor::load_cmd, "load");
    register_cmd(&area_editor::new_mob_cmd, "new_mob");
    register_cmd(&area_editor::new_path_cmd, "new_path");
    register_cmd(&area_editor::new_tree_shadow_cmd, "new_tree_shadow");
    register_cmd(&area_editor::paste_properties_cmd, "paste_properties");
    register_cmd(&area_editor::paste_texture_cmd, "paste_texture");
    register_cmd(&area_editor::quick_play_cmd, "quick_play");
    register_cmd(&area_editor::quit_cmd, "quit");
    register_cmd(&area_editor::redo_cmd, "redo");
    register_cmd(&area_editor::reference_toggle_cmd, "reference_toggle");
    register_cmd(&area_editor::reload_cmd, "reload");
    register_cmd(&area_editor::save_cmd, "save");
    register_cmd(&area_editor::select_all_cmd, "select_all");
    register_cmd(&area_editor::selection_filter_cmd, "selection_filter");
    register_cmd(&area_editor::snap_mode_cmd, "snap_mode");
    register_cmd(&area_editor::undo_cmd, "undo");
    register_cmd(&area_editor::zoom_and_pos_reset_cmd, "zoom_and_pos_reset");
    register_cmd(&area_editor::zoom_everything_cmd, "zoom_everything");
    register_cmd(&area_editor::zoom_in_cmd, "zoom_in");
    register_cmd(&area_editor::zoom_out_cmd, "zoom_out");
    
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
float area_editor::calculate_day_speed(
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
void area_editor::cancel_circle_sector() {
    clear_circle_sector();
    sub_state = EDITOR_SUB_STATE_NONE;
    set_status();
}


/**
 * @brief Cancels the edge drawing operation and returns to normal.
 */
void area_editor::cancel_layout_drawing() {
    clear_layout_drawing();
    sub_state = EDITOR_SUB_STATE_NONE;
    set_status();
}


/**
 * @brief Cancels the vertex moving operation.
 */
void area_editor::cancel_layout_moving() {
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
void area_editor::change_state(const EDITOR_STATE new_state) {
    clear_selection();
    state = new_state;
    sub_state = EDITOR_SUB_STATE_NONE;
    set_status();
}


/**
 * @brief Clears the data about the circular sector creation.
 */
void area_editor::clear_circle_sector() {
    new_circle_sector_step = 0;
    new_circle_sector_points.clear();
}


/**
 * @brief Clears the currently loaded area data.
 */
void area_editor::clear_current_area() {
    reference_file_name.clear();
    update_reference();
    clear_selection();
    clear_circle_sector();
    clear_layout_drawing();
    clear_layout_moving();
    clear_problems();
    
    clear_area_textures();
    
    for(size_t s = 0; s < game.cur_area_data.tree_shadows.size(); s++) {
        game.textures.free(game.cur_area_data.tree_shadows[s]->file_name);
    }
    
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
    show_cross_section = false;
    show_cross_section_grid = false;
    show_blocking_sectors = false;
    show_path_preview = false;
    path_preview.clear();
    //LARGE_FLOAT means they were never given a previous position.
    path_preview_checkpoints[0] = point(LARGE_FLOAT, LARGE_FLOAT);
    path_preview_checkpoints[1] = point(LARGE_FLOAT, LARGE_FLOAT);
    cross_section_checkpoints[0] = point(LARGE_FLOAT, LARGE_FLOAT);
    cross_section_checkpoints[1] = point(LARGE_FLOAT, LARGE_FLOAT);
    
    clear_texture_suggestions();
    
    game.cur_area_data.clear();
    
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
void area_editor::clear_layout_drawing() {
    drawing_nodes.clear();
    drawing_line_result = DRAWING_LINE_RESULT_OK;
    sector_split_info.useless_split_part_2_checkpoint = INVALID;
}


/**
 * @brief Clears the data about the layout moving.
 */
void area_editor::clear_layout_moving() {
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
void area_editor::clear_problems() {
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
void area_editor::clear_selection() {
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
void area_editor::clear_texture_suggestions() {
    for(size_t s = 0; s < texture_suggestions.size(); s++) {
        texture_suggestions[s].destroy();
    }
    texture_suggestions.clear();
}


/**
 * @brief Clears the undo history, deleting the memory allocated for them.
 */
void area_editor::clear_undo_history() {
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
void area_editor::close_load_dialog() {
    if(!loaded_content_yet && game.cur_area_data.folder_name.empty()) {
        //The user cancelled the area selection picker
        //presented when you enter the area editor. Quit out.
        leave();
    }
}


/**
 * @brief Code to run when the options dialog is closed.
 */
void area_editor::close_options_dialog() {
    save_options();
}


/**
 * @brief Creates a new area to work on.
 *
 * @param requested_area_folder_name Name of the folder of the requested area.
 * @param requested_area_type Type of the requested area.
 */
void area_editor::create_area(
    const string &requested_area_folder_name,
    const AREA_TYPE requested_area_type
) {
    clear_current_area();
    
    //Create a sector for it.
    clear_layout_drawing();
    float r = AREA_EDITOR::COMFY_DIST * 10;
    
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
    vector<string> textures =
        folder_to_vector(TEXTURES_FOLDER_PATH, false);
    size_t texture_to_use = INVALID;
    //First, if there's any "grass" texture, use that.
    for(size_t t = 0; t < textures.size(); t++) {
        string lc_name = str_to_lower(textures[t]);
        if(lc_name.find("grass") != string::npos) {
            texture_to_use = t;
            break;
        }
    }
    //No grass texture? Try one with "dirt".
    if(texture_to_use == INVALID) {
        for(size_t t = 0; t < textures.size(); t++) {
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
        new mob_gen(point(), game.config.leader_order[0], 0, "")
    );
    
    //Set its name and type and whatnot.
    game.cur_area_data.name = requested_area_folder_name;
    game.cur_area_data.folder_name = requested_area_folder_name;
    game.cur_area_data.path =
        get_base_area_folder_path(requested_area_type, true) + "/" +
        requested_area_folder_name;
    game.cur_area_data.type = requested_area_type;
    
    //Finish up.
    clear_undo_history();
    update_undo_history();
    area_exists_on_disk = false;
    update_history(game.cur_area_data.path);
    save_options(); //Save the history in the options.
    
    set_status(
        "Created area \"" + requested_area_folder_name + "\" successfully."
    );
}


/**
 * @brief Creates vertexes based on the edge drawing the user has just made.
 *
 * Drawing nodes that are already on vertexes don't count, but the other ones
 * either create edge splits, or create simple vertexes inside a sector.
 */
void area_editor::create_drawing_vertexes() {
    for(size_t n = 0; n < drawing_nodes.size(); n++) {
        layout_drawing_node* n_ptr = &drawing_nodes[n];
        if(n_ptr->on_vertex) continue;
        vertex* new_vertex = nullptr;
        
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
            new_vertex = game.cur_area_data.new_vertex();
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
void area_editor::create_mob_under_cursor() {
    register_change("object creation");
    sub_state = EDITOR_SUB_STATE_NONE;
    point hotspot = snap_point(game.mouse_cursor.w_pos);
    
    if(last_mob_custom_cat_name.empty()) {
        last_mob_custom_cat_name =
            game.config.pikmin_order[0]->custom_category_name;
        last_mob_type =
            game.config.pikmin_order[0];
    }
    
    game.cur_area_data.mob_generators.push_back(
        new mob_gen(hotspot, last_mob_type)
    );
    
    selected_mobs.insert(game.cur_area_data.mob_generators.back());
    
    set_status("Created object.");
}


/**
 * @brief Creates a new area or loads an existing one, depending on whether the
 * specified area exists or not.
 *
 * @param requested_area_folder_name Name of the folder of the requested area.
 * @param requested_area_type Type of the requested area.
 */
void area_editor::create_or_load_area(
    const string &requested_area_folder_name,
    const AREA_TYPE requested_area_type
) {
    string file_to_check =
        get_base_area_folder_path(requested_area_type, true) +
        "/" + requested_area_folder_name + "/" + AREA_GEOMETRY_FILE_NAME;
    if(al_filename_exists(file_to_check.c_str())) {
        //Area exists, load it.
        load_area(
            requested_area_folder_name, requested_area_type, false, true
        );
    } else {
        //Area doesn't exist, create it.
        create_area(requested_area_folder_name, requested_area_type);
    }
    
    state = EDITOR_STATE_MAIN;
}


/**
 * @brief Deletes the current area.
 */
void area_editor::delete_current_area() {
    bool go_to_area_select = false;
    string final_status_text;
    bool final_status_error = false;
    
    if(!area_exists_on_disk) {
        //If the area doesn't exist, since it was never saved,
        //then there's nothing to delete.
        final_status_text =
            "Deleted area \"" + game.cur_area_data.folder_name +
            "\" successfully.";
        go_to_area_select = true;
        
    } else {
        //Start by deleting the user data folder.
        vector<string> non_important_files;
        non_important_files.push_back(AREA_DATA_BACKUP_FILE_NAME);
        non_important_files.push_back(AREA_GEOMETRY_BACKUP_FILE_NAME);
        non_important_files.push_back("Reference.txt");
        wipe_folder(
            get_base_area_folder_path(game.cur_area_data.type, false) +
            "/" + game.cur_area_data.folder_name,
            non_important_files
        );
        
        //And now, the actual area data.
        non_important_files.clear();
        non_important_files.push_back(AREA_DATA_FILE_NAME);
        non_important_files.push_back(AREA_GEOMETRY_FILE_NAME);
        WIPE_FOLDER_RESULT result =
            wipe_folder(
                game.cur_area_data.path,
                non_important_files
            );
            
        //Let's the inform the user of what happened.
        switch(result) {
        case WIPE_FOLDER_RESULT_OK: {
            final_status_text =
                "Deleted area \"" + game.cur_area_data.folder_name +
                "\" successfully.";
            go_to_area_select = true;
            break;
        } case WIPE_FOLDER_RESULT_NOT_FOUND: {
            final_status_text =
                "Area \"" + game.cur_area_data.folder_name +
                "\" deletion failed; folder not found!";
            final_status_error = true;
            go_to_area_select = false;
            break;
        } case WIPE_FOLDER_RESULT_HAS_IMPORTANT: {
            final_status_text =
                "Deleted area \"" + game.cur_area_data.folder_name +
                "\", but folder still has user files!";
            final_status_error = true;
            go_to_area_select = false;
            break;
        } case WIPE_FOLDER_RESULT_DELETE_ERROR: {
            final_status_text =
                "Area \"" + game.cur_area_data.folder_name +
                "\" deletion failed; error while deleting something! "
                "(Permissions?)";
            final_status_error = true;
            go_to_area_select = false;
            break;
        }
        }
        
    }
    
    if(go_to_area_select) {
        clear_current_area();
        area_exists_on_disk = false;
        open_load_dialog();
    }
    
    set_status(final_status_text, final_status_error);
}


/**
 * @brief Handles the logic part of the main loop of the area editor.
 *
 */
void area_editor::do_logic() {
    editor::do_logic_pre();
    
    process_gui();
    
    cursor_snap_timer.tick(game.delta_t);
    path_preview_timer.tick(game.delta_t);
    quick_preview_timer.tick(game.delta_t);
    new_sector_error_tint_timer.tick(game.delta_t);
    undo_save_lock_timer.tick(game.delta_t);
    
    if(
        !game.cur_area_data.folder_name.empty() &&
        area_exists_on_disk &&
        game.options.area_editor_backup_interval > 0
    ) {
        backup_timer.tick(game.delta_t);
    }
    
    for(auto const &l : game.content.liquids) {
        l.second->anim_instance.tick(game.delta_t);
    }
    
    selection_effect += AREA_EDITOR::SELECTION_EFFECT_SPEED * game.delta_t;
    
    editor::do_logic_post();
}


/**
 * @brief Splits the sector using the user's final drawing.
 */
void area_editor::do_sector_split() {
    //Create the drawing's new edges and connect them.
    vector<edge*> drawing_edges;
    for(size_t n = 0; n < drawing_nodes.size() - 1; n++) {
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
    sector* new_sector =
        create_sector_for_layout_drawing(sector_split_info.working_sector);
        
    //Connect the edges to the sectors.
    unsigned char new_sector_side = (is_new_clockwise ? 1 : 0);
    unsigned char working_sector_side = (is_new_clockwise ? 0 : 1);
    
    for(size_t e = 0; e < new_sector_edges.size(); e++) {
        edge* e_ptr = new_sector_edges[e];
        
        if(!e_ptr->sectors[0] && !e_ptr->sectors[1]) {
            //If it's a new edge, set it up properly.
            game.cur_area_data.connect_edge_to_sector(
                e_ptr, sector_split_info.working_sector, working_sector_side
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
    unordered_set<sector*> affected_sectors;
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
void area_editor::draw_canvas_imgui_callback(
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
void area_editor::emit_triangulation_error_status_bar_message(
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
void area_editor::finish_circle_sector() {
    clear_layout_drawing();
    for(size_t p = 0; p < new_circle_sector_points.size(); p++) {
        layout_drawing_node n;
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
void area_editor::finish_layout_moving() {
    unordered_set<sector*> affected_sectors;
    get_affected_sectors(selected_vertexes, affected_sectors);
    map<vertex*, vertex*> merges;
    map<vertex*, edge*> edges_to_split;
    unordered_set<sector*> merge_affected_sectors;
    
    //Find merge vertexes and edges to split, if any.
    for(auto &v : selected_vertexes) {
        point p(v->x, v->y);
        
        vector<std::pair<dist, vertex*> > merge_vertexes =
            get_merge_vertexes(
                p, game.cur_area_data.vertexes,
                AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
            );
            
        for(size_t mv = 0; mv < merge_vertexes.size(); ) {
            vertex* mv_ptr = merge_vertexes[mv].second;
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
        [] (std::pair<dist, vertex*> v1, std::pair<dist, vertex*> v2) -> bool {
            return v1.first < v2.first;
        }
        );
        
        vertex* merge_v = nullptr;
        if(!merge_vertexes.empty()) {
            merge_v = merge_vertexes[0].second;
        }
        
        if(merge_v) {
            merges[v] = merge_v;
            
        } else {
            edge* e_ptr = nullptr;
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
    
    set<edge*> moved_edges;
    for(size_t e = 0; e < game.cur_area_data.edges.size(); e++) {
        edge* e_ptr = game.cur_area_data.edges[e];
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
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); v++) {
        vertex* v_ptr = game.cur_area_data.vertexes[v];
        point p(v_ptr->x, v_ptr->y);
        
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
        
        edge* e_ptr = nullptr;
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
    vector<edge_intersection> intersections =
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
        vertex* crushed_vertex = nullptr;
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
void area_editor::finish_new_sector_drawing() {
    if(drawing_nodes.size() < 3) {
        cancel_layout_drawing();
        return;
    }
    
    //This is the basic idea: create a new sector using the
    //vertexes provided by the user, as a "child" of an existing sector.
    
    //Get the outer sector, so we can know where to start working in.
    sector* outer_sector = nullptr;
    if(!get_drawing_outer_sector(&outer_sector)) {
        //Something went wrong. Abort.
        cancel_layout_drawing();
        set_status(
            "That sector wouldn't have a defined parent! Try again.",
            true
        );
        return;
    }
    
    vector<edge*> outer_sector_old_edges;
    if(outer_sector) {
        outer_sector_old_edges = outer_sector->edges;
    } else {
        for(size_t e = 0; e < game.cur_area_data.edges.size(); e++) {
            edge* e_ptr = game.cur_area_data.edges[e];
            if(e_ptr->sectors[0] == nullptr || e_ptr->sectors[1] == nullptr) {
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
    for(size_t n = 0; n < drawing_nodes.size(); n++) {
        layout_drawing_node* n_ptr = &drawing_nodes[n];
        size_t prev_node_idx =
            sum_and_wrap((int) n, -1, (int) drawing_nodes.size());
        layout_drawing_node* prev_node = &drawing_nodes[prev_node_idx];
        
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
    
    for(size_t e = 0; e < drawing_edges.size(); e++) {
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
    triangulate_sector(new_sector, nullptr, false);
    
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
void area_editor::forget_prepared_state(area_data* prepared_state) {
    delete prepared_state;
}


/**
 * @brief In the options data file, options pertaining to an editor's history
 * have a prefix. This function returns that prefix.
 *
 * @return The prefix.
 */
string area_editor::get_history_option_prefix() const {
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
void area_editor::get_hovered_layout_element(
    vertex** hovered_vertex, edge** hovered_edge, sector** hovered_sector
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
size_t area_editor::get_mission_required_mob_count() const {
    size_t total_required = 0;
    
    if(game.cur_area_data.mission.goal_all_mobs) {
        for(
            size_t m = 0;
            m < game.cur_area_data.mob_generators.size();
            m++
        ) {
            mob_gen* g = game.cur_area_data.mob_generators[m];
            if(
                game.mission_goals[game.cur_area_data.mission.goal]->
                is_mob_applicable(g->type)
            ) {
                total_required++;
            }
        }
    } else {
        total_required =
            game.cur_area_data.mission.goal_mob_idxs.size();
    }
    
    return total_required;
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string area_editor::get_name() const {
    return "area editor";
}


/**
 * @brief Returns the path to the currently opened folder.
 *
 * @return The path, or an empty string if none.
 */
string area_editor::get_opened_folder_path() const {
    if(!game.cur_area_data.folder_name.empty()) {
        return game.cur_area_data.path;
    } else {
        return "";
    }
}


/**
 * @brief Returns a file path, but shortened in such a way that only the text
 * file's name and brief context about its folder remain.
 *
 * @param p The long path name.
 * @return The name.
 */
string area_editor::get_path_short_name(const string &p) const {
    string match = GAME_DATA_FOLDER_PATH + "/" + AREA_TYPES_FOLDER_NAME;
    size_t start = p.find(match);
    if(start == string::npos) {
        return p;
    }
    
    return p.substr(start + match.size() + 1);
}


/**
 * @brief Evaluates the user's drawing to figure out how the split is
 * going to work.
 *
 * @return The evaluation result.
 */
area_editor::SECTOR_SPLIT_RESULT area_editor::get_sector_split_evaluation() {
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
void area_editor::goto_problem() {
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
            
        change_state(EDITOR_STATE_LAYOUT);
        select_edge(problem_edge_intersection.e1);
        select_edge(problem_edge_intersection.e2);
        center_camera(min_coords, max_coords);
        
        break;
        
    } case EPT_BAD_SECTOR: {

        if(game.cur_area_data.problems.non_simples.empty()) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        change_state(EDITOR_STATE_LAYOUT);
        sector* s_ptr = game.cur_area_data.problems.non_simples.begin()->first;
        select_sector(s_ptr);
        center_camera(s_ptr->bbox[0], s_ptr->bbox[1]);
        
        break;
        
    } case EPT_LONE_EDGE: {

        if(game.cur_area_data.problems.lone_edges.empty()) {
            //Uh, old information. Try searching for problems again.
            find_problems();
            return;
        }
        
        edge* e_ptr = *game.cur_area_data.problems.lone_edges.begin();
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

        point min_coords, max_coords;
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
void area_editor::handle_line_error() {
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
void area_editor::load() {
    editor::load();
    
    //Reset some variables.
    is_alt_pressed = false;
    is_ctrl_pressed = false;
    is_shift_pressed = false;
    last_mob_custom_cat_name.clear();
    last_mob_type = nullptr;
    loaded_content_yet = false;
    selected_shadow = nullptr;
    selection_effect = 0.0;
    selection_homogenized = false;
    show_closest_stop = false;
    show_path_preview = false;
    preview_mode = false;
    quick_preview_timer.stop();
    preview_song.clear();
    state = EDITOR_STATE_MAIN;
    set_status();
    
    //Reset some other states.
    clear_problems();
    clear_selection();
    
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
    
    //Load necessary game content.
    game.content.load_all(CONTENT_TYPE_CUSTOM_PARTICLE_GEN, CONTENT_LOAD_LEVEL_BASIC);
    game.content.load_all(CONTENT_TYPE_STATUS_TYPE, CONTENT_LOAD_LEVEL_BASIC);
    game.content.load_all(CONTENT_TYPE_SPIKE_DAMAGE_TYPE, CONTENT_LOAD_LEVEL_BASIC);
    game.content.load_all(CONTENT_TYPE_LIQUID, CONTENT_LOAD_LEVEL_BASIC);
    game.content.load_all(CONTENT_TYPE_SPRAY_TYPE, CONTENT_LOAD_LEVEL_BASIC);
    game.content.load_all(CONTENT_TYPE_HAZARD, CONTENT_LOAD_LEVEL_BASIC);
    game.content.load_all(CONTENT_TYPE_MOB_TYPE, CONTENT_LOAD_LEVEL_BASIC);
    game.content.load_all(CONTENT_TYPE_WEATHER_CONDITION, CONTENT_LOAD_LEVEL_BASIC);
    
    load_custom_mob_cat_types(true);
    
    game.audio.set_current_song(AREA_EDITOR::SONG_NAME, false);
    
    //Set up stuff to show the player.
    
    if(!quick_play_area_path.empty()) {
        string folder_name;
        AREA_TYPE type;
        get_area_info_from_path(
            quick_play_area_path,
            &folder_name,
            &type
        );
        create_or_load_area(folder_name, type);
        game.cam.set_pos(quick_play_cam_pos);
        game.cam.set_zoom(quick_play_cam_z);
        quick_play_area_path.clear();
        
    } else if(!auto_load_area.empty()) {
        string folder_name;
        AREA_TYPE type;
        get_area_info_from_path(
            auto_load_area,
            &folder_name,
            &type
        );
        create_or_load_area(folder_name, type);
        
    } else {
        open_load_dialog();
        
    }
}


/**
 * @brief Load the area from the disk.
 *
 * @param requested_area_folder_name Name of the folder of the requested area.
 * @param requested_area_type Type of the requested area.
 * @param from_backup If false, load it normally.
 * If true, load from a backup, if any.
 * @param should_update_history If true, this loading process should update
 * the user's folder open history.
 */
void area_editor::load_area(
    const string &requested_area_folder_name,
    const AREA_TYPE requested_area_type,
    bool from_backup, bool should_update_history
) {
    clear_current_area();
    
    game.content.load_area(
        requested_area_folder_name, CONTENT_LOAD_LEVEL_EDITOR,
        requested_area_type, from_backup
    );
    
    //Calculate texture suggestions.
    map<string, size_t> texture_uses_map;
    vector<std::pair<string, size_t> > texture_uses_vector;
    
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); s++) {
        const string &n = game.cur_area_data.sectors[s]->texture_info.file_name;
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
            texture_suggestion(texture_uses_vector[u].first)
        );
    }
    
    load_reference();
    
    update_all_edge_offset_caches();
    
    clear_undo_history();
    update_undo_history();
    area_exists_on_disk = true;
    
    game.cam.zoom = 1.0f;
    game.cam.pos = point();
    
    if(should_update_history) {
        update_history(
            get_base_area_folder_path(requested_area_type, true) + "/" +
            requested_area_folder_name
        );
        save_options(); //Save the history in the options.
    }
    
    set_status(
        "Loaded area \"" + requested_area_folder_name + "\" " +
        (from_backup ? "from a backup " : "") +
        "successfully."
    );
}


/**
 * @brief Loads a backup file.
 */
void area_editor::load_backup() {
    load_area(
        game.cur_area_data.folder_name, game.cur_area_data.type, true, false
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
void area_editor::load_reference() {
    data_node file(
        get_base_area_folder_path(game.cur_area_data.type, false) + "/" +
        game.cur_area_data.folder_name + "/Reference.txt"
    );
    
    if(file.file_was_opened) {
        reference_file_name = file.get_child_by_name("file")->value;
        reference_center = s2p(file.get_child_by_name("center")->value);
        reference_size = s2p(file.get_child_by_name("size")->value);
        reference_alpha =
            s2i(
                file.get_child_by_name(
                    "alpha"
                )->get_value_or_default(i2s(AREA_EDITOR::DEF_REFERENCE_ALPHA))
            );
        show_reference = s2b(file.get_child_by_name("visible")->value);
        
    } else {
        reference_file_name.clear();
        reference_center = point();
        reference_size = point();
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
void area_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/**
 * @brief Callback for when the user picks an area from the picker.
 *
 * @param name Name of the area.
 * @param category Unused.
 * @param is_new Is it a new area, or an existing one?
 */
void area_editor::pick_area(
    const string &name, const string &category, bool is_new
) {
    AREA_TYPE type = AREA_TYPE_SIMPLE;
    if(category == "Mission") {
        type = AREA_TYPE_MISSION;
    }
    create_or_load_area(sanitize_file_name(name), type);
    close_top_dialog();
}


/**
 * @brief Callback for when the user picks a texture from the picker.
 *
 * @param name Name of the texture.
 * @param category Unused.
 * @param is_new Unused.
 */
void area_editor::pick_texture(
    const string &name, const string &category, bool is_new
) {
    sector* s_ptr = nullptr;
    string final_name = name;
    if(selected_sectors.size() == 1 || selection_homogenized) {
        s_ptr = *selected_sectors.begin();
    }
    
    if(!s_ptr) {
        return;
    }
    
    if(final_name == "Browse...") {
        FILE_DIALOG_RESULT result = FILE_DIALOG_RESULT_SUCCESS;
        vector<string> f =
            prompt_file_dialog_locked_to_folder(
                TEXTURES_FOLDER_PATH,
                "Please choose the texture to use for the sector.",
                "*.*",
                ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                ALLEGRO_FILECHOOSER_PICTURES,
                &result, game.display
            );
            
        switch(result) {
        case FILE_DIALOG_RESULT_WRONG_FOLDER: {
            //File doesn't belong to the folder.
            set_status("The chosen image is not in the textures folder!", true);
            return;
        } case FILE_DIALOG_RESULT_CANCELED: {
            //User canceled.
            return;
        } case FILE_DIALOG_RESULT_SUCCESS: {
            final_name = f[0];
            set_status("Picked an image successfully.");
            break;
        }
        }
    }
    
    if(s_ptr->texture_info.file_name == final_name) {
        return;
    }
    
    register_change("sector texture change");
    
    update_texture_suggestions(final_name);
    
    update_sector_texture(s_ptr, final_name);
    
    homogenize_selected_sectors();
}


/**
 * @brief Prepares an area state to be delivered to register_change() later,
 * or forgotten altogether with forget_prepared_state().
 *
 * @return The prepared state.
 */
area_data* area_editor::prepare_state() {
    area_data* new_state = new area_data();
    game.cur_area_data.clone(*new_state);
    return new_state;
}


/**
 * @brief Code to run for the circle sector command.
 *
 * @param input_value Value of the player input for the command.
 */
void area_editor::circle_sector_cmd(float input_value) {
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
        !game.cur_area_data.problems.non_simples.empty() ||
        !game.cur_area_data.problems.lone_edges.empty()
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
void area_editor::copy_properties_cmd(float input_value) {
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
void area_editor::delete_area_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    open_dialog(
        "Delete area?",
        std::bind(&area_editor::process_gui_delete_area_dialog, this)
    );
    dialogs.back()->custom_size = point(400, 150);
}


/**
 * @brief Code to run for the delete command.
 *
 * @param input_value Value of the player input for the command.
 */
void area_editor::delete_cmd(float input_value) {
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
void area_editor::duplicate_mobs_cmd(float input_value) {
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
void area_editor::grid_interval_decrease_cmd(float input_value) {
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
void area_editor::grid_interval_increase_cmd(float input_value) {
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
void area_editor::layout_drawing_cmd(float input_value) {
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
        !game.cur_area_data.problems.non_simples.empty() ||
        !game.cur_area_data.problems.lone_edges.empty()
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
void area_editor::load_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    changes_mgr.ask_if_unsaved(
        load_widget_pos,
        "loading an area", "load",
        std::bind(&area_editor::open_load_dialog, this),
    [this] () { return save_area(false); }
    );
}


/**
 * @brief Code to run for the new mob command.
 *
 * @param input_value Value of the player input for the command.
 */
void area_editor::new_mob_cmd(float input_value) {
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
void area_editor::new_path_cmd(float input_value) {
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
void area_editor::new_tree_shadow_cmd(float input_value) {
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
void area_editor::paste_properties_cmd(float input_value) {
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
void area_editor::paste_texture_cmd(float input_value) {
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
void area_editor::quick_play_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!save_area(false)) return;
    quick_play_area_path = game.cur_area_data.path;
    quick_play_cam_pos = game.cam.pos;
    quick_play_cam_z = game.cam.zoom;
    leave();
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void area_editor::quit_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&area_editor::leave, this),
    [this] () { return save_area(false); }
    );
}


/**
 * @brief Code to run for the redo command.
 *
 * @param input_value Value of the player input for the command.
 */
void area_editor::redo_cmd(float input_value) {
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
void area_editor::reference_toggle_cmd(float input_value) {
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
void area_editor::reload_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!area_exists_on_disk) {
        return;
    }
    changes_mgr.ask_if_unsaved(
        reload_widget_pos,
        "reloading the current area", "reload",
    [this] () {
        load_area(
            game.cur_area_data.folder_name, game.cur_area_data.type,
            false, false
        );
    },
    [this] () { return save_area(false); }
    );
}


/**
 * @brief Code to run for the delete edge command.
 *
 * @param input_value Value of the player input for the command.
 */
void area_editor::delete_edge_cmd(float input_value) {
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
    size_t n_before = game.cur_area_data.edges.size();
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
                (int) (n_before - game.cur_area_data.edges.size()),
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
void area_editor::delete_mob_cmd(float input_value) {
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
    size_t n_before = game.cur_area_data.mob_generators.size();
    
    //Delete!
    delete_mobs(selected_mobs);
    
    //Cleanup.
    clear_selection();
    sub_state = EDITOR_SUB_STATE_NONE;
    
    //Report.
    set_status(
        "Deleted " +
        amount_str(
            (int) (n_before - game.cur_area_data.mob_generators.size()),
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
void area_editor::delete_path_cmd(float input_value) {
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
    size_t n_stops_before = game.cur_area_data.path_stops.size();
    size_t n_links_before = game.cur_area_data.get_nr_path_links();
    
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
            (int) (n_stops_before - game.cur_area_data.path_stops.size()),
            "path stop"
        ) +
        ", " +
        amount_str(
            (int) (n_links_before - game.cur_area_data.get_nr_path_links()),
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
void area_editor::delete_tree_shadow_cmd(float input_value) {
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
            s < game.cur_area_data.tree_shadows.size();
            s++
        ) {
            if(
                game.cur_area_data.tree_shadows[s] ==
                selected_shadow
            ) {
                game.cur_area_data.tree_shadows.erase(
                    game.cur_area_data.tree_shadows.begin() + s
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
void area_editor::save_cmd(float input_value) {
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
void area_editor::select_all_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(sub_state == EDITOR_SUB_STATE_NONE && !selecting && !moving) {
        if(state == EDITOR_STATE_LAYOUT) {
            selected_edges.insert(
                game.cur_area_data.edges.begin(),
                game.cur_area_data.edges.end()
            );
            selected_sectors.insert(
                game.cur_area_data.sectors.begin(),
                game.cur_area_data.sectors.end()
            );
            selected_vertexes.insert(
                game.cur_area_data.vertexes.begin(),
                game.cur_area_data.vertexes.end()
            );
            
        } else if(state == EDITOR_STATE_MOBS) {
            selected_mobs.insert(
                game.cur_area_data.mob_generators.begin(),
                game.cur_area_data.mob_generators.end()
            );
            
        } else if(state == EDITOR_STATE_PATHS) {
            selected_path_stops.insert(
                game.cur_area_data.path_stops.begin(),
                game.cur_area_data.path_stops.end()
            );
        }
        
        update_vertex_selection();
        set_selection_status_text();
        
    } else if(
        sub_state == EDITOR_SUB_STATE_MISSION_MOBS
    ) {
        register_change("mission object requirements change");
        for(
            size_t m = 0; m < game.cur_area_data.mob_generators.size(); m++
        ) {
            mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
            if(
                game.mission_goals[game.cur_area_data.mission.goal]->
                is_mob_applicable(m_ptr->type)
            ) {
                game.cur_area_data.mission.goal_mob_idxs.insert(m);
            }
        }
    }
}


/**
 * @brief Code to run for the selection filter command.
 *
 * @param input_value Value of the player input for the command.
 */
void area_editor::selection_filter_cmd(float input_value) {
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
void area_editor::snap_mode_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!is_shift_pressed) {
        game.options.area_editor_snap_mode =
            (area_editor::SNAP_MODE)
            sum_and_wrap(game.options.area_editor_snap_mode, 1, N_SNAP_MODES);
    } else {
        game.options.area_editor_snap_mode =
            (area_editor::SNAP_MODE)
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
void area_editor::undo_cmd(float input_value) {
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
void area_editor::zoom_and_pos_reset_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(game.cam.target_zoom == 1.0f) {
        game.cam.target_pos = point();
    } else {
        game.cam.target_zoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom everything command.
 *
 * @param input_value Value of the player input for the command.
 */
void area_editor::zoom_everything_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    bool got_something = false;
    point min_coords, max_coords;
    
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); v++) {
        vertex* v_ptr = game.cur_area_data.vertexes[v];
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
    
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); m++) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
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
    
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); s++) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
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
void area_editor::zoom_in_cmd(float input_value) {
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
void area_editor::zoom_out_cmd(float input_value) {
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
void area_editor::recreate_drawing_nodes() {
    for(size_t n = 0; n < drawing_nodes.size(); n++) {
        drawing_nodes[n] = layout_drawing_node(this, drawing_nodes[n].raw_spot);
    }
}


/**
 * @brief Redoes the latest undone change to the area using the undo history,
 * if available.
 */
void area_editor::redo() {
    if(redo_history.empty()) {
        set_status("Nothing to redo.");
        return;
    }
    
    //Let's first save the state of things right now so we can feed it into
    //the undo history afterwards.
    area_data* new_state = new area_data();
    game.cur_area_data.clone(*new_state);
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
void area_editor::register_change(
    const string &operation_name, area_data* pre_prepared_state
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
    
    area_data* new_state = pre_prepared_state;
    if(!pre_prepared_state) {
        new_state = new area_data();
        game.cur_area_data.clone(*new_state);
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
 * @brief Removes the current area thumbnail, if any.
 */
void area_editor::remove_thumbnail() {
    game.cur_area_data.thumbnail = nullptr;
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void area_editor::reset_cam_xy() {
    game.cam.target_pos = point();
}


/**
 * @brief Resets the camera's zoom.
 */
void area_editor::reset_cam_zoom() {
    zoom_with_cursor(1.0f);
}


/**
 * @brief Returns to a previously prepared area state.
 *
 * @param prepared_state Prepared state to return to.
 */
void area_editor::rollback_to_prepared_state(area_data* prepared_state) {
    prepared_state->clone(game.cur_area_data);
}


/**
 * @brief Saves the area onto the disk.
 *
 * @param to_backup If false, save normally.
 * If true, save to an auto-backup file.
 * @return Whether it succeded.
 */
bool area_editor::save_area(bool to_backup) {

    //Before we start, let's get rid of unused sectors.
    bool deleted_sectors = false;
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ) {
        if(game.cur_area_data.sectors[s]->edges.empty()) {
            game.cur_area_data.remove_sector(s);
            deleted_sectors = true;
        } else {
            s++;
        }
    }
    if(deleted_sectors && !selected_sectors.empty()) {
        clear_selection();
    }
    
    //And some other cleanup.
    if(game.cur_area_data.song_name == NONE_OPTION) {
        game.cur_area_data.song_name.clear();
    }
    if(game.cur_area_data.weather_name == NONE_OPTION) {
        game.cur_area_data.weather_name.clear();
    }
    game.cur_area_data.engine_version = get_engine_version_string();
    
    //First, the geometry file.
    data_node geometry_file("", "");
    
    //Vertexes.
    data_node* vertexes_node = new data_node("vertexes", "");
    geometry_file.add(vertexes_node);
    
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); v++) {
        vertex* v_ptr = game.cur_area_data.vertexes[v];
        data_node* vertex_node =
            new data_node("v", f2s(v_ptr->x) + " " + f2s(v_ptr->y));
        vertexes_node->add(vertex_node);
    }
    
    //Edges.
    data_node* edges_node = new data_node("edges", "");
    geometry_file.add(edges_node);
    
    for(size_t e = 0; e < game.cur_area_data.edges.size(); e++) {
        edge* e_ptr = game.cur_area_data.edges[e];
        data_node* edge_node = new data_node("e", "");
        edges_node->add(edge_node);
        string s_str;
        for(size_t s = 0; s < 2; s++) {
            if(e_ptr->sector_idxs[s] == INVALID) s_str += "-1";
            else s_str += i2s(e_ptr->sector_idxs[s]);
            s_str += " ";
        }
        s_str.erase(s_str.size() - 1);
        edge_node->add(new data_node("s", s_str));
        edge_node->add(
            new data_node(
                "v",
                i2s(e_ptr->vertex_idxs[0]) + " " + i2s(e_ptr->vertex_idxs[1])
            )
        );
        
        if(e_ptr->wall_shadow_length != LARGE_FLOAT) {
            edge_node->add(
                new data_node("shadow_length", f2s(e_ptr->wall_shadow_length))
            );
        }
        
        if(e_ptr->wall_shadow_color != GEOMETRY::SHADOW_DEF_COLOR) {
            edge_node->add(
                new data_node("shadow_color", c2s(e_ptr->wall_shadow_color))
            );
        }
        
        if(e_ptr->ledge_smoothing_length != 0.0f) {
            edge_node->add(
                new data_node(
                    "smoothing_length",
                    f2s(e_ptr->ledge_smoothing_length)
                )
            );
        }
        
        if(e_ptr->ledge_smoothing_color != GEOMETRY::SMOOTHING_DEF_COLOR) {
            edge_node->add(
                new data_node(
                    "smoothing_color",
                    c2s(e_ptr->ledge_smoothing_color)
                )
            );
        }
    }
    
    //Sectors.
    data_node* sectors_node = new data_node("sectors", "");
    geometry_file.add(sectors_node);
    
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); s++) {
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
        if(s_ptr->brightness != GEOMETRY::DEF_SECTOR_BRIGHTNESS) {
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
    
    for(size_t m = 0; m < game.cur_area_data.mob_generators.size(); m++) {
        mob_gen* m_ptr = game.cur_area_data.mob_generators[m];
        string cat_name = "(Unknown)";
        if(m_ptr->type && m_ptr->type->category) {
            cat_name = m_ptr->type->category->name;
        }
        data_node* mob_node = new data_node(cat_name, "");
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
        for(size_t l = 0; l < m_ptr->link_idxs.size(); l++) {
            if(l > 0) links_str += " ";
            links_str += i2s(m_ptr->link_idxs[l]);
        }
        
        if(!links_str.empty()) {
            mob_node->add(
                new data_node("links", links_str)
            );
        }
        
        if(m_ptr->stored_inside != INVALID) {
            mob_node->add(
                new data_node("stored_inside", i2s(m_ptr->stored_inside))
            );
        }
    }
    
    //Paths.
    data_node* path_stops_node = new data_node("path_stops", "");
    geometry_file.add(path_stops_node);
    
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); s++) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        data_node* path_stop_node = new data_node("s", "");
        path_stops_node->add(path_stop_node);
        
        path_stop_node->add(
            new data_node("pos", f2s(s_ptr->pos.x) + " " + f2s(s_ptr->pos.y))
        );
        if(s_ptr->radius != PATHS::MIN_STOP_RADIUS) {
            path_stop_node->add(
                new data_node("radius", f2s(s_ptr->radius))
            );
        }
        if(s_ptr->flags != 0) {
            path_stop_node->add(
                new data_node("flags", i2s(s_ptr->flags))
            );
        }
        if(!s_ptr->label.empty()) {
            path_stop_node->add(
                new data_node("label", s_ptr->label)
            );
        }
        
        data_node* links_node = new data_node("links", "");
        path_stop_node->add(links_node);
        
        for(size_t l = 0; l < s_ptr->links.size(); l++) {
            path_link* l_ptr = s_ptr->links[l];
            string link_data = i2s(l_ptr->end_idx);
            if(l_ptr->type != PATH_LINK_TYPE_NORMAL) {
                link_data += " " + i2s(l_ptr->type);
            }
            data_node* link_node = new data_node("nr", link_data);
            links_node->add(link_node);
        }
        
    }
    
    //Tree shadows.
    data_node* shadows_node = new data_node("tree_shadows", "");
    geometry_file.add(shadows_node);
    
    for(size_t s = 0; s < game.cur_area_data.tree_shadows.size(); s++) {
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
    
    //Content metadata.
    game.cur_area_data.save_to_data_node(&data_file);
    
    data_file.add(
        new data_node("subtitle", game.cur_area_data.subtitle)
    );
    data_file.add(
        new data_node(
            "difficulty",
            i2s(game.cur_area_data.difficulty)
        )
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
        new data_node("song", game.cur_area_data.song_name)
    );
    data_file.add(
        new data_node("weather", game.cur_area_data.weather_name)
    );
    data_file.add(
        new data_node("day_time_start", i2s(game.cur_area_data.day_time_start))
    );
    data_file.add(
        new data_node("day_time_speed", i2s(game.cur_area_data.day_time_speed))
    );
    data_file.add(
        new data_node("spray_amounts", game.cur_area_data.spray_amounts)
    );
    
    if(game.cur_area_data.type == AREA_TYPE_MISSION) {
        if(game.cur_area_data.mission.goal != MISSION_GOAL_END_MANUALLY) {
            data_file.add(
                new data_node(
                    "mission_goal",
                    game.mission_goals[game.cur_area_data.mission.goal]->
                    get_name()
                )
            );
        }
        if(
            game.cur_area_data.mission.goal == MISSION_GOAL_TIMED_SURVIVAL ||
            game.cur_area_data.mission.goal == MISSION_GOAL_GROW_PIKMIN
        ) {
            data_file.add(
                new data_node(
                    "mission_goal_amount",
                    i2s(game.cur_area_data.mission.goal_amount)
                )
            );
        }
        if(
            game.cur_area_data.mission.goal == MISSION_GOAL_COLLECT_TREASURE ||
            game.cur_area_data.mission.goal == MISSION_GOAL_BATTLE_ENEMIES ||
            game.cur_area_data.mission.goal == MISSION_GOAL_GET_TO_EXIT
        ) {
            data_file.add(
                new data_node(
                    "mission_goal_all_mobs",
                    b2s(game.cur_area_data.mission.goal_all_mobs)
                )
            );
            string mission_mob_idxs;
            for(size_t i : game.cur_area_data.mission.goal_mob_idxs) {
                if(!mission_mob_idxs.empty()) mission_mob_idxs += ";";
                mission_mob_idxs += i2s(i);
            }
            if(!mission_mob_idxs.empty()) {
                data_file.add(
                    new data_node(
                        "mission_required_mobs",
                        mission_mob_idxs
                    )
                );
            }
        }
        if(game.cur_area_data.mission.goal == MISSION_GOAL_GET_TO_EXIT) {
            data_file.add(
                new data_node(
                    "mission_goal_exit_center",
                    p2s(game.cur_area_data.mission.goal_exit_center)
                )
            );
            data_file.add(
                new data_node(
                    "mission_goal_exit_size",
                    p2s(game.cur_area_data.mission.goal_exit_size)
                )
            );
        }
        if(game.cur_area_data.mission.fail_conditions > 0) {
            data_file.add(
                new data_node(
                    "mission_fail_conditions",
                    i2s(game.cur_area_data.mission.fail_conditions)
                )
            );
        }
        if(
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_idx_bitmask(MISSION_FAIL_COND_TOO_FEW_PIKMIN)
            )
        ) {
            data_file.add(
                new data_node(
                    "mission_fail_too_few_pik_amount",
                    i2s(game.cur_area_data.mission.fail_too_few_pik_amount)
                )
            );
        }
        if(
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_idx_bitmask(MISSION_FAIL_COND_TOO_MANY_PIKMIN)
            )
        ) {
            data_file.add(
                new data_node(
                    "mission_fail_too_many_pik_amount",
                    i2s(game.cur_area_data.mission.fail_too_many_pik_amount)
                )
            );
        }
        if(
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_idx_bitmask(MISSION_FAIL_COND_LOSE_PIKMIN)
            )
        ) {
            data_file.add(
                new data_node(
                    "mission_fail_pik_killed",
                    i2s(game.cur_area_data.mission.fail_pik_killed)
                )
            );
        }
        if(
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_idx_bitmask(MISSION_FAIL_COND_LOSE_LEADERS)
            )
        ) {
            data_file.add(
                new data_node(
                    "mission_fail_leaders_kod",
                    i2s(game.cur_area_data.mission.fail_leaders_kod)
                )
            );
        }
        if(
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_idx_bitmask(MISSION_FAIL_COND_KILL_ENEMIES)
            )
        ) {
            data_file.add(
                new data_node(
                    "mission_fail_enemies_killed",
                    i2s(game.cur_area_data.mission.fail_enemies_killed)
                )
            );
        }
        if(
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
            )
        ) {
            data_file.add(
                new data_node(
                    "mission_fail_time_limit",
                    i2s(game.cur_area_data.mission.fail_time_limit)
                )
            );
        }
        if(game.cur_area_data.mission.fail_hud_primary_cond != INVALID) {
            data_file.add(
                new data_node(
                    "mission_fail_hud_primary_cond",
                    i2s(game.cur_area_data.mission.fail_hud_primary_cond)
                )
            );
        }
        if(game.cur_area_data.mission.fail_hud_secondary_cond != INVALID) {
            data_file.add(
                new data_node(
                    "mission_fail_hud_secondary_cond",
                    i2s(game.cur_area_data.mission.fail_hud_secondary_cond)
                )
            );
        }
        data_file.add(
            new data_node(
                "mission_grading_mode",
                i2s(game.cur_area_data.mission.grading_mode)
            )
        );
        if(game.cur_area_data.mission.grading_mode == MISSION_GRADING_MODE_POINTS) {
            if(game.cur_area_data.mission.points_per_pikmin_born != 0) {
                data_file.add(
                    new data_node(
                        "mission_points_per_pikmin_born",
                        i2s(game.cur_area_data.mission.points_per_pikmin_born)
                    )
                );
            }
            if(game.cur_area_data.mission.points_per_pikmin_death != 0) {
                data_file.add(
                    new data_node(
                        "mission_points_per_pikmin_death",
                        i2s(game.cur_area_data.mission.points_per_pikmin_death)
                    )
                );
            }
            if(game.cur_area_data.mission.points_per_sec_left != 0) {
                data_file.add(
                    new data_node(
                        "mission_points_per_sec_left",
                        i2s(game.cur_area_data.mission.points_per_sec_left)
                    )
                );
            }
            if(game.cur_area_data.mission.points_per_sec_passed != 0) {
                data_file.add(
                    new data_node(
                        "mission_points_per_sec_passed",
                        i2s(game.cur_area_data.mission.points_per_sec_passed)
                    )
                );
            }
            if(game.cur_area_data.mission.points_per_treasure_point != 0) {
                data_file.add(
                    new data_node(
                        "mission_points_per_treasure_point",
                        i2s(
                            game.cur_area_data.mission.points_per_treasure_point
                        )
                    )
                );
            }
            if(game.cur_area_data.mission.points_per_enemy_point != 0) {
                data_file.add(
                    new data_node(
                        "mission_points_per_enemy_point",
                        i2s(game.cur_area_data.mission.points_per_enemy_point)
                    )
                );
            }
            if(game.cur_area_data.mission.point_loss_data > 0) {
                data_file.add(
                    new data_node(
                        "mission_point_loss_data",
                        i2s(game.cur_area_data.mission.point_loss_data)
                    )
                );
            }
            if(game.cur_area_data.mission.point_hud_data != 255) {
                data_file.add(
                    new data_node(
                        "mission_point_hud_data",
                        i2s(game.cur_area_data.mission.point_hud_data)
                    )
                );
            }
            if(game.cur_area_data.mission.starting_points != 0) {
                data_file.add(
                    new data_node(
                        "mission_starting_points",
                        i2s(game.cur_area_data.mission.starting_points)
                    )
                );
            }
            data_file.add(
                new data_node(
                    "mission_bronze_req",
                    i2s(game.cur_area_data.mission.bronze_req)
                )
            );
            data_file.add(
                new data_node(
                    "mission_silver_req",
                    i2s(game.cur_area_data.mission.silver_req)
                )
            );
            data_file.add(
                new data_node(
                    "mission_gold_req",
                    i2s(game.cur_area_data.mission.gold_req)
                )
            );
            data_file.add(
                new data_node(
                    "mission_platinum_req",
                    i2s(game.cur_area_data.mission.platinum_req)
                )
            );
        }
    }
    
    //Save the thumbnail, or delete it if none.
    //al_save_bitmap is slow, so let's only write the thumbnail file
    //if there have been changes.
    if(
        (thumbnail_needs_saving && !to_backup) ||
        (thumbnail_backup_needs_saving && to_backup)
    ) {
        string thumb_path =
            get_base_area_folder_path(game.cur_area_data.type, !to_backup) +
            "/" + game.cur_area_data.folder_name +
            (to_backup ? "/Thumbnail_backup.png" : "/Thumbnail.png");
        if(game.cur_area_data.thumbnail) {
            al_save_bitmap(
                thumb_path.c_str(), game.cur_area_data.thumbnail.get()
            );
        } else {
            al_remove_filename(thumb_path.c_str());
        }
        (to_backup ? thumbnail_backup_needs_saving : thumbnail_needs_saving) =
            false;
    }
    
    
    //Finally, save.
    string base_folder;
    string geometry_file_name;
    string data_file_name;
    if(to_backup) {
        base_folder =
            get_base_area_folder_path(game.cur_area_data.type, false) +
            "/" + game.cur_area_data.folder_name;
        geometry_file_name = base_folder + "/" + AREA_GEOMETRY_BACKUP_FILE_NAME;
        data_file_name = base_folder + "/" + AREA_DATA_BACKUP_FILE_NAME;
    } else {
        base_folder = game.cur_area_data.path;
        geometry_file_name = base_folder + "/" + AREA_GEOMETRY_FILE_NAME;
        data_file_name = base_folder + "/" + AREA_DATA_FILE_NAME;
    }
    bool geo_save_ok = geometry_file.save_file(geometry_file_name);
    bool data_save_ok = data_file.save_file(data_file_name);
    
    if(!geo_save_ok || !data_save_ok) {
        show_message_box(
            nullptr, "Save failed!",
            "Could not save the area!",
            (
                "An error occured while saving the area to the folder \"" +
                base_folder + "\". "
                "Make sure that the folder exists and it is not read-only, "
                "and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        
        set_status("Could not save the area!", true);
        
    }
    
    backup_timer.start(game.options.area_editor_backup_interval);
    area_exists_on_disk = true;
    
    save_reference();
    
    bool save_successful = geo_save_ok && data_save_ok;
    
    if(save_successful && !to_backup) {
        //If this was a normal save, save the backup too, so that the
        //maker doesn't have an outdated backup.
        save_backup();
        
        changes_mgr.mark_as_saved();
        set_status("Saved area successfully.");
    }
    
    return save_successful;
}


/**
 * @brief Saves the area onto a backup file.
 */
void area_editor::save_backup() {

    //Restart the timer.
    backup_timer.start(game.options.area_editor_backup_interval);
    
    //First, check if the folder even exists.
    //If not, chances are this is a new area.
    //We should probably create a backup anyway, but if the area is
    //just for testing, the backups are pointless.
    //Plus, creating the backup will create the area's folder on the disk,
    //which will basically mean the area exists, even though this might not be
    //what the user wants, since they haven't saved proper yet.
    
    ALLEGRO_FS_ENTRY* folder_fs_entry =
        al_create_fs_entry(
            (
                get_base_area_folder_path(game.cur_area_data.type, true) +
                "/" + game.cur_area_data.folder_name
            ).c_str()
        );
    bool folder_exists = al_open_directory(folder_fs_entry);
    al_close_directory(folder_fs_entry);
    al_destroy_fs_entry(folder_fs_entry);
    
    if(!folder_exists) return;
    
    save_area(true);
}


/**
 * @brief Saves the reference data to disk, in the area's reference config file.
 */
void area_editor::save_reference() {
    string file_name =
        get_base_area_folder_path(game.cur_area_data.type, false) +
        "/" + game.cur_area_data.folder_name + "/Reference.txt";
        
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
            p2s(reference_center)
        )
    );
    reference_file.add(
        new data_node(
            "size",
            p2s(reference_size)
        )
    );
    reference_file.add(
        new data_node(
            "alpha",
            i2s(reference_alpha)
        )
    );
    reference_file.add(
        new data_node(
            "visible",
            b2s(show_reference)
        )
    );
    
    reference_file.save_file(file_name);
}


/**
 * @brief Selects an edge and its vertexes.
 *
 * @param e Edge to select.
 */
void area_editor::select_edge(edge* e) {
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
void area_editor::select_path_stops_with_label(const string &label) {
    clear_selection();
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); s++) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
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
void area_editor::select_sector(sector* s) {
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
void area_editor::select_tree_shadow(tree_shadow* s_ptr) {
    selected_shadow = s_ptr;
    set_selection_status_text();
}


/**
 * @brief Selects a vertex.
 *
 * @param v Vertex to select.
 */
void area_editor::select_vertex(vertex* v) {
    selected_vertexes.insert(v);
    set_selection_status_text();
    update_vertex_selection();
}


/**
 * @brief Sets the vector of points that make up a new circle sector.
 */
void area_editor::set_new_circle_sector_points() {
    float anchor_angle =
        get_angle(new_circle_sector_center, new_circle_sector_anchor);
    float cursor_angle =
        get_angle(new_circle_sector_center, game.mouse_cursor.w_pos);
    float radius =
        dist(
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
            point(
                new_circle_sector_center.x +
                radius * cos(anchor_angle + delta_a),
                new_circle_sector_center.y +
                radius * sin(anchor_angle + delta_a)
            )
        );
    }
    
    new_circle_sector_valid_edges.clear();
    for(size_t p = 0; p < n_points; p++) {
        point next = get_next_in_vector(new_circle_sector_points, p);
        bool valid = true;
        
        for(size_t e = 0; e < game.cur_area_data.edges.size(); e++) {
            edge* e_ptr = game.cur_area_data.edges[e];
            
            if(
                line_segs_intersect(
                    point(
                        e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                    ),
                    point(
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
void area_editor::set_selection_status_text() {
    set_status();
    
    if(!game.cur_area_data.problems.non_simples.empty()) {
        emit_triangulation_error_status_bar_message(
            game.cur_area_data.problems.non_simples.begin()->second
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
void area_editor::set_state_from_undo_or_redo_history(area_data* state) {
    state->clone(game.cur_area_data);
    
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
void area_editor::setup_sector_split() {
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
        for(size_t e = 0; e < game.cur_area_data.edges.size(); e++) {
            edge* e_ptr = game.cur_area_data.edges[e];
            if(e_ptr->sectors[0] == nullptr || e_ptr->sectors[1] == nullptr) {
                sector_split_info.working_sector_old_edges.push_back(e_ptr);
            }
        }
    }
    
    //First, create vertexes wherever necessary.
    create_drawing_vertexes();
}


/**
 * @brief Procedure to start moving the selected mobs.
 */
void area_editor::start_mob_move() {
    register_change("object movement");
    
    move_closest_mob = nullptr;
    dist move_closest_mob_dist;
    for(auto const &m : selected_mobs) {
        pre_move_mob_coords[m] = m->pos;
        
        dist d(game.mouse_cursor.w_pos, m->pos);
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
void area_editor::start_path_stop_move() {
    register_change("path stop movement");
    
    move_closest_stop = nullptr;
    dist move_closest_stop_dist;
    for(
        auto s = selected_path_stops.begin();
        s != selected_path_stops.end(); ++s
    ) {
        pre_move_stop_coords[*s] = (*s)->pos;
        
        dist d(game.mouse_cursor.w_pos, (*s)->pos);
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
void area_editor::start_vertex_move() {
    pre_move_area_data = prepare_state();
    
    move_closest_vertex = nullptr;
    dist move_closest_vertex_dist;
    for(auto const &v : selected_vertexes) {
        point p(v->x, v->y);
        pre_move_vertex_coords[v] = p;
        
        dist d(game.mouse_cursor.w_pos, p);
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
void area_editor::traverse_sector_for_split(
    const sector* s_ptr, vertex* begin, const vertex* checkpoint,
    vector<edge*>* edges, vector<vertex*>* vertexes,
    bool* working_sector_left
) {
    edge* first_e_ptr = nullptr;
    unsigned char first_edge_visits = 0;
    
    for(unsigned char s = 0; s < 2; s++) {
        vertex* v_ptr = begin;
        vertex* prev_v_ptr = nullptr;
        float prev_e_angle = TAU / 2.0f;
        
        while(true) {
            edge* next_e_ptr = nullptr;
            float next_e_angle = 0.0f;
            vertex* next_v_ptr = nullptr;
            
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
void area_editor::undo() {
    if(undo_history.empty()) {
        set_status("Nothing to undo.");
        return;
    }
    
    //Let's first save the state of things right now so we can feed it into
    //the redo history afterwards.
    area_data* new_state = new area_data();
    game.cur_area_data.clone(*new_state);
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
void area_editor::undo_layout_drawing_node() {
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
void area_editor::unload() {
    editor::unload();
    
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
    
    game.content.unload_all(CONTENT_TYPE_WEATHER_CONDITION);
    game.content.unload_all(CONTENT_TYPE_MOB_TYPE);
    game.content.unload_all(CONTENT_TYPE_HAZARD);
    game.content.unload_all(CONTENT_TYPE_SPRAY_TYPE);
    game.content.unload_all(CONTENT_TYPE_LIQUID);
    game.content.unload_all(CONTENT_TYPE_SPIKE_DAMAGE_TYPE);
    game.content.unload_all(CONTENT_TYPE_STATUS_TYPE);
    game.content.unload_all(CONTENT_TYPE_CUSTOM_PARTICLE_GEN);
}


/**
 * @brief Updates all edge offset caches relevant to the area editor.
 */
void area_editor::update_all_edge_offset_caches() {
    game.wall_smoothing_effect_caches.clear();
    game.wall_smoothing_effect_caches.insert(
        game.wall_smoothing_effect_caches.begin(),
        game.cur_area_data.edges.size(),
        edge_offset_cache()
    );
    update_offset_effect_caches(
        game.wall_smoothing_effect_caches,
        unordered_set<vertex*>(
            game.cur_area_data.vertexes.begin(),
            game.cur_area_data.vertexes.end()
        ),
        does_edge_have_ledge_smoothing,
        get_ledge_smoothing_length,
        get_ledge_smoothing_color
    );
    game.wall_shadow_effect_caches.clear();
    game.wall_shadow_effect_caches.insert(
        game.wall_shadow_effect_caches.begin(),
        game.cur_area_data.edges.size(),
        edge_offset_cache()
    );
    update_offset_effect_caches(
        game.wall_shadow_effect_caches,
        unordered_set<vertex*>(
            game.cur_area_data.vertexes.begin(),
            game.cur_area_data.vertexes.end()
        ),
        does_edge_have_wall_shadow,
        get_wall_shadow_length,
        get_wall_shadow_color
    );
}


/**
 * @brief Updates the status text according to what's going on in the current
 * sector drawing.
 *
 */
void area_editor::update_layout_drawing_status_text() {
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
void area_editor::update_reference() {
    if(reference_bitmap && reference_bitmap != game.bmp_error) {
        al_destroy_bitmap(reference_bitmap);
    }
    reference_bitmap = nullptr;
    
    if(!reference_file_name.empty()) {
        reference_bitmap =
            load_bmp(reference_file_name, nullptr, false, true, true, true);
            
        if(
            reference_size.x == 0 ||
            reference_size.y == 0
        ) {
            //Let's assume this is a new reference. Reset sizes and alpha.
            reference_size.x = al_get_bitmap_width(reference_bitmap);
            reference_size.y = al_get_bitmap_height(reference_bitmap);
            reference_alpha = AREA_EDITOR::DEF_REFERENCE_ALPHA;
        }
    } else {
        reference_center = point();
        reference_size = point();
    }
}


/**
 * @brief Updates a sector's texture.
 *
 * @param s_ptr Sector to update.
 * @param file_name New file name of the texture.
 */
void area_editor::update_sector_texture(
    sector* s_ptr, const string &file_name
) {
    game.textures.free(s_ptr->texture_info.file_name);
    s_ptr->texture_info.file_name = file_name;
    s_ptr->texture_info.bitmap = game.textures.get(file_name);
}


/**
 * @brief Updates the list of texture suggestions, adding a new one or
 * bumping it up.
 *
 * @param n Name of the chosen texture.
 */
void area_editor::update_texture_suggestions(const string &n) {
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
            texture_suggestion(n)
        );
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        texture_suggestion s = texture_suggestions[pos];
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
void area_editor::update_undo_history() {
    while(undo_history.size() > game.options.area_editor_undo_limit) {
        undo_history.pop_back();
    };
}


/**
 * @brief Updates the selection transformation widget's information, since
 * a new vertex was just selected.
 */
void area_editor::update_vertex_selection() {
    point sel_tl(FLT_MAX, FLT_MAX);
    point sel_br(-FLT_MAX, -FLT_MAX);
    for(vertex* v : selected_vertexes) {
        sel_tl.x = std::min(sel_tl.x, v->x);
        sel_tl.y = std::min(sel_tl.y, v->y);
        sel_br.x = std::max(sel_br.x, v->x);
        sel_br.y = std::max(sel_br.y, v->y);
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
area_editor::layout_drawing_node::layout_drawing_node(
    const area_editor* ae_ptr, const point &mouse_click
) :
    raw_spot(mouse_click),
    snapped_spot(mouse_click) {
    
    vector<std::pair<dist, vertex*> > merge_vertexes =
        get_merge_vertexes(
            mouse_click, game.cur_area_data.vertexes,
            AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
        );
    if(!merge_vertexes.empty()) {
        sort(
            merge_vertexes.begin(), merge_vertexes.end(),
        [] (std::pair<dist, vertex*> v1, std::pair<dist, vertex*> v2) -> bool {
            return v1.first < v2.first;
        }
        );
        on_vertex = merge_vertexes[0].second;
        on_vertex_idx = game.cur_area_data.find_vertex_idx(on_vertex);
    }
    
    if(on_vertex) {
        snapped_spot.x = on_vertex->x;
        snapped_spot.y = on_vertex->y;
        
    } else {
        on_edge = ae_ptr->get_edge_under_point(mouse_click);
        
        if(on_edge) {
            on_edge_idx = game.cur_area_data.find_edge_idx(on_edge);
            snapped_spot =
                get_closest_point_in_line_seg(
                    point(on_edge->vertexes[0]->x, on_edge->vertexes[0]->y),
                    point(on_edge->vertexes[1]->x, on_edge->vertexes[1]->y),
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
area_editor::layout_drawing_node::layout_drawing_node() {
}


/**
 * @brief Constructs a new texture suggestion object.
 *
 * @param n File name of the texture.
 */
area_editor::texture_suggestion::texture_suggestion(
    const string &n
) :
    bmp(nullptr),
    name(n) {
    
    bmp = game.textures.get(name, nullptr, false);
}


/**
 * @brief Destroys a texture suggestion.
 */
void area_editor::texture_suggestion::destroy() {
    game.textures.free(name);
}
