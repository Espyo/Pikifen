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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
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
    loadDialogPicker(this) {
    
    zoomMaxLevel = GUI_EDITOR::ZOOM_MAX_LEVEL;
    zoomMinLevel = GUI_EDITOR::ZOOM_MIN_LEVEL;
    
#define registerCmd(ptr, name) \
    commands.push_back( \
                        Command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    registerCmd(
        &GuiEditor::gridIntervalDecreaseCmd, "grid_interval_decrease"
    );
    registerCmd(
        &GuiEditor::gridIntervalIncreaseCmd, "grid_interval_increase"
    );
    registerCmd(&GuiEditor::deleteGuiDefCmd, "delete_gui_def");
    registerCmd(&GuiEditor::loadCmd, "load");
    registerCmd(&GuiEditor::quitCmd, "quit");
    registerCmd(&GuiEditor::reloadCmd, "reload");
    registerCmd(&GuiEditor::saveCmd, "save");
    registerCmd(&GuiEditor::snapModeCmd, "snap_mode");
    registerCmd(&GuiEditor::zoomAndPosResetCmd, "zoom_and_pos_reset");
    registerCmd(&GuiEditor::zoomInCmd, "zoom_in");
    registerCmd(&GuiEditor::zoomOutCmd, "zoom_out");
    
#undef registerCmd
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void GuiEditor::closeLoadDialog() {
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
void GuiEditor::closeOptionsDialog() {
    saveOptions();
}


/**
 * @brief Creates a new GUI definition, with the data from an existing
 * one in the base pack.
 *
 * @param internalName Internal name of the GUI definition.
 * @param pack The existing pack's internal name.
 */
void GuiEditor::createGuiDef(
    const string &internalName, const string &pack
) {
    //Load the base pack one first.
    ContentManifest tempOrigMan;
    tempOrigMan.internalName = internalName;
    tempOrigMan.pack = FOLDER_NAMES::BASE_PACK;
    string origPath =
        game.content.guiDefs.manifestToPath(tempOrigMan);
        
    loadGuiDefFile(origPath, false);
    
    //Change the manifest under the hood so it's pointing to the new one.
    manifest.pack = pack;
    manifest.path = game.content.guiDefs.manifestToPath(manifest);
    
    changesMgr.markAsNonExistent();
    
    setStatus(
        "Created GUI definition \"" +
        manifest.internalName + "\" successfully."
    );
}


/**
 * @brief Deletes the current GUI definition.
 */
void GuiEditor::deleteCurrentGuiDef() {
    string origInternalName = manifest.internalName;
    bool goToLoadDialog = true;
    bool success = false;
    string messageBoxText;
    
    if(!changesMgr.existsOnDisk()) {
        //If the definition doesn't exist on disk, since it was never
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
                "GUI definition \"" + origInternalName +
                "\" deletion failed! The file was not found!";
            goToLoadDialog = false;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            messageBoxText =
                "GUI definition \"" + origInternalName +
                "\" deletion failed! Something went wrong. Please make sure "
                "there are enough permissions to delete the file and "
                "try again.";
            goToLoadDialog = false;
            break;
        }
        }
        
    }
    
    //This code will be run after everything is done, be it after the standard
    //procedure, or after the user hits OK on the message box.
    const auto finishUp = [this, goToLoadDialog] () {
        if(goToLoadDialog) {
            setupForNewGuiDef();
            openLoadDialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        setStatus(
            "Deleted GUI definition \"" + origInternalName +
            "\" successfully."
        );
    } else {
        setStatus(
            "GUI definition \"" + origInternalName +
            "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(messageBoxText.empty()) {
        finishUp();
    } else {
        openMessageDialog(
            "GUI definition deletion failed!",
            messageBoxText,
            finishUp
        );
    }
}


/**
 * @brief Code to run for the delete current GUI definition command.
 *
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::deleteGuiDefCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
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
    dialogs.back()->customSize = Point(600, 0);
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
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-window.
 *
 * @param parentList Unused.
 * @param cmd Unused.
 */
void GuiEditor::drawCanvasDearImGuiCallback(
    const ImDrawList* parentList, const ImDrawCmd* cmd
) {
    game.states.guiEd->drawCanvas();
}


/**
 * @brief Returns some tooltip text that represents a GUI definition
 * file's manifest.
 *
 * @param path Path to the file.
 * @return The tooltip text.
 */
string GuiEditor::getFileTooltip(const string &path) const {
    ContentManifest tempManif;
    game.content.guiDefs.pathToManifest(
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
    mustRecenterCam = true;
    
    game.audio.setCurrentSong(game.sysContentNames.sngEditors, false);
    
    //Automatically load a file if needed, or show the load dialog.
    if(!autoLoadFile.empty()) {
        loadGuiDefFile(autoLoadFile, true);
    } else {
        openLoadDialog();
    }
}


/**
 * @brief Loads a GUI definition file.
 *
 * @param path Path to the file.
 * @param shouldUpdateHistory If true, this loading process should update
 * the user's file open history.
 */
void GuiEditor::loadGuiDefFile(
    const string &path, bool shouldUpdateHistory
) {
    //Setup.
    setupForNewGuiDef();
    changesMgr.markAsNonExistent();
    
    //Load.
    manifest.fillFromPath(path);
    fileNode = DataNode(manifest.path);
    
    if(!fileNode.fileWasOpened) {
        openMessageDialog(
            "Load failed!",
            "Failed to load the GUI definition file \"" + manifest.path + "\"!",
        [this] () { openLoadDialog(); }
        );
        manifest.clear();
        return;
    }
    
    DataNode* positionsNode = fileNode.getChildByName("positions");
    size_t nItems = positionsNode->getNrOfChildren();
    
    for(size_t i = 0; i < nItems; i++) {
        Item newItem;
        DataNode* itemNode = positionsNode->getChild(i);
        newItem.name = itemNode->name;
        vector<string> words = split(itemNode->value);
        if(words.size() != 4) continue;
        newItem.center.x = s2f(words[0]);
        newItem.center.y = s2f(words[1]);
        newItem.size.x = s2f(words[2]);
        newItem.size.y = s2f(words[3]);
        items.push_back(newItem);
    }
    
    //Finish up.
    changesMgr.reset();
    if(shouldUpdateHistory) {
        updateHistory(game.options.guiEd.history, manifest, "");
    }
    setStatus("Loaded file \"" + manifest.internalName + "\" successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void GuiEditor::panCam(const ALLEGRO_EVENT &ev) {
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
 * @brief Callback for when the user picks a file from the picker.
 *
 * @param name Name of the file.
 * @param topCat Unused.
 * @param secCat Unused.
 * @param info Pointer to the file's content manifest.
 * @param isNew Unused.
 */
void GuiEditor::pickGuiDefFile(
    const string &name, const string &topCat, const string &secCat,
    void* info, bool isNew
) {
    ContentManifest* tempManif = (ContentManifest*) info;
    
    auto reallyLoad = [this, tempManif] () {
        closeTopDialog();
        loadGuiDefFile(tempManif->path, true);
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
 * @brief Code to run for the grid interval decrease command.
 *
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::gridIntervalDecreaseCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    float newGridInterval = GUI_EDITOR::GRID_INTERVALS[0];
    for(size_t i = 0; i < GUI_EDITOR::GRID_INTERVALS.size(); i++) {
        if(
            GUI_EDITOR::GRID_INTERVALS[i] >=
            game.options.guiEd.gridInterval
        ) {
            break;
        }
        newGridInterval = GUI_EDITOR::GRID_INTERVALS[i];
    }
    game.options.guiEd.gridInterval = newGridInterval;
    setStatus(
        "Decreased grid interval to " +
        f2s(game.options.guiEd.gridInterval) + "."
    );
}


/**
 * @brief Code to run for the grid interval increase command.
 *
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::gridIntervalIncreaseCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    float newGridInterval = GUI_EDITOR::GRID_INTERVALS.back();
    for(int i = (int) (GUI_EDITOR::GRID_INTERVALS.size() - 1); i >= 0; --i) {
        if(
            GUI_EDITOR::GRID_INTERVALS[i] <=
            game.options.guiEd.gridInterval
        ) {
            break;
        }
        newGridInterval = GUI_EDITOR::GRID_INTERVALS[i];
    }
    game.options.guiEd.gridInterval = newGridInterval;
    setStatus(
        "Increased grid interval to " +
        f2s(game.options.guiEd.gridInterval) + "."
    );
}


/**
 * @brief Code to run for the load command.
 *
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::loadCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        loadWidgetPos,
        "loading a file", "load",
        std::bind(&GuiEditor::openLoadDialog, this),
        std::bind(&GuiEditor::saveGuiDef, this)
    );
}


/**
 * @brief Code to run for the quit command.
 *
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::quitCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        quitWidgetPos,
        "quitting", "quit",
        std::bind(&GuiEditor::leave, this),
        std::bind(&GuiEditor::saveGuiDef, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::reloadCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    if(!changesMgr.existsOnDisk()) return;
    
    changesMgr.askIfUnsaved(
        reloadWidgetPos,
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
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::saveCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    if(!saveGuiDef()) {
        return;
    }
}


/**
 * @brief Code to run for the snap mode command.
 *
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::snapModeCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    game.options.guiEd.snap = !game.options.guiEd.snap;
    string finalStatusText = "Set snap mode to ";
    if(game.options.guiEd.snap) {
        finalStatusText += "nothing";
    } else {
        finalStatusText += "grid";
    }
    finalStatusText += ".";
    setStatus(finalStatusText);
}


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::zoomAndPosResetCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    resetCam(false);
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param inputValue Value of the player input for the command.
 */
void GuiEditor::zoomInCmd(float inputValue) {
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
void GuiEditor::zoomOutCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    game.editorsView.cam.targetZoom =
        std::clamp(
            game.editorsView.cam.targetZoom -
            game.editorsView.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoomMinLevel, zoomMaxLevel
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
    DataNode* positionsNode = fileNode.getChildByName("positions");
    for(size_t i = 0; i < items.size(); i++) {
        DataNode* itemNode = positionsNode->getChild(i);
        itemNode->value = p2s(items[i].center) + " " + p2s(items[i].size);
    }
    
    if(!fileNode.saveFile(manifest.path)) {
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
        changesMgr.markAsSaved();
        return true;
    }
    
    updateHistory(game.options.guiEd.history, manifest, "");
}


/**
 * @brief Sets up the editor for a new GUI definition,
 * be it from an existing file or from scratch.
 */
void GuiEditor::setupForNewGuiDef() {
    manifest.clear();
    items.clear();
    curItem = INVALID;
    
    //We could reset the camera directly, but if the player enters the editor
    //via the auto start maker tool, processGui() won't have a chance
    //to run before we load the file, and that function is what gives
    //us the canvas coordinates necessary for camera centering.
    //Let's flag the need for recentering so it gets handled when possible.
    mustRecenterCam = true;
}


/**
 * @brief Snaps a point to the nearest available grid spot,
 * or keeps the point as is if Shift is pressed.
 *
 * @param p Point to snap.
 * @return The snapped point.
 */
Point GuiEditor::snapPoint(const Point &p) {
    Point finalPoint = p;
    bool doSnap = game.options.guiEd.snap;
    
    if(isCtrlPressed) {
        if(curTransformationWidget.isMovingCenterHandle()) {
            finalPoint =
                snapPointToAxis(
                    finalPoint, curTransformationWidget.getOldCenter()
                );
        }
    }
    
    if(isShiftPressed) {
        doSnap = !doSnap;
    }
    
    if(!doSnap) {
        return finalPoint;
    }
    
    return
        Point(
            round(finalPoint.x / game.options.guiEd.gridInterval) *
            game.options.guiEd.gridInterval,
            round(finalPoint.y / game.options.guiEd.gridInterval) *
            game.options.guiEd.gridInterval
        );
}


/**
 * @brief Unloads the editor from memory.
 */
void GuiEditor::unload() {
    Editor::unload();
    
    items.clear();
    curItem = INVALID;
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    }
    );
}
