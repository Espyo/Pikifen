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
#include "../../utils/allegro_utils.h"
#include "../../utils/string_utils.h"


namespace GUI_EDITOR {

//Possible grid intervals.
const vector<float> GRID_INTERVALS =
{1.0f, 2.0f, 2.5f, 5.0f, 10.0f};

//Width of the text widget that shows the mouse cursor coordinates.
const float MOUSE_COORDS_TEXT_WIDTH = 150.0f;

//Name of the song to play in this state.
const string SONG_NAME = "editors";

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 64.0f;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.5f;

}


/**
 * @brief Constructs a new GUI editor object.
 */
gui_editor::gui_editor() :
    load_dialog_picker(this) {
    
    zoom_max_level = GUI_EDITOR::ZOOM_MAX_LEVEL;
    zoom_min_level = GUI_EDITOR::ZOOM_MIN_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(
        &gui_editor::grid_interval_decrease_cmd, "grid_interval_decrease"
    );
    register_cmd(
        &gui_editor::grid_interval_increase_cmd, "grid_interval_increase"
    );
    register_cmd(&gui_editor::load_cmd, "load");
    register_cmd(&gui_editor::quit_cmd, "quit");
    register_cmd(&gui_editor::reload_cmd, "reload");
    register_cmd(&gui_editor::save_cmd, "save");
    register_cmd(&gui_editor::snap_mode_cmd, "snap_mode");
    register_cmd(&gui_editor::zoom_and_pos_reset_cmd, "zoom_and_pos_reset");
    register_cmd(&gui_editor::zoom_in_cmd, "zoom_in");
    register_cmd(&gui_editor::zoom_out_cmd, "zoom_out");
    
#undef register_cmd
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void gui_editor::close_load_dialog() {
    if(!loaded_content_yet && manifest.internal_name.empty()) {
        //The user cancelled the load dialog
        //presented when you enter the GUI editor. Quit out.
        leave();
    }
}


/**
 * @brief Code to run when the options dialog is closed.
 */
void gui_editor::close_options_dialog() {
    save_options();
}


/**
 * @brief Handles the logic part of the main loop of the GUI editor.
 */
void gui_editor::do_logic() {
    editor::do_logic_pre();
    
    process_gui();
    
    editor::do_logic_post();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void gui_editor::draw_canvas_imgui_callback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.gui_ed->draw_canvas();
}


/**
 * @brief In the options data file, options pertaining to an editor's history
 * have a prefix. This function returns that prefix.
 *
 * @return The prefix.
 */
string gui_editor::get_history_option_prefix() const {
    return "gui_editor_history";
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string gui_editor::get_name() const {
    return "GUI editor";
}


/**
 * @brief Returns the name of the currently opened file, or an empty string
 * if none.
 *
 * @return The name.
 */
string gui_editor::get_opened_file_name() const {
    return manifest.internal_name;
}


/**
 * @brief Loads the GUI editor.
 */
void gui_editor::load() {
    editor::load();
    
    manifest.clear();
    loaded_content_yet = false;
    must_recenter_cam = true;
    
    game.content.load_packs();
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    },
    CONTENT_LOAD_LEVEL_EDITOR
    );
    
    game.audio.set_current_song(GUI_EDITOR::SONG_NAME, false);
    
    if(!auto_load_file.empty()) {
        manifest.fill_from_path(auto_load_file);
        load_file(true);
    } else {
        open_load_dialog();
    }
}


/**
 * @brief Loads the GUI file.
 *
 * @param should_update_history If true, this loading process should update
 * the user's file open history.
 */
void gui_editor::load_file(
    bool should_update_history
) {
    items.clear();
    
    file_node = data_node(manifest.path);
    
    if(!file_node.file_was_opened) {
        set_status("Failed to load the file \"" + manifest.path + "\"!", true);
        open_load_dialog();
        return;
    }
    
    data_node* positions_node = file_node.get_child_by_name("positions");
    size_t n_items = positions_node->get_nr_of_children();
    
    for(size_t i = 0; i < n_items; i++) {
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
    
    changes_mgr.reset();
    loaded_content_yet = true;
    
    //We could reset the camera now, but if the player enters the editor via
    //the auto start maker tool, process_gui() won't have a chance
    //to run before we load the file, and that function is what gives
    //us the canvas coordinates necessary for camera centering.
    //Let's flag the need for recentering so it gets handled when possible.
    must_recenter_cam = true;
    
    if(should_update_history) {
        update_history(manifest.path);
        save_options(); //Save the history in the options.
    }
    
    set_status("Loaded GUI file successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void gui_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/**
 * @brief Callback for when the user picks a file from the picker.
 *
 * @param name Name of the file.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Pointer to the file's content manifest.
 * @param is_new Unused.
 */
void gui_editor::pick_file(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    manifest = *((content_manifest*) info);
    load_file(true);
    close_top_dialog();
}


/**
 * @brief Code to run for the grid interval decrease command.
 *
 * @param input_value Value of the player input for the command.
 */
void gui_editor::grid_interval_decrease_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    float new_grid_interval = GUI_EDITOR::GRID_INTERVALS[0];
    for(size_t i = 0; i < GUI_EDITOR::GRID_INTERVALS.size(); i++) {
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


/**
 * @brief Code to run for the grid interval increase command.
 *
 * @param input_value Value of the player input for the command.
 */
void gui_editor::grid_interval_increase_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
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


/**
 * @brief Code to run for the load command.
 *
 * @param input_value Value of the player input for the command.
 */
void gui_editor::load_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        load_widget_pos,
        "loading a file", "load",
        std::bind(&gui_editor::open_load_dialog, this),
        std::bind(&gui_editor::save_file, this)
    );
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void gui_editor::quit_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&gui_editor::leave, this),
        std::bind(&gui_editor::save_file, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void gui_editor::reload_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        reload_widget_pos,
        "reloading the current file", "reload",
    [this] () { load_file(false); },
    std::bind(&gui_editor::save_file, this)
    );
}


/**
 * @brief Code to run for the save command.
 *
 * @param input_value Value of the player input for the command.
 */
void gui_editor::save_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!save_file()) {
        return;
    }
}


/**
 * @brief Code to run for the snap mode command.
 *
 * @param input_value Value of the player input for the command.
 */
void gui_editor::snap_mode_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
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


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param input_value Value of the player input for the command.
 */
void gui_editor::zoom_and_pos_reset_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    reset_cam(false);
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param input_value Value of the player input for the command.
 */
void gui_editor::zoom_in_cmd(float input_value) {
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
void gui_editor::zoom_out_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        clamp(
            game.cam.target_zoom -
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/**
 * @brief Resets the camera.
 *
 * @param instantaneous Whether the camera moves to its spot instantaneously
 * or not.
 */
void gui_editor::reset_cam(bool instantaneous) {
    center_camera(point(0.0f, 0.0f), point(100.0f, 100.0f), instantaneous);
}


/**
 * @brief Saves the GUI file onto the disk.
 *
 * @return Whether it succeded.
 */
bool gui_editor::save_file() {
    data_node* positions_node = file_node.get_child_by_name("positions");
    for(size_t i = 0; i < items.size(); i++) {
        data_node* item_node = positions_node->get_child(i);
        item_node->value = p2s(items[i].center) + " " + p2s(items[i].size);
    }
    
    if(!file_node.save_file(manifest.path)) {
        show_message_box(
            nullptr, "Save failed!",
            "Could not save the GUI file!",
            (
                "An error occured while saving the GUI data to the file \"" +
                manifest.path + "\". Make sure that the folder it is saving "
                "to exists and it is not read-only, and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        set_status("Could not save the GUI file!", true);
        return false;
    } else {
        set_status("Saved GUI file successfully.");
        changes_mgr.mark_as_saved();
        return true;
    }
}


/**
 * @brief Snaps a point to the nearest available grid spot,
 * or keeps the point as is if Shift is pressed.
 *
 * @param p Point to snap.
 * @return The snapped point.
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


/**
 * @brief Unloads the editor from memory.
 */
void gui_editor::unload() {
    editor::unload();
    
    items.clear();
    cur_item = INVALID;
    
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    }
    );
    game.content.unload_packs();
}
