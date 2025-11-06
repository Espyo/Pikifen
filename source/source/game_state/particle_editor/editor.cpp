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

#include "../../core/game.h"
#include "../../core/load.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/os_utils.h"
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
    
#define registerCmd(ptr, name) \
    commands.push_back( \
        Command(std::bind((ptr), this, std::placeholders::_1), \
            (name)) \
        );
    
    registerCmd(
        &ParticleEditor::gridIntervalDecreaseCmd, "grid_interval_decrease"
    );
    registerCmd(
        &ParticleEditor::gridIntervalIncreaseCmd, "grid_interval_increase"
    );
    registerCmd(&ParticleEditor::gridToggleCmd, "grid_toggle");
    registerCmd(&ParticleEditor::deletePartGenCmd, "delete_part_gen");
    registerCmd(&ParticleEditor::loadCmd, "load");
    registerCmd(&ParticleEditor::quitCmd, "quit");
    registerCmd(
        &ParticleEditor::partMgrPlaybackToggleCmd, "part_mgr_toggle"
    );
    registerCmd(
        &ParticleEditor::partGenPlaybackToggleCmd, "part_gen_toggle"
    );
    registerCmd(
        &ParticleEditor::leaderSilhouetteToggleCmd,
        "leader_silhouette_toggle"
    );
    registerCmd(&ParticleEditor::reloadCmd, "reload");
    registerCmd(&ParticleEditor::saveCmd, "save");
    registerCmd(
        &ParticleEditor::zoomAndPosResetCmd, "zoom_and_pos_reset"
    );
    registerCmd(&ParticleEditor::zoomInCmd, "zoom_in");
    registerCmd(&ParticleEditor::zoomOutCmd, "zoom_out");
    
#undef registerCmd
}


/**
 * @brief Code to run for the particle clearing command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::clearParticlesCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    partMgr.clear();
    setStatus("Cleared particles.");
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
 * @param partGenPath Path to the new particle generator.
 */
void ParticleEditor::createPartGen(
    const string& partGenPath
) {
    //Setup.
    setupForNewPartGenPre();
    changesMgr.markAsNonExistent();
    
    //Create a particle generator with some defaults.
    loadedGen = ParticleGenerator();
    game.content.particleGens.pathToManifest(
        partGenPath, &manifest
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
    string origInternalName = manifest.internalName;
    bool goToLoadDialog = true;
    bool success = false;
    string messageBoxText;
    
    if(!changesMgr.existsOnDisk()) {
        //If the generator doesn't exist in the disk, since it was never
        //saved, then there's nothing to delete.
        success = true;
        goToLoadDialog = true;
        
    } else {
        //Delete the file.
        FS_DELETE_RESULT result = deleteFile(manifest.path);
        
        switch(result) {
        case FS_DELETE_RESULT_OK:
        case FS_DELETE_RESULT_HAS_IMPORTANT: {
            success = true;
            goToLoadDialog = true;
            break;
        } case FS_DELETE_RESULT_NOT_FOUND: {
            success = false;
            messageBoxText =
                "Could not delete particle generator file \"" + manifest.path +
                "\"! The file was not found!";
            goToLoadDialog = false;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            messageBoxText =
                "Could not delete particle generator file \"" + manifest.path +
                "\"! Something went wrong. Please make sure there are enough "
                "permissions to delete the file and try again.";
            goToLoadDialog = false;
            break;
        }
        }
        
    }
    
    //This code will be run after everything is done, be it after the standard
    //procedure, or after the user hits OK on the message box.
    const auto finishUp = [this, goToLoadDialog] () {
        if(goToLoadDialog) {
            setupForNewPartGenPre();
            openLoadDialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        setStatus(
            "Deleted particle generator \"" + origInternalName +
            "\" successfully."
        );
    } else {
        setStatus(
            "Particle generator \"" + origInternalName +
            "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(messageBoxText.empty()) {
        finishUp();
    } else {
        openMessageDialog(
            "Particle generator deletion failed!",
            messageBoxText,
            finishUp
        );
    }
}


/**
 * @brief Code to run for the delete current particle generator command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::deletePartGenCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
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
 * @param parentList Unused.
 * @param cmd Unused.
 */
void ParticleEditor::drawCanvasDearImGuiCallback(
    const ImDrawList* parentList, const ImDrawCmd* cmd
) {
    game.states.particleEd->drawCanvas();
}


/**
 * @brief Code to run for the emission shape toggle command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::emissionShapeToggleCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    emissionShapeVisible = !emissionShapeVisible;
    string stateStr = (emissionShapeVisible ? "Enabled" : "Disabled");
    setStatus(stateStr + " emission shape visibility.");
}


/**
 * @brief Returns some tooltip text that represents a particle generator
 * file's manifest.
 *
 * @param path Path to the file.
 * @return The tooltip text.
 */
string ParticleEditor::getFileTooltip(const string& path) const {
    ContentManifest tempManif;
    game.content.particleGens.pathToManifest(
        path, &tempManif
    );
    return
        "Internal name: " + tempManif.internalName + "\n"
        "File path: " + path + "\n"
        "Pack: " + game.content.packs.list[tempManif.pack].name;
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
 * @brief Code to run for the grid interval decrease command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::gridIntervalDecreaseCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    float newGridInterval = PARTICLE_EDITOR::GRID_INTERVALS[0];
    for(size_t i = 0; i < PARTICLE_EDITOR::GRID_INTERVALS.size(); ++i) {
        if(
            PARTICLE_EDITOR::GRID_INTERVALS[i] >=
            game.options.partEd.gridInterval
        ) {
            break;
        }
        newGridInterval = PARTICLE_EDITOR::GRID_INTERVALS[i];
    }
    game.options.partEd.gridInterval = newGridInterval;
    setStatus(
        "Decreased grid interval to " +
        f2s(game.options.partEd.gridInterval) + "."
    );
}


/**
 * @brief Code to run for the grid interval increase command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::gridIntervalIncreaseCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    float newGridInterval = PARTICLE_EDITOR::GRID_INTERVALS.back();
    for(
        int i = (int) (PARTICLE_EDITOR::GRID_INTERVALS.size() - 1);
        i >= 0; --i
    ) {
        if(
            PARTICLE_EDITOR::GRID_INTERVALS[i] <=
            game.options.partEd.gridInterval
        ) {
            break;
        }
        newGridInterval = PARTICLE_EDITOR::GRID_INTERVALS[i];
    }
    game.options.partEd.gridInterval = newGridInterval;
    setStatus(
        "Increased grid interval to " +
        f2s(game.options.partEd.gridInterval) + "."
    );
}


/**
 * @brief Code to run for the grid toggle command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::gridToggleCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    gridVisible = !gridVisible;
    string stateStr = (gridVisible ? "Enabled" : "Disabled");
    setStatus(stateStr + " grid visibility.");
}


/**
 * @brief Code to run for the leader silhouette toggle command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::leaderSilhouetteToggleCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    leaderSilhouetteVisible = !leaderSilhouetteVisible;
    string stateStr = (leaderSilhouetteVisible ? "Enabled" : "Disabled");
    setStatus(stateStr + " leader silhouette visibility.");
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
    CONTENT_LOAD_LEVEL_EDITOR
    );
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    //Misc. setup.
    game.audio.setCurrentSong(game.sysContentNames.sngEditors, false);
    
    partMgr = ParticleManager(game.options.advanced.maxParticles);
    partMgr.viewports.push_back(&game.editorsView);
    
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
    if(!game.quickPlay.areaPath.empty()) {
        loadPartGenFile(game.quickPlay.content, true);
        game.editorsView.cam.setPos(game.quickPlay.camPos);
        game.editorsView.cam.setZoom(game.quickPlay.camZ);
        game.quickPlay.areaPath.clear();
        
    } else if(!autoLoadFile.empty()) {
        loadPartGenFile(autoLoadFile, true);
        
    } else {
        openLoadDialog();
        
    }
}


/**
 * @brief Code to run for the load command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::loadCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        loadWidgetPos,
        "loading a generator", "load",
        std::bind(&ParticleEditor::openLoadDialog, this),
        std::bind(&ParticleEditor::savePartGen, this)
    );
}


/**
 * @brief Loads a particle generator.
 *
 * @param path Path to the file.
 * @param shouldUpdateHistory If true, this loading process should update
 * the user's file open history.
 */
void ParticleEditor::loadPartGenFile(
    const string& path, const bool shouldUpdateHistory
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
            "Could not load the particle generator file \"" +
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
    
    if(shouldUpdateHistory) {
        updateHistory(game.options.partEd.history, manifest, loadedGen.name);
    }
    
    setStatus(
        "Loaded generator \"" + manifest.internalName + "\" successfully."
    );
}


/**
 * @brief Code to run for the open externally command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::openExternallyCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    if(!changesMgr.existsOnDisk()) {
        setStatus("The generator doesn't exist on disk yet!", true);
        return;
    }
    openFileExplorer(manifest.path);
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void ParticleEditor::panCam(const ALLEGRO_EVENT& ev) {
    game.editorsView.cam.setPos(
        Point(
            game.editorsView.cam.pos.x -
            ev.mouse.dx / game.editorsView.cam.zoom,
            game.editorsView.cam.pos.y -
            ev.mouse.dy / game.editorsView.cam.zoom
        )
    );
}


/**
 * @brief Code to run for the particle generator playback toggle command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::partGenPlaybackToggleCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    genRunning = !genRunning;
    string stateStr = (genRunning ? "Enabled" : "Disabled");
    setStatus(stateStr + " particle generator playback.");
}


/**
 * @brief Code to run for the particle manager playback toggle command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::partMgrPlaybackToggleCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    mgrRunning = !mgrRunning;
    string stateStr = (mgrRunning ? "Enabled" : "Disabled");
    setStatus(stateStr + " particle system playback.");
}


/**
 * @brief Callback for when the user picks a file from the picker.
 *
 * @param name Name of the file.
 * @param topCat Unused.
 * @param secCat Unused.
 * @param info Pointer to the file's content manifest.
 * @param isNew Unused.
 */
void ParticleEditor::pickPartGenFile(
    const string& name, const string& topCat, const string& secCat,
    void* info, bool isNew
) {
    ContentManifest* tempManif = (ContentManifest*) info;
    
    auto reallyLoad = [this, tempManif] () {
        closeTopDialog();
        loadPartGenFile(tempManif->path, true);
    };
    
    if(
        tempManif->pack == FOLDER_NAMES::BASE_PACK &&
        !game.options.advanced.engineDev
    ) {
        openBaseContentWarningDialog(reallyLoad);
    } else {
        reallyLoad();
    }
}


/**
 * @brief Code to run for the quick play command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::quickPlayCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    bool areaFound = false;
    for(size_t t = 0; t < 2; t++) {
        for(size_t a = 0; a < game.content.areas.list[t].size(); a++) {
            if(
                game.content.areas.list[t][a]->manifest->path ==
                game.options.partEd.quickPlayAreaPath
            ) {
                areaFound = true;
                break;
            }
        }
    }
    
    if(!areaFound) return;
    
    if(!savePartGen()) return;
    game.quickPlay.areaPath = game.options.partEd.quickPlayAreaPath;
    game.quickPlay.content = manifest.path;
    game.quickPlay.editor = game.states.particleEd;
    game.quickPlay.camPos = game.editorsView.cam.pos;
    game.quickPlay.camZ = game.editorsView.cam.zoom;
    leave();
}


/**
 * @brief Code to run for the quit command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::quitCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
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
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::reloadCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        reloadWidgetPos,
        "reloading the current generator", "reload",
    [this] () { loadPartGenFile(string(manifest.path), false); },
    std::bind(&ParticleEditor::savePartGen, this)
    );
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
 * @brief Resets the camera's X and Y coordinates.
 */
void ParticleEditor::resetCamXY() {
    game.editorsView.cam.targetPos = Point();
}


/**
 * @brief Resets the camera's zoom.
 */
void ParticleEditor::resetCamZoom() {
    zoomWithCursor(1.0f);
}


/**
 * @brief Code to run for the save command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::saveCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    savePartGen();
}


/**
 * @brief Saves the particle generator to the disk.
 *
 * @return Whether it succeeded.
 */
bool ParticleEditor::savePartGen() {
    loadedGen.engineVersion = getEngineVersionString();
    
    DataNode fileNode = DataNode("", "");
    loadedGen.saveToDataNode(&fileNode);
    
    if(!fileNode.saveFile(manifest.path)) {
        showSystemMessageBox(
            nullptr, "Save failed!",
            "Could not save the particle generator!",
            (
                "An error occurred while saving the particle generator "
                "to the file \"" +
                manifest.path + "\". Make sure that the folder it is saving to "
                "exists and it is not read-only, and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        setStatus("Could not save the particle generator!", true);
        return false;
    } else {
        setStatus("Saved generator successfully.");
        changesMgr.markAsSaved();
        updateHistory(game.options.partEd.history, manifest, loadedGen.name);
        return true;
    }
    
    return false;
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
    
    game.editorsView.cam.setPos(Point());
    game.editorsView.cam.setZoom(1.0f);
}


/**
 * @brief Unloads the editor from memory.
 */
void ParticleEditor::unload() {
    Editor::unload();
    
    partMgr.clear();
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
    }
    );
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
}


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::zoomAndPosResetCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    if(game.editorsView.cam.targetZoom == 1.0f) {
        game.editorsView.cam.targetPos = Point();
    } else {
        game.editorsView.cam.targetZoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::zoomInCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    game.editorsView.cam.targetZoom =
        std::clamp(
            game.editorsView.cam.targetZoom +
            game.editorsView.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoomMinLevel, zoomMaxLevel
        );
}


/**
 * @brief Code to run for the zoom out command.
 *
 * @param inputValue Value of the player input for the command.
 */
void ParticleEditor::zoomOutCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    game.editorsView.cam.targetZoom =
        std::clamp(
            game.editorsView.cam.targetZoom -
            game.editorsView.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoomMinLevel, zoomMaxLevel
        );
}
