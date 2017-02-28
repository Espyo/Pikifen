/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor event handler function.
 */

#include "animation_editor.h"
#include "../functions.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Handles the controls and other events.
 */
void animation_editor::handle_controls(const ALLEGRO_EVENT &ev) {

    if(fade_mgr.is_fading()) return;
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor_s.x = ev.mouse.x;
        mouse_cursor_s.y = ev.mouse.y;
        mouse_cursor_w = mouse_cursor_s;
        al_transform_coordinates(
            &screen_to_world_transform,
            &mouse_cursor_w.x, &mouse_cursor_w.y
        );
        
        lafi::widget* widget_under_mouse =
            gui->get_widget_under_mouse(mouse_cursor_s.x, mouse_cursor_s.y);
        (
            (lafi::label*) gui->widgets["lbl_status_bar"]
        )->text =
            (
                widget_under_mouse ?
                widget_under_mouse->description :
                "(" + i2s(mouse_cursor_w.x) + "," + i2s(mouse_cursor_w.y) + ")"
            );
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(holding_m2) {
            cam_pos.x -= ev.mouse.dx / cam_zoom;
            cam_pos.y -= ev.mouse.dy / cam_zoom;
        }
        
        if(!is_mouse_in_gui(mouse_cursor_s)) {
            if(ev.mouse.dz != 0) {
                //Zoom.
                cam_zoom += (cam_zoom * ev.mouse.dz * 0.1);
                cam_zoom = max(ZOOM_MIN_LEVEL_EDITOR, cam_zoom);
                cam_zoom = min(ZOOM_MAX_LEVEL_EDITOR, cam_zoom);
                
                //Keep a backup of the old mouse coordinates.
                point old_mouse_pos = mouse_cursor_w;
                
                //Figure out where the mouse will be after the zoom.
                update_transformations();
                mouse_cursor_w = mouse_cursor_s;
                al_transform_coordinates(
                    &screen_to_world_transform,
                    &mouse_cursor_w.x, &mouse_cursor_w.y
                );
                
                //Readjust the transformation by shifting the camera
                //so that the cursor ends up where it was before.
                cam_pos.x += (old_mouse_pos.x - mouse_cursor_w.x);
                cam_pos.y += (old_mouse_pos.y - mouse_cursor_w.y);
                update_transformations();
            }
        }
        
        if(holding_m1 && mode == EDITOR_MODE_SPRITE_TRANSFORM) {
            if(sprite_tra_lmb_action == LMB_ACTION_MOVE) {
                cur_sprite->offset.x += (ev.mouse.dx / cam_zoom);
                cur_sprite->offset.y += (ev.mouse.dy / cam_zoom);
                sprite_transform_to_gui();
            } else if(sprite_tra_lmb_action == LMB_ACTION_RESIZE) {
                float new_w = cur_sprite->game_size.x + ev.mouse.dx / cam_zoom;
                float ratio = cur_sprite->game_size.y / cur_sprite->game_size.x;
                cur_sprite->game_size.x = new_w;
                cur_sprite->game_size.y = new_w * ratio;
                sprite_transform_to_gui();
            }
            
        } else if(holding_m1 && mode == EDITOR_MODE_TOP) {
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
            
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        !is_mouse_in_gui(mouse_cursor_s)
    ) {
        if(ev.mouse.button == 1) holding_m1 = true;
        else if(ev.mouse.button == 2) holding_m2 = true;
        else if(ev.mouse.button == 3) {
            cam_zoom = 1;
            cam_pos.x = cam_pos.y = 0;
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 1) holding_m1 = false;
        else if(ev.mouse.button == 2) holding_m2 = false;
    }
    
    sprite* s = NULL;
    if(mode == EDITOR_MODE_ANIMATION) {
        if(cur_frame_nr != INVALID) {
            string name =
                cur_anim->frames[cur_frame_nr].sprite_name;
            size_t s_pos = anims.find_sprite(name);
            if(s_pos != INVALID) s = anims.sprites[s_pos];
        }
    } else if(mode == EDITOR_MODE_HITBOXES) {
        s = cur_sprite;
    }
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1 &&
        mode == EDITOR_MODE_HITBOXES
    ) {
        if(!is_mouse_in_gui(mouse_cursor_s)) {
            if(s) {
                for(size_t h = 0; h < s->hitboxes.size(); ++h) {
                
                    hitbox* h_ptr = &s->hitboxes[h];
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
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && ev.mouse.button == 1
    ) {
        grabbing_hitbox = INVALID;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(grabbing_hitbox != INVALID) {
            hitbox* h_ptr = &s->hitboxes[grabbing_hitbox];
            
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
    
    
    gui->handle_event(ev);
}
