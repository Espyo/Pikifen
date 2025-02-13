/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor event handler function.
 */

#include <algorithm>

#include "editor.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"


/**
 * @brief Handles a key being "char"-typed in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {
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
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        zoom_and_pos_reset_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_C, true)) {
        if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
            comparison = !comparison;
        }
        
    }
}


/**
 * @brief Handles a key being pressed down anywhere.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_G, true)) {
        grid_toggle_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_H, true)) {
        hitboxes_toggle_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        load_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        leader_silhouette_toggle_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        quit_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_R, true)) {
        mob_radius_toggle_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        save_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE)) {
    
        escape_was_pressed = true;
        
        if(!dialogs.empty()) {
            close_top_dialog();
            
        } else {
            switch(state) {
            case EDITOR_STATE_MAIN: {
                quit_cmd(1.0f);
                break;
            }
            }
        }
        
    }
}


/**
 * @brief Handles a key being pressed down in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {
    if(
        key_check(ev.keyboard.keycode, ALLEGRO_KEY_SPACE) ||
        key_check(ev.keyboard.keycode, ALLEGRO_KEY_SPACE, false, true)
    ) {
        play_animation_cmd(1.0f);
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_HOME)) {
        zoom_everything_cmd(1.0f);
        
    }
}


/**
 * @brief Handles the left mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {
    if(state == EDITOR_STATE_HITBOXES || state == EDITOR_STATE_SPRITE_BITMAP) {
        handle_lmb_down(ev);
    }
}


/**
 * @brief Handles the left mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {
    if(is_cursor_in_timeline()) {
        handle_lmb_drag_in_timeline();
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        point cur_sprite_size = cur_sprite->scale * cur_sprite->bmp_size;
        if(
            cur_transformation_widget.handle_mouse_down(
                game.mouse_cursor.w_pos,
                &cur_sprite->offset,
                &cur_sprite_size,
                &cur_sprite->angle,
                1.0f / game.cam.zoom
            )
        ) {
            cur_sprite->scale = cur_sprite_size / cur_sprite->bmp_size;
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(cur_sprite) {
            bool tw_handled = false;
            if(cur_hitbox) {
                if(!side_view) {
                    point hitbox_size(
                        cur_hitbox->radius * 2.0f, cur_hitbox->radius * 2.0f
                    );
                    tw_handled =
                        cur_transformation_widget.handle_mouse_down(
                            game.mouse_cursor.w_pos,
                            &cur_hitbox->pos,
                            &hitbox_size,
                            nullptr,
                            1.0f / game.cam.zoom
                        );
                } else {
                    point hitbox_center(
                        cur_hitbox->pos.x,
                        (-(cur_hitbox->height / 2.0f)) - cur_hitbox->z
                    );
                    point hitbox_size(
                        cur_hitbox->radius * 2.0f, cur_hitbox->height
                    );
                    tw_handled =
                        cur_transformation_widget.handle_mouse_down(
                            game.mouse_cursor.w_pos,
                            &hitbox_center,
                            &hitbox_size,
                            nullptr,
                            1.0f / game.cam.zoom
                        );
                }
            }
            
            if(!tw_handled) {
                vector<size_t> clicked_hitboxes;
                for(size_t h = 0; h < cur_sprite->hitboxes.size(); h++) {
                    hitbox* h_ptr = &cur_sprite->hitboxes[h];
                    
                    if(side_view) {
                        point tl(h_ptr->pos.x - h_ptr->radius, 0.0f);
                        point br(h_ptr->pos.x + h_ptr->radius, 0.0f);
                        if(h_ptr->height != 0.0f) {
                            tl.y = -h_ptr->z - h_ptr->height;
                            br.y = -h_ptr->z;
                        } else {
                            tl.y = -FLT_MAX;
                            br.y = FLT_MAX;
                        }
                        if(
                            bbox_check(
                                tl, br,
                                game.mouse_cursor.w_pos, 1.0f / game.cam.zoom
                            )
                        ) {
                            clicked_hitboxes.push_back(h);
                        }
                    } else {
                        if(
                            dist(game.mouse_cursor.w_pos, h_ptr->pos) <=
                            h_ptr->radius
                        ) {
                            clicked_hitboxes.push_back(h);
                        }
                    }
                }
                
                if(clicked_hitboxes.empty()) {
                    cur_hitbox = nullptr;
                    cur_hitbox_idx = INVALID;
                    
                } else {
                    size_t cur_hitbox_idx_idx = INVALID;
                    for(size_t i = 0; i < clicked_hitboxes.size(); i++) {
                        if(cur_hitbox_idx == clicked_hitboxes[i]) {
                            cur_hitbox_idx_idx = i;
                            break;
                        }
                    }
                    
                    if(cur_hitbox_idx_idx == INVALID) {
                        cur_hitbox_idx_idx = 0;
                    } else {
                        cur_hitbox_idx_idx =
                            sum_and_wrap(
                                (int) cur_hitbox_idx_idx, 1,
                                (int) clicked_hitboxes.size()
                            );
                    }
                    cur_hitbox_idx = clicked_hitboxes[cur_hitbox_idx_idx];
                    cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_idx];
                }
            }
        }
        break;
        
    } case EDITOR_STATE_SPRITE_BITMAP: {
        if(cur_sprite && cur_sprite->parent_bmp) {
            point bmp_size = get_bitmap_dimensions(cur_sprite->parent_bmp);
            point bmp_pos = 0.0f - bmp_size / 2.0f;
            point bmp_click_pos = game.mouse_cursor.w_pos;
            bmp_click_pos.x = floor(bmp_click_pos.x - bmp_pos.x);
            bmp_click_pos.y = floor(bmp_click_pos.y - bmp_pos.y);
            
            if(bmp_click_pos.x < 0 || bmp_click_pos.y < 0) return;
            if(bmp_click_pos.x > bmp_pos.x || bmp_click_pos.y > bmp_pos.y) return;
            
            point selection_tl;
            point selection_br;
            if(
                (
                    cur_sprite->bmp_size.x == 0 ||
                    cur_sprite->bmp_size.y == 0
                ) || !sprite_bmp_add_mode
            ) {
                selection_tl = bmp_click_pos;
                selection_br = bmp_click_pos;
            } else {
                selection_tl = cur_sprite->bmp_pos;
                selection_br.x =
                    cur_sprite->bmp_pos.x + cur_sprite->bmp_size.x - 1;
                selection_br.y =
                    cur_sprite->bmp_pos.y + cur_sprite->bmp_size.y - 1;
            }
            
            bool* selection_pixels = new bool[(int) (bmp_size.x * bmp_size.y)];
            memset(selection_pixels, 0, sizeof(bool) * ((int) (bmp_size.x * bmp_size.y)));
            
            al_lock_bitmap(
                cur_sprite->parent_bmp,
                ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_READONLY
            );
            
            sprite_bmp_flood_fill(
                cur_sprite->parent_bmp, selection_pixels,
                bmp_click_pos.x, bmp_click_pos.y
            );
            
            al_unlock_bitmap(cur_sprite->parent_bmp);
            
            size_t p;
            for(size_t y = 0; y < (size_t) bmp_size.y; y++) {
                for(size_t x = 0; x < (size_t) bmp_size.x; x++) {
                    p = y * bmp_size.x + x;
                    if(!selection_pixels[p]) continue;
                    update_min_max_coords(
                        selection_tl, selection_br, point(x, y)
                    );
                }
            }
            
            delete[] selection_pixels;
            
            cur_sprite->bmp_pos = selection_tl;
            cur_sprite->bmp_size = selection_br - selection_tl + 1;
            cur_sprite->set_bitmap(
                cur_sprite->bmp_name, cur_sprite->bmp_pos, cur_sprite->bmp_size
            );
            changes_mgr.mark_as_changed();
        }
        break;
        
    } case EDITOR_STATE_TOP: {
        if(cur_sprite && cur_sprite->top_visible) {
            cur_transformation_widget.handle_mouse_down(
                game.mouse_cursor.w_pos,
                &cur_sprite->top_pos,
                &cur_sprite->top_size,
                &cur_sprite->top_angle,
                1.0f / game.cam.zoom
            );
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
void animation_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    if(is_cursor_in_timeline()) {
        handle_lmb_drag_in_timeline();
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        point cur_sprite_size = cur_sprite->scale * cur_sprite->bmp_size;
        if(
            cur_transformation_widget.handle_mouse_move(
                game.mouse_cursor.w_pos,
                &cur_sprite->offset,
                &cur_sprite_size,
                &cur_sprite->angle,
                1.0f / game.cam.zoom,
                cur_sprite_keep_aspect_ratio,
                cur_sprite_keep_area,
                -FLT_MAX,
                is_alt_pressed
            )
        ) {
            cur_sprite->scale = cur_sprite_size / cur_sprite->bmp_size;
            changes_mgr.mark_as_changed();
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(cur_sprite && cur_hitbox) {
            bool tw_handled;
            if(!side_view) {
                point hitbox_size(
                    cur_hitbox->radius * 2.0f, cur_hitbox->radius * 2.0f
                );
                tw_handled =
                    cur_transformation_widget.handle_mouse_move(
                        game.mouse_cursor.w_pos,
                        &cur_hitbox->pos,
                        &hitbox_size,
                        nullptr,
                        1.0f / game.cam.zoom,
                        true,
                        false,
                        ANIM_EDITOR::HITBOX_MIN_RADIUS * 2.0f,
                        is_alt_pressed
                    );
                cur_hitbox->radius = hitbox_size.x / 2.0f;
            } else {
                point hitbox_center(
                    cur_hitbox->pos.x,
                    (-(cur_hitbox->height / 2.0f)) - cur_hitbox->z
                );
                point hitbox_size(
                    cur_hitbox->radius * 2.0f, cur_hitbox->height
                );
                tw_handled =
                    cur_transformation_widget.handle_mouse_move(
                        game.mouse_cursor.w_pos,
                        &hitbox_center,
                        &hitbox_size,
                        nullptr,
                        1.0f / game.cam.zoom,
                        false,
                        false,
                        ANIM_EDITOR::HITBOX_MIN_RADIUS * 2.0f,
                        is_alt_pressed
                    );
                cur_hitbox->pos.x = hitbox_center.x;
                cur_hitbox->radius = hitbox_size.x / 2.0f;
                cur_hitbox->z = -(hitbox_center.y + hitbox_size.y / 2.0f);
                cur_hitbox->height = hitbox_size.y;
            }
            
            if(tw_handled) {
                changes_mgr.mark_as_changed();
            }
        }
        break;
        
    } case EDITOR_STATE_TOP: {
        if(cur_sprite && cur_sprite->top_visible) {
            if(
                cur_transformation_widget.handle_mouse_move(
                    game.mouse_cursor.w_pos,
                    &cur_sprite->top_pos,
                    &cur_sprite->top_size,
                    &cur_sprite->top_angle,
                    1.0f / game.cam.zoom,
                    top_keep_aspect_ratio,
                    false,
                    ANIM_EDITOR::TOP_MIN_SIZE,
                    is_alt_pressed
                )
            ) {
                changes_mgr.mark_as_changed();
            }
        }
        break;
        
    }
    }
}


/**
 * @brief Handles the mouse being clicked/dragged in the animation timeline.
 */
void animation_editor::handle_lmb_drag_in_timeline() {
    float cursor_time = get_cursor_timeline_time();
    size_t old_frame_idx = cur_anim_i.cur_frame_idx;
    cur_anim_i.cur_anim->get_frame_and_time(
        cursor_time, &cur_anim_i.cur_frame_idx, &cur_anim_i.cur_frame_time
    );
    if(cur_anim_i.cur_frame_idx != old_frame_idx) {
        frame* f = &cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx];
        if(f->sound_idx != INVALID) {
            play_sound(f->sound_idx);
        }
    }
}


/**
 * @brief Handles the left mouse button being released.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        cur_transformation_widget.handle_mouse_up();
        break;
        
    } case EDITOR_STATE_TOP: {
        if(cur_sprite && cur_sprite->top_visible) {
            cur_transformation_widget.handle_mouse_up();
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(cur_sprite && cur_hitbox) {
            cur_transformation_widget.handle_mouse_up();
        }
        break;
        
    }
    }
}


/**
 * @brief Handles the middle mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_xy();
    }
}


/**
 * @brief Handles the middle mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_zoom();
    }
}


/**
 * @brief Handles the middle mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/**
 * @brief Handles the mouse coordinates being updated.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {
}


/**
 * @brief Handles the mouse wheel being moved in the canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {
    zoom_with_cursor(game.cam.zoom + (game.cam.zoom * ev.mouse.dz * 0.1));
}


/**
 * @brief Handles the right mouse button being double-clicked in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_xy();
    }
}


/**
 * @brief Handles the right mouse button being pressed down in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_zoom();
    }
}


/**
 * @brief Handles the right mouse button being dragged in the
 * canvas exclusively.
 *
 * @param ev Event to handle.
 */
void animation_editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}
