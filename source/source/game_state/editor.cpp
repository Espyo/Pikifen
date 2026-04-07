/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Editor-related functions.
 */

#include <algorithm>

#include "editor.h"

#include "../content/mob_category/mob_category.h"
#include "../content/mob_type/mob_type.h"
#include "../core/drawing.h"
#include "../core/game.h"
#include "../core/load.h"
#include "../core/misc_functions.h"
#include "../lib/imgui/imgui_impl_allegro5.h"
#include "../lib/imgui/imgui_internal.h"
#include "../lib/imgui/imgui_stdlib.h"
#include "../util/allegro_utils.h"
#include "../util/imgui_utils.h"
#include "../util/string_utils.h"

using std::make_pair;


namespace EDITOR {

//Default history maximum size.
const size_t DEF_MAX_HISTORY_SIZE = 6;

//Time until the next click is no longer considered a double-click.
const float DOUBLE_CLICK_TIMEOUT = 0.5f;

//Color of major lines in the grid.
const ALLEGRO_COLOR GRID_COLOR_MAJOR = al_map_rgb(64, 64, 64);

//Color of minor lines in the grid.
const ALLEGRO_COLOR GRID_COLOR_MINOR = al_map_rgb(48, 48, 48);

//Color of origin lines in the grid.
const ALLEGRO_COLOR GRID_COLOR_ORIGIN = al_map_rgb(240, 240, 240);

//Every icon in the icon bitmap file is these many pixels from the previous.
const int ICON_BMP_PADDING = 1;

//Every icon in the icon bitmap file has this size.
const int ICON_BMP_SIZE = 24;

//How much to zoom in/out with the keyboard keys.
const float KEYBOARD_CAM_ZOOM = 0.25f;

//Width of the text widget that shows the mouse cursor coordinates.
const float MOUSE_COORDS_TEXT_WIDTH = 176.0f;

//How quickly the operation error red flash effect cursor shakes.
const float OP_ERROR_CURSOR_SHAKE_SPEED = 55.0f;

//How much the operation error red flash effect cursor shakes left and right.
const float OP_ERROR_CURSOR_SHAKE_WIDTH = 6.0f;

//Width or height of the operation error red flash effect cursor.
const float OP_ERROR_CURSOR_SIZE = 32.0f;

//Thickness of the operation error red flash effect cursor.
const float OP_ERROR_CURSOR_THICKNESS = 5.0f;

//Duration of the operation error red flash effect.
const float OP_ERROR_FLASH_DURATION = 1.5f;

//Picker dialog minimum button size.
const float PICKER_IMG_BUTTON_MIN_SIZE = 32.0f;

//Picker dialog button size.
const float PICKER_IMG_BUTTON_SIZE = 168.0f;

//Multiply time by this much for the rubber band texture animation.
const float RUBBER_BAND_TEXTURE_TIME_MULT = 10.0f;

//Color to use for silhouette icons.
const ALLEGRO_COLOR SILHOUETTE_COLOR = al_map_rgba(240, 240, 240, 160);

//Height of the status bar.
const float STATUS_BAR_HEIGHT = 22.0f;

//Default size of the transformation widget.
const float TW_DEF_SIZE = 32.0f;

//Radius of a handle in the transformation widget.
const float TW_HANDLE_RADIUS = 6.0f;

//Thickness of the outline in the transformation widget.
const float TW_OUTLINE_THICKNESS = 2.0f;

//Thickness of the rotation handle in the transformation widget.
const float TW_ROTATION_HANDLE_THICKNESS = 8.0f;

}


/**
 * @brief Constructs a new editor object.
 *
 */
Editor::Editor() :
    changesMgr(this) {
    
    editorIcons.reserve(N_EDITOR_ICONS);
    for(size_t i = 0; i < N_EDITOR_ICONS; i++) {
        editorIcons.push_back(nullptr);
    }
}


/**
 * @brief Draws a Dear ImGui-like visualizer for an angle.
 *
 * @param angle Angle to show.
 */
void Editor::angleVisualizer(float angle) {
    //Setup.
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 drawPos = ImGui::GetCursorScreenPos();
    
    //Position calculations.
    const float drawSize = drawList->_Data->FontSize;
    ImVec2 drawCenter(drawPos.x + drawSize / 2.0f, drawPos.y + drawSize / 2.0f);
    
    Point tipVertex(drawSize / 2.0f, 0.0f);
    Point rightVertex(0.0f, drawSize * 0.30f);
    Point leftVertex(0.0f, -drawSize * 0.30f);
    Point lineVertex(-drawSize / 2.0f, 0.0f);
    
    tipVertex = rotatePoint(tipVertex, angle);
    rightVertex = rotatePoint(rightVertex, angle);
    leftVertex = rotatePoint(leftVertex, angle);
    lineVertex = rotatePoint(lineVertex, angle);
    
    tipVertex.x += drawCenter.x;
    tipVertex.y += drawCenter.y;
    rightVertex.x += drawCenter.x;
    rightVertex.y += drawCenter.y;
    leftVertex.x += drawCenter.x;
    leftVertex.y += drawCenter.y;
    lineVertex.x += drawCenter.x;
    lineVertex.y += drawCenter.y;
    
    //Draw the triangle.
    drawList->AddTriangleFilled(
        ImVec2(tipVertex.x, tipVertex.y),
        ImVec2(rightVertex.x, rightVertex.y),
        ImVec2(leftVertex.x, leftVertex.y),
        ImGui::GetColorU32(ImGuiCol_Text)
    );
    
    //Draw the line.
    drawList->AddLine(
        drawCenter,
        ImVec2(lineVertex.x, lineVertex.y),
        ImGui::GetColorU32(ImGuiCol_Text)
    );
    
    //Add a dummy to symbolize the space the visualizer took up.
    ImGui::Dummy(ImVec2(drawSize, drawSize));
}


/**
 * @brief Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the window,
 * where it's hard to see.
 *
 * @param minCoords Top-left coordinates of the content to focus on.
 * @param maxCoords Bottom-right coordinates of the content to focus on.
 * @param instantaneous If true, the camera moves there instantaneously.
 * If false, it smoothly gets there over time.
 */
void Editor::centerCamera(
    const Point& minCoords, const Point& maxCoords,
    bool instantaneous
) {
    Point minC = minCoords;
    Point maxC = maxCoords;
    if(minC == maxC) {
        minC = minC - 2.0;
        maxC = maxC + 2.0;
    }
    
    float width = maxC.x - minC.x;
    float height = maxC.y - minC.y;
    
    game.editorsView.cam.targetPos.x = floor(minC.x + width  / 2);
    game.editorsView.cam.targetPos.y = floor(minC.y + height / 2);
    
    float z;
    if(width > height) z = game.editorsView.size.x / width;
    else z = game.editorsView.size.y / height;
    z -= z * 0.1;
    
    game.editorsView.cam.targetZoom = z;
    
    if(instantaneous) {
        game.editorsView.cam.pos = game.editorsView.cam.targetPos;
        game.editorsView.cam.zoom = game.editorsView.cam.targetZoom;
    }
    
    game.editorsView.updateTransformations();
}


/**
 * @brief Closes the topmost dialog that is still open.
 */
void Editor::closeTopDialog() {
    forIdx(d, dialogs) {
        Dialog* dPtr = dialogs[dialogs.size() - (d + 1)];
        if(dPtr->isOpen) {
            dPtr->isOpen = false;
            return;
        }
    }
}


/**
 * @brief Handles the logic part of the main loop of the editor.
 * This is meant to be run after the editor's own logic code.
 */
void Editor::doLogicPost() {
    escapeWasPressed = false;
    game.fadeMgr.tick(game.deltaT);
}


/**
 * @brief Handles the logic part of the main loop of the editor.
 * This is meant to be run before the editor's own logic code.
 */
void Editor::doLogicPre() {
    if(doubleClickTime > 0) {
        doubleClickTime -= game.deltaT;
        if(doubleClickTime < 0) doubleClickTime = 0;
    }
    
    game.editorsView.cam.tick(game.deltaT);
    game.editorsView.updateBox();
    
    opErrorFlashTimer.tick(game.deltaT);
    
    game.editorsView.updateTransformations();
}


/**
 * @brief Draws the grid, using the current game camera.
 *
 * @param interval Interval between grid lines.
 * @param majorColor Color to use for major lines.
 * These are lines that happen at major milestones (i.e. twice the interval).
 * @param minorColor Color to use for minor lines.
 * These are lines that aren't major.
 */
void Editor::drawGrid(
    float interval,
    const ALLEGRO_COLOR& majorColor, const ALLEGRO_COLOR& minorColor
) {
    Point canvasTL = game.editorsView.getTopLeft();
    Point canvasBR = game.editorsView.getBottomRight();
    
    Point camTLCorner = canvasTL;
    Point camBRCorner = canvasBR;
    al_transform_coordinates(
        &game.editorsView.windowToWorldTransform,
        &camTLCorner.x, &camTLCorner.y
    );
    al_transform_coordinates(
        &game.editorsView.windowToWorldTransform,
        &camBRCorner.x, &camBRCorner.y
    );
    
    float x = floor(camTLCorner.x / interval) * interval;
    while(x < camBRCorner.x + interval) {
        ALLEGRO_COLOR c = minorColor;
        bool drawLine = true;
        
        if(fmod(x, interval * 2) == 0) {
            c = majorColor;
            if((interval * 2) * game.editorsView.cam.zoom <= 6) {
                drawLine = false;
            }
        } else {
            if(interval * game.editorsView.cam.zoom <= 6) {
                drawLine = false;
            }
        }
        
        if(drawLine) {
            al_draw_line(
                x, camTLCorner.y,
                x, camBRCorner.y + interval,
                c, 1.0f / game.editorsView.cam.zoom
            );
        }
        x += interval;
    }
    
    float y = floor(camTLCorner.y / interval) * interval;
    while(y < camBRCorner.y + interval) {
        ALLEGRO_COLOR c = minorColor;
        bool drawLine = true;
        
        if(fmod(y, interval * 2) == 0) {
            c = majorColor;
            if((interval * 2) * game.editorsView.cam.zoom <= 6) {
                drawLine = false;
            }
        } else {
            if(interval * game.editorsView.cam.zoom <= 6) {
                drawLine = false;
            }
        }
        
        if(drawLine) {
            al_draw_line(
                camTLCorner.x, y,
                camBRCorner.x + interval, y,
                c, 1.0f / game.editorsView.cam.zoom
            );
        }
        y += interval;
    }
}


/**
 * @brief Draws a small red X on the mouse cursor,
 * signifying an operation has failed.
 */
void Editor::drawOpErrorCursor() {
    const ALLEGRO_COLOR ERROR_COLOR = al_map_rgb(255, 0, 0);
    float errorFlashTimeRatio = opErrorFlashTimer.getRatioLeft();
    if(errorFlashTimeRatio <= 0.0f) return;
    Point pos = opErrorPos;
    drawBitmap(
        game.sysContent.bmpLeaderPrompt,
        Point(
            pos.x,
            pos.y - EDITOR::OP_ERROR_CURSOR_SIZE
        ),
        Point(
            EDITOR::OP_ERROR_CURSOR_SIZE * 2.5f,
            EDITOR::OP_ERROR_CURSOR_SIZE * 2.0f
        ),
        0.0f,
        mapAlpha(errorFlashTimeRatio * 192)
    );
    pos.x +=
        EDITOR::OP_ERROR_CURSOR_SHAKE_WIDTH *
        sin(game.timePassed * EDITOR::OP_ERROR_CURSOR_SHAKE_SPEED) *
        errorFlashTimeRatio;
    pos.y -= EDITOR::OP_ERROR_CURSOR_SIZE;
    al_draw_line(
        pos.x - EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.y - EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.x + EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.y + EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        multAlpha(ERROR_COLOR, errorFlashTimeRatio),
        EDITOR::OP_ERROR_CURSOR_THICKNESS
    );
    al_draw_line(
        pos.x + EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.y - EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.x - EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.y + EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        multAlpha(ERROR_COLOR, errorFlashTimeRatio),
        EDITOR::OP_ERROR_CURSOR_THICKNESS
    );
}


/**
 * @brief Draws things related to the current selection and its transformation
 * widget.
 *
 * @param selCtrl The selection controller.
 * @param traWid The transformation widget.
 */
void Editor::drawSelectionAndTransformationThings(
    const SelectionController& selCtrl,
    const TransformationWidget& traWid
) {
    //Transformation widget, if possible.
    if(selCtrl.isTransformationWidgetAvailable()) {
        Point selectionCenter, selectionSize;
        selCtrl.getTotalBBox(&selectionCenter, &selectionSize);
        if(selectionSize.x != 0.0f) {
            traWid.draw(
                &selectionCenter, &selectionSize,
                nullptr, 1.0f / game.editorsView.cam.zoom
            );
        }
    }
    
    //Selection manager.
    selCtrl.draw(
        game.editorsView.mouseCursorWorldPos, game.editorsView.cam.zoom
    );
}


/**
 * @brief Returns the text that should be displayed on the Dear ImGui list
 * navigation count text widget.
 *
 * @param curItemIdx Index of the current item.
 * @param listSize Current size of the list.
 * @param selectionSize How many items are currently selected.
 * @param itemTerm Term that designates what an item is, in singular.
 * @param itemTermPlural If the term in plural is different from the singular
 * term with an 's', specify it here.
 * @param showTermNormally If false, the term won't show up when showing
 * the usual text, but will show up when grammatically needed.
 * @param curItemName If this item has a name, specify it here.
 * @param curItemNameMono Whether the current item name's text widget should
 * use a monospace font.
 * @param outText1 The first text widget's contents are returned here.
 * @param outText1Disabled Whether the first text widget is disabled is
 * returned here.
 * @param outText2 If not nullptr, the second text widget's contents are
 * returned here, if any.
 * @param outText2Mono If not nullptr, whether the second text widget uses
 * a monospace font is returned here.
 */
void Editor::getGuiNavCurText(
    size_t curItemIdx, size_t listSize, size_t selectionSize,
    const string& itemTerm, const string& itemTermPlural,
    bool showTermNormally, const string& curItemName,
    bool curItemNameMono,
    string* outText1, bool* outText1Disabled,
    string* outText2, bool* outText2Mono
) {
    const string termLowerSingular = strToLower(itemTerm);
    const string termLowerPlural =
        itemTermPlural.empty() ?
        strToLower(itemTerm) + "s" :
        strToLower(itemTermPlural);
    const string termUpperSingular = strToSentence(itemTerm);
    
    *outText1 = "";
    *outText1Disabled = false;
    if(outText2) *outText2 = "";
    if(outText2Mono) *outText2Mono = false;
    
    if(listSize == 0) {
        *outText1 = "(No " + termLowerPlural + ")";
        *outText1Disabled = true;
        return;
    }
    
    if(selectionSize != 1) {
        *outText1 =
            i2s(selectionSize) + " / " + i2s(listSize) + " " +
            termLowerPlural + " selected";
        *outText1Disabled = selectionSize == 0;
        return;
    }
    
    const string curIdxStr =
        listSize == 0 || selectionSize == 0 || curItemIdx == INVALID ?
        "--" :
        ("#" + i2s(curItemIdx + 1));
    *outText1 = showTermNormally ? (termUpperSingular + " ") : "";
    (*outText1) += curIdxStr + " / " + i2s(listSize);
    
    if(!curItemName.empty()) {
        (*outText1) += ":";
        if(outText2) *outText2 = curItemName;
        if(outText2Mono) *outText2Mono = curItemNameMono;
    }
}


/**
 * @brief Returns the maximum number of history entries for this editor.
 *
 * @return The size.
 */
size_t Editor::getHistorySize() const {
    return EDITOR::DEF_MAX_HISTORY_SIZE;
}


/**
 * @brief Returns the position of the last widget, in window coordinates.
 *
 * @return The position.
 */
Point Editor::getLastWidgetPost() {
    return
        Point(
            ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x / 2.0,
            ImGui::GetItemRectMin().y + ImGui::GetItemRectSize().y / 2.0
        );
}


/**
 * @brief Returns a list of all areas for filling in quick play option-related
 * widgets.
 *
 * @param selectedAreaPath Path of the currently selected area.
 * @param outAreaNames The list of area names is returned here.
 * @param outAreaPaths The list of area paths is returned here.
 * @param outSelectedAreaIdx The index of which area is currently selected is
 * returned here, or -1 if none.
 */
void Editor::getQuickPlayAreaList(
    string selectedAreaPath,
    vector<string>* outAreaNames, vector<string>* outAreaPaths,
    int* outSelectedAreaIdx
) const {
    outAreaNames->clear();
    outAreaPaths->clear();
    *outSelectedAreaIdx = -1;
    
    auto scanAreas =
        [&selectedAreaPath, outAreaNames, outAreaPaths, outSelectedAreaIdx]
    (const vector<Area*> areas) {
        forIdx(a, areas) {
            Area* aPtr = areas[a];
            if(aPtr->manifest->path == selectedAreaPath) {
                *outSelectedAreaIdx = (int) outAreaNames->size();
            }
            outAreaNames->push_back(
                aPtr->name + "##" + aPtr->manifest->internalName
            );
            outAreaPaths->push_back(aPtr->manifest->path);
        }
    };
    scanAreas(game.content.areas.list[AREA_TYPE_SIMPLE]);
    scanAreas(game.content.areas.list[AREA_TYPE_MISSION]);
}


/**
 * @brief Returns whether or not Dear ImGui is currently focused on
 * a text widget.
 *
 * @return Whether it's focused.
 */
bool Editor::guiFocusedText() {
    return ImGui::GetIO().WantTextInput;
}


/**
 * @brief Handles an Allegro event for control-related things.
 *
 * @param ev Event to handle.
 */
void Editor::handleAllegroEvent(ALLEGRO_EVENT& ev) {
    if(game.fadeMgr.isFading()) return;
    
    bool isMouseInCanvas =
        dialogs.empty() && !isMouseInGui;
        
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        //General mouse handling.
        
        lastInputWasKeyboard = false;
        handleMouseUpdate(ev);
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        //Mouse button down in general.
        
        //If we started holding one button in the GUI but are now
        //pressing outside, force a mouse-up for that button. And vice-versa.
        if(isM1Pressed && (isM1DragStartInGui != isMouseInGui)) {
            isM1Pressed = false;
            handleLmbUp(ev);
        }
        if(isM2Pressed && (isM2DragStartInGui != isMouseInGui)) {
            isM2Pressed = false;
            handleRmbUp(ev);
        }
        if(isM3Pressed && (isM3DragStartInGui != isMouseInGui)) {
            isM3Pressed = false;
            handleMmbUp(ev);
        }
    }
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        isMouseInCanvas
    ) {
        //Mouse button down, inside the canvas.
        
        switch (ev.mouse.button) {
        case 1: {
            isM1Pressed = true;
            break;
        } case 2: {
            isM2Pressed = true;
            break;
        } case 3: {
            isM3Pressed = true;
            break;
        }
        }
        
        mouseDragStart = Point(ev.mouse.x, ev.mouse.y);
        mouseDragConfirmed = false;
        
        if(
            ev.mouse.button == lastMouseClick &&
            fabs(lastMouseClickPos.x - ev.mouse.x) < 4.0f &&
            fabs(lastMouseClickPos.y - ev.mouse.y) < 4.0f &&
            subState == lastMouseClickSubState &&
            doubleClickTime > 0
        ) {
            //Double-click.
            
            if(guiFocusedText()) {
                //If Dear ImGui needs the keyboard, then a textbox is likely
                //in use. Clicking could change the state of the editor's data,
                //so ignore it now, and let Dear ImGui close the box.
                isM1Pressed = false;
                
            } else {
            
                switch(ev.mouse.button) {
                case 1: {
                    handleLmbDoubleClick(ev);
                    break;
                } case 2: {
                    handleRmbDoubleClick(ev);
                    break;
                } case 3: {
                    handleMmbDoubleClick(ev);
                    break;
                }
                }
                
                doubleClickTime = 0;
            }
            
        } else {
            //Single-click.
            
            if(guiFocusedText()) {
                //If Dear ImGui needs the keyboard, then a textbox is likely
                //in use. Clicking could change the state of the editor's data,
                //so ignore it now, and let Dear ImGui close the box.
                isM1Pressed = false;
                
            } else {
            
                lastMouseClickSubState = subState;
                
                switch(ev.mouse.button) {
                case 1: {
                    handleLmbDown(ev);
                    break;
                } case 2: {
                    handleRmbDown(ev);
                    break;
                } case 3: {
                    handleMmbDown(ev);
                    break;
                }
                }
                
                lastMouseClick = ev.mouse.button;
                lastMouseClickPos.x = ev.mouse.x;
                lastMouseClickPos.y = ev.mouse.y;
                doubleClickTime = EDITOR::DOUBLE_CLICK_TIMEOUT;
                
            }
        }
        
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        //Mouse button up.
        
        switch(ev.mouse.button) {
        case 1: {
            if(isM1Pressed) {
                isM1Pressed = false;
                handleLmbUp(ev);
            }
            break;
        } case 2: {
            if(isM2Pressed) {
                isM2Pressed = false;
                handleRmbUp(ev);
            }
            break;
        } case 3: {
            if(isM3Pressed) {
                isM3Pressed = false;
                handleMmbUp(ev);
            }
            break;
        }
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED
    ) {
        //Mouse movement.
        
        if(
            fabs(ev.mouse.x - mouseDragStart.x) >=
            game.options.editors.mouseDragThreshold ||
            fabs(ev.mouse.y - mouseDragStart.y) >=
            game.options.editors.mouseDragThreshold
        ) {
            mouseDragConfirmed = true;
        }
        
        if(mouseDragConfirmed) {
            if(isM1Pressed) {
                handleLmbDrag(ev);
            }
            if(isM2Pressed) {
                handleRmbDrag(ev);
            }
            if(isM3Pressed) {
                handleMmbDrag(ev);
            }
        }
        if(
            (ev.mouse.dz != 0 || ev.mouse.dw != 0) &&
            isMouseInCanvas
        ) {
            handleMouseWheel(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        //Key down.
        
        lastInputWasKeyboard = true;
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            isShiftPressed = true;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            isCtrlPressed = true;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_ALT ||
            ev.keyboard.keycode == ALLEGRO_KEY_ALTGR
        ) {
            isAltPressed = true;
            
        }
        
        if(dialogs.empty()) {
            handleKeyDownAnywhere(ev);
            if(!guiFocusedText()) {
                handleKeyDownCanvas(ev);
            }
        }
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE &&
            !dialogs.empty()
        ) {
            closeTopDialog();
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        //Key up.
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            isShiftPressed = false;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            isCtrlPressed = false;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_ALT ||
            ev.keyboard.keycode == ALLEGRO_KEY_ALTGR
        ) {
            isAltPressed = false;
            
        }
        
        if(dialogs.empty()) {
            handleKeyUpAnywhere(ev);
            if(!guiFocusedText()) {
                handleKeyUpCanvas(ev);
            }
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        //Key char.
        
        if(dialogs.empty()) {
            handleKeyCharAnywhere(ev);
            if(!guiFocusedText()) {
                handleKeyCharCanvas(ev);
            }
        }
        
    }
    
    if(!dialogs.empty()) {
        if(dialogs.back()->eventCallback) {
            dialogs.back()->eventCallback(&ev);
        }
    }
}


/**
 * @brief Placeholder for handling a key being "char-typed" anywhere.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyCharAnywhere(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling a key being "char-typed" in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyCharCanvas(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling a key being pressed down anywhere.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyDownAnywhere(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling a key being pressed down in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyDownCanvas(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling a key being released anywhere.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyUpAnywhere(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling a key being released in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyUpCanvas(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the left mouse button being double-clicked
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleLmbDoubleClick(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the left mouse button being pressed down
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleLmbDown(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the left mouse button being dragged
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleLmbDrag(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the left mouse button released
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleLmbUp(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being double-clicked
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMmbDoubleClick(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being pressed down
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMmbDown(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being dragged
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMmbDrag(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being released
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMmbUp(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the mouse coordinates being updated.
 *
 * @param ev Event to process.
 */
void Editor::handleMouseUpdate(const ALLEGRO_EVENT& ev) {
    game.editorsView.updateMouseCursor(game.mouseCursor.winPos);
}


/**
 * @brief Placeholder for handling the mouse wheel being turned in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMouseWheel(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the right mouse button being double-clicked
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleRmbDoubleClick(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the right mouse button being pressed down
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleRmbDown(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the right mouse button being dragged
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleRmbDrag(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Placeholder for handling the right mouse button being released
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleRmbUp(const ALLEGRO_EVENT& ev) {}


/**
 * @brief Handles things related to the current selection and its transformation
 * widget when the left mouse button is pressed down.
 *
 * @param selCtrl The selection controller.
 * @param traWid The transformation widget.
 */
void Editor::handleSelectionAndTransformationLmbDown(
    SelectionController& selCtrl, TransformationWidget& traWid
) {
    bool twHandled = false;
    if(selCtrl.isTransformationWidgetAvailable()) {
        Point selectionCenter, selectionSize;
        selCtrl.getTotalBBox(&selectionCenter, &selectionSize);
        twHandled =
            traWid.handleMouseDown(
                game.editorsView.mouseCursorWorldPos,
                &selectionCenter, &selectionSize,
                nullptr, 1.0f / game.editorsView.cam.zoom
            );
    }
    
    if(twHandled) {
        selCtrl.startTransforming();
    } else {
        selCtrl.chooseViaMouseDown(
            game.editorsView.mouseCursorWorldPos,
            isShiftPressed, isCtrlPressed
        );
    }
}


/**
 * @brief Handles things related to the current selection and its transformation
 * widget when the left mouse button is dragged around.
 *
 * @param selCtrl The selection controller.
 * @param traWid The transformation widget.
 * @param mouseCursor Mouse cursor coordinates to use.
 * @param onPreTransform Code to run before any transformation is made, if any.
 * @return Whether any important data changed.
 */
bool Editor::handleSelectionAndTransformationLmbDrag(
    SelectionController& selCtrl, TransformationWidget& traWid,
    const Point& mouseCursor, std::function<void()> onPreTransform
) {
    //Rubber band.
    if(selCtrl.isCreatingRubberBand()) {
        selCtrl.updateRubberBand(
            game.editorsView.mouseCursorWorldPos,
            isShiftPressed, isCtrlPressed
        );
        return false;
    }
    
    //Drag move.
    if(selCtrl.isDragMoving()) {
        if(onPreTransform) onPreTransform();
        selCtrl.updateDragMove(game.editorsView.mouseCursorWorldPos);
        return true;
    }
    
    //Transformation widget.
    if(selCtrl.isTransformationWidgetAvailable()) {
        Point selectionCenter, selectionSize;
        selCtrl.getTotalBBox(&selectionCenter, &selectionSize);
        bool twHandled =
            traWid.handleMouseMove(
                mouseCursor,
                &selectionCenter, &selectionSize,
                nullptr, 1.0f / game.editorsView.cam.zoom,
                false, false, 0.10f, isAltPressed
            );
        if(twHandled) {
            if(onPreTransform) onPreTransform();
            selCtrl.applyTransformation(
                selectionCenter, selectionSize
            );
            return true;
        }
    }
    
    return false;
}


/**
 * @brief Handles things related to the current selection and its transformation
 * widget when the left mouse button is released.
 *
 * @param selCtrl The selection controller.
 * @param traWid The transformation widget.
 */
void Editor::handleSelectionAndTransformationLmbUp(
    SelectionController& selCtrl, TransformationWidget& traWid
) {
    selCtrl.handleMouseUp();
    traWid.handleMouseUp();
}


/**
 * @brief Returns a string describing either an individual item's index,
 * or the total amount of multiple items. Useful for things like the status bar.
 *
 * @param singleIdx Index of the single item, 0-indexed.
 * INVALID if there are 0 or multiple items.
 * @param amount Amount of items.
 * @param singularTerm Term that designates the items, in singular.
 * @param pluralTerm If the term in plural is different from the term in
 * singular plus an 's', specify it here.
 * @return The string.
 */
string Editor::getAmountOrIdxDescription(
    size_t singleIdx, size_t amount,
    const string& singularTerm, const string& pluralTerm
) const {
    if(singleIdx != INVALID && amount == 1) {
        return singularTerm + " #" + i2s(singleIdx + 1);
    } else {
        return amountStr((int) amount, singularTerm, pluralTerm);
    }
}


/**
 * @brief Returns whether a given internal name is good or not.
 *
 * @param name The internal name to check.
 * @return Whether it's good.
 */
bool Editor::isInternalNameGood(const string& name) const {
    forIdx(c, name) {
        char ch = name[c];
        const bool isLowercase = ch >= 'a' && ch <= 'z';
        const bool isDigit = ch >= '0' && ch <= '9';
        const bool isUnderscore = ch == '_';
        if(!isLowercase && !isDigit && !isUnderscore) return false;
    }
    return true;
}


/**
 * @brief Returns whether or not the pressed key corresponds to the specified
 * key combination. Used for keyboard shortcuts.
 *
 * @param pressedKey Key that the user pressed.
 * @param matchKey Key that must be matched in order to return true.
 * @param needsCtrl If true, only returns true if Ctrl was also pressed.
 * @param needsShift If true, only returns true if Shift was also pressed.
 * @return Whether the pressed key corresponds.
 */
bool Editor::keyCheck(
    int pressedKey, int matchKey,
    bool needsCtrl, bool needsShift
) {

    if(pressedKey != matchKey) {
        return false;
    }
    if(needsCtrl != isCtrlPressed) {
        return false;
    }
    if(needsShift != isShiftPressed) {
        return false;
    }
    return true;
}


/**
 * @brief Processes Dear ImGui widgets for visualizing and editing a color
 * keyframe interpolator.
 *
 * @param label Label for a keyframe's value.
 * @param interpolator Interpolator to edit.
 * @param selKeyframeIdx Index of the currently selected keyframe.
 * @return Whether anything in the interpolator was changed.
 */
bool Editor::keyframeEditor(
    const string& label,
    KeyframeInterpolator<ALLEGRO_COLOR>& interpolator,
    size_t& selKeyframeIdx
) {
    ImGui::BeginFrameBox(label + "frameBox");
    
    //Visualizer.
    keyframeVisualizer(interpolator, selKeyframeIdx);
    
    //Organizer.
    bool result = keyframeOrganizer(label, interpolator, selKeyframeIdx);
    
    ImGui::EndFrameBox();
    
    if(interpolator.getKeyframeCount() > 1) {
        //Time value.
        float time = interpolator.getKeyframe(selKeyframeIdx).first;
        if(ImGui::SliderFloat("Time", &time, 0.0f, 1.0f)) {
            interpolator.setKeyframeTime(
                selKeyframeIdx, time, &selKeyframeIdx
            );
            result = true;
        }
        setTooltip(
            "Time at which this keyframe occurs.\n"
            "0 means the beginning, 1 means the end.",
            "", WIDGET_EXPLANATION_SLIDER
        );
    }
    
    //Color editor.
    ALLEGRO_COLOR value = interpolator.getKeyframe(selKeyframeIdx).second;
    if(ImGui::ColorEdit4(label.c_str(), (float*) &value)) {
        interpolator.setKeyframeValue(selKeyframeIdx, value);
        result = true;
    }
    setTooltip("What color to use at this keyframe.");
    
    return result;
}


/**
 * @brief Processes Dear ImGui widgets for visualizing and editing a float
 * keyframe interpolator.
 *
 * @param label Label for a keyframe's value.
 * @param interpolator Interpolator to edit.
 * @param selKeyframeIdx Index of the currently selected keyframe.
 * @return Whether anything in the interpolator was changed.
 */
bool Editor::keyframeEditor(
    const string& label,
    KeyframeInterpolator<float>& interpolator,
    size_t& selKeyframeIdx
) {
    ImGui::BeginFrameBox(label + "frameBox");
    
    //Visualizer.
    keyframeVisualizer(interpolator, selKeyframeIdx);
    
    //Organizer.
    bool result = keyframeOrganizer(label, interpolator, selKeyframeIdx);
    
    ImGui::EndFrameBox();
    
    if(interpolator.getKeyframeCount() > 1) {
        //Time value.
        float time = interpolator.getKeyframe(selKeyframeIdx).first;
        if(ImGui::SliderFloat("Time", &time, 0.0f, 1.0f)) {
            interpolator.setKeyframeTime(
                selKeyframeIdx, time, &selKeyframeIdx
            );
            result = true;
        }
        setTooltip(
            "Time at which this keyframe occurs.\n"
            "0 means the beginning, 1 means the end.",
            "", WIDGET_EXPLANATION_SLIDER
        );
    }
    
    //Float value.
    float value = interpolator.getKeyframe(selKeyframeIdx).second;
    if(ImGui::DragFloat(label.c_str(), &value)) {
        interpolator.setKeyframeValue(selKeyframeIdx, value);
        result = true;
    }
    setTooltip("What value to use at this keyframe.");
    
    return result;
}


/**
 * @brief Processes Dear ImGui widgets for visualizing and editing a point
 * keyframe interpolator.
 *
 * @param label Label for a keyframe's value.
 * @param interpolator Interpolator to edit.
 * @param selKeyframeIdx Index of the currently selected keyframe.
 * @return Whether anything in the interpolator was changed.
 */
bool Editor::keyframeEditor(
    const string& label,
    KeyframeInterpolator<Point>& interpolator,
    size_t& selKeyframeIdx
) {
    ImGui::BeginFrameBox(label + "frameBox");
    
    //Visualizer.
    keyframeVisualizer(interpolator, selKeyframeIdx);
    
    //Organizer.
    bool result = keyframeOrganizer(label, interpolator, selKeyframeIdx);
    
    ImGui::EndFrameBox();
    
    if(interpolator.getKeyframeCount() > 1) {
        //Time value.
        float time = interpolator.getKeyframe(selKeyframeIdx).first;
        if(ImGui::SliderFloat("Time", &time, 0.0f, 1.0f)) {
            interpolator.setKeyframeTime(
                selKeyframeIdx, time, &selKeyframeIdx
            );
            result = true;
        }
        setTooltip(
            "Time at which this keyframe occurs.\n"
            "0 means the beginning, 1 means the end.",
            "", WIDGET_EXPLANATION_SLIDER
        );
    }
    
    //Float values.
    Point value = interpolator.getKeyframe(selKeyframeIdx).second;
    if(ImGui::DragFloat2(label.c_str(), (float*) &value)) {
        interpolator.setKeyframeValue(selKeyframeIdx, value);
        result = true;
    }
    setTooltip("What coordinates to use at this keyframe.");
    
    return result;
}


/**
 * @brief Processes Dear ImGui widgets tha allow organizing keyframe
 * interpolators.
 *
 * @tparam InterT Type of the interpolator value.
 * @param buttonId Prefix for the Dear ImGui ID of the navigation buttons.
 * @param interpolator Interpolator to get data from.
 * @param selKeyframeIdx Index of the currently selected keyframe.
 * @return Whether anything in the interpolator was changed.
 */
template <class InterT>
bool Editor::keyframeOrganizer(
    const string& buttonId,
    KeyframeInterpolator<InterT>& interpolator,
    size_t& selKeyframeIdx
) {
    bool result = false;
    
    //First, some utility setup.
    if(interpolator.getKeyframeCount() == 1) {
        interpolator.setKeyframeTime(0, 0.0f);
    }
    processGuiNavSetup(
        &selKeyframeIdx, interpolator.getKeyframeCount(), false
    );
    
    ImGui::BeginAlign();
    
    //Previous keyframe button.
    if(interpolator.getKeyframeCount() > 1) {
        processGuiNavWidgetPrev(
            &selKeyframeIdx, interpolator.getKeyframeCount(),
            buttonId + "prevButton", 0.75f
        );
        setTooltip("Select the previous keyframe.");
    } else {
        ImGui::Dummy(ImVec2(24, 24));
    }
    
    //Current keyframe text.
    string curKeyframeStr;
    bool curKeyframeDisabled;
    getGuiNavCurText(
        selKeyframeIdx, interpolator.getKeyframeCount(), 1,
        "Keyframe", "", true, "", false,
        &curKeyframeStr, &curKeyframeDisabled, nullptr, nullptr
    );
    ImGui::SameLine();
    ImGui::AlignNextText(curKeyframeStr.c_str());
    curKeyframeDisabled ?
    ImGui::TextDisabled("%s", curKeyframeStr.c_str()) :
    ImGui::Text("%s", curKeyframeStr.c_str());
    
    //Next keyframe button.
    ImGui::SameLine();
    if(interpolator.getKeyframeCount() > 1) {
        ImGui::AlignNextItems({24}, 1.0f);
        processGuiNavWidgetNext(
            &selKeyframeIdx, interpolator.getKeyframeCount(),
            buttonId + "nextButton", 0.75f
        );
        setTooltip("Select the next keyframe.");
    } else {
        ImGui::Dummy(ImVec2(24, 24));
    }
    
    ImGui::EndAlign();
    ImGui::BeginAlign();
    ImGui::AlignNextItems({24, 24});
    
    //Create keyframe button.
    size_t prevSelKeyframeIdx = selKeyframeIdx;
    if(
        processGuiNavWidgetNew(
            &selKeyframeIdx, interpolator.getKeyframeCount(),
            buttonId + "createButton", 0.75f
        )
    ) {
        float prevT = interpolator.getKeyframe(prevSelKeyframeIdx).first;
        float nextT =
            prevSelKeyframeIdx == interpolator.getKeyframeCount() - 1 ?
            1.0f :
            interpolator.getKeyframe(selKeyframeIdx).first;
        float newT = (prevT + nextT) / 2.0f;
        
        interpolator.addNew(newT, interpolator.get(newT));
        setStatus(
            "Created keyframe #" + i2s(selKeyframeIdx + 1) + "."
        );
        result = true;
    }
    setTooltip("Add a new keyframe after the currently selected one.");
    
    //Delete keyframe button.
    ImGui::SameLine();
    if(interpolator.getKeyframeCount() > 1) {
        prevSelKeyframeIdx = selKeyframeIdx;
        if(
            processGuiNavWidgetDel(
                &selKeyframeIdx, interpolator.getKeyframeCount(),
                buttonId + "deleteButton", 0.75f
            )
        ) {
            interpolator.deleteKeyframe(prevSelKeyframeIdx);
            setStatus("Deleted keyframe #" + i2s(prevSelKeyframeIdx + 1) + ".");
            result = true;
        }
        setTooltip("Delete the currently selected keyframe.");
    } else {
        ImGui::Dummy(ImVec2(24, 24));
    }
    
    ImGui::EndAlign();
    
    return result;
}


/**
 * @brief Draws a Dear ImGui-like visualizer for keyframes involving colors.
 *
 * @param interpolator Interpolator to get the information from.
 * @param selKeyframeIdx Index of the currently selected keyframe.
 */
void Editor::keyframeVisualizer(
    KeyframeInterpolator<ALLEGRO_COLOR>& interpolator,
    size_t selKeyframeIdx
) {
    if(interpolator.getKeyframeCount() <= 1) return;
    
    //Setup.
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    //Draw the classic alpha checkerboard background.
    ImGui::RenderColorRectWithAlphaCheckerboard(
        drawList, pos,
        ImVec2(pos.x + (ImGui::GetColumnWidth() - 1), pos.y + 40),
        ImColor(0.0f, 0.0f, 0.0f, 0.0f), 5, ImVec2(0.0f, 0.0f)
    );
    
    //Draw the rectangle of the color from the start to the first keyframe.
    auto firstKf = interpolator.getKeyframe(0);
    ALLEGRO_COLOR cStart = firstKf.second;
    drawList->AddRectFilled(
        pos,
        ImVec2(
            pos.x + (ImGui::GetColumnWidth() - 1) * firstKf.first,
            pos.y + 40
        ),
        ImColor(cStart.r, cStart.g, cStart.b, cStart.a)
    );
    
    //Draw the rectangles of the colors between the keyframes.
    for(size_t t = 0; t < interpolator.getKeyframeCount() - 1; t++) {
        auto kf1 = interpolator.getKeyframe(t);
        auto kf2 = interpolator.getKeyframe(t + 1);
        ALLEGRO_COLOR c1 = kf1.second;
        ALLEGRO_COLOR c2 = kf2.second;
        
        drawList->AddRectFilledMultiColor(
            ImVec2(
                pos.x + (ImGui::GetColumnWidth() - 1) * kf1.first,
                pos.y
            ),
            ImVec2(
                pos.x + (ImGui::GetColumnWidth() - 1) * kf2.first,
                pos.y + 40
            ),
            ImColor(c1.r, c1.g, c1.b, c1.a), ImColor(c2.r, c2.g, c2.b, c2.a),
            ImColor(c2.r, c2.g, c2.b, c2.a), ImColor(c1.r, c1.g, c1.b, c1.a)
        );
    }
    
    //Draw the rectangle of the color from the final keyframe to the end.
    auto lastKf = interpolator.getKeyframe(interpolator.getKeyframeCount() - 1);
    ALLEGRO_COLOR cEnd = lastKf.second;
    drawList->AddRectFilled(
        ImVec2(
            pos.x + (ImGui::GetColumnWidth() - 1) * lastKf.first,
            pos.y
        ),
        ImVec2(pos.x + (ImGui::GetColumnWidth() - 1), pos.y + 40),
        ImColor(cEnd.r, cEnd.g, cEnd.b, cEnd.a)
    );
    
    //Draw the bars indicating the position of each keyframe.
    for(size_t c = 0; c < interpolator.getKeyframeCount(); c++) {
        float time = interpolator.getKeyframe(c).first;
        float lineX = time * (ImGui::GetColumnWidth() - 1);
        ImColor col =
            c == selKeyframeIdx ?
            ImGui::GetColorU32(ImGuiCol_PlotLinesHovered) :
            ImGui::GetColorU32(ImGuiCol_PlotLines);
        drawList->AddRectFilled(
            ImVec2(pos.x + lineX - 2, pos.y),
            ImVec2(pos.x + lineX + 2, pos.y + 43),
            col
        );
    }
    
    //Add a dummy to symbolize the space the visualizer took up.
    ImGui::Dummy(ImVec2(ImGui::GetColumnWidth(), 43));
    setTooltip(
        "This shows what the color looks like at any given point in the\n"
        "timeline. The vertical bars are keyframes, and the colors blend\n"
        "smoothly from one keyframe to the next.\n"
        "If there is only one keyframe, then the color is the same throughout."
    );
}


/**
 * @brief Draws a Dear ImGui-like visualizer for keyframes involving floats.
 *
 * @param interpolator Interpolator to get the information from.
 * @param selKeyframeIdx Index of the currently selected keyframe.
 */
void Editor::keyframeVisualizer(
    KeyframeInterpolator<float>& interpolator,
    size_t selKeyframeIdx
) {
    if(interpolator.getKeyframeCount() <= 1) return;
    
    //The built-in plot widget doesn't allow for dynamic spacing,
    //so we need to make our own.
    
    //Setup.
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImVec2((ImGui::GetColumnWidth() - 1), 40);
    
    float minValue = FLT_MAX;
    float maxValue = -FLT_MAX;
    
    for(size_t t = 0; t < interpolator.getKeyframeCount(); t++) {
        minValue = std::min(interpolator.getKeyframe(t).second, minValue);
        maxValue = std::max(interpolator.getKeyframe(t).second, maxValue);
    }
    
    if(minValue == maxValue) {
        //Add the same space above and below to get a nice line at the middle.
        minValue -= 10.0f;
        maxValue += 10.0f;
    }
    
    //Draw the background.
    drawList->AddRectFilled(
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + (ImGui::GetColumnWidth() - 1), pos.y + 40),
        ImGui::GetColorU32(ImGuiCol_FrameBg)
    );
    
    //Draw the chart line from the start to the first keyframe.
    auto firstKf = interpolator.getKeyframe(0);
    drawList->AddLine(
        ImVec2(
            pos.x,
            pos.y + interpolateNumber(
                firstKf.second, minValue, maxValue, size.y, 1
            )
        ),
        ImVec2(
            pos.x + size.x * firstKf.first,
            pos.y + interpolateNumber(
                firstKf.second, minValue, maxValue, size.y, 1
            )
        ),
        ImGui::GetColorU32(ImGuiCol_PlotLines), 2
    );
    
    //Draw the chart lines between the keyframes.
    for(size_t t = 0; t < interpolator.getKeyframeCount() - 1; t++) {
        auto kf1 = interpolator.getKeyframe(t);
        auto kf2 = interpolator.getKeyframe(t + 1);
        float f1 = kf1.second;
        float f2 = kf2.second;
        
        drawList->AddLine(
            ImVec2(
                pos.x + size.x * kf1.first,
                pos.y + interpolateNumber(
                    f1, minValue, maxValue, size.y, 1
                )
            ),
            ImVec2(
                pos.x + size.x * kf2.first,
                pos.y + interpolateNumber(
                    f2, minValue, maxValue, size.y, 1
                )
            ),
            ImGui::GetColorU32(ImGuiCol_PlotLines), 2
        );
    }
    
    //Draw the chart line from the final keyframe to the end.
    auto lastKf = interpolator.getKeyframe(interpolator.getKeyframeCount() - 1);
    drawList->AddLine(
        ImVec2(
            pos.x + size.x * lastKf.first,
            pos.y + interpolateNumber(
                lastKf.second, minValue, maxValue, size.y, 1
            )
        ),
        ImVec2(
            pos.x + size.x,
            pos.y + interpolateNumber(
                lastKf.second, minValue, maxValue, size.y, 1
            )
        ),
        ImGui::GetColorU32(ImGuiCol_PlotLines), 2
    );
    
    //Draw the bars indicating the position of each keyframe.
    for(size_t c = 0; c < interpolator.getKeyframeCount(); c++) {
        float time = interpolator.getKeyframe(c).first;
        float lineX = time * (ImGui::GetColumnWidth() - 1);
        ImColor col =
            c == selKeyframeIdx ?
            ImGui::GetColorU32(ImGuiCol_PlotLinesHovered) :
            ImGui::GetColorU32(ImGuiCol_PlotLines);
        drawList->AddRectFilled(
            ImVec2(pos.x + lineX - 2, pos.y),
            ImVec2(pos.x + lineX + 2, pos.y + 43),
            col
        );
    }
    
    //Add a dummy to symbolize the space the visualizer took up.
    ImGui::Dummy(ImVec2(ImGui::GetColumnWidth(), 43));
    setTooltip(
        "This shows what the value looks like at any given point in the\n"
        "timeline. The vertical bars are keyframes, and the values blend\n"
        "smoothly from one keyframe to the next.\n"
        "If there is only one keyframe, then the value is the same throughout."
    );
}


/**
 * @brief Draws a Dear ImGui-like visualizer pair for keyframes
 * involving points.
 *
 * @param interpolator Interpolator to get the information from.
 * @param selKeyframeIdx Index of the currently selected keyframe.
 */
void Editor::keyframeVisualizer(
    KeyframeInterpolator<Point>& interpolator,
    size_t selKeyframeIdx
) {
    if(interpolator.getKeyframeCount() <= 1) return;
    
    //Split the interpolator into two, one for each axis.
    KeyframeInterpolator<float> xInter(interpolator.getKeyframe(0).second.x);
    KeyframeInterpolator<float> yInter(interpolator.getKeyframe(0).second.y);
    
    xInter.setKeyframeTime(0, interpolator.getKeyframe(0).first);
    yInter.setKeyframeTime(0, interpolator.getKeyframe(0).first);
    
    for(size_t s = 1; s < interpolator.getKeyframeCount(); s++) {
        auto kf = interpolator.getKeyframe(s);
        xInter.addNew(kf.first, kf.second.x);
        yInter.addNew(kf.first, kf.second.y);
    }
    
    //Draw the two visualizers.
    keyframeVisualizer(xInter, selKeyframeIdx);
    keyframeVisualizer(yInter, selKeyframeIdx);
}


/**
 * @brief Exits out of the editor, with a fade.
 */
void Editor::leave() {
    //Save the user's preferred tree node open states.
    saveOptions();
    
    game.fadeMgr.startFade(false, [] () {
        if(game.quickPlay.areaPath.empty()) {
            game.states.titleScreen->pageToLoad = MAIN_MENU_PAGE_MAKE;
            game.changeState(game.states.titleScreen);
        } else {
            game.states.gameplay->pathOfAreaToLoad =
                game.quickPlay.areaPath;
            game.changeState(game.states.gameplay);
        }
    });
    
    setStatus("Bye!");
}


/**
 * @brief Displays a popup, if applicable, and fills it with selectable items
 * from a list.
 *
 * @param label Name of the popup.
 * @param items List of items.
 * @param pickedItem If an item was picked, set this to its name.
 * @param useMonospace Whether to use a monospace font for the items.
 * @return Whether an item was clicked on.
 */
bool Editor::listPopup(
    const char* label, const vector<string>& items, string* pickedItem,
    bool useMonospace
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        if(escapeWasPressed) {
            ImGui::CloseCurrentPopup();
        }
        if(useMonospace) {
            ImGui::PushFont(
                game.sysContent.fntDearImGuiMonospace,
                game.sysContent.fntDearImGuiMonospace->LegacySize
            );
        }
        forIdx(i, items) {
            string name = items[i];
            bool hitButton =
                useMonospace ?
                monoSelectable(name.c_str()) :
                ImGui::Selectable(name.c_str());
            if(hitButton) {
                *pickedItem = name;
                ret = true;
            }
        }
        if(useMonospace) ImGui::PopFont();
        ImGui::EndPopup();
    }
    return ret;
}


/**
 * @brief Displays a popup, if applicable, and fills it with selectable items
 * from a list.
 *
 * @param label Name of the popup.
 * @param items List of items.
 * @param pickedItemIdx If an item was picked, set this to its index.
 * @param useMonospace Whether to use a monospace font for the items.
 * @return Whether an item was clicked on.
 */
bool Editor::listPopup(
    const char* label, const vector<string>& items, int* pickedItemIdx,
    bool useMonospace
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        if(escapeWasPressed) {
            ImGui::CloseCurrentPopup();
        }
        if(useMonospace) {
            ImGui::PushFont(
                game.sysContent.fntDearImGuiMonospace,
                game.sysContent.fntDearImGuiMonospace->LegacySize
            );
        }
        forIdx(i, items) {
            string name = items[i];
            bool hitButton =
                useMonospace ?
                monoSelectable(name.c_str()) :
                ImGui::Selectable(name.c_str());
            if(hitButton) {
                *pickedItemIdx = (int) i;
                ret = true;
            }
        }
        if(useMonospace) ImGui::PopFont();
        ImGui::EndPopup();
    }
    return ret;
}


/**
 * @brief Loads things common for all editors.
 */
void Editor::load() {
    //Icon sub-bitmaps.
    bmpEditorIcons =
        game.content.bitmaps.list.get(game.sysContentNames.bmpEditorIcons);
    if(bmpEditorIcons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; i++) {
            editorIcons[i] =
                al_create_sub_bitmap(
                    bmpEditorIcons,
                    (int) (EDITOR::ICON_BMP_SIZE * i) +
                    (int) (EDITOR::ICON_BMP_PADDING * i),
                    0,
                    EDITOR::ICON_BMP_SIZE,
                    EDITOR::ICON_BMP_SIZE
                );
        }
    }
    
    //Misc. setup.
    isAltPressed = false;
    isCtrlPressed = false;
    isShiftPressed = false;
    lastInputWasKeyboard = false;
    manifest.clear();
    setStatus();
    changesMgr.reset();
    game.mouseCursor.showInOS();
    game.editorsView.updateTransformations();
    game.editorsView.cam.setPos(Point());
    game.editorsView.cam.setZoom(1.0f);
    updateStyle();
    
    game.fadeMgr.startFade(true, nullptr);
    ImGui::Reset();
}


/**
 * @brief Loads all mob types into the customCatTypes list.
 *
 * @param isAreaEditor If true, mob types that do not appear in the
 * area editor will not be counted for here.
 */
void Editor::loadCustomMobCatTypes(bool isAreaEditor) {
    //Load.
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        MobCategory* cPtr = game.mobCategories.get((MOB_CATEGORY) c);
        vector<string> typeNames;
        cPtr->getTypeNames(typeNames);
        
        forIdx(tn, typeNames) {
            MobType* mtPtr = cPtr->getType(typeNames[tn]);
            
            if(isAreaEditor && !mtPtr->appearsInAreaEditor) {
                continue;
            }
            
            string customCatName = mtPtr->customCategoryName;
            size_t customCatIdx;
            if(!isInMap(customCatNameIdxs, customCatName)) {
                customCatNameIdxs[customCatName] =
                    customCatTypes.size();
                customCatTypes.push_back(vector<MobType*>());
            }
            customCatIdx = customCatNameIdxs[customCatName];
            
            customCatTypes[customCatIdx].push_back(mtPtr);
        }
    }
    
    //Sort.
    std::sort(
        customCatTypes.begin(), customCatTypes.end(),
    [] (const vector<MobType*>& c1, const vector<MobType*>& c2) -> bool {
        return
        c1.front()->customCategoryName <
        c2.front()->customCategoryName;
    }
    );
    forIdx(c, customCatTypes) {
        vector<MobType*>& types = customCatTypes[c];
        //Sort the types within a custom category.
        std::sort(
            types.begin(), types.end(),
        [] (const MobType * t1, const MobType * t2) {
            return t1->name < t2->name;
        }
        );
        //Adjust customCatNameIdxs, since the list of custom category names
        //got shuffled earlier.
        customCatNameIdxs[
            customCatTypes[c][0]->customCategoryName
        ] = c;
    }
}


/**
 * @brief Opens a dialog warning the maker that they're editing something
 * in the base pack. Does not do anything if the player is an engine developer.
 *
 * @param doPickCallback Callback for if we actually have to do the pick.
 */
void Editor::openBaseContentWarningDialog(
    const std::function<void()>& doPickCallback
) {
    if(game.options.advanced.engineDev) {
        doPickCallback();
        return;
    }
    
    openDialog(
        "Base pack warning",
        std::bind(&Editor::processGuiDialogBaseContent, this)
    );
    dialogs.back()->customSize = Point(320, 0);
    baseContentWarningDoPickCallback = doPickCallback;
}


/**
 * @brief Opens a dialog where the user can choose a bitmap from the
 * game content.
 *
 * @param okCallback Callback for when the user chooses a bitmap.
 * @param recommendedFolder If not empty, recommend that the user picks bitmaps
 * with this folder name. Use "." for the graphics root folder.
 */
void Editor::openBitmapDialog(
    std::function<void(const string&)> okCallback,
    const string& recommendedFolder
) {
    bitmapDialogOkCallback = okCallback;
    bitmapDialogRecommendedFolder = recommendedFolder;
    bitmapDialogPicker.pickCallback =
        [this] (
            const string& newBmpName, const string&,
            const string&, void*, bool
    ) {
        bitmapDialogNewBmpName = newBmpName;
    };
    bitmapDialogPicker.needsFilterBoxFocus = true;
    
    openDialog(
        "Choose a bitmap",
        std::bind(&Editor::processGuiDialogBitmap, this)
    );
    dialogs.back()->closeCallback = [this] () {
        if(!bitmapDialogCurBmpName.empty()) {
            game.content.bitmaps.list.free(bitmapDialogCurBmpName);
            bitmapDialogCurBmpName.clear();
            bitmapDialogCurBmpPtr = nullptr;
            bitmapDialogNewBmpName.clear();
            bitmapDialogOkCallback = nullptr;
            bitmapDialogRecommendedFolder = "";
        }
    };
}


/**
 * @brief Opens a dialog.
 *
 * @param title Title of the dialog window.
 * This is normally a request to the user, like "Pick an area.".
 * @param processCallback A function to call when it's time to process
 * the contents inside the dialog.
 */
void Editor::openDialog(
    const string& title,
    const std::function<void()>& processCallback
) {
    Dialog* newDialog = new Dialog();
    
    newDialog->title = title;
    newDialog->processCallback = processCallback;
    
    dialogs.push_back(newDialog);
}


/**
 * @brief Opens a Dear ImGui dialog with a simple message and an "open manual"
 * button, designed for each editor's standard "help" information.
 *
 * @param message Text message.
 * @param page Manual page explaining the editor and content type in
 * more detail.
 */
void Editor::openHelpDialog(
    const string& message, const string& page
) {
    helpDialogMessage = message;
    helpDialogPage = page;
    openDialog("Help", std::bind(&Editor::processGuiDialogHelp, this));
    dialogs.back()->customSize = Point(400, 0);
}


/**
 * @brief Opens an input popup with a given name. Its logic must be run with
 * a call to processGuiPopupInput().
 *
 * @param label Name of the popup.
 */
void Editor::openInputPopup(const char* label) {
    needsInputPopupTextFocus = true;
    ImGui::OpenPopup(label);
}


/**
 * @brief Opens a Dear ImGui dialog with a simple message and an ok button.
 *
 * @param title Message box title.
 * @param message Text message.
 * @param closeCallback Code to run when the dialog is closed, if any.
 */
void Editor::openMessageDialog(
    const string& title, const string& message,
    const std::function<void()>& closeCallback
) {
    messageDialogMessage = message;
    openDialog(title, std::bind(&Editor::processGuiDialogMessage, this));
    dialogs.back()->customSize = Point(400, 0);
    dialogs.back()->closeCallback = closeCallback;
}


/**
 * @brief Opens a dialog where the user can create a new pack.
 */
void Editor::openNewPackDialog() {
    needsNewPackTextFocus = true;
    openDialog(
        "Create a new pack",
        std::bind(&Editor::processGuiDialogNewPack, this)
    );
    dialogs.back()->customSize = Point(520, 0);
}


/**
 * @brief Opens a dialog with "picker" widgets inside, with the given content.
 *
 * @param title Title of the picker's dialog window.
 * This is normally a request to the user, like "Pick an area.".
 * @param items List of items to populate the picker with.
 * @param pickCallback A function to call when the user clicks an item
 * or enters a new one.
 * This function's first argument is the name of the item.
 * Its second argument is the top-level category of the item, or empty string.
 * Its third argument is the second-level category of the item, or empty string.
 * Its fourth argument is a void pointer to any custom info, or nullptr.
 * Its fifth argument is whether it's a new item or not.
 * @param listHeader If not-empty, display this text above the list.
 * @param canMakeNew If true, the user can create a new element,
 * by writing its name on the textbox, and pressing the "+" button.
 * @param useMonospace Whether the items should use a monospace font.
 * @param filter Filter of names. Only items that match this will appear.
 */
void Editor::openPickerDialog(
    const string& title,
    const vector<PickerItem>& items,
    const std::function <void(
        const string&, const string&, const string&, void*, bool
    )>& pickCallback,
    const string& listHeader,
    bool canMakeNew,
    bool useMonospace,
    const string& filter
) {
    Picker* newPicker = new Picker(this);
    
    newPicker->items = items;
    newPicker->listHeader = listHeader;
    newPicker->pickCallback = pickCallback;
    newPicker->canMakeNew = canMakeNew;
    newPicker->useMonospace = useMonospace;
    newPicker->filter = filter;
    
    Dialog* newDialog = new Dialog();
    dialogs.push_back(newDialog);
    
    newDialog->title = title;
    newDialog->processCallback =
        std::bind(&Editor::Picker::process, newPicker);
    newDialog->closeCallback = [newPicker] () {
        delete newPicker;
    };
    newPicker->dialogPtr = newDialog;
}


/**
 * @brief Creates widgets with the goal of placing a disabled text widget to the
 * right side of the panel.
 *
 * @param title Title to write.
 */
void Editor::panelTitle(const char* title) {
    ImGui::SameLine();
    ImGui::BeginAlign();
    ImGui::AlignNextText(title, 1.0f);
    ImGui::TextDisabled("%s", title);
    ImGui::EndAlign();
}


/**
 * @brief Begins a Dear ImGui popup, with logic to close it if
 * Escape was pressed.
 *
 * @param label The popup's label.
 * @param flags Any Dear ImGui popup flags.
 * @return Whether the popup opened.
 */
bool Editor::popup(const char* label, ImGuiWindowFlags flags) {
    bool result = ImGui::BeginPopup(label, flags);
    if(result) {
        if(escapeWasPressed) {
            ImGui::CloseCurrentPopup();
        }
    }
    return result;
}


/**
 * @brief Processes all currently open dialogs for this frame.
 */
void Editor::processDialogs() {
    //Delete closed ones.
    for(size_t d = 0; d < dialogs.size();) {
        Dialog* dPtr = dialogs[d];
        
        if(!dPtr->isOpen) {
            if(dPtr->closeCallback) {
                dPtr->closeCallback();
            }
            delete dPtr;
            dialogs.erase(dialogs.begin() + d);
        } else {
            d++;
        }
    }
    
    //Process the latest one.
    if(!dialogs.empty()) {
        dialogs.back()->process();
    }
}


/**
 * @brief Processes the setup for the "widget" that controls the canvas.
 */
void Editor::processGuiCanvas() {
    ImGui::BeginChild("canvas", ImVec2(0, -EDITOR::STATUS_BAR_HEIGHT));
    ImGui::EndChild();
    isMouseInGui =
        !ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    ImVec2 itemSize = ImGui::GetItemRectSize();
    ImVec2 itemTL = ImGui::GetItemRectMin();
    
    Point curTL(itemTL.x, itemTL.y);
    Point curSize(itemSize.x, itemSize.y);
    Point curCenter = curTL + curSize / 2.0f;
    if(
        curCenter != game.editorsView.center ||
        curSize != game.editorsView.size
    ) {
        game.editorsView.center = curCenter;
        game.editorsView.size = curSize;
        game.editorsView.updateTransformations();
    }
}


/**
 * @brief Processes the base content editing warning dialog for this frame.
 */
void Editor::processGuiDialogBaseContent() {
    //Explanation text.
    ImGui::TextWrapped(
        "You're editing content in the base pack! The base pack is meant to "
        "contain stuff packaged with the engine, designed for other content "
        "to make use of. It's recommended that you don't change it! (Though "
        "you are free to look around.)\n"
        "\n"
        "Please read the manual for more information.\n"
        "\n"
        "Do you want to continue?"
    );
    
    //Go back button.
    ImGui::Spacer();
    ImGui::BeginAlign();
    ImGui::AlignNextItems({70, 70});
    if(ImGui::Button("Go back", ImVec2(70, 30))) {
        closeTopDialog();
    }
    
    //Continue playing button.
    ImGui::SameLine();
    if(ImGui::Button("Continue", ImVec2(70, 30))) {
        closeTopDialog();
        baseContentWarningDoPickCallback();
        baseContentWarningDoPickCallback = nullptr;
    }
    ImGui::EndAlign();
    
    //Open manual button.
    ImGui::BeginAlign();
    ImGui::AlignNextItems({100});
    if(ImGui::Button("Open manual", ImVec2(100, 25))) {
        openManual("making.html#packs");
    }
    ImGui::EndAlign();
}


/**
 * @brief Processes the bitmap picker dialog for this frame.
 */
void Editor::processGuiDialogBitmap() {
    static bool filterWithRecommendedFolder = true;
    
    //Fill the picker's items.
    bitmapDialogPicker.items.clear();
    for(auto& b : game.content.bitmaps.manifests) {
        if(
            !bitmapDialogRecommendedFolder.empty() &&
            filterWithRecommendedFolder
        ) {
            vector<string> parts = split(b.first, "/");
            string folder = parts.size() == 1 ? "." : parts[0];
            if(folder != bitmapDialogRecommendedFolder) continue;
        }
        
        bitmapDialogPicker.items.push_back(
            PickerItem(
                b.first,
                "Pack: " + game.content.packs.list[b.second.pack].name
            )
        );
    }
    
    //Update the image if needed.
    if(bitmapDialogNewBmpName != bitmapDialogCurBmpName) {
        if(!bitmapDialogCurBmpName.empty()) {
            game.content.bitmaps.list.free(bitmapDialogCurBmpName);
        }
        if(bitmapDialogNewBmpName.empty()) {
            bitmapDialogCurBmpPtr = nullptr;
            bitmapDialogCurBmpName.clear();
        } else {
            bitmapDialogCurBmpPtr =
                game.content.bitmaps.list.get(bitmapDialogNewBmpName);
            bitmapDialogCurBmpName = bitmapDialogNewBmpName;
        }
    }
    
    //Column setup.
    ImGui::Columns(2, "colBitmaps");
    ImGui::BeginChild("butOk");
    
    //Ok button.
    ImGui::BeginAlign();
    ImGui::AlignNextItems({200});
    if(!bitmapDialogCurBmpPtr) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Ok", ImVec2(200, 40))) {
        if(bitmapDialogOkCallback) {
            bitmapDialogOkCallback(bitmapDialogCurBmpName);
        }
        closeTopDialog();
    }
    if(!bitmapDialogCurBmpPtr) {
        ImGui::EndDisabled();
    }
    ImGui::EndAlign();
    
    //Recommended folder text.
    string folderStr =
        bitmapDialogRecommendedFolder == "." ?
        "(root)" : bitmapDialogRecommendedFolder;
    ImGui::Spacer();
    ImGui::Text("Recommended folder: %s", folderStr.c_str());
    
    //Recommended folder only checkbox.
    if(!bitmapDialogRecommendedFolder.empty()) {
        ImGui::Checkbox(
            "That folder only", &filterWithRecommendedFolder
        );
        setTooltip(
            "If checked, only images that belong to the\n"
            "recommended folder will be shown in the list."
        );
    }
    
    //Preview text.
    ImGui::Spacer();
    ImGui::Text("Preview:");
    
    //Preview image.
    if(bitmapDialogCurBmpPtr) {
        const int thumbMaxSize = 300;
        Point size =
            resizeToBoxKeepingAspectRatio(
                getBitmapDimensions(bitmapDialogCurBmpPtr),
                Point(thumbMaxSize)
            );
        ImGui::Image(bitmapDialogCurBmpPtr, size);
    }
    
    //Next column.
    ImGui::EndChild();
    ImGui::NextColumn();
    
    //Bitmap picker.
    bitmapDialogPicker.process();
    
    //Reset columns.
    ImGui::Columns(1);
}


/**
 * @brief Processes the help dialog widgets.
 */
void Editor::processGuiDialogHelp() {
    //Text.
    ImGui::BeginAlign();
    static int textWidth = 0;
    if(textWidth != 0) {
        ImGui::AlignNextItems({textWidth});
    }
    ImGui::TextWrapped("%s", helpDialogMessage.c_str());
    textWidth = ImGui::GetItemRectSize().x;
    ImGui::EndAlign();
    
    //Open manual button.
    ImGui::Spacer();
    ImGui::BeginAlign();
    ImGui::AlignNextItems({100, 100});
    if(ImGui::Button("Open manual", ImVec2(100, 40))) {
        openManual(helpDialogPage);
    }
    
    //Ok button.
    ImGui::SameLine();
    if(ImGui::Button("Ok", ImVec2(100, 40))) {
        closeTopDialog();
    }
    ImGui::EndAlign();
}


/**
 * @brief Processes the Dear ImGui message dialog widgets.
 */
void Editor::processGuiDialogMessage() {
    //Text.
    static int textWidth = 0;
    ImGui::BeginAlign();
    if(textWidth != 0) {
        ImGui::AlignNextItems({textWidth});
    }
    ImGui::TextWrapped("%s", messageDialogMessage.c_str());
    textWidth = ImGui::GetItemRectSize().x;
    ImGui::EndAlign();
    
    //Ok button.
    ImGui::Spacer();
    ImGui::BeginAlign();
    ImGui::AlignNextItems({100});
    if(ImGui::Button("Ok", ImVec2(100, 40))) {
        closeTopDialog();
    }
    ImGui::EndAlign();
}


/**
 * @brief Processes the dialog for creating a new pack.
 */
void Editor::processGuiDialogNewPack() {
    static string internalName = "my_pack";
    static string name = "My pack!";
    static string description;
    static string maker;
    string problem;
    bool hitCreateButton = false;
    
    //Internal name input.
    ImGui::FocusOnInputText(needsNewPackTextFocus);
    if(
        monoInputText(
            "Internal name", &internalName,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hitCreateButton = true;
    }
    setTooltip(
        "Internal name of the new pack.\n"
        "Remember to keep it simple, type in lowercase, and use underscores!"
    );
    
    //Name input.
    ImGui::Spacer();
    if(
        ImGui::InputText(
            "Name", &name,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hitCreateButton = true;
    }
    setTooltip("Proper name of the new pack.");
    
    //Description input.
    if(
        ImGui::InputText(
            "Description", &description,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hitCreateButton = true;
    }
    setTooltip("A description of the pack.");
    
    //Maker input.
    if(
        ImGui::InputText(
            "Maker", &maker,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hitCreateButton = true;
    }
    setTooltip("Who made the pack. So really, type your name or nickname.");
    
    //File explanation text.
    string explanation =
        "These properties can be changed later by editing the "
        "pack's data file.\n"
        "There are also more properties; check the manual "
        "for more information!\n"
        "Pack data file path: ";
    ImGui::TextWrapped("%s", explanation.c_str());
    
    //Path text.
    string pathToShow =
        internalName.empty() ?
        "" :
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        internalName + "/" +
        FILE_NAMES::PACK_DATA;
    monoText("%s", pathToShow.c_str());
    
    //Open manual button.
    if(ImGui::Button("Open manual")) {
        openManual("making.html#packs");
    }
    
    //Check if everything's ok.
    if(internalName.empty()) {
        problem = "You have to type an internal name first!";
    } else if(!isInternalNameGood(internalName)) {
        problem =
            "The internal name should only have lowercase letters,\n"
            "numbers, and underscores!";
    } else {
        for(const auto& p : game.content.packs.manifestsWithBase) {
            if(internalName == p) {
                problem = "There is already a pack with that internal name!";
                break;
            }
        }
    }
    if(name.empty()) {
        problem = "You have to type a name first!";
    }
    
    //Create button.
    ImGui::Spacer();
    ImGui::BeginAlign();
    ImGui::AlignNextItems({100});
    if(!problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create pack", ImVec2(100, 40))) {
        hitCreateButton = true;
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    setTooltip(
        problem.empty() ? "Create the pack!" : problem
    );
    ImGui::EndAlign();
    
    //Creation logic.
    if(hitCreateButton) {
        if(!problem.empty()) return;
        game.content.addNewPack(
            internalName, name, description, maker
        );
        forIdx(p, game.content.packs.manifestsWithBase) {
            if(game.content.packs.manifestsWithBase[p] == internalName) {
                newContentDialogPackIdx = (int) p;
                break;
            }
        }
        internalName.clear();
        name.clear();
        description.clear();
        maker.clear();
        closeTopDialog();
    }
}


/**
 * @brief Processes the Dear ImGui unsaved changes confirmation dialog
 * for this frame.
 */
void Editor::processGuiDialogUnsavedChanges() {
    //Explanation 1 text.
    size_t nrUnsavedChanges = changesMgr.getUnsavedChanges();
    string explanation1Str =
        "You have " +
        amountStr((int) nrUnsavedChanges, "unsaved change") +
        ", made in the last " +
        timeToStr3(
            changesMgr.getUnsavedTimeDelta(),
            "h", "m", "s",
            TIME_TO_STR_FLAG_NO_LEADING_ZEROS |
            TIME_TO_STR_FLAG_NO_LEADING_ZERO_PORTIONS
        ) +
        ".";
    ImGui::BeginAlign();
    ImGui::AlignNextText(explanation1Str.c_str());
    ImGui::Text("%s", explanation1Str.c_str());
    ImGui::EndAlign();
    
    //Explanation 3 text.
    string explanation2Str =
        "Do you want to save before " +
        changesMgr.getUnsavedWarningActionLong() + "?";
    ImGui::BeginAlign();
    ImGui::AlignNextText(explanation2Str.c_str());
    ImGui::Text("%s", explanation2Str.c_str());
    ImGui::EndAlign();
    
    //Cancel button.
    ImGui::BeginAlign();
    ImGui::AlignNextItems({180, 180, 180});
    if(ImGui::Button("Cancel", ImVec2(180, 30))) {
        closeTopDialog();
    }
    setTooltip("Never mind and go back.", "Esc");
    
    //Save and then perform the action.
    ImGui::SameLine();
    if(ImGui::Button("Save", ImVec2(180, 30))) {
        closeTopDialog();
        const std::function<bool()>& saveCallback =
            changesMgr.getUnsavedWarningSaveCallback();
        const std::function<void()>& actionCallback =
            changesMgr.getUnsavedWarningActionCallback();
        if(saveCallback()) {
            actionCallback();
        }
    }
    setTooltip(
        "Save first, then " +
        changesMgr.getUnsavedWarningActionShort() + ".",
        "Ctrl + S"
    );
    
    //Perform the action without saving button.
    ImGui::SameLine();
    if(ImGui::Button("Don't save", ImVec2(180, 30))) {
        closeTopDialog();
        const std::function<void()> actionCallback =
            changesMgr.getUnsavedWarningActionCallback();
        actionCallback();
    }
    string dontSaveTooltip =
        changesMgr.getUnsavedWarningActionShort() +
        " without saving.";
    dontSaveTooltip[0] = toupper(dontSaveTooltip[0]);
    setTooltip(dontSaveTooltip, "Ctrl + D");
    ImGui::EndAlign();
}


/**
 * @brief Processes the widgets that allow the player to set a custom
 * editor style.
 */
void Editor::processGuiEditorStyle() {
    //Style node.
    if(saveableTreeNode("options", "Style")) {
    
        //Use custom style checkbox.
        if(
            ImGui::Checkbox(
                "Use custom style", &game.options.editors.useCustomStyle
            )
        ) {
            updateStyle();
        }
        setTooltip(
            "Use a custom color scheme for the editor,\n"
            "instead of the default.\n"
            "Default: " + b2s(OPTIONS::EDITORS_D::USE_CUSTOM_STYLE) + "."
        );
        
        //Primary color.
        if(
            ImGui::ColorEdit3(
                "Custom primary color",
                (float*) &game.options.editors.primaryColor
            )
        ) {
            updateStyle();
        }
        setTooltip(
            "Primary color for the custom style."
        );
        
        //Secondary color.
        if(
            ImGui::ColorEdit3(
                "Custom secondary color",
                (float*) &game.options.editors.secondaryColor
            )
        ) {
            updateStyle();
        }
        setTooltip(
            "Secondary color for the custom style."
        );
        
        //Text color.
        if(
            ImGui::ColorEdit3(
                "Text color",
                (float*) &game.options.editors.textColor
            )
        ) {
            updateStyle();
        }
        setTooltip(
            "Color of text in the custom style."
        );
        
        //Highlight color.
        if(
            ImGui::ColorEdit3(
                "Highlight color",
                (float*) &game.options.editors.highlightColor
            )
        ) {
            updateStyle();
        }
        setTooltip(
            "Color of highlights in the custom style."
        );
        ImGui::TreePop();
    }
}


/**
 * @brief Processes the widgets that show the editor's history.
 *
 * @param history History data to use.
 * @param nameDisplayCallback When an entry's name needs to be displayed as
 * button text, this function gets called with the entry name as an argument,
 * to determine what the final button text will be.
 * @param pickCallback Code to run when an entry is picked.
 * @param tooltipCallback Code to obtain an entry's tooltip with, if any.
 */
void Editor::processGuiHistory(
    const vector<pair<string, string> >& history,
    const std::function<string(const string&)>& nameDisplayCallback,
    const std::function<void(const string&)>& pickCallback,
    const std::function<string(const string&)>& tooltipCallback
) {
    if(saveableTreeNode("load", "History")) {
    
        if(!history.empty() && !history[0].first.empty()) {
        
            size_t nFilledEntries = 0;
            forIdx(h, history) {
                if(!history[h].first.empty()) nFilledEntries++;
            }
            
            forIdx(h, history) {
                string path = history[h].first;
                if(path.empty()) continue;
                
                string name = history[h].second;
                if(name.empty()) name = history[h].first;
                name = nameDisplayCallback(name);
                name = trimWithEllipsis(name, 16);
                
                //History entry button.
                const ImVec2 buttonSize(120, 24);
                if(ImGui::Button((name + "##" + i2s(h)).c_str(), buttonSize)) {
                    pickCallback(path);
                }
                if(tooltipCallback) {
                    setTooltip(tooltipCallback(path));
                }
                ImGui::SetupButtonWrapping(
                    buttonSize.x, (int) (h + 1), (int) nFilledEntries
                );
            }
            
        } else {
        
            //No history text.
            ImGui::TextDisabled("(Empty)");
            
        }
        
        ImGui::TreePop();
        
    }
}


/**
 * @brief Processes the current item Dear ImGui widget for a typical
 * navigation box. This does not alter the list in any way.
 * This is expected to go in-between the previous item button and the
 * next button item, centered.
 *
 * @param curItemName If this item has a name, specify it here.
 * @param curItemNameMono If true, use a monospaced font for the item name.
 * @param showTermNormally If false, the term won't show up when showing
 * the usual text, but will show up when grammatically needed.
 */
void Editor::processGuiNavBoxCur(
    const string& curItemName, bool curItemNameMono, bool showTermNormally
) {
    string text;
    bool textDisabled;
    string itemNameText;
    bool itemNameTextMono;
    getGuiNavCurText(
        *curNavBoxSelIdxPtr, curNavBoxOnGetSize(),
        curNavBoxOnGetSelSize(), curNavBoxItemTerm, curNavBoxItemTermPlural,
        showTermNormally, curItemName, curItemNameMono,
        &text, &textDisabled, &itemNameText, &itemNameTextMono
    );
    
    int textW = ImGui::CalcTextSize(text.c_str()).x;
    if(itemNameTextMono) {
        ImGui::PushFont(
            game.sysContent.fntDearImGuiMonospace,
            game.sysContent.fntDearImGuiMonospace->LegacySize
        );
    }
    int nameTextW = ImGui::CalcTextSize(itemNameText.c_str()).x;
    if(itemNameTextMono) {
        ImGui::PopFont();
    }
    
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::AlignNextItems({textW, nameTextW});
    textDisabled ?
    ImGui::TextDisabled("%s", text.c_str()) :
    ImGui::Text("%s", text.c_str());
    if(!itemNameText.empty()) {
        ImGui::SameLine();
        itemNameTextMono ?
        monoText("%s", itemNameText.c_str()) :
        ImGui::Text("%s", itemNameText.c_str());
    }
}


/**
 * @brief Processes the end of a typical navigation box.
 */
void Editor::processGuiNavBoxEnd() {
    ImGui::EndAlign();
    ImGui::EndFrameBox();
}


/**
 * @brief Processes the next item Dear ImGui widget for a typical
 * navigation box.
 *
 * @return Whether the user pressed the button.
 */
bool Editor::processGuiNavBoxNext() {
    bool pressed = false;
    ImGui::SameLine();
    ImGui::AlignNextItems({32}, 1.0f);
    if(curNavBoxOnGetSize() > 0) {
        if(
            processGuiNavWidgetNext(
                curNavBoxSelIdxPtr, curNavBoxOnGetSize(),
                curNavBoxItemPrefix + "nextButton"
            )
        ) {
            pressed = true;
        }
        setTooltip(
            "Select the next " + strToLower(curNavBoxItemTerm) + "."
        );
    } else {
        processGuiNavBoxPlaceholder();
    }
    return pressed;
}


/**
 * @brief Processes a placeholder button-sized Dear ImGui dummy widget for
 * a typical navigation box.
 */
void Editor::processGuiNavBoxPlaceholder() {
    ImGui::Dummy(ImVec2(32, 32));
}


/**
 * @brief Processes the previous item Dear ImGui widget for a typical
 * navigation box.
 *
 * @return Whether the user pressed the button.
 */
bool Editor::processGuiNavBoxPrev() {
    bool pressed = false;
    if(curNavBoxOnGetSize() > 0) {
        if(
            processGuiNavWidgetPrev(
                curNavBoxSelIdxPtr, curNavBoxOnGetSize(),
                curNavBoxItemPrefix + "prevButton"
            )
        ) {
            pressed = true;
        }
        setTooltip(
            "Select the previous " + strToLower(curNavBoxItemTerm) + "."
        );
    } else {
        processGuiNavBoxPlaceholder();
    }
    return pressed;
}


/**
 * @brief Processes the start of the second line of a typical navigation box.
 *
 * @param nrItems Number of items the second line will contain
 */
void Editor::processGuiNavBoxSecondLine(size_t nrItems) {
    ImGui::EndAlign();
    ImGui::BeginAlign();
    ImGui::AlignNextItems(vector<int>(nrItems, 32));
}


/**
 * @brief Processes the start of a typical navigation box.
 *
 * @param widgetsPrefix Prefix to place before the name of any widgets relevant
 * to the nav box.
 * @param itemsTerm Term that designates the items of the list, in singular.
 * @param itemsTermPlural If the term in plural is different from the singular
 * term with an 's', specify it here.
 * @param selIdxPtr Pointer to the selected item's index.
 * @param onGetSize Callback for when the list size needs to be retrieved.
 * @param onGetSelSize Callback for when the selection size needs
 * to be retrieved.
 */
void Editor::processGuiNavBoxStart(
    const string& widgetsPrefix, const string& itemsTerm,
    const string& itemsTermPlural, size_t* selIdxPtr,
    const std::function<size_t()>& onGetSize,
    const std::function<size_t()>& onGetSelSize
) {
    curNavBoxItemPrefix = widgetsPrefix;
    curNavBoxItemTerm = itemsTerm;
    curNavBoxItemTermPlural = itemsTermPlural;
    curNavBoxSelIdxPtr = selIdxPtr;
    curNavBoxOnGetSize = onGetSize;
    curNavBoxOnGetSelSize = onGetSelSize;
    ImGui::BeginFrameBox(widgetsPrefix + "NavBox");
    ImGui::BeginAlign();
}


/**
 * @brief Processes the Dear ImGui list navigation widgets setup.
 *
 * @param curItemIdx Pointer to the index of the current item.
 * This will be adjusted accordingly to prevent errors.
 * @param listSize Current size of the list.
 * @param allowInvalid If true, INVALID becomes a possible value for the index.
 * If false, INVALID becomes 0.
 */
void Editor::processGuiNavSetup(
    size_t* curItemIdx, size_t listSize, bool allowInvalid
) {
    if(*curItemIdx == INVALID) {
        if(allowInvalid) {
            return;
        } else {
            *curItemIdx = 0;
        }
    }
    
    if(listSize == 0) {
        *curItemIdx = allowInvalid ? INVALID : 0;
    } else if(*curItemIdx >= listSize) {
        *curItemIdx = listSize - 1;
    }
}


/**
 * @brief Processes the Dear ImGui list navigation delete widget. This
 * does not alter the list in any way.
 *
 * @param curItemIdx Pointer to the index of the current item.
 * This will be adjusted accordingly if the item is deleted.
 * @param listSize Current size of the list.
 * @param customButtonId If not empty, use this ID for the button.
 * @param buttonScale Scale the size of the buttons by this much.
 * @return Whether the user pressed the button.
 */
bool Editor::processGuiNavWidgetDel(
    size_t* curItemIdx, size_t listSize,
    const string& customButtonId, float buttonScale
) {
    bool pressed = false;
    
    if(
        ImGui::ImageButton(
            "delItemButton", editorIcons[EDITOR_ICON_REMOVE],
            Point(EDITOR::ICON_BMP_SIZE) * buttonScale
        )
    ) {
        if(listSize == 1) {
            *curItemIdx = 0;
        } else if(*curItemIdx >= listSize - 1) {
            *curItemIdx = listSize - 2;
        }
        pressed = true;
    }
    
    return pressed;
}


/**
 * @brief Processes the Dear ImGui list navigation move left widget.
 *
 * @param curItemIdx Pointer to the index of the current item.
 * This will be adjusted accordingly if the item moved.
 * @param listSize Current size of the list.
 * @param customButtonId If not empty, use this ID for the button.
 * @param buttonScale Scale the size of the buttons by this much.
 * @return Whether the user pressed the button, and it was possible to move
 * left.
 */
bool Editor::processGuiNavWidgetMoveLeft(
    size_t* curItemIdx, size_t listSize,
    const string& customButtonId, float buttonScale
) {
    bool pressed = false;
    
    if(
        ImGui::ImageButton(
            "moveItemLeftButton", editorIcons[EDITOR_ICON_MOVE_LEFT],
            Point(EDITOR::ICON_BMP_SIZE) * buttonScale
        )
    ) {
        if(*curItemIdx == 0) {
            setStatus("This is already the first one.");
        } else {
            pressed = true;
        }
    }
    
    return pressed;
}


/**
 * @brief Processes the Dear ImGui list navigation move right widget.
 *
 * @param curItemIdx Pointer to the index of the current item.
 * This will be adjusted accordingly if the item moved.
 * @param listSize Current size of the list.
 * @param customButtonId If not empty, use this ID for the button.
 * @param buttonScale Scale the size of the buttons by this much.
 * @return Whether the user pressed the button, and it was possible to move
 * right.
 */
bool Editor::processGuiNavWidgetMoveRight(
    size_t* curItemIdx, size_t listSize,
    const string& customButtonId, float buttonScale
) {
    bool pressed = false;
    
    if(
        ImGui::ImageButton(
            "moveItemRightButton", editorIcons[EDITOR_ICON_MOVE_RIGHT],
            Point(EDITOR::ICON_BMP_SIZE) * buttonScale
        )
    ) {
        if(*curItemIdx == listSize - 1) {
            setStatus("This is already the last one.");
        } else {
            pressed = true;
        }
    }
    
    return pressed;
}


/**
 * @brief Processes the Dear ImGui list navigation create widget. This does not
 * alter the list in any way.
 *
 * @param curItemIdx Pointer to the index of the current item.
 * This will be adjusted accordingly if a new item is created.
 * @param listSize Current size of the list.
 * @param customButtonId If not empty, use this ID for the button.
 * @param buttonScale Scale the size of the buttons by this much.
 * @return Whether the user pressed the button.
 */
bool Editor::processGuiNavWidgetNew(
    size_t* curItemIdx, size_t listSize,
    const string& customButtonId, float buttonScale
) {
    bool pressed = false;
    
    if(
        ImGui::ImageButton(
            "createItemButton", editorIcons[EDITOR_ICON_ADD],
            Point(EDITOR::ICON_BMP_SIZE) * buttonScale
        )
    ) {
        if(listSize == 0 || *curItemIdx == INVALID) {
            *curItemIdx = 0;
        } else {
            (*curItemIdx)++;
        }
        pressed = true;
    }
    
    return pressed;
}


/**
 * @brief Processes the Dear ImGui list navigation next widget.
 *
 * @param curItemIdx Pointer to the index of the current item.
 * This will be adjusted accordingly if a different item is chosen.
 * @param listSize Current size of the list.
 * @param customButtonId If not empty, use this ID for the button.
 * @param buttonScale Scale the size of the buttons by this much.
 * @return Whether the user pressed the button.
 */
bool Editor::processGuiNavWidgetNext(
    size_t* curItemIdx, size_t listSize,
    const string& customButtonId, float buttonScale
) {
    bool pressed = false;
    
    if(
        ImGui::ImageButton(
            customButtonId.empty() ? "nextItemButton" : customButtonId,
            editorIcons[EDITOR_ICON_NEXT],
            Point(EDITOR::ICON_BMP_SIZE) * buttonScale
        )
    ) {
        if(listSize > 0) {
            *curItemIdx = sumAndWrap(*curItemIdx, +1, listSize);
            pressed = true;
        }
    }
    
    return pressed;
}


/**
 * @brief Processes the Dear ImGui list navigation previous widget.
 *
 * @param curItemIdx Pointer to the index of the current item.
 * This will be adjusted accordingly if a different item is chosen.
 * @param listSize Current size of the list.
 * @param customButtonId If not empty, use this ID for the button.
 * @param buttonScale Scale the size of the buttons by this much.
 * @return Whether the user pressed the button.
 */
bool Editor::processGuiNavWidgetPrev(
    size_t* curItemIdx, size_t listSize,
    const string& customButtonId, float buttonScale
) {
    bool pressed = false;
    
    if(
        ImGui::ImageButton(
            customButtonId.empty() ? "prevItemButton" : customButtonId,
            editorIcons[EDITOR_ICON_PREVIOUS],
            Point(EDITOR::ICON_BMP_SIZE) * buttonScale
        )
    ) {
        if(listSize > 0) {
            *curItemIdx = sumAndWrap(*curItemIdx, -1, listSize);
            pressed = true;
        }
    }
    
    return pressed;
}


/**
 * @brief Processes a popup, if applicable, opened via openInputPopup(),
 * filling it with a text input for the user to type something in.
 *
 * @param label Name of the popup.
 * @param prompt What to prompt to the user. e.g.: "New name:"
 * @param text Pointer to the starting text, as well as the user's final text.
 * @param useMonospace Whether to use a monospace font.
 * @return Whether the user pressed Return or the Ok button.
 */
bool Editor::processGuiPopupInput(
    const char* label, const char* prompt, string* text, bool useMonospace
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        if(escapeWasPressed) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::Text("%s", prompt);
        ImGui::FocusOnInputText(needsInputPopupTextFocus);
        bool hitEnter = false;
        if(useMonospace) {
            hitEnter =
                monoInputText(
                    "##inputPopupText", text,
                    ImGuiInputTextFlags_EnterReturnsTrue |
                    ImGuiInputTextFlags_AutoSelectAll
                );
        } else {
            hitEnter =
                ImGui::InputText(
                    "##inputPopupText", text,
                    ImGuiInputTextFlags_EnterReturnsTrue |
                    ImGuiInputTextFlags_AutoSelectAll
                );
        }
        if(hitEnter) {
            ret = true;
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Ok")) {
            ret = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return ret;
}


/**
 * @brief Process the text widget in the status bar.
 *
 * This is responsible for showing the text if there's anything to say,
 * showing "Ready." if there's nothing to say,
 * and coloring the text in case it's an error that needs to be flashed red.
 */
void Editor::processGuiStatusBarText() {
    const ALLEGRO_COLOR ERROR_COLOR = al_map_rgb(255, 0, 0);
    float errorFlashTimeRatio = opErrorFlashTimer.getRatioLeft();
    if(errorFlashTimeRatio > 0.0f) {
        ImVec4 normalColorV = ImGui::GetStyle().Colors[ImGuiCol_Text];
        ALLEGRO_COLOR normalColor;
        normalColor.r = normalColorV.x;
        normalColor.g = normalColorV.y;
        normalColor.b = normalColorV.z;
        normalColor.a = normalColorV.w;
        ALLEGRO_COLOR errorFlashColor =
            interpolateColor(
                errorFlashTimeRatio,
                0.0f, 1.0f,
                normalColor, ERROR_COLOR
            );
        ImVec4 errorFlashColorV;
        errorFlashColorV.x = errorFlashColor.r;
        errorFlashColorV.y = errorFlashColor.g;
        errorFlashColorV.z = errorFlashColor.b;
        errorFlashColorV.w = errorFlashColor.a;
        ImGui::PushStyleColor(ImGuiCol_Text, errorFlashColorV);
    }
    ImGui::Text("%s", (statusText.empty() ? "Ready." : statusText.c_str()));
    if(errorFlashTimeRatio) {
        ImGui::PopStyleColor();
    }
}


/**
 * @brief Processes the Dear ImGui widgets that let users select a hazard.
 *
 * @param selectedHazardIname Internal name of the currently selected hazard.
 * @return Whether the hazard was changed.
 */
bool Editor::processGuiWidgetsHazardManagement(string& selectedHazardIname) {
    //Hazard combo.
    int selectedHazardIdx = -1;
    vector<string> allHazardINames = {""};
    vector<string> allHazardLabels = {NONE_OPTION + "##(none)"};
    for(auto& h : game.content.hazards.list) {
        allHazardINames.push_back(h.first);
        allHazardLabels.push_back(h.second.name + "##" + h.first);
        if(selectedHazardIname == h.first) {
            selectedHazardIdx = (int) allHazardLabels.size() - 1;
        }
    }
    
    if(selectedHazardIdx == -1) selectedHazardIdx = 0;
    
    bool result =
        ImGui::Combo("Hazard", &selectedHazardIdx, allHazardLabels);
        
    selectedHazardIname = allHazardINames[selectedHazardIdx];
    
    return result;
}



/**
 * @brief Processes the category and type widgets that allow a user to
 * select a mob type.
 *
 * @param customCatName Pointer to the custom category name reflected
 * in the combo box.
 * @param type Pointer to the type reflected in the combo box.
 * @param packFilter If not empty, only show mob types from this pack.
 * @return Whether the user changed the category/type.
 */
bool Editor::processGuiWidgetsMobType(
    string* customCatName, MobType** type, const string& packFilter
) {
    bool result = false;
    
    //These are used to communicate with the picker dialog, since that one
    //is processed somewhere else entirely.
    static bool internalChangedByDialog = false;
    static string internalCustomCatName;
    static MobType* internalMobType = nullptr;
    
    if(internalChangedByDialog) {
        //Somewhere else in the code, the picker dialog changed these variables
        //to whatever the user picked. Let's use them now, instead of the
        //ones passed by the function's arguments.
        result = true;
        internalChangedByDialog = false;
    } else {
        //The picker dialog hasn't changed these variables. Just use
        //whatever the function's arguments state.
        internalCustomCatName = *customCatName;
        internalMobType = *type;
    }
    
    //Column setup.
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(-1, 62.0f);
    
    //Search button.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 14.0f));
    bool searchButtonPressed =
        ImGui::ImageButton(
            "searchButton", editorIcons[EDITOR_ICON_SEARCH],
            Point(EDITOR::ICON_BMP_SIZE)
        );
    ImGui::PopStyleVar();
    
    vector<vector<MobType*> > finalList;
    if(!packFilter.empty()) {
        forIdx(c, customCatTypes) {
            finalList.push_back(vector<MobType*>());
            forIdx(n, customCatTypes[c]) {
                MobType* mtPtr = customCatTypes[c][n];
                if(mtPtr->manifest && mtPtr->manifest->pack == packFilter) {
                    finalList[c].push_back(mtPtr);
                }
            }
        }
    } else {
        finalList = customCatTypes;
    }
    
    if(searchButtonPressed) {
        vector<PickerItem> items;
        forIdx(c, finalList) {
            forIdx(n, finalList[c]) {
                MobType* mtPtr = finalList[c][n];
                items.push_back(
                    PickerItem(
                        mtPtr->name, mtPtr->customCategoryName
                    )
                );
            }
        }
        openPickerDialog(
            "Pick an object type", items,
            [this, finalList] (
                const string& n, const string& tc, const string& sc, void*, bool
        ) {
            //For clarity, this code will NOT be run within the context
            //of editor::processGuiWidgetsMobType, but will instead
            //be run wherever dialogs are processed.
            internalChangedByDialog = true;
            internalCustomCatName = tc;
            internalMobType = nullptr;
            size_t customCatIdx = customCatNameIdxs[tc];
            const vector<MobType*>& types =
                finalList[customCatIdx];
            forIdx(t, types) {
                if(types[t]->name == n) {
                    internalMobType = types[t];
                    return;
                }
            }
        },
        "", false
        );
    }
    setTooltip(
        "Search for an object type from the entire list."
    );
    
    ImGui::NextColumn();
    
    //Object category combobox.
    vector<string> categories;
    int selectedCategoryIdx = -1;
    forIdx(c, finalList) {
        string cn =
            customCatTypes[c].front()->customCategoryName;
        categories.push_back(cn);
        if(cn == internalCustomCatName) {
            selectedCategoryIdx = (int) c;
        }
    }
    
    if(ImGui::Combo("Category", &selectedCategoryIdx, categories, 15)) {
        result = true;
        internalCustomCatName = categories[selectedCategoryIdx];
        internalMobType =
            finalList[selectedCategoryIdx].empty() ?
            nullptr :
            finalList[selectedCategoryIdx][0];
    }
    setTooltip(
        "What category this object belongs to: a Pikmin, a leader, etc."
    );
    
    if(!internalCustomCatName.empty()) {
    
        //Object type combobox.
        vector<string> typeNames;
        size_t customCatIdx = customCatNameIdxs[internalCustomCatName];
        const vector<MobType*>& types = finalList[customCatIdx];
        forIdx(t, types) {
            MobType* tPtr = types[t];
            typeNames.push_back(tPtr->name);
        }
        
        string selectedTypeName;
        if(internalMobType) {
            selectedTypeName = internalMobType->name;
        }
        if(ImGui::Combo("Type", &selectedTypeName, typeNames, 15)) {
            result = true;
            forIdx(t, types) {
                if(types[t]->name == selectedTypeName) {
                    internalMobType = types[t];
                    break;
                }
            }
        }
        setTooltip(
            "The specific type of object this is, from the chosen category."
        );
    }
    
    ImGui::Columns();
    
    if(result) {
        *customCatName = internalCustomCatName;
        *type = internalMobType;
    }
    
    return result;
}


/**
 * @brief Processes the widgets for the pack selection, in a "new" dialog.
 *
 * @param pack Pointer to the internal name of the pack in the combobox.
 */
bool Editor::processGuiWidgetsNewDialogPack(string* pack) {
    bool changed = false;
    
    //Pack combo.
    vector<string> packs;
    for(const auto& p : game.content.packs.manifestsWithBase) {
        packs.push_back(game.content.packs.list[p].name);
    }
    if(packs.empty()) {
        //Failsafe.
        packs.push_back(FOLDER_NAMES::BASE_PACK);
    }
    newContentDialogPackIdx =
        std::min(newContentDialogPackIdx, (int) packs.size() - 1);
    changed = ImGui::Combo("Pack", &newContentDialogPackIdx, packs);
    setTooltip("What pack it will belong to.");
    
    //New pack button.
    ImGui::SameLine();
    if(ImGui::Button("New pack...")) {
        openNewPackDialog();
    }
    setTooltip("Create a new pack.");
    
    *pack = game.content.packs.manifestsWithBase[newContentDialogPackIdx];
    return changed;
}


/**
 * @brief Process the width and height widgets that allow a user to
 * specify the size of something.
 *
 * @param label Label for the widgets.
 * @param size Size variable to alter.
 * @param vSpeed Variable change speed. Same value you'd pass to
 * ImGui::DragFloat2. 1.0f for default.
 * @param keepAspectRatio If true, changing one will change the other
 * in the same ratio.
 * @param keepArea If true, changing one will change the other
 * such that the total area is preserved.
 * @param minSize Minimum value that either width or height is allowed
 * to have. Use -FLT_MAX for none.
 * @return Whether the user changed one of the values.
 */
bool Editor::processGuiWidgetsSize(
    const char* label, Point& size, float vSpeed,
    bool keepAspectRatio,
    bool keepArea,
    float minSize
) {
    bool ret = false;
    Point newSize = size;
    if(
        ImGui::DragFloat2(
            label, (float*) &newSize, vSpeed, minSize, FLT_MAX
        )
    ) {
    
        bool freeResize = !keepAspectRatio && !keepArea;
        bool valuesValid =
            size.x != 0.0f && size.y != 0.0f &&
            newSize.x != 0.0f && newSize.y != 0.0f;
            
        if(freeResize || !valuesValid) {
            //Just change them, forget about keeping the aspect ratio or area.
            newSize.x = std::max(minSize, newSize.x);
            newSize.y = std::max(minSize, newSize.y);
            
        } else if(keepAspectRatio) {
            //Keep the aspect ratio.
            float ratio = size.x / size.y;
            if(newSize.x != size.x) {
                //Must adjust Y.
                if(minSize != -FLT_MAX) {
                    newSize.x = std::max(minSize * ratio, newSize.x);
                    newSize.x = std::max(minSize, newSize.x);
                }
                newSize.y = newSize.x / ratio;
            } else {
                //Must adjust X.
                if(minSize != -FLT_MAX) {
                    newSize.y = std::max(minSize / ratio, newSize.y);
                    newSize.y = std::max(minSize, newSize.y);
                }
                newSize.x = newSize.y * ratio;
            }
            
        } else {
            //Keep the area.
            double area = (double) size.x * (double) size.y;
            if(newSize.x != size.x) {
                //Must adjust Y.
                if(minSize != -FLT_MAX) {
                    newSize.x = std::max(minSize, newSize.x);
                }
                newSize.y = area / newSize.x;
            } else {
                //Must adjust X.
                if(minSize != -FLT_MAX) {
                    newSize.y = std::max(minSize, newSize.y);
                }
                newSize.x = area / newSize.y;
            }
            
        }
        
        size = newSize;
        ret = true;
    }
    
    return ret;
}


/**
 * @brief Processes an ImGui::TreeNode, except it preemptively opens it or
 * closes it based on the user's preferences.
 *
 * It also saves the user's preferences as they open and close the node.
 * In order for these preferences to be saved to the disk, saveOptions must
 * be called.
 *
 * @param category Category this node belongs to.
 * This is just a generic term, and you likely want to use the panel
 * this node belongs to.
 * @param label Label to give to Dear ImGui.
 * @return Whether the node is open.
 */
bool Editor::saveableTreeNode(const string& category, const string& label) {
    string nodeName = getName() + "/" + category + "/" + label;
    ImGui::SetNextItemOpen(game.options.editors.openNodes[nodeName]);
    ImGui::PushFont(
        game.sysContent.fntDearImGuiHeader,
        game.sysContent.fntDearImGuiHeader->LegacySize
    );
    bool isOpen = ImGui::TreeNode(label.c_str());
    ImGui::PopFont();
    game.options.editors.openNodes[nodeName] = isOpen;
    return isOpen;
}


/**
 * @brief Sets the status bar's text, and notifies the user of an error,
 * if it is an error, by flashing the text.
 *
 * @param text Text to put in the status bar.
 * @param error Whether there was an error or not.
 */
void Editor::setStatus(const string& text, bool error) {
    statusText = text;
    if(error) {
        opErrorFlashTimer.start();
        opErrorPos = game.mouseCursor.winPos;
    }
}


/**
 * @brief Sets the tooltip of the previous widget.
 *
 * @param explanation Text explaining the widget.
 * @param shortcut If the widget has a shortcut key, specify its name here.
 * @param widgetExplanation If the way the widget works needs to be explained,
 * specify the explanation type here.
 */
void Editor::setTooltip(
    const string& explanation, const string& shortcut,
    const WIDGET_EXPLANATION widgetExplanation
) {
    if(!game.options.editors.showTooltips) {
        return;
    }
    
    if(lastInputWasKeyboard) {
        return;
    }
    
    if(
        ImGui::IsItemHovered(
            ImGuiHoveredFlags_AllowWhenDisabled |
            ImGuiHoveredFlags_DelayNormal |
            ImGuiHoveredFlags_NoSharedDelay |
            ImGuiHoveredFlags_Stationary
        )
    ) {
        if(ImGui::BeginTooltip()) {
        
            ImGui::Text("%s", explanation.c_str());
            
            string widgetExplanationText;
            switch(widgetExplanation) {
            case WIDGET_EXPLANATION_NONE: {
                break;
            }
            case WIDGET_EXPLANATION_DRAG: {
                widgetExplanationText =
                    "Click and drag left or right to change.\n"
                    "Hold Alt or Shift to change speed.\n"
                    "Click once or Ctrl + click to write a value.";
                break;
            }
            case WIDGET_EXPLANATION_SLIDER: {
                widgetExplanationText =
                    "Click and/or drag left or right to change.\n"
                    "Ctrl + click to write a value.";
                break;
            }
            }
            
            if(!widgetExplanationText.empty()) {
                ImGui::TextColored(
                    ImVec4(0.50f, 0.50f, 0.50f, 1.0f),
                    "%s", widgetExplanationText.c_str()
                );
            }
            
            if(!shortcut.empty()) {
                ImGui::TextColored(
                    ImVec4(0.70f, 0.70f, 0.70f, 1.0f),
                    "Shortcut key: %s", shortcut.c_str()
                );
            }
            
            ImGui::EndTooltip();
        }
    }
}


/**
 * @brief Snaps a point to either the vertical axis or horizontal axis,
 * depending on the anchor point.
 *
 * @param p Point to snap.
 * @param anchor Anchor point.
 * @return The snapped point.
 */
Point Editor::snapPointToAxis(const Point& p, const Point& anchor) {
    float hDiff = fabs(p.x - anchor.x);
    float vDiff = fabs(p.y - anchor.y);
    if(hDiff > vDiff) {
        return Point(p.x, anchor.y);
    } else {
        return Point(anchor.x, p.y);
    }
}


/**
 * @brief Snaps a point to the nearest grid intersection.
 *
 * @param p Point to snap.
 * @param gridInterval Current grid interval.
 * @return The snapped point.
 */
Point Editor::snapPointToGrid(const Point& p, float gridInterval) {
    return
        Point(
            round(p.x / gridInterval) * gridInterval,
            round(p.y / gridInterval) * gridInterval
        );
}


/**
 * @brief Unloads loaded editor-related content.
 */
void Editor::unload() {
    if(bmpEditorIcons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; i++) {
            al_destroy_bitmap(editorIcons[i]);
            editorIcons[i] = nullptr;
        }
        game.content.bitmaps.list.free(bmpEditorIcons);
        bmpEditorIcons = nullptr;
    }
    customCatNameIdxs.clear();
    customCatTypes.clear();
    game.mouseCursor.hideInOS();
}


/**
 * @brief Updates the history list, by adding a new entry or bumping it up.
 *
 * @param history History data to update.
 * @param manifest Manifest of the entry's content.
 * @param name Proper name of the entry.
 */
void Editor::updateHistory(
    vector<pair<string, string> >& history,
    const ContentManifest& manifest, const string& name
) {
    string finalName = name.empty() ? manifest.internalName : name;
    
    //First, check if it exists.
    size_t pos = INVALID;
    
    forIdx(h, history) {
        if(history[h].first == manifest.path) {
            pos = h;
            break;
        }
    }
    
    if(pos == 0) {
        //Already #1? Just update the name.
        history[0].second = finalName;
    } else if(pos == INVALID) {
        //If it doesn't exist, create it and add it to the top.
        history.insert(
            history.begin(),
            make_pair(manifest.path, finalName)
        );
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        history.erase(history.begin() + pos);
        history.insert(
            history.begin(),
            make_pair(manifest.path, finalName)
        );
    }
    
    if(history.size() > getHistorySize()) {
        history.erase(history.begin() + history.size() - 1);
    }
    
    //Save the history in the options.
    saveOptions();
}


/**
 * @brief Updates the Dear ImGui style based on the player's options.
 */
void Editor::updateStyle() {

    ImGuiStyle* style = &ImGui::GetStyle();
    style->FrameRounding = 3;
    style->IndentSpacing = 25;
    style->GrabMinSize = 15;
    style->ScrollbarSize = 16;
    style->WindowRounding = 5;
    style->PopupRounding = 5;
    style->GrabRounding = 4;
    style->ScrollbarRounding = 12;
    
    if(!game.options.editors.useCustomStyle) {
        //Use the default style.
        memcpy(
            &(ImGui::GetStyle().Colors),
            game.dearImGuiDefaultStyle,
            sizeof(ImVec4) * ImGuiCol_COUNT
        );
        
    } else {
        //Use the custom style.
        
        ALLEGRO_COLOR pri = game.options.editors.primaryColor;
        ALLEGRO_COLOR sec = game.options.editors.secondaryColor;
        ALLEGRO_COLOR txt = game.options.editors.textColor;
        
        ImVec4* colors = style->Colors;
        
        colors[ImGuiCol_Text] =
            ImVec4(txt.r, txt.g, txt.b, 1.0f);
        colors[ImGuiCol_TextDisabled] =
            ImVec4(txt.r * 0.5f, txt.g * 0.5f, txt.b * 0.5f, 1.0f);
        colors[ImGuiCol_WindowBg] =
            ImVec4(pri.r, pri.g, pri.b, 0.94f);
        colors[ImGuiCol_ChildBg] =
            ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] =
            ImVec4(pri.r * 1.3f, pri.g * 1.3f, pri.b * 1.3f, 0.94f);
        colors[ImGuiCol_Border] =
            ImVec4(sec.r, sec.g, sec.b, 0.50f);
        colors[ImGuiCol_BorderShadow] =
            ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] =
            ImVec4(sec.r * 0.4f, sec.g * 0.4f, sec.b * 0.4f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] =
            ImVec4(sec.r * 1.4f, sec.g * 1.4f, sec.b * 1.4f, 0.40f);
        colors[ImGuiCol_FrameBgActive] =
            ImVec4(sec.r * 1.3f, sec.g * 1.3f, sec.b * 1.3f, 0.67f);
        colors[ImGuiCol_TitleBg] =
            ImVec4(pri.r * 0.7f, pri.g * 0.7f, pri.b * 0.7f, 1.0f);
        colors[ImGuiCol_TitleBgActive] =
            ImVec4(sec.r * 0.9f, sec.g * 0.9f, sec.b * 0.9f, 1.0f);
        colors[ImGuiCol_TitleBgCollapsed] =
            ImVec4(pri.r * 0.2f, pri.g * 0.2f, pri.b * 0.2f, 0.51f);
        colors[ImGuiCol_MenuBarBg] =
            ImVec4(pri.r * 0.7f, pri.g * 0.7f, pri.b * 0.7f, 1.0f);
        colors[ImGuiCol_ScrollbarBg] =
            ImVec4(pri.r * 0.7f, pri.g * 0.7f, pri.b * 0.7f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] =
            ImVec4(sec.r, sec.g, sec.b, 1.0f);
        colors[ImGuiCol_ScrollbarGrabHovered] =
            ImVec4(sec.r * 1.1f, sec.g * 1.1f, sec.b * 1.1f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] =
            ImVec4(sec.r * 1.3f, sec.g * 1.3f, sec.b * 1.3f, 1.0f);
        colors[ImGuiCol_CheckMark] =
            ImVec4(sec.r * 1.1f, sec.g * 1.1f, sec.b * 1.1f, 1.0f);
        colors[ImGuiCol_SliderGrab] =
            ImVec4(sec.r * 1.1f, sec.g * 1.1f, sec.b * 1.1f, 1.0f);
        colors[ImGuiCol_SliderGrabActive] =
            ImVec4(sec.r * 1.3f, sec.g * 1.3f, sec.b * 1.3f, 1.0f);
        colors[ImGuiCol_Button] =
            ImVec4(sec.r, sec.g, sec.b, 0.40f);
        colors[ImGuiCol_ButtonHovered] =
            ImVec4(sec.r * 1.1f, sec.g * 1.1f, sec.b * 1.1f, 1.0f);
        colors[ImGuiCol_ButtonActive] =
            ImVec4(sec.r * 1.3f, sec.g * 1.3f, sec.b * 1.3f, 1.0f);
        colors[ImGuiCol_Header] =
            ImVec4(sec.r, sec.g, sec.b, 0.31f);
        colors[ImGuiCol_HeaderHovered] =
            ImVec4(sec.r * 1.1f, sec.g * 1.1f, sec.b * 1.1f, 0.80f);
        colors[ImGuiCol_HeaderActive] =
            ImVec4(sec.r * 1.3f, sec.g * 1.3f, sec.b * 1.3f, 1.0f);
        colors[ImGuiCol_Separator] =
            colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered] =
            ImVec4(sec.r * 1.1f, sec.g * 1.1f, sec.b * 1.1f, 0.78f);
        colors[ImGuiCol_SeparatorActive] =
            ImVec4(sec.r * 1.2f, sec.g * 1.2f, sec.b * 1.2f, 1.0f);
        colors[ImGuiCol_ResizeGrip] =
            ImVec4(sec.r, sec.g, sec.b, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] =
            ImVec4(sec.r * 1.1f, sec.g * 1.1f, sec.b * 1.1f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] =
            ImVec4(sec.r * 1.3f, sec.g * 1.3f, sec.b * 1.3f, 0.95f);
        colors[ImGuiCol_Tab] =
            ImLerp(
                colors[ImGuiCol_Header],
                colors[ImGuiCol_TitleBgActive],
                0.80f
            );
        colors[ImGuiCol_TabHovered] =
            colors[ImGuiCol_HeaderHovered];
        colors[ImGuiCol_TabSelected] =
            ImLerp(
                colors[ImGuiCol_HeaderActive],
                colors[ImGuiCol_TitleBgActive],
                0.60f
            );
        colors[ImGuiCol_TabDimmed] =
            ImLerp(
                colors[ImGuiCol_Tab],
                colors[ImGuiCol_TitleBg],
                0.80f
            );
        colors[ImGuiCol_TabDimmedSelected] =
            ImLerp(
                colors[ImGuiCol_TabSelected],
                colors[ImGuiCol_TitleBg],
                0.40f
            );
        colors[ImGuiCol_PlotLines] =
            ImVec4(sec.r, sec.g, sec.b, 1.0f);
        colors[ImGuiCol_PlotLinesHovered] =
            ImVec4(sec.r * 2, sec.g * 2, sec.b * 2, 1.0f);
        colors[ImGuiCol_PlotHistogram] =
            ImVec4(sec.r, sec.g, sec.b, 1.0f);
        colors[ImGuiCol_PlotHistogramHovered] =
            ImVec4(sec.r * 1.1f, sec.g * 1.1f, sec.b * 1.1f, 1.0f);
        colors[ImGuiCol_TextSelectedBg] =
            ImVec4(sec.r, sec.g, sec.b, 0.35f);
        colors[ImGuiCol_DragDropTarget] =
            ImVec4(sec.r * 1.3f, sec.g * 1.3f, sec.b * 1.3f, 0.90f);
        colors[ImGuiCol_NavCursor] =
            ImVec4(sec.r, sec.g, sec.b, 1.0f);
        colors[ImGuiCol_NavWindowingHighlight] =
            ImVec4(pri.r, pri.g, pri.b, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] =
            ImVec4(pri.r * 0.8f, pri.g * 0.8f, pri.b * 0.8f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] =
            ImVec4(pri.r * 0.8f, pri.g * 0.8f, pri.b * 0.8f, 0.35f);
    }
}


/**
 * @brief Zooms to the specified level, keeping the mouse cursor in
 * the same spot.
 *
 * @param newZoom New zoom level.
 */
void Editor::zoomWithCursor(float newZoom) {
    //Keep a backup of the old mouse coordinates.
    Point oldMousePos = game.editorsView.mouseCursorWorldPos;
    
    //Do the zoom.
    game.editorsView.cam.setZoom(
        std::clamp(newZoom, zoomMinLevel, zoomMaxLevel)
    );
    game.editorsView.updateTransformations();
    
    //Figure out where the mouse will be after the zoom.
    game.editorsView.mouseCursorWorldPos = game.mouseCursor.winPos;
    al_transform_coordinates(
        &game.editorsView.windowToWorldTransform,
        &game.editorsView.mouseCursorWorldPos.x,
        &game.editorsView.mouseCursorWorldPos.y
    );
    
    //Readjust the transformation by shifting the camera
    //so that the mouse cursor ends up where it was before.
    game.editorsView.cam.setPos(
        Point(
            game.editorsView.cam.pos.x +=
                (oldMousePos.x - game.editorsView.mouseCursorWorldPos.x),
            game.editorsView.cam.pos.y +=
                (oldMousePos.y - game.editorsView.mouseCursorWorldPos.y)
        )
    );
    
    //Update the mouse coordinates again.
    game.editorsView.updateTransformations();
    game.editorsView.mouseCursorWorldPos = game.mouseCursor.winPos;
    al_transform_coordinates(
        &game.editorsView.windowToWorldTransform,
        &game.editorsView.mouseCursorWorldPos.x,
        &game.editorsView.mouseCursorWorldPos.y
    );
}


/**
 * @brief Constructs a new changes manager object.
 *
 * @param ed Pointer to the editor.
 */
Editor::ChangesManager::ChangesManager(Editor* ed) :
    ed(ed) {
    
}


/**
 * @brief If there are no unsaved changes, performs a given action.
 * Otherwise, it opens a dialog asking the user if they
 * want to cancel, save and then do the action, or do the action without saving.
 *
 * @param pos Window coordinates to show the warning on.
 * If 0,0, then these will be set to the last processed widget's position.
 * @param actionLong String representing the action the user is attempting
 * in a long format. This is for the main prompt of the warning dialog,
 * so it can be as long as you want. It should start with a lowercase.
 * @param actionShort String representing the action the user is attempting
 * in a short format. This is for the buttons of the warning dialog,
 * so it should ideally be only one word. It should start with a lowercase.
 * @param actionCallback Code to run to perform the action.
 * @param saveCallback Code to run when the unsaved changes must be saved.
 * @return Whether there were unsaved changes.
 */
bool Editor::ChangesManager::askIfUnsaved(
    const Point& pos,
    const string& actionLong, const string& actionShort,
    const std::function<void()>& actionCallback,
    const std::function<bool()>& saveCallback
) {
    if(unsavedChanges > 0) {
        unsavedWarningActionLong = actionLong;
        unsavedWarningActionShort = actionShort;
        unsavedWarningActionCallback = actionCallback;
        unsavedWarningSaveCallback = saveCallback;
        
        ed->openDialog(
            "Unsaved changes!",
            std::bind(&Editor::processGuiDialogUnsavedChanges, ed)
        );
        ed->dialogs.back()->customPos = game.mouseCursor.winPos;
        ed->dialogs.back()->customSize = Point(580, 0);
        ed->dialogs.back()->eventCallback =
        [this] (const ALLEGRO_EVENT * ev) {
            if(ev->type == ALLEGRO_EVENT_KEY_DOWN) {
                if(
                    ed->keyCheck(ev->keyboard.keycode, ALLEGRO_KEY_S, true)
                ) {
                    ed->closeTopDialog();
                    const std::function<bool()>& saveCallback =
                        this->getUnsavedWarningSaveCallback();
                    const std::function<void()>& actionCallback =
                        this->getUnsavedWarningActionCallback();
                    if(saveCallback()) {
                        actionCallback();
                    }
                } else if(
                    ed->keyCheck(ev->keyboard.keycode, ALLEGRO_KEY_D, true)
                ) {
                    ed->closeTopDialog();
                    const std::function<void()>& actionCallback =
                        this->getUnsavedWarningActionCallback();
                    actionCallback();
                }
            }
        };
        
        return true;
        
    } else {
    
        actionCallback();
        
        return false;
    }
}


/**
 * @brief Returns whether the content exists in the disk.
 *
 * @return Whether it exists.
 */
bool Editor::ChangesManager::existsOnDisk() const {
    return inDisk;
}


/**
 * @brief Returns how many unsaved changes have been made so far since the
 * last save.
 *
 * @return The amount.
 */
size_t Editor::ChangesManager::getUnsavedChanges() const {
    return unsavedChanges;
}


/**
 * @brief Returns how long ago was the last time the player went from saved
 * to unsaved, in seconds.
 *
 * @return The time, or 0 if there are no unsaved changes.
 */
float Editor::ChangesManager::getUnsavedTimeDelta() const {
    if(unsavedChanges == 0) return 0.0f;
    return game.timePassed - unsavedTime;
}


/**
 * @brief Returns the current unsaved changes warning action callback.
 *
 * @return The callback.
 */
const std::function<void()>&
Editor::ChangesManager::getUnsavedWarningActionCallback() const {
    return unsavedWarningActionCallback;
}


/**
 * @brief Returns the current unsaved changes warning long action text.
 *
 * @return The text.
 */
const string& Editor::ChangesManager::getUnsavedWarningActionLong()
const {
    return unsavedWarningActionLong;
}


/**
 * @brief Returns the current unsaved changes warning short action text.
 *
 * @return The text.
 */
const string& Editor::ChangesManager::getUnsavedWarningActionShort() const {
    return unsavedWarningActionShort;
}


/**
 * @brief Returns the current unsaved changes warning save callback.
 *
 * @return The callback.
 */
const std::function<bool()>&
Editor::ChangesManager::getUnsavedWarningSaveCallback() const {
    return unsavedWarningSaveCallback;
}


/**
 * @brief Returns whether there are unsaved changes or not.
 *
 * @return Whether there are unsaved changes.
 */
bool Editor::ChangesManager::hasUnsavedChanges() {
    return unsavedChanges != 0;
}


/**
 * @brief Marks that the user has made new changes, which have obviously not yet
 * been saved.
 */
void Editor::ChangesManager::markAsChanged() {
    if(unsavedChanges == 0) {
        unsavedChanges++;
        unsavedTime = game.timePassed;
    } else {
        unsavedChanges++;
    }
}


/**
 * @brief Marks the state of the editor's file as not existing in the disk yet.
 * This also marks it as having unsaved changes.
 */
void Editor::ChangesManager::markAsNonExistent() {
    inDisk = false;
    markAsChanged();
}


/**
 * @brief Marks the state of the editor's file as saved.
 * The unsaved changes warning dialog does not set this, so this should be
 * called manually in those cases.
 */
void Editor::ChangesManager::markAsSaved() {
    unsavedChanges = 0;
    unsavedTime = 0.0f;
    inDisk = true;
}


/**
 * @brief Resets the state of the changes manager.
 */
void Editor::ChangesManager::reset() {
    unsavedChanges = 0;
    unsavedTime = 0.0f;
    inDisk = true;
}


/**
 * @brief Constructs a new command object.
 *
 * @param f Function to run.
 * @param n Name.
 */
Editor::Command::Command(CommandFunc f, const string& n) :
    func(f),
    name(n) {
}


/**
 * @brief Runs the function.
 *
 * @param inputValue Input value, if needed by the command.
 */
void Editor::Command::run(float inputValue) {
    func(inputValue);
}


/**
 * @brief Processes the dialog for this frame.
 */
void Editor::Dialog::process() {
    if(!isOpen) return;
    
    Point size = customSize;
    if(customSize.x == -1.0f && customSize.y == -1.0f) {
        size.x = game.winW * 0.8;
        size.y = game.winH * 0.8;
    }
    Point pos = customPos;
    if(customPos.x == -1.0f && customPos.y == -1.0f) {
        pos.x = game.winW / 2.0f;
        pos.y = game.winH / 2.0f;
    }
    Point tl, br;
    centerAndSizeToCorners(pos, size, &tl, &br);
    if(tl.x < 0.0f) {
        pos.x -= tl.x;
    }
    if(br.x > game.winW) {
        pos.x -= br.x - game.winW;
    }
    if(tl.y < 0.0f) {
        pos.y -= tl.y;
    }
    if(br.y > game.winH) {
        pos.y -= br.y - game.winH;
    }
    ImGui::SetNextWindowPos(
        ImVec2(pos.x, pos.y),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_Once);
    ImGui::OpenPopup((title + "##dialog").c_str());
    
    if(
        ImGui::BeginPopupModal(
            (title + "##dialog").c_str(), &isOpen
        )
    ) {
    
        processCallback();
        
        ImGui::EndPopup();
    }
}


/**
 * @brief Constructs a new picker info object.
 *
 * @param editorPtr Pointer to the editor in charge.
 */
Editor::Picker::Picker(Editor* editorPtr) :
    editorPtr(editorPtr) {
}


/**
 * @brief Processes the picker for this frame.
 */
void Editor::Picker::process() {
    vector<string> topCatNames;
    vector<vector<string> > secCatNames;
    vector<vector<vector<PickerItem> > > finalItems;
    string filterLower = strToLower(filter);
    
    //Figure out the items.
    forIdx(i, items) {
        if(!filter.empty()) {
            string nameLower = strToLower(items[i].name);
            if(nameLower.find(filterLower) == string::npos) {
                continue;
            }
        }
        
        size_t topCatIdx = INVALID;
        forIdx(c, topCatNames) {
            if(topCatNames[c] == items[i].topCategory) {
                topCatIdx = c;
                break;
            }
        }
        
        if(topCatIdx == INVALID) {
            topCatNames.push_back(items[i].topCategory);
            secCatNames.push_back(vector<string>());
            finalItems.push_back(vector<vector<PickerItem> >());
            topCatIdx = topCatNames.size() - 1;
        }
        
        size_t secCatIdx = INVALID;
        forIdx(c, secCatNames[topCatIdx]) {
            if(secCatNames[topCatIdx][c] == items[i].secCategory) {
                secCatIdx = c;
                break;
            }
        }
        
        if(secCatIdx == INVALID) {
            secCatNames[topCatIdx].push_back(items[i].secCategory);
            finalItems[topCatIdx].push_back(vector<PickerItem>());
            secCatIdx = secCatNames[topCatIdx].size() - 1;
        }
        
        finalItems[topCatIdx][secCatIdx].push_back(items[i]);
    }
    
    //Stuff for creating a new item.
    auto tryMakeNew = [this] () {
        if(filter.empty()) return;
        
        if(
            !newItemTopCatChoices.empty() &&
            newItemTopCat.empty()
        ) {
            //The user has to pick a category, but hasn't picked yet.
            //Let's show the pop-up and leave.
            ImGui::OpenPopup("newItemCategory");
            return;
        }
        
        bool isReallyNew = true;
        forIdx(i, items) {
            if(
                filter == items[i].name &&
                newItemTopCat == items[i].topCategory
            ) {
                isReallyNew = false;
                break;
            }
        }
        
        pickCallback(filter, newItemTopCat, "", nullptr, isReallyNew);
        if(dialogPtr) {
            dialogPtr->isOpen = false;
        }
    };
    
    if(canMakeNew) {
        //"New" button.
        ImGui::PushStyleColor(
            ImGuiCol_Button, (ImVec4) ImColor(192, 32, 32)
        );
        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered, (ImVec4) ImColor(208, 48, 48)
        );
        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive, (ImVec4) ImColor(208, 32, 32)
        );
        bool hitCreateButton = ImGui::Button("+", ImVec2(64.0f, 0.0f));
        editorPtr->setTooltip("Create a new item with the given name!");
        if(hitCreateButton) tryMakeNew();
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
    }
    
    //Search filter input.
    string filterWidgetHint =
        canMakeNew ?
        "Search filter or new item name" :
        "Search filter";
        
    ImGui::FocusOnInputText(needsFilterBoxFocus);
    bool hitFilterWidget = false;
    if(filter.empty()) {
        hitFilterWidget =
            ImGui::InputTextWithHint(
                "##filter", filterWidgetHint.c_str(), &filter,
                ImGuiInputTextFlags_EnterReturnsTrue
            );
    } else {
        hitFilterWidget =
            monoInputTextWithHint(
                "##filter", filterWidgetHint.c_str(), &filter,
                ImGuiInputTextFlags_EnterReturnsTrue
            );
    }
    
    if(hitFilterWidget) {
        if(filter.empty()) return;
        
        if(canMakeNew) {
            tryMakeNew();
        } else {
            size_t possibleChoices = 0;
            forIdx(c, finalItems) {
                possibleChoices += finalItems[c].size();
            }
            if(possibleChoices > 0) {
                pickCallback(
                    finalItems[0][0][0].name,
                    finalItems[0][0][0].topCategory,
                    finalItems[0][0][0].secCategory,
                    finalItems[0][0][0].info,
                    false
                );
                if(dialogPtr) {
                    dialogPtr->isOpen = false;
                }
            }
        }
    }
    
    //New item category pop-up.
    if(editorPtr->popup("newItemCategory")) {
        ImGui::Text("%s", "What is the category of the new item?");
        
        if(
            ImGui::BeginChild(
                "categoryList", ImVec2(0.0f, 80.0f), ImGuiChildFlags_Borders
            )
        ) {
            forIdx(c, newItemTopCatChoices) {
                //Item selectable.
                if(ImGui::Selectable(newItemTopCatChoices[c].c_str())) {
                    newItemTopCat = newItemTopCatChoices[c];
                    ImGui::CloseCurrentPopup();
                    tryMakeNew();
                }
            }
            ImGui::EndChild();
        }
        
        //Cancel button.
        if(ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    //List header text.
    if(!listHeader.empty()) {
        ImGui::Text("%s", listHeader.c_str());
    }
    
    //Item list.
    ImGui::BeginChild("list");
    
    forIdx(tc, finalItems) {
    
        bool topCatOpened = true;
        if(!topCatNames[tc].empty()) {
            //Top category node.
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            topCatOpened = ImGui::TreeNode(topCatNames[tc].c_str());
        }
        
        if(!topCatOpened) continue;
        
        forIdx(sc, finalItems[tc]) {
        
            bool secCatOpened = true;
            if(!secCatNames[tc][sc].empty()) {
                //Secondary category node.
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                secCatOpened = ImGui::TreeNode(secCatNames[tc][sc].c_str());
            }
            
            if(!secCatOpened) continue;
            
            forIdx(i, finalItems[tc][sc]) {
                PickerItem* iPtr = &finalItems[tc][sc][i];
                string widgetId = i2s(tc) + "-" + i2s(sc) + "-" + i2s(i);
                ImGui::PushID(widgetId.c_str());
                if(useMonospace) {
                    ImGui::PushFont(
                        game.sysContent.fntDearImGuiMonospace,
                        game.sysContent.fntDearImGuiMonospace->LegacySize
                    );
                }
                
                Point buttonSize;
                
                if(iPtr->bitmap) {
                
                    ImGui::BeginGroup();
                    
                    //Item image button.
                    buttonSize = Point(EDITOR::PICKER_IMG_BUTTON_SIZE);
                    bool buttonPressed =
                        ImGui::ImageButtonOrganized(
                            widgetId + "Button",
                            iPtr->bitmap,
                            buttonSize - 4.0f, buttonSize
                        );
                        
                    if(buttonPressed) {
                        pickCallback(
                            iPtr->name, iPtr->topCategory,
                            iPtr->secCategory, iPtr->info, false
                        );
                        if(dialogPtr) {
                            dialogPtr->isOpen = false;
                        }
                    }
                    
                    //Item name text.
                    string displayName =
                        trimWithEllipsis(
                            getPathLastComponent(iPtr->name), 18
                        );
                    ImGui::TextWrapped("%s", displayName.c_str());
                    
                    //Item spacer widget.
                    ImGui::Dummy(ImVec2(0.0f, 8.0f));
                    ImGui::EndGroup();
                    
                } else {
                
                    //Item button.
                    buttonSize = Point(EDITOR::PICKER_IMG_BUTTON_SIZE, 32.0f);
                    if(
                        ImGui::Button(
                            iPtr->name.c_str(),
                            ImVec2(buttonSize.x, buttonSize.y)
                        )
                    ) {
                        pickCallback(
                            iPtr->name, iPtr->topCategory,
                            iPtr->secCategory, iPtr->info, false
                        );
                        if(dialogPtr) {
                            dialogPtr->isOpen = false;
                        }
                    }
                    
                }
                
                if(!iPtr->tooltip.empty()) {
                    editorPtr->setTooltip(iPtr->tooltip);
                }
                
                ImGui::SetupButtonWrapping(
                    buttonSize.x, (int) (i + 1), (int) finalItems[tc][sc].size()
                );
                
                if(useMonospace) ImGui::PopFont();
                ImGui::PopID();
            }
            
            if(!secCatNames[tc][sc].empty()) {
                ImGui::TreePop();
            }
        }
        
        if(!topCatNames[tc].empty()) {
            ImGui::TreePop();
        }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Constructs a new picker item object.
 *
 * @param name Name of the item.
 * @param topCategory Top-level category it belongs to.
 * If none, use an empty string.
 * @param secCategory Second-level category it belongs to.
 * If none, use an empty string.
 * @param info Information to pass to the code when the item is picked, if any.
 * @param tooltip Tooltip, if any.
 * @param bitmap Bitmap to display on the item. If none, use nullptr.
 */
Editor::PickerItem::PickerItem(
    const string& name, const string& topCategory, const string& secCategory,
    void* info, const string& tooltip, ALLEGRO_BITMAP* bitmap
) :
    name(name),
    topCategory(topCategory),
    secCategory(secCategory),
    info(info),
    tooltip(tooltip),
    bitmap(bitmap) {
    
}


/**
 * @brief Adds an item to the selection.
 *
 * @param idx The item's index.
 * @return Whether the item was unselected.
 */
bool Editor::SelectionList::add(size_t idx) {
    if(list.contains(idx)) return false;
    list.insert(idx);
    return true;
}


/**
 * @brief Selects all items available.
 *
 * @param totalAmount How many items there are in total.
 * @return Whether we didn't already have all selected.
 */
bool Editor::SelectionList::addAll(size_t totalAmount) {
    size_t prevSelSize = list.size();
    clear();
    for(size_t i = 0; i < totalAmount; i++) {
        add(i);
    }
    return list.size() > prevSelSize;
}


/**
 * @brief Clears the selection.
 *
 * @return Whether there were items to clear.
 */
bool Editor::SelectionList::clear() {
    if(list.empty()) return false;
    list.clear();
    return true;
}


/**
 * @brief Returns whether a given items is selected.
 *
 * @param idx The item's index.
 * @return Whether it is selected.
 */
bool Editor::SelectionList::contains(size_t idx) const {
    return list.contains(idx);
}


/**
 * @brief Returns the index of the first selected item, or INVALID if
 * none is selected.
 *
 * @return The index or INVALID.
 */
size_t Editor::SelectionList::getFirstItemIdx() const {
    if(list.size() == 0) {
        return INVALID;
    }
    return *list.begin();
}


/**
 * @brief Returns the list of all selected items.
 *
 * @return The list.
 */
const set<size_t>& Editor::SelectionList::getItemIdxs() const {
    return list;
}


/**
 * @brief Returns how many items are selected.
 *
 * @return The amount.
 */
size_t Editor::SelectionList::getCount() const {
    return list.size();
}


/**
 * @brief Returns the index of the only selected item, or INVALID if
 * multiple or none are selected.
 *
 * @return The index or INVALID.
 */
size_t Editor::SelectionList::getSingleItemIdx() const {
    if(list.size() == 1) {
        return *list.begin();
    } else {
        return INVALID;
    }
}


/**
 * @brief Returns whether any items are selected.
 *
 * @return Whether any are selected.
 */
bool Editor::SelectionList::hasAny() const {
    return !list.empty();
}


/**
 * @brief Returns whether there is are multiple items selected.
 *
 * @return Whether there are multiple selected.
 */
bool Editor::SelectionList::hasMultiple() const {
    return list.size() > 1;
}


/**
 * @brief Returns whether there is only one item selected.
 *
 * @return Whether there is one selected.
 */
bool Editor::SelectionList::hasOne() const {
    return list.size() == 1;
}


/**
 * @brief Removes an item from the selection.
 *
 * @param idx The item's index.
 * @return Whether the item was selected.
 */
bool Editor::SelectionList::remove(size_t idx) {
    if(!list.contains(idx)) return false;
    list.erase(idx);
    return true;
}


/**
 * @brief Sets the selection to be a single item only.
 *
 * @param idx The item to select.
 * @return Whether that item wasn't already selected.
 */
bool Editor::SelectionList::setSingle(size_t idx) {
    if(getSingleItemIdx() == idx) return false;
    clear();
    add(idx);
    return true;
}


/**
 * @brief If drag-moving, updates the items' position.
 *
 * @param offset How much to move the items by, compared to the start
 * of the operation.
 * @return Whether it was drag-moving before this.
 */
bool Editor::SelectionManager::applyDragMove(const Point& offset) {
    if(!enabled) return false;
    if(!onGetInfo || !onSetInfo) return false;
    
    const set<size_t>& list = selectedItems.getItemIdxs();
    for(size_t i : list) {
        Point origPos = preOpItemCenters[i];
        Point iCenter, iSize;
        onGetInfo(i, &iCenter, &iSize);
        onSetInfo(i, origPos + offset, iSize);
    }
    
    return true;
}


/**
 * @brief Applies a transformation the user performed on the geometry of
 * the selected items.
 *
 * @param newCenter The new selection center.
 * @param newSize The new selection size.
 * @return Whether it was able to apply.
 */
bool Editor::SelectionController::applyTransformation(
    const Point& newCenter, const Point& newSize
) {
    if(!enabled) return false;
    if(state != STATE_TW_TRANSFORMING) return false;
    
    Point preTransTL;
    centerAndSizeToCorners(preOpSelCenter, preOpSelSize, &preTransTL, nullptr);
    
    forIdx(m, managers) {
        Point mNewCenter, mNewSize;
        managers[m]->calculateSelectionPortion(
            preOpSelCenter, preOpSelSize,
            newCenter, newSize,
            &mNewCenter, &mNewSize
        );
        managers[m]->applyTransformation(mNewCenter, mNewSize);
    }
    
    return true;
}


/**
 * @brief Selects an item, or items, based on the left mouse button being
 * pressed down. This also starts a rubber band selection box if applicable.
 *
 * @param cursorPos Mouse cursor position.
 * @param rubberBandMod Whether a modifier key that forces rubber band
 * selection box creation was held down.
 * @param addToSelectionMod Whether a modifier key that adds to the
 * selection was held down.
 * @return Whether anything changed.
 */
bool Editor::SelectionController::chooseViaMouseDown(
    const Point& cursorPos, bool rubberBandMod, bool addToSelectionMod
) {
    if(!enabled) return false;
    
    //Get which items are selected and which are under the mouse cursor.
    vector<std::pair<size_t, size_t> > clickedItems;
    std::pair<size_t, size_t> singleSelectedItem = { INVALID, INVALID };
    size_t totalNrSelectedItems = 0;
    size_t totalNrClickedItems = 0;
    forIdx(m, managers) {
        size_t mgrCount = managers[m]->getCount();
        totalNrSelectedItems += mgrCount;
        if(mgrCount == 1) {
            singleSelectedItem =
                std::make_pair(m, managers[m]->getSingleItemIdx());
        }
        
        const vector<size_t>& itemsUnderCursor =
            managers[m]->getItemsUnderCursor(cursorPos);
        forIdx(i, itemsUnderCursor) {
            clickedItems.push_back(std::make_pair(m, itemsUnderCursor[i]));
        }
        totalNrClickedItems += itemsUnderCursor.size();
    }
    
    //Check if we must start a rubber band.
    bool mustStartRubberBand = (totalNrClickedItems == 0) || rubberBandMod;
    if(mustStartRubberBand) {
        if(!addToSelectionMod) {
            forIdx(m, managers) {
                managers[m]->clear();
            }
        }
        startRubberBand(cursorPos);
        return true;
    }
    
    //Figure out which of the clicked items to focus on.
    std::pair<size_t, size_t> finalItem = { INVALID, INVALID };
    if(overlapsCycle && totalNrSelectedItems == 1 && totalNrClickedItems >= 2) {
        finalItem = getNextInVector(clickedItems, singleSelectedItem);
    } else if(totalNrClickedItems > 0) {
        finalItem = clickedItems[0];
    }
    
    //Unselect others, if applicable.
    bool isFinalItemSelected = false;
    if(finalItem.first != INVALID) {
        isFinalItemSelected =
            managers[finalItem.first]->contains(finalItem.second);
    }
    bool keepOldSelection = false;
    if(addToSelectionMod) {
        keepOldSelection = true;
    }
    if(isFinalItemSelected && !clickingSelectedUnselectsOthers) {
        keepOldSelection = true;
    }
    if(!keepOldSelection) {
        forIdx(m, managers) {
            managers[m]->clear();
        }
    }
    
    //Select it.
    if(finalItem.first != INVALID) {
        managers[finalItem.first]->add(finalItem.second);
    }
    
    //Check if we can start drag moving.
    bool canStartDragMove = isOpRuleRespected(dragMoveRule);
    if(totalNrClickedItems > 0 && canStartDragMove) {
        startDragMove(cursorPos);
        return true;
    }
    
    return true;
}


/**
 * @brief Disables the controller.
 *
 * @return Whether it was enabled.
 */
bool Editor::SelectionController::disable() {
    bool wasEnabled = enabled;
    enabled = false;
    return wasEnabled;
}


/**
 * @brief Draws things related to the selection.
 *
 * @param cursorPos Position of the mouse cursor.
 * @param zoom Zoom level to use.
 */
void Editor::SelectionController::draw(
    const Point& cursorPos, float zoom
) const {
    if(!enabled) return;
    if(state == STATE_RUBBER_BAND) {
        //Setup.
        Point rBTL(
            std::min(opStartCursorPos.x, cursorPos.x),
            std::min(opStartCursorPos.y, cursorPos.y)
        );
        Point rBBR(
            std::max(opStartCursorPos.x, cursorPos.x),
            std::max(opStartCursorPos.y, cursorPos.y)
        );
        
        //Interior.
        al_draw_filled_rectangle(
            rBTL.x, rBTL.y, rBBR.x, rBBR.y,
            multAlpha(AREA_EDITOR::SELECTION_COLOR, 0.20f)
        );
        
        //Marching ants outline.
        ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[4];
        const float smallestRSide = std::min(rBBR.x - rBTL.x, rBBR.y - rBTL.y);
        const float thickness =
            std::min(
                smallestRSide,
                4.0f / zoom
            );
        const float textureSize =
            al_get_bitmap_width(game.sysContent.bmpRubberBandSel);
        const float timeOffset =
            game.timePassed * EDITOR::RUBBER_BAND_TEXTURE_TIME_MULT;
            
        for(size_t v = 0; v < 4; v++) {
            av[v].z = 0;
            av[v].color = AREA_EDITOR::SELECTION_COLOR;
        }
        
        enum DIR {
            DIR_RIGHT,
            DIR_DOWN,
            DIR_LEFT,
            DIR_UP,
        };
        
        const auto drawRBLine =
            [&av, &textureSize, &thickness, &timeOffset, zoom]
        (const Point & p1, const Point & p2, DIR dir) {
            float dist = 0.0f;
            float rightTurnX = 0.0f;
            float rightTurnY = 0.0f;
            switch(dir) {
            case DIR_RIGHT: {
                dist = p2.x - p1.x;
                rightTurnY = thickness / 2.0f;
                break;
            } case DIR_DOWN: {
                dist = p2.y - p1.y;
                rightTurnX = -thickness / 2.0f;
                break;
            } case DIR_LEFT: {
                dist = p1.x - p2.x;
                rightTurnY = -thickness / 2.0f;
                break;
            } case DIR_UP: {
                dist = p1.y - p2.y;
                rightTurnX = thickness / 2.0f;
                break;
            }
            }
            
            av[0].x = p1.x - rightTurnX;
            av[0].y = p1.y - rightTurnY;
            av[0].u = 0.0f - timeOffset;
            av[0].v = dist * zoom;
            
            av[1].x = p1.x + rightTurnX;
            av[1].y = p1.y + rightTurnY;
            av[1].u = 0.0f - timeOffset;
            av[1].v = dist * zoom;
            
            av[2].x = p2.x - rightTurnX;
            av[2].y = p2.y - rightTurnY;
            av[2].u = dist * zoom - timeOffset;
            av[2].v = thickness;
            
            av[3].x = p2.x + rightTurnX;
            av[3].y = p2.y + rightTurnY;
            av[3].u = dist * zoom - timeOffset;
            av[3].v = -thickness;
            
            al_draw_prim(
                av, nullptr,
                game.sysContent.bmpRubberBandSel,
                0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP
            );
        };
        
        //Top line (shifted left).
        drawRBLine(
            Point(rBTL.x, rBTL.y + thickness / 2.0f),
            Point(rBBR.x - thickness, rBTL.y + thickness / 2.0f),
            DIR_RIGHT
        );
        
        //Right line (shifted up).
        drawRBLine(
            Point(rBBR.x - thickness / 2.0f, rBTL.y),
            Point(rBBR.x - thickness / 2.0f, rBBR.y - thickness),
            DIR_DOWN
        );
        
        //Bottom line (shifted right).
        drawRBLine(
            Point(rBBR.x, rBBR.y - thickness / 2.0f),
            Point(rBTL.x + thickness, rBBR.y - thickness / 2.0f),
            DIR_LEFT
        );
        
        //Left line (shifted down).
        drawRBLine(
            Point(rBTL.x + thickness / 2.0f, rBBR.y),
            Point(rBTL.x + thickness / 2.0f, rBTL.y + thickness),
            DIR_UP
        );
    }
}


/**
 * @brief Enables the controller.
 *
 * @return Whether it was disabled.
 */
bool Editor::SelectionController::enable() {
    bool wasDisabled = !enabled;
    enabled = true;
    return wasDisabled;
}


/**
 * @brief Returns the total bounding box of every manager's selected items.
 *
 * @param outCenter The center is returned here.
 * @param outSize The size is returned here.
 * @param outCentersOnlyCenter If not nullptr, the center of the box that
 * delimits the centers only is returned here.
 * @param outCentersOnlySize If not nullptr, the size of the box that delimits
 * the centers only is returned here.
 * @return Whether there are any selected items.
 */
bool Editor::SelectionController::getTotalBBox(
    Point* outCenter, Point* outSize,
    Point* outCentersOnlyCenter, Point* outCentersOnlySize
) const {
    Point totalCenter, totalSize;
    Point centersOnlyTotalCenter, centersOnlyTotalSize;
    bool hasFirst = false;
    
    forIdx(m, managers) {
        Point mCenter, mSize;
        Point mCOCenter, mCOSize;
        managers[m]->getBBox(&mCenter, &mSize, &mCOCenter, &mCOSize);
        if(mSize.x == 0.0f) continue;
        
        if(!hasFirst) {
            totalCenter = mCenter;
            totalSize = mSize;
            centersOnlyTotalCenter = mCOCenter;
            centersOnlyTotalSize = mCOSize;
            hasFirst = true;
        } else {
            combineBBoxes(
                totalCenter, totalSize,
                mCenter, mSize,
                &totalCenter, &totalSize
            );
            combineBBoxes(
                centersOnlyTotalCenter, centersOnlyTotalSize,
                mCOCenter, mCOSize,
                &centersOnlyTotalCenter, &centersOnlyTotalSize
            );
        }
    }
    
    *outCenter = totalCenter;
    *outSize = totalSize;
    if(outCentersOnlyCenter) *outCentersOnlyCenter = centersOnlyTotalCenter;
    if(outCentersOnlySize) *outCentersOnlySize = centersOnlyTotalSize;
    
    return false;
}

/**
 * @brief Returns the total number of selected items.
 *
 * @return The total.
 */
size_t Editor::SelectionController::getSelectionTotalCount() const {
    size_t total = 0;
    forIdx(m, managers) {
        total += managers[m]->getCount();
    }
    return total;
}


/**
 * @brief A shorthand for handling things to do when the left mouse
 * is released.
 *
 * @return Whether it succeeded.
 */
bool Editor::SelectionController::handleMouseUp() {
    bool success = stopRubberBand();
    success |= stopTransforming();
    success |= stopDragMove();
    return success;
}


/**
 * @brief Returns whether the user is currently moving the selected items
 * by dragging one of them.
 *
 * @return Whether it is drag moving.
 */
bool Editor::SelectionController::isDragMoving() const {
    return state == STATE_DRAG_MOVING;
}


/**
 * @brief Returns whether the given operation rule is currently respected.
 *
 * @param rule The rule
 * @return Whether it's respected.
 */
bool Editor::SelectionController::isOpRuleRespected(OP_RULE rule) const {
    switch(rule) {
    case OP_RULE_ALWAYS: {
        break;
    } case OP_RULE_ONE_ITEM: {
        if(getSelectionTotalCount() != 1) return false;
        break;
    } case OP_RULE_MULTIPLE_ITEMS: {
        if(getSelectionTotalCount() <= 1) return false;
        break;
    } case OP_RULE_NEVER: {
        return false;
        break;
    }
    }
    
    return true;
}


/**
 * @brief Returns whether the transformation widget is available right now.
 *
 * @return Whether it is available.
 */
bool Editor::SelectionController::isTransformationWidgetAvailable() const {
    return getSelectionTotalCount() && isOpRuleRespected(twTransformRule);
}


/**
 * @brief Returns whether the user is currently transforming the selection.
 *
 * @return Whether it is transforming.
 */
bool Editor::SelectionController::isTransforming() const {
    return state == STATE_TW_TRANSFORMING;
}


/**
 * @brief Returns whether the user is currently creating a rubber band.
 *
 * @return Whether it is creating.
 */
bool Editor::SelectionController::isCreatingRubberBand() const {
    return state == STATE_RUBBER_BAND;
}


/**
 * @brief Starts the creation of a rubber band selection box.
 *
 * @param cursorPos Position of the cursor.
 * @return Whether it was idling before this.
 */
bool Editor::SelectionController::startRubberBand(const Point& cursorPos) {
    if(!enabled) return false;
    bool wasIdle = state == STATE_IDLING;
    state = STATE_RUBBER_BAND;
    opStartCursorPos = cursorPos;
    return wasIdle;
}


/**
 * @brief Starts moving the selected items via dragging one of them.
 *
 * @param cursorPos Position of the cursor.
 * @return Whether it was idling before this.
 */
bool Editor::SelectionController::startDragMove(const Point& cursorPos) {
    if(!enabled) return false;
    bool wasIdle = state == STATE_IDLING;
    state = STATE_DRAG_MOVING;
    opStartCursorPos = cursorPos;
    
    bool gotClosest = false;
    Distance closestDist;
    
    forIdx(m, managers) {
        managers[m]->startOperation();
        const set<size_t>& list = managers[m]->getItemIdxs();
        
        for(size_t i : list) {
            Point iCenter, iSize;
            managers[m]->getItemInfo(i, &iCenter, &iSize);
            Distance d(game.editorsView.mouseCursorWorldPos, iCenter);
            if(!gotClosest || d < closestDist) {
                gotClosest = true;
                closestDist = d;
                preOpPivotItemPos = iCenter;
            }
        }
    }
    
    return wasIdle;
}


/**
 * @brief Starts a transformation of the selection bounding box.
 *
 * @return Whether it was idling before this.
 */
bool Editor::SelectionController::startTransforming() {
    if(!enabled) return false;
    bool wasIdle = state == STATE_IDLING;
    state = STATE_TW_TRANSFORMING;
    
    getTotalBBox(
        &preOpSelCenter, &preOpSelSize,
        &preOpCentersOnlySelCenter, &preOpCentersOnlySelSize
    );
    
    forIdx(m, managers) {
        managers[m]->startOperation();
    }
    
    return wasIdle;
}


/**
 * @brief Finishes moving items via dragging one of them.
 *
 * @return Whether it was in drag-move mode before this.
 */
bool Editor::SelectionController::stopDragMove() {
    if(state != STATE_DRAG_MOVING) return false;
    
    state = STATE_IDLING;
    return true;
}


/**
 * @brief Finishes the creation of a rubber band selection box.
 *
 * @return Whether it was creating a rubber band selection box before this.
 */
bool Editor::SelectionController::stopRubberBand() {
    if(state != STATE_RUBBER_BAND) return false;
    
    state = STATE_IDLING;
    return true;
}


/**
 * @brief Stops a transformation of the selection bounding box.
 *
 * @return Whether it was in a transformation before this.
 */
bool Editor::SelectionController::stopTransforming() {
    if(state != STATE_TW_TRANSFORMING) return false;
    state = STATE_IDLING;
    return true;
}


/**
 * @brief If drag-moving, updates the mouse cursor's position.
 *
 * @param cursorPos Mouse cursor position.
 * @return Whether it was drag-moving before this.
 */
bool Editor::SelectionController::updateDragMove(const Point& cursorPos) {
    if(!enabled) return false;
    if(state != STATE_DRAG_MOVING) return false;
    
    Point mouseOffset = cursorPos - opStartCursorPos;
    Point newPivotPos = preOpPivotItemPos + mouseOffset;
    if(onSnapPoint) newPivotPos = onSnapPoint(newPivotPos);
    Point totalMoveOffset = newPivotPos - preOpPivotItemPos;
    
    forIdx(m, managers) {
        managers[m]->applyDragMove(totalMoveOffset);
    }
    
    return true;
}


/**
 * @brief If creating a rubber band selection box, updates the cursor position.
 *
 * @param cursorPos New cursor position.
 * @param rubberBandMod Whether a modifier key that forces rubber band
 * selection box creation was held down.
 * @param addToSelectionMod Whether a modifier key that adds to the
 * selection was held down.
 * @return Whether it was creating a rubber band selection box before this.
 */
bool Editor::SelectionController::updateRubberBand(
    const Point& cursorPos, bool rubberBandMod, bool addToSelectionMod
) {
    if(!enabled) return false;
    if(state != STATE_RUBBER_BAND) return false;
    
    Point rubberBandTL =
        Point(
            std::min(opStartCursorPos.x, cursorPos.x),
            std::min(opStartCursorPos.y, cursorPos.y)
        );
    Point rubberBandBR =
        Point(
            std::max(opStartCursorPos.x, cursorPos.x),
            std::max(opStartCursorPos.y, cursorPos.y)
        );
        
    forIdx(m, managers) {
        if(!addToSelectionMod) {
            managers[m]->clear();
        }
        
        size_t nrTotalItems = managers[m]->getNrTotalItems();
        for(size_t i = 0; i < nrTotalItems; i++) {
            if(!managers[m]->getItemIsEligible(i)) continue;
            Point iCenter, iSize;
            managers[m]->getItemInfo(i, &iCenter, &iSize);
            Point iTL, iBR;
            centerAndSizeToCorners(iCenter, iSize, &iTL, &iBR);
            
            if(
                iTL.x >= rubberBandTL.x &&
                iTL.y >= rubberBandTL.y &&
                iBR.x <= rubberBandBR.x &&
                iBR.y <= rubberBandBR.y
            ) {
                managers[m]->add(i);
            }
        }
    }
    
    return true;
}


/**
 * @brief Adds an item to the selection.
 *
 * @param idx The item's index.
 * @return Whether the item was unselected.
 */
bool Editor::SelectionManager::add(size_t idx) {
    if(!enabled) return false;
    if(!selectedItems.add(idx)) return false;
    homogenized = false;
    return true;
}


/**
 * @brief Selects all items available.
 *
 * @param totalAmount How many items there are in total.
 * @return Whether we didn't already have all selected.
 */
bool Editor::SelectionManager::addAll(size_t totalAmount) {
    if(!enabled) return false;
    return selectedItems.addAll(totalAmount);
}


/**
 * @brief Applies a transformation the user performed on the geometry of
 * the selected items.
 *
 * @param newCenter The new selection center.
 * @param newSize The new selection size.
 * @return Whether it was able to apply.
 */
bool Editor::SelectionManager::applyTransformation(
    const Point& newCenter, const Point& newSize
) {
    if(!enabled) return false;
    if(preOpSelSize.x <= 0.0f || preOpSelSize.y <= 0.0f) return false;
    if(!onGetInfo || !onSetInfo) return false;
    
    Point preTransTL = preOpSelCenter - preOpSelSize / 2.0f;
    Point preTransCentersOnlyTL =
        preOpCentersOnlySelCenter - preOpCentersOnlySelSize / 2.0f;
        
    Point centersOnlyOffset = preTransCentersOnlyTL - preTransTL;
    Point newTL = newCenter - newSize / 2.0f;
    Point newCentersOnlyTL = newTL + centersOnlyOffset;
    Point newCentersOnlySize =
        newSize - (preOpSelSize - preOpCentersOnlySelSize);
    newCentersOnlySize.x = std::max(0.0f, newCentersOnlySize.x);
    newCentersOnlySize.y = std::max(0.0f, newCentersOnlySize.y);
    
    const set<size_t>& list = selectedItems.getItemIdxs();
    for(size_t i : list) {
        Point iCenter, iSize;
        onGetInfo(i, &iCenter, &iSize);
        
        if(itemsCanResize) {
            //Position and resize the item according to the new shape.
            Point preTransCenterRatio =
                (preOpItemCenters[i] - preTransTL) / preOpSelSize;
            Point preTransSizeRatio =
                preOpItemSizes[i] / preOpSelSize;
            iCenter = newTL + preTransCenterRatio * newSize;
            iSize = preTransSizeRatio * newSize;
            
        } else {
            if(list.size() == 1) {
                //If there's only one item and it can't be resized, just move
                //it. Pretty simple scenario.
                iCenter = newCenter;
            } else {
                //Position the item based on the "centers-only" bounding
                //box. Keep its size.
                if(preOpCentersOnlySelSize.x > 0.0f) {
                    float preTransCenterRatioX =
                        (preOpItemCenters[i].x - preTransCentersOnlyTL.x) /
                        preOpCentersOnlySelSize.x;
                    iCenter.x =
                        newCentersOnlyTL.x +
                        preTransCenterRatioX * newCentersOnlySize.x;
                } else {
                    iCenter.x = newCenter.x;
                }
                if(preOpCentersOnlySelSize.y > 0.0f) {
                    float preTransCenterRatioY =
                        (preOpItemCenters[i].y - preTransCentersOnlyTL.y) /
                        preOpCentersOnlySelSize.y;
                    iCenter.y =
                        newCentersOnlyTL.y +
                        preTransCenterRatioY * newCentersOnlySize.y;
                } else {
                    iCenter.y = newCenter.y;
                }
            }
        }
        
        onSetInfo(i, iCenter, iSize);
    }
    
    return true;
}


/**
 * @brief Clears the selection.
 *
 * @return Whether there were items to clear.
 */
bool Editor::SelectionManager::clear() {
    if(!enabled) return false;
    if(!selectedItems.clear()) return false;
    homogenized = false;
    return true;
}


/**
 * @brief Returns whether a given items is selected.
 *
 * @param idx The item's index.
 * @return Whether it is selected.
 */
bool Editor::SelectionManager::contains(size_t idx) const {
    return selectedItems.contains(idx);
}


/**
 * @brief Disables the manager.
 *
 * @return Whether it was enabled.
 */
bool Editor::SelectionManager::disable() {
    bool wasEnabled = enabled;
    enabled = false;
    selectedItems.clear();
    return wasEnabled;
}


/**
 * @brief Enables the manager.
 *
 * @return Whether it was disabled.
 */
bool Editor::SelectionManager::enable() {
    bool wasDisabled = !enabled;
    enabled = true;
    return wasDisabled;
}


/**
 * @brief Returns the center point and size of the bounding box of the
 * selected items.
 *
 * @param outCenter The center of the box is returned here.
 * @param outSize The dimensions of the box are returned here.
 * @param outCentersOnlyCenter If not nullptr, the center of the box that
 * delimits the centers only is returned here.
 * @param outCentersOnlySize If not nullptr, the size of the box that delimits
 * the centers only is returned here.
 * @return Whether there are any selected items.
 */
bool Editor::SelectionManager::getBBox(
    Point* outCenter, Point* outSize,
    Point* outCentersOnlyCenter, Point* outCentersOnlySize
) const {
    *outCenter = Point();
    *outSize = Point();
    if(!selectedItems.hasAny()) return false;
    
    Point minCoords(FLT_MAX);
    Point maxCoords(-FLT_MAX);
    Point centersOnlyMinCoords(FLT_MAX);
    Point centersOnlyMaxCoords(-FLT_MAX);
    const set<size_t>& list = selectedItems.getItemIdxs();
    
    for(size_t i : list) {
        Point iCenter, iSize;
        getItemInfo(i, &iCenter, &iSize);
        updateMinCoords(
            minCoords, iCenter - iSize / 2.0f
        );
        updateMaxCoords(
            maxCoords, iCenter + iSize / 2.0f
        );
        updateMinMaxCoords(
            centersOnlyMinCoords, centersOnlyMaxCoords, iCenter
        );
    }
    
    cornersToCenterAndSize(minCoords, maxCoords, outCenter, outSize);
    cornersToCenterAndSize(
        centersOnlyMinCoords, centersOnlyMaxCoords,
        outCentersOnlyCenter, outCentersOnlySize
    );
    
    return true;
}


/**
 * @brief Returns how many items are selected.
 *
 * @return The amount.
 */
size_t Editor::SelectionManager::getCount() const {
    return selectedItems.getCount();
}


/**
 * @brief If the manager's selection bounding box is part of a larger box,
 * and said larger box has a new center or size, this can be used to determine
 * which portion of the larger box's new dimensions apply to this manager,
 * such that proportions are kept.
 *
 * @param largerPreTransCenter The larger bounding box's pre-transformation
 * center.
 * @param largerPreTransSize The larger bounding box's pre-transformation
 * size.
 * @param largerNewCenter The larger bounding box's new center.
 * @param largerNewSize The larger bounding box's new size.
 * @param outPortionedNewCenter The manager's portion's new center.
 * @param outPortionedNewSize The manager's portion's new size.
 */
void Editor::SelectionManager::calculateSelectionPortion(
    const Point& largerPreTransCenter, const Point& largerPreTransSize,
    const Point& largerNewCenter, const Point& largerNewSize,
    Point* outPortionedNewCenter, Point* outPortionedNewSize
) const {
    //The size is pretty easy.
    Point sizeRatio = preOpSelSize / largerPreTransSize;
    *outPortionedNewSize = largerNewSize * sizeRatio;
    
    //For the center, let's use corners instead.
    Point posRatio =
        getPointPosRatioInRectangle(
            preOpSelCenter, largerPreTransCenter, largerPreTransSize
        );
    Point largerNewTL;
    centerAndSizeToCorners(
        largerNewCenter, largerNewSize, &largerNewTL, nullptr
    );
    *outPortionedNewCenter = largerNewTL + largerNewSize * posRatio;
}


/**
 * @brief Returns which items are under the mouse cursor.
 *
 * @param cursorPos Cursor position.
 * @return The list.
 */
vector<size_t> Editor::SelectionManager::getItemsUnderCursor(
    const Point& cursorPos
) const {
    vector<size_t> result;
    size_t nrTotalItems = getNrTotalItems();
    
    for(size_t i = 0; i < nrTotalItems; i++) {
        if(!getItemIsEligible(i)) continue;
        Point iCenter, iSize;
        getItemInfo(i, &iCenter, &iSize);
        if(itemsAreRectangular) {
            if(isPointInRectangle(cursorPos, iCenter, iSize)) {
                result.push_back(i);
            }
        } else {
            if(Distance(cursorPos, iCenter) <= iSize.x / 2.0f) {
                result.push_back(i);
            }
        }
    }
    
    return result;
}


/**
 * @brief Returns whether the selection has been homogenized by the user.
 *
 * @return Whether it was homogenized.
 */
bool Editor::SelectionManager::isHomogenized() const {
    return homogenized;
}


/**
 * @brief Returns the index of the first selected item, or INVALID if
 * none is selected.
 *
 * @return The index or INVALID.
 */
size_t Editor::SelectionManager::getFirstItemIdx() const {
    return selectedItems.getFirstItemIdx();
}


/**
 * @brief Returns the list of all selected items.
 *
 * @return The list.
 */
const set<size_t>& Editor::SelectionManager::getItemIdxs() const {
    return selectedItems.getItemIdxs();
}


/**
 * @brief Returns the info of an item, or 0,0 if not possible.
 *
 * @param idx The item's index.
 * @param outCenter The item's center is returned here.
 * @param outSize The item's size is returned here.
 * @return The info.
 */
void Editor::SelectionManager::getItemInfo(
    size_t idx, Point* outCenter, Point* outSize
) const {
    if(onGetInfo) {
        onGetInfo(idx, outCenter, outSize);
    } else {
        *outCenter = Point();
        *outSize = Point();
    }
}


/**
 * @brief Returns whether an item is eligible to be selected, or true
 * if not possible.
 *
 * @param idx The item's index.
 * @return Whether it's eligible.
 */
bool Editor::SelectionManager::getItemIsEligible(size_t idx) const {
    if(onIsEligible) {
        return onIsEligible(idx);
    }
    return true;
}


/**
 * @brief Returns the total number of items in the editor,
 * or 0 if not possible.
 *
 * @return The number.
 */
size_t Editor::SelectionManager::getNrTotalItems() const {
    if(onGetTotal) {
        return onGetTotal();
    }
    return 0;
}


/**
 * @brief Returns the index of the only selected item, or INVALID if
 * multiple or none are selected.
 *
 * @return The index or INVALID.
 */
size_t Editor::SelectionManager::getSingleItemIdx() const {
    return selectedItems.getSingleItemIdx();
}


/**
 * @brief Returns whether any items are selected.
 *
 * @return Whether any are selected.
 */
bool Editor::SelectionManager::hasAny() const {
    return selectedItems.hasAny();
}


/**
 * @brief Returns whether there is are multiple items selected.
 *
 * @return Whether there are multiple selected.
 */
bool Editor::SelectionManager::hasMultiple() const {
    return selectedItems.hasMultiple();
}


/**
 * @brief Returns whether there is only one item selected.
 *
 * @return Whether there is one selected.
 */
bool Editor::SelectionManager::hasOne() const {
    return selectedItems.hasOne();
}


/**
 * @brief Removes an item from the selection.
 *
 * @param idx The item's index.
 * @return Whether the item was selected.
 */
bool Editor::SelectionManager::remove(size_t idx) {
    if(!enabled) return false;
    return selectedItems.remove(idx);
}


/**
 * @brief Sets the selection to be a single item only.
 *
 * @param idx The item to select.
 * @return Whether that item wasn't already selected.
 */
bool Editor::SelectionManager::setSingle(size_t idx) {
    if(!enabled) return false;
    return selectedItems.setSingle(idx);
}


/**
 * @brief Sets whether the selection was homogenized by the user.
 *
 * @param homogenized The new value.
 * @return Whether that wasn't already the case.
 */
bool Editor::SelectionManager::setHomogenized(bool homogenized) {
    if(this->homogenized == homogenized) return false;
    this->homogenized = homogenized;
    return true;
}


/**
 * @brief Sets up everything to start an operation.
 *
 * @return Whether it succeeded.
 */
bool Editor::SelectionManager::startOperation() {
    if(!enabled) return false;
    
    preOpItemCenters.clear();
    preOpItemSizes.clear();
    
    const set<size_t>& list = getItemIdxs();
    for(size_t i : list) {
        Point iCenter, iSize;
        getItemInfo(i, &iCenter, &iSize);
        preOpItemCenters[i] = iCenter;
        preOpItemSizes[i] = iSize;
    }
    
    getBBox(
        &preOpSelCenter, &preOpSelSize,
        &preOpCentersOnlySelCenter, &preOpCentersOnlySelSize
    );
    
    return true;
}


/**
 * @brief Draws the widget.
 *
 * @param center Center point.
 * @param size Width and height. If nullptr, no scale handles will be drawn.
 * @param angle Angle. If nullptr, the rotation handle will not be drawn.
 * @param zoom Zoom the widget's components by this much.
 */
void Editor::TransformationWidget::draw(
    const Point* const center, const Point* const size,
    const float* const angle, float zoom
) const {
    const ALLEGRO_COLOR ROT_HANDLE_COLOR = al_map_rgb(64, 64, 192);
    const ALLEGRO_COLOR NORMAL_HANDLE_COLOR = al_map_rgb(96, 96, 224);
    const ALLEGRO_COLOR OUTLINE_COLOR = al_map_rgb(32, 32, 160);
    if(!center) return;
    
    Point handles[9];
    float radius;
    getLocations(center, size, angle, handles, &radius, nullptr);
    
    //Draw the rotation handle.
    if(angle && radius >= 0.0f) {
        al_draw_circle(
            center->x, center->y, radius,
            ROT_HANDLE_COLOR, EDITOR::TW_ROTATION_HANDLE_THICKNESS * zoom
        );
    }
    
    //Draw the outline.
    Point corners[4] = {
        handles[0],
        handles[2],
        handles[8],
        handles[6],
    };
    for(unsigned char c = 0; c < 4; c++) {
        size_t c2 = sumAndWrap(c, 1, 4);
        al_draw_line(
            corners[c].x, corners[c].y,
            corners[c2].x, corners[c2].y,
            OUTLINE_COLOR, EDITOR::TW_OUTLINE_THICKNESS * zoom
        );
    }
    
    //Draw the translation and scale handles.
    for(unsigned char h = 0; h < 9; h++) {
        if(!size && h != 4) continue;
        al_draw_filled_circle(
            handles[h].x, handles[h].y,
            EDITOR::TW_HANDLE_RADIUS * zoom, NORMAL_HANDLE_COLOR
        );
    }
}


/**
 * @brief Returns the location of all handles, based on the information it
 * was fed.
 *
 * @param center Center point.
 * @param size Width and height. If nullptr, the default size is used.
 * @param angle Angle. If nullptr, zero is used.
 * @param handles Return the location of all nine translation and scale
 * handles here.
 * @param radius Return the angle handle's radius here.
 * @param outTransform If not nullptr, the transformation used is
 * returned here.
 * The transformation will only rotate and translate, not scale.
 */
void Editor::TransformationWidget::getLocations(
    const Point* const center, const Point* const size,
    const float* const angle, Point* handles, float* radius,
    ALLEGRO_TRANSFORM* outTransform
) const {
    Point sizeToUse(EDITOR::TW_DEF_SIZE, EDITOR::TW_DEF_SIZE);
    if(size) sizeToUse = *size;
    
    //First, the Allegro transformation.
    ALLEGRO_TRANSFORM transformToUse;
    al_identity_transform(&transformToUse);
    if(angle) {
        al_rotate_transform(&transformToUse, *angle);
    }
    al_translate_transform(&transformToUse, center->x, center->y);
    
    //Get the coordinates of all translation and scale handles.
    handles[0] = { -sizeToUse.x / 2.0f, -sizeToUse.y / 2.0f };
    handles[1] = { 0.0f,                  -sizeToUse.y / 2.0f };
    handles[2] = { sizeToUse.x / 2.0f,  -sizeToUse.y / 2.0f };
    handles[3] = { -sizeToUse.x / 2.0f, 0.0f                  };
    handles[4] = { 0.0f,                  0.0f                  };
    handles[5] = { sizeToUse.x / 2.0f,  0.0f                  };
    handles[6] = { -sizeToUse.x / 2.0f, sizeToUse.y / 2.0f  };
    handles[7] = { 0.0f,                  sizeToUse.y / 2.0f  };
    handles[8] = { sizeToUse.x / 2.0f,  sizeToUse.y / 2.0f  };
    
    for(unsigned char h = 0; h < 9; h++) {
        al_transform_coordinates(
            &transformToUse, &handles[h].x, &handles[h].y
        );
    }
    
    float diameter = Distance(Point(), sizeToUse).toFloat();
    if(diameter == 0.0f) {
        *radius = 0.0f;
    } else {
        *radius = diameter / 2.0f;
    }
    
    if(outTransform) *outTransform = transformToUse;
}


/**
 * @brief Returns the center point before the user dragged the central handle.
 *
 * @return The old center.
 */
Point Editor::TransformationWidget::getOldCenter() const {
    return oldCenter;
}


/**
 * @brief Handles the user having held the left mouse button down.
 *
 * @param mouseCoords Mouse coordinates.
 * @param center Center point.
 * @param size Width and height. If nullptr, no scale handling
 * will be performed.
 * @param angle Angle. If nullptr, no rotation handling will be performed.
 * @param zoom Zoom the widget's components by this much.
 * @return Whether the user clicked on a handle.
 */
bool Editor::TransformationWidget::handleMouseDown(
    const Point& mouseCoords, const Point* const center,
    const Point* const size, const float* const angle, float zoom
) {
    if(!center) return false;
    
    Point handles[9];
    float radius;
    getLocations(center, size, angle, handles, &radius, nullptr);
    
    //Check if the user clicked on a translation or scale handle.
    for(unsigned char h = 0; h < 9; h++) {
        if(
            Distance(handles[h], mouseCoords) <=
            EDITOR::TW_HANDLE_RADIUS * zoom
        ) {
            if(h == 4) {
                movingHandle = h;
                oldCenter = *center;
                return true;
            } else if(size) {
                movingHandle = h;
                oldSize = *size;
                return true;
            }
        }
    }
    
    //Check if the user clicked on the rotation handle.
    if(angle) {
        Distance d(*center, mouseCoords);
        if(
            d >= radius - EDITOR::TW_ROTATION_HANDLE_THICKNESS / 2.0f * zoom &&
            d <= radius + EDITOR::TW_ROTATION_HANDLE_THICKNESS / 2.0f * zoom
        ) {
            movingHandle = 9;
            oldAngle = *angle;
            oldMouseAngle = getAngle(*center, mouseCoords);
            return true;
        }
    }
    
    return false;
}


/**
 * @brief Handles the user having moved the mouse cursor.
 *
 * @param mouseCoords Mouse coordinates.
 * @param center Center point.
 * @param size Width and height. If nullptr, no scale handling
 * will be performed.
 * @param angle Angle. If nullptr, no rotation handling will be performed.
 * @param zoom Zoom the widget's components by this much.
 * @param keepAspectRatio If true, aspect ratio is kept when resizing.
 * @param keepArea If true, keep the same total area.
 * Used for squash and stretch.
 * @param minSize Minimum possible size for the width or height.
 * Use -FLT_MAX for none.
 * @param lockCenter If true, scaling happens with the center locked.
 * If false, the opposite edge or corner is locked instead.
 * @return Whether the user is dragging a handle.
 */
bool Editor::TransformationWidget::handleMouseMove(
    const Point& mouseCoords, Point* center, Point* size, float* angle,
    float zoom, bool keepAspectRatio, bool keepArea,
    float minSize, bool lockCenter
) {
    if(!center) return false;
    
    if(movingHandle == -1) {
        return false;
    }
    
    //Logic for moving the center handle.
    if(movingHandle == 4) {
        *center = mouseCoords;
        return true;
    }
    
    //Logic for moving the rotation handle.
    if(movingHandle == 9 && angle) {
        *angle =
            oldAngle +
            getAngle(*center, mouseCoords) - oldMouseAngle;
        return true;
    }
    
    //From here on out, it's logic to move a scale handle.
    if(!size) {
        return false;
    }
    
    ALLEGRO_TRANSFORM t;
    Point handles[9];
    float radius;
    getLocations(center, size, angle, handles, &radius, &t);
    al_invert_transform(&t);
    
    Point transformedMouse = mouseCoords;
    Point transformedCenter = *center;
    Point newSize = oldSize;
    al_transform_coordinates(&t, &transformedMouse.x, &transformedMouse.y);
    al_transform_coordinates(&t, &transformedCenter.x, &transformedCenter.y);
    bool scalingX = false;
    bool scalingY = false;
    
    switch(movingHandle) {
    case 0:
    case 3:
    case 6: {
        newSize.x = size->x / 2.0f - transformedMouse.x;
        scalingX = true;
        break;
    } case 2:
    case 5:
    case 8: {
        newSize.x = transformedMouse.x - (-size->x / 2.0f);
        scalingX = true;
        break;
    }
    }
    
    switch(movingHandle) {
    case 0:
    case 1:
    case 2: {
        newSize.y = (size->y / 2.0f) - transformedMouse.y;
        scalingY = true;
        break;
    } case 6:
    case 7:
    case 8: {
        newSize.y = transformedMouse.y - (-size->y / 2.0f);
        scalingY = true;
        break;
    }
    }
    
    newSize.x = std::max(minSize, newSize.x);
    newSize.y = std::max(minSize, newSize.y);
    
    if(keepAspectRatio && oldSize.x != 0.0f && oldSize.y != 0.0f) {
        float scaleToUse;
        float wScale = newSize.x / oldSize.x;
        float hScale = newSize.y / oldSize.y;
        if(!scalingY) {
            scaleToUse = wScale;
        } else if(!scalingX) {
            scaleToUse = hScale;
        } else {
            if(fabs(wScale) > fabs(hScale)) {
                scaleToUse = wScale;
            } else {
                scaleToUse = hScale;
            }
        }
        scaleToUse = std::max(minSize / oldSize.x, scaleToUse);
        scaleToUse = std::max(minSize / oldSize.y, scaleToUse);
        newSize = oldSize * scaleToUse;
        
    } else if(keepArea && oldSize.x != 0.0f && oldSize.y != 0.0f) {
        bool byX;
        float wScale = newSize.x / oldSize.x;
        float hScale = newSize.y / oldSize.y;
        double oldArea = (double) oldSize.x * (double) oldSize.y;
        if(!scalingY) {
            byX = true;
        } else if(!scalingX) {
            byX = false;
        } else {
            if(fabs(wScale) < fabs(hScale)) {
                byX = true;
            } else {
                byX = false;
            }
        }
        if(byX) {
            if(minSize != -FLT_MAX) {
                newSize.x = std::max(minSize, newSize.x);
            }
            newSize.y = oldArea / newSize.x;
        } else {
            if(minSize != -FLT_MAX) {
                newSize.y = std::max(minSize, newSize.y);
            }
            newSize.x = oldArea / newSize.y;
        }
    }
    
    switch(movingHandle) {
    case 0:
    case 3:
    case 6: {
        if(!lockCenter) {
            transformedCenter.x = (size->x / 2.0f) - newSize.x / 2.0f;
        }
        break;
    } case 2:
    case 5:
    case 8: {
        if(!lockCenter) {
            transformedCenter.x = (-size->x / 2.0f) + newSize.x / 2.0f;
        }
        break;
    }
    }
    
    switch(movingHandle) {
    case 0:
    case 1:
    case 2: {
        if(!lockCenter) {
            transformedCenter.y = (size->y / 2.0f) - newSize.y / 2.0f;
        }
        break;
    } case 6:
    case 7:
    case 8: {
        if(!lockCenter) {
            transformedCenter.y = (-size->y / 2.0f) + newSize.y / 2.0f;
        }
        break;
    }
    }
    
    Point newCenter = transformedCenter;
    al_invert_transform(&t);
    al_transform_coordinates(&t, &newCenter.x, &newCenter.y);
    
    *center = newCenter;
    *size = newSize;
    
    return true;
}


/**
 * @brief Handles the user having released the left mouse button.
 *
 * @return Whether the user stopped dragging a handle.
 */
bool Editor::TransformationWidget::handleMouseUp() {
    if(movingHandle == -1) {
        return false;
    }
    
    movingHandle = -1;
    return true;
}


/**
 * @brief Is the user currently moving the central handle?
 *
 * @return Whether the user is moving the handle.
 */
bool Editor::TransformationWidget::isMovingCenterHandle() {
    return (movingHandle == 4);
}


/**
 * @brief Is the user currently moving a handle?
 *
 * @return Whether the user is moving a handle.
 */
bool Editor::TransformationWidget::isMovingHandle() {
    return (movingHandle != -1);
}
