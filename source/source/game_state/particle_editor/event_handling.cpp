/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle editor event handler function.
 */

#include "editor.h"

#include "../../core/game.h"


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
        zoom_and_pos_reset_cmd(1.0f);
        
    }
}


/**
 * @brief Handles a key being pressed down anywhere.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_G, true)) {
        grid_toggle_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        load_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        quit_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        save_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_SPACE, false, true)) {
        part_mgr_playback_toggle_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_SPACE)) {
        if(!gui_needs_keyboard()) {
            part_gen_playback_toggle_cmd(1.0f);
        }
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_D)) {
        clear_particles_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        leader_silhouette_toggle_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_R, true)) {
        emission_shape_toggle_cmd(1.0f);
        
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
    generator_pos_offset = game.mouse_cursor.w_pos;
}


/**
 * @brief Handles the left mouse button being dragged in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    generator_pos_offset = game.mouse_cursor.w_pos;
}


/**
 * @brief Handles the left mouse button being released.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {
    generator_pos_offset = point();
}


/**
 * @brief Handles the middle mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void particle_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        zoom_and_pos_reset_cmd(1.0f);
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
        zoom_and_pos_reset_cmd(1.0f);
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
