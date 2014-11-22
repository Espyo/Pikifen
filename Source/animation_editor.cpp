/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor-related functions.
 */

#include <algorithm>

#include <allegro5/allegro.h>

#include "animation.h"
#include "animation_editor.h"
#include "drawing.h"
#include "functions.h"
#include "LAFI/angle_picker.h"
#include "LAFI/button.h"
#include "LAFI/checkbox.h"
#include "LAFI/frame.h"
#include "LAFI/minor.h"
#include "LAFI/radio_button.h"
#include "LAFI/scrollbar.h"
#include "LAFI/textbox.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Handles the main loop of the animation editor, both the logic and drawing.
 */
void animation_editor::do_logic() {
    //---Logic---
    if(ed_anim_playing && ed_mode == EDITOR_MODE_ANIMATION && ed_cur_anim && ed_cur_frame_instance_nr != string::npos) {
        frame_instance* fi = &ed_cur_anim->frame_instances[ed_cur_frame_instance_nr];
        if(fi->duration != 0) {
            ed_cur_frame_time += delta_t;
            
            while(ed_cur_frame_time > fi->duration) {
                ed_cur_frame_time = ed_cur_frame_time - fi->duration;
                ed_cur_frame_instance_nr++;
                if(ed_cur_frame_instance_nr >= ed_cur_anim->frame_instances.size()) {
                    ed_cur_frame_instance_nr = (ed_cur_anim->loop_frame >= ed_cur_anim->frame_instances.size()) ? 0 : ed_cur_anim->loop_frame;
                }
                fi = &ed_cur_anim->frame_instances[ed_cur_frame_instance_nr];
            }
        } else {
            ed_anim_playing = false;
        }
        gui_load_animation();
    }
    
    ed_cur_hitbox_alpha += M_PI * 3 * delta_t;
    
    //---Drawing.---
    
    
    ed_gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, cam_x + ((scr_w - 208) / 2 / cam_zoom), cam_y + (scr_h / 2 / cam_zoom));
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_set_clipping_rectangle(0, 0, scr_w - 208, scr_h - 16); {
    
        al_clear_to_color(al_map_rgb(128, 144, 128));
        
        frame* f = NULL;
        
        if(ed_mode == EDITOR_MODE_ANIMATION) {
            if(ed_cur_frame_instance_nr != string::npos) {
                string name = ed_cur_anim->frame_instances[ed_cur_frame_instance_nr].frame_name;
                size_t f_pos = ed_anims.find_frame(name);
                if(f_pos != string::npos) f = ed_anims.frames[f_pos];
            }
        } else if(ed_mode == EDITOR_MODE_FRAME || ed_mode == EDITOR_MODE_TOP) {
            f = ed_cur_frame;
        }
        
        if(f) {
            if(f->bitmap) {
                draw_sprite(
                    f->bitmap,
                    f->offs_x, f->offs_y,
                    f->game_w, f->game_h
                );
            }
            
            if(ed_hitboxes_visible) {
                size_t n_hitboxes = f->hitbox_instances.size();
                for(size_t h = 0; h < n_hitboxes; h++) {
                    hitbox_instance* h_ptr = &f->hitbox_instances[h];
                    
                    ALLEGRO_COLOR hitbox_color, hitbox_outline_color;
                    unsigned char hitbox_outline_alpha = 63 + 192 * ((sin(ed_cur_hitbox_alpha) / 2) + 0.5);
                    
                    size_t h_pos = ed_anims.find_hitbox(h_ptr->hitbox_name);
                    if(h_pos == string::npos) {
                        hitbox_color = al_map_rgba(128, 128, 128, 192); hitbox_outline_color = al_map_rgba(0, 0, 0, 255);
                    } else {
                        unsigned char type = ed_anims.hitboxes[h_pos]->type;
                        if(type == HITBOX_TYPE_NORMAL) {
                            hitbox_color = al_map_rgba(0, 128, 0, 192); hitbox_outline_color = al_map_rgba(0, 64, 0, 255);
                        } else if(type == HITBOX_TYPE_ATTACK) {
                            hitbox_color = al_map_rgba(128, 0, 0, 192); hitbox_outline_color = al_map_rgba(64, 0, 0, 255);
                        } else {
                            hitbox_color = al_map_rgba(128, 128, 0, 192); hitbox_outline_color = al_map_rgba(64, 64, 0, 255);
                        }
                    }
                    
                    al_draw_filled_circle(
                        h_ptr->x,
                        h_ptr->y,
                        h_ptr->radius,
                        hitbox_color
                    );
                    
                    al_draw_circle(
                        h_ptr->x,
                        h_ptr->y,
                        h_ptr->radius,
                        (ed_cur_hitbox_instance_nr == h ? change_alpha(hitbox_outline_color, hitbox_outline_alpha) : hitbox_outline_color),
                        (ed_cur_hitbox_instance_nr == h ? 2 / cam_zoom : 1 / cam_zoom)
                    );
                }
            }
            
            if(f->top_visible && ed_mob_type_list == MOB_CATEGORY_PIKMIN) {
                draw_sprite(
                    ed_top_bmp[ed_maturity],
                    f->top_x, f->top_y,
                    f->top_w, f->top_h,
                    f->top_angle
                );
            }
        }
        
        if(ed_hitboxes_visible) {
            float cam_leftmost = -cam_x - (scr_w / 2 / cam_zoom);
            float cam_topmost = -cam_y - (scr_h / 2 / cam_zoom);
            float cam_rightmost = cam_leftmost + (scr_w / cam_zoom);
            float cam_bottommost = cam_topmost + (scr_h / cam_zoom);
            
            al_draw_line(0, cam_topmost, 0, cam_bottommost, al_map_rgb(240, 240, 240), 1 / cam_zoom);
            al_draw_line(cam_leftmost, 0, cam_rightmost, 0, al_map_rgb(240, 240, 240), 1 / cam_zoom);
        }
        
    } al_reset_clipping_rectangle();
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    al_flip_display();
}

/* ----------------------------------------------------------------------------
 * Loads the animation's data onto the gui.
 */
void animation_editor::gui_load_animation() {
    lafi::widget* f = ed_gui->widgets["frm_anims"];
    
    ((lafi::button*) f->widgets["but_anim"])->text = ed_cur_anim ? ed_cur_anim->name : "";
    
    if(!ed_cur_anim) {
        hide_widget(f->widgets["frm_anim"]);
    } else {
        show_widget(f->widgets["frm_anim"]);
        
        ((lafi::button*) f->widgets["but_anim"])->text = ed_cur_anim->name;
        ((lafi::textbox*) f->widgets["frm_anim"]->widgets["txt_loop"])->text = i2s(ed_cur_anim->loop_frame + 1);
        
        gui_load_frame_instance();
    }
}

/* ----------------------------------------------------------------------------
 * Loads the frame's data onto the gui.
 */
void animation_editor::gui_load_frame() {
    lafi::widget* f = ed_gui->widgets["frm_frames"];
    
    ((lafi::button*) f->widgets["but_frame"])->text = ed_cur_frame ? ed_cur_frame->name : "";
    
    if(!ed_cur_frame) {
        hide_widget(f->widgets["frm_frame"]);
    } else {
        show_widget(f->widgets["frm_frame"]);
        
        f = f->widgets["frm_frame"];
        
        ((lafi::textbox*) f->widgets["txt_file"])->text = ed_cur_frame->file;
        ((lafi::textbox*) f->widgets["txt_filex"])->text = i2s(ed_cur_frame->file_x);
        ((lafi::textbox*) f->widgets["txt_filey"])->text = i2s(ed_cur_frame->file_y);
        ((lafi::textbox*) f->widgets["txt_filew"])->text = i2s(ed_cur_frame->file_w);
        ((lafi::textbox*) f->widgets["txt_fileh"])->text = i2s(ed_cur_frame->file_h);
        ((lafi::textbox*) f->widgets["txt_gamew"])->text = f2s(ed_cur_frame->game_w);
        ((lafi::textbox*) f->widgets["txt_gameh"])->text = f2s(ed_cur_frame->game_h);
        ((lafi::textbox*) f->widgets["txt_offsx"])->text = f2s(ed_cur_frame->offs_x);
        ((lafi::textbox*) f->widgets["txt_offsy"])->text = f2s(ed_cur_frame->offs_y);
        
        if(ed_mob_type_list == MOB_CATEGORY_PIKMIN) enable_widget(f->widgets["but_top"])
            else disable_widget(f->widgets["but_top"]);
            
        gui_load_hitbox_instance();
    }
}

/* ----------------------------------------------------------------------------
 * Loads the frame instance's data onto the gui.
 */
void animation_editor::gui_load_frame_instance() {
    lafi::widget* f = ed_gui->widgets["frm_anims"]->widgets["frm_anim"];
    bool valid = ed_cur_frame_instance_nr != string::npos && ed_cur_anim;
    
    ((lafi::label*) f->widgets["lbl_f_nr"])->text =
        "Current frame: " +
        (valid ? i2s((ed_cur_frame_instance_nr + 1)) : "--") +
        " / " + i2s(ed_cur_anim->frame_instances.size());
        
    if(!valid) {
        hide_widget(f->widgets["frm_frame_i"]);
    } else {
        show_widget(f->widgets["frm_frame_i"]);
        
        ((lafi::button*) f->widgets["frm_frame_i"]->widgets["but_frame"])->text = ed_cur_anim->frame_instances[ed_cur_frame_instance_nr].frame_name;
        ((lafi::textbox*) f->widgets["frm_frame_i"]->widgets["txt_dur"])->text = f2s(ed_cur_anim->frame_instances[ed_cur_frame_instance_nr].duration);
    }
}

/* ----------------------------------------------------------------------------
 * Loads the hitbox' data onto the gui.
 */
void animation_editor::gui_load_hitbox() {
    lafi::widget* f = ed_gui->widgets["frm_hitboxes"];
    
    ((lafi::button*) f->widgets["but_hitbox"])->text = ed_cur_hitbox ? ed_cur_hitbox->name : "";
    
    open_hitbox_type(ed_cur_hitbox ? ed_cur_hitbox->type : 255);
    
    if(!ed_cur_hitbox) {
        hide_widget(f->widgets["frm_hitbox"]);
    } else {
        show_widget(f->widgets["frm_hitbox"]);
        
        f = f->widgets["frm_hitbox"];
        
        if(ed_cur_hitbox->type == HITBOX_TYPE_NORMAL) {
            ((lafi::textbox*) f->widgets["frm_normal"]->widgets["txt_mult"])->text = f2s(ed_cur_hitbox->multiplier);
            if(ed_cur_hitbox->can_pikmin_latch) ((lafi::checkbox*) f->widgets["frm_normal"]->widgets["chk_latch"])->check();
            else ((lafi::checkbox*) f->widgets["frm_normal"]->widgets["chk_latch"])->uncheck();
            ((lafi::textbox*) f->widgets["frm_normal"]->widgets["txt_hazards"])->text = ed_cur_hitbox->elements;
            
        } else if(ed_cur_hitbox->type == HITBOX_TYPE_ATTACK) {
            ((lafi::textbox*)      f->widgets["frm_attack"]->widgets["txt_mult"])->text = f2s(ed_cur_hitbox->multiplier);
            ((lafi::textbox*)      f->widgets["frm_attack"]->widgets["txt_hazards"])->text = ed_cur_hitbox->elements;
            ((lafi::checkbox*)     f->widgets["frm_attack"]->widgets["chk_outward"])->set(ed_cur_hitbox->knockback_outward);
            ((lafi::angle_picker*) f->widgets["frm_attack"]->widgets["ang_angle"])->set_angle_rads(ed_cur_hitbox->knockback_angle);
            ((lafi::textbox*)      f->widgets["frm_attack"]->widgets["txt_knockback"])->text = f2s(ed_cur_hitbox->knockback);
            
            if(ed_cur_hitbox->knockback_outward) {
                disable_widget(f->widgets["frm_attack"]->widgets["ang_angle"]);
            } else {
                enable_widget(f->widgets["frm_attack"]->widgets["ang_angle"]);
            }
            
        }
    }
}

/* ----------------------------------------------------------------------------
 * Loads the hitbox instance's data onto the gui.
 */
void animation_editor::gui_load_hitbox_instance() {
    lafi::widget* f = ed_gui->widgets["frm_frames"]->widgets["frm_frame"];
    bool valid = ed_cur_hitbox_instance_nr != string::npos && ed_cur_frame;
    
    ((lafi::label*) f->widgets["lbl_h_nr"])->text =
        "Current hitbox: " +
        (valid ? i2s((ed_cur_hitbox_instance_nr + 1)) : "--") +
        " / " + i2s(ed_cur_frame->hitbox_instances.size());
        
    f = f->widgets["frm_hitbox_i"];
    
    if(!valid) {
        hide_widget(f);
    } else {
        show_widget(f);
        ((lafi::button*) f->widgets["but_hitbox"])->text = ed_cur_frame->hitbox_instances[ed_cur_hitbox_instance_nr].hitbox_name;
        ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(ed_cur_frame->hitbox_instances[ed_cur_hitbox_instance_nr].x);
        ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(ed_cur_frame->hitbox_instances[ed_cur_hitbox_instance_nr].y);
        ((lafi::textbox*) f->widgets["txt_z"])->text = f2s(ed_cur_frame->hitbox_instances[ed_cur_hitbox_instance_nr].z);
        ((lafi::textbox*) f->widgets["txt_r"])->text = f2s(ed_cur_frame->hitbox_instances[ed_cur_hitbox_instance_nr].radius);
    }
}

/* ----------------------------------------------------------------------------
 * Loads the Pikmin top's data onto the gui.
 */
void animation_editor::gui_load_top() {
    lafi::widget* f = ed_gui->widgets["frm_top"];
    
    lafi::checkbox* c = (lafi::checkbox*) f->widgets["chk_visible"];
    if(ed_cur_frame->top_visible) c->check();
    else c->uncheck();
    
    ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(ed_cur_frame->top_x);
    ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(ed_cur_frame->top_y);
    ((lafi::textbox*) f->widgets["txt_w"])->text = f2s(ed_cur_frame->top_w);
    ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(ed_cur_frame->top_h);
    ((lafi::angle_picker*) f->widgets["ang_angle"])->set_angle_rads(ed_cur_frame->top_angle);
}

/* ----------------------------------------------------------------------------
 * Saves the animation's data from the gui.
 */
void animation_editor::gui_save_animation() {
    if(!ed_cur_anim) return;
    
    lafi::widget* f = ed_gui->widgets["frm_anims"]->widgets["frm_anim"];
    
    ed_cur_anim->loop_frame = s2i(((lafi::textbox*) f->widgets["txt_loop"])->text) - 1;
    if(ed_cur_anim->loop_frame >= ed_cur_anim->frame_instances.size()) ed_cur_anim->loop_frame = 0;
    
    gui_save_frame_instance();
    gui_load_animation();
}

/* ----------------------------------------------------------------------------
 * Saves the frame's data from the gui.
 */
void animation_editor::gui_save_frame() {
    if(!ed_cur_frame) return;
    
    lafi::widget* f = ed_gui->widgets["frm_frames"]->widgets["frm_frame"];
    
    string new_file;
    int new_fx, new_fy, new_fw, new_fh;
    
    new_file =                 ((lafi::textbox*) f->widgets["txt_file"])->text;
    new_fx =               s2i(((lafi::textbox*) f->widgets["txt_filex"])->text);
    new_fy =               s2i(((lafi::textbox*) f->widgets["txt_filey"])->text);
    new_fw =               s2i(((lafi::textbox*) f->widgets["txt_filew"])->text);
    new_fh =               s2i(((lafi::textbox*) f->widgets["txt_fileh"])->text);
    ed_cur_frame->game_w = s2f(((lafi::textbox*) f->widgets["txt_gamew"])->text);
    ed_cur_frame->game_h = s2f(((lafi::textbox*) f->widgets["txt_gameh"])->text);
    ed_cur_frame->offs_x = s2f(((lafi::textbox*) f->widgets["txt_offsx"])->text);
    ed_cur_frame->offs_y = s2f(((lafi::textbox*) f->widgets["txt_offsy"])->text);
    
    if(ed_cur_frame->file != new_file || ed_cur_frame->file_x != new_fx || ed_cur_frame->file_y != new_fy || ed_cur_frame->file_w != new_fw || ed_cur_frame->file_h != new_fh) {
        //Changed something image-wise. Recreate it.
        if(ed_cur_frame->parent_bmp) bitmaps.detach(ed_cur_frame->file);
        if(ed_cur_frame->bitmap) al_destroy_bitmap(ed_cur_frame->bitmap);
        ed_cur_frame->parent_bmp = bitmaps.get(new_file, NULL);
        if(ed_cur_frame->parent_bmp) ed_cur_frame->bitmap = al_create_sub_bitmap(ed_cur_frame->parent_bmp, new_fx, new_fy, new_fw, new_fh);
        
        ed_cur_frame->file = new_file;
        ed_cur_frame->file_x = new_fx;
        ed_cur_frame->file_y = new_fy;
        ed_cur_frame->file_w = new_fw;
        ed_cur_frame->file_h = new_fh;
    }
    
    gui_save_hitbox_instance();
    gui_load_frame();
}

/* ----------------------------------------------------------------------------
 * Saves the frame instance's data from the gui.
 */
void animation_editor::gui_save_frame_instance() {
    bool valid = ed_cur_frame_instance_nr != string::npos && ed_cur_anim;
    if(!valid) return;
    
    lafi::widget* f = ed_gui->widgets["frm_anims"]->widgets["frm_anim"];
    
    frame_instance* fi = &ed_cur_anim->frame_instances[ed_cur_frame_instance_nr];
    fi->duration = s2f(((lafi::textbox*) f->widgets["frm_frame_i"]->widgets["txt_dur"])->text);
    if(fi->duration < 0) fi->duration = 0;
    
    gui_load_frame_instance();
}

/* ----------------------------------------------------------------------------
 * Saves the hitbox' data from the gui.
 */
void animation_editor::gui_save_hitbox() {
    if(!ed_cur_hitbox) return;
    
    lafi::widget* f = ed_gui->widgets["frm_hitboxes"]->widgets["frm_hitbox"];
    
    if(((lafi::radio_button*) f->widgets["rad_normal"])->selected) ed_cur_hitbox->type = HITBOX_TYPE_NORMAL;
    else if(((lafi::radio_button*) f->widgets["rad_attack"])->selected) ed_cur_hitbox->type = HITBOX_TYPE_ATTACK;
    
    if(ed_cur_hitbox->type == HITBOX_TYPE_NORMAL) {
        ed_cur_hitbox->multiplier = s2f(((lafi::textbox*) f->widgets["frm_normal"]->widgets["txt_mult"])->text);
        ed_cur_hitbox->can_pikmin_latch = ((lafi::checkbox*) f->widgets["frm_normal"]->widgets["chk_latch"])->checked;
        ed_cur_hitbox->elements = ((lafi::textbox*) f->widgets["frm_normal"]->widgets["txt_hazards"])->text;
        
    } else if(ed_cur_hitbox->type == HITBOX_TYPE_ATTACK) {
        ed_cur_hitbox->multiplier = s2f(((lafi::textbox*) f->widgets["frm_attack"]->widgets["txt_mult"])->text);
        ed_cur_hitbox->elements = ((lafi::textbox*) f->widgets["frm_attack"]->widgets["txt_hazards"])->text;
        ed_cur_hitbox->knockback_outward = ((lafi::checkbox*) f->widgets["frm_attack"]->widgets["chk_outward"])->checked;
        ed_cur_hitbox->knockback_angle = ((lafi::angle_picker*) f->widgets["frm_attack"]->widgets["ang_angle"])->get_angle_rads();
        ed_cur_hitbox->knockback = s2f(((lafi::textbox*) f->widgets["frm_attack"]->widgets["txt_knockback"])->text);
        
    }
    
    gui_load_hitbox();
}

/* ----------------------------------------------------------------------------
 * Saves the hitbox instance's data from the gui.
 */
void animation_editor::gui_save_hitbox_instance() {
    bool valid = ed_cur_hitbox_instance_nr != string::npos && ed_cur_frame;
    if(!valid) return;
    
    lafi::widget* f = ed_gui->widgets["frm_frames"]->widgets["frm_frame"]->widgets["frm_hitbox_i"];
    
    hitbox_instance* hi = &ed_cur_frame->hitbox_instances[ed_cur_hitbox_instance_nr];
    
    hi->x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    hi->y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    hi->z = s2f(((lafi::textbox*) f->widgets["txt_z"])->text);
    hi->radius = s2f(((lafi::textbox*) f->widgets["txt_r"])->text);
    if(hi->radius <= 0) hi->radius = 16;
    
    gui_load_hitbox_instance();
}

/* ----------------------------------------------------------------------------
 * Saves the Pikmin top's data from the gui.
 */
void animation_editor::gui_save_top() {
    lafi::widget* f = ed_gui->widgets["frm_top"];
    
    ed_cur_frame->top_visible = ((lafi::checkbox*) f->widgets["chk_visible"])->checked;
    ed_cur_frame->top_x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    ed_cur_frame->top_y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    ed_cur_frame->top_w = s2f(((lafi::textbox*) f->widgets["txt_w"])->text);
    ed_cur_frame->top_h = s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    ed_cur_frame->top_angle = ((lafi::angle_picker*) f->widgets["ang_angle"])->get_angle_rads();
    
    gui_load_top();
}

/* ----------------------------------------------------------------------------
 * Handles the controls and other events.
 */
void animation_editor::handle_controls(ALLEGRO_EVENT ev) {
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor_x = ev.mouse.x / cam_zoom - cam_x - ((scr_w - 208) / 2 / cam_zoom);
        mouse_cursor_y = ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
        lafi::widget* wum = ed_gui->get_widget_under_mouse(ev.mouse.x, ev.mouse.y); //Widget under mouse.
        ((lafi::label*) ed_gui->widgets["lbl_status_bar"])->text = (wum ? wum->description : "(" + i2s(mouse_cursor_x) + "," + i2s(mouse_cursor_y) + ")");
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(ed_holding_m2) {
            cam_x += ev.mouse.dx / cam_zoom;
            cam_y += ev.mouse.dy / cam_zoom;
        }
        
        if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) {
            if(ev.mouse.dz != 0) {
                //Zoom.
                float new_zoom = cam_zoom + (cam_zoom * ev.mouse.dz * 0.1);
                new_zoom = max(ZOOM_MIN_LEVEL_EDITOR, new_zoom);
                new_zoom = min(ZOOM_MAX_LEVEL_EDITOR, new_zoom);
                float new_mc_x = ev.mouse.x / new_zoom - cam_x - ((scr_w - 208) / 2 / new_zoom);
                float new_mc_y = ev.mouse.y / new_zoom - cam_y - (scr_h / 2 / new_zoom);
                
                cam_x -= (mouse_cursor_x - new_mc_x);
                cam_y -= (mouse_cursor_y - new_mc_y);
                mouse_cursor_x = new_mc_x;
                mouse_cursor_y = new_mc_y;
                cam_zoom = new_zoom;
            }
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if(ev.mouse.button == 2) ed_holding_m2 = true;
        if(ev.mouse.button == 3) {
            cam_zoom = 1;
            cam_x = cam_y = 0;
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 2) ed_holding_m2 = false;
    }
    
    frame* f = NULL;
    if(ed_mode == EDITOR_MODE_ANIMATION) {
        if(ed_cur_frame_instance_nr != string::npos) {
            string name = ed_cur_anim->frame_instances[ed_cur_frame_instance_nr].frame_name;
            size_t f_pos = ed_anims.find_frame(name);
            if(f_pos != string::npos) f = ed_anims.frames[f_pos];
        }
    } else if(ed_mode == EDITOR_MODE_FRAME) {
        f = ed_cur_frame;
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1 && ed_mode == EDITOR_MODE_FRAME) {
        if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) {
            if(f) {
                for(size_t h = 0; h < f->hitbox_instances.size(); h++) {
                
                    hitbox_instance* hi_ptr = &f->hitbox_instances[h];
                    float d = dist(mouse_cursor_x, mouse_cursor_y, hi_ptr->x, hi_ptr->y);
                    if(d <= hi_ptr->radius) {
                        gui_save_hitbox_instance();
                        ed_cur_hitbox_instance_nr = h;
                        gui_load_hitbox_instance();
                        
                        ed_grabbing_hitbox = h;
                        ed_grabbing_hitbox_edge = (d > hi_ptr->radius - 5 / cam_zoom);
                        
                        //If the user grabbed the outermost 5 pixels, change radius. Else move hitbox.
                        if(ed_grabbing_hitbox_edge) {
                            float anchor_angle = atan2(hi_ptr->y - mouse_cursor_y, hi_ptr->x - mouse_cursor_x);
                            //These variables will actually serve to store the anchor.
                            ed_grabbing_hitbox_x = hi_ptr->x + cos(anchor_angle) * hi_ptr->radius;
                            ed_grabbing_hitbox_y = hi_ptr->y + sin(anchor_angle) * hi_ptr->radius;
                        } else {
                            ed_grabbing_hitbox_x = hi_ptr->x - mouse_cursor_x;
                            ed_grabbing_hitbox_y = hi_ptr->y - mouse_cursor_y;
                        }
                    }
                }
            }
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && ev.mouse.button == 1) {
        ed_grabbing_hitbox = string::npos;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(ed_grabbing_hitbox != string::npos) {
            hitbox_instance* hi_ptr = &f->hitbox_instances[ed_grabbing_hitbox];
            
            if(ed_grabbing_hitbox_edge) {
                hi_ptr->radius =
                    dist(
                        mouse_cursor_x,
                        mouse_cursor_y,
                        ed_grabbing_hitbox_x,
                        ed_grabbing_hitbox_y
                    ) / 2;
                hi_ptr->x = (mouse_cursor_x + ed_grabbing_hitbox_x) / 2;
                hi_ptr->y = (mouse_cursor_y + ed_grabbing_hitbox_y) / 2;
                
            } else {
                hi_ptr->x = mouse_cursor_x + ed_grabbing_hitbox_x;
                hi_ptr->y = mouse_cursor_y + ed_grabbing_hitbox_y;
            }
            
            gui_load_hitbox_instance();
        }
    }
    
    
    ed_gui->handle_event(ev);
}

/* ----------------------------------------------------------------------------
 * Loads the animation editor.
 */
void animation_editor::load() {
    ed_mode = EDITOR_MODE_MAIN;
    
    lafi::style* s = new lafi::style(al_map_rgb(192, 192, 208), al_map_rgb(0, 0, 32), al_map_rgb(96, 128, 160));
    ed_gui = new lafi::gui(scr_w, scr_h, s);
    
    
    //Main frame.
    lafi::frame* frm_main = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    ed_gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add("lbl_category", new lafi::label(0, 0, 0, 0, "Category:"), 100, 16);
    frm_main->easy_row();
    frm_main->easy_add("but_category", new lafi::button(0, 0, 0, 0), 100, 32);
    frm_main->easy_row();
    frm_main->easy_add("lbl_object", new lafi::label(0, 0, 0, 0, "Object:"), 100, 16);
    frm_main->easy_row();
    frm_main->easy_add("but_object", new lafi::button(0, 0, 0, 0), 100, 32);
    int y = frm_main->easy_row();
    
    lafi::frame* frm_object = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_main->add("frm_object", frm_object);
    frm_object->easy_row();
    frm_object->easy_add("but_anims", new lafi::button(0, 0, 0, 0, "Edit animations"), 100, 32);
    frm_object->easy_row();
    frm_object->easy_add("but_frames", new lafi::button(0, 0, 0, 0, "Edit frames"), 100, 32);
    frm_object->easy_row();
    frm_object->easy_add("but_hitboxes", new lafi::button(0, 0, 0, 0, "Edit hitboxes"), 100, 32);
    frm_object->easy_row();
    frm_object->easy_add("lbl_n_anims", new lafi::label(0, 0, 0, 0), 100, 12);
    frm_object->easy_row();
    frm_object->easy_add("lbl_n_frames", new lafi::label(0, 0, 0, 0), 100, 12);
    frm_object->easy_row();
    frm_object->easy_add("lbl_n_hitboxes", new lafi::label(0, 0, 0, 0), 100, 12);
    frm_object->easy_row();
    
    
    //Animations frame.
    lafi::frame* frm_anims = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_anims);
    ed_gui->add("frm_anims", frm_anims);
    
    frm_anims->easy_row();
    frm_anims->easy_add("but_back",     new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_anims->easy_row();
    frm_anims->easy_add("lbl_anim",     new lafi::label( 0, 0, 0, 0, "Animation:"), 85, 16);
    frm_anims->easy_add("but_del_anim", new lafi::button(0, 0, 0, 0, "-"), 15, 16);
    frm_anims->easy_row();
    frm_anims->easy_add("but_anim",     new lafi::button(0, 0, 0, 0), 100, 32);
    y = frm_anims->easy_row();
    
    lafi::frame* frm_anim = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_anims->add("frm_anim", frm_anim);
    frm_anim->easy_row();
    frm_anim->easy_add("lin_1",    new lafi::line(  0, 0, 0, 0), 20, 12);
    frm_anim->easy_add("lbl_data", new lafi::label( 0, 0, 0, 0, "Animation data", ALLEGRO_ALIGN_CENTER), 60, 12);
    frm_anim->easy_add("lin_2",    new lafi::line(  0, 0, 0, 0), 20, 12);
    frm_anim->easy_row();
    frm_anim->easy_add("lbl_loop", new lafi::label( 0, 0, 0, 0, "Loop frame:"), 50, 16);
    frm_anim->easy_add("txt_loop", new lafi::textbox( 0, 0, 0, 0), 50, 16);
    frm_anim->easy_row();
    frm_anim->easy_add("lin_3",    new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_anim->easy_add("lbl_list", new lafi::label( 0, 0, 0, 0, "Frame list", ALLEGRO_ALIGN_CENTER), 50, 12);
    frm_anim->easy_add("lin_4",    new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_anim->easy_row();
    frm_anim->easy_add("lbl_f_nr", new lafi::label( 0, 0, 0, 0), 100, 16);
    frm_anim->easy_row();
    frm_anim->easy_add("but_play", new lafi::button(0, 0, 0, 0, "P"), 20, 32);
    frm_anim->easy_add("but_prev", new lafi::button(0, 0, 0, 0, "<"), 20, 32);
    frm_anim->easy_add("but_next", new lafi::button(0, 0, 0, 0, ">"), 20, 32);
    frm_anim->easy_add("but_add",  new lafi::button(0, 0, 0, 0, "+"), 20, 32);
    frm_anim->easy_add("but_rem",  new lafi::button(0, 0, 0, 0, "-"), 20, 32);
    y += frm_anim->easy_row();
    
    lafi::frame* frm_frame_i = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_anim->add("frm_frame_i", frm_frame_i);
    frm_frame_i->easy_row();
    frm_frame_i->easy_add("lbl_frame", new lafi::label(  0, 0, 0, 0, "Frame:"), 30, 16);
    frm_frame_i->easy_add("but_frame", new lafi::button(0, 0, 0, 0), 70, 24);
    frm_frame_i->easy_row();
    frm_frame_i->easy_add("lbl_dur",   new lafi::label(  0, 0, 0, 0, "Duration:"), 40, 16);
    frm_frame_i->easy_add("txt_dur",   new lafi::textbox(0, 0, 0, 0), 60, 16);
    frm_frame_i->easy_row();
    
    
    //Frames frame.
    lafi::frame* frm_frames = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_frames);
    ed_gui->add("frm_frames", frm_frames);
    
    frm_frames->easy_row();
    frm_frames->easy_add("but_back",      new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_frames->easy_row();
    frm_frames->easy_add("lbl_frame",     new lafi::label( 0, 0, 0, 0, "Frame:"), 85, 16);
    frm_frames->easy_add("but_del_frame", new lafi::button(0, 0, 0, 0, "-"), 15, 16);
    frm_frames->easy_row();
    frm_frames->easy_add("but_frame",     new lafi::button(0, 0, 0, 0), 100, 32);
    y = frm_frames->easy_row();
    
    lafi::frame* frm_frame = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_frames->add("frm_frame", frm_frame);
    frm_frame->easy_row();
    frm_frame->easy_add("lin_1",      new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_frame->easy_add("lbl_f_data", new lafi::label(0, 0, 0, 0, "Frame data", ALLEGRO_ALIGN_CENTER), 50, 12);
    frm_frame->easy_add("lin_2",      new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_file",   new lafi::label(0, 0, 0, 0, "File:"), 25, 16);
    frm_frame->easy_add("txt_file",   new lafi::textbox(0, 0, 0, 0), 75, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_filexy", new lafi::label(0, 0, 0, 0, "File X&Y:"), 45, 16);
    frm_frame->easy_add("txt_filex",  new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_add("txt_filey",  new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_filewh", new lafi::label(0, 0, 0, 0, "File W&H:"), 45, 16);
    frm_frame->easy_add("txt_filew",  new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_add("txt_fileh",  new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_gamewh", new lafi::label(0, 0, 0, 0, "Game W&H:"), 45, 16);
    frm_frame->easy_add("txt_gamew",  new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_add("txt_gameh",  new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_offsxy", new lafi::label(0, 0, 0, 0, "Offset X&Y:"), 45, 16);
    frm_frame->easy_add("txt_offsx",  new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_add("txt_offsy",  new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("but_top",    new lafi::button(0, 0, 0, 0, "Edit Pikmin top"), 100, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("lin_3",      new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_frame->easy_add("lbl_list",   new lafi::label( 0, 0, 0, 0, "Hitbox list", ALLEGRO_ALIGN_CENTER), 50, 12);
    frm_frame->easy_add("lin_4",      new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_h_nr",   new lafi::label( 0, 0, 0, 0), 100, 12);
    frm_frame->easy_row();
    frm_frame->easy_add("but_prev",   new lafi::button(0, 0, 0, 0, "<"), 20, 24);
    frm_frame->easy_add("but_next",   new lafi::button(0, 0, 0, 0, ">"), 20, 24);
    frm_frame->easy_add("but_add",    new lafi::button(0, 0, 0, 0, "+"), 20, 24);
    frm_frame->easy_add("but_rem",    new lafi::button(0, 0, 0, 0, "-"), 20, 24);
    y += frm_frame->easy_row();
    
    lafi::frame* frm_hitbox_i = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_frame->add("frm_hitbox_i", frm_hitbox_i);
    frm_hitbox_i->easy_row();
    frm_hitbox_i->easy_add("lbl_hitbox", new lafi::label(0, 0, 0, 0, "Hitbox:"), 30, 16);
    frm_hitbox_i->easy_add("but_hitbox", new lafi::button(0, 0, 0, 0), 70, 24);
    frm_hitbox_i->easy_row();
    frm_hitbox_i->easy_add("lbl_xy",     new lafi::label(0, 0, 0, 0, "X, Y:"), 45, 16);
    frm_hitbox_i->easy_add("txt_x",      new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_hitbox_i->easy_add("txt_y",      new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_hitbox_i->easy_row();
    frm_hitbox_i->easy_add("lbl_zr",     new lafi::label(0, 0, 0, 0, "Z, Radius:"), 45, 16);
    frm_hitbox_i->easy_add("txt_z",      new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_hitbox_i->easy_add("txt_r",      new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_hitbox_i->easy_row();
    
    
    //Hitboxes frame.
    lafi::frame* frm_hitboxes = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_hitboxes);
    ed_gui->add("frm_hitboxes", frm_hitboxes);
    
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("but_back",   new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("lbl_hitbox", new lafi::label( 0, 0, 0, 0, "Hitbox:"), 85, 16);
    frm_hitboxes->easy_add("but_del_h",  new lafi::button(0, 0, 0, 0, "-"), 15, 16);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("but_hitbox", new lafi::button(0, 0, 0, 0), 100, 32);
    y = frm_hitboxes->easy_row();
    
    lafi::frame* frm_hitbox = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_hitboxes->add("frm_hitbox", frm_hitbox);
    frm_hitbox->easy_row();
    frm_hitbox->easy_add("lin_1",      new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_hitbox->easy_add("lbl_h_data", new lafi::label(0, 0, 0, 0, "Hitbox data", ALLEGRO_ALIGN_CENTER), 50, 12);
    frm_hitbox->easy_add("lin_2",      new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_hitbox->easy_row();
    frm_hitbox->easy_add("lbl_h_type",   new lafi::label(0, 0, 0, 0, "Hitbox type:"), 100, 12);
    frm_hitbox->easy_row();
    frm_hitbox->easy_add("rad_normal",   new lafi::radio_button(0, 0, 0, 0, "Normal"), 50, 16);
    frm_hitbox->easy_add("rad_attack",   new lafi::radio_button(0, 0, 0, 0, "Attack"), 50, 16);
    y += frm_hitbox->easy_row();
    
    lafi::frame* frm_normal = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    hide_widget(frm_normal);
    frm_hitbox->add("frm_normal", frm_normal);
    
    frm_normal->easy_row();
    frm_normal->easy_add("lbl_mult",    new lafi::label(0, 0, 0, 0, "Defense mult.:"), 60, 16);
    frm_normal->easy_add("txt_mult",    new lafi::textbox(0, 0, 0, 0), 40, 16);
    frm_normal->easy_row();
    frm_normal->easy_add("chk_latch",   new lafi::checkbox(0, 0, 0, 0, "Pikmin can latch"), 100, 16);
    frm_normal->easy_row();
    frm_normal->easy_add("lbl_hazards", new lafi::label(0, 0, 0, 0, "Hazards:"), 100, 12);
    frm_normal->easy_row();
    frm_normal->easy_add("txt_hazards", new lafi::textbox(0, 0, 0, 0), 100, 16);
    frm_normal->easy_row();
    
    lafi::frame* frm_attack = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    hide_widget(frm_attack);
    frm_hitbox->add("frm_attack", frm_attack);
    
    frm_attack->easy_row();
    frm_attack->easy_add("lbl_mult",    new lafi::label(0, 0, 0, 0, "Attack mult.:"), 60, 16);
    frm_attack->easy_add("txt_mult",    new lafi::textbox(0, 0, 0, 0), 40, 16);
    frm_attack->easy_row();
    frm_attack->easy_add("lbl_hazards", new lafi::label(0, 0, 0, 0, "Hazards:"), 100, 12);
    frm_attack->easy_row();
    frm_attack->easy_add("txt_hazards", new lafi::textbox(0, 0, 0, 0), 100, 16);
    frm_attack->easy_row();
    frm_attack->easy_add("chk_outward", new lafi::checkbox(0, 0, 0, 0, "Outward knockback"), 100, 16);
    frm_attack->easy_row();
    frm_attack->easy_add("lbl_angle", new lafi::label(0, 0, 0, 0, "Angle:"), 60, 16);
    frm_attack->easy_add("ang_angle", new lafi::angle_picker(0, 0, 0, 0), 40, 24);
    frm_attack->easy_row();
    frm_attack->easy_add("lbl_knockback", new lafi::label(0, 0, 0, 0, "Knockback:"), 60, 16);
    frm_attack->easy_add("txt_knockback", new lafi::textbox(0, 0, 0, 0), 40, 16);
    frm_attack->easy_row();
    
    
    //Picker frame.
    lafi::frame* frm_picker = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_picker);
    ed_gui->add("frm_picker", frm_picker);
    
    frm_picker->add("but_back", new lafi::button(     scr_w - 200, 8, scr_w - 104, 24, "Back"));
    frm_picker->add("txt_new", new lafi::textbox(     scr_w - 200, 40, scr_w - 48, 56));
    frm_picker->add("but_new", new lafi::button(      scr_w - 40,  32, scr_w - 8,  64, "+"));
    frm_picker->add("frm_list", new lafi::frame(      scr_w - 200, 72, scr_w - 32, scr_h - 56));
    frm_picker->add("bar_scroll", new lafi::scrollbar(scr_w - 24,  72, scr_w - 8,  scr_h - 56));
    
    
    //Pikmin top frame.
    lafi::frame* frm_top = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_top);
    ed_gui->add("frm_top", frm_top);
    
    frm_top->easy_row();
    frm_top->easy_add("but_back", new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_top->easy_row();
    frm_top->easy_add("chk_visible", new lafi::checkbox(0, 0, 0, 0, "Visible"), 100, 16);
    frm_top->easy_row();
    frm_top->easy_add("lbl_xy", new lafi::label(0, 0, 0, 0, "X&Y:"), 20, 16);
    frm_top->easy_add("txt_x", new lafi::textbox(0, 0, 0, 0), 40, 16);
    frm_top->easy_add("txt_y", new lafi::textbox(0, 0, 0, 0), 40, 16);
    frm_top->easy_row();
    frm_top->easy_add("lbl_wh", new lafi::label(0, 0, 0, 0, "W&H:"), 20, 16);
    frm_top->easy_add("txt_w", new lafi::textbox(0, 0, 0, 0), 40, 16);
    frm_top->easy_add("txt_h", new lafi::textbox(0, 0, 0, 0), 40, 16);
    frm_top->easy_row();
    frm_top->easy_add("lbl_angle", new lafi::label(0, 0, 0, 0, "Angle:"), 40, 16);
    frm_top->easy_add("ang_angle", new lafi::angle_picker(0, 0, 0, 0), 60, 24);
    frm_top->easy_row();
    frm_top->easy_add("but_maturity", new lafi::button(0, 0, 0, 0, "Change maturity"), 100, 24);
    frm_top->easy_row();
    
    
    //Bottom bar.
    lafi::frame* frm_bottom = new lafi::frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    ed_gui->add("frm_bottom", frm_bottom);
    frm_bottom->easy_row();
    frm_bottom->easy_add("but_toggle_hitboxes", new lafi::button(0, 0, 0, 0, "Hit"), 25, 32);
    frm_bottom->easy_add("but_load", new lafi::button(           0, 0, 0, 0, "Load"), 25, 32);
    frm_bottom->easy_add("but_save", new lafi::button(           0, 0, 0, 0, "Save"), 25, 32);
    frm_bottom->easy_add("but_quit", new lafi::button(           0, 0, 0, 0, "X"), 25, 32);
    frm_bottom->easy_row();
    
    
    //Properties -- main.
    frm_main->widgets["but_category"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_OBJECT, false);
    };
    frm_main->widgets["but_category"]->description = "Pick a category.";
    frm_main->widgets["but_object"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_OBJECT + 1 + ed_mob_type_list, false);
    };
    frm_main->widgets["but_object"]->description = "Pick an object to edit.";
    frm_main->widgets["frm_object"]->widgets["but_anims"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_cur_hitbox_instance_nr = string::npos;
        if(ed_cur_anim) if(ed_cur_anim->frame_instances.size()) ed_cur_frame_instance_nr = 0;
        ed_mode = EDITOR_MODE_ANIMATION;
        hide_widget(ed_gui->widgets["frm_main"]); show_widget(ed_gui->widgets["frm_anims"]);
        gui_load_animation();
    };
    frm_main->widgets["frm_object"]->widgets["but_anims"]->description = "Change the way the animations look like.";
    frm_main->widgets["frm_object"]->widgets["but_frames"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_mode = EDITOR_MODE_FRAME;
        if(ed_cur_frame) if(ed_cur_frame->hitbox_instances.size()) ed_cur_hitbox_instance_nr = 0;
        hide_widget(ed_gui->widgets["frm_main"]); show_widget(ed_gui->widgets["frm_frames"]);
        gui_load_frame();
    };
    frm_main->widgets["frm_object"]->widgets["but_frames"]->description = "Change how each individual frame looks like.";
    frm_main->widgets["frm_object"]->widgets["but_hitboxes"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_mode = EDITOR_MODE_HITBOX;
        hide_widget(ed_gui->widgets["frm_main"]); show_widget(ed_gui->widgets["frm_hitboxes"]);
        gui_load_hitbox();
    };
    frm_main->widgets["frm_object"]->widgets["but_hitboxes"]->description = "Change the way each hitbox works.";
    
    
    //Properties -- animations.
    auto lambda_gui_save_animation = [] (lafi::widget*) { gui_save_animation(); };
    auto lambda_gui_save_frame_instance = [] (lafi::widget*) { gui_save_frame_instance(); };
    
    frm_anims->widgets["but_back"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        ed_anim_playing = false;
        hide_widget(ed_gui->widgets["frm_anims"]); show_widget(ed_gui->widgets["frm_main"]);
        update_stats();
    };
    frm_anims->widgets["but_back"]->description = "Go back to the main menu.";
    frm_anims->widgets["but_del_anim"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        if(!ed_cur_anim) return;
        ed_anims.animations.erase(ed_anims.animations.begin() + ed_anims.find_animation(ed_cur_anim->name));
        ed_anim_playing = false;
        ed_cur_anim = NULL;
        ed_cur_frame_instance_nr = string::npos;
        ed_cur_hitbox_instance_nr = string::npos;
        gui_load_animation();
    };
    frm_anims->widgets["but_del_anim"]->description = "Delete the current animation.";
    frm_anims->widgets["but_anim"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_anim_playing = false;
        open_picker(ANIMATION_EDITOR_PICKER_ANIMATION, true);
    };
    frm_anims->widgets["but_anim"]->description = "Pick an animation to edit.";
    frm_anims->widgets["frm_anim"]->widgets["but_play"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        if(ed_cur_anim->frame_instances.size() < 2) {
            ed_anim_playing = false;
        } else {
            ed_anim_playing = !ed_anim_playing;
            if(ed_cur_anim->frame_instances.size() > 0 && ed_cur_frame_instance_nr == string::npos) ed_cur_frame_instance_nr = 0;
            ed_cur_frame_time = 0;
        }
    };
    frm_anims->widgets["frm_anim"]->widgets["but_play"]->description = "Play or pause the animation.";
    frm_anims->widgets["frm_anim"]->widgets["but_prev"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_anim_playing = false;
        if(ed_cur_anim->frame_instances.size() > 0) {
            if(ed_cur_frame_instance_nr == string::npos) ed_cur_frame_instance_nr = 0;
            else if(ed_cur_frame_instance_nr == 0) ed_cur_frame_instance_nr = ed_cur_anim->frame_instances.size() - 1;
            else ed_cur_frame_instance_nr--;
        }
        gui_load_frame_instance();
    };
    frm_anims->widgets["frm_anim"]->widgets["but_prev"]->description = "Previous frame.";
    frm_anims->widgets["frm_anim"]->widgets["but_next"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_anim_playing = false;
        if(ed_cur_anim->frame_instances.size() > 0) {
            if(ed_cur_frame_instance_nr == ed_cur_anim->frame_instances.size() - 1 || ed_cur_frame_instance_nr == string::npos) ed_cur_frame_instance_nr = 0;
            else ed_cur_frame_instance_nr++;
        }
        gui_load_frame_instance();
    };
    frm_anims->widgets["frm_anim"]->widgets["but_next"]->description = "Next frame.";
    frm_anims->widgets["frm_anim"]->widgets["but_add"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_anim_playing = false;
        if(ed_cur_frame_instance_nr != string::npos) {
            ed_cur_frame_instance_nr++;
            ed_cur_anim->frame_instances.insert(
                ed_cur_anim->frame_instances.begin() + ed_cur_frame_instance_nr,
                frame_instance(ed_cur_anim->frame_instances[ed_cur_frame_instance_nr - 1])
            );
        } else {
            ed_cur_anim->frame_instances.push_back(frame_instance());
            ed_cur_frame_instance_nr = 0;
        }
        gui_load_frame_instance();
    };
    frm_anims->widgets["frm_anim"]->widgets["but_add"]->description = "Add a new frame after the current one (via copy).";
    frm_anims->widgets["frm_anim"]->widgets["but_rem"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_anim_playing = false;
        if(ed_cur_frame_instance_nr != string::npos) {
            ed_cur_anim->frame_instances.erase(ed_cur_anim->frame_instances.begin() + ed_cur_frame_instance_nr);
            if(ed_cur_anim->frame_instances.size() == 0) ed_cur_frame_instance_nr = string::npos;
            else if(ed_cur_frame_instance_nr >= ed_cur_anim->frame_instances.size()) ed_cur_frame_instance_nr = ed_cur_anim->frame_instances.size() - 1;
        }
        gui_load_frame_instance();
    };
    frm_anims->widgets["frm_anim"]->widgets["but_rem"]->description = "Remove the current frame.";
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["but_frame"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_anim_playing = false;
        open_picker(ANIMATION_EDITOR_PICKER_FRAME_INSTANCE, false);
    };
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["but_frame"]->description = "Pick the frame to use here.";
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["txt_dur"]->lose_focus_handler = lambda_gui_save_frame_instance;
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["txt_dur"]->mouse_down_handler = [] (lafi::widget*, int, int, int) {
        ed_anim_playing = false;
    };
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["txt_dur"]->description = "How long this frame lasts for, in seconds.";
    frm_anims->widgets["frm_anim"]->widgets["txt_loop"]->lose_focus_handler = lambda_gui_save_animation;
    frm_anims->widgets["frm_anim"]->widgets["txt_loop"]->description = "When the animation reaches the last frame, it loops back to this one.";
    frm_anims->register_accelerator(ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL, frm_anims->widgets["frm_anim"]->widgets["but_next"]);
    frm_anims->register_accelerator(ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_SHIFT, frm_anims->widgets["frm_anim"]->widgets["but_prev"]);
    
    
    //Properties -- frames.
    auto lambda_gui_save_frame = [] (lafi::widget*) { gui_save_frame(); };
    auto lambda_gui_save_hitbox_instance = [] (lafi::widget*) { gui_save_hitbox_instance(); };
    
    frm_frames->widgets["but_back"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        hide_widget(ed_gui->widgets["frm_frames"]); show_widget(ed_gui->widgets["frm_main"]);
        update_stats();
    };
    frm_frames->widgets["but_back"]->description = "Go back to the main menu.";
    frm_frames->widgets["but_del_frame"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        if(!ed_cur_frame) return;
        ed_anims.frames.erase(ed_anims.frames.begin() + ed_anims.find_frame(ed_cur_frame->name));
        ed_cur_frame = NULL;
        ed_cur_hitbox_instance_nr = string::npos;
        gui_load_frame();
    };
    frm_frames->widgets["but_del_frame"]->description = "Delete the current frame.";
    frm_frames->widgets["but_frame"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_FRAME, true);
    };
    frm_frames->widgets["but_frame"]->description = "Pick a frame to edit.";
    frm_frames->widgets["frm_frame"]->widgets["but_prev"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        if(ed_cur_frame->hitbox_instances.size()) {
            if(ed_cur_hitbox_instance_nr == string::npos) ed_cur_hitbox_instance_nr = 0;
            else if(ed_cur_hitbox_instance_nr == 0) ed_cur_hitbox_instance_nr = ed_cur_frame->hitbox_instances.size() - 1;
            else ed_cur_hitbox_instance_nr--;
        }
        gui_load_hitbox_instance();
    };
    frm_frames->widgets["frm_frame"]->widgets["but_top"]->description = "Edit the Pikmin's top (leaf/bud/flower) for this frame.";
    frm_frames->widgets["frm_frame"]->widgets["but_top"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        show_widget(ed_gui->widgets["frm_top"]);
        hide_widget(ed_gui->widgets["frm_frames"]);
        ed_mode = EDITOR_MODE_TOP;
        gui_load_top();
    };
    frm_frames->widgets["frm_frame"]->widgets["but_prev"]->description = "Previous hitbox.";
    frm_frames->widgets["frm_frame"]->widgets["but_next"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        if(ed_cur_frame->hitbox_instances.size()) {
            if(ed_cur_hitbox_instance_nr == string::npos) ed_cur_hitbox_instance_nr = 0;
            ed_cur_hitbox_instance_nr = (ed_cur_hitbox_instance_nr + 1) % ed_cur_frame->hitbox_instances.size();
        }
        gui_load_hitbox_instance();
    };
    frm_frames->widgets["frm_frame"]->widgets["but_next"]->description = "Next hitbox.";
    frm_frames->widgets["frm_frame"]->widgets["but_add"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        if(ed_cur_hitbox_instance_nr != string::npos) {
            ed_cur_hitbox_instance_nr++;
            ed_cur_frame->hitbox_instances.insert(
                ed_cur_frame->hitbox_instances.begin() + ed_cur_hitbox_instance_nr,
                hitbox_instance()
            );
        } else {
            ed_cur_frame->hitbox_instances.push_back(hitbox_instance());
            ed_cur_hitbox_instance_nr = 0;
        }
        gui_load_hitbox_instance();
    };
    frm_frames->widgets["frm_frame"]->widgets["but_add"]->description = "Add a new hitbox after the current one.";
    frm_frames->widgets["frm_frame"]->widgets["but_rem"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        if(ed_cur_hitbox_instance_nr != string::npos) {
            ed_cur_frame->hitbox_instances.erase(ed_cur_frame->hitbox_instances.begin() + ed_cur_hitbox_instance_nr);
            if(ed_cur_frame->hitbox_instances.size() == 0) ed_cur_hitbox_instance_nr = string::npos;
            else if(ed_cur_hitbox_instance_nr >= ed_cur_frame->hitbox_instances.size()) ed_cur_hitbox_instance_nr = ed_cur_frame->hitbox_instances.size() - 1;
        }
        gui_load_hitbox_instance();
    };
    frm_frames->widgets["frm_frame"]->widgets["but_rem"]->description = "Remove the current hitbox.";
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["but_hitbox"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_HITBOX_INSTANCE, false);
    };
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["but_hitbox"]->description = "Pick the hitbox to use here.";
    frm_frames->widgets["frm_frame"]->widgets["txt_file"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_file"]->description = "Name (and extension) of the image file where the sprite is.";
    frm_frames->widgets["frm_frame"]->widgets["txt_filex"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_filex"]->description = "X of the top-left corner of the sprite.";
    frm_frames->widgets["frm_frame"]->widgets["txt_filey"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_filey"]->description = "Y of the top-left corner of the sprite.";
    frm_frames->widgets["frm_frame"]->widgets["txt_filew"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_filew"]->description = "Width of the sprite, in the file.";
    frm_frames->widgets["frm_frame"]->widgets["txt_fileh"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_fileh"]->description = "Height of the sprite, in the file.";
    frm_frames->widgets["frm_frame"]->widgets["txt_gamew"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_gamew"]->description = "In-game width.";
    frm_frames->widgets["frm_frame"]->widgets["txt_gameh"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_gameh"]->description = "In-game height.";
    frm_frames->widgets["frm_frame"]->widgets["txt_offsx"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_offsx"]->description = "In-game, offset by this much, horizontally.";
    frm_frames->widgets["frm_frame"]->widgets["txt_offsy"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_offsy"]->description = "In-game, offset by this much, vertically.";
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["txt_x"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["txt_x"]->description = "X of the hitbox' center.";
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["txt_y"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["txt_y"]->description = "Y of the hitbox' center.";
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["txt_z"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["txt_z"]->description = "Z of the hitbox' center.";
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["txt_r"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_frames->widgets["frm_frame"]->widgets["frm_hitbox_i"]->widgets["txt_r"]->description = "Hitbox' radius.";
    frm_frames->register_accelerator(ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL, frm_frames->widgets["frm_frame"]->widgets["but_next"]);
    frm_frames->register_accelerator(ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_SHIFT, frm_frames->widgets["frm_frame"]->widgets["but_prev"]);
    
    
    //Properties -- hitboxes.
    auto lambda_gui_save_hitbox = [] (lafi::widget*) { gui_save_hitbox(); };
    auto lambda_gui_save_hitbox_click = [] (lafi::widget*, int, int) { gui_save_hitbox(); };
    
    frm_hitboxes->widgets["but_back"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_mode = EDITOR_MODE_MAIN;
        hide_widget(ed_gui->widgets["frm_hitboxes"]); show_widget(ed_gui->widgets["frm_main"]);
        update_stats();
    };
    frm_hitboxes->widgets["but_back"]->description = "Go back to the main menu.";
    frm_hitboxes->widgets["but_del_h"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        if(!ed_cur_hitbox) return;
        ed_anims.hitboxes.erase(ed_anims.hitboxes.begin() + ed_anims.find_hitbox(ed_cur_hitbox->name));
        ed_cur_hitbox = NULL;
        gui_load_hitbox();
    };
    frm_hitboxes->widgets["but_del_h"]->description = "Delete the current hitbox.";
    frm_hitboxes->widgets["but_hitbox"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_HITBOX, true);
    };
    frm_hitbox->widgets["rad_normal"]->left_mouse_click_handler = lambda_gui_save_hitbox_click;
    frm_hitbox->widgets["rad_attack"]->left_mouse_click_handler = lambda_gui_save_hitbox_click;
    frm_normal->widgets["txt_mult"]->lose_focus_handler = lambda_gui_save_hitbox;
    frm_normal->widgets["chk_latch"]->left_mouse_click_handler = lambda_gui_save_hitbox_click;
    frm_normal->widgets["txt_hazards"]->lose_focus_handler = lambda_gui_save_hitbox;
    frm_attack->widgets["txt_mult"]->lose_focus_handler = lambda_gui_save_hitbox;
    frm_attack->widgets["txt_hazards"]->lose_focus_handler = lambda_gui_save_hitbox;
    frm_attack->widgets["chk_outward"]->left_mouse_click_handler = lambda_gui_save_hitbox_click;
    frm_attack->widgets["ang_angle"]->lose_focus_handler = lambda_gui_save_hitbox;
    frm_attack->widgets["txt_knockback"]->lose_focus_handler = lambda_gui_save_hitbox;
    frm_hitboxes->widgets["but_hitbox"]->description = "Pick a hitbox to edit.";
    frm_normal->widgets["txt_mult"]->description = "Defense multiplier. 0 = invulnerable.";
    frm_normal->widgets["chk_latch"]->description = "Can the Pikmin latch on to this hitbox?";
    frm_normal->widgets["txt_hazards"]->description = "List of hazards, comma separated.";
    frm_attack->widgets["txt_mult"]->description = "Attack multiplier.";
    frm_attack->widgets["txt_hazards"]->description = "List of hazards, comma separated.";
    frm_attack->widgets["chk_outward"]->description = "Makes Pikmin be knocked away from the center.";
    frm_attack->widgets["ang_angle"]->description = "Angle the Pikmin are knocked towards.";
    frm_attack->widgets["txt_knockback"]->description = "Knockback strength.";
    
    
    //Properties -- picker.
    frm_picker->widgets["but_back"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ((lafi::textbox*) ed_gui->widgets["frm_picker"]->widgets["txt_new"])->text.clear();
        
        hide_widget(ed_gui->widgets["frm_picker"]);
        show_widget(ed_gui->widgets["frm_bottom"]);
        if(ed_mode == EDITOR_MODE_MAIN) {
            show_widget(ed_gui->widgets["frm_main"]);
        } else if(ed_mode == EDITOR_MODE_ANIMATION) {
            show_widget(ed_gui->widgets["frm_anims"]);
        } else if(ed_mode == EDITOR_MODE_FRAME) {
            show_widget(ed_gui->widgets["frm_frames"]);
        } else if(ed_mode == EDITOR_MODE_HITBOX) {
            show_widget(ed_gui->widgets["frm_hitboxes"]);
        }
    };
    frm_picker->widgets["but_back"]->description = "Cancel.";
    ((lafi::textbox*) frm_picker->widgets["txt_new"])->enter_key_widget = frm_picker->widgets["but_new"];
    frm_picker->widgets["but_new"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        string name = ((lafi::textbox*) ed_gui->widgets["frm_picker"]->widgets["txt_new"])->text;
        if(name.size() == 0) return;
        
        if(ed_mode == EDITOR_MODE_ANIMATION) {
            if(ed_anims.find_animation(name) != string::npos) return;
            ed_anims.animations.push_back(new animation(name));
            pick(name, ANIMATION_EDITOR_PICKER_ANIMATION);
            
        } else if(ed_mode == EDITOR_MODE_FRAME) {
            if(ed_anims.find_frame(name) != string::npos) return;
            ed_anims.frames.push_back(new frame(name));
            pick(name, ANIMATION_EDITOR_PICKER_FRAME);
            
        } else if(ed_mode == EDITOR_MODE_HITBOX) {
            if(ed_anims.find_hitbox(name) != string::npos) return;
            ed_anims.hitboxes.push_back(new hitbox(name));
            pick(name, ANIMATION_EDITOR_PICKER_HITBOX);
        }
        
        ((lafi::textbox*) ed_gui->widgets["frm_picker"]->widgets["txt_new"])->text.clear();
    };
    frm_picker->widgets["but_new"]->description = "Create a new one with the name on the textbox.";
    frm_picker->widgets["frm_list"]->mouse_wheel_handler = [] (lafi::widget*, int dy, int) {
        lafi::scrollbar* s = (lafi::scrollbar*) ed_gui->widgets["frm_picker"]->widgets["bar_scroll"];
        if(s->widgets.find("but_bar") != s->widgets.end()) {
            s->move_button(0, (s->widgets["but_bar"]->y1 + s->widgets["but_bar"]->y2) / 2 - 30 * dy);
        }
    };
    
    
    //Properties -- Pikmin top.
    auto lambda_save_top = [] (lafi::widget*) { gui_save_top(); };
    auto lambda_save_top_click = [] (lafi::widget*, int, int) { gui_save_top(); };
    frm_top->widgets["but_back"]->description = "Go back.";
    frm_top->widgets["but_back"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        show_widget(ed_gui->widgets["frm_frames"]);
        hide_widget(ed_gui->widgets["frm_top"]);
        ed_mode = EDITOR_MODE_FRAME;
    };
    frm_top->widgets["chk_visible"]->description = "Is the top visible in this frame?";
    frm_top->widgets["chk_visible"]->left_mouse_click_handler = lambda_save_top_click;
    frm_top->widgets["txt_x"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["txt_y"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["txt_w"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["txt_h"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["ang_angle"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["but_maturity"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_maturity = (ed_maturity + 1) % 3;
    };
    frm_top->widgets["but_maturity"]->description = "View a different maturity top.";
    
    
    //Properties -- bottom bar.
    frm_bottom->widgets["but_toggle_hitboxes"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        ed_hitboxes_visible = !ed_hitboxes_visible;
    };
    frm_bottom->widgets["but_toggle_hitboxes"]->description = "Toggle hitbox and center-point grid visibility.";
    frm_bottom->widgets["but_load"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        load_animation_set();
        hide_widget(ed_gui->widgets["frm_anims"]);
        hide_widget(ed_gui->widgets["frm_frames"]);
        hide_widget(ed_gui->widgets["frm_hitboxes"]);
        show_widget(ed_gui->widgets["frm_main"]);
        ed_mode = EDITOR_MODE_MAIN;
        update_stats();
    };
    frm_bottom->widgets["but_load"]->description = "Load the object from the text file.";
    frm_bottom->widgets["but_save"]->left_mouse_click_handler = [] (lafi::widget*, int, int) {
        save_animation_set();
    };
    frm_bottom->widgets["but_save"]->description = "Save the object to the text file.";
    frm_bottom->widgets["but_quit"]->description = "Quit the animation editor.";
    
    //ToDo quit button.
    
    lafi::label* ed_gui_status_bar = new lafi::label(0, scr_h - 16, scr_w - 208, scr_h);
    ed_gui->add("lbl_status_bar", ed_gui_status_bar);
    
    update_stats();
    disable_widget(frm_bottom->widgets["but_load"]);
    disable_widget(frm_bottom->widgets["but_save"]);
}

/* ----------------------------------------------------------------------------
 * Loads the animation set for the current object.
 */
void animation_editor::load_animation_set() {
    ed_anims.destroy();
    
    data_node file = data_node(ed_file_name);
    if(!file.file_was_opened) {
        file.save_file(ed_file_name, true);
    }
    ed_anims = load_animation_set(&file);
    
    ed_anim_playing = false;
    ed_cur_anim = NULL;
    ed_cur_frame = NULL;
    ed_cur_hitbox = NULL;
    ed_cur_frame_instance_nr = string::npos;
    ed_cur_hitbox_instance_nr = string::npos;
    if(ed_anims.animations.size() > 0) {
        ed_cur_anim = ed_anims.animations[0];
        if(ed_cur_anim->frame_instances.size()) ed_cur_frame_instance_nr = 0;
    }
    if(ed_anims.frames.size() > 0) {
        ed_cur_frame = ed_anims.frames[0];
        if(ed_cur_frame->hitbox_instances.size()) ed_cur_hitbox_instance_nr = 0;
    }
    if(ed_anims.hitboxes.size() > 0) {
        ed_cur_hitbox = ed_anims.hitboxes[0];
    }
    
    enable_widget(ed_gui->widgets["frm_bottom"]->widgets["but_load"]);
    enable_widget(ed_gui->widgets["frm_bottom"]->widgets["but_save"]);
    
    cam_x = cam_y = 0;
    cam_zoom = 1;
}

/* ----------------------------------------------------------------------------
 * Opens the correct radio button and frame for the specified hitbox type.
 */
void animation_editor::open_hitbox_type(unsigned char type) {
    lafi::widget* f = ed_gui->widgets["frm_hitboxes"]->widgets["frm_hitbox"];
    
    ((lafi::radio_button*) f->widgets["rad_normal"])->unselect();
    ((lafi::radio_button*) f->widgets["rad_attack"])->unselect();
    
    hide_widget(f->widgets["frm_normal"]);
    hide_widget(f->widgets["frm_attack"]);
    
    if(type == HITBOX_TYPE_NORMAL) {
        ((lafi::radio_button*) f->widgets["rad_normal"])->select();
        show_widget(f->widgets["frm_normal"]);
    } else if(type == HITBOX_TYPE_ATTACK) {
        ((lafi::radio_button*) f->widgets["rad_attack"])->select();
        show_widget(f->widgets["frm_attack"]);
    }
}

/* ----------------------------------------------------------------------------
 * Opens the frame where you pick from a list.
 * For the type, use animation_editor::ANIMATION_EDITOR_PICKER_*.
 */
void animation_editor::open_picker(unsigned char type, bool can_make_new) {
    show_widget(ed_gui->widgets["frm_picker"]);
    hide_widget(ed_gui->widgets["frm_bottom"]);
    
    lafi::widget* f = ed_gui->widgets["frm_picker"]->widgets["frm_list"];
    
    if(can_make_new) {
        enable_widget(ed_gui->widgets["frm_picker"]->widgets["txt_new"]);
        enable_widget(ed_gui->widgets["frm_picker"]->widgets["but_new"]);
    } else {
        disable_widget(ed_gui->widgets["frm_picker"]->widgets["txt_new"]);
        disable_widget(ed_gui->widgets["frm_picker"]->widgets["but_new"]);
    }
    
    while(f->widgets.size()) {
        f->remove(f->widgets.begin()->first);
    }
    
    vector<string> elements;
    if(type == ANIMATION_EDITOR_PICKER_OBJECT) {
        elements.push_back("Enemies");
        elements.push_back("Leaders");
        elements.push_back("Onions");
        elements.push_back("Pellets");
        elements.push_back("Pikmin");
        elements.push_back("Treasures");
    } else if(type == ANIMATION_EDITOR_PICKER_ANIMATION) {
        for(size_t a = 0; a < ed_anims.animations.size(); a++) {
            elements.push_back(ed_anims.animations[a]->name);
        }
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME || type == ANIMATION_EDITOR_PICKER_FRAME_INSTANCE) {
        for(size_t f = 0; f < ed_anims.frames.size(); f++) {
            elements.push_back(ed_anims.frames[f]->name);
        }
    } else if(type == ANIMATION_EDITOR_PICKER_HITBOX || type == ANIMATION_EDITOR_PICKER_HITBOX_INSTANCE) {
        for(size_t h = 0; h < ed_anims.hitboxes.size(); h++) {
            elements.push_back(ed_anims.hitboxes[h]->name);
        }
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_ENEMIES) {
        elements = folder_to_vector(ENEMIES_FOLDER, true);
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_LEADERS) {
        elements = folder_to_vector(LEADERS_FOLDER, true);
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_ONIONS) {
        elements = folder_to_vector(ONIONS_FOLDER, true);
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_PELLETS) {
        elements = folder_to_vector(PELLETS_FOLDER, true);
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_PIKMIN) {
        elements = folder_to_vector(PIKMIN_FOLDER, true);
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_TREASURES) {
        elements = folder_to_vector(TREASURES_FOLDER, true);
    }
    
    if(type >= ANIMATION_EDITOR_PICKER_OBJECT) {
        hide_widget(ed_gui->widgets["frm_main"]);
    } else if(type == ANIMATION_EDITOR_PICKER_ANIMATION || type == ANIMATION_EDITOR_PICKER_FRAME_INSTANCE) {
        hide_widget(ed_gui->widgets["frm_anims"]);
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME || type == ANIMATION_EDITOR_PICKER_HITBOX_INSTANCE) {
        hide_widget(ed_gui->widgets["frm_frames"]);
    } else if(type == ANIMATION_EDITOR_PICKER_HITBOX) {
        hide_widget(ed_gui->widgets["frm_hitboxes"]);
    }
    
    f->easy_reset();
    f->easy_row();
    for(size_t e = 0; e < elements.size(); e++) {
        lafi::button* b = new lafi::button(0, 0, 0, 0, elements[e]);
        string name = elements[e];
        b->left_mouse_click_handler = [name, type] (lafi::widget*, int, int) {
            pick(name, type);
        };
        f->easy_add("but_" + i2s(e), b, 100, 24);
        f->easy_row(0);
    }
    
    ((lafi::scrollbar*) ed_gui->widgets["frm_picker"]->widgets["bar_scroll"])->make_widget_scroll(f);
}

/* ----------------------------------------------------------------------------
 * Closes the list picker frame.
 */
void animation_editor::pick(string name, unsigned char type) {
    hide_widget(ed_gui->widgets["frm_picker"]);
    show_widget(ed_gui->widgets["frm_bottom"]);
    
    if(type == ANIMATION_EDITOR_PICKER_OBJECT) {
        if(name == "Enemies")        ed_mob_type_list = MOB_CATEGORY_ENEMIES;
        else if(name == "Leaders")   ed_mob_type_list = MOB_CATEGORY_LEADERS;
        else if(name == "Onions")    ed_mob_type_list = MOB_CATEGORY_ONIONS;
        else if(name == "Pellets")   ed_mob_type_list = MOB_CATEGORY_PELLETS;
        else if(name == "Pikmin")    ed_mob_type_list = MOB_CATEGORY_PIKMIN;
        else if(name == "Treasures") ed_mob_type_list = MOB_CATEGORY_TREASURES;
        ed_object_name.clear();
        update_stats();
        disable_widget(ed_gui->widgets["frm_bottom"]->widgets["but_load"]);
        disable_widget(ed_gui->widgets["frm_bottom"]->widgets["but_save"]);
        
    } else if(type == ANIMATION_EDITOR_PICKER_ANIMATION) {
        ed_cur_anim = ed_anims.animations[ed_anims.find_animation(name)];
        ed_cur_frame_instance_nr = (ed_cur_anim->frame_instances.size()) ? 0 : string::npos;
        ed_cur_hitbox_instance_nr = string::npos;
        show_widget(ed_gui->widgets["frm_anims"]);
        gui_load_animation();
        
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME_INSTANCE) {
        ed_cur_anim->frame_instances[ed_cur_frame_instance_nr].frame_name = name;
        ed_cur_anim->frame_instances[ed_cur_frame_instance_nr].frame_ptr = ed_anims.frames[ed_anims.find_frame(name)];
        show_widget(ed_gui->widgets["frm_anims"]);
        gui_load_frame_instance();
        
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME) {
        ed_cur_frame = ed_anims.frames[ed_anims.find_frame(name)];
        ed_cur_hitbox_instance_nr = (ed_cur_frame->hitbox_instances.size()) ? 0 : string::npos;
        show_widget(ed_gui->widgets["frm_frames"]);
        gui_load_frame();
        
    } else if(type == ANIMATION_EDITOR_PICKER_HITBOX_INSTANCE) {
        ed_cur_frame->hitbox_instances[ed_cur_hitbox_instance_nr].hitbox_name = name;
        ed_cur_frame->hitbox_instances[ed_cur_hitbox_instance_nr].hitbox_ptr = ed_anims.hitboxes[ed_anims.find_hitbox(name)];
        show_widget(ed_gui->widgets["frm_frames"]);
        gui_load_hitbox_instance();
        
    } else if(type == ANIMATION_EDITOR_PICKER_HITBOX) {
        ed_cur_hitbox = ed_anims.hitboxes[ed_anims.find_hitbox(name)];
        show_widget(ed_gui->widgets["frm_hitboxes"]);
        gui_load_hitbox();
        
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_ENEMIES) {
        ed_file_name = ENEMIES_FOLDER;
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_LEADERS) {
        ed_file_name = LEADERS_FOLDER;
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_ONIONS) {
        ed_file_name = ONIONS_FOLDER;
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_PELLETS) {
        ed_file_name = PELLETS_FOLDER;
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_PIKMIN) {
        ed_file_name = PIKMIN_FOLDER;
    } else if(type == ANIMATION_EDITOR_PICKER_OBJECT + 1 + MOB_CATEGORY_TREASURES) {
        ed_file_name = TREASURES_FOLDER;
    }
    
    if(type > ANIMATION_EDITOR_PICKER_OBJECT) {
        string temp_path_start = ed_file_name;
        ed_file_name += "/" + name + "/Animations.txt";
        ed_object_name = name;
        load_animation_set();
        
        //Top bitmap.
        for(unsigned char t = 0; t < 3; t++) {
            if(ed_top_bmp[t] && ed_top_bmp[t] != bmp_error) {
                al_destroy_bitmap(ed_top_bmp[t]);
                ed_top_bmp[t] = NULL;
            }
        }
        
        if(ed_mob_type_list == MOB_CATEGORY_PIKMIN) {
            data_node data = data_node(temp_path_start + "/" + name + "/Data.txt");
            ed_top_bmp[0] = load_bmp(data.get_child_by_name("top_leaf")->value, &data);
            ed_top_bmp[1] = load_bmp(data.get_child_by_name("top_bud")->value, &data);
            ed_top_bmp[2] = load_bmp(data.get_child_by_name("top_flower")->value, &data);
        }
    }
    if(type >= ANIMATION_EDITOR_PICKER_OBJECT) {
        show_widget(ed_gui->widgets["frm_main"]);
        update_stats();
    }
}

/* ----------------------------------------------------------------------------
 * Saves the animation set onto the mob's file.
 */
void animation_editor::save_animation_set() {
    data_node file_node = data_node("", "");
    
    data_node* animations_node = new data_node("animations", "");
    file_node.add(animations_node);
    
    for(size_t a = 0; a < ed_anims.animations.size(); a++) {
        data_node* anim_node = new data_node(ed_anims.animations[a]->name, "");
        animations_node->add(anim_node);
        
        anim_node->add(new data_node("loop_frame", i2s(ed_anims.animations[a]->loop_frame)));
        data_node* frame_instances_node = new data_node("frame_instances", "");
        anim_node->add(frame_instances_node);
        
        for(size_t fi = 0; fi < ed_anims.animations[a]->frame_instances.size(); fi++) {
            frame_instance* fi_ptr = &ed_anims.animations[a]->frame_instances[fi];
            
            data_node* frame_instance_node = new data_node(fi_ptr->frame_name, "");
            frame_instances_node->add(frame_instance_node);
            
            frame_instance_node->add(new data_node("duration", f2s(fi_ptr->duration)));
        }
    }
    
    data_node* frames_node = new data_node("frames", "");
    file_node.add(frames_node);
    
    for(size_t f = 0; f < ed_anims.frames.size(); f++) {
        data_node* frame_node = new data_node(ed_anims.frames[f]->name, "");
        frames_node->add(frame_node);
        
        frame_node->add(new data_node("file", ed_anims.frames[f]->file));
        frame_node->add(new data_node("file_x", i2s(ed_anims.frames[f]->file_x)));
        frame_node->add(new data_node("file_y", i2s(ed_anims.frames[f]->file_y)));
        frame_node->add(new data_node("file_w", i2s(ed_anims.frames[f]->file_w)));
        frame_node->add(new data_node("file_h", i2s(ed_anims.frames[f]->file_h)));
        frame_node->add(new data_node("game_w", f2s(ed_anims.frames[f]->game_w)));
        frame_node->add(new data_node("game_h", f2s(ed_anims.frames[f]->game_h)));
        frame_node->add(new data_node("offs_x", f2s(ed_anims.frames[f]->offs_x)));
        frame_node->add(new data_node("offs_y", f2s(ed_anims.frames[f]->offs_y)));
        
        if(ed_mob_type_list == MOB_CATEGORY_PIKMIN) {
            frame_node->add(new data_node("top_visible", b2s(ed_anims.frames[f]->top_visible)));
            frame_node->add(new data_node("top_x", f2s(ed_anims.frames[f]->top_x)));
            frame_node->add(new data_node("top_y", f2s(ed_anims.frames[f]->top_y)));
            frame_node->add(new data_node("top_w", f2s(ed_anims.frames[f]->top_w)));
            frame_node->add(new data_node("top_h", f2s(ed_anims.frames[f]->top_h)));
            frame_node->add(new data_node("top_angle", f2s(ed_anims.frames[f]->top_angle)));
        }
        
        data_node* hitbox_instances_node = new data_node("hitbox_instances", "");
        frame_node->add(hitbox_instances_node);
        
        for(size_t hi = 0; hi < ed_anims.frames[f]->hitbox_instances.size(); hi++) {
            hitbox_instance* hi_ptr = &ed_anims.frames[f]->hitbox_instances[hi];
            
            data_node* hitbox_instance_node = new data_node(hi_ptr->hitbox_name, "");
            hitbox_instances_node->add(hitbox_instance_node);
            
            hitbox_instance_node->add(
                new data_node(
                    "coords",
                    f2s(hi_ptr->x) + " " + f2s(hi_ptr->y) + " " + f2s(hi_ptr->z)
                )
            );
            hitbox_instance_node->add(new data_node("radius", f2s(hi_ptr->radius)));
        }
    }
    
    data_node* hitboxes_node = new data_node("hitboxes", "");
    file_node.add(hitboxes_node);
    
    for(size_t h = 0; h < ed_anims.hitboxes.size(); h++) {
        data_node* hitbox_node = new data_node(ed_anims.hitboxes[h]->name, "");
        hitboxes_node->add(hitbox_node);
        
        hitbox_node->add(new data_node("type", i2s(ed_anims.hitboxes[h]->type)));
        hitbox_node->add(new data_node("multiplier", f2s(ed_anims.hitboxes[h]->multiplier)));
        hitbox_node->add(new data_node("can_pikmin_latch", b2s(ed_anims.hitboxes[h]->can_pikmin_latch)));
        hitbox_node->add(new data_node("elements", ed_anims.hitboxes[h]->elements));
        hitbox_node->add(new data_node("outward", b2s(ed_anims.hitboxes[h]->knockback_outward)));
        hitbox_node->add(new data_node("angle", f2s(ed_anims.hitboxes[h]->knockback_angle)));
        hitbox_node->add(new data_node("knockback", f2s(ed_anims.hitboxes[h]->knockback)));
    }
    
    file_node.save_file(ed_file_name);
}

/* ----------------------------------------------------------------------------
 * Update the stats on the main menu, as well as some other minor things.
 */
void animation_editor::update_stats() {
    lafi::widget* f = ed_gui->widgets["frm_main"];
    string s;
    
    if(ed_mob_type_list == MOB_CATEGORY_ENEMIES)        s = "Enemies";
    else if(ed_mob_type_list == MOB_CATEGORY_LEADERS)   s = "Leaders";
    else if(ed_mob_type_list == MOB_CATEGORY_ONIONS)    s = "Onions";
    else if(ed_mob_type_list == MOB_CATEGORY_PELLETS)   s = "Pellets";
    else if(ed_mob_type_list == MOB_CATEGORY_PIKMIN)    s = "Pikmin";
    else if(ed_mob_type_list == MOB_CATEGORY_TREASURES) s = "Treasures";
    
    ((lafi::button*) f->widgets["but_category"])->text = s;
    ((lafi::button*) f->widgets["but_object"])->text = ed_object_name;
    
    f = f->widgets["frm_object"];
    if(ed_object_name.size()) { show_widget(f); } //Why the curly braces? Try removing them. You should get an "illegal else" error. Why? ...Good question.
    else hide_widget(f);
    
    ((lafi::label*) f->widgets["lbl_n_anims"])->text = "Animations: " + i2s(ed_anims.animations.size());
    ((lafi::label*) f->widgets["lbl_n_frames"])->text = "Frames: " + i2s(ed_anims.frames.size());
    ((lafi::label*) f->widgets["lbl_n_hitboxes"])->text = "Hitboxes: " + i2s(ed_anims.hitboxes.size());
}