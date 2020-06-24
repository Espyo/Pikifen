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
 * Handles a key being "char"-typed on the canvas exclusively.
 */
void animation_editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {
    if(is_dialog_open) {
        return;
    }
    
    switch(ev.keyboard.keycode) {
    case ALLEGRO_KEY_LEFT: {
        game.cam.target_pos.x -= KEYBOARD_PAN_AMOUNT / game.cam.zoom;
        break;
    } case ALLEGRO_KEY_RIGHT: {
        game.cam.target_pos.x += KEYBOARD_PAN_AMOUNT / game.cam.zoom;
        break;
    } case ALLEGRO_KEY_UP: {
        game.cam.target_pos.y -= KEYBOARD_PAN_AMOUNT / game.cam.zoom;
        break;
    } case ALLEGRO_KEY_DOWN: {
        game.cam.target_pos.y += KEYBOARD_PAN_AMOUNT / game.cam.zoom;
        break;
    } case ALLEGRO_KEY_MINUS: {
        game.cam.target_zoom =
            clamp(
                game.cam.target_zoom - game.cam.zoom * KEYBOARD_CAM_ZOOM,
                zoom_min_level, zoom_max_level
            );
        break;
    } case ALLEGRO_KEY_EQUALS: {
        game.cam.target_zoom =
            clamp(
                game.cam.target_zoom + game.cam.zoom * KEYBOARD_CAM_ZOOM,
                zoom_min_level, zoom_max_level
            );
        break;
    } case ALLEGRO_KEY_0: {
        if(game.cam.target_zoom == 1.0f) {
            game.cam.target_pos = point();
        } else {
            game.cam.target_zoom = 1.0f;
        }
        break;
    } case ALLEGRO_KEY_C: {
        if(state == EDITOR_STATE_SPRITE_TRANSFORM && is_ctrl_pressed) {
            comparison = !comparison;
        }
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down anywhere.
 */
void animation_editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {
    if(is_dialog_open) {
        return;
    }
    
    switch(ev.keyboard.keycode) {
    case ALLEGRO_KEY_G: {
        if(is_ctrl_pressed) {
            press_grid_button();
        }
        break;
    } case ALLEGRO_KEY_H: {
        if(is_ctrl_pressed) {
            press_hitboxes_button();
        }
        break;
    } case ALLEGRO_KEY_L: {
        if(is_ctrl_pressed) {
            press_load_button();
        }
        break;
    } case ALLEGRO_KEY_P: {
        if(is_ctrl_pressed) {
            press_pikmin_silhouette_button();
        }
        break;
    } case ALLEGRO_KEY_Q: {
        if(is_ctrl_pressed) {
            press_quit_button();
        }
        break;
    } case ALLEGRO_KEY_R: {
        if(is_ctrl_pressed) {
            press_mob_radius_button();
        }
        break;
    } case ALLEGRO_KEY_S: {
        if(is_ctrl_pressed) {
            press_save_button();
        }
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down on the canvas exclusively.
 */
void animation_editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {
    if(is_dialog_open) {
        return;
    }
    
    switch(ev.keyboard.keycode) {
    case ALLEGRO_KEY_SPACE: {
        press_play_animation_button();
        break;
    } case ALLEGRO_KEY_HOME: {
        sprite* s_ptr = cur_sprite;
        if(!s_ptr && cur_anim && cur_frame_nr != INVALID) {
            string name =
                cur_anim->frames[cur_frame_nr].sprite_name;
            size_t s_pos = anims.find_sprite(name);
            if(s_pos != INVALID) s_ptr = anims.sprites[s_pos];
        }
        if(!s_ptr || !s_ptr->bitmap) return;
        
        point cmin, cmax;
        get_transformed_rectangle_bounding_box(
            s_ptr->offset, s_ptr->file_size * s_ptr->scale,
            s_ptr->angle, &cmin, &cmax
        );
        
        if(s_ptr->top_visible) {
            point top_min, top_max;
            get_transformed_rectangle_bounding_box(
                s_ptr->top_pos, s_ptr->top_size,
                s_ptr->top_angle,
                &top_min, &top_max
            );
            cmin.x = std::min(cmin.x, top_min.x);
            cmin.y = std::min(cmin.y, top_min.y);
            cmax.x = std::max(cmax.x, top_max.x);
            cmax.y = std::max(cmax.y, top_max.y);
        }
        
        for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
            hitbox* h_ptr = &s_ptr->hitboxes[h];
            cmin.x = std::min(cmin.x, h_ptr->pos.x - h_ptr->radius);
            cmin.y = std::min(cmin.y, h_ptr->pos.y - h_ptr->radius);
            cmax.x = std::max(cmax.x, h_ptr->pos.x + h_ptr->radius);
            cmax.y = std::max(cmax.y, h_ptr->pos.y + h_ptr->radius);
        }
        
        center_camera(cmin, cmax);
        break;
    } case ALLEGRO_KEY_ESCAPE: {

        if(is_dialog_open) {
            is_dialog_open = false;
            if(dialog_close_callback) {
                dialog_close_callback();
            }
            
        } else {
            switch(state) {
            case EDITOR_STATE_MAIN: {
                press_quit_button();
                break;
            }
            }
        }
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being double-clicked.
 */
void animation_editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {
    if(state == EDITOR_STATE_HITBOXES || state == EDITOR_STATE_SPRITE_BITMAP) {
        handle_lmb_down(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being pressed down.
 */
void animation_editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {
    if(is_dialog_open) {
        return;
    }
    
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        if(cur_sprite_tc.handle_mouse_down(game.mouse_cursor_w)) {
            handle_cur_sprite_tc();
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(cur_sprite) {
            bool tc_handled = false;
            if(cur_hitbox) {
                tc_handled =
                    cur_hitbox_tc.handle_mouse_down(game.mouse_cursor_w);
            }
            
            if(tc_handled) {
                handle_cur_hitbox_tc();
            } else {
                vector<size_t> clicked_hitboxes;
                for(size_t h = 0; h < cur_sprite->hitboxes.size(); ++h) {
                    hitbox* h_ptr = &cur_sprite->hitboxes[h];
                    
                    if(side_view) {
                        if(
                            bbox_check(
                                point(
                                    h_ptr->pos.x - h_ptr->radius,
                                    -h_ptr->z - h_ptr->height
                                ),
                                point(
                                    h_ptr->pos.x + h_ptr->radius,
                                    -h_ptr->z
                                ),
                                game.mouse_cursor_w, 1 / game.cam.zoom
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
                        cur_hitbox_nr_index = clicked_hitboxes[0];
                    }
                    
                    cur_hitbox_nr_index =
                        sum_and_wrap(
                            cur_hitbox_nr_index, 1, clicked_hitboxes.size()
                        );
                    cur_hitbox_nr = clicked_hitboxes[cur_hitbox_nr_index];
                    cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_nr];
                    update_cur_hitbox_tc();
                    
                    made_new_changes = true;
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
                (cur_sprite->file_size.x == 0 || cur_sprite->file_size.y == 0) ||
                !sprite_bmp_add_mode
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
            if(top_tc.handle_mouse_down(game.mouse_cursor_w)) {
                handle_top_tc();
            }
        }
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being dragged.
 */
void animation_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        if(cur_sprite_tc.handle_mouse_move(game.mouse_cursor_w)) {
            handle_cur_sprite_tc();
            made_new_changes = true;
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(cur_sprite && cur_hitbox) {
            if(cur_hitbox_tc.handle_mouse_move(game.mouse_cursor_w)) {
                handle_cur_hitbox_tc();
                made_new_changes = true;
            }
        }
        break;
        
    } case EDITOR_STATE_TOP: {
        if(cur_sprite && cur_sprite->top_visible) {
            if(top_tc.handle_mouse_move(game.mouse_cursor_w)) {
                handle_top_tc();
            }
        }
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being released.
 */
void animation_editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {
    switch(state) {
    case EDITOR_STATE_SPRITE_TRANSFORM: {
        cur_sprite_tc.handle_mouse_up();
        break;
        
    } case EDITOR_STATE_TOP: {
        if(cur_sprite && cur_sprite->top_visible) {
            top_tc.handle_mouse_up();
        }
        break;
        
    } case EDITOR_STATE_HITBOXES: {
        if(cur_sprite && cur_hitbox) {
            cur_hitbox_tc.handle_mouse_up();
        }
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being double-clicked.
 */
void animation_editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_xy(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being pressed down.
 */
void animation_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        reset_cam_zoom(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being dragged.
 */
void animation_editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the mouse coordinates being updated.
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
 * Handles the mouse wheel being moved.
 */
void animation_editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {
    if(is_dialog_open || is_mouse_in_gui) return;
    
    zoom_with_cursor(game.cam.zoom + (game.cam.zoom * ev.mouse.dz * 0.1));
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being double-clicked.
 */
void animation_editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_xy(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged.
 */
void animation_editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {
    if(game.options.editor_mmb_pan) {
        reset_cam_zoom(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged.
 */
void animation_editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {
    if(!game.options.editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Pans the camera around.
 */
void animation_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/* ----------------------------------------------------------------------------
 * Resets the camera's X and Y coordinates.
 */
void animation_editor::reset_cam_xy(const ALLEGRO_EVENT &ev) {
    game.cam.target_pos = point();
}


/* ----------------------------------------------------------------------------
 * Resets the camera's zoom.
 */
void animation_editor::reset_cam_zoom(const ALLEGRO_EVENT &ev) {
    zoom_with_cursor(1.0f);
}
