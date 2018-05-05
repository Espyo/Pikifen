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

#include "animation_editor.h"
#include "../LAFI/textbox.h"
#include "../functions.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Handles a key being pressed down.
 */
void animation_editor::handle_key_down(const ALLEGRO_EVENT &ev) {

}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being double-clicked.
 */
void animation_editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {

}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being pressed down.
 */
void animation_editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {
    if(!(frm_picker->flags & lafi::FLAG_INVISIBLE)) {
        return;
    }
    
    if(mode == EDITOR_MODE_SPRITE_TRANSFORM) {
        if(cur_sprite_tc.handle_mouse_down(mouse_cursor_w)) {
            cur_sprite_tc_to_gui();
        }
        
    } else if(mode == EDITOR_MODE_HITBOXES) {
        if(cur_sprite) {
            for(size_t h = 0; h < cur_sprite->hitboxes.size(); ++h) {
            
                hitbox* h_ptr = &cur_sprite->hitboxes[h];
                dist d(mouse_cursor_w, h_ptr->pos);
                if(d <= h_ptr->radius) {
                    gui_to_hitbox();
                    cur_hitbox_nr = h;
                    hitbox_to_gui();
                    
                    grabbing_hitbox = h;
                    grabbing_hitbox_edge =
                        (d > h_ptr->radius - 5 / cam_zoom);
                        
                    //If the user grabbed the outermost 5 pixels,
                    //change radius. Else move hitbox.
                    if(grabbing_hitbox_edge) {
                        float anchor_angle =
                            get_angle(mouse_cursor_w, h_ptr->pos);
                        //These variables will actually serve
                        //to store the anchor.
                        grabbing_hitbox_point.x =
                            h_ptr->pos.x +
                            cos(anchor_angle) * h_ptr->radius;
                        grabbing_hitbox_point.y =
                            h_ptr->pos.y +
                            sin(anchor_angle) * h_ptr->radius;
                    } else {
                        grabbing_hitbox_point =
                            h_ptr->pos - mouse_cursor_w;
                    }
                    
                    made_changes = true;
                }
            }
        }
        
    } else if(
        mode == EDITOR_MODE_SPRITE_BITMAP && cur_sprite &&
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
        
        point selection_tl;
        point selection_br;
        if(cur_sprite->file_size.x == 0 || cur_sprite->file_size.y == 0) {
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
        for(size_t y = 0; y < bmp_h; ++y) {
            for(size_t x = 0; x < bmp_w; ++x) {
                p = y * bmp_w + x;
                if(!selection_pixels[p]) continue;
                selection_tl.x = min(selection_tl.x, (float) x);
                selection_tl.y = min(selection_tl.y, (float) y);
                selection_br.x = max(selection_br.x, (float) x);
                selection_br.y = max(selection_br.y, (float) y);
            }
        }
        
        delete[] selection_pixels;
        
        ((lafi::textbox*) frm_sprite_bmp->widgets["txt_x"])->text =
            i2s(selection_tl.x);
        ((lafi::textbox*) frm_sprite_bmp->widgets["txt_y"])->text =
            i2s(selection_tl.y);
        ((lafi::textbox*) frm_sprite_bmp->widgets["txt_w"])->text =
            i2s(selection_br.x - selection_tl.x + 1);
        ((lafi::textbox*) frm_sprite_bmp->widgets["txt_h"])->text =
            i2s(selection_br.y - selection_tl.y + 1);
        gui_to_sprite_bmp();
        
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being dragged.
 */
void animation_editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {
    if(mode == EDITOR_MODE_TOP) {
        if(top_lmb_action == LMB_ACTION_MOVE) {
            cur_sprite->top_pos.x += (ev.mouse.dx / cam_zoom);
            cur_sprite->top_pos.y += (ev.mouse.dy / cam_zoom);
            top_to_gui();
        } else if(top_lmb_action == LMB_ACTION_RESIZE) {
            float new_w = cur_sprite->top_size.x + ev.mouse.dx / cam_zoom;
            float ratio = cur_sprite->top_size.y / cur_sprite->top_size.x;
            cur_sprite->top_size.x = new_w;
            cur_sprite->top_size.y = new_w * ratio;
            top_to_gui();
        } else if(top_lmb_action == LMB_ACTION_ROTATE) {
            cur_sprite->top_angle += ev.mouse.dx / cam_zoom;
            top_to_gui();
        }
        
    } else if(mode == EDITOR_MODE_SPRITE_TRANSFORM) {
        if(cur_sprite_tc.handle_mouse_move(mouse_cursor_w)) {
            cur_sprite_tc_to_gui();
        }
        
    } else if(mode == EDITOR_MODE_HITBOXES) {
        if(grabbing_hitbox != INVALID) {
            hitbox* h_ptr = &cur_sprite->hitboxes[grabbing_hitbox];
            
            if(grabbing_hitbox_edge) {
                h_ptr->radius =
                    dist(mouse_cursor_w, grabbing_hitbox_point).to_float() / 2;
                h_ptr->pos = (mouse_cursor_w + grabbing_hitbox_point) / 2.0;
                
            } else {
                h_ptr->pos = mouse_cursor_w + grabbing_hitbox_point;
            }
            
            hitbox_to_gui();
            made_changes = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the left mouse button being released.
 */
void animation_editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {
    if(mode == EDITOR_MODE_SPRITE_TRANSFORM) {
        cur_sprite_tc.handle_mouse_up();
    }
    grabbing_hitbox = INVALID;
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being double-clicked.
 */
void animation_editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {
    cam_pos.x = 0;
    cam_pos.y = 0;
}


/* ----------------------------------------------------------------------------
 * Handles the middle mouse button being pressed down.
 */
void animation_editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {
    zoom(1.0f);
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
    
    bool empty_status_bar = (mode == EDITOR_MODE_SPRITE_BITMAP);
    lafi::widget* widget_under_mouse = NULL;
    if(!is_mouse_in_gui(mouse_cursor_s)) {
        widget_under_mouse = NULL;
    } else {
        widget_under_mouse =
            gui->get_widget_under_mouse(mouse_cursor_s.x, mouse_cursor_s.y);
    }
    (
        (lafi::label*) gui->widgets["lbl_status_bar"]
    )->text =
        (
            empty_status_bar ? "" :
            widget_under_mouse ?
            widget_under_mouse->description :
            "(" + i2s(mouse_cursor_w.x) + "," + i2s(mouse_cursor_w.y) + ")"
        );
}


/* ----------------------------------------------------------------------------
 * Handles the mouse wheel being moved.
 */
void animation_editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {
    zoom(cam_zoom + (cam_zoom * ev.mouse.dz * 0.1));
}


/* ----------------------------------------------------------------------------
 * Handles the right mouse button being dragged.
 */
void animation_editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {
    cam_pos.x -= ev.mouse.dx / cam_zoom;
    cam_pos.y -= ev.mouse.dy / cam_zoom;
}
