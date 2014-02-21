#ifndef ANIMATION_EDITOR_INCLUDED
#define ANIMATION_EDITOR_INCLUDED

#include <allegro5/allegro.h>

#include "animation.h"
#include "functions.h"
#include "LAFI/checkbox.h"
#include "LAFI/frame.h"
#include "LAFI/radio_button.h"
#include "LAFI/textbox.h"
#include "vars.h"

//Loads the animation's fields using the data on the animation.
void load_animation_fields() {
    lafi_widget* f = ed_gui->widgets["frm_animation"];
    lafi_widget* ff = f->widgets["frm_frame"];
    bool none = ed_cur_frame_nr == string::npos;
    ext_frame* cur_frame = none ? NULL : &ed_anim[ed_cur_frame_nr];
    
    ((lafi_textbox*) ff->widgets["txt_frame_file"])->text = none ? "" : cur_frame->file;
    ((lafi_textbox*) ff->widgets["txt_frame_fx"])->text =   none ? "" : to_string((long long) cur_frame->file_x);
    ((lafi_textbox*) ff->widgets["txt_frame_fy"])->text =   none ? "" : to_string((long long) cur_frame->file_y);
    ((lafi_textbox*) ff->widgets["txt_frame_fw"])->text =   none ? "" : to_string((long long) cur_frame->file_w);
    ((lafi_textbox*) ff->widgets["txt_frame_fh"])->text =   none ? "" : to_string((long long) cur_frame->file_h);
    ((lafi_textbox*) ff->widgets["txt_frame_gw"])->text =   none ? "" : to_string((long double) cur_frame->f.game_w);
    ((lafi_textbox*) ff->widgets["txt_frame_gh"])->text =   none ? "" : to_string((long double) cur_frame->f.game_h);
    ((lafi_textbox*) ff->widgets["txt_frame_d"])->text =    none ? "" : to_string((long double) cur_frame->f.duration);
    
    ff->flags = none ? LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE : 0;
    
    ((lafi_label*) f->widgets["lbl_frame_info"])->text =
        "Frame " +
        ((ed_cur_frame_nr == string::npos) ? "--" : to_string((long long) ed_cur_frame_nr + 1)) +
        "/" + to_string((long long) ed_anim.size());
}

//Saves the animation using the text fields on the GUI.
void save_animation() {
    if(ed_cur_frame_nr == string::npos) return;
    
    lafi_widget* f = ed_gui->widgets["frm_animation"];
    lafi_widget* ff = f->widgets["frm_frame"];
    ext_frame* cur_frame = &ed_anim[ed_cur_frame_nr];
    string new_file;
    int new_fx, new_fy, new_fw, new_fh;
    
    new_file =                   ((lafi_textbox*) ff->widgets["txt_frame_file"])->text;
    new_fx =                atoi(((lafi_textbox*) ff->widgets["txt_frame_fx"])->text.c_str());
    new_fy =                atoi(((lafi_textbox*) ff->widgets["txt_frame_fy"])->text.c_str());
    new_fw =                atoi(((lafi_textbox*) ff->widgets["txt_frame_fw"])->text.c_str());
    new_fh =                atoi(((lafi_textbox*) ff->widgets["txt_frame_fh"])->text.c_str());
    cur_frame->f.game_w =   atof(((lafi_textbox*) ff->widgets["txt_frame_gw"])->text.c_str());
    cur_frame->f.game_h =   atof(((lafi_textbox*) ff->widgets["txt_frame_gh"])->text.c_str());
    cur_frame->f.duration = atof(((lafi_textbox*) ff->widgets["txt_frame_d"])->text.c_str());
    
    if(cur_frame->file != new_file || cur_frame->file_x != new_fx || cur_frame->file_y != new_fy || cur_frame->file_w != new_fw || cur_frame->file_h != new_fh) {
        //Changed something image-wise. Recreate it.
        if(cur_frame->parent_bmp != bmp_error) al_destroy_bitmap(cur_frame->parent_bmp);
        al_destroy_bitmap(cur_frame->f.bitmap);
        cur_frame->parent_bmp = load_bmp(new_file);
        cur_frame->f.bitmap = al_create_sub_bitmap(cur_frame->parent_bmp, new_fx, new_fy, new_fw, new_fh);
        
        cur_frame->file = new_file;
        cur_frame->file_x = new_fx;
        cur_frame->file_y = new_fy;
        cur_frame->file_w = new_fw;
        cur_frame->file_h = new_fh;
    }
    
    load_animation_fields();
}

//Loads a hitbox' fields using the data on the hitbox.
void load_hitbox_fields() {
    lafi_widget* frame = ed_gui->widgets["frm_hitboxes"]->widgets["frm_hitbox"];
    bool none = ed_cur_hitbox_nr == string::npos;
    hitbox* cur_hitbox = none ? NULL : &ed_anim[ed_cur_frame_nr].f.hitboxes[ed_cur_hitbox_nr];
    
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
        
    frame->flags = none ? LAFI_FLAG_DISABLED | LAFI_FLAG_INVISIBLE : 0;
}

//Saves a hitbox' data using the fields.
void save_hitbox() {
    if(ed_cur_hitbox_nr == string::npos) return;
    lafi_widget* frame = ed_gui->widgets["frm_hitboxes"]->widgets["frm_hitbox"];
    hitbox* cur_hitbox = &ed_anim[ed_cur_frame_nr].f.hitboxes[ed_cur_hitbox_nr];
    
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
    
    load_hitbox_fields();
}

void load_animation() {
    for(size_t f = 0; f < ed_anim.size(); f++) {
        if(ed_anim[f].parent_bmp != bmp_error)
            al_destroy_bitmap(ed_anim[f].parent_bmp);
        al_destroy_bitmap(ed_anim[f].f.bitmap);
    }
    data_node file = data_node("Test.txt");
    ed_anim = load_frames(file.get_child_by_name("frames"));
    ed_cur_frame_nr = ed_anim.size() > 0 ? 0 : string::npos;
}

void do_animation_editor_logic() {
    //---Logic---
    if(ed_anim_playing && ed_cur_frame_nr != string::npos) {
        frame* cur_frame = &ed_anim[ed_cur_frame_nr].f;
        if(cur_frame->duration != 0) {
            ed_cur_frame_time += 1.0 / game_fps;
            
            while(ed_cur_frame_time > cur_frame->duration) {
                ed_cur_frame_time = ed_cur_frame_time - cur_frame->duration;
                ed_cur_frame_nr = (ed_cur_frame_nr + 1) % ed_anim.size();
                cur_frame = &ed_anim[ed_cur_frame_nr].f;
            }
        }
        load_animation_fields();
    }
    
    lafi_widget* wum = NULL; //Widget under mouse.
    wum = ed_gui->widgets["frm_bottom"]->mouse_over_widget;
    if(!wum) {
        if(ed_mode == EDITOR_MODE_NORMAL) {
            wum = ed_gui->widgets["frm_animation"]->widgets["frm_frame"]->mouse_over_widget;
            if(!wum) wum = ed_gui->widgets["frm_animation"]->mouse_over_widget;
        } else {
            wum = ed_gui->widgets["frm_hitboxes"]->widgets["frm_hitbox"]->mouse_over_widget;
            if(!wum) wum = ed_gui->widgets["frm_hitboxes"]->mouse_over_widget;
        }
    }
    
    if(wum) {
        ed_gui_status_bar->text = wum->description;
    } else {
        if(ed_mode == EDITOR_MODE_SELECT_HITBOX) ed_gui_status_bar->text = "Current mode: select hitbox mode.";
        else if(ed_mode == EDITOR_MODE_NEW_HITBOX) ed_gui_status_bar->text = "Current mode: new hitbox mode.";
        else if(ed_mode == EDITOR_MODE_DELETE_HITBOX) ed_gui_status_bar->text = "Current mode: delete hitbox mode.";
    }
    
    ed_cur_hitbox_angle += M_PI / game_fps;
    
    //---Drawing.---
    al_clear_to_color(al_map_rgb(0, 0, 64));
    
    ed_gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, cam_x + ((scr_w - 208) / 2 / cam_zoom), cam_y + (scr_h / 2 / cam_zoom));
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    if(ed_cur_frame_nr != string::npos) {
        al_set_clipping_rectangle(0, 0, scr_w - 208, scr_h - 16); {
        
            ext_frame* cur_frame = &ed_anim[ed_cur_frame_nr];
            draw_sprite(
                cur_frame->f.bitmap,
                0, 0,
                cur_frame->f.game_w, cur_frame->f.game_h
            );
            
            if(ed_hitboxes_visible && ed_cur_frame_nr != string::npos) {
                size_t n_hitboxes = cur_frame->f.hitboxes.size();
                for(size_t h = 0; h < n_hitboxes; h++) {
                    hitbox* h_ptr = &cur_frame->f.hitboxes[h];
                    
                    ALLEGRO_COLOR hitbox_color, hitbox_outline_color;
                    if(h_ptr->type == HITBOX_TYPE_NORMAL) {
                        hitbox_color = al_map_rgba(0, 128, 0, 192); hitbox_outline_color = al_map_rgba(0, 64, 0, 255);
                    } else if(h_ptr->type == HITBOX_TYPE_ATTACK) {
                        hitbox_color = al_map_rgba(128, 0, 0, 192); hitbox_outline_color = al_map_rgba(64, 0, 0, 255);
                    } else if(h_ptr->type == HITBOX_TYPE_SHAKE) {
                        hitbox_color = al_map_rgba(0, 0, 192, 192); hitbox_outline_color = al_map_rgba(0, 0, 96, 255);
                    } else {
                        hitbox_color = al_map_rgba(128, 128, 0, 192); hitbox_outline_color = al_map_rgba(64, 64, 0, 255);
                    }
                    
                    al_draw_filled_circle(
                        h_ptr->x,
                        h_ptr->y,
                        h_ptr->radius,
                        hitbox_color
                    );
                    if(h == ed_cur_hitbox_nr) {
                        al_draw_arc(
                            h_ptr->x, h_ptr->y,
                            h_ptr->radius,
                            ed_cur_hitbox_angle, M_PI * 1.8,
                            hitbox_outline_color, 2 / cam_zoom
                        );
                    } else {
                        al_draw_circle(
                            h_ptr->x,
                            h_ptr->y,
                            h_ptr->radius,
                            hitbox_outline_color, 1 / cam_zoom
                        );
                    }
                }
            }
            
            if(ed_new_hitbox_center_x != FLT_MAX) {
                al_draw_filled_circle(ed_new_hitbox_center_x, ed_new_hitbox_center_y, ed_new_hitbox_radius, al_map_rgba(0, 64, 0, 128));
            }
            
        } al_reset_clipping_rectangle();
    }
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    al_flip_display();
}

void handle_animation_editor_controls(ALLEGRO_EVENT ev) {
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        mouse_cursor_x = ev.mouse.x / cam_zoom - cam_x - ((scr_w - 208) / 2 / cam_zoom);
        mouse_cursor_y = ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(ed_holding_m2) {
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        cam_zoom += cam_zoom * ev.mouse.dz * 0.1;
        if(cam_zoom <= ZOOM_MIN_LEVEL_EDITOR) cam_zoom = ZOOM_MIN_LEVEL_EDITOR;
        if(cam_zoom >= ZOOM_MAX_LEVEL_EDITOR) cam_zoom = ZOOM_MAX_LEVEL_EDITOR;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if(ev.mouse.button == 2) ed_holding_m2 = true;
        if(ev.mouse.button == 3) {
            cam_zoom = 1;
            cam_x = cam_y = 0;
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 2) ed_holding_m2 = false;
    }
    
    
    if(ed_mode == EDITOR_MODE_NEW_HITBOX) {
    
        if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(ed_new_hitbox_center_x != FLT_MAX) {
                ed_new_hitbox_radius = dist(ed_new_hitbox_center_x, ed_new_hitbox_center_y, mouse_cursor_x, mouse_cursor_y);
            }
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
            if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) {
                ed_new_hitbox_center_x = mouse_cursor_x;
                ed_new_hitbox_center_y = mouse_cursor_y;
                ed_new_hitbox_radius = 0;
            }
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && ev.mouse.button == 1 && ed_new_hitbox_center_x != FLT_MAX) {
            if(ed_new_hitbox_radius <= 1) ed_new_hitbox_radius = 32;
            ed_anim[ed_cur_frame_nr].f.hitboxes.push_back(
                hitbox(
                    ed_new_hitbox_center_x,
                    ed_new_hitbox_center_y,
                    0, ed_new_hitbox_radius
                ));
            ed_cur_hitbox_nr = ed_anim[ed_cur_frame_nr].f.hitboxes.size() - 1;
            ed_new_hitbox_center_x = ed_new_hitbox_center_y = FLT_MAX;
            load_hitbox_fields();
        }
        
    } else {
    
        if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
            if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) {
            
                if(ed_cur_frame_nr != string::npos) {
                    frame* cur_frame = &ed_anim[ed_cur_frame_nr].f;
                    for(size_t h = 0; h < cur_frame->hitboxes.size(); ) {
                        hitbox* h_ptr = &cur_frame->hitboxes[h];
                        if(dist(mouse_cursor_x, mouse_cursor_y, h_ptr->x, h_ptr->y) <= h_ptr->radius) {
                            if(ed_mode == EDITOR_MODE_SELECT_HITBOX) {
                                save_hitbox();
                                ed_cur_hitbox_nr = h;
                                load_hitbox_fields();
                            } else if(ed_mode == EDITOR_MODE_DELETE_HITBOX) {
                                cur_frame->hitboxes.erase(ed_anim[ed_cur_frame_nr].f.hitboxes.begin() + h);
                                ed_cur_hitbox_nr = string::npos;
                                load_hitbox_fields();
                                break;
                            }
                        }
                        h++;
                    }
                }
            }
        }
        
    }
    
    ed_gui->handle_event(ev);
}

#endif //ifndef ANIMATION_EDITOR_INCLUDED