/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Functions about the animation editor's GUI.
 */

#include "animation_editor.h"
#include "../LAFI/angle_picker.h"
#include "../LAFI/button.h"
#include "../LAFI/checkbox.h"
#include "../LAFI/image.h"
#include "../LAFI/radio_button.h"
#include "../LAFI/scrollbar.h"
#include "../LAFI/textbox.h"
#include "../functions.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Switches to the correct frame, depending on the current editor state.
 */
void animation_editor::change_to_right_frame() {
    hide_all_frames();
    
    if(state == EDITOR_STATE_MAIN) {
        frm_main->show();
    } else if(state == EDITOR_STATE_ANIMATION) {
        frm_anims->show();
    } else if(state == EDITOR_STATE_SPRITE) {
        frm_sprites->show();
    } else if(state == EDITOR_STATE_BODY_PART) {
        frm_body_parts->show();
    } else if(state == EDITOR_STATE_SPRITE_BITMAP) {
        frm_sprite_bmp->show();
    } else if(state == EDITOR_STATE_SPRITE_TRANSFORM) {
        frm_sprite_tra->show();
    } else if(state == EDITOR_STATE_HITBOXES) {
        frm_hitboxes->show();
    } else if(state == EDITOR_STATE_TOP) {
        frm_top->show();
    } else if(state == EDITOR_STATE_LOAD) {
        frm_load->show();
    } else if(state == EDITOR_STATE_TOOLS) {
        frm_tools->show();
    } else if(state == EDITOR_STATE_OPTIONS) {
        frm_options->show();
        options_to_gui();
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new item from the picker frame, given the item's name.
 */
void animation_editor::create_new_from_picker(
    const size_t picker_id, const string &name
) {
    if(
        picker_id == PICKER_EDIT_ANIMATION
    ) {
        if(anims.find_animation(name) == INVALID) {
            anims.animations.push_back(new animation(name));
        }
        pick(picker_id, name, "");
        
    } else if(
        picker_id == PICKER_EDIT_SPRITE
    ) {
        if(anims.find_sprite(name) == INVALID) {
            anims.sprites.push_back(new sprite(name));
            anims.sprites.back()->create_hitboxes(
                &anims,
                loaded_mob_type ? loaded_mob_type->height : 128,
                loaded_mob_type ? loaded_mob_type->radius : 32
            );
        }
        pick(picker_id, name, "");
        
    }
}


/* ----------------------------------------------------------------------------
 * Adds the current hitbox's transformation controller data to the GUI.
 */
void animation_editor::cur_hitbox_tc_to_gui() {
    if(!cur_sprite && !cur_hitbox) return;
    
    if(side_view) {
        set_textbox_text(
            frm_hitbox, "txt_x", f2s(cur_hitbox_tc.get_center().x)
        );
        set_textbox_text(
            frm_hitbox, "txt_r", f2s(cur_hitbox_tc.get_size().x / 2.0)
        );
        set_textbox_text(
            frm_hitbox, "txt_z",
            f2s(
                -(
                    cur_hitbox_tc.get_center().y +
                    cur_hitbox_tc.get_size().y / 2.0
                )
            )
        );
        set_textbox_text(
            frm_hitbox, "txt_h", f2s(cur_hitbox_tc.get_size().y)
        );
    } else {
        set_textbox_text(
            frm_hitbox, "txt_x", f2s(cur_hitbox_tc.get_center().x)
        );
        set_textbox_text(
            frm_hitbox, "txt_y", f2s(cur_hitbox_tc.get_center().y)
        );
        set_textbox_text(
            frm_hitbox, "txt_r", f2s(cur_hitbox_tc.get_size().x / 2.0)
        );
    }
    gui_to_hitbox();
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
        frm_sprite_tra, "txt_sx", f2s(
            cur_sprite_tc.get_size().x / cur_sprite->file_size.x
        )
    );
    set_textbox_text(
        frm_sprite_tra, "txt_sy", f2s(
            cur_sprite_tc.get_size().y / cur_sprite->file_size.y
        )
    );
    set_angle_picker_angle(
        frm_sprite_tra, "ang_a", cur_sprite_tc.get_angle()
    );
    gui_to_sprite_transform();
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
    if(cur_hitbox) {
        set_label_text(frm_hitboxes, "lbl_name", cur_hitbox->body_part_name);
        set_textbox_text(frm_hitbox, "txt_x", f2s(cur_hitbox->pos.x));
        set_textbox_text(frm_hitbox, "txt_y", f2s(cur_hitbox->pos.y));
        set_textbox_text(frm_hitbox, "txt_z", f2s(cur_hitbox->z));
        set_textbox_text(frm_hitbox, "txt_h", f2s(cur_hitbox->height));
        set_textbox_text(frm_hitbox, "txt_r", f2s(cur_hitbox->radius));
    }
    
    open_hitbox_type(cur_hitbox ? cur_hitbox->type : 255);
    
    if(cur_hitbox) {
        frm_hitbox->show();
        if(cur_hitbox->type == HITBOX_TYPE_NORMAL) {
            set_textbox_text(frm_normal_h, "txt_mult", f2s(cur_hitbox->value));
            set_checkbox_check(
                frm_normal_h, "chk_latch", cur_hitbox->can_pikmin_latch
            );
            set_textbox_text(
                frm_normal_h, "txt_hazards", cur_hitbox->hazards_str
            );
            
        } else if(cur_hitbox->type == HITBOX_TYPE_ATTACK) {
            set_textbox_text(
                frm_attack_h, "txt_value", f2s(cur_hitbox->value)
            );
            set_textbox_text(
                frm_attack_h, "txt_hazards", cur_hitbox->hazards_str
            );
            set_checkbox_check(
                frm_attack_h, "chk_outward", cur_hitbox->knockback_outward
            );
            set_angle_picker_angle(
                frm_attack_h, "ang_angle", cur_hitbox->knockback_angle
            );
            set_textbox_text(
                frm_attack_h, "txt_knockback", f2s(cur_hitbox->knockback)
            );
            set_textbox_text(
                frm_attack_h, "txt_wither", i2s(cur_hitbox->wither_chance)
            );
            
            if(cur_hitbox->knockback_outward) {
                disable_widget(frm_attack_h->widgets["ang_angle"]);
            } else {
                enable_widget(frm_attack_h->widgets["ang_angle"]);
            }
            
        }
    } else {
        frm_hitbox->hide();
    }
    
    if(cur_hitbox) {
        update_cur_hitbox_tc();
    }
}


/* ----------------------------------------------------------------------------
 * Loads the options data onto the GUI.
 */
void animation_editor::options_to_gui() {
    set_checkbox_check(frm_options, "chk_mmb_pan", animation_editor_mmb_pan);
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
        
        if(anims.body_parts.empty()) {
            disable_widget(frm_sprite->widgets["but_hitboxes"]);
        } else {
            enable_widget(frm_sprite->widgets["but_hitboxes"]);
        }
        
        if(
            loaded_mob_type &&
            loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
        ) {
            enable_widget(frm_sprite->widgets["but_top"]);
        } else {
            disable_widget(frm_sprite->widgets["but_top"]);
        }
    }
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
    set_textbox_text(frm_sprite_tra, "txt_sx", f2s(cur_sprite->scale.x));
    set_textbox_text(frm_sprite_tra, "txt_sy", f2s(cur_sprite->scale.y));
    set_angle_picker_angle(frm_sprite_tra, "ang_a", cur_sprite->angle);
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
    
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the body part's data from the gui.
 */
void animation_editor::gui_to_body_part() {
    body_part_to_gui();
    
    made_new_changes = true;
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
    
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the hitbox's data to memory using info on the gui.
 */
void animation_editor::gui_to_hitbox() {
    bool valid = cur_sprite && cur_hitbox;
    if(!valid) return;
    
    cur_hitbox->pos.x = s2f(get_textbox_text(frm_hitbox, "txt_x"));
    cur_hitbox->pos.y = s2f(get_textbox_text(frm_hitbox, "txt_y"));
    cur_hitbox->z = s2f(get_textbox_text(frm_hitbox, "txt_z"));
    
    cur_hitbox->height = s2f(get_textbox_text(frm_hitbox, "txt_h"));
    cur_hitbox->radius = s2f(get_textbox_text(frm_hitbox, "txt_r"));
    if(cur_hitbox->radius <= 0) cur_hitbox->radius = 16;
    
    if(get_radio_selection(frm_hitbox, "rad_normal")) {
        cur_hitbox->type = HITBOX_TYPE_NORMAL;
    } else if(get_radio_selection(frm_hitbox, "rad_attack")) {
        cur_hitbox->type = HITBOX_TYPE_ATTACK;
    } else {
        cur_hitbox->type = HITBOX_TYPE_DISABLED;
    }
    
    if(cur_hitbox->type == HITBOX_TYPE_NORMAL) {
        cur_hitbox->value =
            s2f(get_textbox_text(frm_normal_h, "txt_mult"));
        cur_hitbox->can_pikmin_latch =
            get_checkbox_check(frm_normal_h, "chk_latch");
        cur_hitbox->hazards_str =
            get_textbox_text(frm_normal_h, "txt_hazards");
            
    } else if(cur_hitbox->type == HITBOX_TYPE_ATTACK) {
        cur_hitbox->value =
            s2f(get_textbox_text(frm_attack_h, "txt_value"));
        cur_hitbox->hazards_str =
            get_textbox_text(frm_attack_h, "txt_hazards");
        cur_hitbox->knockback_outward =
            get_checkbox_check(frm_attack_h, "chk_outward");
        cur_hitbox->knockback_angle =
            get_angle_picker_angle(frm_attack_h, "ang_angle");
        cur_hitbox->knockback =
            s2f(get_textbox_text(frm_attack_h, "txt_knockback"));
        cur_hitbox->wither_chance =
            s2i(get_textbox_text(frm_attack_h, "txt_wither"));
    }
    
    hitbox_to_gui();
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the options data to memory using info on the gui.
 */
void animation_editor::gui_to_options() {
    animation_editor_mmb_pan =
        get_checkbox_check(frm_options, "chk_mmb_pan");
        
    save_options();
    options_to_gui();
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
    
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Saves the sprite's transform data to memory using info on the gui.
 */
void animation_editor::gui_to_sprite_transform() {
    cur_sprite->offset.x =
        s2f(get_textbox_text(frm_sprite_tra, "txt_x"));
    cur_sprite->offset.y =
        s2f(get_textbox_text(frm_sprite_tra, "txt_y"));
    cur_sprite->scale.x =
        s2f(get_textbox_text(frm_sprite_tra, "txt_sx"));
    cur_sprite->scale.y =
        s2f(get_textbox_text(frm_sprite_tra, "txt_sy"));
    cur_sprite->angle =
        get_angle_picker_angle(frm_sprite_tra, "ang_a");
    comparison =
        get_checkbox_check(frm_sprite_tra, "chk_compare");
        
    comparison_blink =
        get_checkbox_check(frm_sprite_comp, "chk_compare_blink");
    comparison_above =
        get_checkbox_check(frm_sprite_comp, "chk_compare_above");
    comparison_tint =
        get_checkbox_check(frm_sprite_comp, "chk_tint");
        
    cur_sprite_tc.set_center(cur_sprite->offset);
    cur_sprite_tc.set_size(
        point(
            cur_sprite->file_size.x * cur_sprite->scale.x,
            cur_sprite->file_size.y * cur_sprite->scale.y
        )
    );
    cur_sprite_tc.set_angle(cur_sprite->angle);
    cur_sprite_tc.keep_aspect_ratio =
        get_checkbox_check(frm_sprite_tra, "chk_ratio");
        
    sprite_transform_to_gui();
    made_new_changes = true;
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
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Hides all menu frames.
 */
void animation_editor::hide_all_frames() {
    frm_main->hide();
    frm_picker->hide();
    frm_load->hide();
    frm_anims->hide();
    frm_sprites->hide();
    frm_sprite_bmp->hide();
    frm_sprite_tra->hide();
    frm_hitboxes->hide();
    frm_top->hide();
    frm_body_parts->hide();
    frm_tools->hide();
    frm_options->hide();
}


/* ----------------------------------------------------------------------------
 * Opens the frame where you pick from a list.
 * For the ID of the picker, use animation_editor::PICKER_*.
 * The content to use is decided from that.
 */
void animation_editor::open_picker(
    const unsigned char id, const bool can_make_new
) {
    vector<pair<string, string> > elements;
    string title;
    
    if(
        id == PICKER_LOAD_MOB_TYPE
    ) {
        for(unsigned char f = 0; f < N_MOB_CATEGORIES; ++f) {
            //0 is none.
            if(f == MOB_CATEGORY_NONE) continue;
            
            vector<string> names;
            mob_category* cat = mob_categories.get(f);
            cat->get_type_names(names);
            string cat_name = mob_categories.get(f)->plural_name;
            
            for(size_t n = 0; n < names.size(); ++n) {
                elements.push_back(make_pair(cat_name, names[n]));
            }
        }
        title = "Choose an object type.";
        
    } else if(
        id == PICKER_LOAD_GLOBAL_ANIM
    ) {
        vector<string> files =
            folder_to_vector(ANIMATIONS_FOLDER_PATH, false, NULL);
        for(size_t f = 0; f < files.size(); ++f) {
            elements.push_back(make_pair("", files[f]));
        }
        title = "Choose an animation.";
        
    } else if(
        id == PICKER_EDIT_ANIMATION ||
        id == PICKER_IMPORT_ANIMATION ||
        id == PICKER_RENAME_ANIMATION
    ) {
        for(size_t a = 0; a < anims.animations.size(); ++a) {
            elements.push_back(make_pair("", anims.animations[a]->name));
        }
        title = "Choose an animation.";
        
    } else if(
        id == PICKER_EDIT_SPRITE ||
        id == PICKER_SET_FRAME_SPRITE ||
        id == PICKER_IMPORT_SPRITE ||
        id == PICKER_IMPORT_SPRITE_BITMAP ||
        id == PICKER_IMPORT_SPRITE_TRANSFORMATION ||
        id == PICKER_IMPORT_SPRITE_HITBOXES ||
        id == PICKER_IMPORT_SPRITE_TOP ||
        id == PICKER_COMPARE_SPRITE ||
        id == PICKER_RENAME_SPRITE
    ) {
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            if(
                id == PICKER_IMPORT_SPRITE ||
                id == PICKER_IMPORT_SPRITE_BITMAP ||
                id == PICKER_IMPORT_SPRITE_TRANSFORMATION ||
                id == PICKER_IMPORT_SPRITE_HITBOXES ||
                id == PICKER_IMPORT_SPRITE_TOP ||
                id == PICKER_COMPARE_SPRITE
            ) {
                if(anims.sprites[s] == cur_sprite) continue;
            }
            elements.push_back(make_pair("", anims.sprites[s]->name));
        }
        title = "Choose a sprite.";
        
    }
    
    generate_and_open_picker(id, elements, title, can_make_new);
}


/* ----------------------------------------------------------------------------
 * Picks an element from the picker, closes the picker, and then
 * does something with the chosen element.
 */
void animation_editor::pick(
    const size_t picker_id, const string &name, const string &category
) {
    if(
        picker_id == PICKER_LOAD_MOB_TYPE
    ) {
        loaded_mob_type =
            mob_categories.get_from_pname(category)->get_type(name);
            
        file_path =
            TYPES_FOLDER_PATH + "/" +
            loaded_mob_type->category->plural_name + "/" +
            loaded_mob_type->folder_name + "/Animations.txt";
        load_animation_database(true);
        
    } else if(
        picker_id == PICKER_LOAD_GLOBAL_ANIM
    ) {
        loaded_mob_type = NULL;
        file_path = ANIMATIONS_FOLDER_PATH + "/" + name;
        load_animation_database(true);
        
    } else if(
        picker_id == PICKER_EDIT_ANIMATION
    ) {
        pick_animation(name);
        
    } else if(
        picker_id == PICKER_IMPORT_ANIMATION
    ) {
        import_animation_data(name);
        
    } else if(
        picker_id == PICKER_EDIT_SPRITE
    ) {
        pick_sprite(name);
        
    } else if(
        picker_id == PICKER_SET_FRAME_SPRITE
    ) {
        cur_anim->frames[cur_frame_nr].sprite_name =
            name;
        cur_anim->frames[cur_frame_nr].sprite_ptr =
            anims.sprites[anims.find_sprite(name)];
        frame_to_gui();
        
    } else if(
        picker_id == PICKER_IMPORT_SPRITE
    ) {
        import_sprite_file_data(name);
        import_sprite_transformation_data(name);
        import_sprite_hitbox_data(name);
        import_sprite_top_data(name);
        
    } else if(
        picker_id == PICKER_IMPORT_SPRITE_BITMAP
    ) {
        import_sprite_file_data(name);
        
    } else if(
        picker_id == PICKER_IMPORT_SPRITE_TRANSFORMATION
    ) {
        import_sprite_transformation_data(name);
        
    } else if(
        picker_id == PICKER_IMPORT_SPRITE_HITBOXES
    ) {
        import_sprite_hitbox_data(name);
        
    } else if(
        picker_id == PICKER_IMPORT_SPRITE_TOP
    ) {
        import_sprite_top_data(name);
        
    } else if(
        picker_id == PICKER_COMPARE_SPRITE
    ) {
        comparison_sprite = anims.sprites[anims.find_sprite(name)];
        sprite_transform_to_gui();
        
    } else if(
        picker_id == PICKER_RENAME_ANIMATION
    ) {
        set_button_text(frm_tools, "but_rename_anim_name", name);
        
    } else if(
        picker_id == PICKER_RENAME_SPRITE
    ) {
        set_button_text(frm_tools, "but_rename_sprite_name", name);
        
    }
    
    frm_toolbar->show();
    change_to_right_frame();
}


/* ----------------------------------------------------------------------------
 * Picks an animation to edit.
 */
void animation_editor::pick_animation(const string &name) {
    cur_anim = anims.animations[anims.find_animation(name)];
    cur_frame_nr =
        (cur_anim->frames.size()) ? 0 : INVALID;
    cur_sprite = NULL;
    cur_hitbox = NULL;
    cur_hitbox_nr = INVALID;
    animation_to_gui();
}


/* ----------------------------------------------------------------------------
 * Picks a sprite to edit.
 */
void animation_editor::pick_sprite(const string &name) {
    cur_sprite = anims.sprites[anims.find_sprite(name)];
    cur_hitbox = NULL;
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
        (lafi::frame*) frm_load->widgets["frm_list"];
        
    while(!f->widgets.empty()) {
        f->remove(f->widgets.begin()->first);
    }
    
    if(animation_editor_history.empty()) return;
    
    f->easy_reset();
    f->easy_row();
    
    for(size_t h = 0; h < animation_editor_history.size(); ++h) {
    
        string name = animation_editor_history[h];
        if(name.empty()) continue;
        
        string button_text;
        if(name.find(TYPES_FOLDER_PATH) != string::npos) {
            vector<string> path_parts = split(name, "/");
            if(
                path_parts.size() > 3 &&
                path_parts[path_parts.size() - 1] == "Animations.txt"
            ) {
                button_text =
                    path_parts[path_parts.size() - 3] + "/" +
                    path_parts[path_parts.size() - 2];
            }
        } else if(name.find(ANIMATIONS_FOLDER_PATH) != string::npos) {
            vector<string> path_parts = split(name, "/");
            if(!path_parts.empty()) {
                button_text = path_parts[path_parts.size() - 1];
            }
        }
        if(button_text.empty()) {
            button_text = get_cut_path(name);
        }
        
        lafi::button* b =
            new lafi::button(0, 0, 0, 0, button_text);
            
        auto lambda = [name, this] (lafi::widget*, int, int) {
            file_path = name;
            loaded_mob_type = NULL;
            load_animation_database(true);
        };
        b->left_mouse_click_handler = lambda;
        f->easy_add("but_" + i2s(h), b, 100, 32);
        f->easy_row(0);
    }
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
 * Updates the current hitbox's transformation controller, based on whether
 * we're using the side view or not.
 */
void animation_editor::update_cur_hitbox_tc() {
    if(!cur_hitbox) return;
    if(side_view) {
        cur_hitbox_tc.set_center(
            point(
                cur_hitbox->pos.x,
                (-(cur_hitbox->height / 2.0)) - cur_hitbox->z
            )
        );
        cur_hitbox_tc.set_size(
            point(cur_hitbox->radius * 2, cur_hitbox->height)
        );
    } else {
        cur_hitbox_tc.set_center(cur_hitbox->pos);
        cur_hitbox_tc.set_size(
            point(cur_hitbox->radius * 2, cur_hitbox->radius * 2)
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
