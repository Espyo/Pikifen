/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General area editor-related functions.
 */

#include <algorithm>

#include "editor.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


using std::set;
using std::size_t;
using std::string;
using std::unordered_set;
using std::vector;


namespace AREA_EDITOR {

//Color for blocking sectors in the "show blocking sectors" mode.
const ALLEGRO_COLOR BLOCKING_COLOR = al_map_rgba(100, 32, 32, 192);

//A comfortable distance, useful for many scenarios.
const float COMFY_DIST = 32.0f;

//Radius to use when drawing a cross-section point.
const float CROSS_SECTION_POINT_RADIUS = 8.0f;

//The cursor snap for heavy modes updates these many times a second.
const float CURSOR_SNAP_UPDATE_INTERVAL = 0.05f;

//Scale the debug text by this much.
const float DEBUG_TEXT_SCALE = 1.3f;

//Default reference image opacity.
const unsigned char DEF_REFERENCE_ALPHA = 128;

//Amount to pan the camera by when using the keyboard.
const float KEYBOARD_PAN_AMOUNT = 32.0f;

//Maximum number of points that a circle sector can be created with.
const unsigned char MAX_CIRCLE_SECTOR_POINTS = 32;

//Maximum grid interval.
const float MAX_GRID_INTERVAL = 4096;

//Maximum number of texture suggestions.
const size_t MAX_TEXTURE_SUGGESTIONS = 20;

//Text color for various measurement labels in the canvas.
const ALLEGRO_COLOR MEASUREMENT_COLOR = al_map_rgb(64, 255, 64);

//Minimum number of points that a circle sector can be created with.
const unsigned char MIN_CIRCLE_SECTOR_POINTS = 3;

//Minimum grid interval.
const float MIN_GRID_INTERVAL = 2.0;

//Thickness to use when drawing a mob link line.
const float MOB_LINK_THICKNESS = 2.0f;

//How long to tint the new sector's line(s) red for.
const float NEW_SECTOR_ERROR_TINT_DURATION = 1.5f;

//Color for non-blocking sectors in the "show blocking sectors" mode.
const ALLEGRO_COLOR NON_BLOCKING_COLOR = al_map_rgba(64, 160, 64, 192);

//Thickness to use when drawing a path link line.
const float PATH_LINK_THICKNESS = 3.0f;

//Radius to use when drawing a path preview checkpoint.
const float PATH_PREVIEW_CHECKPOINT_RADIUS = 8.0f;

//Only fetch the path these many seconds after the player stops the checkpoints.
const float PATH_PREVIEW_TIMER_DUR = 0.1f;

//Scale the letters on the "points" of various features by this much.
const float POINT_LETTER_TEXT_SCALE = 1.5f;

//Quick previewing lasts this long in total, including the fade out.
const float QUICK_PREVIEW_DURATION = 4.0f;

//Minimum width or height that the reference image can have.
const float REFERENCE_MIN_SIZE = 5.0f;

//Color of a selected element, or the selection box.
const unsigned char SELECTION_COLOR[3] = {255, 255, 0};

//Speed at which the selection effect's "wheel" spins, in radians per second.
const float SELECTION_EFFECT_SPEED = TAU * 2;

//Padding for the transformation widget when manipulating the selection.
const float SELECTION_TW_PADDING = 8.0f;

//Wait this long before letting a new repeat undo operation be saved.
const float UNDO_SAVE_LOCK_DURATION = 1.0f;

//Minimum distance between two vertexes for them to merge.
const float VERTEX_MERGE_RADIUS = 10.0f;

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 8.0f;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.01f;

}


/**
 * @brief Constructs a new area editor object.
 */
AreaEditor::AreaEditor() :
    backupTimer(game.options.areaEd.backupInterval),
    loadDialogPicker(this) {
    
    enableFlag(pathPreviewSettings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES);
    pathPreviewTimer =
    Timer(AREA_EDITOR::PATH_PREVIEW_TIMER_DUR, [this] () {
        pathPreviewDist = calculatePreviewPath();
    });
    
    undoSaveLockTimer =
        Timer(
            AREA_EDITOR::UNDO_SAVE_LOCK_DURATION,
    [this] () {undoSaveLockOperation.clear();}
        );
        
    if(game.options.areaEd.backupInterval > 0) {
        backupTimer =
            Timer(
                game.options.areaEd.backupInterval,
        [this] () {saveBackup();}
            );
    }
    
    zoomMaxLevel = AREA_EDITOR::ZOOM_MAX_LEVEL;
    zoomMinLevel = AREA_EDITOR::ZOOM_MIN_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        Command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(&AreaEditor::circleSectorCmd, "circle_sector");
    register_cmd(&AreaEditor::copyPropertiesCmd, "copy_properties");
    register_cmd(&AreaEditor::deleteAreaCmd, "delete_area");
    register_cmd(&AreaEditor::deleteCmd, "delete");
    register_cmd(&AreaEditor::deleteEdgeCmd, "delete_edge");
    register_cmd(&AreaEditor::deleteTreeShadowCmd, "delete_tree_shadow");
    register_cmd(&AreaEditor::duplicateMobsCmd, "duplicate_mobs");
    register_cmd(
        &AreaEditor::gridIntervalDecreaseCmd, "grid_interval_decrease"
    );
    register_cmd(
        &AreaEditor::gridIntervalIncreaseCmd, "grid_interval_increase"
    );
    register_cmd(&AreaEditor::layoutDrawingCmd, "layout_drawing");
    register_cmd(&AreaEditor::loadCmd, "load");
    register_cmd(&AreaEditor::newMobCmd, "new_mob");
    register_cmd(&AreaEditor::newPathCmd, "new_path");
    register_cmd(&AreaEditor::newTreeShadowCmd, "new_tree_shadow");
    register_cmd(&AreaEditor::pastePropertiesCmd, "paste_properties");
    register_cmd(&AreaEditor::pasteTextureCmd, "paste_texture");
    register_cmd(&AreaEditor::quickPlayCmd, "quick_play");
    register_cmd(&AreaEditor::quitCmd, "quit");
    register_cmd(&AreaEditor::redoCmd, "redo");
    register_cmd(&AreaEditor::referenceToggleCmd, "reference_toggle");
    register_cmd(&AreaEditor::reloadCmd, "reload");
    register_cmd(&AreaEditor::saveCmd, "save");
    register_cmd(&AreaEditor::selectAllCmd, "select_all");
    register_cmd(&AreaEditor::selectionFilterCmd, "selection_filter");
    register_cmd(&AreaEditor::snapModeCmd, "snap_mode");
    register_cmd(&AreaEditor::undoCmd, "undo");
    register_cmd(&AreaEditor::zoomAndPosResetCmd, "zoom_and_pos_reset");
    register_cmd(&AreaEditor::zoomEverythingCmd, "zoom_everything");
    register_cmd(&AreaEditor::zoomInCmd, "zoom_in");
    register_cmd(&AreaEditor::zoomOutCmd, "zoom_out");
    
#undef register_cmd
}


/**
 * @brief Calculates what the day speed should be, taking into account
 * the specified start day time, end day time, and mission duration.
 *
 * @param day_start_min Day start time, in minutes.
 * @param day_end_min Day end time, in minutes.
 * @param mission_min Mission duration, in minutes.
 * @return The day speed.
 */
float AreaEditor::calculateDaySpeed(
    float day_start_min, float day_end_min,
    float mission_min
) {
    if(mission_min == 0.0f) return 0.0f;
    float aux_day_end_min = day_end_min;
    if(day_end_min < day_start_min) {
        aux_day_end_min += 24 * 60;
    }
    return (aux_day_end_min - day_start_min) / mission_min;
}


/**
 * @brief Cancels the circular sector creation operation and returns to normal.
 */
void AreaEditor::cancelCircleSector() {
    clearCircleSector();
    subState = EDITOR_SUB_STATE_NONE;
    setStatus();
}


/**
 * @brief Cancels the edge drawing operation and returns to normal.
 */
void AreaEditor::cancelLayoutDrawing() {
    clearLayoutDrawing();
    subState = EDITOR_SUB_STATE_NONE;
    setStatus();
}


/**
 * @brief Cancels the vertex moving operation.
 */
void AreaEditor::cancelLayoutMoving() {
    for(auto const &v : selectedVertexes) {
        v->x = preMoveVertexCoords[v].x;
        v->y = preMoveVertexCoords[v].y;
    }
    clearLayoutMoving();
}


/**
 * @brief Changes to a new state, cleaning up whatever is needed.
 *
 * @param new_state The new state.
 */
void AreaEditor::changeState(const EDITOR_STATE new_state) {
    clearSelection();
    state = new_state;
    subState = EDITOR_SUB_STATE_NONE;
    setStatus();
}


/**
 * @brief Clears the data about the circular sector creation.
 */
void AreaEditor::clearCircleSector() {
    newCircleSectorStep = 0;
    newCircleSectorPoints.clear();
}


/**
 * @brief Clears the currently loaded area data.
 */
void AreaEditor::clearCurrentArea() {
    referenceFilePath.clear();
    updateReference();
    clearSelection();
    clearCircleSector();
    clearLayoutDrawing();
    clearLayoutMoving();
    clearProblems();
    
    clearAreaTextures();
    
    if(game.curAreaData) {
        for(size_t s = 0; s < game.curAreaData->treeShadows.size(); s++) {
            game.content.bitmaps.list.free(game.curAreaData->treeShadows[s]->bmpName);
        }
    }
    
    game.cam.setPos(Point());
    game.cam.set_zoom(1.0f);
    showCrossSection = false;
    showCrossSectionGrid = false;
    showBlockingSectors = false;
    showPathPreview = false;
    pathPreview.clear();
    //LARGE_FLOAT means they were never given a previous position.
    pathPreviewCheckpoints[0] = Point(LARGE_FLOAT);
    pathPreviewCheckpoints[1] = Point(LARGE_FLOAT);
    crossSectionCheckpoints[0] = Point(LARGE_FLOAT);
    crossSectionCheckpoints[1] = Point(LARGE_FLOAT);
    
    clearTextureSuggestions();
    
    game.content.unloadCurrentArea(CONTENT_LOAD_LEVEL_EDITOR);
    
    changesMgr.reset();
    backupTimer.start(game.options.areaEd.backupInterval);
    
    thumbnailNeedsSaving = false;
    thumbnailBackupNeedsSaving = false;
    
    subState = EDITOR_SUB_STATE_NONE;
    state = EDITOR_STATE_MAIN;
}


/**
 * @brief Clears the data about the layout drawing.
 */
void AreaEditor::clearLayoutDrawing() {
    drawingNodes.clear();
    drawingLineResult = DRAWING_LINE_RESULT_OK;
    sectorSplitInfo.uselessSplitPart2Checkpoint = INVALID;
}


/**
 * @brief Clears the data about the layout moving.
 */
void AreaEditor::clearLayoutMoving() {
    if(preMoveAreaData) {
        forgetPreparedState(preMoveAreaData);
        preMoveAreaData = nullptr;
    }
    preMoveVertexCoords.clear();
    clearSelection();
    moving = false;
}


/**
 * @brief Clears the data about the current problems, if any.
 */
void AreaEditor::clearProblems() {
    problemType = EPT_NONE_YET;
    problemTitle.clear();
    problemDescription.clear();
    problemEdgeIntersection.e1 = nullptr;
    problemEdgeIntersection.e2 = nullptr;
    problemMobPtr = nullptr;
    problemPathStopPtr = nullptr;
    problemSectorPtr = nullptr;
    problemShadowPtr = nullptr;
    problemVertexPtr = nullptr;
}


/**
 * @brief Clears the data about the current selection.
 */
void AreaEditor::clearSelection() {
    if(subState == EDITOR_SUB_STATE_OCTEE) {
        subState = EDITOR_SUB_STATE_NONE;
    }
    
    selectedVertexes.clear();
    selectedEdges.clear();
    selectedSectors.clear();
    selectedMobs.clear();
    selectedPathStops.clear();
    selectedPathLinks.clear();
    selectedShadow = nullptr;
    selectionHomogenized = false;
    setSelectionStatusText();
}


/**
 * @brief Clears the list of texture suggestions. This frees up the bitmaps.
 */
void AreaEditor::clearTextureSuggestions() {
    for(size_t s = 0; s < textureSuggestions.size(); s++) {
        textureSuggestions[s].destroy();
    }
    textureSuggestions.clear();
}


/**
 * @brief Clears the undo history, deleting the memory allocated for them.
 */
void AreaEditor::clearUndoHistory() {
    for(size_t h = 0; h < undoHistory.size(); h++) {
        delete undoHistory[h].first;
    }
    undoHistory.clear();
    for(size_t h = 0; h < redoHistory.size(); h++) {
        delete redoHistory[h].first;
    }
    redoHistory.clear();
}


/**
 * @brief Code to run when the area picker is closed.
 */
void AreaEditor::closeLoadDialog() {
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
void AreaEditor::closeOptionsDialog() {
    saveOptions();
}


/**
 * @brief Creates a new area to work on.
 *
 * @param requested_area_path Path to the requested area's folder.
 */
void AreaEditor::createArea(const string &requested_area_path) {
    //Setup.
    setupForNewAreaPre();
    changesMgr.markAsNonExistent();
    
    //Basic area data.
    game.curAreaData = new Area();
    game.content.areas.pathToManifest(
        requested_area_path, &manifest, &game.curAreaData->type
    );
    game.curAreaData->manifest = &manifest;
    game.curAreaData->userDataPath =
        FOLDER_PATHS_FROM_ROOT::AREA_USER_DATA + "/" +
        manifest.pack + "/" +
        (
            game.curAreaData->type == AREA_TYPE_SIMPLE ?
            FOLDER_NAMES::SIMPLE_AREAS :
            FOLDER_NAMES::MISSION_AREAS
        ) + "/" +
        manifest.internalName;
        
    //Create a sector for it.
    clearLayoutDrawing();
    float r = AREA_EDITOR::COMFY_DIST * 10;
    
    LayoutDrawingNode n;
    n.rawSpot = Point(-r, -r);
    n.snappedSpot = n.rawSpot;
    drawingNodes.push_back(n);
    
    n.rawSpot = Point(r, -r);
    n.snappedSpot = n.rawSpot;
    drawingNodes.push_back(n);
    
    n.rawSpot = Point(r, r);
    n.snappedSpot = n.rawSpot;
    drawingNodes.push_back(n);
    
    n.rawSpot = Point(-r, r);
    n.snappedSpot = n.rawSpot;
    drawingNodes.push_back(n);
    
    finishNewSectorDrawing();
    
    clearSelection();
    
    //Give a texture to give to this sector.
    string texture_to_use = findGoodFirstTexture();
    if(!texture_to_use.empty()) {
        updateSectorTexture(game.curAreaData->sectors[0], texture_to_use);
        updateTextureSuggestions(texture_to_use);
    }
    
    //Now add a leader. The first available.
    game.curAreaData->mobGenerators.push_back(
        new MobGen(Point(), game.config.leaders.order[0], 0, "")
    );
    
    //Finish up.
    setupForNewAreaPost();
    updateHistory(game.options.areaEd.history, manifest, "");
    
    setStatus(
        "Created area \"" + manifest.internalName + "\" successfully."
    );
}


/**
 * @brief Creates vertexes based on the edge drawing the user has just made.
 *
 * Drawing nodes that are already on vertexes don't count, but the other ones
 * either create edge splits, or create simple vertexes inside a sector.
 */
void AreaEditor::createDrawingVertexes() {
    for(size_t n = 0; n < drawingNodes.size(); n++) {
        LayoutDrawingNode* n_ptr = &drawingNodes[n];
        if(n_ptr->onVertex) continue;
        Vertex* new_vertex = nullptr;
        
        if(n_ptr->onEdge) {
            new_vertex = splitEdge(n_ptr->onEdge, n_ptr->snappedSpot);
            
            //The split created new edges, so let's check future nodes
            //and update them, since they could've landed on new edges.
            for(size_t n2 = n; n2 < drawingNodes.size(); n2++) {
                if(drawingNodes[n2].onEdge == n_ptr->onEdge) {
                    drawingNodes[n2].onEdge =
                        getEdgeUnderPoint(drawingNodes[n2].snappedSpot);
                }
            }
        } else {
            new_vertex = game.curAreaData->newVertex();
            new_vertex->x = n_ptr->snappedSpot.x;
            new_vertex->y = n_ptr->snappedSpot.y;
            n_ptr->isNewVertex = true;
        }
        
        n_ptr->onVertex = new_vertex;
    }
}


/**
 * @brief Creates a new mob where the cursor is.
 */
void AreaEditor::createMobUnderCursor() {
    registerChange("object creation");
    subState = EDITOR_SUB_STATE_NONE;
    Point hotspot = snapPoint(game.mouseCursor.wPos);
    
    if(lastMobCustomCatName.empty()) {
        lastMobCustomCatName =
            game.config.pikmin.order[0]->customCategoryName;
        lastMobType =
            game.config.pikmin.order[0];
    }
    
    game.curAreaData->mobGenerators.push_back(
        new MobGen(hotspot, lastMobType)
    );
    
    selectedMobs.insert(game.curAreaData->mobGenerators.back());
    
    setStatus("Created object.");
}


/**
 * @brief Deletes the current area.
 */
void AreaEditor::deleteCurrentArea() {
    string orig_internal_name = manifest.internalName;
    bool go_to_load_dialog = true;
    bool success = false;
    string message_box_text;
    
    //Start by deleting the user data, if any.
    vector<string> non_important_files;
    non_important_files.push_back(FILE_NAMES::AREA_MAIN_DATA);
    non_important_files.push_back(FILE_NAMES::AREA_GEOMETRY);
    non_important_files.push_back(FILE_NAMES::AREA_REFERENCE_CONFIG);
    wipeFolder(
        game.curAreaData->userDataPath,
        non_important_files
    );
    
    if(!changesMgr.existsOnDisk()) {
        //If the area doesn't exist on disk, since it was never
        //saved, then there's nothing to delete.
        success = true;
        go_to_load_dialog = true;
        
    } else {
        //Delete the actual area data.
        non_important_files.clear();
        non_important_files.push_back(FILE_NAMES::AREA_MAIN_DATA);
        non_important_files.push_back(FILE_NAMES::AREA_GEOMETRY);
        FS_DELETE_RESULT result =
            wipeFolder(
                manifest.path,
                non_important_files
            );
            
        switch(result) {
        case FS_DELETE_RESULT_OK: {
            success = true;
            go_to_load_dialog = true;
            break;
        } case FS_DELETE_RESULT_NOT_FOUND: {
            success = false;
            message_box_text =
                "Area \"" + orig_internal_name +
                "\" deletion failed! The folder was not found!";
            go_to_load_dialog = false;
            break;
        } case FS_DELETE_RESULT_HAS_IMPORTANT: {
            success = true;
            message_box_text =
                "The area \"" + orig_internal_name + "\" was deleted "
                "successfully, but the folder still has user files, which "
                "have not been deleted.";
            go_to_load_dialog = true;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            message_box_text =
                "Area \"" + orig_internal_name +
                "\" deletion failed! Something went wrong. Please make sure "
                "there are enough permissions to delete the folder and "
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
            setupForNewAreaPre();
            openLoadDialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        setStatus(
            "Deleted area \"" + orig_internal_name + "\" successfully."
        );
    } else {
        setStatus(
            "Area \"" + orig_internal_name + "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(message_box_text.empty()) {
        finish_up();
    } else {
        openMessageDialog(
            "Area deletion failed!",
            message_box_text,
            finish_up
        );
    }
}


/**
 * @brief Handles the logic part of the main loop of the area editor.
 *
 */
void AreaEditor::doLogic() {
    Editor::doLogicPre();
    
    processGui();
    
    cursorSnapTimer.tick(game.deltaT);
    pathPreviewTimer.tick(game.deltaT);
    quickPreviewTimer.tick(game.deltaT);
    newSectorErrorTintTimer.tick(game.deltaT);
    undoSaveLockTimer.tick(game.deltaT);
    
    if(
        game.curAreaData &&
        !manifest.internalName.empty() &&
        game.options.areaEd.backupInterval > 0
    ) {
        backupTimer.tick(game.deltaT);
    }
    
    selectionEffect += AREA_EDITOR::SELECTION_EFFECT_SPEED * game.deltaT;
    
    Editor::doLogicPost();
}


/**
 * @brief Splits the sector using the user's final drawing.
 */
void AreaEditor::doSectorSplit() {
    //Create the drawing's new edges and connect them.
    vector<Edge*> drawing_edges;
    for(size_t n = 0; n < drawingNodes.size() - 1; n++) {
        LayoutDrawingNode* n_ptr = &drawingNodes[n];
        LayoutDrawingNode* next_node = &drawingNodes[n + 1];
        
        Edge* new_node_edge = game.curAreaData->newEdge();
        
        game.curAreaData->connectEdgeToVertex(
            new_node_edge, n_ptr->onVertex, 0
        );
        game.curAreaData->connectEdgeToVertex(
            new_node_edge, next_node->onVertex, 1
        );
        
        drawing_edges.push_back(new_node_edge);
    }
    
    //Most of the time, the new sector can be made using the drawing edges
    //and the traversed edges from traversal stage 1. However, if the drawing
    //is made against an inner sector of our working sector, then there's a
    //50-50 chance that using the first set of traversed edges would result in
    //a sector that would engulf that inner sector. Instead, we'll have to use
    //the traversed edges from traversal stage 2.
    //Let's figure out which stage to use now.
    vector<Edge*> new_sector_edges = drawing_edges;
    vector<Vertex*> new_sector_vertexes;
    for(size_t d = 0; d < drawingNodes.size(); d++) {
        new_sector_vertexes.push_back(drawingNodes[d].onVertex);
    }
    
    //To figure it out, pretend we're using stage 1's data, and gather
    //the vertexes that would make the new sector. Then, check if
    //the result is clockwise or not.
    //Since the new sector is supposed to replace area from the working sector,
    //we can use that orientation and see if it matches with the sides that
    //the working sector belongs to. If not, we need the data from stage 2.
    //Oh, and in this loop, we can skip the last, since it's already
    //the same as the first drawing node.
    for(
        size_t t = 0;
        t < sectorSplitInfo.traversedVertexes[0].size() - 1;
        t++
    ) {
        new_sector_vertexes.push_back(
            sectorSplitInfo.traversedVertexes[0][t]
        );
    }
    
    bool is_new_clockwise = isPolygonClockwise(new_sector_vertexes);
    
    if(is_new_clockwise == sectorSplitInfo.isWorkingAtStage1Left) {
        //Darn, the new sector goes clockwise, which means the new sector's to
        //the right of these edges, when traversing them in stage 1's order,
        //but we know from before that the working sector is to the left!
        //(Or vice-versa.) This means that the drawing is against an inner
        //sector (it's the only way this can happen), and that this selection
        //of vertexes would result in a sector that's going around that
        //inner sector. Let's swap to the traversal stage 2 data.
        
        new_sector_vertexes.clear();
        for(size_t d = 0; d < drawingNodes.size(); d++) {
            new_sector_vertexes.push_back(drawingNodes[d].onVertex);
        }
        //Same as before, skip the last.
        for(
            size_t t = 0;
            t < sectorSplitInfo.traversedVertexes[1].size() - 1;
            t++
        ) {
            new_sector_vertexes.push_back(
                sectorSplitInfo.traversedVertexes[1][t]
            );
        }
        
        for(
            size_t t = 0;
            t < sectorSplitInfo.traversedEdges[1].size();
            t++
        ) {
            new_sector_edges.push_back(
                sectorSplitInfo.traversedEdges[1][t]
            );
        }
        
    } else {
        //We can use stage 1's data!
        //The vertexes are already in place, so let's fill in the edges.
        for(
            size_t t = 0;
            t < sectorSplitInfo.traversedEdges[0].size();
            t++
        ) {
            new_sector_edges.push_back(
                sectorSplitInfo.traversedEdges[0][t]
            );
        }
        
    }
    
    //Organize all edge vertexes such that they follow the same order.
    for(size_t e = 0; e < new_sector_edges.size(); e++) {
        if(new_sector_edges[e]->vertexes[0] != new_sector_vertexes[e]) {
            new_sector_edges[e]->swapVertexes();
        }
    }
    
    //Create the new sector, empty.
    Sector* new_sector =
        createSectorForLayoutDrawing(sectorSplitInfo.workingSector);
        
    //Connect the edges to the sectors.
    unsigned char new_sector_side = (is_new_clockwise ? 1 : 0);
    unsigned char working_sector_side = (is_new_clockwise ? 0 : 1);
    
    for(size_t e = 0; e < new_sector_edges.size(); e++) {
        Edge* e_ptr = new_sector_edges[e];
        
        if(!e_ptr->sectors[0] && !e_ptr->sectors[1]) {
            //If it's a new edge, set it up properly.
            game.curAreaData->connectEdgeToSector(
                e_ptr, sectorSplitInfo.workingSector, working_sector_side
            );
            game.curAreaData->connectEdgeToSector(
                e_ptr, new_sector, new_sector_side
            );
            
        } else {
            //If not, let's transfer from the working sector to the new one.
            game.curAreaData->connectEdgeToSector(
                e_ptr, new_sector, new_sector_side
            );
            
        }
    }
    
    //The new sector is created, but only its outer edges exist.
    //Triangulate these so we can check what's inside.
    triangulateSector(new_sector, nullptr, false);
    
    //All sectors inside the new one need to know that
    //their outer sector changed. Since we're only checking from the edges
    //that used to be long to the working sector, the edges that were created
    //with the drawing will not be included.
    updateInnerSectorsOuterSector(
        sectorSplitInfo.workingSectorOldEdges,
        sectorSplitInfo.workingSector,
        new_sector
    );
    
    //Finally, update all affected sectors. Only the working sector and
    //the new sector have had their triangles changed, so work only on those.
    unordered_set<Sector*> affected_sectors;
    affected_sectors.insert(sectorSplitInfo.workingSector);
    affected_sectors.insert(new_sector);
    updateAffectedSectors(affected_sectors);
    
    //Select one of the two sectors, making it ready for editing.
    //We want to select the smallest of the two, because it's the "most new".
    //If you have a sector that's a really complex shape, and you split
    //such that one of the post-split sectors is a triangle, chances are you
    //had that complex shape, and you wanted to make a new triangle from it,
    //not that you had a "triangle" and wanted to make a complex shape.
    clearSelection();
    
    if(!sectorSplitInfo.workingSector) {
        selectSector(new_sector);
    } else {
        float working_sector_area =
            (
                sectorSplitInfo.workingSector->bbox[1].x -
                sectorSplitInfo.workingSector->bbox[0].x
            ) * (
                sectorSplitInfo.workingSector->bbox[1].y -
                sectorSplitInfo.workingSector->bbox[0].y
            );
        float new_sector_area =
            (new_sector->bbox[1].x - new_sector->bbox[0].x) *
            (new_sector->bbox[1].y - new_sector->bbox[0].y);
            
        if(working_sector_area < new_sector_area) {
            selectSector(sectorSplitInfo.workingSector);
        } else {
            selectSector(new_sector);
        }
    }
    
    clearLayoutDrawing();
    subState = EDITOR_SUB_STATE_NONE;
    
    registerChange("sector split", sectorSplitInfo.preSplitAreaData);
    if(!sectorSplitInfo.workingSector) {
        setStatus(
            "Created sector with " +
            amountStr((int) new_sector->edges.size(), "edge") + "."
        );
    } else {
        setStatus(
            "Split sector, creating one with " +
            amountStr((int) new_sector->edges.size(), "edge") + ", one with " +
            amountStr(
                (int) sectorSplitInfo.workingSector->edges.size(),
                "edge"
            ) + "."
        );
    }
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void AreaEditor::drawCanvasDearImGuiCallback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.areaEd->drawCanvas();
}


/**
 * @brief Emits a message onto the status bar, based on the given
 * triangulation error.
 *
 * @param error The triangulation error.
 */
void AreaEditor::emitTriangulationErrorStatusBarMessage(
    const TRIANGULATION_ERROR error
) {
    switch(error) {
    case TRIANGULATION_ERROR_LONE_EDGES: {
        setStatus(
            "Some sectors have lone edges!",
            true
        );
        break;
    } case TRIANGULATION_ERROR_NOT_CLOSED: {
        setStatus(
            "Some sectors are not closed!",
            true
        );
        break;
    } case TRIANGULATION_ERROR_NO_EARS: {
        setStatus(
            "Some sectors could not be triangulated!",
            true
        );
        break;
    } case TRIANGULATION_ERROR_INVALID_ARGS: {
        setStatus(
            "An unknown error has occured with some sectors!",
            true
        );
        break;
    } case TRIANGULATION_ERROR_NONE: {
        break;
    }
    }
}


/**
 * @brief Finishes drawing a circular sector.
 */
void AreaEditor::finishCircleSector() {
    clearLayoutDrawing();
    for(size_t p = 0; p < newCircleSectorPoints.size(); p++) {
        LayoutDrawingNode n;
        n.rawSpot = newCircleSectorPoints[p];
        n.snappedSpot = n.rawSpot;
        n.onSector = getSector(n.rawSpot, nullptr, false);
        drawingNodes.push_back(n);
    }
    finishNewSectorDrawing();
    
    clearCircleSector();
    subState = EDITOR_SUB_STATE_NONE;
}


/**
 * @brief Finishes a vertex moving procedure.
 */
void AreaEditor::finishLayoutMoving() {
    unordered_set<Sector*> affected_sectors;
    getAffectedSectors(selectedVertexes, affected_sectors);
    map<Vertex*, Vertex*> merges;
    map<Vertex*, Edge*> edges_to_split;
    unordered_set<Sector*> merge_affected_sectors;
    
    //Find merge vertexes and edges to split, if any.
    for(auto &v : selectedVertexes) {
        Point p = v2p(v);
        
        vector<std::pair<Distance, Vertex*> > merge_vertexes =
            getMergeVertexes(
                p, game.curAreaData->vertexes,
                AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
            );
            
        for(size_t mv = 0; mv < merge_vertexes.size(); ) {
            Vertex* mv_ptr = merge_vertexes[mv].second;
            if(
                mv_ptr == v ||
                selectedVertexes.find(mv_ptr) != selectedVertexes.end()
            ) {
                merge_vertexes.erase(merge_vertexes.begin() + mv);
            } else {
                mv++;
            }
        }
        
        sort(
            merge_vertexes.begin(), merge_vertexes.end(),
        [] (std::pair<Distance, Vertex*> v1, std::pair<Distance, Vertex*> v2) -> bool {
            return v1.first < v2.first;
        }
        );
        
        Vertex* merge_v = nullptr;
        if(!merge_vertexes.empty()) {
            merge_v = merge_vertexes[0].second;
        }
        
        if(merge_v) {
            merges[v] = merge_v;
            
        } else {
            Edge* e_ptr = nullptr;
            bool e_ptr_v1_selected = false;
            bool e_ptr_v2_selected = false;
            
            do {
                e_ptr = getEdgeUnderPoint(p, e_ptr);
                if(e_ptr) {
                    e_ptr_v1_selected =
                        selectedVertexes.find(e_ptr->vertexes[0]) !=
                        selectedVertexes.end();
                    e_ptr_v2_selected =
                        selectedVertexes.find(e_ptr->vertexes[1]) !=
                        selectedVertexes.end();
                }
            } while(
                e_ptr != nullptr &&
                (
                    v->hasEdge(e_ptr) ||
                    e_ptr_v1_selected || e_ptr_v2_selected
                )
            );
            
            if(e_ptr) {
                edges_to_split[v] = e_ptr;
            }
        }
    }
    
    set<Edge*> moved_edges;
    for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
        Edge* e_ptr = game.curAreaData->edges[e];
        bool both_selected = true;
        for(size_t v = 0; v < 2; v++) {
            if(
                selectedVertexes.find(e_ptr->vertexes[v]) ==
                selectedVertexes.end()
            ) {
                both_selected = false;
                break;
            }
        }
        if(both_selected) {
            moved_edges.insert(e_ptr);
        }
    }
    
    //If an edge is moving into a stationary vertex, it needs to be split.
    //Let's find such edges.
    for(size_t v = 0; v < game.curAreaData->vertexes.size(); v++) {
        Vertex* v_ptr = game.curAreaData->vertexes[v];
        Point p = v2p(v_ptr);
        
        if(selectedVertexes.find(v_ptr) != selectedVertexes.end()) {
            continue;
        }
        bool is_merge_target = false;
        for(auto const &m : merges) {
            if(m.second == v_ptr) {
                //This vertex will have some other vertex merge into it; skip.
                is_merge_target = true;
                break;
            }
        }
        if(is_merge_target) continue;
        
        Edge* e_ptr = nullptr;
        bool valid = true;
        do {
            e_ptr = getEdgeUnderPoint(p, e_ptr);
            if(e_ptr) {
                if(v_ptr->hasEdge(e_ptr)) {
                    valid = false;
                }
                if(moved_edges.find(e_ptr) == moved_edges.end()) {
                    valid = false;
                }
            }
        } while(e_ptr && !valid);
        if(e_ptr) {
            edges_to_split[v_ptr] = e_ptr;
        }
    }
    
    //Before moving on and making changes, check if the move causes problems.
    //Start by checking all crossing edges, but removing all of the ones that
    //come from edge splits or vertex merges.
    vector<EdgeIntersection> intersections =
        getIntersectingEdges();
    for(auto &m : merges) {
        for(size_t e1 = 0; e1 < m.first->edges.size(); e1++) {
            for(size_t e2 = 0; e2 < m.second->edges.size(); e2++) {
                for(size_t i = 0; i < intersections.size();) {
                    if(
                        intersections[i].contains(m.first->edges[e1]) &&
                        intersections[i].contains(m.second->edges[e2])
                    ) {
                        intersections.erase(intersections.begin() + i);
                    } else {
                        i++;
                    }
                }
            }
        }
    }
    for(auto &v : edges_to_split) {
        for(size_t e = 0; e < v.first->edges.size(); e++) {
            for(size_t i = 0; i < intersections.size();) {
                if(
                    intersections[i].contains(v.first->edges[e]) &&
                    intersections[i].contains(v.second)
                ) {
                    intersections.erase(intersections.begin() + i);
                } else {
                    i++;
                }
            }
        }
    }
    
    //If we ended up with any intersection still, abort!
    if(!intersections.empty()) {
        cancelLayoutMoving();
        forgetPreparedState(preMoveAreaData);
        preMoveAreaData = nullptr;
        setStatus("That move would cause edges to intersect!", true);
        return;
    }
    
    //If there's a vertex between any dragged vertex and its merge, and this
    //vertex was meant to be a merge destination itself, then don't do it.
    //When the first merge happens, this vertex will be gone, and we'll be
    //unable to use it for the second merge. There are no plans to support
    //this complex corner case, so abort!
    for(auto &m : merges) {
        Vertex* crushed_vertex = nullptr;
        if(m.first->is2ndDegreeNeighbor(m.second, &crushed_vertex)) {
        
            for(auto const &m2 : merges) {
                if(m2.second == crushed_vertex) {
                    cancelLayoutMoving();
                    forgetPreparedState(preMoveAreaData);
                    preMoveAreaData = nullptr;
                    setStatus(
                        "That move would crush an edge that's in the middle!",
                        true
                    );
                    return;
                }
            }
        }
    }
    
    //Merge vertexes and split edges now.
    for(auto v = edges_to_split.begin(); v != edges_to_split.end(); ++v) {
        merges[v->first] = splitEdge(v->second, v2p(v->first));
        //This split could've thrown off the edge pointer of a different
        //vertex to merge. Let's re-calculate.
        Edge* new_edge = game.curAreaData->edges.back();
        auto v2 = v;
        ++v2;
        for(; v2 != edges_to_split.end(); ++v2) {
            if(v->second != v2->second) continue;
            v2->second =
                getCorrectPostSplitEdge(v2->first, v2->second, new_edge);
        }
    }
    for(auto const &m : merges) {
        mergeVertex(m.first, m.second, &merge_affected_sectors);
    }
    
    affected_sectors.insert(
        merge_affected_sectors.begin(), merge_affected_sectors.end()
    );
    
    //Update all affected sectors.
    updateAffectedSectors(affected_sectors);
    
    registerChange("vertex movement", preMoveAreaData);
    preMoveAreaData = nullptr;
    clearLayoutMoving();
}


/**
 * @brief Finishes creating a new sector.
 */
void AreaEditor::finishNewSectorDrawing() {
    if(drawingNodes.size() < 3) {
        cancelLayoutDrawing();
        return;
    }
    
    //This is the basic idea: create a new sector using the
    //vertexes provided by the user, as a "child" of an existing sector.
    
    //Get the outer sector, so we can know where to start working in.
    Sector* outer_sector = nullptr;
    if(!getDrawingOuterSector(&outer_sector)) {
        //Something went wrong. Abort.
        cancelLayoutDrawing();
        setStatus(
            "That sector wouldn't have a defined parent! Try again.",
            true
        );
        return;
    }
    
    vector<Edge*> outer_sector_old_edges;
    if(outer_sector) {
        outer_sector_old_edges = outer_sector->edges;
    } else {
        for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
            Edge* e_ptr = game.curAreaData->edges[e];
            if(e_ptr->sectors[0] == nullptr || e_ptr->sectors[1] == nullptr) {
                outer_sector_old_edges.push_back(e_ptr);
            }
        }
    }
    
    registerChange("sector creation");
    
    //First, create vertexes wherever necessary.
    createDrawingVertexes();
    
    //Now that all nodes have a vertex, create the necessary edges.
    vector<Vertex*> drawing_vertexes;
    vector<Edge*> drawing_edges;
    for(size_t n = 0; n < drawingNodes.size(); n++) {
        LayoutDrawingNode* n_ptr = &drawingNodes[n];
        size_t prev_node_idx =
            sumAndWrap((int) n, -1, (int) drawingNodes.size());
        LayoutDrawingNode* prev_node = &drawingNodes[prev_node_idx];
        
        drawing_vertexes.push_back(n_ptr->onVertex);
        
        Edge* prev_node_edge =
            n_ptr->onVertex->getEdgeByNeighbor(prev_node->onVertex);
            
        if(!prev_node_edge) {
            prev_node_edge = game.curAreaData->newEdge();
            
            game.curAreaData->connectEdgeToVertex(
                prev_node_edge, prev_node->onVertex, 0
            );
            game.curAreaData->connectEdgeToVertex(
                prev_node_edge, n_ptr->onVertex, 1
            );
        }
        
        drawing_edges.push_back(prev_node_edge);
    }
    
    //Create the new sector, empty.
    Sector* new_sector = createSectorForLayoutDrawing(outer_sector);
    
    //Connect the edges to the sectors.
    bool is_clockwise = isPolygonClockwise(drawing_vertexes);
    unsigned char inner_sector_side = (is_clockwise ? 1 : 0);
    unsigned char outer_sector_side = (is_clockwise ? 0 : 1);
    
    for(size_t e = 0; e < drawing_edges.size(); e++) {
        Edge* e_ptr = drawing_edges[e];
        
        game.curAreaData->connectEdgeToSector(
            e_ptr, outer_sector, outer_sector_side
        );
        game.curAreaData->connectEdgeToSector(
            e_ptr, new_sector, inner_sector_side
        );
    }
    
    //The new sector is created, but only its outer edges exist.
    //Triangulate these so we can check what's inside.
    triangulateSector(new_sector, nullptr, false);
    
    //All sectors inside the new one need to know that
    //their outer sector changed.
    updateInnerSectorsOuterSector(
        outer_sector_old_edges, outer_sector, new_sector
    );
    
    //Finally, update all affected sectors. Only the working sector and
    //the new sector have had their triangles changed, so work only on those.
    unordered_set<Sector*> affected_sectors;
    affected_sectors.insert(new_sector);
    affected_sectors.insert(outer_sector);
    updateAffectedSectors(affected_sectors);
    
    //Select the new sector, making it ready for editing.
    clearSelection();
    selectSector(new_sector);
    
    clearLayoutDrawing();
    subState = EDITOR_SUB_STATE_NONE;
    
    setStatus(
        "Created sector with " +
        amountStr((int) new_sector->edges.size(), "edge") + ", " +
        amountStr((int) drawing_vertexes.size(), "vertex", "vertexes") + "."
    );
}


/**
 * @brief Forgets a pre-prepared area state that was almost ready to be added to
 * the undo history.
 *
 * @param prepared_state The prepared state to forget.
 */
void AreaEditor::forgetPreparedState(Area* prepared_state) {
    delete prepared_state;
}


/**
 * @brief Returns some tooltip text that represents an area folder's manifest.
 *
 * @param path Path to the folder.
 * @param user_data_path Path to the area's user data folder, if applicable.
 * @return The tooltip text.
 */
string AreaEditor::getFolderTooltip(
    const string &path, const string &user_data_path
) const {
    ContentManifest temp_manif;
    AREA_TYPE type;
    game.content.areas.pathToManifest(
        path, &temp_manif, &type
    );
    string result =
        "Internal name: " + temp_manif.internalName + "\n"
        "Area type: " + (type == AREA_TYPE_SIMPLE ? "simple" : "mission") + "\n"
        "Folder path: " + path + "\n"
        "Pack: " + game.content.packs.list[temp_manif.pack].name;
    if(!user_data_path.empty()) {
        result +=
            "\nUser data folder path: " + user_data_path;
    }
    return result;
}


/**
 * @brief Returns which layout element the mouse is over, if any.
 * It will only return one of them.
 *
 * @param hovered_vertex If a vertex is hovered, it is returned here.
 * @param hovered_edge If an edge is hovered, it is returned here.
 * @param hovered_sector If a sector is hovered, it is returned here.
 */
void AreaEditor::getHoveredLayoutElement(
    Vertex** hovered_vertex, Edge** hovered_edge, Sector** hovered_sector
) const {
    *hovered_vertex = getVertexUnderPoint(game.mouseCursor.wPos);
    *hovered_edge = nullptr;
    *hovered_sector = nullptr;
    
    if(*hovered_vertex) return;
    
    if(selectionFilter != SELECTION_FILTER_VERTEXES) {
        *hovered_edge = getEdgeUnderPoint(game.mouseCursor.wPos);
    }
    
    if(*hovered_edge) return;
    
    if(selectionFilter == SELECTION_FILTER_SECTORS) {
        *hovered_sector = getSectorUnderPoint(game.mouseCursor.wPos);
    }
}


/**
 * @brief Returns the number of required mobs for this mission.
 *
 * @return The number.
 */
size_t AreaEditor::getMissionRequiredMobCount() const {
    size_t total_required = 0;
    
    if(game.curAreaData->mission.goalAllMobs) {
        for(
            size_t m = 0;
            m < game.curAreaData->mobGenerators.size();
            m++
        ) {
            MobGen* g = game.curAreaData->mobGenerators[m];
            if(
                game.missionGoals[game.curAreaData->mission.goal]->
                isMobApplicable(g->type)
            ) {
                total_required++;
            }
        }
    } else {
        total_required =
            game.curAreaData->mission.goalMobIdxs.size();
    }
    
    return total_required;
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string AreaEditor::getName() const {
    return "area editor";
}


/**
 * @brief Returns the path to the currently opened content,
 * or an empty string if none.
 *
 * @return The path.
 */
string AreaEditor::getOpenedContentPath() const {
    if(
        game.curAreaData &&
        game.curAreaData->manifest &&
        !manifest.internalName.empty()
    ) {
        return manifest.path;
    } else {
        return "";
    }
}


/**
 * @brief Returns the current height offset for the quick sector height set
 * mode.
 *
 * @return The offset.
 */
float AreaEditor::getQuickHeightSetOffset() const {
    float offset = quickHeightSetStartPos.y - game.mouseCursor.sPos.y;
    offset = floor(offset / 2.0f);
    offset = floor(offset / 10.0f);
    offset *= 10.0f;
    return offset;
}


/**
 * @brief Evaluates the user's drawing to figure out how the split is
 * going to work.
 *
 * @return The evaluation result.
 */
AreaEditor::SECTOR_SPLIT_RESULT AreaEditor::getSectorSplitEvaluation() {
    sectorSplitInfo.traversedEdges[0].clear();
    sectorSplitInfo.traversedEdges[1].clear();
    sectorSplitInfo.traversedVertexes[0].clear();
    sectorSplitInfo.traversedVertexes[1].clear();
    
    //Traverse the sector, starting on the last point of the drawing,
    //going edge by edge, until we hit that point again.
    //During traversal, collect a list of traversed edges and vertexes.
    //This traversal happens in two stages. In the first stage, collect them
    //into the first set of vectors. Once the traversal reaches the checkpoint,
    //it restarts and goes in the opposite direction, collecting edges and
    //vertexes into the second set of vectors from here on out. Normally,
    //we only need the data from stage 1 to create a sector, but as we'll see
    //later on, we may need to use data from stage 2 instead.
    traverseSectorForSplit(
        sectorSplitInfo.workingSector,
        drawingNodes.back().onVertex,
        drawingNodes[0].onVertex,
        sectorSplitInfo.traversedEdges,
        sectorSplitInfo.traversedVertexes,
        &sectorSplitInfo.isWorkingAtStage1Left
    );
    
    if(sectorSplitInfo.traversedEdges[0].empty()) {
        //Something went wrong.
        return SECTOR_SPLIT_RESULT_INVALID;
    }
    
    if(sectorSplitInfo.traversedEdges[1].empty()) {
        //If the sector's neighboring edges were traversed entirely
        //without finding the drawing's last point, then that point is in a set
        //of edges different from the drawing's first point. This can happen
        //if the points are in different inner sectors, or if only
        //one of them is in an inner sector.
        //If the user were to split in this way, the sector would still be
        //in one piece, except with a disallowed gash. Cancel.
        return SECTOR_SPLIT_RESULT_USELESS;
    }
    
    return SECTOR_SPLIT_RESULT_OK;
}


/**
 * @brief Focuses the camera on the problem found, if any.
 */
void AreaEditor::goToProblem() {
    switch(problemType) {
    case EPT_NONE:
    case EPT_NONE_YET: {
        return;
        
    } case EPT_INTERSECTING_EDGES: {

        if(
            !problemEdgeIntersection.e1 || !problemEdgeIntersection.e2
        ) {
            //Uh, old information. Try searching for problems again.
            findProblems();
            return;
        }
        
        Point min_coords = v2p(problemEdgeIntersection.e1->vertexes[0]);
        Point max_coords = min_coords;
        
        updateMinMaxCoords(
            min_coords, max_coords,
            v2p(problemEdgeIntersection.e1->vertexes[1])
        );
        updateMinMaxCoords(
            min_coords, max_coords,
            v2p(problemEdgeIntersection.e2->vertexes[0])
        );
        updateMinMaxCoords(
            min_coords, max_coords,
            v2p(problemEdgeIntersection.e2->vertexes[1])
        );
        
        changeState(EDITOR_STATE_LAYOUT);
        selectEdge(problemEdgeIntersection.e1);
        selectEdge(problemEdgeIntersection.e2);
        centerCamera(min_coords, max_coords);
        
        break;
        
    } case EPT_BAD_SECTOR: {

        if(game.curAreaData->problems.nonSimples.empty()) {
            //Uh, old information. Try searching for problems again.
            findProblems();
            return;
        }
        
        changeState(EDITOR_STATE_LAYOUT);
        Sector* s_ptr = game.curAreaData->problems.nonSimples.begin()->first;
        selectSector(s_ptr);
        centerCamera(s_ptr->bbox[0], s_ptr->bbox[1]);
        
        break;
        
    } case EPT_LONE_EDGE: {

        if(game.curAreaData->problems.loneEdges.empty()) {
            //Uh, old information. Try searching for problems again.
            findProblems();
            return;
        }
        
        Edge* e_ptr = *game.curAreaData->problems.loneEdges.begin();
        Point min_coords = v2p(e_ptr->vertexes[0]);
        Point max_coords = min_coords;
        updateMinMaxCoords(
            min_coords, max_coords, v2p(e_ptr->vertexes[1])
        );
        
        changeState(EDITOR_STATE_LAYOUT);
        selectEdge(e_ptr);
        centerCamera(min_coords, max_coords);
        
        break;
        
    } case EPT_OVERLAPPING_VERTEXES: {

        if(!problemVertexPtr) {
            //Uh, old information. Try searching for problems again.
            findProblems();
            return;
        }
        
        changeState(EDITOR_STATE_LAYOUT);
        selectVertex(problemVertexPtr);
        centerCamera(
            Point(
                problemVertexPtr->x - 64,
                problemVertexPtr->y - 64
            ),
            Point(
                problemVertexPtr->x + 64,
                problemVertexPtr->y + 64
            )
        );
        
        break;
        
    } case EPT_UNKNOWN_TEXTURE: {

        if(!problemSectorPtr) {
            //Uh, old information. Try searching for problems again.
            findProblems();
            return;
        }
        
        changeState(EDITOR_STATE_LAYOUT);
        selectSector(problemSectorPtr);
        centerCamera(problemSectorPtr->bbox[0], problemSectorPtr->bbox[1]);
        
        break;
        
    } case EPT_TYPELESS_MOB:
    case EPT_MOB_OOB:
    case EPT_MOB_IN_WALL:
    case EPT_MOB_LINKS_TO_SELF:
    case EPT_MOB_STORED_IN_LOOP:
    case EPT_SECTORLESS_BRIDGE:
    case EPT_PILE_BRIDGE_PATH: {

        if(!problemMobPtr) {
            //Uh, old information. Try searching for problems again.
            findProblems();
            return;
        }
        
        changeState(EDITOR_STATE_MOBS);
        selectedMobs.insert(problemMobPtr);
        centerCamera(problemMobPtr->pos - 64, problemMobPtr->pos + 64);
        
        break;
        
    } case EPT_LONE_PATH_STOP:
    case EPT_PATH_STOPS_TOGETHER:
    case EPT_PATH_STOP_ON_LINK:
    case EPT_PATH_STOP_OOB: {

        if(!problemPathStopPtr) {
            //Uh, old information. Try searching for problems again.
            findProblems();
            return;
        }
        
        changeState(EDITOR_STATE_PATHS);
        selectedPathStops.insert(problemPathStopPtr);
        centerCamera(
            problemPathStopPtr->pos - 64,
            problemPathStopPtr->pos + 64
        );
        
        break;
        
    } case EPT_UNKNOWN_SHADOW: {

        Point min_coords, max_coords;
        getTransformedRectangleBBox(
            problemShadowPtr->center, problemShadowPtr->size,
            problemShadowPtr->angle, &min_coords, &max_coords
        );
        
        changeState(EDITOR_STATE_DETAILS);
        selectTreeShadow(problemShadowPtr);
        centerCamera(min_coords, max_coords);
        
        break;
        
    } default: {
        //Nowhere to go.
        break;
        
    }
    }
}


/**
 * @brief Handles an error in the line the user is trying to draw.
 */
void AreaEditor::handleLineError() {
    newSectorErrorTintTimer.start();
    switch(drawingLineResult) {
    case DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX: {
        break;
    } case DRAWING_LINE_RESULT_ALONG_EDGE: {
        setStatus(
            "That line is drawn on top of an edge!",
            true
        );
        break;
    } case DRAWING_LINE_RESULT_CROSSES_DRAWING: {
        setStatus(
            "That line crosses other lines in the drawing!",
            true
        );
        break;
    } case DRAWING_LINE_RESULT_CROSSES_EDGES: {
        setStatus(
            "That line crosses existing edges!",
            true
        );
        break;
    } case DRAWING_LINE_RESULT_WAYWARD_SECTOR: {
        setStatus(
            "That line goes out of the sector you're drawing on!",
            true
        );
        break;
    } case DRAWING_LINE_RESULT_OK: {
        break;
    }
    }
}


/**
 * @brief Loads the area editor.
 */
void AreaEditor::load() {
    Editor::load();
    
    //Load necessary game content.
    game.content.reloadPacks();
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_MOB_TYPE,
        CONTENT_TYPE_WEATHER_CONDITION,
        CONTENT_TYPE_AREA,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    loadCustomMobCatTypes(true);
    
    //Misc. setup.
    lastMobCustomCatName.clear();
    lastMobType = nullptr;
    selectedShadow = nullptr;
    selectionEffect = 0.0;
    selectionHomogenized = false;
    showClosestStop = false;
    showPathPreview = false;
    previewMode = false;
    quickPreviewTimer.stop();
    previewSong.clear();
    clearProblems();
    clearSelection();
    
    changeState(EDITOR_STATE_MAIN);
    game.audio.setCurrentSong(game.sysContentNames.sngEditors, false);
    
    //Automatically load a file if needed, or show the load dialog.
    if(!quickPlayAreaPath.empty()) {
        loadAreaFolder(quickPlayAreaPath, false, true);
        game.cam.setPos(quickPlayCamPos);
        game.cam.set_zoom(quickPlayCamZ);
        quickPlayAreaPath.clear();
        
    } else if(!autoLoadFolder.empty()) {
        loadAreaFolder(autoLoadFolder, false, true);
        
    } else {
        openLoadDialog();
        
    }
}


/**
 * @brief Load the area from the disk.
 *
 * @param requested_area_path Path to the requested area's folder.
 * @param from_backup If false, load it normally.
 * If true, load from a backup, if any.
 * @param should_update_history If true, this loading process should update
 * the user's folder open history.
 */
void AreaEditor::loadAreaFolder(
    const string &requested_area_path,
    bool from_backup, bool should_update_history
) {
    //Setup.
    setupForNewAreaPre();
    changesMgr.markAsNonExistent();
    
    //Load.
    AREA_TYPE requested_area_type;
    game.content.areas.pathToManifest(
        requested_area_path, &manifest, &requested_area_type
    );
    if(
        !game.content.loadAreaAsCurrent(
            requested_area_path, &manifest,
            CONTENT_LOAD_LEVEL_EDITOR, from_backup
        )
    ) {
        openMessageDialog(
            "Load failed!",
            "Failed to load the area folder \"" + manifest.path + "\"!",
        [this] () { openLoadDialog(); }
        );
        manifest.clear();
        return;
    }
    
    //Calculate texture suggestions.
    map<string, size_t> texture_uses_map;
    vector<std::pair<string, size_t> > texture_uses_vector;
    
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        const string &n = game.curAreaData->sectors[s]->textureInfo.bmpName;
        if(n.empty()) continue;
        texture_uses_map[n]++;
    }
    for(auto &u : texture_uses_map) {
        texture_uses_vector.push_back(make_pair(u.first, u.second));
    }
    sort(
        texture_uses_vector.begin(), texture_uses_vector.end(),
    [] (std::pair<string, size_t> u1, std::pair<string, size_t> u2) -> bool {
        return u1.second > u2.second;
    }
    );
    
    for(
        size_t u = 0;
        u < texture_uses_vector.size() && u <
        AREA_EDITOR::MAX_TEXTURE_SUGGESTIONS;
        u++
    ) {
        textureSuggestions.push_back(
            TextureSuggestion(texture_uses_vector[u].first)
        );
    }
    
    //Other tasks.
    loadReference();
    
    //Finish up.
    changesMgr.reset();
    setupForNewAreaPost();
    if(should_update_history) {
        updateHistory(game.options.areaEd.history, manifest, game.curAreaData->name);
    }
    setStatus(
        "Loaded area \"" + manifest.internalName + "\" " +
        (from_backup ? "from a backup " : "") +
        "successfully."
    );
}


/**
 * @brief Loads a backup file.
 */
void AreaEditor::loadBackup() {
    loadAreaFolder(
        manifest.path,
        true, false
    );
    backupTimer.start(game.options.areaEd.backupInterval);
    changesMgr.markAsChanged();
    
    //We don't know if the backup's thumbnail is different from the standard
    //copy's thumbnail. To be safe, just mark it as needing a save. Loading a
    //backup is such a rare operation that it's worth the effort of re-saving
    //the bitmap.
    thumbnailNeedsSaving = true;
}


/**
 * @brief Loads the reference image data from the reference configuration file.
 */
void AreaEditor::loadReference() {
    DataNode file(game.curAreaData->userDataPath + "/" + FILE_NAMES::AREA_REFERENCE_CONFIG);
    
    if(file.fileWasOpened) {
        referenceFilePath = file.getChildByName("file")->value;
        referenceCenter = s2p(file.getChildByName("center")->value);
        referenceSize = s2p(file.getChildByName("size")->value);
        referenceAlpha =
            s2i(
                file.getChildByName(
                    "alpha"
                )->getValueOrDefault(i2s(AREA_EDITOR::DEF_REFERENCE_ALPHA))
            );
        showReference = s2b(file.getChildByName("visible")->value);
        
    } else {
        referenceFilePath.clear();
        referenceCenter = Point();
        referenceSize = Point();
        referenceAlpha = 0;
        showReference = true;
    }
    
    updateReference();
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void AreaEditor::panCam(const ALLEGRO_EVENT &ev) {
    game.cam.setPos(
        Point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/**
 * @brief Callback for when the user picks an area from the picker.
 *
 * @param name Name of the area.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Pointer to the area's manifest.
 * @param is_new Is it a new area, or an existing one?
 */
void AreaEditor::pickAreaFolder(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    ContentManifest* temp_manif = (ContentManifest*) info;
    
    auto really_load = [this, temp_manif] () {
        closeTopDialog();
        loadAreaFolder(temp_manif->path, false, true);
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
 * @brief Callback for when the user picks a texture from the picker.
 *
 * @param name Name of the texture.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Unused.
 * @param is_new Unused.
 */
void AreaEditor::pickTexture(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    Sector* s_ptr = nullptr;
    if(selectedSectors.size() == 1 || selectionHomogenized) {
        s_ptr = *selectedSectors.begin();
    }
    if(!s_ptr) return;
    
    if(name == "Choose another...") {
        openBitmapDialog(
        [this, s_ptr] (const string &bmp) {
            if(s_ptr->textureInfo.bmpName == bmp) return;
            registerChange("sector texture change");
            updateTextureSuggestions(bmp);
            updateSectorTexture(s_ptr, bmp);
            homogenizeSelectedSectors();
            setStatus("Picked an image successfully.");
        },
        FOLDER_NAMES::TEXTURES
        );
    } else {
        if(s_ptr->textureInfo.bmpName == name) return;
        registerChange("sector texture change");
        updateTextureSuggestions(name);
        updateSectorTexture(s_ptr, name);
        homogenizeSelectedSectors();
    }
}


/**
 * @brief Prepares an area state to be delivered to registerChange() later,
 * or forgotten altogether with forgetPreparedState().
 *
 * @return The prepared state.
 */
Area* AreaEditor::prepareState() {
    Area* new_state = new Area();
    game.curAreaData->clone(*new_state);
    return new_state;
}


/**
 * @brief Code to run for the circle sector command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::circleSectorCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(
        subState == EDITOR_SUB_STATE_DRAWING ||
        subState == EDITOR_SUB_STATE_CIRCLE_SECTOR
    ) {
        return;
    }
    
    if(
        !game.curAreaData->problems.nonSimples.empty() ||
        !game.curAreaData->problems.loneEdges.empty()
    ) {
        setStatus(
            "Please fix any broken sectors or edges before trying to make "
            "a new sector!",
            true
        );
        return;
    }
    
    clearSelection();
    clearCircleSector();
    setStatus("Use the canvas to place a circular sector.");
    subState = EDITOR_SUB_STATE_CIRCLE_SECTOR;
}


/**
 * @brief Code to run for the copy properties command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::copyPropertiesCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(!selectedSectors.empty()) {
            copySectorProperties();
        } else {
            copyEdgeProperties();
        }
        break;
    } case EDITOR_STATE_MOBS: {
        copyMobProperties();
        break;
    } case EDITOR_STATE_PATHS: {
        copyPathLinkProperties();
        break;
    }
    }
}


/**
 * @brief Code to run for the delete current area command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::deleteAreaCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    openDialog(
        "Delete area?",
        std::bind(&AreaEditor::processGuiDeleteAreaDialog, this)
    );
    dialogs.back()->customSize = Point(600, 0);
}


/**
 * @brief Code to run for the delete command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::deleteCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        deleteEdgeCmd(1.0f);
        break;
    } case EDITOR_STATE_MOBS: {
        deleteMobCmd(1.0f);
        break;
    } case EDITOR_STATE_PATHS: {
        deletePathCmd(1.0f);
        break;
    } case EDITOR_STATE_DETAILS: {
        deleteTreeShadowCmd(1.0f);
        break;
    }
    }
}


/**
 * @brief Code to run for the duplicate mobs command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::duplicateMobsCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(
        subState == EDITOR_SUB_STATE_NEW_MOB ||
        subState == EDITOR_SUB_STATE_DUPLICATE_MOB ||
        subState == EDITOR_SUB_STATE_STORE_MOB_INSIDE ||
        subState == EDITOR_SUB_STATE_ADD_MOB_LINK ||
        subState == EDITOR_SUB_STATE_DEL_MOB_LINK
    ) {
        return;
    }
    
    if(selectedMobs.empty()) {
        setStatus("You have to select mobs to duplicate!", true);
    } else {
        setStatus("Use the canvas to place the duplicated objects.");
        subState = EDITOR_SUB_STATE_DUPLICATE_MOB;
    }
}


/**
 * @brief Code to run for the grid interval decrease command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::gridIntervalDecreaseCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.options.areaEd.gridInterval =
        std::max(
            game.options.areaEd.gridInterval * 0.5f,
            AREA_EDITOR::MIN_GRID_INTERVAL
        );
    setStatus(
        "Decreased grid interval to " +
        i2s(game.options.areaEd.gridInterval) + "."
    );
}


/**
 * @brief Code to run for the grid interval increase command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::gridIntervalIncreaseCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.options.areaEd.gridInterval =
        std::min(
            game.options.areaEd.gridInterval * 2.0f,
            AREA_EDITOR::MAX_GRID_INTERVAL
        );
    setStatus(
        "Increased grid interval to " +
        i2s(game.options.areaEd.gridInterval) + "."
    );
}


/**
 * @brief Code to run for the layout drawing command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::layoutDrawingCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(
        subState == EDITOR_SUB_STATE_DRAWING ||
        subState == EDITOR_SUB_STATE_CIRCLE_SECTOR
    ) {
        return;
    }
    
    if(
        !game.curAreaData->problems.nonSimples.empty() ||
        !game.curAreaData->problems.loneEdges.empty()
    ) {
        setStatus(
            "Please fix any broken sectors or edges before trying to make "
            "a new sector!",
            true
        );
        return;
    }
    
    clearSelection();
    clearLayoutDrawing();
    updateLayoutDrawingStatusText();
    subState = EDITOR_SUB_STATE_DRAWING;
}


/**
 * @brief Code to run for the load area command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::loadCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    changesMgr.askIfUnsaved(
        loadWidgetPos,
        "loading an area", "load",
        std::bind(&AreaEditor::openLoadDialog, this),
    [this] () { return saveArea(false); }
    );
}


/**
 * @brief Code to run for the new mob command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::newMobCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(
        subState == EDITOR_SUB_STATE_NEW_MOB ||
        subState == EDITOR_SUB_STATE_DUPLICATE_MOB ||
        subState == EDITOR_SUB_STATE_STORE_MOB_INSIDE ||
        subState == EDITOR_SUB_STATE_ADD_MOB_LINK ||
        subState == EDITOR_SUB_STATE_DEL_MOB_LINK
    ) {
        return;
    }
    
    clearSelection();
    setStatus("Use the canvas to place a new object.");
    subState = EDITOR_SUB_STATE_NEW_MOB;
}


/**
 * @brief Code to run for the new path command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::newPathCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(subState == EDITOR_SUB_STATE_PATH_DRAWING) {
        return;
    }
    
    clearSelection();
    pathDrawingStop1 = nullptr;
    setStatus("Use the canvas to draw a path.");
    subState = EDITOR_SUB_STATE_PATH_DRAWING;
}


/**
 * @brief Code to run for the new tree shadow command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::newTreeShadowCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(subState == EDITOR_SUB_STATE_NEW_SHADOW) {
        return;
    }
    
    clearSelection();
    setStatus("Use the canvas to place a new tree shadow.");
    subState = EDITOR_SUB_STATE_NEW_SHADOW;
}


/**
 * @brief Code to run for the paste properties command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::pastePropertiesCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(subState != EDITOR_SUB_STATE_NONE) return;
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(!selectedSectors.empty()) {
            pasteSectorProperties();
        } else {
            pasteEdgeProperties();
        }
        break;
    } case EDITOR_STATE_MOBS: {
        pasteMobProperties();
        break;
    } case EDITOR_STATE_PATHS: {
        pastePathLinkProperties();
        break;
    }
    }
}


/**
 * @brief Code to run for the paste texture command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::pasteTextureCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(state != EDITOR_STATE_LAYOUT) return;
    if(subState != EDITOR_SUB_STATE_NONE) return;
    pasteSectorTexture();
}


/**
 * @brief Code to run for the quick play command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::quickPlayCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!saveArea(false)) return;
    quickPlayAreaPath = manifest.path;
    quickPlayCamPos = game.cam.pos;
    quickPlayCamZ = game.cam.zoom;
    leave();
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::quitCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changesMgr.askIfUnsaved(
        quitWidgetPos,
        "quitting", "quit",
        std::bind(&AreaEditor::leave, this),
    [this] () { return saveArea(false); }
    );
}


/**
 * @brief Code to run for the redo command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::redoCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(
        subState != EDITOR_SUB_STATE_NONE ||
        moving || selecting || curTransformationWidget.isMovingHandle()
    ) {
        setStatus("Can't redo in the middle of an operation!", true);
        return;
    }
    
    redo();
}


/**
 * @brief Code to run for the reference toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::referenceToggleCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    showReference = !showReference;
    string state_str = (showReference ? "Enabled" : "Disabled");
    saveReference();
    setStatus(state_str + " reference image visibility.");
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::reloadCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!changesMgr.existsOnDisk()) {
        setStatus(
            "You can't reload this area since it's never been saved!", true
        );
        return;
    }
    
    changesMgr.askIfUnsaved(
        reloadWidgetPos,
        "reloading the current area", "reload",
    [this] () {
        loadAreaFolder(string(manifest.path), false, false);
    },
    [this] () { return saveArea(false); }
    );
}


/**
 * @brief Code to run for the delete edge command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::deleteEdgeCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    //Check if the user can delete.
    if(moving || selecting) {
        return;
    }
    
    if(selectedEdges.empty()) {
        setStatus("You have to select edges to delete!", true);
        return;
    }
    
    //Prepare everything.
    registerChange("edge deletion");
    size_t n_before = game.curAreaData->edges.size();
    size_t n_selected = selectedEdges.size();
    
    //Delete!
    bool success = deleteEdges(selectedEdges);
    
    //Cleanup.
    clearSelection();
    subState = EDITOR_SUB_STATE_NONE;
    
    //Report.
    if(success) {
        setStatus(
            "Deleted " +
            amountStr(
                (int) (n_before - game.curAreaData->edges.size()),
                "edge"
            ) +
            " (" + i2s(n_selected) + " were selected)."
        );
    }
}


/**
 * @brief Code to run for the delete mob command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::deleteMobCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    //Check if the user can delete.
    if(moving || selecting) {
        return;
    }
    
    if(selectedMobs.empty()) {
        setStatus("You have to select mobs to delete!", true);
        return;
    }
    
    //Prepare everything.
    registerChange("object deletion");
    size_t n_before = game.curAreaData->mobGenerators.size();
    
    //Delete!
    deleteMobs(selectedMobs);
    
    //Cleanup.
    clearSelection();
    subState = EDITOR_SUB_STATE_NONE;
    
    //Report.
    setStatus(
        "Deleted " +
        amountStr(
            (int) (n_before - game.curAreaData->mobGenerators.size()),
            "object"
        ) +
        "."
    );
}


/**
 * @brief Code to run for the delete path command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::deletePathCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    //Check if the user can delete.
    if(moving || selecting) {
        return;
    }
    
    if(selectedPathLinks.empty() && selectedPathStops.empty()) {
        setStatus("You have to select something to delete!", true);
        return;
    }
    
    //Prepare everything.
    registerChange("path deletion");
    size_t n_stops_before = game.curAreaData->pathStops.size();
    size_t n_links_before = game.curAreaData->getNrPathLinks();
    
    //Delete!
    deletePathLinks(selectedPathLinks);
    deletePathStops(selectedPathStops);
    
    //Cleanup.
    clearSelection();
    subState = EDITOR_SUB_STATE_NONE;
    pathPreview.clear(); //Clear so it doesn't reference deleted stops.
    pathPreviewTimer.start(false);
    
    //Report.
    setStatus(
        "Deleted " +
        amountStr(
            (int) (n_stops_before - game.curAreaData->pathStops.size()),
            "path stop"
        ) +
        ", " +
        amountStr(
            (int) (n_links_before - game.curAreaData->getNrPathLinks()),
            "path link"
        ) +
        "."
    );
}


/**
 * @brief Code to run for the remove tree shadow command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::deleteTreeShadowCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(moving || selecting) {
        return;
    }
    
    if(!selectedShadow) {
        setStatus("You have to select a shadow to delete!", true);
    } else {
        registerChange("tree shadow deletion");
        for(
            size_t s = 0;
            s < game.curAreaData->treeShadows.size();
            s++
        ) {
            if(
                game.curAreaData->treeShadows[s] ==
                selectedShadow
            ) {
                game.curAreaData->treeShadows.erase(
                    game.curAreaData->treeShadows.begin() + s
                );
                delete selectedShadow;
                selectedShadow = nullptr;
                break;
            }
        }
        setStatus("Deleted tree shadow.");
    }
}


/**
 * @brief Code to run for the save button command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::saveCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!saveArea(false)) {
        return;
    }
}


/**
 * @brief Code to run for the select all command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::selectAllCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(subState == EDITOR_SUB_STATE_NONE && !selecting && !moving) {
        if(state == EDITOR_STATE_LAYOUT) {
            selectedEdges.insert(
                game.curAreaData->edges.begin(),
                game.curAreaData->edges.end()
            );
            selectedSectors.insert(
                game.curAreaData->sectors.begin(),
                game.curAreaData->sectors.end()
            );
            selectedVertexes.insert(
                game.curAreaData->vertexes.begin(),
                game.curAreaData->vertexes.end()
            );
            
        } else if(state == EDITOR_STATE_MOBS) {
            selectedMobs.insert(
                game.curAreaData->mobGenerators.begin(),
                game.curAreaData->mobGenerators.end()
            );
            
        } else if(state == EDITOR_STATE_PATHS) {
            selectedPathStops.insert(
                game.curAreaData->pathStops.begin(),
                game.curAreaData->pathStops.end()
            );
        }
        
        updateVertexSelection();
        setSelectionStatusText();
        
    } else if(
        subState == EDITOR_SUB_STATE_MISSION_MOBS
    ) {
        registerChange("mission object requirements change");
        for(
            size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++
        ) {
            MobGen* m_ptr = game.curAreaData->mobGenerators[m];
            if(
                game.missionGoals[game.curAreaData->mission.goal]->
                isMobApplicable(m_ptr->type)
            ) {
                game.curAreaData->mission.goalMobIdxs.insert(m);
            }
        }
    }
}


/**
 * @brief Code to run for the selection filter command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::selectionFilterCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    clearSelection();
    if(!isShiftPressed) {
        selectionFilter =
            (SELECTION_FILTER)
            sumAndWrap(selectionFilter, 1, N_SELECTION_FILTERS);
    } else {
        selectionFilter =
            (SELECTION_FILTER)
            sumAndWrap(selectionFilter, -1, N_SELECTION_FILTERS);
    }
    
    string final_status_text = "Set selection filter to ";
    switch(selectionFilter) {
    case SELECTION_FILTER_SECTORS: {
        final_status_text += "sectors + edges + vertexes";
        break;
    } case SELECTION_FILTER_EDGES: {
        final_status_text += "edges + vertexes";
        break;
    } case SELECTION_FILTER_VERTEXES: {
        final_status_text += "vertexes";
        break;
    } case N_SELECTION_FILTERS: {
        break;
    }
    }
    final_status_text += ".";
    setStatus(final_status_text);
}


/**
 * @brief Code to run for the snap mode command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::snapModeCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!isShiftPressed) {
        game.options.areaEd.snapMode =
            (AreaEditor::SNAP_MODE)
            sumAndWrap(game.options.areaEd.snapMode, 1, N_SNAP_MODES);
    } else {
        game.options.areaEd.snapMode =
            (AreaEditor::SNAP_MODE)
            sumAndWrap(game.options.areaEd.snapMode, -1, N_SNAP_MODES);
    }
    
    string final_status_text = "Set snap mode to ";
    switch(game.options.areaEd.snapMode) {
    case SNAP_MODE_GRID: {
        final_status_text += "grid";
        break;
    } case SNAP_MODE_VERTEXES: {
        final_status_text += "vertexes";
        break;
    } case SNAP_MODE_EDGES: {
        final_status_text += "edges";
        break;
    } case SNAP_MODE_NOTHING: {
        final_status_text += "nothing";
        break;
    } case N_SNAP_MODES: {
        break;
    }
    }
    final_status_text += ".";
    setStatus(final_status_text);
}


/**
 * @brief Code to run for the undo command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::undoCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(
        subState != EDITOR_SUB_STATE_NONE ||
        moving || selecting || curTransformationWidget.isMovingHandle()
    ) {
        setStatus("Can't undo in the middle of an operation!", true);
        return;
    }
    
    undo();
}


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::zoomAndPosResetCmd(float input_value) {
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
void AreaEditor::zoomEverythingCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    bool got_something = false;
    Point min_coords, max_coords;
    
    for(size_t v = 0; v < game.curAreaData->vertexes.size(); v++) {
        Vertex* v_ptr = game.curAreaData->vertexes[v];
        if(v_ptr->x < min_coords.x || !got_something) {
            min_coords.x = v_ptr->x;
        }
        if(v_ptr->y < min_coords.y || !got_something) {
            min_coords.y = v_ptr->y;
        }
        if(v_ptr->x > max_coords.x || !got_something) {
            max_coords.x = v_ptr->x;
        }
        if(v_ptr->y > max_coords.y || !got_something) {
            max_coords.y = v_ptr->y;
        }
        got_something = true;
    }
    
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        if(m_ptr->pos.x < min_coords.x || !got_something) {
            min_coords.x = m_ptr->pos.x;
        }
        if(m_ptr->pos.y < min_coords.y || !got_something) {
            min_coords.y = m_ptr->pos.y;
        }
        if(m_ptr->pos.x > max_coords.x || !got_something) {
            max_coords.x = m_ptr->pos.x;
        }
        if(m_ptr->pos.y > max_coords.y || !got_something) {
            max_coords.y = m_ptr->pos.y;
        }
        got_something = true;
    }
    
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* s_ptr = game.curAreaData->pathStops[s];
        if(s_ptr->pos.x < min_coords.x || !got_something) {
            min_coords.x = s_ptr->pos.x;
        }
        if(s_ptr->pos.y < min_coords.y || !got_something) {
            min_coords.y = s_ptr->pos.y;
        }
        if(s_ptr->pos.x > max_coords.x || !got_something) {
            max_coords.x = s_ptr->pos.x;
        }
        if(s_ptr->pos.y > max_coords.y || !got_something) {
            max_coords.y = s_ptr->pos.y;
        }
        got_something = true;
    }
    
    if(!got_something) return;
    
    centerCamera(min_coords, max_coords);
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param input_value Value of the player input for the command.
 */
void AreaEditor::zoomInCmd(float input_value) {
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
void AreaEditor::zoomOutCmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.targetZoom =
        std::clamp(
            game.cam.targetZoom -
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoomMinLevel, zoomMaxLevel
        );
}


/**
 * @brief Recreates the current drawing's nodes. Useful if the data the nodes
 * were holding is stale, like if the area's state had to be reverted
 * mid-drawing.
 *
 */
void AreaEditor::recreateDrawingNodes() {
    for(size_t n = 0; n < drawingNodes.size(); n++) {
        drawingNodes[n] = LayoutDrawingNode(this, drawingNodes[n].rawSpot);
    }
}


/**
 * @brief Redoes the latest undone change to the area using the undo history,
 * if available.
 */
void AreaEditor::redo() {
    if(redoHistory.empty()) {
        setStatus("Nothing to redo.");
        return;
    }
    
    //Let's first save the state of things right now so we can feed it into
    //the undo history afterwards.
    Area* new_state = new Area();
    game.curAreaData->clone(*new_state);
    string operation_name = redoHistory.front().second;
    
    //Change the area state.
    setStateFromUndoOrRedoHistory(redoHistory.front().first);
    
    //Feed the previous state into the undo history.
    undoHistory.push_front(make_pair(new_state, operation_name));
    delete redoHistory.front().first;
    redoHistory.pop_front();
    
    setStatus("Redo successful: " + operation_name + ".");
}


/**
 * @brief Saves the state of the area in the undo history.
 *
 * When this happens, a timer is set. During this timer, if the next change's
 * operation is the same as the previous one's, then it is ignored.
 * This is useful to stop, for instance, a slider
 * drag from saving several dozen operations in the undo history.
 *
 * @param operation_name Name of the operation.
 * @param pre_prepared_state If you have the area state prepared from
 * elsewhere in the code, specify it here.
 * Otherwise, it uses the current area state.
 */
void AreaEditor::registerChange(
    const string &operation_name, Area* pre_prepared_state
) {
    changesMgr.markAsChanged();
    
    if(game.options.areaEd.undoLimit == 0) {
        if(pre_prepared_state) {
            forgetPreparedState(pre_prepared_state);
        }
        return;
    }
    
    if(!undoSaveLockOperation.empty()) {
        if(undoSaveLockOperation == operation_name) {
            undoSaveLockTimer.start();
            return;
        }
    }
    
    Area* new_state = pre_prepared_state;
    if(!pre_prepared_state) {
        new_state = new Area();
        game.curAreaData->clone(*new_state);
    }
    undoHistory.push_front(make_pair(new_state, operation_name));
    
    for(size_t h = 0; h < redoHistory.size(); h++) {
        delete redoHistory[h].first;
    }
    redoHistory.clear();
    
    undoSaveLockOperation = operation_name;
    undoSaveLockTimer.start();
    
    updateUndoHistory();
}


/**
 * @brief Reloads all loaded areas.
 */
void AreaEditor::reloadAreas() {
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
    }
    );
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
}


/**
 * @brief Removes the current area thumbnail, if any.
 */
void AreaEditor::removeThumbnail() {
    game.curAreaData->thumbnail = nullptr;
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void AreaEditor::resetCamXY() {
    game.cam.targetPos = Point();
}


/**
 * @brief Resets the camera's zoom.
 */
void AreaEditor::resetCamZoom() {
    zoomWithCursor(1.0f);
}


/**
 * @brief Returns to a previously prepared area state.
 *
 * @param prepared_state Prepared state to return to.
 */
void AreaEditor::rollbackToPreparedState(Area* prepared_state) {
    prepared_state->clone(*(game.curAreaData));
}


/**
 * @brief Saves the area onto the disk.
 *
 * @param to_backup If false, save normally.
 * If true, save to an auto-backup file.
 * @return Whether it succeded.
 */
bool AreaEditor::saveArea(bool to_backup) {

    //First, some cleanup.
    bool deleted_sectors;
    game.curAreaData->cleanup(&deleted_sectors);
    if(deleted_sectors && !selectedSectors.empty()) {
        clearSelection();
    }
    
    //Store everything into the relevant data nodes.
    DataNode geometry_file("", "");
    DataNode main_data_file("", "");
    game.curAreaData->saveGeometryToDataNode(&geometry_file);
    game.curAreaData->saveMainDataToDataNode(&main_data_file);
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
        game.curAreaData->saveMissionDataToDataNode(&main_data_file);
    }
    
    //Save the thumbnail, or delete it if none.
    //al_save_bitmap is slow, so let's only write the thumbnail file
    //if there have been changes.
    if(
        (thumbnailNeedsSaving && !to_backup) ||
        (thumbnailBackupNeedsSaving && to_backup)
    ) {
        game.curAreaData->saveThumbnail(to_backup);
        (to_backup ? thumbnailBackupNeedsSaving : thumbnailNeedsSaving) =
            false;
    }
    
    //Finally, actually save to disk.
    string base_folder_path =
        to_backup ? game.curAreaData->userDataPath : manifest.path;
    string main_data_file_path =
        base_folder_path + "/" + FILE_NAMES::AREA_MAIN_DATA;
    string geometry_file_path =
        base_folder_path + "/" + FILE_NAMES::AREA_GEOMETRY;
        
    bool geo_save_ok = geometry_file.saveFile(geometry_file_path);
    bool main_data_save_ok = main_data_file.saveFile(main_data_file_path);
    
    if(!geo_save_ok || !main_data_save_ok) {
        showSystemMessageBox(
            nullptr, "Save failed!",
            "Could not save the area!",
            (
                "An error occured while saving the area to the folder \"" +
                base_folder_path + "\". "
                "Make sure that the folder exists and it is not read-only, "
                "and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        
        setStatus("Could not save the area!", true);
        
    }
    
    //Set up some things post-save.
    backupTimer.start(game.options.areaEd.backupInterval);
    
    saveReference();
    
    bool save_successful = geo_save_ok && main_data_save_ok;
    if(save_successful && !to_backup) {
        //If this was a normal save, save the backup too, so that the
        //maker doesn't have an outdated backup.
        saveBackup();
        
        changesMgr.markAsSaved();
        setStatus("Saved area successfully.");
        
        updateHistory(game.options.areaEd.history, manifest, game.curAreaData->name);
    }
    
    return save_successful;
}


/**
 * @brief Saves the area onto a backup file.
 */
void AreaEditor::saveBackup() {

    //Restart the timer.
    backupTimer.start(game.options.areaEd.backupInterval);
    
    saveArea(true);
}


/**
 * @brief Saves the reference data to disk, in the area's reference config file.
 */
void AreaEditor::saveReference() {
    string file_path =
        game.curAreaData->userDataPath + "/" + FILE_NAMES::AREA_REFERENCE_CONFIG;
        
    if(!referenceBitmap) {
        //The user doesn't want a reference any more.
        //Delete its config file.
        al_remove_filename(file_path.c_str());
        return;
    }
    
    DataNode reference_file("", "");
    GetterWriter gw(&reference_file);
    
    gw.get("file", referenceFilePath);
    gw.get("center", referenceCenter);
    gw.get("size", referenceSize);
    gw.get("alpha", referenceAlpha);
    gw.get("visible", showReference);
    
    reference_file.saveFile(file_path);
}


/**
 * @brief Selects an edge and its vertexes.
 *
 * @param e Edge to select.
 */
void AreaEditor::selectEdge(Edge* e) {
    if(selectionFilter == SELECTION_FILTER_VERTEXES) return;
    selectedEdges.insert(e);
    for(size_t v = 0; v < 2; v++) {
        selectVertex(e->vertexes[v]);
    }
    setSelectionStatusText();
}


/**
 * @brief Selects all path stops with the given label.
 *
 * @param label Label to search for.
 */
void AreaEditor::selectPathStopsWithLabel(const string &label) {
    clearSelection();
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        PathStop* s_ptr = game.curAreaData->pathStops[s];
        if(s_ptr->label == label) {
            selectedPathStops.insert(s_ptr);
        }
    }
    setSelectionStatusText();
}


/**
 * @brief Selects a sector and its edges and vertexes.
 *
 * @param s Sector to select.
 */
void AreaEditor::selectSector(Sector* s) {
    if(selectionFilter != SELECTION_FILTER_SECTORS) return;
    selectedSectors.insert(s);
    for(size_t e = 0; e < s->edges.size(); e++) {
        selectEdge(s->edges[e]);
    }
    setSelectionStatusText();
}


/**
 * @brief Selects a tree shadow.
 *
 * @param s_ptr Tree shadow to select.
 */
void AreaEditor::selectTreeShadow(TreeShadow* s_ptr) {
    selectedShadow = s_ptr;
    setSelectionStatusText();
}


/**
 * @brief Selects a vertex.
 *
 * @param v Vertex to select.
 */
void AreaEditor::selectVertex(Vertex* v) {
    selectedVertexes.insert(v);
    setSelectionStatusText();
    updateVertexSelection();
}


/**
 * @brief Sets the vector of points that make up a new circle sector.
 */
void AreaEditor::setNewCircleSectorPoints() {
    float anchor_angle =
        getAngle(newCircleSectorCenter, newCircleSectorAnchor);
    float cursor_angle =
        getAngle(newCircleSectorCenter, game.mouseCursor.wPos);
    float radius =
        Distance(
            newCircleSectorCenter, newCircleSectorAnchor
        ).toFloat();
    float angle_dif =
        getAngleSmallestDiff(cursor_angle, anchor_angle);
        
    size_t n_points = AREA_EDITOR::MAX_CIRCLE_SECTOR_POINTS;
    if(angle_dif > 0) {
        n_points = round(TAU / angle_dif);
    }
    n_points =
        std::clamp(
            (unsigned char) n_points,
            AREA_EDITOR::MIN_CIRCLE_SECTOR_POINTS,
            AREA_EDITOR::MAX_CIRCLE_SECTOR_POINTS
        );
        
    newCircleSectorPoints.clear();
    for(size_t p = 0; p < n_points; p++) {
        float delta_a = (TAU / n_points) * p;
        newCircleSectorPoints.push_back(
            Point(
                newCircleSectorCenter.x +
                radius * cos(anchor_angle + delta_a),
                newCircleSectorCenter.y +
                radius * sin(anchor_angle + delta_a)
            )
        );
    }
    
    newCircleSectorValidEdges.clear();
    for(size_t p = 0; p < n_points; p++) {
        Point next = getNextInVector(newCircleSectorPoints, p);
        bool valid = true;
        
        for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
            Edge* e_ptr = game.curAreaData->edges[e];
            
            if(
                lineSegsIntersect(
                    v2p(e_ptr->vertexes[0]), v2p(e_ptr->vertexes[1]),
                    newCircleSectorPoints[p], next,
                    nullptr, nullptr
                )
            ) {
                valid = false;
                break;
            }
        }
        
        newCircleSectorValidEdges.push_back(valid);
    }
}


/**
 * @brief Sets the status text based on how many things are selected.
 */
void AreaEditor::setSelectionStatusText() {
    setStatus();
    
    if(game.curAreaData && !game.curAreaData->problems.nonSimples.empty()) {
        emitTriangulationErrorStatusBarMessage(
            game.curAreaData->problems.nonSimples.begin()->second
        );
    }
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(!selectedVertexes.empty()) {
            setStatus(
                "Selected " +
                amountStr(
                    (int) selectedSectors.size(), "sector"
                ) +
                ", " +
                amountStr(
                    (int) selectedEdges.size(), "edge"
                ) +
                ", " +
                amountStr(
                    (int) selectedVertexes.size(), "vertex", "vertexes"
                ) +
                "."
            );
        }
        break;
        
    } case EDITOR_STATE_MOBS: {
        if(!selectedMobs.empty()) {
            setStatus(
                "Selected " +
                amountStr((int) selectedMobs.size(), "object") +
                "."
            );
        }
        break;
        
    } case EDITOR_STATE_PATHS: {
        if(!selectedPathLinks.empty() || !selectedPathStops.empty()) {
            size_t normals_found = 0;
            size_t one_ways_found = 0;
            for(const auto &l : selectedPathLinks) {
                if(l->endPtr->get_link(l->startPtr)) {
                    //They both link to each other. So it's a two-way.
                    normals_found++;
                } else {
                    one_ways_found++;
                }
            }
            setStatus(
                "Selected " +
                amountStr((int) selectedPathStops.size(), "path stop") +
                ", " +
                amountStr(
                    (int) ((normals_found / 2.0f) + one_ways_found),
                    "path link"
                ) +
                "."
            );
        }
        break;
        
    } case EDITOR_STATE_DETAILS: {
        if(selectedShadow) {
            setStatus("Selected a tree shadow.");
        }
        break;
        
    }
    }
}


/**
 * @brief Changes the state of the area using one of the saved states in the
 * undo history or redo history.
 *
 * @param state State to load.
 */
void AreaEditor::setStateFromUndoOrRedoHistory(Area* state) {
    state->clone(*(game.curAreaData));
    
    undoSaveLockTimer.stop();
    undoSaveLockOperation.clear();
    updateUndoHistory();
    
    clearSelection();
    clearCircleSector();
    clearLayoutDrawing();
    clearLayoutMoving();
    clearProblems();
    
    updateAllEdgeOffsetCaches();
    
    pathPreview.clear(); //Clear so it doesn't reference deleted stops.
    pathPreviewTimer.start(false);
    
    changesMgr.markAsChanged();
}


/**
 * @brief Sets up the editor's logic to split a sector.
 */
void AreaEditor::setupSectorSplit() {
    if(drawingNodes.size() < 2) {
        cancelLayoutDrawing();
        return;
    }
    
    sectorSplitInfo.preSplitAreaData = prepareState();
    
    //The idea is as follows: To split the working sector, we create a new
    //sector that takes up some of the same area as the working sector.
    //To do so, we traverse the sector's edges, from the last split point,
    //until we find the first split point. That path, plus the split, make up
    //the new sector.
    //Normally that's all, but if the cut is made against inner sectors of
    //the working sector, things get a bit trickier.
    //If the edges we traversed end up creating a sector that consumers that
    //inner sector, that won't do. Instead, the inner sector will have to be
    //created based on traversal in the opposite direction.
    //At the end, when the new sector is made, check its insides to see if
    //it must adopt some of the working sector's children sectors.
    
    //Figure out what the working sector is.
    //The middle point of two drawing nodes will always be in the working
    //sector, so it's a great place to check.
    sectorSplitInfo.workingSector =
        getSectorUnderPoint(
            (drawingNodes[0].snappedSpot + drawingNodes[1].snappedSpot) /
            2.0f
        );
    sectorSplitInfo.workingSectorOldEdges.clear();
    if(sectorSplitInfo.workingSector) {
        sectorSplitInfo.workingSectorOldEdges =
            sectorSplitInfo.workingSector->edges;
    } else {
        for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
            Edge* e_ptr = game.curAreaData->edges[e];
            if(e_ptr->sectors[0] == nullptr || e_ptr->sectors[1] == nullptr) {
                sectorSplitInfo.workingSectorOldEdges.push_back(e_ptr);
            }
        }
    }
    
    //First, create vertexes wherever necessary.
    createDrawingVertexes();
}


/**
 * @brief Sets up the editor for a new area,
 * be it from an existing file or from scratch, after the actual creation/load
 * takes place.
 */
void AreaEditor::setupForNewAreaPost() {
    clearUndoHistory();
    updateUndoHistory();
    updateAllEdgeOffsetCaches();
}


/**
 * @brief Sets up the editor for a new area,
 * be it from an existing file or from scratch, before the actual creation/load
 * takes place.
 */
void AreaEditor::setupForNewAreaPre() {
    clearCurrentArea();
    manifest.clear();
    
    game.cam.zoom = 1.0f;
    game.cam.pos = Point();
    
    state = EDITOR_STATE_MAIN;
    
    //At this point we'll have nearly unloaded some assets like the thumbnail.
    //Since Dear ImGui still hasn't rendered the current frame, which could
    //have had those assets on-screen, if it tries now it'll crash. So skip.
    game.skipDearImGuiFrame = true;
}


/**
 * @brief Procedure to start moving the selected mobs.
 */
void AreaEditor::startMobMove() {
    registerChange("object movement");
    
    moveClosestMob = nullptr;
    Distance move_closest_mob_dist;
    for(auto const &m : selectedMobs) {
        preMoveMobCoords[m] = m->pos;
        
        Distance d(game.mouseCursor.wPos, m->pos);
        if(!moveClosestMob || d < move_closest_mob_dist) {
            moveClosestMob = m;
            move_closest_mob_dist = d;
            moveStartPos = m->pos;
        }
    }
    
    moveMouseStartPos = game.mouseCursor.wPos;
    moving = true;
}


/**
 * @brief Procedure to start moving the selected path stops.
 */
void AreaEditor::startPathStopMove() {
    registerChange("path stop movement");
    
    moveClosestStop = nullptr;
    Distance move_closest_stop_dist;
    for(
        auto s = selectedPathStops.begin();
        s != selectedPathStops.end(); ++s
    ) {
        preMoveStopCoords[*s] = (*s)->pos;
        
        Distance d(game.mouseCursor.wPos, (*s)->pos);
        if(!moveClosestStop || d < move_closest_stop_dist) {
            moveClosestStop = *s;
            move_closest_stop_dist = d;
            moveStartPos = (*s)->pos;
        }
    }
    
    moveMouseStartPos = game.mouseCursor.wPos;
    moving = true;
}


/**
 * @brief Procedure to start moving the selected vertexes.
 */
void AreaEditor::startVertexMove() {
    preMoveAreaData = prepareState();
    
    moveClosestVertex = nullptr;
    Distance move_closest_vertex_dist;
    for(auto const &v : selectedVertexes) {
        Point p = v2p(v);
        preMoveVertexCoords[v] = p;
        
        Distance d(game.mouseCursor.wPos, p);
        if(!moveClosestVertex || d < move_closest_vertex_dist) {
            moveClosestVertex = v;
            move_closest_vertex_dist = d;
            moveStartPos = p;
        }
    }
    
    moveMouseStartPos = game.mouseCursor.wPos;
    moving = true;
}


/**
 * @brief Traverses a sector's edges, in order, going from neighbor to neighbor.
 *
 * Traversal starts at a vertex, and during stage 1, the encountered
 * edges/vertexes are saved in the first set of vectors.
 * The direction of travel depends on whatever the first edge is in the
 * list of edges connected to the first vertex.
 * Eventually, we should find the checkpoint vertex during traversal;
 * at this point, the algorithm will switch to stage 2 and start over,
 * this time going in the opposite direction from before, and
 * saving encountered edges/vertexes in the second set of vectors.
 * Finally, the traversal should stop when the checkpoint vertex is hit again.
 * If the sector has inner sectors, not all edges will be encountered, since
 * this algorithm only goes neighbor by neighbor.
 * If the checkpoint vertex is never found, stage 2's data will be empty.
 *
 * @param s_ptr Sector to traverse.
 * @param begin Vertex to begin in.
 * @param checkpoint Vertex to switch stages at.
 * @param edges Pointer to an array of two vectors.
 * Edges encountered during each stage are inserted into either one
 * of these vectors.
 * @param vertexes Pointer to an array of two vectors.
 * Vertexes encountered during each stage are inserted into either one
 * of these vectors.
 * @param working_sector_left This bool will be set to true if,
 * during stage 1 traversal, the working sector is to the left,
 * and false if to the right.
 */
void AreaEditor::traverseSectorForSplit(
    const Sector* s_ptr, Vertex* begin, const Vertex* checkpoint,
    vector<Edge*>* edges, vector<Vertex*>* vertexes,
    bool* working_sector_left
) {
    Edge* first_e_ptr = nullptr;
    unsigned char first_edge_visits = 0;
    
    for(unsigned char s = 0; s < 2; s++) {
        Vertex* v_ptr = begin;
        Vertex* prev_v_ptr = nullptr;
        float prev_e_angle = TAU / 2.0f;
        
        while(true) {
            Edge* next_e_ptr = nullptr;
            float next_e_angle = 0.0f;
            Vertex* next_v_ptr = nullptr;
            
            findTraceEdge(
                v_ptr, prev_v_ptr, s_ptr, prev_e_angle, s == 0,
                &next_e_ptr, &next_e_angle, &next_v_ptr, nullptr
            );
            
            if(!next_e_ptr) {
                return;
            }
            
            if(!first_e_ptr) {
                first_e_ptr = next_e_ptr;
                //In stage 1, travelling in this direction, is the
                //working sector to the left or to the right?
                if(next_e_ptr->vertexes[0] == begin) {
                    //This edge travels in the same direction as us. Side 0 is
                    //to the left, side 1 is to the right, so just check if the
                    //working sector is to the left.
                    *working_sector_left = (next_e_ptr->sectors[0] == s_ptr);
                } else {
                    //This edge travels the opposite way. Same logic as above,
                    //but reversed.
                    *working_sector_left = (next_e_ptr->sectors[1] == s_ptr);
                }
            }
            
            prev_v_ptr = v_ptr;
            prev_e_angle = next_e_angle;
            v_ptr = next_v_ptr;
            
            edges[s].push_back(next_e_ptr);
            vertexes[s].push_back(next_v_ptr);
            
            if(next_v_ptr == checkpoint) {
                //Enter stage 2, or quit.
                break;
            }
            
            if(next_e_ptr == first_e_ptr) {
                first_edge_visits++;
                if(first_edge_visits == 2) {
                    //We retreaded old ground without finding the checkpoint?
                    //Finish the algorithm right now.
                    return;
                }
            }
        }
    }
}


/**
 * @brief Undoes the last change to the area using the undo history,
 * if available.
 */
void AreaEditor::undo() {
    if(undoHistory.empty()) {
        setStatus("Nothing to undo.");
        return;
    }
    
    //Let's first save the state of things right now so we can feed it into
    //the redo history afterwards.
    Area* new_state = new Area();
    game.curAreaData->clone(*new_state);
    string operation_name = undoHistory.front().second;
    
    //Change the area state.
    setStateFromUndoOrRedoHistory(undoHistory.front().first);
    
    //Feed the previous state into the redo history.
    redoHistory.push_front(make_pair(new_state, operation_name));
    delete undoHistory.front().first;
    undoHistory.pop_front();
    
    setStatus("Undo successful: " + operation_name + ".");
}


/**
 * @brief Undoes the last placed layout drawing node.
 */
void AreaEditor::undoLayoutDrawingNode() {
    if(drawingNodes.empty()) return;
    drawingNodes.erase(
        drawingNodes.begin() + drawingNodes.size() - 1
    );
    if(
        sectorSplitInfo.uselessSplitPart2Checkpoint != INVALID &&
        drawingNodes.size() < sectorSplitInfo.uselessSplitPart2Checkpoint
    ) {
        //Back to before useless split part 2. Remove the checkpoint.
        sectorSplitInfo.uselessSplitPart2Checkpoint = INVALID;
    }
    updateLayoutDrawingStatusText();
}


/**
 * @brief Unloads the editor from memory.
 */
void AreaEditor::unload() {
    Editor::unload();
    
    clearUndoHistory();
    
    if(copyBufferSector) {
        delete copyBufferSector;
        copyBufferSector = nullptr;
    }
    if(copyBufferEdge) {
        delete copyBufferEdge;
        copyBufferEdge = nullptr;
    }
    if(copyBufferMob) {
        delete copyBufferMob;
        copyBufferMob = nullptr;
    }
    if(copyBufferPathLink) {
        delete copyBufferPathLink;
        copyBufferPathLink = nullptr;
    }
    
    clearCurrentArea();
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
        CONTENT_TYPE_WEATHER_CONDITION,
        CONTENT_TYPE_MOB_TYPE,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
}


/**
 * @brief Updates all edge offset caches relevant to the area editor.
 */
void AreaEditor::updateAllEdgeOffsetCaches() {
    game.wallSmoothingEffectCaches.clear();
    game.wallSmoothingEffectCaches.insert(
        game.wallSmoothingEffectCaches.begin(),
        game.curAreaData->edges.size(),
        EdgeOffsetCache()
    );
    updateOffsetEffectCaches(
        game.wallSmoothingEffectCaches,
        unordered_set<Vertex*>(
            game.curAreaData->vertexes.begin(),
            game.curAreaData->vertexes.end()
        ),
        doesEdgeHaveLedgeSmoothing,
        getLedgeSmoothingLength,
        getLedgeSmoothingColor
    );
    game.wallShadowEffectCaches.clear();
    game.wallShadowEffectCaches.insert(
        game.wallShadowEffectCaches.begin(),
        game.curAreaData->edges.size(),
        EdgeOffsetCache()
    );
    updateOffsetEffectCaches(
        game.wallShadowEffectCaches,
        unordered_set<Vertex*>(
            game.curAreaData->vertexes.begin(),
            game.curAreaData->vertexes.end()
        ),
        doesEdgeHaveWallShadow,
        getWallShadowLength,
        getWallShadowColor
    );
    game.liquidLimitEffectCaches.clear();
    game.liquidLimitEffectCaches.insert(
        game.liquidLimitEffectCaches.begin(),
        game.curAreaData->edges.size(),
        EdgeOffsetCache()
    );
    updateOffsetEffectCaches(
        game.liquidLimitEffectCaches,
        unordered_set<Vertex*>(
            game.curAreaData->vertexes.begin(),
            game.curAreaData->vertexes.end()
        ),
        doesEdgeHaveLiquidLimit,
        getLiquidLimitLength,
        getLiquidLimitColor
    );
}


/**
 * @brief Updates the status text according to what's going on in the current
 * sector drawing.
 *
 */
void AreaEditor::updateLayoutDrawingStatusText() {
    bool useless_split_part_2 = false;
    if(
        sectorSplitInfo.uselessSplitPart2Checkpoint !=
        INVALID &&
        drawingNodes.size() >=
        sectorSplitInfo.uselessSplitPart2Checkpoint
    ) {
        useless_split_part_2 = true;
    }
    
    if(useless_split_part_2) {
        setStatus(
            "To split this sector, continue your "
            "drawing to make a new sector."
        );
    } else {
        setStatus("Use the canvas to draw a sector.");
    }
}


/**
 * @brief Updates the reference image's bitmap, since its file name
 * just changed.
 */
void AreaEditor::updateReference() {
    if(referenceBitmap && referenceBitmap != game.bmpError) {
        al_destroy_bitmap(referenceBitmap);
    }
    referenceBitmap = nullptr;
    
    if(!referenceFilePath.empty()) {
        referenceBitmap =
            loadBmp(referenceFilePath, nullptr, false, true, true);
            
        if(
            referenceSize.x == 0 ||
            referenceSize.y == 0
        ) {
            //Let's assume this is a new reference. Reset sizes and alpha.
            referenceSize = getBitmapDimensions(referenceBitmap);
            referenceAlpha = AREA_EDITOR::DEF_REFERENCE_ALPHA;
        }
    } else {
        referenceCenter = Point();
        referenceSize = Point();
    }
}


/**
 * @brief Updates a sector's texture.
 *
 * @param s_ptr Sector to update.
 * @param internal_name Internal name of the new texture.
 */
void AreaEditor::updateSectorTexture(
    Sector* s_ptr, const string &internal_name
) {
    game.content.bitmaps.list.free(s_ptr->textureInfo.bmpName);
    s_ptr->textureInfo.bmpName = internal_name;
    s_ptr->textureInfo.bitmap = game.content.bitmaps.list.get(internal_name);
}


/**
 * @brief Updates the list of texture suggestions, adding a new one or
 * bumping it up.
 *
 * @param n Name of the chosen texture.
 */
void AreaEditor::updateTextureSuggestions(const string &n) {
    //First, check if it exists.
    size_t pos = INVALID;
    
    for(size_t s = 0; s < textureSuggestions.size(); s++) {
        if(textureSuggestions[s].name == n) {
            pos = s;
            break;
        }
    }
    
    if(pos == 0) {
        //Already #1? Never mind.
        return;
    } else if(pos == INVALID) {
        //If it doesn't exist, create it and add it to the top.
        textureSuggestions.insert(
            textureSuggestions.begin(),
            TextureSuggestion(n)
        );
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        TextureSuggestion s = textureSuggestions[pos];
        textureSuggestions.erase(textureSuggestions.begin() + pos);
        textureSuggestions.insert(textureSuggestions.begin(), s);
    }
    
    if(textureSuggestions.size() > AREA_EDITOR::MAX_TEXTURE_SUGGESTIONS) {
        textureSuggestions[textureSuggestions.size() - 1].destroy();
        textureSuggestions.erase(
            textureSuggestions.begin() + textureSuggestions.size() - 1
        );
    }
}


/**
 * @brief Updates the state and description of the undo button based on
 * the undo history.
 */
void AreaEditor::updateUndoHistory() {
    while(undoHistory.size() > game.options.areaEd.undoLimit) {
        undoHistory.pop_back();
    };
}


/**
 * @brief Updates the selection transformation widget's information, since
 * a new vertex was just selected.
 */
void AreaEditor::updateVertexSelection() {
    Point sel_tl(FLT_MAX, FLT_MAX);
    Point sel_br(-FLT_MAX, -FLT_MAX);
    for(Vertex* v : selectedVertexes) {
        updateMinMaxCoords(sel_tl, sel_br, v2p(v));
    }
    sel_tl.x -= AREA_EDITOR::SELECTION_TW_PADDING;
    sel_tl.y -= AREA_EDITOR::SELECTION_TW_PADDING;
    sel_br.x += AREA_EDITOR::SELECTION_TW_PADDING;
    sel_br.y += AREA_EDITOR::SELECTION_TW_PADDING;
    selectionCenter = (sel_br + sel_tl) / 2.0f;
    selectionSize = sel_br - sel_tl;
    selectionAngle = 0.0f;
    selectionOrigCenter = selectionCenter;
    selectionOrigSize = selectionSize;
    selectionOrigAngle = selectionAngle;
}


/**
 * @brief Constructs a new layout drawing node object.
 *
 * @param ae_ptr Pointer to the area editor instance in charge.
 * @param mouse_click Coordinates of the mouse click.
 */
AreaEditor::LayoutDrawingNode::LayoutDrawingNode(
    const AreaEditor* ae_ptr, const Point &mouse_click
) :
    rawSpot(mouse_click),
    snappedSpot(mouse_click) {
    
    vector<std::pair<Distance, Vertex*> > merge_vertexes =
        getMergeVertexes(
            mouse_click, game.curAreaData->vertexes,
            AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
        );
    if(!merge_vertexes.empty()) {
        sort(
            merge_vertexes.begin(), merge_vertexes.end(),
        [] (std::pair<Distance, Vertex*> v1, std::pair<Distance, Vertex*> v2) -> bool {
            return v1.first < v2.first;
        }
        );
        onVertex = merge_vertexes[0].second;
        onVertexIdx = game.curAreaData->findVertexIdx(onVertex);
    }
    
    if(onVertex) {
        snappedSpot.x = onVertex->x;
        snappedSpot.y = onVertex->y;
        
    } else {
        onEdge = ae_ptr->getEdgeUnderPoint(mouse_click);
        
        if(onEdge) {
            onEdgeIdx = game.curAreaData->findEdgeIdx(onEdge);
            snappedSpot =
                getClosestPointInLineSeg(
                    v2p(onEdge->vertexes[0]), v2p(onEdge->vertexes[1]),
                    mouse_click
                );
                
        } else {
            onSector = getSector(mouse_click, &onSectorIdx, false);
            
        }
    }
}


/**
 * @brief Constructs a new layout drawing node object.
 *
 */
AreaEditor::LayoutDrawingNode::LayoutDrawingNode() {
}


/**
 * @brief Constructs a new texture suggestion object.
 *
 * @param n File name of the texture.
 */
AreaEditor::TextureSuggestion::TextureSuggestion(
    const string &n
) :
    bmp(nullptr),
    name(n) {
    
    bmp = game.content.bitmaps.list.get(name, nullptr, false);
}


/**
 * @brief Destroys a texture suggestion.
 */
void AreaEditor::TextureSuggestion::destroy() {
    game.content.bitmaps.list.free(name);
}
