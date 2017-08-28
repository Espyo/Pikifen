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

#include "area_editor.h"
#include "../functions.h"
#include "../load.h"
#include "../vars.h"

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
 * Initializes area editor class stuff.
 */
area_editor::area_editor() :
    state(EDITOR_STATE_MAIN),
    backup_timer(editor_backup_interval),
    double_click_time(0),
    path_preview_timer(0),
    show_reference(false) {
    
}


/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor mode.
 */
void area_editor::change_to_right_frame() {
    //TODO
    sub_state = EDITOR_SUB_STATE_NONE;
    
    hide_all_frames();
    
    if(state == EDITOR_STATE_MAIN) {
        frm_main->show();
    } else if(state == EDITOR_STATE_LAYOUT) {
        frm_layout->show();
    } else if(state == EDITOR_STATE_ASB) {
        frm_asb->show();
    } else if(state == EDITOR_STATE_TEXTURE) {
        frm_texture->show();
    } else if(state == EDITOR_STATE_OBJECTS) {
        frm_objects->show();
    } else if(state == EDITOR_STATE_PATHS) {
        frm_paths->show();
    } else if(state == EDITOR_STATE_DETAILS) {
        frm_details->show();
    } else if(state == EDITOR_STATE_REVIEW) {
        frm_review->show();
    } else if(state == EDITOR_STATE_TOOLS) {
        frm_tools->show();
    } else if(state == EDITOR_STATE_OPTIONS) {
        frm_options->show();
    }
}


/* ----------------------------------------------------------------------------
 * Clears the currently loaded area data.
 */
void area_editor::clear_current_area() {
    //TODO
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

    update_transformations();
    
    if(double_click_time > 0) {
        double_click_time -= delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
    
    path_preview_timer.tick(delta_t);
    
    if(!cur_area_data.name.empty() && editor_backup_interval > 0) {
        backup_timer.tick(delta_t);
    }
    
    fade_mgr.tick(delta_t);
    
}


/* ----------------------------------------------------------------------------
 * Hides all menu frames.
 */
void area_editor::hide_all_frames() {
    //TODO
    frm_main->hide();
    frm_layout->hide();
    frm_asb->hide();
    frm_texture->hide();
    frm_asa->hide();
    frm_objects->hide();
    frm_paths->hide();
    frm_details->hide();
    frm_review->hide();
    frm_tools->hide();
    frm_options->hide();
}


/* ----------------------------------------------------------------------------
 * Opens the frame where you pick from a list.
 * For the type of content, use area_editor_old::AREA_EDITOR_PICKER_*.
 */
void area_editor::open_picker(const unsigned char type) {
    //TODO.
}


/* ----------------------------------------------------------------------------
 * Picks an item and closes the list picker frame.
 */
void area_editor::pick(const string &name, const unsigned char type) {
    //TODO
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
