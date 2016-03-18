/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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
#include "misc_structs.h"
#include "vars.h"


/* ----------------------------------------------------------------------------
 * Initializes animation editor class stuff.
 */
animation_editor::animation_editor() :
    cur_anim(NULL),
    anim_playing(false),
    cur_frame(NULL),
    cur_frame_instance_nr(INVALID),
    cur_frame_time(0),
    cur_hitbox_alpha(0),
    cur_hitbox_instance_nr(INVALID),
    cur_hitbox_nr(INVALID),
    file_dialog(NULL),
    grabbing_hitbox(INVALID),
    grabbing_hitbox_edge(false),
    grabbing_hitbox_x(0),
    grabbing_hitbox_y(0),
    gui(NULL),
    hitboxes_visible(true),
    holding_m1(false),
    holding_m2(false),
    is_pikmin(false),
    made_changes(false),
    maturity(0),
    mode(EDITOR_MODE_MAIN),
    new_hitbox_corner_x(FLT_MAX),
    new_hitbox_corner_y(FLT_MAX),
    sec_mode(ESM_NONE),
    wum(NULL) {
    top_bmp[0] = NULL;
    top_bmp[1] = NULL;
    top_bmp[2] = NULL;
    
}


/* ----------------------------------------------------------------------------
 * Closes the change warning box.
 */
void animation_editor::close_changes_warning() {
    hide_widget(gui->widgets["frm_changes"]);
    show_widget(gui->widgets["frm_bottom"]);
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the animation editor.
 */
void animation_editor::do_logic() {
    if(anim_playing && mode == EDITOR_MODE_ANIMATION && cur_anim && cur_frame_instance_nr != INVALID) {
        frame_instance* fi = &cur_anim->frame_instances[cur_frame_instance_nr];
        if(fi->duration != 0) {
            cur_frame_time += delta_t;
            
            while(cur_frame_time > fi->duration) {
                cur_frame_time = cur_frame_time - fi->duration;
                cur_frame_instance_nr++;
                if(cur_frame_instance_nr >= cur_anim->frame_instances.size()) {
                    cur_frame_instance_nr = (cur_anim->loop_frame >= cur_anim->frame_instances.size()) ? 0 : cur_anim->loop_frame;
                }
                fi = &cur_anim->frame_instances[cur_frame_instance_nr];
            }
        } else {
            anim_playing = false;
        }
        gui_load_animation();
    }
    
    cur_hitbox_alpha += M_PI * 3 * delta_t;
    
    fade_mgr.tick(delta_t);
    
}


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the animation editor.
 */
void animation_editor::do_drawing() {

    gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, cam_x + ((scr_w - 208) / 2.0 / cam_zoom), cam_y + (scr_h / 2.0 / cam_zoom));
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_set_clipping_rectangle(0, 0, scr_w - 208, scr_h - 16); {
    
        al_clear_to_color(al_map_rgb(128, 144, 128));
        
        frame* f = NULL;
        
        if(mode == EDITOR_MODE_ANIMATION) {
            if(cur_frame_instance_nr != INVALID) {
                string name = cur_anim->frame_instances[cur_frame_instance_nr].frame_name;
                size_t f_pos = anims.find_frame(name);
                if(f_pos != INVALID) f = anims.frames[f_pos];
            }
        } else if(mode == EDITOR_MODE_FRAME || mode == EDITOR_MODE_TOP || mode == EDITOR_MODE_HITBOX_INSTANCES) {
            f = cur_frame;
        }
        
        if(f) {
            if(f->bitmap) {
                draw_sprite(
                    f->bitmap,
                    f->offs_x, f->offs_y,
                    f->game_w, f->game_h
                );
            }
            
            if(hitboxes_visible) {
                size_t n_hitboxes = f->hitbox_instances.size();
                for(size_t h = 0; h < n_hitboxes; ++h) {
                    hitbox_instance* h_ptr = &f->hitbox_instances[h];
                    
                    ALLEGRO_COLOR hitbox_color, hitbox_outline_color;
                    unsigned char hitbox_outline_alpha = 63 + 192 * ((sin(cur_hitbox_alpha) / 2.0) + 0.5);
                    
                    if(h_ptr->type == HITBOX_TYPE_NORMAL) {
                        hitbox_color = al_map_rgba(0, 128, 0, 192); hitbox_outline_color = al_map_rgba(0, 64, 0, 255);
                    } else if(h_ptr->type == HITBOX_TYPE_ATTACK) {
                        hitbox_color = al_map_rgba(128, 0, 0, 192); hitbox_outline_color = al_map_rgba(64, 0, 0, 255);
                    } else {
                        hitbox_color = al_map_rgba(128, 128, 0, 192); hitbox_outline_color = al_map_rgba(64, 64, 0, 255);
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
                        (cur_hitbox_instance_nr == h ? change_alpha(hitbox_outline_color, hitbox_outline_alpha) : hitbox_outline_color),
                        (cur_hitbox_instance_nr == h ? 2 / cam_zoom : 1 / cam_zoom)
                    );
                }
            }
            
            if(f->top_visible && is_pikmin) {
                draw_sprite(
                    top_bmp[maturity],
                    f->top_x, f->top_y,
                    f->top_w, f->top_h,
                    f->top_angle
                );
            }
        }
        
        if(hitboxes_visible) {
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
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Loads the animation's data onto the gui.
 */
void animation_editor::gui_load_animation() {
    lafi::widget* f = gui->widgets["frm_anims"];
    
    ((lafi::button*) f->widgets["but_anim"])->text = cur_anim ? cur_anim->name : "";
    
    if(!cur_anim) {
        hide_widget(f->widgets["frm_anim"]);
    } else {
        show_widget(f->widgets["frm_anim"]);
        
        ((lafi::button*) f->widgets["but_anim"])->text = cur_anim->name;
        ((lafi::textbox*) f->widgets["frm_anim"]->widgets["txt_loop"])->text = i2s(cur_anim->loop_frame + 1);
        
        gui_load_frame_instance();
    }
}


/* ----------------------------------------------------------------------------
 * Loads the frame's data onto the gui.
 */
void animation_editor::gui_load_frame() {
    lafi::widget* f = gui->widgets["frm_frames"];
    
    ((lafi::button*) f->widgets["but_frame"])->text = cur_frame ? cur_frame->name : "";
    
    if(!cur_frame) {
        hide_widget(f->widgets["frm_frame"]);
    } else {
        show_widget(f->widgets["frm_frame"]);
        
        f = f->widgets["frm_frame"];
        
        ((lafi::textbox*) f->widgets["txt_file"])->text = cur_frame->file;
        ((lafi::textbox*) f->widgets["txt_filex"])->text = i2s(cur_frame->file_x);
        ((lafi::textbox*) f->widgets["txt_filey"])->text = i2s(cur_frame->file_y);
        ((lafi::textbox*) f->widgets["txt_filew"])->text = i2s(cur_frame->file_w);
        ((lafi::textbox*) f->widgets["txt_fileh"])->text = i2s(cur_frame->file_h);
        ((lafi::textbox*) f->widgets["txt_gamew"])->text = f2s(cur_frame->game_w);
        ((lafi::textbox*) f->widgets["txt_gameh"])->text = f2s(cur_frame->game_h);
        ((lafi::textbox*) f->widgets["txt_offsx"])->text = f2s(cur_frame->offs_x);
        ((lafi::textbox*) f->widgets["txt_offsy"])->text = f2s(cur_frame->offs_y);
        
        if(is_pikmin) {
            enable_widget(f->widgets["but_top"]);
        } else {
            disable_widget(f->widgets["but_top"]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the frame instance's data onto the gui.
 */
void animation_editor::gui_load_frame_instance() {
    lafi::widget* f = gui->widgets["frm_anims"]->widgets["frm_anim"];
    bool valid = cur_frame_instance_nr != INVALID && cur_anim;
    
    ((lafi::label*) f->widgets["lbl_f_nr"])->text =
        "Current frame: " +
        (valid ? i2s((cur_frame_instance_nr + 1)) : "--") +
        " / " + i2s(cur_anim->frame_instances.size());
        
    if(!valid) {
        hide_widget(f->widgets["frm_frame_i"]);
    } else {
        show_widget(f->widgets["frm_frame_i"]);
        
        ((lafi::button*) f->widgets["frm_frame_i"]->widgets["but_frame"])->text = cur_anim->frame_instances[cur_frame_instance_nr].frame_name;
        ((lafi::textbox*) f->widgets["frm_frame_i"]->widgets["txt_dur"])->text = f2s(cur_anim->frame_instances[cur_frame_instance_nr].duration);
    }
}


/* ----------------------------------------------------------------------------
 * Loads the hitbox's data onto the gui.
 */
void animation_editor::gui_load_hitbox() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_hitboxes"];
    
    ((lafi::label*) f->widgets["lbl_nr"])->text =
        (
            anims.hitboxes.empty() ?
            "--/0" :
            (string) (i2s(cur_hitbox_nr + 1) + "/" + i2s(anims.hitboxes.size()))
        );
        
    if(anims.hitboxes.empty()) {
        hide_widget(f->widgets["frm_hitbox"]);
        return;
    }
    
    f = (lafi::frame*) f->widgets["frm_hitbox"];
    show_widget(f);
    
    ((lafi::textbox*) f->widgets["txt_name"])->text = anims.hitboxes[cur_hitbox_nr]->name;
}


/* ----------------------------------------------------------------------------
 * Loads the hitbox instance's data onto the gui.
 */
void animation_editor::gui_load_hitbox_instance() {
    lafi::widget* f = gui->widgets["frm_hitbox_is"]->widgets["frm_hitbox_i"];
    
    hitbox_instance* cur_hi = NULL;
    if(!cur_frame->hitbox_instances.empty()) cur_hi = &cur_frame->hitbox_instances[cur_hitbox_instance_nr];
    if(cur_hi) {
        ((lafi::label*) gui->widgets["frm_hitbox_is"]->widgets["lbl_name"])->text = cur_hi->hitbox_name;
        ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(cur_hi->x);
        ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(cur_hi->y);
        ((lafi::textbox*) f->widgets["txt_z"])->text = f2s(cur_hi->z);
        ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(cur_hi->height);
        ((lafi::textbox*) f->widgets["txt_r"])->text = f2s(cur_hi->radius);
    }
    
    open_hitbox_type(cur_hi ? cur_hi->type : 255);
    
    if(cur_hi) {
        show_widget(f);
        if(cur_hi->type == HITBOX_TYPE_NORMAL) {
            f = f->widgets["frm_normal"];
            ((lafi::textbox*) f->widgets["txt_mult"])->text = f2s(cur_hi->multiplier);
            if(cur_hi->can_pikmin_latch) ((lafi::checkbox*) f->widgets["chk_latch"])->check();
            else ((lafi::checkbox*) f->widgets["chk_latch"])->uncheck();
            ((lafi::textbox*) f->widgets["txt_hazards"])->text = cur_hi->hazards;
            
        } else if(cur_hi->type == HITBOX_TYPE_ATTACK) {
            f = f->widgets["frm_attack"];
            ((lafi::textbox*)      f->widgets["txt_mult"])->text = f2s(cur_hi->multiplier);
            ((lafi::textbox*)      f->widgets["txt_hazards"])->text = cur_hi->hazards;
            ((lafi::checkbox*)     f->widgets["chk_outward"])->set(cur_hi->knockback_outward);
            ((lafi::angle_picker*) f->widgets["ang_angle"])->set_angle_rads(cur_hi->knockback_angle);
            ((lafi::textbox*)      f->widgets["txt_knockback"])->text = f2s(cur_hi->knockback);
            
            if(cur_hi->knockback_outward) {
                disable_widget(f->widgets["ang_angle"]);
            } else {
                enable_widget(f->widgets["ang_angle"]);
            }
            
        }
    } else {
        hide_widget(f);
    }
}


/* ----------------------------------------------------------------------------
 * Loads the Pikmin top's data onto the gui.
 */
void animation_editor::gui_load_top() {
    lafi::widget* f = gui->widgets["frm_top"];
    
    lafi::checkbox* c = (lafi::checkbox*) f->widgets["chk_visible"];
    if(cur_frame->top_visible) c->check();
    else c->uncheck();
    
    ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(cur_frame->top_x);
    ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(cur_frame->top_y);
    ((lafi::textbox*) f->widgets["txt_w"])->text = f2s(cur_frame->top_w);
    ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(cur_frame->top_h);
    ((lafi::angle_picker*) f->widgets["ang_angle"])->set_angle_rads(cur_frame->top_angle);
}


/* ----------------------------------------------------------------------------
 * Saves the animation's data from the gui.
 */
void animation_editor::gui_save_animation() {
    if(!cur_anim) return;
    
    lafi::widget* f = gui->widgets["frm_anims"]->widgets["frm_anim"];
    
    cur_anim->loop_frame = s2i(((lafi::textbox*) f->widgets["txt_loop"])->text) - 1;
    if(cur_anim->loop_frame >= cur_anim->frame_instances.size()) cur_anim->loop_frame = 0;
    
    gui_save_frame_instance();
    gui_load_animation();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the frame's data from the gui.
 */
void animation_editor::gui_save_frame() {
    if(!cur_frame) return;
    
    lafi::widget* f = gui->widgets["frm_frames"]->widgets["frm_frame"];
    
    string new_file;
    int new_fx, new_fy, new_fw, new_fh;
    
    new_file =                 ((lafi::textbox*) f->widgets["txt_file"])->text;
    new_fx =               s2i(((lafi::textbox*) f->widgets["txt_filex"])->text);
    new_fy =               s2i(((lafi::textbox*) f->widgets["txt_filey"])->text);
    new_fw =               s2i(((lafi::textbox*) f->widgets["txt_filew"])->text);
    new_fh =               s2i(((lafi::textbox*) f->widgets["txt_fileh"])->text);
    cur_frame->game_w = s2f(((lafi::textbox*) f->widgets["txt_gamew"])->text);
    cur_frame->game_h = s2f(((lafi::textbox*) f->widgets["txt_gameh"])->text);
    cur_frame->offs_x = s2f(((lafi::textbox*) f->widgets["txt_offsx"])->text);
    cur_frame->offs_y = s2f(((lafi::textbox*) f->widgets["txt_offsy"])->text);
    
    //Automatically fill in the in-game width/height if it hasn't been set yet.
    if(cur_frame->game_w == 0.0f) cur_frame->game_w = new_fw;
    if(cur_frame->game_h == 0.0f) cur_frame->game_h = new_fh;
    
    if(cur_frame->file != new_file || cur_frame->file_x != new_fx || cur_frame->file_y != new_fy || cur_frame->file_w != new_fw || cur_frame->file_h != new_fh) {
        //Changed something image-wise. Recreate it.
        if(cur_frame->parent_bmp) bitmaps.detach(cur_frame->file);
        if(cur_frame->bitmap) al_destroy_bitmap(cur_frame->bitmap);
        cur_frame->bitmap = NULL;
        cur_frame->parent_bmp = bitmaps.get(new_file, NULL);
        if(cur_frame->parent_bmp) cur_frame->bitmap = al_create_sub_bitmap(cur_frame->parent_bmp, new_fx, new_fy, new_fw, new_fh);
        
        cur_frame->file = new_file;
        cur_frame->file_x = new_fx;
        cur_frame->file_y = new_fy;
        cur_frame->file_w = new_fw;
        cur_frame->file_h = new_fh;
    }
    
    last_file_used = new_file;
    
    gui_save_hitbox_instance();
    gui_load_frame();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the frame instance's data from the gui.
 */
void animation_editor::gui_save_frame_instance() {
    bool valid = cur_frame_instance_nr != INVALID && cur_anim;
    if(!valid) return;
    
    lafi::widget* f = gui->widgets["frm_anims"]->widgets["frm_anim"];
    
    frame_instance* fi = &cur_anim->frame_instances[cur_frame_instance_nr];
    fi->duration = s2f(((lafi::textbox*) f->widgets["frm_frame_i"]->widgets["txt_dur"])->text);
    if(fi->duration < 0) fi->duration = 0;
    
    gui_load_frame_instance();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the hitbox' data from the gui.
 */
void animation_editor::gui_save_hitbox() {
    gui_load_hitbox();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the hitbox instance's data from the gui.
 */
void animation_editor::gui_save_hitbox_instance() {
    bool valid = cur_hitbox_instance_nr != INVALID && cur_frame;
    if(!valid) return;
    
    lafi::widget* f = gui->widgets["frm_hitbox_is"]->widgets["frm_hitbox_i"];
    
    hitbox_instance* hi = &cur_frame->hitbox_instances[cur_hitbox_instance_nr];
    
    hi->x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    hi->y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    hi->z = s2f(((lafi::textbox*) f->widgets["txt_z"])->text);
    
    hi->height = s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    hi->radius = s2f(((lafi::textbox*) f->widgets["txt_r"])->text);
    if(hi->radius <= 0) hi->radius = 16;
    
    hitbox_instance* cur_hi = &cur_frame->hitbox_instances[cur_hitbox_instance_nr];
    
    if(((lafi::radio_button*) f->widgets["rad_normal"])->selected) cur_hi->type = HITBOX_TYPE_NORMAL;
    else if(((lafi::radio_button*) f->widgets["rad_attack"])->selected) cur_hi->type = HITBOX_TYPE_ATTACK;
    else cur_hi->type = HITBOX_TYPE_DISABLED;
    
    if(cur_hi->type == HITBOX_TYPE_NORMAL) {
        cur_hi->multiplier = s2f(((lafi::textbox*) f->widgets["frm_normal"]->widgets["txt_mult"])->text);
        cur_hi->can_pikmin_latch = ((lafi::checkbox*) f->widgets["frm_normal"]->widgets["chk_latch"])->checked;
        cur_hi->hazards = ((lafi::textbox*) f->widgets["frm_normal"]->widgets["txt_hazards"])->text;
        
    } else if(cur_hi->type == HITBOX_TYPE_ATTACK) {
        cur_hi->multiplier = s2f(((lafi::textbox*) f->widgets["frm_attack"]->widgets["txt_mult"])->text);
        cur_hi->hazards = ((lafi::textbox*) f->widgets["frm_attack"]->widgets["txt_hazards"])->text;
        cur_hi->knockback_outward = ((lafi::checkbox*) f->widgets["frm_attack"]->widgets["chk_outward"])->checked;
        cur_hi->knockback_angle = ((lafi::angle_picker*) f->widgets["frm_attack"]->widgets["ang_angle"])->get_angle_rads();
        cur_hi->knockback = s2f(((lafi::textbox*) f->widgets["frm_attack"]->widgets["txt_knockback"])->text);
        
    }
    
    gui_load_hitbox_instance();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the Pikmin top's data from the gui.
 */
void animation_editor::gui_save_top() {
    lafi::widget* f = gui->widgets["frm_top"];
    
    cur_frame->top_visible = ((lafi::checkbox*) f->widgets["chk_visible"])->checked;
    cur_frame->top_x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_frame->top_y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_frame->top_w = s2f(((lafi::textbox*) f->widgets["txt_w"])->text);
    cur_frame->top_h = s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    cur_frame->top_angle = ((lafi::angle_picker*) f->widgets["ang_angle"])->get_angle_rads();
    
    gui_load_top();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Handles the controls and other events.
 */
void animation_editor::handle_controls(ALLEGRO_EVENT ev) {

    if(fade_mgr.is_fading()) return;
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor_x = ev.mouse.x / cam_zoom - cam_x - ((scr_w - 208) / 2 / cam_zoom);
        mouse_cursor_y = ev.mouse.y / cam_zoom - cam_y - (scr_h / 2 / cam_zoom);
        lafi::widget* wum = gui->get_widget_under_mouse(ev.mouse.x, ev.mouse.y); //Widget under mouse.
        ((lafi::label*) gui->widgets["lbl_status_bar"])->text = (wum ? wum->description : "(" + i2s(mouse_cursor_x) + "," + i2s(mouse_cursor_y) + ")");
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(holding_m2) {
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
        if(ev.mouse.button == 2) holding_m2 = true;
        if(ev.mouse.button == 3) {
            cam_zoom = 1;
            cam_x = cam_y = 0;
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if(ev.mouse.button == 2) holding_m2 = false;
    }
    
    frame* f = NULL;
    if(mode == EDITOR_MODE_ANIMATION) {
        if(cur_frame_instance_nr != INVALID) {
            string name = cur_anim->frame_instances[cur_frame_instance_nr].frame_name;
            size_t f_pos = anims.find_frame(name);
            if(f_pos != INVALID) f = anims.frames[f_pos];
        }
    } else if(mode == EDITOR_MODE_HITBOX_INSTANCES) {
        f = cur_frame;
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1 && mode == EDITOR_MODE_HITBOX_INSTANCES) {
        if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) {
            if(f) {
                for(size_t h = 0; h < f->hitbox_instances.size(); ++h) {
                
                    hitbox_instance* hi_ptr = &f->hitbox_instances[h];
                    dist d(mouse_cursor_x, mouse_cursor_y, hi_ptr->x, hi_ptr->y);
                    if(d <= hi_ptr->radius) {
                        gui_save_hitbox_instance();
                        cur_hitbox_instance_nr = h;
                        gui_load_hitbox_instance();
                        
                        grabbing_hitbox = h;
                        grabbing_hitbox_edge = (d > hi_ptr->radius - 5 / cam_zoom);
                        
                        //If the user grabbed the outermost 5 pixels, change radius. Else move hitbox.
                        if(grabbing_hitbox_edge) {
                            float anchor_angle = atan2(hi_ptr->y - mouse_cursor_y, hi_ptr->x - mouse_cursor_x);
                            //These variables will actually serve to store the anchor.
                            grabbing_hitbox_x = hi_ptr->x + cos(anchor_angle) * hi_ptr->radius;
                            grabbing_hitbox_y = hi_ptr->y + sin(anchor_angle) * hi_ptr->radius;
                        } else {
                            grabbing_hitbox_x = hi_ptr->x - mouse_cursor_x;
                            grabbing_hitbox_y = hi_ptr->y - mouse_cursor_y;
                        }
                        
                        made_changes = true;
                    }
                }
            }
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && ev.mouse.button == 1) {
        grabbing_hitbox = INVALID;
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        if(grabbing_hitbox != INVALID) {
            hitbox_instance* hi_ptr = &f->hitbox_instances[grabbing_hitbox];
            
            if(grabbing_hitbox_edge) {
                hi_ptr->radius =
                    dist(
                        mouse_cursor_x,
                        mouse_cursor_y,
                        grabbing_hitbox_x,
                        grabbing_hitbox_y
                    ).to_float() / 2;
                hi_ptr->x = (mouse_cursor_x + grabbing_hitbox_x) / 2;
                hi_ptr->y = (mouse_cursor_y + grabbing_hitbox_y) / 2;
                
            } else {
                hi_ptr->x = mouse_cursor_x + grabbing_hitbox_x;
                hi_ptr->y = mouse_cursor_y + grabbing_hitbox_y;
            }
            
            gui_load_hitbox_instance();
            made_changes = true;
        }
        
    }
    
    
    gui->handle_event(ev);
}


/* ----------------------------------------------------------------------------
 * Loads the animation editor.
 */
void animation_editor::load() {
    mode = EDITOR_MODE_MAIN;
    file_path.clear();
    
    fade_mgr.start_fade(true, nullptr);
    
    lafi::style* s = new lafi::style(al_map_rgb(192, 192, 208), al_map_rgb(0, 0, 32), al_map_rgb(96, 128, 160));
    gui = new lafi::gui(scr_w, scr_h, s);
    
    
    //Main frame.
    lafi::frame* frm_main = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add("lbl_file", new lafi::label(0, 0, 0, 0, "Choose a file:"), 100, 16);
    frm_main->easy_row();
    frm_main->easy_add("but_file", new lafi::button(0, 0, 0, 0), 100, 32);
    int y = frm_main->easy_row();
    
    lafi::frame* frm_object = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_main->add("frm_object", frm_object);
    frm_object->easy_row();
    frm_object->easy_add("but_anims",      new lafi::button(0, 0, 0, 0, "Edit animations"), 100, 32);
    frm_object->easy_row();
    frm_object->easy_add("but_frames",     new lafi::button(0, 0, 0, 0, "Edit frames"), 100, 32);
    frm_object->easy_row();
    frm_object->easy_add("but_hitboxes",   new lafi::button(0, 0, 0, 0, "Edit hitboxes"), 100, 32);
    frm_object->easy_row();
    frm_object->easy_add("lbl_n_anims",    new lafi::label(0, 0, 0, 0), 100, 12);
    frm_object->easy_row();
    frm_object->easy_add("lbl_n_frames",   new lafi::label(0, 0, 0, 0), 100, 12);
    frm_object->easy_row();
    frm_object->easy_add("lbl_n_hitboxes", new lafi::label(0, 0, 0, 0), 100, 12);
    frm_object->easy_row();
    
    
    //Animations frame.
    lafi::frame* frm_anims = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_anims);
    gui->add("frm_anims", frm_anims);
    
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
    frm_anim->easy_add("lin_1",    new lafi::line(  0, 0, 0, 0), 15, 12);
    frm_anim->easy_add("lbl_data", new lafi::label( 0, 0, 0, 0, "Animation data", ALLEGRO_ALIGN_CENTER), 70, 12);
    frm_anim->easy_add("lin_2",    new lafi::line(  0, 0, 0, 0), 15, 12);
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
    gui->add("frm_frames", frm_frames);
    
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
    frm_frame->easy_add("lin_1",         new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_frame->easy_add("lbl_f_data",    new lafi::label(0, 0, 0, 0, "Frame data", ALLEGRO_ALIGN_CENTER), 50, 12);
    frm_frame->easy_add("lin_2",         new lafi::line(  0, 0, 0, 0), 25, 12);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_file",      new lafi::label(0, 0, 0, 0, "File:"), 25, 16);
    frm_frame->easy_add("txt_file",      new lafi::textbox(0, 0, 0, 0), 75, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_filexy",    new lafi::label(0, 0, 0, 0, "File XY:"), 45, 16);
    frm_frame->easy_add("txt_filex",     new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_add("txt_filey",     new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_filewh",    new lafi::label(0, 0, 0, 0, "File WH:"), 45, 16);
    frm_frame->easy_add("txt_filew",     new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_add("txt_fileh",     new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_gamewh",    new lafi::label(0, 0, 0, 0, "Game WH:"), 45, 16);
    frm_frame->easy_add("txt_gamew",     new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_add("txt_gameh",     new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("lbl_offsxy",    new lafi::label(0, 0, 0, 0, "Offset XY:"), 45, 16);
    frm_frame->easy_add("txt_offsx",     new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_add("txt_offsy",     new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_frame->easy_row();
    frm_frame->easy_add("but_hitbox_is", new lafi::button(0, 0, 0, 0, "Edit hitboxes"), 100, 32);
    frm_frame->easy_row();
    frm_frame->easy_add("but_top",       new lafi::button(0, 0, 0, 0, "Edit Pikmin top"), 100, 32);
    frm_frame->easy_row();
    
    
    //Hitboxes instances frame.
    lafi::frame* frm_hitbox_is = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_hitbox_is);
    gui->add("frm_hitbox_is", frm_hitbox_is);
    
    frm_hitbox_is->easy_row();
    frm_hitbox_is->easy_add("but_back",   new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_hitbox_is->easy_row();
    frm_hitbox_is->easy_add("but_prev",   new lafi::button(0, 0, 0, 0, "<"), 20, 24);
    frm_hitbox_is->easy_add("but_next",   new lafi::button(0, 0, 0, 0, ">"), 20, 24);
    frm_hitbox_is->easy_row();
    frm_hitbox_is->easy_add("lbl_n",      new lafi::label( 0, 0, 0, 0, "Hitbox:"), 30, 24);
    frm_hitbox_is->easy_add("lbl_name",   new lafi::label( 0, 0, 0, 0), 70, 24);
    y = frm_hitbox_is->easy_row();
    
    lafi::frame* frm_hitbox_i = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_hitbox_is->add("frm_hitbox_i", frm_hitbox_i);
    frm_hitbox_i->easy_row();
    frm_hitbox_i->easy_add("lbl_xy",       new lafi::label(0, 0, 0, 0, "X, Y:"), 45, 16);
    frm_hitbox_i->easy_add("txt_x",        new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_hitbox_i->easy_add("txt_y",        new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_hitbox_i->easy_row();
    frm_hitbox_i->easy_add("lbl_zh",       new lafi::label(0, 0, 0, 0, "Z, Height:"), 45, 16);
    frm_hitbox_i->easy_add("txt_z",        new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_hitbox_i->easy_add("txt_h",        new lafi::textbox(0, 0, 0, 0), 27.5, 16);
    frm_hitbox_i->easy_row();
    frm_hitbox_i->easy_add("lbl_r",        new lafi::label(0, 0, 0, 0, "Radius:"), 45, 16);
    frm_hitbox_i->easy_add("txt_r",        new lafi::textbox(0, 0, 0, 0), 55, 16);
    frm_hitbox_i->easy_row();
    
    frm_hitbox_i->easy_add("lbl_h_type",   new lafi::label(0, 0, 0, 0, "Hitbox type:"), 100, 12);
    frm_hitbox_i->easy_row();
    frm_hitbox_i->easy_add("rad_normal",   new lafi::radio_button(0, 0, 0, 0, "Normal"), 50, 16);
    frm_hitbox_i->easy_add("rad_attack",   new lafi::radio_button(0, 0, 0, 0, "Attack"), 50, 16);
    frm_hitbox_i->easy_row();
    frm_hitbox_i->easy_add("rad_disabled", new lafi::radio_button(0, 0, 0, 0, "Disabled"), 100, 16);
    y += frm_hitbox_i->easy_row();
    
    lafi::frame* frm_normal = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    hide_widget(frm_normal);
    frm_hitbox_i->add("frm_normal", frm_normal);
    
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
    frm_hitbox_i->add("frm_attack", frm_attack);
    
    frm_attack->easy_row();
    frm_attack->easy_add("lbl_mult",      new lafi::label(0, 0, 0, 0, "Attack mult.:"), 60, 16);
    frm_attack->easy_add("txt_mult",      new lafi::textbox(0, 0, 0, 0), 40, 16);
    frm_attack->easy_row();
    frm_attack->easy_add("lbl_hazards",   new lafi::label(0, 0, 0, 0, "Hazards:"), 100, 12);
    frm_attack->easy_row();
    frm_attack->easy_add("txt_hazards",   new lafi::textbox(0, 0, 0, 0), 100, 16);
    frm_attack->easy_row();
    frm_attack->easy_add("chk_outward",   new lafi::checkbox(0, 0, 0, 0, "Outward knockback"), 100, 16);
    frm_attack->easy_row();
    frm_attack->easy_add("lbl_angle",     new lafi::label(0, 0, 0, 0, "Angle:"), 60, 16);
    frm_attack->easy_add("ang_angle",     new lafi::angle_picker(0, 0, 0, 0), 40, 24);
    frm_attack->easy_row();
    frm_attack->easy_add("lbl_knockback", new lafi::label(0, 0, 0, 0, "Knockback:"), 60, 16);
    frm_attack->easy_add("txt_knockback", new lafi::textbox(0, 0, 0, 0), 40, 16);
    frm_attack->easy_row();
    
    
    //Hitboxes frame.
    lafi::frame* frm_hitboxes = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_hitboxes);
    gui->add("frm_hitboxes", frm_hitboxes);
    
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("but_back",   new lafi::button(0, 0, 0, 0, "Back"), 50, 16);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("lbl_inst1",  new lafi::label(0, 0, 0, 0, "The lower a hitbox's"), 100, 12);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("lbl_inst2",  new lafi::label(0, 0, 0, 0, "number, the more"), 100, 12);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("lbl_inst3",  new lafi::label(0, 0, 0, 0, "priority it has when"), 100, 12);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("lbl_inst4",  new lafi::label(0, 0, 0, 0, "checking collisions."), 100, 12);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("dummy",      new lafi::dummy(), 100, 16);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("txt_add",    new lafi::textbox(0, 0, 0, 0, ""), 80, 16);
    frm_hitboxes->easy_add("but_add",    new lafi::button(0, 0, 0, 0, "+"), 20, 24);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("but_prev",   new lafi::button(0, 0, 0, 0, "<"), 20, 24);
    frm_hitboxes->easy_add("but_next",   new lafi::button(0, 0, 0, 0, ">"), 20, 24);
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add("lbl_n",      new lafi::label(0, 0, 0, 0, "Hitbox nr:"), 50, 16);
    frm_hitboxes->easy_add("lbl_nr",     new lafi::label(0, 0, 0, 0, ""), 50, 16);
    y = frm_hitboxes->easy_row();
    
    lafi::frame* frm_hitbox = new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_hitboxes->add("frm_hitbox", frm_hitbox);
    frm_hitbox->easy_row();
    frm_hitbox->easy_add("lbl_na",     new lafi::label(0, 0, 0, 0, "Name:"), 30, 16);
    frm_hitbox->easy_add("txt_name",   new lafi::textbox(0, 0, 0, 0, ""), 70, 16);
    frm_hitbox->easy_row();
    frm_hitbox->easy_add("but_left",   new lafi::button(0, 0, 0, 0, "<="), 20, 24);
    frm_hitbox->easy_add("but_right",  new lafi::button(0, 0, 0, 0, "=>"), 20, 24);
    frm_hitbox->easy_add("but_rem",    new lafi::button(0, 0, 0, 0, "-"), 20, 24);
    frm_hitbox->easy_row();
    
    
    //Changes warning.
    lafi::frame* frm_changes = new lafi::frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    gui->add("frm_changes", frm_changes);
    hide_widget(frm_changes);
    frm_changes->easy_row();
    frm_changes->easy_add("lbl_text1", new lafi::label(0, 0, 0, 0, "Warning: you have", ALLEGRO_ALIGN_LEFT), 80, 8);
    frm_changes->easy_row();
    frm_changes->easy_add("lbl_text2", new lafi::label(0, 0, 0, 0, "unsaved changes!", ALLEGRO_ALIGN_LEFT), 80, 8);
    frm_changes->easy_row();
    frm_changes->add("but_ok", new lafi::button(scr_w - 40, scr_h - 40, scr_w - 8, scr_h - 8, "Ok"));
    
    
    //Picker frame.
    lafi::frame* frm_picker = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_picker);
    gui->add("frm_picker", frm_picker);
    
    frm_picker->add("but_back", new lafi::button(     scr_w - 200, 8, scr_w - 104, 24, "Back"));
    frm_picker->add("txt_new", new lafi::textbox(     scr_w - 200, 40, scr_w - 48, 56));
    frm_picker->add("but_new", new lafi::button(      scr_w - 40,  32, scr_w - 8,  64, "+"));
    frm_picker->add("frm_list", new lafi::frame(      scr_w - 200, 72, scr_w - 32, scr_h - 56));
    frm_picker->add("bar_scroll", new lafi::scrollbar(scr_w - 24,  72, scr_w - 8,  scr_h - 56));
    
    
    //Pikmin top frame.
    lafi::frame* frm_top = new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_top);
    gui->add("frm_top", frm_top);
    
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
    gui->add("frm_bottom", frm_bottom);
    frm_bottom->easy_row();
    frm_bottom->easy_add("but_toggle_hitboxes", new lafi::button(0, 0, 0, 0, "Hit"), 25, 32);
    frm_bottom->easy_add("but_load", new lafi::button(           0, 0, 0, 0, "Load"), 25, 32);
    frm_bottom->easy_add("but_save", new lafi::button(           0, 0, 0, 0, "Save"), 25, 32);
    frm_bottom->easy_add("but_quit", new lafi::button(           0, 0, 0, 0, "X"), 25, 32);
    frm_bottom->easy_row();
    
    
    //Properties -- main.
    frm_main->widgets["but_file"]->left_mouse_click_handler = [this, frm_main] (lafi::widget*, int, int) {
        al_show_native_file_dialog(display, file_dialog);
        
        setlocale(LC_ALL, "C"); //Reset the locale, which gets set by Allegro's native dialogs...and breaks s2f().
        
        if(al_get_native_file_dialog_count(file_dialog) == 0) return;
        
        file_path = al_get_native_file_dialog_path(file_dialog, 0);
        file_path = replace_all(file_path, "\\", "/");
        load_animation_pool();
        
        vector<string> file_path_parts = split(file_path, "/");
        string name_to_show = file_path;
        if(file_path.size() > 20) {
            //Show the last 20 characters.
            name_to_show = "..." + file_path.substr(file_path.size() - 20, 20);
        }
        ((lafi::button*) frm_main->widgets["but_file"])->text = name_to_show;
        
        //Top bitmap.
        for(unsigned char t = 0; t < 3; ++t) {
            if(top_bmp[t] && top_bmp[t] != bmp_error) {
                al_destroy_bitmap(top_bmp[t]);
                top_bmp[t] = NULL;
            }
        }
        
        if(file_path.find(PIKMIN_FOLDER) != string::npos) {
            is_pikmin = true;
            data_node data = data_node(PIKMIN_FOLDER + "/" + file_path_parts[file_path_parts.size() - 2] + "/Data.txt");
            top_bmp[0] = load_bmp(data.get_child_by_name("top_leaf")->value, &data);
            top_bmp[1] = load_bmp(data.get_child_by_name("top_bud")->value, &data);
            top_bmp[2] = load_bmp(data.get_child_by_name("top_flower")->value, &data);
        } else {
            is_pikmin = false;
        }
        
        update_stats();
    };
    frm_main->widgets["but_file"]->description = "Pick a file to load or create.";
    frm_main->widgets["frm_object"]->widgets["but_anims"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        cur_hitbox_instance_nr = INVALID;
        if(cur_anim) if(cur_anim->frame_instances.size()) cur_frame_instance_nr = 0;
        mode = EDITOR_MODE_ANIMATION;
        hide_widget(this->gui->widgets["frm_main"]); show_widget(this->gui->widgets["frm_anims"]);
        gui_load_animation();
    };
    frm_main->widgets["frm_object"]->widgets["but_anims"]->description = "Change the way the animations look like.";
    frm_main->widgets["frm_object"]->widgets["but_frames"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_FRAME;
        cur_hitbox_instance_nr = INVALID;
        hide_widget(this->gui->widgets["frm_main"]); show_widget(this->gui->widgets["frm_frames"]);
        gui_load_frame();
    };
    frm_main->widgets["frm_object"]->widgets["but_frames"]->description = "Change how each individual frame looks like.";
    frm_main->widgets["frm_object"]->widgets["but_hitboxes"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_HITBOX;
        hide_widget(this->gui->widgets["frm_main"]);
        show_widget(this->gui->widgets["frm_hitboxes"]);
        cur_hitbox_nr = 0;
        gui_load_hitbox();
    };
    frm_main->widgets["frm_object"]->widgets["but_hitboxes"]->description = "Change what hitboxes exist, and their order.";
    
    
    //Properties -- animations.
    auto lambda_gui_save_animation = [this] (lafi::widget*) { gui_save_animation(); };
    auto lambda_gui_save_frame_instance = [this] (lafi::widget*) { gui_save_frame_instance(); };
    
    frm_anims->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        anim_playing = false;
        hide_widget(this->gui->widgets["frm_anims"]); show_widget(this->gui->widgets["frm_main"]);
        update_stats();
    };
    frm_anims->widgets["but_back"]->description = "Go back to the main menu.";
    frm_anims->widgets["but_del_anim"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(!cur_anim) return;
        anims.animations.erase(anims.animations.begin() + anims.find_animation(cur_anim->name));
        anim_playing = false;
        cur_anim = NULL;
        cur_frame_instance_nr = INVALID;
        cur_hitbox_instance_nr = INVALID;
        gui_load_animation();
        made_changes = true;
    };
    frm_anims->widgets["but_del_anim"]->description = "Delete the current animation.";
    frm_anims->widgets["but_anim"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        anim_playing = false;
        open_picker(ANIMATION_EDITOR_PICKER_ANIMATION, true);
    };
    frm_anims->widgets["but_anim"]->description = "Pick an animation to edit.";
    frm_anims->widgets["frm_anim"]->widgets["but_play"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(cur_anim->frame_instances.size() < 2) {
            anim_playing = false;
        } else {
            anim_playing = !anim_playing;
            if(!cur_anim->frame_instances.empty() && cur_frame_instance_nr == INVALID) cur_frame_instance_nr = 0;
            cur_frame_time = 0;
        }
    };
    frm_anims->widgets["frm_anim"]->widgets["but_play"]->description = "Play or pause the animation.";
    frm_anims->widgets["frm_anim"]->widgets["but_prev"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        anim_playing = false;
        if(!cur_anim->frame_instances.empty()) {
            if(cur_frame_instance_nr == INVALID) cur_frame_instance_nr = 0;
            else if(cur_frame_instance_nr == 0) cur_frame_instance_nr = cur_anim->frame_instances.size() - 1;
            else cur_frame_instance_nr--;
        }
        gui_load_frame_instance();
    };
    frm_anims->widgets["frm_anim"]->widgets["but_prev"]->description = "Previous frame.";
    frm_anims->widgets["frm_anim"]->widgets["but_next"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        anim_playing = false;
        if(!cur_anim->frame_instances.empty()) {
            if(cur_frame_instance_nr == cur_anim->frame_instances.size() - 1 || cur_frame_instance_nr == INVALID) cur_frame_instance_nr = 0;
            else cur_frame_instance_nr++;
        }
        gui_load_frame_instance();
    };
    frm_anims->widgets["frm_anim"]->widgets["but_next"]->description = "Next frame.";
    frm_anims->widgets["frm_anim"]->widgets["but_add"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        anim_playing = false;
        if(cur_frame_instance_nr != INVALID) {
            cur_frame_instance_nr++;
            cur_anim->frame_instances.insert(
                cur_anim->frame_instances.begin() + cur_frame_instance_nr,
                frame_instance(cur_anim->frame_instances[cur_frame_instance_nr - 1])
            );
        } else {
            cur_anim->frame_instances.push_back(frame_instance());
            cur_frame_instance_nr = 0;
        }
        gui_load_frame_instance();
        made_changes = true;
    };
    frm_anims->widgets["frm_anim"]->widgets["but_add"]->description = "Add a new frame after the current one (via copy).";
    frm_anims->widgets["frm_anim"]->widgets["but_rem"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        anim_playing = false;
        if(cur_frame_instance_nr != INVALID) {
            cur_anim->frame_instances.erase(cur_anim->frame_instances.begin() + cur_frame_instance_nr);
            if(cur_anim->frame_instances.empty()) cur_frame_instance_nr = INVALID;
            else if(cur_frame_instance_nr >= cur_anim->frame_instances.size()) cur_frame_instance_nr = cur_anim->frame_instances.size() - 1;
        }
        gui_load_frame_instance();
        made_changes = true;
    };
    frm_anims->widgets["frm_anim"]->widgets["but_rem"]->description = "Remove the current frame.";
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["but_frame"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        anim_playing = false;
        open_picker(ANIMATION_EDITOR_PICKER_FRAME_INSTANCE, false);
    };
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["but_frame"]->description = "Pick the frame to use here.";
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["txt_dur"]->lose_focus_handler = lambda_gui_save_frame_instance;
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["txt_dur"]->mouse_down_handler = [this] (lafi::widget*, int, int, int) {
        anim_playing = false;
    };
    frm_anims->widgets["frm_anim"]->widgets["frm_frame_i"]->widgets["txt_dur"]->description = "How long this frame lasts for, in seconds.";
    frm_anims->widgets["frm_anim"]->widgets["txt_loop"]->lose_focus_handler = lambda_gui_save_animation;
    frm_anims->widgets["frm_anim"]->widgets["txt_loop"]->description = "The animation loops back to this frame when it ends.";
    frm_anims->register_accelerator(ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL, frm_anims->widgets["frm_anim"]->widgets["but_next"]);
    frm_anims->register_accelerator(ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_SHIFT, frm_anims->widgets["frm_anim"]->widgets["but_prev"]);
    
    
    //Properties -- frames.
    auto lambda_gui_save_frame = [this] (lafi::widget*) { gui_save_frame(); };
    
    frm_frames->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        hide_widget(this->gui->widgets["frm_frames"]); show_widget(this->gui->widgets["frm_main"]);
        update_stats();
    };
    frm_frames->widgets["but_back"]->description = "Go back to the main menu.";
    frm_frames->widgets["but_del_frame"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(!cur_frame) return;
        anims.frames.erase(anims.frames.begin() + anims.find_frame(cur_frame->name));
        cur_frame = NULL;
        cur_hitbox_instance_nr = INVALID;
        gui_load_frame();
        made_changes = true;
    };
    frm_frames->widgets["but_del_frame"]->description = "Delete the current frame.";
    frm_frames->widgets["but_frame"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_FRAME, true);
    };
    frm_frames->widgets["but_frame"]->description = "Pick a frame to edit.";
    frm_frames->widgets["frm_frame"]->widgets["but_hitbox_is"]->description = "Edit this frame's hitboxes.";
    frm_frames->widgets["frm_frame"]->widgets["but_hitbox_is"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_hitbox_is"]);
        hide_widget(this->gui->widgets["frm_frames"]);
        mode = EDITOR_MODE_HITBOX_INSTANCES;
        cur_hitbox_instance_nr = 0;
        gui_load_hitbox_instance();
    };
    frm_frames->widgets["frm_frame"]->widgets["but_top"]->description = "Edit the Pikmin's top (maturity) for this frame.";
    frm_frames->widgets["frm_frame"]->widgets["but_top"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_top"]);
        hide_widget(this->gui->widgets["frm_frames"]);
        mode = EDITOR_MODE_TOP;
        gui_load_top();
    };
    
    frm_frames->widgets["frm_frame"]->widgets["txt_file"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_file"]->description = "Name (+extension) of the file with the sprite.";
    frm_frames->widgets["frm_frame"]->widgets["txt_filex"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_filex"]->description = "X of the top-left corner of the sprite.";
    frm_frames->widgets["frm_frame"]->widgets["txt_filey"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_filey"]->description = "Y of the top-left corner of the sprite.";
    frm_frames->widgets["frm_frame"]->widgets["txt_filew"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_filew"]->description = "Width of the sprite, in the file.";
    frm_frames->widgets["frm_frame"]->widgets["txt_fileh"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_fileh"]->description = "Height of the sprite, in the file.";
    frm_frames->widgets["frm_frame"]->widgets["txt_gamew"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_gamew"]->description = "In-game sprite width.";
    frm_frames->widgets["frm_frame"]->widgets["txt_gameh"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_gameh"]->description = "In-game sprite height.";
    frm_frames->widgets["frm_frame"]->widgets["txt_offsx"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_offsx"]->description = "In-game, offset by this much, horizontally.";
    frm_frames->widgets["frm_frame"]->widgets["txt_offsy"]->lose_focus_handler = lambda_gui_save_frame;
    frm_frames->widgets["frm_frame"]->widgets["txt_offsy"]->description = "In-game, offset by this much, vertically.";
    
    
    //Properties -- hitbox instances.
    auto lambda_gui_save_hitbox_instance = [this] (lafi::widget*) { gui_save_hitbox_instance(); };
    auto lambda_gui_save_hitbox_instance_click = [this] (lafi::widget*, int, int) { gui_save_hitbox_instance(); };
    
    frm_hitbox_is->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_FRAME;
        hide_widget(this->gui->widgets["frm_hitbox_is"]);
        show_widget(this->gui->widgets["frm_frames"]);
        cur_hitbox_instance_nr = INVALID;
        update_stats();
    };
    frm_hitbox_is->widgets["but_prev"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(cur_frame->hitbox_instances.size()) {
            if(cur_hitbox_instance_nr == INVALID) cur_hitbox_instance_nr = 0;
            else if(cur_hitbox_instance_nr == 0) cur_hitbox_instance_nr = cur_frame->hitbox_instances.size() - 1;
            else cur_hitbox_instance_nr--;
        }
        gui_load_hitbox_instance();
    };
    frm_hitbox_is->widgets["but_next"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(cur_frame->hitbox_instances.size()) {
            if(cur_hitbox_instance_nr == INVALID) cur_hitbox_instance_nr = 0;
            cur_hitbox_instance_nr = (cur_hitbox_instance_nr + 1) % cur_frame->hitbox_instances.size();
        }
        gui_load_hitbox_instance();
    };
    frm_hitbox_i->register_accelerator(ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL, frm_hitbox_is->widgets["but_next"]);
    frm_hitbox_i->register_accelerator(ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_SHIFT, frm_hitbox_is->widgets["but_prev"]);
    
    frm_hitbox_is->widgets["but_back"]->description = "Go back to the frame editor.";
    frm_hitbox_is->widgets["but_prev"]->description = "Previous hitbox.";
    frm_hitbox_is->widgets["but_next"]->description = "Next hitbox.";
    frm_hitbox_i->widgets["txt_x"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_hitbox_i->widgets["txt_x"]->description = "X of the hitbox's center.";
    frm_hitbox_i->widgets["txt_y"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_hitbox_i->widgets["txt_y"]->description = "Y of the hitbox's center.";
    frm_hitbox_i->widgets["txt_z"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_hitbox_i->widgets["txt_z"]->description = "Altitude of the hitbox's bottom.";
    frm_hitbox_i->widgets["txt_h"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_hitbox_i->widgets["txt_h"]->description = "Hitbox's height.";
    frm_hitbox_i->widgets["txt_r"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_hitbox_i->widgets["txt_r"]->description = "Hitbox's radius.";
    frm_hitbox_i->widgets["rad_normal"]->left_mouse_click_handler = lambda_gui_save_hitbox_instance_click;
    frm_hitbox_i->widgets["rad_attack"]->left_mouse_click_handler = lambda_gui_save_hitbox_instance_click;
    frm_hitbox_i->widgets["rad_disabled"]->left_mouse_click_handler = lambda_gui_save_hitbox_instance_click;
    frm_hitbox_i->widgets["rad_normal"]->description = "Normal hitbox, one that can be damaged.";
    frm_hitbox_i->widgets["rad_attack"]->description = "Attack hitbox, one that damages opponents.";
    frm_hitbox_i->widgets["rad_disabled"]->description = "This hitbox will be non-existent.";
    frm_normal->widgets["txt_mult"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_normal->widgets["chk_latch"]->left_mouse_click_handler = lambda_gui_save_hitbox_instance_click;
    frm_normal->widgets["txt_hazards"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_attack->widgets["txt_mult"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_attack->widgets["txt_hazards"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_attack->widgets["chk_outward"]->left_mouse_click_handler = lambda_gui_save_hitbox_instance_click;
    frm_attack->widgets["ang_angle"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_attack->widgets["txt_knockback"]->lose_focus_handler = lambda_gui_save_hitbox_instance;
    frm_normal->widgets["txt_mult"]->description = "Defense multiplier. 0 = invulnerable.";
    frm_normal->widgets["chk_latch"]->description = "Can the Pikmin latch on to this hitbox?";
    frm_normal->widgets["txt_hazards"]->description = "List of hazards, comma separated.";
    frm_attack->widgets["txt_mult"]->description = "Attack multiplier.";
    frm_attack->widgets["txt_hazards"]->description = "List of hazards, comma separated.";
    frm_attack->widgets["chk_outward"]->description = "Makes Pikmin be knocked away from the center.";
    frm_attack->widgets["ang_angle"]->description = "Angle the Pikmin are knocked towards.";
    frm_attack->widgets["txt_knockback"]->description = "Knockback strength.";
    
    
    //Properties -- hitboxes.
    auto lambda_gui_save_hitbox = [this] (lafi::widget*) { gui_save_hitbox(); };
    auto lambda_gui_save_hitbox_click = [this] (lafi::widget*, int, int) { gui_save_hitbox(); };
    frm_hitboxes->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        hide_widget(this->gui->widgets["frm_hitboxes"]);
        show_widget(this->gui->widgets["frm_main"]);
        update_stats();
    };
    frm_hitboxes->widgets["but_prev"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(anims.hitboxes.empty()) return;
        if(cur_hitbox_nr == 0) cur_hitbox_nr = anims.hitboxes.size() - 1;
        else cur_hitbox_nr--;
        gui_load_hitbox();
    };
    frm_hitboxes->widgets["but_next"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(anims.hitboxes.empty()) return;
        cur_hitbox_nr = (cur_hitbox_nr + 1) % anims.hitboxes.size();
        gui_load_hitbox();
    };
    frm_hitboxes->widgets["but_add"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        string name = ((lafi::textbox*) this->gui->widgets["frm_hitboxes"]->widgets["txt_add"])->text;
        if(name.empty()) return;
        for(size_t h = 0; h < anims.hitboxes.size(); ++h) {
            if(anims.hitboxes[h]->name == name) {
                cur_hitbox_nr = h;
                gui_load_hitbox();
                return;
            }
        }
        anims.hitboxes.insert(
            anims.hitboxes.begin() + cur_hitbox_nr + (anims.hitboxes.empty() ? 0 : 1),
            new hitbox(name)
        );
        if(anims.hitboxes.size() == 1) cur_hitbox_nr = 0;
        else cur_hitbox_nr++;
        update_hitboxes();
        gui_load_hitbox();
        made_changes = true;
    };
    frm_hitbox->widgets["txt_name"]->lose_focus_handler = [this] (lafi::widget * t) {
        string new_name = ((lafi::textbox*) t)->text;
        if(new_name.empty()) {
            gui_load_hitbox();
            return;
        }
        for(size_t h = 0; h < anims.hitboxes.size(); ++h) {
            if(h == cur_hitbox_nr) continue;
            if(anims.hitboxes[h]->name == new_name) {
                gui_load_hitbox();
                return;
            }
        }
        anims.hitboxes[cur_hitbox_nr]->name = new_name;
        update_hitboxes();
        gui_load_hitbox();
        made_changes = true;
    };
    frm_hitbox->widgets["but_left"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(anims.hitboxes.size() < 2) return;
        size_t prev_nr = (cur_hitbox_nr == 0) ? anims.hitboxes.size() - 1 : cur_hitbox_nr - 1;
        hitbox* cur_h = anims.hitboxes[cur_hitbox_nr];
        anims.hitboxes.erase(anims.hitboxes.begin() + cur_hitbox_nr);
        anims.hitboxes.insert(anims.hitboxes.begin() + prev_nr, cur_h);
        cur_hitbox_nr = prev_nr;
        update_hitboxes();
        gui_load_hitbox();
        made_changes = true;
    };
    frm_hitbox->widgets["but_right"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(anims.hitboxes.size() < 2) return;
        size_t next_nr = (cur_hitbox_nr + 1) % anims.hitboxes.size();
        hitbox* cur_h = anims.hitboxes[cur_hitbox_nr];
        anims.hitboxes.erase(anims.hitboxes.begin() + cur_hitbox_nr);
        anims.hitboxes.insert(anims.hitboxes.begin() + next_nr, cur_h);
        cur_hitbox_nr = next_nr;
        update_hitboxes();
        gui_load_hitbox();
        made_changes = true;
    };
    frm_hitbox->widgets["but_rem"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        delete anims.hitboxes[cur_hitbox_nr];
        anims.hitboxes.erase(anims.hitboxes.begin() + cur_hitbox_nr);
        if(cur_hitbox_nr > 0) cur_hitbox_nr--;
        update_hitboxes();
        gui_load_hitbox();
        made_changes = true;
    };
    frm_hitboxes->widgets["but_back"]->description = "Go back to the main menu.";
    frm_hitboxes->widgets["but_prev"]->description = "Previous hitbox.";
    frm_hitboxes->widgets["but_next"]->description = "Next hitbox.";
    frm_hitboxes->widgets["txt_add"]->description  = "Name of the hitbox you want to create.";
    frm_hitboxes->widgets["but_add"]->description  = "Create a new hitbox (after the current one).";
    frm_hitbox->widgets["txt_name"]->description   = "Name of this hitbox.";
    frm_hitbox->widgets["but_left"]->description   = "Move this hitbox to the left on the list.";
    frm_hitbox->widgets["but_right"]->description  = "Move this hitbox to the right on the list.";
    frm_hitbox->widgets["but_rem"]->description    = "Delete this hitbox.";
    
    
    //Properties -- picker.
    frm_picker->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        ((lafi::textbox*) this->gui->widgets["frm_picker"]->widgets["txt_new"])->text.clear();
        
        hide_widget(this->gui->widgets["frm_picker"]);
        show_widget(this->gui->widgets["frm_bottom"]);
        if(mode == EDITOR_MODE_MAIN) {
            show_widget(this->gui->widgets["frm_main"]);
        } else if(mode == EDITOR_MODE_ANIMATION) {
            show_widget(this->gui->widgets["frm_anims"]);
        } else if(mode == EDITOR_MODE_FRAME) {
            show_widget(this->gui->widgets["frm_frames"]);
        }
    };
    frm_picker->widgets["but_back"]->description = "Cancel.";
    ((lafi::textbox*) frm_picker->widgets["txt_new"])->enter_key_widget = frm_picker->widgets["but_new"];
    frm_picker->widgets["but_new"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        string name = ((lafi::textbox*) this->gui->widgets["frm_picker"]->widgets["txt_new"])->text;
        if(name.empty()) return;
        
        if(mode == EDITOR_MODE_ANIMATION) {
            if(anims.find_animation(name) != INVALID) return;
            anims.animations.push_back(new animation(name));
            pick(name, ANIMATION_EDITOR_PICKER_ANIMATION);
            
        } else if(mode == EDITOR_MODE_FRAME) {
            if(anims.find_frame(name) != INVALID) return;
            anims.frames.push_back(new frame(name));
            anims.frames.back()->create_hitbox_instances(&anims);
            pick(name, ANIMATION_EDITOR_PICKER_FRAME);
            
        }
        
        made_changes = true;
        
        ((lafi::textbox*) this->gui->widgets["frm_picker"]->widgets["txt_new"])->text.clear();
    };
    frm_picker->widgets["but_new"]->description = "Create a new one with the name on the textbox.";
    frm_picker->widgets["frm_list"]->mouse_wheel_handler = [this] (lafi::widget*, int dy, int) {
        lafi::scrollbar* s = (lafi::scrollbar*) this->gui->widgets["frm_picker"]->widgets["bar_scroll"];
        if(s->widgets.find("but_bar") != s->widgets.end()) {
            s->move_button(0, (s->widgets["but_bar"]->y1 + s->widgets["but_bar"]->y2) / 2 - 30 * dy);
        }
    };
    
    
    //Properties -- Pikmin top.
    auto lambda_save_top = [this] (lafi::widget*) { gui_save_top(); };
    auto lambda_save_top_click = [this] (lafi::widget*, int, int) { gui_save_top(); };
    frm_top->widgets["but_back"]->description = "Go back.";
    frm_top->widgets["but_back"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_frames"]);
        hide_widget(this->gui->widgets["frm_top"]);
        mode = EDITOR_MODE_FRAME;
    };
    frm_top->widgets["chk_visible"]->description = "Is the top visible in this frame?";
    frm_top->widgets["chk_visible"]->left_mouse_click_handler = lambda_save_top_click;
    frm_top->widgets["txt_x"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["txt_y"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["txt_w"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["txt_h"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["ang_angle"]->lose_focus_handler = lambda_save_top;
    frm_top->widgets["but_maturity"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        maturity = (maturity + 1) % 3;
    };
    frm_top->widgets["but_maturity"]->description = "View a different maturity top.";
    
    
    //Properties -- bottom bar.
    frm_bottom->widgets["but_toggle_hitboxes"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        hitboxes_visible = !hitboxes_visible;
    };
    frm_bottom->widgets["but_toggle_hitboxes"]->description = "Toggle hitbox and center-point grid visibility.";
    frm_bottom->widgets["but_load"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            load_animation_pool();
            hide_widget(this->gui->widgets["frm_anims"]);
            hide_widget(this->gui->widgets["frm_frames"]);
            hide_widget(this->gui->widgets["frm_hitboxes"]);
            show_widget(this->gui->widgets["frm_main"]);
            mode = EDITOR_MODE_MAIN;
            update_stats();
        }
    };
    frm_bottom->widgets["but_load"]->description = "Load the object from the text file.";
    frm_bottom->widgets["but_save"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        save_animation_pool();
    };
    frm_bottom->widgets["but_save"]->description = "Save the object to the text file.";
    frm_bottom->widgets["but_quit"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            leave();
        }
    };
    frm_bottom->widgets["but_quit"]->description = "Quit the animation editor.";
    
    lafi::label* gui_status_bar = new lafi::label(0, scr_h - 16, scr_w - 208, scr_h);
    gui->add("lbl_status_bar", gui_status_bar);
    
    
    //Properties -- changes warning.
    frm_changes->widgets["but_ok"]->left_mouse_click_handler = [this] (lafi::widget*, int, int) { close_changes_warning(); };
    
    
    //File dialog.
    file_dialog =
        al_create_native_file_dialog(
            NULL,
            "Please choose an animation text file to load or create.",
            "*.txt",
            0
        );
        
        
    update_stats();
    disable_widget(frm_bottom->widgets["but_load"]);
    disable_widget(frm_bottom->widgets["but_save"]);
}


/* ----------------------------------------------------------------------------
 * Loads the animation pool for the current object.
 */
void animation_editor::load_animation_pool() {
    anims.destroy();
    
    data_node file = data_node(file_path);
    if(!file.file_was_opened) {
        file.save_file(file_path, true);
    }
    anims = load_animation_pool_from_file(&file);
    
    anim_playing = false;
    cur_anim = NULL;
    cur_frame = NULL;
    cur_frame_instance_nr = INVALID;
    cur_hitbox_instance_nr = INVALID;
    if(!anims.animations.empty()) {
        cur_anim = anims.animations[0];
        if(cur_anim->frame_instances.size()) cur_frame_instance_nr = 0;
    }
    if(!anims.frames.empty()) {
        cur_frame = anims.frames[0];
        if(cur_frame->hitbox_instances.size()) cur_hitbox_instance_nr = 0;
    }
    
    enable_widget(gui->widgets["frm_bottom"]->widgets["but_load"]);
    enable_widget(gui->widgets["frm_bottom"]->widgets["but_save"]);
    hide_widget(gui->widgets["frm_hitbox_is"]);
    hide_widget(gui->widgets["frm_top"]);
    
    cam_x = cam_y = 0;
    cam_zoom = 1;
    
    //Find the most popular file name to suggest for new frames.
    last_file_used.clear();
    
    if(!anims.frames.empty()) {
        map<string, size_t> file_uses_map;
        vector<pair<size_t, string> > file_uses_vector;
        for(size_t f = 0; f < anims.frames.size(); ++f) {
            file_uses_map[anims.frames[f]->file]++;
        }
        for(auto u = file_uses_map.begin(); u != file_uses_map.end(); ++u) {
            file_uses_vector.push_back(make_pair(u->second, u->first));
        }
        sort(file_uses_vector.begin(), file_uses_vector.end(), [] (pair<size_t, string> u1, pair<size_t, string> u2) -> bool {
            return u1.first > u2.first;
        });
        last_file_used = file_uses_vector[0].second;
    }
}


/* ----------------------------------------------------------------------------
 * Opens the correct radio button and frame for the specified hitbox type.
 */
void animation_editor::open_hitbox_type(unsigned char type) {
    lafi::widget* f = gui->widgets["frm_hitbox_is"]->widgets["frm_hitbox_i"];
    
    ((lafi::radio_button*) f->widgets["rad_normal"])->unselect();
    ((lafi::radio_button*) f->widgets["rad_attack"])->unselect();
    ((lafi::radio_button*) f->widgets["rad_disabled"])->unselect();
    
    hide_widget(f->widgets["frm_normal"]);
    hide_widget(f->widgets["frm_attack"]);
    
    if(type == HITBOX_TYPE_NORMAL) {
        ((lafi::radio_button*) f->widgets["rad_normal"])->select();
        show_widget(f->widgets["frm_normal"]);
    } else if(type == HITBOX_TYPE_ATTACK) {
        ((lafi::radio_button*) f->widgets["rad_attack"])->select();
        show_widget(f->widgets["frm_attack"]);
    } else {
        ((lafi::radio_button*) f->widgets["rad_disabled"])->select();
    }
}


/* ----------------------------------------------------------------------------
 * Opens the frame where you pick from a list.
 * For the type, use animation_editor::ANIMATION_EDITOR_PICKER_*.
 */
void animation_editor::open_picker(unsigned char type, bool can_make_new) {
    show_widget(gui->widgets["frm_picker"]);
    hide_widget(gui->widgets["frm_bottom"]);
    
    lafi::widget* f = gui->widgets["frm_picker"]->widgets["frm_list"];
    
    if(can_make_new) {
        enable_widget(gui->widgets["frm_picker"]->widgets["txt_new"]);
        enable_widget(gui->widgets["frm_picker"]->widgets["but_new"]);
    } else {
        disable_widget(gui->widgets["frm_picker"]->widgets["txt_new"]);
        disable_widget(gui->widgets["frm_picker"]->widgets["but_new"]);
    }
    
    while(!f->widgets.empty()) {
        f->remove(f->widgets.begin()->first);
    }
    
    vector<string> elements;
    if(type == ANIMATION_EDITOR_PICKER_ANIMATION) {
        for(size_t a = 0; a < anims.animations.size(); ++a) {
            elements.push_back(anims.animations[a]->name);
        }
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME || type == ANIMATION_EDITOR_PICKER_FRAME_INSTANCE) {
        for(size_t f = 0; f < anims.frames.size(); ++f) {
            elements.push_back(anims.frames[f]->name);
        }
    }
    
    if(type == ANIMATION_EDITOR_PICKER_ANIMATION || type == ANIMATION_EDITOR_PICKER_FRAME_INSTANCE) {
        hide_widget(gui->widgets["frm_anims"]);
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME || type == ANIMATION_EDITOR_PICKER_HITBOX_INSTANCE) {
        hide_widget(gui->widgets["frm_frames"]);
    }
    
    f->easy_reset();
    f->easy_row();
    for(size_t e = 0; e < elements.size(); ++e) {
        lafi::button* b = new lafi::button(0, 0, 0, 0, elements[e]);
        string name = elements[e];
        b->left_mouse_click_handler = [name, type, this] (lafi::widget*, int, int) {
            pick(name, type);
        };
        f->easy_add("but_" + i2s(e), b, 100, 24);
        f->easy_row(0);
    }
    
    ((lafi::scrollbar*) gui->widgets["frm_picker"]->widgets["bar_scroll"])->make_widget_scroll(f);
}


/* ----------------------------------------------------------------------------
 * Closes the list picker frame.
 */
void animation_editor::pick(string name, unsigned char type) {
    hide_widget(gui->widgets["frm_picker"]);
    show_widget(gui->widgets["frm_bottom"]);
    
    if(type == ANIMATION_EDITOR_PICKER_ANIMATION) {
        cur_anim = anims.animations[anims.find_animation(name)];
        cur_frame_instance_nr = (cur_anim->frame_instances.size()) ? 0 : INVALID;
        cur_hitbox_instance_nr = INVALID;
        show_widget(gui->widgets["frm_anims"]);
        gui_load_animation();
        
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME_INSTANCE) {
        cur_anim->frame_instances[cur_frame_instance_nr].frame_name = name;
        cur_anim->frame_instances[cur_frame_instance_nr].frame_ptr = anims.frames[anims.find_frame(name)];
        show_widget(gui->widgets["frm_anims"]);
        gui_load_frame_instance();
        
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME) {
        cur_frame = anims.frames[anims.find_frame(name)];
        cur_hitbox_instance_nr = INVALID;
        if(cur_frame->file.empty()) {
            //New frame. Suggest file name.
            cur_frame->file = last_file_used;
        }
        show_widget(gui->widgets["frm_frames"]);
        gui_load_frame();
        
    }
}


/* ----------------------------------------------------------------------------
 * Saves the animation pool onto the mob's file.
 */
void animation_editor::save_animation_pool() {
    data_node file_node = data_node("", "");
    
    data_node* animations_node = new data_node("animations", "");
    file_node.add(animations_node);
    
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        data_node* anim_node = new data_node(anims.animations[a]->name, "");
        animations_node->add(anim_node);
        
        anim_node->add(new data_node("loop_frame", i2s(anims.animations[a]->loop_frame)));
        data_node* frame_instances_node = new data_node("frame_instances", "");
        anim_node->add(frame_instances_node);
        
        for(size_t fi = 0; fi < anims.animations[a]->frame_instances.size(); ++fi) {
            frame_instance* fi_ptr = &anims.animations[a]->frame_instances[fi];
            
            data_node* frame_instance_node = new data_node(fi_ptr->frame_name, "");
            frame_instances_node->add(frame_instance_node);
            
            frame_instance_node->add(new data_node("duration", f2s(fi_ptr->duration)));
        }
    }
    
    data_node* frames_node = new data_node("frames", "");
    file_node.add(frames_node);
    
    for(size_t f = 0; f < anims.frames.size(); ++f) {
        data_node* frame_node = new data_node(anims.frames[f]->name, "");
        frames_node->add(frame_node);
        
        frame_node->add(new data_node("file", anims.frames[f]->file));
        frame_node->add(new data_node("file_x", i2s(anims.frames[f]->file_x)));
        frame_node->add(new data_node("file_y", i2s(anims.frames[f]->file_y)));
        frame_node->add(new data_node("file_w", i2s(anims.frames[f]->file_w)));
        frame_node->add(new data_node("file_h", i2s(anims.frames[f]->file_h)));
        frame_node->add(new data_node("game_w", f2s(anims.frames[f]->game_w)));
        frame_node->add(new data_node("game_h", f2s(anims.frames[f]->game_h)));
        frame_node->add(new data_node("offs_x", f2s(anims.frames[f]->offs_x)));
        frame_node->add(new data_node("offs_y", f2s(anims.frames[f]->offs_y)));
        
        if(is_pikmin) {
            frame_node->add(new data_node("top_visible", b2s(anims.frames[f]->top_visible)));
            frame_node->add(new data_node("top_x", f2s(anims.frames[f]->top_x)));
            frame_node->add(new data_node("top_y", f2s(anims.frames[f]->top_y)));
            frame_node->add(new data_node("top_w", f2s(anims.frames[f]->top_w)));
            frame_node->add(new data_node("top_h", f2s(anims.frames[f]->top_h)));
            frame_node->add(new data_node("top_angle", f2s(anims.frames[f]->top_angle)));
        }
        
        data_node* hitbox_instances_node = new data_node("hitbox_instances", "");
        frame_node->add(hitbox_instances_node);
        
        for(size_t hi = 0; hi < anims.frames[f]->hitbox_instances.size(); ++hi) {
            hitbox_instance* hi_ptr = &anims.frames[f]->hitbox_instances[hi];
            
            data_node* hitbox_instance_node = new data_node(hi_ptr->hitbox_name, "");
            hitbox_instances_node->add(hitbox_instance_node);
            
            hitbox_instance_node->add(
                new data_node(
                    "coords",
                    f2s(hi_ptr->x) + " " + f2s(hi_ptr->y) + " " + f2s(hi_ptr->z)
                )
            );
            hitbox_instance_node->add(new data_node("height", f2s(hi_ptr->height)));
            hitbox_instance_node->add(new data_node("radius", f2s(hi_ptr->radius)));
            hitbox_instance_node->add(new data_node("type", i2s(hi_ptr->type)));
            hitbox_instance_node->add(new data_node("multiplier", f2s(hi_ptr->multiplier)));
            hitbox_instance_node->add(new data_node("can_pikmin_latch", b2s(hi_ptr->can_pikmin_latch)));
            hitbox_instance_node->add(new data_node("hazards", hi_ptr->hazards));
            hitbox_instance_node->add(new data_node("outward", b2s(hi_ptr->knockback_outward)));
            hitbox_instance_node->add(new data_node("angle", f2s(hi_ptr->knockback_angle)));
            hitbox_instance_node->add(new data_node("knockback", f2s(hi_ptr->knockback)));
        }
    }
    
    data_node* hitboxes_node = new data_node("hitboxes", "");
    file_node.add(hitboxes_node);
    
    for(size_t h = 0; h < anims.hitboxes.size(); ++h) {
        data_node* hitbox_node = new data_node(anims.hitboxes[h]->name, "");
        hitboxes_node->add(hitbox_node);
        
    }
    
    file_node.save_file(file_path);
    made_changes = false;
}


/* ----------------------------------------------------------------------------
 * Shows the "unsaved changes" warning.
 */
void animation_editor::show_changes_warning() {
    show_widget(gui->widgets["frm_changes"]);
    hide_widget(gui->widgets["frm_bottom"]);
    
    made_changes = false;
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void animation_editor::unload() {
    anims.destroy();
    delete(gui->style);
    delete(gui);
    al_destroy_native_file_dialog(file_dialog);
}

/* ----------------------------------------------------------------------------
 * Update every frame's hitbox instances in light of new hitbox info.
 */
void animation_editor::update_hitboxes() {
    for(size_t f = 0; f < anims.frames.size(); ++f) {
    
        frame* f_ptr = anims.frames[f];
        
        //Start by deleting non-existent hitboxes.
        for(size_t hi = 0; hi < f_ptr->hitbox_instances.size();) {
            string h_name = f_ptr->hitbox_instances[hi].hitbox_name;
            bool name_found = false;
            
            for(size_t h = 0; h < anims.hitboxes.size(); ++h) {
                if(anims.hitboxes[h]->name == h_name) {
                    name_found = true;
                    break;
                }
            }
            
            if(!name_found) {
                f_ptr->hitbox_instances.erase(f_ptr->hitbox_instances.begin() + hi);
            } else {
                hi++;
            }
        }
        
        //Add missing hitboxes.
        for(size_t h = 0; h < anims.hitboxes.size(); ++h) {
            bool hitbox_found = false;
            string name = anims.hitboxes[h]->name;
            
            for(size_t hi = 0; hi < f_ptr->hitbox_instances.size(); ++hi) {
                if(f_ptr->hitbox_instances[hi].hitbox_name == name) {
                    hitbox_found = true;
                    break;
                }
            }
            
            if(!hitbox_found) {
                f_ptr->hitbox_instances.push_back(hitbox_instance(name));
            }
        }
        
        //Sort them with the new order.
        std::sort(f_ptr->hitbox_instances.begin(), f_ptr->hitbox_instances.end(), [this] (hitbox_instance hi1, hitbox_instance hi2) -> bool {
            size_t pos1 = 0, pos2 = 1;
            for(size_t h = 0; h < anims.hitboxes.size(); ++h) {
                if(anims.hitboxes[h]->name == hi1.hitbox_name) pos1 = h;
                if(anims.hitboxes[h]->name == hi2.hitbox_name) pos2 = h;
            }
            return pos1 < pos2;
        });
    }
}


/* ----------------------------------------------------------------------------
 * Update the stats on the main menu, as well as some other minor things.
 */
void animation_editor::update_stats() {
    lafi::widget* f = gui->widgets["frm_main"]->widgets["frm_object"];
    if(file_path.empty()) {
        hide_widget(f);
    } else {
        show_widget(f);
    }
    
    ((lafi::label*) f->widgets["lbl_n_anims"])->text = "Animations: " + i2s(anims.animations.size());
    ((lafi::label*) f->widgets["lbl_n_frames"])->text = "Frames: " + i2s(anims.frames.size());
    ((lafi::label*) f->widgets["lbl_n_hitboxes"])->text = "Hitboxes: " + i2s(anims.hitboxes.size());
}


void animation_editor::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
}
