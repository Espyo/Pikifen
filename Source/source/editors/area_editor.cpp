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

/* ----------------------------------------------------------------------------
 * Initializes area editor class stuff.
 */
area_editor::area_editor() :
    state(EDITOR_STATE_MAIN),
    backup_timer(editor_backup_interval),
    double_click_time(0),
    path_preview_timer(0) {
    
}


/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor mode.
 */
void area_editor::change_to_right_frame() {
    sub_state = EDITOR_SUB_STATE_NONE;
    
    hide_all_frames();
    
    //TODO
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
