/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General animation editor-related functions.
 */

#include <algorithm>
#include <queue>

#include "editor.h"

#include "../../core/game.h"
#include "../../core/load.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


using std::queue;

namespace ANIM_EDITOR {

//Threshold for the flood-fill algorithm when picking sprite bitmap parts.
const float FLOOD_FILL_ALPHA_THRESHOLD = 0.008;

//Grid interval in the animation editor.
const float GRID_INTERVAL = 16.0f;

//Minimum radius that a hitbox can have.
const float HITBOX_MIN_RADIUS = 1.0f;

//Amount to pan the camera by when using the keyboard.
const float KEYBOARD_PAN_AMOUNT = 32.0f;

//How tall the animation timeline header is.
const size_t TIMELINE_HEADER_HEIGHT = 12;

//How tall the animation timeline is, in total.
const size_t TIMELINE_HEIGHT = 48;

//Size of each side of the triangle that marks the loop frame.
const size_t TIMELINE_LOOP_TRI_SIZE = 8;

//Pad the left, right, and bottom of the timeline by this much.
const size_t TIMELINE_PADDING = 6;

//Minimum width or height a Pikmin top can have.
const float TOP_MIN_SIZE = 1.0f;

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 32.0f;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.05f;

}


/**
 * @brief Constructs a new animation editor object.
 */
AnimationEditor::AnimationEditor() :
    loadDialogPicker(this) {
    
    comparisonBlinkTimer =
        Timer(
            0.6,
    [this] () {
        this->comparisonBlinkShow = !this->comparisonBlinkShow;
        this->comparisonBlinkTimer.start();
    }
        );
    comparisonBlinkTimer.start();
    
    zoomMinLevel = ANIM_EDITOR::ZOOM_MIN_LEVEL;
    zoomMaxLevel = ANIM_EDITOR::ZOOM_MAX_LEVEL;
    
#define registerCmd(ptr, name) \
    commands.push_back( \
        Command(std::bind((ptr), this, std::placeholders::_1), \
            (name)) \
        );
    
    registerCmd(&AnimationEditor::gridToggleCmd, "grid_toggle");
    registerCmd(&AnimationEditor::hitboxesToggleCmd, "hitboxes_toggle");
    registerCmd(
        &AnimationEditor::leaderSilhouetteToggleCmd,
        "leader_silhouette_toggle"
    );
    registerCmd(&AnimationEditor::deleteAnimDbCmd, "delete_anim_db");
    registerCmd(&AnimationEditor::loadCmd, "load");
    registerCmd(&AnimationEditor::mobRadiusToggleCmd, "mob_radius_toggle");
    registerCmd(&AnimationEditor::playPauseAnimCmd, "play_pause_anim");
    registerCmd(&AnimationEditor::restartAnimCmd, "restart_anim");
    registerCmd(&AnimationEditor::quitCmd, "quit");
    registerCmd(&AnimationEditor::reloadCmd, "reload");
    registerCmd(&AnimationEditor::saveCmd, "save");
    registerCmd(
        &AnimationEditor::zoomAndPosResetCmd, "zoom_and_pos_reset"
    );
    registerCmd(&AnimationEditor::zoomEverythingCmd, "zoom_everything");
    registerCmd(&AnimationEditor::zoomInCmd, "zoom_in");
    registerCmd(&AnimationEditor::zoomOutCmd, "zoom_out");
    
#undef registerCmd
    
}


/**
 * @brief Goes through all sprites that match the given old bitmap properties,
 * and gives them the new properties.
 */
void AnimationEditor::applyChangesToAllMatchingSprites(
    const Point& oldPos, const Point& oldSize,
    const Point& newPos, const Point& newSize
) {
    if(oldPos == newPos && oldSize == newSize) {
        setStatus("No changes to make.");
        return;
    }
    
    size_t spritesAffected = 0;
    for(size_t s = 0; s < db.sprites.size(); s++) {
        Sprite* sPtr = db.sprites[s];
        if(sPtr->bmpPos == oldPos && sPtr->bmpSize == oldSize) {
            sPtr->bmpPos = newPos;
            sPtr->bmpSize = newSize;
            sPtr->setBitmap(sPtr->bmpName, sPtr->bmpPos, sPtr->bmpSize);
            changesMgr.markAsChanged();
            spritesAffected++;
        }
    }
    
    setStatus(
        "Changed " + i2s(spritesAffected) + " other " +
        amountStr(spritesAffected, "sprite", "", true) + "."
    );
}


/**
 * @brief Centers the camera on the sprite's parent bitmap, so the user
 * can choose what part of the bitmap they want to use for the sprite.
 *
 * @param instant If true, change the camera instantly.
 */
void AnimationEditor::centerCameraOnSpriteBitmap(bool instant) {
    if(curSprite && curSprite->parentBmp) {
        Point bmpSize = getBitmapDimensions(curSprite->parentBmp);
        Point bmpPos = 0.0f - bmpSize / 2.0f;
        
        centerCamera(bmpPos, bmpPos + bmpSize);
    } else {
        game.editorsView.cam.targetZoom = 1.0f;
        game.editorsView.cam.targetPos = Point();
    }
    
    if(instant) {
        game.editorsView.cam.pos = game.editorsView.cam.targetPos;
        game.editorsView.cam.zoom = game.editorsView.cam.targetZoom;
    }
    game.editorsView.updateTransformations();
}


/**
 * @brief Changes to a new state, cleaning up whatever is needed.
 *
 * @param newState The new state.
 */
void AnimationEditor::changeState(const EDITOR_STATE newState) {
    comparison = false;
    comparisonSprite = nullptr;
    state = newState;
    setStatus();
    stopSounds();
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void AnimationEditor::closeLoadDialog() {
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
void AnimationEditor::closeOptionsDialog() {
    saveOptions();
}


/**
 * @brief Creates a new, empty animation database.
 *
 * @param path Path to the requested animation database's file.
 */
void AnimationEditor::createAnimDb(const string& path) {
    setupForNewAnimDbPre();
    changesMgr.markAsNonExistent();
    
    manifest.fillFromPath(path);
    db.manifest = &manifest;
    setupForNewAnimDbPost();
    
    setStatus(
        "Created animation database \"" +
        manifest.internalName + "\" successfully."
    );
}


/**
 * @brief Code to run for the delete current animation database command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::deleteAnimDbCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    openDialog(
        "Delete animation database?",
        std::bind(&AnimationEditor::processGuiDeleteAnimDbDialog, this)
    );
    dialogs.back()->customSize = Point(600, 0);
}


/**
 * @brief Deletes the current animation database.
 */
void AnimationEditor::deleteCurrentAnimDb() {
    string origInternalName = manifest.internalName;
    bool goToLoadDialog = true;
    bool success = false;
    string messageBoxText;
    
    if(!changesMgr.existsOnDisk()) {
        //If the database doesn't exist in the disk, since it was never
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
                "Could not delete animation database file \"" + manifest.path +
                "\"! The file was not found!";
            goToLoadDialog = false;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            messageBoxText =
                "Could not delete animation database file \"" + manifest.path +
                "\"! Something went wrong. Please make sure "
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
            setupForNewAnimDbPre();
            openLoadDialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        setStatus(
            "Deleted animation database \"" + origInternalName +
            "\" successfully."
        );
    } else {
        setStatus(
            "Animation database \"" + origInternalName +
            "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(messageBoxText.empty()) {
        finishUp();
    } else {
        openMessageDialog(
            "Animation database deletion failed!",
            messageBoxText,
            finishUp
        );
    }
}


/**
 * @brief Handles the logic part of the main loop of the animation editor.
 */
void AnimationEditor::doLogic() {
    Editor::doLogicPre();
    
    processGui();
    
    if(
        animPlaying && state == EDITOR_STATE_ANIMATION &&
        curAnimInst.validFrame()
    ) {
        Frame* f = &curAnimInst.curAnim->frames[curAnimInst.curFrameIdx];
        if(f->duration != 0) {
            vector<size_t> frameSounds;
            curAnimInst.tick(game.deltaT, nullptr, &frameSounds);
            
            for(size_t s = 0; s < frameSounds.size(); s++) {
                playSound(frameSounds[s]);
            }
        } else {
            animPlaying = false;
        }
    }
    
    curHitboxAlpha += TAU * 1.5 * game.deltaT;
    
    if(comparisonBlink) {
        comparisonBlinkTimer.tick(game.deltaT);
    } else {
        comparisonBlinkShow = true;
    }
    
    Editor::doLogicPost();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn.
 *
 * @param parentList Unused.
 * @param cmd Unused.
 */
void AnimationEditor::drawCanvasDearImGuiCallback(
    const ImDrawList* parentList, const ImDrawCmd* cmd
) {
    game.states.animationEd->drawCanvas();
}


/**
 * @brief Returns the time in the animation in which the mouse cursor is
 * currently located, if the mouse cursor is within the timeline.
 *
 * @return The time.
 */
float AnimationEditor::getCursorTimelineTime() {
    if(!curAnimInst.validFrame()) {
        return 0.0f;
    }
    
    Point canvasTL = game.editorsView.getTopLeft();
    Point canvasBR = game.editorsView.getBottomRight();
    float animX1 = canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING;
    float animW = (canvasBR.x - ANIM_EDITOR::TIMELINE_PADDING) - animX1;
    float mouseX = game.mouseCursor.winPos.x - animX1;
    mouseX = std::clamp(mouseX, 0.0f, animW);
    return curAnimInst.curAnim->getDuration() * (mouseX / animW);
}


/**
 * @brief Returns some tooltip text that represents an animation database
 * file's manifest.
 *
 * @param path Path to the file.
 * @return The tooltip text.
 */
string AnimationEditor::getFileTooltip(const string& path) const {
    if(path.find(FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/") != string::npos) {
        ContentManifest tempManif;
        string cat;
        string type;
        game.content.mobAnimDbs.pathToManifest(
            path, &tempManif, &cat, &type
        );
        return
            "File path: " + path + "\n"
            "Pack: " + game.content.packs.list[tempManif.pack].name + "\n"
            "Mob's internal name: " + type + " (category " + cat + ")";
    } else {
        ContentManifest tempManif;
        game.content.globalAnimDbs.pathToManifest(
            path, &tempManif
        );
        return
            "Internal name: " + tempManif.internalName + "\n"
            "File path: " + path + "\n"
            "Pack: " + game.content.packs.list[tempManif.pack].name;
    }
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string AnimationEditor::getName() const {
    return "animation editor";
}


/**
 * @brief Returns the name to give the current database's entry for the history.
 *
 * @return The name.
 */
string AnimationEditor::getNameForHistory() const {
    if(loadedMobType) {
        return
            loadedMobType->name.empty() ?
            loadedMobType->manifest->internalName :
            loadedMobType->name;
    } else {
        return
            db.name.empty() ?
            manifest.internalName :
            db.name;
    }
}


/**
 * @brief Returns the path to the currently opened content,
 * or an empty string if none.
 *
 * @return The path.
 */
string AnimationEditor::getOpenedContentPath() const {
    return manifest.path;
}


/**
 * @brief Code to run for the grid toggle command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::gridToggleCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    gridVisible = !gridVisible;
    string stateStr = (gridVisible ? "Enabled" : "Disabled");
    setStatus(stateStr + " grid visibility.");
}


/**
 * @brief Code to run for the hitboxes toggle command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::hitboxesToggleCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    hitboxesVisible = !hitboxesVisible;
    string stateStr = (hitboxesVisible ? "Enabled" : "Disabled");
    setStatus(stateStr + " hitbox visibility.");
}


/**
 * @brief Imports the animation data from a different animation to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::importAnimationData(const string& name) {
    Animation* a = db.animations[db.findAnimation(name)];
    
    curAnimInst.curAnim->frames = a->frames;
    curAnimInst.curAnim->hitRate = a->hitRate;
    curAnimInst.curAnim->loopFrame = a->loopFrame;
    
    changesMgr.markAsChanged();
}


/**
 * @brief Imports the sprite bitmap data from a different sprite to the current.
 *
 * @param name Name of the sprite to import from.
 */
void AnimationEditor::importSpriteBmpData(const string& name) {
    Sprite* s = db.sprites[db.findSprite(name)];
    
    curSprite->setBitmap(s->bmpName, s->bmpPos, s->bmpSize);
    
    changesMgr.markAsChanged();
}


/**
 * @brief Imports the sprite hitbox data from a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::importSpriteHitboxData(const string& name) {
    for(size_t s = 0; s < db.sprites.size(); s++) {
        if(db.sprites[s]->name == name) {
            curSprite->hitboxes = db.sprites[s]->hitboxes;
        }
    }
    
    updateCurHitbox();
    
    changesMgr.markAsChanged();
}


/**
 * @brief Imports the sprite top data from a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::importSpriteTopData(const string& name) {
    Sprite* s = db.sprites[db.findSprite(name)];
    curSprite->topVisible = s->topVisible;
    curSprite->topPos = s->topPos;
    curSprite->topSize = s->topSize;
    curSprite->topAngle = s->topAngle;
    
    changesMgr.markAsChanged();
}


/**
 * @brief Imports the sprite transformation data from
 * a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::importSpriteTransformationData(const string& name) {
    Sprite* s = db.sprites[db.findSprite(name)];
    curSprite->offset = s->offset;
    curSprite->scale = s->scale;
    curSprite->angle = s->angle;
    curSprite->tint = s->tint;
    
    changesMgr.markAsChanged();
}


/**
 * @brief Returns whether the mouse cursor is inside the animation
 * timeline or not.
 *
 * @return Whether the cursor is inside.
 */
bool AnimationEditor::isCursorInTimeline() {
    Point canvasTL = game.editorsView.getTopLeft();
    Point canvasBR = game.editorsView.getBottomRight();
    return
        state == EDITOR_STATE_ANIMATION &&
        game.mouseCursor.winPos.x >= canvasTL.x &&
        game.mouseCursor.winPos.x <= canvasBR.x &&
        game.mouseCursor.winPos.y >= canvasBR.y -
        ANIM_EDITOR::TIMELINE_HEIGHT &&
        game.mouseCursor.winPos.y <= canvasBR.y;
}


/**
 * @brief Code to run for the leader silhouette toggle command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::leaderSilhouetteToggleCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    leaderSilhouetteVisible = !leaderSilhouetteVisible;
    string stateStr = (leaderSilhouetteVisible ? "Enabled" : "Disabled");
    setStatus(stateStr + " leader silhouette visibility.");
}


/**
 * @brief Loads the animation editor.
 */
void AnimationEditor::load() {
    Editor::load();
    
    //Load necessary game content.
    game.content.reloadPacks();
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_MOB_TYPE,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    loadCustomMobCatTypes(false);
    
    //Misc. setup.
    sideView = false;
    
    changeState(EDITOR_STATE_MAIN);
    game.audio.setCurrentSong(game.sysContentNames.sngEditors, false);
    
    //Set the background.
    if(!game.options.animEd.bgPath.empty()) {
        bg =
            loadBmp(
                game.options.animEd.bgPath,
                nullptr, false, false, false
            );
        useBg = true;
    } else {
        useBg = false;
    }
    
    //Automatically load a file if needed, or show the load dialog.
    if(!game.quickPlay.areaPath.empty()) {
        loadAnimDbFile(game.quickPlay.content, true);
        game.editorsView.cam.setPos(game.quickPlay.camPos);
        game.editorsView.cam.setZoom(game.quickPlay.camZ);
        game.quickPlay.areaPath.clear();
        
    } else if(!autoLoadFile.empty()) {
        loadAnimDbFile(autoLoadFile, true);
        
    } else {
        openLoadDialog();
        
    }
}


/**
 * @brief Loads an animation database.
 *
 * @param path Path to the file.
 * @param shouldUpdateHistory If true, this loading process should update
 * the user's file open history.
 */
void AnimationEditor::loadAnimDbFile(
    const string& path, bool shouldUpdateHistory
) {
    //Setup.
    setupForNewAnimDbPre();
    changesMgr.markAsNonExistent();
    
    //Load.
    manifest.fillFromPath(path);
    DataNode file = DataNode(manifest.path);
    
    if(!file.fileWasOpened) {
        openMessageDialog(
            "Load failed!",
            "Could not load the animation database file \"" +
            manifest.path + "\"!",
        [this] () { openLoadDialog(); }
        );
        manifest.clear();
        return;
    }
    
    db.manifest = &manifest;
    db.loadFromDataNode(&file);
    
    //Find the most popular file name to suggest for new sprites.
    lastSpritesheetUsed.clear();
    
    if(!db.sprites.empty()) {
        map<string, size_t> fileUsesMap;
        vector<std::pair<size_t, string> > fileUsesVector;
        for(size_t f = 0; f < db.sprites.size(); f++) {
            fileUsesMap[db.sprites[f]->bmpName]++;
        }
        for(auto& u : fileUsesMap) {
            fileUsesVector.push_back(make_pair(u.second, u.first));
        }
        std::sort(
            fileUsesVector.begin(),
            fileUsesVector.end(),
            [] (
                std::pair<size_t, string> u1, std::pair<size_t, string> u2
        ) -> bool {
            return u1.first > u2.first;
        }
        );
        lastSpritesheetUsed = fileUsesVector[0].second;
    }
    
    //Finish up.
    changesMgr.reset();
    setupForNewAnimDbPost();
    if(shouldUpdateHistory) {
        updateHistory(
            game.options.animEd.history, manifest, getNameForHistory()
        );
    }
    
    setStatus(
        "Loaded database \"" + manifest.internalName + "\" successfully."
    );
}


/**
 * @brief Code to run for the load file command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::loadCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        loadWidgetPos,
        "loading a database", "load",
        std::bind(&AnimationEditor::openLoadDialog, this),
        std::bind(&AnimationEditor::saveAnimDb, this)
    );
}


/**
 * @brief Code to run for the mob radius toggle command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::mobRadiusToggleCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    mobRadiusVisible = !mobRadiusVisible;
    string stateStr = (mobRadiusVisible ? "Enabled" : "Disabled");
    setStatus(stateStr + " object radius visibility.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::panCam(const ALLEGRO_EVENT& ev) {
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
 * @brief Callback for when the user picks an animation from the picker.
 *
 * @param name Name of the animation.
 * @param topCat Unused.
 * @param secCat Unused.
 * @param info Unused.
 * @param isNew Is this a new animation or an existing one?
 */
void AnimationEditor::pickAnimation(
    const string& name, const string& topCat, const string& secCat,
    void* info, bool isNew
) {
    if(isNew) {
        db.animations.push_back(new Animation(name));
        db.sortAlphabetically();
        changesMgr.markAsChanged();
        setStatus("Created animation \"" + name + "\".");
    }
    curAnimInst.clear();
    curAnimInst.animDb = &db;
    curAnimInst.curAnim = db.animations[db.findAnimation(name)];
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
void AnimationEditor::pickAnimDbFile(
    const string& name, const string& topCat, const string& secCat,
    void* info, bool isNew
) {
    ContentManifest* tempManif = (ContentManifest*) info;
    string path = tempManif->path;
    auto reallyLoad = [this, path] () {
        closeTopDialog();
        loadAnimDbFile(path, true);
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
 * @brief Callback for when the user picks a sprite from the picker.
 *
 * @param name Name of the sprite.
 * @param topCat Unused.
 * @param secCat Unused.
 * @param info Unused.
 * @param isNew Is this a new sprite or an existing one?
 */
void AnimationEditor::pickSprite(
    const string& name, const string& topCat, const string& secCat,
    void* info, bool isNew
) {
    if(isNew) {
        if(db.findSprite(name) == INVALID) {
            db.sprites.push_back(new Sprite(name));
            db.sprites.back()->createHitboxes(
                &db,
                loadedMobType ? loadedMobType->height : 128,
                loadedMobType ? loadedMobType->radius : 32
            );
            db.sortAlphabetically();
            changesMgr.markAsChanged();
            setStatus("Created sprite \"" + name + "\".");
        }
    }
    curSprite = db.sprites[db.findSprite(name)];
    updateCurHitbox();
    
    if(isNew) {
        //New sprite. Suggest file name.
        curSprite->setBitmap(lastSpritesheetUsed, Point(), Point());
    }
}


/**
 * @brief Code to run for the play/pause animation command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::playPauseAnimCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    if(!curAnimInst.validFrame()) {
        animPlaying = false;
        return;
    }
    
    animPlaying = !animPlaying;
    if(animPlaying) {
        setStatus("Animation playback started.");
    } else {
        setStatus("Animation playback stopped.");
    }
}


/**
 * @brief Plays one of the mob's sounds.
 *
 * @param soundIdx Index of the sound data in the mob type's sound list.
 */
void AnimationEditor::playSound(size_t soundIdx) {
    if(!loadedMobType) return;
    MobType::Sound* soundData = &loadedMobType->sounds[soundIdx];
    if(!soundData->sample) return;
    size_t id =
        game.audio.createUiSoundSource(soundData->sample, soundData->config);
    animSoundIds.push_back(id);
}


/**
 * @brief Code to run for the quick play command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::quickPlayCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    bool areaFound = false;
    for(size_t t = 0; t < 2; t++) {
        for(size_t a = 0; a < game.content.areas.list[t].size(); a++) {
            if(
                game.content.areas.list[t][a]->manifest->path ==
                game.options.animEd.quickPlayAreaPath
            ) {
                areaFound = true;
                break;
            }
        }
    }
    
    if(!areaFound) return;
    
    if(!saveAnimDb()) return;
    game.quickPlay.areaPath = game.options.animEd.quickPlayAreaPath;
    game.quickPlay.content = manifest.path;
    game.quickPlay.editor = game.states.animationEd;
    game.quickPlay.camPos = game.editorsView.cam.pos;
    game.quickPlay.camZ = game.editorsView.cam.zoom;
    leave();
}


/**
 * @brief Code to run for the quit command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::quitCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        quitWidgetPos,
        "quitting", "quit",
    [this] () {
        stopSounds();
        leave();
    },
    std::bind(&AnimationEditor::saveAnimDb, this)
    );
}


/**
 * @brief Reloads all loaded animation databases.
 */
void AnimationEditor::reloadAnimDbs() {
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_MOB_ANIMATION,
    }
    );
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_GLOBAL_ANIMATION,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::reloadCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    if(!changesMgr.existsOnDisk()) return;
    
    changesMgr.askIfUnsaved(
        reloadWidgetPos,
        "reloading the current database", "reload",
    [this] () { loadAnimDbFile(string(manifest.path), false); },
    std::bind(&AnimationEditor::saveAnimDb, this)
    );
}


/**
 * @brief Renames an animation to the given name.
 *
 * @param anim Animation to rename.
 * @param newName Its new name.
 */
void AnimationEditor::renameAnimation(
    Animation* anim, const string& newName
) {
    //Check if it's valid.
    if(!anim) {
        return;
    }
    
    const string oldName = anim->name;
    
    //Check if the name is the same.
    if(newName == oldName) {
        setStatus();
        return;
    }
    
    //Check if the name is empty.
    if(newName.empty()) {
        setStatus("You need to specify the animation's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t a = 0; a < db.animations.size(); a++) {
        if(db.animations[a]->name == newName) {
            setStatus(
                "An animation by the name \"" + newName + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    anim->name = newName;
    
    changesMgr.markAsChanged();
    setStatus(
        "Renamed animation \"" + oldName + "\" to \"" + newName + "\"."
    );
}


/**
 * @brief Renames a body part to the given name.
 *
 * @param part Body part to rename.
 * @param newName Its new name.
 */
void AnimationEditor::renameBodyPart(
    BodyPart* part, const string& newName
) {
    //Check if it's valid.
    if(!part) {
        return;
    }
    
    const string oldName = part->name;
    
    //Check if the name is the same.
    if(newName == oldName) {
        setStatus();
        return;
    }
    
    //Check if the name is empty.
    if(newName.empty()) {
        setStatus("You need to specify the body part's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t b = 0; b < db.bodyParts.size(); b++) {
        if(db.bodyParts[b]->name == newName) {
            setStatus(
                "A body part by the name \"" + newName + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    for(size_t s = 0; s < db.sprites.size(); s++) {
        for(size_t h = 0; h < db.sprites[s]->hitboxes.size(); h++) {
            if(db.sprites[s]->hitboxes[h].bodyPartName == oldName) {
                db.sprites[s]->hitboxes[h].bodyPartName = newName;
            }
        }
    }
    part->name = newName;
    updateHitboxes();
    
    changesMgr.markAsChanged();
    setStatus(
        "Renamed body part \"" + oldName + "\" to \"" + newName + "\"."
    );
}


/**
 * @brief Renames a sprite to the given name.
 *
 * @param spr Sprite to rename.
 * @param newName Its new name.
 */
void AnimationEditor::renameSprite(
    Sprite* spr, const string& newName
) {
    //Check if it's valid.
    if(!spr) {
        return;
    }
    
    const string oldName = spr->name;
    
    //Check if the name is the same.
    if(newName == oldName) {
        setStatus();
        return;
    }
    
    //Check if the name is empty.
    if(newName.empty()) {
        setStatus("You need to specify the sprite's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t s = 0; s < db.sprites.size(); s++) {
        if(db.sprites[s]->name == newName) {
            setStatus(
                "A sprite by the name \"" + newName + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    spr->name = newName;
    for(size_t a = 0; a < db.animations.size(); a++) {
        Animation* aPtr = db.animations[a];
        for(size_t f = 0; f < aPtr->frames.size(); f++) {
            if(aPtr->frames[f].spriteName == oldName) {
                aPtr->frames[f].spriteName = newName;
            }
        }
    }
    
    changesMgr.markAsChanged();
    setStatus(
        "Renamed sprite \"" + oldName + "\" to \"" + newName + "\"."
    );
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void AnimationEditor::resetCamXY() {
    game.editorsView.cam.targetPos = Point();
}


/**
 * @brief Resets the camera's zoom.
 */
void AnimationEditor::resetCamZoom() {
    zoomWithCursor(1.0f);
}


/**
 * @brief Resizes all sprites, hitboxes, etc. by a multiplier.
 *
 * @param mult Multiplier to resize by.
 */
void AnimationEditor::resizeEverything(float mult) {
    if(mult == 0.0f) {
        setStatus("Can't resize everything to size 0!", true);
        return;
    }
    if(mult == 1.0f) {
        setStatus(
            "Resizing everything by 1 wouldn't make a difference!", true
        );
        return;
    }
    
    for(size_t s = 0; s < db.sprites.size(); s++) {
        resizeSprite(db.sprites[s], mult);
    }
    
    changesMgr.markAsChanged();
    setStatus("Resized everything by " + f2s(mult) + ".");
}


/**
 * @brief Resizes a sprite by a multiplier.
 *
 * @param s Sprite to resize.
 * @param mult Multiplier to resize by.
 */
void AnimationEditor::resizeSprite(Sprite* s, float mult) {
    if(mult == 0.0f) {
        setStatus("Can't resize a sprite to size 0!", true);
        return;
    }
    if(mult == 1.0f) {
        setStatus("Resizing a sprite by 1 wouldn't make a difference!", true);
        return;
    }
    
    s->scale *= mult;
    s->offset *= mult;
    s->topPos *= mult;
    s->topSize *= mult;
    
    for(size_t h = 0; h < s->hitboxes.size(); h++) {
        Hitbox* hPtr = &s->hitboxes[h];
        
        hPtr->radius = fabs(hPtr->radius * mult);
        hPtr->pos *= mult;
    }
    
    changesMgr.markAsChanged();
    setStatus("Resized sprite by " + f2s(mult) + ".");
}


/**
 * @brief Code to run for the restart animation command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::restartAnimCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    if(!curAnimInst.validFrame()) {
        animPlaying = false;
        return;
    }
    
    curAnimInst.toStart();
    animPlaying = true;
    setStatus("Animation playback started from the beginning.");
}


/**
 * @brief Saves the animation database onto the mob's file.
 *
 * @return Whether it succeeded.
 */
bool AnimationEditor::saveAnimDb() {
    db.engineVersion = getEngineVersionString();
    db.sortAlphabetically();
    
    DataNode fileNode = DataNode("", "");
    
    db.saveToDataNode(
        &fileNode,
        loadedMobType && loadedMobType->category->id == MOB_CATEGORY_PIKMIN
    );
    
    if(!fileNode.saveFile(manifest.path)) {
        showSystemMessageBox(
            nullptr, "Save failed!",
            "Could not save the animation database!",
            (
                "An error occurred while saving the animation database to "
                "the file \"" + manifest.path + "\". Make sure that the "
                "folder it is saving to exists and it is not read-only, "
                "and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        setStatus("Could not save the animation database!", true);
        return false;
        
    } else {
        setStatus("Saved database successfully.");
        changesMgr.markAsSaved();
        updateHistory(
            game.options.animEd.history, manifest, getNameForHistory()
        );
        return true;
        
    }
}


/**
 * @brief Code to run for the save command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::saveCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    saveAnimDb();
}


/**
 * @brief Sets all sprite scales to the value specified in the textbox.
 *
 * @param scale Value to set the scales to.
 */
void AnimationEditor::setAllSpriteScales(float scale) {
    if(scale == 0) {
        setStatus("The scales can't be 0!", true);
        return;
    }
    
    for(size_t s = 0; s < db.sprites.size(); s++) {
        Sprite* sPtr = db.sprites[s];
        sPtr->scale.x = scale;
        sPtr->scale.y = scale;
    }
    
    changesMgr.markAsChanged();
    setStatus("Set all sprite scales to " + f2s(scale) + ".");
}


/**
 * @brief Sets the current frame to be the most apt sprite it can find,
 * given the current circumstances.
 *
 * Basically, it picks a sprite that's called something similar to
 * the current animation.
 */
void AnimationEditor::setBestFrameSprite() {
    if(db.sprites.empty()) return;
    
    //Find the sprites that match the most characters with the animation name.
    //Let's set the starting best score to 3, as an arbitrary way to
    //sift out results that technically match, but likely aren't the same
    //term. Example: If the animation is called "running", and there is no
    //"runnning" sprite, we probably don't want a match with "rummaging".
    //Unless it's the exact same word.
    //Also, set the final sprite index to 0 so that if something goes wrong,
    //we default to the first sprite on the list.
    size_t finalSpriteIdx = 0;
    vector<size_t> bestSpriteIdxs;
    
    if(db.sprites.size() > 1) {
        size_t bestScore = 3;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            size_t score = 0;
            if(
                strToLower(curAnimInst.curAnim->name) ==
                strToLower(db.sprites[s]->name)
            ) {
                score = 9999;
            } else {
                score =
                    getMatchingStringStarts(
                        strToLower(curAnimInst.curAnim->name),
                        strToLower(db.sprites[s]->name)
                    ).size();
            }
            
            if(score < bestScore) {
                continue;
            }
            if(score > bestScore) {
                bestScore = score;
                bestSpriteIdxs.clear();
            }
            bestSpriteIdxs.push_back(s);
        }
    }
    
    if(bestSpriteIdxs.size() == 1) {
        //If there's only one best match, go for it.
        finalSpriteIdx = bestSpriteIdxs[0];
        
    } else if(bestSpriteIdxs.size() > 1) {
        //Sort them alphabetically and pick the first.
        std::sort(
            bestSpriteIdxs.begin(),
            bestSpriteIdxs.end(),
        [this, &bestSpriteIdxs] (size_t s1, size_t s2) {
            return
                strToLower(db.sprites[s1]->name) <
                strToLower(db.sprites[s2]->name);
        });
        finalSpriteIdx = bestSpriteIdxs[0];
    }
    
    //Finally, set the frame info then.
    Frame* curFramePtr =
        &curAnimInst.curAnim->frames[curAnimInst.curFrameIdx];
    curFramePtr->spriteIdx = finalSpriteIdx;
    curFramePtr->spritePtr = db.sprites[finalSpriteIdx];
    curFramePtr->spriteName = db.sprites[finalSpriteIdx]->name;
}


/**
 * @brief Sets up the editor for a new animation database,
 * be it from an existing file or from scratch, after the actual creation/load
 * takes place.
 */
void AnimationEditor::setupForNewAnimDbPost() {
    if(
        manifest.path.find(FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/") !=
        string::npos
    ) {
        vector<string> pathParts = split(manifest.path, "/");
        if(
            pathParts.size() > 3 &&
            pathParts[pathParts.size() - 1] == FILE_NAMES::MOB_TYPE_ANIMATION
        ) {
            MobCategory* cat =
                game.mobCategories.getFromFolderName(
                    pathParts[pathParts.size() - 3]
                );
            if(cat) {
                loadedMobType =
                    cat->getType(pathParts[pathParts.size() - 2]);
            }
        }
    }
    
    //Top bitmaps.
    for(unsigned char t = 0; t < N_MATURITIES; t++) {
        if(topBmp[t]) topBmp[t] = nullptr;
    }
    
    if(
        loadedMobType &&
        loadedMobType->category->id == MOB_CATEGORY_PIKMIN
    ) {
        for(size_t m = 0; m < N_MATURITIES; m++) {
            topBmp[m] = ((PikminType*) loadedMobType)->bmpTop[m];
        }
    }
    
    if(loadedMobType && db.name == "animations") {
        //Let's give it a proper default name, instead of the internal name
        //in the manifest, which is just "animations".
        db.name = loadedMobType->name + " animations";
    }
    
    if(loadedMobType) db.fillSoundIdxCaches(loadedMobType);
}


/**
 * @brief Sets up the editor for a new animation database,
 * be it from an existing file or from scratch, before the actual creation/load
 * takes place.
 */
void AnimationEditor::setupForNewAnimDbPre() {
    game.editorsView.updateTransformations();
    
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        //Ideally, states would be handled by a state machine, and this
        //logic would be placed in the sprite bitmap state's "on exit" code...
        game.editorsView.cam.setPos(preSpriteBmpCamPos);
        game.editorsView.cam.setZoom(preSpriteBmpCamZoom);
    }
    
    db.destroy();
    curAnimInst.clear();
    manifest.clear();
    animPlaying = false;
    curSprite = nullptr;
    curHitbox = nullptr;
    curHitboxIdx = 0;
    loadedMobType = nullptr;
    
    game.editorsView.cam.setPos(Point());
    game.editorsView.cam.setZoom(1.0f);
    changeState(EDITOR_STATE_MAIN);
    
    //At this point we'll have nearly unloaded stuff like the current sprite.
    //Since Dear ImGui still hasn't rendered the current frame, which could
    //have had those assets visible, if it tries now it'll crash. So skip.
    game.skipDearImGuiFrame = true;
}


/**
 * @brief Performs a flood fill on the bitmap sprite, to see what parts
 * contain non-alpha pixels, based on a starting position.
 *
 * @param bmp Locked bitmap to check.
 * @param selectionPixels Array that controls which pixels are selected or not.
 * @param x X coordinate to start on.
 * @param y Y coordinate to start on.
 */
void AnimationEditor::spriteBmpFloodFill(
    ALLEGRO_BITMAP* bmp, bool* selectionPixels, int x, int y
) {
    //https://en.wikipedia.org/wiki/Flood_fill#The_algorithm
    int bmpW = al_get_bitmap_width(bmp);
    int bmpH = al_get_bitmap_height(bmp);
    
    if(x < 0 || x > bmpW) return;
    if(y < 0 || y > bmpH) return;
    if(selectionPixels[y * bmpW + x]) return;
    if(al_get_pixel(bmp, x, y).a < ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD) {
        return;
    }
    
    /**
     * @brief A point, but with integer coordinates.
     */
    struct IntPoint {
    
        //--- Members ---
        
        //X coordinate.
        int x = 0;
        
        //Y coordinate.
        int y = 0;
        
        
        //--- Function definitions ---
        
        /**
         * @brief Constructs a new int point object.
         *
         * @param p The float point coordinates.
         */
        explicit IntPoint(const Point& p) :
            x(p.x),
            y(p.y) { }
            
        /**
         * @brief Constructs a new int point object.
         *
         * @param x X coordinate.
         * @param y Y coordinate.
         */
        IntPoint(int x, int y) :
            x(x),
            y(y) { }
            
    };
    
    queue<IntPoint> pixelsLeft;
    pixelsLeft.push(IntPoint(x, y));
    
    while(!pixelsLeft.empty()) {
        IntPoint p = pixelsLeft.front();
        pixelsLeft.pop();
        
        if(
            selectionPixels[(p.y) * bmpW + p.x] ||
            al_get_pixel(bmp, p.x, p.y).a <
            ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
        ) {
            continue;
        }
        
        IntPoint offset = p;
        vector<IntPoint> columns;
        columns.push_back(p);
        
        bool add = true;
        //Keep going left and adding columns to check.
        while(add) {
            if(offset.x  == 0) {
                add = false;
            } else {
                offset.x--;
                if(selectionPixels[offset.y * bmpW + offset.x]) {
                    add = false;
                } else if(
                    al_get_pixel(bmp, offset.x, offset.y).a <
                    ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
                ) {
                    add = false;
                } else {
                    columns.push_back(offset);
                }
            }
        }
        offset = p;
        add = true;
        //Keep going right and adding columns to check.
        while(add) {
            if(offset.x == bmpW - 1) {
                add = false;
            } else {
                offset.x++;
                if(selectionPixels[offset.y * bmpW + offset.x]) {
                    add = false;
                } else if(
                    al_get_pixel(bmp, offset.x, offset.y).a <
                    ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
                ) {
                    add = false;
                } else {
                    columns.push_back(offset);
                }
            }
        }
        
        for(size_t c = 0; c < columns.size(); c++) {
            //For each column obtained, mark the pixel there,
            //and check the pixels above and below, to see if they should be
            //processed next.
            IntPoint col = columns[c];
            selectionPixels[col.y * bmpW + col.x] = true;
            if(
                col.y > 0 &&
                !selectionPixels[(col.y - 1) * bmpW + col.x] &&
                al_get_pixel(bmp, col.x, col.y - 1).a >=
                ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixelsLeft.push(IntPoint(col.x, col.y - 1));
            }
            if(
                col.y < bmpH - 1 &&
                !selectionPixels[(col.y + 1) * bmpW + col.x] &&
                al_get_pixel(bmp, col.x, col.y + 1).a >=
                ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixelsLeft.push(IntPoint(col.x, col.y + 1));
            }
        }
    }
}


/**
 * @brief Stops all of the mob's sounds that are playing.
 */
void AnimationEditor::stopSounds() {
    for(size_t s = 0; s < animSoundIds.size(); s++) {
        game.audio.destroySoundSource(animSoundIds[s]);
    }
    animSoundIds.clear();
}


/**
 * @brief Unloads the editor from memory.
 */
void AnimationEditor::unload() {
    Editor::unload();
    
    db.destroy();
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MOB_TYPE,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_AREA,
    }
    );
    
    if(bg) {
        al_destroy_bitmap(bg);
        bg = nullptr;
    }
}


/**
 * @brief Updates the current hitbox pointer to match the same body part as
 * before, but on the hitbox of the current sprite. If not applicable,
 * it chooses a valid hitbox.
 *
 */
void AnimationEditor::updateCurHitbox() {
    if(curSprite->hitboxes.empty()) {
        curHitbox = nullptr;
        curHitboxIdx = INVALID;
        return;
    }
    
    curHitboxIdx = std::min(curHitboxIdx, curSprite->hitboxes.size() - 1);
    curHitbox = &curSprite->hitboxes[curHitboxIdx];
}


/**
 * @brief Update every frame's hitbox instances in light of new hitbox info.
 */
void AnimationEditor::updateHitboxes() {
    for(size_t s = 0; s < db.sprites.size(); s++) {
    
        Sprite* sPtr = db.sprites[s];
        
        //Start by deleting non-existent hitboxes.
        for(size_t h = 0; h < sPtr->hitboxes.size();) {
            string hName = sPtr->hitboxes[h].bodyPartName;
            bool nameFound = false;
            
            for(size_t b = 0; b < db.bodyParts.size(); b++) {
                if(db.bodyParts[b]->name == hName) {
                    nameFound = true;
                    break;
                }
            }
            
            if(!nameFound) {
                sPtr->hitboxes.erase(
                    sPtr->hitboxes.begin() + h
                );
            } else {
                h++;
            }
        }
        
        //Add missing hitboxes.
        for(size_t b = 0; b < db.bodyParts.size(); b++) {
            bool hitboxFound = false;
            const string& name = db.bodyParts[b]->name;
            
            for(size_t h = 0; h < sPtr->hitboxes.size(); h++) {
                if(sPtr->hitboxes[h].bodyPartName == name) {
                    hitboxFound = true;
                    break;
                }
            }
            
            if(!hitboxFound) {
                sPtr->hitboxes.push_back(
                    Hitbox(
                        name, INVALID, nullptr, Point(), 0,
                        loadedMobType ? loadedMobType->height : 128,
                        loadedMobType ? loadedMobType->radius : 32
                    )
                );
            }
        }
        
        //Sort them with the new order.
        std::sort(
            sPtr->hitboxes.begin(),
            sPtr->hitboxes.end(),
        [this] (const Hitbox & h1, const Hitbox & h2) -> bool {
            size_t pos1 = 0, pos2 = 1;
            for(size_t b = 0; b < db.bodyParts.size(); b++) {
                if(db.bodyParts[b]->name == h1.bodyPartName) pos1 = b;
                if(db.bodyParts[b]->name == h2.bodyPartName) pos2 = b;
            }
            return pos1 < pos2;
        }
        );
    }
}


/**
 * @brief Code to run when the zoom and position reset button widget is pressed.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::zoomAndPosResetCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    if(game.editorsView.cam.targetZoom == 1.0f) {
        game.editorsView.cam.targetPos = Point();
    } else {
        game.editorsView.cam.targetZoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom everything command.
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::zoomEverythingCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    Sprite* sPtr = curSprite;
    if(!sPtr && curAnimInst.validFrame()) {
        const string& name =
            curAnimInst.curAnim->frames[curAnimInst.curFrameIdx].spriteName;
        size_t sPos = db.findSprite(name);
        if(sPos != INVALID) sPtr = db.sprites[sPos];
    }
    if(!sPtr || !sPtr->bitmap) return;
    
    Point cmin, cmax;
    getTransformedRectangleBBox(
        sPtr->offset, sPtr->bmpSize * sPtr->scale,
        sPtr->angle, &cmin, &cmax
    );
    
    if(sPtr->topVisible) {
        Point topMin, topMax;
        getTransformedRectangleBBox(
            sPtr->topPos, sPtr->topSize,
            sPtr->topAngle,
            &topMin, &topMax
        );
        updateMinCoords(cmin, topMin);
        updateMaxCoords(cmax, topMax);
    }
    
    for(size_t h = 0; h < sPtr->hitboxes.size(); h++) {
        Hitbox* hPtr = &sPtr->hitboxes[h];
        updateMinCoords(cmin, hPtr->pos - hPtr->radius);
        updateMaxCoords(cmax, hPtr->pos + hPtr->radius);
    }
    
    centerCamera(cmin, cmax);
}


/**
 * @brief Code to run for the zoom in command
 *
 * @param inputValue Value of the player input for the command.
 */
void AnimationEditor::zoomInCmd(float inputValue) {
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
void AnimationEditor::zoomOutCmd(float inputValue) {
    if(inputValue < 0.5f) return;
    
    game.editorsView.cam.targetZoom =
        std::clamp(
            game.editorsView.cam.targetZoom -
            game.editorsView.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoomMinLevel, zoomMaxLevel
        );
}
