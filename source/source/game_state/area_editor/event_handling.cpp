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
        debug_edge_idxs = !debug_edge_idxs;
        if(debug_edge_idxs) {
            setStatus("Enabled debug edge index display.");
        } else {
            setStatus("Disabled debug edge index display.");
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F3)) {
        debug_sector_idxs = !debug_sector_idxs;
        if(debug_sector_idxs) {
            setStatus("Enabled debug sector index display.");
        } else {
            setStatus("Disabled debug sector index display.");
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F4)) {
        debug_vertex_idxs = !debug_vertex_idxs;
        if(debug_vertex_idxs) {
            setStatus("Enabled debug vertex index display.");
        } else {
            setStatus("Disabled debug vertex index display.");
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F5)) {
        debug_triangulation = !debug_triangulation;
        if(debug_triangulation) {
            setStatus("Enabled debug triangulation display.");
        } else {
            setStatus("Disabled debug triangulation display.");
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_F6)) {
        debug_path_idxs = !debug_path_idxs;
        if(debug_path_idxs) {
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
        game.cam.target_pos.x -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_RIGHT)) {
        game.cam.target_pos.x +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_UP)) {
        game.cam.target_pos.y -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_DOWN)) {
        game.cam.target_pos.y +=
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
        if(state == EDITOR_STATE_MOBS && sub_state == EDITOR_SUB_STATE_NONE) {
            rotateMobGensToPoint(game.mouse_cursor.w_pos);
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
        escape_was_pressed = true;
        
        if(!dialogs.empty()) {
            closeTopDialog();
            
        } else if(state == EDITOR_STATE_LAYOUT) {
            if(sub_state == EDITOR_SUB_STATE_DRAWING) {
                cancelLayoutDrawing();
            } else if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
                cancelCircleSector();
            } else if(sub_state == EDITOR_SUB_STATE_NONE && moving) {
                cancelLayoutMoving();
            } else if(sub_state == EDITOR_SUB_STATE_NONE) {
                clearSelection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_MOBS) {
            if(
                sub_state == EDITOR_SUB_STATE_NEW_MOB ||
                sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB ||
                sub_state == EDITOR_SUB_STATE_STORE_MOB_INSIDE ||
                sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK ||
                sub_state == EDITOR_SUB_STATE_DEL_MOB_LINK
            ) {
                sub_state = EDITOR_SUB_STATE_NONE;
                setStatus();
            } else if(sub_state == EDITOR_SUB_STATE_MISSION_MOBS) {
                changeState(EDITOR_STATE_GAMEPLAY);
            } else if(sub_state == EDITOR_SUB_STATE_NONE) {
                clearSelection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_PATHS) {
            if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
                sub_state = EDITOR_SUB_STATE_NONE;
                setStatus();
            } else if(sub_state == EDITOR_SUB_STATE_NONE) {
                clearSelection();
                selecting = false;
            }
            
        } else if(state == EDITOR_STATE_DETAILS) {
            if(sub_state == EDITOR_SUB_STATE_NEW_SHADOW) {
                sub_state = EDITOR_SUB_STATE_NONE;
                setStatus();
            } else if(sub_state == EDITOR_SUB_STATE_NONE) {
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
            path_drawing_normals = false;
        } else if(sub_state == EDITOR_SUB_STATE_OCTEE) {
            octee_mode = OCTEE_MODE_OFFSET;
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_2)) {
        if(state == EDITOR_STATE_PATHS) {
            path_drawing_normals = true;
        } else if(sub_state == EDITOR_SUB_STATE_OCTEE) {
            octee_mode = OCTEE_MODE_SCALE;
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_3)) {
        if(sub_state == EDITOR_SUB_STATE_OCTEE) {
            octee_mode = OCTEE_MODE_ANGLE;
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_A, true)) {
        selectAllCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_C)) {
        if(
            state == EDITOR_STATE_LAYOUT &&
            sub_state == EDITOR_SUB_STATE_NONE &&
            !moving && !selecting
        ) {
            circleSectorCmd(1.0f);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_C, true)) {
        copyPropertiesCmd(1.0f);
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_D)) {
        if(
            !moving && !selecting &&
            game.options.area_editor.advanced_mode
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
        if(state == EDITOR_STATE_LAYOUT && sub_state == EDITOR_SUB_STATE_NONE) {
            if(selected_sectors.empty()) {
                setStatus(
                    "To set a sector's height, you must first select a sector!",
                    true
                );
            } else {
                sub_state = EDITOR_SUB_STATE_QUICK_HEIGHT_SET;
                quick_height_set_start_pos = game.mouse_cursor.s_pos;
                for(const auto &s : selected_sectors) {
                    quick_height_set_start_heights[s] = s->z;
                }
                setStatus(
                    "Move the cursor up or down to change the sector's height."
                );
            }
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_L)) {
        if(
            !moving && !selecting &&
            game.options.area_editor.advanced_mode
        ) {
            changeState(EDITOR_STATE_LAYOUT);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_L, false, true)) {
        switch(state) {
        case EDITOR_STATE_MOBS: {
            if(selected_mobs.size() == 1 || selection_homogenized) {
                if(sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK) {
                    sub_state = EDITOR_SUB_STATE_NONE;
                } else {
                    sub_state = EDITOR_SUB_STATE_ADD_MOB_LINK;
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
            game.options.area_editor.advanced_mode
        ) {
            changeState(EDITOR_STATE_MOBS);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P)) {
        if(
            !moving && !selecting &&
            game.options.area_editor.advanced_mode
        ) {
            changeState(EDITOR_STATE_PATHS);
        }
        
    } else if(keyCheck(ev.keyboard.keycode, ALLEGRO_KEY_P, false, true)) {
        preview_mode = !preview_mode;
        
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
            sub_state == EDITOR_SUB_STATE_QUICK_HEIGHT_SET
        ) {
            quick_height_set_start_heights.clear();
            sub_state = EDITOR_SUB_STATE_NONE;
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
    if(is_ctrl_pressed) {
        handleLmbDown(ev);
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_LAYOUT: {
        if(sub_state == EDITOR_SUB_STATE_NONE) {
            Vertex* clicked_vertex =
                getVertexUnderPoint(game.mouse_cursor.w_pos);
            if(!clicked_vertex) {
                Edge* clicked_edge =
                    getEdgeUnderPoint(game.mouse_cursor.w_pos);
                if(clicked_edge) {
                    registerChange("edge split");
                    Vertex* new_vertex =
                        splitEdge(clicked_edge, game.mouse_cursor.w_pos);
                    clearSelection();
                    selected_vertexes.insert(new_vertex);
                    updateVertexSelection();
                }
            }
        }
        break;
        
    }
    case EDITOR_STATE_MOBS: {
        if(sub_state == EDITOR_SUB_STATE_NONE) {
            MobGen* clicked_mob =
                getMobUnderPoint(game.mouse_cursor.w_pos);
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
        if(sub_state == EDITOR_SUB_STATE_NONE) {
            bool clicked_stop =
                getPathStopUnderPoint(game.mouse_cursor.w_pos);
            if(!clicked_stop) {
                PathLink* clicked_link_1;
                PathLink* clicked_link_2;
                bool clicked_link =
                    getPathLinkUnderPoint(
                        game.mouse_cursor.w_pos,
                        &clicked_link_1, &clicked_link_2
                    );
                if(clicked_link) {
                    registerChange("path link split");
                    PathStop* new_stop =
                        splitPathLink(
                            clicked_link_1, clicked_link_2,
                            game.mouse_cursor.w_pos
                        );
                    clearSelection();
                    selected_path_stops.insert(new_stop);
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

        if(sub_state == EDITOR_SUB_STATE_MISSION_EXIT) {
            cur_transformation_widget.handleMouseDown(
                game.mouse_cursor.w_pos,
                &game.cur_area_data->mission.goal_exit_center,
                &game.cur_area_data->mission.goal_exit_size,
                nullptr,
                1.0f / game.cam.zoom
            );
        }
        break;
        
    }
    case EDITOR_STATE_LAYOUT: {

        switch(sub_state) {
        case EDITOR_SUB_STATE_DRAWING: {
    
            //Drawing the layout.
            Point hotspot = snapPoint(game.mouse_cursor.w_pos);
            
            //First, check if the user is trying to undo the previous node.
            if(
                !drawing_nodes.empty() &&
                Distance(
                    hotspot,
                    Point(
                        drawing_nodes.back().snapped_spot.x,
                        drawing_nodes.back().snapped_spot.y
                    )
                ) <= AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
            ) {
                undoLayoutDrawingNode();
                return;
            }
            
            if(drawing_nodes.empty()) {
                //First node.
                drawing_nodes.push_back(LayoutDrawingNode(this, hotspot));
                
            } else {
            
                checkDrawingLine(hotspot);
                
                bool needs_reverse = false;
                if(drawing_line_result == DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX) {
                    //Instead of throwing an error, let's swap the order around.
                    needs_reverse = true;
                    drawing_line_result = DRAWING_LINE_RESULT_OK;
                }
                
                if(drawing_line_result != DRAWING_LINE_RESULT_OK) {
                    handleLineError();
                    
                } else if(
                    Distance(hotspot, drawing_nodes.begin()->snapped_spot) <=
                    AREA_EDITOR::VERTEX_MERGE_RADIUS / game.cam.zoom
                ) {
                    //Back to the first vertex. Finish the drawing.
                    finishNewSectorDrawing();
                    
                } else {
                    //Add a new node.
                    drawing_nodes.push_back(LayoutDrawingNode(this, hotspot));
                    
                    if(needs_reverse) {
                        //This is now a sector split drawing.
                        std::reverse(
                            drawing_nodes.begin(), drawing_nodes.end()
                        );
                    }
                    
                    if(
                        drawing_nodes.back().on_edge ||
                        drawing_nodes.back().on_vertex
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
                                sector_split_info.pre_split_area_data
                            );
                            forgetPreparedState(
                                sector_split_info.pre_split_area_data
                            );
                            clearSelection();
                            clearLayoutDrawing();
                            sub_state = EDITOR_SUB_STATE_NONE;
                            setStatus(
                                "That's not a valid split!",
                                true
                            );
                            break;
                            
                        } case SECTOR_SPLIT_RESULT_USELESS: {
                            rollbackToPreparedState(
                                sector_split_info.pre_split_area_data
                            );
                            forgetPreparedState(
                                sector_split_info.pre_split_area_data
                            );
                            recreateDrawingNodes();
                            sector_split_info.useless_split_part_2_checkpoint =
                                drawing_nodes.size();
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
            Point hotspot = snapPoint(game.mouse_cursor.w_pos);
            
            if(new_circle_sector_step == 0) {
                new_circle_sector_center = hotspot;
                new_circle_sector_anchor = new_circle_sector_center;
                new_circle_sector_step++;
                
            } else if(new_circle_sector_step == 1) {
                new_circle_sector_anchor = hotspot;
                setNewCircleSectorPoints();
                new_circle_sector_step++;
                
            } else {
                setNewCircleSectorPoints();
                
                bool all_valid = true;
                for(
                    size_t e = 0; e < new_circle_sector_valid_edges.size(); e++
                ) {
                    if(!new_circle_sector_valid_edges[e]) {
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
            octee_drag_start = game.mouse_cursor.w_pos;
            Sector* s_ptr = *selected_sectors.begin();
            octee_orig_angle = s_ptr->texture_info.rot;
            octee_orig_offset = s_ptr->texture_info.translation;
            octee_orig_scale = s_ptr->texture_info.scale;
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            bool tw_handled = false;
            if(
                game.options.area_editor.sel_trans &&
                selected_vertexes.size() >= 2
            ) {
                tw_handled =
                    cur_transformation_widget.handleMouseDown(
                        game.mouse_cursor.w_pos,
                        &selection_center,
                        &selection_size,
                        &selection_angle,
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
                
                if(!is_shift_pressed) {
                    if(clicked_vertex || clicked_edge || clicked_sector) {
                        start_new_selection = false;
                    }
                    
                }
                
                if(start_new_selection) {
                    if(!is_ctrl_pressed) clearSelection();
                    selecting = true;
                    selection_start = game.mouse_cursor.w_pos;
                    selection_end = game.mouse_cursor.w_pos;
                    
                } else {
                
                    if(clicked_vertex) {
                        if(
                            selected_vertexes.find(clicked_vertex) ==
                            selected_vertexes.end()
                        ) {
                            if(!is_ctrl_pressed) {
                                clearSelection();
                            }
                            selectVertex(clicked_vertex);
                        }
                    } else if(clicked_edge) {
                        if(
                            selected_edges.find(clicked_edge) ==
                            selected_edges.end()
                        ) {
                            if(!is_ctrl_pressed) {
                                clearSelection();
                            }
                            selectEdge(clicked_edge);
                        }
                    } else {
                        if(
                            selected_sectors.find(clicked_sector) ==
                            selected_sectors.end()
                        ) {
                            if(!is_ctrl_pressed) {
                                clearSelection();
                            }
                            selectSector(clicked_sector);
                        }
                    }
                    
                }
                
                selection_homogenized = false;
                setSelectionStatusText();
                
            }
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_MOBS: {

        switch(sub_state) {
        case EDITOR_SUB_STATE_NEW_MOB: {
    
            //Create a mob where the cursor is.
            createMobUnderCursor();
            
            break;
            
        } case EDITOR_SUB_STATE_DUPLICATE_MOB: {
    
            //Duplicate the current mobs to where the cursor is.
            registerChange("object duplication");
            sub_state = EDITOR_SUB_STATE_NONE;
            Point hotspot = snapPoint(game.mouse_cursor.w_pos);
            
            Point selection_tl = (*selected_mobs.begin())->pos;
            Point selection_br = selection_tl;
            for(auto m = selected_mobs.begin(); m != selected_mobs.end(); ++m) {
                if(m == selected_mobs.begin()) continue;
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
            
            for(auto const &m : selected_mobs) {
                MobGen* new_mg = new MobGen(*m);
                new_mg->pos = Point(hotspot + (m->pos) - new_selection_center);
                game.cur_area_data->mob_generators.push_back(new_mg);
                mobs_to_select.insert(new_mg);
            }
            
            clearSelection();
            selected_mobs = mobs_to_select;
            
            setStatus(
                "Duplicated " +
                amountStr((int) selected_mobs.size(), "object") + "."
            );
            
            break;
            
        } case EDITOR_SUB_STATE_STORE_MOB_INSIDE: {
    
            //Store the mob inside another.
            size_t target_idx;
            MobGen* target =
                getMobUnderPoint(game.mouse_cursor.w_pos, &target_idx);
            if(!target) return;
            
            for(auto const &m : selected_mobs) {
                if(m == target) {
                    setStatus(
                        "You can't store to an object inside itself!",
                        true
                    );
                    return;
                }
            }
            MobGen* m_ptr = *(selected_mobs.begin());
            if(m_ptr->stored_inside == target_idx) {
                setStatus(
                    "The object is already stored inside that object!",
                    true
                );
                return;
            }
            
            registerChange("Object in object storing");
            
            m_ptr->stored_inside = target_idx;
            
            homogenizeSelectedMobs();
            
            sub_state = EDITOR_SUB_STATE_NONE;
            setStatus("Stored the object inside another.");
            
            break;
            
        } case EDITOR_SUB_STATE_ADD_MOB_LINK: {
    
            //Link two mobs.
            MobGen* target = getMobUnderPoint(game.mouse_cursor.w_pos);
            if(!target) return;
            
            for(auto const &m : selected_mobs) {
                if(m == target) {
                    setStatus(
                        "You can't link to an object to itself!",
                        true
                    );
                    return;
                }
            }
            MobGen* m_ptr = *(selected_mobs.begin());
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
            m_ptr->link_idxs.push_back(
                game.cur_area_data->findMobGenIdx(target)
            );
            
            homogenizeSelectedMobs();
            
            sub_state = EDITOR_SUB_STATE_NONE;
            setStatus("Linked the two objects.");
            
            break;
            
        } case EDITOR_SUB_STATE_DEL_MOB_LINK: {
    
            //Delete a mob link.
            MobGen* target = getMobUnderPoint(game.mouse_cursor.w_pos);
            MobGen* m_ptr = *(selected_mobs.begin());
            
            if(!target) {
                std::pair<MobGen*, MobGen*> data1;
                std::pair<MobGen*, MobGen*> data2;
                if(
                    !getMobLinkUnderPoint(
                        game.mouse_cursor.w_pos, &data1, &data2
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
                m_ptr->link_idxs.erase(m_ptr->link_idxs.begin() + link_i);
            }
            
            homogenizeSelectedMobs();
            
            sub_state = EDITOR_SUB_STATE_NONE;
            setStatus("Deleted object link.");
            
            break;
            
        } case EDITOR_SUB_STATE_MISSION_MOBS: {
    
            size_t clicked_mob_idx;
            MobGen* clicked_mob =
                getMobUnderPoint(game.mouse_cursor.w_pos, &clicked_mob_idx);
                
            if(
                clicked_mob_idx != INVALID &&
                game.mission_goals[game.cur_area_data->mission.goal]->
                isMobApplicable(clicked_mob->type)
            ) {
                registerChange("mission object requirements change");
                auto it =
                    game.cur_area_data->mission.goal_mob_idxs.find(
                        clicked_mob_idx
                    );
                if(it == game.cur_area_data->mission.goal_mob_idxs.end()) {
                    game.cur_area_data->mission.goal_mob_idxs.insert(
                        clicked_mob_idx
                    );
                } else {
                    game.cur_area_data->mission.goal_mob_idxs.erase(it);
                }
            }
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            //Start a new mob selection or select something.
            bool start_new_selection = true;
            MobGen* clicked_mob = getMobUnderPoint(game.mouse_cursor.w_pos);
            
            if(!is_shift_pressed) {
                if(clicked_mob) {
                    start_new_selection = false;
                }
            }
            
            if(start_new_selection) {
                if(!is_ctrl_pressed) clearSelection();
                selecting = true;
                selection_start = game.mouse_cursor.w_pos;
                selection_end = game.mouse_cursor.w_pos;
                
            } else {
                if(selected_mobs.find(clicked_mob) == selected_mobs.end()) {
                    if(!is_ctrl_pressed) {
                        clearSelection();
                    }
                    selected_mobs.insert(clicked_mob);
                }
                
            }
            
            selection_homogenized = false;
            setSelectionStatusText();
            
            break;
            
        }
        }
        
        break;
        
    } case EDITOR_STATE_PATHS: {

        switch(sub_state) {
        case EDITOR_SUB_STATE_PATH_DRAWING: {
    
            //Drawing a path.
            Point hotspot =
                snapPoint(game.mouse_cursor.w_pos);
            PathStop* clicked_stop =
                getPathStopUnderPoint(game.mouse_cursor.w_pos);
                
            //Split a link, if one was clicked.
            if(!clicked_stop) {
                PathLink* clicked_link_1;
                PathLink* clicked_link_2;
                bool clicked_link =
                    getPathLinkUnderPoint(
                        game.mouse_cursor.w_pos,
                        &clicked_link_1, &clicked_link_2
                    );
                if(clicked_link) {
                    registerChange("path link split");
                    clicked_stop =
                        splitPathLink(
                            clicked_link_1, clicked_link_2,
                            game.mouse_cursor.w_pos
                        );
                    clearSelection();
                    selected_path_stops.insert(clicked_stop);
                }
            }
            
            if(path_drawing_stop_1) {
                //A starting stop already exists, so now we create a link.
                PathStop* next_stop = nullptr;
                if(clicked_stop) {
                    if(clicked_stop == path_drawing_stop_1) {
                        path_drawing_stop_1 = nullptr;
                    } else {
                        next_stop = clicked_stop;
                    }
                } else {
                    registerChange("path stop creation");
                    next_stop = new PathStop(hotspot);
                    next_stop->flags = path_drawing_flags;
                    next_stop->label = path_drawing_label;
                    game.cur_area_data->path_stops.push_back(next_stop);
                    setStatus("Created path stop.");
                }
                
                if(next_stop) {
                    registerChange("path stop link");
                    path_drawing_stop_1->addLink(
                        next_stop, path_drawing_normals
                    );
                    PathLink* l1 = path_drawing_stop_1->get_link(next_stop);
                    PathLink* l2 = next_stop->get_link(path_drawing_stop_1);
                    l1->type = path_drawing_type;
                    if(l2) {
                        l2->type = path_drawing_type;
                    }
                    game.cur_area_data->fixPathStopIdxs(path_drawing_stop_1);
                    game.cur_area_data->fixPathStopIdxs(next_stop);
                    next_stop->calculateDistsPlusNeighbors();
                    setStatus("Created path link.");
                    
                    if(clicked_stop) {
                        path_drawing_stop_1 = nullptr;
                    } else {
                        path_drawing_stop_1 = next_stop;
                    }
                }
                
            } else {
                //We need to create or assign a starting stop.
                if(clicked_stop) {
                    path_drawing_stop_1 = clicked_stop;
                } else {
                    registerChange("path stop creation");
                    path_drawing_stop_1 = new PathStop(hotspot);
                    path_drawing_stop_1->flags = path_drawing_flags;
                    path_drawing_stop_1->label = path_drawing_label;
                    game.cur_area_data->path_stops.push_back(
                        path_drawing_stop_1
                    );
                    setStatus("Created path stop.");
                }
                
            }
            
            path_preview.clear(); //Clear so it doesn't reference deleted stops.
            path_preview_timer.start(false);
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            //First, check if the user clicked on a path preview checkpoint.
            if(show_path_preview) {
                for(unsigned char c = 0; c < 2; c++) {
                    if(
                        BBoxCheck(
                            path_preview_checkpoints[c],
                            game.mouse_cursor.w_pos,
                            AREA_EDITOR::PATH_PREVIEW_CHECKPOINT_RADIUS /
                            game.cam.zoom
                        )
                    ) {
                        clearSelection();
                        moving_path_preview_checkpoint = c;
                        return;
                    }
                }
            }
            
            //Start a new path selection or select something.
            bool start_new_selection = true;
            
            PathStop* clicked_stop =
                getPathStopUnderPoint(game.mouse_cursor.w_pos);
            PathLink* clicked_link_1;
            PathLink* clicked_link_2;
            bool clicked_link =
                getPathLinkUnderPoint(
                    game.mouse_cursor.w_pos,
                    &clicked_link_1, &clicked_link_2
                );
            if(!is_shift_pressed) {
                if(clicked_stop || clicked_link) {
                    start_new_selection = false;
                }
                
            }
            
            if(start_new_selection) {
                if(!is_ctrl_pressed) clearSelection();
                selecting = true;
                selection_start = game.mouse_cursor.w_pos;
                selection_end = game.mouse_cursor.w_pos;
                
            } else {
            
                if(clicked_stop) {
                    if(
                        selected_path_stops.find(clicked_stop) ==
                        selected_path_stops.end()
                    ) {
                        if(!is_ctrl_pressed) {
                            clearSelection();
                        }
                        selected_path_stops.insert(clicked_stop);
                    }
                } else {
                    if(
                        selected_path_links.find(clicked_link_1) ==
                        selected_path_links.end()
                    ) {
                        if(!is_ctrl_pressed) {
                            clearSelection();
                        }
                        selected_path_links.insert(clicked_link_1);
                        if(clicked_link_2 != nullptr) {
                            selected_path_links.insert(clicked_link_2);
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

        switch(sub_state) {
        case EDITOR_SUB_STATE_NEW_SHADOW: {
    
            //Create a new shadow where the cursor is.
            registerChange("tree shadow creation");
            sub_state = EDITOR_SUB_STATE_NONE;
            Point hotspot = snapPoint(game.mouse_cursor.w_pos);
            
            TreeShadow* new_shadow = new TreeShadow(hotspot);
            new_shadow->bitmap = game.bmp_error;
            
            game.cur_area_data->tree_shadows.push_back(new_shadow);
            
            selectTreeShadow(new_shadow);
            
            break;
            
        } case EDITOR_SUB_STATE_NONE: {
    
            bool transformation_handled = false;
            if(selected_shadow) {
                transformation_handled =
                    cur_transformation_widget.handleMouseDown(
                        game.mouse_cursor.w_pos,
                        &selected_shadow->center,
                        &selected_shadow->size,
                        &selected_shadow->angle,
                        1.0f / game.cam.zoom
                    );
            }
            
            if(!transformation_handled) {
                //Select a tree shadow.
                selected_shadow = nullptr;
                for(
                    size_t s = 0;
                    s < game.cur_area_data->tree_shadows.size(); s++
                ) {
                
                    TreeShadow* s_ptr = game.cur_area_data->tree_shadows[s];
                    Point min_coords, max_coords;
                    getTransformedRectangleBBox(
                        s_ptr->center, s_ptr->size, s_ptr->angle,
                        &min_coords, &max_coords
                    );
                    
                    if(
                        game.mouse_cursor.w_pos.x >= min_coords.x &&
                        game.mouse_cursor.w_pos.x <= max_coords.x &&
                        game.mouse_cursor.w_pos.y >= min_coords.y &&
                        game.mouse_cursor.w_pos.y <= max_coords.y
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

        if(reference_bitmap) {
            cur_transformation_widget.handleMouseDown(
                game.mouse_cursor.w_pos,
                &reference_center,
                &reference_size,
                nullptr,
                1.0f / game.cam.zoom
            );
        }
        
        break;
        
    } case EDITOR_STATE_REVIEW: {

        if(show_cross_section) {
            moving_cross_section_point = -1;
            for(unsigned char p = 0; p < 2; p++) {
                if(
                    BBoxCheck(
                        cross_section_checkpoints[p], game.mouse_cursor.w_pos,
                        AREA_EDITOR::CROSS_SECTION_POINT_RADIUS / game.cam.zoom
                    )
                ) {
                    moving_cross_section_point = p;
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
    
        Point selection_tl = selection_start;
        Point selection_br = selection_start;
        updateMinMaxCoords(selection_tl, selection_br, selection_end);
        selection_end = game.mouse_cursor.w_pos;
        
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
    
            //Selection box around the layout.
            if(!is_ctrl_pressed) clearSelection();
            
            for(size_t v = 0; v < game.cur_area_data->vertexes.size(); v++) {
                Vertex* v_ptr = game.cur_area_data->vertexes[v];
                
                if(
                    v_ptr->x >= selection_tl.x &&
                    v_ptr->x <= selection_br.x &&
                    v_ptr->y >= selection_tl.y &&
                    v_ptr->y <= selection_br.y
                ) {
                    selected_vertexes.insert(v_ptr);
                }
            }
            updateVertexSelection();
            
            if(selection_filter != SELECTION_FILTER_VERTEXES) {
                for(size_t e = 0; e < game.cur_area_data->edges.size(); e++) {
                    Edge* e_ptr = game.cur_area_data->edges[e];
                    
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
                        selected_edges.insert(e_ptr);
                    }
                }
            }
            
            if(selection_filter == SELECTION_FILTER_SECTORS) {
                for(size_t s = 0; s < game.cur_area_data->sectors.size(); s++) {
                    Sector* s_ptr = game.cur_area_data->sectors[s];
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
                        selected_sectors.insert(s_ptr);
                    }
                }
            }
            
            selection_homogenized = false;
            setSelectionStatusText();
            
            break;
            
        } case EDITOR_STATE_MOBS: {
    
            //Selection box around mobs.
            if(!is_ctrl_pressed) clearSelection();
            
            for(
                size_t m = 0;
                m < game.cur_area_data->mob_generators.size(); m++
            ) {
                MobGen* m_ptr = game.cur_area_data->mob_generators[m];
                float radius = getMobGenRadius(m_ptr);
                
                if(
                    m_ptr->pos.x - radius >= selection_tl.x &&
                    m_ptr->pos.x + radius <= selection_br.x &&
                    m_ptr->pos.y - radius >= selection_tl.y &&
                    m_ptr->pos.y + radius <= selection_br.y
                ) {
                    selected_mobs.insert(m_ptr);
                }
            }
            
            selection_homogenized = false;
            setSelectionStatusText();
            
            break;
            
        } case EDITOR_STATE_PATHS: {
    
            //Selection box around path stops.
            if(!is_ctrl_pressed) clearSelection();
            
            for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
                PathStop* s_ptr = game.cur_area_data->path_stops[s];
                
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
                    selected_path_stops.insert(s_ptr);
                }
            }
            
            for(size_t s = 0; s < game.cur_area_data->path_stops.size(); s++) {
                PathStop* s_ptr = game.cur_area_data->path_stops[s];
                for(size_t l = 0; l < s_ptr->links.size(); l++) {
                    PathStop* s2_ptr = s_ptr->links[l]->end_ptr;
                    
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
                        selected_path_links.insert(s_ptr->links[l]);
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
    
            if(sub_state == EDITOR_SUB_STATE_MISSION_EXIT) {
                Point exit_center =
                    game.cur_area_data->mission.goal_exit_center;
                Point exit_size =
                    game.cur_area_data->mission.goal_exit_size;
                if(
                    cur_transformation_widget.handleMouseMove(
                        snapPoint(game.mouse_cursor.w_pos, true),
                        &exit_center, &exit_size,
                        nullptr,
                        1.0f / game.cam.zoom,
                        false,
                        false,
                        MISSION::EXIT_MIN_SIZE,
                        is_alt_pressed
                    )
                ) {
                    registerChange("mission exit change");
                    game.cur_area_data->mission.goal_exit_center = exit_center;
                    game.cur_area_data->mission.goal_exit_size = exit_size;
                }
            }
            break;
            
        }
        case EDITOR_STATE_LAYOUT: {
    
            bool tw_handled = false;
            if(
                game.options.area_editor.sel_trans &&
                selected_vertexes.size() >= 2
            ) {
                tw_handled =
                    cur_transformation_widget.handleMouseMove(
                        snapPoint(game.mouse_cursor.w_pos, true),
                        &selection_center,
                        &selection_size,
                        &selection_angle,
                        1.0f / game.cam.zoom,
                        false,
                        false,
                        AREA_EDITOR::SELECTION_TW_PADDING * 2.0f,
                        is_alt_pressed
                    );
                if(tw_handled) {
                    if(!moving) {
                        startVertexMove();
                    }
                    
                    ALLEGRO_TRANSFORM t;
                    al_identity_transform(&t);
                    al_scale_transform(
                        &t,
                        selection_size.x / selection_orig_size.x,
                        selection_size.y / selection_orig_size.y
                    );
                    al_translate_transform(
                        &t,
                        selection_center.x - selection_orig_center.x,
                        selection_center.y - selection_orig_center.y
                    );
                    al_rotate_transform(
                        &t,
                        selection_angle - selection_orig_angle
                    );
                    
                    for(Vertex* v : selected_vertexes) {
                        Point p = pre_move_vertex_coords[v];
                        p -= selection_orig_center;
                        al_transform_coordinates(&t, &p.x, &p.y);
                        p += selection_orig_center;
                        v->x = p.x;
                        v->y = p.y;
                    }
                }
            }
            
            if(
                !tw_handled &&
                !selected_vertexes.empty() &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move vertexes.
                if(!moving) {
                    startVertexMove();
                }
                
                Point mouse_offset =
                    game.mouse_cursor.w_pos - move_mouse_start_pos;
                Point closest_vertex_new_p =
                    snapPoint(
                        move_start_pos + mouse_offset, true
                    );
                Point offset =
                    closest_vertex_new_p - move_start_pos;
                for(Vertex* v : selected_vertexes) {
                    Point orig = pre_move_vertex_coords[v];
                    v->x = orig.x + offset.x;
                    v->y = orig.y + offset.y;
                }
                
            } else if(
                sub_state == EDITOR_SUB_STATE_OCTEE && moving
            ) {
                //Move sector texture transformation property.
                Sector* s_ptr = *selected_sectors.begin();
                
                switch(octee_mode) {
                case OCTEE_MODE_OFFSET: {
                    registerChange("sector texture offset change");
                    Point diff = (game.mouse_cursor.w_pos - octee_drag_start);
                    diff = rotatePoint(diff, -s_ptr->texture_info.rot);
                    diff = diff / s_ptr->texture_info.scale;
                    s_ptr->texture_info.translation = octee_orig_offset + diff;
                    break;
                } case OCTEE_MODE_SCALE: {
                    registerChange("sector texture scale change");
                    Point diff = (game.mouse_cursor.w_pos - octee_drag_start);
                    diff = rotatePoint(diff, -s_ptr->texture_info.rot);
                    Point drag_start_rot =
                        rotatePoint(
                            octee_drag_start, -s_ptr->texture_info.rot
                        );
                    diff = diff / drag_start_rot * octee_orig_scale;
                    s_ptr->texture_info.scale = octee_orig_scale + diff;
                    break;
                } case OCTEE_MODE_ANGLE: {
                    registerChange("sector texture angle change");
                    float drag_start_a = getAngle(octee_drag_start);
                    float cursor_a = getAngle(game.mouse_cursor.w_pos);
                    s_ptr->texture_info.rot =
                        octee_orig_angle + (cursor_a - drag_start_a);
                    break;
                }
                };
                
                homogenizeSelectedSectors();
            }
            
            break;
            
        } case EDITOR_STATE_MOBS: {
    
            if(
                !selected_mobs.empty() &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move mobs.
                if(!moving) {
                    startMobMove();
                }
                
                Point mouse_offset =
                    game.mouse_cursor.w_pos - move_mouse_start_pos;
                Point closest_mob_new_p =
                    snapPoint(move_start_pos + mouse_offset);
                Point offset = closest_mob_new_p - move_start_pos;
                for(
                    auto m = selected_mobs.begin();
                    m != selected_mobs.end(); ++m
                ) {
                    Point orig = pre_move_mob_coords[*m];
                    (*m)->pos = orig + offset;
                }
            }
            
            break;
            
        } case EDITOR_STATE_PATHS: {
    
            if(
                !selected_path_stops.empty() &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move path stops.
                if(!moving) {
                    startPathStopMove();
                }
                
                Point mouse_offset =
                    game.mouse_cursor.w_pos - move_mouse_start_pos;
                Point closest_stop_new_p =
                    snapPoint(move_start_pos + mouse_offset);
                Point offset = closest_stop_new_p - move_start_pos;
                for(
                    auto s = selected_path_stops.begin();
                    s != selected_path_stops.end(); ++s
                ) {
                    Point orig = pre_move_stop_coords[*s];
                    (*s)->pos.x = orig.x + offset.x;
                    (*s)->pos.y = orig.y + offset.y;
                }
                
                for(
                    auto s = selected_path_stops.begin();
                    s != selected_path_stops.end(); ++s
                ) {
                    (*s)->calculateDistsPlusNeighbors();
                }
                
                path_preview_timer.start(false);
                
            } else if(
                moving_path_preview_checkpoint != -1 &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move path preview checkpoints.
                path_preview_checkpoints[moving_path_preview_checkpoint] =
                    snapPoint(game.mouse_cursor.w_pos);
                path_preview_timer.start(false);
            }
            
            break;
            
        } case EDITOR_STATE_DETAILS: {
    
            if(
                selected_shadow &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Move tree shadow.
                Point shadow_center = selected_shadow->center;
                Point shadow_size = selected_shadow->size;
                float shadow_angle = selected_shadow->angle;
                if(
                    cur_transformation_widget.handleMouseMove(
                        snapPoint(game.mouse_cursor.w_pos),
                        &shadow_center,
                        &shadow_size,
                        &shadow_angle,
                        1.0f / game.cam.zoom,
                        selected_shadow_keep_aspect_ratio,
                        false,
                        -FLT_MAX,
                        is_alt_pressed
                    )
                ) {
                    registerChange("tree shadow transformation");
                    selected_shadow->center = shadow_center;
                    selected_shadow->size = shadow_size;
                    selected_shadow->angle = shadow_angle;
                }
            }
            
            break;
            
        } case EDITOR_STATE_TOOLS: {
    
            //Move reference handle.
            cur_transformation_widget.handleMouseMove(
                snapPoint(game.mouse_cursor.w_pos),
                &reference_center,
                &reference_size,
                nullptr,
                1.0f / game.cam.zoom,
                reference_keep_aspect_ratio,
                false,
                5.0f,
                is_alt_pressed
            );
            
            break;
            
        } case EDITOR_STATE_REVIEW: {
    
            //Move cross-section points.
            if(moving_cross_section_point != -1) {
                cross_section_checkpoints[moving_cross_section_point] =
                    snapPoint(game.mouse_cursor.w_pos);
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
            state == EDITOR_STATE_LAYOUT && sub_state != EDITOR_SUB_STATE_OCTEE
        ) {
            finishLayoutMoving();
        }
        moving = false;
    }
    
    cur_transformation_widget.handleMouseUp();
    
    moving_path_preview_checkpoint = -1;
    moving_cross_section_point = -1;
}


/**
 * @brief Handles the middle mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleMmbDoubleClick(const ALLEGRO_EVENT &ev) {
    if(!game.options.editors.mmb_pan) {
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
    if(!game.options.editors.mmb_pan) {
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
    if(game.options.editors.mmb_pan) {
        panCam(ev);
    }
}


/**
 * @brief Handles the mouse coordinates being updated.
 *
 * @param ev Event to handle.
 */
void AreaEditor::handleMouseUpdate(const ALLEGRO_EVENT &ev) {
    game.mouse_cursor.s_pos.x = ev.mouse.x;
    game.mouse_cursor.s_pos.y = ev.mouse.y;
    game.mouse_cursor.w_pos = game.mouse_cursor.s_pos;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor.w_pos.x, &game.mouse_cursor.w_pos.y
    );
    
    //Update highlighted elements.
    highlighted_vertex = nullptr;
    highlighted_edge = nullptr;
    highlighted_sector = nullptr;
    highlighted_mob = nullptr;
    highlighted_path_stop = nullptr;
    highlighted_path_link = nullptr;
    if(!is_mouse_in_gui) {
        switch(state) {
        case EDITOR_STATE_LAYOUT: {
            getHoveredLayoutElement(
                &highlighted_vertex, &highlighted_edge, &highlighted_sector
            );
            break;
        } case EDITOR_STATE_MOBS: {
            highlighted_mob =
                getMobUnderPoint(game.mouse_cursor.w_pos);
            break;
        } case EDITOR_STATE_PATHS: {
            PathLink* hovered_link_1;
            
            highlighted_path_stop =
                getPathStopUnderPoint(game.mouse_cursor.w_pos);
                
            if(highlighted_path_stop == nullptr) {
                //Selecting the stop takes priority,
                //so keep the link null if there's a stop.
                getPathLinkUnderPoint(
                    game.mouse_cursor.w_pos,
                    &hovered_link_1, &highlighted_path_link
                );
                if(highlighted_path_link == nullptr) {
                    highlighted_path_link = hovered_link_1;
                }
            }
            break;
        }
        }
    }
    
    if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        Point hotspot = snapPoint(game.mouse_cursor.w_pos, true);
        if(new_circle_sector_step == 1) {
            new_circle_sector_anchor = hotspot;
        } else {
            setNewCircleSectorPoints();
        }
    }
    
    if(sub_state == EDITOR_SUB_STATE_QUICK_HEIGHT_SET) {
        float offset = getQuickHeightSetOffset();
        registerChange("quick sector height set");
        for(auto &s : selected_sectors) {
            s->z = quick_height_set_start_heights[s] + offset;
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
    if(game.options.editors.mmb_pan) {
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
    if(game.options.editors.mmb_pan) {
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
    if(!game.options.editors.mmb_pan) {
        panCam(ev);
    }
}
