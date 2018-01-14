/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General animation editor-related functions.
 */

#include <algorithm>

#include <allegro5/allegro.h>

#include "animation_editor.h"
#include "../animation.h"
#include "../drawing.h"
#include "../functions.h"
#include "../LAFI/angle_picker.h"
#include "../LAFI/button.h"
#include "../LAFI/checkbox.h"
#include "../LAFI/frame.h"
#include "../LAFI/minor.h"
#include "../LAFI/radio_button.h"
#include "../LAFI/scrollbar.h"
#include "../LAFI/textbox.h"
#include "../load.h"
#include "../misc_structs.h"
#include "../vars.h"


//Maximum zoom level possible in the editor.
const float animation_editor::ZOOM_MAX_LEVEL_EDITOR = 32.0f;
//Minimum zoom level possible in the editor.
const float animation_editor::ZOOM_MIN_LEVEL_EDITOR = 0.05f;

const string animation_editor::DELETE_ICON = "Delete.png";
const string animation_editor::DUPLICATE_ICON = "Duplicate.png";
const string animation_editor::EXIT_ICON = "Exit.png";
const string animation_editor::HITBOXES_ICON = "Hitboxes.png";
const string animation_editor::LOAD_ICON = "Load.png";
const string animation_editor::MOVE_LEFT_ICON = "Move_left.png";
const string animation_editor::MOVE_RIGHT_ICON = "Move_right.png";
const string animation_editor::NEW_ICON = "New.png";
const string animation_editor::NEXT_ICON = "Next.png";
const string animation_editor::PLAY_PAUSE_ICON = "Play_pause.png";
const string animation_editor::PREVIOUS_ICON = "Previous.png";
const string animation_editor::SAVE_ICON = "Save.png";


/* ----------------------------------------------------------------------------
 * Initializes animation editor class stuff.
 */
animation_editor::animation_editor() :
    editor(),
    anim_playing(false),
    comparison(true),
    comparison_sprite(nullptr),
    comparison_blink(true),
    comparison_blink_show(true),
    comparison_blink_timer(0),
    cur_anim(NULL),
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
    hitboxes_visible(true),
    is_pikmin(false),
    sprite_tra_lmb_action(LMB_ACTION_MOVE),
    top_lmb_action(LMB_ACTION_MOVE) {
    
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
 * Handles the logic part of the main loop of the animation editor.
 */
void animation_editor::do_logic() {

    update_transformations();
    
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
        animation_to_gui();
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
    
    al_use_transform(&world_to_screen_transform);
    
    al_set_clipping_rectangle(
        0, 0, gui_x, status_bar_y
    ); {
    
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
            mode == EDITOR_MODE_SPRITE_TRANSFORM
        ) {
            s = cur_sprite;
            
        }
        
        if(s) {
            if(s->bitmap) {
                draw_sprite(s->bitmap, s->offset, s->game_size);
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
                        h_ptr->pos.x,
                        h_ptr->pos.y,
                        h_ptr->radius,
                        hitbox_color
                    );
                    
                    al_draw_circle(
                        h_ptr->pos.x,
                        h_ptr->pos.y,
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
                    s->top_pos, s->top_size,
                    s->top_angle
                );
            }
            
            if(
                comparison && comparison_blink_show &&
                comparison_sprite && comparison_sprite->bitmap
            ) {
                draw_sprite(
                    comparison_sprite->bitmap,
                    comparison_sprite->offset, comparison_sprite->game_size,
                    0
                );
            }
        }
        
        if(hitboxes_visible) {
            point cam_top_left_corner(0, 0);
            point cam_bottom_right_corner(gui_x, status_bar_y);
            al_transform_coordinates(
                &screen_to_world_transform,
                &cam_top_left_corner.x, &cam_top_left_corner.y
            );
            al_transform_coordinates(
                &screen_to_world_transform,
                &cam_bottom_right_corner.x, &cam_bottom_right_corner.y
            );
            
            al_draw_line(
                0, cam_top_left_corner.y, 0, cam_bottom_right_corner.y,
                al_map_rgb(240, 240, 240), 1 / cam_zoom
            );
            al_draw_line(
                cam_top_left_corner.x, 0, cam_bottom_right_corner.x, 0,
                al_map_rgb(240, 240, 240), 1 / cam_zoom
            );
        }
        
    } al_reset_clipping_rectangle();
    
    al_use_transform(&identity_transform);
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Returns a file path, but cropped to fit on the gui's buttons.
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
void animation_editor::animation_to_gui() {
    lafi::widget* f = gui->widgets["frm_anims"];
    
    ((lafi::button*) f->widgets["but_anim"])->text =
        cur_anim ? cur_anim->name : "";
        
    if(!cur_anim) {
        f->widgets["frm_anim"]->hide();
    } else {
        f->widgets["frm_anim"]->show();
        
        ((lafi::button*) f->widgets["but_anim"])->text = cur_anim->name;
        f = f->widgets["frm_anim"];
        ((lafi::textbox*) f->widgets["txt_loop"])->text =
            i2s(cur_anim->loop_frame + 1);
            
        if(cur_anim->hit_rate == 100) {
            ((lafi::checkbox*) f->widgets["chk_missable"])->uncheck();
            f->widgets["lbl_hit_rate"]->hide();
            f->widgets["txt_hit_rate"]->hide();
            f->widgets["lbl_hit_rate_p"]->hide();
            ((lafi::textbox*) f->widgets["txt_hit_rate"])->text = "100";
            
        } else {
            ((lafi::checkbox*) f->widgets["chk_missable"])->check();
            f->widgets["lbl_hit_rate"]->show();
            f->widgets["txt_hit_rate"]->show();
            f->widgets["lbl_hit_rate_p"]->show();
            ((lafi::textbox*) f->widgets["txt_hit_rate"])->text =
                i2s(cur_anim->hit_rate);
                
        }
        
        frame_to_gui();
    }
}


/* ----------------------------------------------------------------------------
 * Loads the body part's data onto the gui.
 */
void animation_editor::body_part_to_gui() {
    lafi::frame* f = (lafi::frame*) gui->widgets["frm_body_parts"];
    
    ((lafi::label*) f->widgets["lbl_nr"])->text =
        (
            anims.body_parts.empty() ?
            "--/0" :
            (string) (
                i2s(cur_body_part_nr + 1) + "/" +
                i2s(anims.body_parts.size())
            )
        );
        
    if(anims.body_parts.empty()) {
        f->widgets["frm_body_part"]->hide();
        return;
    }
    
    f = (lafi::frame*) f->widgets["frm_body_part"];
    f->show();
    
    ((lafi::textbox*) f->widgets["txt_name"])->text =
        anims.body_parts[cur_body_part_nr]->name;
}


/* ----------------------------------------------------------------------------
 * Loads the frame's data from memory to the gui.
 */
void animation_editor::frame_to_gui() {
    lafi::widget* f = gui->widgets["frm_anims"]->widgets["frm_anim"];
    bool valid = cur_frame_nr != INVALID && cur_anim;
    
    ((lafi::label*) f->widgets["lbl_f_nr"])->text =
        "Current frame: " +
        (valid ? i2s((cur_frame_nr + 1)) : "--") +
        " / " + i2s(cur_anim->frames.size());
        
    f = f->widgets["frm_frame"];
    if(!valid) {
        f->hide();
    } else {
        f->show();
        
        ((lafi::button*) f->widgets["but_sprite"])->text =
            cur_anim->frames[cur_frame_nr].sprite_name;
        ((lafi::textbox*) f->widgets["txt_dur"])->text =
            f2s(cur_anim->frames[cur_frame_nr].duration);
            
        if(cur_anim->frames[cur_frame_nr].signal != INVALID) {
            ((lafi::checkbox*) f->widgets["chk_signal"])->check();
            f->widgets["txt_signal"]->show();
            ((lafi::textbox*) f->widgets["txt_signal"])->text =
                i2s(cur_anim->frames[cur_frame_nr].signal);
        } else {
            ((lafi::checkbox*) f->widgets["chk_signal"])->uncheck();
            f->widgets["txt_signal"]->hide();
            ((lafi::textbox*) f->widgets["txt_signal"])->text = "0";
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Loads the hitbox's data from memory to the gui.
 */
void animation_editor::hitbox_to_gui() {
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
        ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(cur_h->pos.x);
        ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(cur_h->pos.y);
        ((lafi::textbox*) f->widgets["txt_z"])->text = f2s(cur_h->z);
        ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(cur_h->height);
        ((lafi::textbox*) f->widgets["txt_r"])->text = f2s(cur_h->radius);
    }
    
    open_hitbox_type(cur_h ? cur_h->type : 255);
    
    if(cur_h) {
        f->show();
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
        f->hide();
    }
}


/* ----------------------------------------------------------------------------
 * Loads the frame's data from memory to the gui.
 */
void animation_editor::sprite_to_gui() {
    lafi::widget* f = gui->widgets["frm_sprites"];
    
    ((lafi::button*) f->widgets["but_sprite"])->text =
        cur_sprite ? cur_sprite->name : "";
        
    if(!cur_sprite) {
        f->widgets["frm_sprite"]->hide();
    } else {
        f->widgets["frm_sprite"]->show();
        
        f = f->widgets["frm_sprite"];
        
        ((lafi::textbox*) f->widgets["txt_file"])->text =
            cur_sprite->file;
        ((lafi::textbox*) f->widgets["txt_filex"])->text =
            i2s(cur_sprite->file_pos.x);
        ((lafi::textbox*) f->widgets["txt_filey"])->text =
            i2s(cur_sprite->file_pos.y);
        ((lafi::textbox*) f->widgets["txt_filew"])->text =
            i2s(cur_sprite->file_size.x);
        ((lafi::textbox*) f->widgets["txt_fileh"])->text =
            i2s(cur_sprite->file_size.y);
        ((lafi::textbox*) f->widgets["txt_gamew"])->text =
            f2s(cur_sprite->game_size.x);
        ((lafi::textbox*) f->widgets["txt_gameh"])->text =
            f2s(cur_sprite->game_size.y);
        ((lafi::textbox*) f->widgets["txt_offsx"])->text =
            f2s(cur_sprite->offset.x);
        ((lafi::textbox*) f->widgets["txt_offsy"])->text =
            f2s(cur_sprite->offset.y);
            
        if(is_pikmin) {
            enable_widget(f->widgets["but_top"]);
        } else {
            disable_widget(f->widgets["but_top"]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the sprite transformation's data from memory to the gui.
 */
void animation_editor::sprite_transform_to_gui() {
    lafi::widget* f = gui->widgets["frm_sprite_tra"];
    
    ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(cur_sprite->offset.x);
    ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(cur_sprite->offset.y);
    ((lafi::textbox*) f->widgets["txt_w"])->text = f2s(cur_sprite->game_size.x);
    ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(cur_sprite->game_size.y);
    ((lafi::checkbox*) f->widgets["chk_compare"])->set(comparison);
    ((lafi::checkbox*) f->widgets["chk_compare_blink"])->set(
        comparison_blink
    );
    ((lafi::button*) f->widgets["but_compare"])->text =
        (comparison_sprite ? comparison_sprite->name : "");
}


/* ----------------------------------------------------------------------------
 * Loads the Pikmin top's data onto the gui.
 */
void animation_editor::top_to_gui() {
    lafi::widget* f = gui->widgets["frm_top"];
    
    lafi::checkbox* c = (lafi::checkbox*) f->widgets["chk_visible"];
    if(cur_sprite->top_visible) c->check();
    else c->uncheck();
    
    ((lafi::textbox*) f->widgets["txt_x"])->text = f2s(cur_sprite->top_pos.x);
    ((lafi::textbox*) f->widgets["txt_y"])->text = f2s(cur_sprite->top_pos.y);
    ((lafi::textbox*) f->widgets["txt_w"])->text = f2s(cur_sprite->top_size.x);
    ((lafi::textbox*) f->widgets["txt_h"])->text = f2s(cur_sprite->top_size.y);
    (
        (lafi::angle_picker*) f->widgets["ang_angle"]
    )->set_angle_rads(cur_sprite->top_angle);
}


/* ----------------------------------------------------------------------------
 * Saves the animation's data to memory using info on the gui.
 */
void animation_editor::gui_to_animation() {
    if(!cur_anim) return;
    
    lafi::widget* f = gui->widgets["frm_anims"]->widgets["frm_anim"];
    
    cur_anim->loop_frame =
        s2i(((lafi::textbox*) f->widgets["txt_loop"])->text) - 1;
    if(cur_anim->loop_frame >= cur_anim->frames.size()) {
        cur_anim->loop_frame = 0;
    }
    if(((lafi::checkbox*) f->widgets["chk_missable"])->checked) {
        cur_anim->hit_rate =
            s2i(((lafi::textbox*) f->widgets["txt_hit_rate"])->text);
        cur_anim->hit_rate = clamp(cur_anim->hit_rate, 0, 100);
    } else {
        cur_anim->hit_rate = 100;
    }
    
    gui_to_frame();
    animation_to_gui();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the body part's data from the gui.
 */
void animation_editor::gui_to_body_part() {
    body_part_to_gui();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the frame's data to memory using info on the gui.
 */
void animation_editor::gui_to_frame() {
    bool valid = cur_frame_nr != INVALID && cur_anim;
    if(!valid) return;
    
    lafi::widget* fw = gui->widgets["frm_anims"]->widgets["frm_anim"];
    fw = fw->widgets["frm_frame"];
    
    frame* f = &cur_anim->frames[cur_frame_nr];
    f->duration =
        s2f(
            (
                (lafi::textbox*)
                fw->widgets["txt_dur"]
            )->text
        );
    if(f->duration < 0) f->duration = 0;
    
    if(((lafi::checkbox*) fw->widgets["chk_signal"])->checked) {
        f->signal = s2i(((lafi::textbox*) fw->widgets["txt_signal"])->text);
    } else {
        f->signal = INVALID;
    }
    
    frame_to_gui();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the hitbox's data to memory using info on the gui.
 */
void animation_editor::gui_to_hitbox() {
    bool valid = cur_hitbox_nr != INVALID && cur_sprite;
    if(!valid) return;
    
    lafi::widget* f = gui->widgets["frm_hitboxes"]->widgets["frm_hitbox"];
    
    hitbox* h = &cur_sprite->hitboxes[cur_hitbox_nr];
    
    h->pos.x = s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    h->pos.y = s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    h->z = s2f(((lafi::textbox*) f->widgets["txt_z"])->text);
    
    h->height = s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    h->radius = s2f(((lafi::textbox*) f->widgets["txt_r"])->text);
    if(h->radius <= 0) h->radius = 16;
    
    hitbox* cur_h =
        &cur_sprite->hitboxes[cur_hitbox_nr];
        
    if(((lafi::radio_button*) f->widgets["rad_normal"])->selected) {
        cur_h->type = HITBOX_TYPE_NORMAL;
    } else if(((lafi::radio_button*) f->widgets["rad_attack"])->selected) {
        cur_h->type = HITBOX_TYPE_ATTACK;
    } else {
        cur_h->type = HITBOX_TYPE_DISABLED;
    }
    
    if(cur_h->type == HITBOX_TYPE_NORMAL) {
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
            
    } else if(cur_h->type == HITBOX_TYPE_ATTACK) {
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
    
    hitbox_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the sprite's data to memory using info on the gui.
 */
void animation_editor::gui_to_sprite() {
    if(!cur_sprite) return;
    
    lafi::widget* f = gui->widgets["frm_sprites"]->widgets["frm_sprite"];
    
    string new_file;
    point new_f_pos, new_f_size;
    
    new_file =
        ((lafi::textbox*) f->widgets["txt_file"])->text;
    new_f_pos.x =
        s2i(((lafi::textbox*) f->widgets["txt_filex"])->text);
    new_f_pos.y =
        s2i(((lafi::textbox*) f->widgets["txt_filey"])->text);
    new_f_size.x =
        s2i(((lafi::textbox*) f->widgets["txt_filew"])->text);
    new_f_size.y =
        s2i(((lafi::textbox*) f->widgets["txt_fileh"])->text);
    cur_sprite->game_size.x =
        s2f(((lafi::textbox*) f->widgets["txt_gamew"])->text);
    cur_sprite->game_size.y =
        s2f(((lafi::textbox*) f->widgets["txt_gameh"])->text);
    cur_sprite->offset.x =
        s2f(((lafi::textbox*) f->widgets["txt_offsx"])->text);
    cur_sprite->offset.y =
        s2f(((lafi::textbox*) f->widgets["txt_offsy"])->text);
        
    //Automatically fill in the in-game width/height if it hasn't been set yet.
    if(cur_sprite->game_size.x == 0.0f) {
        cur_sprite->game_size.x = new_f_size.x;
    }
    if(cur_sprite->game_size.y == 0.0f) {
        cur_sprite->game_size.y = new_f_size.y;
    }
    
    if(
        cur_sprite->file != new_file ||
        cur_sprite->file_pos.x != new_f_pos.x ||
        cur_sprite->file_pos.y != new_f_pos.y ||
        cur_sprite->file_size.x != new_f_size.x ||
        cur_sprite->file_size.y != new_f_size.y
    ) {
        //Changed something image-wise. Recreate it.
        if(cur_sprite->parent_bmp) bitmaps.detach(cur_sprite->file);
        if(cur_sprite->bitmap) al_destroy_bitmap(cur_sprite->bitmap);
        cur_sprite->bitmap = NULL;
        cur_sprite->parent_bmp = bitmaps.get(new_file, NULL);
        if(cur_sprite->parent_bmp) {
            cur_sprite->bitmap =
                al_create_sub_bitmap(
                    cur_sprite->parent_bmp, new_f_pos.x,
                    new_f_pos.y, new_f_size.x, new_f_size.y
                );
        }
        cur_sprite->file = new_file;
        cur_sprite->file_pos = new_f_pos;
        cur_sprite->file_size = new_f_size;
    }
    
    last_file_used = new_file;
    
    gui_to_hitbox();
    sprite_to_gui();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the sprite's transform data to memory using info on the gui.
 */
void animation_editor::gui_to_sprite_transform() {
    lafi::widget* f = gui->widgets["frm_sprite_tra"];
    
    cur_sprite->offset.x =
        s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_sprite->offset.y =
        s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_sprite->game_size.x =
        s2f(((lafi::textbox*) f->widgets["txt_w"])->text);
    cur_sprite->game_size.y =
        s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    comparison =
        ((lafi::checkbox*) f->widgets["chk_compare"])->checked;
    comparison_blink =
        ((lafi::checkbox*) f->widgets["chk_compare_blink"])->checked;
        
    sprite_tra_lmb_action = LMB_ACTION_NONE;
    if(((lafi::checkbox*) f->widgets["chk_mousexy"])->checked) {
        sprite_tra_lmb_action = LMB_ACTION_MOVE;
    } else if(((lafi::checkbox*) f->widgets["chk_mousewh"])->checked) {
        sprite_tra_lmb_action = LMB_ACTION_RESIZE;
    }
    
    sprite_transform_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the Pikmin top's data to memory using info on the gui.
 */
void animation_editor::gui_to_top() {
    lafi::widget* f = gui->widgets["frm_top"];
    
    cur_sprite->top_visible =
        ((lafi::checkbox*) f->widgets["chk_visible"])->checked;
    cur_sprite->top_pos.x =
        s2f(((lafi::textbox*) f->widgets["txt_x"])->text);
    cur_sprite->top_pos.y =
        s2f(((lafi::textbox*) f->widgets["txt_y"])->text);
    cur_sprite->top_size.x =
        s2f(((lafi::textbox*) f->widgets["txt_w"])->text);
    cur_sprite->top_size.y =
        s2f(((lafi::textbox*) f->widgets["txt_h"])->text);
    cur_sprite->top_angle =
        ((lafi::angle_picker*) f->widgets["ang_angle"])->get_angle_rads();
        
    top_lmb_action = LMB_ACTION_NONE;
    if(((lafi::checkbox*) f->widgets["chk_mousexy"])->checked) {
        top_lmb_action = LMB_ACTION_MOVE;
    } else if(((lafi::checkbox*) f->widgets["chk_mousewh"])->checked) {
        top_lmb_action = LMB_ACTION_RESIZE;
    } else if(((lafi::checkbox*) f->widgets["chk_mousea"])->checked) {
        top_lmb_action = LMB_ACTION_ROTATE;
    }
    
    top_to_gui();
    made_changes = true;
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
    gui->widgets["frm_hitboxes"]->hide();
    gui->widgets["frm_top"]->hide();
    
    cam_pos.x = cam_pos.y = 0;
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
    for(unsigned char t = 0; t < N_MATURITIES; ++t) {
        if(top_bmp[t] && top_bmp[t] != bmp_error) {
            al_destroy_bitmap(top_bmp[t]);
            top_bmp[t] = NULL;
        }
    }
    
    if(file_path.find(PIKMIN_FOLDER_PATH) != string::npos) {
        is_pikmin = true;
        data_node data =
            load_data_file(
                PIKMIN_FOLDER_PATH + "/" +
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
    
    mode = EDITOR_MODE_MAIN;
    change_to_right_frame();
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
    
    f->widgets["frm_normal"]->hide();
    f->widgets["frm_attack"]->hide();
    
    if(type == HITBOX_TYPE_NORMAL) {
        ((lafi::radio_button*) f->widgets["rad_normal"])->select();
        f->widgets["frm_normal"]->show();
    } else if(type == HITBOX_TYPE_ATTACK) {
        ((lafi::radio_button*) f->widgets["rad_attack"])->select();
        f->widgets["frm_attack"]->show();
    } else {
        ((lafi::radio_button*) f->widgets["rad_disabled"])->select();
    }
}


/* ----------------------------------------------------------------------------
 * Opens the frame where you pick from a list.
 * For the type of content, use animation_editor::ANIMATION_EDITOR_PICKER_*.
 */
void animation_editor::open_picker(
    const unsigned char type, const bool can_make_new
) {
    vector<pair<string, string> > elements;
    string title;
    
    picker_type = type;
    
    if(type == ANIMATION_EDITOR_PICKER_ANIMATION) {
        for(size_t a = 0; a < anims.animations.size(); ++a) {
            elements.push_back(make_pair("", anims.animations[a]->name));
        }
        title = "Choose an animation.";
    } else if(type == ANIMATION_EDITOR_PICKER_SPRITE) {
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            elements.push_back(make_pair("", anims.sprites[s]->name));
        }
        title = "Choose a sprite.";
    }
    
    generate_and_open_picker(elements, title, can_make_new);
}


/* ----------------------------------------------------------------------------
 * Picks an item and closes the list picker frame.
 */
void animation_editor::pick(const string &name, const string &category) {
    if(picker_type == ANIMATION_EDITOR_PICKER_ANIMATION) {
        if(mode == EDITOR_MODE_TOOLS) {
            (
                (lafi::button*)
                gui->widgets["frm_tools"]->widgets["but_rename_anim_name"]
            )->text = name;
        } else {
            cur_anim = anims.animations[anims.find_animation(name)];
            cur_frame_nr =
                (cur_anim->frames.size()) ? 0 : INVALID;
            cur_hitbox_nr = INVALID;
            animation_to_gui();
        }
        
    } else if(picker_type == ANIMATION_EDITOR_PICKER_SPRITE) {
        if(mode == EDITOR_MODE_ANIMATION) {
            cur_anim->frames[cur_frame_nr].sprite_name =
                name;
            cur_anim->frames[cur_frame_nr].sprite_ptr =
                anims.sprites[anims.find_sprite(name)];
            frame_to_gui();
        } else if(mode == EDITOR_MODE_SPRITE_TRANSFORM) {
            comparison_sprite = anims.sprites[anims.find_sprite(name)];
            sprite_transform_to_gui();
        } else if(mode == EDITOR_MODE_TOOLS) {
            (
                (lafi::button*)
                gui->widgets["frm_tools"]->widgets["but_rename_sprite_name"]
            )->text = name;
        } else if(mode == EDITOR_MODE_HITBOXES) {
            for(size_t s = 0; s < anims.sprites.size(); ++s) {
                if(anims.sprites[s]->name == name) {
                    cur_sprite->hitboxes = anims.sprites[s]->hitboxes;
                }
            }
            cur_hitbox_nr = 0;
            hitbox_to_gui();
        } else {
            cur_sprite = anims.sprites[anims.find_sprite(name)];
            cur_hitbox_nr = INVALID;
            if(cur_sprite->file.empty()) {
                //New frame. Suggest file name.
                cur_sprite->file = last_file_used;
            }
            sprite_to_gui();
        }
        
    }
    
    show_bottom_frame();
    change_to_right_frame();
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
            show_bottom_frame();
            change_to_right_frame();
            
            update_animation_editor_history(name);
            save_options(); //Save the history on the options.
        };
        b->left_mouse_click_handler = lambda;
        f->easy_add("but_" + i2s(h), b, 100, 32);
        f->easy_row();
    }
}


/* ----------------------------------------------------------------------------
 * Renames the chosen animation to the chosen name, in the "tools" menu.
 */
void animation_editor::rename_animation() {
    lafi::button* but_ptr =
        (
            (lafi::button*)
            gui->widgets["frm_tools"]->widgets["but_rename_anim_name"]
        );
    lafi::textbox* txt_ptr =
        (
            (lafi::textbox*)
            gui->widgets["frm_tools"]->widgets["txt_rename_anim"]
        );
    size_t old_anim_id = INVALID;
    string old_name = but_ptr->text;
    string new_name = txt_ptr->text;
    
    if(new_name.empty()) return;
    
    //Check if the name already exists.
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        if(anims.animations[a]->name == old_name) old_anim_id = a;
        if(anims.animations[a]->name == new_name) return;
    }
    
    if(old_anim_id == INVALID) return;
    
    anims.animations[old_anim_id]->name = new_name;
    
    but_ptr->text = "";
    txt_ptr->text = "";
    
}


/* ----------------------------------------------------------------------------
 * Renames the chosen sprite to the chosen name, in the "tools" menu.
 */
void animation_editor::rename_sprite() {
    lafi::button* but_ptr =
        (
            (lafi::button*)
            gui->widgets["frm_tools"]->widgets["but_rename_sprite_name"]
        );
    lafi::textbox* txt_ptr =
        (
            (lafi::textbox*)
            gui->widgets["frm_tools"]->widgets["txt_rename_sprite"]
        );
    size_t old_sprite_id = INVALID;
    string old_name = but_ptr->text;
    string new_name = txt_ptr->text;
    
    if(new_name.empty()) return;
    
    //Check if the name already exists.
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        if(anims.sprites[s]->name == old_name) old_sprite_id = s;
        if(anims.sprites[s]->name == new_name) return;
    }
    
    if(old_sprite_id == INVALID) return;
    
    anims.sprites[old_sprite_id]->name = new_name;
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        animation* a_ptr = anims.animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); ++f) {
            if(a_ptr->frames[f].sprite_name == old_name) {
                a_ptr->frames[f].sprite_name = new_name;
            }
        }
    }
    
    but_ptr->text = "";
    txt_ptr->text = "";
    
}


/* ----------------------------------------------------------------------------
 * Resizes all sprite game-width/height by a factor compared
 * to the respective file-width/height.
 */
void animation_editor::resize_by_resolution() {
    lafi::textbox* txt_resize =
        (lafi::textbox*) gui->widgets["frm_tools"]->widgets["txt_resolution"];
    float mult = s2f(txt_resize->text);
    if(mult == 0) return;
    mult = 1.0 / mult;
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        sprite* s_ptr = anims.sprites[s];
        s_ptr->game_size = s_ptr->file_size * mult;
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
        
        s_ptr->game_size *= mult;
        s_ptr->offset    *= mult;
        s_ptr->top_pos   *= mult;
        s_ptr->top_size  *= mult;
        
        for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
            hitbox* h_ptr = &s_ptr->hitboxes[h];
            
            h_ptr->radius *= mult;
            h_ptr->pos    *= mult;
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
        if(anims.animations[a]->hit_rate != 100) {
            anim_node->add(
                new data_node("hit_rate", i2s(anims.animations[a]->hit_rate))
            );
        }
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
            if(f_ptr->signal != INVALID) {
                frame_node->add(
                    new data_node("signal", i2s(f_ptr->signal))
                );
            }
        }
    }
    
    data_node* sprites_node = new data_node("sprites", "");
    file_node.add(sprites_node);
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        sprite* s_ptr = anims.sprites[s];
        data_node* sprite_node = new data_node(anims.sprites[s]->name, "");
        sprites_node->add(sprite_node);
        
        sprite_node->add(new data_node("file",      s_ptr->file));
        sprite_node->add(new data_node("file_pos",  p2s(s_ptr->file_pos)));
        sprite_node->add(new data_node("file_size", p2s(s_ptr->file_size)));
        sprite_node->add(new data_node("game_size", p2s(s_ptr->game_size)));
        sprite_node->add(new data_node("offset",    p2s(s_ptr->offset)));
        
        if(is_pikmin) {
            sprite_node->add(
                new data_node("top_visible", b2s(s_ptr->top_visible))
            );
            sprite_node->add(
                new data_node("top_pos", p2s(s_ptr->top_pos))
            );
            sprite_node->add(
                new data_node("top_size", p2s(s_ptr->top_size))
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
                    f2s(h_ptr->pos.x) + " " + f2s(h_ptr->pos.y) +
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
        data_node* body_part_node =
            new data_node(anims.body_parts[b]->name, "");
        body_parts_node->add(body_part_node);
        
    }
    
    file_node.save_file(file_path);
    made_changes = false;
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void animation_editor::unload() {
    anims.destroy();
    delete(gui->style);
    //TODO warning: deleting object of polymorphic class type 'lafi::gui'
    //which has non-virtual destructor might cause undefined behaviour
    //[-Wdelete-non-virtual-dtor]
    delete(gui);
    al_destroy_native_file_dialog(file_dialog);
    
    unload_hazards();
    unload_status_types(false);
    
    icons.clear();
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
        f->hide();
    } else {
        f->show();
    }
    
    ((lafi::label*) f->widgets["lbl_n_anims"])->text =
        "Animations: " + i2s(anims.animations.size());
    ((lafi::label*) f->widgets["lbl_n_sprites"])->text =
        "Sprites: " + i2s(anims.sprites.size());
    ((lafi::label*) f->widgets["lbl_n_body_parts"])->text =
        "Body parts: " + i2s(anims.body_parts.size());
}


/* ----------------------------------------------------------------------------
 * Creates a new item from the picker frame, given its name.
 */
void animation_editor::create_new_from_picker(const string &name) {
    if(mode == EDITOR_MODE_ANIMATION) {
        if(anims.find_animation(name) != INVALID) return;
        anims.animations.push_back(new animation(name));
        pick(name, "");
        
    } else if(mode == EDITOR_MODE_SPRITE) {
        if(anims.find_sprite(name) != INVALID) return;
        anims.sprites.push_back(new sprite(name));
        anims.sprites.back()->create_hitboxes(&anims);
        pick(name, "");
        
    }
}


/* ----------------------------------------------------------------------------
 * Hides all menu frames.
 */
void animation_editor::hide_all_frames() {
    gui->widgets["frm_main"]->hide();
    gui->widgets["frm_picker"]->hide();
    gui->widgets["frm_history"]->hide();
    gui->widgets["frm_anims"]->hide();
    gui->widgets["frm_sprites"]->hide();
    gui->widgets["frm_sprite_tra"]->hide();
    gui->widgets["frm_hitboxes"]->hide();
    gui->widgets["frm_top"]->hide();
    gui->widgets["frm_body_parts"]->hide();
    gui->widgets["frm_tools"]->hide();
}


/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor mode.
 */
void animation_editor::change_to_right_frame() {
    hide_all_frames();
    
    if(mode == EDITOR_MODE_MAIN) {
        gui->widgets["frm_main"]->show();
    } else if(mode == EDITOR_MODE_ANIMATION) {
        gui->widgets["frm_anims"]->show();
    } else if(mode == EDITOR_MODE_SPRITE) {
        gui->widgets["frm_sprites"]->show();
    } else if(mode == EDITOR_MODE_BODY_PART) {
        gui->widgets["frm_body_parts"]->show();
    } else if(mode == EDITOR_MODE_HITBOXES) {
        gui->widgets["frm_hitboxes"]->show();
    } else if(mode == EDITOR_MODE_SPRITE_TRANSFORM) {
        gui->widgets["frm_sprite_tra"]->show();
    } else if(mode == EDITOR_MODE_TOP) {
        gui->widgets["frm_top"]->show();
    } else if(mode == EDITOR_MODE_HISTORY) {
        gui->widgets["frm_history"]->show();
    } else if(mode == EDITOR_MODE_TOOLS) {
        gui->widgets["frm_tools"]->show();
    }
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void animation_editor::update_transformations() {
    //World coordinates to screen coordinates.
    world_to_screen_transform = identity_transform;
    al_translate_transform(
        &world_to_screen_transform,
        -cam_pos.x + gui_x / 2.0 / cam_zoom,
        -cam_pos.y + status_bar_y / 2.0 / cam_zoom
    );
    al_scale_transform(&world_to_screen_transform, cam_zoom, cam_zoom);
    
    //Screen coordinates to world coordinates.
    screen_to_world_transform = world_to_screen_transform;
    al_invert_transform(&screen_to_world_transform);
}
