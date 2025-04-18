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
#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../core/load.h"
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

//Picker dialog button size.
const float PICKER_IMG_BUTTON_SIZE = 168.0f;

//Picker dialog minimum button size.
const float PICKER_IMG_BUTTON_MIN_SIZE = 32.0f;

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
 * @brief Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the window,
 * where it's hard to see.
 *
 * @param min_coords Top-left coordinates of the content to focus on.
 * @param max_coords Bottom-right coordinates of the content to focus on.
 * @param instantaneous If true, the camera moves there instantaneously.
 * If false, it smoothly gets there over time.
 */
void Editor::centerCamera(
    const Point &min_coords, const Point &max_coords,
    bool instantaneous
) {
    Point min_c = min_coords;
    Point max_c = max_coords;
    if(min_c == max_c) {
        min_c = min_c - 2.0;
        max_c = max_c + 2.0;
    }
    
    float width = max_c.x - min_c.x;
    float height = max_c.y - min_c.y;
    
    game.cam.targetPos.x = floor(min_c.x + width  / 2);
    game.cam.targetPos.y = floor(min_c.y + height / 2);
    
    float z;
    if(width > height) z = (canvasBR.x - canvasTL.x) / width;
    else z = (canvasBR.y - canvasTL.y) / height;
    z -= z * 0.1;
    
    game.cam.targetZoom = z;
    
    if(instantaneous) {
        game.cam.pos = game.cam.targetPos;
        game.cam.zoom = game.cam.targetZoom;
    }
    
    updateTransformations();
}


/**
 * @brief Closes the topmost dialog that is still open.
 */
void Editor::closeTopDialog() {
    for(size_t d = 0; d < dialogs.size(); d++) {
        Dialog* d_ptr = dialogs[dialogs.size() - (d + 1)];
        if(d_ptr->isOpen) {
            d_ptr->isOpen = false;
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
    
    game.cam.tick(game.deltaT);
    game.cam.updateBox();
    
    opErrorFlashTimer.tick(game.deltaT);
    
    updateTransformations();
}


/**
 * @brief Draws the grid, using the current game camera.
 *
 * @param interval Interval between grid lines.
 * @param major_color Color to use for major lines.
 * These are lines that happen at major milestones (i.e. twice the interval).
 * @param minor_color Color to use for minor lines.
 * These are lines that aren't major.
 */
void Editor::drawGrid(
    float interval,
    const ALLEGRO_COLOR &major_color, const ALLEGRO_COLOR &minor_color
) {
    Point cam_top_left_corner(0, 0);
    Point cam_bottom_right_corner(canvasBR.x, canvasBR.y);
    al_transform_coordinates(
        &game.windowToWorldTransform,
        &cam_top_left_corner.x, &cam_top_left_corner.y
    );
    al_transform_coordinates(
        &game.windowToWorldTransform,
        &cam_bottom_right_corner.x, &cam_bottom_right_corner.y
    );
    
    float x = floor(cam_top_left_corner.x / interval) * interval;
    while(x < cam_bottom_right_corner.x + interval) {
        ALLEGRO_COLOR c = minor_color;
        bool draw_line = true;
        
        if(fmod(x, interval * 2) == 0) {
            c = major_color;
            if((interval * 2) * game.cam.zoom <= 6) {
                draw_line = false;
            }
        } else {
            if(interval * game.cam.zoom <= 6) {
                draw_line = false;
            }
        }
        
        if(draw_line) {
            al_draw_line(
                x, cam_top_left_corner.y,
                x, cam_bottom_right_corner.y + interval,
                c, 1.0f / game.cam.zoom
            );
        }
        x += interval;
    }
    
    float y = floor(cam_top_left_corner.y / interval) * interval;
    while(y < cam_bottom_right_corner.y + interval) {
        ALLEGRO_COLOR c = minor_color;
        bool draw_line = true;
        
        if(fmod(y, interval * 2) == 0) {
            c = major_color;
            if((interval * 2) * game.cam.zoom <= 6) {
                draw_line = false;
            }
        } else {
            if(interval * game.cam.zoom <= 6) {
                draw_line = false;
            }
        }
        
        if(draw_line) {
            al_draw_line(
                cam_top_left_corner.x, y,
                cam_bottom_right_corner.x + interval, y,
                c, 1.0f / game.cam.zoom
            );
        }
        y += interval;
    }
}


/**
 * @brief Draws a small red X on the cursor, signifying an operation has failed.
 */
void Editor::drawOpErrorCursor() {
    float error_flash_time_ratio = opErrorFlashTimer.getRatioLeft();
    if(error_flash_time_ratio <= 0.0f) return;
    Point pos = opErrorPos;
    drawBitmap(
        game.sysContent.bmpNotification,
        Point(
            pos.x,
            pos.y - EDITOR::OP_ERROR_CURSOR_SIZE
        ),
        Point(
            EDITOR::OP_ERROR_CURSOR_SIZE * 2.5f,
            EDITOR::OP_ERROR_CURSOR_SIZE * 2.0f
        ),
        0.0f,
        mapAlpha(error_flash_time_ratio * 192)
    );
    pos.x +=
        EDITOR::OP_ERROR_CURSOR_SHAKE_WIDTH *
        sin(game.timePassed * EDITOR::OP_ERROR_CURSOR_SHAKE_SPEED) *
        error_flash_time_ratio;
    pos.y -= EDITOR::OP_ERROR_CURSOR_SIZE;
    al_draw_line(
        pos.x - EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.y - EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.x + EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.y + EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        al_map_rgba_f(1.0f, 0.0f, 0.0f, error_flash_time_ratio),
        EDITOR::OP_ERROR_CURSOR_THICKNESS
    );
    al_draw_line(
        pos.x + EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.y - EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.x - EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        pos.y + EDITOR::OP_ERROR_CURSOR_SIZE / 2.0f,
        al_map_rgba_f(1.0f, 0.0f, 0.0f, error_flash_time_ratio),
        EDITOR::OP_ERROR_CURSOR_THICKNESS
    );
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
 * @brief Returns whether or not Dear ImGui currently needs the keyboard
 * right now.
 *
 * @return Whether it needs the keyboard.
 */
bool Editor::guiNeedsKeyboard() {
    //WantCaptureKeyboard returns true if LMB is held, and I'm not quite sure
    //why. If we know LMB is held because of the canvas, then we can
    //safely assume it's none of Dear ImGui's business, so we can ignore
    //WantCaptureKeyboard's true.
    return ImGui::GetIO().WantCaptureKeyboard && !isM1Pressed;
}


/**
 * @brief Handles an Allegro event for control-related things.
 *
 * @param ev Event to handle.
 */
void Editor::handleAllegroEvent(ALLEGRO_EVENT &ev) {
    if(game.fadeMgr.isFading()) return;
    
    bool is_mouse_in_canvas =
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
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        is_mouse_in_canvas
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
            
            if(guiNeedsKeyboard()) {
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
            
            if(guiNeedsKeyboard()) {
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
            isM1Pressed = false;
            handleLmbUp(ev);
            break;
        } case 2: {
            isM2Pressed = false;
            handleRmbUp(ev);
            break;
        } case 3: {
            isM3Pressed = false;
            handleMmbUp(ev);
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
            is_mouse_in_canvas
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
            if(!guiNeedsKeyboard()) {
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
            if(!guiNeedsKeyboard()) {
                handleKeyUpCanvas(ev);
            }
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        //Key char.
        
        if(dialogs.empty()) {
            handleKeyCharAnywhere(ev);
            if(!guiNeedsKeyboard()) {
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
void Editor::handleKeyCharAnywhere(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being "char-typed" in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyCharCanvas(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being pressed down anywhere.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyDownAnywhere(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being pressed down in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyDownCanvas(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being released anywhere.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyUpAnywhere(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being released in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleKeyUpCanvas(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the left mouse button being double-clicked
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleLmbDoubleClick(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the left mouse button being pressed down
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleLmbDown(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the left mouse button being dragged
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleLmbDrag(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the left mouse button released
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleLmbUp(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being double-clicked
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMmbDoubleClick(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being pressed down
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMmbDown(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being dragged
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMmbDrag(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being released
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMmbUp(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the mouse coordinates being updated.
 *
 * @param ev Event to process.
 */
void Editor::handleMouseUpdate(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the mouse wheel being turned in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleMouseWheel(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the right mouse button being double-clicked
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleRmbDoubleClick(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the right mouse button being pressed down
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleRmbDown(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the right mouse button being dragged
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleRmbDrag(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the right mouse button being released
 * in the canvas.
 *
 * @param ev Event to process.
 */
void Editor::handleRmbUp(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Returns whether a given internal name is good or not.
 *
 * @param name The internal name to check.
 * @return Whether it's good.
 */
bool Editor::isInternalNameGood(const string &name) const {
    for(size_t c = 0; c < name.size(); c++) {
        char ch = name[c];
        const bool is_lowercase = ch >= 'a' && ch <= 'z';
        const bool is_digit = ch >= '0' && ch <= '9';
        const bool is_underscore = ch == '_';
        if(!is_lowercase && !is_digit && !is_underscore) return false;
    }
    return true;
}


/**
 * @brief Returns whether or not the pressed key corresponds to the specified
 * key combination. Used for keyboard shortcuts.
 *
 * @param pressed_key Key that the user pressed.
 * @param match_key Key that must be matched in order to return true.
 * @param needs_ctrl If true, only returns true if Ctrl was also pressed.
 * @param needs_shift If true, only returns true if Shift was also pressed.
 * @return Whether the pressed key corresponds.
 */
bool Editor::keyCheck(
    int pressed_key, int match_key,
    bool needs_ctrl, bool needs_shift
) {

    if(pressed_key != match_key) {
        return false;
    }
    if(needs_ctrl != isCtrlPressed) {
        return false;
    }
    if(needs_shift != isShiftPressed) {
        return false;
    }
    return true;
}


/**
 * @brief Draws a Dear ImGui-like visualizer for keyframes involving colors.
 *
 * @param interpolator Interpolator to get the information from.
 * @param sel_keyframe_idx Index of the currently selected keyframe.
 */
void Editor::keyframeVisualizer(
    KeyframeInterpolator<ALLEGRO_COLOR> &interpolator,
    size_t sel_keyframe_idx
) {
    if(interpolator.getKeyframeCount() <= 1) return;
    
    //Setup.
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    //Draw the classic alpha checkboard background.
    ImGui::RenderColorRectWithAlphaCheckerboard(
        draw_list, pos,
        ImVec2(pos.x + (ImGui::GetColumnWidth() - 1), pos.y + 40),
        ImColor(0.0f, 0.0f, 0.0f, 0.0f), 5, ImVec2(0.0f, 0.0f)
    );
    
    //Draw the rectangle of the color from the start to the first keyframe.
    auto first_kf = interpolator.getKeyframe(0);
    ALLEGRO_COLOR c_start = first_kf.second;
    draw_list->AddRectFilled(
        pos,
        ImVec2(
            pos.x + (ImGui::GetColumnWidth() - 1) * first_kf.first,
            pos.y + 40
        ),
        ImColor(c_start.r, c_start.g, c_start.b, c_start.a)
    );
    
    //Draw the rectangles of the colors between the keyframes.
    for(size_t t = 0; t < interpolator.getKeyframeCount() - 1; t++) {
        auto kf_1 = interpolator.getKeyframe(t);
        auto kf_2 = interpolator.getKeyframe(t + 1);
        ALLEGRO_COLOR c1 = kf_1.second;
        ALLEGRO_COLOR c2 = kf_2.second;
        
        draw_list->AddRectFilledMultiColor(
            ImVec2(
                pos.x + (ImGui::GetColumnWidth() - 1) * kf_1.first,
                pos.y
            ),
            ImVec2(
                pos.x + (ImGui::GetColumnWidth() - 1) * kf_2.first,
                pos.y + 40
            ),
            ImColor(c1.r, c1.g, c1.b, c1.a), ImColor(c2.r, c2.g, c2.b, c2.a),
            ImColor(c2.r, c2.g, c2.b, c2.a), ImColor(c1.r, c1.g, c1.b, c1.a)
        );
    }
    
    //Draw the rectangle of the color from the final keyframe to the end.
    auto last_kf = interpolator.getKeyframe(interpolator.getKeyframeCount() - 1);
    ALLEGRO_COLOR c_end = last_kf.second;
    draw_list->AddRectFilled(
        ImVec2(
            pos.x + (ImGui::GetColumnWidth() - 1) * last_kf.first,
            pos.y
        ),
        ImVec2(pos.x + (ImGui::GetColumnWidth() - 1), pos.y + 40),
        ImColor(c_end.r, c_end.g, c_end.b, c_end.a)
    );
    
    //Draw the bars indicating the position of each keyframe.
    for(size_t c = 0; c < interpolator.getKeyframeCount(); c++) {
        float time = interpolator.getKeyframe(c).first;
        float line_x = time * (ImGui::GetColumnWidth() - 1);
        ImColor col =
            c == sel_keyframe_idx ?
            ImGui::GetColorU32(ImGuiCol_PlotLinesHovered) :
            ImGui::GetColorU32(ImGuiCol_PlotLines);
        draw_list->AddRectFilled(
            ImVec2(pos.x + line_x - 2, pos.y),
            ImVec2(pos.x + line_x + 2, pos.y + 43),
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
 * @param sel_keyframe_idx Index of the currently selected keyframe.
 */
void Editor::keyframeVisualizer(
    KeyframeInterpolator<float> &interpolator,
    size_t sel_keyframe_idx
) {
    if(interpolator.getKeyframeCount() <= 1) return;
    
    //The built in plot widget doesn't allow for dynamic spacing,
    //so we need to make our own.
    
    //Setup.
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImVec2((ImGui::GetColumnWidth() - 1), 40);
    
    float min_value = FLT_MAX;
    float max_value = -FLT_MAX;
    
    for(size_t t = 0; t < interpolator.getKeyframeCount(); t++) {
        min_value = std::min(interpolator.getKeyframe(t).second, min_value);
        max_value = std::max(interpolator.getKeyframe(t).second, max_value);
    }
    
    if(min_value == max_value) {
        //Add the same space above and below to get a nice line at the middle.
        min_value -= 10.0f;
        max_value += 10.0f;
    }
    
    //Draw the background.
    draw_list->AddRectFilled(
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + (ImGui::GetColumnWidth() - 1), pos.y + 40),
        ImGui::GetColorU32(ImGuiCol_FrameBg)
    );
    
    //Draw the chart line from the start to the first keyframe.
    auto first_kf = interpolator.getKeyframe(0);
    draw_list->AddLine(
        ImVec2(
            pos.x,
            pos.y + interpolateNumber(
                first_kf.second, min_value, max_value, size.y, 1
            )
        ),
        ImVec2(
            pos.x + size.x * first_kf.first,
            pos.y + interpolateNumber(
                first_kf.second, min_value, max_value, size.y, 1
            )
        ),
        ImGui::GetColorU32(ImGuiCol_PlotLines)
    );
    
    //Draw the chart lines between the keyframes.
    for(size_t t = 0; t < interpolator.getKeyframeCount() - 1; t++) {
        auto kf_1 = interpolator.getKeyframe(t);
        auto kf_2 = interpolator.getKeyframe(t + 1);
        float f1 = kf_1.second;
        float f2 = kf_2.second;
        
        draw_list->AddLine(
            ImVec2(
                pos.x + size.x * kf_1.first,
                pos.y + interpolateNumber(
                    f1, min_value, max_value, size.y, 1
                )
            ),
            ImVec2(
                pos.x + size.x * kf_2.first,
                pos.y + interpolateNumber(
                    f2, min_value, max_value, size.y, 1
                )
            ),
            ImGui::GetColorU32(ImGuiCol_PlotLines)
        );
    }
    
    //Draw the chart line from the final keyframe to the end.
    auto last_kf = interpolator.getKeyframe(interpolator.getKeyframeCount() - 1);
    draw_list->AddLine(
        ImVec2(
            pos.x + size.x * last_kf.first,
            pos.y + interpolateNumber(
                last_kf.second, min_value, max_value, size.y, 1
            )
        ),
        ImVec2(
            pos.x + size.x,
            pos.y + interpolateNumber(
                last_kf.second, min_value, max_value, size.y, 1
            )
        ),
        ImGui::GetColorU32(ImGuiCol_PlotLines)
    );
    
    //Draw the bars indicating the position of each keyframe.
    for(size_t c = 0; c < interpolator.getKeyframeCount(); c++) {
        float time = interpolator.getKeyframe(c).first;
        float line_x = time * (ImGui::GetColumnWidth() - 1);
        ImColor col =
            c == sel_keyframe_idx ?
            ImGui::GetColorU32(ImGuiCol_PlotLinesHovered) :
            ImGui::GetColorU32(ImGuiCol_PlotLines);
        draw_list->AddRectFilled(
            ImVec2(pos.x + line_x - 2, pos.y),
            ImVec2(pos.x + line_x + 2, pos.y + 43),
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
 * @param sel_keyframe_idx Index of the currently selected keyframe.
 */
void Editor::keyframeVisualizer(
    KeyframeInterpolator<Point> &interpolator,
    size_t sel_keyframe_idx
) {
    if(interpolator.getKeyframeCount() <= 1) return;
    
    //Split the interpolator into two, one for each axis.
    KeyframeInterpolator<float> x_inter(interpolator.getKeyframe(0).second.x);
    KeyframeInterpolator<float> y_inter(interpolator.getKeyframe(0).second.y);
    
    x_inter.setKeyframeTime(0, interpolator.getKeyframe(0).first);
    y_inter.setKeyframeTime(0, interpolator.getKeyframe(0).first);
    
    for(size_t s = 1; s < interpolator.getKeyframeCount(); s++) {
        auto kf = interpolator.getKeyframe(s);
        x_inter.add(kf.first, kf.second.x);
        y_inter.add(kf.first, kf.second.y);
    }
    
    //Draw the two visualizers.
    keyframeVisualizer(x_inter, sel_keyframe_idx);
    keyframeVisualizer(y_inter, sel_keyframe_idx);
}


/**
 * @brief Processes Dear ImGui widgets tha allow organizing keyframe
 * interpolators.
 *
 * @tparam InterT Type of the interpolator value.
 * @param button_id Prefix for the Dear ImGui ID of the navigation buttons.
 * @param interpolator Interpolator to get data from.
 * @param sel_keyframe_idx Index of the currently selected keyframe.
 * @return Whether anything in the interpolator was changed.
 */
template <class InterT>
bool Editor::keyframeOrganizer(
    const string &button_id,
    KeyframeInterpolator<InterT> &interpolator,
    size_t &sel_keyframe_idx
) {
    bool result = false;
    
    //First, some utility setup.
    if(interpolator.getKeyframeCount() == 1) {
        interpolator.setKeyframeTime(0, 0.0f);
    }
    
    //Current keyframe text.
    ImGui::Text(
        "Keyframe: %lu/%lu",
        sel_keyframe_idx + 1,
        interpolator.getKeyframeCount()
    );
    
    if(interpolator.getKeyframeCount() > 1) {
        //Previous keyframe button.
        ImGui::SameLine();
        string prevLabel = button_id + "prevButton";
        if(
            ImGui::ImageButton(
                prevLabel, editorIcons[EDITOR_ICON_PREVIOUS],
                Point(EDITOR::ICON_BMP_SIZE / 2.0f)
            )
        ) {
            if(sel_keyframe_idx == 0) {
                sel_keyframe_idx = interpolator.getKeyframeCount() - 1;
            } else {
                sel_keyframe_idx--;
            }
        }
        setTooltip(
            "Select the previous keyframe."
        );
        
        //Next keyframe button.
        ImGui::SameLine();
        string nextLabel = button_id + "nextButton";
        if(
            ImGui::ImageButton(
                nextLabel, editorIcons[EDITOR_ICON_NEXT],
                Point(EDITOR::ICON_BMP_SIZE / 2.0f)
            )
        ) {
            if(sel_keyframe_idx == interpolator.getKeyframeCount() - 1) {
                sel_keyframe_idx = 0;
            } else {
                sel_keyframe_idx++;
            }
        }
        setTooltip(
            "Select the next keyframe."
        );
    }
    
    //Add keyframe button.
    ImGui::SameLine();
    string addLabel = button_id + "addButton";
    if(
        ImGui::ImageButton(
            addLabel, editorIcons[EDITOR_ICON_ADD],
            Point(EDITOR::ICON_BMP_SIZE / 2.0f)
        )
    ) {
        float prev_t = interpolator.getKeyframe(sel_keyframe_idx).first;
        float next_t =
            sel_keyframe_idx == interpolator.getKeyframeCount() - 1 ?
            1.0f :
            interpolator.getKeyframe(sel_keyframe_idx + 1).first;
        float new_t = (prev_t + next_t) / 2.0f;
        
        interpolator.add(new_t, interpolator.get(new_t));
        sel_keyframe_idx++;
        setStatus(
            "Added keyframe #" + i2s(sel_keyframe_idx + 1) + "."
        );
        result = true;
    }
    setTooltip(
        "Add a new keyframe after the currently selected one.\n"
        "It will go between the current one and the one after."
    );
    
    if(interpolator.getKeyframeCount() > 1) {
        //Delete frame button.
        ImGui::SameLine();
        string removeButton = button_id + "removeButton";
        if(
            ImGui::ImageButton(
                removeButton, editorIcons[EDITOR_ICON_REMOVE],
                Point(EDITOR::ICON_BMP_SIZE / 2.0f)
            )
        ) {
            size_t deleted_frame_idx = sel_keyframe_idx;
            interpolator.remove(deleted_frame_idx);
            if(sel_keyframe_idx == interpolator.getKeyframeCount()) {
                sel_keyframe_idx--;
            }
            setStatus(
                "Deleted keyframe #" + i2s(deleted_frame_idx + 1) + "."
            );
            result = true;
        }
        setTooltip(
            "Delete the currently selected keyframe."
        );
    }
    
    return result;
}


/**
 * @brief Processes Dear ImGui widgets for visualizing and editing a color
 * keyframe interpolator.
 *
 * @param label Label for a keyframe's value.
 * @param interpolator Interpolator to edit.
 * @param sel_keyframe_idx Index of the currently selected keyframe.
 * @return Whether anything in the interpolator was changed.
 */
bool Editor::keyframeEditor(
    const string &label,
    KeyframeInterpolator<ALLEGRO_COLOR> &interpolator,
    size_t &sel_keyframe_idx
) {
    //Visualizer.
    keyframeVisualizer(interpolator, sel_keyframe_idx);
    
    //Organizer.
    bool result = keyframeOrganizer(label, interpolator, sel_keyframe_idx);
    
    if(interpolator.getKeyframeCount() > 1) {
        //Time value.
        float time = interpolator.getKeyframe(sel_keyframe_idx).first;
        if(ImGui::SliderFloat("Time", &time, 0.0f, 1.0f)) {
            interpolator.setKeyframeTime(
                sel_keyframe_idx, time, &sel_keyframe_idx
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
    ALLEGRO_COLOR value = interpolator.getKeyframe(sel_keyframe_idx).second;
    if(ImGui::ColorEdit4(label.c_str(), (float*) &value)) {
        interpolator.setKeyframeValue(sel_keyframe_idx, value);
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
 * @param sel_keyframe_idx Index of the currently selected keyframe.
 * @return Whether anything in the interpolator was changed.
 */
bool Editor::keyframeEditor(
    const string &label,
    KeyframeInterpolator<float> &interpolator,
    size_t &sel_keyframe_idx
) {
    //Visualizer.
    keyframeVisualizer(interpolator, sel_keyframe_idx);
    
    //Organizer.
    bool result = keyframeOrganizer(label, interpolator, sel_keyframe_idx);
    
    if(interpolator.getKeyframeCount() > 1) {
        //Time value.
        float time = interpolator.getKeyframe(sel_keyframe_idx).first;
        if(ImGui::SliderFloat("Time", &time, 0.0f, 1.0f)) {
            interpolator.setKeyframeTime(
                sel_keyframe_idx, time, &sel_keyframe_idx
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
    float value = interpolator.getKeyframe(sel_keyframe_idx).second;
    if(ImGui::DragFloat(label.c_str(), &value)) {
        interpolator.setKeyframeValue(sel_keyframe_idx, value);
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
 * @param sel_keyframe_idx Index of the currently selected keyframe.
 * @return Whether anything in the interpolator was changed.
 */
bool Editor::keyframeEditor(
    const string &label,
    KeyframeInterpolator<Point> &interpolator,
    size_t &sel_keyframe_idx
) {
    //Visualizer.
    keyframeVisualizer(interpolator, sel_keyframe_idx);
    
    //Organizer.
    bool result = keyframeOrganizer(label, interpolator, sel_keyframe_idx);
    
    if(interpolator.getKeyframeCount() > 1) {
        //Time value.
        float time = interpolator.getKeyframe(sel_keyframe_idx).first;
        if(ImGui::SliderFloat("Time", &time, 0.0f, 1.0f)) {
            interpolator.setKeyframeTime(
                sel_keyframe_idx, time, &sel_keyframe_idx
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
    Point value = interpolator.getKeyframe(sel_keyframe_idx).second;
    if(ImGui::DragFloat2(label.c_str(), (float*) &value)) {
        interpolator.setKeyframeValue(sel_keyframe_idx, value);
        result = true;
    }
    setTooltip("What coordinates to use at this keyframe.");
    
    return result;
}


/**
 * @brief Exits out of the editor, with a fade.
 */
void Editor::leave() {
    //Save the user's preferred tree node open states.
    saveOptions();
    
    game.fadeMgr.startFade(false, [] () {
        if(game.states.areaEd->quickPlayAreaPath.empty()) {
            game.states.titleScreen->pageToLoad = MAIN_MENU_PAGE_MAKE;
            game.changeState(game.states.titleScreen);
        } else {
            game.states.gameplay->pathOfAreaToLoad =
                game.states.areaEd->quickPlayAreaPath;
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
 * @param picked_item If an item was picked, set this to its name.
 * @param use_monospace Whether to use a monospace font for the items.
 * @return Whether an item was clicked on.
 */
bool Editor::listPopup(
    const char* label, const vector<string> &items, string* picked_item,
    bool use_monospace
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        if(escapeWasPressed) {
            ImGui::CloseCurrentPopup();
        }
        if(use_monospace) ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
        for(size_t i = 0; i < items.size(); i++) {
            string name = items[i];
            bool hit_button =
                use_monospace ?
                monoSelectable(name.c_str()) :
                ImGui::Selectable(name.c_str());
            if(hit_button) {
                *picked_item = name;
                ret = true;
            }
        }
        if(use_monospace) ImGui::PopFont();
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
 * @param picked_item_idx If an item was picked, set this to its index.
 * @param use_monospace Whether to use a monospace font for the items.
 * @return Whether an item was clicked on.
 */
bool Editor::listPopup(
    const char* label, const vector<string> &items, int* picked_item_idx,
    bool use_monospace
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        if(escapeWasPressed) {
            ImGui::CloseCurrentPopup();
        }
        if(use_monospace) ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
        for(size_t i = 0; i < items.size(); i++) {
            string name = items[i];
            bool hit_button =
                use_monospace ?
                monoSelectable(name.c_str()) :
                ImGui::Selectable(name.c_str());
            if(hit_button) {
                *picked_item_idx = (int) i;
                ret = true;
            }
        }
        if(use_monospace) ImGui::PopFont();
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
    game.mouseCursor.show();
    game.cam.setPos(Point());
    game.cam.set_zoom(1.0f);
    updateStyle();
    
    game.fadeMgr.startFade(true, nullptr);
    ImGui::Reset();
}


/**
 * @brief Loads all mob types into the customCatTypes list.
 *
 * @param is_area_editor If true, mob types that do not appear in the
 * area editor will not be counted for here.
 */
void Editor::loadCustomMobCatTypes(bool is_area_editor) {
    //Load.
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        MobCategory* c_ptr = game.mobCategories.get((MOB_CATEGORY) c);
        vector<string> type_names;
        c_ptr->getTypeNames(type_names);
        
        for(size_t tn = 0; tn < type_names.size(); tn++) {
            MobType* mt_ptr = c_ptr->getType(type_names[tn]);
            
            if(is_area_editor && !mt_ptr->appearsInAreaEditor) {
                continue;
            }
            
            string custom_cat_name = mt_ptr->customCategoryName;
            size_t custom_cat_idx;
            map<string, size_t>::iterator custom_cat_idx_it =
                customCatNameIdxs.find(custom_cat_name);
            if(custom_cat_idx_it == customCatNameIdxs.end()) {
                customCatNameIdxs[custom_cat_name] =
                    customCatTypes.size();
                customCatTypes.push_back(vector<MobType*>());
            }
            custom_cat_idx = customCatNameIdxs[custom_cat_name];
            
            customCatTypes[custom_cat_idx].push_back(mt_ptr);
        }
    }
    
    //Sort.
    std::sort(
        customCatTypes.begin(), customCatTypes.end(),
    [] (const vector<MobType*> &c1, const vector<MobType*> &c2) -> bool {
        return
        c1.front()->customCategoryName <
        c2.front()->customCategoryName;
    }
    );
    for(size_t c = 0; c < customCatTypes.size(); c++) {
        vector<MobType*> &types = customCatTypes[c];
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
 * @param do_pick_callback Callback for if we actually have to do the pick.
 */
void Editor::openBaseContentWarningDialog(
    const std::function<void()> &do_pick_callback
) {
    if(game.options.advanced.engineDev) {
        do_pick_callback();
        return;
    }
    
    openDialog(
        "Base pack warning",
        std::bind(&Editor::processGuiBaseContentWarningDialog, this)
    );
    dialogs.back()->customSize = Point(320, 0);
    baseContentWarningDoPickCallback = do_pick_callback;
}


/**
 * @brief Opens a dialog where the user can choose a bitmap from the
 * game content.
 *
 * @param ok_callback Callback for when the user chooses a bitmap.
 * @param recommended_folder If not empty, recommend that the user picks bitmaps
 * with this folder name. Use "." for the graphics root folder.
 */
void Editor::openBitmapDialog(
    std::function<void(const string &)> ok_callback,
    const string &recommended_folder
) {
    bitmapDialogOkCallback = ok_callback;
    bitmapDialogRecommendedFolder = recommended_folder;
    bitmapDialogPicker.pickCallback =
        [this] (
            const string &new_bmp_name, const string &, const string &, void*, bool
    ) {
        bitmapDialogNewBmpName = new_bmp_name;
    };
    bitmapDialogPicker.needsFilterBoxFocus = true;
    
    openDialog(
        "Choose a bitmap",
        std::bind(&Editor::processGuiBitmapDialog, this)
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
 * @param process_callback A function to call when it's time to process
 * the contents inside the dialog.
 */
void Editor::openDialog(
    const string &title,
    const std::function<void()> &process_callback
) {
    Dialog* new_dialog = new Dialog();
    
    new_dialog->title = title;
    new_dialog->processCallback = process_callback;
    
    dialogs.push_back(new_dialog);
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
    const string &message, const string &page
) {
    helpDialogMessage = message;
    helpDialogPage = page;
    openDialog("Help", std::bind(&Editor::processGuiHelpDialog, this));
    dialogs.back()->customSize = Point(400, 0);
}


/**
 * @brief Opens an input popup with a given name. Its logic must be run with
 * a call to processGuiInputPopup().
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
 * @param close_callback Code to run when the dialog is closed, if any.
 */
void Editor::openMessageDialog(
    const string &title, const string &message,
    const std::function<void()> &close_callback
) {
    messageDialogMessage = message;
    openDialog(title, std::bind(&Editor::processGuiMessageDialog, this));
    dialogs.back()->customSize = Point(400, 0);
    dialogs.back()->closeCallback = close_callback;
}


/**
 * @brief Opens a dialog where the user can create a new pack.
 */
void Editor::openNewPackDialog() {
    needsNewPackTextFocus = true;
    openDialog(
        "Create a new pack",
        std::bind(&Editor::processGuiNewPackDialog, this)
    );
    dialogs.back()->customSize = Point(520, 0);
}


/**
 * @brief Opens a dialog with "picker" widgets inside, with the given content.
 *
 * @param title Title of the picker's dialog window.
 * This is normally a request to the user, like "Pick an area.".
 * @param items List of items to populate the picker with.
 * @param pick_callback A function to call when the user clicks an item
 * or enters a new one.
 * This function's first argument is the name of the item.
 * Its second argument is the top-level category of the item, or empty string.
 * Its third argument is the second-level category of the item, or empty string.
 * Its fourth argument is a void pointer to any custom info, or nullptr.
 * Its fifth argument is whether it's a new item or not.
 * @param list_header If not-empty, display this text above the list.
 * @param can_make_new If true, the user can create a new element,
 * by writing its name on the textbox, and pressing the "+" button.
 * @param use_monospace Whether the items should use a monospace font.
 * @param filter Filter of names. Only items that match this will appear.
 */
void Editor::openPickerDialog(
    const string &title,
    const vector<PickerItem> &items,
    const std::function <void(
        const string &, const string &, const string &, void*, bool
    )> &pick_callback,
    const string &list_header,
    bool can_make_new,
    bool use_monospace,
    const string &filter
) {
    Picker* new_picker = new Picker(this);
    
    new_picker->items = items;
    new_picker->listHeader = list_header;
    new_picker->pickCallback = pick_callback;
    new_picker->canMakeNew = can_make_new;
    new_picker->useMonospace = use_monospace;
    new_picker->filter = filter;
    
    Dialog* new_dialog = new Dialog();
    dialogs.push_back(new_dialog);
    
    new_dialog->title = title;
    new_dialog->processCallback =
        std::bind(&Editor::Picker::process, new_picker);
    new_dialog->closeCallback = [new_picker] () {
        delete new_picker;
    };
    new_picker->dialogPtr = new_dialog;
}


/**
 * @brief Creates widgets with the goal of placing a disabled text widget to the
 * right side of the panel.
 *
 * @param title Title to write.
 */
void Editor::panelTitle(const char* title) {
    ImGui::SameLine(
        ImGui::GetContentRegionAvail().x -
        (ImGui::CalcTextSize(title).x + 1)
    );
    ImGui::TextDisabled("%s", title);
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
        Dialog* d_ptr = dialogs[d];
        
        if(!d_ptr->isOpen) {
            if(d_ptr->closeCallback) {
                d_ptr->closeCallback();
            }
            delete d_ptr;
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
 * @brief Processes the base content editing warning dialog for this frame.
 */
void Editor::processGuiBaseContentWarningDialog() {
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
    ImGui::SetupCentering(148);
    if(ImGui::Button("Go back", ImVec2(70, 30))) {
        closeTopDialog();
    }
    
    //Continue button.
    ImGui::SameLine();
    if(ImGui::Button("Continue", ImVec2(70, 30))) {
        baseContentWarningDoPickCallback();
        baseContentWarningDoPickCallback = nullptr;
        closeTopDialog();
    }
    
    //Open manual button.
    ImGui::SetupCentering(100);
    if(ImGui::Button("Open manual", ImVec2(100, 25))) {
        openManual("making.html#packs");
    }
}


/**
 * @brief Processes the bitmap picker dialog for this frame.
 */
void Editor::processGuiBitmapDialog() {
    static bool filter_with_recommended_folder = true;
    
    //Fill the picker's items.
    bitmapDialogPicker.items.clear();
    for(auto &b : game.content.bitmaps.manifests) {
        if(
            !bitmapDialogRecommendedFolder.empty() &&
            filter_with_recommended_folder
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
    ImGui::SetupCentering(200);
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
    
    //Recommended folder text.
    string folder_str =
        bitmapDialogRecommendedFolder == "." ?
        "(root)" : bitmapDialogRecommendedFolder;
    ImGui::Spacer();
    ImGui::Text("Recommended folder: %s", folder_str.c_str());
    
    //Recommended folder only checkbox.
    if(!bitmapDialogRecommendedFolder.empty()) {
        ImGui::Checkbox(
            "That folder only", &filter_with_recommended_folder
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
        const int thumb_max_size = 300;
        Point size =
            resizeToBoxKeepingAspectRatio(
                getBitmapDimensions(bitmapDialogCurBmpPtr),
                Point(thumb_max_size)
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
 * @brief Processes the Dear ImGui widgets that let users select a hazard.
 *
 * @param selected_hazard_iname Internal name of the currently selected hazard.
 * @return Whether the hazard was changed.
 */
bool Editor::processGuiHazardManagementWidgets(string &selected_hazard_iname) {
    //Hazard combo.
    int selected_hazard_idx = -1;
    vector<string> all_hazard_inames = {""};
    vector<string> all_hazard_labels = {NONE_OPTION + "##(none)"};
    for(auto &h : game.content.hazards.list) {
        all_hazard_inames.push_back(h.first);
        all_hazard_labels.push_back(h.second.name + "##" + h.first);
        if(selected_hazard_iname == h.first) {
            selected_hazard_idx = all_hazard_labels.size() - 1;
        }
    }
    
    if(selected_hazard_idx == -1) selected_hazard_idx = 0;
    
    bool result =
        ImGui::Combo("Hazard", &selected_hazard_idx, all_hazard_labels);
        
    selected_hazard_iname = all_hazard_inames[selected_hazard_idx];
    
    return result;
}


/**
 * @brief Processes the help dialog widgets.
 */
void Editor::processGuiHelpDialog() {
    //Text.
    static int text_width = 0;
    if(text_width != 0) {
        ImGui::SetupCentering(text_width);
    }
    ImGui::TextWrapped("%s", helpDialogMessage.c_str());
    text_width = ImGui::GetItemRectSize().x;
    
    //Open manual button.
    ImGui::Spacer();
    ImGui::SetupCentering(200);
    if(ImGui::Button("Open manual", ImVec2(100, 40))) {
        openManual(helpDialogPage);
    }
    
    //Ok button.
    ImGui::SameLine();
    if(ImGui::Button("Ok", ImVec2(100, 40))) {
        closeTopDialog();
    }
}


/**
 * @brief Processes the widgets that show the editor's history.
 *
 * @param name_display_callback When an entry's name needs to be displayed as
 * button text, this function gets called with the entry name as an argument,
 * to determine what the final button text will be.
 * @param pick_callback Code to run when an entry is picked.
 * @param tooltip_callback Code to obtain an entry's tooltip with, if any.
 */
void Editor::processGuiHistory(
    const vector<pair<string, string> > &history,
    const std::function<string(const string &)> &name_display_callback,
    const std::function<void(const string &)> &pick_callback,
    const std::function<string(const string &)> &tooltip_callback
) {
    if(saveableTreeNode("load", "History")) {
    
        if(!history.empty() && !history[0].first.empty()) {
        
            size_t n_filled_entries = 0;
            for(size_t h = 0; h < history.size(); h++) {
                if(!history[h].first.empty()) n_filled_entries++;
            }
            
            for(size_t h = 0; h < history.size(); h++) {
                string path = history[h].first;
                if(path.empty()) continue;
                
                string name = history[h].second;
                if(name.empty()) name = history[h].first;
                name = name_display_callback(name);
                name = trimWithEllipsis(name, 16);
                
                //History entry button.
                const ImVec2 button_size(120, 24);
                if(ImGui::Button((name + "##" + i2s(h)).c_str(), button_size)) {
                    pick_callback(path);
                }
                if(tooltip_callback) {
                    setTooltip(tooltip_callback(path));
                }
                ImGui::SetupButtonWrapping(
                    button_size.x, (int) (h + 1), (int) n_filled_entries
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
 * @brief Processes a popup, if applicable, opened via openInputPopup(),
 * filling it with a text input for the user to type something in.
 *
 * @param label Name of the popup.
 * @param prompt What to prompt to the user. e.g.: "New name:"
 * @param text Pointer to the starting text, as well as the user's final text.
 * @param use_monospace Whether to use a monospace font.
 * @return Whether the user pressed Return or the Ok button.
 */
bool Editor::processGuiInputPopup(
    const char* label, const char* prompt, string* text, bool use_monospace
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        if(escapeWasPressed) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::Text("%s", prompt);
        ImGui::FocusOnInputText(needsInputPopupTextFocus);
        bool hit_enter = false;
        if(use_monospace) {
            hit_enter =
                monoInputText(
                    "##inputPopupText", text,
                    ImGuiInputTextFlags_EnterReturnsTrue |
                    ImGuiInputTextFlags_AutoSelectAll
                );
        } else {
            hit_enter =
                ImGui::InputText(
                    "##inputPopupText", text,
                    ImGuiInputTextFlags_EnterReturnsTrue |
                    ImGuiInputTextFlags_AutoSelectAll
                );
        }
        if(hit_enter) {
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
 * @brief Processes the Dear ImGui message dialog widgets.
 */
void Editor::processGuiMessageDialog() {
    //Text.
    static int text_width = 0;
    if(text_width != 0) {
        ImGui::SetupCentering(text_width);
    }
    ImGui::TextWrapped("%s", messageDialogMessage.c_str());
    text_width = ImGui::GetItemRectSize().x;
    
    //Ok button.
    ImGui::Spacer();
    ImGui::SetupCentering(100);
    if(ImGui::Button("Ok", ImVec2(100, 40))) {
        closeTopDialog();
    }
}


/**
 * @brief Processes the category and type widgets that allow a user to
 * select a mob type.
 *
 * @param custom_cat_name Pointer to the custom category name reflected
 * in the combo box.
 * @param type Pointer to the type reflected in the combo box.
 * @param pack_filter If not empty, only show mob types from this pack.
 * @return Whether the user changed the category/type.
 */
bool Editor::processGuiMobTypeWidgets(
    string* custom_cat_name, MobType** type, const string &pack_filter
) {
    bool result = false;
    
    //These are used to communicate with the picker dialog, since that one
    //is processed somewhere else entirely.
    static bool internal_changed_by_dialog = false;
    static string internal_custom_cat_name;
    static MobType* internal_mob_type = nullptr;
    
    if(internal_changed_by_dialog) {
        //Somewhere else in the code, the picker dialog changed these variables
        //to whatever the user picked. Let's use them now, instead of the
        //ones passed by the function's arguments.
        result = true;
        internal_changed_by_dialog = false;
    } else {
        //The picker dialog hasn't changed these variables. Just use
        //whatever the function's arguments state.
        internal_custom_cat_name = *custom_cat_name;
        internal_mob_type = *type;
    }
    
    //Column setup.
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(-1, 62.0f);
    
    //Search button.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 14.0f));
    bool search_button_pressed =
        ImGui::ImageButton(
            "searchButton", editorIcons[EDITOR_ICON_SEARCH],
            Point(EDITOR::ICON_BMP_SIZE)
        );
    ImGui::PopStyleVar();
    
    vector<vector<MobType*> > final_list;
    if(!pack_filter.empty()) {
        for(size_t c = 0; c < customCatTypes.size(); c++) {
            final_list.push_back(vector<MobType*>());
            for(size_t n = 0; n < customCatTypes[c].size(); n++) {
                MobType* mt_ptr = customCatTypes[c][n];
                if(mt_ptr->manifest && mt_ptr->manifest->pack == pack_filter) {
                    final_list[c].push_back(mt_ptr);
                }
            }
        }
    } else {
        final_list = customCatTypes;
    }
    
    if(search_button_pressed) {
        vector<PickerItem> items;
        for(size_t c = 0; c < final_list.size(); c++) {
            for(size_t n = 0; n < final_list[c].size(); n++) {
                MobType* mt_ptr = final_list[c][n];
                items.push_back(
                    PickerItem(
                        mt_ptr->name, mt_ptr->customCategoryName
                    )
                );
            }
        }
        openPickerDialog(
            "Pick an object type", items,
            [this, final_list] (
                const string &n, const string &tc, const string &sc, void*, bool
        ) {
            //For clarity, this code will NOT be run within the context
            //of editor::processGuiMobTypeWidgets, but will instead
            //be run wherever dialogs are processed.
            internal_changed_by_dialog = true;
            internal_custom_cat_name = tc;
            internal_mob_type = nullptr;
            size_t custom_cat_idx = customCatNameIdxs[tc];
            const vector<MobType*> &types =
                final_list[custom_cat_idx];
            for(size_t t = 0; t < types.size(); t++) {
                if(types[t]->name == n) {
                    internal_mob_type = types[t];
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
    int selected_category_idx = -1;
    for(size_t c = 0; c < final_list.size(); c++) {
        string cn =
            customCatTypes[c].front()->customCategoryName;
        categories.push_back(cn);
        if(cn == internal_custom_cat_name) {
            selected_category_idx = (int) c;
        }
    }
    
    if(ImGui::Combo("Category", &selected_category_idx, categories, 15)) {
        result = true;
        internal_custom_cat_name = categories[selected_category_idx];
        internal_mob_type =
            final_list[selected_category_idx].empty() ?
            nullptr :
            final_list[selected_category_idx][0];
    }
    setTooltip(
        "What category this object belongs to: a Pikmin, a leader, etc."
    );
    
    if(!internal_custom_cat_name.empty()) {
    
        //Object type combobox.
        vector<string> type_names;
        size_t custom_cat_idx = customCatNameIdxs[internal_custom_cat_name];
        const vector<MobType*> &types = final_list[custom_cat_idx];
        for(size_t t = 0; t < types.size(); t++) {
            MobType* t_ptr = types[t];
            type_names.push_back(t_ptr->name);
        }
        
        string selected_type_name;
        if(internal_mob_type) {
            selected_type_name = internal_mob_type->name;
        }
        if(ImGui::Combo("Type", &selected_type_name, type_names, 15)) {
            result = true;
            for(size_t t = 0; t < types.size(); t++) {
                if(types[t]->name == selected_type_name) {
                    internal_mob_type = types[t];
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
        *custom_cat_name = internal_custom_cat_name;
        *type = internal_mob_type;
    }
    
    return result;
}


/**
 * @brief Processes the widgets for the pack selection, in a "new" dialog.
 *
 * @param pack Pointer to the internal name of the pack in the combobox.
 */
bool Editor::processGuiNewDialogPackWidgets(string* pack) {
    bool changed = false;
    
    //Pack combo.
    vector<string> packs;
    for(const auto &p : game.content.packs.manifestsWithBase) {
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
 * @brief Processes the dialog for creating a new pack.
 */
void Editor::processGuiNewPackDialog() {
    static string internal_name = "my_pack";
    static string name = "My pack!";
    static string description;
    static string maker;
    string problem;
    bool hit_create_button = false;
    
    //Internal name input.
    ImGui::FocusOnInputText(needsNewPackTextFocus);
    if(
        monoInputText(
            "Internal name", &internal_name,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hit_create_button = true;
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
        hit_create_button = true;
    }
    setTooltip("Proper name of the new pack.");
    
    //Description input.
    if(
        ImGui::InputText(
            "Description", &description,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hit_create_button = true;
    }
    setTooltip("A description of the pack.");
    
    //Maker input.
    if(
        ImGui::InputText(
            "Maker", &maker,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hit_create_button = true;
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
    string path_to_show =
        internal_name.empty() ?
        "" :
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
        internal_name + "/" +
        FILE_NAMES::PACK_DATA;
    monoText("%s", path_to_show.c_str());
    
    //Open manual button.
    if(ImGui::Button("Open manual")) {
        openManual("making.html#packs");
    }
    
    //Check if everything's ok.
    if(internal_name.empty()) {
        problem = "You have to type an internal name first!";
    } else if(!isInternalNameGood(internal_name)) {
        problem =
            "The internal name should only have lowercase letters,\n"
            "numbers, and underscores!";
    } else {
        for(const auto &p : game.content.packs.manifestsWithBase) {
            if(internal_name == p) {
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
    ImGui::SetupCentering(100);
    if(!problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create pack", ImVec2(100, 40))) {
        hit_create_button = true;
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    setTooltip(
        problem.empty() ? "Create the pack!" : problem
    );
    
    //Creation logic.
    if(hit_create_button) {
        if(!problem.empty()) return;
        game.content.createPack(
            internal_name, name, description, maker
        );
        for(size_t p = 0; p < game.content.packs.manifestsWithBase.size(); p++) {
            if(game.content.packs.manifestsWithBase[p] == internal_name) {
                newContentDialogPackIdx = p;
                break;
            }
        }
        internal_name.clear();
        name.clear();
        description.clear();
        maker.clear();
        closeTopDialog();
    }
}


/**
 * @brief Process the width and height widgets that allow a user to
 * specify the size of something.
 *
 * @param label Label for the widgets.
 * @param size Size variable to alter.
 * @param v_speed Variable change speed. Same value you'd pass to
 * ImGui::DragFloat2. 1.0f for default.
 * @param keep_aspect_ratio If true, changing one will change the other
 * in the same ratio.
 * @param keep_area If true, changing one will change the other
 * such that the total area is preserved.
 * @param min_size Minimum value that either width or height is allowed
 * to have. Use -FLT_MAX for none.
 * @return Whether the user changed one of the values.
 */
bool Editor::processGuiSizeWidgets(
    const char* label, Point &size, float v_speed,
    bool keep_aspect_ratio,
    bool keep_area,
    float min_size
) {
    bool ret = false;
    Point new_size = size;
    if(
        ImGui::DragFloat2(
            label, (float*) &new_size, v_speed, min_size, FLT_MAX
        )
    ) {
    
        bool free_resize = !keep_aspect_ratio && !keep_area;
        bool values_valid =
            size.x != 0.0f && size.y != 0.0f &&
            new_size.x != 0.0f && new_size.y != 0.0f;
            
        if(free_resize || !values_valid) {
            //Just change them, forget about keeping the aspect ratio or area.
            new_size.x = std::max(min_size, new_size.x);
            new_size.y = std::max(min_size, new_size.y);
            
        } else if(keep_aspect_ratio) {
            //Keep the aspect ratio.
            float ratio = size.x / size.y;
            if(new_size.x != size.x) {
                //Must adjust Y.
                if(min_size != -FLT_MAX) {
                    new_size.x = std::max(min_size * ratio, new_size.x);
                    new_size.x = std::max(min_size, new_size.x);
                }
                new_size.y = new_size.x / ratio;
            } else {
                //Must adjust X.
                if(min_size != -FLT_MAX) {
                    new_size.y = std::max(min_size / ratio, new_size.y);
                    new_size.y = std::max(min_size, new_size.y);
                }
                new_size.x = new_size.y * ratio;
            }
            
        } else {
            //Keep the area.
            double area = (double) size.x * (double) size.y;
            if(new_size.x != size.x) {
                //Must adjust Y.
                if(min_size != -FLT_MAX) {
                    new_size.x = std::max(min_size, new_size.x);
                }
                new_size.y = area / new_size.x;
            } else {
                //Must adjust X.
                if(min_size != -FLT_MAX) {
                    new_size.y = std::max(min_size, new_size.y);
                }
                new_size.x = area / new_size.y;
            }
            
        }
        
        size = new_size;
        ret = true;
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
    float error_flash_time_ratio = opErrorFlashTimer.getRatioLeft();
    if(error_flash_time_ratio > 0.0f) {
        ImVec4 normal_color_v = ImGui::GetStyle().Colors[ImGuiCol_Text];
        ALLEGRO_COLOR normal_color;
        normal_color.r = normal_color_v.x;
        normal_color.g = normal_color_v.y;
        normal_color.b = normal_color_v.z;
        normal_color.a = normal_color_v.w;
        ALLEGRO_COLOR error_flash_color =
            interpolateColor(
                error_flash_time_ratio,
                0.0f, 1.0f,
                normal_color, al_map_rgb(255, 0, 0)
            );
        ImVec4 error_flash_color_v;
        error_flash_color_v.x = error_flash_color.r;
        error_flash_color_v.y = error_flash_color.g;
        error_flash_color_v.z = error_flash_color.b;
        error_flash_color_v.w = error_flash_color.a;
        ImGui::PushStyleColor(ImGuiCol_Text, error_flash_color_v);
    }
    ImGui::Text("%s", (statusText.empty() ? "Ready." : statusText.c_str()));
    if(error_flash_time_ratio) {
        ImGui::PopStyleColor();
    }
}


/**
 * @brief Processes the Dear ImGui unsaved changes confirmation dialog
 * for this frame.
 */
void Editor::processGuiUnsavedChangesDialog() {
    //Explanation 1 text.
    size_t nr_unsaved_changes = changesMgr.getUnsavedChanges();
    string explanation1_str =
        "You have " +
        amountStr((int) nr_unsaved_changes, "unsaved change") +
        ", made in the last " +
        timeToStr3(
            changesMgr.getUnsavedTimeDelta(),
            "h", "m", "s",
            TIME_TO_STR_FLAG_NO_LEADING_ZEROS |
            TIME_TO_STR_FLAG_NO_LEADING_ZERO_PORTIONS
        ) +
        ".";
    ImGui::SetupCentering(ImGui::CalcTextSize(explanation1_str.c_str()).x);
    ImGui::Text("%s", explanation1_str.c_str());
    
    //Explanation 3 text.
    string explanation2_str =
        "Do you want to save before " +
        changesMgr.getUnsavedWarningActionLong() + "?";
    ImGui::SetupCentering(ImGui::CalcTextSize(explanation2_str.c_str()).x);
    ImGui::Text("%s", explanation2_str.c_str());
    
    //Cancel button.
    ImGui::SetupCentering(180 + 180 + 180 + 20);
    if(ImGui::Button("Cancel", ImVec2(180, 30))) {
        closeTopDialog();
    }
    setTooltip("Never mind and go back.", "Esc");
    
    //Save and then perform the action.
    ImGui::SameLine(0.0f, 10);
    if(ImGui::Button("Save", ImVec2(180, 30))) {
        closeTopDialog();
        const std::function<bool()> &save_callback =
            changesMgr.getUnsavedWarningSaveCallback();
        const std::function<void()> &action_callback =
            changesMgr.getUnsavedWarningActionCallback();
        if(save_callback()) {
            action_callback();
        }
    }
    setTooltip(
        "Save first, then " +
        changesMgr.getUnsavedWarningActionShort() + ".",
        "Ctrl + S"
    );
    
    //Perform the action without saving button.
    ImGui::SameLine(0.0f, 10);
    if(ImGui::Button("Don't save", ImVec2(180, 30))) {
        closeTopDialog();
        const std::function<void()> action_callback =
            changesMgr.getUnsavedWarningActionCallback();
        action_callback();
    }
    string dont_save_tooltip =
        changesMgr.getUnsavedWarningActionShort() +
        " without saving.";
    dont_save_tooltip[0] = toupper(dont_save_tooltip[0]);
    setTooltip(dont_save_tooltip, "Ctrl + D");
}


/**
 * @brief Processes an ImGui::TreeNode, except it pre-emptively opens it or
 * closes it based on the user's preferences.
 *
 * It also saves the user's preferences as they open and close the node.
 * In order for these preferences to be saved onto disk, save_options must
 * be called.
 *
 * @param category Category this node belongs to.
 * This is just a generic term, and you likely want to use the panel
 * this node belongs to.
 * @param label Label to give to Dear ImGui.
 * @return Whether the node is open.
 */
bool Editor::saveableTreeNode(const string &category, const string &label) {
    string node_name = getName() + "/" + category + "/" + label;
    ImGui::SetNextItemOpen(game.options.editors.openNodes[node_name]);
    ImGui::PushFont(game.sysContent.fntDearImGuiHeader);
    bool is_open = ImGui::TreeNode(label.c_str());
    ImGui::PopFont();
    game.options.editors.openNodes[node_name] = is_open;
    return is_open;
}


/**
 * @brief Sets the status bar, and notifies the user of an error,
 * if it is an error, by flashing the text.
 *
 * @param text Text to put in the status bar.
 * @param error Whether there was an error or not.
 */
void Editor::setStatus(const string &text, bool error) {
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
 * @param widget_explanation If the way the widget works needs to be explained,
 * specify the explanation type here.
 */
void Editor::setTooltip(
    const string &explanation, const string &shortcut,
    const WIDGET_EXPLANATION widget_explanation
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
            
            string widget_explanation_text;
            switch(widget_explanation) {
            case WIDGET_EXPLANATION_NONE: {
                break;
            }
            case WIDGET_EXPLANATION_DRAG: {
                widget_explanation_text =
                    "Click and drag left or right to change.\n"
                    "Hold Alt or Shift to change speed.\n"
                    "Click once or Ctrl + click to write a value.";
                break;
            }
            case WIDGET_EXPLANATION_SLIDER: {
                widget_explanation_text =
                    "Click and/or drag left or right to change.\n"
                    "Ctrl + click to write a value.";
                break;
            }
            }
            
            if(!widget_explanation_text.empty()) {
                ImGui::TextColored(
                    ImVec4(0.50f, 0.50f, 0.50f, 1.0f),
                    "%s", widget_explanation_text.c_str()
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
Point Editor::snapPointToAxis(const Point &p, const Point &anchor) {
    float h_diff = fabs(p.x - anchor.x);
    float v_diff = fabs(p.y - anchor.y);
    if(h_diff > v_diff) {
        return Point(p.x, anchor.y);
    } else {
        return Point(anchor.x, p.y);
    }
}


/**
 * @brief Snaps a point to the nearest grid intersection.
 *
 * @param p Point to snap.
 * @param grid_interval Current grid interval.
 * @return The snapped point.
 */
Point Editor::snapPointToGrid(const Point &p, float grid_interval) {
    return
        Point(
            round(p.x / grid_interval) * grid_interval,
            round(p.y / grid_interval) * grid_interval
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
    game.mouseCursor.hide();
}


/**
 * @brief Updates the history list, by adding a new entry or bumping it up.
 *
 * @param manifest Manifest of the entry's content.
 * @param name Proper name of the entry.
 */
void Editor::updateHistory(
    vector<pair<string, string> > &history,
    const ContentManifest &manifest, const string &name
) {
    string final_name = name.empty() ? manifest.internalName : name;
    
    //First, check if it exists.
    size_t pos = INVALID;
    
    for(size_t h = 0; h < history.size(); h++) {
        if(history[h].first == manifest.path) {
            pos = h;
            break;
        }
    }
    
    if(pos == 0) {
        //Already #1? Just update the name.
        history[0].second = final_name;
    } else if(pos == INVALID) {
        //If it doesn't exist, create it and add it to the top.
        history.insert(
            history.begin(),
            make_pair(manifest.path, final_name)
        );
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        history.erase(history.begin() + pos);
        history.insert(
            history.begin(),
            make_pair(manifest.path, final_name)
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
            game.DearImGuiDefaultStyle,
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
 * @brief Updates the transformations, with the current camera coordinates,
 * zoom, etc.
 */
void Editor::updateTransformations() {
    //World coordinates to window coordinates.
    Point canvas_center(
        (canvasTL.x + canvasBR.x) / 2.0,
        (canvasTL.y + canvasBR.y) / 2.0
    );
    game.worldToWindowTransform = game.identityTransform;
    al_translate_transform(
        &game.worldToWindowTransform,
        -game.cam.pos.x + canvas_center.x / game.cam.zoom,
        -game.cam.pos.y + canvas_center.y / game.cam.zoom
    );
    al_scale_transform(
        &game.worldToWindowTransform, game.cam.zoom, game.cam.zoom
    );
    
    //Window coordinates to world coordinates.
    game.windowToWorldTransform = game.worldToWindowTransform;
    al_invert_transform(&game.windowToWorldTransform);
}


/**
 * @brief Zooms to the specified level, keeping the mouse cursor in
 * the same spot.
 *
 * @param new_zoom New zoom level.
 */
void Editor::zoomWithCursor(float new_zoom) {
    //Keep a backup of the old mouse coordinates.
    Point old_mouse_pos = game.mouseCursor.worldPos;
    
    //Do the zoom.
    game.cam.set_zoom(std::clamp(new_zoom, zoomMinLevel, zoomMaxLevel));
    updateTransformations();
    
    //Figure out where the mouse will be after the zoom.
    game.mouseCursor.worldPos = game.mouseCursor.winPos;
    al_transform_coordinates(
        &game.windowToWorldTransform,
        &game.mouseCursor.worldPos.x, &game.mouseCursor.worldPos.y
    );
    
    //Readjust the transformation by shifting the camera
    //so that the cursor ends up where it was before.
    game.cam.setPos(
        Point(
            game.cam.pos.x += (old_mouse_pos.x - game.mouseCursor.worldPos.x),
            game.cam.pos.y += (old_mouse_pos.y - game.mouseCursor.worldPos.y)
        )
    );
    
    //Update the mouse coordinates again.
    updateTransformations();
    game.mouseCursor.worldPos = game.mouseCursor.winPos;
    al_transform_coordinates(
        &game.windowToWorldTransform,
        &game.mouseCursor.worldPos.x, &game.mouseCursor.worldPos.y
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
 * @param action_long String representing the action the user is attempting
 * in a long format. This is for the main prompt of the warning dialog,
 * so it can be as long as you want. It should start with a lowercase.
 * @param action_short String representing the action the user is attempting
 * in a short format. This is for the buttons of the warning dialog,
 * so it should ideally be only one word. It should start with a lowercase.
 * @param action_callback Code to run to perform the action.
 * @param save_callback Code to run when the unsaved changes must be saved.
 * @return Whether there were unsaved changes.
 */
bool Editor::ChangesManager::askIfUnsaved(
    const Point &pos,
    const string &action_long, const string &action_short,
    const std::function<void()> &action_callback,
    const std::function<bool()> &save_callback
) {
    if(unsavedChanges > 0) {
        unsavedWarningActionLong = action_long;
        unsavedWarningActionShort = action_short;
        unsavedWarningActionCallback = action_callback;
        unsavedWarningSaveCallback = save_callback;
        
        ed->openDialog(
            "Unsaved changes!",
            std::bind(&Editor::processGuiUnsavedChangesDialog, ed)
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
                    const std::function<bool()> &save_callback =
                        this->getUnsavedWarningSaveCallback();
                    const std::function<void()> &action_callback =
                        this->getUnsavedWarningActionCallback();
                    if(save_callback()) {
                        action_callback();
                    }
                } else if(
                    ed->keyCheck(ev->keyboard.keycode, ALLEGRO_KEY_D, true)
                ) {
                    ed->closeTopDialog();
                    const std::function<void()> &action_callback =
                        this->getUnsavedWarningActionCallback();
                    action_callback();
                }
            }
        };
        
        return true;
        
    } else {
    
        action_callback();
        
        return false;
    }
}


/**
 * @brief Returns whether the content exists on the disk.
 *
 * @return Whether it exists.
 */
bool Editor::ChangesManager::existsOnDisk() const {
    return onDisk;
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
 * @brief Returns the current unsaved changes warning long action text.
 *
 * @return The text.
 */
const string &Editor::ChangesManager::getUnsavedWarningActionLong()
const {
    return unsavedWarningActionLong;
}


/**
 * @brief Returns the current unsaved changes warning short action text.
 *
 * @return The text.
 */
const string &Editor::ChangesManager::getUnsavedWarningActionShort()
const {
    return unsavedWarningActionShort;
}


/**
 * @brief Returns the current unsaved changes warning action callback.
 *
 * @return The callback.
 */
const std::function<void()> &
Editor::ChangesManager::getUnsavedWarningActionCallback() const {
    return unsavedWarningActionCallback;
}


/**
 * @brief Returns the current unsaved changes warning save callback.
 *
 * @return The callback.
 */
const std::function<bool()> &
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
 * @brief Marks the state of the editor's file as not existing on disk yet.
 * This also marks it as having unsaved changes.
 */
void Editor::ChangesManager::markAsNonExistent() {
    onDisk = false;
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
    onDisk = true;
}


/**
 * @brief Resets the state of the changes manager.
 */
void Editor::ChangesManager::reset() {
    unsavedChanges = 0;
    unsavedTime = 0.0f;
    onDisk = true;
}


/**
 * @brief Constructs a new command object.
 *
 * @param f Function to run.
 * @param n Name.
 */
Editor::Command::Command(command_func_t f, const string &n) :
    func(f),
    name(n) {
}


/**
 * @brief Runs the function.
 *
 * @param input_value Input value, if needed by the command.
 */
void Editor::Command::run(float input_value) {
    func(input_value);
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
    Point tl = pos - size / 2.0f;
    Point br = pos + size / 2.0f;
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
 * @param editor_ptr Pointer to the editor in charge.
 */
Editor::Picker::Picker(Editor* editor_ptr) :
    editorPtr(editor_ptr) {
}


/**
 * @brief Processes the picker for this frame.
 */
void Editor::Picker::process() {
    vector<string> top_cat_names;
    vector<vector<string> > sec_cat_names;
    vector<vector<vector<PickerItem> > > final_items;
    string filter_lower = strToLower(filter);
    
    //Figure out the items.
    for(size_t i = 0; i < items.size(); i++) {
        if(!filter.empty()) {
            string name_lower = strToLower(items[i].name);
            if(name_lower.find(filter_lower) == string::npos) {
                continue;
            }
        }
        
        size_t top_cat_idx = INVALID;
        for(size_t c = 0; c < top_cat_names.size(); c++) {
            if(top_cat_names[c] == items[i].topCategory) {
                top_cat_idx = c;
                break;
            }
        }
        
        if(top_cat_idx == INVALID) {
            top_cat_names.push_back(items[i].topCategory);
            sec_cat_names.push_back(vector<string>());
            final_items.push_back(vector<vector<PickerItem> >());
            top_cat_idx = top_cat_names.size() - 1;
        }
        
        size_t sec_cat_idx = INVALID;
        for(size_t c = 0; c < sec_cat_names[top_cat_idx].size(); c++) {
            if(sec_cat_names[top_cat_idx][c] == items[i].secCategory) {
                sec_cat_idx = c;
                break;
            }
        }
        
        if(sec_cat_idx == INVALID) {
            sec_cat_names[top_cat_idx].push_back(items[i].secCategory);
            final_items[top_cat_idx].push_back(vector<PickerItem>());
            sec_cat_idx = sec_cat_names[top_cat_idx].size() - 1;
        }
        
        final_items[top_cat_idx][sec_cat_idx].push_back(items[i]);
    }
    
    //Stuff for creating a new item.
    auto try_make_new = [this] () {
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
        
        bool is_really_new = true;
        for(size_t i = 0; i < items.size(); i++) {
            if(
                filter == items[i].name &&
                newItemTopCat == items[i].topCategory
            ) {
                is_really_new = false;
                break;
            }
        }
        
        pickCallback(filter, newItemTopCat, "", nullptr, is_really_new);
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
        bool hit_create_button = ImGui::Button("+", ImVec2(64.0f, 0.0f));
        editorPtr->setTooltip("Create a new item with the given name!");
        if(hit_create_button) try_make_new();
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
    }
    
    //Search filter input.
    string filter_widget_hint =
        canMakeNew ?
        "Search filter or new item name" :
        "Search filter";
        
    ImGui::FocusOnInputText(needsFilterBoxFocus);
    bool hit_filter_widget = false;
    if(filter.empty()) {
        hit_filter_widget =
            ImGui::InputTextWithHint(
                "##filter", filter_widget_hint.c_str(), &filter,
                ImGuiInputTextFlags_EnterReturnsTrue
            );
    } else {
        hit_filter_widget =
            monoInputTextWithHint(
                "##filter", filter_widget_hint.c_str(), &filter,
                ImGuiInputTextFlags_EnterReturnsTrue
            );
    }
    
    if(hit_filter_widget) {
        if(filter.empty()) return;
        
        if(canMakeNew) {
            try_make_new();
        } else {
            size_t possible_choices = 0;
            for(size_t c = 0; c < final_items.size(); c++) {
                possible_choices += final_items[c].size();
            }
            if(possible_choices > 0) {
                pickCallback(
                    final_items[0][0][0].name,
                    final_items[0][0][0].topCategory,
                    final_items[0][0][0].secCategory,
                    final_items[0][0][0].info,
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
            for(size_t c = 0; c < newItemTopCatChoices.size(); c++) {
                //Item selectable.
                if(ImGui::Selectable(newItemTopCatChoices[c].c_str())) {
                    newItemTopCat = newItemTopCatChoices[c];
                    ImGui::CloseCurrentPopup();
                    try_make_new();
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
    
    for(size_t tc = 0; tc < final_items.size(); tc++) {
    
        bool top_cat_opened = true;
        if(!top_cat_names[tc].empty()) {
            //Top category node.
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            top_cat_opened = ImGui::TreeNode(top_cat_names[tc].c_str());
        }
        
        if(!top_cat_opened) continue;
        
        for(size_t sc = 0; sc < final_items[tc].size(); sc++) {
        
            bool sec_cat_opened = true;
            if(!sec_cat_names[tc][sc].empty()) {
                //Secondary category node.
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                sec_cat_opened = ImGui::TreeNode(sec_cat_names[tc][sc].c_str());
            }
            
            if(!sec_cat_opened) continue;
            
            for(size_t i = 0; i < final_items[tc][sc].size(); i++) {
                PickerItem* i_ptr = &final_items[tc][sc][i];
                string widgetId = i2s(tc) + "-" + i2s(sc) + "-" + i2s(i);
                ImGui::PushID(widgetId.c_str());
                if(useMonospace) {
                    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
                }
                
                Point button_size;
                
                if(i_ptr->bitmap) {
                
                    ImGui::BeginGroup();
                    
                    //Item image button.
                    button_size = Point(EDITOR::PICKER_IMG_BUTTON_SIZE);
                    bool button_pressed =
                        ImGui::ImageButtonOrganized(
                            widgetId + "Button",
                            i_ptr->bitmap,
                            button_size - 4.0f, button_size
                        );
                        
                    if(button_pressed) {
                        pickCallback(
                            i_ptr->name, i_ptr->topCategory, i_ptr->secCategory, i_ptr->info, false
                        );
                        if(dialogPtr) {
                            dialogPtr->isOpen = false;
                        }
                    }
                    
                    //Item name text.
                    string display_name =
                        trimWithEllipsis(
                            getPathLastComponent(i_ptr->name), 18
                        );
                    ImGui::TextWrapped("%s", display_name.c_str());
                    
                    //Item spacer widget.
                    ImGui::Dummy(ImVec2(0.0f, 8.0f));
                    ImGui::EndGroup();
                    
                } else {
                
                    //Item button.
                    button_size = Point(EDITOR::PICKER_IMG_BUTTON_SIZE, 32.0f);
                    if(ImGui::Button(i_ptr->name.c_str(), ImVec2(button_size.x, button_size.y))) {
                        pickCallback(
                            i_ptr->name, i_ptr->topCategory, i_ptr->secCategory, i_ptr->info, false
                        );
                        if(dialogPtr) {
                            dialogPtr->isOpen = false;
                        }
                    }
                    
                }
                
                if(!i_ptr->tooltip.empty()) {
                    editorPtr->setTooltip(i_ptr->tooltip);
                }
                
                ImGui::SetupButtonWrapping(
                    button_size.x, (int) (i + 1), (int) final_items[tc][sc].size()
                );
                
                if(useMonospace) ImGui::PopFont();
                ImGui::PopID();
            }
            
            if(!sec_cat_names[tc][sc].empty() && sec_cat_opened) {
                ImGui::TreePop();
            }
        }
        
        if(!top_cat_names[tc].empty() && top_cat_opened) {
            ImGui::TreePop();
        }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Constructs a new picker item object.
 *
 * @param name Name of the item.
 * @param top_category Top-level category it belongs to.
 * If none, use an empty string.
 * @param sec_category Second-level category it belongs to.
 * If none, use an empty string.
 * @param info Information to pass to the code when the item is picked, if any.
 * @param tooltip Tooltip, if any.
 * @param bitmap Bitmap to display on the item. If none, use nullptr.
 */
Editor::PickerItem::PickerItem(
    const string &name, const string &top_category, const string &sec_category,
    void* info, const string &tooltip, ALLEGRO_BITMAP* bitmap
) :
    name(name),
    topCategory(top_category),
    secCategory(sec_category),
    info(info),
    tooltip(tooltip),
    bitmap(bitmap) {
    
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
    if(!center) return;
    
    Point handles[9];
    float radius;
    getLocations(center, size, angle, handles, &radius, nullptr);
    
    //Draw the rotation handle.
    if(angle && radius >= 0.0f) {
        al_draw_circle(
            center->x, center->y, radius,
            al_map_rgb(64, 64, 192), EDITOR::TW_ROTATION_HANDLE_THICKNESS * zoom
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
            al_map_rgb(32, 32, 160), EDITOR::TW_OUTLINE_THICKNESS * zoom
        );
    }
    
    //Draw the translation and scale handles.
    for(unsigned char h = 0; h < 9; h++) {
        if(!size && h != 4) continue;
        al_draw_filled_circle(
            handles[h].x, handles[h].y,
            EDITOR::TW_HANDLE_RADIUS * zoom, al_map_rgb(96, 96, 224)
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
 * @param out_transform If not nullptr, the transformation used is
 * returned here.
 * The transformation will only rotate and translate, not scale.
 */
void Editor::TransformationWidget::getLocations(
    const Point* const center, const Point* const size,
    const float* const angle, Point* handles, float* radius,
    ALLEGRO_TRANSFORM* out_transform
) const {
    Point size_to_use(EDITOR::TW_DEF_SIZE, EDITOR::TW_DEF_SIZE);
    if(size) size_to_use = *size;
    
    //First, the Allegro transformation.
    ALLEGRO_TRANSFORM transform_to_use;
    al_identity_transform(&transform_to_use);
    if(angle) {
        al_rotate_transform(&transform_to_use, *angle);
    }
    al_translate_transform(&transform_to_use, center->x, center->y);
    
    //Get the coordinates of all translation and scale handles.
    handles[0] = { -size_to_use.x / 2.0f, -size_to_use.y / 2.0f };
    handles[1] = { 0.0f,                  -size_to_use.y / 2.0f };
    handles[2] = { size_to_use.x / 2.0f,  -size_to_use.y / 2.0f };
    handles[3] = { -size_to_use.x / 2.0f, 0.0f                  };
    handles[4] = { 0.0f,                  0.0f                  };
    handles[5] = { size_to_use.x / 2.0f,  0.0f                  };
    handles[6] = { -size_to_use.x / 2.0f, size_to_use.y / 2.0f  };
    handles[7] = { 0.0f,                  size_to_use.y / 2.0f  };
    handles[8] = { size_to_use.x / 2.0f,  size_to_use.y / 2.0f  };
    
    for(unsigned char h = 0; h < 9; h++) {
        al_transform_coordinates(
            &transform_to_use, &handles[h].x, &handles[h].y
        );
    }
    
    float diameter = Distance(Point(), size_to_use).toFloat();
    if(diameter == 0.0f) {
        *radius = 0.0f;
    } else {
        *radius = diameter / 2.0f;
    }
    
    if(out_transform) *out_transform = transform_to_use;
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
 * @param mouse_coords Mouse coordinates.
 * @param center Center point.
 * @param size Width and height. If nullptr, no scale handling will be performed.
 * @param angle Angle. If nullptr, no rotation handling will be performed.
 * @param zoom Zoom the widget's components by this much.
 * @return Whether the user clicked on a handle.
 */
bool Editor::TransformationWidget::handleMouseDown(
    const Point &mouse_coords, const Point* const center,
    const Point* const size, const float* const angle, float zoom
) {
    if(!center) return false;
    
    Point handles[9];
    float radius;
    getLocations(center, size, angle, handles, &radius, nullptr);
    
    //Check if the user clicked on a translation or scale handle.
    for(unsigned char h = 0; h < 9; h++) {
        if(Distance(handles[h], mouse_coords) <= EDITOR::TW_HANDLE_RADIUS * zoom) {
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
        Distance d(*center, mouse_coords);
        if(
            d >= radius - EDITOR::TW_ROTATION_HANDLE_THICKNESS / 2.0f * zoom &&
            d <= radius + EDITOR::TW_ROTATION_HANDLE_THICKNESS / 2.0f * zoom
        ) {
            movingHandle = 9;
            oldAngle = *angle;
            oldMouseAngle = getAngle(*center, mouse_coords);
            return true;
        }
    }
    
    return false;
}


/**
 * @brief Handles the user having moved the mouse cursor.
 *
 * @param mouse_coords Mouse coordinates.
 * @param center Center point.
 * @param size Width and height. If nullptr, no scale handling will be performed.
 * @param angle Angle. If nullptr, no rotation handling will be performed.
 * @param zoom Zoom the widget's components by this much.
 * @param keep_aspect_ratio If true, aspect ratio is kept when resizing.
 * @param keep_area If true, keep the same total area.
 * Used for squash and stretch.
 * @param min_size Minimum possible size for the width or height.
 * Use -FLT_MAX for none.
 * @param lock_center If true, scaling happens with the center locked.
 * If false, the opposite edge or corner is locked instead.
 * @return Whether the user is dragging a handle.
 */
bool Editor::TransformationWidget::handleMouseMove(
    const Point &mouse_coords, Point* center, Point* size, float* angle,
    float zoom, bool keep_aspect_ratio, bool keep_area,
    float min_size, bool lock_center
) {
    if(!center) return false;
    
    if(movingHandle == -1) {
        return false;
    }
    
    //Logic for moving the center handle.
    if(movingHandle == 4) {
        *center = mouse_coords;
        return true;
    }
    
    //Logic for moving the rotation handle.
    if(movingHandle == 9 && angle) {
        *angle =
            oldAngle +
            getAngle(*center, mouse_coords) - oldMouseAngle;
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
    
    Point transformed_mouse = mouse_coords;
    Point transformed_center = *center;
    Point new_size = oldSize;
    al_transform_coordinates(&t, &transformed_mouse.x, &transformed_mouse.y);
    al_transform_coordinates(&t, &transformed_center.x, &transformed_center.y);
    bool scaling_x = false;
    bool scaling_y = false;
    
    switch(movingHandle) {
    case 0:
    case 3:
    case 6: {
        new_size.x = size->x / 2.0f - transformed_mouse.x;
        scaling_x = true;
        break;
    } case 2:
    case 5:
    case 8: {
        new_size.x = transformed_mouse.x - (-size->x / 2.0f);
        scaling_x = true;
        break;
    }
    }
    
    switch(movingHandle) {
    case 0:
    case 1:
    case 2: {
        new_size.y = (size->y / 2.0f) - transformed_mouse.y;
        scaling_y = true;
        break;
    } case 6:
    case 7:
    case 8: {
        new_size.y = transformed_mouse.y - (-size->y / 2.0f);
        scaling_y = true;
        break;
    }
    }
    
    new_size.x = std::max(min_size, new_size.x);
    new_size.y = std::max(min_size, new_size.y);
    
    if(keep_aspect_ratio && oldSize.x != 0.0f && oldSize.y != 0.0f) {
        float scale_to_use;
        float w_scale = new_size.x / oldSize.x;
        float h_scale = new_size.y / oldSize.y;
        if(!scaling_y) {
            scale_to_use = w_scale;
        } else if(!scaling_x) {
            scale_to_use = h_scale;
        } else {
            if(fabs(w_scale) > fabs(h_scale)) {
                scale_to_use = w_scale;
            } else {
                scale_to_use = h_scale;
            }
        }
        scale_to_use = std::max(min_size / oldSize.x, scale_to_use);
        scale_to_use = std::max(min_size / oldSize.y, scale_to_use);
        new_size = oldSize * scale_to_use;
        
    } else if(keep_area && oldSize.x != 0.0f && oldSize.y != 0.0f) {
        bool by_x;
        float w_scale = new_size.x / oldSize.x;
        float h_scale = new_size.y / oldSize.y;
        double old_area = (double) oldSize.x * (double) oldSize.y;
        if(!scaling_y) {
            by_x = true;
        } else if(!scaling_x) {
            by_x = false;
        } else {
            if(fabs(w_scale) < fabs(h_scale)) {
                by_x = true;
            } else {
                by_x = false;
            }
        }
        if(by_x) {
            if(min_size != -FLT_MAX) {
                new_size.x = std::max(min_size, new_size.x);
            }
            new_size.y = old_area / new_size.x;
        } else {
            if(min_size != -FLT_MAX) {
                new_size.y = std::max(min_size, new_size.y);
            }
            new_size.x = old_area / new_size.y;
        }
    }
    
    switch(movingHandle) {
    case 0:
    case 3:
    case 6: {
        if(!lock_center) {
            transformed_center.x = (size->x / 2.0f) - new_size.x / 2.0f;
        }
        break;
    } case 2:
    case 5:
    case 8: {
        if(!lock_center) {
            transformed_center.x = (-size->x / 2.0f) + new_size.x / 2.0f;
        }
        break;
    }
    }
    
    switch(movingHandle) {
    case 0:
    case 1:
    case 2: {
        if(!lock_center) {
            transformed_center.y = (size->y / 2.0f) - new_size.y / 2.0f;
        }
        break;
    } case 6:
    case 7:
    case 8: {
        if(!lock_center) {
            transformed_center.y = (-size->y / 2.0f) + new_size.y / 2.0f;
        }
        break;
    }
    }
    
    Point new_center = transformed_center;
    al_invert_transform(&t);
    al_transform_coordinates(&t, &new_center.x, &new_center.y);
    
    *center = new_center;
    *size = new_size;
    
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
