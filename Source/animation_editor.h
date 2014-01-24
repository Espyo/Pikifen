#ifndef ANIMATION_EDITOR_INCLUDED
#define ANIMATION_EDITOR_INCLUDED

#include <allegro5/allegro.h>

#include "functions.h"
#include "LAFI/checkbox.h"
#include "LAFI/frame.h"
#include "LAFI/radio_button.h"
#include "LAFI/textbox.h"
#include "vars.h"

void update_hitbox_gui() {
    lafi_widget* frame = editor_gui->widgets["frm_right"];
    
    ((lafi_frame*) frame->widgets["frm_normal_hitbox"])->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
    ((lafi_frame*) frame->widgets["frm_attack_hitbox"])->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
    ((lafi_frame*) frame->widgets["frm_shake_hitbox"])->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
    ((lafi_frame*) frame->widgets["frm_chomp_hitbox"])->flags = LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE;
    
    if(((lafi_radio_button*) frame->widgets["rad_hitbox_normal"])->selected)
        ((lafi_frame*) frame->widgets["frm_normal_hitbox"])->flags = 0;
    if(((lafi_radio_button*) frame->widgets["rad_hitbox_attack"])->selected)
        ((lafi_frame*) frame->widgets["frm_attack_hitbox"])->flags = 0;
    if(((lafi_radio_button*) frame->widgets["rad_hitbox_shake"])->selected)
        ((lafi_frame*) frame->widgets["frm_shake_hitbox"])->flags = 0;
    if(((lafi_radio_button*) frame->widgets["rad_hitbox_chomp"])->selected)
        ((lafi_frame*) frame->widgets["frm_chomp_hitbox"])->flags = 0;
        
}

//Loads a hitbox' fields using the data on the hitbox.
void load_hitbox_fields() {
    lafi_widget* frame = editor_gui->widgets["frm_right"];
    bool none = editor_cur_hitbox_nr == string::npos;
    hitbox* cur_hitbox = none ? NULL : &editor_cur_hitboxes[editor_cur_hitbox_nr];
    
    ((lafi_textbox*) frame->widgets["txt_hitbox_name"])->text = none ? "" : cur_hitbox->name;
    ((lafi_textbox*) frame->widgets["txt_hitbox_x"])->text = none ? "" : to_string((long double) cur_hitbox->x);
    ((lafi_textbox*) frame->widgets["txt_hitbox_y"])->text = none ? "" : to_string((long double) cur_hitbox->y);
    ((lafi_textbox*) frame->widgets["txt_hitbox_z"])->text = none ? "" : to_string((long double) cur_hitbox->z);
    ((lafi_textbox*) frame->widgets["txt_hitbox_r"])->text = none ? "" : to_string((long double) cur_hitbox->radius);
    
    ((lafi_radio_button*) frame->widgets["rad_hitbox_normal"])->unselect();
    ((lafi_radio_button*) frame->widgets["rad_hitbox_attack"])->unselect();
    ((lafi_radio_button*) frame->widgets["rad_hitbox_shake"])->unselect();
    ((lafi_radio_button*) frame->widgets["rad_hitbox_chomp"])->unselect();
    
    if(!none) {
        if(cur_hitbox->type == HITBOX_TYPE_NORMAL) ((lafi_radio_button*) frame->widgets["rad_hitbox_normal"])->select();
        else if(cur_hitbox->type == HITBOX_TYPE_ATTACK) ((lafi_radio_button*) frame->widgets["rad_hitbox_attack"])->select();
        else if(cur_hitbox->type == HITBOX_TYPE_SHAKE) ((lafi_radio_button*) frame->widgets["rad_hitbox_shake"])->select();
        else if(cur_hitbox->type == HITBOX_TYPE_CHOMP) ((lafi_radio_button*) frame->widgets["rad_hitbox_chomp"])->select();
    }
    
    ((lafi_textbox*) frame->widgets["frm_normal_hitbox"]->widgets["txt_hitbox_defense"])->text = none ? "" : to_string((long double) cur_hitbox->multiplier);
    bool latch = none ? false : cur_hitbox->can_pikmin_latch;
    latch ? ((lafi_checkbox*) frame->widgets["frm_normal_hitbox"]->widgets["chk_hitbox_latch"])->check() : ((lafi_checkbox*) frame->widgets["frm_normal_hitbox"]->widgets["chk_hitbox_latch"])->uncheck();
    ((lafi_textbox*) frame->widgets["frm_attack_hitbox"]->widgets["txt_hitbox_attack"])->text = none ? "" : to_string((long double) cur_hitbox->multiplier);
    ((lafi_textbox*) frame->widgets["frm_shake_hitbox"]->widgets["txt_hitbox_shake_angle"])->text = none ? "" : to_string((long double) cur_hitbox->shake_angle);
    bool swallow = none ? false : cur_hitbox->swallow;
    swallow ? ((lafi_checkbox*) frame->widgets["frm_chomp_hitbox"]->widgets["chk_hitbox_swallow"])->check() : ((lafi_checkbox*) frame->widgets["frm_chomp_hitbox"]->widgets["chk_hitbox_swallow"])->uncheck();
    
    update_hitbox_gui();
}

//Saves a hitbox' data using the fields.
void save_hitbox() {
    if(editor_cur_hitbox_nr == string::npos) return;
    lafi_widget* frame = editor_gui->widgets["frm_right"];
    hitbox* cur_hitbox = &editor_cur_hitboxes[editor_cur_hitbox_nr];
    
    cur_hitbox->name = ((lafi_textbox*) frame->widgets["txt_hitbox_name"])->text;
    cur_hitbox->x = tof(((lafi_textbox*) frame->widgets["txt_hitbox_x"])->text);
    cur_hitbox->y = tof(((lafi_textbox*) frame->widgets["txt_hitbox_y"])->text);
    cur_hitbox->z = tof(((lafi_textbox*) frame->widgets["txt_hitbox_z"])->text);
    cur_hitbox->radius = tof(((lafi_textbox*) frame->widgets["txt_hitbox_r"])->text);
    
    if(((lafi_radio_button*) frame->widgets["rad_hitbox_normal"])->selected) {
        cur_hitbox->type = HITBOX_TYPE_NORMAL;
        cur_hitbox->multiplier = tof(((lafi_textbox*) frame->widgets["frm_normal_hitbox"]->widgets["txt_hitbox_defense"])->text);
        cur_hitbox->can_pikmin_latch = ((lafi_checkbox*) frame->widgets["frm_normal_hitbox"]->widgets["chk_hitbox_latch"])->checked;
    } else if(((lafi_radio_button*) frame->widgets["rad_hitbox_attack"])->selected) {
        cur_hitbox->type = HITBOX_TYPE_ATTACK;
        cur_hitbox->multiplier = tof(((lafi_textbox*) frame->widgets["frm_attack_hitbox"]->widgets["txt_hitbox_attack"])->text);
    } else if(((lafi_radio_button*) frame->widgets["rad_hitbox_shake"])->selected) {
        cur_hitbox->type = HITBOX_TYPE_SHAKE;
        cur_hitbox->shake_angle = tof(((lafi_textbox*) frame->widgets["frm_shake_hitbox"]->widgets["txt_hitbox_shake_angle"])->text);
    } else {
        cur_hitbox->type = HITBOX_TYPE_CHOMP;
        cur_hitbox->swallow = ((lafi_checkbox*) frame->widgets["frm_chomp_hitbox"]->widgets["chk_hitbox_swallow"])->checked;
    }
    
}

void do_animation_editor_logic() {
    //---Drawing.---
    al_clear_to_color(al_map_rgb(0, 0, 64));
    
    editor_gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, cam_x + ((scr_w - 208) / 2 / cam_zoom), cam_y + (scr_h / 2 / cam_zoom));
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_set_clipping_rectangle(0, 0, scr_w - 208, scr_h - 16); {
    
        draw_sprite(
            editor_cur_bmp,
            0, 0,
            128, 128
        );
        
        for(size_t h = 0; h < editor_cur_hitboxes.size(); h++) {
            al_draw_filled_circle(
                editor_cur_hitboxes[h].x,
                editor_cur_hitboxes[h].y,
                editor_cur_hitboxes[h].radius,
                al_map_rgba(0, 128, 0, 192)
            );
            al_draw_circle(
                editor_cur_hitboxes[h].x,
                editor_cur_hitboxes[h].y,
                editor_cur_hitboxes[h].radius,
                al_map_rgba(0, 64, 0, 255), (h == editor_cur_hitbox_nr ? 3 / cam_zoom : 1 / cam_zoom)
            );
        }
        
        if(editor_new_hitbox_center_x != FLT_MAX) {
            al_draw_filled_circle(editor_new_hitbox_center_x, editor_new_hitbox_center_y, editor_new_hitbox_radius, al_map_rgba(0, 64, 0, 128));
        }
        
    } al_reset_clipping_rectangle();
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    al_flip_display();
    
    //---Logic---
    lafi_widget* mow = editor_gui->widgets["frm_right"]->mouse_over_widget;
    if(mow) {
        editor_gui_status_bar->text = mow->description;
    } else {
        if(editor_mode == EDITOR_MODE_NORMAL) editor_gui_status_bar->text = "Current mode: normal mode.";
        else if(editor_mode == EDITOR_MODE_NEW_HITBOX) editor_gui_status_bar->text = "Current mode: new hitbox mode.";
        else if(editor_mode == EDITOR_MODE_DELETE_HITBOX) editor_gui_status_bar->text = "Current mode: delete hitbox mode.";
    }
}

void handle_animation_editor_controls(ALLEGRO_EVENT ev) {
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        mouse_cursor_x = ev.mouse.x / cam_zoom - cam_x - ((scr_w - 208) / 2 / cam_zoom);
        mouse_cursor_y = ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(editor_holding_m2) {
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        cam_zoom += cam_zoom * ev.mouse.dz * 0.1;
        if(cam_zoom <= ZOOM_MIN_LEVEL_EDITOR) cam_zoom = ZOOM_MIN_LEVEL_EDITOR;
        if(cam_zoom >= ZOOM_MAX_LEVEL_EDITOR) cam_zoom = ZOOM_MAX_LEVEL_EDITOR;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if(ev.mouse.button == 2) editor_holding_m2 = true;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 2) editor_holding_m2 = false;
    }
    
    
    if(editor_mode == EDITOR_MODE_NEW_HITBOX) {
    
        if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(editor_new_hitbox_center_x != FLT_MAX) {
                editor_new_hitbox_radius = dist(editor_new_hitbox_center_x, editor_new_hitbox_center_y, mouse_cursor_x, mouse_cursor_y);
            }
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
            if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) {
                editor_new_hitbox_center_x = mouse_cursor_x;
                editor_new_hitbox_center_y = mouse_cursor_y;
                editor_new_hitbox_radius = 0;
            }
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && ev.mouse.button == 1 && editor_new_hitbox_center_x != FLT_MAX) {
            if(editor_new_hitbox_radius <= 1) editor_new_hitbox_radius = 32;
            editor_cur_hitboxes.push_back(
                hitbox(
                    editor_new_hitbox_center_x,
                    editor_new_hitbox_center_y,
                    0, editor_new_hitbox_radius
                ));
            editor_cur_hitbox_nr = editor_cur_hitboxes.size() - 1;
            editor_new_hitbox_center_x = editor_new_hitbox_center_y = FLT_MAX;
            load_hitbox_fields();
            update_hitbox_gui();
        }
        
    } else {
    
        if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
            if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) {
            
                for(size_t h = 0; h < editor_cur_hitboxes.size();) {
                    if(dist(mouse_cursor_x, mouse_cursor_y, editor_cur_hitboxes[h].x, editor_cur_hitboxes[h].y) <= editor_cur_hitboxes[h].radius) {
                        if(editor_mode == EDITOR_MODE_NORMAL) {
                            save_hitbox();
                            editor_cur_hitbox_nr = h;
                            load_hitbox_fields();
                        } else if(editor_mode == EDITOR_MODE_DELETE_HITBOX) {
                            editor_cur_hitboxes.erase(editor_cur_hitboxes.begin() + h);
                            editor_cur_hitbox_nr = string::npos;
                            load_hitbox_fields();
                            break;
                        }
                    }
                    h++;
                }
            }
        }
        
    }
    
    editor_gui->handle_event(ev);
}

#endif //ifndef ANIMATION_EDITOR_INCLUDED