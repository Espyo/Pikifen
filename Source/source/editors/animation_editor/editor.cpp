/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General animation editor-related functions.
 */

#include <algorithm>
#include <queue>

#include <allegro5/allegro.h>

#include "editor.h"

#include "../../functions.h"
#include "../../game.h"
#include "../../LAFI/angle_picker.h"
#include "../../LAFI/button.h"
#include "../../LAFI/checkbox.h"
#include "../../LAFI/frame.h"
#include "../../LAFI/minor.h"
#include "../../LAFI/radio_button.h"
#include "../../LAFI/scrollbar.h"
#include "../../LAFI/textbox.h"
#include "../../load.h"
#include "../../mob_categories/pikmin_category.h"
#include "../../utils/math_utils.h"
#include "../../utils/string_utils.h"

using std::queue;


//Amount to pan the camera by when using the keyboard.
const float animation_editor::KEYBOARD_PAN_AMOUNT = 32.0f;
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
    comparison_above(true),
    comparison_blink(true),
    comparison_blink_show(true),
    comparison_blink_timer(0),
    comparison_sprite(nullptr),
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
    loaded_mob_type(nullptr),
    mob_radius_visible(false),
    origin_visible(true),
    pikmin_silhouette_visible(false),
    side_view(false) {
    
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
    cur_sprite_tc.allow_rotation = true;
    top_tc.keep_aspect_ratio = true;
    top_tc.allow_rotation = true;
    
    zoom_min_level = ZOOM_MIN_LEVEL_EDITOR;
    zoom_max_level = ZOOM_MAX_LEVEL_EDITOR;
}


/* ----------------------------------------------------------------------------
 * Centers the camera on the sprite's parent bitmap, so the user can choose
 * what part of the bitmap they want to use for the sprite.
 */
void animation_editor::center_camera_on_sprite_bitmap() {
    if(cur_sprite && cur_sprite->parent_bmp) {
        int bmp_w = al_get_bitmap_width(cur_sprite->parent_bmp);
        int bmp_h = al_get_bitmap_height(cur_sprite->parent_bmp);
        int bmp_x = -bmp_w / 2.0;
        int bmp_y = -bmp_h / 2.0;
        
        center_camera(point(bmp_x, bmp_y), point(bmp_x + bmp_w, bmp_y + bmp_h));
    } else {
        game.cam.zoom = 1.0f;
        game.cam.pos = point();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the animation editor.
 */
void animation_editor::do_logic() {
    editor_old::do_logic_pre();
    
    if(
        anim_playing && state == EDITOR_STATE_ANIMATION &&
        cur_anim && cur_frame_nr != INVALID
    ) {
        frame* f = &cur_anim->frames[cur_frame_nr];
        if(f->duration != 0) {
            cur_frame_time += game.delta_t;
            
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
    
    cur_hitbox_alpha += TAU * 1.5 * game.delta_t;
    
    if(comparison_blink) {
        comparison_blink_timer.tick(game.delta_t);
    } else {
        comparison_blink_show = true;
    }
    
    editor_old::do_logic_post();
}


/* ----------------------------------------------------------------------------
 * Enters the side view mode.
 */
void animation_editor::enter_side_view() {
    side_view = true;
    set_checkbox_check(frm_hitboxes, "chk_side_view", true);
    update_cur_hitbox_tc();
    cur_hitbox_tc.keep_aspect_ratio = false;
}


/* ----------------------------------------------------------------------------
 * Exits the side view mode.
 */
void animation_editor::exit_side_view() {
    side_view = false;
    set_checkbox_check(frm_hitboxes, "chk_side_view", false);
    update_cur_hitbox_tc();
    cur_hitbox_tc.keep_aspect_ratio = true;
}


/* ----------------------------------------------------------------------------
 * Returns a file path, but cropped to fit on the gui's buttons.
 * This implies cutting it in two lines, and even replacing the start with
 * ellipsis, if needed.
 */
string animation_editor::get_cut_path(const string &p) const {
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
 * Returns the name of this state.
 */
string animation_editor::get_name() const {
    return "animation editor";
}


/* ----------------------------------------------------------------------------
 * Imports the animation data from a different animation to the current.
 */
void animation_editor::import_animation_data(const string &name) {
    animation* a = anims.animations[anims.find_animation(name)];
    
    cur_anim->frames = a->frames;
    cur_anim->hit_rate = a->hit_rate;
    cur_anim->loop_frame = a->loop_frame;
    
    animation_to_gui();
    emit_status_bar_message("Data imported.", false);
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
    cur_hitbox_nr = INVALID;
    cur_hitbox = NULL;
    if(!cur_sprite->hitboxes.empty()) {
        cur_hitbox_nr = 0;
        cur_hitbox = &cur_sprite->hitboxes[0];
    }
    hitbox_to_gui();
    emit_status_bar_message("Data imported.", false);
}


/* ----------------------------------------------------------------------------
 * Imports the sprite top data from a different sprite to the current.
 */
void animation_editor::import_sprite_top_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    set_checkbox_check(frm_top, "chk_visible", s->top_visible);
    set_textbox_text(frm_top, "txt_x", f2s(s->top_pos.x));
    set_textbox_text(frm_top, "txt_y", f2s(s->top_pos.y));
    set_textbox_text(frm_top, "txt_w", f2s(s->top_size.x));
    set_textbox_text(frm_top, "txt_h", f2s(s->top_size.y));
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
    set_textbox_text(frm_sprite_tra, "txt_x", f2s(s->offset.x));
    set_textbox_text(frm_sprite_tra, "txt_y", f2s(s->offset.y));
    set_textbox_text(frm_sprite_tra, "txt_sx", f2s(s->scale.x));
    set_textbox_text(frm_sprite_tra, "txt_sy", f2s(s->scale.y));
    set_angle_picker_angle(frm_sprite_tra, "ang_a", s->angle);
    
    gui_to_sprite_transform();
    emit_status_bar_message("Data imported.", false);
}


/* ----------------------------------------------------------------------------
 * Loads the animation database for the current object.
 */
void animation_editor::load_animation_database(
    const bool should_update_history
) {
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        //Ideally, states would be handled by a state machine, and this
        //logic would be placed in the sprite bitmap state's "on exit" code...
        game.cam.pos = pre_sprite_bmp_cam_pos;
        game.cam.zoom = pre_sprite_bmp_cam_zoom;
    }
    
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
    
    enable_widget(frm_toolbar->widgets["but_reload"]);
    enable_widget(frm_toolbar->widgets["but_save"]);
    frm_hitboxes->hide();
    frm_top->hide();
    
    game.cam.pos.x = game.cam.pos.y = 0;
    game.cam.zoom = 1;
    
    //Find the most popular file name to suggest for new sprites.
    last_file_used.clear();
    
    if(!anims.sprites.empty()) {
        map<string, size_t> file_uses_map;
        vector<std::pair<size_t, string> > file_uses_vector;
        for(size_t f = 0; f < anims.sprites.size(); ++f) {
            file_uses_map[anims.sprites[f]->file]++;
        }
        for(auto &u : file_uses_map) {
            file_uses_vector.push_back(make_pair(u.second, u.first));
        }
        sort(
            file_uses_vector.begin(),
            file_uses_vector.end(),
            [] (
                std::pair<size_t, string> u1, std::pair<size_t, string> u2
        ) -> bool {
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
                game.mob_categories.get_from_folder_name(
                    TYPES_FOLDER_PATH + "/" + path_parts[path_parts.size() - 3]
                );
            if(cat) {
                loaded_mob_type =
                    game.mob_categories.find_mob_type_from_folder_name(
                        cat,
                        path_parts[path_parts.size() - 2]
                    );
            }
        }
    }
    
    //Top bitmap.
    for(unsigned char t = 0; t < N_MATURITIES; ++t) {
        if(top_bmp[t] && top_bmp[t] != game.bmp_error) {
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
                loaded_mob_type->category->folder + "/" +
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
    
    if(should_update_history) {
        update_history(file_path);
        save_options(); //Save the history on the options.
    }
    
    frm_toolbar->show();
    state = EDITOR_STATE_MAIN;
    change_to_right_frame();
    loaded_content_yet = true;
    update_stats();
    
    emit_status_bar_message("Loaded successfully.", false);
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
    
    switch(type) {
    case HITBOX_TYPE_NORMAL: {
        set_radio_selection(f, "rad_normal", true);
        frm_normal_h->show();
        break;
    } case HITBOX_TYPE_ATTACK: {
        set_radio_selection(f, "rad_attack", true);
        frm_attack_h->show();
        break;
    } case HITBOX_TYPE_DISABLED: {
        set_radio_selection(f, "rad_disabled", true);
        break;
    }
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
    anims.sort_alphabetically();
    
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
    anims.sort_alphabetically();
    
    but_ptr->text = "";
    txt_ptr->text = "";
    
    made_new_changes = true;
    emit_status_bar_message("Renamed successfully.", false);
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
        
        s_ptr->scale    *= mult;
        s_ptr->offset   *= mult;
        s_ptr->top_pos  *= mult;
        s_ptr->top_size *= mult;
        
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
        
        if(anims.animations[a]->loop_frame > 0) {
            anim_node->add(
                new data_node(
                    "loop_frame", i2s(anims.animations[a]->loop_frame)
                )
            );
        }
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
        if(s_ptr->offset.x != 0.0 || s_ptr->offset.y != 0.0) {
            sprite_node->add(new data_node("offset", p2s(s_ptr->offset)));
        }
        if(s_ptr->scale.x != 1.0 || s_ptr->scale.y != 1.0) {
            sprite_node->add(new data_node("scale", p2s(s_ptr->scale)));
        }
        if(s_ptr->angle != 0.0) {
            sprite_node->add(new data_node("angle", f2s(s_ptr->angle)));
        }
        
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
        
        if(!s_ptr->hitboxes.empty()) {
            data_node* hitboxes_node =
                new data_node("hitboxes", "");
            sprite_node->add(hitboxes_node);
            
            for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
                hitbox* h_ptr = &s_ptr->hitboxes[h];
                
                data_node* hitbox_node =
                    new data_node(h_ptr->body_part_name, "");
                hitboxes_node->add(hitbox_node);
                
                hitbox_node->add(
                    new data_node("coords", p2s(h_ptr->pos, &h_ptr->z))
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
                if(
                    h_ptr->type == HITBOX_TYPE_NORMAL &&
                    h_ptr->can_pikmin_latch
                ) {
                    hitbox_node->add(
                        new data_node(
                            "can_pikmin_latch", b2s(h_ptr->can_pikmin_latch)
                        )
                    );
                }
                if(!h_ptr->hazards_str.empty()) {
                    hitbox_node->add(
                        new data_node("hazards", h_ptr->hazards_str)
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback_outward
                ) {
                    hitbox_node->add(
                        new data_node(
                            "knockback_outward",
                            b2s(h_ptr->knockback_outward)
                        )
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback_angle != 0
                ) {
                    hitbox_node->add(
                        new data_node(
                            "knockback_angle", f2s(h_ptr->knockback_angle)
                        )
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback != 0
                ) {
                    hitbox_node->add(
                        new data_node("knockback", f2s(h_ptr->knockback))
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->wither_chance > 0
                ) {
                    hitbox_node->add(
                        new data_node(
                            "wither_chance", i2s(h_ptr->wither_chance)
                        )
                    );
                }
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
    
    if(!file_node.save_file(file_path)) {
        show_message_box(
            NULL, "Save failed!",
            "Could not save the animation!",
            (
                "An error occured while saving the animation to the file \"" +
                file_path + "\". Make sure that the folder it is saving to "
                "exists and it is not read-only, and try again."
            ).c_str(),
            NULL,
            ALLEGRO_MESSAGEBOX_WARN
        );
        emit_status_bar_message("Could not save the animation!", true);
    } else {
        emit_status_bar_message("Saved successfully.", false);
    }
    made_new_changes = false;
}


/* ----------------------------------------------------------------------------
 * Sets all sprite scales to the value specified in the textbox.
 */
void animation_editor::set_all_sprite_scales() {
    float mult = s2f(get_textbox_text(frm_tools, "txt_set_scales"));
    
    if(mult == 0) {
        emit_status_bar_message("The scales can't be 0!", true);
        return;
    }
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        sprite* s_ptr = anims.sprites[s];
        s_ptr->scale.x = mult;
        s_ptr->scale.y = mult;
    }
    
    set_textbox_text(frm_tools, "txt_set_scales", "");
    made_new_changes = true;
    emit_status_bar_message("Sprite scales set successfully.", false);
}


static const float FLOOD_FILL_ALPHA_THRESHOLD = 0.008;

/* ----------------------------------------------------------------------------
 * Performs a flood fill on the bitmap sprite, to see what parts
 * contain non-alpha pixels, based on a starting position.
 * bmp:              Locked bitmap to check.
 * selection_pixels: Array that controls which pixels are selected or not.
 * x, y:             Image coordinates to start on.
 */
void animation_editor::sprite_bmp_flood_fill(
    ALLEGRO_BITMAP* bmp, bool* selection_pixels, const int x, const int y
) {
    //https://en.wikipedia.org/wiki/Flood_fill#The_algorithm
    int bmp_w = al_get_bitmap_width(bmp);
    int bmp_h = al_get_bitmap_height(bmp);
    
    if(x < 0 || x > bmp_w) return;
    if(y < 0 || y > bmp_h) return;
    if(selection_pixels[y * bmp_w + x]) return;
    if(al_get_pixel(bmp, x, y).a < FLOOD_FILL_ALPHA_THRESHOLD) {
        return;
    }
    
    struct int_point {
        int x;
        int y;
        int_point(point p) :
            x(p.x),
            y(p.y) { }
        int_point(int x, int y) :
            x(x),
            y(y) { }
    };
    
    queue<int_point> pixels_left;
    pixels_left.push(int_point(x, y));
    
    while(!pixels_left.empty()) {
        int_point p = pixels_left.front();
        pixels_left.pop();
        
        if(
            selection_pixels[(p.y) * bmp_w + p.x] ||
            al_get_pixel(bmp, p.x, p.y).a < FLOOD_FILL_ALPHA_THRESHOLD
        ) {
            continue;
        }
        
        int_point offset = p;
        vector<int_point> columns;
        columns.push_back(p);
        
        bool add = true;
        //Keep going left and adding columns to check.
        while(add) {
            if(offset.x  == 0) {
                add = false;
            } else {
                offset.x--;
                if(selection_pixels[offset.y * bmp_w + offset.x]) {
                    add = false;
                } else if(
                    al_get_pixel(bmp, offset.x, offset.y).a <
                    FLOOD_FILL_ALPHA_THRESHOLD
                ) {
                    add = false;
                } else {
                    columns.push_back(offset);
                }
            }
        }
        offset = p;
        add = true;
        //Keep going right and adding columns to check.
        while(add) {
            if(offset.x == bmp_w - 1) {
                add = false;
            } else {
                offset.x++;
                if(selection_pixels[offset.y * bmp_w + offset.x]) {
                    add = false;
                } else if(
                    al_get_pixel(bmp, offset.x, offset.y).a <
                    FLOOD_FILL_ALPHA_THRESHOLD
                ) {
                    add = false;
                } else {
                    columns.push_back(offset);
                }
            }
        }
        
        for(size_t c = 0; c < columns.size(); ++c) {
            //For each column obtained, mark the pixel there,
            //and check the pixels above and below, to see if they should be
            //processed next.
            int_point col = columns[c];
            selection_pixels[col.y * bmp_w + col.x] = true;
            if(
                col.y > 0 &&
                !selection_pixels[(col.y - 1) * bmp_w + col.x] &&
                al_get_pixel(bmp, col.x, col.y - 1).a >=
                FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(int_point(col.x, col.y - 1));
            }
            if(
                col.y < bmp_h - 1 &&
                !selection_pixels[(col.y + 1) * bmp_w + col.x] &&
                al_get_pixel(bmp, col.x, col.y + 1).a >=
                FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(int_point(col.x, col.y + 1));
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void animation_editor::unload() {
    editor_old::unload();
    
    anims.destroy();
    delete(gui_style);
    delete(faded_style);
    delete(gui);
    
    unload_mob_types(false);
    unload_spike_damage_types();
    unload_hazards();
    unload_liquids();
    unload_spray_types();
    unload_status_types(false);
    unload_custom_particle_generators();
}


/* ----------------------------------------------------------------------------
 * Updates the history list, by adding a new entry or bumping it up.
 */
void animation_editor::update_history(const string &n) {
    //First, check if it exists.
    size_t pos = INVALID;
    
    for(size_t h = 0; h < history.size(); ++h) {
        if(history[h] == n) {
            pos = h;
            break;
        }
    }
    
    if(pos == 0) {
        //Already #1? Never mind.
        return;
    } else if(pos == INVALID) {
        //If it doesn't exist, create it and add it to the top.
        history.insert(history.begin(), n);
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        history.erase(history.begin() + pos);
        history.insert(history.begin(), n);
    }
    
    if(history.size() > HISTORY_SIZE) {
        history.erase(history.begin() + history.size() - 1);
    }
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
