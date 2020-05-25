/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor Dear ImGui logic.
 */

#include "editor.h"

#include "../../game.h"
#include "../../imgui/imgui_impl_allegro5.h"
#include "../../utils/imgui_utils.h"
#include "../../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Processes ImGui for this frame.
 */
void animation_editor::process_gui() {
    //Initial setup.
    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();
    
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.win_w, game.win_h));
    ImGui::Begin(
        "Animation editor", NULL,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse
    );
    
    //The menu bar.
    process_gui_menu_bar();
    
    //The two main columns that split the canvas (+ toolbar + status bar)
    //and control panel.
    ImGui::Columns(2, "colMain");
    
    //Do the toolbar.
    process_gui_toolbar();
    
    //Draw the canvas now.
    ImGui::BeginChild("canvas", ImVec2(0, -18));
    ImGui::EndChild();
    is_mouse_in_gui = !ImGui::IsItemHovered();
    ImVec2 tl = ImGui::GetItemRectMin();
    canvas_tl.x = tl.x;
    canvas_tl.y = tl.y;
    ImVec2 br = ImGui::GetItemRectMax();
    canvas_br.x = br.x;
    canvas_br.y = br.y;
    ImGui::GetWindowDrawList()->AddCallback(draw_canvas_imgui_callback, NULL);
    
    //Status bar.
    process_gui_status_bar();
    
    //Set up the separator for the control panel.
    ImGui::NextColumn();
    
    if(canvas_separator_x == -1) {
        canvas_separator_x = game.win_w * 0.675;
        ImGui::SetColumnWidth(0, canvas_separator_x);
    } else {
        canvas_separator_x = ImGui::GetColumnOffset(1);
    }
    
    //Do the control panel now.
    process_gui_control_panel();
    ImGui::NextColumn();
    
    //Finish the main window.
    ImGui::Columns(1);
    ImGui::End();
    
    //Process the picker dialog, if any.
    picker.process();
    
    //Finishing setup.
    ImGui::EndFrame();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui control panel for this frame.
 */
void animation_editor::process_gui_control_panel() {
    ImGui::BeginChild("panel");
    
    //Basically, just show the correct panel for the current state.
    switch(state) {
    case EDITOR_STATE_MAIN: {
        process_gui_panel_main();
        break;
    } case EDITOR_STATE_ANIMATION: {
        process_gui_panel_animation();
        break;
    } case EDITOR_STATE_SPRITE: {
        //TODO process_gui_panel_sprite();
        break;
    } case EDITOR_STATE_BODY_PART: {
        //TODO process_gui_panel_body_part();
        break;
    } case EDITOR_STATE_HITBOXES: {
        //TODO process_gui_panel_hitboxes();
        break;
    } case EDITOR_STATE_SPRITE_BITMAP: {
        //TODO process_gui_panel_sprite_bitmap();
        break;
    } case EDITOR_STATE_SPRITE_TRANSFORM: {
        //TODO process_gui_panel_sprite_transform();
        break;
    } case EDITOR_STATE_TOP: {
        //TODO process_gui_panel_top();
        break;
    } case EDITOR_STATE_LOAD: {
        //TODO process_gui_panel_load();
        break;
    } case EDITOR_STATE_TOOLS: {
        //TODO process_gui_panel_tools();
        break;
    } case EDITOR_STATE_OPTIONS: {
        //TODO process_gui_panel_options();
        break;
    }
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui menu bar for this frame.
 */
void animation_editor::process_gui_menu_bar() {
    if(ImGui::BeginMenuBar()) {
        //TODO
        ImGui::EndMenuBar();
    }
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui animation control panel for this frame.
 */
void animation_editor::process_gui_panel_animation() {
    ImGui::BeginChild("main");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("ANIMATIONS", 118.0f);
    
    //Change current animation button.
    if(ImGui::Button("Change")) {
        vector<picker_item> anim_names;
        for(size_t a = 0; a < anims.animations.size(); ++a) {
            anim_names.push_back(picker_item(anims.animations[a]->name));
        }
        picker.set(
            anim_names,
            "Pick an animation, or create a new one",
            std::bind(
                &animation_editor::pick_animation, this,
                std::placeholders::_1,
                std::placeholders::_2
            ),
            "",
            true
        );
    }
    set_tooltip(
        "Pick an animation, or create a new one."
    );
    
    //Current animation text.
    ImGui::SameLine();
    ImGui::Text("Animation: %s", cur_anim ? cur_anim->name.c_str() : "(None)");
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Previous animation button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_PREVIOUS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Select the previous animation in the list."
    );
    
    //Next animation button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_NEXT],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Select the next animation in the list."
    );
    
    //Delete animation button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_REMOVE],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Delete the current animation."
    );
    
    //Import animation button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_DUPLICATE],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Import the data from another animation."
    );
    
    if(cur_anim) {
        //Animation data node.
        if(saveable_tree_node("animation", "Animation data")) {
        
            //Loop frame value.
            int loop_frame = cur_anim->loop_frame + 1;
            ImGui::DragInt(
                "Loop frame", &loop_frame, 0.1, 1,
                cur_anim->frames.empty() ? 1 : cur_anim->frames.size()
            );
            set_tooltip(
                "The animation loops back to this frame when it reaches the "
                "last one."
            );
            cur_anim->loop_frame = loop_frame - 1;
            
            //Hit rate slider.
            int hit_rate = cur_anim->hit_rate;
            ImGui::SliderInt("Hit rate", &hit_rate, 0, 100);
            cur_anim->hit_rate = hit_rate;
            set_tooltip(
                "If this attack can knock back Pikmin, this indicates the chance "
                "that it will miss.\n"
                "0 means it will always hit, 50 means it will miss half the "
                "time, etc."
            );
            
            ImGui::TreePop();
        }
        
        //Frame list node.
        if(saveable_tree_node("animation", "Frame list")) {
        
            frame* frame_ptr = NULL;
            if(cur_frame_nr != INVALID && cur_anim) {
                frame_ptr = &(cur_anim->frames[cur_frame_nr]);
            }
            
            //Current frame text.
            ImGui::Text(
                "Current frame: %s / %i",
                frame_ptr ? i2s(cur_frame_nr + 1).c_str() : "--",
                (int) cur_anim->frames.size()
            );
            
            //Play/pause button.
            if(
                ImGui::ImageButton(
                    editor_icons[ICON_PLAY_PAUSE],
                    ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                )
            ) {
                //TODO
            }
            set_tooltip(
                "Play or pause the animation.",
                "Spacebar"
            );
            
            //Previous frame button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    editor_icons[ICON_PREVIOUS],
                    ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                )
            ) {
                //TODO
            }
            set_tooltip(
                "Previous frame."
            );
            
            //Next frame button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    editor_icons[ICON_NEXT],
                    ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                )
            ) {
                //TODO
            }
            set_tooltip(
                "Next frame."
            );
            
            //Add frame button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    editor_icons[ICON_ADD],
                    ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                )
            ) {
                //TODO
            }
            set_tooltip(
                "Add a new frame after the curret one, by copying data from "
                "the current one."
            );
            
            //Delete frame button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    editor_icons[ICON_REMOVE],
                    ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                )
            ) {
                //TODO
            }
            set_tooltip(
                "Delete the current frame."
            );
            
            //Sprite combobox.
            vector<string> sprite_names;
            for(size_t s = 0; s < anims.sprites.size(); ++s) {
                sprite_names.push_back(anims.sprites[s]->name);
            }
            ImGui::Combo(
                "Sprite", &frame_ptr->sprite_name, sprite_names
            );
            set_tooltip(
                "The sprite to use for this frame."
            );
            
            //Duration value.
            ImGui::DragFloat(
                "Duration", &frame_ptr->duration, 0.01, 0.0f, 9999.0f
            );
            set_tooltip(
                "How long this frame lasts for, in seconds."
            );
            
            //Signal checkbox.
            bool use_signal = (frame_ptr->signal != INVALID);
            if(ImGui::Checkbox("Signal", &use_signal)) {
                if(use_signal) {
                    frame_ptr->signal = 0;
                } else {
                    frame_ptr->signal = INVALID;
                }
            }
            
            //Signal value.
            if(use_signal) {
                ImGui::SameLine();
                int f_signal = frame_ptr->signal;
                ImGui::DragInt("##signal", &f_signal, 0.1, 0, 9999);
                frame_ptr->signal = f_signal;
            }
            
            //Spacer dummy widget.
            ImGui::Dummy(ImVec2(0, 16));
            
            //Apply duration to all button.
            if(ImGui::Button("Apply duration to all frames")) {
                //TODO
            }
            
            ImGui::TreePop();
        }
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui main control panel for this frame.
 */
void animation_editor::process_gui_panel_main() {
    ImGui::BeginChild("main");
    
    //TODO current file stuff.
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Animations button.
    if(
        ImGui::ImageButtonAndText(
            editor_icons[ICON_ANIMATIONS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE),
            16.0f,
            "Animations"
        )
    ) {
        change_state(EDITOR_STATE_ANIMATION);
    }
    set_tooltip(
        "Change the way the animations look like."
    );
    
    //Sprites button.
    if(
        ImGui::ImageButtonAndText(
            editor_icons[ICON_SPRITES],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE),
            16.0f,
            "Sprites"
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Change how each individual sprite looks like."
    );
    
    //Body parts button.
    if(
        ImGui::ImageButtonAndText(
            editor_icons[ICON_BODY_PARTS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE),
            16.0f,
            "Body parts"
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Change what body parts exist, and their order."
    );
    
    //Tools button.
    if(
        ImGui::ImageButtonAndText(
            editor_icons[ICON_TOOLS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE),
            16.0f,
            "Tools"
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Special tools to help with specific tasks."
    );
    
    //Options button.
    if(
        ImGui::ImageButtonAndText(
            editor_icons[ICON_OPTIONS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE),
            16.0f,
            "Options"
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Options for the area editor."
    );
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Stats node.
    if(saveable_tree_node("main", "Stats")) {
    
        //Animation amount text.
        ImGui::Text(
            "Animations: %i", (int) anims.animations.size()
        );
        
        //Sprite amount text.
        ImGui::Text(
            "Sprites: %i", (int) anims.sprites.size()
        );
        
        //Body part amount text.
        ImGui::Text(
            "Body parts: %i", (int) anims.body_parts.size()
        );
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}

/* ----------------------------------------------------------------------------
 * Processes the ImGui status bar for this frame.
 */
void animation_editor::process_gui_status_bar() {
    //TODO
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui toolbar for this frame.
 */
void animation_editor::process_gui_toolbar() {
    //TODO
}
