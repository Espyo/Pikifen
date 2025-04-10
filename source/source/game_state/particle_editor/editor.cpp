/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General particle editor-related functions.
 */

#include "editor.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


namespace PARTICLE_EDITOR {

//Possible grid intervals.
const vector<float> GRID_INTERVALS =
{4.0f, 8.0f, 16.0f, 32.0f, 64.0f};

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 64;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.5f;

}


/**
 * @brief Constructs a new particle editor object.
 */
ParticleEditor::ParticleEditor() :
    load_dialog_picker(this) {
    
    zoom_max_level = PARTICLE_EDITOR::ZOOM_MAX_LEVEL;
    zoom_min_level = PARTICLE_EDITOR::ZOOM_MIN_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        Command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(
        &ParticleEditor::gridIntervalDecreaseCmd, "grid_interval_decrease"
    );
    register_cmd(
        &ParticleEditor::gridIntervalIncreaseCmd, "grid_interval_increase"
    );
    register_cmd(&ParticleEditor::gridToggleCmd, "grid_toggle");
    register_cmd(&ParticleEditor::deletePartGenCmd, "delete_part_gen");
    register_cmd(&ParticleEditor::loadCmd, "load");
    register_cmd(&ParticleEditor::quitCmd, "quit");
    register_cmd(
        &ParticleEditor::partMgrPlaybackToggleCmd, "part_mgr_toggle"
    );
    register_cmd(
        &ParticleEditor::partGenPlaybackToggleCmd, "part_gen_toggle"
    );
    register_cmd(
        &ParticleEditor::leaderSilhouetteToggleCmd,
        "leader_silhouette_toggle"
    );
    register_cmd(&ParticleEditor::reloadCmd, "reload");
    register_cmd(&ParticleEditor::saveCmd, "save");
    register_cmd(
        &ParticleEditor::zoomAndPosResetCmd, "zoom_and_pos_reset"
    );
    register_cmd(&ParticleEditor::zoomInCmd, "zoom_in");
    register_cmd(&ParticleEditor::zoomOutCmd, "zoom_out");
    
#undef register_cmd
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void ParticleEditor::closeLoadDialog() {
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
void ParticleEditor::closeOptionsDialog() {
    saveOptions();
}


/**
 * @brief Creates a new, empty particle generator.
 *
 * @param part_gen_path Path to the new particle generator.
 */
void ParticleEditor::createPartGen(
    const string &part_gen_path
) {
    //Setup.
    setupForNewPartGenPre();
    changes_mgr.markAsNonExistent();
    
    //Create a particle generator with some defaults.
    loaded_gen = ParticleGenerator();
    game.content.particle_gen.pathToManifest(
        part_gen_path, &manifest
    );
    loaded_gen.manifest = &manifest;
    loaded_gen.base_particle.duration = 1.0f;
    loaded_gen.base_particle.setBitmap("");
    loaded_gen.base_particle.size =
        KeyframeInterpolator<float>(32);
    loaded_gen.base_particle.color =
        KeyframeInterpolator<ALLEGRO_COLOR>(mapAlpha(255));
    loaded_gen.base_particle.color.add(1, mapAlpha(0));
    
    loaded_gen.emission.interval = 0.5f;
    loaded_gen.emission.number = 1;
    loaded_gen.base_particle.outwards_speed =
        KeyframeInterpolator<float>(32);
        
    //Finish up.
    setupForNewPartGenPost();
    updateHistory(game.options.particle_editor.history, manifest, "");
    setStatus(
        "Created particle generator \"" + manifest.internal_name +
        "\" successfully."
    );
}


/**
 * @brief Deletes the current particle generator.
 */
void ParticleEditor::deleteCurrentPartGen() {
    string orig_internal_name = manifest.internal_name;
    bool go_to_load_dialog = true;
    bool success = false;
    string message_box_text;
    
    if(!changes_mgr.existsOnDisk()) {
        //If the generator doesn't exist on disk, since it was never
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
                "Particle generator \"" + orig_internal_name +
                "\" deletion failed! The file was not found!";
            go_to_load_dialog = false;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            message_box_text =
                "Particle generator \"" + orig_internal_name +
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
            setupForNewPartGenPre();
            openLoadDialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        setStatus(
            "Deleted particle generator \"" + orig_internal_name +
            "\" successfully."
        );
    } else {
        setStatus(
            "Particle generator \"" + orig_internal_name +
            "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(message_box_text.empty()) {
        finish_up();
    } else {
        openMessageDialog(
            "Particle generator deletion failed!",
            message_box_text,
            finish_up
        );
    }
}


/**
 * @brief Code to run for the delete current particle generator command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::deletePartGenCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    openDialog(
        "Delete particle generator?",
        std::bind(&ParticleEditor::processGuiDeletePartGenDialog, this)
    );
    dialogs.back()->custom_size = Point(600, 0);
}


/**
 * @brief Handles the logic part of the main loop of the GUI editor.
 */
void ParticleEditor::doLogic() {
    Editor::doLogicPre();
    
    processGui();
    
    if(mgr_running) {
        if(gen_running) {
            loaded_gen.follow_pos_offset =
                rotatePoint(generator_pos_offset, -generator_angle_offset);
            loaded_gen.tick(game.delta_t, part_mgr);
            //If the particles are meant to emit once, turn them off.
            if(loaded_gen.emission.interval == 0) {
                gen_running = false;
            }
        }
        part_mgr.tickAll(game.delta_t);
    }
    
    Editor::doLogicPost();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void ParticleEditor::drawCanvasDearImGuiCallback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.particle_ed->drawCanvas();
}


/**
 * @brief Returns some tooltip text that represents a particle generator
 * file's manifest.
 *
 * @param path Path to the file.
 * @return The tooltip text.
 */
string ParticleEditor::getFileTooltip(const string &path) const {
    ContentManifest temp_manif;
    game.content.particle_gen.pathToManifest(
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
string ParticleEditor::getName() const {
    return "Particle editor";
}


/**
 * @brief Returns the path to the currently opened content,
 * or an empty string if none.
 *
 * @return The path.
 */
string ParticleEditor::getOpenedContentPath() const {
    return manifest.path;
}


/**
 * @brief Loads the GUI editor.
 */
void ParticleEditor::load() {
    Editor::load();
    
    //Load necessary game content.
    game.content.reloadPacks();
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    //Misc. setup.
    game.audio.setCurrentSong(game.sys_content_names.sng_editors, false);
    
    part_mgr = ParticleManager(game.options.advanced.max_particles);
    
    //Set the background.
    if(!game.options.particle_editor.bg_path.empty()) {
        bg =
            loadBmp(
                game.options.particle_editor.bg_path,
                nullptr, false, false, false
            );
        use_bg = true;
    } else {
        use_bg = false;
    }
    
    //Automatically load a file if needed, or show the load dialog.
    if(!auto_load_file.empty()) {
        loadPartGenFile(auto_load_file, true);
    } else {
        openLoadDialog();
    }
}


/**
 * @brief Loads a particle generator.
 *
 * @param path Path to the file.
 * @param should_update_history If true, this loading process should update
 * the user's file open history.
 */
void ParticleEditor::loadPartGenFile(
    const string &path, const bool should_update_history
) {
    //Setup.
    setupForNewPartGenPre();
    changes_mgr.markAsNonExistent();
    
    //Load.
    manifest.fillFromPath(path);
    DataNode file = DataNode(manifest.path);
    
    if(!file.fileWasOpened) {
        openMessageDialog(
            "Load failed!",
            "Failed to load the particle generator file \"" +
            manifest.path + "\"!",
        [this] () { openLoadDialog(); }
        );
        manifest.clear();
        return;
    }
    
    loaded_gen.manifest = &manifest;
    loaded_gen.loadFromDataNode(&file, CONTENT_LOAD_LEVEL_FULL);
    
    //Finish up.
    setupForNewPartGenPost();
    changes_mgr.reset();
    
    if(should_update_history) {
        updateHistory(game.options.particle_editor.history, manifest, loaded_gen.name);
    }
    
    setStatus("Loaded file \"" + manifest.internal_name + "\" successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::panCam(const ALLEGRO_EVENT &ev) {
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
void ParticleEditor::pickPartGenFile(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    ContentManifest* temp_manif = (ContentManifest*) info;
    
    auto really_load = [this, temp_manif] () {
        closeTopDialog();
        loadPartGenFile(temp_manif->path, true);
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
 * @brief Reloads all loaded particle generators.
 */
void ParticleEditor::reloadPartGens() {
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
}


/**
 * @brief Code to run for the grid interval decrease command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::gridIntervalDecreaseCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    float new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS[0];
    for(size_t i = 0; i < PARTICLE_EDITOR::GRID_INTERVALS.size(); ++i) {
        if(
            PARTICLE_EDITOR::GRID_INTERVALS[i] >=
            game.options.particle_editor.grid_interval
        ) {
            break;
        }
        new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS[i];
    }
    game.options.particle_editor.grid_interval = new_grid_interval;
    setStatus(
        "Decreased grid interval to " +
        f2s(game.options.particle_editor.grid_interval) + "."
    );
}


/**
 * @brief Code to run for the grid interval increase command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::gridIntervalIncreaseCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    float new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS.back();
    for(int i = (int) (PARTICLE_EDITOR::GRID_INTERVALS.size() - 1); i >= 0; --i) {
        if(
            PARTICLE_EDITOR::GRID_INTERVALS[i] <=
            game.options.particle_editor.grid_interval
        ) {
            break;
        }
        new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS[i];
    }
    game.options.particle_editor.grid_interval = new_grid_interval;
    setStatus(
        "Increased grid interval to " +
        f2s(game.options.particle_editor.grid_interval) + "."
    );
}


/**
 * @brief Code to run for the grid toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::gridToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    grid_visible = !grid_visible;
    string state_str = (grid_visible ? "Enabled" : "Disabled");
    setStatus(state_str + " grid visibility.");
}


/**
 * @brief Code to run for the load command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::loadCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.askIfUnsaved(
        load_widget_pos,
        "loading a file", "load",
        std::bind(&ParticleEditor::openLoadDialog, this),
        std::bind(&ParticleEditor::savePartGen, this)
    );
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::quitCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.askIfUnsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&ParticleEditor::leave, this),
        std::bind(&ParticleEditor::savePartGen, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::reloadCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.askIfUnsaved(
        reload_widget_pos,
        "reloading the current file", "reload",
    [this] () { loadPartGenFile(string(manifest.path), false); },
    std::bind(&ParticleEditor::savePartGen, this)
    );
}


/**
 * @brief Code to run for the save command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::saveCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    savePartGen();
}


/**
 * @brief Sets up the editor for a new particle generator,
 * be it from an existing file or from scratch, after the actual creation/load
 * takes place.
 */
void ParticleEditor::setupForNewPartGenPost() {
    loaded_gen.follow_angle = &generator_angle_offset;
}


/**
 * @brief Sets up the editor for a new particle generator,
 * be it from an existing file or from scratch, before the actual creation/load
 * takes place.
 */
void ParticleEditor::setupForNewPartGenPre() {
    part_mgr.clear();
    changes_mgr.reset();
    manifest.clear();
    
    mgr_running = true;
    gen_running = true;
    generator_angle_offset = 0.0f;
    selected_color_keyframe = 0;
    selected_size_keyframe = 0;
    selected_linear_speed_keyframe = 0;
    selected_oribital_velocity_keyframe = 0;
    selected_outward_velocity_keyframe = 0;
    loaded_gen = ParticleGenerator();
    
    game.cam.setPos(Point());
    game.cam.set_zoom(1.0f);
}


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::zoomAndPosResetCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(game.cam.target_zoom == 1.0f) {
        game.cam.target_pos = Point();
    } else {
        game.cam.target_zoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::zoomInCmd(float input_value) {
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
void ParticleEditor::zoomOutCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        std::clamp(
            game.cam.target_zoom -
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/**
 * @brief Code to run for the leader silhouette toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::leaderSilhouetteToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    leader_silhouette_visible = !leader_silhouette_visible;
    string state_str = (leader_silhouette_visible ? "Enabled" : "Disabled");
    setStatus(state_str + " leader silhouette visibility.");
}


/**
 * @brief Code to run for the particle clearing command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::clearParticlesCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    part_mgr.clear();
    setStatus("Cleared particles.");
}


/**
 * @brief Code to run for the emission shape toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::emissionShapeToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    emission_shape_visible = !emission_shape_visible;
    string state_str = (emission_shape_visible ? "Enabled" : "Disabled");
    setStatus(state_str + " emission shape visibility.");
}


/**
 * @brief Code to run for the particle generator playback toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::partGenPlaybackToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    gen_running = !gen_running;
    string state_str = (gen_running ? "Enabled" : "Disabled");
    setStatus(state_str + " particle generator playback.");
}


/**
 * @brief Code to run for the particle manager playback toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::partMgrPlaybackToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    mgr_running = !mgr_running;
    string state_str = (mgr_running ? "Enabled" : "Disabled");
    setStatus(state_str + " particle system playback.");
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void ParticleEditor::resetCamXY() {
    game.cam.target_pos = Point();
}


/**
 * @brief Resets the camera's zoom.
 */
void ParticleEditor::resetCamZoom() {
    zoomWithCursor(1.0f);
}


/**
 * @brief Saves the particle generator onto the disk.
 *
 * @return Whether it succeded.
 */
bool ParticleEditor::savePartGen() {
    loaded_gen.engine_version = getEngineVersionString();
    
    DataNode file_node = DataNode("", "");
    loaded_gen.saveToDataNode(&file_node);
    
    if(!file_node.saveFile(manifest.path)) {
        showSystemMessageBox(
            nullptr, "Save failed!",
            "Could not save the particle file!",
            (
                "An error occured while saving the particle generator to the file \"" +
                manifest.path + "\". Make sure that the folder it is saving to "
                "exists and it is not read-only, and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        setStatus("Could not save the particle generator file!", true);
        return false;
    } else {
        setStatus("Saved file successfully.");
        changes_mgr.markAsSaved();
        updateHistory(game.options.particle_editor.history, manifest, loaded_gen.name);
        return true;
    }
    
    return false;
}


/**
 * @brief Unloads the editor from memory.
 */
void ParticleEditor::unload() {
    Editor::unload();
    
    part_mgr.clear();
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
}
