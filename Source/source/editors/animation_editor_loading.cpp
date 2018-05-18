/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor loading function.
 */

#include "animation_editor.h"
#include "../LAFI/angle_picker.h"
#include "../LAFI/button.h"
#include "../LAFI/checkbox.h"
#include "../LAFI/frame.h"
#include "../LAFI/minor.h"
#include "../LAFI/radio_button.h"
#include "../LAFI/textbox.h"
#include "../functions.h"
#include "../load.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Loads the animation editor.
 */
void animation_editor::load() {
    editor::load();
    
    fade_mgr.start_fade(true, nullptr);
    
    update_canvas_coordinates();
    state = EDITOR_STATE_MAIN;
    file_path.clear();
    
    load_custom_particle_generators(false);
    load_status_types(false);
    load_liquids(false);
    load_hazards();
    load_spike_damage_types();
    load_mob_types(false);
    
    lafi::style* s =
        new lafi::style(
        al_map_rgb(192, 192, 208),
        al_map_rgb(32, 32, 64),
        al_map_rgb(96, 128, 160),
        font_builtin
    );
    gui = new lafi::gui(scr_w, scr_h, s);
    
    
    //Main -- declarations.
    frm_main =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add(
        "lbl_file",
        new lafi::label("Current file:"), 100, 16
    );
    frm_main->easy_row();
    frm_main->easy_add(
        "but_file",
        new lafi::button(), 100, 32
    );
    int y = frm_main->easy_row();
    
    frm_object =
        new lafi::frame(canvas_br.x, y, scr_w, scr_h);
    frm_main->add("frm_object", frm_object);
    frm_object->easy_row();
    frm_object->easy_add(
        "but_anims",
        new lafi::button("Animations", "", editor_icons[ICON_ANIMATIONS]),
        50, 48
    );
    frm_object->easy_add(
        "but_sprites",
        new lafi::button("Sprites", "", editor_icons[ICON_SPRITES]),
        50, 48
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "dum_1", new lafi::dummy(), 25, 48
    );
    frm_object->easy_add(
        "but_body_parts",
        new lafi::button("Body parts", "", editor_icons[ICON_BODY_PARTS]),
        50, 48
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_tools",
        new lafi::button("Special tools", "", editor_icons[ICON_TOOLS]),
        50, 48
    );
    frm_object->easy_add(
        "but_options",
        new lafi::button("Options", "", editor_icons[ICON_OPTIONS]),
        50, 48
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_n_anims",
        new lafi::label(), 100, 12
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_n_sprites",
        new lafi::label(), 100, 12
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "lbl_n_body_parts",
        new lafi::label(), 100, 12
    );
    frm_object->easy_row();
    
    
    //Main -- properties.
    frm_main->widgets["but_file"]->left_mouse_click_handler =
    [this] (lafi::widget * w, int, int) {
        if(!check_new_unsaved_changes(w)) {
            state = EDITOR_STATE_LOAD;
            populate_history();
            frm_toolbar->hide();
            change_to_right_frame();
        }
    };
    frm_main->widgets["but_file"]->description =
        "Pick a file to load or create.";
        
    frm_object->widgets["but_anims"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_hitbox = NULL;
        cur_hitbox_nr = INVALID;
        if(cur_anim) {
            if(cur_anim->frames.size()) cur_frame_nr = 0;
        }
        state = EDITOR_STATE_ANIMATION;
        change_to_right_frame();
        animation_to_gui();
    };
    frm_object->widgets["but_anims"]->description =
        "Change the way the animations look like.";
        
    frm_object->widgets["but_sprites"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_SPRITE;
        cur_hitbox = NULL;
        cur_hitbox_nr = INVALID;
        change_to_right_frame();
        sprite_to_gui();
    };
    frm_object->widgets["but_sprites"]->description =
        "Change how each individual sprite looks like.";
        
    frm_object->widgets["but_body_parts"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_BODY_PART;
        change_to_right_frame();
        cur_body_part_nr = 0;
        body_part_to_gui();
    };
    frm_object->widgets["but_body_parts"]->description =
        "Change what body parts exist, and their order.";
        
    frm_object->widgets["but_tools"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_TOOLS;
        change_to_right_frame();
    };
    frm_object->widgets["but_tools"]->description =
        "Special tools to help with specific tasks.";
        
    frm_object->widgets["but_options"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_OPTIONS;
        change_to_right_frame();
    };
    frm_object->widgets["but_options"]->description =
        "Options for the area editor.";
        
        
    //History -- declarations.
    frm_history =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    frm_history->hide();
    gui->add("frm_history", frm_history);
    
    frm_history->easy_row();
    frm_history->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "lbl_load",
        new lafi::label("Load:"), 100, 16
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "but_object",
        new lafi::button("Object animation"), 100, 32
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "but_global",
        new lafi::button("Global animation"), 100, 32
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "but_browse",
        new lafi::button("Other..."), 100, 32
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "dum_1",
        new lafi::dummy(), 100, 12
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "lbl_hist",
        new lafi::label("History:"), 100, 16
    );
    y = frm_history->easy_row();
    frm_history->add(
        "frm_list",
        new lafi::frame(canvas_br.x, y, scr_w, scr_h)
    );
    
    
    //History -- properties.
    frm_history->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        frm_toolbar->show();
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_history->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_history->widgets["but_object"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_MOB_TYPES, false);
    };
    frm_history->widgets["but_object"]->description =
        "Load the animations of an object type.";
        
    frm_history->widgets["but_global"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_GLOBAL_ANIMS, false);
    };
    frm_history->widgets["but_global"]->description =
        "Load a global generic animation.";
        
    frm_history->widgets["but_browse"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        string last_file_opened;
        if(animation_editor_history.size()) {
            last_file_opened = animation_editor_history[0];
        }
        
        vector<string> f =
            prompt_file_dialog(
                last_file_opened,
                "Please choose an animation text file to load or create.",
                "*.txt", 0
            );
            
        if(f.empty() || f[0].empty()) {
            return;
        }
        
        file_path = f[0];
        
        loaded_mob_type = NULL;
        load_animation_database(true);
    };
    frm_history->widgets["but_browse"]->description =
        "Pick a file to load or create.";
        
        
    //Animations -- declarations.
    frm_anims =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    frm_anims->hide();
    gui->add("frm_anims", frm_anims);
    
    frm_anims->easy_row();
    frm_anims->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_anims->easy_row();
    frm_anims->easy_add(
        "lbl_anim",
        new lafi::label("Animation:"), 85, 16
    );
    frm_anims->easy_add(
        "but_del_anim",
        new lafi::button("", "", editor_icons[ICON_REMOVE]), 15, 32
    );
    frm_anims->easy_row();
    frm_anims->easy_add(
        "but_anim",
        new lafi::button(), 100, 32
    );
    y = frm_anims->easy_row();
    
    frm_anim =
        new lafi::frame(canvas_br.x, y, scr_w, scr_h);
    frm_anims->add("frm_anim", frm_anim);
    frm_anim->easy_row();
    frm_anim->easy_add(
        "lin_1",
        new lafi::line(), 15, 12
    );
    frm_anim->easy_add(
        "lbl_data",
        new lafi::label("Animation data", ALLEGRO_ALIGN_CENTER),
        70, 12
    );
    frm_anim->easy_add(
        "lin_2",
        new lafi::line(), 15, 12
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "lbl_loop",
        new lafi::label("Loop frame:"), 50, 16
    );
    frm_anim->easy_add(
        "txt_loop",
        new lafi::textbox(), 50, 16
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "chk_missable",
        new lafi::checkbox("Missable attack"), 100, 16
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "dum_1",
        new lafi::dummy(), 10, 16
    );
    frm_anim->easy_add(
        "lbl_hit_rate",
        new lafi::label("Hit rate:"), 50, 16
    );
    frm_anim->easy_add(
        "txt_hit_rate",
        new lafi::textbox(), 30, 16
    );
    frm_anim->easy_add(
        "lbl_hit_rate_p",
        new lafi::label("%"), 10, 16
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "lin_3",
        new lafi::line(), 25, 12
    );
    frm_anim->easy_add(
        "lbl_list",
        new lafi::label("Frame list", ALLEGRO_ALIGN_CENTER),
        50, 12
    );
    frm_anim->easy_add(
        "lin_4",
        new lafi::line(), 25, 12
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "lbl_f_nr",
        new lafi::label(), 100, 16
    );
    frm_anim->easy_row();
    frm_anim->easy_add(
        "but_play",
        new lafi::button("", "", editor_icons[ICON_PLAY_PAUSE]), 20, 32
    );
    frm_anim->easy_add(
        "but_prev",
        new lafi::button("", "", editor_icons[ICON_PREVIOUS]), 20, 32
    );
    frm_anim->easy_add(
        "but_next",
        new lafi::button("", "", editor_icons[ICON_NEXT]), 20, 32
    );
    frm_anim->easy_add(
        "but_add",
        new lafi::button("", "", editor_icons[ICON_ADD]), 20, 32
    );
    frm_anim->easy_add(
        "but_rem",
        new lafi::button("", "", editor_icons[ICON_REMOVE]), 20, 32
    );
    y += frm_anim->easy_row();
    
    frm_frame =
        new lafi::frame(canvas_br.x, y, scr_w, scr_h);
    frm_anim->add("frm_frame", frm_frame);
    
    frm_frame->easy_row();
    frm_frame->easy_add(
        "lbl_sprite",
        new lafi::label("Sprite:"), 30, 16
    );
    frm_frame->easy_add(
        "but_sprite",
        new lafi::button(), 70, 24
    );
    frm_frame->easy_row();
    frm_frame->easy_add(
        "lbl_dur",
        new lafi::label("Duration:"), 40, 16
    );
    frm_frame->easy_add(
        "txt_dur",
        new lafi::textbox(), 60, 16
    );
    frm_frame->easy_row();
    frm_frame->easy_add(
        "chk_signal",
        new lafi::checkbox("Signal"), 50, 16
    );
    frm_frame->easy_add(
        "txt_signal",
        new lafi::textbox(), 50, 16
    );
    frm_frame->easy_row();
    frm_frame->easy_add(
        "dum_1",
        new lafi::dummy(), 100, 16
    );
    frm_frame->easy_row();
    frm_frame->easy_add(
        "but_dur_all",
        new lafi::button("Apply duration to all"), 100, 24
    );
    frm_frame->easy_row();
    
    
    //Animations -- properties.
    auto lambda_gui_to_animation =
    [this] (lafi::widget*) { gui_to_animation(); };
    auto lambda_gui_to_frame =
    [this] (lafi::widget*) { gui_to_frame(); };
    
    frm_anims->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MAIN;
        anim_playing = false;
        change_to_right_frame();
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
        cur_hitbox = NULL;
        cur_hitbox_nr = INVALID;
        animation_to_gui();
        made_new_changes = true;
        emit_status_bar_message("Animation deleted.", false);
    };
    frm_anims->widgets["but_del_anim"]->description =
        "Delete the current animation.";
        
    frm_anims->widgets["but_anim"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        anim_playing = false;
        this->frm_anims->hide();
        open_picker(ANIMATION_EDITOR_PICKER_ANIMATION, true);
    };
    frm_anims->widgets["but_anim"]->description =
        "Pick an animation to edit.";
        
    frm_anim->widgets["txt_loop"]->lose_focus_handler =
        lambda_gui_to_animation;
    frm_anim->widgets["txt_loop"]->description =
        "The animation loops back to this frame when it ends.";
        
    frm_anim->widgets["chk_missable"]->left_mouse_click_handler =
    [this] (lafi::widget * c, int, int) {
        if(((lafi::checkbox*) c)->checked) {
            set_textbox_text(this->frm_anim, "txt_hit_rate", "50");
        }
        gui_to_animation();
    };
    frm_anim->widgets["chk_missable"]->description =
        "Is it an attack that can knock back Pikmin, but miss?";
        
    frm_anim->widgets["txt_hit_rate"]->lose_focus_handler =
        lambda_gui_to_animation;
    frm_anim->widgets["txt_hit_rate"]->description =
        "Chance that a Pikmin will actually be knocked back.";
        
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
        frame_to_gui();
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
        frame_to_gui();
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
        frame_to_gui();
        made_new_changes = true;
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
        frame_to_gui();
        made_new_changes = true;
    };
    frm_anim->widgets["but_rem"]->description =
        "Remove the current frame.";
        
    frm_frame->widgets["but_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        anim_playing = false;
        this->frm_anims->hide();
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, false);
    };
    frm_frame->widgets["but_sprite"]->description =
        "Pick the sprite to use for this frame.";
        
    frm_frame->widgets["txt_dur"]->lose_focus_handler =
        lambda_gui_to_frame;
    frm_frame->widgets["txt_dur"]->mouse_down_handler =
    [this] (lafi::widget*, int, int, int) {
        anim_playing = false;
    };
    frm_frame->widgets["txt_dur"]->description =
        "How long this frame lasts for, in seconds.";
        
    frm_frame->widgets["chk_signal"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        gui_to_frame();
    };
    
    frm_frame->widgets["chk_signal"]->description =
        "Does this frame send a signal to the script?";
        
    frm_frame->widgets["txt_signal"]->lose_focus_handler =
        lambda_gui_to_frame;
    frm_frame->widgets["txt_signal"]->description =
        "Number of the signal.";
        
    frm_frame->widgets["but_dur_all"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        float d = cur_anim->frames[cur_frame_nr].duration;
        for(size_t i = 0; i < cur_anim->frames.size(); ++i) {
            cur_anim->frames[i].duration = d;
        }
        emit_status_bar_message(
            "Applied the duration " + f2s(d) + " to all frames.", false
        );
    };
    frm_frame->widgets["but_dur_all"]->description =
        "Apply this duration to all frames on this animation.";
        
    frm_anims->register_accelerator(
        ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL,
        frm_anim->widgets["but_next"]
    );
    frm_anims->register_accelerator(
        ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_SHIFT,
        frm_anim->widgets["but_prev"]
    );
    
    
    //Sprites -- declarations.
    frm_sprites =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    frm_sprites->hide();
    gui->add("frm_sprites", frm_sprites);
    
    frm_sprites->easy_row();
    frm_sprites->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_sprites->easy_row();
    frm_sprites->easy_add(
        "lbl_sprite",
        new lafi::label("Sprite:"), 100, 8
    );
    frm_sprites->easy_row();
    frm_sprites->easy_add(
        "but_sprite",
        new lafi::button(), 100, 32
    );
    y = frm_sprites->easy_row();
    
    frm_sprite =
        new lafi::frame(canvas_br.x, y, scr_w, scr_h);
    frm_sprites->add("frm_sprite", frm_sprite);
    
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_prev_sprite",
        new lafi::button("", "", editor_icons[ICON_PREVIOUS]), 20, 32
    );
    frm_sprite->easy_add(
        "but_next_sprite",
        new lafi::button("", "", editor_icons[ICON_NEXT]), 20, 32
    );
    frm_sprite->easy_add(
        "but_del_sprite",
        new lafi::button("", "", editor_icons[ICON_REMOVE]), 20, 32
    );
    frm_sprite->easy_add(
        "but_import",
        new lafi::button("", "", editor_icons[ICON_DUPLICATE]), 20, 32
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "lin_1",
        new lafi::line(), 25, 12
    );
    frm_sprite->easy_add(
        "lbl_f_data",
        new lafi::label("Sprite data", ALLEGRO_ALIGN_CENTER), 50, 12
    );
    frm_sprite->easy_add(
        "lin_2",
        new lafi::line(), 25, 12
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_bitmap",
        new lafi::button("Bitmap file"), 100, 32
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_transform",
        new lafi::button("Transformations"), 100, 32
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_hitboxes",
        new lafi::button("Hitboxes"), 100, 32
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_top",
        new lafi::button("Pikmin top"), 100, 32
    );
    frm_sprite->easy_row();
    
    
    //Sprites -- properties.
    auto lambda_sprite_transform = [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_SPRITE_TRANSFORM;
        change_to_right_frame();
        comparison_sprite = NULL;
        sprite_transform_to_gui();
    };
    
    frm_sprites->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
        update_stats();
    };
    frm_sprites->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_sprites->widgets["but_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        picker_disambig = PICKER_DISAMBIG_LOAD;
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, true);
    };
    frm_sprites->widgets["but_sprite"]->description =
        "Pick a sprite to edit.";
        
    frm_sprite->widgets["but_prev_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_sprite && !anims.sprites.empty()) {
            pick_sprite(anims.sprites[0]->name);
        } else {
            size_t s = 0;
            for(; s < anims.sprites.size(); ++s) {
                if(anims.sprites[s]->name == cur_sprite->name) break;
            }
            s = sum_and_wrap(s, -1, anims.sprites.size());
            pick_sprite(anims.sprites[s]->name);
        }
    };
    frm_sprite->widgets["but_prev_sprite"]->description =
        "Jump to the previous sprite in the list.";
        
    frm_sprite->widgets["but_next_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_sprite && !anims.sprites.empty()) {
            pick_sprite(anims.sprites[0]->name);
        } else {
            size_t s = 0;
            for(; s < anims.sprites.size(); ++s) {
                if(anims.sprites[s]->name == cur_sprite->name) break;
            }
            s = sum_and_wrap(s, 1, anims.sprites.size());
            pick_sprite(anims.sprites[s]->name);
        }
    };
    frm_sprite->widgets["but_next_sprite"]->description =
        "Jump to the next sprite in the list.";
        
    frm_sprite->widgets["but_del_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(!cur_sprite) return;
        anims.sprites.erase(
            anims.sprites.begin() + anims.find_sprite(cur_sprite->name)
        );
        cur_sprite = NULL;
        cur_hitbox = NULL;
        cur_hitbox_nr = INVALID;
        sprite_to_gui();
        made_new_changes = true;
        emit_status_bar_message("Sprite deleted.", false);
    };
    frm_sprite->widgets["but_del_sprite"]->description =
        "Delete the current sprite.";
        
    frm_sprite->widgets["but_import"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        picker_disambig = PICKER_DISAMBIG_IMPORT;
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, false);
    };
    frm_sprite->widgets["but_import"]->description =
        "Import the data from another sprite.";
        
    frm_sprite->widgets["but_bitmap"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_SPRITE_BITMAP;
        sprite_bmp_to_gui();
        change_to_right_frame();
    };
    frm_sprite->widgets["but_bitmap"]->description =
        "Pick what part of an image file makes up this sprite.";
        
    frm_sprite->widgets["but_transform"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_sprite_tc.set_center(cur_sprite->offset);
        cur_sprite_tc.set_size(
            point(
                cur_sprite->file_size.x * cur_sprite->scale.x,
                cur_sprite->file_size.y * cur_sprite->scale.y
            )
        );
        cur_sprite_tc.set_angle(cur_sprite->angle);
        state = EDITOR_STATE_SPRITE_TRANSFORM;
        sprite_transform_to_gui();
        change_to_right_frame();
    };
    frm_sprite->widgets["but_transform"]->description =
        "Offset, scale, or rotate the sprite's image.";
        
    frm_sprite->widgets["but_hitboxes"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_HITBOXES;
        if(cur_sprite && !cur_sprite->hitboxes.empty()) {
            cur_hitbox = &cur_sprite->hitboxes[0];
            cur_hitbox_nr = 0;
        }
        hitbox_to_gui();
        change_to_right_frame();
    };
    frm_sprite->widgets["but_hitboxes"]->description =
        "Edit this frame's hitboxes.";
        
    frm_sprite->widgets["but_top"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        top_tc.set_center(cur_sprite->top_pos);
        top_tc.set_size(cur_sprite->top_size);
        state = EDITOR_STATE_TOP;
        change_to_right_frame();
        top_to_gui();
    };
    frm_sprite->widgets["but_top"]->description =
        "Edit the Pikmin's top (maturity) for this sprite.";
        
        
    //Sprite bitmap -- declarations.
    frm_sprite_bmp =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    frm_sprite_bmp->hide();
    gui->add("frm_sprite_bmp", frm_sprite_bmp);
    
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "but_import",
        new lafi::button("", "", editor_icons[ICON_DUPLICATE]), 20, 32
    );
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "lbl_file",
        new lafi::label("File:"), 25, 16
    );
    frm_sprite_bmp->easy_add(
        "txt_file",
        new lafi::textbox(), 60, 16
    );
    frm_sprite_bmp->easy_add(
        "but_file",
        new lafi::button("..."), 15, 16
    );
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "lbl_xy",
        new lafi::label("X&Y:"), 40, 16
    );
    frm_sprite_bmp->easy_add(
        "txt_x",
        new lafi::textbox(), 30, 16
    );
    frm_sprite_bmp->easy_add(
        "txt_y",
        new lafi::textbox(), 30, 16
    );
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "lbl_wh",
        new lafi::label("W&H:"), 40, 16
    );
    frm_sprite_bmp->easy_add(
        "txt_w",
        new lafi::textbox(), 30, 16
    );
    frm_sprite_bmp->easy_add(
        "txt_h",
        new lafi::textbox(), 30, 16
    );
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "dum_1",
        new lafi::dummy(), 100, 16
    );
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "lbl_click1",
        new lafi::label("Click parts of the image"), 100, 12
    );
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "lbl_click2",
        new lafi::label("on the left to expand"), 100, 12
    );
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "lbl_click3",
        new lafi::label("the selection limits."), 100, 12
    );
    frm_sprite_bmp->easy_row();
    frm_sprite_bmp->easy_add(
        "but_clear",
        new lafi::button("Clear selection"), 100, 16
    );
    frm_sprite_bmp->easy_row();
    
    
    //Sprite bitmap -- properties.
    auto lambda_gui_to_sprite_bmp =
    [this] (lafi::widget*) { gui_to_sprite_bmp(); };
    
    frm_sprite_bmp->widgets["but_back"]->description =
        "Go back to the sprite editor.";
    frm_sprite_bmp->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_SPRITE;
        change_to_right_frame();
    };
    
    frm_sprite_bmp->widgets["but_import"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, false);
    };
    frm_sprite_bmp->widgets["but_import"]->description =
        "Import bitmap data from a different sprite.";
        
    frm_sprite_bmp->widgets["txt_file"]->lose_focus_handler =
        lambda_gui_to_sprite_bmp;
    frm_sprite_bmp->widgets["txt_file"]->description =
        "Name (+extension) of the file with the sprite.";
        
    frm_sprite_bmp->widgets["but_file"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        vector<string> f =
            prompt_file_dialog(
                GRAPHICS_FOLDER_PATH + "/",
                "Please choose the bitmap to get the sprites from.",
                "*.png",
                ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                ALLEGRO_FILECHOOSER_PICTURES
            );
            
        if(f.empty() || f[0].empty()) {
            return;
        }
        
        size_t folder_pos = f[0].find(GRAPHICS_FOLDER_PATH);
        if(folder_pos == string::npos) {
            //This isn't in the graphics folder!
            emit_status_bar_message(
                "The chosen image is not in the graphics folder!",
                true
            );
            return;
        } else {
            f[0] =
                f[0].substr(
                    folder_pos + GRAPHICS_FOLDER_PATH.size() + 1,
                    string::npos
                );
        }
        
        set_textbox_text(this->frm_sprite_bmp, "txt_file", f[0]);
        this->frm_sprite_bmp->widgets["txt_file"]->call_lose_focus_handler();
    };
    frm_sprite_bmp->widgets["but_file"]->description =
        "Browse for the file to use, in the Graphics folder.";
        
    frm_sprite_bmp->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_to_sprite_bmp;
    frm_sprite_bmp->widgets["txt_x"]->description =
        "X of the top-left corner of the sprite.";
        
    frm_sprite_bmp->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_to_sprite_bmp;
    frm_sprite_bmp->widgets["txt_y"]->description =
        "Y of the top-left corner of the sprite.";
        
    frm_sprite_bmp->widgets["txt_w"]->lose_focus_handler =
        lambda_gui_to_sprite_bmp;
    frm_sprite_bmp->widgets["txt_w"]->description =
        "Width of the sprite, in the file.";
        
    frm_sprite_bmp->widgets["txt_h"]->lose_focus_handler =
        lambda_gui_to_sprite_bmp;
    frm_sprite_bmp->widgets["txt_h"]->description =
        "Height of the sprite, in the file.";
        
    frm_sprite_bmp->widgets["but_clear"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_sprite->file_pos = point();
        cur_sprite->file_size = point();
        sprite_bmp_to_gui();
    };
    frm_sprite_bmp->widgets["but_clear"]->description =
        "Clear the selection so you can start your clicks over.";
        
        
    //Sprite transform -- declarations.
    frm_sprite_tra =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    frm_sprite_tra->hide();
    gui->add("frm_sprite_tra", frm_sprite_tra);
    
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "but_import",
        new lafi::button("", "", editor_icons[ICON_DUPLICATE]), 20, 32
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "lbl_xy",
        new lafi::label("X, Y:"), 25, 16
    );
    frm_sprite_tra->easy_add(
        "txt_x",
        new lafi::textbox(), 37.5, 16
    );
    frm_sprite_tra->easy_add(
        "txt_y",
        new lafi::textbox(), 37.5, 16
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "lbl_scale",
        new lafi::label("Scale:"), 25, 16
    );
    frm_sprite_tra->easy_add(
        "txt_sx",
        new lafi::textbox(), 37.5, 16
    );
    frm_sprite_tra->easy_add(
        "txt_sy",
        new lafi::textbox(), 37.5, 16
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "chk_ratio",
        new lafi::checkbox("Keep aspect ratio", true), 100, 16
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "lbl_angle",
        new lafi::label("Angle:"), 50, 16
    );
    frm_sprite_tra->easy_add(
        "ang_a",
        new lafi::angle_picker(), 50, 24
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "dum_1",
        new lafi::dummy(), 100, 8
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "chk_compare",
        new lafi::checkbox("Comparison sprite"), 100, 16
    );
    y = frm_sprite_tra->easy_row();
    
    frm_sprite_comp =
        new lafi::frame(canvas_br.x, y, scr_w, scr_h);
    frm_sprite_tra->add("frm_sprite_comp", frm_sprite_comp);
    
    frm_sprite_comp->easy_row();
    frm_sprite_comp->easy_add(
        "but_compare",
        new lafi::button(), 100, 24
    );
    frm_sprite_comp->easy_row();
    frm_sprite_comp->easy_add(
        "chk_compare_blink",
        new lafi::checkbox("Blink comparison"), 100, 16
    );
    frm_sprite_comp->easy_row();
    frm_sprite_comp->easy_add(
        "chk_compare_above",
        new lafi::checkbox("Comparison above"), 100, 16
    );
    frm_sprite_comp->easy_row();
    frm_sprite_comp->easy_add(
        "chk_tint",
        new lafi::checkbox("Tint both"), 100, 16
    );
    frm_sprite_comp->easy_row();
    
    
    //Sprite transform -- properties.
    auto lambda_save_sprite_tra =
    [this] (lafi::widget*) { gui_to_sprite_transform(); };
    auto lambda_save_sprite_tra_click =
    [this] (lafi::widget*, int, int) { gui_to_sprite_transform(); };
    
    frm_sprite_tra->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        this->comparison_sprite = NULL;
        state = EDITOR_STATE_SPRITE;
        change_to_right_frame();
        sprite_to_gui();
    };
    frm_sprite_tra->widgets["but_back"]->description =
        "Go back to the sprite editor.";
        
    frm_sprite_tra->widgets["but_import"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        picker_disambig = PICKER_DISAMBIG_IMPORT;
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, false);
    };
    frm_sprite_tra->widgets["but_import"]->description =
        "Import transformation data from a different sprite.";
        
    frm_sprite_tra->widgets["txt_x"]->lose_focus_handler =
        lambda_save_sprite_tra;
    frm_sprite_tra->widgets["txt_x"]->description =
        "Offset the sprite's graphic by this much, horizontally.";
        
    frm_sprite_tra->widgets["txt_y"]->lose_focus_handler =
        lambda_save_sprite_tra;
    frm_sprite_tra->widgets["txt_y"]->description =
        "Offset the sprite's graphic by this much, vertically.";
        
    frm_sprite_tra->widgets["txt_sx"]->lose_focus_handler =
        lambda_save_sprite_tra;
    frm_sprite_tra->widgets["txt_sx"]->description =
        "Scale the sprite's graphic width by this.";
        
    frm_sprite_tra->widgets["txt_sy"]->lose_focus_handler =
        lambda_save_sprite_tra;
    frm_sprite_tra->widgets["txt_sy"]->description =
        "Scale the sprite's graphic height by this.";
        
    frm_sprite_tra->widgets["chk_ratio"]->left_mouse_click_handler =
        lambda_save_sprite_tra_click;
    frm_sprite_tra->widgets["chk_ratio"]->description =
        "Lock width/height proportion when changing either one.";
        
    frm_sprite_tra->widgets["ang_a"]->lose_focus_handler =
        lambda_save_sprite_tra;
    frm_sprite_tra->widgets["ang_a"]->description =
        "Rotate the sprite's graphic by this angle.";
        
    frm_sprite_tra->widgets["chk_compare"]->left_mouse_click_handler =
        lambda_save_sprite_tra_click;
    frm_sprite_tra->widgets["chk_compare"]->description =
        "Overlay a different sprite for comparison purposes. (Ctrl+C)";
        
    frm_sprite_comp->widgets["but_compare"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        picker_disambig = PICKER_DISAMBIG_COMPARISON;
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, false);
    };
    frm_sprite_comp->widgets["but_compare"]->description =
        "Sprite to compare with.";
        
    frm_sprite_comp->widgets["chk_compare_blink"]->left_mouse_click_handler =
        lambda_save_sprite_tra_click;
    frm_sprite_comp->widgets["chk_compare_blink"]->description =
        "Blink the comparison in and out?";
        
    frm_sprite_comp->widgets["chk_compare_above"]->left_mouse_click_handler =
        lambda_save_sprite_tra_click;
    frm_sprite_comp->widgets["chk_compare_above"]->description =
        "Should the comparison appear above or below the working sprite?";
        
    frm_sprite_comp->widgets["chk_tint"]->left_mouse_click_handler =
        lambda_save_sprite_tra_click;
    frm_sprite_comp->widgets["chk_tint"]->description =
        "Tint the working sprite blue and the comparison orange.";
        
        
    //Hitboxes -- declarations.
    frm_hitboxes =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    frm_hitboxes->hide();
    gui->add("frm_hitboxes", frm_hitboxes);
    
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add(
        "but_prev",
        new lafi::button("", "", editor_icons[ICON_PREVIOUS]), 20, 32
    );
    frm_hitboxes->easy_add(
        "but_next",
        new lafi::button("", "", editor_icons[ICON_NEXT]), 20, 32
    );
    frm_hitboxes->easy_add(
        "but_import",
        new lafi::button("", "", editor_icons[ICON_DUPLICATE]), 20, 32
    );
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add(
        "lbl_n",
        new lafi::label("Hitbox:"), 30, 24
    );
    frm_hitboxes->easy_add(
        "lbl_name",
        new lafi::label(), 70, 24
    );
    y = frm_hitboxes->easy_row();
    
    frm_hitbox =
        new lafi::frame(canvas_br.x, y, scr_w, scr_h);
    frm_hitboxes->add("frm_hitbox", frm_hitbox);
    
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "lbl_xy",
        new lafi::label("X, Y:"), 45, 16
    );
    frm_hitbox->easy_add(
        "txt_x",
        new lafi::textbox(), 27.5, 16
    );
    frm_hitbox->easy_add(
        "txt_y",
        new lafi::textbox(), 27.5, 16
    );
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "lbl_r",
        new lafi::label("Radius:"), 45, 16
    );
    frm_hitbox->easy_add(
        "txt_r",
        new lafi::textbox(), 55, 16
    );
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "lbl_zh",
        new lafi::label("Z, Height:"), 45, 16
    );
    frm_hitbox->easy_add(
        "txt_z",
        new lafi::textbox(), 27.5, 16
    );
    frm_hitbox->easy_add(
        "txt_h",
        new lafi::textbox(), 27.5, 16
    );
    frm_hitbox->easy_row();
    
    frm_hitbox->easy_add(
        "lbl_h_type",
        new lafi::label("Hitbox type:"), 100, 12
    );
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "rad_normal",
        new lafi::radio_button("Normal"), 50, 16
    );
    frm_hitbox->easy_add(
        "rad_attack",
        new lafi::radio_button("Attack"), 50, 16
    );
    frm_hitbox->easy_row();
    frm_hitbox->easy_add(
        "rad_disabled",
        new lafi::radio_button("Disabled"), 100, 16
    );
    y += frm_hitbox->easy_row();
    
    frm_normal_h =
        new lafi::frame(canvas_br.x, y, scr_w, scr_h);
    frm_normal_h->hide();
    frm_hitbox->add("frm_normal_h", frm_normal_h);
    
    frm_normal_h->easy_row();
    frm_normal_h->easy_add(
        "lbl_mult",
        new lafi::label("Defense mult.:"), 60, 16
    );
    frm_normal_h->easy_add(
        "txt_mult",
        new lafi::textbox(), 40, 16
    );
    frm_normal_h->easy_row();
    frm_normal_h->easy_add(
        "chk_latch",
        new lafi::checkbox("Pikmin can latch"), 100, 16
    );
    frm_normal_h->easy_row();
    frm_normal_h->easy_add(
        "lbl_hazards",
        new lafi::label("Hazards:"), 100, 12
    );
    frm_normal_h->easy_row();
    frm_normal_h->easy_add(
        "txt_hazards",
        new lafi::textbox(), 100, 16
    );
    frm_normal_h->easy_row();
    
    frm_attack_h =
        new lafi::frame(canvas_br.x, y, scr_w, scr_h);
    frm_attack_h->hide();
    frm_hitbox->add("frm_attack_h", frm_attack_h);
    
    frm_attack_h->easy_row();
    frm_attack_h->easy_add(
        "lbl_value",
        new lafi::label("Power:"), 60, 16
    );
    frm_attack_h->easy_add(
        "txt_value",
        new lafi::textbox(), 40, 16
    );
    frm_attack_h->easy_row();
    frm_attack_h->easy_add(
        "lbl_hazards",
        new lafi::label("Hazards:"), 100, 12
    );
    frm_attack_h->easy_row();
    frm_attack_h->easy_add(
        "txt_hazards",
        new lafi::textbox(), 100, 16
    );
    frm_attack_h->easy_row();
    frm_attack_h->easy_add(
        "chk_outward",
        new lafi::checkbox("Outward knockback"), 100, 16
    );
    frm_attack_h->easy_row();
    frm_attack_h->easy_add(
        "lbl_angle",
        new lafi::label("KB angle:"), 60, 16
    );
    frm_attack_h->easy_add(
        "ang_angle",
        new lafi::angle_picker(), 40, 24
    );
    frm_attack_h->easy_row();
    frm_attack_h->easy_add(
        "lbl_knockback",
        new lafi::label("KB strength:"), 60, 16
    );
    frm_attack_h->easy_add(
        "txt_knockback",
        new lafi::textbox(), 40, 16
    );
    frm_attack_h->easy_row();
    frm_attack_h->easy_add(
        "lbl_wither",
        new lafi::label("Wither chance:"), 60, 16
    );
    frm_attack_h->easy_add(
        "txt_wither",
        new lafi::textbox(), 30, 16
    );
    frm_attack_h->easy_add(
        "lbl_wither_per",
        new lafi::label("%"), 10, 16
    );
    frm_attack_h->easy_row();
    
    
    //Hitboxes -- properties.
    auto lambda_gui_to_hitbox_instance =
    [this] (lafi::widget*) { gui_to_hitbox(); };
    auto lambda_gui_to_hitbox_instance_click =
    [this] (lafi::widget*, int, int) { gui_to_hitbox(); };
    
    frm_hitboxes->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_SPRITE;
        change_to_right_frame();
        cur_hitbox = NULL;
        cur_hitbox_nr = INVALID;
        update_stats();
    };
    frm_hitboxes->widgets["but_back"]->description =
        "Go back to the frame editor.";
        
    frm_hitboxes->widgets["but_prev"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        gui_to_hitbox();
        if(cur_sprite->hitboxes.size()) {
            if(!cur_hitbox) {
                cur_hitbox = &cur_sprite->hitboxes[0];
                cur_hitbox_nr = 0;
            } else {
                cur_hitbox_nr =
                    sum_and_wrap(
                        cur_hitbox_nr, -1, cur_sprite->hitboxes.size()
                    );
                cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_nr];
            }
        }
        hitbox_to_gui();
    };
    frm_hitboxes->widgets["but_prev"]->description =
        "Previous hitbox.";
        
    frm_hitboxes->widgets["but_next"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        gui_to_hitbox();
        if(cur_sprite->hitboxes.size()) {
            if(cur_hitbox_nr == INVALID) {
                cur_hitbox = &cur_sprite->hitboxes[0];
                cur_hitbox_nr = 0;
            } else {
                cur_hitbox_nr =
                    sum_and_wrap(cur_hitbox_nr, 1, cur_sprite->hitboxes.size());
                cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_nr];
            }
        }
        hitbox_to_gui();
    };
    frm_hitboxes->widgets["but_next"]->description =
        "Next hitbox.";
        
    frm_hitboxes->widgets["but_import"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, false);
    };
    frm_hitboxes->widgets["but_import"]->description =
        "Import hitbox data from another sprite.";
        
    frm_hitbox->register_accelerator(
        ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL,
        frm_hitboxes->widgets["but_next"]
    );
    frm_hitbox->register_accelerator(
        ALLEGRO_KEY_TAB, ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_SHIFT,
        frm_hitboxes->widgets["but_prev"]
    );
    
    frm_hitbox->widgets["txt_x"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_hitbox->widgets["txt_x"]->description =
        "X of the hitbox's center.";
        
    frm_hitbox->widgets["txt_y"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_hitbox->widgets["txt_y"]->description =
        "Y of the hitbox's center.";
        
    frm_hitbox->widgets["txt_z"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_hitbox->widgets["txt_z"]->description =
        "Altitude of the hitbox's bottom.";
        
    frm_hitbox->widgets["txt_h"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_hitbox->widgets["txt_h"]->description =
        "Hitbox's height. 0 = spans infinitely vertically.";
        
    frm_hitbox->widgets["txt_r"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_hitbox->widgets["txt_r"]->description =
        "Hitbox's radius.";
        
    frm_hitbox->widgets["rad_normal"]->left_mouse_click_handler =
        lambda_gui_to_hitbox_instance_click;
    frm_hitbox->widgets["rad_normal"]->description =
        "Normal hitbox, one that can be damaged.";
        
    frm_hitbox->widgets["rad_attack"]->left_mouse_click_handler =
        lambda_gui_to_hitbox_instance_click;
    frm_hitbox->widgets["rad_attack"]->description =
        "Attack hitbox, one that damages opponents.";
        
    frm_hitbox->widgets["rad_disabled"]->left_mouse_click_handler =
        lambda_gui_to_hitbox_instance_click;
    frm_hitbox->widgets["rad_disabled"]->description =
        "This hitbox will be non-existent.";
        
    frm_normal_h->widgets["txt_mult"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_normal_h->widgets["txt_mult"]->description =
        "Defense multiplier. 0 = invulnerable.";
        
    frm_normal_h->widgets["chk_latch"]->left_mouse_click_handler =
        lambda_gui_to_hitbox_instance_click;
    frm_normal_h->widgets["chk_latch"]->description =
        "Can the Pikmin latch on to this hitbox?";
        
    frm_normal_h->widgets["txt_hazards"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_normal_h->widgets["txt_hazards"]->description =
        "List of hazards, semicolon separated.";
        
    frm_attack_h->widgets["txt_value"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_attack_h->widgets["txt_value"]->description =
        "Attack power, in hit points.";
        
    frm_attack_h->widgets["txt_hazards"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_attack_h->widgets["txt_hazards"]->description =
        "List of hazards, semicolon separated.";
        
    frm_attack_h->widgets["chk_outward"]->left_mouse_click_handler =
        lambda_gui_to_hitbox_instance_click;
    frm_attack_h->widgets["chk_outward"]->description =
        "Makes Pikmin be knocked away from the center.";
        
    frm_attack_h->widgets["ang_angle"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_attack_h->widgets["ang_angle"]->description =
        "Angle the Pikmin are knocked towards.";
        
    frm_attack_h->widgets["txt_knockback"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_attack_h->widgets["txt_knockback"]->description =
        "Knockback strength.";
        
    frm_attack_h->widgets["txt_wither"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_attack_h->widgets["txt_wither"]->description =
        "Chance of the attack lowering a Pikmin's maturity by one.";
        
        
    //Pikmin top -- declarations.
    frm_top =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    frm_top->hide();
    gui->add("frm_top", frm_top);
    
    frm_top->easy_row();
    frm_top->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "but_import",
        new lafi::button("", "", editor_icons[ICON_DUPLICATE]), 20, 32
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "chk_visible",
        new lafi::checkbox("Visible"), 100, 16
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "lbl_xy",
        new lafi::label("X&Y:"), 20, 16
    );
    frm_top->easy_add(
        "txt_x",
        new lafi::textbox(), 40, 16
    );
    frm_top->easy_add(
        "txt_y",
        new lafi::textbox(), 40, 16
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "lbl_wh",
        new lafi::label("W&H:"), 20, 16
    );
    frm_top->easy_add(
        "txt_w",
        new lafi::textbox(), 40, 16
    );
    frm_top->easy_add(
        "txt_h",
        new lafi::textbox(), 40, 16
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "chk_ratio",
        new lafi::checkbox("Keep aspect ratio", true), 100, 16
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "lbl_angle",
        new lafi::label("Angle:"), 40, 16
    );
    frm_top->easy_add(
        "ang_angle",
        new lafi::angle_picker(), 60, 24
    );
    frm_top->easy_row();
    frm_top->easy_add(
        "but_maturity",
        new lafi::button("Change maturity"), 100, 24
    );
    frm_top->easy_row();
    
    
    //Pikmin top -- properties.
    auto lambda_save_top =
    [this] (lafi::widget*) { gui_to_top(); };
    auto lambda_save_top_click =
    [this] (lafi::widget*, int, int) { gui_to_top(); };
    
    frm_top->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_SPRITE;
        change_to_right_frame();
    };
    frm_top->widgets["but_back"]->description =
        "Go back to the sprite editor.";
        
    frm_top->widgets["but_import"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, false);
    };
    frm_top->widgets["but_import"]->description =
        "Import transformation data from a different sprite.";
        
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
        
    frm_top->widgets["chk_ratio"]->left_mouse_click_handler =
        lambda_save_top_click;
    frm_top->widgets["chk_ratio"]->description =
        "Lock width/height proportion when changing either one.";
        
    frm_top->widgets["ang_angle"]->lose_focus_handler =
        lambda_save_top;
    frm_top->widgets["ang_angle"]->description =
        "Angle of the top.";
        
    frm_top->widgets["but_maturity"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_maturity = sum_and_wrap(cur_maturity, 1, N_MATURITIES);
    };
    frm_top->widgets["but_maturity"]->description =
        "View a different maturity top.";
        
        
    //Body parts -- declarations.
    frm_body_parts =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    frm_body_parts->hide();
    gui->add("frm_body_parts", frm_body_parts);
    
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_inst1",
        new lafi::label("The lower a part's"), 100, 12
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_inst2",
        new lafi::label("number, the more"), 100, 12
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_inst3",
        new lafi::label("priority it has when"), 100, 12
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_inst4",
        new lafi::label("checking collisions."), 100, 12
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "dummy",
        new lafi::dummy(), 100, 16
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "txt_add",
        new lafi::textbox(), 80, 16
    );
    frm_body_parts->easy_add(
        "but_add",
        new lafi::button("", "", editor_icons[ICON_ADD]), 20, 32
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "but_prev",
        new lafi::button("", "", editor_icons[ICON_PREVIOUS]), 20, 32
    );
    frm_body_parts->easy_add(
        "but_next",
        new lafi::button("", "", editor_icons[ICON_NEXT]), 20, 32
    );
    frm_body_parts->easy_add(
        "but_del",
        new lafi::button("", "", editor_icons[ICON_REMOVE]), 20, 32
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_n",
        new lafi::label("Part nr:"), 50, 16
    );
    frm_body_parts->easy_add(
        "lbl_nr",
        new lafi::label(), 50, 16
    );
    y = frm_body_parts->easy_row();
    
    frm_body_part =
        new lafi::frame(canvas_br.x, y, scr_w, scr_h);
    frm_body_parts->add("frm_body_part", frm_body_part);
    
    frm_body_part->easy_row();
    frm_body_part->easy_add(
        "lbl_na",
        new lafi::label("Name:"), 30, 16
    );
    frm_body_part->easy_add(
        "txt_name",
        new lafi::textbox(), 70, 16
    );
    frm_body_part->easy_row();
    frm_body_part->easy_add(
        "but_left",
        new lafi::button("", "", editor_icons[ICON_MOVE_LEFT]), 20, 32
    );
    frm_body_part->easy_add(
        "but_right",
        new lafi::button("", "", editor_icons[ICON_MOVE_RIGHT]), 20, 32
    );
    frm_body_part->easy_row();
    
    
    //Body parts -- properties.
    frm_body_parts->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
        update_stats();
    };
    frm_body_parts->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    ((lafi::textbox*) frm_body_parts->widgets["txt_add"])->enter_key_widget =
        frm_body_parts->widgets["but_add"];
    frm_body_parts->widgets["txt_add"]->description =
        "Name of the body part you want to create.";
        
    frm_body_parts->widgets["but_add"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        string name =
            get_textbox_text(this->frm_body_parts, "txt_add");
        set_textbox_text(this->frm_body_parts, "txt_add", "");
        if(name.empty()) return;
        for(size_t b = 0; b < anims.body_parts.size(); ++b) {
            if(anims.body_parts[b]->name == name) {
                cur_body_part_nr = b;
                body_part_to_gui();
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
        body_part_to_gui();
        made_new_changes = true;
    };
    frm_body_parts->widgets["but_add"]->description =
        "Create a new body part (after the current one).";
        
    frm_body_parts->widgets["but_prev"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(anims.body_parts.empty()) return;
        cur_body_part_nr =
            sum_and_wrap(cur_body_part_nr, -1, anims.body_parts.size());
        body_part_to_gui();
    };
    frm_body_parts->widgets["but_prev"]->description =
        "Previous body part.";
        
    frm_body_parts->widgets["but_next"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(anims.body_parts.empty()) return;
        cur_body_part_nr =
            sum_and_wrap(cur_body_part_nr, 1, anims.body_parts.size());
        body_part_to_gui();
    };
    frm_body_parts->widgets["but_next"]->description =
        "Next body part.";
        
    frm_body_part->widgets["txt_name"]->lose_focus_handler =
    [this] (lafi::widget * t) {
        string new_name = ((lafi::textbox*) t)->text;
        if(new_name.empty()) {
            body_part_to_gui();
            return;
        }
        for(size_t b = 0; b < anims.body_parts.size(); ++b) {
            if(b == cur_body_part_nr) continue;
            if(anims.body_parts[b]->name == new_name) {
                body_part_to_gui();
                return;
            }
        }
        anims.body_parts[cur_body_part_nr]->name = new_name;
        update_hitboxes();
        body_part_to_gui();
        made_new_changes = true;
    };
    frm_body_part->widgets["txt_name"]->description =
        "Name of this body part.";
        
    frm_body_part->widgets["but_left"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(anims.body_parts.size() < 2) return;
        size_t prev_nr =
            sum_and_wrap(cur_body_part_nr, -1, anims.body_parts.size());
        body_part* cur_bp = anims.body_parts[cur_body_part_nr];
        anims.body_parts.erase(anims.body_parts.begin() + cur_body_part_nr);
        anims.body_parts.insert(anims.body_parts.begin() + prev_nr, cur_bp);
        cur_body_part_nr = prev_nr;
        update_hitboxes();
        body_part_to_gui();
        made_new_changes = true;
    };
    frm_body_part->widgets["but_left"]->description =
        "Move this part to the left in the list.";
        
    frm_body_part->widgets["but_right"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(anims.body_parts.size() < 2) return;
        size_t next_nr =
            sum_and_wrap(cur_body_part_nr, 1, anims.body_parts.size());
        body_part* cur_bp = anims.body_parts[cur_body_part_nr];
        anims.body_parts.erase(anims.body_parts.begin() + cur_body_part_nr);
        anims.body_parts.insert(anims.body_parts.begin() + next_nr, cur_bp);
        cur_body_part_nr = next_nr;
        update_hitboxes();
        body_part_to_gui();
        made_new_changes = true;
    };
    frm_body_part->widgets["but_right"]->description =
        "Move this part to the right in the list.";
        
    frm_body_parts->widgets["but_del"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(cur_body_part_nr == INVALID || anims.body_parts.empty()) return;
        delete anims.body_parts[cur_body_part_nr];
        anims.body_parts.erase(anims.body_parts.begin() + cur_body_part_nr);
        if(cur_body_part_nr > 0) cur_body_part_nr--;
        update_hitboxes();
        body_part_to_gui();
        made_new_changes = true;
    };
    frm_body_parts->widgets["but_del"]->description =
        "Delete this body part.";
        
        
    //Tools -- declarations.
    frm_tools =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    frm_tools->hide();
    gui->add("frm_tools", frm_tools);
    
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_resize",
        new lafi::label("Resize everything:"), 100, 8
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "txt_resize",
        new lafi::textbox(), 80, 16
    );
    frm_tools->easy_add(
        "but_resize",
        new lafi::button("Ok"), 20, 24
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_set_scales",
        new lafi::label("Set all sprite scales:"), 100, 8
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "txt_set_scales",
        new lafi::textbox(), 80, 16
    );
    frm_tools->easy_add(
        "but_set_scales",
        new lafi::button("Ok"), 20, 24
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_rename_anim_1",
        new lafi::label("Rename animation:"), 100, 12
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_rename_anim_name",
        new lafi::button(), 100, 24
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_rename_anim_2",
        new lafi::label("To:"), 15, 16
    );
    frm_tools->easy_add(
        "txt_rename_anim",
        new lafi::textbox(), 65, 16
    );
    frm_tools->easy_add(
        "but_rename_anim_ok",
        new lafi::button("Ok"), 20, 24
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_rename_sprite_1",
        new lafi::label("Rename sprite:"), 100, 12
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "but_rename_sprite_name",
        new lafi::button(), 100, 24
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_rename_sprite_2",
        new lafi::label("To:"), 15, 16
    );
    frm_tools->easy_add(
        "txt_rename_sprite",
        new lafi::textbox(), 65, 16
    );
    frm_tools->easy_add(
        "but_rename_sprite_ok",
        new lafi::button("Ok"), 20, 24
    );
    frm_tools->easy_row();
    
    
    //Tools -- properties.
    frm_tools->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
        update_stats();
    };
    frm_tools->widgets["but_back"]->description =
        "Go back to the main menu.";
        
    frm_tools->widgets["txt_resize"]->description =
        "Resize multiplier. (0.5=half, 2=double, etc.)";
        
    frm_tools->widgets["but_resize"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        resize_everything();
    };
    frm_tools->widgets["but_resize"]->description =
        "Resize all in-game X/Y and W/H by the given amount.";
        
    frm_tools->widgets["txt_set_scales"]->description =
        "New scale.";
        
    frm_tools->widgets["but_set_scales"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        set_all_sprite_scales();
    };
    frm_tools->widgets["but_set_scales"]->description =
        "Sets the X and Y scale of all sprites to the given value.";
        
    frm_tools->widgets["but_rename_anim_name"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_ANIMATION, false);
    };
    frm_tools->widgets["but_rename_anim_name"]->description =
        "Pick an animation to rename.";
        
    frm_tools->widgets["txt_rename_anim"]->description =
        "Insert the animation's new name here.";
        
    frm_tools->widgets["but_rename_anim_ok"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        rename_animation();
    };
    frm_tools->widgets["but_rename_anim_ok"]->description =
        "Do the rename, if the new name is valid.";
        
    frm_tools->widgets["but_rename_sprite_name"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, false);
    };
    frm_tools->widgets["but_rename_sprite_name"]->description =
        "Pick a sprite to rename.";
        
    frm_tools->widgets["txt_rename_sprite"]->description =
        "Insert the sprite's new name here.";
        
    frm_tools->widgets["but_rename_sprite_ok"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        rename_sprite();
    };
    frm_tools->widgets["but_rename_sprite_ok"]->description =
        "Do the rename, if the new name is valid.";
        
        
    //Options -- declarations.
    frm_options =
        new lafi::frame(canvas_br.x, 0, scr_w, scr_h);
    gui->add("frm_options", frm_options);
    
    frm_options->easy_row();
    frm_options->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_options->easy_row();
    frm_options->easy_add(
        "chk_mmb_pan",
        new lafi::checkbox("Use MMB to pan"), 100, 16
    );
    frm_options->easy_row();
    
    
    //Options -- properties.
    auto lambda_gui_to_options =
    [this] (lafi::widget*) {
        gui_to_options();
    };
    auto lambda_gui_to_options_click =
    [this] (lafi::widget*, int, int) {
        gui_to_options();
    };
    
    frm_options->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        state = EDITOR_STATE_MAIN;
        change_to_right_frame();
    };
    frm_options->widgets["but_back"]->description =
        "Close the options.";
        
    frm_options->widgets["chk_mmb_pan"]->left_mouse_click_handler =
        lambda_gui_to_options_click;
    frm_options->widgets["chk_mmb_pan"]->description =
        "Use the middle mouse button to pan the camera "
        "(and RMB to reset camera/zoom).";
        
        
    //Toolbar -- declarations.
    create_toolbar_frame();
    
    frm_toolbar->easy_row(4, 4, 4);
    frm_toolbar->easy_add(
        "but_load",
        new lafi::button("", "", editor_icons[ICON_LOAD]), 32, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_add(
        "but_save",
        new lafi::button("", "", editor_icons[ICON_SAVE]), 32, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_add(
        "dum_1",
        new lafi::dummy(), 12, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_add(
        "but_toggle_origin",
        new lafi::button("", "", editor_icons[ICON_ORIGIN]), 32, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_add(
        "but_toggle_hitboxes",
        new lafi::button("", "", editor_icons[ICON_HITBOXES]), 32, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_add(
        "but_toggle_mob_radius",
        new lafi::button("", "", editor_icons[ICON_MOB_RADIUS]), 32, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_add(
        "but_toggle_pik_sil",
        new lafi::button("", "", editor_icons[ICON_PIKMIN_SILHOUETTE]), 32, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_add(
        "dum_2",
        new lafi::dummy(), 12, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_add(
        "but_help",
        new lafi::button("", "", editor_icons[ICON_HELP]), 32, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_add(
        "but_quit",
        new lafi::button("", "", editor_icons[ICON_QUIT]), 32, 32,
        lafi::EASY_FLAG_WIDTH_PX
    );
    frm_toolbar->easy_row(4, 4, 4);
    
    
    //Toolbar -- properties.
    frm_toolbar->widgets["but_load"]->left_mouse_click_handler =
    [this] (lafi::widget * w, int, int) {
        if(!check_new_unsaved_changes(w)) {
            load_animation_database(false);
        }
    };
    frm_toolbar->widgets["but_load"]->description =
        "Load the object from the text file.";
        
    frm_toolbar->widgets["but_save"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        save_animation_database();
    };
    frm_toolbar->widgets["but_save"]->description =
        "Save the object to the text file.";
        
    frm_toolbar->widgets["but_toggle_origin"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        origin_visible = !origin_visible;
    };
    frm_toolbar->widgets["but_toggle_origin"]->description =
        "Toggle visibility of the center-point (origin).";
        
    frm_toolbar->widgets["but_toggle_hitboxes"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        hitboxes_visible = !hitboxes_visible;
    };
    frm_toolbar->widgets["but_toggle_hitboxes"]->description =
        "Toggle visibility of the hitboxes, if any.";
        
    frm_toolbar->widgets["but_toggle_mob_radius"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mob_radius_visible = !mob_radius_visible;
    };
    frm_toolbar->widgets["but_toggle_mob_radius"]->description =
        "Toggle visibility of the mob's radius, if applicable.";
        
    frm_toolbar->widgets["but_toggle_pik_sil"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        pikmin_silhouette_visible = !pikmin_silhouette_visible;
    };
    frm_toolbar->widgets["but_toggle_pik_sil"]->description =
        "Toggle visibility of a lying Pikmin silhouette.";
        
    frm_toolbar->widgets["but_help"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        string help_str =
            "To create an animation, first you need some image file "
            "to get the animation frames from, featuring the object "
            "you want to edit in the different poses. After that, "
            "you define what sprites exist (what parts of the image match "
            "what poses), and then create animations, populating "
            "their frames with the sprites.\n\n"
            "If you need more help on how to use the animation editor, "
            "check out the tutorial on\n" + ANIMATION_EDITOR_TUTORIAL_URL;
        al_show_native_message_box(
            display, "Help", "Animation editor help",
            help_str.c_str(), NULL, 0
        );
    };
    frm_toolbar->widgets["but_help"]->description =
        "Display some information about the animation editor.";
        
    frm_toolbar->widgets["but_quit"]->left_mouse_click_handler =
    [this] (lafi::widget * w, int, int) {
        if(!check_new_unsaved_changes(w)) {
            leave();
        }
    };
    frm_toolbar->widgets["but_quit"]->description =
        "Quit the animation editor.";
        
    disable_widget(frm_toolbar->widgets["but_load"]);
    disable_widget(frm_toolbar->widgets["but_save"]);
    
    create_picker_frame();
    create_status_bar();
    
    update_stats();
    
    loaded_content_yet = false;
    
    if(!auto_load_anim.empty()) {
        loaded_mob_type = NULL;
        file_path = auto_load_anim;
        load_animation_database(true);
    }
}
