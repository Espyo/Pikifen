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
        mouse_cursor_x =
            ev.mouse.x / cam_zoom - cam_x -
            (gui_x / 2 / cam_zoom);
        mouse_cursor_y =
            ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
        lafi::widget* widget_under_mouse =
            gui->get_widget_under_mouse(ev.mouse.x, ev.mouse.y);
        (
            (lafi::label*) gui->widgets["lbl_status_bar"]
        )->text =
            (
                widget_under_mouse ?
                widget_under_mouse->description :
                "(" + i2s(mouse_cursor_x) + "," + i2s(mouse_cursor_y) + ")"
            );
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(holding_m2) {
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        if(!is_mouse_in_gui(ev.mouse.x, ev.mouse.y)) {
            if(ev.mouse.dz != 0) {
                //Zoom.
                float new_zoom = cam_zoom + (cam_zoom * ev.mouse.dz * 0.1);
                new_zoom = max(ZOOM_MIN_LEVEL_EDITOR, new_zoom);
                new_zoom = min(ZOOM_MAX_LEVEL_EDITOR, new_zoom);
                float new_mc_x =
                    ev.mouse.x / new_zoom - cam_x -
                    (gui_x / 2 / new_zoom);
                float new_mc_y =
                    ev.mouse.y / new_zoom - cam_y - (scr_h / 2 / new_zoom);
                    
                cam_x -= (mouse_cursor_x - new_mc_x);
                cam_y -= (mouse_cursor_y - new_mc_y);
                mouse_cursor_x = new_mc_x;
                mouse_cursor_y = new_mc_y;
                cam_zoom = new_zoom;
            }
        }
        
        if(holding_m1 && mode == EDITOR_MODE_SPRITE_OFFSET) {
            cur_sprite->offs_x += ev.mouse.dx / cam_zoom;
            cur_sprite->offs_y += ev.mouse.dy / cam_zoom;
            sprite_offset_to_gui();
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        !is_mouse_in_gui(ev.mouse.x, ev.mouse.y)
    ) {
        if(ev.mouse.button == 1) holding_m1 = true;
        else if(ev.mouse.button == 2) holding_m2 = true;
        else if(ev.mouse.button == 3) {
            cam_zoom = 1;
            cam_x = cam_y = 0;
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
        if(!is_mouse_in_gui(ev.mouse.x, ev.mouse.y)) {
            if(s) {
                for(size_t h = 0; h < s->hitboxes.size(); ++h) {
                
                    hitbox* h_ptr = &s->hitboxes[h];
                    dist d(
                        mouse_cursor_x, mouse_cursor_y, h_ptr->x, h_ptr->y
                    );
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
                                atan2(
                                    h_ptr->y - mouse_cursor_y,
                                    h_ptr->x - mouse_cursor_x
                                );
                            //These variables will actually serve
                            //to store the anchor.
                            grabbing_hitbox_x =
                                h_ptr->x + cos(anchor_angle) * h_ptr->radius;
                            grabbing_hitbox_y =
                                h_ptr->y + sin(anchor_angle) * h_ptr->radius;
                        } else {
                            grabbing_hitbox_x =
                                h_ptr->x - mouse_cursor_x;
                            grabbing_hitbox_y =
                                h_ptr->y - mouse_cursor_y;
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
                    dist(
                        mouse_cursor_x,
                        mouse_cursor_y,
                        grabbing_hitbox_x,
                        grabbing_hitbox_y
                    ).to_float() / 2;
                h_ptr->x = (mouse_cursor_x + grabbing_hitbox_x) / 2;
                h_ptr->y = (mouse_cursor_y + grabbing_hitbox_y) / 2;
                
            } else {
                h_ptr->x = mouse_cursor_x + grabbing_hitbox_x;
                h_ptr->y = mouse_cursor_y + grabbing_hitbox_y;
            }
            
            hitbox_to_gui();
            made_changes = true;
        }
        
    }
    
    
    gui->handle_event(ev);
}
