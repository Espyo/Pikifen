/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor event handler function.
 */

#include <algorithm>
#include <allegro5/allegro.h>

#include "editor.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"


using std::set;


/**
 * @brief Handles a key being "char"-typed anywhere.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleKeyCharAnywhere(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F2)) {
        debugEdgeIdxs = !debugEdgeIdxs;
        if(debugEdgeIdxs) {
            setStatus("Enabled debug edge index display.");
        } else {
            setStatus("Disabled debug edge index display.");
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F3)) {
        debugSectorIdxs = !debugSectorIdxs;
        if(debugSectorIdxs) {
            setStatus("Enabled debug sector index display.");
        } else {
            setStatus("Disabled debug sector index display.");
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F4)) {
        debugVertexIdxs = !debugVertexIdxs;
        if(debugVertexIdxs) {
            setStatus("Enabled debug vertex index display.");
        } else {
            setStatus("Disabled debug vertex index display.");
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F5)) {
        debugTriangulation = !debugTriangulation;
        if(debugTriangulation) {
            setStatus("Enabled debug triangulation display.");
        } else {
            setStatus("Disabled debug triangulation display.");
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F6)) {
        debugPathIdxs = !debugPathIdxs;
        if(debugPathIdxs) {
            setStatus("Enabled debug path index display.");
        } else {
            setStatus("Disabled debug path index display.");
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_Y, true)) {
        redoCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_Z, true)) {
        undoCmd(1.0f);
        
    }
}


/**
 * @brief Handles a key being "char"-typed in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleKeyCharCanvas(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_LEFT)) {
        game.editorsView.cam.targetPos.x -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_RIGHT)) {
        game.editorsView.cam.targetPos.x +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_UP)) {
        game.editorsView.cam.targetPos.y -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_DOWN)) {
        game.editorsView.cam.targetPos.y +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.editorsView.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_MINUS)) {
        zoomOutCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS)) {
        //Nope, that's not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        zoomInCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_MINUS, false, true)) {
        gridIntervalDecreaseCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS, false, true)) {
        //Again, not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        gridIntervalIncreaseCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        zoomAndPosResetCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_R)) {
        if(state == EDITOR_STATE_MOBS && subState == EDITOR_SUB_STATE_NONE) {
            rotateMobGensToPoint(game.editorsView.mouseCursorWorldPos);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_X)) {
        snapModeCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_X, false, true)) {
        //Toggles the snap modes backwards.
        snapModeCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_BACKSPACE)) {
        undoLayoutDrawingNode();
        
    }
}


/**
 * @brief Handles a key being pressed down anywhere.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleKeyDownAnywhere(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        loadCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        quickPlayCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        quitCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_R, true)) {
        referenceToggleCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        saveCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE)) {
        escapeWasPressed = true;
        
        if(!dialogs.empty()) {
            closeTopDialog();
            
        } else if(state == EDITOR_STATE_LAYOUT) {
            if(subState == EDITOR_SUB_STATE_DRAWING) {
                cancelLayoutDrawing();
            } else if(subState == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
                cancelCircleSector();
            } else if(subState == EDITOR_SUB_STATE_NONE && moving) {
                cancelLayoutMoving();
            } else if(subState == EDITOR_SUB_STATE_NONE) {
                clearSelection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_MOBS) {
            if(
                subState == EDITOR_SUB_STATE_NEW_MOB ||
                subState == EDITOR_SUB_STATE_DUPLICATE_MOB ||
                subState == EDITOR_SUB_STATE_STORE_MOB_INSIDE ||
                subState == EDITOR_SUB_STATE_NEW_MOB_LINK ||
                subState == EDITOR_SUB_STATE_DEL_MOB_LINK
            ) {
                subState = EDITOR_SUB_STATE_NONE;
                setStatus();
            } else if(subState == EDITOR_SUB_STATE_MISSION_MOBS) {
                changeState(EDITOR_STATE_GAMEPLAY);
            } else if(subState == EDITOR_SUB_STATE_NONE) {
                clearSelection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_PATHS) {
            if(subState == EDITOR_SUB_STATE_PATH_DRAWING) {
                subState = EDITOR_SUB_STATE_NONE;
                setStatus();
            } else if(subState == EDITOR_SUB_STATE_NONE) {
                clearSelection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_DETAILS) {
            if(subState == EDITOR_SUB_STATE_NEW_SHADOW) {
                subState = EDITOR_SUB_STATE_NONE;
                setStatus();
            } else if(subState == EDITOR_SUB_STATE_NONE) {
                clearSelection();
            }
            
        } else if(state == EDITOR_STATE_MAIN) {
            quitCmd(1.0f);
            
        }
        
    }
}


/**
 * @brief Handles a key being pressed down in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleKeyDownCanvas(const ALLEGRO_EVENT& ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_1)) {
        if(state == EDITOR_STATE_PATHS) {
            pathDrawingNormals = false;
        } else if(subState == EDITOR_SUB_STATE_OCTEE) {
            octeeMode = OCTEE_MODE_OFFSET;
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_2)) {
        if(state == EDITOR_STATE_PATHS) {
            pathDrawingNormals = true;
        } else if(subState == EDITOR_SUB_STATE_OCTEE) {
            octeeMode = OCTEE_MODE_SCALE;
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_3)) {
        if(subState == EDITOR_SUB_STATE_OCTEE) {
            octeeMode = OCTEE_MODE_ANGLE;
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_A, true)) {
        selectAllCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_C)) {
        if(
            state == EDITOR_STATE_LAYOUT &&
            subState == EDITOR_SUB_STATE_NONE &&
            !moving && !selecting
        ) {
            circleSectorCmd(1.0f);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_C, true)) {
        copyPropertiesCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_D)) {
        if(
            !moving && !selecting &&
            game.options.areaEd.advancedMode
        ) {
            changeState(EDITOR_STATE_DETAILS);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_D, true)) {
        if(state == EDITOR_STATE_MOBS && !moving && !selecting) {
            duplicateMobsCmd(1.0f);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F)) {
        selectionFilterCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F, false, true)) {
        //Toggles the filter modes backwards.
        selectionFilterCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_H)) {
        if(state == EDITOR_STATE_LAYOUT && subState == EDITOR_SUB_STATE_NONE) {
            if(selectedSectors.empty()) {
                setStatus(
                    "To set a sector's height, you must first select a sector!",
                    true
                );
            } else {
                subState = EDITOR_SUB_STATE_QUICK_HEIGHT_SET;
                quickHeightSetStartPos = game.mouseCursor.winPos;
                for(const auto& s : selectedSectors) {
                    quickHeightSetStartHeights[s] = s->z;
                }
                setStatus(
                    "Move the cursor up or down to change the sector's height."
                );
            }
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_L)) {
        if(
            !moving && !selecting &&
            game.options.areaEd.advancedMode
        ) {
            changeState(EDITOR_STATE_LAYOUT);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_L, false, true)) {
        switch(state) {
        case EDITOR_STATE_MOBS: {
            if(selectedMobs.size() == 1 || selectionHomogenized) {
                if(subState == EDITOR_SUB_STATE_NEW_MOB_LINK) {
                    subState = EDITOR_SUB_STATE_NONE;
                } else {
                    subState = EDITOR_SUB_STATE_NEW_MOB_LINK;
                }
            }
            break;
        }
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_N)) {
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
            layoutDrawingCmd(1.0f);
            break;
        } case EDITOR_STATE_MOBS: {
            addNewMobCmd(1.0f);
            break;
        } case EDITOR_STATE_PATHS: {
            addNewPathCmd(1.0f);
            break;
        } case EDITOR_STATE_DETAILS: {
            addNewTreeShadowCmd(1.0f);
            break;
        }
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_O)) {
        if(
            !moving && !selecting &&
            game.options.areaEd.advancedMode
        ) {
            changeState(EDITOR_STATE_MOBS);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P)) {
        if(
            !moving && !selecting &&
            game.options.areaEd.advancedMode
        ) {
            changeState(EDITOR_STATE_PATHS);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P, false, true)) {
        previewMode = !previewMode;
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_T, true)) {
        pasteTextureCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_V, true)) {
        pastePropertiesCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_DELETE)) {
        deleteCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_HOME)) {
        zoomEverythingCmd(1.0f);
        
    }
}


/**
 * @brief Handles a keyboard key being released anywhere.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleKeyUpAnywhere(const ALLEGRO_EVENT& ev) {
    if(ev.keyboard.keycode == ALLEGRO_KEY_H) {
        if(
            state == EDITOR_STATE_LAYOUT &&
            subState == EDITOR_SUB_STATE_QUICK_HEIGHT_SET
        ) {
            quickHeightSetStartHeights.clear();
            subState = EDITOR_SUB_STATE_NONE;
            setStatus();
        }
    }
}


/**
 * @brief Handles the left mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDoubleClick(const ALLEGRO_EVENT& ev) {
    if(isCtrlPressed) {
        handleLmbDown(ev);
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(subState == EDITOR_SUB_STATE_NONE) {
            Vertex* clickedVertex =
                getVertexUnderPoint(game.editorsView.mouseCursorWorldPos);
            if(!clickedVertex) {
                Edge* clickedEdge =
                    getEdgeUnderPoint(game.editorsView.mouseCursorWorldPos);
                if(clickedEdge) {
                    registerChange("edge split");
                    Vertex* newVertex =
                        splitEdge(
                            clickedEdge, game.editorsView.mouseCursorWorldPos
                        );
                    clearSelection();
                    selectedVertexes.insert(newVertex);
                    updateVertexSelection();
                }
            }
        }
        break;
        
    }
    case EDITOR_STATE_MOBS: {
        if(subState == EDITOR_SUB_STATE_NONE) {
            MobGen* clickedMob =
                getMobUnderPoint(game.editorsView.mouseCursorWorldPos);
            if(!clickedMob) {
                addNewMobUnderCursor();
                //Quit now, otherwise the code after this will simulate a
                //regular click, and if the mob is on the grid and the cursor
                //isn't, this will deselect the mob.
                return;
            }
        }
        break;
        
    }
    case EDITOR_STATE_PATHS: {
        if(subState == EDITOR_SUB_STATE_NONE) {
            bool clickedStop =
                getPathStopUnderPoint(game.editorsView.mouseCursorWorldPos);
            if(!clickedStop) {
                PathLink* clickedLink1;
                PathLink* clickedLink2;
                bool clickedLink =
                    getPathLinkUnderPoint(
                        game.editorsView.mouseCursorWorldPos,
                        &clickedLink1, &clickedLink2
                    );
                if(clickedLink) {
                    registerChange("path link split");
                    PathStop* newStop =
                        splitPathLink(
                            clickedLink1, clickedLink2,
                            snapPoint(game.editorsView.mouseCursorWorldPos)
                        );
                    clearSelection();
                    selectedPathStops.insert(newStop);
                }
            }
        }
        break;
        
    }
    default: {
        break;
        
    }
    }
    
    handleLmbDown(ev);
}


/**
 * @brief Handles the left mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDown(const ALLEGRO_EVENT& ev) {
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        handleLmbDownLayout(ev);
        break;
        
    } case EDITOR_STATE_MOBS: {
        handleLmbDownMobs(ev);
        break;
        
    } case EDITOR_STATE_PATHS: {
        handleLmbDownPaths(ev);
        break;
        
    } case EDITOR_STATE_DETAILS: {
        handleLmbDownDetails(ev);
        break;
        
    } case EDITOR_STATE_TOOLS: {
        handleLmbDownTools(ev);
        break;
        
    } case EDITOR_STATE_REVIEW: {
        handleLmbDownReview(ev);
        break;
        
    }
    }
}


/**
 * @brief Handles the left mouse button being pressed in the canvas exclusively,
 * while in the details mode.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDownDetails(const ALLEGRO_EVENT& ev) {
    switch(subState) {
    case EDITOR_SUB_STATE_NEW_SHADOW: {

        //Create a new shadow where the cursor is.
        registerChange("tree shadow creation");
        subState = EDITOR_SUB_STATE_NONE;
        Point hotspot = snapPoint(game.editorsView.mouseCursorWorldPos);
        
        TreeShadow* newShadow = new TreeShadow(hotspot);
        newShadow->bitmap = game.bmpError;
        
        game.curAreaData->treeShadows.push_back(newShadow);
        
        selectTreeShadow(newShadow);
        setStatus(
            "Created tree shadow #" + i2s(selectedShadowIdx + 1) + "."
        );
        
        break;
        
    } case EDITOR_SUB_STATE_NONE: {

        bool twHandled = false;
        if(selectedShadow) {
            twHandled =
                curTransformationWidget.handleMouseDown(
                    game.editorsView.mouseCursorWorldPos,
                    &selectedShadow->pose.pos,
                    &selectedShadow->pose.size,
                    &selectedShadow->pose.angle,
                    1.0f / game.editorsView.cam.zoom
                );
        } else if(selectedRegion) {
            twHandled =
                curTransformationWidget.handleMouseDown(
                    game.editorsView.mouseCursorWorldPos,
                    &selectedRegion->center,
                    &selectedRegion->size,
                    nullptr,
                    1.0f / game.editorsView.cam.zoom
                );
        }
        
        if(!twHandled) {
            //Select a tree shadow.
            selectedShadow = nullptr;
            selectedShadowIdx = INVALID;
            for(
                size_t s = 0;
                s < game.curAreaData->treeShadows.size(); s++
            ) {
            
                TreeShadow* sPtr = game.curAreaData->treeShadows[s];
                Point minCoords, maxCoords;
                getTransformedRectangleBBox(
                    sPtr->pose.pos, sPtr->pose.size, sPtr->pose.angle,
                    &minCoords, &maxCoords
                );
                
                if(
                    game.editorsView.mouseCursorWorldPos.x >= minCoords.x &&
                    game.editorsView.mouseCursorWorldPos.x <= maxCoords.x &&
                    game.editorsView.mouseCursorWorldPos.y >= minCoords.y &&
                    game.editorsView.mouseCursorWorldPos.y <= maxCoords.y
                ) {
                    selectTreeShadow(sPtr);
                    break;
                }
            }
            
            //Select a region.
            selectedRegion = nullptr;
            selectedRegionIdx = INVALID;
            for(size_t r = 0; r < game.curAreaData->regions.size(); r++) {
            
                AreaRegion* rPtr = game.curAreaData->regions[r];
                Point minCoords = rPtr->center - rPtr->size / 2.0f;
                Point maxCoords = rPtr->center + rPtr->size / 2.0f;
                
                if(
                    game.editorsView.mouseCursorWorldPos.x >= minCoords.x &&
                    game.editorsView.mouseCursorWorldPos.x <= maxCoords.x &&
                    game.editorsView.mouseCursorWorldPos.y >= minCoords.y &&
                    game.editorsView.mouseCursorWorldPos.y <= maxCoords.y
                ) {
                    selectRegion(rPtr);
                    break;
                }
            }
            
            setSelectionStatusText();
        }
        
        break;
        
    }
    }
}


/**
 * @brief Handles the left mouse button being pressed in the canvas exclusively,
 * while in the layout mode.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDownLayout(const ALLEGRO_EVENT& ev) {
    switch(subState) {
    case EDITOR_SUB_STATE_DRAWING: {

        handleLmbDownLayoutDrawing(ev);
        break;
        
    } case EDITOR_SUB_STATE_CIRCLE_SECTOR: {

        //Create a new circular sector.
        Point hotspot = snapPoint(game.editorsView.mouseCursorWorldPos);
        
        if(newCircleSectorStep == 0) {
            newCircleSectorCenter = hotspot;
            newCircleSectorAnchor = newCircleSectorCenter;
            newCircleSectorStep++;
            
        } else if(newCircleSectorStep == 1) {
            newCircleSectorAnchor = hotspot;
            setNewCircleSectorPoints();
            newCircleSectorStep++;
            
        } else {
            setNewCircleSectorPoints();
            
            bool allValid = true;
            for(
                size_t e = 0; e < newCircleSectorValidEdges.size(); e++
            ) {
                if(!newCircleSectorValidEdges[e]) {
                    allValid = false;
                    break;
                }
            }
            if(!allValid) {
                setStatus("Some lines touch existing edges!", true);
            } else {
                finishCircleSector();
            }
            
        }
        
        break;
        
    } case EDITOR_SUB_STATE_OCTEE: {

        moving = true;
        octeeDragStart = game.editorsView.mouseCursorWorldPos;
        Sector* sPtr = *selectedSectors.begin();
        octeeOrigAngle = sPtr->textureInfo.tf.rot;
        octeeOrigOffset = sPtr->textureInfo.tf.trans;
        octeeOrigScale = sPtr->textureInfo.tf.scale;
        
        break;
        
    } case EDITOR_SUB_STATE_NONE: {

        bool twHandled = false;
        if(
            game.options.areaEd.selTrans &&
            selectedVertexes.size() >= 2
        ) {
            twHandled =
                curTransformationWidget.handleMouseDown(
                    game.editorsView.mouseCursorWorldPos,
                    &selectionCenter,
                    &selectionSize,
                    &selectionAngle,
                    1.0f / game.editorsView.cam.zoom
                );
        }
        
        if(!twHandled) {
        
            //Start a new layout selection or select something.
            bool startNewSelection = true;
            
            Vertex* clickedVertex = nullptr;
            Edge* clickedEdge = nullptr;
            Sector* clickedSector = nullptr;
            getHoveredLayoutElement(
                &clickedVertex, &clickedEdge, &clickedSector
            );
            
            if(!isShiftPressed) {
                if(clickedVertex || clickedEdge || clickedSector) {
                    startNewSelection = false;
                }
                
            }
            
            if(startNewSelection) {
                if(!isCtrlPressed) clearSelection();
                selecting = true;
                selectionStart = game.editorsView.mouseCursorWorldPos;
                selectionEnd = game.editorsView.mouseCursorWorldPos;
                
            } else {
            
                if(clickedVertex) {
                    if(!isInContainer(selectedVertexes, clickedVertex)) {
                        if(!isCtrlPressed) {
                            clearSelection();
                        }
                        selectVertex(clickedVertex);
                    }
                } else if(clickedEdge) {
                    if(!isInContainer(selectedEdges, clickedEdge)) {
                        if(!isCtrlPressed) {
                            clearSelection();
                        }
                        selectEdge(clickedEdge);
                    }
                } else {
                    if(!isInContainer(selectedSectors, clickedSector)) {
                        if(!isCtrlPressed) {
                            clearSelection();
                        }
                        selectSector(clickedSector);
                    }
                }
                
            }
            
            selectionHomogenized = false;
            setSelectionStatusText();
            
        }
        break;
        
    }
    }
}


/**
 * @brief Handles the left mouse button being pressed in the canvas exclusively,
 * while drawing in the layout mode.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDownLayoutDrawing(const ALLEGRO_EVENT& ev) {
    //Drawing the layout.
    Point hotspot = snapPoint(game.editorsView.mouseCursorWorldPos);
    
    //First, check if the user is trying to undo the previous node.
    if(
        !drawingNodes.empty() &&
        Distance(
            hotspot,
            Point(
                drawingNodes.back().snappedSpot.x,
                drawingNodes.back().snappedSpot.y
            )
        ) <= AREA_EDITOR::VERTEX_MERGE_RADIUS / game.editorsView.cam.zoom
    ) {
        undoLayoutDrawingNode();
        return;
    }
    
    if(drawingNodes.empty()) {
        //First node.
        drawingNodes.push_back(LayoutDrawingNode(this, hotspot));
        
    } else {
    
        checkDrawingLine(hotspot);
        
        bool needsReverse = false;
        if(drawingLineResult == DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX) {
            //Instead of throwing an error, let's swap the order around.
            needsReverse = true;
            drawingLineResult = DRAWING_LINE_RESULT_OK;
        }
        
        if(drawingLineResult != DRAWING_LINE_RESULT_OK) {
            handleLineError();
            
        } else if(
            Distance(hotspot, drawingNodes.begin()->snappedSpot) <=
            AREA_EDITOR::VERTEX_MERGE_RADIUS / game.editorsView.cam.zoom
        ) {
            //Back to the first vertex. Finish the drawing.
            finishNewSectorDrawing();
            
        } else {
            //Create a new node.
            drawingNodes.push_back(LayoutDrawingNode(this, hotspot));
            
            if(needsReverse) {
                //This is now a sector split drawing.
                std::reverse(
                    drawingNodes.begin(), drawingNodes.end()
                );
            }
            
            if(
                drawingNodes.back().onEdge ||
                drawingNodes.back().onVertex
            ) {
                //Split the sector.
                setupSectorSplit();
                SECTOR_SPLIT_RESULT result =
                    getSectorSplitEvaluation();
                switch(result) {
                case SECTOR_SPLIT_RESULT_OK: {
                    doSectorSplit();
                    break;
                    
                } case SECTOR_SPLIT_RESULT_INVALID: {
                    rollbackToPreparedState(
                        sectorSplitInfo.preSplitAreaData
                    );
                    forgetPreparedState(
                        sectorSplitInfo.preSplitAreaData
                    );
                    clearSelection();
                    clearLayoutDrawing();
                    subState = EDITOR_SUB_STATE_NONE;
                    setStatus(
                        "That's not a valid split!",
                        true
                    );
                    break;
                    
                } case SECTOR_SPLIT_RESULT_USELESS: {
                    rollbackToPreparedState(
                        sectorSplitInfo.preSplitAreaData
                    );
                    forgetPreparedState(
                        sectorSplitInfo.preSplitAreaData
                    );
                    recreateDrawingNodes();
                    sectorSplitInfo.uselessSplitPart2Checkpoint =
                        drawingNodes.size();
                    updateLayoutDrawingStatusText();
                    break;
                }
                }
            }
        }
    }
}


/**
 * @brief Handles the left mouse button being pressed in the canvas exclusively,
 * while in the mobs mode.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDownMobs(const ALLEGRO_EVENT& ev) {
    switch(subState) {
    case EDITOR_SUB_STATE_NEW_MOB: {

        //Create a mob where the cursor is.
        addNewMobUnderCursor();
        
        break;
        
    } case EDITOR_SUB_STATE_DUPLICATE_MOB: {

        //Duplicate the current mobs to where the cursor is.
        registerChange("object duplication");
        subState = EDITOR_SUB_STATE_NONE;
        Point hotspot = snapPoint(game.editorsView.mouseCursorWorldPos);
        
        Point selectionTL = (*selectedMobs.begin())->pos;
        Point selectionBR = selectionTL;
        for(auto m = selectedMobs.begin(); m != selectedMobs.end(); ++m) {
            if(m == selectedMobs.begin()) continue;
            if((*m)->pos.x < selectionTL.x) {
                selectionTL.x = (*m)->pos.x;
            }
            if((*m)->pos.x > selectionBR.x) {
                selectionBR.x = (*m)->pos.x;
            }
            if((*m)->pos.y < selectionTL.y) {
                selectionTL.y = (*m)->pos.y;
            }
            if((*m)->pos.y > selectionBR.y) {
                selectionBR.y = (*m)->pos.y;
            }
        }
        Point newSelectionCenter = (selectionBR + selectionTL) / 2.0;
        set<MobGen*> mobsToSelect;
        
        for(auto const& m : selectedMobs) {
            MobGen* newMg = new MobGen(*m);
            newMg->pos = Point(hotspot + (m->pos) - newSelectionCenter);
            game.curAreaData->mobGenerators.push_back(newMg);
            mobsToSelect.insert(newMg);
        }
        
        clearSelection();
        selectedMobs = mobsToSelect;
        
        setStatus(
            "Duplicated " +
            amountStr((int) selectedMobs.size(), "object") + "."
        );
        
        break;
        
    } case EDITOR_SUB_STATE_STORE_MOB_INSIDE: {

        //Store the mob inside another.
        size_t targetIdx;
        MobGen* target =
            getMobUnderPoint(game.editorsView.mouseCursorWorldPos, &targetIdx);
        if(!target) return;
        
        for(auto const& m : selectedMobs) {
            if(m == target) {
                setStatus(
                    "You can't store to an object inside itself!",
                    true
                );
                return;
            }
        }
        MobGen* mPtr = *(selectedMobs.begin());
        if(mPtr->storedInside == targetIdx) {
            setStatus(
                "The object is already stored inside that object!",
                true
            );
            return;
        }
        
        registerChange("Object in object storing");
        
        mPtr->storedInside = targetIdx;
        
        homogenizeSelectedMobs();
        
        subState = EDITOR_SUB_STATE_NONE;
        setStatus("Stored the object inside another.");
        
        break;
        
    } case EDITOR_SUB_STATE_NEW_MOB_LINK: {

        //Link two mobs.
        MobGen* target = getMobUnderPoint(game.editorsView.mouseCursorWorldPos);
        if(!target) return;
        
        for(auto const& m : selectedMobs) {
            if(m == target) {
                setStatus(
                    "You can't link to an object to itself!",
                    true
                );
                return;
            }
        }
        MobGen* mPtr = *(selectedMobs.begin());
        for(size_t l = 0; l < mPtr->links.size(); l++) {
            if(mPtr->links[l] == target) {
                setStatus(
                    "The object already links to that object!",
                    true
                );
                return;
            }
        }
        
        registerChange("Object link creation");
        
        mPtr->links.push_back(target);
        mPtr->linkIdxs.push_back(
            game.curAreaData->findMobGenIdx(target)
        );
        
        homogenizeSelectedMobs();
        
        subState = EDITOR_SUB_STATE_NONE;
        setStatus("Linked the two objects.");
        
        break;
        
    } case EDITOR_SUB_STATE_DEL_MOB_LINK: {

        //Delete a mob link.
        MobGen* target = getMobUnderPoint(game.editorsView.mouseCursorWorldPos);
        MobGen* mPtr = *(selectedMobs.begin());
        
        if(!target) {
            std::pair<MobGen*, MobGen*> data1;
            std::pair<MobGen*, MobGen*> data2;
            if(
                !getMobLinkUnderPoint(
                    game.editorsView.mouseCursorWorldPos, &data1, &data2
                )
            ) {
                return;
            }
            
            if(
                data1.first != mPtr &&
                data1.second != mPtr &&
                data2.first != mPtr &&
                data2.second != mPtr
            ) {
                setStatus(
                    "That link does not belong to the current object!",
                    true
                );
                return;
            }
            
            if(data1.first == mPtr) {
                target = data1.second;
            } else if(data2.first == mPtr) {
                target = data2.second;
            }
        }
        
        size_t linkI = 0;
        for(; linkI < mPtr->links.size(); linkI++) {
            if(mPtr->links[linkI] == target) {
                break;
            }
        }
        
        if(linkI == mPtr->links.size()) {
            setStatus(
                "That object is not linked by the current one!",
                true
            );
            return;
        } else {
            registerChange("Object link deletion");
            mPtr->links.erase(mPtr->links.begin() + linkI);
            mPtr->linkIdxs.erase(mPtr->linkIdxs.begin() + linkI);
        }
        
        homogenizeSelectedMobs();
        
        subState = EDITOR_SUB_STATE_NONE;
        setStatus("Deleted object link.");
        
        break;
        
    } case EDITOR_SUB_STATE_MISSION_MOBS: {

        size_t clickedMobIdx;
        getMobUnderPoint(
            game.editorsView.mouseCursorWorldPos, &clickedMobIdx
        );
        
        if(clickedMobIdx != INVALID) {
            auto& listRef =
                game.curAreaData->mission.mobChecklists[
                    curMobChecklistIdx
                ].mobIdxs;
            registerChange("mission mob checklist choice change");
            auto it = std::find(listRef.begin(), listRef.end(), clickedMobIdx);
            if(it == listRef.end()) {
                listRef.push_back(clickedMobIdx);
            } else {
                listRef.erase(it);
            }
        }
        
        break;
        
    } case EDITOR_SUB_STATE_NONE: {

        //Start a new mob selection or select something.
        bool startNewSelection = true;
        MobGen* clickedMob =
            getMobUnderPoint(game.editorsView.mouseCursorWorldPos);
            
        if(!isShiftPressed) {
            if(clickedMob) {
                startNewSelection = false;
            }
        }
        
        if(startNewSelection) {
            if(!isCtrlPressed) clearSelection();
            selecting = true;
            selectionStart = game.editorsView.mouseCursorWorldPos;
            selectionEnd = game.editorsView.mouseCursorWorldPos;
            
        } else {
            if(!isInContainer(selectedMobs, clickedMob)) {
                if(!isCtrlPressed) {
                    clearSelection();
                }
                selectedMobs.insert(clickedMob);
            }
            
        }
        
        selectionHomogenized = false;
        setSelectionStatusText();
        
        break;
        
    }
    }
}


/**
 * @brief Handles the left mouse button being pressed in the canvas exclusively,
 * while in the paths mode.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDownPaths(const ALLEGRO_EVENT& ev) {
    switch(subState) {
    case EDITOR_SUB_STATE_PATH_DRAWING: {

        //Drawing a path.
        Point hotspot =
            snapPoint(game.editorsView.mouseCursorWorldPos);
        PathStop* clickedStop =
            getPathStopUnderPoint(game.editorsView.mouseCursorWorldPos);
            
        //Split a link, if one was clicked.
        if(!clickedStop) {
            PathLink* clickedLink1;
            PathLink* clickedLink2;
            bool clickedLink =
                getPathLinkUnderPoint(
                    game.editorsView.mouseCursorWorldPos,
                    &clickedLink1, &clickedLink2
                );
            if(clickedLink) {
                registerChange("path link split");
                clickedStop =
                    splitPathLink(
                        clickedLink1, clickedLink2,
                        snapPoint(game.editorsView.mouseCursorWorldPos)
                    );
                clearSelection();
                selectedPathStops.insert(clickedStop);
            }
        }
        
        if(pathDrawingStop1) {
            //A starting stop already exists, so now we create a link.
            PathStop* nextStop = nullptr;
            if(clickedStop) {
                if(clickedStop == pathDrawingStop1) {
                    pathDrawingStop1 = nullptr;
                } else {
                    nextStop = clickedStop;
                }
            } else {
                registerChange("path stop creation");
                nextStop = new PathStop(hotspot);
                nextStop->flags = pathDrawingFlags;
                nextStop->label = pathDrawingLabel;
                game.curAreaData->pathStops.push_back(nextStop);
                setStatus("Created path stop.");
            }
            
            if(nextStop) {
                registerChange("path stop link");
                pathDrawingStop1->addNewLink(
                    nextStop, pathDrawingNormals
                );
                PathLink* l1 = pathDrawingStop1->getLink(nextStop);
                PathLink* l2 = nextStop->getLink(pathDrawingStop1);
                l1->type = pathDrawingType;
                if(l2) {
                    l2->type = pathDrawingType;
                }
                game.curAreaData->fixPathStopIdxs(pathDrawingStop1);
                game.curAreaData->fixPathStopIdxs(nextStop);
                nextStop->calculateDistsPlusNeighbors();
                setStatus("Created path link.");
                
                if(clickedStop) {
                    pathDrawingStop1 = nullptr;
                } else {
                    pathDrawingStop1 = nextStop;
                }
            }
            
        } else {
            //We need to create or assign a starting stop.
            if(clickedStop) {
                pathDrawingStop1 = clickedStop;
            } else {
                registerChange("path stop creation");
                pathDrawingStop1 = new PathStop(hotspot);
                pathDrawingStop1->flags = pathDrawingFlags;
                pathDrawingStop1->label = pathDrawingLabel;
                game.curAreaData->pathStops.push_back(
                    pathDrawingStop1
                );
                setStatus("Created path stop.");
            }
            
        }
        
        pathPreview.clear(); //Clear so it doesn't reference deleted stops.
        pathPreviewTimer.start(false);
        
        break;
        
    } case EDITOR_SUB_STATE_NONE: {

        //First, check if the user clicked on a path preview checkpoint.
        if(showPathPreview) {
            for(unsigned char c = 0; c < 2; c++) {
                if(
                    bBoxCheck(
                        pathPreviewCheckpoints[c],
                        game.editorsView.mouseCursorWorldPos,
                        AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS /
                        game.editorsView.cam.zoom
                    )
                ) {
                    clearSelection();
                    movingPathPreviewCheckpoint = c;
                    return;
                }
            }
        }
        
        //Start a new path selection or select something.
        bool startNewSelection = true;
        
        PathStop* clickedStop =
            getPathStopUnderPoint(game.editorsView.mouseCursorWorldPos);
        PathLink* clickedLink1;
        PathLink* clickedLink2;
        bool clickedLink =
            getPathLinkUnderPoint(
                game.editorsView.mouseCursorWorldPos,
                &clickedLink1, &clickedLink2
            );
        if(!isShiftPressed) {
            if(clickedStop || clickedLink) {
                startNewSelection = false;
            }
            
        }
        
        if(startNewSelection) {
            if(!isCtrlPressed) clearSelection();
            selecting = true;
            selectionStart = game.editorsView.mouseCursorWorldPos;
            selectionEnd = game.editorsView.mouseCursorWorldPos;
            
        } else {
        
            if(clickedStop) {
                if(!isInContainer(selectedPathStops, clickedStop)) {
                    if(!isCtrlPressed) {
                        clearSelection();
                    }
                    selectedPathStops.insert(clickedStop);
                }
            } else {
                if(!isInContainer(selectedPathLinks, clickedLink1)) {
                    if(!isCtrlPressed) {
                        clearSelection();
                    }
                    selectedPathLinks.insert(clickedLink1);
                    if(clickedLink2 != nullptr) {
                        selectedPathLinks.insert(clickedLink2);
                    }
                }
            }
            
            setSelectionStatusText();
            
        }
        
        break;
        
    }
    }
}


/**
 * @brief Handles the left mouse button being pressed in the canvas exclusively,
 * while in the review mode.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDownReview(const ALLEGRO_EVENT& ev) {
    if(showCrossSection) {
        movingCrossSectionPoint = -1;
        for(unsigned char p = 0; p < 2; p++) {
            if(
                bBoxCheck(
                    crossSectionCheckpoints[p],
                    game.editorsView.mouseCursorWorldPos,
                    AREA_EDITOR::CROSS_SECTION_POINT_RADIUS /
                    game.editorsView.cam.zoom
                )
            ) {
                movingCrossSectionPoint = p;
                break;
            }
        }
    }
}


/**
 * @brief Handles the left mouse button being pressed in the canvas exclusively,
 * while in the tools mode.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDownTools(const ALLEGRO_EVENT& ev) {
    if(referenceBitmap) {
        curTransformationWidget.handleMouseDown(
            game.editorsView.mouseCursorWorldPos,
            &referenceCenter,
            &referenceSize,
            nullptr,
            1.0f / game.editorsView.cam.zoom
        );
    }
}


/**
 * @brief Handles the left mouse button being dragged in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDrag(const ALLEGRO_EVENT& ev) {
    if(selecting) {
    
        Point selectionTL = selectionStart;
        Point selectionBR = selectionStart;
        updateMinMaxCoords(selectionTL, selectionBR, selectionEnd);
        selectionEnd = game.editorsView.mouseCursorWorldPos;
        
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
    
            //Selection box around the layout.
            if(!isCtrlPressed) clearSelection();
            
            for(size_t v = 0; v < game.curAreaData->vertexes.size(); v++) {
                Vertex* vPtr = game.curAreaData->vertexes[v];
                
                if(
                    vPtr->x >= selectionTL.x &&
                    vPtr->x <= selectionBR.x &&
                    vPtr->y >= selectionTL.y &&
                    vPtr->y <= selectionBR.y
                ) {
                    selectedVertexes.insert(vPtr);
                }
            }
            updateVertexSelection();
            
            if(selectionFilter != SELECTION_FILTER_VERTEXES) {
                for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
                    Edge* ePtr = game.curAreaData->edges[e];
                    
                    if(
                        ePtr->vertexes[0]->x >= selectionTL.x &&
                        ePtr->vertexes[0]->x <= selectionBR.x &&
                        ePtr->vertexes[0]->y >= selectionTL.y &&
                        ePtr->vertexes[0]->y <= selectionBR.y &&
                        ePtr->vertexes[1]->x >= selectionTL.x &&
                        ePtr->vertexes[1]->x <= selectionBR.x &&
                        ePtr->vertexes[1]->y >= selectionTL.y &&
                        ePtr->vertexes[1]->y <= selectionBR.y
                    ) {
                        selectedEdges.insert(ePtr);
                    }
                }
            }
            
            if(selectionFilter == SELECTION_FILTER_SECTORS) {
                for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
                    Sector* sPtr = game.curAreaData->sectors[s];
                    bool validSector = true;
                    
                    for(size_t e = 0; e < sPtr->edges.size(); e++) {
                        Edge* ePtr = sPtr->edges[e];
                        
                        if(
                            ePtr->vertexes[0]->x < selectionTL.x ||
                            ePtr->vertexes[0]->x > selectionBR.x ||
                            ePtr->vertexes[0]->y < selectionTL.y ||
                            ePtr->vertexes[0]->y > selectionBR.y ||
                            ePtr->vertexes[1]->x < selectionTL.x ||
                            ePtr->vertexes[1]->x > selectionBR.x ||
                            ePtr->vertexes[1]->y < selectionTL.y ||
                            ePtr->vertexes[1]->y > selectionBR.y
                        ) {
                            validSector = false;
                            break;
                        }
                    }
                    
                    if(validSector) {
                        selectedSectors.insert(sPtr);
                    }
                }
            }
            
            selectionHomogenized = false;
            setSelectionStatusText();
            
            break;
            
        } case EDITOR_STATE_MOBS: {
    
            //Selection box around mobs.
            if(!isCtrlPressed) clearSelection();
            
            for(
                size_t m = 0;
                m < game.curAreaData->mobGenerators.size(); m++
            ) {
                MobGen* mPtr = game.curAreaData->mobGenerators[m];
                float radius = getMobGenRadius(mPtr);
                
                if(
                    mPtr->pos.x - radius >= selectionTL.x &&
                    mPtr->pos.x + radius <= selectionBR.x &&
                    mPtr->pos.y - radius >= selectionTL.y &&
                    mPtr->pos.y + radius <= selectionBR.y
                ) {
                    selectedMobs.insert(mPtr);
                }
            }
            
            selectionHomogenized = false;
            setSelectionStatusText();
            
            break;
            
        } case EDITOR_STATE_PATHS: {
    
            //Selection box around path stops.
            if(!isCtrlPressed) clearSelection();
            
            for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
                PathStop* sPtr = game.curAreaData->pathStops[s];
                
                if(
                    sPtr->pos.x -
                    sPtr->radius >= selectionTL.x &&
                    sPtr->pos.x +
                    sPtr->radius <= selectionBR.x &&
                    sPtr->pos.y -
                    sPtr->radius >= selectionTL.y &&
                    sPtr->pos.y +
                    sPtr->radius <= selectionBR.y
                ) {
                    selectedPathStops.insert(sPtr);
                }
            }
            
            for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
                PathStop* sPtr = game.curAreaData->pathStops[s];
                for(size_t l = 0; l < sPtr->links.size(); l++) {
                    PathStop* s2Ptr = sPtr->links[l]->endPtr;
                    
                    if(
                        sPtr->pos.x >= selectionTL.x &&
                        sPtr->pos.x <= selectionBR.x &&
                        sPtr->pos.y >= selectionTL.y &&
                        sPtr->pos.y <= selectionBR.y &&
                        s2Ptr->pos.x >= selectionTL.x &&
                        s2Ptr->pos.x <= selectionBR.x &&
                        s2Ptr->pos.y >= selectionTL.y &&
                        s2Ptr->pos.y <= selectionBR.y
                    ) {
                        selectedPathLinks.insert(sPtr->links[l]);
                    }
                }
            }
            
            setSelectionStatusText();
            
            break;
            
        }
        }
        
    } else {
    
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
    
            bool twHandled = false;
            if(
                game.options.areaEd.selTrans &&
                selectedVertexes.size() >= 2
            ) {
                twHandled =
                    curTransformationWidget.handleMouseMove(
                        snapPoint(game.editorsView.mouseCursorWorldPos, true),
                        &selectionCenter,
                        &selectionSize,
                        &selectionAngle,
                        1.0f / game.editorsView.cam.zoom,
                        false,
                        false,
                        AREA_EDITOR::SELECTION_TW_PADDING * 2.0f,
                        isAltPressed
                    );
                if(twHandled) {
                    if(!moving) {
                        startVertexMove();
                    }
                    
                    ALLEGRO_TRANSFORM t;
                    al_identity_transform(&t);
                    al_scale_transform(
                        &t,
                        selectionSize.x / selectionOrigSize.x,
                        selectionSize.y / selectionOrigSize.y
                    );
                    al_translate_transform(
                        &t,
                        selectionCenter.x - selectionOrigCenter.x,
                        selectionCenter.y - selectionOrigCenter.y
                    );
                    al_rotate_transform(
                        &t,
                        selectionAngle - selectionOrigAngle
                    );
                    
                    for(Vertex* v : selectedVertexes) {
                        Point p = preMoveVertexCoords[v];
                        p -= selectionOrigCenter;
                        al_transform_coordinates(&t, &p.x, &p.y);
                        p += selectionOrigCenter;
                        v->x = p.x;
                        v->y = p.y;
                    }
                }
            }
            
            if(
                !twHandled &&
                !selectedVertexes.empty() &&
                subState == EDITOR_SUB_STATE_NONE
            ) {
                //Move vertexes.
                if(!moving) {
                    startVertexMove();
                }
                
                Point mouseOffset =
                    game.editorsView.mouseCursorWorldPos - moveMouseStartPos;
                Point closestVertexNewP =
                    snapPoint(
                        moveStartPos + mouseOffset, true
                    );
                Point offset =
                    closestVertexNewP - moveStartPos;
                for(Vertex* v : selectedVertexes) {
                    Point orig = preMoveVertexCoords[v];
                    v->x = orig.x + offset.x;
                    v->y = orig.y + offset.y;
                }
                
            } else if(
                subState == EDITOR_SUB_STATE_OCTEE && moving
            ) {
                //Move sector texture transformation property.
                Sector* sPtr = *selectedSectors.begin();
                
                switch(octeeMode) {
                case OCTEE_MODE_OFFSET: {
                    registerChange("sector texture offset change");
                    Point diff =
                        game.editorsView.mouseCursorWorldPos - octeeDragStart;
                    diff = rotatePoint(diff, -sPtr->textureInfo.tf.rot);
                    diff = diff / sPtr->textureInfo.tf.scale;
                    sPtr->textureInfo.tf.trans = octeeOrigOffset + diff;
                    break;
                } case OCTEE_MODE_SCALE: {
                    registerChange("sector texture scale change");
                    Point diff =
                        game.editorsView.mouseCursorWorldPos - octeeDragStart;
                    diff = rotatePoint(diff, -sPtr->textureInfo.tf.rot);
                    Point dragStartRot =
                        rotatePoint(
                            octeeDragStart, -sPtr->textureInfo.tf.rot
                        );
                    diff = diff / dragStartRot * octeeOrigScale;
                    sPtr->textureInfo.tf.scale = octeeOrigScale + diff;
                    break;
                } case OCTEE_MODE_ANGLE: {
                    registerChange("sector texture angle change");
                    float dragStartA = getAngle(octeeDragStart);
                    float cursorA =
                        getAngle(game.editorsView.mouseCursorWorldPos);
                    sPtr->textureInfo.tf.rot =
                        octeeOrigAngle + (cursorA - dragStartA);
                    break;
                }
                };
                
                homogenizeSelectedSectors();
            }
            
            break;
            
        } case EDITOR_STATE_MOBS: {
    
            if(
                !selectedMobs.empty() &&
                subState == EDITOR_SUB_STATE_NONE
            ) {
                //Move mobs.
                if(!moving) {
                    startMobMove();
                }
                
                Point mouseOffset =
                    game.editorsView.mouseCursorWorldPos - moveMouseStartPos;
                Point closestMobNewP =
                    snapPoint(moveStartPos + mouseOffset);
                Point offset = closestMobNewP - moveStartPos;
                for(
                    auto m = selectedMobs.begin();
                    m != selectedMobs.end(); ++m
                ) {
                    Point orig = preMoveMobCoords[*m];
                    (*m)->pos = orig + offset;
                }
            }
            
            break;
            
        } case EDITOR_STATE_PATHS: {
    
            if(
                !selectedPathStops.empty() &&
                subState == EDITOR_SUB_STATE_NONE
            ) {
                //Move path stops.
                if(!moving) {
                    startPathStopMove();
                }
                
                Point mouseOffset =
                    game.editorsView.mouseCursorWorldPos - moveMouseStartPos;
                Point closestStopNewP =
                    snapPoint(moveStartPos + mouseOffset);
                Point offset = closestStopNewP - moveStartPos;
                for(
                    auto s = selectedPathStops.begin();
                    s != selectedPathStops.end(); ++s
                ) {
                    Point orig = preMoveStopCoords[*s];
                    (*s)->pos.x = orig.x + offset.x;
                    (*s)->pos.y = orig.y + offset.y;
                }
                
                for(
                    auto s = selectedPathStops.begin();
                    s != selectedPathStops.end(); ++s
                ) {
                    (*s)->calculateDistsPlusNeighbors();
                }
                
                pathPreviewTimer.start(false);
                
            } else if(
                movingPathPreviewCheckpoint != -1 &&
                subState == EDITOR_SUB_STATE_NONE
            ) {
                //Move path preview checkpoints.
                pathPreviewCheckpoints[movingPathPreviewCheckpoint] =
                    snapPoint(game.editorsView.mouseCursorWorldPos);
                pathPreviewTimer.start(false);
            }
            
            break;
            
        } case EDITOR_STATE_DETAILS: {
    
            if(
                selectedShadow &&
                subState == EDITOR_SUB_STATE_NONE
            ) {
                //Move tree shadow.
                Point shadowCenter = selectedShadow->pose.pos;
                Point shadowSize = selectedShadow->pose.size;
                float shadowAngle = selectedShadow->pose.angle;
                if(
                    curTransformationWidget.handleMouseMove(
                        snapPoint(game.editorsView.mouseCursorWorldPos),
                        &shadowCenter,
                        &shadowSize,
                        &shadowAngle,
                        1.0f / game.editorsView.cam.zoom,
                        selectedShadowKeepAspectRatio,
                        false,
                        -FLT_MAX,
                        isAltPressed
                    )
                ) {
                    registerChange("tree shadow transformation");
                    selectedShadow->pose.pos = shadowCenter;
                    selectedShadow->pose.size = shadowSize;
                    selectedShadow->pose.angle = shadowAngle;
                }
            } else if(
                selectedRegion &&
                subState == EDITOR_SUB_STATE_NONE
            ) {
                //Move region.
                Point regionCenter = selectedRegion->center;
                Point regionSize = selectedRegion->size;
                if(
                    curTransformationWidget.handleMouseMove(
                        snapPoint(game.editorsView.mouseCursorWorldPos),
                        &regionCenter,
                        &regionSize,
                        nullptr,
                        1.0f / game.editorsView.cam.zoom,
                        false,
                        false,
                        -FLT_MAX,
                        isAltPressed
                    )
                ) {
                    registerChange("region transformation");
                    selectedRegion->center = regionCenter;
                    selectedRegion->size = regionSize;
                }
            }
            
            break;
            
        } case EDITOR_STATE_TOOLS: {
    
            //Move reference handle.
            curTransformationWidget.handleMouseMove(
                snapPoint(game.editorsView.mouseCursorWorldPos),
                &referenceCenter,
                &referenceSize,
                nullptr,
                1.0f / game.editorsView.cam.zoom,
                referenceKeepAspectRatio,
                false,
                5.0f,
                isAltPressed
            );
            
            break;
            
        } case EDITOR_STATE_REVIEW: {
    
            //Move cross-section points.
            if(movingCrossSectionPoint != -1) {
                crossSectionCheckpoints[movingCrossSectionPoint] =
                    snapPoint(game.editorsView.mouseCursorWorldPos);
            }
            
            break;
            
        }
        }
        
    }
}


/**
 * @brief Handles the left mouse button being released.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbUp(const ALLEGRO_EVENT& ev) {
    selecting = false;
    
    if(moving) {
        if(
            state == EDITOR_STATE_LAYOUT && subState != EDITOR_SUB_STATE_OCTEE
        ) {
            finishLayoutMoving();
        }
        moving = false;
    }
    
    curTransformationWidget.handleMouseUp();
    
    movingPathPreviewCheckpoint = -1;
    movingCrossSectionPoint = -1;
}


/**
 * @brief Handles the middle mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleMmbDoubleClick(const ALLEGRO_EVENT& ev) {
    if(!game.options.editors.mmbPan) {
        resetCamXY();
    }
}


/**
 * @brief Handles the middle mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleMmbDown(const ALLEGRO_EVENT& ev) {
    if(!game.options.editors.mmbPan) {
        resetCamZoom();
    }
}


/**
 * @brief Handles the middle mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleMmbDrag(const ALLEGRO_EVENT& ev) {
    if(game.options.editors.mmbPan) {
        panCam(ev);
    }
}


/**
 * @brief Handles the mouse coordinates being updated.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleMouseUpdate(const ALLEGRO_EVENT& ev) {
    Editor::handleMouseUpdate(ev);
    
    //Update highlighted elements.
    highlightedVertex = nullptr;
    highlightedEdge = nullptr;
    highlightedSector = nullptr;
    highlightedMob = nullptr;
    highlightedPathStop = nullptr;
    highlightedPathLink = nullptr;
    if(!isMouseInGui) {
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
            getHoveredLayoutElement(
                &highlightedVertex, &highlightedEdge, &highlightedSector
            );
            break;
        } case EDITOR_STATE_MOBS: {
            highlightedMob =
                getMobUnderPoint(game.editorsView.mouseCursorWorldPos);
            break;
        } case EDITOR_STATE_PATHS: {
            PathLink* hoveredLink1;
            
            highlightedPathStop =
                getPathStopUnderPoint(game.editorsView.mouseCursorWorldPos);
                
            if(highlightedPathStop == nullptr) {
                //Selecting the stop takes priority,
                //so keep the link null if there's a stop.
                getPathLinkUnderPoint(
                    game.editorsView.mouseCursorWorldPos,
                    &hoveredLink1, &highlightedPathLink
                );
                if(highlightedPathLink == nullptr) {
                    highlightedPathLink = hoveredLink1;
                }
            }
            break;
        }
        }
    }
    
    if(subState == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        Point hotspot = snapPoint(game.editorsView.mouseCursorWorldPos, true);
        if(newCircleSectorStep == 1) {
            newCircleSectorAnchor = hotspot;
        } else {
            setNewCircleSectorPoints();
        }
    }
    
    if(subState == EDITOR_SUB_STATE_QUICK_HEIGHT_SET) {
        float offset = getQuickHeightSetOffset();
        registerChange("quick sector height set");
        for(auto& s : selectedSectors) {
            s->z = quickHeightSetStartHeights[s] + offset;
        }
        updateAllEdgeOffsetCaches();
    }
}


/**
 * @brief Handles the mouse wheel being moved in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleMouseWheel(const ALLEGRO_EVENT& ev) {
    zoomWithCursor(
        game.editorsView.cam.zoom +
        (game.editorsView.cam.zoom * ev.mouse.dz * 0.1)
    );
}


/**
 * @brief Handles the right mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleRmbDoubleClick(const ALLEGRO_EVENT& ev) {
    if(game.options.editors.mmbPan) {
        resetCamXY();
    }
}


/**
 * @brief Handles the right mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleRmbDown(const ALLEGRO_EVENT& ev) {
    if(game.options.editors.mmbPan) {
        resetCamZoom();
    }
}


/**
 * @brief Handles the right mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleRmbDrag(const ALLEGRO_EVENT& ev) {
    if(!game.options.editors.mmbPan) {
        panCam(ev);
    }
}
