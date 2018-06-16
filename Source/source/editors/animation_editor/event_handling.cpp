/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor event handler function.
 */

#include "editor.h"
#include "../../LAFI/textbox.h"
#include "../../functions.h"
#include "../../vars.h"

/* ----------------------------------------------------------------------------
 * Handles a key being "char"-typed on the canvas exclusively.
 */
void animation_editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
        cam_pos.x -= DEF_AREA_EDITOR_GRID_INTERVAL / cam_zoom;
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
        cam_pos.x += DEF_AREA_EDITOR_GRID_INTERVAL / cam_zoom;
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_UP) {
        cam_pos.y -= DEF_AREA_EDITOR_GRID_INTERVAL / cam_zoom;
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
        cam_pos.y += DEF_AREA_EDITOR_GRID_INTERVAL / cam_zoom;
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_MINUS) {
        zoom(cam_zoom - (cam_zoom * KEYBOARD_CAM_ZOOM), false);
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_EQUALS) {
        zoom(cam_zoom + (cam_zoom * KEYBOARD_CAM_ZOOM), false);
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_0) {
        if(cam_zoom == 1.0f) {
            cam_pos.x = 0.0f;
            cam_pos.y = 0.0f;
        } else {
            zoom(1.0f, false);
        }
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_C) {
        if(state == EDITOR_STATE_SPRITE_TRANSFORM && is_ctrl_pressed) {
            comparison = !comparison;
            sprite_transform_to_gui();
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down anywhere.
 */
void animation_editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(ev.keyboard.keycode == ALLEGRO_KEY_H && is_ctrl_pressed) {
        frm_toolbar->widgets["but_toggle_hitboxes"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_L && is_ctrl_pressed) {
        frm_toolbar->widgets["but_reload"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_O && is_ctrl_pressed) {
        frm_toolbar->widgets["but_toggle_origin"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_P && is_ctrl_pressed) {
        frm_toolbar->widgets["but_toggle_pik_sil"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_Q && is_ctrl_pressed) {
        frm_toolbar->widgets["but_quit"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_R && is_ctrl_pressed) {
        frm_toolbar->widgets["but_toggle_mob_radius"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_S && is_ctrl_pressed) {
        frm_toolbar->widgets["but_save"]->simulate_click();
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles a key being pressed down on the canvas exclusively.
 */
void animation_editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(ev.keyboard.keycode == ALLEGRO_KEY_SPACE) {
        frm_anim->widgets["but_play"]->simulate_click();
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_HOME) {
        if(!cur_sprite && !cur_sprite->bitmap) return;
        point cmin, cmax;
        get_transformed_rectangle_bounding_box(
            cur_sprite->offset, cur_sprite->file_size * cur_sprite->scale,
            cur_sprite->angle, &cmin, &cmax
        );
        
        if(cur_sprite->top_visible) {
            point top_min, top_max;
            get_transformed_rectangle_bounding_box(
                cur_sprite->top_pos, cur_sprite->top_size,
                cur_sprite->top_angle,
                &top_min, &top_max
            );
            cmin.x = min(cmin.x, top_min.x);
            cmin.y = min(cmin.y, top_min.y);
            cmax.x = max(cmax.x, top_max.x);
            cmax.y = max(cmax.y, top_max.y);
        }
        
        for(size_t h = 0; h < cur_sprite->hitboxes.size(); ++h) {
            hitbox* h_ptr = &cur_sprite->hitboxes[h];
            cmin.x = min(cmin.x, h_ptr->pos.x - h_ptr->radius);
            cmin.y = min(cmin.y, h_ptr->pos.y - h_ptr->radius);
            cmax.x = max(cmax.x, h_ptr->pos.x + h_ptr->radius);
            cmax.y = max(cmax.y, h_ptr->pos.y + h_ptr->radius);
        }
        
        center_camera(cmin, cmax);
        
    } else if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
        if(state == EDITOR_STATE_MAIN) {
            frm_toolbar->widgets["but_quit"]->simulate_click();
        } else if (state == EDITOR_STATE_LOAD) {
            frm_load->widgets["but_back"]->simulate_click();
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
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
        if(cur_sprite_tc.handle_mouse_down(mouse_cursor_w)) {
            cur_sprite_tc_to_gui();
        }
        
    } else if(state == EDITOR_STATE_HITBOXES) {
        if(cur_sprite) {
            bool tc_handled = false;
            if(cur_hitbox) {
                tc_handled = cur_hitbox_tc.handle_mouse_down(mouse_cursor_w);
            }
            
            if(tc_handled) {
                cur_hitbox_tc_to_gui();
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
                                mouse_cursor_w, 1 / cam_zoom
                            )
                        ) {
                            clicked_hitboxes.push_back(h);
                        }
                    } else {
                        if(dist(mouse_cursor_w, h_ptr->pos) <= h_ptr->radius) {
                            clicked_hitboxes.push_back(h);
                        }
                    }
                }
                
                if(clicked_hitboxes.empty()) {
                    cur_hitbox = NULL;
                    cur_hitbox_nr = INVALID;
                    hitbox_to_gui();
                    
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
                    hitbox_to_gui();
                    
                    made_new_changes = true;
                }
            }
        }
        
    } else if(
        state == EDITOR_STATE_SPRITE_BITMAP && cur_sprite &&
        cur_sprite->parent_bmp
    ) {
        int bmp_w = al_get_bitmap_width(cur_sprite->parent_bmp);
        int bmp_h = al_get_bitmap_height(cur_sprite->parent_bmp);
        int bmp_x = -bmp_w / 2.0;
        int bmp_y = -bmp_h / 2.0;
        point bmp_click_pos = mouse_cursor_w;
        bmp_click_pos.x = floor(bmp_click_pos.x - bmp_x);
        bmp_click_pos.y = floor(bmp_click_pos.y - bmp_y);
        
        if(bmp_click_pos.x < 0 || bmp_click_pos.y < 0) return;
        if(bmp_click_pos.x > bmp_w || bmp_click_pos.y > bmp_h) return;
        
        bool add = get_checkbox_check(frm_sprite_bmp, "chk_add");
        
        point selection_tl;
        point selection_br;
        if(
            (cur_sprite->file_size.x == 0 || cur_sprite->file_size.y == 0) ||
            !add
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
            bmp_click_pos.x, bmp_click_pos.y, bmp_w, bmp_h
        );
        
        al_unlock_bitmap(cur_sprite->parent_bmp);
        
        size_t p;
        for(size_t y = 0; y < (size_t) bmp_h; ++y) {
            for(size_t x = 0; x < (size_t) bmp_w; ++x) {
                p = y * bmp_w + x;
                if(!selection_pixels[p]) continue;
                selection_tl.x = min(selection_tl.x, (float) x);
                selection_tl.y = min(selection_tl.y, (float) y);
                selection_br.x = max(selection_br.x, (float) x);
                selection_br.y = max(selection_br.y, (float) y);
            }
        }
        
        delete[] selection_pixels;
        
        set_textbox_text(
            frm_sprite_bmp, "txt_x", i2s(selection_tl.x)
        );
        set_textbox_text(
            frm_sprite_bmp, "txt_y", i2s(selection_tl.y)
        );
        set_textbox_text(
            frm_sprite_bmp, "txt_w", i2s(selection_br.x - selection_tl.x + 1)
        );
        set_textbox_text(
            frm_sprite_bmp, "txt_h", i2s(selection_br.y - selection_tl.y + 1)
        );
        gui_to_sprite_bmp();
        
    } else if(
        state == EDITOR_STATE_TOP && cur_sprite && cur_sprite->top_visible
    ) {
        if(top_tc.handle_mouse_down(mouse_cursor_w)) {
            top_tc_to_gui();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being dragged.
 */
void animation_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
        if(cur_sprite_tc.handle_mouse_move(mouse_cursor_w)) {
            cur_sprite_tc_to_gui();
            made_new_changes = true;
        }
        
    } else if(state == EDITOR_STATE_HITBOXES) {
        if(cur_sprite && cur_hitbox) {
            if(cur_hitbox_tc.handle_mouse_move(mouse_cursor_w)) {
                cur_hitbox_tc_to_gui();
                made_new_changes = true;
            }
        }
    } else if(
        state == EDITOR_STATE_TOP && cur_sprite && cur_sprite->top_visible
    ) {
        if(top_tc.handle_mouse_move(mouse_cursor_w)) {
            top_tc_to_gui();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being released.
 */
void animation_editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {
    if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
        cur_sprite_tc.handle_mouse_up();
    } else if(
        state == EDITOR_STATE_TOP && cur_sprite && cur_sprite->top_visible
    ) {
        top_tc.handle_mouse_up();
    } else if(state == EDITOR_STATE_HITBOXES) {
        if(cur_sprite && cur_hitbox) {
            cur_hitbox_tc.handle_mouse_up();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being double-clicked.
 */
void animation_editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {
    if(!animation_editor_mmb_pan) {
        reset_cam_xy(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being pressed down.
 */
void animation_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    if(!animation_editor_mmb_pan) {
        reset_cam_zoom(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being dragged.
 */
void animation_editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {
    if(animation_editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the mouse coordinates being updated.
 */
void animation_editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {
    mouse_cursor_s.x = ev.mouse.x;
    mouse_cursor_s.y = ev.mouse.y;
    mouse_cursor_w = mouse_cursor_s;
    al_transform_coordinates(
        &screen_to_world_transform,
        &mouse_cursor_w.x, &mouse_cursor_w.y
    );
    
    update_status_bar(
        state == EDITOR_STATE_SPRITE_BITMAP,
        state == EDITOR_STATE_HITBOXES && side_view
    );
}


/* ----------------------------------------------------------------------------
 * Handles the mouse wheel being moved.
 */
void animation_editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {
    zoom(cam_zoom + (cam_zoom * ev.mouse.dz * 0.1));
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being double-clicked.
 */
void animation_editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {
    if(animation_editor_mmb_pan) {
        reset_cam_xy(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged.
 */
void animation_editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {
    if(animation_editor_mmb_pan) {
        reset_cam_zoom(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged.
 */
void animation_editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {
    if(!animation_editor_mmb_pan) {
        pan_cam(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Pans the camera around.
 */
void animation_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    cam_pos.x -= ev.mouse.dx / cam_zoom;
    cam_pos.y -= ev.mouse.dy / cam_zoom;
}


/* ----------------------------------------------------------------------------
 * Resets the camera's X and Y coordinates.
 */
void animation_editor::reset_cam_xy(const ALLEGRO_EVENT &ev) {
    cam_pos.x = 0;
    cam_pos.y = 0;
}


/* ----------------------------------------------------------------------------
 * Resets the camera's zoom.
 */
void animation_editor::reset_cam_zoom(const ALLEGRO_EVENT &ev) {
    zoom(1.0f);
}
