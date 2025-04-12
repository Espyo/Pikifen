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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"


using std::set;


/**
 * @brief Handles a key being "char"-typed anywhere.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleKeyCharAnywhere(const ALLEGRO_EVENT &ev) {
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
void AreaEditor::handleKeyCharCanvas(const ALLEGRO_EVENT &ev) {
    if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_LEFT)) {
        game.cam.targetPos.x -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_RIGHT)) {
        game.cam.targetPos.x +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_UP)) {
        game.cam.targetPos.y -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_DOWN)) {
        game.cam.targetPos.y +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
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
            rotateMobGensToPoint(game.mouseCursor.wPos);
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
void AreaEditor::handleKeyDownAnywhere(const ALLEGRO_EVENT &ev) {
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
                subState == EDITOR_SUB_STATE_ADD_MOB_LINK ||
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
void AreaEditor::handleKeyDownCanvas(const ALLEGRO_EVENT &ev) {
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
                quickHeightSetStartPos = game.mouseCursor.sPos;
                for(const auto &s : selectedSectors) {
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
                if(subState == EDITOR_SUB_STATE_ADD_MOB_LINK) {
                    subState = EDITOR_SUB_STATE_NONE;
                } else {
                    subState = EDITOR_SUB_STATE_ADD_MOB_LINK;
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
            newMobCmd(1.0f);
            break;
        } case EDITOR_STATE_PATHS: {
            newPathCmd(1.0f);
            break;
        } case EDITOR_STATE_DETAILS: {
            newTreeShadowCmd(1.0f);
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
void AreaEditor::handleKeyUpAnywhere(const ALLEGRO_EVENT &ev) {
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
void AreaEditor::handleLmbDoubleClick(const ALLEGRO_EVENT &ev) {
    if(isCtrlPressed) {
        handleLmbDown(ev);
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(subState == EDITOR_SUB_STATE_NONE) {
            Vertex* clicked_vertex =
                getVertexUnderPoint(game.mouseCursor.wPos);
            if(!clicked_vertex) {
                Edge* clicked_edge =
                    getEdgeUnderPoint(game.mouseCursor.wPos);
                if(clicked_edge) {
                    registerChange("edge split");
                    Vertex* new_vertex =
                        splitEdge(clicked_edge, game.mouseCursor.wPos);
                    clearSelection();
                    selectedVertexes.insert(new_vertex);
                    updateVertexSelection();
                }
            }
        }
        break;
        
    }
    case EDITOR_STATE_MOBS: {
        if(subState == EDITOR_SUB_STATE_NONE) {
            MobGen* clicked_mob =
                getMobUnderPoint(game.mouseCursor.wPos);
            if(!clicked_mob) {
                createMobUnderCursor();
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
            bool clicked_stop =
                getPathStopUnderPoint(game.mouseCursor.wPos);
            if(!clicked_stop) {
                PathLink* clicked_link_1;
                PathLink* clicked_link_2;
                bool clicked_link =
                    getPathLinkUnderPoint(
                        game.mouseCursor.wPos,
                        &clicked_link_1, &clicked_link_2
                    );
                if(clicked_link) {
                    registerChange("path link split");
                    PathStop* new_stop =
                        splitPathLink(
                            clicked_link_1, clicked_link_2,
                            game.mouseCursor.wPos
                        );
                    clearSelection();
                    selectedPathStops.insert(new_stop);
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
void AreaEditor::handleLmbDown(const ALLEGRO_EVENT &ev) {
    switch(state) {
    case EDITOR_STATE_GAMEPLAY: {

        if(subState == EDITOR_SUB_STATE_MISSION_EXIT) {
            curTransformationWidget.handleMouseDown(
                game.mouseCursor.wPos,
                &game.curAreaData->mission.goalExitCenter,
                &game.curAreaData->mission.goalExitSize,
                nullptr,
                1.0f / game.cam.zoom
            );
        }
        break;
        
    }
    case EDITOR_STATE_LAYOUT: {

        switch(subState) {
        case EDITOR_SUB_STATE_DRAWING: {
    
            //Drawing the layout.
            Point hotspot = snapPoint(game.mouseCursor.wPos);
            
            //First, check if the user is trying to undo the previous node.
            if(
                !drawingNodes.empty() &&
                Distance(
                    hotspot,
                    Point(
                        drawingNodes.back().snappedSpot.x,
                        drawingNodes.back().snappedSpot.y
                    )
                ) <= AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
            ) {
                undoLayoutDrawingNode();
                return;
            }
            
            if(drawingNodes.empty()) {
                //First node.
                drawingNodes.push_back(LayoutDrawingNode(this, hotspot));
                
            } else {
            
                checkDrawingLine(hotspot);
                
                bool needs_reverse = false;
                if(drawingLineResult == DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX) {
                    //Instead of throwing an error, let's swap the order around.
                    needs_reverse = true;
                    drawingLineResult = DRAWING_LINE_RESULT_OK;
                }
                
                if(drawingLineResult != DRAWING_LINE_RESULT_OK) {
                    handleLineError();
                    
                } else if(
                    Distance(hotspot, drawingNodes.begin()->snappedSpot) <=
                    AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
                ) {
                    //Back to the first vertex. Finish the drawing.
                    finishNewSectorDrawing();
                    
                } else {
                    //Add a new node.
                    drawingNodes.push_back(LayoutDrawingNode(this, hotspot));
                    
                    if(needs_reverse) {
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
            
            break;
            
        } case EDITOR_SUB_STATE_CIRCLE_SECTOR: {
    
            //Create a new circular sector.
            Point hotspot = snapPoint(game.mouseCursor.wPos);
            
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
                
                bool all_valid = true;
                for(
                    size_t e = 0; e < newCircleSectorValidEdges.size(); e++
                ) {
                    if(!newCircleSectorValidEdges[e]) {
                        all_valid = false;
                        break;
                    }
                }
                if(!all_valid) {
                    setStatus("Some lines touch existing edges!", true);
                } else {
                    finishCircleSector();
                }
                
            }
            
            break;
            
        } case EDITOR_SUB_STATE_OCTEE: {
    
            moving = true;
            octeeDragStart = game.mouseCursor.wPos;
            Sector* s_ptr = *selectedSectors.begin();
            octeeOrigAngle = s_ptr->textureInfo.rot;
            octeeOrigOffset = s_ptr->textureInfo.translation;
            octeeOrigScale = s_ptr->textureInfo.scale;
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            bool tw_handled = false;
            if(
                game.options.areaEd.selTrans &&
                selectedVertexes.size() >= 2
            ) {
                tw_handled =
                    curTransformationWidget.handleMouseDown(
                        game.mouseCursor.wPos,
                        &selectionCenter,
                        &selectionSize,
                        &selectionAngle,
                        1.0f / game.cam.zoom
                    );
            }
            
            if(!tw_handled) {
            
                //Start a new layout selection or select something.
                bool start_new_selection = true;
                
                Vertex* clicked_vertex = nullptr;
                Edge* clicked_edge = nullptr;
                Sector* clicked_sector = nullptr;
                getHoveredLayoutElement(
                    &clicked_vertex, &clicked_edge, &clicked_sector
                );
                
                if(!isShiftPressed) {
                    if(clicked_vertex || clicked_edge || clicked_sector) {
                        start_new_selection = false;
                    }
                    
                }
                
                if(start_new_selection) {
                    if(!isCtrlPressed) clearSelection();
                    selecting = true;
                    selectionStart = game.mouseCursor.wPos;
                    selectionEnd = game.mouseCursor.wPos;
                    
                } else {
                
                    if(clicked_vertex) {
                        if(
                            selectedVertexes.find(clicked_vertex) ==
                            selectedVertexes.end()
                        ) {
                            if(!isCtrlPressed) {
                                clearSelection();
                            }
                            selectVertex(clicked_vertex);
                        }
                    } else if(clicked_edge) {
                        if(
                            selectedEdges.find(clicked_edge) ==
                            selectedEdges.end()
                        ) {
                            if(!isCtrlPressed) {
                                clearSelection();
                            }
                            selectEdge(clicked_edge);
                        }
                    } else {
                        if(
                            selectedSectors.find(clicked_sector) ==
                            selectedSectors.end()
                        ) {
                            if(!isCtrlPressed) {
                                clearSelection();
                            }
                            selectSector(clicked_sector);
                        }
                    }
                    
                }
                
                selectionHomogenized = false;
                setSelectionStatusText();
                
            }
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_MOBS: {

        switch(subState) {
        case EDITOR_SUB_STATE_NEW_MOB: {
    
            //Create a mob where the cursor is.
            createMobUnderCursor();
            
            break;
            
        } case EDITOR_SUB_STATE_DUPLICATE_MOB: {
    
            //Duplicate the current mobs to where the cursor is.
            registerChange("object duplication");
            subState = EDITOR_SUB_STATE_NONE;
            Point hotspot = snapPoint(game.mouseCursor.wPos);
            
            Point selection_tl = (*selectedMobs.begin())->pos;
            Point selection_br = selection_tl;
            for(auto m = selectedMobs.begin(); m != selectedMobs.end(); ++m) {
                if(m == selectedMobs.begin()) continue;
                if((*m)->pos.x < selection_tl.x) {
                    selection_tl.x = (*m)->pos.x;
                }
                if((*m)->pos.x > selection_br.x) {
                    selection_br.x = (*m)->pos.x;
                }
                if((*m)->pos.y < selection_tl.y) {
                    selection_tl.y = (*m)->pos.y;
                }
                if((*m)->pos.y > selection_br.y) {
                    selection_br.y = (*m)->pos.y;
                }
            }
            Point new_selection_center = (selection_br + selection_tl) / 2.0;
            set<MobGen*> mobs_to_select;
            
            for(auto const &m : selectedMobs) {
                MobGen* new_mg = new MobGen(*m);
                new_mg->pos = Point(hotspot + (m->pos) - new_selection_center);
                game.curAreaData->mobGenerators.push_back(new_mg);
                mobs_to_select.insert(new_mg);
            }
            
            clearSelection();
            selectedMobs = mobs_to_select;
            
            setStatus(
                "Duplicated " +
                amountStr((int) selectedMobs.size(), "object") + "."
            );
            
            break;
            
        } case EDITOR_SUB_STATE_STORE_MOB_INSIDE: {
    
            //Store the mob inside another.
            size_t target_idx;
            MobGen* target =
                getMobUnderPoint(game.mouseCursor.wPos, &target_idx);
            if(!target) return;
            
            for(auto const &m : selectedMobs) {
                if(m == target) {
                    setStatus(
                        "You can't store to an object inside itself!",
                        true
                    );
                    return;
                }
            }
            MobGen* m_ptr = *(selectedMobs.begin());
            if(m_ptr->storedInside == target_idx) {
                setStatus(
                    "The object is already stored inside that object!",
                    true
                );
                return;
            }
            
            registerChange("Object in object storing");
            
            m_ptr->storedInside = target_idx;
            
            homogenizeSelectedMobs();
            
            subState = EDITOR_SUB_STATE_NONE;
            setStatus("Stored the object inside another.");
            
            break;
            
        } case EDITOR_SUB_STATE_ADD_MOB_LINK: {
    
            //Link two mobs.
            MobGen* target = getMobUnderPoint(game.mouseCursor.wPos);
            if(!target) return;
            
            for(auto const &m : selectedMobs) {
                if(m == target) {
                    setStatus(
                        "You can't link to an object to itself!",
                        true
                    );
                    return;
                }
            }
            MobGen* m_ptr = *(selectedMobs.begin());
            for(size_t l = 0; l < m_ptr->links.size(); l++) {
                if(m_ptr->links[l] == target) {
                    setStatus(
                        "The object already links to that object!",
                        true
                    );
                    return;
                }
            }
            
            registerChange("Object link creation");
            
            m_ptr->links.push_back(target);
            m_ptr->linkIdxs.push_back(
                game.curAreaData->findMobGenIdx(target)
            );
            
            homogenizeSelectedMobs();
            
            subState = EDITOR_SUB_STATE_NONE;
            setStatus("Linked the two objects.");
            
            break;
            
        } case EDITOR_SUB_STATE_DEL_MOB_LINK: {
    
            //Delete a mob link.
            MobGen* target = getMobUnderPoint(game.mouseCursor.wPos);
            MobGen* m_ptr = *(selectedMobs.begin());
            
            if(!target) {
                std::pair<MobGen*, MobGen*> data1;
                std::pair<MobGen*, MobGen*> data2;
                if(
                    !getMobLinkUnderPoint(
                        game.mouseCursor.wPos, &data1, &data2
                    )
                ) {
                    return;
                }
                
                if(
                    data1.first != m_ptr &&
                    data1.second != m_ptr &&
                    data2.first != m_ptr &&
                    data2.second != m_ptr
                ) {
                    setStatus(
                        "That link does not belong to the current object!",
                        true
                    );
                    return;
                }
                
                if(data1.first == m_ptr) {
                    target = data1.second;
                } else if(data2.first == m_ptr) {
                    target = data2.second;
                }
            }
            
            size_t link_i = 0;
            for(; link_i < m_ptr->links.size(); link_i++) {
                if(m_ptr->links[link_i] == target) {
                    break;
                }
            }
            
            if(link_i == m_ptr->links.size()) {
                setStatus(
                    "That object is not linked by the current one!",
                    true
                );
                return;
            } else {
                registerChange("Object link deletion");
                m_ptr->links.erase(m_ptr->links.begin() + link_i);
                m_ptr->linkIdxs.erase(m_ptr->linkIdxs.begin() + link_i);
            }
            
            homogenizeSelectedMobs();
            
            subState = EDITOR_SUB_STATE_NONE;
            setStatus("Deleted object link.");
            
            break;
            
        } case EDITOR_SUB_STATE_MISSION_MOBS: {
    
            size_t clicked_mob_idx;
            MobGen* clicked_mob =
                getMobUnderPoint(game.mouseCursor.wPos, &clicked_mob_idx);
                
            if(
                clicked_mob_idx != INVALID &&
                game.missionGoals[game.curAreaData->mission.goal]->
                isMobApplicable(clicked_mob->type)
            ) {
                registerChange("mission object requirements change");
                auto it =
                    game.curAreaData->mission.goalMobIdxs.find(
                        clicked_mob_idx
                    );
                if(it == game.curAreaData->mission.goalMobIdxs.end()) {
                    game.curAreaData->mission.goalMobIdxs.insert(
                        clicked_mob_idx
                    );
                } else {
                    game.curAreaData->mission.goalMobIdxs.erase(it);
                }
            }
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            //Start a new mob selection or select something.
            bool start_new_selection = true;
            MobGen* clicked_mob = getMobUnderPoint(game.mouseCursor.wPos);
            
            if(!isShiftPressed) {
                if(clicked_mob) {
                    start_new_selection = false;
                }
            }
            
            if(start_new_selection) {
                if(!isCtrlPressed) clearSelection();
                selecting = true;
                selectionStart = game.mouseCursor.wPos;
                selectionEnd = game.mouseCursor.wPos;
                
            } else {
                if(selectedMobs.find(clicked_mob) == selectedMobs.end()) {
                    if(!isCtrlPressed) {
                        clearSelection();
                    }
                    selectedMobs.insert(clicked_mob);
                }
                
            }
            
            selectionHomogenized = false;
            setSelectionStatusText();
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_PATHS: {

        switch(subState) {
        case EDITOR_SUB_STATE_PATH_DRAWING: {
    
            //Drawing a path.
            Point hotspot =
                snapPoint(game.mouseCursor.wPos);
            PathStop* clicked_stop =
                getPathStopUnderPoint(game.mouseCursor.wPos);
                
            //Split a link, if one was clicked.
            if(!clicked_stop) {
                PathLink* clicked_link_1;
                PathLink* clicked_link_2;
                bool clicked_link =
                    getPathLinkUnderPoint(
                        game.mouseCursor.wPos,
                        &clicked_link_1, &clicked_link_2
                    );
                if(clicked_link) {
                    registerChange("path link split");
                    clicked_stop =
                        splitPathLink(
                            clicked_link_1, clicked_link_2,
                            game.mouseCursor.wPos
                        );
                    clearSelection();
                    selectedPathStops.insert(clicked_stop);
                }
            }
            
            if(pathDrawingStop1) {
                //A starting stop already exists, so now we create a link.
                PathStop* next_stop = nullptr;
                if(clicked_stop) {
                    if(clicked_stop == pathDrawingStop1) {
                        pathDrawingStop1 = nullptr;
                    } else {
                        next_stop = clicked_stop;
                    }
                } else {
                    registerChange("path stop creation");
                    next_stop = new PathStop(hotspot);
                    next_stop->flags = pathDrawingFlags;
                    next_stop->label = pathDrawingLabel;
                    game.curAreaData->pathStops.push_back(next_stop);
                    setStatus("Created path stop.");
                }
                
                if(next_stop) {
                    registerChange("path stop link");
                    pathDrawingStop1->addLink(
                        next_stop, pathDrawingNormals
                    );
                    PathLink* l1 = pathDrawingStop1->get_link(next_stop);
                    PathLink* l2 = next_stop->get_link(pathDrawingStop1);
                    l1->type = pathDrawingType;
                    if(l2) {
                        l2->type = pathDrawingType;
                    }
                    game.curAreaData->fixPathStopIdxs(pathDrawingStop1);
                    game.curAreaData->fixPathStopIdxs(next_stop);
                    next_stop->calculateDistsPlusNeighbors();
                    setStatus("Created path link.");
                    
                    if(clicked_stop) {
                        pathDrawingStop1 = nullptr;
                    } else {
                        pathDrawingStop1 = next_stop;
                    }
                }
                
            } else {
                //We need to create or assign a starting stop.
                if(clicked_stop) {
                    pathDrawingStop1 = clicked_stop;
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
                        BBoxCheck(
                            pathPreviewCheckpoints[c],
                            game.mouseCursor.wPos,
                            AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS /
                            game.cam.zoom
                        )
                    ) {
                        clearSelection();
                        movingPathPreviewCheckpoint = c;
                        return;
                    }
                }
            }
            
            //Start a new path selection or select something.
            bool start_new_selection = true;
            
            PathStop* clicked_stop =
                getPathStopUnderPoint(game.mouseCursor.wPos);
            PathLink* clicked_link_1;
            PathLink* clicked_link_2;
            bool clicked_link =
                getPathLinkUnderPoint(
                    game.mouseCursor.wPos,
                    &clicked_link_1, &clicked_link_2
                );
            if(!isShiftPressed) {
                if(clicked_stop || clicked_link) {
                    start_new_selection = false;
                }
                
            }
            
            if(start_new_selection) {
                if(!isCtrlPressed) clearSelection();
                selecting = true;
                selectionStart = game.mouseCursor.wPos;
                selectionEnd = game.mouseCursor.wPos;
                
            } else {
            
                if(clicked_stop) {
                    if(
                        selectedPathStops.find(clicked_stop) ==
                        selectedPathStops.end()
                    ) {
                        if(!isCtrlPressed) {
                            clearSelection();
                        }
                        selectedPathStops.insert(clicked_stop);
                    }
                } else {
                    if(
                        selectedPathLinks.find(clicked_link_1) ==
                        selectedPathLinks.end()
                    ) {
                        if(!isCtrlPressed) {
                            clearSelection();
                        }
                        selectedPathLinks.insert(clicked_link_1);
                        if(clicked_link_2 != nullptr) {
                            selectedPathLinks.insert(clicked_link_2);
                        }
                    }
                }
                
                setSelectionStatusText();
                
            }
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_DETAILS: {

        switch(subState) {
        case EDITOR_SUB_STATE_NEW_SHADOW: {
    
            //Create a new shadow where the cursor is.
            registerChange("tree shadow creation");
            subState = EDITOR_SUB_STATE_NONE;
            Point hotspot = snapPoint(game.mouseCursor.wPos);
            
            TreeShadow* new_shadow = new TreeShadow(hotspot);
            new_shadow->bitmap = game.bmpError;
            
            game.curAreaData->treeShadows.push_back(new_shadow);
            
            selectTreeShadow(new_shadow);
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            bool transformation_handled = false;
            if(selectedShadow) {
                transformation_handled =
                    curTransformationWidget.handleMouseDown(
                        game.mouseCursor.wPos,
                        &selectedShadow->center,
                        &selectedShadow->size,
                        &selectedShadow->angle,
                        1.0f / game.cam.zoom
                    );
            }
            
            if(!transformation_handled) {
                //Select a tree shadow.
                selectedShadow = nullptr;
                for(
                    size_t s = 0;
                    s < game.curAreaData->treeShadows.size(); s++
                ) {
                
                    TreeShadow* s_ptr = game.curAreaData->treeShadows[s];
                    Point min_coords, max_coords;
                    getTransformedRectangleBBox(
                        s_ptr->center, s_ptr->size, s_ptr->angle,
                        &min_coords, &max_coords
                    );
                    
                    if(
                        game.mouseCursor.wPos.x >= min_coords.x &&
                        game.mouseCursor.wPos.x <= max_coords.x &&
                        game.mouseCursor.wPos.y >= min_coords.y &&
                        game.mouseCursor.wPos.y <= max_coords.y
                    ) {
                        selectTreeShadow(s_ptr);
                        break;
                    }
                }
                
                setSelectionStatusText();
            }
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_TOOLS: {

        if(referenceBitmap) {
            curTransformationWidget.handleMouseDown(
                game.mouseCursor.wPos,
                &referenceCenter,
                &referenceSize,
                nullptr,
                1.0f / game.cam.zoom
            );
        }
        
        break;
        
    } case EDITOR_STATE_REVIEW: {

        if(showCrossSection) {
            movingCrossSectionPoint = -1;
            for(unsigned char p = 0; p < 2; p++) {
                if(
                    BBoxCheck(
                        crossSectionCheckpoints[p], game.mouseCursor.wPos,
                        AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.cam.zoom
                    )
                ) {
                    movingCrossSectionPoint = p;
                    break;
                }
            }
        }
        
        break;
        
    }
    }
}


/**
 * @brief Handles the left mouse button being dragged in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleLmbDrag(const ALLEGRO_EVENT &ev) {
    if(selecting) {
    
        Point selection_tl = selectionStart;
        Point selection_br = selectionStart;
        updateMinMaxCoords(selection_tl, selection_br, selectionEnd);
        selectionEnd = game.mouseCursor.wPos;
        
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
    
            //Selection box around the layout.
            if(!isCtrlPressed) clearSelection();
            
            for(size_t v = 0; v < game.curAreaData->vertexes.size(); v++) {
                Vertex* v_ptr = game.curAreaData->vertexes[v];
                
                if(
                    v_ptr->x >= selection_tl.x &&
                    v_ptr->x <= selection_br.x &&
                    v_ptr->y >= selection_tl.y &&
                    v_ptr->y <= selection_br.y
                ) {
                    selectedVertexes.insert(v_ptr);
                }
            }
            updateVertexSelection();
            
            if(selectionFilter != SELECTION_FILTER_VERTEXES) {
                for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
                    Edge* e_ptr = game.curAreaData->edges[e];
                    
                    if(
                        e_ptr->vertexes[0]->x >= selection_tl.x &&
                        e_ptr->vertexes[0]->x <= selection_br.x &&
                        e_ptr->vertexes[0]->y >= selection_tl.y &&
                        e_ptr->vertexes[0]->y <= selection_br.y &&
                        e_ptr->vertexes[1]->x >= selection_tl.x &&
                        e_ptr->vertexes[1]->x <= selection_br.x &&
                        e_ptr->vertexes[1]->y >= selection_tl.y &&
                        e_ptr->vertexes[1]->y <= selection_br.y
                    ) {
                        selectedEdges.insert(e_ptr);
                    }
                }
            }
            
            if(selectionFilter == SELECTION_FILTER_SECTORS) {
                for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
                    Sector* s_ptr = game.curAreaData->sectors[s];
                    bool valid_sector = true;
                    
                    for(size_t e = 0; e < s_ptr->edges.size(); e++) {
                        Edge* e_ptr = s_ptr->edges[e];
                        
                        if(
                            e_ptr->vertexes[0]->x < selection_tl.x ||
                            e_ptr->vertexes[0]->x > selection_br.x ||
                            e_ptr->vertexes[0]->y < selection_tl.y ||
                            e_ptr->vertexes[0]->y > selection_br.y ||
                            e_ptr->vertexes[1]->x < selection_tl.x ||
                            e_ptr->vertexes[1]->x > selection_br.x ||
                            e_ptr->vertexes[1]->y < selection_tl.y ||
                            e_ptr->vertexes[1]->y > selection_br.y
                        ) {
                            valid_sector = false;
                            break;
                        }
                    }
                    
                    if(valid_sector) {
                        selectedSectors.insert(s_ptr);
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
                MobGen* m_ptr = game.curAreaData->mobGenerators[m];
                float radius = getMobGenRadius(m_ptr);
                
                if(
                    m_ptr->pos.x - radius >= selection_tl.x &&
                    m_ptr->pos.x + radius <= selection_br.x &&
                    m_ptr->pos.y - radius >= selection_tl.y &&
                    m_ptr->pos.y + radius <= selection_br.y
                ) {
                    selectedMobs.insert(m_ptr);
                }
            }
            
            selectionHomogenized = false;
            setSelectionStatusText();
            
            break;
            
        } case EDITOR_STATE_PATHS: {
    
            //Selection box around path stops.
            if(!isCtrlPressed) clearSelection();
            
            for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
                PathStop* s_ptr = game.curAreaData->pathStops[s];
                
                if(
                    s_ptr->pos.x -
                    s_ptr->radius >= selection_tl.x &&
                    s_ptr->pos.x +
                    s_ptr->radius <= selection_br.x &&
                    s_ptr->pos.y -
                    s_ptr->radius >= selection_tl.y &&
                    s_ptr->pos.y +
                    s_ptr->radius <= selection_br.y
                ) {
                    selectedPathStops.insert(s_ptr);
                }
            }
            
            for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
                PathStop* s_ptr = game.curAreaData->pathStops[s];
                for(size_t l = 0; l < s_ptr->links.size(); l++) {
                    PathStop* s2_ptr = s_ptr->links[l]->endPtr;
                    
                    if(
                        s_ptr->pos.x >= selection_tl.x &&
                        s_ptr->pos.x <= selection_br.x &&
                        s_ptr->pos.y >= selection_tl.y &&
                        s_ptr->pos.y <= selection_br.y &&
                        s2_ptr->pos.x >= selection_tl.x &&
                        s2_ptr->pos.x <= selection_br.x &&
                        s2_ptr->pos.y >= selection_tl.y &&
                        s2_ptr->pos.y <= selection_br.y
                    ) {
                        selectedPathLinks.insert(s_ptr->links[l]);
                    }
                }
            }
            
            setSelectionStatusText();
            
            break;
            
        }
        }
        
    } else {
    
        switch(state) {
        case EDITOR_STATE_GAMEPLAY: {
    
            if(subState == EDITOR_SUB_STATE_MISSION_EXIT) {
                Point exit_center =
                    game.curAreaData->mission.goalExitCenter;
                Point exit_size =
                    game.curAreaData->mission.goalExitSize;
                if(
                    curTransformationWidget.handleMouseMove(
                        snapPoint(game.mouseCursor.wPos, true),
                        &exit_center, &exit_size,
                        nullptr,
                        1.0f / game.cam.zoom,
                        false,
                        false,
                        MISSION::EXIT_MIN_SIZE,
                        isAltPressed
                    )
                ) {
                    registerChange("mission exit change");
                    game.curAreaData->mission.goalExitCenter = exit_center;
                    game.curAreaData->mission.goalExitSize = exit_size;
                }
            }
            break;
            
        }
        case EDITOR_STATE_LAYOUT: {
    
            bool tw_handled = false;
            if(
                game.options.areaEd.selTrans &&
                selectedVertexes.size() >= 2
            ) {
                tw_handled =
                    curTransformationWidget.handleMouseMove(
                        snapPoint(game.mouseCursor.wPos, true),
                        &selectionCenter,
                        &selectionSize,
                        &selectionAngle,
                        1.0f / game.cam.zoom,
                        false,
                        false,
                        AREA_EDITOR::SELECTION_TW_PADDING * 2.0f,
                        isAltPressed
                    );
                if(tw_handled) {
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
                !tw_handled &&
                !selectedVertexes.empty() &&
                subState == EDITOR_SUB_STATE_NONE
            ) {
                //Move vertexes.
                if(!moving) {
                    startVertexMove();
                }
                
                Point mouse_offset =
                    game.mouseCursor.wPos - moveMouseStartPos;
                Point closest_vertex_new_p =
                    snapPoint(
                        moveStartPos + mouse_offset, true
                    );
                Point offset =
                    closest_vertex_new_p - moveStartPos;
                for(Vertex* v : selectedVertexes) {
                    Point orig = preMoveVertexCoords[v];
                    v->x = orig.x + offset.x;
                    v->y = orig.y + offset.y;
                }
                
            } else if(
                subState == EDITOR_SUB_STATE_OCTEE && moving
            ) {
                //Move sector texture transformation property.
                Sector* s_ptr = *selectedSectors.begin();
                
                switch(octeeMode) {
                case OCTEE_MODE_OFFSET: {
                    registerChange("sector texture offset change");
                    Point diff = (game.mouseCursor.wPos - octeeDragStart);
                    diff = rotatePoint(diff, -s_ptr->textureInfo.rot);
                    diff = diff / s_ptr->textureInfo.scale;
                    s_ptr->textureInfo.translation = octeeOrigOffset + diff;
                    break;
                } case OCTEE_MODE_SCALE: {
                    registerChange("sector texture scale change");
                    Point diff = (game.mouseCursor.wPos - octeeDragStart);
                    diff = rotatePoint(diff, -s_ptr->textureInfo.rot);
                    Point drag_start_rot =
                        rotatePoint(
                            octeeDragStart, -s_ptr->textureInfo.rot
                        );
                    diff = diff / drag_start_rot * octeeOrigScale;
                    s_ptr->textureInfo.scale = octeeOrigScale + diff;
                    break;
                } case OCTEE_MODE_ANGLE: {
                    registerChange("sector texture angle change");
                    float drag_start_a = getAngle(octeeDragStart);
                    float cursor_a = getAngle(game.mouseCursor.wPos);
                    s_ptr->textureInfo.rot =
                        octeeOrigAngle + (cursor_a - drag_start_a);
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
                
                Point mouse_offset =
                    game.mouseCursor.wPos - moveMouseStartPos;
                Point closest_mob_new_p =
                    snapPoint(moveStartPos + mouse_offset);
                Point offset = closest_mob_new_p - moveStartPos;
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
                
                Point mouse_offset =
                    game.mouseCursor.wPos - moveMouseStartPos;
                Point closest_stop_new_p =
                    snapPoint(moveStartPos + mouse_offset);
                Point offset = closest_stop_new_p - moveStartPos;
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
                    snapPoint(game.mouseCursor.wPos);
                pathPreviewTimer.start(false);
            }
            
            break;
            
        } case EDITOR_STATE_DETAILS: {
    
            if(
                selectedShadow &&
                subState == EDITOR_SUB_STATE_NONE
            ) {
                //Move tree shadow.
                Point shadow_center = selectedShadow->center;
                Point shadow_size = selectedShadow->size;
                float shadow_angle = selectedShadow->angle;
                if(
                    curTransformationWidget.handleMouseMove(
                        snapPoint(game.mouseCursor.wPos),
                        &shadow_center,
                        &shadow_size,
                        &shadow_angle,
                        1.0f / game.cam.zoom,
                        selectedShadowKeepAspectRatio,
                        false,
                        -FLT_MAX,
                        isAltPressed
                    )
                ) {
                    registerChange("tree shadow transformation");
                    selectedShadow->center = shadow_center;
                    selectedShadow->size = shadow_size;
                    selectedShadow->angle = shadow_angle;
                }
            }
            
            break;
            
        } case EDITOR_STATE_TOOLS: {
    
            //Move reference handle.
            curTransformationWidget.handleMouseMove(
                snapPoint(game.mouseCursor.wPos),
                &referenceCenter,
                &referenceSize,
                nullptr,
                1.0f / game.cam.zoom,
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
                    snapPoint(game.mouseCursor.wPos);
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
void AreaEditor::handleLmbUp(const ALLEGRO_EVENT &ev) {
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
void AreaEditor::handleMmbDoubleClick(const ALLEGRO_EVENT &ev) {
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
void AreaEditor::handleMmbDown(const ALLEGRO_EVENT &ev) {
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
void AreaEditor::handleMmbDrag(const ALLEGRO_EVENT &ev) {
    if(game.options.editors.mmbPan) {
        panCam(ev);
    }
}


/**
 * @brief Handles the mouse coordinates being updated.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleMouseUpdate(const ALLEGRO_EVENT &ev) {
    game.mouseCursor.sPos.x = ev.mouse.x;
    game.mouseCursor.sPos.y = ev.mouse.y;
    game.mouseCursor.wPos = game.mouseCursor.sPos;
    al_transform_coordinates(
        &game.screenToWorldTransform,
        &game.mouseCursor.wPos.x, &game.mouseCursor.wPos.y
    );
    
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
                getMobUnderPoint(game.mouseCursor.wPos);
            break;
        } case EDITOR_STATE_PATHS: {
            PathLink* hovered_link_1;
            
            highlightedPathStop =
                getPathStopUnderPoint(game.mouseCursor.wPos);
                
            if(highlightedPathStop == nullptr) {
                //Selecting the stop takes priority,
                //so keep the link null if there's a stop.
                getPathLinkUnderPoint(
                    game.mouseCursor.wPos,
                    &hovered_link_1, &highlightedPathLink
                );
                if(highlightedPathLink == nullptr) {
                    highlightedPathLink = hovered_link_1;
                }
            }
            break;
        }
        }
    }
    
    if(subState == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        Point hotspot = snapPoint(game.mouseCursor.wPos, true);
        if(newCircleSectorStep == 1) {
            newCircleSectorAnchor = hotspot;
        } else {
            setNewCircleSectorPoints();
        }
    }
    
    if(subState == EDITOR_SUB_STATE_QUICK_HEIGHT_SET) {
        float offset = getQuickHeightSetOffset();
        registerChange("quick sector height set");
        for(auto &s : selectedSectors) {
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
void AreaEditor::handleMouseWheel(const ALLEGRO_EVENT &ev) {
    zoomWithCursor(game.cam.zoom + (game.cam.zoom * ev.mouse.dz * 0.1));
}


/**
 * @brief Handles the right mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleRmbDoubleClick(const ALLEGRO_EVENT &ev) {
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
void AreaEditor::handleRmbDown(const ALLEGRO_EVENT &ev) {
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
void AreaEditor::handleRmbDrag(const ALLEGRO_EVENT &ev) {
    if(!game.options.editors.mmbPan) {
        panCam(ev);
    }
}
