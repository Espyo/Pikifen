/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General GUI editor-related functions.
 */

#include "editor.h"

#include "../../functions.h"
#include "../../game.h"
#include "../../utils/string_utils.h"


namespace GUI_EDITOR {
//Possible grid intervals.
const vector<float> GRID_INTERVALS =
{1.0f, 2.0f, 2.5f, 5.0f, 10.0f};
//Width of the text widget that shows the mouse cursor coordinates.
const float MOUSE_COORDS_TEXT_WIDTH = 150.0f;
//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 64.0f;
//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.5f;
}


/* ----------------------------------------------------------------------------
 * Initializes GUI editor class stuff.
 */
gui_editor::gui_editor() :
    cur_item(INVALID),
    load_dialog_picker(this),
    must_focus_on_cur_item(false),
    must_recenter_cam(false) {
    
    zoom_max_level = GUI_EDITOR::ZOOM_MAX_LEVEL;
    zoom_min_level = GUI_EDITOR::ZOOM_MIN_LEVEL;
}


/* ----------------------------------------------------------------------------
 * Code to run when the load dialog is closed.
 */
void gui_editor::close_load_dialog() {
    if(!loaded_content_yet && file_name.empty()) {
        //The user cancelled the load dialog
        //presented when you enter the GUI editor. Quit out.
        leave();
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the options dialog is closed.
 */
void gui_editor::close_options_dialog() {
    save_options();
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the GUI editor.
 */
void gui_editor::do_logic() {
    editor::do_logic_pre();
    
    process_gui();
    
    editor::do_logic_post();
}


/* ----------------------------------------------------------------------------
 * Dear ImGui callback for when the canvas needs to be drawn on-screen.
 * parent_list:
 *   Unused.
 * cmd:
 *   Unused.
 */
void gui_editor::draw_canvas_imgui_callback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.gui_ed->draw_canvas();
}


/* ----------------------------------------------------------------------------
 * In the options data file, options pertaining to an editor's history
 * have a prefix. This function returns that prefix.
 */
string gui_editor::get_history_option_prefix() const {
    return "gui_editor_history";
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string gui_editor::get_name() const {
    return "GUI editor";
}


/* ----------------------------------------------------------------------------
 * Returns the name of the currently opened file, or an empty string if none.
 */
string gui_editor::get_opened_file_name() const {
    return file_name;
}


/* ----------------------------------------------------------------------------
 * Loads the GUI editor.
 */
void gui_editor::load() {
    editor::load();
    
    file_name.clear();
    loaded_content_yet = false;
    must_recenter_cam = true;
    
    if(!auto_load_file.empty()) {
        file_name = auto_load_file;
        load_file(true);
    } else {
        open_load_dialog();
    }
}


/* ----------------------------------------------------------------------------
 * Loads the GUI file.
 * should_update_history:
 *   If true, this loading process should update the user's file open history.
 */
void gui_editor::load_file(
    const bool should_update_history
) {
    items.clear();
    
    file_node = data_node(GUI_FOLDER_PATH + "/" + file_name);
    
    if(!file_node.file_was_opened) {
        set_status("Failed to load the file \"" + file_name + "\"!", true);
        open_load_dialog();
        return;
    }
    
    data_node* positions_node = file_node.get_child_by_name("positions");
    size_t n_items = positions_node->get_nr_of_children();
    
    for(size_t i = 0; i < n_items; ++i) {
        item new_item;
        data_node* item_node = positions_node->get_child(i);
        new_item.name = item_node->name;
        vector<string> words = split(item_node->value);
        if(words.size() != 4) continue;
        new_item.center.x = s2f(words[0]);
        new_item.center.y = s2f(words[1]);
        new_item.size.x = s2f(words[2]);
        new_item.size.y = s2f(words[3]);
        items.push_back(new_item);
    }
    
    cur_item = INVALID;
    
    has_unsaved_changes = false;
    was_warned_about_unsaved_changes = false;
    loaded_content_yet = true;
    
    //We could reset the camera now, but if the player enters the editor via
    //the auto start maker tool, process_gui() won't have a chance
    //to run before we load the file, and that function is what gives
    //us the canvas coordinates necessary for camera centering.
    //Let's flag the need for recentering so it gets handled when possible.
    must_recenter_cam = true;
    
    if(should_update_history) {
        update_history(file_name);
        save_options(); //Save the history in the options.
    }
    
    set_status("Loaded GUI file successfully.");
}


/* ----------------------------------------------------------------------------
 * Pans the camera around.
 * ev:
 *   Event to handle.
 */
void gui_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/* ----------------------------------------------------------------------------
 * Callback for when the user picks a file from the picker.
 * name:
 *   Name of the file.
 * category:
 *   Unused.
 * is_new:
 *   Unused
 */
void gui_editor::pick_file(
    const string &name, const string &category, const bool is_new
) {
    file_name = name;
    load_file(true);
    close_top_dialog();
}


/* ----------------------------------------------------------------------------
 * Code to run when the grid interval decrease button is pressed.
 */
void gui_editor::press_grid_interval_decrease_button() {
    float new_grid_interval = GUI_EDITOR::GRID_INTERVALS[0];
    for(size_t i = 0; i < GUI_EDITOR::GRID_INTERVALS.size(); ++i) {
        if(
            GUI_EDITOR::GRID_INTERVALS[i] >=
            game.options.gui_editor_grid_interval
        ) {
            break;
        }
        new_grid_interval = GUI_EDITOR::GRID_INTERVALS[i];
    }
    game.options.gui_editor_grid_interval = new_grid_interval;
    set_status(
        "Decreased grid interval to " +
        f2s(game.options.gui_editor_grid_interval) + "."
    );
}


/* ----------------------------------------------------------------------------
 * Code to run when the grid interval increase button is pressed.
 */
void gui_editor::press_grid_interval_increase_button() {
    float new_grid_interval = GUI_EDITOR::GRID_INTERVALS.back();
    for(int i = (int) (GUI_EDITOR::GRID_INTERVALS.size() - 1); i >= 0; --i) {
        if(
            GUI_EDITOR::GRID_INTERVALS[i] <=
            game.options.gui_editor_grid_interval
        ) {
            break;
        }
        new_grid_interval = GUI_EDITOR::GRID_INTERVALS[i];
    }
    game.options.gui_editor_grid_interval = new_grid_interval;
    set_status(
        "Increased grid interval to " +
        f2s(game.options.gui_editor_grid_interval) + "."
    );
}


/* ----------------------------------------------------------------------------
 * Code to run when the load button widget is pressed.
 */
void gui_editor::press_load_button() {
    if(!check_new_unsaved_changes(load_widget_pos)) {
        open_load_dialog();
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the quit button widget is pressed.
 */
void gui_editor::press_quit_button() {
    if(!check_new_unsaved_changes(quit_widget_pos)) {
        set_status("Bye!");
        leave();
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the reload button widget is pressed.
 */
void gui_editor::press_reload_button() {
    if(!check_new_unsaved_changes(reload_widget_pos)) {
        load_file(false);
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the save button widget is pressed.
 */
void gui_editor::press_save_button() {
    if(!save_file()) {
        return;
    }
    has_unsaved_changes = false;
    was_warned_about_unsaved_changes = false;
    set_status("Saved GUI file successfully.");
}


/* ----------------------------------------------------------------------------
 * Code to run when the snap mode button widget is pressed.
 */
void gui_editor::press_snap_mode_button() {
    game.options.gui_editor_snap = !game.options.gui_editor_snap;
    string final_status_text = "Set snap mode to ";
    if(game.options.gui_editor_snap) {
        final_status_text += "nothing";
    } else {
        final_status_text += "grid";
    }
    final_status_text += ".";
    set_status(final_status_text);
}


/* ----------------------------------------------------------------------------
 * Code to run when the zoom and position reset button widget is pressed.
 */
void gui_editor::press_zoom_and_pos_reset_button() {
    reset_cam(false);
}


/* ----------------------------------------------------------------------------
 * Code to run when the zoom in button widget is pressed.
 */
void gui_editor::press_zoom_in_button() {
    game.cam.target_zoom =
        clamp(
            game.cam.target_zoom +
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/* ----------------------------------------------------------------------------
 * Code to run when the zoom out button widget is pressed.
 */
void gui_editor::press_zoom_out_button() {
    game.cam.target_zoom =
        clamp(
            game.cam.target_zoom -
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/* ----------------------------------------------------------------------------
 * Resets the camera.
 * instantaneous:
 *   Whether the camera moves to its spot instantaneously or not.
 */
void gui_editor::reset_cam(const bool instantaneous) {
    center_camera(point(0.0f, 0.0f), point(100.0f, 100.0f), instantaneous);
}


/* ----------------------------------------------------------------------------
 * Saves the GUI file onto the disk.
 * Returns true on success, false on failure.
 */
bool gui_editor::save_file() {
    data_node* positions_node = file_node.get_child_by_name("positions");
    for(size_t i = 0; i < items.size(); ++i) {
        data_node* item_node = positions_node->get_child(i);
        item_node->value = p2s(items[i].center) + " " + p2s(items[i].size);
    }
    
    string file_path = GUI_FOLDER_PATH + "/" + file_name;
    
    if(!file_node.save_file(file_path)) {
        show_message_box(
            NULL, "Save failed!",
            "Could not save the GUI file!",
            (
                "An error occured while saving the GUI data to the file \"" +
                file_path + "\". Make sure that the folder it is saving to "
                "exists and it is not read-only, and try again."
            ).c_str(),
            NULL,
            ALLEGRO_MESSAGEBOX_WARN
        );
        set_status("Could not save the GUI file!", true);
        return false;
    } else {
        set_status("Saved GUI file successfully.");
    }
    has_unsaved_changes = false;
    was_warned_about_unsaved_changes = false;
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Snaps a point to the nearest available grid spot, or keeps the point as is
 * if Shift is pressed.
 * p:
 *   Point to snap.
 */
point gui_editor::snap_point(const point &p) {
    point final_point = p;
    bool do_snap = game.options.gui_editor_snap;
    
    if(is_ctrl_pressed) {
        if(cur_transformation_widget.is_moving_center_handle()) {
            final_point =
                snap_point_to_axis(
                    final_point, cur_transformation_widget.get_old_center()
                );
        }
    }
    
    if(is_shift_pressed) {
        do_snap = !do_snap;
    }
    
    if(!do_snap) {
        return final_point;
    }
    
    return
        point(
            round(final_point.x / game.options.gui_editor_grid_interval) *
            game.options.gui_editor_grid_interval,
            round(final_point.y / game.options.gui_editor_grid_interval) *
            game.options.gui_editor_grid_interval
        );
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void gui_editor::unload() {
    editor::unload();
    
    items.clear();
    cur_item = INVALID;
}
