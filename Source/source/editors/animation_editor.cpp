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
    cur_hitbox(nullptr),
    cur_hitbox_alpha(0),
    cur_hitbox_nr(INVALID),
    cur_maturity(0),
    cur_sprite(NULL),
    hitboxes_visible(true),
    mob_radius_visible(false),
    origin_visible(true),
    pikmin_silhouette_visible(false),
    loaded_mob_type(nullptr) {
    
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
    
    cur_hitbox_tc.keep_aspect_ratio = true;
    cur_sprite_tc.keep_aspect_ratio = true;
    top_tc.keep_aspect_ratio = true;
    top_tc.allow_rotation = true;
    
    zoom_min_level = ZOOM_MIN_LEVEL_EDITOR;
    zoom_max_level = ZOOM_MAX_LEVEL_EDITOR;
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
        canvas_tl.x, canvas_tl.y,
        canvas_br.x - canvas_tl.x, canvas_br.y - canvas_tl.y
    );
    
    al_clear_to_color(al_map_rgb(128, 144, 128));
    
    sprite* s = NULL;
    bool draw_origin = origin_visible;
    bool draw_hitboxes = hitboxes_visible;
    bool draw_mob_radius = mob_radius_visible;
    bool draw_pikmin_silhouette = pikmin_silhouette_visible;
    
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
    
        draw_origin = false;
        draw_hitboxes = false;
        draw_mob_radius = false;
        draw_pikmin_silhouette = false;
        
        int bmp_w = al_get_bitmap_width(s->parent_bmp);
        int bmp_h = al_get_bitmap_height(s->parent_bmp);
        int bmp_x = -bmp_w / 2.0;
        int bmp_y = -bmp_h / 2.0;
        al_draw_bitmap(s->parent_bmp, bmp_x, bmp_y, 0);
        
        point scene_tl = point(-1, -1);
        point scene_br = point(canvas_br.x + 1, canvas_br.y + 1);
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
        
        if(
            s->top_visible && loaded_mob_type
            && loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
        ) {
            draw_bitmap(
                top_bmp[cur_maturity],
                s->top_pos, s->top_size,
                s->top_angle
            );
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
        } else if(mode == EDITOR_MODE_HITBOXES) {
            if(cur_sprite && cur_hitbox) {
                cur_hitbox_tc.draw_handles();
            }
        }
        
    }
    
    if(draw_origin) {
        point cam_top_left_corner(0, 0);
        point cam_bottom_right_corner(canvas_br.x, canvas_br.y);
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
    
    if(draw_mob_radius && loaded_mob_type) {
        al_draw_circle(
            0, 0, loaded_mob_type->radius,
            al_map_rgb(240, 240, 240), 1 / cam_zoom
        );
    }
    
    if(draw_pikmin_silhouette) {
        float x_offset = 32;
        if(loaded_mob_type) {
            x_offset += loaded_mob_type->radius;
        }
        draw_bitmap(
            bmp_pikmin_silhouette, point(x_offset, 0),
            point(-1, standard_pikmin_height),
            0, al_map_rgba(240, 240, 240, 160)
        );
    }
    
    al_reset_clipping_rectangle();
    al_use_transform(&identity_transform);
    
    draw_unsaved_changes_warning();
    
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
    emit_status_bar_message("Data imported.", false);
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
    cur_hitbox = NULL;
    hitbox_to_gui();
    emit_status_bar_message("Data imported.", false);
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
    emit_status_bar_message("Data imported.", false);
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
    emit_status_bar_message("Data imported.", false);
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
 * Loads the animation database for the current object.
 */
void animation_editor::load_animation_database(const bool update_history) {
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
    cur_hitbox = NULL;
    cur_hitbox_nr = INVALID;
    if(!anims.animations.empty()) {
        cur_anim = anims.animations[0];
        if(cur_anim->frames.size()) cur_frame_nr = 0;
    }
    if(!anims.sprites.empty()) {
        cur_sprite = anims.sprites[0];
        if(!cur_sprite->hitboxes.empty()) {
            cur_hitbox = &cur_sprite->hitboxes[0];
            cur_hitbox_nr = 0;
        }
    }
    
    enable_widget(frm_toolbar->widgets["but_load"]);
    enable_widget(frm_toolbar->widgets["but_save"]);
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
    
    if(file_path.find(TYPES_FOLDER_PATH) != string::npos) {
        vector<string> path_parts = split(file_path, "/");
        if(
            path_parts.size() > 3 &&
            path_parts[path_parts.size() - 1] == "Animations.txt"
        ) {
            mob_category* cat =
                mob_categories.get_from_pname(
                    path_parts[path_parts.size() - 3]
                );
            if(cat) {
                loaded_mob_type =
                    mob_categories.find_mob_type_from_folder_name(
                        cat,
                        path_parts[path_parts.size() - 2]
                    );
            }
        }
    }
    
    //Top bitmap.
    for(unsigned char t = 0; t < N_MATURITIES; ++t) {
        if(top_bmp[t] && top_bmp[t] != bmp_error) {
            al_destroy_bitmap(top_bmp[t]);
            top_bmp[t] = NULL;
        }
    }
    
    if(
        loaded_mob_type &&
        loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
    ) {
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
    }
    
    if(update_history) {
        update_animation_editor_history(file_path);
        save_options(); //Save the history on the options.
    }
    
    frm_toolbar->show();
    mode = EDITOR_MODE_MAIN;
    change_to_right_frame();
    loaded_content_yet = true;
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
    
    if(new_name.empty()) {
        emit_status_bar_message("You need to specify the new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        if(anims.animations[a]->name == old_name) old_anim_id = a;
        if(anims.animations[a]->name == new_name) {
            emit_status_bar_message("That name is already being used!", true);
            return;
        }
    }
    
    if(old_anim_id == INVALID) return;
    
    anims.animations[old_anim_id]->name = new_name;
    
    but_ptr->text = "";
    txt_ptr->text = "";
    
    made_new_changes = true;
    emit_status_bar_message("Renamed successfully.", false);
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
    
    if(new_name.empty()) {
        emit_status_bar_message("You need to specify the new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        if(anims.sprites[s]->name == old_name) old_sprite_id = s;
        if(anims.sprites[s]->name == new_name) {
            emit_status_bar_message("That name is already being used!", true);
            return;
        }
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
    
    made_new_changes = true;
    emit_status_bar_message("Renamed successfully.", false);
}


/* ----------------------------------------------------------------------------
 * Resizes all sprite game-width/height by a factor compared
 * to the respective file-width/height.
 */
void animation_editor::resize_by_resolution() {
    float mult = s2f(get_textbox_text(frm_tools, "txt_resolution"));
    
    if(mult == 0) {
        emit_status_bar_message("Can't resize sprites to size 0!", true);
        return;
    }
    
    mult = 1.0 / mult;
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        sprite* s_ptr = anims.sprites[s];
        s_ptr->game_size = s_ptr->file_size * mult;
    }
    
    set_textbox_text(frm_tools, "txt_resolution", "");
    made_new_changes = true;
    emit_status_bar_message("Resized successfully.", false);
}


/* ----------------------------------------------------------------------------
 * Resizes sprites, body parts, etc. by a multiplier.
 */
void animation_editor::resize_everything() {
    float mult = s2f(get_textbox_text(frm_tools, "txt_resize"));
    
    if(mult == 0) {
        emit_status_bar_message("Can't resize everything to size 0!", true);
        return;
    }
    
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
    
    set_textbox_text(frm_tools, "txt_resize", "");
    made_new_changes = true;
    emit_status_bar_message("Resized successfully.", false);
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
        
        if(
            loaded_mob_type &&
            loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
        ) {
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
    made_new_changes = false;
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void animation_editor::unload() {
    editor::unload();
    
    anims.destroy();
    delete(gui->style);
    //TODO warning: deleting object of polymorphic class type 'lafi::gui'
    //which has non-virtual destructor might cause undefined behaviour
    //[-Wdelete-non-virtual-dtor]
    delete(gui);
    
    unload_mob_types(false);
    unload_spike_damage_types();
    unload_hazards();
    unload_liquids();
    unload_status_types(false);
    unload_custom_particle_generators();
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
                s_ptr->hitboxes.push_back(
                    hitbox(
                        name, INVALID, NULL, point(), 0,
                        loaded_mob_type ? loaded_mob_type->height : 128,
                        loaded_mob_type ? loaded_mob_type->radius : 32
                    )
                );
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
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void animation_editor::update_transformations() {
    //World coordinates to screen coordinates.
    world_to_screen_transform = identity_transform;
    al_translate_transform(
        &world_to_screen_transform,
        -cam_pos.x + canvas_br.x / 2.0 / cam_zoom,
        -cam_pos.y + canvas_br.y / 2.0 / cam_zoom
    );
    al_scale_transform(&world_to_screen_transform, cam_zoom, cam_zoom);
    
    //Screen coordinates to world coordinates.
    screen_to_world_transform = world_to_screen_transform;
    al_invert_transform(&screen_to_world_transform);
}
