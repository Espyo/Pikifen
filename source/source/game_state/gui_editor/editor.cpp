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
        &GuiEditor::gridIntervalDecreaseCmd, "grid_interval_decrease"
    );
    register_cmd(
        &GuiEditor::gridIntervalIncreaseCmd, "grid_interval_increase"
    );
    register_cmd(&GuiEditor::deleteGuiDefCmd, "delete_gui_def");
    register_cmd(&GuiEditor::loadCmd, "load");
    register_cmd(&GuiEditor::quitCmd, "quit");
    register_cmd(&GuiEditor::reloadCmd, "reload");
    register_cmd(&GuiEditor::saveCmd, "save");
    register_cmd(&GuiEditor::snapModeCmd, "snap_mode");
    register_cmd(&GuiEditor::zoomAndPosResetCmd, "zoom_and_pos_reset");
    register_cmd(&GuiEditor::zoomInCmd, "zoom_in");
    register_cmd(&GuiEditor::zoomOutCmd, "zoom_out");
    
#undef register_cmd
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void GuiEditor::closeLoadDialog() {
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
void GuiEditor::closeOptionsDialog() {
    saveOptions();
}


/**
 * @brief Creates a new GUI definition, with the data from an existing
 * one in the base pack.
 *
 * @param internal_name Internal name of the GUI definition.
 * @param dest_pack The new definition's pack.
 */
void GuiEditor::createGuiDef(
    const string &internal_name, const string &pack
) {
    //Load the base pack one first.
    ContentManifest temp_orig_man;
    temp_orig_man.internal_name = internal_name;
    temp_orig_man.pack = FOLDER_NAMES::BASE_PACK;
    string orig_path =
        game.content.gui_defs.manifestToPath(temp_orig_man);
        
    loadGuiDefFile(orig_path, false);
    
    //Change the manifest under the hood so it's pointing to the new one.
    manifest.pack = pack;
    manifest.path = game.content.gui_defs.manifestToPath(manifest);
    
    changes_mgr.markAsNonExistent();
    
    setStatus(
        "Created GUI definition \"" +
        manifest.internal_name + "\" successfully."
    );
}


/**
 * @brief Deletes the current GUI definition.
 */
void GuiEditor::deleteCurrentGuiDef() {
    string orig_internal_name = manifest.internal_name;
    bool go_to_load_dialog = true;
    bool success = false;
    string message_box_text;
    
    if(!changes_mgr.existsOnDisk()) {
        //If the definition doesn't exist on disk, since it was never
        //saved, then there's nothing to delete.
        success = true;
        go_to_load_dialog = true;
        
    } else {
        //Delete the file.
        FS_DELETE_RESULT result = deleteFile(manifest.path);
        
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
            setupForNewGuiDef();
            openLoadDialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        setStatus(
            "Deleted GUI definition \"" + orig_internal_name +
            "\" successfully."
        );
    } else {
        setStatus(
            "GUI definition \"" + orig_internal_name +
            "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(message_box_text.empty()) {
        finish_up();
    } else {
        openMessageDialog(
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
void GuiEditor::deleteGuiDefCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(manifest.pack == FOLDER_NAMES::BASE_PACK) {
        openMessageDialog(
            "Can't delete GUI definition!",
            "This GUI definition is in the base pack, so it can't be deleted!",
            nullptr
        );
        return;
    }
    
    openDialog(
        "Delete GUI definition?",
        std::bind(&GuiEditor::processGuiDeleteGuiDefDialog, this)
    );
    dialogs.back()->custom_size = Point(600, 0);
}


/**
 * @brief Handles the logic part of the main loop of the GUI editor.
 */
void GuiEditor::doLogic() {
    Editor::doLogicPre();
    
    processGui();
    
    Editor::doLogicPost();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void GuiEditor::drawCanvasDearImGuiCallback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.gui_ed->drawCanvas();
}


/**
 * @brief Returns some tooltip text that represents a GUI definition
 * file's manifest.
 *
 * @param path Path to the file.
 * @return The tooltip text.
 */
string GuiEditor::getFileTooltip(const string &path) const {
    ContentManifest temp_manif;
    game.content.gui_defs.pathToManifest(
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
string GuiEditor::getName() const {
    return "GUI editor";
}


/**
 * @brief Returns the path to the currently opened content,
 * or an empty string if none.
 *
 * @return The path.
 */
string GuiEditor::getOpenedContentPath() const {
    return manifest.path;
}


/**
 * @brief Loads the GUI editor.
 */
void GuiEditor::load() {
    Editor::load();
    
    //Load necessary game content.
    game.content.reloadPacks();
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    },
    CONTENT_LOAD_LEVEL_EDITOR
    );
    
    //Misc. setup.
    must_recenter_cam = true;
    
    game.audio.setCurrentSong(game.sys_content_names.sng_editors, false);
    
    //Automatically load a file if needed, or show the load dialog.
    if(!auto_load_file.empty()) {
        loadGuiDefFile(auto_load_file, true);
    } else {
        openLoadDialog();
    }
}


/**
 * @brief Loads a GUI definition file.
 *
 * @param path Path to the file.
 * @param should_update_history If true, this loading process should update
 * the user's file open history.
 */
void GuiEditor::loadGuiDefFile(
    const string &path, bool should_update_history
) {
    //Setup.
    setupForNewGuiDef();
    changes_mgr.markAsNonExistent();
    
    //Load.
    manifest.fillFromPath(path);
    file_node = DataNode(manifest.path);
    
    if(!file_node.fileWasOpened) {
        openMessageDialog(
            "Load failed!",
            "Failed to load the GUI definition file \"" + manifest.path + "\"!",
        [this] () { openLoadDialog(); }
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
        updateHistory(game.options.gui_editor.history, manifest, "");
    }
    setStatus("Loaded file \"" + manifest.internal_name + "\" successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void GuiEditor::panCam(const ALLEGRO_EVENT &ev) {
    game.cam.setPos(
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
void GuiEditor::pickGuiDefFile(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    ContentManifest* temp_manif = (ContentManifest*) info;
    
    auto really_load = [this, temp_manif] () {
        closeTopDialog();
        loadGuiDefFile(temp_manif->path, true);
    };
    
    if(
        temp_manif->pack == FOLDER_NAMES::BASE_PACK &&
        !game.options.advanced.engine_dev
    ) {
        openBaseContentWarningDialog(really_load);
    } else {
        really_load();
    }
}


/**
 * @brief Code to run for the grid interval decrease command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::gridIntervalDecreaseCmd(float input_value) {
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
    setStatus(
        "Decreased grid interval to " +
        f2s(game.options.gui_editor.grid_interval) + "."
    );
}


/**
 * @brief Code to run for the grid interval increase command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::gridIntervalIncreaseCmd(float input_value) {
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
    setStatus(
        "Increased grid interval to " +
        f2s(game.options.gui_editor.grid_interval) + "."
    );
}


/**
 * @brief Code to run for the load command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::loadCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.askIfUnsaved(
        load_widget_pos,
        "loading a file", "load",
        std::bind(&GuiEditor::openLoadDialog, this),
        std::bind(&GuiEditor::saveGuiDef, this)
    );
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::quitCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.askIfUnsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&GuiEditor::leave, this),
        std::bind(&GuiEditor::saveGuiDef, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::reloadCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!changes_mgr.existsOnDisk()) return;
    
    changes_mgr.askIfUnsaved(
        reload_widget_pos,
        "reloading the current file", "reload",
    [this] () { loadGuiDefFile(string(manifest.path), false); },
    std::bind(&GuiEditor::saveGuiDef, this)
    );
}


/**
 * @brief Reloads all loaded GUI definitions.
 */
void GuiEditor::reloadGuiDefs() {
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    }
    );
    game.content.loadAll(
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
void GuiEditor::saveCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!saveGuiDef()) {
        return;
    }
}


/**
 * @brief Code to run for the snap mode command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::snapModeCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.options.gui_editor.snap = !game.options.gui_editor.snap;
    string final_status_text = "Set snap mode to ";
    if(game.options.gui_editor.snap) {
        final_status_text += "nothing";
    } else {
        final_status_text += "grid";
    }
    final_status_text += ".";
    setStatus(final_status_text);
}


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::zoomAndPosResetCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    resetCam(false);
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param input_value Value of the player input for the command.
 */
void GuiEditor::zoomInCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        std::clamp(
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
void GuiEditor::zoomOutCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        std::clamp(
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
void GuiEditor::resetCam(bool instantaneous) {
    centerCamera(Point(0.0f), Point(100.0f), instantaneous);
}


/**
 * @brief Saves the GUI file onto the disk.
 *
 * @return Whether it succeded.
 */
bool GuiEditor::saveGuiDef() {
    DataNode* positions_node = file_node.getChildByName("positions");
    for(size_t i = 0; i < items.size(); i++) {
        DataNode* item_node = positions_node->getChild(i);
        item_node->value = p2s(items[i].center) + " " + p2s(items[i].size);
    }
    
    if(!file_node.saveFile(manifest.path)) {
        showSystemMessageBox(
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
        setStatus("Could not save the GUI file!", true);
        return false;
    } else {
        setStatus("Saved GUI file successfully.");
        changes_mgr.markAsSaved();
        return true;
    }
    
    updateHistory(game.options.gui_editor.history, manifest, "");
}


/**
 * @brief Sets up the editor for a new GUI definition,
 * be it from an existing file or from scratch.
 */
void GuiEditor::setupForNewGuiDef() {
    manifest.clear();
    items.clear();
    cur_item = INVALID;
    
    //We could reset the camera directly, but if the player enters the editor
    //via the auto start maker tool, processGui() won't have a chance
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
Point GuiEditor::snapPoint(const Point &p) {
    Point final_point = p;
    bool do_snap = game.options.gui_editor.snap;
    
    if(is_ctrl_pressed) {
        if(cur_transformation_widget.isMovingCenterHandle()) {
            final_point =
                snapPointToAxis(
                    final_point, cur_transformation_widget.getOldCenter()
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
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    }
    );
}
