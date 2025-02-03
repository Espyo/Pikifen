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

//Picker dialog maximum button size.
const float PICKER_IMG_BUTTON_MAX_SIZE = 160.0f;

//Picker dialog minimum button size.
const float PICKER_IMG_BUTTON_MIN_SIZE = 32.0f;

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
editor::editor() :
    changes_mgr(this) {
    
    editor_icons.reserve(N_EDITOR_ICONS);
    for(size_t i = 0; i < N_EDITOR_ICONS; i++) {
        editor_icons.push_back(nullptr);
    }
}


/**
 * @brief Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the screen,
 * where it's hard to see.
 *
 * @param min_coords Top-left coordinates of the content to focus on.
 * @param max_coords Bottom-right coordinates of the content to focus on.
 * @param instantaneous If true, the camera moves there instantaneously.
 * If false, it smoothly gets there over time.
 */
void editor::center_camera(
    const point &min_coords, const point &max_coords,
    bool instantaneous
) {
    point min_c = min_coords;
    point max_c = max_coords;
    if(min_c == max_c) {
        min_c = min_c - 2.0;
        max_c = max_c + 2.0;
    }
    
    float width = max_c.x - min_c.x;
    float height = max_c.y - min_c.y;
    
    game.cam.target_pos.x = floor(min_c.x + width  / 2);
    game.cam.target_pos.y = floor(min_c.y + height / 2);
    
    float z;
    if(width > height) z = (canvas_br.x - canvas_tl.x) / width;
    else z = (canvas_br.y - canvas_tl.y) / height;
    z -= z * 0.1;
    
    game.cam.target_zoom = z;
    
    if(instantaneous) {
        game.cam.pos = game.cam.target_pos;
        game.cam.zoom = game.cam.target_zoom;
    }
    
    update_transformations();
}


/**
 * @brief Closes the topmost dialog that is still open.
 */
void editor::close_top_dialog() {
    for(size_t d = 0; d < dialogs.size(); d++) {
        dialog_info* d_ptr = dialogs[dialogs.size() - (d + 1)];
        if(d_ptr->is_open) {
            d_ptr->is_open = false;
            return;
        }
    }
}


/**
 * @brief Handles the logic part of the main loop of the editor.
 * This is meant to be run after the editor's own logic code.
 */
void editor::do_logic_post() {
    escape_was_pressed = false;
    game.fade_mgr.tick(game.delta_t);
}


/**
 * @brief Handles the logic part of the main loop of the editor.
 * This is meant to be run before the editor's own logic code.
 */
void editor::do_logic_pre() {
    if(double_click_time > 0) {
        double_click_time -= game.delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
    
    game.cam.tick(game.delta_t);
    game.cam.update_box();
    
    op_error_flash_timer.tick(game.delta_t);
    
    update_transformations();
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
void editor::draw_grid(
    float interval,
    const ALLEGRO_COLOR &major_color, const ALLEGRO_COLOR &minor_color
) {
    point cam_top_left_corner(0, 0);
    point cam_bottom_right_corner(canvas_br.x, canvas_br.y);
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &cam_top_left_corner.x, &cam_top_left_corner.y
    );
    al_transform_coordinates(
        &game.screen_to_world_transform,
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
void editor::draw_op_error_cursor() {
    float error_flash_time_ratio = op_error_flash_timer.get_ratio_left();
    if(error_flash_time_ratio <= 0.0f) return;
    point pos = op_error_pos;
    draw_bitmap(
        game.sys_assets.bmp_notification,
        point(
            pos.x,
            pos.y - EDITOR::OP_ERROR_CURSOR_SIZE
        ),
        point(
            EDITOR::OP_ERROR_CURSOR_SIZE * 2.5f,
            EDITOR::OP_ERROR_CURSOR_SIZE * 2.0f
        ),
        0.0f,
        map_alpha(error_flash_time_ratio * 192)
    );
    pos.x +=
        EDITOR::OP_ERROR_CURSOR_SHAKE_WIDTH *
        sin(game.time_passed * EDITOR::OP_ERROR_CURSOR_SHAKE_SPEED) *
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
size_t editor::get_history_size() const {
    return EDITOR::DEF_MAX_HISTORY_SIZE;
}


/**
 * @brief Returns the position of the last widget, in screen coordinates.
 *
 * @return The position.
 */
point editor::get_last_widget_pos() {
    return
        point(
            ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x / 2.0,
            ImGui::GetItemRectMin().y + ImGui::GetItemRectSize().y / 2.0
        );
}


/**
 * @brief Handles an Allegro event for control-related things.
 *
 * @param ev Event to handle.
 */
void editor::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    bool is_mouse_in_canvas =
        dialogs.empty() && !is_mouse_in_gui;
    //WantCaptureKeyboard returns true if LMB is held, and I'm not quite sure
    //why. If we know LMB is held because of the canvas, then we can
    //safely assume it's none of Dear ImGui's business, so we can ignore
    //WantCaptureKeyboard's true.
    bool does_imgui_need_keyboard =
        ImGui::GetIO().WantCaptureKeyboard && !is_m1_pressed;
        
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        //General mouse handling.
        
        last_input_was_keyboard = false;
        handle_mouse_update(ev);
    }
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        is_mouse_in_canvas
    ) {
        //Mouse button down, inside the canvas.
        
        switch (ev.mouse.button) {
        case 1: {
            is_m1_pressed = true;
            break;
        } case 2: {
            is_m2_pressed = true;
            break;
        } case 3: {
            is_m3_pressed = true;
            break;
        }
        }
        
        mouse_drag_start = point(ev.mouse.x, ev.mouse.y);
        mouse_drag_confirmed = false;
        
        if(
            ev.mouse.button == last_mouse_click &&
            fabs(last_mouse_click_pos.x - ev.mouse.x) < 4.0f &&
            fabs(last_mouse_click_pos.y - ev.mouse.y) < 4.0f &&
            sub_state == last_mouse_click_sub_state &&
            double_click_time > 0
        ) {
            //Double-click.
            
            if(does_imgui_need_keyboard) {
                //If Dear ImGui needs the keyboard, then a textbox is likely
                //in use. Clicking could change the state of the editor's data,
                //so ignore it now, and let Dear ImGui close the box.
                is_m1_pressed = false;
                
            } else {
            
                switch(ev.mouse.button) {
                case 1: {
                    handle_lmb_double_click(ev);
                    break;
                } case 2: {
                    handle_rmb_double_click(ev);
                    break;
                } case 3: {
                    handle_mmb_double_click(ev);
                    break;
                }
                }
                
                double_click_time = 0;
            }
            
        } else {
            //Single-click.
            
            if(does_imgui_need_keyboard) {
                //If Dear ImGui needs the keyboard, then a textbox is likely
                //in use. Clicking could change the state of the editor's data,
                //so ignore it now, and let Dear ImGui close the box.
                is_m1_pressed = false;
                
            } else {
            
                last_mouse_click_sub_state = sub_state;
                
                switch(ev.mouse.button) {
                case 1: {
                    handle_lmb_down(ev);
                    break;
                } case 2: {
                    handle_rmb_down(ev);
                    break;
                } case 3: {
                    handle_mmb_down(ev);
                    break;
                }
                }
                
                last_mouse_click = ev.mouse.button;
                last_mouse_click_pos.x = ev.mouse.x;
                last_mouse_click_pos.y = ev.mouse.y;
                double_click_time = EDITOR::DOUBLE_CLICK_TIMEOUT;
                
            }
        }
        
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        //Mouse button up.
        
        switch(ev.mouse.button) {
        case 1: {
            is_m1_pressed = false;
            handle_lmb_up(ev);
            break;
        } case 2: {
            is_m2_pressed = false;
            handle_rmb_up(ev);
            break;
        } case 3: {
            is_m3_pressed = false;
            handle_mmb_up(ev);
            break;
        }
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED
    ) {
        //Mouse movement.
        
        if(
            fabs(ev.mouse.x - mouse_drag_start.x) >=
            game.options.editor_mouse_drag_threshold ||
            fabs(ev.mouse.y - mouse_drag_start.y) >=
            game.options.editor_mouse_drag_threshold
        ) {
            mouse_drag_confirmed = true;
        }
        
        if(mouse_drag_confirmed) {
            if(is_m1_pressed) {
                handle_lmb_drag(ev);
            }
            if(is_m2_pressed) {
                handle_rmb_drag(ev);
            }
            if(is_m3_pressed) {
                handle_mmb_drag(ev);
            }
        }
        if(
            (ev.mouse.dz != 0 || ev.mouse.dw != 0) &&
            is_mouse_in_canvas
        ) {
            handle_mouse_wheel(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        //Key down.
        
        last_input_was_keyboard = true;
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            is_shift_pressed = true;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            is_ctrl_pressed = true;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_ALT ||
            ev.keyboard.keycode == ALLEGRO_KEY_ALTGR
        ) {
            is_alt_pressed = true;
            
        }
        
        if(dialogs.empty()) {
            handle_key_down_anywhere(ev);
            if(!does_imgui_need_keyboard) {
                handle_key_down_canvas(ev);
            }
        }
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE &&
            !dialogs.empty()
        ) {
            close_top_dialog();
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        //Key up.
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            is_shift_pressed = false;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            is_ctrl_pressed = false;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_ALT ||
            ev.keyboard.keycode == ALLEGRO_KEY_ALTGR
        ) {
            is_alt_pressed = false;
            
        }
        
        if(dialogs.empty()) {
            handle_key_up_anywhere(ev);
            if(!does_imgui_need_keyboard) {
                handle_key_up_canvas(ev);
            }
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        //Key char.
        
        if(dialogs.empty()) {
            handle_key_char_anywhere(ev);
            if(!does_imgui_need_keyboard) {
                handle_key_char_canvas(ev);
            }
        }
        
    }
    
    if(!dialogs.empty()) {
        if(dialogs.back()->event_callback) {
            dialogs.back()->event_callback(&ev);
        }
    }
}


/**
 * @brief Placeholder for handling a key being "char-typed" anywhere.
 *
 * @param ev Event to process.
 */
void editor::handle_key_char_anywhere(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being "char-typed" in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being pressed down anywhere.
 *
 * @param ev Event to process.
 */
void editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being pressed down in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being released anywhere.
 *
 * @param ev Event to process.
 */
void editor::handle_key_up_anywhere(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling a key being released in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_key_up_canvas(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the left mouse button being double-clicked
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the left mouse button being pressed down
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the left mouse button being dragged
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the left mouse button released
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being double-clicked
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being pressed down
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being dragged
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the middle mouse button being released
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_mmb_up(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the mouse coordinates being updated.
 *
 * @param ev Event to process.
 */
void editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the mouse wheel being turned in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the right mouse button being double-clicked
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the right mouse button being pressed down
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the right mouse button being dragged
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Placeholder for handling the right mouse button being released
 * in the canvas.
 *
 * @param ev Event to process.
 */
void editor::handle_rmb_up(const ALLEGRO_EVENT &ev) {}


/**
 * @brief Displays a popup, if applicable, and fills it with a text input
 * for the user to type something in.
 *
 * @param label Name of the popup.
 * @param prompt What to prompt to the user. e.g.: "New name:"
 * @param text Pointer to the starting text, as well as the user's final text.
 * @return Whether the user pressed Return or the Ok button.
 */
bool editor::input_popup(
    const char* label, const char* prompt, string* text
) {
    bool ret = false;
    needs_input_popup_text_focus = true;
    if(ImGui::BeginPopup(label)) {
        if(escape_was_pressed) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::Text("%s", prompt);
        ImGui::FocusOnInputText(needs_input_popup_text_focus);
        if(
            ImGui::InputText(
                "##inputPopupText", text,
                ImGuiInputTextFlags_EnterReturnsTrue |
                ImGuiInputTextFlags_AutoSelectAll
            )
        ) {
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
 * @brief Returns whether a given internal name is good or not.
 *
 * @param name The internal name to check.
 * @return Whether it's good.
 */
bool editor::is_internal_name_good(const string &name) const {
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
bool editor::key_check(
    int pressed_key, int match_key,
    bool needs_ctrl, bool needs_shift
) {

    if(pressed_key != match_key) {
        return false;
    }
    if(needs_ctrl != is_ctrl_pressed) {
        return false;
    }
    if(needs_shift != is_shift_pressed) {
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
void editor::keyframe_visualizer(
    keyframe_interpolator<ALLEGRO_COLOR> &interpolator,
    size_t sel_keyframe_idx
) {
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
    auto first_kf = interpolator.get_keyframe(0);
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
    for(size_t t = 0; t < interpolator.keyframe_count() - 1; t++) {
        auto kf_1 = interpolator.get_keyframe(t);
        auto kf_2 = interpolator.get_keyframe(t + 1);
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
    auto last_kf = interpolator.get_keyframe(interpolator.keyframe_count() - 1);
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
    for(size_t c = 0; c < interpolator.keyframe_count(); c++) {
        float time = interpolator.get_keyframe(c).first;
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
    set_tooltip(
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
void editor::keyframe_visualizer(
    keyframe_interpolator<float> &interpolator,
    size_t sel_keyframe_idx
) {
    //The built in plot widget doesn't allow for dynamic spacing,
    //so we need to make our own.
    
    //Setup.
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImVec2((ImGui::GetColumnWidth() - 1), 40);
    
    float max_value = -FLT_MAX;
    float min_value = FLT_MAX;
    
    for(size_t t = 0; t < interpolator.keyframe_count(); t++) {
        max_value = std::max(interpolator.get_keyframe(t).second, max_value);
        min_value = std::min(interpolator.get_keyframe(t).second, min_value);
    }
    
    //Draw the background.
    draw_list->AddRectFilled(
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + (ImGui::GetColumnWidth() - 1), pos.y + 40),
        ImGui::GetColorU32(ImGuiCol_FrameBg)
    );
    
    //Draw the chart line from the start to the first keyframe.
    auto first_kf = interpolator.get_keyframe(0);
    draw_list->AddLine(
        ImVec2(
            pos.x,
            pos.y + interpolate_number(
                first_kf.second, min_value, max_value, size.y, 1
            )
        ),
        ImVec2(
            pos.x + size.x * first_kf.first,
            pos.y + interpolate_number(
                first_kf.second, min_value, max_value, size.y, 1
            )
        ),
        ImGui::GetColorU32(ImGuiCol_PlotLines)
    );
    
    //Draw the chart lines between the keyframes.
    for(size_t t = 0; t < interpolator.keyframe_count() - 1; t++) {
        auto kf_1 = interpolator.get_keyframe(t);
        auto kf_2 = interpolator.get_keyframe(t + 1);
        float f1 = kf_1.second;
        float f2 = kf_2.second;
        
        draw_list->AddLine(
            ImVec2(
                pos.x + size.x * kf_1.first,
                pos.y + interpolate_number(
                    f1, min_value, max_value, size.y, 1
                )
            ),
            ImVec2(
                pos.x + size.x * kf_2.first,
                pos.y + interpolate_number(
                    f2, min_value, max_value, size.y, 1
                )
            ),
            ImGui::GetColorU32(ImGuiCol_PlotLines)
        );
    }
    
    //Draw the chart line from the final keyframe to the end.
    auto last_kf = interpolator.get_keyframe(interpolator.keyframe_count() - 1);
    draw_list->AddLine(
        ImVec2(
            pos.x + size.x * last_kf.first,
            pos.y + interpolate_number(
                last_kf.second, min_value, max_value, size.y, 1
            )
        ),
        ImVec2(
            pos.x + size.x,
            pos.y + interpolate_number(
                last_kf.second, min_value, max_value, size.y, 1
            )
        ),
        ImGui::GetColorU32(ImGuiCol_PlotLines)
    );
    
    //Draw the bars indicating the position of each keyframe.
    for(size_t c = 0; c < interpolator.keyframe_count(); c++) {
        float time = interpolator.get_keyframe(c).first;
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
    set_tooltip(
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
void editor::keyframe_visualizer(
    keyframe_interpolator<point> &interpolator,
    size_t sel_keyframe_idx
) {
    //Split the interpolator into two, one for each axis.
    keyframe_interpolator<float> x_inter(interpolator.get_keyframe(0).second.x);
    keyframe_interpolator<float> y_inter(interpolator.get_keyframe(0).second.y);
    
    x_inter.set_keyframe_time(0, interpolator.get_keyframe(0).first);
    y_inter.set_keyframe_time(0, interpolator.get_keyframe(0).first);
    
    for(size_t s = 1; s < interpolator.keyframe_count(); s++) {
        auto kf = interpolator.get_keyframe(s);
        x_inter.add(kf.first, kf.second.x);
        y_inter.add(kf.first, kf.second.y);
    }
    
    //Draw the two visualizers.
    keyframe_visualizer(x_inter, sel_keyframe_idx);
    keyframe_visualizer(y_inter, sel_keyframe_idx);
}


/**
 * @brief Processes Dear ImGui widgets tha allow organizing keyframe
 * interpolators.
 *
 * @tparam inter_t Type of the interpolator value.
 * @param button_id Prefix for the Dear ImGui ID of the navigation buttons.
 * @param interpolator Interpolator to get data from.
 * @param sel_keyframe_idx Index of the currently selected keyframe.
 * @return Whether anything in the interpolator was changed.
 */
template <class inter_t>
bool editor::keyframe_organizer(
    const string &button_id,
    keyframe_interpolator<inter_t> &interpolator,
    size_t &sel_keyframe_idx
) {
    bool result = false;
    
    //Current keyframe text.
    ImGui::Text(
        "Keyframe: %lu/%lu",
        sel_keyframe_idx + 1,
        interpolator.keyframe_count()
    );
    
    //Previous keyframe button.
    ImGui::SameLine();
    string prevLabel = button_id + "prevButton";
    if(
        ImGui::ImageButton(
            prevLabel, editor_icons[EDITOR_ICON_PREVIOUS],
            point(EDITOR::ICON_BMP_SIZE / 2.0f)
        )
    ) {
        if(sel_keyframe_idx == 0) {
            sel_keyframe_idx = interpolator.keyframe_count() - 1;
        } else {
            sel_keyframe_idx--;
        }
    }
    set_tooltip(
        "Select the previous keyframe."
    );
    
    //Next keyframe button.
    ImGui::SameLine();
    string nextLabel = button_id + "nextButton";
    if(
        ImGui::ImageButton(
            nextLabel, editor_icons[EDITOR_ICON_NEXT],
            point(EDITOR::ICON_BMP_SIZE / 2.0f)
        )
    ) {
        if(sel_keyframe_idx == interpolator.keyframe_count() - 1) {
            sel_keyframe_idx = 0;
        } else {
            sel_keyframe_idx++;
        }
    }
    set_tooltip(
        "Select the next keyframe."
    );
    
    //Add keyframe button.
    ImGui::SameLine();
    string addLabel = button_id + "addButton";
    if(
        ImGui::ImageButton(
            addLabel, editor_icons[EDITOR_ICON_ADD],
            point(EDITOR::ICON_BMP_SIZE / 2.0f)
        )
    ) {
        float prev_t = interpolator.get_keyframe(sel_keyframe_idx).first;
        float next_t =
            sel_keyframe_idx == interpolator.keyframe_count() - 1 ?
            1.0f :
            interpolator.get_keyframe(sel_keyframe_idx + 1).first;
        float new_t = (prev_t + next_t) / 2.0f;
        
        interpolator.add(new_t, interpolator.get(new_t));
        sel_keyframe_idx++;
        set_status(
            "Added keyframe #" + i2s(sel_keyframe_idx + 1) + "."
        );
        result = true;
    }
    set_tooltip(
        "Add a new keyframe after the currently selected one.\n"
        "It will go between the current one and the one after."
    );
    
    if(interpolator.keyframe_count() > 1) {
        //Delete frame button.
        ImGui::SameLine();
        string removeButton = button_id + "removeButton";
        if(
            ImGui::ImageButton(
                removeButton, editor_icons[EDITOR_ICON_REMOVE],
                point(EDITOR::ICON_BMP_SIZE / 2.0f)
            )
        ) {
            size_t deleted_frame_idx = sel_keyframe_idx;
            interpolator.remove(deleted_frame_idx);
            if(sel_keyframe_idx == interpolator.keyframe_count()) {
                sel_keyframe_idx--;
            }
            set_status(
                "Deleted keyframe #" + i2s(deleted_frame_idx + 1) + "."
            );
            result = true;
        }
        set_tooltip(
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
bool editor::keyframe_editor(
    const string &label,
    keyframe_interpolator<ALLEGRO_COLOR> &interpolator,
    size_t &sel_keyframe_idx
) {
    //Visualizer.
    keyframe_visualizer(interpolator, sel_keyframe_idx);
    
    //Organizer.
    bool result = keyframe_organizer(label, interpolator, sel_keyframe_idx);
    
    //Time value.
    float time = interpolator.get_keyframe(sel_keyframe_idx).first;
    if(ImGui::SliderFloat("Time", &time, 0.0f, 1.0f)) {
        interpolator.set_keyframe_time(
            sel_keyframe_idx, time, &sel_keyframe_idx
        );
        result = true;
    }
    set_tooltip(
        "Time at which this keyframe occurs.\n"
        "0 means the beginning, 1 means the end.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Color editor.
    ALLEGRO_COLOR value = interpolator.get_keyframe(sel_keyframe_idx).second;
    if(ImGui::ColorEdit4(label.c_str(), (float*) &value)) {
        interpolator.set_keyframe_value(sel_keyframe_idx, value);
        result = true;
    }
    set_tooltip("What color to use at this keyframe.");
    
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
bool editor::keyframe_editor(
    const string &label,
    keyframe_interpolator<float> &interpolator,
    size_t &sel_keyframe_idx
) {
    //Visualizer.
    keyframe_visualizer(interpolator, sel_keyframe_idx);
    
    //Organizer.
    bool result = keyframe_organizer(label, interpolator, sel_keyframe_idx);
    
    //Time value.
    float time = interpolator.get_keyframe(sel_keyframe_idx).first;
    if(ImGui::SliderFloat("Time", &time, 0.0f, 1.0f)) {
        interpolator.set_keyframe_time(
            sel_keyframe_idx, time, &sel_keyframe_idx
        );
        result = true;
    }
    set_tooltip(
        "Time at which this keyframe occurs.\n"
        "0 means the beginning, 1 means the end.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Float value.
    float value = interpolator.get_keyframe(sel_keyframe_idx).second;
    if(ImGui::DragFloat(label.c_str(), &value)) {
        interpolator.set_keyframe_value(sel_keyframe_idx, value);
        result = true;
    }
    set_tooltip("What value to use at this keyframe.");
    
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
bool editor::keyframe_editor(
    const string &label,
    keyframe_interpolator<point> &interpolator,
    size_t &sel_keyframe_idx
) {
    //Visualizer.
    keyframe_visualizer(interpolator, sel_keyframe_idx);
    
    //Organizer.
    bool result = keyframe_organizer(label, interpolator, sel_keyframe_idx);
    
    //Time value.
    float time = interpolator.get_keyframe(sel_keyframe_idx).first;
    if(ImGui::SliderFloat("Time", &time, 0.0f, 1.0f)) {
        interpolator.set_keyframe_time(
            sel_keyframe_idx, time, &sel_keyframe_idx
        );
        result = true;
    }
    set_tooltip(
        "Time at which this keyframe occurs.\n"
        "0 means the beginning, 1 means the end.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Float values.
    point value = interpolator.get_keyframe(sel_keyframe_idx).second;
    if(ImGui::DragFloat2(label.c_str(), (float*) &value)) {
        interpolator.set_keyframe_value(sel_keyframe_idx, value);
        result = true;
    }
    set_tooltip("What coordinates to use at this keyframe.");
    
    return result;
}


/**
 * @brief Exits out of the editor, with a fade.
 */
void editor::leave() {
    //Save the user's preferred tree node open states.
    save_options();
    
    game.fade_mgr.start_fade(false, [] () {
        if(game.states.area_ed->quick_play_area_path.empty()) {
            game.states.main_menu->page_to_load = MAIN_MENU_PAGE_MAKE;
            game.change_state(game.states.main_menu);
        } else {
            game.states.gameplay->path_of_area_to_load =
                game.states.area_ed->quick_play_area_path;
            game.change_state(game.states.gameplay);
        }
    });
    
    set_status("Bye!");
}


/**
 * @brief Displays a popup, if applicable, and fills it with selectable items
 * from a list.
 *
 * @param label Name of the popup.
 * @param items List of items.
 * @param picked_item If an item was picked, set this to its name.
 * @return Whether an item was clicked on.
 */
bool editor::list_popup(
    const char* label, const vector<string> &items, string* picked_item
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        if(escape_was_pressed) {
            ImGui::CloseCurrentPopup();
        }
        for(size_t i = 0; i < items.size(); i++) {
            string name = items[i];
            if(ImGui::Selectable(name.c_str())) {
                *picked_item = name;
                ret = true;
            }
        }
        ImGui::EndPopup();
    }
    return ret;
}


/**
 * @brief Loads things common for all editors.
 */
void editor::load() {
    //Icon sub-bitmaps.
    bmp_editor_icons =
        game.content.bitmaps.list.get(game.asset_file_names.bmp_editor_icons);
    if(bmp_editor_icons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; i++) {
            editor_icons[i] =
                al_create_sub_bitmap(
                    bmp_editor_icons,
                    (int) (EDITOR::ICON_BMP_SIZE * i) +
                    (int) (EDITOR::ICON_BMP_PADDING * i),
                    0,
                    EDITOR::ICON_BMP_SIZE,
                    EDITOR::ICON_BMP_SIZE
                );
        }
    }
    
    //Misc. setup.
    is_alt_pressed = false;
    is_ctrl_pressed = false;
    is_shift_pressed = false;
    last_input_was_keyboard = false;
    manifest.clear();
    set_status();
    changes_mgr.reset();
    game.mouse_cursor.show();
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
    update_style();
    
    game.fade_mgr.start_fade(true, nullptr);
    ImGui::Reset();
}


/**
 * @brief Loads all mob types into the custom_cat_types list.
 *
 * @param is_area_editor If true, mob types that do not appear in the
 * area editor will not be counted for here.
 */
void editor::load_custom_mob_cat_types(bool is_area_editor) {
    //Load.
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        mob_category* c_ptr = game.mob_categories.get((MOB_CATEGORY) c);
        vector<string> type_names;
        c_ptr->get_type_names(type_names);
        
        for(size_t tn = 0; tn < type_names.size(); tn++) {
            mob_type* mt_ptr = c_ptr->get_type(type_names[tn]);
            
            if(is_area_editor && !mt_ptr->appears_in_area_editor) {
                continue;
            }
            
            string custom_cat_name = mt_ptr->custom_category_name;
            size_t custom_cat_idx;
            map<string, size_t>::iterator custom_cat_idx_it =
                custom_cat_name_idxs.find(custom_cat_name);
            if(custom_cat_idx_it == custom_cat_name_idxs.end()) {
                custom_cat_name_idxs[custom_cat_name] =
                    custom_cat_types.size();
                custom_cat_types.push_back(vector<mob_type*>());
            }
            custom_cat_idx = custom_cat_name_idxs[custom_cat_name];
            
            custom_cat_types[custom_cat_idx].push_back(mt_ptr);
        }
    }
    
    //Sort.
    std::sort(
        custom_cat_types.begin(), custom_cat_types.end(),
    [] (const vector<mob_type*> &c1, const vector<mob_type*> &c2) -> bool {
        return
        c1.front()->custom_category_name <
        c2.front()->custom_category_name;
    }
    );
    for(size_t c = 0; c < custom_cat_types.size(); c++) {
        vector<mob_type*> &types = custom_cat_types[c];
        //Sort the types within a custom category.
        std::sort(
            types.begin(), types.end(),
        [] (const mob_type * t1, const mob_type * t2) {
            return t1->name < t2->name;
        }
        );
        //Adjust custom_cat_name_idxs, since the list of custom category names
        //got shuffled earlier.
        custom_cat_name_idxs[
            custom_cat_types[c][0]->custom_category_name
        ] = c;
    }
}


/**
 * @brief Opens a dialog warning the maker that they're editing something
 * in the base pack. Does not do anything if the player is an engine developer.
 *
 * @param do_pick_callback Callback for if we actually have to do the pick.
 */
void editor::open_base_content_warning_dialog(
    const std::function<void()> &do_pick_callback
) {
    if(game.options.engine_developer) {
        do_pick_callback();
        return;
    }
    
    open_dialog(
        "Base pack warning",
        std::bind(&editor::process_gui_base_content_warning_dialog, this)
    );
    dialogs.back()->custom_size = point(320, 0);
    base_content_warning_do_pick_callback = do_pick_callback;
}


/**
 * @brief Opens a dialog where the user can choose a bitmap from the
 * game content.
 *
 * @param ok_callback Callback for when the user chooses a bitmap.
 * @param recommended_folder If not empty, recommend that the user picks bitmaps
 * with this folder name. Use "." for the graphics root folder.
 */
void editor::open_bitmap_dialog(
    std::function<void(const string &)> ok_callback,
    const string &recommended_folder
) {
    bitmap_dialog_ok_callback = ok_callback;
    bitmap_dialog_recommended_folder = recommended_folder;
    bitmap_dialog_picker.pick_callback =
        [this] (
            const string &new_bmp_name, const string &, const string &, void*, bool
    ) {
        bitmap_dialog_new_bmp_name = new_bmp_name;
    };
    
    open_dialog(
        "Choose a bitmap",
        std::bind(&editor::process_gui_bitmap_dialog, this)
    );
    dialogs.back()->close_callback = [this] () {
        if(!bitmap_dialog_cur_bmp_name.empty()) {
            game.content.bitmaps.list.free(bitmap_dialog_cur_bmp_name);
            bitmap_dialog_cur_bmp_name.clear();
            bitmap_dialog_cur_bmp_ptr = nullptr;
            bitmap_dialog_new_bmp_name.clear();
            bitmap_dialog_ok_callback = nullptr;
            bitmap_dialog_recommended_folder = "";
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
void editor::open_dialog(
    const string &title,
    const std::function<void()> &process_callback
) {
    dialog_info* new_dialog = new dialog_info();
    
    new_dialog->title = title;
    new_dialog->process_callback = process_callback;
    
    dialogs.push_back(new_dialog);
}


/**
 * @brief Opens a dialog with a simple message and an "open manual" button,
 * designed for each editor's standard "help" information.
 *
 * @param message Text message.
 * @param page Manual page explaining the editor and content type in
 * more detail.
 */
void editor::open_help_dialog(
    const string &message, const string &page
) {
    help_dialog_message = message;
    help_dialog_page = page;
    open_dialog("Help", std::bind(&editor::process_gui_help_dialog, this));
    dialogs.back()->custom_size = point(400, 0);
}


/**
 * @brief Opens a dialog with a simple message and an ok button.
 *
 * @param title Message box title.
 * @param message Text message.
 * @param close_callback Code to run when the dialog is closed, if any.
 */
void editor::open_message_dialog(
    const string &title, const string &message,
    const std::function<void()> &close_callback
) {
    message_dialog_message = message;
    open_dialog(title, std::bind(&editor::process_gui_message_dialog, this));
    dialogs.back()->custom_size = point(400, 0);
    dialogs.back()->close_callback = close_callback;
}


/**
 * @brief Opens a dialog where the user can create a new pack.
 */
void editor::open_new_pack_dialog() {
    needs_new_pack_text_focus = true;
    open_dialog(
        "Create a new pack",
        std::bind(&editor::process_gui_new_pack_dialog, this)
    );
    dialogs.back()->custom_size = point(520, 0);
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
 * @param filter Filter of names. Only items that match this will appear.
 */
void editor::open_picker_dialog(
    const string &title,
    const vector<picker_item> &items,
    const std::function <void(
        const string &, const string &, const string &, void*, bool
    )> &pick_callback,
    const string &list_header,
    bool can_make_new,
    const string &filter
) {
    picker_info* new_picker = new picker_info(this);
    
    new_picker->items = items;
    new_picker->list_header = list_header;
    new_picker->pick_callback = pick_callback;
    new_picker->can_make_new = can_make_new;
    new_picker->filter = filter;
    
    dialog_info* new_dialog = new dialog_info();
    dialogs.push_back(new_dialog);
    
    new_dialog->title = title;
    new_dialog->process_callback =
        std::bind(&editor::picker_info::process, new_picker);
    new_dialog->close_callback = [new_picker] () {
        delete new_picker;
    };
    new_picker->dialog_ptr = new_dialog;
}


/**
 * @brief Creates widgets with the goal of placing a disabled text widget to the
 * right side of the panel.
 *
 * @param title Title to write.
 */
void editor::panel_title(const char* title) {
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
bool editor::popup(const char* label, ImGuiWindowFlags flags) {
    bool result = ImGui::BeginPopup(label, flags);
    if(result) {
        if(escape_was_pressed) {
            ImGui::CloseCurrentPopup();
        }
    }
    return result;
}


/**
 * @brief Processes all currently open dialogs for this frame.
 */
void editor::process_dialogs() {
    //Delete closed ones.
    for(size_t d = 0; d < dialogs.size();) {
        dialog_info* d_ptr = dialogs[d];
        
        if(!d_ptr->is_open) {
            if(d_ptr->close_callback) {
                d_ptr->close_callback();
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
void editor::process_gui_base_content_warning_dialog() {
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
        close_top_dialog();
    }
    
    //Continue button.
    ImGui::SameLine();
    if(ImGui::Button("Continue", ImVec2(70, 30))) {
        base_content_warning_do_pick_callback();
        base_content_warning_do_pick_callback = nullptr;
        close_top_dialog();
    }
    
    //Open manual button.
    ImGui::SetupCentering(100);
    if(ImGui::Button("Open manual", ImVec2(100, 25))) {
        open_manual("making.html#packs");
    }
}


/**
 * @brief Processes the bitmap picker dialog for this frame.
 */
void editor::process_gui_bitmap_dialog() {
    static bool filter_with_recommended_folder = true;
    
    //Fill the picker's items.
    bitmap_dialog_picker.items.clear();
    for(auto &b : game.content.bitmaps.manifests) {
        if(
            !bitmap_dialog_recommended_folder.empty() &&
            filter_with_recommended_folder
        ) {
            vector<string> parts = split(b.first, "/");
            string folder = parts.size() == 1 ? "." : parts[0];
            if(folder != bitmap_dialog_recommended_folder) continue;
        }
        
        bitmap_dialog_picker.items.push_back(
            picker_item(
                b.first,
                "Pack: " + game.content.packs.list[b.second.pack].name
            )
        );
    }
    
    //Update the image if needed.
    if(bitmap_dialog_new_bmp_name != bitmap_dialog_cur_bmp_name) {
        if(!bitmap_dialog_cur_bmp_name.empty()) {
            game.content.bitmaps.list.free(bitmap_dialog_cur_bmp_name);
        }
        if(bitmap_dialog_new_bmp_name.empty()) {
            bitmap_dialog_cur_bmp_ptr = nullptr;
            bitmap_dialog_cur_bmp_name.clear();
        } else {
            bitmap_dialog_cur_bmp_ptr =
                game.content.bitmaps.list.get(bitmap_dialog_new_bmp_name);
            bitmap_dialog_cur_bmp_name = bitmap_dialog_new_bmp_name;
        }
    }
    
    //Column setup.
    ImGui::Columns(2, "colBitmaps");
    ImGui::BeginChild("butOk");
    
    //Ok button.
    ImGui::SetupCentering(200);
    if(!bitmap_dialog_cur_bmp_ptr) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Ok", ImVec2(200, 40))) {
        if(bitmap_dialog_ok_callback) {
            bitmap_dialog_ok_callback(bitmap_dialog_cur_bmp_name);
        }
        close_top_dialog();
    }
    if(!bitmap_dialog_cur_bmp_ptr) {
        ImGui::EndDisabled();
    }
    
    //Recommended folder text.
    string folder_str =
        bitmap_dialog_recommended_folder == "." ?
        "(root)" : bitmap_dialog_recommended_folder;
    ImGui::Spacer();
    ImGui::Text("Recommended folder: %s", folder_str.c_str());
    
    //Recommended folder only checkbox.
    if(!bitmap_dialog_recommended_folder.empty()) {
        ImGui::Checkbox(
            "That folder only", &filter_with_recommended_folder
        );
        set_tooltip(
            "If checked, only images that belong to the\n"
            "recommended folder will be shown in the list."
        );
    }
    
    //Preview text.
    ImGui::Spacer();
    ImGui::Text("Preview:");
    
    //Preview image.
    if(bitmap_dialog_cur_bmp_ptr) {
        const int thumb_max_size = 300;
        point size =
            resize_to_box_keeping_aspect_ratio(
                get_bitmap_dimensions(bitmap_dialog_cur_bmp_ptr),
                point(thumb_max_size)
            );
        ImGui::Image(bitmap_dialog_cur_bmp_ptr, size);
    }
    
    //Next column.
    ImGui::EndChild();
    ImGui::NextColumn();
    
    //Bitmap picker.
    bitmap_dialog_picker.process();
    
    //Reset columns.
    ImGui::Columns(1);
}


/**
 * @brief Processes the widgets that allow the player to set a custom
 * editor style.
 */
void editor::process_gui_editor_style() {
    //Style node.
    if(saveable_tree_node("options", "Style")) {
    
        //Use custom style checkbox.
        if(
            ImGui::Checkbox(
                "Use custom style", &game.options.editor_use_custom_style
            )
        ) {
            update_style();
        }
        set_tooltip(
            "Use a custom color scheme for the editor,\n"
            "instead of the default.\n"
            "Default: " + b2s(OPTIONS::DEF_EDITOR_USE_CUSTOM_STYLE) + "."
        );
        
        //Primary color.
        if(
            ImGui::ColorEdit3(
                "Custom primary color",
                (float*) &game.options.editor_primary_color
            )
        ) {
            update_style();
        }
        set_tooltip(
            "Primary color for the custom style."
        );
        
        //Secondary color.
        if(
            ImGui::ColorEdit3(
                "Custom secondary color",
                (float*) &game.options.editor_secondary_color
            )
        ) {
            update_style();
        }
        set_tooltip(
            "Secondary color for the custom style."
        );
        
        //Text color.
        if(
            ImGui::ColorEdit3(
                "Text color",
                (float*) &game.options.editor_text_color
            )
        ) {
            update_style();
        }
        set_tooltip(
            "Color of text in the custom style."
        );
        
        //Highlight color.
        if(
            ImGui::ColorEdit3(
                "Highlight color",
                (float*) &game.options.editor_highlight_color
            )
        ) {
            update_style();
        }
        set_tooltip(
            "Color of highlights in the custom style."
        );
        ImGui::TreePop();
    }
}


/**
 * @brief Processes the help dialog widgets.
 */
void editor::process_gui_help_dialog() {
    //Text.
    static int text_width = 0;
    if(text_width != 0) {
        ImGui::SetupCentering(text_width);
    }
    ImGui::TextWrapped("%s", help_dialog_message.c_str());
    text_width = ImGui::GetItemRectSize().x;
    
    //Open manual button.
    ImGui::Spacer();
    ImGui::SetupCentering(200);
    if(ImGui::Button("Open manual", ImVec2(100, 40))) {
        open_manual(help_dialog_page);
    }
    
    //Ok button.
    ImGui::SameLine();
    if(ImGui::Button("Ok", ImVec2(100, 40))) {
        close_top_dialog();
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
void editor::process_gui_history(
    const std::function<string(const string &)> &name_display_callback,
    const std::function<void(const string &)> &pick_callback,
    const std::function<string(const string &)> &tooltip_callback
) {
    if(saveable_tree_node("load", "History")) {
    
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
                name = trim_with_ellipsis(name, 16);
                
                //History entry button.
                const ImVec2 button_size(120, 24);
                if(ImGui::Button((name + "##" + i2s(h)).c_str(), button_size)) {
                    pick_callback(path);
                }
                if(tooltip_callback) {
                    set_tooltip(tooltip_callback(path));
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
 * @brief Processes the message dialog widgets.
 */
void editor::process_gui_message_dialog() {
    //Text.
    static int text_width = 0;
    if(text_width != 0) {
        ImGui::SetupCentering(text_width);
    }
    ImGui::TextWrapped("%s", message_dialog_message.c_str());
    text_width = ImGui::GetItemRectSize().x;
    
    //Ok button.
    ImGui::Spacer();
    ImGui::SetupCentering(100);
    if(ImGui::Button("Ok", ImVec2(100, 40))) {
        close_top_dialog();
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
bool editor::process_gui_mob_type_widgets(
    string* custom_cat_name, mob_type** type, const string &pack_filter
) {
    bool result = false;
    
    //These are used to communicate with the picker dialog, since that one
    //is processed somewhere else entirely.
    static bool internal_changed_by_dialog = false;
    static string internal_custom_cat_name;
    static mob_type* internal_mob_type = nullptr;
    
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
    ImGui::SetColumnWidth(-1, 51.0f);
    
    //Search button.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9.0f, 9.0f));
    bool search_button_pressed =
        ImGui::ImageButton(
            "searchButton", editor_icons[EDITOR_ICON_SEARCH],
            point(EDITOR::ICON_BMP_SIZE)
        );
    ImGui::PopStyleVar();
    
    vector<vector<mob_type*> > final_list;
    if(!pack_filter.empty()) {
        for(size_t c = 0; c < custom_cat_types.size(); c++) {
            final_list.push_back(vector<mob_type*>());
            for(size_t n = 0; n < custom_cat_types[c].size(); n++) {
                mob_type* mt_ptr = custom_cat_types[c][n];
                if(mt_ptr->manifest && mt_ptr->manifest->pack == pack_filter) {
                    final_list[c].push_back(mt_ptr);
                }
            }
        }
    } else {
        final_list = custom_cat_types;
    }
    
    if(search_button_pressed) {
        vector<picker_item> items;
        for(size_t c = 0; c < final_list.size(); c++) {
            for(size_t n = 0; n < final_list[c].size(); n++) {
                mob_type* mt_ptr = final_list[c][n];
                items.push_back(
                    picker_item(
                        mt_ptr->name, mt_ptr->custom_category_name
                    )
                );
            }
        }
        open_picker_dialog(
            "Pick an object type", items,
            [this, final_list] (
                const string &n, const string &tc, const string &sc, void*, bool
        ) {
            //For clarity, this code will NOT be run within the context
            //of editor::process_gui_mob_type_widgets, but will instead
            //be run wherever dialogs are processed.
            internal_changed_by_dialog = true;
            internal_custom_cat_name = tc;
            internal_mob_type = nullptr;
            size_t custom_cat_idx = custom_cat_name_idxs[tc];
            const vector<mob_type*> &types =
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
    set_tooltip(
        "Search for an object type from the entire list."
    );
    
    ImGui::NextColumn();
    
    //Object category combobox.
    vector<string> categories;
    int selected_category_idx = -1;
    for(size_t c = 0; c < final_list.size(); c++) {
        string cn =
            custom_cat_types[c].front()->custom_category_name;
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
    set_tooltip(
        "What category this object belongs to: a Pikmin, a leader, etc."
    );
    
    if(!internal_custom_cat_name.empty()) {
    
        //Object type combobox.
        vector<string> type_names;
        size_t custom_cat_idx = custom_cat_name_idxs[internal_custom_cat_name];
        const vector<mob_type*> &types = final_list[custom_cat_idx];
        for(size_t t = 0; t < types.size(); t++) {
            mob_type* t_ptr = types[t];
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
        set_tooltip(
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
bool editor::process_gui_new_dialog_pack_widgets(string* pack) {
    static int pack_idx = 0;
    bool changed = false;
    
    //Pack combo.
    vector<string> packs;
    for(const auto &p : game.content.packs.manifests_with_base) {
        packs.push_back(game.content.packs.list[p].name);
    }
    if(packs.empty()) {
        //Failsafe.
        packs.push_back(FOLDER_NAMES::BASE_PACK);
    }
    changed = ImGui::Combo("Pack", &pack_idx, packs);
    set_tooltip("What pack it will belong to.");
    
    //New pack button.
    ImGui::SameLine();
    if(ImGui::Button("New pack...")) {
        open_new_pack_dialog();
    }
    set_tooltip("Create a new pack.");
    
    *pack = game.content.packs.manifests_with_base[pack_idx];
    return changed;
}


/**
 * @brief Processes the dialog for creating a new pack.
 */
void editor::process_gui_new_pack_dialog() {
    static string internal_name = "my_pack";
    static string name = "My pack!";
    static string description;
    static string maker;
    string problem;
    bool hit_create_button = false;
    
    //Internal name input.
    ImGui::FocusOnInputText(needs_new_pack_text_focus);
    if(
        ImGui::InputText(
            "Internal name", &internal_name,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hit_create_button = true;
    }
    set_tooltip(
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
    set_tooltip("Proper name of the new pack.");
    
    //Description input.
    if(
        ImGui::InputText(
            "Description", &description,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hit_create_button = true;
    }
    set_tooltip("A description of the pack.");
    
    //Maker input.
    if(
        ImGui::InputText(
            "Maker", &maker,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hit_create_button = true;
    }
    set_tooltip("Who made the pack. So really, type your name or nickname.");
    
    //File explanation text.
    string explanation =
        "These properties can be changed later by editing the "
        "pack's data file.\n"
        "There are also more properties; check the manual "
        "for more information!\n"
        "Pack data file path: " +
        (
            internal_name.empty() ?
            "" :
            FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" +
            internal_name + "/" +
            FILE_NAMES::PACK_DATA
        );
    ImGui::TextWrapped("%s", explanation.c_str());
    
    //Open manual button.
    if(ImGui::Button("Open manual")) {
        open_manual("making.html#packs");
    }
    
    //Check if everything's ok.
    if(internal_name.empty()) {
        problem = "You have to type an internal name first!";
    } else if(!is_internal_name_good(internal_name)) {
        problem =
            "The internal name should only have lowercase letters,\n"
            "numbers, and underscores!";
    } else {
        for(const auto &p : game.content.packs.manifests_with_base) {
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
    set_tooltip(
        problem.empty() ? "Create the pack!" : problem
    );
    
    //Creation logic.
    if(hit_create_button) {
        if(!problem.empty()) return;
        game.content.create_pack(
            internal_name, name, description, maker
        );
        internal_name.clear();
        name.clear();
        description.clear();
        maker.clear();
        close_top_dialog();
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
bool editor::process_gui_size_widgets(
    const char* label, point &size, float v_speed,
    bool keep_aspect_ratio,
    bool keep_area,
    float min_size
) {
    bool ret = false;
    point new_size = size;
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
void editor::process_gui_status_bar_text() {
    float error_flash_time_ratio = op_error_flash_timer.get_ratio_left();
    if(error_flash_time_ratio > 0.0f) {
        ImVec4 normal_color_v = ImGui::GetStyle().Colors[ImGuiCol_Text];
        ALLEGRO_COLOR normal_color;
        normal_color.r = normal_color_v.x;
        normal_color.g = normal_color_v.y;
        normal_color.b = normal_color_v.z;
        normal_color.a = normal_color_v.w;
        ALLEGRO_COLOR error_flash_color =
            interpolate_color(
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
    ImGui::Text("%s", (status_text.empty() ? "Ready." : status_text.c_str()));
    if(error_flash_time_ratio) {
        ImGui::PopStyleColor();
    }
}


/**
 * @brief Processes the Dear ImGui unsaved changes confirmation dialog
 * for this frame.
 */
void editor::process_gui_unsaved_changes_dialog() {
    //Explanation 1 text.
    size_t nr_unsaved_changes = changes_mgr.get_unsaved_changes();
    string explanation1_str =
        "You have " +
        amount_str((int) nr_unsaved_changes, "unsaved change") +
        ", made in the last " +
        time_to_str3(
            changes_mgr.get_unsaved_time_delta(),
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
        changes_mgr.get_unsaved_warning_action_long() + "?";
    ImGui::SetupCentering(ImGui::CalcTextSize(explanation2_str.c_str()).x);
    ImGui::Text("%s", explanation2_str.c_str());
    
    //Cancel button.
    ImGui::SetupCentering(180 + 180 + 180 + 20);
    if(ImGui::Button("Cancel", ImVec2(180, 30))) {
        close_top_dialog();
    }
    set_tooltip("Never mind and go back.", "Esc");
    
    //Save and then perform the action.
    ImGui::SameLine(0.0f, 10);
    if(ImGui::Button("Save", ImVec2(180, 30))) {
        close_top_dialog();
        const std::function<bool()> &save_callback =
            changes_mgr.get_unsaved_warning_save_callback();
        const std::function<void()> &action_callback =
            changes_mgr.get_unsaved_warning_action_callback();
        if(save_callback()) {
            action_callback();
        }
    }
    set_tooltip(
        "Save first, then " +
        changes_mgr.get_unsaved_warning_action_short() + ".",
        "Ctrl + S"
    );
    
    //Perform the action without saving button.
    ImGui::SameLine(0.0f, 10);
    if(ImGui::Button("Don't save", ImVec2(180, 30))) {
        close_top_dialog();
        const std::function<void()> action_callback =
            changes_mgr.get_unsaved_warning_action_callback();
        action_callback();
    }
    string dont_save_tooltip =
        changes_mgr.get_unsaved_warning_action_short() +
        " without saving.";
    dont_save_tooltip[0] = toupper(dont_save_tooltip[0]);
    set_tooltip(dont_save_tooltip, "Ctrl + D");
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
bool editor::saveable_tree_node(const string &category, const string &label) {
    string node_name = get_name() + "/" + category + "/" + label;
    ImGui::SetNextItemOpen(game.options.editor_open_nodes[node_name]);
    bool is_open = ImGui::TreeNode(label.c_str());
    game.options.editor_open_nodes[node_name] = is_open;
    return is_open;
}


/**
 * @brief Sets the status bar, and notifies the user of an error,
 * if it is an error, by flashing the text.
 *
 * @param text Text to put in the status bar.
 * @param error Whether there was an error or not.
 */
void editor::set_status(const string &text, bool error) {
    status_text = text;
    if(error) {
        op_error_flash_timer.start();
        op_error_pos = game.mouse_cursor.s_pos;
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
void editor::set_tooltip(
    const string &explanation, const string &shortcut,
    const WIDGET_EXPLANATION widget_explanation
) {
    if(!game.options.editor_show_tooltips) {
        return;
    }
    
    if(last_input_was_keyboard) {
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
point editor::snap_point_to_axis(const point &p, const point &anchor) {
    float h_diff = fabs(p.x - anchor.x);
    float v_diff = fabs(p.y - anchor.y);
    if(h_diff > v_diff) {
        return point(p.x, anchor.y);
    } else {
        return point(anchor.x, p.y);
    }
}


/**
 * @brief Snaps a point to the nearest grid intersection.
 *
 * @param p Point to snap.
 * @param grid_interval Current grid interval.
 * @return The snapped point.
 */
point editor::snap_point_to_grid(const point &p, float grid_interval) {
    return
        point(
            round(p.x / grid_interval) * grid_interval,
            round(p.y / grid_interval) * grid_interval
        );
}


/**
 * @brief Unloads loaded editor-related content.
 */
void editor::unload() {
    if(bmp_editor_icons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; i++) {
            al_destroy_bitmap(editor_icons[i]);
            editor_icons[i] = nullptr;
        }
        game.content.bitmaps.list.free(bmp_editor_icons);
        bmp_editor_icons = nullptr;
    }
    custom_cat_name_idxs.clear();
    custom_cat_types.clear();
    game.mouse_cursor.hide();
}


/**
 * @brief Updates the history list, by adding a new entry or bumping it up.
 *
 * @param manifest Manifest of the entry's content.
 * @param name Proper name of the entry.
 */
void editor::update_history(
    const content_manifest &manifest, const string &name
) {
    string final_name = name.empty() ? manifest.internal_name : name;
    
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
    
    if(history.size() > get_history_size()) {
        history.erase(history.begin() + history.size() - 1);
    }
    
    //Save the history in the options.
    save_options();
}


/**
 * @brief Updates the Dear ImGui style based on the player's options.
 */
void editor::update_style() {

    ImGuiStyle* style = &ImGui::GetStyle();
    style->FrameRounding = 3;
    style->IndentSpacing = 25;
    style->GrabMinSize = 15;
    style->ScrollbarSize = 16;
    style->WindowRounding = 5;
    style->PopupRounding = 5;
    style->GrabRounding = 4;
    style->ScrollbarRounding = 12;
    
    if(!game.options.editor_use_custom_style) {
        //Use the default style.
        memcpy(
            &(ImGui::GetStyle().Colors),
            game.imgui_default_style,
            sizeof(ImVec4) * ImGuiCol_COUNT
        );
        
    } else {
        //Use the custom style.
        
        ALLEGRO_COLOR pri = game.options.editor_primary_color;
        ALLEGRO_COLOR sec = game.options.editor_secondary_color;
        ALLEGRO_COLOR txt = game.options.editor_text_color;
        
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
void editor::update_transformations() {
    //World coordinates to screen coordinates.
    point canvas_center(
        (canvas_tl.x + canvas_br.x) / 2.0,
        (canvas_tl.y + canvas_br.y) / 2.0
    );
    game.world_to_screen_transform = game.identity_transform;
    al_translate_transform(
        &game.world_to_screen_transform,
        -game.cam.pos.x + canvas_center.x / game.cam.zoom,
        -game.cam.pos.y + canvas_center.y / game.cam.zoom
    );
    al_scale_transform(
        &game.world_to_screen_transform, game.cam.zoom, game.cam.zoom
    );
    
    //Screen coordinates to world coordinates.
    game.screen_to_world_transform = game.world_to_screen_transform;
    al_invert_transform(&game.screen_to_world_transform);
}


/**
 * @brief Zooms to the specified level, keeping the mouse cursor in
 * the same spot.
 *
 * @param new_zoom New zoom level.
 */
void editor::zoom_with_cursor(float new_zoom) {
    //Keep a backup of the old mouse coordinates.
    point old_mouse_pos = game.mouse_cursor.w_pos;
    
    //Do the zoom.
    game.cam.set_zoom(clamp(new_zoom, zoom_min_level, zoom_max_level));
    update_transformations();
    
    //Figure out where the mouse will be after the zoom.
    game.mouse_cursor.w_pos = game.mouse_cursor.s_pos;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor.w_pos.x, &game.mouse_cursor.w_pos.y
    );
    
    //Readjust the transformation by shifting the camera
    //so that the cursor ends up where it was before.
    game.cam.set_pos(
        point(
            game.cam.pos.x += (old_mouse_pos.x - game.mouse_cursor.w_pos.x),
            game.cam.pos.y += (old_mouse_pos.y - game.mouse_cursor.w_pos.y)
        )
    );
    
    //Update the mouse coordinates again.
    update_transformations();
    game.mouse_cursor.w_pos = game.mouse_cursor.s_pos;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor.w_pos.x, &game.mouse_cursor.w_pos.y
    );
}


/**
 * @brief Constructs a new changes manager object.
 *
 * @param ed Pointer to the editor.
 */
editor::changes_manager::changes_manager(editor* ed) :
    ed(ed) {
    
}


/**
 * @brief If there are no unsaved changes, performs a given action.
 * Otherwise, it opens a dialog asking the user if they
 * want to cancel, save and then do the action, or do the action without saving.
 *
 * @param pos Screen coordinates to show the warning on.
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
bool editor::changes_manager::ask_if_unsaved(
    const point &pos,
    const string &action_long, const string &action_short,
    const std::function<void()> &action_callback,
    const std::function<bool()> &save_callback
) {
    if(unsaved_changes > 0) {
        unsaved_warning_action_long = action_long;
        unsaved_warning_action_short = action_short;
        unsaved_warning_action_callback = action_callback;
        unsaved_warning_save_callback = save_callback;
        
        ed->open_dialog(
            "Unsaved changes!",
            std::bind(&editor::process_gui_unsaved_changes_dialog, ed)
        );
        ed->dialogs.back()->custom_pos = game.mouse_cursor.s_pos;
        ed->dialogs.back()->custom_size = point(580, 0);
        ed->dialogs.back()->event_callback =
        [this] (const ALLEGRO_EVENT * ev) {
            if(ev->type == ALLEGRO_EVENT_KEY_DOWN) {
                if(
                    ed->key_check(ev->keyboard.keycode, ALLEGRO_KEY_S, true)
                ) {
                    ed->close_top_dialog();
                    const std::function<bool()> &save_callback =
                        this->get_unsaved_warning_save_callback();
                    const std::function<void()> &action_callback =
                        this->get_unsaved_warning_action_callback();
                    if(save_callback()) {
                        action_callback();
                    }
                } else if(
                    ed->key_check(ev->keyboard.keycode, ALLEGRO_KEY_D, true)
                ) {
                    ed->close_top_dialog();
                    const std::function<void()> &action_callback =
                        this->get_unsaved_warning_action_callback();
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
bool editor::changes_manager::exists_on_disk() const {
    return on_disk;
}


/**
 * @brief Returns how many unsaved changes have been made so far since the
 * last save.
 *
 * @return The amount.
 */
size_t editor::changes_manager::get_unsaved_changes() const {
    return unsaved_changes;
}


/**
 * @brief Returns how long ago was the last time the player went from saved
 * to unsaved, in seconds.
 *
 * @return The time, or 0 if there are no unsaved changes.
 */
float editor::changes_manager::get_unsaved_time_delta() const {
    if(unsaved_changes == 0) return 0.0f;
    return game.time_passed - unsaved_time;
}


/**
 * @brief Returns the current unsaved changes warning long action text.
 *
 * @return The text.
 */
const string &editor::changes_manager::get_unsaved_warning_action_long()
const {
    return unsaved_warning_action_long;
}


/**
 * @brief Returns the current unsaved changes warning short action text.
 *
 * @return The text.
 */
const string &editor::changes_manager::get_unsaved_warning_action_short()
const {
    return unsaved_warning_action_short;
}


/**
 * @brief Returns the current unsaved changes warning action callback.
 *
 * @return The callback.
 */
const std::function<void()> &
editor::changes_manager::get_unsaved_warning_action_callback() const {
    return unsaved_warning_action_callback;
}


/**
 * @brief Returns the current unsaved changes warning save callback.
 *
 * @return The callback.
 */
const std::function<bool()> &
editor::changes_manager::get_unsaved_warning_save_callback() const {
    return unsaved_warning_save_callback;
}


/**
 * @brief Returns whether there are unsaved changes or not.
 *
 * @return Whether there are unsaved changes.
 */
bool editor::changes_manager::has_unsaved_changes() {
    return unsaved_changes != 0;
}


/**
 * @brief Marks that the user has made new changes, which have obviously not yet
 * been saved.
 */
void editor::changes_manager::mark_as_changed() {
    if(unsaved_changes == 0) {
        unsaved_changes++;
        unsaved_time = game.time_passed;
    } else {
        unsaved_changes++;
    }
}


/**
 * @brief Marks the state of the editor's file as not existing on disk yet.
 * This also marks it as having unsaved changes.
 */
void editor::changes_manager::mark_as_non_existent() {
    on_disk = false;
    mark_as_changed();
}


/**
 * @brief Marks the state of the editor's file as saved.
 * The unsaved changes warning dialog does not set this, so this should be
 * called manually in those cases.
 */
void editor::changes_manager::mark_as_saved() {
    unsaved_changes = 0;
    unsaved_time = 0.0f;
    on_disk = true;
}


/**
 * @brief Resets the state of the changes manager.
 */
void editor::changes_manager::reset() {
    unsaved_changes = 0;
    unsaved_time = 0.0f;
    on_disk = true;
}


/**
 * @brief Constructs a new command object.
 *
 * @param f Function to run.
 * @param n Name.
 */
editor::command::command(command_func_t f, const string &n) :
    func(f),
    name(n) {
}


/**
 * @brief Runs the function.
 *
 * @param input_value Input value, if needed by the command.
 */
void editor::command::run(float input_value) {
    func(input_value);
}


/**
 * @brief Processes the dialog for this frame.
 */
void editor::dialog_info::process() {
    if(!is_open) return;
    
    point size = custom_size;
    if(custom_size.x == -1.0f && custom_size.y == -1.0f) {
        size.x = game.win_w * 0.8;
        size.y = game.win_h * 0.8;
    }
    point pos = custom_pos;
    if(custom_pos.x == -1.0f && custom_pos.y == -1.0f) {
        pos.x = game.win_w / 2.0f;
        pos.y = game.win_h / 2.0f;
    }
    point tl = pos - size / 2.0f;
    point br = pos + size / 2.0f;
    if(tl.x < 0.0f) {
        pos.x -= tl.x;
    }
    if(br.x > game.win_w) {
        pos.x -= br.x - game.win_w;
    }
    if(tl.y < 0.0f) {
        pos.y -= tl.y;
    }
    if(br.y > game.win_h) {
        pos.y -= br.y - game.win_h;
    }
    ImGui::SetNextWindowPos(
        ImVec2(pos.x, pos.y),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_Once);
    ImGui::OpenPopup((title + "##dialog").c_str());
    
    if(
        ImGui::BeginPopupModal(
            (title + "##dialog").c_str(), &is_open
        )
    ) {
    
        process_callback();
        
        ImGui::EndPopup();
    }
}


/**
 * @brief Constructs a new picker info object.
 *
 * @param editor_ptr Pointer to the editor in charge.
 */
editor::picker_info::picker_info(editor* editor_ptr) :
    editor_ptr(editor_ptr) {
}


/**
 * @brief Processes the picker for this frame.
 */
void editor::picker_info::process() {
    vector<string> top_cat_names;
    vector<vector<string> > sec_cat_names;
    vector<vector<vector<picker_item> > > final_items;
    string filter_lower = str_to_lower(filter);
    
    for(size_t i = 0; i < items.size(); i++) {
        if(!filter.empty()) {
            string name_lower = str_to_lower(items[i].name);
            if(name_lower.find(filter_lower) == string::npos) {
                continue;
            }
        }
        
        size_t top_cat_idx = INVALID;
        for(size_t c = 0; c < top_cat_names.size(); c++) {
            if(top_cat_names[c] == items[i].top_category) {
                top_cat_idx = c;
                break;
            }
        }
        
        if(top_cat_idx == INVALID) {
            top_cat_names.push_back(items[i].top_category);
            sec_cat_names.push_back(vector<string>());
            final_items.push_back(vector<vector<picker_item> >());
            top_cat_idx = top_cat_names.size() - 1;
        }
        
        size_t sec_cat_idx = INVALID;
        for(size_t c = 0; c < sec_cat_names[top_cat_idx].size(); c++) {
            if(sec_cat_names[top_cat_idx][c] == items[i].sec_category) {
                sec_cat_idx = c;
                break;
            }
        }
        
        if(sec_cat_idx == INVALID) {
            sec_cat_names[top_cat_idx].push_back(items[i].sec_category);
            final_items[top_cat_idx].push_back(vector<picker_item>());
            sec_cat_idx = sec_cat_names[top_cat_idx].size() - 1;
        }
        
        final_items[top_cat_idx][sec_cat_idx].push_back(items[i]);
    }
    
    auto try_make_new = [this] () {
        if(filter.empty()) return;
        
        if(
            !new_item_top_cat_choices.empty() &&
            new_item_top_cat.empty()
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
                new_item_top_cat == items[i].top_category
            ) {
                is_really_new = false;
                break;
            }
        }
        
        pick_callback(filter, new_item_top_cat, "", nullptr, is_really_new);
        if(dialog_ptr) {
            dialog_ptr->is_open = false;
        }
    };
    
    if(can_make_new) {
        ImGui::PushStyleColor(
            ImGuiCol_Button, (ImVec4) ImColor(192, 32, 32)
        );
        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered, (ImVec4) ImColor(208, 48, 48)
        );
        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive, (ImVec4) ImColor(208, 32, 32)
        );
        if(ImGui::Button("+", ImVec2(64.0f, 32.0f))) {
            try_make_new();
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
    }
    
    string filter_widget_hint =
        can_make_new ?
        "Search filter or new item name" :
        "Search filter";
        
    ImGui::FocusOnInputText(needs_filter_box_focus);
    if(
        ImGui::InputTextWithHint(
            "##filter", filter_widget_hint.c_str(), &filter,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        if(filter.empty()) return;
        
        if(can_make_new) {
            try_make_new();
        } else {
            size_t possible_choices = 0;
            for(size_t c = 0; c < final_items.size(); c++) {
                possible_choices += final_items[c].size();
            }
            if(possible_choices > 0) {
                pick_callback(
                    final_items[0][0][0].name,
                    final_items[0][0][0].top_category,
                    final_items[0][0][0].sec_category,
                    final_items[0][0][0].info,
                    false
                );
                if(dialog_ptr) {
                    dialog_ptr->is_open = false;
                }
            }
        }
    }
    
    if(editor_ptr->popup("newItemCategory")) {
        ImGui::Text("%s", "What is the category of the new item?");
        
        if(
            ImGui::BeginChild(
                "categoryList", ImVec2(0.0f, 80.0f), ImGuiChildFlags_Borders
            )
        ) {
            for(size_t c = 0; c < new_item_top_cat_choices.size(); c++) {
                if(ImGui::Selectable(new_item_top_cat_choices[c].c_str())) {
                    new_item_top_cat = new_item_top_cat_choices[c];
                    ImGui::CloseCurrentPopup();
                    try_make_new();
                }
            }
            ImGui::EndChild();
        }
        if(ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    if(!list_header.empty()) {
        ImGui::Text("%s", list_header.c_str());
    }
    
    ImGui::BeginChild("list");
    
    for(size_t tc = 0; tc < final_items.size(); tc++) {
    
        bool top_cat_opened = true;
        if(!top_cat_names[tc].empty()) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            top_cat_opened = ImGui::TreeNode(top_cat_names[tc].c_str());
        }
        
        if(!top_cat_opened) continue;
        
        for(size_t sc = 0; sc < final_items[tc].size(); sc++) {
        
            bool sec_cat_opened = true;
            if(!sec_cat_names[tc][sc].empty()) {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                sec_cat_opened = ImGui::TreeNode(sec_cat_names[tc][sc].c_str());
            }
            
            if(!sec_cat_opened) continue;
            
            for(size_t i = 0; i < final_items[tc][sc].size(); i++) {
                picker_item* i_ptr = &final_items[tc][sc][i];
                string widgetId = i2s(tc) + "-" + i2s(sc) + "-" + i2s(i);
                ImGui::PushID(widgetId.c_str());
                
                point button_size;
                
                if(i_ptr->bitmap) {
                
                    ImGui::BeginGroup();
                    
                    point bmp_size(
                        al_get_bitmap_width(i_ptr->bitmap),
                        al_get_bitmap_height(i_ptr->bitmap)
                    );
                    if(bmp_size.x > 0.0f && bmp_size.x > bmp_size.y) {
                        float ratio = bmp_size.y / bmp_size.x;
                        button_size =
                            point(
                                EDITOR::PICKER_IMG_BUTTON_MAX_SIZE,
                                EDITOR::PICKER_IMG_BUTTON_MAX_SIZE * ratio
                            );
                    } else if(bmp_size.y > 0.0f) {
                        float ratio = bmp_size.x / bmp_size.y;
                        button_size =
                            point(
                                EDITOR::PICKER_IMG_BUTTON_MAX_SIZE * ratio,
                                EDITOR::PICKER_IMG_BUTTON_MAX_SIZE
                            );
                    }
                    button_size.x =
                        std::max(
                            button_size.x,
                            EDITOR::PICKER_IMG_BUTTON_MIN_SIZE
                        );
                    button_size.y =
                        std::max(
                            button_size.y,
                            EDITOR::PICKER_IMG_BUTTON_MIN_SIZE
                        );
                        
                    ImGui::PushStyleVar(
                        ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f)
                    );
                    bool button_pressed =
                        ImGui::ImageButton(
                            widgetId + "Button",
                            i_ptr->bitmap,
                            button_size
                        );
                    ImGui::PopStyleVar();
                    
                    if(button_pressed) {
                        pick_callback(
                            i_ptr->name, i_ptr->top_category, i_ptr->sec_category, i_ptr->info, false
                        );
                        if(dialog_ptr) {
                            dialog_ptr->is_open = false;
                        }
                    }
                    ImGui::SetNextItemWidth(20.0f);
                    ImGui::TextWrapped("%s", i_ptr->name.c_str());
                    ImGui::Dummy(ImVec2(0.0f, 8.0f));
                    ImGui::EndGroup();
                    
                } else {
                
                    button_size = point(168.0f, 32.0f);
                    if(ImGui::Button(i_ptr->name.c_str(), ImVec2(button_size.x, button_size.y))) {
                        pick_callback(
                            i_ptr->name, i_ptr->top_category, i_ptr->sec_category, i_ptr->info, false
                        );
                        if(dialog_ptr) {
                            dialog_ptr->is_open = false;
                        }
                    }
                    
                }
                
                if(!i_ptr->tooltip.empty()) {
                    editor_ptr->set_tooltip(i_ptr->tooltip);
                }
                
                ImGui::SetupButtonWrapping(
                    button_size.x, (int) (i + 1), (int) final_items[tc][sc].size()
                );
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
editor::picker_item::picker_item(
    const string &name, const string &top_category, const string &sec_category,
    void* info, const string &tooltip, ALLEGRO_BITMAP* bitmap
) :
    name(name),
    top_category(top_category),
    sec_category(sec_category),
    info(info),
    tooltip(tooltip),
    bitmap(bitmap) {
    
}


/**
 * @brief Draws the widget on-screen.
 *
 * @param center Center point.
 * @param size Width and height. If nullptr, no scale handles will be drawn.
 * @param angle Angle. If nullptr, the rotation handle will not be drawn.
 * @param zoom Zoom the widget's components by this much.
 */
void editor::transformation_widget::draw(
    const point* const center, const point* const size,
    const float* const angle, float zoom
) const {
    if(!center) return;
    
    point handles[9];
    float radius;
    get_locations(center, size, angle, handles, &radius, nullptr);
    
    //Draw the rotation handle.
    if(angle && radius >= 0.0f) {
        al_draw_circle(
            center->x, center->y, radius,
            al_map_rgb(64, 64, 192), EDITOR::TW_ROTATION_HANDLE_THICKNESS * zoom
        );
    }
    
    //Draw the outline.
    point corners[4] = {
        handles[0],
        handles[2],
        handles[8],
        handles[6],
    };
    for(unsigned char c = 0; c < 4; c++) {
        size_t c2 = sum_and_wrap(c, 1, 4);
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
void editor::transformation_widget::get_locations(
    const point* const center, const point* const size,
    const float* const angle, point* handles, float* radius,
    ALLEGRO_TRANSFORM* out_transform
) const {
    point size_to_use(EDITOR::TW_DEF_SIZE, EDITOR::TW_DEF_SIZE);
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
    
    float diameter = dist(point(), size_to_use).to_float();
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
point editor::transformation_widget::get_old_center() const {
    return old_center;
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
bool editor::transformation_widget::handle_mouse_down(
    const point &mouse_coords, const point* const center,
    const point* const size, const float* const angle, float zoom
) {
    if(!center) return false;
    
    point handles[9];
    float radius;
    get_locations(center, size, angle, handles, &radius, nullptr);
    
    //Check if the user clicked on a translation or scale handle.
    for(unsigned char h = 0; h < 9; h++) {
        if(dist(handles[h], mouse_coords) <= EDITOR::TW_HANDLE_RADIUS * zoom) {
            if(h == 4) {
                moving_handle = h;
                old_center = *center;
                return true;
            } else if(size) {
                moving_handle = h;
                old_size = *size;
                return true;
            }
        }
    }
    
    //Check if the user clicked on the rotation handle.
    if(angle) {
        dist d(*center, mouse_coords);
        if(
            d >= radius - EDITOR::TW_ROTATION_HANDLE_THICKNESS / 2.0f * zoom &&
            d <= radius + EDITOR::TW_ROTATION_HANDLE_THICKNESS / 2.0f * zoom
        ) {
            moving_handle = 9;
            old_angle = *angle;
            old_mouse_angle = get_angle(*center, mouse_coords);
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
bool editor::transformation_widget::handle_mouse_move(
    const point &mouse_coords, point* center, point* size, float* angle,
    float zoom, bool keep_aspect_ratio, bool keep_area,
    float min_size, bool lock_center
) {
    if(!center) return false;
    
    if(moving_handle == -1) {
        return false;
    }
    
    //Logic for moving the center handle.
    if(moving_handle == 4) {
        *center = mouse_coords;
        return true;
    }
    
    //Logic for moving the rotation handle.
    if(moving_handle == 9 && angle) {
        *angle =
            old_angle +
            get_angle(*center, mouse_coords) - old_mouse_angle;
        return true;
    }
    
    //From here on out, it's logic to move a scale handle.
    if(!size) {
        return false;
    }
    
    ALLEGRO_TRANSFORM t;
    point handles[9];
    float radius;
    get_locations(center, size, angle, handles, &radius, &t);
    al_invert_transform(&t);
    
    point transformed_mouse = mouse_coords;
    point transformed_center = *center;
    point new_size = old_size;
    al_transform_coordinates(&t, &transformed_mouse.x, &transformed_mouse.y);
    al_transform_coordinates(&t, &transformed_center.x, &transformed_center.y);
    bool scaling_x = false;
    bool scaling_y = false;
    
    switch(moving_handle) {
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
    
    switch(moving_handle) {
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
    
    if(keep_aspect_ratio && old_size.x != 0.0f && old_size.y != 0.0f) {
        float scale_to_use;
        float w_scale = new_size.x / old_size.x;
        float h_scale = new_size.y / old_size.y;
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
        scale_to_use = std::max(min_size / old_size.x, scale_to_use);
        scale_to_use = std::max(min_size / old_size.y, scale_to_use);
        new_size = old_size * scale_to_use;
        
    } else if(keep_area && old_size.x != 0.0f && old_size.y != 0.0f) {
        bool by_x;
        float w_scale = new_size.x / old_size.x;
        float h_scale = new_size.y / old_size.y;
        double old_area = (double) old_size.x * (double) old_size.y;
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
    
    switch(moving_handle) {
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
    
    switch(moving_handle) {
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
    
    point new_center = transformed_center;
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
bool editor::transformation_widget::handle_mouse_up() {
    if(moving_handle == -1) {
        return false;
    }
    
    moving_handle = -1;
    return true;
}


/**
 * @brief Is the user currently moving the central handle?
 *
 * @return Whether the user is moving the handle.
 */
bool editor::transformation_widget::is_moving_center_handle() {
    return (moving_handle == 4);
}


/**
 * @brief Is the user currently moving a handle?
 *
 * @return Whether the user is moving a handle.
 */
bool editor::transformation_widget::is_moving_handle() {
    return (moving_handle != -1);
}
