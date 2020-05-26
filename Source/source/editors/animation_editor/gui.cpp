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
#include "../../functions.h"
#include "../../imgui/imgui_impl_allegro5.h"
#include "../../imgui/imgui_stdlib.h"
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
        process_gui_panel_sprite();
        break;
    } case EDITOR_STATE_BODY_PART: {
        process_gui_panel_body_part();
        break;
    } case EDITOR_STATE_HITBOXES: {
        process_gui_panel_sprite_hitboxes();
        break;
    } case EDITOR_STATE_SPRITE_BITMAP: {
        process_gui_panel_sprite_bitmap();
        break;
    } case EDITOR_STATE_SPRITE_TRANSFORM: {
        process_gui_panel_sprite_transform();
        break;
    } case EDITOR_STATE_TOP: {
        process_gui_panel_sprite_top();
        break;
    } case EDITOR_STATE_LOAD: {
        process_gui_panel_load();
        break;
    } case EDITOR_STATE_TOOLS: {
        process_gui_panel_tools();
        break;
    } case EDITOR_STATE_OPTIONS: {
        process_gui_panel_options();
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
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load/create file item.
            if(ImGui::MenuItem("Load or create file...")) {
                //TODO
            }
            
            //Quit editor item.
            if(ImGui::MenuItem("Quit")) {
                press_quit_button();
            }
            
            ImGui::EndMenu();
            
        }
        
        //Help menu.
        if(ImGui::BeginMenu("Help")) {
        
            //Show tooltips item.
            if(
                ImGui::MenuItem(
                    "Show tooltips", "", &game.options.editor_show_tooltips
                )
            ) {
                save_options();
            }
            
            //General help item.
            if(ImGui::MenuItem("Help...")) {
                string help_str =
                    "To create an animation, first you need some image file "
                    "to get the animation frames from, featuring the object "
                    "you want to edit in the different poses. After that, "
                    "you define what sprites exist (what parts of the image "
                    "match what poses), and then create animations, populating "
                    "their frames with the sprites.\n\n"
                    "If you need more help on how to use the animation editor, "
                    "check out the tutorial on\n" +
                    ANIMATION_EDITOR_TUTORIAL_URL;
                show_message_box(
                    game.display, "Help", "Animation editor help",
                    help_str.c_str(), NULL, 0
                );
            }
            
            ImGui::EndMenu();
            
        }
        
        ImGui::EndMenuBar();
        
    }
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui animation control panel for this frame.
 */
void animation_editor::process_gui_panel_animation() {
    ImGui::BeginChild("animation");
    
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
 * Processes the ImGui body part control panel for this frame.
 */
void animation_editor::process_gui_panel_body_part() {
    ImGui::BeginChild("bodyPart");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("BODY PARTS", 108.0f);
    
    //Explanation text.
    ImGui::TextWrapped(
        "The higher on the list, the more priority that body part's hitboxes "
        "have when the game checks collisions. Drag and drop items in the list "
        "to sort them."
    );
    
    //New body part name.
    static string new_part_name;
    static int selected_part = 0;;
    ImGui::InputText("New part name", &new_part_name);
    
    //Add body part button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_ADD],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Create a new body part, using the name in the text box above.\n"
        "It will be placed after the currently selected body part."
    );
    
    //Delete body part button.
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
        "Delete the currently selected body part from the list."
    );
    
    //Body part list.
    if(ImGui::BeginChild("partsList", ImVec2(0.0f, 80.0f), true)) {
    
        for(size_t p = 0; p < anims.body_parts.size(); ++p) {
        
            //Body part selectable.
            bool is_selected = (p == selected_part);
            ImGui::Selectable(anims.body_parts[p]->name.c_str(), &is_selected);
            
            if(ImGui::IsItemActive()) {
                selected_part = p;
                if(!ImGui::IsItemHovered()) {
                    int p2 =
                        p + (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
                    if(p2 >= 0 && p2 < anims.body_parts.size()) {
                        body_part* p_ptr = anims.body_parts[p];
                        anims.body_parts[p] = anims.body_parts[p2];
                        anims.body_parts[p2] = p_ptr;
                        ImGui::ResetMouseDragDelta();
                    }
                }
            }
            
        }
        
        ImGui::EndChild();
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui load part control panel for this frame.
 */
void animation_editor::process_gui_panel_load() {
    //TODO
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
        change_state(EDITOR_STATE_SPRITE);
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
        change_state(EDITOR_STATE_BODY_PART);
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
        change_state(EDITOR_STATE_TOOLS);
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
        change_state(EDITOR_STATE_OPTIONS);
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
 * Processes the ImGui options control panel for this frame.
 */
void animation_editor::process_gui_panel_options() {
    ImGui::BeginChild("options");
    
    //Back button.
    if(ImGui::Button("Save and go back")) {
        save_options();
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("OPTIONS", 88.0f);
    
    //Controls node.
    if(saveable_tree_node("options", "Controls")) {
    
        //Middle mouse button pans checkbox.
        ImGui::Checkbox("Use MMB to pan", &game.options.editor_mmb_pan);
        set_tooltip(
            "Use the middle mouse button to pan the camera "
            "(and RMB to reset camera/zoom)."
        );
        
        //Drag threshold value.
        int drag_threshold = (int) game.options.editor_mouse_drag_threshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Drag threshold", &drag_threshold,
            0.1f, 0, 9999
        );
        set_tooltip(
            "Cursor must move these many pixels to be considered a drag."
        );
        game.options.editor_mouse_drag_threshold = drag_threshold;
        
        ImGui::TreePop();
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui sprite control panel for this frame.
 */
void animation_editor::process_gui_panel_sprite() {
    ImGui::BeginChild("sprite");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("SPRITES", 88.0f);
    
    //Change current sprite button.
    if(ImGui::Button("Change")) {
        vector<picker_item> sprite_names;
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            sprite_names.push_back(picker_item(anims.sprites[s]->name));
        }
        picker.set(
            sprite_names,
            "Pick a sprite, or create a new one",
            std::bind(
                &animation_editor::pick_sprite, this,
                std::placeholders::_1,
                std::placeholders::_2
            ),
            "",
            true
        );
    }
    set_tooltip(
        "Pick a sprite, or create a new one."
    );
    
    //Current sprite text.
    ImGui::SameLine();
    ImGui::Text("Sprite: %s", cur_sprite ? cur_sprite->name.c_str() : "(None)");
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Previous sprite button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_PREVIOUS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Select the previous sprite in the list."
    );
    
    //Next sprite button.
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
        "Select the next sprite in the list."
    );
    
    //Delete sprite button.
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
        "Delete the current sprite."
    );
    
    //Import sprite button.
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
        "Import the data from another sprite."
    );
    
    if(cur_sprite) {
    
        //Sprite bitmap button.
        if(ImGui::Button("Bitmap")) {
            change_state(EDITOR_STATE_SPRITE_BITMAP);
        }
        set_tooltip(
            "Pick what part of an image makes up this sprite."
        );
        
        //Sprite transformation button.
        if(ImGui::Button("Transformation")) {
            change_state(EDITOR_STATE_SPRITE_TRANSFORM);
        }
        set_tooltip(
            "Offset, scale, or rotate the sprite's image."
        );
        
        //Sprite hitboxes button.
        if(ImGui::Button("Hitboxes")) {
            change_state(EDITOR_STATE_HITBOXES);
        }
        set_tooltip(
            "Edit this sprite's hitboxes."
        );
        
        //Sprite Pikmin top button.
        if(ImGui::Button("Pikmin top")) {
            change_state(EDITOR_STATE_TOP);
        }
        set_tooltip(
            "Edit the Pikmin's top (maturity) for this sprite."
        );
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui sprite bitmap control panel for this frame.
 */
void animation_editor::process_gui_panel_sprite_bitmap() {
    ImGui::BeginChild("spriteBitmap");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panel_title("BITMAP", 78.0f);
    
    //Import bitmap data button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_DUPLICATE],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Import the bitmap data from another sprite."
    );
    
    //Browse for spritesheet button.
    if(ImGui::Button("...")) {
        //TODO
    }
    set_tooltip("Browse for a spritesheet file to use.");
    
    //Spritesheet file name input.
    ImGui::SameLine();
    ImGui::InputText("File", &cur_sprite->file);
    set_tooltip(
        "File name of the bitmap to use as a spritesheet, in the "
        "Graphics folder. Extension included. e.g. "
        "\"Large_Fly.png\""
    );
    
    //Sprite top-left coordinates value.
    if(
        ImGui::DragFloat2("Top-left", (float*) &cur_sprite->file_pos)
    ) {
        //TODO
    }
    
    //Sprite size value.
    if(
        ImGui::DragFloat2("Size", (float*) &cur_sprite->file_size)
    ) {
        //TODO
    }
    
    //Canvas explanation text.
    ImGui::TextWrapped(
        "Click parts of the image on the left to %s the selection limits.",
        sprite_bmp_add_mode ? "expand" : "set"
    );
    
    //Add to selection checkbox.
    if(
        ImGui::Checkbox("Add to selection", &sprite_bmp_add_mode)
    ) {
        //TODO
    }
    set_tooltip(
        "Add to the existing selection instead of replacing it."
    );
    
    //Clear selection button.
    if(
        ImGui::Button("Clear selection")
    ) {
        //TODO
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui sprite hitboxes control panel for this frame.
 */
void animation_editor::process_gui_panel_sprite_hitboxes() {
    ImGui::BeginChild("spriteHitboxes");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panel_title("HITBOXES", 96.0f);
    
    //Previous hitbox button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_PREVIOUS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Select the previous hitbox."
    );
    
    //Next hitbox button.
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
        "Select the next hitbox."
    );
    
    //Import hitbox data button.
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
        "Import the hitbox data from another sprite."
    );
    
    //Side view checkbox.
    ImGui::Checkbox("Use side view", &side_view);
    set_tooltip(
        "Use a side view of the object, so you can adjust hitboxes "
        "horizontally."
    );
    
    //Hitbox name text.
    ImGui::Text(
        "Hitbox: %s",
        cur_hitbox ? cur_hitbox->body_part_name.c_str() : "(None)"
    );
    
    //Hitbox center value.
    if(
        ImGui::DragFloat2("Center", (float*) &cur_hitbox->pos, 0.1f)
    ) {
        //TODO
    }
    
    //Hitbox radius value.
    if(
        ImGui::DragFloat("Radius", &cur_hitbox->radius, 0.01f, 0.001f, 9999.0f)
    ) {
        //TODO
    }
    
    //Hitbox Z value.
    if(
        ImGui::DragFloat("Z", &cur_hitbox->z, 0.1f)
    ) {
        //TODO
    }
    set_tooltip(
        "Altitude of the hitbox's bottom."
    );
    
    //Hitbox height value.
    if(
        ImGui::DragFloat("Height", &cur_hitbox->height, 0.1f, 0.0f, 9999.0f)
    ) {
        //TODO
    }
    set_tooltip(
        "Hitbox's height. 0 = spans infinitely vertically."
    );
    
    //Hitbox type text.
    ImGui::Text("Hitbox type:");
    
    //Normal hitbox radio button.
    int type_int = cur_hitbox->type;
    ImGui::RadioButton("Normal", &type_int, HITBOX_TYPE_NORMAL);
    set_tooltip(
        "Normal hitbox, one that can be damaged."
    );
    
    //Attack hitbox radio button.
    ImGui::RadioButton("Attack", &type_int, HITBOX_TYPE_ATTACK);
    set_tooltip(
        "Attack hitbox, one that damages opponents."
    );
    
    //Disabled hitbox radio button.
    ImGui::RadioButton("Disabled", &type_int, HITBOX_TYPE_DISABLED);
    set_tooltip(
        "Disabled hitbox, one that cannot be interacted with."
    );
    cur_hitbox->type = type_int;
    
    switch(cur_hitbox->type) {
    case HITBOX_TYPE_NORMAL: {

        //Defense multiplier value.
        ImGui::SetNextItemWidth(64.0f);
        if(ImGui::DragFloat("Defense multiplier", &cur_hitbox->value, 0.01)) {
            //TODO
        }
        set_tooltip(
            "Opponent attacks will have their damage divided by this amount.\n"
            "0 = invulnerable."
        );
        
        //Pikmin latch checkbox.
        if(ImGui::Checkbox("Pikmin can latch", &cur_hitbox->can_pikmin_latch)) {
            //TODO
        }
        set_tooltip(
            "Can the Pikmin latch on to this hitbox?"
        );
        
        //Hazards input.
        //TODO replace with a list like in the area editor.
        if(ImGui::InputText("Hazards", &cur_hitbox->hazards_str)) {
            //TODO
        }
        set_tooltip(
            "List of hazards, separated by semicolon."
        );
        
        break;
    } case HITBOX_TYPE_ATTACK: {

        //Power value.
        if(ImGui::DragFloat("Power", &cur_hitbox->value, 0.01)) {
            //TODO
        }
        set_tooltip(
            "Attack power, in hit points."
        );
        
        //Hazards input.
        //TODO replace with a list like in the area editor.
        if(ImGui::InputText("Hazards", &cur_hitbox->hazards_str)) {
            //TODO
        }
        set_tooltip(
            "List of hazards, separated by semicolon."
        );
        
        //Outward knockback checkbox.
        if(
            ImGui::Checkbox("Outward knockback", &cur_hitbox->knockback_outward)
        ) {
            //TODO
        }
        set_tooltip(
            "If true, opponents are knocked away from the hitbox's center."
        );
        
        //Knockback angle value.
        if(!cur_hitbox->knockback_outward) {
            if(
                ImGui::SliderAngle(
                    "Knockback angle", &cur_hitbox->knockback_angle,
                    0.0f, 360.0f
                )
            ) {
                //TODO
            }
        }
        
        //Knockback strength value.
        if(ImGui::DragFloat("Knockback value", &cur_hitbox->knockback, 0.01)) {
            //TODO
        }
        set_tooltip(
            "How strong the knockback is. 3 is a good value."
        );
        
        //Wither chance value.
        int wither_chance_int = cur_hitbox->wither_chance;
        if(
            ImGui::SliderInt("Wither chance", &wither_chance_int, 0, 100)
        ) {
            //TODO
        }
        set_tooltip(
            "Chance of the attack lowering a Pikmin's maturity by one."
        );
        
        break;
    }
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui sprite top control panel for this frame.
 */
void animation_editor::process_gui_panel_sprite_top() {
    ImGui::BeginChild("spriteTop");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panel_title("TOP", 60.0f);
    
    //Import top data button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_DUPLICATE],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Import the top data from another sprite."
    );
    
    //Visible checkbox.
    if(ImGui::Checkbox("Visible", &cur_sprite->top_visible)) {
        //TODO
    }
    set_tooltip(
        "Is the top visible in this sprite?"
    );
    
    //Top center value.
    if(
        ImGui::DragFloat2("Center", (float*) &cur_sprite->top_pos, 0.01f)
    ) {
        //TODO
    }
    
    //Top size value.
    if(
        ImGui::DragFloat2("Size", (float*) &cur_sprite->top_size, 0.01f)
    ) {
        //TODO
    }
    
    //Keep aspect ratio checkbox.
    ImGui::Indent();
    ImGui::Checkbox("Keep aspect ratio", &top_tc.keep_aspect_ratio);
    ImGui::Unindent();
    set_tooltip("Keep the aspect ratio when resizing the top.");
    
    //Top angle value.
    if(ImGui::SliderAngle("Angle", &cur_sprite->top_angle, 0.0f, 360.0f)) {
        //TODO
    }
    
    //Toggle maturity button.
    if(ImGui::Button("Toggle maturity")) {
        //TODO
    }
    set_tooltip(
        "View a different maturity top."
    );
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui sprite transform control panel for this frame.
 */
void animation_editor::process_gui_panel_sprite_transform() {
    ImGui::BeginChild("spriteTransform");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panel_title("TRANSFORM", 102.0f);
    
    //Import transformation data button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_DUPLICATE],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO
    }
    set_tooltip(
        "Import the transformation data from another sprite."
    );
    
    //Sprite offset value.
    if(
        ImGui::DragFloat2("Offset", (float*) &cur_sprite->offset, 0.1)
    ) {
        //TODO
    }
    
    //Sprite scale value.
    if(
        ImGui::DragFloat2("Scale", (float*) &cur_sprite->scale, 0.01)
    ) {
        //TODO
    }
    
    //Sprite flip X button.
    ImGui::Indent();
    if(
        ImGui::Button("Flip X")
    ) {
        cur_sprite->scale.x *= -1.0f;
    }
    
    //Sprite flip Y button.
    ImGui::SameLine();
    if(
        ImGui::Button("Flip Y")
    ) {
        cur_sprite->scale.y *= -1.0f;
    }
    
    //Keep aspect ratio checkbox.
    ImGui::Checkbox("Keep aspect ratio", &cur_sprite_tc.keep_aspect_ratio);
    ImGui::Unindent();
    set_tooltip("Keep the aspect ratio when resizing the sprite.");
    
    //Sprite angle value.
    if(ImGui::SliderAngle("Angle", &cur_sprite->angle, 0.0f, 360.0f)) {
        //TODO
    }
    
    //Comparison sprite node.
    if(saveable_tree_node("transformation", "Comparison sprite")) {
    
        //Use comparison checkbox.
        ImGui::Checkbox("Use comparison", &comparison);
        
        if(comparison) {
        
            //Comparison sprite combobox.
            vector<string> all_sprites;
            for(size_t s = 0; s < anims.sprites.size(); ++s) {
                all_sprites.push_back(anims.sprites[s]->name);
            }
            string comparison_sprite_name;
            if(ImGui::Combo("Sprite", &comparison_sprite_name, all_sprites)) {
                //TODO
            }
            set_tooltip(
                "Choose another sprite to serve as a comparison."
            );
            
            //Comparison blinks checkbox.
            ImGui::Checkbox("Blink comparison", &comparison_blink);
            set_tooltip(
                "Blink the comparison in and out?"
            );
            
            //Comparison above checkbox.
            ImGui::Checkbox("Comparison above", &comparison_above);
            set_tooltip(
                "Should the comparison appear above or below the working "
                "sprite?"
            );
            
            //Tint both checkbox.
            ImGui::Checkbox("Tint both", &comparison_tint);
            set_tooltip(
                "Tint the working sprite blue, and the comparison "
                "sprite orange."
            );
            
        }
        
        ImGui::TreePop();
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui tools control panel for this frame.
 */
void animation_editor::process_gui_panel_tools() {
    ImGui::BeginChild("tools");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("TOOLS", 74.0f);
    
    //Resize everything value.
    static float resize_mult = 1.0f;
    ImGui::SetNextItemWidth(96.0f);
    ImGui::DragFloat("##resizeMult", &resize_mult, 0.01);
    
    //Resize everything button.
    ImGui::SameLine();
    if(ImGui::Button("Resize everything")) {
        //TODO
    }
    set_tooltip(
        "Resize everything by the given multiplier.\n"
        "0.5 resizes everyting to half size, 2.0 to double, etc."
    );
    
    //Set sprite scales value.
    static float scales_value = 1.0f;
    ImGui::SetNextItemWidth(96.0f);
    ImGui::DragFloat("##scalesValue", &scales_value, 0.01);
    
    //Set sprite scales button.
    ImGui::SameLine();
    if(ImGui::Button("Set all scales")) {
        //TODO
    }
    set_tooltip(
        "Set the X/Y scales of all sprites to the given value."
    );
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui status bar for this frame.
 */
void animation_editor::process_gui_status_bar() {
    const float MOUSE_COORDS_TEXT_WIDTH = 150.0f;
    
    //Status bar text.
    ImGui::Text("%s", status_text.c_str());
    
    //Spacer dummy widget.
    ImGui::SameLine();
    float size =
        canvas_separator_x - ImGui::GetItemRectSize().x -
        MOUSE_COORDS_TEXT_WIDTH;
    ImGui::Dummy(ImVec2(size, 0));
    
    //Mouse coordinates text.
    if(!is_mouse_in_gui || is_m1_pressed) {
        ImGui::SameLine();
        ImGui::Text(
            "%s, %s",
            box_string(f2s(game.mouse_cursor_w.x), 7).c_str(),
            box_string(f2s(game.mouse_cursor_w.y), 7).c_str()
        );
    }
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui toolbar for this frame.
 */
void animation_editor::process_gui_toolbar() {
    //Quit button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_QUIT],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        press_quit_button();
    }
    quit_widget_pos = get_last_widget_pos();
    set_tooltip(
        "Quit the animation editor.",
        "Ctrl + Q"
    );
    
    //Reload button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_LOAD],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO press_reload_button();
    }
    reload_widget_pos = get_last_widget_pos();
    set_tooltip(
        "Discard all changes made and load the file again.",
        "Ctrl + L"
    );
    
    //Save button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_SAVE],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO press_save_button();
    }
    set_tooltip(
        "Save the animation data into the files on disk.",
        "Ctrl + S"
    );
    
    //Toggle origin button.
    ImGui::SameLine(0, 16);
    if(
        ImGui::ImageButton(
            editor_icons[ICON_ORIGIN],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO press_origin_button();
    }
    set_tooltip(
        "Toggle visibility of the center-point (origin).",
        "Ctrl + O"
    );
    
    //Toggle hitboxes button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_HITBOXES],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO press_hitboxes_button();
    }
    set_tooltip(
        "Toggle visibility of the hitboxes, if any.",
        "Ctrl + H"
    );
    
    //Toggle mob radius button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_MOB_RADIUS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO press_mob_radius_button();
    }
    set_tooltip(
        "Toggle visibility of the mob's radius, if applicable.",
        "Ctrl + R"
    );
    
    //Toggle Pikmin silhouette button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_PIKMIN_SILHOUETTE],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        //TODO press_pikmin_silhouette_button();
    }
    set_tooltip(
        "Toggle visibility of a lying Pikmin silhouette.",
        "Ctrl + P"
    );
}
