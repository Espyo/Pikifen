/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
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
    fade_mgr.start_fade(true, nullptr);
    
    update_gui_coordinates();
    mode = EDITOR_MODE_MAIN;
    file_path.clear();
    
    load_custom_particle_generators(false);
    load_status_types(false);
    load_liquids(false);
    load_hazards();
    
    lafi::style* s =
        new lafi::style(
        al_map_rgb(192, 192, 208),
        al_map_rgb(32, 32, 64),
        al_map_rgb(96, 128, 160),
        font_builtin
    );
    gui = new lafi::gui(scr_w, scr_h, s);
    
    
    //Main -- declarations.
    lafi::frame* frm_main =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    gui->add("frm_main", frm_main);
    
    frm_main->easy_row();
    frm_main->easy_add(
        "lbl_file",
        new lafi::label("Choose a file:"), 100, 16
    );
    frm_main->easy_row();
    frm_main->easy_add(
        "but_file",
        new lafi::button(), 100, 32
    );
    int y = frm_main->easy_row();
    
    lafi::frame* frm_object =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_main->add("frm_object", frm_object);
    frm_object->easy_row();
    frm_object->easy_add(
        "but_anims",
        new lafi::button("Edit animations"), 100, 32
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_sprites",
        new lafi::button("Edit sprites"), 100, 32
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_body_parts",
        new lafi::button("Edit body parts"), 100, 32
    );
    frm_object->easy_row();
    frm_object->easy_add(
        "but_tools",
        new lafi::button("Special tools"), 100, 32
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
    [this, frm_main] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_HISTORY;
        populate_history();
        hide_bottom_frame();
        change_to_right_frame();
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
        change_to_right_frame();
        animation_to_gui();
    };
    frm_object->widgets["but_anims"]->description =
        "Change the way the animations look like.";
        
    frm_object->widgets["but_sprites"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SPRITE;
        cur_hitbox_nr = INVALID;
        change_to_right_frame();
        sprite_to_gui();
    };
    frm_object->widgets["but_sprites"]->description =
        "Change how each individual sprite looks like.";
        
    frm_object->widgets["but_body_parts"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_BODY_PART;
        change_to_right_frame();
        cur_body_part_nr = 0;
        body_part_to_gui();
    };
    frm_object->widgets["but_body_parts"]->description =
        "Change what body parts exist, and their order.";
        
    frm_object->widgets["but_tools"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_TOOLS;
        change_to_right_frame();
    };
    frm_object->widgets["but_tools"]->description =
        "Special tools to help with specific tasks.";
        
        
    //History -- declarations.
    lafi::frame* frm_history =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    hide_widget(frm_history);
    gui->add("frm_history", frm_history);
    
    frm_history->easy_row();
    frm_history->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "but_browse",
        new lafi::button("Browse"), 100, 24
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "dum_1",
        new lafi::dummy(), 100, 16
    );
    frm_history->easy_row();
    frm_history->easy_add(
        "lbl_hist",
        new lafi::label("History:"), 100, 16
    );
    y = frm_history->easy_row();
    frm_history->add(
        "frm_list",
        new lafi::frame(gui_x, y, scr_w, scr_h - 48)
    );
    
    
    //History -- properties.
    frm_history->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        show_bottom_frame();
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
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
        show_bottom_frame();
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
    };
    frm_history->widgets["but_browse"]->description =
        "Pick a file to load or create.";
        
        
    //Animations -- declarations.
    lafi::frame* frm_anims =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    hide_widget(frm_anims);
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
        new lafi::button("", "", icons.get(DELETE_ICON)), 15, 24
    );
    frm_anims->easy_row();
    frm_anims->easy_add(
        "but_anim",
        new lafi::button(), 100, 32
    );
    y = frm_anims->easy_row();
    
    lafi::frame* frm_anim =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
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
        new lafi::button("", "", icons.get(PLAY_PAUSE_ICON)), 20, 32
    );
    frm_anim->easy_add(
        "but_prev",
        new lafi::button("", "", icons.get(PREVIOUS_ICON)), 20, 32
    );
    frm_anim->easy_add(
        "but_next",
        new lafi::button("", "", icons.get(NEXT_ICON)), 20, 32
    );
    frm_anim->easy_add(
        "but_add",
        new lafi::button("", "", icons.get(NEW_ICON)), 20, 32
    );
    frm_anim->easy_add(
        "but_rem",
        new lafi::button("", "", icons.get(DELETE_ICON)), 20, 32
    );
    y += frm_anim->easy_row();
    
    lafi::frame* frm_frame =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
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
        mode = EDITOR_MODE_MAIN;
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
        cur_hitbox_nr = INVALID;
        animation_to_gui();
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
        lambda_gui_to_animation;
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
        frame_to_gui();
        made_changes = true;
    };
    frm_anim->widgets["but_rem"]->description =
        "Remove the current frame.";
        
    frm_frame->widgets["but_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        anim_playing = false;
        hide_widget(this->gui->widgets["frm_anims"]);
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
    
    
    //Sprites -- declarations.
    lafi::frame* frm_sprites =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    hide_widget(frm_sprites);
    gui->add("frm_sprites", frm_sprites);
    
    frm_sprites->easy_row();
    frm_sprites->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_sprites->easy_row();
    frm_sprites->easy_add(
        "lbl_sprite",
        new lafi::label("Sprite:"), 85, 16
    );
    frm_sprites->easy_add(
        "but_del_sprite",
        new lafi::button("", "", icons.get(DELETE_ICON)), 15, 32
    );
    frm_sprites->easy_row();
    frm_sprites->easy_add(
        "but_sprite",
        new lafi::button(), 100, 32
    );
    y = frm_sprites->easy_row();
    
    lafi::frame* frm_sprite =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_sprites->add("frm_sprite", frm_sprite);
    
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
        "lbl_file",
        new lafi::label("File:"), 25, 16
    );
    frm_sprite->easy_add(
        "txt_file",
        new lafi::textbox(), 75, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "lbl_filexy",
        new lafi::label("File XY:"), 45, 16
    );
    frm_sprite->easy_add(
        "txt_filex",
        new lafi::textbox(), 27.5, 16
    );
    frm_sprite->easy_add(
        "txt_filey",
        new lafi::textbox(), 27.5, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "lbl_filewh",
        new lafi::label("File WH:"), 45, 16
    );
    frm_sprite->easy_add(
        "txt_filew",
        new lafi::textbox(), 27.5, 16
    );
    frm_sprite->easy_add(
        "txt_fileh",
        new lafi::textbox(), 27.5, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_offsxy",
        new lafi::button("Offset:"), 45, 16
    );
    frm_sprite->easy_add(
        "txt_offsx",
        new lafi::textbox(), 27.5, 16
    );
    frm_sprite->easy_add(
        "txt_offsy",
        new lafi::textbox(), 27.5, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_gamewh",
        new lafi::button("Game WH:"), 45, 16
    );
    frm_sprite->easy_add(
        "txt_gamew",
        new lafi::textbox(), 27.5, 16
    );
    frm_sprite->easy_add(
        "txt_gameh",
        new lafi::textbox(), 27.5, 16
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_hitboxes",
        new lafi::button("Edit hitboxes"), 100, 32
    );
    frm_sprite->easy_row();
    frm_sprite->easy_add(
        "but_top",
        new lafi::button("Edit Pikmin top"), 100, 32
    );
    frm_sprite->easy_row();
    
    
    //Sprites -- properties.
    auto lambda_gui_to_sprite = [this] (lafi::widget*) { gui_to_sprite(); };
    auto lambda_sprite_transform = [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SPRITE_TRANSFORM;
        change_to_right_frame();
        comparison_sprite = NULL;
        sprite_transform_to_gui();
    };
    
    frm_sprites->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
        change_to_right_frame();
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
        sprite_to_gui();
        made_changes = true;
    };
    frm_sprites->widgets["but_del_sprite"]->description =
        "Delete the current sprite.";
        
    frm_sprites->widgets["but_sprite"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        hide_widget(this->gui->widgets["frm_sprites"]);
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, true);
    };
    frm_sprites->widgets["but_sprite"]->description =
        "Pick a sprite to edit.";
        
    frm_sprite->widgets["txt_file"]->lose_focus_handler =
        lambda_gui_to_sprite;
    frm_sprite->widgets["txt_file"]->description =
        "Name (+extension) of the file with the sprite.";
        
    frm_sprite->widgets["txt_filex"]->lose_focus_handler =
        lambda_gui_to_sprite;
    frm_sprite->widgets["txt_filex"]->description =
        "X of the top-left corner of the sprite.";
        
    frm_sprite->widgets["txt_filey"]->lose_focus_handler =
        lambda_gui_to_sprite;
    frm_sprite->widgets["txt_filey"]->description =
        "Y of the top-left corner of the sprite.";
        
    frm_sprite->widgets["txt_filew"]->lose_focus_handler =
        lambda_gui_to_sprite;
    frm_sprite->widgets["txt_filew"]->description =
        "Width of the sprite, in the file.";
        
    frm_sprite->widgets["txt_fileh"]->lose_focus_handler =
        lambda_gui_to_sprite;
    frm_sprite->widgets["txt_fileh"]->description =
        "Height of the sprite, in the file.";
        
    frm_sprite->widgets["but_offsxy"]->left_mouse_click_handler =
        lambda_sprite_transform;
    frm_sprite->widgets["but_offsxy"]->description =
        "Click this button for an offset helper tool.";
        
    frm_sprite->widgets["txt_offsx"]->lose_focus_handler =
        lambda_gui_to_sprite;
    frm_sprite->widgets["txt_offsx"]->description =
        "In-game, offset by this much, horizontally.";
        
    frm_sprite->widgets["txt_offsy"]->lose_focus_handler =
        lambda_gui_to_sprite;
    frm_sprite->widgets["txt_offsy"]->description =
        "In-game, offset by this much, vertically.";
        
    frm_sprite->widgets["but_gamewh"]->left_mouse_click_handler =
        lambda_sprite_transform;
    frm_sprite->widgets["but_gamewh"]->description =
        "Click this button for a resize helper tool.";
        
    frm_sprite->widgets["txt_gamew"]->lose_focus_handler =
        lambda_gui_to_sprite;
    frm_sprite->widgets["txt_gamew"]->description =
        "In-game sprite width.";
        
    frm_sprite->widgets["txt_gameh"]->lose_focus_handler =
        lambda_gui_to_sprite;
    frm_sprite->widgets["txt_gameh"]->description =
        "In-game sprite height.";
        
    frm_sprite->widgets["but_hitboxes"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_HITBOXES;
        cur_hitbox_nr = 0;
        hitbox_to_gui();
        change_to_right_frame();
    };
    frm_sprite->widgets["but_hitboxes"]->description =
        "Edit this frame's hitboxes.";
        
    frm_sprite->widgets["but_top"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_TOP;
        change_to_right_frame();
        top_to_gui();
    };
    frm_sprite->widgets["but_top"]->description =
        "Edit the Pikmin's top (maturity) for this sprite.";
        
        
    //Sprite transform -- declarations.
    lafi::frame* frm_sprite_tra =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    hide_widget(frm_sprite_tra);
    gui->add("frm_sprite_tra", frm_sprite_tra);
    
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "lbl_xy",
        new lafi::label("X, Y:"), 25, 16
    );
    frm_sprite_tra->easy_add(
        "txt_x",
        new lafi::textbox(""), 37.5, 16
    );
    frm_sprite_tra->easy_add(
        "txt_y",
        new lafi::textbox(""), 37.5, 16
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "dum_1",
        new lafi::dummy(), 20, 12
    );
    frm_sprite_tra->easy_add(
        "chk_mousexy",
        new lafi::checkbox("Move with LMB", true), 80, 12
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "lbl_wh",
        new lafi::label("W, H:"), 25, 16
    );
    frm_sprite_tra->easy_add(
        "txt_w",
        new lafi::textbox(""), 37.5, 16
    );
    frm_sprite_tra->easy_add(
        "txt_h",
        new lafi::textbox(""), 37.5, 16
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "dum_2",
        new lafi::dummy(), 20, 12
    );
    frm_sprite_tra->easy_add(
        "chk_mousewh",
        new lafi::checkbox("Resize with LMB"), 80, 12
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "lin_1",
        new lafi::line(), 100, 8
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "chk_compare",
        new lafi::checkbox("Comparison sprite"), 100, 16
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "dum_3",
        new lafi::dummy(), 10, 24
    );
    frm_sprite_tra->easy_add(
        "but_compare",
        new lafi::button(""), 90, 24
    );
    frm_sprite_tra->easy_row();
    frm_sprite_tra->easy_add(
        "dum_4",
        new lafi::dummy(), 10, 16
    );
    frm_sprite_tra->easy_add(
        "chk_compare_blink",
        new lafi::checkbox("Blink comparison?"), 90, 16
    );
    frm_sprite_tra->easy_row();
    
    
    //Sprite transform -- properties.
    auto lambda_save_sprite_tra =
    [this] (lafi::widget*) { gui_to_sprite_transform(); };
    auto lambda_save_sprite_tra_click =
    [this] (lafi::widget*, int, int) { gui_to_sprite_transform(); };
    
    frm_sprite_tra->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        this->comparison_sprite = NULL;
        mode = EDITOR_MODE_SPRITE;
        change_to_right_frame();
        sprite_to_gui();
    };
    frm_sprite_tra->widgets["but_back"]->description =
        "Go back to the sprite editor.";
        
    frm_sprite_tra->widgets["txt_x"]->lose_focus_handler =
        lambda_save_sprite_tra;
    frm_sprite_tra->widgets["txt_x"]->description =
        "In-game, offset by this much, horizontally.";
        
    frm_sprite_tra->widgets["txt_y"]->lose_focus_handler =
        lambda_save_sprite_tra;
    frm_sprite_tra->widgets["txt_y"]->description =
        "In-game, offset by this much, vertically.";
        
    frm_sprite_tra->widgets["chk_mousexy"]->description =
        "Allows moving with the left mouse button.";
    frm_sprite_tra->widgets["chk_mousexy"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        (
            (lafi::checkbox*)
            this->gui->widgets["frm_sprite_tra"]->widgets["chk_mousewh"]
        )->uncheck();
        gui_to_sprite_transform();
    };
    
    frm_sprite_tra->widgets["txt_w"]->lose_focus_handler =
        lambda_save_sprite_tra;
    frm_sprite_tra->widgets["txt_w"]->description =
        "In-game sprite width.";
        
    frm_sprite_tra->widgets["txt_h"]->lose_focus_handler =
        lambda_save_sprite_tra;
    frm_sprite_tra->widgets["txt_h"]->description =
        "In-game sprite height.";
        
    frm_sprite_tra->widgets["chk_mousewh"]->description =
        "Allows resizing with the left mouse button.";
    frm_sprite_tra->widgets["chk_mousewh"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        (
            (lafi::checkbox*)
            this->gui->widgets["frm_sprite_tra"]->widgets["chk_mousexy"]
        )->uncheck();
        gui_to_sprite_transform();
    };
    
    frm_sprite_tra->widgets["chk_compare"]->left_mouse_click_handler =
        lambda_save_sprite_tra_click;
    frm_sprite_tra->widgets["chk_compare"]->description =
        "Overlay a different sprite for comparison purposes.";
        
    frm_sprite_tra->widgets["but_compare"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        hide_widget(this->gui->widgets["frm_sprite_tra"]);
        open_picker(ANIMATION_EDITOR_PICKER_SPRITE, false);
    };
    frm_sprite_tra->widgets["but_compare"]->description =
        "Sprite to compare with.";
        
    frm_sprite_tra->widgets["chk_compare_blink"]->left_mouse_click_handler =
        lambda_save_sprite_tra_click;
    frm_sprite_tra->widgets["chk_compare_blink"]->description =
        "Blink the comparison in and out?";
        
        
    //Hitboxes -- declarations.
    lafi::frame* frm_hitboxes =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    hide_widget(frm_hitboxes);
    gui->add("frm_hitboxes", frm_hitboxes);
    
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
    );
    frm_hitboxes->easy_row();
    frm_hitboxes->easy_add(
        "but_prev",
        new lafi::button("", "", icons.get(PREVIOUS_ICON)), 20, 24
    );
    frm_hitboxes->easy_add(
        "but_next",
        new lafi::button("", "", icons.get(NEXT_ICON)), 20, 24
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
    
    lafi::frame* frm_hitbox =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
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
        "lbl_r",
        new lafi::label("Radius:"), 45, 16
    );
    frm_hitbox->easy_add(
        "txt_r",
        new lafi::textbox(), 55, 16
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
    
    lafi::frame* frm_normal =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    hide_widget(frm_normal);
    frm_hitbox->add("frm_normal", frm_normal);
    
    frm_normal->easy_row();
    frm_normal->easy_add(
        "lbl_mult",
        new lafi::label("Defense mult.:"), 60, 16
    );
    frm_normal->easy_add(
        "txt_mult",
        new lafi::textbox(), 40, 16
    );
    frm_normal->easy_row();
    frm_normal->easy_add(
        "chk_latch",
        new lafi::checkbox("Pikmin can latch"), 100, 16
    );
    frm_normal->easy_row();
    frm_normal->easy_add(
        "lbl_hazards",
        new lafi::label("Hazards:"), 100, 12
    );
    frm_normal->easy_row();
    frm_normal->easy_add(
        "txt_hazards",
        new lafi::textbox(), 100, 16
    );
    frm_normal->easy_row();
    
    lafi::frame* frm_attack =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    hide_widget(frm_attack);
    frm_hitbox->add("frm_attack", frm_attack);
    
    frm_attack->easy_row();
    frm_attack->easy_add(
        "lbl_mult",
        new lafi::label("Attack mult.:"), 60, 16
    );
    frm_attack->easy_add(
        "txt_mult",
        new lafi::textbox(), 40, 16
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "lbl_hazards",
        new lafi::label("Hazards:"), 100, 12
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "txt_hazards",
        new lafi::textbox(), 100, 16
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "chk_outward",
        new lafi::checkbox("Outward knockback"), 100, 16
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "lbl_angle",
        new lafi::label("KB angle:"), 60, 16
    );
    frm_attack->easy_add(
        "ang_angle",
        new lafi::angle_picker(), 40, 24
    );
    frm_attack->easy_row();
    frm_attack->easy_add(
        "lbl_knockback",
        new lafi::label("KB strength:"), 60, 16
    );
    frm_attack->easy_add(
        "txt_knockback",
        new lafi::textbox(), 40, 16
    );
    frm_attack->easy_row();
    
    
    //Hitboxes -- properties.
    auto lambda_gui_to_hitbox_instance =
    [this] (lafi::widget*) { gui_to_hitbox(); };
    auto lambda_gui_to_hitbox_instance_click =
    [this] (lafi::widget*, int, int) { gui_to_hitbox(); };
    
    frm_hitboxes->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_SPRITE;
        change_to_right_frame();
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
            } else {
                cur_hitbox_nr =
                    sum_and_wrap(
                        cur_hitbox_nr, -1, cur_sprite->hitboxes.size()
                    );
            }
        }
        hitbox_to_gui();
    };
    frm_hitboxes->widgets["but_prev"]->description =
        "Previous hitbox.";
        
    frm_hitboxes->widgets["but_next"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        if(cur_sprite->hitboxes.size()) {
            if(cur_hitbox_nr == INVALID) {
                cur_hitbox_nr = 0;
            } else {
                cur_hitbox_nr =
                    sum_and_wrap(cur_hitbox_nr, 1, cur_sprite->hitboxes.size());
            }
        }
        hitbox_to_gui();
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
        
    frm_normal->widgets["txt_mult"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_normal->widgets["txt_mult"]->description =
        "Defense multiplier. 0 = invulnerable.";
        
    frm_normal->widgets["chk_latch"]->left_mouse_click_handler =
        lambda_gui_to_hitbox_instance_click;
    frm_normal->widgets["chk_latch"]->description =
        "Can the Pikmin latch on to this hitbox?";
        
    frm_normal->widgets["txt_hazards"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_normal->widgets["txt_hazards"]->description =
        "List of hazards, semicolon separated.";
        
    frm_attack->widgets["txt_mult"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_attack->widgets["txt_mult"]->description =
        "Attack multiplier.";
        
    frm_attack->widgets["txt_hazards"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_attack->widgets["txt_hazards"]->description =
        "List of hazards, semicolon separated.";
        
    frm_attack->widgets["chk_outward"]->left_mouse_click_handler =
        lambda_gui_to_hitbox_instance_click;
    frm_attack->widgets["chk_outward"]->description =
        "Makes Pikmin be knocked away from the center.";
        
    frm_attack->widgets["ang_angle"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_attack->widgets["ang_angle"]->description =
        "Angle the Pikmin are knocked towards.";
        
    frm_attack->widgets["txt_knockback"]->lose_focus_handler =
        lambda_gui_to_hitbox_instance;
    frm_attack->widgets["txt_knockback"]->description =
        "Knockback strength.";
        
        
    //Pikmin top -- declarations.
    lafi::frame* frm_top =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    hide_widget(frm_top);
    gui->add("frm_top", frm_top);
    
    frm_top->easy_row();
    frm_top->easy_add(
        "but_back",
        new lafi::button("Back"), 50, 16
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
        "dum_1",
        new lafi::dummy(), 20, 12
    );
    frm_top->easy_add(
        "chk_mousexy",
        new lafi::checkbox("Move with LMB", true), 100, 12
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
        "dum_2",
        new lafi::dummy(), 20, 12
    );
    frm_top->easy_add(
        "chk_mousewh",
        new lafi::checkbox("Resize with LMB"), 100, 12
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
        "dum_3",
        new lafi::dummy(), 20, 12
    );
    frm_top->easy_add(
        "chk_mousea",
        new lafi::checkbox("Rotate with LMB"), 100, 12
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
        mode = EDITOR_MODE_SPRITE;
        change_to_right_frame();
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
        
    frm_top->widgets["chk_mousexy"]->description =
        "Allows moving with the left mouse button.";
    frm_top->widgets["chk_mousexy"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        (
            (lafi::checkbox*)
            this->gui->widgets["frm_top"]->widgets["chk_mousewh"]
        )->uncheck();
        (
            (lafi::checkbox*)
            this->gui->widgets["frm_top"]->widgets["chk_mousea"]
        )->uncheck();
        gui_to_top();
    };
    
    frm_top->widgets["txt_w"]->lose_focus_handler =
        lambda_save_top;
    frm_top->widgets["txt_w"]->description =
        "In-game width of the top.";
        
    frm_top->widgets["txt_h"]->lose_focus_handler =
        lambda_save_top;
    frm_top->widgets["txt_h"]->description =
        "In-game height of the top.";
        
    frm_top->widgets["chk_mousewh"]->description =
        "Allows resizing with the left mouse button.";
    frm_top->widgets["chk_mousewh"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        (
            (lafi::checkbox*)
            this->gui->widgets["frm_top"]->widgets["chk_mousexy"]
        )->uncheck();
        (
            (lafi::checkbox*)
            this->gui->widgets["frm_top"]->widgets["chk_mousea"]
        )->uncheck();
        gui_to_top();
    };
    
    frm_top->widgets["ang_angle"]->lose_focus_handler =
        lambda_save_top;
    frm_top->widgets["ang_angle"]->description =
        "Angle of the top.";
        
    frm_top->widgets["chk_mousea"]->description =
        "Allows rotating with the left mouse button.";
    frm_top->widgets["chk_mousea"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        (
            (lafi::checkbox*)
            this->gui->widgets["frm_top"]->widgets["chk_mousexy"]
        )->uncheck();
        (
            (lafi::checkbox*)
            this->gui->widgets["frm_top"]->widgets["chk_mousewh"]
        )->uncheck();
        gui_to_top();
    };
    
    frm_top->widgets["but_maturity"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        cur_maturity = sum_and_wrap(cur_maturity, 1, N_MATURITIES);
    };
    frm_top->widgets["but_maturity"]->description =
        "View a different maturity top.";
        
        
    //Body parts -- declarations.
    lafi::frame* frm_body_parts =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    hide_widget(frm_body_parts);
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
        new lafi::textbox(""), 80, 16
    );
    frm_body_parts->easy_add(
        "but_add",
        new lafi::button("", "", icons.get(NEW_ICON)), 20, 24
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "but_prev",
        new lafi::button("", "", icons.get(PREVIOUS_ICON)), 20, 24
    );
    frm_body_parts->easy_add(
        "but_next",
        new lafi::button("", "", icons.get(NEXT_ICON)), 20, 24
    );
    frm_body_parts->easy_add(
        "but_del",
        new lafi::button("", "", icons.get(DELETE_ICON)), 20, 24
    );
    frm_body_parts->easy_row();
    frm_body_parts->easy_add(
        "lbl_n",
        new lafi::label("Part nr:"), 50, 16
    );
    frm_body_parts->easy_add(
        "lbl_nr",
        new lafi::label(""), 50, 16
    );
    y = frm_body_parts->easy_row();
    
    lafi::frame* frm_body_part =
        new lafi::frame(gui_x, y, scr_w, scr_h - 48);
    frm_body_parts->add("frm_body_part", frm_body_part);
    
    frm_body_part->easy_row();
    frm_body_part->easy_add(
        "lbl_na",
        new lafi::label("Name:"), 30, 16
    );
    frm_body_part->easy_add(
        "txt_name",
        new lafi::textbox(""), 70, 16
    );
    frm_body_part->easy_row();
    frm_body_part->easy_add(
        "but_left",
        new lafi::button("", "", icons.get(MOVE_LEFT_ICON)), 20, 24
    );
    frm_body_part->easy_add(
        "but_right",
        new lafi::button("", "", icons.get(MOVE_RIGHT_ICON)), 20, 24
    );
    frm_body_part->easy_row();
    
    
    //Body parts -- properties.
    auto lambda_gui_to_hitbox =
    [this] (lafi::widget*) { gui_to_body_part(); };
    auto lambda_gui_to_hitbox_click =
    [this] (lafi::widget*, int, int) { gui_to_body_part(); };
    
    frm_body_parts->widgets["but_back"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        mode = EDITOR_MODE_MAIN;
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
            (
                (lafi::textbox*)
                this->gui->widgets["frm_body_parts"]->widgets["txt_add"]
            )->text;
        (
            (lafi::textbox*)
            this->gui->widgets["frm_body_parts"]->widgets["txt_add"]
        )->text.clear();
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
        made_changes = true;
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
        made_changes = true;
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
        made_changes = true;
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
        made_changes = true;
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
        made_changes = true;
    };
    frm_body_parts->widgets["but_del"]->description =
        "Delete this body part.";
        
        
    //Tools -- declarations.
    lafi::frame* frm_tools =
        new lafi::frame(gui_x, 0, scr_w, scr_h - 48);
    hide_widget(frm_tools);
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
        "lbl_resolution_1",
        new lafi::label("Set all sprite in-game"), 100, 8
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "lbl_resolution_2",
        new lafi::label("W/H by resolution:"), 100, 8
    );
    frm_tools->easy_row();
    frm_tools->easy_add(
        "txt_resolution",
        new lafi::textbox(), 80, 16
    );
    frm_tools->easy_add(
        "but_resolution",
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
        new lafi::button(""), 100, 24
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
        new lafi::button(""), 100, 24
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
        mode = EDITOR_MODE_MAIN;
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
        
    frm_tools->widgets["txt_resolution"]->description =
        "Resolution. (2=half-size in-game, 0.5=double, etc.)";
        
    frm_tools->widgets["but_resolution"]->left_mouse_click_handler =
    [this] (lafi::widget*, int, int) {
        resize_by_resolution();
    };
    frm_tools->widgets["but_resolution"]->description =
        "Resize all in-game W/H with the given resolution.";
        
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
        
        
    //Bottom bar -- declarations.
    lafi::frame* frm_bottom =
        new lafi::frame(gui_x, scr_h - 48, scr_w, scr_h);
    gui->add("frm_bottom", frm_bottom);
    
    frm_bottom->easy_row();
    frm_bottom->easy_add(
        "but_toggle_hitboxes",
        new lafi::button("", "", icons.get(HITBOXES_ICON)), 25, 32
    );
    frm_bottom->easy_add(
        "but_load",
        new lafi::button("", "", icons.get(LOAD_ICON)), 25, 32
    );
    frm_bottom->easy_add(
        "but_save",
        new lafi::button("", "", icons.get(SAVE_ICON)), 25, 32
    );
    frm_bottom->easy_add(
        "but_quit",
        new lafi::button("", "", icons.get(EXIT_ICON)), 25, 32
    );
    frm_bottom->easy_row();
    
    lafi::label* gui_status_bar =
        new lafi::label(0, status_bar_y, gui_x, scr_h);
    gui->add("lbl_status_bar", gui_status_bar);
    
    
    //Bottom bar -- properties.
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
        
    //File dialog.
    file_dialog =
        al_create_native_file_dialog(
            NULL,
            "Please choose an animation text file to load or create.",
            "*.txt",
            0
        );
        
    create_changes_warning_frame();
    create_picker_frame(true);
    
    disable_widget(frm_bottom->widgets["but_load"]);
    disable_widget(frm_bottom->widgets["but_save"]);
    
    update_stats();
    
    if(!auto_load_anim.empty()) {
        file_path = auto_load_anim;
        load_animation_database();
    }
    
}
