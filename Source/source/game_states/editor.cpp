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

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../libs/imgui/imgui_impl_allegro5.h"
#include "../libs/imgui/imgui_internal.h"
#include "../libs/imgui/imgui_stdlib.h"
#include "../load.h"
#include "../mob_categories/mob_category.h"
#include "../mob_types/mob_type.h"
#include "../utils/imgui_utils.h"
#include "../utils/string_utils.h"


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


/* ----------------------------------------------------------------------------
 * Initializes editor class stuff.
 */
editor::editor() :
    bmp_editor_icons(nullptr),
    canvas_separator_x(-1),
    changes_mgr(this),
    double_click_time(0),
    escape_was_pressed(false),
    is_alt_pressed(false),
    is_ctrl_pressed(false),
    is_m1_pressed(false),
    is_m2_pressed(false),
    is_m3_pressed(false),
    is_mouse_in_gui(false),
    is_shift_pressed(false),
    last_mouse_click(INVALID),
    last_mouse_click_sub_state(INVALID),
    last_input_was_keyboard(false),
    loaded_content_yet(false),
    mouse_drag_confirmed(false),
    op_error_flash_timer(EDITOR::OP_ERROR_FLASH_DURATION),
    state(0),
    sub_state(0),
    zoom_max_level(0),
    zoom_min_level(0) {
    
    editor_icons.reserve(N_EDITOR_ICONS);
    for(size_t i = 0; i < N_EDITOR_ICONS; ++i) {
        editor_icons.push_back(NULL);
    }
}


/* ----------------------------------------------------------------------------
 * Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the screen,
 * where it's hard to see.
 * min_coords:
 *   Top-left coordinates of the content to focus on.
 * max_coords:
 *   Bottom-right coordinates of the content to focus on.
 * instantaneous:
 *   If true, the camera moves there instantaneously. If false, it smoothly
 *   gets there over time.
 */
void editor::center_camera(
    const point &min_coords, const point &max_coords,
    const bool instantaneous
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


/* ----------------------------------------------------------------------------
 * Closes the topmost dialog.
 */
void editor::close_top_dialog() {
    if(dialogs.empty()) return;
    dialogs.back()->is_open = false;
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the editor. This is meant to
 * be run after the editor's own logic code.
 */
void editor::do_logic_post() {
    escape_was_pressed = false;
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the editor. This is meant to
 * be run before the editor's own logic code.
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


/* ----------------------------------------------------------------------------
 * Draws the grid, using the current game camera.
 * interval:
 *   Interval between grid lines.
 * major_color:
 *   Color to use for major lines. These are lines that happen at major
 *   milestones (i.e. twice the interval).
 * minor_color:
 *   Color to use for minor lines. These are lines that aren't major.
 */
void editor::draw_grid(
    const float interval,
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


/* ----------------------------------------------------------------------------
 * Draws a small red X on the cursor, signifying an operation has failed.
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


/* ----------------------------------------------------------------------------
 * Returns the maximum number of history entries for this editor.
 */
size_t editor::get_history_size() const {
    return EDITOR::DEF_MAX_HISTORY_SIZE;
}


/* ----------------------------------------------------------------------------
 * Returns the position of the last widget, in screen coordinates.
 */
point editor::get_last_widget_pos() {
    return
        point(
            ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x / 2.0,
            ImGui::GetItemRectMin().y + ImGui::GetItemRectSize().y / 2.0
        );
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event for control-related things.
 * ev:
 *   Event to handle.
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
        
    ImGui_ImplAllegro5_ProcessEvent(&ev);
    
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


/* ----------------------------------------------------------------------------
 * Placeholder for handling a key being "char-typed" anywhere.
 * ev:
 *   Event to process.
 */
void editor::handle_key_char_anywhere(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling a key being "char-typed" in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling a key being pressed down anywhere.
 * ev:
 *   Event to process.
 */
void editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling a key being pressed down in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling a key being released anywhere.
 * ev:
 *   Event to process.
 */
void editor::handle_key_up_anywhere(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling a key being released in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_key_up_canvas(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the left mouse button being double-clicked
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the left mouse button being pressed down
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the left mouse button being dragged
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the left mouse button released
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the middle mouse button being double-clicked
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the middle mouse button being pressed down
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the middle mouse button being dragged
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the middle mouse button being released
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_mmb_up(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the mouse coordinates being updated.
 * ev:
 *   Event to process.
 */
void editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the mouse wheel being turned in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the right mouse button being double-clicked
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the right mouse button being pressed down
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the right mouse button being dragged
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Placeholder for handling the right mouse button being released
 * in the canvas.
 * ev:
 *   Event to process.
 */
void editor::handle_rmb_up(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Displays a popup, if applicable, and fills it with a text input for the
 * user to type something in.
 * Returns true if the user pressed Return or the Ok button.
 * label:
 *   Name of the popup.
 * prompt:
 *   What to prompt to the user. e.g.: "New name:"
 * text:
 *   Pointer to the starting text, as well as the user's final text.
 */
bool editor::input_popup(
    const char* label, const char* prompt, string* text
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        if(escape_was_pressed) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::Text("%s", prompt);
        if(!ImGui::IsAnyItemActive()) {
            ImGui::SetKeyboardFocusHere();
        }
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


/* ----------------------------------------------------------------------------
 * Returns whether or not the pressed key corresponds to the specified
 * key combination. Used for keyboard shortcuts.
 * pressed_key:
 *   Key that the user pressed.
 * match_key:
 *   Key that must be matched in order to return true.
 * needs_ctrl:
 *   If true, only returns true if Ctrl was also pressed.
 * needs_shift:
 *   If true, only returns true if Shift was also pressed.
 */
bool editor::key_check(
    const int pressed_key, const int match_key,
    const bool needs_ctrl, const bool needs_shift
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


/* ----------------------------------------------------------------------------
 * Exits out of the editor, with a fade.
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


/* ----------------------------------------------------------------------------
 * Displays a popup, if applicable, and fills it with selectable items
 * from a list. Returns true if one of the items was clicked on,
 * false otherwise.
 * label:
 *   Name of the popup.
 * items:
 *   List of items.
 * picked_item:
 *   If an item was picked, set this to its name.
 */
bool editor::list_popup(
    const char* label, const vector<string> &items, string* picked_item
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        if(escape_was_pressed) {
            ImGui::CloseCurrentPopup();
        }
        for(size_t i = 0; i < items.size(); ++i) {
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


/* ----------------------------------------------------------------------------
 * Loads content common for all editors.
 */
void editor::load() {
    game.mouse_cursor.show();
    
    bmp_editor_icons =
        load_bmp(game.asset_file_names.editor_icons, NULL, true, false);
    if(bmp_editor_icons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; ++i) {
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
    
    last_input_was_keyboard = false;
    changes_mgr.reset();
    
    game.fade_mgr.start_fade(true, nullptr);
    
    //Set the editor style.
    update_style();
    
    ImGui::Reset();
}


/* ----------------------------------------------------------------------------
 * Loads all mob types into the custom_cat_types list.
 * is_area_editor:
 *   If true, mob types that do not appear in the area editor will not
 *   be counted for here.
 */
void editor::load_custom_mob_cat_types(const bool is_area_editor) {
    //Load.
    for(size_t c = 0; c < N_MOB_CATEGORIES; ++c) {
        mob_category* c_ptr = game.mob_categories.get((MOB_CATEGORIES) c);
        vector<string> type_names;
        c_ptr->get_type_names(type_names);
        
        for(size_t tn = 0; tn < type_names.size(); ++tn) {
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
    for(size_t c = 0; c < custom_cat_types.size(); ++c) {
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


/* ----------------------------------------------------------------------------
 * Opens a dialog.
 * title:
 *   Title of the dialog window. This is normally a request to the user,
 *   like "Pick an area.".
 * process_callback:
 *   A function to call when it's time to process the contents inside
 *   the dialog.
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


/* ----------------------------------------------------------------------------
 * Opens a dialog with "picker" widgets inside, with the given content.
 * title:
 *   Title of the picker's dialog window. This is normally
 *   a request to the user, like "Pick an area.".
 * items:
 *   List of items to populate the picker with.
 * pick_callback:
 *   A function to call when the user clicks an item or enters a new one.
 *   This function's first argument is the name of the item.
 *   Its second argument is the category of the item, or an empty string.
 *   Its third argument is whether it's a new item or not.
 * list_header:
 *   If not-empty, display this text above the list.
 * can_make_new:
 *   If true, the user can create a new element, by writing its
 *   name on the textbox, and pressing the "+" button.
 * filter:
 *   Filter of names. Only items that match this will appear.
 */
void editor::open_picker_dialog(
    const string &title,
    const vector<picker_item> &items,
    const std::function <void(
        const string &, const string &, const bool
    )> &pick_callback,
    const string &list_header,
    const bool can_make_new,
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
    new_picker->dialog_ptr = new_dialog;
}


/* ----------------------------------------------------------------------------
 * Creates widgets with the goal of placing a disabled text widget to the
 * right side of the panel.
 * title:
 *   Title to write.
 */
void editor::panel_title(const char* title) {
    ImGui::SameLine(
        ImGui::GetContentRegionAvail().x -
        (ImGui::CalcTextSize(title).x + 1)
    );
    ImGui::TextDisabled("%s", title);
}


/* ----------------------------------------------------------------------------
 * Begins a Dear ImGui popup, with logic to close it if Escape was pressed.
 * label:
 *   The popup's label.
 * flags:
 *   Any Dear ImGui popup flags.
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


/* ----------------------------------------------------------------------------
 * Processes all currently open dialogs for this frame.
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
            ++d;
        }
    }
    
    //Process the latest one.
    if(!dialogs.empty()) {
        dialogs.back()->process();
    }
}


/* ----------------------------------------------------------------------------
 * Processes the widgets that allow the player to set a custom editor style.
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


/* ----------------------------------------------------------------------------
 * Processes the widgets that show the editor's history.
 * name_display_callback:
 *   When an entry's name needs to be displayed as button text, this function
 *   gets called with the entry name as an argument, to determine what the
 *   final button text will be.
 * pick_callback:
 *   Code to run when an entry is picked.
 */
void editor::process_gui_history(
    const std::function<string(const string &)> &name_display_callback,
    const std::function<void(const string &)> &pick_callback
) {
    if(saveable_tree_node("load", "History")) {
    
        if(!history.empty() && !history[0].empty()) {
        
            for(size_t h = 0; h < history.size(); ++h) {
                string name = history[h];
                if(name.empty()) continue;
                
                string button_text = name_display_callback(name);
                
                //History number text.
                ImGui::Text("%i.", (int) (h + 1));
                
                //History entry button.
                ImGui::SameLine();
                if(ImGui::Button((button_text + "##" + i2s(h)).c_str())) {
                    pick_callback(name);
                }
            }
            
        } else {
        
            //No history text.
            ImGui::TextDisabled("(Empty)");
            
        }
        
        ImGui::TreePop();
        
    }
}


/* ----------------------------------------------------------------------------
 * Processes the category and type widgets that allow a user to select a mob
 * type.
 * Returns true if the user changed the category or type, false otherwise.
 * custom_cat_name:
 *   Pointer to the custom category name reflected in the combo box.
 * type:
 *   Pointer to the type reflected in the combo box.
 */
bool editor::process_gui_mob_type_widgets(
    string* custom_cat_name, mob_type** type
) {
    bool result = false;
    
    //These are used to communicate with the picker dialog, since that one
    //is processed somewhere else entirely.
    static bool internal_changed_by_dialog = false;
    static string internal_custom_cat_name;
    static mob_type* internal_mob_type = NULL;
    
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
    ImGui::Columns(2, NULL, false);
    ImGui::SetColumnWidth(-1, 51.0f);
    
    //Search button.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9.0f, 9.0f));
    bool search_button_pressed =
        ImGui::ImageButton(
            "searchButton",
            editor_icons[ICON_SEARCH],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE),
            ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f)
        );
    ImGui::PopStyleVar();
    
    if(search_button_pressed) {
        vector<picker_item> items;
        for(size_t c = 0; c < custom_cat_types.size(); ++c) {
            for(size_t n = 0; n < custom_cat_types[c].size(); ++n) {
                mob_type* mt_ptr = custom_cat_types[c][n];
                items.push_back(
                    picker_item(mt_ptr->name, mt_ptr->custom_category_name)
                );
            }
        }
        open_picker_dialog(
            "Pick an object type", items,
        [this] (const string  &n, const string  &c, const bool) {
            //For clarity, this code will NOT be run within the context
            //of editor::process_gui_mob_type_widgets, but will instead
            //be run wherever dialogs are processed.
            internal_changed_by_dialog = true;
            internal_custom_cat_name = c;
            internal_mob_type = NULL;
            size_t custom_cat_idx = custom_cat_name_idxs[c];
            const vector<mob_type*> &types =
                custom_cat_types[custom_cat_idx];
            for(size_t t = 0; t < types.size(); ++t) {
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
    for(size_t c = 0; c < custom_cat_types.size(); ++c) {
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
        internal_mob_type = custom_cat_types[selected_category_idx][0];
    }
    set_tooltip(
        "What category this object belongs to: a Pikmin, a leader, etc."
    );
    
    if(!internal_custom_cat_name.empty()) {
    
        //Object type combobox.
        vector<string> type_names;
        size_t custom_cat_idx = custom_cat_name_idxs[internal_custom_cat_name];
        const vector<mob_type*> &types = custom_cat_types[custom_cat_idx];
        for(size_t t = 0; t < types.size(); ++t) {
            mob_type* t_ptr = types[t];
            type_names.push_back(t_ptr->name);
        }
        
        string selected_type_name;
        if(internal_mob_type) {
            selected_type_name = internal_mob_type->name;
        }
        if(ImGui::Combo("Type", &selected_type_name, type_names, 15)) {
            result = true;
            for(size_t t = 0; t < types.size(); ++t) {
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


/* ----------------------------------------------------------------------------
 * Process the width and height widgets that allow a user to
 * specify the size of something.
 * Returns true if the user changed one of the values.
 * label:
 *   Label for the widgets.
 * size:
 *   Size variable to alter.
 * v_speed:
 *   Variable change speed. Same value you'd pass to ImGui::DragFloat2.
 *   1.0f for default.
 * keep_aspect_ratio:
 *   If true, changing one will change the other in the same ratio.
 * min_size:
 *   Minimum value that either width or height is allowed to have.
 *   Use -FLT_MAX for none.
 * pre_change_callback:
 *   Callback to call when the width or height is changed, before it actually
 *   changes.
 */
bool editor::process_gui_size_widgets(
    const char* label, point &size, const float v_speed,
    const bool keep_aspect_ratio,
    const float min_size,
    const std::function<void()> &pre_change_callback
) {
    bool ret = false;
    point new_size = size;
    if(
        ImGui::DragFloat2(
            label, (float*) &new_size, v_speed, min_size, FLT_MAX
        )
    ) {
        if(pre_change_callback) {
            pre_change_callback();
        }
        
        if(
            !keep_aspect_ratio ||
            size.x == 0.0f || size.y == 0.0f ||
            new_size.x == 0.0f || new_size.y == 0.0f
        ) {
            //Just change them, and forget about keeping the aspect ratio.
            new_size.x = std::max(min_size, new_size.x);
            new_size.y = std::max(min_size, new_size.y);
        } else {
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
        }
        size = new_size;
        
        ret = true;
    }
    
    return ret;
}


/* ----------------------------------------------------------------------------
 * Process the text widget in the status bar. This is responsible for
 * showing the text if there's anything to say, showing "Ready." if there's
 * nothing to say, and coloring the text in case it's an error that needs to be
 * flashed red.
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


/* ----------------------------------------------------------------------------
 * Processes the Dear ImGui unsaved changes confirmation dialog for this frame.
 */
void editor::process_gui_unsaved_changes_dialog() {
    //Explanation 1 text.
    size_t nr_unsaved_changes = changes_mgr.get_unsaved_changes();
    string explanation1_str =
        "You have " +
        nr_and_plural(nr_unsaved_changes, "unsaved change") +
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
    string save_first_button_label =
        "Save, then " +
        changes_mgr.get_unsaved_warning_action_short() +
        "##save";
    if(ImGui::Button(save_first_button_label.c_str(), ImVec2(180, 30))) {
        close_top_dialog();
        const std::function<bool()> save_callback =
            changes_mgr.get_unsaved_warning_save_callback();
        const std::function<void()> action_callback =
            changes_mgr.get_unsaved_warning_action_callback();
        if(save_callback()) {
            action_callback();
        }
    }
    set_tooltip("Save first and then proceed.", "Ctrl + S");
    
    //Perform the action without saving button.
    ImGui::SameLine(0.0f, 10);
    string dont_save_button_label =
        changes_mgr.get_unsaved_warning_action_short() +
        " without saving##noSave";
    dont_save_button_label[0] = toupper(dont_save_button_label[0]);
    if(ImGui::Button(dont_save_button_label.c_str(), ImVec2(180, 30))) {
        close_top_dialog();
        const std::function<void()> action_callback =
            changes_mgr.get_unsaved_warning_action_callback();
        action_callback();
    }
    set_tooltip("Proceed without saving.", "Ctrl + D");
}


/* ----------------------------------------------------------------------------
 * Processes an ImGui::TreeNode, except it pre-emptively opens it or closes it
 * based on the user's preferences. It also saves the user's preferences as
 * they open and close the node.
 * In order for these preferences to be saved onto disk, save_options must
 * be called.
 * category:
 *   Category this node belongs to. This is just a generic term, and
 *   you likely want to use the panel this node belongs to.
 * label:
 *   Label to give to Dear ImGui.
 */
bool editor::saveable_tree_node(const string &category, const string &label) {
    string node_name = get_name() + "/" + category + "/" + label;
    ImGui::SetNextItemOpen(game.options.editor_open_nodes[node_name]);
    bool is_open = ImGui::TreeNode(label.c_str());
    game.options.editor_open_nodes[node_name] = is_open;
    return is_open;
}


/* ----------------------------------------------------------------------------
 * Sets the status bar, and notifies the user of an error, if it is an error,
 * by flashing the text.
 * text:
 *   Text to put in the status bar.
 * error:
 *   Whether there was an error or not.
 */
void editor::set_status(const string &text, const bool error) {
    status_text = text;
    if(error) {
        op_error_flash_timer.start();
        op_error_pos = game.mouse_cursor.s_pos;
    }
}


/* ----------------------------------------------------------------------------
 * Sets the tooltip of the previous widget.
 * explanation:
 *   Text explaining the widget.
 * shortcut:
 *   If the widget has a shortcut key, specify its name here.
 * widget_explanation:
 *   If the way the widget works needs to be explained, specify
 *   the explanation type here.
 */
void editor::set_tooltip(
    const string &explanation, const string &shortcut,
    const WIDGET_EXPLANATIONS widget_explanation
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


/* ----------------------------------------------------------------------------
 * Snaps a point to either the vertical axis or horizontal axis, depending
 * on the anchor point.
 * p:
 *   Point to snap.
 * anchor:
 *   Anchor point.
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


/* ----------------------------------------------------------------------------
 * Snaps a point to the nearest grid intersection.
 * p:
 *   Point to snap.
 * grid_interval:
 *   Current grid interval.
 */
point editor::snap_point_to_grid(const point &p, const float grid_interval) {
    return
        point(
            round(p.x / grid_interval) * grid_interval,
            round(p.y / grid_interval) * grid_interval
        );
}


/* ----------------------------------------------------------------------------
 * Unloads loaded editor-related content.
 */
void editor::unload() {
    if(bmp_editor_icons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; ++i) {
            al_destroy_bitmap(editor_icons[i]);
            editor_icons[i] = NULL;
        }
        al_destroy_bitmap(bmp_editor_icons);
        bmp_editor_icons = NULL;
    }
    custom_cat_name_idxs.clear();
    custom_cat_types.clear();
    game.mouse_cursor.hide();
}


/* ----------------------------------------------------------------------------
 * Updates the history list, by adding a new entry or bumping it up.
 * n:
 *   Name of the entry.
 */
void editor::update_history(const string &n) {
    //First, check if it exists.
    size_t pos = INVALID;
    
    for(size_t h = 0; h < history.size(); ++h) {
        if(history[h] == n) {
            pos = h;
            break;
        }
    }
    
    if(pos == 0) {
        //Already #1? Never mind.
        return;
    } else if(pos == INVALID) {
        //If it doesn't exist, create it and add it to the top.
        history.insert(history.begin(), n);
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        history.erase(history.begin() + pos);
        history.insert(history.begin(), n);
    }
    
    if(history.size() > get_history_size()) {
        history.erase(history.begin() + history.size() - 1);
    }
}


/* ----------------------------------------------------------------------------
 * Updates the Dear ImGui style based on the player's options.
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
            ImVec4(pri.r * 0.4f, pri.g * 0.4f, pri.b * 0.4f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] =
            ImVec4(pri.r * 1.4f, pri.g * 1.4f, pri.b * 1.4f, 0.40f);
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
        colors[ImGuiCol_TabActive] =
            ImLerp(
                colors[ImGuiCol_HeaderActive],
                colors[ImGuiCol_TitleBgActive],
                0.60f
            );
        colors[ImGuiCol_TabUnfocused] =
            ImLerp(
                colors[ImGuiCol_Tab],
                colors[ImGuiCol_TitleBg],
                0.80f
            );
        colors[ImGuiCol_TabUnfocusedActive] =
            ImLerp(
                colors[ImGuiCol_TabActive],
                colors[ImGuiCol_TitleBg],
                0.40f
            );
        colors[ImGuiCol_PlotLines] =
            ImVec4(pri.r * 2, pri.g * 2, pri.b * 2, 1.0f);
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
        colors[ImGuiCol_NavHighlight] =
            ImVec4(sec.r, sec.g, sec.b, 1.0f);
        colors[ImGuiCol_NavWindowingHighlight] =
            ImVec4(pri.r, pri.g, pri.b, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] =
            ImVec4(pri.r * 0.8f, pri.g * 0.8f, pri.b * 0.8f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] =
            ImVec4(pri.r * 0.8f, pri.g * 0.8f, pri.b * 0.8f, 0.35f);
    }
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
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


/* ----------------------------------------------------------------------------
 * Zooms to the specified level, keeping the mouse cursor in the same spot.
 * new_zoom:
 *   New zoom level.
 */
void editor::zoom_with_cursor(const float new_zoom) {
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


/* ----------------------------------------------------------------------------
 * Creates a new changes manager.
 * ed:
 *   Pointer to the editor.
 */
editor::changes_manager::changes_manager(editor* ed) :
    ed(ed),
    unsaved_changes(0),
    unsaved_time(0.0f),
    unsaved_warning_action_callback(nullptr),
    unsaved_warning_save_callback(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * If there are no unsaved changes, performs a given action.
 * Otherwise, it opens a dialog asking the user if they
 * want to cancel, save and then do the action, or do the action without saving.
 * Returns true if there are unsaved changes, false otherwise.
 * pos:
 *   Screen coordinates to show the warning on.
 *   If 0,0, then these will be set to the last processed widget's position.
 * action_long:
 *   String representing the action the user is attempting in a long format.
 *   This is for the main prompt of the warning dialog, so it can be as
 *   long as you want. It should start with a lowercase.
 * action_short:
 *   String representing the action the user is attempting in a short format.
 *   This is for the buttons of the warning dialog, so it should ideally be
 *   only one word. It should start with a lowercase.
 * action_callback:
 *   Code to run to perform the action.
 * save_callback:
 *   Code to run when the unsaved changes must be saved.
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
        ed->dialogs.back()->custom_size = point(580, 100);
        ed->dialogs.back()->event_callback =
        [this] (ALLEGRO_EVENT * ev) {
            if(ev->type == ALLEGRO_EVENT_KEY_DOWN) {
                if(
                    ed->key_check(ev->keyboard.keycode, ALLEGRO_KEY_S, true)
                ) {
                    ed->close_top_dialog();
                    const std::function<bool()> save_callback =
                        this->get_unsaved_warning_save_callback();
                    const std::function<void()> action_callback =
                        this->get_unsaved_warning_action_callback();
                    if(save_callback()) {
                        action_callback();
                    }
                } else if(
                    ed->key_check(ev->keyboard.keycode, ALLEGRO_KEY_D, true)
                ) {
                    ed->close_top_dialog();
                    const std::function<void()> action_callback =
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


/* ----------------------------------------------------------------------------
 * Returns how many unsaved changes have been made so far since the last save.
 */
size_t editor::changes_manager::get_unsaved_changes() const {
    return unsaved_changes;
}


/* ----------------------------------------------------------------------------
 * Returns how long ago was the last time the player went from saved to unsaved,
 * in seconds. Returns 0 if it's currently saved.
 */
float editor::changes_manager::get_unsaved_time_delta() const {
    if(unsaved_changes == 0) return 0.0f;
    return game.time_passed - unsaved_time;
}


/* ----------------------------------------------------------------------------
 * Returns the current unsaved changes warning long action text.
 */
const string &editor::changes_manager::get_unsaved_warning_action_long()
const {
    return unsaved_warning_action_long;
}


/* ----------------------------------------------------------------------------
 * Returns the current unsaved changes warning short action text.
 */
const string &editor::changes_manager::get_unsaved_warning_action_short()
const {
    return unsaved_warning_action_short;
}


/* ----------------------------------------------------------------------------
 * Returns the current unsaved changes warning action callback.
 */
const std::function<void()> &
editor::changes_manager::get_unsaved_warning_action_callback() const {
    return unsaved_warning_action_callback;
}


/* ----------------------------------------------------------------------------
 * Returns the current unsaved changes warning save callback.
 */
const std::function<bool()> &
editor::changes_manager::get_unsaved_warning_save_callback() const {
    return unsaved_warning_save_callback;
}


/* ----------------------------------------------------------------------------
 * Returns whether there are unsaved changes or not.
 */
bool editor::changes_manager::has_unsaved_changes() {
    return unsaved_changes != 0;
}


/* ----------------------------------------------------------------------------
 * Marks that the user has made new changes, which have obviously not yet
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


/* ----------------------------------------------------------------------------
 * Marks the state of the editor's file as saved.
 * The unsaved changes warning dialog does not set this, so this should be
 * called manually in those cases.
 */
void editor::changes_manager::mark_as_saved() {
    unsaved_changes = 0;
    unsaved_time = 0.0f;
}


/* ----------------------------------------------------------------------------
 * Resets the state of the changes manager.
 */
void editor::changes_manager::reset() {
    unsaved_changes = 0;
    unsaved_time = 0.0f;
}


/* ----------------------------------------------------------------------------
 * Creates a new dialog info.
 */
editor::dialog_info::dialog_info() :
    process_callback(nullptr),
    event_callback(nullptr),
    close_callback(nullptr),
    is_open(true),
    custom_pos(-1.0f, -1.0f) {
    
}


/* ----------------------------------------------------------------------------
 * Processes the dialog for this frame.
 */
void editor::dialog_info::process() {
    if(!is_open) return;
    
    point size = custom_size;
    if(custom_size.x == 0.0f && custom_size.y == 0.0f) {
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


/* ----------------------------------------------------------------------------
 * Creates a new picker info.
 * editor_ptr:
 *   Pointer to the editor in charge.
 */
editor::picker_info::picker_info(editor* editor_ptr) :
    editor_ptr(editor_ptr),
    needs_filter_box_focus(true),
    pick_callback(nullptr),
    can_make_new(false),
    dialog_ptr(nullptr) {
}


/* ----------------------------------------------------------------------------
 * Processes the picker for this frame.
 */
void editor::picker_info::process() {
    ImGuiStyle &style = ImGui::GetStyle();
    vector<string> category_names;
    vector<vector<picker_item> > final_items;
    string filter_lower = str_to_lower(filter);
    
    for(size_t i = 0; i < items.size(); ++i) {
        if(!filter.empty()) {
            string name_lower = str_to_lower(items[i].name);
            if(name_lower.find(filter_lower) == string::npos) {
                continue;
            }
        }
        
        size_t cat_index = INVALID;
        for(size_t c = 0; c < category_names.size(); ++c) {
            if(category_names[c] == items[i].category) {
                cat_index = c;
                break;
            }
        }
        
        if(cat_index == INVALID) {
            category_names.push_back(items[i].category);
            final_items.push_back(vector<picker_item>());
            cat_index = category_names.size() - 1;
        }
        
        final_items[cat_index].push_back(items[i]);
    }
    
    auto try_make_new = [this] () {
        if(filter.empty()) return;
        
        if(
            !new_item_category_choices.empty() &&
            new_item_category.empty()
        ) {
            //The user has to pick a category, but hasn't picked yet.
            //Let's show the pop-up and leave.
            ImGui::OpenPopup("newItemCategory");
            return;
        }
        
        bool is_really_new = true;
        for(size_t i = 0; i < items.size(); ++i) {
            if(
                filter == items[i].name &&
                new_item_category == items[i].category
            ) {
                is_really_new = false;
                break;
            }
        }
        
        pick_callback(filter, new_item_category, is_really_new);
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
        ImGui::PushStyleColor
        (ImGuiCol_ButtonActive, (ImVec4) ImColor(208, 32, 32)
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
        
    if(!ImGui::IsAnyItemActive() && needs_filter_box_focus) {
        ImGui::SetKeyboardFocusHere();
        needs_filter_box_focus = false;
    }
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
            for(size_t c = 0; c < final_items.size(); ++c) {
                possible_choices += final_items[c].size();
            }
            if(possible_choices == 1) {
                pick_callback(
                    final_items[0][0].name,
                    final_items[0][0].category,
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
                "categoryList", ImVec2(0.0f, 80.0f), ImGuiChildFlags_Border
            )
        ) {
            for(size_t c = 0; c < new_item_category_choices.size(); ++c) {
                if(ImGui::Selectable(new_item_category_choices[c].c_str())) {
                    new_item_category = new_item_category_choices[c];
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
    
    float picker_x2 =
        ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        
    for(size_t c = 0; c < final_items.size(); ++c) {
    
        bool show = true;
        if(!category_names[c].empty()) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            show = ImGui::TreeNode(category_names[c].c_str());
        }
        
        if(show) {
            for(size_t i = 0; i < final_items[c].size(); ++i) {
                picker_item* i_ptr = &final_items[c][i];
                string widgetId = i2s(c) + "-" + i2s(i);
                ImGui::PushID(widgetId.c_str());
                
                ImVec2 button_size;
                
                if(i_ptr->bitmap) {
                
                    ImGui::BeginGroup();
                    
                    point bmp_size(
                        al_get_bitmap_width(i_ptr->bitmap),
                        al_get_bitmap_height(i_ptr->bitmap)
                    );
                    if(bmp_size.x > 0.0f && bmp_size.x > bmp_size.y) {
                        float ratio = bmp_size.y / bmp_size.x;
                        button_size =
                            ImVec2(
                                EDITOR::PICKER_IMG_BUTTON_MAX_SIZE,
                                EDITOR::PICKER_IMG_BUTTON_MAX_SIZE * ratio
                            );
                    } else if(bmp_size.y > 0.0f) {
                        float ratio = bmp_size.x / bmp_size.y;
                        button_size =
                            ImVec2(
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
                            (widgetId + "Button").c_str(),
                            (void*) i_ptr->bitmap,
                            button_size,
                            ImVec2(0.0f, 0.0f),
                            ImVec2(1.0f, 1.0f)
                        );
                    ImGui::PopStyleVar();
                    
                    if(button_pressed) {
                        pick_callback(
                            i_ptr->name, i_ptr->category, false
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
                
                    button_size = ImVec2(168.0f, 32.0f);
                    if(ImGui::Button(i_ptr->name.c_str(), button_size)) {
                        pick_callback(
                            i_ptr->name, i_ptr->category, false
                        );
                        if(dialog_ptr) {
                            dialog_ptr->is_open = false;
                        }
                    }
                    
                }
                
                float last_x2 = ImGui::GetItemRectMax().x;
                float next_x2 = last_x2 + style.ItemSpacing.x + button_size.x;
                if(i + 1 < final_items[c].size() && next_x2 < picker_x2) {
                    ImGui::SameLine();
                }
                ImGui::PopID();
            }
            
            if(!category_names[c].empty()) {
                ImGui::TreePop();
            }
            
        }
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Creates a picker item.
 * name:
 *   Name of the item.
 * category:
 *   Category it belongs to. If none, use an empty string.
 * bitmap:
 *   Bitmap to display on the item. If none, use NULL.
 */
editor::picker_item::picker_item(
    const string &name, const string &category, ALLEGRO_BITMAP* bitmap
) :
    name(name),
    category(category),
    bitmap(bitmap) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new transformation widget.
 */
editor::transformation_widget::transformation_widget() :
    moving_handle(-1),
    old_angle(0.0f),
    old_mouse_angle(0.0f) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the widget on-screen.
 * center:
 *   Center point.
 * size:
 *   Width and height. If NULL, no scale handles will be drawn.
 * angle:
 *   Angle. If NULL, the rotation handle will not be drawn.
 * zoom:
 *   Zoom the widget's components by this much.
 */
void editor::transformation_widget::draw(
    const point* const center, const point* const size,
    const float* const angle, const float zoom
) const {
    if(!center) return;
    
    point handles[9];
    float radius;
    get_locations(center, size, angle, handles, &radius, NULL);
    
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
    for(unsigned char c = 0; c < 4; ++c) {
        size_t c2 = sum_and_wrap(c, 1, 4);
        al_draw_line(
            corners[c].x, corners[c].y,
            corners[c2].x, corners[c2].y,
            al_map_rgb(32, 32, 160), EDITOR::TW_OUTLINE_THICKNESS * zoom
        );
    }
    
    //Draw the translation and scale handles.
    for(unsigned char h = 0; h < 9; ++h) {
        if(!size && h != 4) continue;
        al_draw_filled_circle(
            handles[h].x, handles[h].y,
            EDITOR::TW_HANDLE_RADIUS * zoom, al_map_rgb(96, 96, 224)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Returns the location of all handles, based on the information it
 * was fed.
 * center:
 *   Center point.
 * size:
 *   Width and height. If NULL, the default size is used.
 * angle:
 *   Angle. If NULL, zero is used.
 * handles:
 *   Return the location of all nine translation and scale handles here.
 * radius:
 *   Return the angle handle's radius here.
 * transform:
 *   If not NULL, return the transformation used here.
 *   The transformation will only rotate and translate, not scale.
 */
void editor::transformation_widget::get_locations(
    const point* const center, const point* const size,
    const float* const angle, point* handles, float* radius,
    ALLEGRO_TRANSFORM* transform
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
    
    for(unsigned char h = 0; h < 9; ++h) {
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
    
    if(transform) *transform = transform_to_use;
}


/* ----------------------------------------------------------------------------
 * Returns the center point before the user dragged the central handle.
 */
point editor::transformation_widget::get_old_center() const {
    return old_center;
}


/* ----------------------------------------------------------------------------
 * Handles the user having held the left mouse button down.
 * Returns true if the user did click on a handle.
 * mouse_coords:
 *   Mouse coordinates.
 * center:
 *   Center point.
 * size:
 *   Width and height. If NULL, no scale handling will be performed.
 * angle:
 *   Angle. If NULL, no rotation handling will be performed.
 * zoom:
 *   Zoom the widget's components by this much.
 */
bool editor::transformation_widget::handle_mouse_down(
    const point &mouse_coords, const point* const center,
    const point* const size, const float* const angle, const float zoom
) {
    if(!center) return false;
    
    point handles[9];
    float radius;
    get_locations(center, size, angle, handles, &radius, NULL);
    
    //Check if the user clicked on a translation or scale handle.
    for(unsigned char h = 0; h < 9; ++h) {
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


/* ----------------------------------------------------------------------------
 * Handles the user having moved the mouse cursor.
 * Returns true if the user is dragging a handle.
 * mouse_coords:
 *   Mouse coordinates.
 * center:
 *   Center point.
 * size:
 *   Width and height. If NULL, no scale handling will be performed.
 * angle:
 *   Angle. If NULL, no rotation handling will be performed.
 * zoom:
 *   Zoom the widget's components by this much.
 * keep_aspect_ratio:
 *   If true, aspect ratio is kept when resizing.
 * min_size:
 *   Minimum possible size for the width or height. Use -FLT_MAX for none.
 * lock_center:
 *   If true, scaling happens with the center locked. If false, the opposite
 *   edge or corner is locked instead.
 */
bool editor::transformation_widget::handle_mouse_move(
    const point &mouse_coords, point* center, point* size, float* angle,
    const float zoom, const bool keep_aspect_ratio, const float min_size,
    const bool lock_center
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


/* ----------------------------------------------------------------------------
 * Handles the user having released the left mouse button.
 * Returns true if the user stopped dragging a handle.
 */
bool editor::transformation_widget::handle_mouse_up() {
    if(moving_handle == -1) {
        return false;
    }
    
    moving_handle = -1;
    return true;
}


/* ----------------------------------------------------------------------------
 * Is the user currently moving the central handle?
 */
bool editor::transformation_widget::is_moving_center_handle() {
    return (moving_handle == 4);
}


/* ----------------------------------------------------------------------------
 * Is the user currently moving a handle?
 */
bool editor::transformation_widget::is_moving_handle() {
    return (moving_handle != -1);
}
