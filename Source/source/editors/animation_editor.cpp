/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
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

const string animation_editor::ICON_DELETE = "Delete.png";
const string animation_editor::ICON_DUPLICATE = "Duplicate.png";
const string animation_editor::ICON_EXIT = "Exit.png";
const string animation_editor::ICON_HITBOXES = "Hitboxes.png";
const string animation_editor::ICON_INFO = "Info.png";
const string animation_editor::ICON_LOAD = "Load.png";
const string animation_editor::ICON_MOVE_LEFT = "Move_left.png";
const string animation_editor::ICON_MOVE_RIGHT = "Move_right.png";
const string animation_editor::ICON_NEW = "New.png";
const string animation_editor::ICON_NEXT = "Next.png";
const string animation_editor::ICON_PLAY_PAUSE = "Play_pause.png";
const string animation_editor::ICON_PREVIOUS = "Previous.png";
const string animation_editor::ICON_SAVE = "Save.png";


/* ----------------------------------------------------------------------------
 * Initializes animation editor class stuff.
 */
animation_editor::animation_editor() :
    anim_playing(false),
    comparison(false),
    comparison_sprite(nullptr),
    comparison_blink(true),
    comparison_blink_show(true),
    comparison_blink_timer(0),
    comparison_above(true),
    comparison_tint(true),
    cur_anim(NULL),
    cur_body_part_nr(INVALID),
    cur_frame_nr(INVALID),
    cur_frame_time(0),
    cur_hitbox_alpha(0),
    cur_hitbox_nr(INVALID),
    cur_maturity(0),
    cur_sprite(NULL),
    grabbing_hitbox(INVALID),
    grabbing_hitbox_edge(false),
    hitboxes_visible(true),
    is_pikmin(false) {
    
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
    
    cur_sprite_tc.keep_aspect_ratio = true;
    top_tc.keep_aspect_ratio = true;
    top_tc.allow_rotation = true;
    
    zoom_min_level = ZOOM_MIN_LEVEL_EDITOR;
    zoom_max_level = ZOOM_MAX_LEVEL_EDITOR;
}


/* ----------------------------------------------------------------------------
 * Adds the current sprite's transformation controller data to the GUI.
 */
void animation_editor::cur_sprite_tc_to_gui() {
    set_textbox_text(
        frm_sprite_tra, "txt_x", f2s(cur_sprite_tc.get_center().x)
    );
    set_textbox_text(
        frm_sprite_tra, "txt_y", f2s(cur_sprite_tc.get_center().y)
    );
    set_textbox_text(
        frm_sprite_tra, "txt_w", f2s(cur_sprite_tc.get_size().x)
    );
    set_textbox_text(
        frm_sprite_tra, "txt_h", f2s(cur_sprite_tc.get_size().y)
    );
    gui_to_sprite_transform();
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the animation editor.
 */
void animation_editor::do_logic() {
    editor::do_logic();
    
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
    
}


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the animation editor.
 */
void animation_editor::do_drawing() {

    gui->draw();
    
    al_use_transform(&world_to_screen_transform);
    
    al_set_clipping_rectangle(
        0, 0, gui_x, status_bar_y
    );
    
    al_clear_to_color(al_map_rgb(128, 144, 128));
    
    sprite* s = NULL;
    bool draw_hitboxes = hitboxes_visible;
    
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
        mode == EDITOR_MODE_SPRITE_BITMAP ||
        mode == EDITOR_MODE_SPRITE_TRANSFORM
    ) {
        s = cur_sprite;
        
    }
    
    if(mode == EDITOR_MODE_SPRITE_TRANSFORM || mode == EDITOR_MODE_TOP) {
        draw_hitboxes = false;
    }
    
    if(mode == EDITOR_MODE_SPRITE_BITMAP && s && s->parent_bmp) {
    
        draw_hitboxes = false;
        
        int bmp_w = al_get_bitmap_width(s->parent_bmp);
        int bmp_h = al_get_bitmap_height(s->parent_bmp);
        int bmp_x = -bmp_w / 2.0;
        int bmp_y = -bmp_h / 2.0;
        al_draw_bitmap(s->parent_bmp, bmp_x, bmp_y, 0);
        
        point scene_tl = point(-1, -1);
        point scene_br = point(gui_x + 1, status_bar_y + 1);
        al_transform_coordinates(
            &screen_to_world_transform, &scene_tl.x, &scene_tl.y
        );
        al_transform_coordinates(
            &screen_to_world_transform, &scene_br.x, &scene_br.y
        );
        
        for(unsigned char x = 0; x < 3; ++x) {
            point rec_tl, rec_br;
            if(x == 0) {
                rec_tl.x = scene_tl.x;
                rec_br.x = bmp_x + s->file_pos.x;
            } else if(x == 1) {
                rec_tl.x = bmp_x + s->file_pos.x;
                rec_br.x = bmp_x + s->file_pos.x + s->file_size.x;
            } else {
                rec_tl.x = bmp_x + s->file_pos.x + s->file_size.x;
                rec_br.x = scene_br.x;
            }
            for(unsigned char y = 0; y < 3; ++y) {
                if(x == 1 && y == 1) continue;
                if(y == 0) {
                    rec_tl.y = scene_tl.y;
                    rec_br.y = bmp_y + s->file_pos.y;
                } else if(y == 1) {
                    rec_tl.y = bmp_y + s->file_pos.y;
                    rec_br.y = bmp_y + s->file_pos.y + s->file_size.y;
                } else {
                    rec_tl.y = bmp_y + s->file_pos.y + s->file_size.y;
                    rec_br.y = scene_br.y;
                }
                
                al_draw_filled_rectangle(
                    rec_tl.x, rec_tl.y,
                    rec_br.x, rec_br.y,
                    al_map_rgba(0, 0, 0, 128)
                );
            }
        }
        
        if(s->file_size.x > 0 && s->file_size.y > 0) {
        
            unsigned char outline_alpha =
                255 * ((sin(cur_hitbox_alpha) / 2.0) + 0.5);
            al_draw_rectangle(
                bmp_x + s->file_pos.x + 0.5,
                bmp_y + s->file_pos.y + 0.5,
                bmp_x + s->file_pos.x + s->file_size.x - 0.5,
                bmp_y + s->file_pos.y + s->file_size.y - 0.5,
                al_map_rgba(224, 192, 0, outline_alpha), 1.0
            );
        }
        
    } else if(s) {
    
        if(!comparison_above) {
            draw_comparison();
        }
        
        if(s->bitmap) {
            ALLEGRO_COLOR tint;
            if(
                mode == EDITOR_MODE_SPRITE_TRANSFORM &&
                comparison && comparison_tint &&
                comparison_sprite && comparison_sprite->bitmap
            ) {
                tint = al_map_rgb(0, 128, 255);
            } else {
                tint = al_map_rgb(255, 255, 255);
            }
            draw_bitmap(s->bitmap, s->offset, s->game_size, 0, tint);
        }
        
        if(draw_hitboxes) {
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
            draw_bitmap(
                top_bmp[cur_maturity],
                s->top_pos, s->top_size,
                s->top_angle
            );
        }
        
        if(comparison_above) {
            draw_comparison();
        }
        
        if(mode == EDITOR_MODE_SPRITE_TRANSFORM) {
            cur_sprite_tc.draw_handles();
        } else if(
            mode == EDITOR_MODE_TOP &&
            cur_sprite && cur_sprite->top_visible
        ) {
            top_tc.draw_handles();
        }
        
    }
    
    if(draw_hitboxes) {
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
    
    al_reset_clipping_rectangle();
    
    al_use_transform(&identity_transform);
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Draws the comparison sprite on the canvas, all tinted and everything.
 */
void animation_editor::draw_comparison() {
    if(
        comparison && comparison_blink_show &&
        comparison_sprite && comparison_sprite->bitmap
    ) {
        ALLEGRO_COLOR tint;
        if(comparison_tint) {
            tint = al_map_rgb(255, 128, 0);
        } else {
            tint = al_map_rgb(255, 255, 255);
        }
        draw_bitmap(
            comparison_sprite->bitmap,
            comparison_sprite->offset, comparison_sprite->game_size,
            0, tint
        );
    }
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
    set_button_text(frm_anims, "but_anim", cur_anim ? cur_anim->name : "");
    
    if(!cur_anim) {
        frm_anim->hide();
    } else {
        frm_anim->show();
        
        set_button_text(frm_anims, "but_anim", cur_anim->name);
        set_textbox_text(frm_anim, "txt_loop", i2s(cur_anim->loop_frame + 1));
        
        if(cur_anim->hit_rate == 100) {
            set_checkbox_check(frm_anim, "chk_missable", false);
            frm_anim->widgets["lbl_hit_rate"]->hide();
            frm_anim->widgets["txt_hit_rate"]->hide();
            frm_anim->widgets["lbl_hit_rate_p"]->hide();
            set_textbox_text(frm_anim, "txt_hit_rate", "100");
            
        } else {
            set_checkbox_check(frm_anim, "chk_missable", true);
            frm_anim->widgets["lbl_hit_rate"]->show();
            frm_anim->widgets["txt_hit_rate"]->show();
            frm_anim->widgets["lbl_hit_rate_p"]->show();
            set_textbox_text(frm_anim, "txt_hit_rate", i2s(cur_anim->hit_rate));
            
        }
        
        frame_to_gui();
    }
}


/* ----------------------------------------------------------------------------
 * Loads the body part's data onto the gui.
 */
void animation_editor::body_part_to_gui() {
    set_label_text(
        frm_body_parts, "lbl_nr",
        (
            anims.body_parts.empty() ?
            "--/0" :
            (string) (
                i2s(cur_body_part_nr + 1) + "/" +
                i2s(anims.body_parts.size())
            )
        )
    );
    
    if(anims.body_parts.empty()) {
        frm_body_part->hide();
        return;
    }
    
    frm_body_part->show();
    
    set_textbox_text(
        frm_body_part, "txt_name", anims.body_parts[cur_body_part_nr]->name
    );
}


/* ----------------------------------------------------------------------------
 * Loads the frame's data from memory to the gui.
 */
void animation_editor::frame_to_gui() {
    bool valid = cur_frame_nr != INVALID && cur_anim;
    
    set_label_text(
        frm_anim, "lbl_f_nr",
        "Current frame: " +
        (valid ? i2s((cur_frame_nr + 1)) : "--") +
        " / " + i2s(cur_anim->frames.size())
    );
    
    if(!valid) {
        frm_frame->hide();
    } else {
        frm_frame->show();
        
        set_button_text(
            frm_frame, "but_sprite", cur_anim->frames[cur_frame_nr].sprite_name
        );
        set_textbox_text(
            frm_frame, "txt_dur", f2s(cur_anim->frames[cur_frame_nr].duration)
        );
        
        if(cur_anim->frames[cur_frame_nr].signal != INVALID) {
            set_checkbox_check(frm_frame, "chk_signal", true);
            frm_frame->widgets["txt_signal"]->show();
            set_textbox_text(
                frm_frame, "txt_signal",
                i2s(cur_anim->frames[cur_frame_nr].signal)
            );
        } else {
            set_checkbox_check(frm_frame, "chk_signal", false);
            frm_frame->widgets["txt_signal"]->hide();
            set_textbox_text(frm_frame, "txt_signal", "0");
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Loads the hitbox's data from memory to the gui.
 */
void animation_editor::hitbox_to_gui() {
    hitbox* cur_h = NULL;
    if(!cur_sprite->hitboxes.empty()) {
        cur_h = &cur_sprite->hitboxes[cur_hitbox_nr];
    }
    if(cur_h) {
        set_label_text(frm_hitboxes, "lbl_name", cur_h->body_part_name);
        set_textbox_text(frm_hitbox, "txt_x", f2s(cur_h->pos.x));
        set_textbox_text(frm_hitbox, "txt_y", f2s(cur_h->pos.y));
        set_textbox_text(frm_hitbox, "txt_z", f2s(cur_h->z));
        set_textbox_text(frm_hitbox, "txt_h", f2s(cur_h->height));
        set_textbox_text(frm_hitbox, "txt_r", f2s(cur_h->radius));
    }
    
    open_hitbox_type(cur_h ? cur_h->type : 255);
    
    if(cur_h) {
        frm_hitbox->show();
        if(cur_h->type == HITBOX_TYPE_NORMAL) {
            set_textbox_text(frm_normal_h, "txt_mult", f2s(cur_h->value));
            set_checkbox_check(
                frm_normal_h, "chk_latch", cur_h->can_pikmin_latch
            );
            set_textbox_text(frm_normal_h, "txt_hazards", cur_h->hazards_str);
            
        } else if(cur_h->type == HITBOX_TYPE_ATTACK) {
            set_textbox_text(frm_attack_h, "txt_value", f2s(cur_h->value));
            set_textbox_text(frm_attack_h, "txt_hazards", cur_h->hazards_str);
            set_checkbox_check(
                frm_attack_h, "chk_outward", cur_h->knockback_outward
            );
            set_angle_picker_angle(
                frm_attack_h, "ang_angle", cur_h->knockback_angle
            );
            set_textbox_text(
                frm_attack_h, "txt_knockback", f2s(cur_h->knockback)
            );
            set_textbox_text(
                frm_attack_h, "txt_wither", i2s(cur_h->wither_chance)
            );
            
            if(cur_h->knockback_outward) {
                disable_widget(frm_attack_h->widgets["ang_angle"]);
            } else {
                enable_widget(frm_attack_h->widgets["ang_angle"]);
            }
            
        }
    } else {
        frm_hitbox->hide();
    }
}


/* ----------------------------------------------------------------------------
 * Imports the sprite file data from a different sprite to the current.
 */
void animation_editor::import_sprite_file_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    set_textbox_text(frm_sprite_bmp, "txt_file", s->file);
    set_textbox_text(frm_sprite_bmp, "txt_x", i2s(s->file_pos.x));
    set_textbox_text(frm_sprite_bmp, "txt_y", i2s(s->file_pos.y));
    set_textbox_text(frm_sprite_bmp, "txt_w", i2s(s->file_size.x));
    set_textbox_text(frm_sprite_bmp, "txt_h", i2s(s->file_size.y));
    gui_to_sprite_bmp();
}


/* ----------------------------------------------------------------------------
 * Imports the sprite hitbox data from a different sprite to the current.
 */
void animation_editor::import_sprite_hitbox_data(const string &name) {
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        if(anims.sprites[s]->name == name) {
            cur_sprite->hitboxes = anims.sprites[s]->hitboxes;
        }
    }
    cur_hitbox_nr = 0;
    hitbox_to_gui();
}


/* ----------------------------------------------------------------------------
 * Imports the sprite top data from a different sprite to the current.
 */
void animation_editor::import_sprite_top_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    set_checkbox_check(frm_top, "chk_visible", s->top_visible);
    set_textbox_text(frm_top, "txt_x", i2s(s->top_pos.x));
    set_textbox_text(frm_top, "txt_y", i2s(s->top_pos.y));
    set_textbox_text(frm_top, "txt_w", i2s(s->top_size.x));
    set_textbox_text(frm_top, "txt_h", i2s(s->top_size.y));
    set_angle_picker_angle(frm_top, "ang_angle", s->top_angle);
    
    gui_to_top();
}


/* ----------------------------------------------------------------------------
 * Imports the sprite transformation data from
 * a different sprite to the current.
 */
void animation_editor::import_sprite_transformation_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    set_textbox_text(frm_sprite_tra, "txt_x", i2s(s->offset.x));
    set_textbox_text(frm_sprite_tra, "txt_y", i2s(s->offset.y));
    set_textbox_text(frm_sprite_tra, "txt_w", i2s(s->game_size.x));
    set_textbox_text(frm_sprite_tra, "txt_h", i2s(s->game_size.y));
    
    gui_to_sprite_transform();
}


/* ----------------------------------------------------------------------------
 * Loads the sprite's data from memory to the gui.
 */
void animation_editor::sprite_to_gui() {
    set_button_text(
        frm_sprites, "but_sprite", cur_sprite ? cur_sprite->name : ""
    );
    
    if(!cur_sprite) {
        frm_sprite->hide();
    } else {
        frm_sprite->show();
        
        if(is_pikmin) {
            enable_widget(frm_sprite->widgets["but_top"]);
        } else {
            disable_widget(frm_sprite->widgets["but_top"]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Performs a recursive flood fill on the bitmap sprite, to see what parts
 * contain non-alpha pixels, based on a starting position.
 * bmp:              Locked bitmap to check.
 * selection_pixels: Array that controls which pixels are selected or not.
 * x, y:             Image coordinates of the current iteration.
 * bmp_w, bmp_h:     Bitmap dimensions.
 */
void animation_editor::sprite_bmp_flood_fill(
    ALLEGRO_BITMAP* bmp, bool* selection_pixels,
    const int x, const int y, const int bmp_w, const int bmp_h
) {
    if(x < 0 || x > bmp_w) return;
    if(y < 0 || y > bmp_h) return;
    if(selection_pixels[y * bmp_w + x]) return;
    if(al_get_pixel(bmp, x, y).a < 0.008) {
        return;
    }
    
    selection_pixels[y * bmp_w + x] = true;
    sprite_bmp_flood_fill(
        bmp, selection_pixels, x + 1, y, bmp_w, bmp_h
    );
    sprite_bmp_flood_fill(
        bmp, selection_pixels, x - 1, y, bmp_w, bmp_h
    );
    sprite_bmp_flood_fill(
        bmp, selection_pixels, x, y + 1, bmp_w, bmp_h
    );
    sprite_bmp_flood_fill(
        bmp, selection_pixels, x, y - 1, bmp_w, bmp_h
    );
    
}



/* ----------------------------------------------------------------------------
 * Loads the sprite's bitmap data from memory to the gui.
 */
void animation_editor::sprite_bmp_to_gui() {
    set_textbox_text(frm_sprite_bmp, "txt_file", cur_sprite->file);
    set_textbox_text(frm_sprite_bmp, "txt_x", i2s(cur_sprite->file_pos.x));
    set_textbox_text(frm_sprite_bmp, "txt_y", i2s(cur_sprite->file_pos.y));
    set_textbox_text(frm_sprite_bmp, "txt_w", i2s(cur_sprite->file_size.x));
    set_textbox_text(frm_sprite_bmp, "txt_h", i2s(cur_sprite->file_size.y));
}


/* ----------------------------------------------------------------------------
 * Loads the sprite transformation's data from memory to the gui.
 */
void animation_editor::sprite_transform_to_gui() {
    set_textbox_text(frm_sprite_tra, "txt_x", f2s(cur_sprite->offset.x));
    set_textbox_text(frm_sprite_tra, "txt_y", f2s(cur_sprite->offset.y));
    set_textbox_text(frm_sprite_tra, "txt_w", f2s(cur_sprite->game_size.x));
    set_textbox_text(frm_sprite_tra, "txt_h", f2s(cur_sprite->game_size.y));
    set_checkbox_check(frm_sprite_tra, "chk_compare", comparison);
    
    if(comparison) {
        frm_sprite_comp->show();
    } else {
        frm_sprite_comp->hide();
    }
    
    set_checkbox_check(frm_sprite_comp, "chk_compare_blink", comparison_blink);
    set_checkbox_check(frm_sprite_comp, "chk_compare_above", comparison_above);
    set_checkbox_check(frm_sprite_comp, "chk_tint", comparison_tint);
    set_button_text(
        frm_sprite_comp, "but_compare",
        comparison_sprite ? comparison_sprite->name : ""
    );
}


/* ----------------------------------------------------------------------------
 * Loads the Pikmin top's data onto the gui.
 */
void animation_editor::top_to_gui() {
    lafi::checkbox* c = (lafi::checkbox*) frm_top->widgets["chk_visible"];
    if(cur_sprite->top_visible) c->check();
    else c->uncheck();
    
    set_textbox_text(frm_top, "txt_x", f2s(cur_sprite->top_pos.x));
    set_textbox_text(frm_top, "txt_y", f2s(cur_sprite->top_pos.y));
    set_textbox_text(frm_top, "txt_w", f2s(cur_sprite->top_size.x));
    set_textbox_text(frm_top, "txt_h", f2s(cur_sprite->top_size.y));
    set_angle_picker_angle(frm_top, "ang_angle", cur_sprite->top_angle);
}


/* ----------------------------------------------------------------------------
 * Saves the animation's data to memory using info on the gui.
 */
void animation_editor::gui_to_animation() {
    if(!cur_anim) return;
    
    cur_anim->loop_frame =
        s2i(get_textbox_text(frm_anim, "txt_loop")) - 1;
    if(cur_anim->loop_frame >= cur_anim->frames.size()) {
        cur_anim->loop_frame = 0;
    }
    if(get_checkbox_check(frm_anim, "chk_missable")) {
        cur_anim->hit_rate =
            s2i(get_textbox_text(frm_anim, "txt_hit_rate"));
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
    
    frame* f = &cur_anim->frames[cur_frame_nr];
    f->duration =
        s2f(get_textbox_text(frm_frame, "txt_dur"));
    if(f->duration < 0) f->duration = 0;
    
    if(get_checkbox_check(frm_frame, "chk_signal")) {
        f->signal = s2i(get_textbox_text(frm_frame, "txt_signal"));
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
    
    hitbox* h = &cur_sprite->hitboxes[cur_hitbox_nr];
    
    h->pos.x = s2f(get_textbox_text(frm_hitbox, "txt_x"));
    h->pos.y = s2f(get_textbox_text(frm_hitbox, "txt_y"));
    h->z = s2f(get_textbox_text(frm_hitbox, "txt_z"));
    
    h->height = s2f(get_textbox_text(frm_hitbox, "txt_h"));
    h->radius = s2f(get_textbox_text(frm_hitbox, "txt_r"));
    if(h->radius <= 0) h->radius = 16;
    
    hitbox* cur_h =
        &cur_sprite->hitboxes[cur_hitbox_nr];
        
    if(get_radio_selection(frm_hitbox, "rad_normal")) {
        cur_h->type = HITBOX_TYPE_NORMAL;
    } else if(get_radio_selection(frm_hitbox, "rad_attack")) {
        cur_h->type = HITBOX_TYPE_ATTACK;
    } else {
        cur_h->type = HITBOX_TYPE_DISABLED;
    }
    
    if(cur_h->type == HITBOX_TYPE_NORMAL) {
        cur_h->value =
            s2f(get_textbox_text(frm_normal_h, "txt_mult"));
        cur_h->can_pikmin_latch =
            get_checkbox_check(frm_normal_h, "chk_latch");
        cur_h->hazards_str =
            get_textbox_text(frm_normal_h, "txt_hazards");
            
    } else if(cur_h->type == HITBOX_TYPE_ATTACK) {
        cur_h->value =
            s2f(get_textbox_text(frm_attack_h, "txt_value"));
        cur_h->hazards_str =
            get_textbox_text(frm_attack_h, "txt_hazards");
        cur_h->knockback_outward =
            get_checkbox_check(frm_attack_h, "chk_outward");
        cur_h->knockback_angle =
            get_angle_picker_angle(frm_attack_h, "ang_angle");
        cur_h->knockback =
            s2f(get_textbox_text(frm_attack_h, "txt_knockback"));
        cur_h->wither_chance =
            s2i(get_textbox_text(frm_attack_h, "txt_wither"));
    }
    
    hitbox_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the sprite's data to memory using info on the gui.
 */
void animation_editor::gui_to_sprite() {
    //TODO will this be unused after the Sprite frame is split?
    if(!cur_sprite) return;
    
    cur_sprite->game_size.x =
        s2f(get_textbox_text(frm_sprite, "txt_gamew"));
    cur_sprite->game_size.y =
        s2f(get_textbox_text(frm_sprite, "txt_gameh"));
    cur_sprite->offset.x =
        s2f(get_textbox_text(frm_sprite, "txt_offsx"));
    cur_sprite->offset.y =
        s2f(get_textbox_text(frm_sprite, "txt_offsy"));
        
    sprite_to_gui();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the sprite's bitmap data to memory using info on the gui.
 */
void animation_editor::gui_to_sprite_bmp() {
    if(!cur_sprite) return;
    
    string new_file;
    point new_f_pos, new_f_size;
    
    new_file =
        get_textbox_text(frm_sprite_bmp, "txt_file");
    new_f_pos.x =
        s2i(get_textbox_text(frm_sprite_bmp, "txt_x"));
    new_f_pos.y =
        s2i(get_textbox_text(frm_sprite_bmp, "txt_y"));
    new_f_size.x =
        s2i(get_textbox_text(frm_sprite_bmp, "txt_w"));
    new_f_size.y =
        s2i(get_textbox_text(frm_sprite_bmp, "txt_h"));
        
    if(
        cur_sprite->file != new_file ||
        cur_sprite->file_pos.x != new_f_pos.x ||
        cur_sprite->file_pos.y != new_f_pos.y ||
        cur_sprite->file_size.x != new_f_size.x ||
        cur_sprite->file_size.y != new_f_size.y
    ) {
        //Changed something image-wise. Recreate it.
        cur_sprite->set_bitmap(new_file, new_f_pos, new_f_size);
    }
    
    last_file_used = new_file;
    
    gui_to_hitbox();
    sprite_bmp_to_gui();
    
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the sprite's transform data to memory using info on the gui.
 */
void animation_editor::gui_to_sprite_transform() {
    cur_sprite->offset.x =
        s2f(get_textbox_text(frm_sprite_tra, "txt_x"));
    cur_sprite->offset.y =
        s2f(get_textbox_text(frm_sprite_tra, "txt_y"));
    cur_sprite->game_size.x =
        s2f(get_textbox_text(frm_sprite_tra, "txt_w"));
    cur_sprite->game_size.y =
        s2f(get_textbox_text(frm_sprite_tra, "txt_h"));
    comparison =
        get_checkbox_check(frm_sprite_tra, "chk_compare");
        
    comparison_blink =
        get_checkbox_check(frm_sprite_comp, "chk_compare_blink");
    comparison_above =
        get_checkbox_check(frm_sprite_comp, "chk_compare_above");
    comparison_tint =
        get_checkbox_check(frm_sprite_comp, "chk_tint");
        
    cur_sprite_tc.set_center(cur_sprite->offset);
    cur_sprite_tc.set_size(cur_sprite->game_size);
    cur_sprite_tc.keep_aspect_ratio =
        get_checkbox_check(frm_sprite_tra, "chk_ratio");
        
    sprite_transform_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the Pikmin top's data to memory using info on the gui.
 */
void animation_editor::gui_to_top() {
    cur_sprite->top_visible =
        get_checkbox_check(frm_top, "chk_visible");
    cur_sprite->top_pos.x =
        s2f(get_textbox_text(frm_top, "txt_x"));
    cur_sprite->top_pos.y =
        s2f(get_textbox_text(frm_top, "txt_y"));
    cur_sprite->top_size.x =
        s2f(get_textbox_text(frm_top, "txt_w"));
    cur_sprite->top_size.y =
        s2f(get_textbox_text(frm_top, "txt_h"));
    cur_sprite->top_angle =
        get_angle_picker_angle(frm_top, "ang_angle");
        
    top_tc.set_center(cur_sprite->top_pos);
    top_tc.set_size(cur_sprite->top_size);
    top_tc.set_angle(cur_sprite->top_angle);
    top_tc.keep_aspect_ratio =
        get_checkbox_check(frm_top, "chk_ratio");
        
    top_to_gui();
    made_changes = true;
}


/* ----------------------------------------------------------------------------
 * Loads the animation database for the current object.
 */
void animation_editor::load_animation_database() {
    file_path = standardize_path(file_path);
    
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
    
    enable_widget(frm_bottom->widgets["but_load"]);
    enable_widget(frm_bottom->widgets["but_save"]);
    frm_hitboxes->hide();
    frm_top->hide();
    
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
    set_button_text(frm_main, "but_file", get_cut_path(file_path));
    
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
    lafi::widget* f = frm_hitboxes->widgets["frm_hitbox"];
    
    set_radio_selection(f, "rad_normal", false);
    set_radio_selection(f, "rad_attack", false);
    set_radio_selection(f, "rad_disabled", false);
    
    frm_normal_h->hide();
    frm_attack_h->hide();
    
    if(type == HITBOX_TYPE_NORMAL) {
        set_radio_selection(f, "rad_normal", true);
        frm_normal_h->show();
    } else if(type == HITBOX_TYPE_ATTACK) {
        set_radio_selection(f, "rad_attack", true);
        frm_attack_h->show();
    } else {
        set_radio_selection(f, "rad_disabled", true);
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
            set_button_text(frm_tools, "but_rename_anim_name", name);
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
        } else if(mode == EDITOR_MODE_SPRITE_BITMAP) {
            import_sprite_file_data(name);
        } else if(
            mode == EDITOR_MODE_SPRITE_TRANSFORM &&
            picker_disambig == PICKER_DISAMBIG_COMPARISON
        ) {
            comparison_sprite = anims.sprites[anims.find_sprite(name)];
            sprite_transform_to_gui();
        } else if(
            mode == EDITOR_MODE_SPRITE_TRANSFORM &&
            picker_disambig == PICKER_DISAMBIG_IMPORT
        ) {
            import_sprite_transformation_data(name);
        } else if(mode == EDITOR_MODE_TOP) {
            import_sprite_top_data(name);
        } else if(mode == EDITOR_MODE_TOOLS) {
            set_button_text(frm_tools, "but_rename_sprite_name", name);
        } else if(mode == EDITOR_MODE_HITBOXES) {
            import_sprite_hitbox_data(name);
        } else if(
            mode == EDITOR_MODE_SPRITE &&
            picker_disambig == PICKER_DISAMBIG_LOAD
        ) {
            pick_sprite(name);
        } else if(
            mode == EDITOR_MODE_SPRITE &&
            picker_disambig == PICKER_DISAMBIG_IMPORT
        ) {
            import_sprite_file_data(name);
            import_sprite_transformation_data(name);
            import_sprite_hitbox_data(name);
            import_sprite_top_data(name);
        }
        
    }
    
    show_bottom_frame();
    change_to_right_frame();
}


/* ----------------------------------------------------------------------------
 * Picks a sprite.
 */
void animation_editor::pick_sprite(const string &name) {
    cur_sprite = anims.sprites[anims.find_sprite(name)];
    cur_hitbox_nr = INVALID;
    if(cur_sprite->file.empty()) {
        //New frame. Suggest file name.
        cur_sprite->file = last_file_used;
        cur_sprite->set_bitmap(last_file_used, point(), point());
    }
    sprite_to_gui();
}


/* ----------------------------------------------------------------------------
 * Populates the history frame with the most recent files.
 */
void animation_editor::populate_history() {
    lafi::frame* f =
        (lafi::frame*) frm_history->widgets["frm_list"];
        
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
        (lafi::button*) frm_tools->widgets["but_rename_anim_name"];
    lafi::textbox* txt_ptr =
        (lafi::textbox*) frm_tools->widgets["txt_rename_anim"];
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
        (lafi::button*) frm_tools->widgets["but_rename_sprite_name"];
    lafi::textbox* txt_ptr =
        (lafi::textbox*) frm_tools->widgets["txt_rename_sprite"];
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
        (lafi::textbox*) frm_tools->widgets["txt_resolution"];
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
        (lafi::textbox*) frm_tools->widgets["txt_resize"];
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
                new data_node("value", f2s(h_ptr->value))
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
            if(h_ptr->wither_chance > 0) {
                hitbox_node->add(
                    new data_node("wither_chance", i2s(h_ptr->wither_chance))
                );
            }
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
 * Adds the current sprite Pikmin top's transformation controller data
 * to the GUI.
 */
void animation_editor::top_tc_to_gui() {
    set_textbox_text(
        frm_top, "txt_x", f2s(top_tc.get_center().x)
    );
    set_textbox_text(
        frm_top, "txt_y", f2s(top_tc.get_center().y)
    );
    set_textbox_text(
        frm_top, "txt_w", f2s(top_tc.get_size().x)
    );
    set_textbox_text(
        frm_top, "txt_h", f2s(top_tc.get_size().y)
    );
    set_angle_picker_angle(
        frm_top, "ang_angle", top_tc.get_angle()
    );
    gui_to_top();
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
    if(file_path.empty()) {
        frm_object->hide();
    } else {
        frm_object->show();
    }
    
    set_label_text(
        frm_object, "lbl_n_anims", "Animations: " + i2s(anims.animations.size())
    );
    set_label_text(
        frm_object, "lbl_n_sprites", "Sprites: " + i2s(anims.sprites.size())
    );
    set_label_text(
        frm_object, "lbl_n_body_parts",
        "Body parts: " + i2s(anims.body_parts.size())
    );
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
    frm_main->hide();
    frm_picker->hide();
    frm_history->hide();
    frm_anims->hide();
    frm_sprites->hide();
    frm_sprite_bmp->hide();
    frm_sprite_tra->hide();
    frm_hitboxes->hide();
    frm_top->hide();
    frm_body_parts->hide();
    frm_tools->hide();
}


/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor mode.
 */
void animation_editor::change_to_right_frame() {
    hide_all_frames();
    
    if(mode == EDITOR_MODE_MAIN) {
        frm_main->show();
    } else if(mode == EDITOR_MODE_ANIMATION) {
        frm_anims->show();
    } else if(mode == EDITOR_MODE_SPRITE) {
        frm_sprites->show();
    } else if(mode == EDITOR_MODE_BODY_PART) {
        frm_body_parts->show();
    } else if(mode == EDITOR_MODE_SPRITE_BITMAP) {
        frm_sprite_bmp->show();
    } else if(mode == EDITOR_MODE_SPRITE_TRANSFORM) {
        frm_sprite_tra->show();
    } else if(mode == EDITOR_MODE_HITBOXES) {
        frm_hitboxes->show();
    } else if(mode == EDITOR_MODE_TOP) {
        frm_top->show();
    } else if(mode == EDITOR_MODE_HISTORY) {
        frm_history->show();
    } else if(mode == EDITOR_MODE_TOOLS) {
        frm_tools->show();
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
