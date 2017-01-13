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
    comparison(true),
    comparison_sprite(nullptr),
    comparison_blink(true),
    comparison_blink_show(true),
    comparison_blink_timer(0),
    cur_body_part_nr(INVALID),
    cur_frame_nr(INVALID),
    cur_frame_time(0),
    cur_hitbox_alpha(0),
    cur_hitbox_nr(INVALID),
    cur_maturity(0),
    cur_sprite(NULL),
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
    mode(EDITOR_MODE_MAIN),
    new_hitbox_corner_x(FLT_MAX),
    new_hitbox_corner_y(FLT_MAX),
    sec_mode(ESM_NONE) {
    
    top_bmp[0] = NULL;
    top_bmp[1] = NULL;
    top_bmp[2] = NULL;
    comparison_blink_timer =
        timer(
            0.6,
    [this] () {
        this->comparison_blink_show = !this->comparison_blink_show;
        this->comparison_blink_timer.start();
    }
        );
    comparison_blink_timer.start();
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
    if(
        anim_playing && mode == EDITOR_MODE_ANIMATION &&
        cur_anim && cur_frame_nr != INVALID
    ) {
        frame* f = &cur_anim->frames[cur_frame_nr];
        if(f->duration != 0) {
            cur_frame_time += delta_t;
            
            while(cur_frame_time > f->duration) {
                cur_frame_time = cur_frame_time - f->duration;
                cur_frame_nr++;
                if(cur_frame_nr >= cur_anim->frames.size()) {
                    cur_frame_nr =
                        (
                            cur_anim->loop_frame >=
                            cur_anim->frames.size()
                        ) ? 0 : cur_anim->loop_frame;
                }
                f = &cur_anim->frames[cur_frame_nr];
            }
        } else {
            anim_playing = false;
        }
        gui_load_animation();
    }
    
    cur_hitbox_alpha += M_PI * 3 * delta_t;
    
    if(comparison_blink) {
        comparison_blink_timer.tick(delta_t);
    } else {
        comparison_blink_show = true;
    }
    
    fade_mgr.tick(delta_t);
    
}


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the animation editor.
 */
void animation_editor::do_drawing() {

    gui->draw();
    
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(
        &transform,
        cam_x + ((scr_w - 208) / 2.0 / cam_zoom),
        cam_y + (scr_h / 2.0 / cam_zoom)
    );
    al_scale_transform(&transform, cam_zoom, cam_zoom);
    al_use_transform(&transform);
    
    al_set_clipping_rectangle(0, 0, scr_w - 208, scr_h - 16); {
    
        al_clear_to_color(al_map_rgb(128, 144, 128));
        
        sprite* s = NULL;
        
        if(mode == EDITOR_MODE_ANIMATION) {
            if(cur_frame_nr != INVALID) {
                string name =
                    cur_anim->frames[cur_frame_nr].sprite_name;
                size_t s_pos = anims.find_sprite(name);
                if(s_pos != INVALID) s = anims.sprites[s_pos];
            }
            
        } else if(
            mode == EDITOR_MODE_SPRITE || mode == EDITOR_MODE_TOP ||
            mode == EDITOR_MODE_HITBOXES ||
            mode == EDITOR_MODE_SPRITE_OFFSET
        ) {
            s = cur_sprite;
            
        }
        
        if(s) {
            if(s->bitmap) {
                draw_sprite(
                    s->bitmap,
                    s->offs_x, s->offs_y,
                    s->game_w, s->game_h
                );
            }
            
            if(hitboxes_visible) {
                size_t n_hitboxes = s->hitboxes.size();
                for(size_t h = 0; h < n_hitboxes; ++h) {
                    hitbox* h_ptr = &s->hitboxes[h];
                    
                    ALLEGRO_COLOR hitbox_color, hitbox_outline_color;
                    unsigned char hitbox_outline_alpha =
                        63 + 192 * ((sin(cur_hitbox_alpha) / 2.0) + 0.5);
                        
                    if(h_ptr->type == HITBOX_TYPE_NORMAL) {
                        hitbox_color = al_map_rgba(0, 128, 0, 128);
                        hitbox_outline_color = al_map_rgba(0, 64, 0, 255);
                    } else if(h_ptr->type == HITBOX_TYPE_ATTACK) {
                        hitbox_color = al_map_rgba(128, 0, 0, 128);
                        hitbox_outline_color = al_map_rgba(64, 0, 0, 255);
                    } else {
                        hitbox_color = al_map_rgba(128, 128, 0, 128);
                        hitbox_outline_color = al_map_rgba(64, 64, 0, 255);
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
                        (
                            cur_hitbox_nr == h ?
                            change_alpha(
                                hitbox_outline_color,
                                hitbox_outline_alpha
                            ) :
                            hitbox_outline_color
                        ),
                        (
                            cur_hitbox_nr == h ?
                            3 / cam_zoom :
                            2 / cam_zoom
                        )
                    );
                }
            }
            
            if(s->top_visible && is_pikmin) {
                draw_sprite(
                    top_bmp[cur_maturity],
                    s->top_x, s->top_y,
                    s->top_w, s->top_h,
                    s->top_angle
                );
            }
            
            if(
                comparison && comparison_blink_show &&
                comparison_sprite && comparison_sprite->bitmap
            ) {
                draw_sprite(
                    comparison_sprite->bitmap,
                    comparison_sprite->offs_x, comparison_sprite->offs_y,
                    comparison_sprite->game_w, comparison_sprite->game_h,
                    0
                );
            }
        }
        
        if(hitboxes_visible) {
            float cam_leftmost = -cam_x - (scr_w / 2 / cam_zoom);
            float cam_topmost = -cam_y - (scr_h / 2 / cam_zoom);
            float cam_rightmost = cam_leftmost + (scr_w / cam_zoom);
            float cam_bottommost = cam_topmost + (scr_h / cam_zoom);
            
            al_draw_line(
                0, cam_topmost, 0, cam_bottommost,
                al_map_rgb(240, 240, 240), 1 / cam_zoom
            );
            al_draw_line(
                cam_leftmost, 0, cam_rightmost, 0,
                al_map_rgb(240, 240, 240), 1 / cam_zoom
            );
        }
        
    } al_reset_clipping_rectangle();
    
    ALLEGRO_TRANSFORM id_transform;
    al_identity_transform(&id_transform);
    al_use_transform(&id_transform);
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Returns a file path, but cropped to fit on the GUI's buttons.
 * This implies cutting it in two lines, and even replacing the start with
 * ellipsis, if needed.
 */
string animation_editor::get_cut_path(const string &p) {
    if(p.size() <= 22) return p;
    
    string result = p;
    if(p.size() > 44) {
        result = "..." + p.substr(p.size() - 41, 41);
    }
    
    if(p.size() > 22) {
        result =
            result.substr(0, result.size() / 2) + "\n" +
            result.substr(result.size() / 2, (result.size() / 2) + 1);
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Loads the animation's data onto the gui.
 */
void animation_editor::gui_load_animation() {
    lafi::widget* f = gui->widgets["frm_anims"];
    
    ((lafi::button*) f->widgets["but_anim"])->text =
        cur_anim ? cur_anim->name : "";
        
    if(!cur_anim) {
        hide_widget(f->widgets["frm_anim"]);
    } else {
        show_widget(f->widgets["frm_anim"]);
        
        ((lafi::button*) f->widgets["but_anim"])->text = cur_anim->name;
        ((lafi::textbox*) f->widgets["frm_anim"]->widgets["txt_loop"])->text =
            i2s(cur_anim->loop_frame + 1);
            
        gui_load_frame();
    }
}


/* ----------------------------------------------------------------------------
 * Loads the frame's data onto the gui.
 */
void animation_editor::gui_load_sprite() {
    lafi::widget* f = gui->widgets["frm_sprites"];
    
    ((lafi::button*) f->widgets["but_sprite"])->text =
        cur_sprite ? cur_sprite->name : "";
        
    if(!cur_sprite) {
        hide_widget(f->widgets["frm_sprite"]);
    } else {
        show_widget(f->widgets["frm_sprite"]);
        
        f = f->widgets["frm_sprite"];
        
        ((lafi::textbox*) f->widgets["txt_file"])->text =
            cur_sprite->file;
        ((lafi::textbox*) f->widgets["txt_filex"])->text =
            i2s(cur_sprite->file_x);
        ((lafi::textbox*) f->widgets["txt_filey"])->text =
            i2s(cur_sprite->file_y);
        ((lafi::textbox*) f->widgets["txt_filew"])->text =
            i2s(cur_sprite->file_w);
        ((lafi::textbox*) f->widgets["txt_fileh"])->text =
            i2s(cur_sprite->file_h);
        ((lafi::textbox*) f->widgets["txt_gamew"])->text =
            f2s(cur_sprite->game_w);
        ((lafi::textbox*) f->widgets["txt_gameh"])->text =
            f2s(cur_sprite->game_h);
        ((lafi::textbox*) f->widgets["txt_offsx"])->text =
            f2s(cur_sprite->offs_x);
        ((lafi::textbox*) f->widgets["txt_offsy"])->text =
            f2s(cur_sprite->offs_y);
            
        if(is_pikmin) {
            enable_widget(f->widgets["but_top"]);
        } else {
            disable_widget(f->widgets["but_top"]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the frame's data onto the gui.
 */
void animation_editor::gui_load_frame() {
    lafi::widget* f = gui->widgets["frm_anims"]->widgets["frm_anim"];
    bool valid = cur_frame_nr != INVALID && cur_anim;
    
    ((lafi::label*) f->widgets["lbl_f_nr"])->text =
        "Current frame: " +
        (valid ? i2s((cur_frame_nr + 1)) : "--") +
        " / " + i2s(cur_anim->frames.size());
        
    if(!valid) {
        hide_widget(f->widgets["frm_frame"]);
    } else {
        show_widget(f->widgets["frm_frame"]);
        
        (
            (lafi::button*) f->widgets["frm_frame"]->widgets["but_sprite"]
        )->text =
            cur_anim->frames[cur_frame_nr].sprite_name;
        (
            (lafi::textbox*) f->widgets["frm_frame"]->widgets["txt_dur"]
        )->text =
            f2s(cur_anim->frames[cur_frame_nr].duration);
    }
}


/* ----------------------------------------------------------------------------
 * Loads the sprite offset's data onto the gui.
 */
void animation_editor::gui_load_sprite_offset() {
    lafi::widget* f = gui->widgets["frm_offset"];
    
    ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(cur_sprite->offs_x);
    ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(cur_sprite->offs_y);
    ((lafi::checkbox*) f->widgets["chk_compare"])->set(comparison);
    ((lafi::checkbox*) f->widgets["chk_compare_blink"])->set(
        comparison_blink
    );
    ((lafi::button*) f->widgets["but_compare"])->text =
        (comparison_sprite ? comparison_sprite->name : "");
}


/* ----------------------------------------------------------------------------
 * Loads the body part's data onto the gui.
 */
void animation_editor::gui_load_body_part() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_body_parts"];
    
    ((lafi::label*) f->widgets["lbl_nr"])->text =
        (
            anims.body_parts.empty() ?
            "--/0" :
            (string) (i2s(cur_body_part_nr + 1) + "/" + i2s(anims.body_parts.size()))
        );
        
    if(anims.body_parts.empty()) {
        hide_widget(f->widgets["frm_body_part"]);
        return;
    }
    
    f = (lafi::frame*) f->widgets["frm_body_part"];
    show_widget(f);
    
    ((lafi::textbox*) f->widgets["txt_name"])->text =
        anims.body_parts[cur_body_part_nr]->name;
}


/* ----------------------------------------------------------------------------
 * Loads the hitbox's data onto the gui.
 */
void animation_editor::gui_load_hitbox() {
    lafi::widget* f = gui->widgets["frm_hitboxes"]->widgets["frm_hitbox"];
    
    hitbox* cur_h = NULL;
    if(!cur_sprite->hitboxes.empty()) {
        cur_h = &cur_sprite->hitboxes[cur_hitbox_nr];
    }
    if(cur_h) {
        (
            (lafi::label*) gui->widgets["frm_hitboxes"]->widgets["lbl_name"]
        )->text =
            cur_h->body_part_name;
        ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(cur_h->x);
        ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(cur_h->y);
        ((lafi::textbox*) f->widgets["txt_z"])->text = f2s(cur_h->z);
        ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(cur_h->height);
        ((lafi::textbox*) f->widgets["txt_r"])->text = f2s(cur_h->radius);
    }
    
    open_hitbox_type(cur_h ? cur_h->type : 255);
    
    if(cur_h) {
        show_widget(f);
        if(cur_h->type == HITBOX_TYPE_NORMAL) {
            f = f->widgets["frm_normal"];
            ((lafi::textbox*) f->widgets["txt_mult"])->text =
                f2s(cur_h->multiplier);
            if(cur_h->can_pikmin_latch) {
                ((lafi::checkbox*) f->widgets["chk_latch"])->check();
            } else {
                ((lafi::checkbox*) f->widgets["chk_latch"])->uncheck();
            }
            (
                (lafi::textbox*) f->widgets["txt_hazards"]
            )->text =
                cur_h->hazards_str;
                
        } else if(cur_h->type == HITBOX_TYPE_ATTACK) {
            f = f->widgets["frm_attack"];
            ((lafi::textbox*) f->widgets["txt_mult"])->text =
                f2s(cur_h->multiplier);
            ((lafi::textbox*) f->widgets["txt_hazards"])->text =
                cur_h->hazards_str;
            (
                (lafi::checkbox*) f->widgets["chk_outward"]
            )->set(cur_h->knockback_outward);
            (
                (lafi::angle_picker*) f->widgets["ang_angle"]
            )->set_angle_rads(cur_h->knockback_angle);
            (
                (lafi::textbox*) f->widgets["txt_knockback"]
            )->text = f2s(cur_h->knockback);
            
            if(cur_h->knockback_outward) {
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
    if(cur_sprite->top_visible) c->check();
    else c->uncheck();
    
    ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(cur_sprite->top_x);
    ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(cur_sprite->top_y);
    ((lafi::textbox*) f->widgets["txt_w"])->text = f2s(cur_sprite->top_w);
    ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(cur_sprite->top_h);
    (
        (lafi::angle_picker*) f->widgets["ang_angle"]
    )->set_angle_rads(cur_sprite->top_angle);
}


/* ----------------------------------------------------------------------------
 * Saves the animation's data from the gui.
 */
void animation_editor::gui_save_animation() {
    if(!cur_anim) return;
    
    lafi::widget* f = gui->widgets["frm_anims"]->widgets["frm_anim"];
    
    cur_anim->loop_frame =
        s2i(((lafi::textbox*) f->widgets["txt_loop"])->text) - 1;
    if(cur_anim->loop_frame >= cur_anim->frames.size()) {
        cur_anim->loop_frame = 0;
    }
    
    gui_save_frame();
    gui_load_animation();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the sprite's data from the gui.
 */
void animation_editor::gui_save_sprite() {
    if(!cur_sprite) return;
    
    lafi::widget* f = gui->widgets["frm_sprites"]->widgets["frm_sprite"];
    
    string new_file;
    int new_fx, new_fy, new_fw, new_fh;
    
    new_file =          ((lafi::textbox*) f->widgets["txt_file"])->text;
    new_fx =            s2i(((lafi::textbox*) f->widgets["txt_filex"])->text);
    new_fy =            s2i(((lafi::textbox*) f->widgets["txt_filey"])->text);
    new_fw =            s2i(((lafi::textbox*) f->widgets["txt_filew"])->text);
    new_fh =            s2i(((lafi::textbox*) f->widgets["txt_fileh"])->text);
    cur_sprite->game_w = s2f(((lafi::textbox*) f->widgets["txt_gamew"])->text);
    cur_sprite->game_h = s2f(((lafi::textbox*) f->widgets["txt_gameh"])->text);
    cur_sprite->offs_x = s2f(((lafi::textbox*) f->widgets["txt_offsx"])->text);
    cur_sprite->offs_y = s2f(((lafi::textbox*) f->widgets["txt_offsy"])->text);
    
    //Automatically fill in the in-game width/height if it hasn't been set yet.
    if(cur_sprite->game_w == 0.0f) cur_sprite->game_w = new_fw;
    if(cur_sprite->game_h == 0.0f) cur_sprite->game_h = new_fh;
    
    if(
        cur_sprite->file != new_file ||
        cur_sprite->file_x != new_fx || cur_sprite->file_y != new_fy ||
        cur_sprite->file_w != new_fw || cur_sprite->file_h != new_fh
    ) {
        //Changed something image-wise. Recreate it.
        if(cur_sprite->parent_bmp) bitmaps.detach(cur_sprite->file);
        if(cur_sprite->bitmap) al_destroy_bitmap(cur_sprite->bitmap);
        cur_sprite->bitmap = NULL;
        cur_sprite->parent_bmp = bitmaps.get(new_file, NULL);
        if(cur_sprite->parent_bmp) {
            cur_sprite->bitmap =
                al_create_sub_bitmap(
                    cur_sprite->parent_bmp, new_fx, new_fy, new_fw, new_fh
                );
        }
        cur_sprite->file = new_file;
        cur_sprite->file_x = new_fx;
        cur_sprite->file_y = new_fy;
        cur_sprite->file_w = new_fw;
        cur_sprite->file_h = new_fh;
    }
    
    last_file_used = new_file;
    
    gui_save_hitbox();
    gui_load_sprite();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the frame's data from the gui.
 */
void animation_editor::gui_save_frame() {
    bool valid = cur_frame_nr != INVALID && cur_anim;
    if(!valid) return;
    
    lafi::widget* fw = gui->widgets["frm_anims"]->widgets["frm_anim"];
    
    frame* f = &cur_anim->frames[cur_frame_nr];
    f->duration =
        s2f(
            (
                (lafi::textbox*)
                fw->widgets["frm_frame"]->widgets["txt_dur"]
            )->text
        );
    if(f->duration < 0) f->duration = 0;
    
    gui_load_frame();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the sprite's bitmap offset data data from the gui.
 */
void animation_editor::gui_save_sprite_offset() {
    lafi::widget* f = gui->widgets["frm_offset"];
    
    cur_sprite->offs_x =
        s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_sprite->offs_y =
        s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    comparison =
        ((lafi::checkbox*) f->widgets["chk_compare"])->checked;
    comparison_blink =
        ((lafi::checkbox*) f->widgets["chk_compare_blink"])->checked;
        
    gui_load_sprite_offset();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the body part's data from the gui.
 */
void animation_editor::gui_save_body_part() {
    gui_load_body_part();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the hitbox's data from the gui.
 */
void animation_editor::gui_save_hitbox() {
    bool valid = cur_hitbox_nr != INVALID && cur_sprite;
    if(!valid) return;
    
    lafi::widget* f = gui->widgets["frm_hitboxes"]->widgets["frm_hitbox"];
    
    hitbox* h = &cur_sprite->htboxes[cur_htbox_nr];
    
    h->x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    h->y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    h->z = s2f(((lafi::textbox*) f->widgets["txt_z"])->text);
    
    h->height = s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    h->radius = s2f(((lafi::textbox*) f->widgets["txt_r"])->text);
    if(h->radius <= 0) h->radius = 16;
    
    htbox* cur_h =
        &cur_sprite->htboxes[cur_htbox_nr];
        
    if(((lafi::radio_button*) f->widgets["rad_normal"])->selected) {
        cur_h->type = hTBOX_TYPE_NORMAL;
    } else if(((lafi::radio_button*) f->widgets["rad_attack"])->selected) {
        cur_h->type = hTBOX_TYPE_ATTACK;
    } else {
        cur_h->type = hTBOX_TYPE_DISABLED;
    }
    
    if(cur_h->type == hTBOX_TYPE_NORMAL) {
        cur_h->multiplier =
            s2f(
                (
                    (lafi::textbox*)
                    f->widgets["frm_normal"]->widgets["txt_mult"]
                )->text
            );
        cur_h->can_pikmin_latch =
            (
                (lafi::checkbox*)
                f->widgets["frm_normal"]->widgets["chk_latch"]
            )->checked;
        cur_h->hazards_str =
            (
                (lafi::textbox*)
                f->widgets["frm_normal"]->widgets["txt_hazards"]
            )->text;
            
    } else if(cur_h->type == hTBOX_TYPE_ATTACK) {
        cur_h->multiplier =
            s2f(
                (
                    (lafi::textbox*)
                    f->widgets["frm_attack"]->widgets["txt_mult"]
                )->text
            );
        cur_h->hazards_str =
            (
                (lafi::textbox*)
                f->widgets["frm_attack"]->widgets["txt_hazards"]
            )->text;
        cur_h->knockback_outward =
            (
                (lafi::checkbox*)
                f->widgets["frm_attack"]->widgets["chk_outward"]
            )->checked;
        cur_h->knockback_angle =
            (
                (lafi::angle_picker*)
                f->widgets["frm_attack"]->widgets["ang_angle"]
            )->get_angle_rads();
        cur_h->knockback =
            s2f(
                (
                    (lafi::textbox*)
                    f->widgets["frm_attack"]->widgets["txt_knockback"]
                )->text
            );
            
    }
    
    gui_load_htbox();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the Pikmin top's data from the gui.
 */
void animation_editor::gui_save_top() {
    lafi::widget* f = gui->widgets["frm_top"];
    
    cur_sprite->top_visible =
        ((lafi::checkbox*) f->widgets["chk_visible"])->checked;
    cur_sprite->top_x =
        s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_sprite->top_y =
        s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_sprite->top_w =
        s2f(((lafi::textbox*) f->widgets["txt_w"])->text);
    cur_sprite->top_h =
        s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    cur_sprite->top_angle =
        ((lafi::angle_picker*) f->widgets["ang_angle"])->get_angle_rads();
        
    gui_load_top();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Handles the controls and other events.
 */
void animation_editor::handle_controls(ALLEGRO_EVENT ev) {

    if(fade_mgr.is_fading()) return;
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        mouse_cursor_x =
            ev.mouse.x / cam_zoom - cam_x -
            ((scr_w - 208) / 2 / cam_zoom);
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
        
        if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) {
            if(ev.mouse.dz != 0) {
                //Zoom.
                float new_zoom = cam_zoom + (cam_zoom * ev.mouse.dz * 0.1);
                new_zoom = max(ZOOM_MIN_LEVEL_EDITOR, new_zoom);
                new_zoom = min(ZOOM_MAX_LEVEL_EDITOR, new_zoom);
                float new_mc_x =
                    ev.mouse.x / new_zoom - cam_x -
                    ((scr_w - 208) / 2 / new_zoom);
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
            gui_load_sprite_offset();
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        ev.mouse.x <= scr_w - 208 && ev.mouse.y < scr_h - 16
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
        if(ev.mouse.x < scr_w - 208 && ev.mouse.y < scr_h - 16) {
            if(s) {
                for(size_t h = 0; h < s->hitboxes.size(); ++h) {
                
                    hitbox* h_ptr = &s->hitboxes[h];
                    dist d(
                        mouse_cursor_x, mouse_cursor_y, h_ptr->x, h_ptr->y
                    );
                    if(d <= h_ptr->radius) {
                        gui_save_hitbox();
                        cur_hitbox_nr = h;
                        gui_load_hitbox();
                        
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
            
            gui_load_hitbox();
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
    
    load_custom_particle_generators();
    load_status_types();
    load_liquids();
    load_hazards();
    
    fade_mgr.start_fade(true, nullptr);
    
    lafi::style* s =
        new lafi::style(
        al_map_rgb(192, 192, 208),
        al_map_rgb(0, 0, 32),
        al_map_rgb(96, 128, 160)
    );
    gui = new lafi::gui(scr_w, scr_h, s);
    
    
    //Main frame.
    lafi::frame* frm_main =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add(
        "lbl_file",
        new lafi::label(0, 0, 0, 0, "Choose a file:"), 100, 16
    );
    frm_main->easy_row();
    frm_main->easy_add(
        "but_file",
        new lafi::button(0, 0, 0, 0), 100, 32
    );
    int y = frm_main->easy_row();
    
    lafi::frame* frm_object =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_main->add("frm_object", frm_object);
    frm_object->easy_row();
    frm_object->easy_add(
        "but_anims",
        new lafi::button(0, 0, 0, 0, "Edit animations"), 100, 32
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_sprites",
        new lafi::button(0, 0, 0, 0, "Edit sprites"), 100, 32
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_body_parts",
        new lafi::button(0, 0, 0, 0, "Edit body parts"), 100, 32
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_tools",
        new lafi::button(0, 0, 0, 0, "Special tools"), 100, 32
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_n_anims",
        new lafi::label(0, 0, 0, 0), 100, 12
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_n_sprites",
        new lafi::label(0, 0, 0, 0), 100, 12
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_n_body_parts",
        new lafi::label(0, 0, 0, 0), 100, 12
    );
    frm_object->easy_row();
    
    
    //Properties -- main.
    frm_main->widgets["but_file"]->left_mouse_click_handler =
    [this, frm_main] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_HISTORY;
        populate_history();
        hide_widget(this->gui->widgets["frm_main"]);
        show_widget(this->gui->widgets["frm_history"]);
    };
    frm_main->widgets["but_file"]->description =
        "Pick a file to load or create.";
        
    frm_object->widgets["but_anims"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_hitbox_nr = INVALID;
        if(cur_anim) {
            if(cur_anim->frames.size()) cur_frame_nr = 0;
        }
        mode = EDITOR_MODE_ANIMATION;
        hide_widget(this->gui->widgets["frm_main"]);
        show_widget(this->gui->widgets["frm_anims"]);
        gui_load_animation();
    };
    frm_object->widgets["but_anims"]->description =
        "Change the way the animations look like.";
        
    frm_object->widgets["but_sprites"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SPRITE;
        cur_hitbox_nr = INVALID;
        hide_widget(this->gui->widgets["frm_main"]);
        show_widget(this->gui->widgets["frm_sprites"]);
        gui_load_sprite();
    };
    frm_object->widgets["but_sprites"]->description =
        "Change how each individual sprite looks like.";
        
    frm_object->widgets["but_body_parts"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_BODY_PART;
        hide_widget(this->gui->widgets["frm_main"]);
        show_widget(this->gui->widgets["frm_body_parts"]);
        cur_body_part_nr = 0;
        gui_load_body_part();
    };
    frm_object->widgets["but_body_parts"]->description =
        "Change what body parts exist, and their order.";
        
    frm_object->widgets["but_tools"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_TOOLS;
        hide_widget(this->gui->widgets["frm_main"]);
        show_widget(this->gui->widgets["frm_tools"]);
    };
    frm_object->widgets["but_tools"]->description =
        "Special tools to help with specific tasks.";
        
        
    //History.
    lafi::frame* frm_history =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_history);
    gui->add("frm_history", frm_history);
    
    frm_history->easy_row();
    frm_history->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "but_browse",
        new lafi::button(0, 0, 0, 0, "Browse"), 100, 24
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "dum_1",
        new lafi::dummy(0, 0, 0, 0), 100, 16
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "lbl_hist",
        new lafi::label(0, 0, 0, 0, "History:"), 100, 16
    );
    y = frm_history->easy_row();
    frm_history->add(
        "frm_list",
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48)
    );
    
    
    //Properties -- history.
    frm_history->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        hide_widget(this->gui->widgets["frm_history"]);
        show_widget(this->gui->widgets["frm_main"]);
    };
    frm_history->widgets["but_back"]->description =
        "Go back to the main menu.";
    frm_history->widgets["but_browse"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        al_show_native_file_dialog(display, file_dialog);
        
        //Reset the locale, which gets set by Allegro's native dialogs...
        //and breaks s2f().
        setlocale(LC_ALL, "C");
        
        if(al_get_native_file_dialog_count(file_dialog) == 0) return;
        file_path = al_get_native_file_dialog_path(file_dialog, 0);
        if(file_path.empty()) return;
        
        load_animation_database();
        update_animation_editor_history(file_path);
        save_options(); //Save the history on the options.
        mode = EDITOR_MODE_MAIN;
        hide_widget(this->gui->widgets["frm_history"]);
        show_widget(this->gui->widgets["frm_main"]);
    };
    frm_history->widgets["but_browse"]->description =
        "Pick a file to load or create.";
        
        
    //Animations frame.
    lafi::frame* frm_anims =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_anims);
    gui->add("frm_anims", frm_anims);
    
    frm_anims->easy_row();
    frm_anims->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_anims->easy_row();
    frm_anims->easy_add(
        "lbl_anim",
        new lafi::label(0, 0, 0, 0, "Animation:"), 85, 16
    );
    frm_anims->easy_add(
        "but_del_anim",
        new lafi::button(0, 0, 0, 0, "-"), 15, 16
    );
    frm_anims->easy_row();
    frm_anims->easy_add(
        "but_anim",
        new lafi::button(0, 0, 0, 0), 100, 32
    );
    y = frm_anims->easy_row();
    
    lafi::frame* frm_anim =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_anims->add("frm_anim", frm_anim);
    frm_anim->easy_row();
    frm_anim->easy_add(
        "lin_1",
        new lafi::line(0, 0, 0, 0), 15, 12
    );
    frm_anim->easy_add(
        "lbl_data",
        new lafi::label(0, 0, 0, 0, "Animation data", ALLEGRO_ALIGN_CENTER),
        70, 12
    );
    frm_anim->easy_add(
        "lin_2",
        new lafi::line(0, 0, 0, 0), 15, 12
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "lbl_loop",
        new lafi::label(0, 0, 0, 0, "Loop frame:"), 50, 16
    );
    frm_anim->easy_add(
        "txt_loop",
        new lafi::textbox(0, 0, 0, 0), 50, 16
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "lin_3",
        new lafi::line(0, 0, 0, 0), 25, 12
    );
    frm_anim->easy_add(
        "lbl_list",
        new lafi::label(0, 0, 0, 0, "Frame list", ALLEGRO_ALIGN_CENTER),
        50, 12
    );
    frm_anim->easy_add(
        "lin_4",
        new lafi::line(0, 0, 0, 0), 25, 12
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "lbl_f_nr",
        new lafi::label(0, 0, 0, 0), 100, 16
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "but_play",
        new lafi::button(0, 0, 0, 0, "P"), 20, 32
    );
    frm_anim->easy_add(
        "but_prev",
        new lafi::button(0, 0, 0, 0, "<"), 20, 32
    );
    frm_anim->easy_add(
        "but_next",
        new lafi::button(0, 0, 0, 0, ">"), 20, 32
    );
    frm_anim->easy_add(
        "but_add",
        new lafi::button(0, 0, 0, 0, "+"), 20, 32
    );
    frm_anim->easy_add(
        "but_rem",
        new lafi::button(0, 0, 0, 0, "-"), 20, 32
    );
    y += frm_anim->easy_row();
    
    lafi::frame* frm_frame =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_anim->add("frm_frame", frm_frame);
    
    frm_frame->easy_row();
    frm_frame->easy_add(
        "lbl_sprite",
        new lafi::label(0, 0, 0, 0, "Sprite:"), 30, 16
    );
    frm_frame->easy_add(
        "but_sprite",
        new lafi::button(0, 0, 0, 0), 70, 24
    );
    frm_frame->easy_row();
    frm_frame->easy_add(
        "lbl_dur",
        new lafi::label(0, 0, 0, 0, "Duration:"), 40, 16
    );
    frm_frame->easy_add(
        "txt_dur",
        new lafi::textbox(0, 0, 0, 0), 60, 16
    );
    frm_frame->easy_row();
    frm_frame->easy_add(
        "dum_1",
        new lafi::dummy(0, 0, 0, 0), 100, 16
    );
    frm_frame->easy_row();
    frm_frame->easy_add(
        "but_dur_all",
        new lafi::button(0, 0, 0, 0, "Apply duration to all"), 100, 24
    );
    frm_frame->easy_row();
    
    
    //Properties -- animations.
    auto lambda_gui_save_animation =
    [this] (lafi::widget*) { gui_save_animation(); };
    auto lambda_gui_save_frame_instance =
    [this] (lafi::widget*) { gui_save_frame(); };
    
    frm_anims->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        anim_playing = false;
        hide_widget(this->gui->widgets["frm_anims"]);
        show_widget(this->gui->widgets["frm_main"]);
        update_stats();
    };
    frm_anims->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_anims->widgets["but_del_anim"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_anim) return;
        anims.animations.erase(
            anims.animations.begin() +
            anims.find_animation(cur_anim->name)
        );
        anim_playing = false;
        cur_anim = NULL;
        cur_frame_nr = INVALID;
        cur_hitbox_nr = INVALID;
        gui_load_animation();
        made_changes = true;
    };
    frm_anims->widgets["but_del_anim"]->description =
        "Delete the current animation.";
        
    frm_anims->widgets["but_anim"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        anim_playing = false;
        hide_widget(this->gui->widgets["frm_anims"]);
        open_picker(ANIMATION_EDITOR_PICKER_ANIMATION, true);
    };
    frm_anims->widgets["but_anim"]->description =
        "Pick an animation to edit.";
        
    frm_anim->widgets["txt_loop"]->lose_focus_handler =
        lambda_gui_save_animation;
    frm_anim->widgets["txt_loop"]->description =
        "The animation loops back to this frame when it ends.";
        
    frm_anim->widgets["but_play"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(cur_anim->frames.size() < 2) {
            anim_playing = false;
        } else {
            anim_playing = !anim_playing;
            if(
                !cur_anim->frames.empty() &&
                cur_frame_nr == INVALID
            ) {
                cur_frame_nr = 0;
            }
            cur_frame_time = 0;
        }
    };
    frm_anim->widgets["but_play"]->description =
        "Play or pause the animation.";
        
    frm_anim->widgets["but_prev"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        anim_playing = false;
        if(!cur_anim->frames.empty()) {
            if(cur_frame_nr == INVALID) {
                cur_frame_nr = 0;
            } else if(cur_frame_nr == 0) {
                cur_frame_nr =
                    cur_anim->frames.size() - 1;
            } else cur_frame_nr--;
        }
        gui_load_frame();
    };
    frm_anim->widgets["but_prev"]->description =
        "Previous frame.";
        
    frm_anim->widgets["but_next"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        anim_playing = false;
        if(!cur_anim->frames.empty()) {
            if(
                cur_frame_nr ==
                cur_anim->frames.size() - 1 ||
                cur_frame_nr == INVALID
            ) {
                cur_frame_nr = 0;
            } else {
                cur_frame_nr++;
            }
        }
        gui_load_frame();
    };
    frm_anim->widgets["but_next"]->description =
        "Next frame.";
        
    frm_anim->widgets["but_add"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        anim_playing = false;
        if(cur_frame_nr != INVALID) {
            cur_frame_nr++;
            cur_anim->frames.insert(
                cur_anim->frames.begin() + cur_frame_nr,
                frame(
                    cur_anim->frames[cur_frame_nr - 1]
                )
            );
        } else {
            cur_anim->frames.push_back(frame());
            cur_frame_nr = 0;
        }
        gui_load_frame();
        made_changes = true;
    };
    frm_anim->widgets["but_add"]->description =
        "Add a new frame after the current one (via copy).";
        
    frm_anim->widgets["but_rem"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        anim_playing = false;
        if(cur_frame_nr != INVALID) {
            cur_anim->frames.erase(
                cur_anim->frames.begin() + cur_frame_nr
            );
            if(cur_anim->frames.empty()) {
                cur_frame_nr = INVALID;
            } else if(
                cur_frame_nr >=
                cur_anim->frames.size()
            ) {
                cur_frame_nr =
                    cur_anim->frames.size() - 1;
            }
        }
        gui_load_frame();
        made_changes = true;
    };
    frm_anim->widgets["but_rem"]->description =
        "Remove the current frame.";
        
    frm_frame->widgets["but_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        anim_playing = false;
        hide_widget(this->gui->widgets["frm_anims"]);
        open_picker(ANIMATION_EDITOR_PICKER_FRAME_INSTANCE, false);
    };
    frm_frame->widgets["but_sprite"]->description =
        "Pick the sprite to use for this frame.";
        
    frm_frame->widgets["txt_dur"]->lose_focus_handler =
        lambda_gui_save_frame_instance;
    frm_frame->widgets["txt_dur"]->mouse_down_handler =
    [this] (lafi::widget*, int, int, int) {
        anim_playing = false;
    };
    frm_frame->widgets["txt_dur"]->description =
        "How long this frame lasts for, in seconds.";
        
    frm_frame->widgets["but_dur_all"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        float d = cur_anim->frames[cur_frame_nr].duration;
        for(size_t i = 0; i < cur_anim->frames.size(); ++i) {
            cur_anim->frames[i].duration = d;
        }
    };
    frm_frame->widgets["but_dur_all"]->description =
        "Apply this duration to all frames on this animation.";
        
    frm_anims->register_accelerator(
        ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL,
        frm_anims->widgets["frm_anim"]->widgets["but_next"]
    );
    frm_anims->register_accelerator(
        ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_SHIFT,
        frm_anims->widgets["frm_anim"]->widgets["but_prev"]
    );
    
    
    //Sprites frame.
    lafi::frame* frm_sprites =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_sprites);
    gui->add("frm_sprites", frm_sprites);
    
    frm_sprites->easy_row();
    frm_sprites->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_sprites->easy_row();
    frm_sprites->easy_add(
        "lbl_sprite",
        new lafi::label(0, 0, 0, 0, "Sprite:"), 85, 16
    );
    frm_sprites->easy_add(
        "but_del_sprite",
        new lafi::button(0, 0, 0, 0, "-"), 15, 16
    );
    frm_sprites->easy_row();
    frm_sprites->easy_add(
        "but_sprite",
        new lafi::button(0, 0, 0, 0), 100, 32
    );
    y = frm_sprites->easy_row();
    
    lafi::frame* frm_sprite =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_sprites->add("frm_sprite", frm_sprite);
    
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "lin_1",
        new lafi::line(0, 0, 0, 0), 25, 12
    );
    frm_sprite->easy_add(
        "lbl_f_data",
        new lafi::label(0, 0, 0, 0, "Sprite data", ALLEGRO_ALIGN_CENTER), 50, 12
    );
    frm_sprite->easy_add(
        "lin_2",
        new lafi::line(0, 0, 0, 0), 25, 12
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "lbl_file",
        new lafi::label(0, 0, 0, 0, "File:"), 25, 16
    );
    frm_sprite->easy_add(
        "txt_file",
        new lafi::textbox(0, 0, 0, 0), 75, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "lbl_filexy",
        new lafi::label(0, 0, 0, 0, "File XY:"), 45, 16
    );
    frm_sprite->easy_add(
        "txt_filex",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_sprite->easy_add(
        "txt_filey",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "lbl_filewh",
        new lafi::label(0, 0, 0, 0, "File WH:"), 45, 16
    );
    frm_sprite->easy_add(
        "txt_filew",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_sprite->easy_add(
        "txt_fileh",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "lbl_gamewh",
        new lafi::label(0, 0, 0, 0, "Game WH:"), 45, 16
    );
    frm_sprite->easy_add(
        "txt_gamew",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_sprite->easy_add(
        "txt_gameh",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_offsxy",
        new lafi::button(0, 0, 0, 0, "Offset:"), 45, 16
    );
    frm_sprite->easy_add(
        "txt_offsx",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_sprite->easy_add(
        "txt_offsy",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_hitboxes",
        new lafi::button(0, 0, 0, 0, "Edit hitboxes"), 100, 32
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_top",
        new lafi::button(0, 0, 0, 0, "Edit Pikmin top"), 100, 32
    );
    frm_sprite->easy_row();
    
    
    //Properties -- sprites.
    auto lambda_gui_save_frame = [this] (lafi::widget*) { gui_save_sprite(); };
    
    frm_sprites->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        hide_widget(this->gui->widgets["frm_sprites"]);
        show_widget(this->gui->widgets["frm_main"]);
        update_stats();
    };
    frm_sprites->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_sprites->widgets["but_del_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_sprite) return;
        anims.sprites.erase(
            anims.sprites.begin() + anims.find_sprite(cur_sprite->name)
        );
        cur_sprite = NULL;
        cur_hitbox_nr = INVALID;
        gui_load_sprite();
        made_changes = true;
    };
    frm_sprites->widgets["but_del_sprite"]->description =
        "Delete the current sprite.";
        
    frm_sprites->widgets["but_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        hide_widget(this->gui->widgets["frm_sprites"]);
        open_picker(ANIMATION_EDITOR_PICKER_FRAME, true);
    };
    frm_sprites->widgets["but_sprite"]->description =
        "Pick a sprite to edit.";
        
    frm_sprite->widgets["txt_file"]->lose_focus_handler =
        lambda_gui_save_frame;
    frm_sprite->widgets["txt_file"]->description =
        "Name (+extension) of the file with the sprite.";
        
    frm_sprite->widgets["txt_filex"]->lose_focus_handler =
        lambda_gui_save_frame;
    frm_sprite->widgets["txt_filex"]->description =
        "X of the top-left corner of the sprite.";
        
    frm_sprite->widgets["txt_filey"]->lose_focus_handler =
        lambda_gui_save_frame;
    frm_sprite->widgets["txt_filey"]->description =
        "Y of the top-left corner of the sprite.";
        
    frm_sprite->widgets["txt_filew"]->lose_focus_handler =
        lambda_gui_save_frame;
    frm_sprite->widgets["txt_filew"]->description =
        "Width of the sprite, in the file.";
        
    frm_sprite->widgets["txt_fileh"]->lose_focus_handler =
        lambda_gui_save_frame;
    frm_sprite->widgets["txt_fileh"]->description =
        "Height of the sprite, in the file.";
        
    frm_sprite->widgets["txt_gamew"]->lose_focus_handler =
        lambda_gui_save_frame;
    frm_sprite->widgets["txt_gamew"]->description =
        "In-game sprite width.";
        
    frm_sprite->widgets["txt_gameh"]->lose_focus_handler =
        lambda_gui_save_frame;
    frm_sprite->widgets["txt_gameh"]->description =
        "In-game sprite height.";
        
    frm_sprite->widgets["but_offsxy"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_offset"]);
        hide_widget(this->gui->widgets["frm_sprites"]);
        mode = EDITOR_MODE_SPRITE_OFFSET;
        comparison_sprite = NULL;
        gui_load_sprite_offset();
    };
    frm_sprite->widgets["but_offsxy"]->description =
        "Click this button for an offset wizard tool.";
        
    frm_sprite->widgets["txt_offsx"]->lose_focus_handler =
        lambda_gui_save_frame;
    frm_sprite->widgets["txt_offsx"]->description =
        "In-game, offset by this much, horizontally.";
        
    frm_sprite->widgets["txt_offsy"]->lose_focus_handler =
        lambda_gui_save_frame;
    frm_sprite->widgets["txt_offsy"]->description =
        "In-game, offset by this much, vertically.";
        
    frm_sprite->widgets["but_hitboxes"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_hitboxes"]);
        hide_widget(this->gui->widgets["frm_sprites"]);
        mode = EDITOR_MODE_HITBOXES;
        cur_hitbox_nr = 0;
        gui_load_hitbox();
    };
    frm_sprite->widgets["but_hitboxes"]->description =
        "Edit this frame's hitboxes.";
        
    frm_sprite->widgets["but_top"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_top"]);
        hide_widget(this->gui->widgets["frm_sprites"]);
        mode = EDITOR_MODE_TOP;
        gui_load_top();
    };
    frm_sprite->widgets["but_top"]->description =
        "Edit the Pikmin's top (maturity) for this sprite.";
        
        
    //Sprite offset frame.
    lafi::frame* frm_offset =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_offset);
    gui->add("frm_offset", frm_offset);
    
    frm_offset->easy_row();
    frm_offset->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_offset->easy_row();
    frm_offset->easy_add(
        "lbl_xy",
        new lafi::label(0, 0, 0, 0, "X, Y:"), 25, 16
    );
    frm_offset->easy_add(
        "txt_x",
        new lafi::textbox(0, 0, 0, 0, ""), 37.5, 16
    );
    frm_offset->easy_add(
        "txt_y",
        new lafi::textbox(0, 0, 0, 0, ""), 37.5, 16
    );
    frm_offset->easy_row();
    frm_offset->easy_add(
        "lbl_mouse1",
        new lafi::label(0, 0, 0, 0, "Or move it with"), 100, 8
    );
    frm_offset->easy_row();
    frm_offset->easy_add(
        "lbl_mouse2",
        new lafi::label(0, 0, 0, 0, "the left mouse button."), 100, 8
    );
    frm_offset->easy_row();
    frm_offset->easy_add(
        "lin_1",
        new lafi::line(0, 0, 0, 0), 100, 8
    );
    frm_offset->easy_row();
    frm_offset->easy_add(
        "chk_compare",
        new lafi::checkbox(0, 0, 0, 0, "Comparison sprite"), 100, 16
    );
    frm_offset->easy_row();
    frm_offset->easy_add(
        "dum_1",
        new lafi::dummy(0, 0, 0, 0), 10, 24
    );
    frm_offset->easy_add(
        "but_compare",
        new lafi::button(0, 0, 0, 0, ""), 90, 24
    );
    frm_offset->easy_row();
    frm_offset->easy_add(
        "dum_2",
        new lafi::dummy(0, 0, 0, 0), 10, 16
    );
    frm_offset->easy_add(
        "chk_compare_blink",
        new lafi::checkbox(0, 0, 0, 0, "Blink comparison?"), 90, 16
    );
    frm_offset->easy_row();
    
    
    //Properties -- Sprite offset.
    auto lambda_save_offset =
    [this] (lafi::widget*) { gui_save_sprite_offset(); };
    auto lambda_save_offset_click =
    [this] (lafi::widget*, int, int) { gui_save_sprite_offset(); };
    
    frm_offset->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_sprites"]);
        hide_widget(this->gui->widgets["frm_offset"]);
        this->comparison_sprite = NULL;
        mode = EDITOR_MODE_SPRITE;
        gui_load_sprite();
    };
    frm_offset->widgets["but_back"]->description =
        "Go back to the sprite editor.";
        
    frm_offset->widgets["txt_x"]->lose_focus_handler =
        lambda_save_offset;
    frm_offset->widgets["txt_x"]->description =
        "In-game, offset by this much, horizontally.";
        
    frm_offset->widgets["txt_y"]->lose_focus_handler =
        lambda_save_offset;
    frm_offset->widgets["txt_y"]->description =
        "In-game, offset by this much, vertically.";
        
    frm_offset->widgets["chk_compare"]->left_mouse_click_handler =
        lambda_save_offset_click;
    frm_offset->widgets["chk_compare"]->description =
        "Overlay a different sprite for comparison purposes.";
        
    frm_offset->widgets["but_compare"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        hide_widget(this->gui->widgets["frm_offset"]);
        open_picker(ANIMATION_EDITOR_PICKER_FRAME, false);
    };
    frm_offset->widgets["but_compare"]->description =
        "Sprite to compare with.";
        
    frm_offset->widgets["chk_compare_blink"]->left_mouse_click_handler =
        lambda_save_offset_click;
    frm_offset->widgets["chk_compare_blink"]->description =
        "Blink the comparison in and out?";
        
        
    //Hitboxes frame.
    lafi::frame* frm_hitboxes =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_hitboxes);
    gui->add("frm_hitboxes", frm_hitboxes);
    
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add(
        "but_prev",
        new lafi::button(0, 0, 0, 0, "<"), 20, 24
    );
    frm_hitboxes->easy_add(
        "but_next",
        new lafi::button(0, 0, 0, 0, ">"), 20, 24
    );
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add(
        "lbl_n",
        new lafi::label(0, 0, 0, 0, "Hitbox:"), 30, 24
    );
    frm_hitboxes->easy_add(
        "lbl_name",
        new lafi::label(0, 0, 0, 0), 70, 24
    );
    y = frm_hitboxes->easy_row();
    
    lafi::frame* frm_hitbox =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_hitboxes->add("frm_hitbox", frm_hitbox);
    
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "lbl_xy",
        new lafi::label(0, 0, 0, 0, "X, Y:"), 45, 16
    );
    frm_hitbox->easy_add(
        "txt_x",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_hitbox->easy_add(
        "txt_y",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "lbl_zh",
        new lafi::label(0, 0, 0, 0, "Z, Height:"), 45, 16
    );
    frm_hitbox->easy_add(
        "txt_z",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_hitbox->easy_add(
        "txt_h",
        new lafi::textbox(0, 0, 0, 0), 27.5, 16
    );
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "lbl_r",
        new lafi::label(0, 0, 0, 0, "Radius:"), 45, 16
    );
    frm_hitbox->easy_add(
        "txt_r",
        new lafi::textbox(0, 0, 0, 0), 55, 16
    );
    frm_hitbox->easy_row();
    
    frm_hitbox->easy_add(
        "lbl_h_type",
        new lafi::label(0, 0, 0, 0, "Hitbox type:"), 100, 12
    );
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "rad_normal",
        new lafi::radio_button(0, 0, 0, 0, "Normal"), 50, 16
    );
    frm_hitbox->easy_add(
        "rad_attack",
        new lafi::radio_button(0, 0, 0, 0, "Attack"), 50, 16
    );
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "rad_disabled",
        new lafi::radio_button(0, 0, 0, 0, "Disabled"), 100, 16
    );
    y += frm_hitbox->easy_row();
    
    lafi::frame* frm_normal =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    hide_widget(frm_normal);
    frm_hitbox->add("frm_normal", frm_normal);
    
    frm_normal->easy_row();
    frm_normal->easy_add(
        "lbl_mult",
        new lafi::label(0, 0, 0, 0, "Defense mult.:"), 60, 16
    );
    frm_normal->easy_add(
        "txt_mult",
        new lafi::textbox(0, 0, 0, 0), 40, 16
    );
    frm_normal->easy_row();
    frm_normal->easy_add(
        "chk_latch",
        new lafi::checkbox(0, 0, 0, 0, "Pikmin can latch"), 100, 16
    );
    frm_normal->easy_row();
    frm_normal->easy_add(
        "lbl_hazards",
        new lafi::label(0, 0, 0, 0, "Hazards:"), 100, 12
    );
    frm_normal->easy_row();
    frm_normal->easy_add(
        "txt_hazards",
        new lafi::textbox(0, 0, 0, 0), 100, 16
    );
    frm_normal->easy_row();
    
    lafi::frame* frm_attack =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    hide_widget(frm_attack);
    frm_hitbox->add("frm_attack", frm_attack);
    
    frm_attack->easy_row();
    frm_attack->easy_add(
        "lbl_mult",
        new lafi::label(0, 0, 0, 0, "Attack mult.:"), 60, 16
    );
    frm_attack->easy_add(
        "txt_mult",
        new lafi::textbox(0, 0, 0, 0), 40, 16
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "lbl_hazards",
        new lafi::label(0, 0, 0, 0, "Hazards:"), 100, 12
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "txt_hazards",
        new lafi::textbox(0, 0, 0, 0), 100, 16
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "chk_outward",
        new lafi::checkbox(0, 0, 0, 0, "Outward knockback"), 100, 16
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "lbl_angle",
        new lafi::label(0, 0, 0, 0, "KB angle:"), 60, 16
    );
    frm_attack->easy_add(
        "ang_angle",
        new lafi::angle_picker(0, 0, 0, 0), 40, 24
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "lbl_knockback",
        new lafi::label(0, 0, 0, 0, "KB strength:"), 60, 16
    );
    frm_attack->easy_add(
        "txt_knockback",
        new lafi::textbox(0, 0, 0, 0), 40, 16
    );
    frm_attack->easy_row();
    
    
    //Properties -- hitboxes.
    auto lambda_gui_save_hitbox_instance =
    [this] (lafi::widget*) { gui_save_hitbox(); };
    auto lambda_gui_save_hitbox_instance_click =
    [this] (lafi::widget*, int, int) { gui_save_hitbox(); };
    
    frm_hitboxes->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SPRITE;
        hide_widget(this->gui->widgets["frm_hitboxes"]);
        show_widget(this->gui->widgets["frm_sprites"]);
        cur_hitbox_nr = INVALID;
        update_stats();
    };
    frm_hitboxes->widgets["but_back"]->description =
        "Go back to the frame editor.";
        
    frm_hitboxes->widgets["but_prev"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(cur_sprite->hitboxes.size()) {
            if(cur_hitbox_nr == INVALID) {
                cur_hitbox_nr = 0;
            } else if(cur_hitbox_nr == 0) {
                cur_hitbox_nr =
                    cur_sprite->hitboxes.size() - 1;
            } else {
                cur_hitbox_nr--;
            }
        }
        gui_load_hitbox();
    };
    frm_hitboxes->widgets["but_prev"]->description =
        "Previous hitbox.";
        
    frm_hitboxes->widgets["but_next"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(cur_sprite->hitboxes.size()) {
            if(cur_hitbox_nr == INVALID) {
                cur_hitbox_nr = 0;
            }
            cur_hitbox_nr =
                (cur_hitbox_nr + 1) %
                cur_sprite->hitboxes.size();
        }
        gui_load_hitbox();
    };
    frm_hitboxes->widgets["but_next"]->description =
        "Next hitbox.";
        
    frm_hitbox->register_accelerator(
        ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL,
        frm_hitboxes->widgets["but_next"]
    );
    frm_hitbox->register_accelerator(
        ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_SHIFT,
        frm_hitboxes->widgets["but_prev"]
    );
    
    frm_hitbox->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_hitbox->widgets["txt_x"]->description =
        "X of the hitbox's center.";
        
    frm_hitbox->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_hitbox->widgets["txt_y"]->description =
        "Y of the hitbox's center.";
        
    frm_hitbox->widgets["txt_z"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_hitbox->widgets["txt_z"]->description =
        "Altitude of the hitbox's bottom.";
        
    frm_hitbox->widgets["txt_h"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_hitbox->widgets["txt_h"]->description =
        "Hitbox's height. 0 = spans infinitely vertically.";
        
    frm_hitbox->widgets["txt_r"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_hitbox->widgets["txt_r"]->description =
        "Hitbox's radius.";
        
    frm_hitbox->widgets["rad_normal"]->left_mouse_click_handler =
        lambda_gui_save_hitbox_instance_click;
    frm_hitbox->widgets["rad_normal"]->description =
        "Normal hitbox, one that can be damaged.";
        
    frm_hitbox->widgets["rad_attack"]->left_mouse_click_handler =
        lambda_gui_save_hitbox_instance_click;
    frm_hitbox->widgets["rad_attack"]->description =
        "Attack hitbox, one that damages opponents.";
        
    frm_hitbox->widgets["rad_disabled"]->left_mouse_click_handler =
        lambda_gui_save_hitbox_instance_click;
    frm_hitbox->widgets["rad_disabled"]->description =
        "This hitbox will be non-existent.";
        
    frm_normal->widgets["txt_mult"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_normal->widgets["txt_mult"]->description =
        "Defense multiplier. 0 = invulnerable.";
        
    frm_normal->widgets["chk_latch"]->left_mouse_click_handler =
        lambda_gui_save_hitbox_instance_click;
    frm_normal->widgets["chk_latch"]->description =
        "Can the Pikmin latch on to this hitbox?";
        
    frm_normal->widgets["txt_hazards"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_normal->widgets["txt_hazards"]->description =
        "List of hazards, semicolon separated.";
        
    frm_attack->widgets["txt_mult"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_attack->widgets["txt_mult"]->description =
        "Attack multiplier.";
        
    frm_attack->widgets["txt_hazards"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_attack->widgets["txt_hazards"]->description =
        "List of hazards, semicolon separated.";
        
    frm_attack->widgets["chk_outward"]->left_mouse_click_handler =
        lambda_gui_save_hitbox_instance_click;
    frm_attack->widgets["chk_outward"]->description =
        "Makes Pikmin be knocked away from the center.";
        
    frm_attack->widgets["ang_angle"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_attack->widgets["ang_angle"]->description =
        "Angle the Pikmin are knocked towards.";
        
    frm_attack->widgets["txt_knockback"]->lose_focus_handler =
        lambda_gui_save_hitbox_instance;
    frm_attack->widgets["txt_knockback"]->description =
        "Knockback strength.";
        
        
    //Pikmin top frame.
    lafi::frame* frm_top =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_top);
    gui->add("frm_top", frm_top);
    
    frm_top->easy_row();
    frm_top->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "chk_visible",
        new lafi::checkbox(0, 0, 0, 0, "Visible"), 100, 16
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "lbl_xy",
        new lafi::label(0, 0, 0, 0, "X&Y:"), 20, 16
    );
    frm_top->easy_add(
        "txt_x",
        new lafi::textbox(0, 0, 0, 0), 40, 16
    );
    frm_top->easy_add(
        "txt_y",
        new lafi::textbox(0, 0, 0, 0), 40, 16
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "lbl_wh",
        new lafi::label(0, 0, 0, 0, "W&H:"), 20, 16
    );
    frm_top->easy_add(
        "txt_w",
        new lafi::textbox(0, 0, 0, 0), 40, 16
    );
    frm_top->easy_add(
        "txt_h",
        new lafi::textbox(0, 0, 0, 0), 40, 16
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "lbl_angle",
        new lafi::label(0, 0, 0, 0, "Angle:"), 40, 16
    );
    frm_top->easy_add(
        "ang_angle",
        new lafi::angle_picker(0, 0, 0, 0), 60, 24
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "but_maturity",
        new lafi::button(0, 0, 0, 0, "Change maturity"), 100, 24
    );
    frm_top->easy_row();
    
    
    //Properties -- Pikmin top.
    auto lambda_save_top =
    [this] (lafi::widget*) { gui_save_top(); };
    auto lambda_save_top_click =
    [this] (lafi::widget*, int, int) { gui_save_top(); };
    
    frm_top->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_widget(this->gui->widgets["frm_sprites"]);
        hide_widget(this->gui->widgets["frm_top"]);
        mode = EDITOR_MODE_SPRITE;
    };
    frm_top->widgets["but_back"]->description =
        "Go back to the sprite editor.";
        
    frm_top->widgets["chk_visible"]->left_mouse_click_handler =
        lambda_save_top_click;
    frm_top->widgets["chk_visible"]->description =
        "Is the top visible in this sprite?";
        
    frm_top->widgets["txt_x"]->lose_focus_handler =
        lambda_save_top;
    frm_top->widgets["txt_x"]->description =
        "X position of the top's center.";
        
    frm_top->widgets["txt_y"]->lose_focus_handler =
        lambda_save_top;
    frm_top->widgets["txt_y"]->description =
        "Y position of the top's center.";
        
    frm_top->widgets["txt_w"]->lose_focus_handler =
        lambda_save_top;
    frm_top->widgets["txt_w"]->description =
        "In-game width of the top.";
        
    frm_top->widgets["txt_h"]->lose_focus_handler =
        lambda_save_top;
    frm_top->widgets["txt_h"]->description =
        "In-game height of the top.";
        
    frm_top->widgets["ang_angle"]->lose_focus_handler =
        lambda_save_top;
    frm_top->widgets["ang_angle"]->description =
        "Angle of the top.";
        
    frm_top->widgets["but_maturity"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_maturity = (cur_maturity + 1) % 3;
    };
    frm_top->widgets["but_maturity"]->description =
        "View a different maturity top.";
        
        
    //Body parts frame.
    lafi::frame* frm_body_parts =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_body_parts);
    gui->add("frm_body_parts", frm_body_parts);
    
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_inst1",
        new lafi::label(0, 0, 0, 0, "The lower a part's"), 100, 12
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_inst2",
        new lafi::label(0, 0, 0, 0, "number, the more"), 100, 12
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_inst3",
        new lafi::label(0, 0, 0, 0, "priority it has when"), 100, 12
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_inst4",
        new lafi::label(0, 0, 0, 0, "checking collisions."), 100, 12
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "dummy",
        new lafi::dummy(), 100, 16
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "txt_add",
        new lafi::textbox(0, 0, 0, 0, ""), 80, 16
    );
    frm_body_parts->easy_add(
        "but_add",
        new lafi::button(0, 0, 0, 0, "+"), 20, 24
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "but_prev",
        new lafi::button(0, 0, 0, 0, "<"), 20, 24
    );
    frm_body_parts->easy_add(
        "but_next",
        new lafi::button(0, 0, 0, 0, ">"), 20, 24
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_n",
        new lafi::label(0, 0, 0, 0, "Part nr:"), 50, 16
    );
    frm_body_parts->easy_add(
        "lbl_nr",
        new lafi::label(0, 0, 0, 0, ""), 50, 16
    );
    y = frm_body_parts->easy_row();
    
    lafi::frame* frm_body_part =
        new lafi::frame(scr_w - 208, y, scr_w, scr_h - 48);
    frm_body_parts->add("frm_body_part", frm_body_part);
    
    frm_body_part->easy_row();
    frm_body_part->easy_add(
        "lbl_na",
        new lafi::label(0, 0, 0, 0, "Name:"), 30, 16
    );
    frm_body_part->easy_add(
        "txt_name",
        new lafi::textbox(0, 0, 0, 0, ""), 70, 16
    );
    frm_body_part->easy_row();
    frm_body_part->easy_add(
        "but_left",
        new lafi::button(0, 0, 0, 0, "<="), 20, 24
    );
    frm_body_part->easy_add(
        "but_right",
        new lafi::button(0, 0, 0, 0, "=>"), 20, 24
    );
    frm_body_part->easy_add(
        "but_rem",
        new lafi::button(0, 0, 0, 0, "-"), 20, 24
    );
    frm_body_part->easy_row();
    
    
    //Properties -- body parts.
    auto lambda_gui_save_hitbox =
    [this] (lafi::widget*) { gui_save_body_part(); };
    auto lambda_gui_save_hitbox_click =
    [this] (lafi::widget*, int, int) { gui_save_body_part(); };
    
    frm_body_parts->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        hide_widget(this->gui->widgets["frm_body_parts"]);
        show_widget(this->gui->widgets["frm_main"]);
        update_stats();
    };
    frm_body_parts->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_body_parts->widgets["txt_add"]->description =
        "Name of the body part you want to create.";
        
    frm_body_parts->widgets["but_add"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        string name =
            (
                (lafi::textbox*)
                this->gui->widgets["frm_body_parts"]->widgets["txt_add"]
            )->text;
        if(name.empty()) return;
        for(size_t b = 0; b < anims.body_parts.size(); ++b) {
            if(anims.body_parts[b]->name == name) {
                cur_body_part_nr = b;
                gui_load_body_part();
                return;
            }
        }
        anims.body_parts.insert(
            anims.body_parts.begin() + cur_body_part_nr +
            (anims.body_parts.empty() ? 0 : 1),
            new body_part(name)
        );
        if(anims.body_parts.size() == 1) cur_body_part_nr = 0;
        else cur_body_part_nr++;
        update_hitboxes();
        gui_load_body_part();
        made_changes = true;
    };
    frm_body_parts->widgets["but_add"]->description =
        "Create a new body part (after the current one).";
        
    frm_body_parts->widgets["but_prev"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(anims.body_parts.empty()) return;
        if(cur_body_part_nr == 0) cur_body_part_nr = anims.body_parts.size() - 1;
        else cur_body_part_nr--;
        gui_load_body_part();
    };
    frm_body_parts->widgets["but_prev"]->description =
        "Previous body part.";
        
    frm_body_parts->widgets["but_next"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(anims.body_parts.empty()) return;
        cur_body_part_nr = (cur_body_part_nr + 1) % anims.body_parts.size();
        gui_load_body_part();
    };
    frm_body_parts->widgets["but_next"]->description =
        "Next body part.";
        
    frm_body_part->widgets["txt_name"]->lose_focus_handler =
    [this] (lafi::widget * t) {
        string new_name = ((lafi::textbox*) t)->text;
        if(new_name.empty()) {
            gui_load_body_part();
            return;
        }
        for(size_t b = 0; b < anims.body_parts.size(); ++b) {
            if(b == cur_body_part_nr) continue;
            if(anims.body_parts[b]->name == new_name) {
                gui_load_body_part();
                return;
            }
        }
        anims.body_parts[cur_body_part_nr]->name = new_name;
        update_hitboxes();
        gui_load_body_part();
        made_changes = true;
    };
    frm_body_part->widgets["txt_name"]->description =
        "Name of this body part.";
        
    frm_body_part->widgets["but_left"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(anims.body_parts.size() < 2) return;
        size_t prev_nr =
            (cur_body_part_nr == 0) ?
            anims.body_parts.size() - 1 :
            cur_body_part_nr - 1;
        body_part* cur_bp = anims.body_parts[cur_body_part_nr];
        anims.body_parts.erase(anims.body_parts.begin() + cur_body_part_nr);
        anims.body_parts.insert(anims.body_parts.begin() + prev_nr, cur_bp);
        cur_body_part_nr = prev_nr;
        update_hitboxes();
        gui_load_body_part();
        made_changes = true;
    };
    frm_body_part->widgets["but_left"]->description =
        "Move this part to the left in the list.";
        
    frm_body_part->widgets["but_right"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(anims.body_parts.size() < 2) return;
        size_t next_nr = (cur_body_part_nr + 1) % anims.body_parts.size();
        body_part* cur_bp = anims.body_parts[cur_body_part_nr];
        anims.body_parts.erase(anims.body_parts.begin() + cur_body_part_nr);
        anims.body_parts.insert(anims.body_parts.begin() + next_nr, cur_bp);
        cur_body_part_nr = next_nr;
        update_hitboxes();
        gui_load_body_part();
        made_changes = true;
    };
    frm_body_part->widgets["but_right"]->description =
        "Move this part to the right in the list.";
        
    frm_body_part->widgets["but_rem"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        delete anims.body_parts[cur_body_part_nr];
        anims.body_parts.erase(anims.body_parts.begin() + cur_body_part_nr);
        if(cur_body_part_nr > 0) cur_body_part_nr--;
        update_hitboxes();
        gui_load_body_part();
        made_changes = true;
    };
    frm_body_part->widgets["but_rem"]->description =
        "Delete this body part.";
        
        
    //Tools frame.
    lafi::frame* frm_tools =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_tools);
    gui->add("frm_tools", frm_tools);
    
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_back",
        new lafi::button(0, 0, 0, 0, "Back"), 50, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_resize",
        new lafi::label(0, 0, 0, 0, "Resize everything:"), 100, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "txt_resize",
        new lafi::textbox(0, 0, 0, 0), 80, 16
    );
    frm_tools->easy_add(
        "but_resize",
        new lafi::button(0, 0, 0, 0, "Ok"), 20, 24
    );
    frm_tools->easy_row();
    
    
    //Properties -- tools.
    frm_tools->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        hide_widget(this->gui->widgets["frm_tools"]);
        show_widget(this->gui->widgets["frm_main"]);
        update_stats();
    };
    frm_tools->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_tools->widgets["txt_resize"]->description =
        "Resize multiplier. (0.5 = half, 2 = double)";
        
    frm_tools->widgets["but_resize"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        resize_everything();
    };
    frm_tools->widgets["but_resize"]->description =
        "Resize all in-game X/Y and W/H by the given amount.";
        
        
    //Picker frame.
    lafi::frame* frm_picker =
        new lafi::frame(scr_w - 208, 0, scr_w, scr_h - 48);
    hide_widget(frm_picker);
    gui->add("frm_picker", frm_picker);
    
    frm_picker->add(
        "but_back",
        new lafi::button(scr_w - 200, 8, scr_w - 104, 24, "Back")
    );
    frm_picker->add(
        "txt_new",
        new lafi::textbox(scr_w - 200, 40, scr_w - 48, 56)
    );
    frm_picker->add(
        "but_new",
        new lafi::button(scr_w - 40,  32, scr_w - 8,  64, "+")
    );
    frm_picker->add(
        "frm_list",
        new lafi::frame(scr_w - 200, 72, scr_w - 32, scr_h - 56)
    );
    frm_picker->add(
        "bar_scroll",
        new lafi::scrollbar(scr_w - 24,  72, scr_w - 8,  scr_h - 56)
    );
    
    
    //Properties -- picker.
    frm_picker->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        (
            (lafi::textbox*)
            this->gui->widgets["frm_picker"]->widgets["txt_new"]
        )->text.clear();
        
        hide_widget(this->gui->widgets["frm_picker"]);
        show_widget(this->gui->widgets["frm_bottom"]);
        if(mode == EDITOR_MODE_MAIN) {
            show_widget(this->gui->widgets["frm_main"]);
        } else if(mode == EDITOR_MODE_ANIMATION) {
            show_widget(this->gui->widgets["frm_anims"]);
        } else if(mode == EDITOR_MODE_SPRITE) {
            show_widget(this->gui->widgets["frm_sprites"]);
        } else if(mode == EDITOR_MODE_SPRITE_OFFSET) {
            show_widget(this->gui->widgets["frm_offset"]);
        }
    };
    frm_picker->widgets["but_back"]->description =
        "Cancel.";
        
    ((lafi::textbox*)frm_picker->widgets["txt_new"])->enter_key_widget =
        frm_picker->widgets["but_new"];
        
    frm_picker->widgets["but_new"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        string name =
            (
                (lafi::textbox*)
                this->gui->widgets["frm_picker"]->widgets["txt_new"]
            )->text;
        if(name.empty()) return;
        
        if(mode == EDITOR_MODE_ANIMATION) {
            if(anims.find_animation(name) != INVALID) return;
            anims.animations.push_back(new animation(name));
            pick(name, ANIMATION_EDITOR_PICKER_ANIMATION);
            
        } else if(mode == EDITOR_MODE_SPRITE) {
            if(anims.find_sprite(name) != INVALID) return;
            anims.sprites.push_back(new sprite(name));
            anims.sprites.back()->create_hitboxes(&anims);
            pick(name, ANIMATION_EDITOR_PICKER_FRAME);
            
        }
        
        made_changes = true;
        
        (
            (lafi::textbox*)
            this->gui->widgets["frm_picker"]->widgets["txt_new"]
        )->text.clear();
    };
    frm_picker->widgets["but_new"]->description =
        "Create a new one with the name on the textbox.";
        
    frm_picker->widgets["frm_list"]->mouse_wheel_handler =
    [this] (lafi::widget*, int dy, int) {
        lafi::scrollbar* s =
            (lafi::scrollbar*)
            this->gui->widgets["frm_picker"]->widgets["bar_scroll"];
        if(s->widgets.find("but_bar") != s->widgets.end()) {
            s->move_button(
                0,
                (s->widgets["but_bar"]->y1 + s->widgets["but_bar"]->y2) /
                2 - 30 * dy
            );
        }
    };
    
    
    //Bottom bar.
    lafi::frame* frm_bottom =
        new lafi::frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    gui->add("frm_bottom", frm_bottom);
    
    frm_bottom->easy_row();
    frm_bottom->easy_add(
        "but_toggle_hitboxes",
        new lafi::button(0, 0, 0, 0, "HB"), 25, 32
    );
    frm_bottom->easy_add(
        "but_load",
        new lafi::button(0, 0, 0, 0, "Load"), 25, 32
    );
    frm_bottom->easy_add(
        "but_save",
        new lafi::button(0, 0, 0, 0, "Save"), 25, 32
    );
    frm_bottom->easy_add(
        "but_quit",
        new lafi::button(0, 0, 0, 0, "Quit"), 25, 32
    );
    frm_bottom->easy_row();
    
    lafi::label* gui_status_bar =
        new lafi::label(0, scr_h - 16, scr_w - 208, scr_h);
    gui->add("lbl_status_bar", gui_status_bar);
    
    
    //Properties -- bottom bar.
    frm_bottom->widgets["but_toggle_hitboxes"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        hitboxes_visible = !hitboxes_visible;
    };
    frm_bottom->widgets["but_toggle_hitboxes"]->description =
        "Toggle hitbox and center-point grid visibility.";
        
    frm_bottom->widgets["but_load"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            load_animation_database();
        }
    };
    frm_bottom->widgets["but_load"]->description =
        "Load the object from the text file.";
        
    frm_bottom->widgets["but_save"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        save_animation_database();
    };
    frm_bottom->widgets["but_save"]->description =
        "Save the object to the text file.";
        
    frm_bottom->widgets["but_quit"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(made_changes) {
            this->show_changes_warning();
        } else {
            leave();
        }
    };
    frm_bottom->widgets["but_quit"]->description =
        "Quit the animation editor.";
        
        
    //Changes warning.
    lafi::frame* frm_changes =
        new lafi::frame(scr_w - 208, scr_h - 48, scr_w, scr_h);
    hide_widget(frm_changes);
    gui->add("frm_changes", frm_changes);
    
    frm_changes->easy_row();
    frm_changes->easy_add(
        "lbl_text1",
        new lafi::label(0, 0, 0, 0, "Warning: you have", ALLEGRO_ALIGN_LEFT),
        80, 8
    );
    frm_changes->easy_row();
    frm_changes->easy_add(
        "lbl_text2",
        new lafi::label(0, 0, 0, 0, "unsaved changes!", ALLEGRO_ALIGN_LEFT),
        80, 8
    );
    frm_changes->easy_row();
    frm_changes->add(
        "but_ok",
        new lafi::button(scr_w - 40, scr_h - 40, scr_w - 8, scr_h - 8, "Ok")
    );
    
    
    //Properties -- changes warning.
    frm_changes->widgets["but_ok"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) { close_changes_warning(); };
    
    
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
    
    if(!auto_load_anim.empty()) {
        file_path = auto_load_anim;
        load_animation_database();
    }
    
}


/* ----------------------------------------------------------------------------
 * Loads the animation database for the current object.
 */
void animation_editor::load_animation_database() {
    file_path = replace_all(file_path, "\\", "/");
    
    anims.destroy();
    
    data_node file = data_node(file_path);
    if(!file.file_was_opened) {
        file.save_file(file_path, true);
    }
    anims = load_animation_database_from_file(&file);
    
    anim_playing = false;
    cur_anim = NULL;
    cur_sprite = NULL;
    cur_frame_nr = INVALID;
    cur_hitbox_nr = INVALID;
    if(!anims.animations.empty()) {
        cur_anim = anims.animations[0];
        if(cur_anim->frames.size()) cur_frame_nr = 0;
    }
    if(!anims.sprites.empty()) {
        cur_sprite = anims.sprites[0];
        if(cur_sprite->hitboxes.size()) cur_hitbox_nr = 0;
    }
    
    enable_widget(gui->widgets["frm_bottom"]->widgets["but_load"]);
    enable_widget(gui->widgets["frm_bottom"]->widgets["but_save"]);
    hide_widget(gui->widgets["frm_hitboxes"]);
    hide_widget(gui->widgets["frm_top"]);
    
    cam_x = cam_y = 0;
    cam_zoom = 1;
    
    //Find the most popular file name to suggest for new sprites.
    last_file_used.clear();
    
    if(!anims.sprites.empty()) {
        map<string, size_t> file_uses_map;
        vector<pair<size_t, string> > file_uses_vector;
        for(size_t f = 0; f < anims.sprites.size(); ++f) {
            file_uses_map[anims.sprites[f]->file]++;
        }
        for(auto u = file_uses_map.begin(); u != file_uses_map.end(); ++u) {
            file_uses_vector.push_back(make_pair(u->second, u->first));
        }
        sort(
            file_uses_vector.begin(),
            file_uses_vector.end(),
        [] (pair<size_t, string> u1, pair<size_t, string> u2) -> bool {
            return u1.first > u2.first;
        }
        );
        last_file_used = file_uses_vector[0].second;
    }
    
    vector<string> file_path_parts = split(file_path, "/");
    ((lafi::button*) gui->widgets["frm_main"]->widgets["but_file"])->text =
        get_cut_path(file_path);
        
    //Top bitmap.
    for(unsigned char t = 0; t < 3; ++t) {
        if(top_bmp[t] && top_bmp[t] != bmp_error) {
            al_destroy_bitmap(top_bmp[t]);
            top_bmp[t] = NULL;
        }
    }
    
    if(file_path.find(PIKMIN_FOLDER) != string::npos) {
        is_pikmin = true;
        data_node data =
            data_node(
                PIKMIN_FOLDER + "/" +
                file_path_parts[file_path_parts.size() - 2] +
                "/Data.txt"
            );
        top_bmp[0] =
            load_bmp(data.get_child_by_name("top_leaf")->value, &data);
        top_bmp[1] =
            load_bmp(data.get_child_by_name("top_bud")->value, &data);
        top_bmp[2] =
            load_bmp(data.get_child_by_name("top_flower")->value, &data);
    } else {
        is_pikmin = false;
    }
    
    hide_widget(this->gui->widgets["frm_anims"]);
    hide_widget(this->gui->widgets["frm_sprites"]);
    hide_widget(this->gui->widgets["frm_body_parts"]);
    show_widget(this->gui->widgets["frm_main"]);
    mode = EDITOR_MODE_MAIN;
    update_stats();
}


/* ----------------------------------------------------------------------------
 * Opens the correct radio button and frame for the specified hitbox type.
 */
void animation_editor::open_hitbox_type(unsigned char type) {
    lafi::widget* f = gui->widgets["frm_hitboxes"]->widgets["frm_hitbox"];
    
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
    } else if(
        type == ANIMATION_EDITOR_PICKER_FRAME ||
        type == ANIMATION_EDITOR_PICKER_FRAME_INSTANCE
    ) {
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            elements.push_back(anims.sprites[s]->name);
        }
    }
    
    f->easy_reset();
    f->easy_row();
    for(size_t e = 0; e < elements.size(); ++e) {
        lafi::button* b = new lafi::button(0, 0, 0, 0, elements[e]);
        string name = elements[e];
        b->left_mouse_click_handler =
        [name, type, this] (lafi::widget*, int, int) {
            pick(name, type);
        };
        f->easy_add("but_" + i2s(e), b, 100, 24);
        f->easy_row(0);
    }
    
    (
        (lafi::scrollbar*) gui->widgets["frm_picker"]->widgets["bar_scroll"]
    )->make_widget_scroll(f);
}


/* ----------------------------------------------------------------------------
 * Closes the list picker frame.
 */
void animation_editor::pick(string name, unsigned char type) {
    hide_widget(gui->widgets["frm_picker"]);
    show_widget(gui->widgets["frm_bottom"]);
    
    if(type == ANIMATION_EDITOR_PICKER_ANIMATION) {
        cur_anim = anims.animations[anims.find_animation(name)];
        cur_frame_nr =
            (cur_anim->frames.size()) ? 0 : INVALID;
        cur_hitbox_nr = INVALID;
        show_widget(gui->widgets["frm_anims"]);
        gui_load_animation();
        
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME_INSTANCE) {
        cur_anim->frames[cur_frame_nr].sprite_name =
            name;
        cur_anim->frames[cur_frame_nr].sprite_ptr =
            anims.sprites[anims.find_sprite(name)];
        show_widget(gui->widgets["frm_anims"]);
        gui_load_frame();
        
    } else if(type == ANIMATION_EDITOR_PICKER_FRAME) {
        if(mode == EDITOR_MODE_SPRITE_OFFSET) {
            comparison_sprite = anims.sprites[anims.find_sprite(name)];
            show_widget(gui->widgets["frm_offset"]);
            gui_load_sprite_offset();
        } else {
            cur_sprite = anims.sprites[anims.find_sprite(name)];
            cur_hitbox_nr = INVALID;
            if(cur_sprite->file.empty()) {
                //New frame. Suggest file name.
                cur_sprite->file = last_file_used;
            }
            show_widget(gui->widgets["frm_sprites"]);
            gui_load_sprite();
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Populates the history frame with the most recent files.
 */
void animation_editor::populate_history() {
    lafi::frame* f =
        (lafi::frame*) gui->widgets["frm_history"]->widgets["frm_list"];
        
    while(!f->widgets.empty()) {
        f->remove(f->widgets.begin()->first);
    }
    
    if(animation_editor_history.empty()) return;
    
    f->easy_reset();
    f->easy_row();
    
    for(size_t h = 0; h < animation_editor_history.size(); ++h) {
    
        string name = animation_editor_history[h];
        if(name.empty()) continue;
        
        lafi::button* b =
            new lafi::button(0, 0, 0, 0, get_cut_path(name));
            
        auto lambda = [name, this] (lafi::widget*, int, int) {
            file_path = name;
            load_animation_database();
            
            mode = EDITOR_MODE_MAIN;
            hide_widget(this->gui->widgets["frm_history"]);
            show_widget(this->gui->widgets["frm_main"]);
            
            update_animation_editor_history(name);
            save_options(); //Save the history on the options.
        };
        b->left_mouse_click_handler = lambda;
        f->easy_add("but_" + i2s(h), b, 100, 32);
        f->easy_row();
    }
}


/* ----------------------------------------------------------------------------
 * Resizes sprites, body parts, etc. by a multiplier.
 */
void animation_editor::resize_everything() {
    lafi::textbox* txt_resize =
        (lafi::textbox*) gui->widgets["frm_tools"]->widgets["txt_resize"];
    float mult = s2f(txt_resize->text);
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        sprite* s_ptr = anims.sprites[s];
        
        s_ptr->game_w *= mult;
        s_ptr->game_h *= mult;
        s_ptr->offs_x *= mult;
        s_ptr->offs_y *= mult;
        s_ptr->top_x  *= mult;
        s_ptr->top_y  *= mult;
        s_ptr->top_w  *= mult;
        s_ptr->top_h  *= mult;
        
        for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
            hitbox* h_ptr = &s_ptr->hitboxes[h];
            
            h_ptr->radius *= mult;
            h_ptr->x      *= mult;
            h_ptr->y      *= mult;
        }
    }
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the animation database onto the mob's file.
 */
void animation_editor::save_animation_database() {
    data_node file_node = data_node("", "");
    
    data_node* animations_node = new data_node("animations", "");
    file_node.add(animations_node);
    
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        data_node* anim_node = new data_node(anims.animations[a]->name, "");
        animations_node->add(anim_node);
        
        anim_node->add(
            new data_node("loop_frame", i2s(anims.animations[a]->loop_frame))
        );
        data_node* frames_node = new data_node("frames", "");
        anim_node->add(frames_node);
        
        for(
            size_t f = 0; f < anims.animations[a]->frames.size();
            ++f
        ) {
            frame* f_ptr = &anims.animations[a]->frames[f];
            
            data_node* frame_node =
                new data_node(f_ptr->sprite_name, "");
            frames_node->add(frame_node);
            
            frame_node->add(
                new data_node("duration", f2s(f_ptr->duration))
            );
        }
    }
    
    data_node* sprites_node = new data_node("sprites", "");
    file_node.add(sprites_node);
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        sprite* s_ptr = anims.sprites[s];
        data_node* sprite_node = new data_node(anims.sprites[s]->name, "");
        sprites_node->add(sprite_node);
        
        sprite_node->add(new data_node("file", s_ptr->file));
        sprite_node->add(new data_node("file_x", i2s(s_ptr->file_x)));
        sprite_node->add(new data_node("file_y", i2s(s_ptr->file_y)));
        sprite_node->add(new data_node("file_w", i2s(s_ptr->file_w)));
        sprite_node->add(new data_node("file_h", i2s(s_ptr->file_h)));
        sprite_node->add(new data_node("game_w", f2s(s_ptr->game_w)));
        sprite_node->add(new data_node("game_h", f2s(s_ptr->game_h)));
        sprite_node->add(new data_node("offs_x", f2s(s_ptr->offs_x)));
        sprite_node->add(new data_node("offs_y", f2s(s_ptr->offs_y)));
        
        if(is_pikmin) {
            sprite_node->add(
                new data_node("top_visible", b2s(s_ptr->top_visible))
            );
            sprite_node->add(
                new data_node("top_x", f2s(s_ptr->top_x))
            );
            sprite_node->add(
                new data_node("top_y", f2s(s_ptr->top_y))
            );
            sprite_node->add(
                new data_node("top_w", f2s(s_ptr->top_w))
            );
            sprite_node->add(
                new data_node("top_h", f2s(s_ptr->top_h))
            );
            sprite_node->add(
                new data_node("top_angle", f2s(s_ptr->top_angle))
            );
        }
        
        data_node* hitboxes_node =
            new data_node("hitboxes", "");
        sprite_node->add(hitboxes_node);
        
        for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
            hitbox* h_ptr = &s_ptr->hitboxes[h];
            
            data_node* hitbox_node =
                new data_node(h_ptr->body_part_name, "");
            hitboxes_node->add(hitbox_node);
            
            hitbox_node->add(
                new data_node(
                    "coords",
                    f2s(h_ptr->x) + " " + f2s(h_ptr->y) +
                    " " + f2s(h_ptr->z)
                )
            );
            hitbox_node->add(
                new data_node("height", f2s(h_ptr->height))
            );
            hitbox_node->add(
                new data_node("radius", f2s(h_ptr->radius))
            );
            hitbox_node->add(
                new data_node("type", i2s(h_ptr->type))
            );
            hitbox_node->add(
                new data_node("multiplier", f2s(h_ptr->multiplier))
            );
            hitbox_node->add(
                new data_node("can_pikmin_latch", b2s(h_ptr->can_pikmin_latch))
            );
            hitbox_node->add(
                new data_node("hazards", h_ptr->hazards_str)
            );
            hitbox_node->add(
                new data_node("outward", b2s(h_ptr->knockback_outward))
            );
            hitbox_node->add(
                new data_node("angle", f2s(h_ptr->knockback_angle))
            );
            hitbox_node->add(
                new data_node("knockback", f2s(h_ptr->knockback))
            );
        }
    }
    
    data_node* body_parts_node = new data_node("body_parts", "");
    file_node.add(body_parts_node);
    
    for(size_t b = 0; b < anims.body_parts.size(); ++b) {
        data_node* body_part_node = new data_node(anims.body_parts[b]->name, "");
        body_parts_node->add(body_part_node);
        
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
    
    unload_hazards();
    unload_status_types();
}

/* ----------------------------------------------------------------------------
 * Update every frame's hitbox instances in light of new hitbox info.
 */
void animation_editor::update_hitboxes() {
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
    
        sprite* s_ptr = anims.sprites[s];
        
        //Start by deleting non-existent hitboxes.
        for(size_t h = 0; h < s_ptr->hitboxes.size();) {
            string h_name = s_ptr->hitboxes[h].body_part_name;
            bool name_found = false;
            
            for(size_t b = 0; b < anims.body_parts.size(); ++b) {
                if(anims.body_parts[b]->name == h_name) {
                    name_found = true;
                    break;
                }
            }
            
            if(!name_found) {
                s_ptr->hitboxes.erase(
                    s_ptr->hitboxes.begin() + h
                );
            } else {
                h++;
            }
        }
        
        //Add missing hitboxes.
        for(size_t b = 0; b < anims.body_parts.size(); ++b) {
            bool hitbox_found = false;
            string name = anims.body_parts[b]->name;
            
            for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
                if(s_ptr->hitboxes[h].body_part_name == name) {
                    hitbox_found = true;
                    break;
                }
            }
            
            if(!hitbox_found) {
                s_ptr->hitboxes.push_back(hitbox(name));
            }
        }
        
        //Sort them with the new order.
        std::sort(
            s_ptr->hitboxes.begin(),
            s_ptr->hitboxes.end(),
        [this] (hitbox h1, hitbox h2) -> bool {
            size_t pos1 = 0, pos2 = 1;
            for(size_t b = 0; b < anims.body_parts.size(); ++b) {
                if(anims.body_parts[b]->name == h1.body_part_name) pos1 = b;
                if(anims.body_parts[b]->name == h2.body_part_name) pos2 = b;
            }
            return pos1 < pos2;
        }
        );
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
    
    ((lafi::label*) f->widgets["lbl_n_anims"])->text =
        "Animations: " + i2s(anims.animations.size());
    ((lafi::label*) f->widgets["lbl_n_sprites"])->text =
        "Sprites: " + i2s(anims.sprites.size());
    ((lafi::label*) f->widgets["lbl_n_body_parts"])->text =
        "Body parts: " + i2s(anims.body_parts.size());
}


/* ----------------------------------------------------------------------------
 * Exits out of the animation editor, with a fade.
 */
void animation_editor::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
}
