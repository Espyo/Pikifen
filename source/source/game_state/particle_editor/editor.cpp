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
    loadDialogPicker(this) {
    
    zoomMaxLevel = PARTICLE_EDITOR::ZOOM_MAX_LEVEL;
    zoomMinLevel = PARTICLE_EDITOR::ZOOM_MIN_LEVEL;
    
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
    if(manifest.internalName.empty() && dialogs.size() == 1) {
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
    changesMgr.markAsNonExistent();
    
    //Create a particle generator with some defaults.
    loadedGen = ParticleGenerator();
    game.content.particleGens.pathToManifest(
        part_gen_path, &manifest
    );
    loadedGen.manifest = &manifest;
    loadedGen.baseParticle.duration = 1.0f;
    loadedGen.baseParticle.setBitmap("");
    loadedGen.baseParticle.size =
        KeyframeInterpolator<float>(32);
    loadedGen.baseParticle.color =
        KeyframeInterpolator<ALLEGRO_COLOR>(mapAlpha(255));
    loadedGen.baseParticle.color.add(1, mapAlpha(0));
    
    loadedGen.emission.interval = 0.5f;
    loadedGen.emission.number = 1;
    loadedGen.baseParticle.outwardsSpeed =
        KeyframeInterpolator<float>(32);
        
    //Finish up.
    setupForNewPartGenPost();
    updateHistory(game.options.partEd.history, manifest, "");
    setStatus(
        "Created particle generator \"" + manifest.internalName +
        "\" successfully."
    );
}


/**
 * @brief Deletes the current particle generator.
 */
void ParticleEditor::deleteCurrentPartGen() {
    string orig_internal_name = manifest.internalName;
    bool go_to_load_dialog = true;
    bool success = false;
    string message_box_text;
    
    if(!changesMgr.existsOnDisk()) {
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
    dialogs.back()->customSize = Point(600, 0);
}


/**
 * @brief Handles the logic part of the main loop of the GUI editor.
 */
void ParticleEditor::doLogic() {
    Editor::doLogicPre();
    
    processGui();
    
    if(mgrRunning) {
        if(genRunning) {
            loadedGen.followPosOffset =
                rotatePoint(generatorPosOffset, -generatorAngleOffset);
            loadedGen.tick(game.deltaT, partMgr);
            //If the particles are meant to emit once, turn them off.
            if(loadedGen.emission.interval == 0) {
                genRunning = false;
            }
        }
        partMgr.tickAll(game.deltaT);
    }
    
    Editor::doLogicPost();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-window.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void ParticleEditor::drawCanvasDearImGuiCallback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.particleEd->drawCanvas();
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
    game.content.particleGens.pathToManifest(
        path, &temp_manif
    );
    return
        "Internal name: " + temp_manif.internalName + "\n"
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
    game.audio.setCurrentSong(game.sysContentNames.sngEditors, false);
    
    partMgr = ParticleManager(game.options.advanced.maxParticles);
    
    //Set the background.
    if(!game.options.partEd.bgPath.empty()) {
        bg =
            loadBmp(
                game.options.partEd.bgPath,
                nullptr, false, false, false
            );
        useBg = true;
    } else {
        useBg = false;
    }
    
    //Automatically load a file if needed, or show the load dialog.
    if(!autoLoadFile.empty()) {
        loadPartGenFile(autoLoadFile, true);
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
    changesMgr.markAsNonExistent();
    
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
    
    loadedGen.manifest = &manifest;
    loadedGen.loadFromDataNode(&file, CONTENT_LOAD_LEVEL_FULL);
    
    //Finish up.
    setupForNewPartGenPost();
    changesMgr.reset();
    
    if(should_update_history) {
        updateHistory(game.options.partEd.history, manifest, loadedGen.name);
    }
    
    setStatus("Loaded file \"" + manifest.internalName + "\" successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::panCam(const ALLEGRO_EVENT &ev) {
    game.view.cam.setPos(
        Point(
            game.view.cam.pos.x - ev.mouse.dx / game.view.cam.zoom,
            game.view.cam.pos.y - ev.mouse.dy / game.view.cam.zoom
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
        !game.options.advanced.engineDev
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
            game.options.partEd.gridInterval
        ) {
            break;
        }
        new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS[i];
    }
    game.options.partEd.gridInterval = new_grid_interval;
    setStatus(
        "Decreased grid interval to " +
        f2s(game.options.partEd.gridInterval) + "."
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
            game.options.partEd.gridInterval
        ) {
            break;
        }
        new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS[i];
    }
    game.options.partEd.gridInterval = new_grid_interval;
    setStatus(
        "Increased grid interval to " +
        f2s(game.options.partEd.gridInterval) + "."
    );
}


/**
 * @brief Code to run for the grid toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::gridToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    gridVisible = !gridVisible;
    string state_str = (gridVisible ? "Enabled" : "Disabled");
    setStatus(state_str + " grid visibility.");
}


/**
 * @brief Code to run for the load command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::loadCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        loadWidgetPos,
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
    
    changesMgr.askIfUnsaved(
        quitWidgetPos,
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
    
    changesMgr.askIfUnsaved(
        reloadWidgetPos,
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
    loadedGen.followAngle = &generatorAngleOffset;
}


/**
 * @brief Sets up the editor for a new particle generator,
 * be it from an existing file or from scratch, before the actual creation/load
 * takes place.
 */
void ParticleEditor::setupForNewPartGenPre() {
    partMgr.clear();
    changesMgr.reset();
    manifest.clear();
    
    mgrRunning = true;
    genRunning = true;
    generatorAngleOffset = 0.0f;
    selectedColorKeyframe = 0;
    selectedSizeKeyframe = 0;
    selectedLinearSpeedKeyframe = 0;
    selectedOrbitalVelocityKeyframe = 0;
    selectedOutwardVelocityKeyframe = 0;
    loadedGen = ParticleGenerator();
    
    game.view.cam.setPos(Point());
    game.view.cam.setZoom(1.0f);
}


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::zoomAndPosResetCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(game.view.cam.targetZoom == 1.0f) {
        game.view.cam.targetPos = Point();
    } else {
        game.view.cam.targetZoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::zoomInCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.view.cam.targetZoom =
        std::clamp(
            game.view.cam.targetZoom +
            game.view.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoomMinLevel, zoomMaxLevel
        );
}


/**
 * @brief Code to run for the zoom out command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::zoomOutCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.view.cam.targetZoom =
        std::clamp(
            game.view.cam.targetZoom -
            game.view.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoomMinLevel, zoomMaxLevel
        );
}


/**
 * @brief Code to run for the leader silhouette toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::leaderSilhouetteToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    leaderSilhouetteVisible = !leaderSilhouetteVisible;
    string state_str = (leaderSilhouetteVisible ? "Enabled" : "Disabled");
    setStatus(state_str + " leader silhouette visibility.");
}


/**
 * @brief Code to run for the particle clearing command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::clearParticlesCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    partMgr.clear();
    setStatus("Cleared particles.");
}


/**
 * @brief Code to run for the emission shape toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::emissionShapeToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    emissionShapeVisible = !emissionShapeVisible;
    string state_str = (emissionShapeVisible ? "Enabled" : "Disabled");
    setStatus(state_str + " emission shape visibility.");
}


/**
 * @brief Code to run for the particle generator playback toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::partGenPlaybackToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    genRunning = !genRunning;
    string state_str = (genRunning ? "Enabled" : "Disabled");
    setStatus(state_str + " particle generator playback.");
}


/**
 * @brief Code to run for the particle manager playback toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::partMgrPlaybackToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    mgrRunning = !mgrRunning;
    string state_str = (mgrRunning ? "Enabled" : "Disabled");
    setStatus(state_str + " particle system playback.");
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void ParticleEditor::resetCamXY() {
    game.view.cam.targetPos = Point();
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
    loadedGen.engineVersion = getEngineVersionString();
    
    DataNode file_node = DataNode("", "");
    loadedGen.saveToDataNode(&file_node);
    
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
        changesMgr.markAsSaved();
        updateHistory(game.options.partEd.history, manifest, loadedGen.name);
        return true;
    }
    
    return false;
}


/**
 * @brief Unloads the editor from memory.
 */
void ParticleEditor::unload() {
    Editor::unload();
    
    partMgr.clear();
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
}
