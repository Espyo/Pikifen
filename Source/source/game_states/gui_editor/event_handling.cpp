/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * GUI editor event handler function.
 */

#include "editor.h"

#include "../../game.h"


/* ----------------------------------------------------------------------------
 * Handles a key being "char"-typed on the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {
    if(!dialogs.empty() || is_gui_focused) return;
    
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_LEFT)) {
        game.cam.target_pos.x -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_RIGHT)) {
        game.cam.target_pos.x +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_UP)) {
        game.cam.target_pos.y -=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_DOWN)) {
        game.cam.target_pos.y +=
            AREA_EDITOR::KEYBOARD_PAN_AMOUNT / game.cam.zoom;
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_MINUS)) {
        game.cam.target_zoom =
            clamp(
                game.cam.target_zoom -
                game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
                zoom_min_level, zoom_max_level
            );
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS)) {
        //Nope, that's not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        game.cam.target_zoom =
            clamp(
                game.cam.target_zoom +
                game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
                zoom_min_level, zoom_max_level
            );
            
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_MINUS, false, true)) {
        press_grid_interval_decrease_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS, false, true)) {
        //Again, not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        press_grid_interval_increase_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        reset_cam(false);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_X)) {
        press_snap_mode_button();
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down anywhere.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {
    if(!dialogs.empty()) return;
    
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        press_load_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        press_quit_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        press_save_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE)) {
        if(!dialogs.empty()) {
            close_top_dialog();
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down on the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {
    if(!dialogs.empty() || is_gui_focused) return;
    
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_HOME)) {
        reset_cam(false);
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being double-clicked.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {
    if(!dialogs.empty() || is_mouse_in_gui) return;
    if(ImGui::GetIO().WantCaptureKeyboard) {
        //A textbox is in use. Clicking could change the state of the area,
        //so ignore it now, and let Dear ImGui close the box.
        return;
    }
    
    handle_lmb_down(ev);
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being pressed down.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {
    if(!dialogs.empty() || is_mouse_in_gui) {
        return;
    }
    if(ImGui::GetIO().WantCaptureKeyboard) {
        //A textbox is in use. Clicking could change the state of the file,
        //so ignore it now, and let Dear ImGui close the box.
        is_m1_pressed = false;
        return;
    }
    
    bool tw_handled = false;
    if(cur_item != INVALID && items[cur_item].size.x != 0.0f) {
        tw_handled =
            cur_transformation_widget.handle_mouse_down(
                game.mouse_cursor_w,
                &items[cur_item].center,
                &items[cur_item].size,
                NULL,
                1.0f / game.cam.zoom
            );
    }
    
    if(!tw_handled) {
        vector<size_t> clicked_items;
        for(size_t i = 0; i < items.size(); ++i) {
            item* item_ptr = &items[i];
            if(
                is_point_in_rectangle(
                    game.mouse_cursor_w,
                    item_ptr->center,
                    item_ptr->size
                )
            ) {
                clicked_items.push_back(i);
            }
        }
        
        if(clicked_items.empty()) {
            cur_item = INVALID;
            
        } else {
            size_t cur_item_index = INVALID;
            for(size_t i = 0; i < clicked_items.size(); ++i) {
                if(cur_item == clicked_items[i]) {
                    cur_item_index = i;
                    break;
                }
            }
            
            if(cur_item_index == INVALID) {
                cur_item_index = 0;
            } else {
                cur_item_index =
                    sum_and_wrap(
                        (int) cur_item_index, 1,
                        (int) clicked_items.size()
                    );
            }
            cur_item = clicked_items[cur_item_index];
            must_focus_on_cur_item = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being dragged.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    if(!dialogs.empty()) return;
    
    if(cur_item != INVALID && items[cur_item].size.x != 0.0f) {
        bool tw_handled =
            cur_transformation_widget.handle_mouse_move(
                snap_point(game.mouse_cursor_w),
                &items[cur_item].center,
                &items[cur_item].size,
                NULL,
                1.0f / game.cam.zoom,
                false,
                0.10f
            );
        if(tw_handled) {
            mark_new_changes();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being pressed down.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    if(!dialogs.empty() || is_mouse_in_gui) return;
    
    if(!game.options.editor_mmb_pan) {
        reset_cam(false);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being dragged.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the mouse coordinates being updated.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {
    game.mouse_cursor_s.x = ev.mouse.x;
    game.mouse_cursor_s.y = ev.mouse.y;
    game.mouse_cursor_w = game.mouse_cursor_s;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
    );
}


/* ----------------------------------------------------------------------------
 * Handles the mouse wheel being moved.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {
    if(!dialogs.empty() || is_mouse_in_gui) return;
    
    zoom_with_cursor(game.cam.zoom + (game.cam.zoom * ev.mouse.dz * 0.1));
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {
    if(!dialogs.empty() || is_mouse_in_gui) return;
    
    if(game.options.editor_mmb_pan) {
        reset_cam(false);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged.
 * ev:
 *   Event to handle.
 */
void gui_editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Pans the camera around.
 * ev:
 *   Event to handle.
 */
void gui_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/* ----------------------------------------------------------------------------
 * Resets the camera.
 * instantaneous:
 *   Whether the camera moves to its spot instantaneously or not.
 */
void gui_editor::reset_cam(const bool instantaneous) {
    center_camera(point(0.0f, 0.0f), point(100.0f, 100.0f), instantaneous);
}
