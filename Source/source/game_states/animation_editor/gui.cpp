/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor Dear ImGui logic.
 */

#include <algorithm>

#include "editor.h"

#include "../../functions.h"
#include "../../game.h"
#include "../../imgui/imgui_impl_allegro5.h"
#include "../../imgui/imgui_stdlib.h"
#include "../../utils/imgui_utils.h"
#include "../../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Opens the "load" dialog.
 */
void animation_editor::open_load_dialog() {
    global_anim_files_cache =
        folder_to_vector(ANIMATIONS_FOLDER_PATH, false, NULL);
    for(size_t f = 0; f < global_anim_files_cache.size(); ++f) {
        global_anim_files_cache[f] =
            remove_extension(global_anim_files_cache[f]);
    }
    
    open_dialog(
        "Load a file or create a new one",
        std::bind(&animation_editor::process_gui_load_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&animation_editor::close_load_dialog, this);
    reset_load_dialog = true;
}


/* ----------------------------------------------------------------------------
 * Opens the options dialog.
 */
void animation_editor::open_options_dialog() {
    open_dialog(
        "Options",
        std::bind(&animation_editor::process_gui_options_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&animation_editor::close_options_dialog, this);
}


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
    is_mouse_in_gui =
        !ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
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
    
    //Process the dialogs, if any.
    process_dialogs();
    
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
    } case EDITOR_STATE_TOOLS: {
        process_gui_panel_tools();
        break;
    }
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the list of the current hitbox's hazards,
 * as well as the widgets necessary to control it, for this frame.
 */
void animation_editor::process_gui_hitbox_hazards() {
    //Hitbox hazards node.
    if(saveable_tree_node("hitbox", "Hazards")) {
    
        static int selected_hazard_nr = 0;
        
        //Hitbox hazard addition button.
        if(
            ImGui::ImageButton(
                editor_icons[ICON_ADD],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("addHazard");
        }
        set_tooltip(
            "Add a new hazard to the list of hazards this hitbox has.\n"
            "Click to open a pop-up for you to choose from."
        );
        
        //Hitbox hazard addition popup.
        vector<string> all_hazards_list;
        for(auto h : game.hazards) {
            all_hazards_list.push_back(h.first);
        }
        string picked_hazard;
        if(
            list_popup(
                "addHazard", all_hazards_list, &picked_hazard
            )
        ) {
            vector<string> list =
                semicolon_list_to_vector(cur_hitbox->hazards_str);
            if(
                std::find(list.begin(), list.end(), picked_hazard) ==
                list.end()
            ) {
                if(!cur_hitbox->hazards_str.empty()) {
                    cur_hitbox->hazards_str += ";";
                }
                cur_hitbox->hazards_str += picked_hazard;
                selected_hazard_nr = list.size();
                made_new_changes = true;
                status_text =
                    "Added hazard \"" + picked_hazard + "\" to the hitbox.";
            }
        }
        
        //Hitbox hazard removal button.
        if(selected_hazard_nr >= 0 && !cur_hitbox->hazards_str.empty()) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    editor_icons[ICON_REMOVE],
                    ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                )
            ) {
                vector<string> list =
                    semicolon_list_to_vector(cur_hitbox->hazards_str);
                if(
                    selected_hazard_nr >= 0 &&
                    selected_hazard_nr < (int) list.size()
                ) {
                    string hazard_name = list[selected_hazard_nr];
                    cur_hitbox->hazards_str.clear();
                    for(size_t h = 0; h < list.size(); ++h) {
                        if(h == (size_t) selected_hazard_nr) continue;
                        cur_hitbox->hazards_str += list[h] + ";";
                    }
                    if(!cur_hitbox->hazards_str.empty()) {
                        //Delete the trailing semicolon.
                        cur_hitbox->hazards_str.pop_back();
                    }
                    selected_hazard_nr =
                        std::min(
                            selected_hazard_nr, (int) list.size() - 2
                        );
                    made_new_changes = true;
                    status_text =
                        "Removed hazard \"" + hazard_name +
                        "\" from the hitbox.";
                }
            }
            set_tooltip(
                "Remove the selected hazard from the list of "
                "hazards this hitbox has."
            );
        }
        
        //Hitbox hazard list.
        ImGui::ListBox(
            "Hazards", &selected_hazard_nr,
            semicolon_list_to_vector(cur_hitbox->hazards_str),
            4
        );
        set_tooltip(
            "List of hazards this hitbox has."
        );
        
        ImGui::TreePop();
    }
}


/* ----------------------------------------------------------------------------
 * Processes the "load" dialog for this frame.
 */
void animation_editor::process_gui_load_dialog() {
    //History node.
    if(saveable_tree_node("load", "History")) {
    
        if(!history[0].empty()) {
        
            for(size_t h = 0; h < history.size(); ++h) {
                string name = history[h];
                if(name.empty()) continue;
                
                string button_text = get_path_short_name(name);
                
                //History number text.
                ImGui::Text("%i.", (int) (h + 1));
                
                //History entry button.
                ImGui::SameLine();
                if(ImGui::Button((button_text + "##" + i2s(h)).c_str())) {
                    file_path = name;
                    loaded_mob_type = NULL;
                    load_animation_database(true);
                    close_top_dialog();
                }
            }
            
        } else {
        
            //No history text.
            ImGui::TextDisabled("(Empty)");
            
        }
        
        ImGui::TreePop();
        
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Object animation node.
    if(saveable_tree_node("load", "Object animation")) {
        static mob_category* cat = NULL;
        static mob_type* typ = NULL;
        
        if(reset_load_dialog) {
            cat = game.mob_categories.get(MOB_CATEGORY_PIKMIN);
            typ = game.config.pikmin_order[0];
            reset_load_dialog = false;
        }
        
        //Category and type comboboxes.
        process_mob_type_widgets(&cat, &typ, false);
        
        //Load button.
        if(ImGui::Button("Load", ImVec2(96.0f, 32.0f))) {
            if(typ) {
                loaded_mob_type = typ;
                file_path =
                    loaded_mob_type->category->folder + "/" +
                    loaded_mob_type->folder_name + "/Animations.txt";
                load_animation_database(true);
                close_top_dialog();
            }
        }
        set_tooltip(
            "Load/create the animation file for the chosen mob type."
        );
        
        ImGui::TreePop();
        
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Global animation node.
    if(saveable_tree_node("load", "Global animation")) {
    
        //Animations combobox.
        static string chosen_anim;
        if(!global_anim_files_cache.empty() && chosen_anim.empty()) {
            chosen_anim = global_anim_files_cache[0];
        }
        ImGui::Combo("Animation", &chosen_anim, global_anim_files_cache);
        
        //Load button.
        if(ImGui::Button("Load", ImVec2(96.0f, 32.0f))) {
            if(!chosen_anim.empty()) {
                loaded_mob_type = NULL;
                file_path = ANIMATIONS_FOLDER_PATH + "/" + chosen_anim + ".txt";
                load_animation_database(true);
                close_top_dialog();
            }
        }
        set_tooltip(
            "Load the animation file for the chosen generic global animation."
        );
        
        ImGui::TreePop();
        
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Other node.
    if(saveable_tree_node("load", "Other")) {
    
        //Load button.
        if(ImGui::Button("Browse...", ImVec2(96.0f, 32.0f))) {
            string last_file_opened;
            if(history.size()) {
                last_file_opened = history[0];
            }
            
            vector<string> f =
                prompt_file_dialog(
                    last_file_opened,
                    "Please choose an animation text file to load or create.",
                    "*.txt", 0
                );
                
            if(!f.empty() && !f[0].empty()) {
                file_path = f[0];
                
                loaded_mob_type = NULL;
                load_animation_database(true);
                close_top_dialog();
            }
        }
        set_tooltip(
            "Browse your disk for an animation file to load/create."
        );
        
        ImGui::TreePop();
        
    }
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui menu bar for this frame.
 */
void animation_editor::process_gui_menu_bar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Reload current file item.
            if(ImGui::MenuItem("Reload current file")) {
                press_reload_button();
            }
            reload_widget_pos = get_last_widget_pos();
            
            //Options menu item.
            if(ImGui::MenuItem("Options")) {
                open_options_dialog();
            }
            
            //Quit editor item.
            if(ImGui::MenuItem("Quit", "Ctrl+Q")) {
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
                string state_str =
                    game.options.editor_show_tooltips ? "Enabled" : "Disabled";
                status_text = state_str + " tooltips.";
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
                    "check out the tutorial in the included manual.";
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
 * Processes the options dialog for this frame.
 */
void animation_editor::process_gui_options_dialog() {
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
            0.1f, 0, INT_MAX
        );
        set_tooltip(
            "Cursor must move these many pixels to be considered a drag."
        );
        game.options.editor_mouse_drag_threshold = drag_threshold;
        
        ImGui::TreePop();
        
    }

    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));

    //Style node.
    if (saveable_tree_node("options", "Style")) {
        if (ImGui::ColorEdit3("Primary Color", game.options.editor_primary_color)) {
            update_style(game.options.editor_primary_color, game.options.editor_secondary_color);
        }
        if (ImGui::ColorEdit3("Secondary Color", game.options.editor_secondary_color)) {
            update_style(game.options.editor_primary_color, game.options.editor_secondary_color);
        }

        ImGui::TreePop();
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
    
    //Current animation text.
    size_t cur_anim_nr = INVALID;
    if(cur_anim) {
        cur_anim_nr = anims.find_animation(cur_anim->name);
    }
    ImGui::Text(
        "Current animation: %s / %i",
        (cur_anim_nr == INVALID ? "--" : i2s(cur_anim_nr + 1).c_str()),
        (int) anims.animations.size()
    );
    
    //Previous animation button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_PREVIOUS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        if(!anims.animations.empty()) {
            if(!cur_anim) {
                pick_animation(anims.animations[0]->name, "", false);
            } else {
                size_t new_nr =
                    sum_and_wrap(
                        anims.find_animation(cur_anim->name),
                        -1,
                        anims.animations.size()
                    );
                pick_animation(anims.animations[new_nr]->name, "", false);
            }
        }
    }
    set_tooltip(
        "Previous\nanimation."
    );
    
    //Change current animation button.
    string anim_button_name =
        (cur_anim ? cur_anim->name : NONE_OPTION) + "##anim";
    ImVec2 anim_button_size(
        -(EDITOR_ICON_BMP_SIZE + 16.0f), EDITOR_ICON_BMP_SIZE + 6.0f
    );
    ImGui::SameLine();
    if(ImGui::Button(anim_button_name.c_str(), anim_button_size)) {
        vector<picker_item> anim_names;
        for(size_t a = 0; a < anims.animations.size(); ++a) {
            anim_names.push_back(picker_item(anims.animations[a]->name));
        }
        open_picker(
            "Pick an animation, or create a new one",
            anim_names,
            std::bind(
                &animation_editor::pick_animation, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3
            ),
            "",
            true
        );
    }
    set_tooltip(
        "Pick an animation, or create a new one."
    );
    
    //Next animation button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_NEXT],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        if(!anims.animations.empty()) {
            if(!cur_anim) {
                pick_animation(anims.animations[0]->name, "", false);
            } else {
                size_t new_nr =
                    sum_and_wrap(
                        anims.find_animation(cur_anim->name),
                        1,
                        anims.animations.size()
                    );
                pick_animation(anims.animations[new_nr]->name, "", false);
            }
        }
    }
    set_tooltip(
        "Next\nanimation."
    );
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    if(cur_anim) {
    
        //Delete animation button.
        if(
            ImGui::ImageButton(
                editor_icons[ICON_REMOVE],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            if(cur_anim) {
                string cur_anim_name = cur_anim->name;
                size_t nr = anims.find_animation(cur_anim_name);
                anims.animations.erase(anims.animations.begin() + nr);
                if(anims.animations.empty()) {
                    nr = INVALID;
                    cur_anim = NULL;
                    cur_frame_nr = INVALID;
                } else {
                    nr = std::min(nr, anims.animations.size() - 1);
                    pick_animation(anims.animations[nr]->name, "", false);
                }
                anim_playing = false;
                made_new_changes = true;
                status_text = "Deleted animation \"" + cur_anim_name + "\".";
            } else {
                status_text = "You have to select an animation to delete!";
            }
        }
        set_tooltip(
            "Delete the current animation."
        );
        
    }
    
    if(cur_anim) {
    
        if(anims.animations.size() > 1) {
        
            //Import animation button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    editor_icons[ICON_DUPLICATE],
                    ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                )
            ) {
                ImGui::OpenPopup("importAnim");
            }
            set_tooltip(
                "Import the data from another animation."
            );
            
            //Import animation popup.
            vector<string> import_anim_names;
            for(size_t a = 0; a < anims.animations.size(); ++a) {
                if(anims.animations[a] == cur_anim) continue;
                import_anim_names.push_back(anims.animations[a]->name);
            }
            string picked_anim;
            if(list_popup("importAnim", import_anim_names, &picked_anim)) {
                import_animation_data(picked_anim);
                status_text =
                    "Imported animation data from \"" + picked_anim + "\".";
            }
            
        }
        
        //Rename animation button.
        static string rename_anim_name;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                editor_icons[ICON_INFO],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            rename_anim_name = cur_anim->name;
            ImGui::OpenPopup("renameAnim");
        }
        set_tooltip(
            "Rename the current animation."
        );
        
        //Rename animation popup.
        if(input_popup("renameAnim", "New name:", &rename_anim_name)) {
            rename_animation(cur_anim, rename_anim_name);
        }
        
        //Animation data node.
        if(saveable_tree_node("animation", "Animation data")) {
        
            //Loop frame value.
            int loop_frame = cur_anim->loop_frame + 1;
            if(
                ImGui::DragInt(
                    "Loop frame", &loop_frame, 0.1f, 1,
                    cur_anim->frames.empty() ? 1 : cur_anim->frames.size()
                )
            ) {
                made_new_changes = true;
            }
            set_tooltip(
                "The animation loops back to this frame when it "
                "reaches the last one."
            );
            loop_frame =
                clamp(
                    loop_frame, 1,
                    cur_anim->frames.empty() ? 1 : cur_anim->frames.size()
                );
            cur_anim->loop_frame = loop_frame - 1;
            
            //Hit rate slider.
            int hit_rate = cur_anim->hit_rate;
            if(ImGui::SliderInt("Hit rate", &hit_rate, 0, 100)) {
                made_new_changes = true;
                cur_anim->hit_rate = hit_rate;
            }
            set_tooltip(
                "If this attack can knock back Pikmin, this indicates "
                "the chance that it will hit.\n"
                "0 means it will always miss, 50 means it will hit "
                "half the time, etc."
            );
            
            ImGui::TreePop();
        }
        
        //Spacer dummy widget.
        ImGui::Dummy(ImVec2(0, 16));
        
        //Frame list node.
        if(saveable_tree_node("animation", "Frame list")) {
        
            frame* frame_ptr = NULL;
            if(cur_anim) {
                if(cur_frame_nr == INVALID && !cur_anim->frames.empty()) {
                    cur_frame_nr = 0;
                }
                if(cur_frame_nr != INVALID) {
                    frame_ptr = &(cur_anim->frames[cur_frame_nr]);
                }
            }
            
            //Current frame text.
            ImGui::Text(
                "Current frame: %s / %i",
                frame_ptr ? i2s(cur_frame_nr + 1).c_str() : "--",
                (int) cur_anim->frames.size()
            );
            
            if(frame_ptr) {
                //Play/pause button.
                if(
                    ImGui::ImageButton(
                        editor_icons[ICON_PLAY_PAUSE],
                        ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                    )
                ) {
                    press_play_animation_button();
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
                    anim_playing = false;
                    if(!cur_anim->frames.empty()) {
                        if(cur_frame_nr == INVALID) {
                            cur_frame_nr = 0;
                        } else if(cur_frame_nr == 0) {
                            cur_frame_nr =
                                cur_anim->frames.size() - 1;
                        } else {
                            cur_frame_nr--;
                        }
                    }
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
                }
                set_tooltip(
                    "Next frame."
                );
                
            }
            
            //Add frame button.
            if(frame_ptr) {
                ImGui::SameLine();
            }
            if(
                ImGui::ImageButton(
                    editor_icons[ICON_ADD],
                    ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                )
            ) {
                anim_playing = false;
                if(cur_frame_nr != INVALID) {
                    cur_frame_nr++;
                    cur_anim->frames.insert(
                        cur_anim->frames.begin() + cur_frame_nr,
                        frame(cur_anim->frames[cur_frame_nr - 1])
                    );
                } else {
                    cur_anim->frames.push_back(frame());
                    cur_frame_nr = 0;
                }
                frame_ptr = &(cur_anim->frames[cur_frame_nr]);
                made_new_changes = true;
                status_text = "Added frame #" + i2s(cur_frame_nr + 1) + ".";
            }
            set_tooltip(
                "Add a new frame after the curret one, by copying "
                "data from the current one."
            );
            
            if(frame_ptr) {
            
                //Delete frame button.
                ImGui::SameLine();
                if(
                    ImGui::ImageButton(
                        editor_icons[ICON_REMOVE],
                        ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                    )
                ) {
                    anim_playing = false;
                    size_t deleted_frame_nr = cur_frame_nr;
                    if(cur_frame_nr != INVALID) {
                        cur_anim->frames.erase(
                            cur_anim->frames.begin() + cur_frame_nr
                        );
                        if(cur_anim->frames.empty()) {
                            cur_frame_nr = INVALID;
                            frame_ptr = NULL;
                        } else if(cur_frame_nr >= cur_anim->frames.size()) {
                            cur_frame_nr = cur_anim->frames.size() - 1;
                            frame_ptr = &(cur_anim->frames[cur_frame_nr]);
                        }
                        made_new_changes = true;
                        status_text =
                            "Deleted frame #" + i2s(deleted_frame_nr + 1) + ".";
                    }
                }
                set_tooltip(
                    "Delete the current frame."
                );
                
            }
            
            if(frame_ptr) {
            
                //Sprite combobox.
                vector<string> sprite_names;
                for(size_t s = 0; s < anims.sprites.size(); ++s) {
                    sprite_names.push_back(anims.sprites[s]->name);
                }
                if(
                    ImGui::Combo(
                        "Sprite", &frame_ptr->sprite_name, sprite_names
                    )
                ) {
                    made_new_changes = true;
                }
                set_tooltip(
                    "The sprite to use for this frame."
                );
                
                //Duration value.
                if(
                    ImGui::DragFloat(
                        "Duration", &frame_ptr->duration, 0.005, 0.0f, FLT_MAX
                    )
                ) {
                    made_new_changes = true;
                }
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
                    made_new_changes = true;
                }
                
                //Signal value.
                if(use_signal) {
                    ImGui::SameLine();
                    int f_signal = frame_ptr->signal;
                    if(
                        ImGui::DragInt("##signal", &f_signal, 0.1, 0, INT_MAX)
                    ) {
                        made_new_changes = true;
                        frame_ptr->signal = f_signal;
                    }
                }
                
                //Spacer dummy widget.
                ImGui::Dummy(ImVec2(0, 16));
                
                //Apply duration to all button.
                if(ImGui::Button("Apply duration to all frames")) {
                    float d = cur_anim->frames[cur_frame_nr].duration;
                    for(size_t i = 0; i < cur_anim->frames.size(); ++i) {
                        cur_anim->frames[i].duration = d;
                    }
                    made_new_changes = true;
                    status_text =
                        "Applied the duration " + f2s(d) + " to all frames.";
                }
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
    
    static string new_part_name;
    static int selected_part = 0;
    
    //Add body part button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_ADD],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        new_part_name.clear();
        ImGui::OpenPopup("newPartName");
    }
    set_tooltip(
        "Create a new body part."
        "It will be placed after the currently selected body part."
    );
    
    //Add body part popup.
    if(input_popup("newPartName", "New body part's name:", &new_part_name)) {
        if(!new_part_name.empty()) {
            bool already_exists = false;
            for(size_t b = 0; b < anims.body_parts.size(); ++b) {
                if(anims.body_parts[b]->name == new_part_name) {
                    selected_part = b;
                    already_exists = true;
                }
            }
            if(!already_exists) {
                selected_part = std::max(0, selected_part);
                anims.body_parts.insert(
                    anims.body_parts.begin() + selected_part +
                    (anims.body_parts.empty() ? 0 : 1),
                    new body_part(new_part_name)
                );
                if(anims.body_parts.size() == 1) {
                    selected_part = 0;
                } else {
                    selected_part++;
                }
                update_hitboxes();
                made_new_changes = true;
                status_text = "Created body part \"" + new_part_name + "\".";
                new_part_name.clear();
            } else {
                status_text =
                    "A body part by the name \"" + new_part_name +
                    "\" already exists!";
            }
        }
    }
    
    if(!anims.body_parts.empty()) {
    
        //Delete body part button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                editor_icons[ICON_REMOVE],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            if(selected_part >= 0 && !anims.body_parts.empty()) {
                string deleted_part_name =
                    anims.body_parts[selected_part]->name;
                delete anims.body_parts[selected_part];
                anims.body_parts.erase(
                    anims.body_parts.begin() + selected_part
                );
                if(anims.body_parts.empty()) {
                    selected_part = -1;
                } else if(selected_part > 0) {
                    selected_part--;
                }
                update_hitboxes();
                made_new_changes = true;
                status_text =
                    "Deleted body part \"" + deleted_part_name + "\".";
            }
        }
        set_tooltip(
            "Delete the currently selected body part from the list."
        );
        
        //Rename body part button.
        static string rename_part_name;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                editor_icons[ICON_INFO],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            rename_part_name = anims.body_parts[selected_part]->name;
            ImGui::OpenPopup("renamePart");
        }
        set_tooltip(
            "Rename the current body part."
        );
        
        //Rename body part popup.
        if(input_popup("renamePart", "New name:", &rename_part_name)) {
            rename_body_part(
                anims.body_parts[selected_part], rename_part_name
            );
        }
        
        //Body part list.
        if(ImGui::BeginChild("partsList", ImVec2(0.0f, 80.0f), true)) {
        
            for(size_t p = 0; p < anims.body_parts.size(); ++p) {
            
                //Body part selectable.
                bool is_selected = (p == (size_t) selected_part);
                ImGui::Selectable(
                    anims.body_parts[p]->name.c_str(), &is_selected
                );
                
                if(ImGui::IsItemActive()) {
                    selected_part = p;
                    if(!ImGui::IsItemHovered()) {
                        int p2 =
                            p + (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
                        if(p2 >= 0 && p2 < (int) anims.body_parts.size()) {
                            body_part* p_ptr = anims.body_parts[p];
                            anims.body_parts[p] = anims.body_parts[p2];
                            anims.body_parts[p2] = p_ptr;
                            ImGui::ResetMouseDragDelta();
                            update_hitboxes();
                            made_new_changes = true;
                        }
                    }
                }
                
            }
            
            ImGui::EndChild();
            
        }
        
    }
    
    if(anims.body_parts.size() > 1) {
    
        //Spacer dummy widget.
        ImGui::Dummy(ImVec2(0, 16));
        
        //Explanation text.
        ImGui::TextWrapped(
            "The higher on the list, the more priority that body "
            "part's hitboxes have when the game checks collisions. "
            "Drag and drop items in the list to sort them."
        );
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui main control panel for this frame.
 */
void animation_editor::process_gui_panel_main() {
    ImGui::BeginChild("main");
    
    //Current file text.
    ImGui::Text("Current file: %s", get_path_short_name(file_path).c_str());
    set_tooltip("Full file path: " + file_path);
    
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
        if(!cur_anim && !anims.animations.empty()) {
            cur_anim = anims.animations[0];
            if(cur_anim->frames.size()) cur_frame_nr = 0;
        }
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
        if(!cur_sprite && !anims.sprites.empty()) {
            cur_sprite = anims.sprites[0];
        }
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
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Stats node.
    if(saveable_tree_node("main", "Stats")) {
    
        //Animation amount text.
        ImGui::BulletText(
            "Animations: %i", (int) anims.animations.size()
        );
        
        //Sprite amount text.
        ImGui::BulletText(
            "Sprites: %i", (int) anims.sprites.size()
        );
        
        //Body part amount text.
        ImGui::BulletText(
            "Body parts: %i", (int) anims.body_parts.size()
        );
        
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
    
    //Current sprite text.
    size_t cur_sprite_nr = INVALID;
    if(cur_sprite) {
        cur_sprite_nr = anims.find_sprite(cur_sprite->name);
    }
    ImGui::Text(
        "Current sprite: %s / %i",
        (cur_sprite_nr == INVALID ? "--" : i2s(cur_sprite_nr + 1).c_str()),
        (int) anims.sprites.size()
    );
    
    //Previous sprite button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_PREVIOUS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        if(!anims.sprites.empty()) {
            if(!cur_sprite) {
                pick_sprite(anims.sprites[0]->name, "", false);
            } else {
                size_t new_nr =
                    sum_and_wrap(
                        anims.find_sprite(cur_sprite->name),
                        -1,
                        anims.sprites.size()
                    );
                pick_sprite(anims.sprites[new_nr]->name, "", false);
            }
        }
    }
    set_tooltip(
        "Previous\nsprite."
    );
    
    //Change current sprite button.
    string sprite_button_name =
        (cur_sprite ? cur_sprite->name : NONE_OPTION) + "##sprite";
    ImVec2 sprite_button_size(
        -(EDITOR_ICON_BMP_SIZE + 16.0f), EDITOR_ICON_BMP_SIZE + 6.0f
    );
    ImGui::SameLine();
    if(ImGui::Button(sprite_button_name.c_str(), sprite_button_size)) {
        vector<picker_item> sprite_names;
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            sprite_names.push_back(picker_item(anims.sprites[s]->name));
        }
        open_picker(
            "Pick a sprite, or create a new one",
            sprite_names,
            std::bind(
                &animation_editor::pick_sprite, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3
            ),
            "",
            true
        );
    }
    set_tooltip(
        "Pick a sprite, or create a new one."
    );
    
    //Next sprite button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_NEXT],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        if(!anims.sprites.empty()) {
            if(!cur_sprite) {
                pick_sprite(anims.sprites[0]->name, "", false);
            } else {
                size_t new_nr =
                    sum_and_wrap(
                        anims.find_sprite(cur_sprite->name),
                        1,
                        anims.sprites.size()
                    );
                pick_sprite(anims.sprites[new_nr]->name, "", false);
            }
        }
    }
    set_tooltip(
        "Next\nsprite."
    );
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    if(cur_sprite) {
        //Delete sprite button.
        if(
            ImGui::ImageButton(
                editor_icons[ICON_REMOVE],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            if(cur_sprite) {
                string deleted_sprite_name = cur_sprite->name;
                size_t nr = anims.find_sprite(deleted_sprite_name);
                anims.sprites.erase(anims.sprites.begin() + nr);
                if(anims.sprites.empty()) {
                    nr = INVALID;
                    cur_sprite = NULL;
                    cur_hitbox = NULL;
                    cur_hitbox_nr = INVALID;
                } else {
                    nr = std::min(nr, anims.sprites.size() - 1);
                    pick_sprite(anims.sprites[nr]->name, "", false);
                }
                made_new_changes = true;
                status_text = "Deleted sprite \"" + deleted_sprite_name + "\".";
            } else {
                status_text = "You have to select a sprite to delete!";
            }
        }
        set_tooltip(
            "Delete the current sprite."
        );
    }
    
    if(cur_sprite) {
    
        if(anims.sprites.size() > 1) {
        
            //Import sprite button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    editor_icons[ICON_DUPLICATE],
                    ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
                )
            ) {
                ImGui::OpenPopup("importSprite");
            }
            set_tooltip(
                "Import the data from another sprite."
            );
            
            //Import sprite popup.
            vector<string> import_sprite_names;
            for(size_t s = 0; s < anims.sprites.size(); ++s) {
                if(anims.sprites[s] == cur_sprite) continue;
                import_sprite_names.push_back(anims.sprites[s]->name);
            }
            string picked_sprite;
            if(
                list_popup("importSprite", import_sprite_names, &picked_sprite)
            ) {
                import_sprite_file_data(picked_sprite);
                import_sprite_transformation_data(picked_sprite);
                import_sprite_hitbox_data(picked_sprite);
                import_sprite_top_data(picked_sprite);
                status_text =
                    "Imported all sprite data from \"" + picked_sprite + "\".";
            }
            
        }
        
        //Rename sprite button.
        static string rename_sprite_name;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                editor_icons[ICON_INFO],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            rename_sprite_name = cur_sprite->name;
            ImGui::OpenPopup("renameSprite");
        }
        set_tooltip(
            "Rename the current sprite."
        );
        
        //Rename sprite popup.
        if(input_popup("renameSprite", "New name:", &rename_sprite_name)) {
            rename_sprite(cur_sprite, rename_sprite_name);
        }
        
        //Resize sprite button.
        static string resize_sprite_mult;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                editor_icons[ICON_RESIZE],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            resize_sprite_mult = "1.0";
            ImGui::OpenPopup("resizeSprite");
        }
        set_tooltip(
            "Resize the current sprite."
        );
        
        //Resize sprite popup.
        if(input_popup("resizeSprite", "Resize by:", &resize_sprite_mult)) {
            resize_sprite(cur_sprite, s2f(resize_sprite_mult));
        }
        
        ImVec2 mode_buttons_size(-1.0f, 24.0f);
        
        //Sprite bitmap button.
        if(ImGui::Button("Bitmap", mode_buttons_size)) {
            pre_sprite_bmp_cam_pos = game.cam.target_pos;
            pre_sprite_bmp_cam_zoom = game.cam.target_zoom;
            center_camera_on_sprite_bitmap(true);
            change_state(EDITOR_STATE_SPRITE_BITMAP);
        }
        set_tooltip(
            "Pick what part of an image makes up this sprite."
        );
        
        if(cur_sprite->bitmap) {
            //Sprite transformation button.
            if(ImGui::Button("Transformation", mode_buttons_size)) {
                change_state(EDITOR_STATE_SPRITE_TRANSFORM);
            }
            set_tooltip(
                "Offset, scale, or rotate the sprite's image."
            );
        }
        
        if(!anims.body_parts.empty()) {
            //Sprite hitboxes button.
            if(ImGui::Button("Hitboxes", mode_buttons_size)) {
                if(cur_sprite && !cur_sprite->hitboxes.empty()) {
                    cur_hitbox = &cur_sprite->hitboxes[0];
                    cur_hitbox_nr = 0;
                    change_state(EDITOR_STATE_HITBOXES);
                }
            }
            set_tooltip(
                "Edit this sprite's hitboxes."
            );
        }
        
        if(
            loaded_mob_type &&
            loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
        ) {
        
            //Sprite Pikmin top button.
            if(ImGui::Button("Pikmin top", mode_buttons_size)) {
                change_state(EDITOR_STATE_TOP);
            }
            set_tooltip(
                "Edit the Pikmin's top (maturity) for this sprite."
            );
            
        }
        
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
        game.cam.set_pos(pre_sprite_bmp_cam_pos);
        game.cam.set_zoom(pre_sprite_bmp_cam_zoom);
        change_state(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panel_title("BITMAP", 78.0f);
    
    if(anims.sprites.size() > 1) {
    
        //Import bitmap data button.
        if(
            ImGui::ImageButton(
                editor_icons[ICON_DUPLICATE],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteBitmap");
        }
        set_tooltip(
            "Import the bitmap data from another sprite."
        );
        
        //Import bitmap popup.
        vector<string> import_sprite_names;
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            if(anims.sprites[s] == cur_sprite) continue;
            import_sprite_names.push_back(anims.sprites[s]->name);
        }
        string picked_sprite;
        if(
            list_popup(
                "importSpriteBitmap", import_sprite_names, &picked_sprite
            )
        ) {
            import_sprite_file_data(picked_sprite);
            center_camera_on_sprite_bitmap(false);
            status_text = "Imported file data from \"" + picked_sprite + "\".";
        }
        
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Browse for spritesheet button.
    if(ImGui::Button("...")) {
        FILE_DIALOG_RESULTS result = FILE_DIALOG_RES_SUCCESS;
        vector<string> f =
            prompt_file_dialog_locked_to_folder(
                GRAPHICS_FOLDER_PATH,
                "Please choose the bitmap to get the sprites from.",
                "*.png",
                ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                ALLEGRO_FILECHOOSER_PICTURES,
                &result
            );
            
        switch(result) {
        case FILE_DIALOG_RES_WRONG_FOLDER: {
            //File doesn't belong to the folder.
            status_text = "The chosen image is not in the graphics folder!";
            break;
        } case FILE_DIALOG_RES_CANCELED: {
            //User canceled.
            break;
        } case FILE_DIALOG_RES_SUCCESS: {
            cur_sprite->set_bitmap(
                f[0], cur_sprite->file_pos, cur_sprite->file_size
            );
            center_camera_on_sprite_bitmap(true);
            made_new_changes = true;
            status_text = "Picked an image successfully.";
            break;
        }
        }
    }
    set_tooltip("Browse for a spritesheet file to use.");
    
    //Spritesheet file name input.
    string file_name = cur_sprite->file;
    ImGui::SameLine();
    if(ImGui::InputText("File", &file_name)) {
        cur_sprite->set_bitmap(
            file_name, cur_sprite->file_pos, cur_sprite->file_size
        );
        center_camera_on_sprite_bitmap(true);
        made_new_changes = true;
    }
    set_tooltip(
        "File name of the bitmap to use as a spritesheet, in the "
        "Graphics folder. Extension included. e.g. "
        "\"Large_Fly.png\""
    );
    
    //Sprite top-left coordinates value.
    int top_left[2] =
    { (int) cur_sprite->file_pos.x, (int) cur_sprite->file_pos.y };
    if(
        ImGui::DragInt2(
            "Top-left", top_left, 0.05f, 0.0f, INT_MAX
        )
    ) {
        cur_sprite->set_bitmap(
            cur_sprite->file,
            point(top_left[0], top_left[1]), cur_sprite->file_size
        );
        made_new_changes = true;
    }
    
    //Sprite size value.
    int size[2] =
    { (int) cur_sprite->file_size.x, (int) cur_sprite->file_size.y };
    if(
        ImGui::DragInt2(
            "Size", size, 0.05f, 0.0f, INT_MAX
        )
    ) {
        cur_sprite->set_bitmap(
            cur_sprite->file,
            cur_sprite->file_pos, point(size[0], size[1])
        );
        made_new_changes = true;
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Canvas explanation text.
    ImGui::TextWrapped(
        "Click parts of the image on the left to %s the selection limits.",
        sprite_bmp_add_mode ? "expand" : "set"
    );
    
    //Add to selection checkbox.
    ImGui::Checkbox("Add to selection", &sprite_bmp_add_mode);
    set_tooltip(
        "Add to the existing selection instead of replacing it."
    );
    
    if(
        cur_sprite->file_pos.x != 0.0f ||
        cur_sprite->file_pos.y != 0.0f ||
        cur_sprite->file_size.x != 0.0f ||
        cur_sprite->file_size.y != 0.0f
    ) {
    
        //Clear selection button.
        if(
            ImGui::Button("Clear selection")
        ) {
            cur_sprite->file_pos = point();
            cur_sprite->file_size = point();
            cur_sprite->set_bitmap(
                cur_sprite->file, cur_sprite->file_pos, cur_sprite->file_size
            );
            made_new_changes = true;
        }
        
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
        cur_hitbox = NULL;
        cur_hitbox_nr = INVALID;
        change_state(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panel_title("HITBOXES", 96.0f);
    
    //Hitbox name text.
    ImGui::Text(
        "Hitbox: %s",
        cur_hitbox ? cur_hitbox->body_part_name.c_str() : NONE_OPTION.c_str()
    );
    
    //Previous hitbox button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_PREVIOUS],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
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
    }
    set_tooltip(
        "Select the next hitbox."
    );
    
    if(cur_hitbox && anims.sprites.size() > 1) {
    
        //Import hitbox data button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                editor_icons[ICON_DUPLICATE],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteHitboxes");
        }
        set_tooltip(
            "Import the hitbox data from another sprite."
        );
        
        //Import sprite popup.
        vector<string> import_sprite_names;
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            if(anims.sprites[s] == cur_sprite) continue;
            import_sprite_names.push_back(anims.sprites[s]->name);
        }
        string picked_sprite;
        if(
            list_popup(
                "importSpriteHitboxes", import_sprite_names, &picked_sprite
            )
        ) {
            import_sprite_hitbox_data(picked_sprite);
            status_text =
                "Imported hitbox data from \"" + picked_sprite + "\".";
        }
        
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Side view checkbox.
    ImGui::Checkbox("Use side view", &side_view);
    set_tooltip(
        "Use a side view of the object, so you can adjust hitboxes "
        "horizontally."
    );
    
    if(cur_hitbox) {
        //Hitbox center value.
        if(ImGui::DragFloat2("Center", (float*) &cur_hitbox->pos, 0.05f)) {
            made_new_changes = true;
        }
        
        //Hitbox radius value.
        if(
            ImGui::DragFloat(
                "Radius", &cur_hitbox->radius, 0.05f, 0.001f, FLT_MAX
            )
        ) {
            made_new_changes = true;
        }
        cur_hitbox->radius = std::max(HITBOX_MIN_RADIUS, cur_hitbox->radius);
        
        //Hitbox Z value.
        if(ImGui::DragFloat("Z", &cur_hitbox->z, 0.1f)) {
            made_new_changes = true;
        }
        set_tooltip(
            "Altitude of the hitbox's bottom."
        );
        
        if(
            ImGui::DragFloat("Height", &cur_hitbox->height, 0.1f, 0.0f, FLT_MAX)
        ) {
            made_new_changes = true;
        }
        set_tooltip(
            "Hitbox's height. 0 = spans infinitely vertically."
        );
        cur_hitbox->height = std::max(0.0f, cur_hitbox->height);
        
        //Spacer dummy widget.
        ImGui::Dummy(ImVec2(0, 16));
        
        //Hitbox type text.
        ImGui::Text("Hitbox type:");
        
        //Normal hitbox radio button.
        int type_int = cur_hitbox->type;
        if(ImGui::RadioButton("Normal", &type_int, HITBOX_TYPE_NORMAL)) {
            made_new_changes = true;
        }
        set_tooltip(
            "Normal hitbox, one that can be damaged."
        );
        
        //Attack hitbox radio button.
        if(ImGui::RadioButton("Attack", &type_int, HITBOX_TYPE_ATTACK)) {
            made_new_changes = true;
        }
        set_tooltip(
            "Attack hitbox, one that damages opponents."
        );
        
        //Disabled hitbox radio button.
        if(ImGui::RadioButton("Disabled", &type_int, HITBOX_TYPE_DISABLED)) {
            made_new_changes = true;
        }
        set_tooltip(
            "Disabled hitbox, one that cannot be interacted with."
        );
        cur_hitbox->type = type_int;
        
        ImGui::Indent();
        
        switch(cur_hitbox->type) {
        case HITBOX_TYPE_NORMAL: {
    
            //Defense multiplier value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat("Defense multiplier", &cur_hitbox->value, 0.01)
            ) {
                made_new_changes = true;
            }
            set_tooltip(
                "Opponent attacks will have their damage divided "
                "by this amount.\n"
                "0 = invulnerable."
            );
            
            //Pikmin latch checkbox.
            if(
                ImGui::Checkbox(
                    "Pikmin can latch", &cur_hitbox->can_pikmin_latch
                )
            ) {
                made_new_changes = true;
            }
            set_tooltip(
                "Can the Pikmin latch on to this hitbox?"
            );
            
            //Spacer dummy widget.
            ImGui::Dummy(ImVec2(0, 16));
            
            //Hazards list.
            process_gui_hitbox_hazards();
            
            break;
        } case HITBOX_TYPE_ATTACK: {
    
            //Power value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat("Power", &cur_hitbox->value, 0.01)
            ) {
                made_new_changes = true;
            }
            set_tooltip(
                "Attack power, in hit points."
            );
            
            //Outward knockback checkbox.
            if(
                ImGui::Checkbox(
                    "Outward knockback", &cur_hitbox->knockback_outward
                )
            ) {
                made_new_changes = true;
            }
            set_tooltip(
                "If true, opponents are knocked away from the hitbox's center."
            );
            
            //Knockback angle value.
            if(!cur_hitbox->knockback_outward) {
                cur_hitbox->knockback_angle =
                    normalize_angle(cur_hitbox->knockback_angle);
                ImGui::SetNextItemWidth(128.0f);
                if(
                    ImGui::SliderAngle(
                        "Knockback angle", &cur_hitbox->knockback_angle,
                        0.0f, 360.0f, "%.2f"
                    )
                ) {
                    made_new_changes = true;
                }
            }
            
            //Knockback strength value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat(
                    "Knockback value", &cur_hitbox->knockback, 0.01
                )
            ) {
                made_new_changes = true;
            }
            set_tooltip(
                "How strong the knockback is. 3 is a good value."
            );
            
            //Wither chance value.
            int wither_chance_int = cur_hitbox->wither_chance;
            ImGui::SetNextItemWidth(128.0f);
            if(ImGui::SliderInt("Wither chance", &wither_chance_int, 0, 100)) {
                made_new_changes = true;
                cur_hitbox->wither_chance = wither_chance_int;
            }
            set_tooltip(
                "Chance of the attack lowering a Pikmin's maturity by one."
            );
            
            //Spacer dummy widget.
            ImGui::Dummy(ImVec2(0, 16));
            
            //Hazards list.
            process_gui_hitbox_hazards();
            
            break;
        }
        }
        
        ImGui::Unindent();
        
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
    
    if(anims.sprites.size() > 1) {
    
        //Import top data button.
        if(
            ImGui::ImageButton(
                editor_icons[ICON_DUPLICATE],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteTop");
        }
        set_tooltip(
            "Import the top data from another sprite."
        );
        
        //Import sprite popup.
        vector<string> import_sprite_names;
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            if(anims.sprites[s] == cur_sprite) continue;
            import_sprite_names.push_back(anims.sprites[s]->name);
        }
        string picked_sprite;
        if(
            list_popup(
                "importSpriteTop", import_sprite_names, &picked_sprite
            )
        ) {
            import_sprite_top_data(picked_sprite);
            status_text =
                "Imported Pikmin top data from \"" + picked_sprite + "\".";
        }
        
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Visible checkbox.
    if(ImGui::Checkbox("Visible", &cur_sprite->top_visible)) {
        made_new_changes = true;
    }
    set_tooltip(
        "Is the top visible in this sprite?"
    );
    
    if(cur_sprite->top_visible) {
    
        //Top center value.
        if(
            ImGui::DragFloat2("Center", (float*) &cur_sprite->top_pos, 0.05f)
        ) {
            made_new_changes = true;
        }
        
        //Top size value.
        if(
            process_size_widgets(
                "Size", cur_sprite->top_size, 0.01f,
                top_keep_aspect_ratio, TOP_MIN_SIZE
            )
        ) {
            made_new_changes = true;
        }
        
        //Keep aspect ratio checkbox.
        ImGui::Indent();
        ImGui::Checkbox("Keep aspect ratio", &top_keep_aspect_ratio);
        ImGui::Unindent();
        set_tooltip("Keep the aspect ratio when resizing the top.");
        
        //Top angle value.
        cur_sprite->top_angle = normalize_angle(cur_sprite->top_angle);
        if(
            ImGui::SliderAngle(
                "Angle", &cur_sprite->top_angle, 0.0f, 360.0f, "%.2f"
            )
        ) {
            made_new_changes = true;
        }
        
        //Spacer dummy widget.
        ImGui::Dummy(ImVec2(0, 16));
        
        //Toggle maturity button.
        if(ImGui::Button("Toggle maturity")) {
            cur_maturity = sum_and_wrap(cur_maturity, 1, N_MATURITIES);
        }
        set_tooltip(
            "View a different maturity top."
        );
        
    }
    
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
    
    if(anims.sprites.size() > 1) {
    
        //Import transformation data button.
        if(
            ImGui::ImageButton(
                editor_icons[ICON_DUPLICATE],
                ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteTransform");
        }
        set_tooltip(
            "Import the transformation data from another sprite."
        );
        
        //Import sprite popup.
        vector<string> import_sprite_names;
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            if(anims.sprites[s] == cur_sprite) continue;
            import_sprite_names.push_back(anims.sprites[s]->name);
        }
        string picked_sprite;
        if(
            list_popup(
                "importSpriteTransform", import_sprite_names, &picked_sprite
            )
        ) {
            import_sprite_transformation_data(picked_sprite);
            status_text =
                "Imported transformation data from \"" + picked_sprite + "\".";
        }
        
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Sprite offset value.
    if(
        ImGui::DragFloat2("Offset", (float*) &cur_sprite->offset, 0.05f)
    ) {
        made_new_changes = true;
    }
    
    //Sprite scale value.
    if(
        process_size_widgets(
            "Scale",
            cur_sprite->scale,
            0.005f,
            cur_sprite_keep_aspect_ratio,
            -FLT_MAX
        )
    ) {
        made_new_changes = true;
    }
    
    //Sprite flip X button.
    ImGui::Indent();
    if(
        ImGui::Button("Flip X")
    ) {
        cur_sprite->scale.x *= -1.0f;
        made_new_changes = true;
    }
    
    //Sprite flip Y button.
    ImGui::SameLine();
    if(
        ImGui::Button("Flip Y")
    ) {
        cur_sprite->scale.y *= -1.0f;
        made_new_changes = true;
    }
    
    //Keep aspect ratio checkbox.
    ImGui::Checkbox("Keep aspect ratio", &cur_sprite_keep_aspect_ratio);
    ImGui::Unindent();
    set_tooltip("Keep the aspect ratio when resizing the sprite.");
    
    //Sprite angle value.
    cur_sprite->angle = normalize_angle(cur_sprite->angle);
    if(
        ImGui::SliderAngle("Angle", &cur_sprite->angle, 0.0f, 360.0f, "%.2f")
    ) {
        made_new_changes = true;
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    if(anims.sprites.size() > 1) {
    
        //Comparison sprite node.
        if(saveable_tree_node("transformation", "Comparison sprite")) {
        
            //Use comparison checkbox.
            ImGui::Checkbox("Use comparison", &comparison);
            set_tooltip(
                "Show another sprite, to help you align and scale this one.",
                "Ctrl + C"
            );
            
            if(comparison) {
            
                //Comparison sprite combobox.
                vector<string> all_sprites;
                for(size_t s = 0; s < anims.sprites.size(); ++s) {
                    if(cur_sprite == anims.sprites[s]) continue;
                    all_sprites.push_back(anims.sprites[s]->name);
                }
                static string comparison_sprite_name;
                ImGui::Combo("Sprite", &comparison_sprite_name, all_sprites);
                set_tooltip(
                    "Choose another sprite to serve as a comparison."
                );
                comparison_sprite =
                    anims.sprites[anims.find_sprite(comparison_sprite_name)];
                    
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
        resize_everything(resize_mult);
        resize_mult = 1.0f;
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
        set_all_sprite_scales(scales_value);
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
    ImGui::Text("%s", (status_text.empty() ? "Ready." : status_text.c_str()));
    
    //Spacer dummy widget.
    ImGui::SameLine();
    float size =
        canvas_separator_x - ImGui::GetItemRectSize().x -
        MOUSE_COORDS_TEXT_WIDTH;
    ImGui::Dummy(ImVec2(size, 0));
    
    //Mouse coordinates text.
    if(
        (!is_mouse_in_gui || is_m1_pressed) &&
        state != EDITOR_STATE_SPRITE_BITMAP &&
        (state != EDITOR_STATE_HITBOXES || !side_view)
    ) {
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
    
    //Load button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_LOAD],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        press_load_button();
    }
    load_widget_pos = get_last_widget_pos();
    set_tooltip(
        "Pick a file to load.",
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
        press_save_button();
    }
    set_tooltip(
        "Save the animation data into the files on disk.",
        "Ctrl + S"
    );
    
    //Toggle grid button.
    ImGui::SameLine(0, 16);
    if(
        ImGui::ImageButton(
            editor_icons[ICON_GRID],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        press_grid_button();
    }
    set_tooltip(
        "Toggle visibility of the grid.",
        "Ctrl + G"
    );
    
    //Toggle hitboxes button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            editor_icons[ICON_HITBOXES],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE)
        )
    ) {
        press_hitboxes_button();
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
        press_mob_radius_button();
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
        press_pikmin_silhouette_button();
    }
    set_tooltip(
        "Toggle visibility of a lying Pikmin silhouette.",
        "Ctrl + P"
    );
}
