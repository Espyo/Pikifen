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

#include "../../functions.h"
#include "../../game.h"
#include "../../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Handles a key being "char"-typed in the canvas exclusively.
 * ev:
 *   Event to handle.
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
        press_zoom_out_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_EQUALS)) {
        //Nope, that's not a typo. The plus key is ALLEGRO_KEY_EQUALS.
        press_zoom_in_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_0)) {
        press_zoom_and_pos_reset_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_C, true)) {
        if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
            comparison = !comparison;
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down anywhere.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_G, true)) {
        press_grid_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_H, true)) {
        press_hitboxes_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_L, true)) {
        press_load_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_P, true)) {
        press_leader_silhouette_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_Q, true)) {
        press_quit_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_R, true)) {
        press_mob_radius_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_S, true)) {
        press_save_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE)) {
    
        escape_was_pressed = true;
        
        if(!dialogs.empty()) {
            close_top_dialog();
            
        } else {
            switch(state) {
            case EDITOR_STATE_MAIN: {
                press_quit_button();
                break;
            }
            }
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {
    if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_SPACE)) {
        press_play_animation_button();
        
    } else if(key_check(ev.keyboard.keycode, ALLEGRO_KEY_HOME)) {
        press_zoom_everything_button();
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being double-clicked in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {
    if(state == EDITOR_STATE_HITBOXES || state == EDITOR_STATE_SPRITE_BITMAP) {
        handle_lmb_down(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being pressed down in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {
    if(is_cursor_in_timeline()) {
        handle_lmb_drag_in_timeline();
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        point cur_sprite_size = cur_sprite->scale * cur_sprite->file_size;
        if(
            cur_transformation_widget.handle_mouse_down(
                game.mouse_cursor_w,
                &cur_sprite->offset,
                &cur_sprite_size,
                &cur_sprite->angle,
                1.0f / game.cam.zoom
            )
        ) {
            cur_sprite->scale = cur_sprite_size / cur_sprite->file_size;
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
                            game.mouse_cursor_w,
                            &cur_hitbox->pos,
                            &hitbox_size,
                            NULL,
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
                            game.mouse_cursor_w,
                            &hitbox_center,
                            &hitbox_size,
                            NULL,
                            1.0f / game.cam.zoom
                        );
                }
            }
            
            if(!tw_handled) {
                vector<size_t> clicked_hitboxes;
                for(size_t h = 0; h < cur_sprite->hitboxes.size(); ++h) {
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
                                game.mouse_cursor_w, 1.0f / game.cam.zoom
                            )
                        ) {
                            clicked_hitboxes.push_back(h);
                        }
                    } else {
                        if(
                            dist(game.mouse_cursor_w, h_ptr->pos) <=
                            h_ptr->radius
                        ) {
                            clicked_hitboxes.push_back(h);
                        }
                    }
                }
                
                if(clicked_hitboxes.empty()) {
                    cur_hitbox = NULL;
                    cur_hitbox_nr = INVALID;
                    
                } else {
                    size_t cur_hitbox_nr_index = INVALID;
                    for(size_t i = 0; i < clicked_hitboxes.size(); ++i) {
                        if(cur_hitbox_nr == clicked_hitboxes[i]) {
                            cur_hitbox_nr_index = i;
                            break;
                        }
                    }
                    
                    if(cur_hitbox_nr_index == INVALID) {
                        cur_hitbox_nr_index = 0;
                    } else {
                        cur_hitbox_nr_index =
                            sum_and_wrap(
                                (int) cur_hitbox_nr_index, 1,
                                (int) clicked_hitboxes.size()
                            );
                    }
                    cur_hitbox_nr = clicked_hitboxes[cur_hitbox_nr_index];
                    cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_nr];
                }
            }
        }
        break;
        
    } case EDITOR_STATE_SPRITE_BITMAP: {
        if(cur_sprite && cur_sprite->parent_bmp) {
            int bmp_w = al_get_bitmap_width(cur_sprite->parent_bmp);
            int bmp_h = al_get_bitmap_height(cur_sprite->parent_bmp);
            int bmp_x = -bmp_w / 2.0;
            int bmp_y = -bmp_h / 2.0;
            point bmp_click_pos = game.mouse_cursor_w;
            bmp_click_pos.x = floor(bmp_click_pos.x - bmp_x);
            bmp_click_pos.y = floor(bmp_click_pos.y - bmp_y);
            
            if(bmp_click_pos.x < 0 || bmp_click_pos.y < 0) return;
            if(bmp_click_pos.x > bmp_w || bmp_click_pos.y > bmp_h) return;
            
            point selection_tl;
            point selection_br;
            if(
                (
                    cur_sprite->file_size.x == 0 ||
                    cur_sprite->file_size.y == 0
                ) || !sprite_bmp_add_mode
            ) {
                selection_tl = bmp_click_pos;
                selection_br = bmp_click_pos;
            } else {
                selection_tl = cur_sprite->file_pos;
                selection_br.x =
                    cur_sprite->file_pos.x + cur_sprite->file_size.x - 1;
                selection_br.y =
                    cur_sprite->file_pos.y + cur_sprite->file_size.y - 1;
            }
            
            bool* selection_pixels = new bool[bmp_w * bmp_h];
            memset(selection_pixels, 0, sizeof(bool) * bmp_w * bmp_h);
            
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
            for(size_t y = 0; y < (size_t) bmp_h; ++y) {
                for(size_t x = 0; x < (size_t) bmp_w; ++x) {
                    p = y * bmp_w + x;
                    if(!selection_pixels[p]) continue;
                    selection_tl.x = std::min(selection_tl.x, (float) x);
                    selection_tl.y = std::min(selection_tl.y, (float) y);
                    selection_br.x = std::max(selection_br.x, (float) x);
                    selection_br.y = std::max(selection_br.y, (float) y);
                }
            }
            
            delete[] selection_pixels;
            
            cur_sprite->file_pos = selection_tl;
            cur_sprite->file_size = selection_br - selection_tl + 1;
            cur_sprite->set_bitmap(
                cur_sprite->file, cur_sprite->file_pos, cur_sprite->file_size
            );
        }
        break;
        
    } case EDITOR_STATE_TOP: {
        if(cur_sprite && cur_sprite->top_visible) {
            cur_transformation_widget.handle_mouse_down(
                game.mouse_cursor_w,
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


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being dragged in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    if(is_cursor_in_timeline()) {
        handle_lmb_drag_in_timeline();
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        point cur_sprite_size = cur_sprite->scale * cur_sprite->file_size;
        if(
            cur_transformation_widget.handle_mouse_move(
                game.mouse_cursor_w,
                &cur_sprite->offset,
                &cur_sprite_size,
                &cur_sprite->angle,
                1.0f / game.cam.zoom,
                cur_sprite_keep_aspect_ratio,
                -FLT_MAX,
                is_alt_pressed
            )
        ) {
            cur_sprite->scale = cur_sprite_size / cur_sprite->file_size;
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
                        game.mouse_cursor_w,
                        &cur_hitbox->pos,
                        &hitbox_size,
                        NULL,
                        1.0f / game.cam.zoom,
                        true,
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
                        game.mouse_cursor_w,
                        &hitbox_center,
                        &hitbox_size,
                        NULL,
                        1.0f / game.cam.zoom,
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
            cur_transformation_widget.handle_mouse_move(
                game.mouse_cursor_w,
                &cur_sprite->top_pos,
                &cur_sprite->top_size,
                &cur_sprite->top_angle,
                1.0f / game.cam.zoom,
                top_keep_aspect_ratio,
                ANIM_EDITOR::TOP_MIN_SIZE,
                is_alt_pressed
            );
        }
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the mouse being clicked/dragged in the animation timeline.
 */
void animation_editor::handle_lmb_drag_in_timeline() {
    float cursor_time = get_cursor_timeline_time();
    cur_anim->get_frame_and_time(cursor_time, &cur_frame_nr, &cur_frame_time);
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being released.
 * ev:
 *   Event to handle.
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


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being double-clicked in the
 * canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_xy();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being pressed down in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_zoom();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being dragged in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the mouse coordinates being updated.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {
    game.mouse_cursor_s.x = ev.mouse.x;
    game.mouse_cursor_s.y = ev.mouse.y;
    game.mouse_cursor_w = game.mouse_cursor_s;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
    );
}


/* ----------------------------------------------------------------------------
 * Handles the mouse wheel being moved in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {
    zoom_with_cursor(game.cam.zoom + (game.cam.zoom * ev.mouse.dz * 0.1));
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being double-clicked in the
 * canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_xy();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being pressed down in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_zoom();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged in the canvas exclusively.
 * ev:
 *   Event to handle.
 */
void animation_editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}
