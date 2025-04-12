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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
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
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        Command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(&AnimationEditor::gridToggleCmd, "grid_toggle");
    register_cmd(&AnimationEditor::hitboxesToggleCmd, "hitboxes_toggle");
    register_cmd(
        &AnimationEditor::leaderSilhouetteToggleCmd,
        "leader_silhouette_toggle"
    );
    register_cmd(&AnimationEditor::deleteAnimDbCmd, "delete_anim_db");
    register_cmd(&AnimationEditor::loadCmd, "load");
    register_cmd(&AnimationEditor::mobRadiusToggleCmd, "mob_radius_toggle");
    register_cmd(&AnimationEditor::playPauseAnimCmd, "play_pause_anim");
    register_cmd(&AnimationEditor::restartAnimCmd, "restart_anim");
    register_cmd(&AnimationEditor::quitCmd, "quit");
    register_cmd(&AnimationEditor::reloadCmd, "reload");
    register_cmd(&AnimationEditor::saveCmd, "save");
    register_cmd(
        &AnimationEditor::zoomAndPosResetCmd, "zoom_and_pos_reset"
    );
    register_cmd(&AnimationEditor::zoomEverythingCmd, "zoom_everything");
    register_cmd(&AnimationEditor::zoomInCmd, "zoom_in");
    register_cmd(&AnimationEditor::zoomOutCmd, "zoom_out");
    
#undef register_cmd
    
}


/**
 * @brief Centers the camera on the sprite's parent bitmap, so the user
 * can choose what part of the bitmap they want to use for the sprite.
 *
 * @param instant If true, change the camera instantly.
 */
void AnimationEditor::centerCameraOnSpriteBitmap(bool instant) {
    if(curSprite && curSprite->parentBmp) {
        Point bmp_size = getBitmapDimensions(curSprite->parentBmp);
        Point bmp_pos = 0.0f - bmp_size / 2.0f;
        
        centerCamera(bmp_pos, bmp_pos + bmp_size);
    } else {
        game.cam.targetZoom = 1.0f;
        game.cam.targetPos = Point();
    }
    
    if(instant) {
        game.cam.pos = game.cam.targetPos;
        game.cam.zoom = game.cam.targetZoom;
    }
    updateTransformations();
}


/**
 * @brief Changes to a new state, cleaning up whatever is needed.
 *
 * @param new_state The new state.
 */
void AnimationEditor::changeState(const EDITOR_STATE new_state) {
    comparison = false;
    comparisonSprite = nullptr;
    state = new_state;
    setStatus();
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
void AnimationEditor::createAnimDb(const string &path) {
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
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::deleteAnimDbCmd(float input_value) {
    if(input_value < 0.5f) return;
    
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
    string orig_internal_name = manifest.internalName;
    bool go_to_load_dialog = true;
    bool success = false;
    string message_box_text;
    
    if(!changesMgr.existsOnDisk()) {
        //If the database doesn't exist on disk, since it was never
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
                "Animation database \"" + orig_internal_name +
                "\" deletion failed! The file was not found!";
            go_to_load_dialog = false;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            message_box_text =
                "Animation database \"" + orig_internal_name +
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
            setupForNewAnimDbPre();
            openLoadDialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        setStatus(
            "Deleted animation database \"" + orig_internal_name +
            "\" successfully."
        );
    } else {
        setStatus(
            "Animation database \"" + orig_internal_name +
            "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(message_box_text.empty()) {
        finish_up();
    } else {
        openMessageDialog(
            "Animation database deletion failed!",
            message_box_text,
            finish_up
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
            vector<size_t> frame_sounds;
            curAnimInst.tick(game.deltaT, nullptr, &frame_sounds);
            
            for(size_t s = 0; s < frame_sounds.size(); s++) {
                playSound(frame_sounds[s]);
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
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void AnimationEditor::drawCanvasDearImGuiCallback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
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
    float anim_x1 = canvasTL.x + ANIM_EDITOR::TIMELINE_PADDING;
    float anim_w = (canvasBR.x - ANIM_EDITOR::TIMELINE_PADDING) - anim_x1;
    float mouse_x = game.mouseCursor.sPos.x - anim_x1;
    mouse_x = std::clamp(mouse_x, 0.0f, anim_w);
    return curAnimInst.curAnim->getDuration() * (mouse_x / anim_w);
}


/**
 * @brief Returns some tooltip text that represents an animation database
 * file's manifest.
 *
 * @param path Path to the file.
 * @return The tooltip text.
 */
string AnimationEditor::getFileTooltip(const string &path) const {
    if(path.find(FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/") != string::npos) {
        ContentManifest temp_manif;
        string cat;
        string type;
        game.content.mobAnimDbs.pathToManifest(
            path, &temp_manif, &cat, &type
        );
        return
            "File path: " + path + "\n"
            "Pack: " + game.content.packs.list[temp_manif.pack].name + "\n"
            "Mob's internal name: " + type + " (category " + cat + ")";
    } else {
        ContentManifest temp_manif;
        game.content.globalAnimDbs.pathToManifest(
            path, &temp_manif
        );
        return
            "Internal name: " + temp_manif.internalName + "\n"
            "File path: " + path + "\n"
            "Pack: " + game.content.packs.list[temp_manif.pack].name;
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
 * @brief Imports the animation data from a different animation to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::importAnimationData(const string &name) {
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
void AnimationEditor::importSpriteBmpData(const string &name) {
    Sprite* s = db.sprites[db.findSprite(name)];
    
    curSprite->setBitmap(s->bmpName, s->bmpPos, s->bmpSize);
    
    changesMgr.markAsChanged();
}


/**
 * @brief Imports the sprite hitbox data from a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::importSpriteHitboxData(const string &name) {
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
void AnimationEditor::importSpriteTopData(const string &name) {
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
void AnimationEditor::importSpriteTransformationData(const string &name) {
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
    return
        state == EDITOR_STATE_ANIMATION &&
        game.mouseCursor.sPos.x >= canvasTL.x &&
        game.mouseCursor.sPos.x <= canvasBR.x &&
        game.mouseCursor.sPos.y >= canvasBR.y -
        ANIM_EDITOR::TIMELINE_HEIGHT &&
        game.mouseCursor.sPos.y <= canvasBR.y;
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
    if(!autoLoadFile.empty()) {
        loadAnimDbFile(autoLoadFile, true);
    } else {
        openLoadDialog();
    }
}


/**
 * @brief Loads an animation database.
 *
 * @param path Path to the file.
 * @param should_update_history If true, this loading process should update
 * the user's file open history.
 */
void AnimationEditor::loadAnimDbFile(
    const string &path, bool should_update_history
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
            "Failed to load the animation database file \"" +
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
        map<string, size_t> file_uses_map;
        vector<std::pair<size_t, string> > file_uses_vector;
        for(size_t f = 0; f < db.sprites.size(); f++) {
            file_uses_map[db.sprites[f]->bmpName]++;
        }
        for(auto &u : file_uses_map) {
            file_uses_vector.push_back(make_pair(u.second, u.first));
        }
        std::sort(
            file_uses_vector.begin(),
            file_uses_vector.end(),
            [] (
                std::pair<size_t, string> u1, std::pair<size_t, string> u2
        ) -> bool {
            return u1.first > u2.first;
        }
        );
        lastSpritesheetUsed = file_uses_vector[0].second;
    }
    
    //Finish up.
    changesMgr.reset();
    setupForNewAnimDbPost();
    if(should_update_history) {
        updateHistory(game.options.animEd.history, manifest, getNameForHistory());
    }
    
    setStatus("Loaded file \"" + manifest.internalName + "\" successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::panCam(const ALLEGRO_EVENT &ev) {
    game.cam.setPos(
        Point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/**
 * @brief Callback for when the user picks an animation from the picker.
 *
 * @param name Name of the animation.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Unused.
 * @param is_new Is this a new animation or an existing one?
 */
void AnimationEditor::pickAnimation(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    if(is_new) {
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
 * @brief Callback for when the user picks a sprite from the picker.
 *
 * @param name Name of the sprite.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Unused.
 * @param is_new Is this a new sprite or an existing one?
 */
void AnimationEditor::pickSprite(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    if(is_new) {
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
    
    if(is_new) {
        //New sprite. Suggest file name.
        curSprite->setBitmap(lastSpritesheetUsed, Point(), Point());
    }
}


/**
 * @brief Plays one of the mob's sounds.
 *
 * @param sound_idx Index of the sound data in the mob type's sound list.
 */
void AnimationEditor::playSound(size_t sound_idx) {
    if(!loadedMobType) return;
    MobType::Sound* sound_data = &loadedMobType->sounds[sound_idx];
    if(!sound_data->sample) return;
    game.audio.createUiSoundsource(
        sound_data->sample,
        sound_data->config
    );
}


/**
 * @brief Code to run for the grid toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::gridToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    gridVisible = !gridVisible;
    string state_str = (gridVisible ? "Enabled" : "Disabled");
    setStatus(state_str + " grid visibility.");
}


/**
 * @brief Code to run for the hitboxes toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::hitboxesToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    hitboxesVisible = !hitboxesVisible;
    string state_str = (hitboxesVisible ? "Enabled" : "Disabled");
    setStatus(state_str + " hitbox visibility.");
}


/**
 * @brief Code to run for the leader silhouette toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::leaderSilhouetteToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    leaderSilhouetteVisible = !leaderSilhouetteVisible;
    string state_str = (leaderSilhouetteVisible ? "Enabled" : "Disabled");
    setStatus(state_str + " leader silhouette visibility.");
}


/**
 * @brief Code to run for the load file command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::loadCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        loadWidgetPos,
        "loading a file", "load",
        std::bind(&AnimationEditor::openLoadDialog, this),
        std::bind(&AnimationEditor::saveAnimDb, this)
    );
}


/**
 * @brief Code to run for the mob radius toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::mobRadiusToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    mobRadiusVisible = !mobRadiusVisible;
    string state_str = (mobRadiusVisible ? "Enabled" : "Disabled");
    setStatus(state_str + " object radius visibility.");
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
void AnimationEditor::pickAnimDbFile(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    ContentManifest* temp_manif = (ContentManifest*) info;
    string path = temp_manif->path;
    auto really_load = [this, path] () {
        closeTopDialog();
        loadAnimDbFile(path, true);
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
 * @brief Code to run for the play/pause animation command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::playPauseAnimCmd(float input_value) {
    if(input_value < 0.5f) return;
    
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
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::quitCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        quitWidgetPos,
        "quitting", "quit",
        std::bind(&AnimationEditor::leave, this),
        std::bind(&AnimationEditor::saveAnimDb, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::reloadCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!changesMgr.existsOnDisk()) return;
    
    changesMgr.askIfUnsaved(
        reloadWidgetPos,
        "reloading the current file", "reload",
    [this] () { loadAnimDbFile(string(manifest.path), false); },
    std::bind(&AnimationEditor::saveAnimDb, this)
    );
}


/**
 * @brief Code to run for the restart animation command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::restartAnimCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!curAnimInst.validFrame()) {
        animPlaying = false;
        return;
    }
    
    curAnimInst.toStart();
    animPlaying = true;
    setStatus("Animation playback started from the beginning.");
}


/**
 * @brief Code to run for the save command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::saveCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    saveAnimDb();
}


/**
 * @brief Code to run when the zoom and position reset button widget is pressed.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::zoomAndPosResetCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(game.cam.targetZoom == 1.0f) {
        game.cam.targetPos = Point();
    } else {
        game.cam.targetZoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom everything command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::zoomEverythingCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    Sprite* s_ptr = curSprite;
    if(!s_ptr && curAnimInst.validFrame()) {
        const string &name =
            curAnimInst.curAnim->frames[curAnimInst.curFrameIdx].spriteName;
        size_t s_pos = db.findSprite(name);
        if(s_pos != INVALID) s_ptr = db.sprites[s_pos];
    }
    if(!s_ptr || !s_ptr->bitmap) return;
    
    Point cmin, cmax;
    getTransformedRectangleBBox(
        s_ptr->offset, s_ptr->bmpSize * s_ptr->scale,
        s_ptr->angle, &cmin, &cmax
    );
    
    if(s_ptr->topVisible) {
        Point top_min, top_max;
        getTransformedRectangleBBox(
            s_ptr->topPos, s_ptr->topSize,
            s_ptr->topAngle,
            &top_min, &top_max
        );
        updateMinCoords(cmin, top_min);
        updateMaxCoords(cmax, top_max);
    }
    
    for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
        Hitbox* h_ptr = &s_ptr->hitboxes[h];
        updateMinCoords(cmin, h_ptr->pos - h_ptr->radius);
        updateMaxCoords(cmax, h_ptr->pos + h_ptr->radius);
    }
    
    centerCamera(cmin, cmax);
}


/**
 * @brief Code to run for the zoom in command
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::zoomInCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.targetZoom =
        std::clamp(
            game.cam.targetZoom +
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoomMinLevel, zoomMaxLevel
        );
}


/**
 * @brief Code to run for the zoom out command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::zoomOutCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.targetZoom =
        std::clamp(
            game.cam.targetZoom -
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoomMinLevel, zoomMaxLevel
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
 * @brief Renames an animation to the given name.
 *
 * @param anim Animation to rename.
 * @param new_name Its new name.
 */
void AnimationEditor::renameAnimation(
    Animation* anim, const string &new_name
) {
    //Check if it's valid.
    if(!anim) {
        return;
    }
    
    const string old_name = anim->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        setStatus();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        setStatus("You need to specify the animation's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t a = 0; a < db.animations.size(); a++) {
        if(db.animations[a]->name == new_name) {
            setStatus(
                "An animation by the name \"" + new_name + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    anim->name = new_name;
    
    changesMgr.markAsChanged();
    setStatus(
        "Renamed animation \"" + old_name + "\" to \"" + new_name + "\"."
    );
}


/**
 * @brief Renames a body part to the given name.
 *
 * @param part Body part to rename.
 * @param new_name Its new name.
 */
void AnimationEditor::renameBodyPart(
    BodyPart* part, const string &new_name
) {
    //Check if it's valid.
    if(!part) {
        return;
    }
    
    const string old_name = part->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        setStatus();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        setStatus("You need to specify the body part's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t b = 0; b < db.bodyParts.size(); b++) {
        if(db.bodyParts[b]->name == new_name) {
            setStatus(
                "A body part by the name \"" + new_name + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    for(size_t s = 0; s < db.sprites.size(); s++) {
        for(size_t h = 0; h < db.sprites[s]->hitboxes.size(); h++) {
            if(db.sprites[s]->hitboxes[h].bodyPartName == old_name) {
                db.sprites[s]->hitboxes[h].bodyPartName = new_name;
            }
        }
    }
    part->name = new_name;
    updateHitboxes();
    
    changesMgr.markAsChanged();
    setStatus(
        "Renamed body part \"" + old_name + "\" to \"" + new_name + "\"."
    );
}


/**
 * @brief Renames a sprite to the given name.
 *
 * @param spr Sprite to rename.
 * @param new_name Its new name.
 */
void AnimationEditor::renameSprite(
    Sprite* spr, const string &new_name
) {
    //Check if it's valid.
    if(!spr) {
        return;
    }
    
    const string old_name = spr->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        setStatus();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        setStatus("You need to specify the sprite's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t s = 0; s < db.sprites.size(); s++) {
        if(db.sprites[s]->name == new_name) {
            setStatus(
                "A sprite by the name \"" + new_name + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    spr->name = new_name;
    for(size_t a = 0; a < db.animations.size(); a++) {
        Animation* a_ptr = db.animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); f++) {
            if(a_ptr->frames[f].spriteName == old_name) {
                a_ptr->frames[f].spriteName = new_name;
            }
        }
    }
    
    changesMgr.markAsChanged();
    setStatus(
        "Renamed sprite \"" + old_name + "\" to \"" + new_name + "\"."
    );
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void AnimationEditor::resetCamXY() {
    game.cam.targetPos = Point();
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
    
    s->scale    *= mult;
    s->offset   *= mult;
    s->topPos  *= mult;
    s->topSize *= mult;
    
    for(size_t h = 0; h < s->hitboxes.size(); h++) {
        Hitbox* h_ptr = &s->hitboxes[h];
        
        h_ptr->radius = fabs(h_ptr->radius * mult);
        h_ptr->pos    *= mult;
    }
    
    changesMgr.markAsChanged();
    setStatus("Resized sprite by " + f2s(mult) + ".");
}


/**
 * @brief Saves the animation database onto the mob's file.
 *
 * @return Whether it succeded.
 */
bool AnimationEditor::saveAnimDb() {
    db.engineVersion = getEngineVersionString();
    db.sortAlphabetically();
    
    DataNode file_node = DataNode("", "");
    
    db.saveToDataNode(
        &file_node,
        loadedMobType && loadedMobType->category->id == MOB_CATEGORY_PIKMIN
    );
    
    if(!file_node.saveFile(manifest.path)) {
        showSystemMessageBox(
            nullptr, "Save failed!",
            "Could not save the animation database!",
            (
                "An error occured while saving the animation database to "
                "the file \"" + manifest.path + "\". Make sure that the "
                "folder it is saving to exists and it is not read-only, "
                "and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        setStatus("Could not save the animation file!", true);
        return false;
        
    } else {
        setStatus("Saved file successfully.");
        changesMgr.markAsSaved();
        updateHistory(game.options.animEd.history, manifest, getNameForHistory());
        return true;
        
    }
}


/**
 * @brief Sets up the editor for a new animation database,
 * be it from an existing file or from scratch, after the actual creation/load
 * takes place.
 */
void AnimationEditor::setupForNewAnimDbPost() {
    vector<string> file_path_parts = split(manifest.path, "/");
    
    if(manifest.path.find(FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/") != string::npos) {
        vector<string> path_parts = split(manifest.path, "/");
        if(
            path_parts.size() > 3 &&
            path_parts[path_parts.size() - 1] == FILE_NAMES::MOB_TYPE_ANIMATION
        ) {
            MobCategory* cat =
                game.mobCategories.getFromFolderName(
                    path_parts[path_parts.size() - 3]
                );
            if(cat) {
                loadedMobType =
                    cat->getType(path_parts[path_parts.size() - 2]);
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
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        //Ideally, states would be handled by a state machine, and this
        //logic would be placed in the sprite bitmap state's "on exit" code...
        game.cam.setPos(preSpriteBmpCamPos);
        game.cam.set_zoom(preSpriteBmpCamZoom);
    }
    
    db.destroy();
    curAnimInst.clear();
    manifest.clear();
    animPlaying = false;
    curSprite = nullptr;
    curHitbox = nullptr;
    curHitboxIdx = 0;
    loadedMobType = nullptr;
    
    game.cam.setPos(Point());
    game.cam.set_zoom(1.0f);
    changeState(EDITOR_STATE_MAIN);
    
    //At this point we'll have nearly unloaded stuff like the current sprite.
    //Since Dear ImGui still hasn't rendered the current frame, which could
    //have had those assets on-screen, if it tries now it'll crash. So skip.
    game.skipDearImGuiFrame = true;
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
        Sprite* s_ptr = db.sprites[s];
        s_ptr->scale.x = scale;
        s_ptr->scale.y = scale;
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
    size_t final_sprite_idx = 0;
    vector<size_t> best_sprite_idxs;
    
    if(db.sprites.size() > 1) {
        size_t best_score = 3;
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
            
            if(score < best_score) {
                continue;
            }
            if(score > best_score) {
                best_score = score;
                best_sprite_idxs.clear();
            }
            best_sprite_idxs.push_back(s);
        }
    }
    
    if(best_sprite_idxs.size() == 1) {
        //If there's only one best match, go for it.
        final_sprite_idx = best_sprite_idxs[0];
        
    } else if(best_sprite_idxs.size() > 1) {
        //Sort them alphabetically and pick the first.
        std::sort(
            best_sprite_idxs.begin(),
            best_sprite_idxs.end(),
        [this, &best_sprite_idxs] (size_t s1, size_t s2) {
            return
                strToLower(db.sprites[s1]->name) <
                strToLower(db.sprites[s2]->name);
        });
        final_sprite_idx = best_sprite_idxs[0];
    }
    
    //Finally, set the frame info then.
    Frame* cur_frame_ptr =
        &curAnimInst.curAnim->frames[curAnimInst.curFrameIdx];
    cur_frame_ptr->spriteIdx = final_sprite_idx;
    cur_frame_ptr->spritePtr = db.sprites[final_sprite_idx];
    cur_frame_ptr->spriteName = db.sprites[final_sprite_idx]->name;
}


/**
 * @brief Performs a flood fill on the bitmap sprite, to see what parts
 * contain non-alpha pixels, based on a starting position.
 *
 * @param bmp Locked bitmap to check.
 * @param selection_pixels Array that controls which pixels are selected or not.
 * @param x X coordinate to start on.
 * @param y Y coordinate to start on.
 */
void AnimationEditor::spriteBmpFloodFill(
    ALLEGRO_BITMAP* bmp, bool* selection_pixels, int x, int y
) {
    //https://en.wikipedia.org/wiki/Flood_fill#The_algorithm
    int bmp_w = al_get_bitmap_width(bmp);
    int bmp_h = al_get_bitmap_height(bmp);
    
    if(x < 0 || x > bmp_w) return;
    if(y < 0 || y > bmp_h) return;
    if(selection_pixels[y * bmp_w + x]) return;
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
        explicit IntPoint(const Point &p) :
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
    
    queue<IntPoint> pixels_left;
    pixels_left.push(IntPoint(x, y));
    
    while(!pixels_left.empty()) {
        IntPoint p = pixels_left.front();
        pixels_left.pop();
        
        if(
            selection_pixels[(p.y) * bmp_w + p.x] ||
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
                if(selection_pixels[offset.y * bmp_w + offset.x]) {
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
            if(offset.x == bmp_w - 1) {
                add = false;
            } else {
                offset.x++;
                if(selection_pixels[offset.y * bmp_w + offset.x]) {
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
            selection_pixels[col.y * bmp_w + col.x] = true;
            if(
                col.y > 0 &&
                !selection_pixels[(col.y - 1) * bmp_w + col.x] &&
                al_get_pixel(bmp, col.x, col.y - 1).a >=
                ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(IntPoint(col.x, col.y - 1));
            }
            if(
                col.y < bmp_h - 1 &&
                !selection_pixels[(col.y + 1) * bmp_w + col.x] &&
                al_get_pixel(bmp, col.x, col.y + 1).a >=
                ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(IntPoint(col.x, col.y + 1));
            }
        }
    }
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
    
        Sprite* s_ptr = db.sprites[s];
        
        //Start by deleting non-existent hitboxes.
        for(size_t h = 0; h < s_ptr->hitboxes.size();) {
            string h_name = s_ptr->hitboxes[h].bodyPartName;
            bool name_found = false;
            
            for(size_t b = 0; b < db.bodyParts.size(); b++) {
                if(db.bodyParts[b]->name == h_name) {
                    name_found = true;
                    break;
                }
            }
            
            if(!name_found) {
                s_ptr->hitboxes.erase(
                    s_ptr->hitboxes.begin() + h
                );
            } else {
                h++;
            }
        }
        
        //Add missing hitboxes.
        for(size_t b = 0; b < db.bodyParts.size(); b++) {
            bool hitbox_found = false;
            const string &name = db.bodyParts[b]->name;
            
            for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
                if(s_ptr->hitboxes[h].bodyPartName == name) {
                    hitbox_found = true;
                    break;
                }
            }
            
            if(!hitbox_found) {
                s_ptr->hitboxes.push_back(
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
            s_ptr->hitboxes.begin(),
            s_ptr->hitboxes.end(),
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
