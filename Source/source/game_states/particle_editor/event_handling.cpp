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


/**
 * @brief Handles a key being "char"-typed in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {
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
        zoom_out_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS)) {
        //Nope, that's not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        zoom_in_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_MINUS, false, true)) {
        grid_interval_decrease_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS, false, true)) {
        //Again, not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        grid_interval_increase_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        reset_cam(false);
        
    }
}


/**
 * @brief Handles a key being pressed down anywhere.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        load_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        quit_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        save_cmd(1.0f);
        
    } else if (key_check(ev.keyboard.keycode, ALLEGRO_KEY_SPACE)) {
        particle_playback_toggle_cmd(1.0f);

    } else if (key_check(ev.keyboard.keycode, ALLEGRO_KEY_D)) {
        clear_particles_cmd(1.0f);

    } else if (key_check(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        leader_silhouette_toggle_cmd(1.0f);

    } else if (key_check(ev.keyboard.keycode, ALLEGRO_KEY_R, true)) {
        emission_outline_toggle_cmd(1.0f);

    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE)) {
        escape_was_pressed = true;
        
        if(!dialogs.empty()) {
            close_top_dialog();
        }
        
    }
}


/**
 * @brief Handles a key being pressed down in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_HOME)) {
        reset_cam(false);
        
    }
}


/**
 * @brief Handles the left mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {
    handle_lmb_down(ev);
}


/**
 * @brief Handles the left mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {
    /*
    bool tw_handled = false;
    if(cur_item != INVALID && items[cur_item].size.x != 0.0f) {
        tw_handled =
            cur_transformation_widget.handle_mouse_down(
                game.mouse_cursor.w_pos,
                &items[cur_item].center,
                &items[cur_item].size,
                nullptr,
                1.0f / game.cam.zoom
            );
    }
    
    if(!tw_handled) {
        vector<size_t> clicked_items;
        for(size_t i = 0; i < items.size(); ++i) {
            item* item_ptr = &items[i];
            if(
                is_point_in_rectangle(
                    game.mouse_cursor.w_pos,
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
            size_t cur_item_idx = INVALID;
            for(size_t i = 0; i < clicked_items.size(); ++i) {
                if(cur_item == clicked_items[i]) {
                    cur_item_idx = i;
                    break;
                }
            }
            
            if(cur_item_idx == INVALID) {
                cur_item_idx = 0;
            } else {
                cur_item_idx =
                    sum_and_wrap(
                        (int) cur_item_idx, 1,
                        (int) clicked_items.size()
                    );
            }
            cur_item = clicked_items[cur_item_idx];
            must_focus_on_cur_item = true;
        }
    }
    */
}


/**
 * @brief Handles the left mouse button being dragged in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    /*
    if(cur_item != INVALID && items[cur_item].size.x != 0.0f) {
        bool tw_handled =
            cur_transformation_widget.handle_mouse_move(
                snap_point(game.mouse_cursor.w_pos),
                &items[cur_item].center,
                &items[cur_item].size,
                nullptr,
                1.0f / game.cam.zoom,
                false,
                false,
                0.10f,
                is_alt_pressed
            );
        if(tw_handled) {
            changes_mgr.mark_as_changed();
        }
    }
    */
}


/**
 * @brief Handles the left mouse button being released.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {
    cur_transformation_widget.handle_mouse_up();
}


/**
 * @brief Handles the middle mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam(false);
    }
}


/**
 * @brief Handles the middle mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/**
 * @brief Handles the mouse coordinates being updated.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {

}


/**
 * @brief Handles the mouse wheel being moved in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {
    zoom_with_cursor(game.cam.zoom + (game.cam.zoom * ev.mouse.dz * 0.1));
}


/**
 * @brief Handles the right mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam(false);
    }
}


/**
 * @brief Handles the right mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}
