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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


namespace GUI_EDITOR {

//Possible grid intervals.
const vector<float> GRID_INTERVALS =
{1.0f, 2.0f, 2.5f, 5.0f, 10.0f};

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 64.0f;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.5f;

}


/**
 * @brief Constructs a new GUI editor object.
 */
GuiEditor::GuiEditor() :
    load_dialog_picker(this) {
    
    zoom_max_level = GUI_EDITOR::ZOOM_MAX_LEVEL;
    zoom_min_level = GUI_EDITOR::ZOOM_MIN_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        Command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(
        &GuiEditor::grid_interval_decrease_cmd, "grid_interval_decrease"
    );
    register_cmd(
        &GuiEditor::grid_interval_increase_cmd, "grid_interval_increase"
    );
    register_cmd(&GuiEditor::delete_gui_def_cmd, "delete_gui_def");
    register_cmd(&GuiEditor::load_cmd, "load");
    register_cmd(&GuiEditor::quit_cmd, "quit");
    register_cmd(&GuiEditor::reload_cmd, "reload");
    register_cmd(&GuiEditor::save_cmd, "save");
    register_cmd(&GuiEditor::snap_mode_cmd, "snap_mode");
    register_cmd(&GuiEditor::zoom_and_pos_reset_cmd, "zoom_and_pos_reset");
    register_cmd(&GuiEditor::zoom_in_cmd, "zoom_in");
    register_cmd(&GuiEditor::zoom_out_cmd, "zoom_out");
    
#undef register_cmd
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void GuiEditor::close_load_dialog() {
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
void GuiEditor::close_options_dialog() {
    save_options();
}


/**
 * @brief Creates a new GUI definition, with the data from an existing
 * one in the base pack.
 *
 * @param internal_name Internal name of the GUI definition.
 * @param dest_pack The new definition's pack.
 */
void GuiEditor::create_gui_def(
    const string &internal_name, const string &pack
) {
    //Load the base pack one first.
    ContentManifest temp_orig_man;
    temp_orig_man.internal_name = internal_name;
    temp_orig_man.pack = FOLDER_NAMES::BASE_PACK;
    string orig_path =
        game.content.gui_defs.manifest_to_path(temp_orig_man);
        
    load_gui_def_file(orig_path, false);
    
    //Change the manifest under the hood so it's pointing to the new one.
    manifest.pack = pack;
    manifest.path = game.content.gui_defs.manifest_to_path(manifest);
    
    changes_mgr.mark_as_non_existent();
    
    set_status(
        "Created GUI definition \"" +
        manifest.internal_name + "\" successfully."
    );
}


/**
 * @brief Deletes the current GUI definition.
 */
void GuiEditor::delete_current_gui_def() {
    string orig_internal_name = manifest.internal_name;
    bool go_to_load_dialog = true;
    bool success = false;
    string message_box_text;
    
    if(!changes_mgr.exists_on_disk()) {
        //If the definition doesn't exist on disk, since it was never
        //saved, then there's nothing to delete.
        success = true;
        go_to_load_dialog = true;
        
    } else {
        //Delete the file.
        FS_DELETE_RESULT result = delete_file(manifest.path);
        
        switch(result) {
        case FS_DELETE_RESULT_OK:
        case FS_DELETE_RESULT_HAS_IMPORTANT: {
            success = true;
            go_to_load_dialog = true;
            break;
        } case FS_DELETE_RESULT_NOT_FOUND: {
            success = false;
            message_box_text =
                "GUI definition \"" + orig_internal_name +
                "\" deletion failed! The file was not found!";
            go_to_load_dialog = false;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            message_box_text =
                "GUI definition \"" + orig_internal_name +
                "\" deletion failed! Something went wrong. Please make sure "
                "there are enough permissions to delete the file and "
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
            setup_for_new_gui_def();
            open_load_dialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        set_status(
            "Deleted GUI definition \"" + orig_internal_name +
            "\" successfully."
        );
    } else {
        set_status(
            "GUI definition \"" + orig_internal_name +
            "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(message_box_text.empty()) {
        finish_up();
    } else {
        open_message_dialog(
            "GUI definition deletion failed!",
            message_box_text,
            finish_up
        );
    }
}


/**
 * @brief Code to run for the delete current GUI definition command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::delete_gui_def_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(manifest.pack == FOLDER_NAMES::BASE_PACK) {
        open_message_dialog(
            "Can't delete GUI definition!",
            "This GUI definition is in the base pack, so it can't be deleted!",
            nullptr
        );
        return;
    }
    
    open_dialog(
        "Delete GUI definition?",
        std::bind(&GuiEditor::process_gui_delete_gui_def_dialog, this)
    );
    dialogs.back()->custom_size = Point(600, 0);
}


/**
 * @brief Handles the logic part of the main loop of the GUI editor.
 */
void GuiEditor::do_logic() {
    Editor::do_logic_pre();
    
    process_gui();
    
    Editor::do_logic_post();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void GuiEditor::draw_canvas_imgui_callback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.gui_ed->draw_canvas();
}


/**
 * @brief Returns some tooltip text that represents a GUI definition
 * file's manifest.
 *
 * @param path Path to the file.
 * @return The tooltip text.
 */
string GuiEditor::get_file_tooltip(const string &path) const {
    ContentManifest temp_manif;
    game.content.gui_defs.path_to_manifest(
        path, &temp_manif
    );
    return
        "Internal name: " + temp_manif.internal_name + "\n"
        "File path: " + path + "\n"
        "Pack: " + game.content.packs.list[temp_manif.pack].name;
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string GuiEditor::get_name() const {
    return "GUI editor";
}


/**
 * @brief Returns the path to the currently opened content,
 * or an empty string if none.
 *
 * @return The path.
 */
string GuiEditor::get_opened_content_path() const {
    return manifest.path;
}


/**
 * @brief Loads the GUI editor.
 */
void GuiEditor::load() {
    Editor::load();
    
    //Load necessary game content.
    game.content.reload_packs();
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    },
    CONTENT_LOAD_LEVEL_EDITOR
    );
    
    //Misc. setup.
    must_recenter_cam = true;
    
    game.audio.set_current_song(game.sys_content_names.sng_editors, false);
    
    //Automatically load a file if needed, or show the load dialog.
    if(!auto_load_file.empty()) {
        load_gui_def_file(auto_load_file, true);
    } else {
        open_load_dialog();
    }
}


/**
 * @brief Loads a GUI definition file.
 *
 * @param path Path to the file.
 * @param should_update_history If true, this loading process should update
 * the user's file open history.
 */
void GuiEditor::load_gui_def_file(
    const string &path, bool should_update_history
) {
    //Setup.
    setup_for_new_gui_def();
    changes_mgr.mark_as_non_existent();
    
    //Load.
    manifest.fill_from_path(path);
    file_node = DataNode(manifest.path);
    
    if(!file_node.fileWasOpened) {
        open_message_dialog(
            "Load failed!",
            "Failed to load the GUI definition file \"" + manifest.path + "\"!",
        [this] () { open_load_dialog(); }
        );
        manifest.clear();
        return;
    }
    
    DataNode* positions_node = file_node.getChildByName("positions");
    size_t n_items = positions_node->getNrOfChildren();
    
    for(size_t i = 0; i < n_items; i++) {
        Item new_item;
        DataNode* item_node = positions_node->getChild(i);
        new_item.name = item_node->name;
        vector<string> words = split(item_node->value);
        if(words.size() != 4) continue;
        new_item.center.x = s2f(words[0]);
        new_item.center.y = s2f(words[1]);
        new_item.size.x = s2f(words[2]);
        new_item.size.y = s2f(words[3]);
        items.push_back(new_item);
    }
    
    //Finish up.
    changes_mgr.reset();
    if(should_update_history) {
        update_history(game.options.gui_editor.history, manifest, "");
    }
    set_status("Loaded file \"" + manifest.internal_name + "\" successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void GuiEditor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        Point(
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
void GuiEditor::pick_gui_def_file(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    ContentManifest* temp_manif = (ContentManifest*) info;
    
    auto really_load = [this, temp_manif] () {
        close_top_dialog();
        load_gui_def_file(temp_manif->path, true);
    };
    
    if(
        temp_manif->pack == FOLDER_NAMES::BASE_PACK &&
        !game.options.advanced.engine_dev
    ) {
        open_base_content_warning_dialog(really_load);
    } else {
        really_load();
    }
}


/**
 * @brief Code to run for the grid interval decrease command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::grid_interval_decrease_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    float new_grid_interval = GUI_EDITOR::GRID_INTERVALS[0];
    for(size_t i = 0; i < GUI_EDITOR::GRID_INTERVALS.size(); i++) {
        if(
            GUI_EDITOR::GRID_INTERVALS[i] >=
            game.options.gui_editor.grid_interval
        ) {
            break;
        }
        new_grid_interval = GUI_EDITOR::GRID_INTERVALS[i];
    }
    game.options.gui_editor.grid_interval = new_grid_interval;
    set_status(
        "Decreased grid interval to " +
        f2s(game.options.gui_editor.grid_interval) + "."
    );
}


/**
 * @brief Code to run for the grid interval increase command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::grid_interval_increase_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    float new_grid_interval = GUI_EDITOR::GRID_INTERVALS.back();
    for(int i = (int) (GUI_EDITOR::GRID_INTERVALS.size() - 1); i >= 0; --i) {
        if(
            GUI_EDITOR::GRID_INTERVALS[i] <=
            game.options.gui_editor.grid_interval
        ) {
            break;
        }
        new_grid_interval = GUI_EDITOR::GRID_INTERVALS[i];
    }
    game.options.gui_editor.grid_interval = new_grid_interval;
    set_status(
        "Increased grid interval to " +
        f2s(game.options.gui_editor.grid_interval) + "."
    );
}


/**
 * @brief Code to run for the load command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::load_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        load_widget_pos,
        "loading a file", "load",
        std::bind(&GuiEditor::open_load_dialog, this),
        std::bind(&GuiEditor::save_gui_def, this)
    );
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::quit_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&GuiEditor::leave, this),
        std::bind(&GuiEditor::save_gui_def, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::reload_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!changes_mgr.exists_on_disk()) return;
    
    changes_mgr.ask_if_unsaved(
        reload_widget_pos,
        "reloading the current file", "reload",
    [this] () { load_gui_def_file(string(manifest.path), false); },
    std::bind(&GuiEditor::save_gui_def, this)
    );
}


/**
 * @brief Reloads all loaded GUI definitions.
 */
void GuiEditor::reload_gui_defs() {
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    }
    );
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
}


/**
 * @brief Code to run for the save command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::save_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!save_gui_def()) {
        return;
    }
}


/**
 * @brief Code to run for the snap mode command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::snap_mode_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.options.gui_editor.snap = !game.options.gui_editor.snap;
    string final_status_text = "Set snap mode to ";
    if(game.options.gui_editor.snap) {
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
void GuiEditor::zoom_and_pos_reset_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    reset_cam(false);
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::zoom_in_cmd(float input_value) {
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
void GuiEditor::zoom_out_cmd(float input_value) {
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
void GuiEditor::reset_cam(bool instantaneous) {
    center_camera(Point(0.0f), Point(100.0f), instantaneous);
}


/**
 * @brief Saves the GUI file onto the disk.
 *
 * @return Whether it succeded.
 */
bool GuiEditor::save_gui_def() {
    DataNode* positions_node = file_node.getChildByName("positions");
    for(size_t i = 0; i < items.size(); i++) {
        DataNode* item_node = positions_node->getChild(i);
        item_node->value = p2s(items[i].center) + " " + p2s(items[i].size);
    }
    
    if(!file_node.saveFile(manifest.path)) {
        show_system_message_box(
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
    
    update_history(game.options.gui_editor.history, manifest, "");
}


/**
 * @brief Sets up the editor for a new GUI definition,
 * be it from an existing file or from scratch.
 */
void GuiEditor::setup_for_new_gui_def() {
    manifest.clear();
    items.clear();
    cur_item = INVALID;
    
    //We could reset the camera directly, but if the player enters the editor
    //via the auto start maker tool, process_gui() won't have a chance
    //to run before we load the file, and that function is what gives
    //us the canvas coordinates necessary for camera centering.
    //Let's flag the need for recentering so it gets handled when possible.
    must_recenter_cam = true;
}


/**
 * @brief Snaps a point to the nearest available grid spot,
 * or keeps the point as is if Shift is pressed.
 *
 * @param p Point to snap.
 * @return The snapped point.
 */
Point GuiEditor::snap_point(const Point &p) {
    Point final_point = p;
    bool do_snap = game.options.gui_editor.snap;
    
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
        Point(
            round(final_point.x / game.options.gui_editor.grid_interval) *
            game.options.gui_editor.grid_interval,
            round(final_point.y / game.options.gui_editor.grid_interval) *
            game.options.gui_editor.grid_interval
        );
}


/**
 * @brief Unloads the editor from memory.
 */
void GuiEditor::unload() {
    Editor::unload();
    
    items.clear();
    cur_item = INVALID;
    
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    }
    );
}
