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
#include "../../libs/imgui/imgui_impl_allegro5.h"
#include "../../libs/imgui/imgui_stdlib.h"
#include "../../load.h"
#include "../../utils/allegro_utils.h"
#include "../../utils/imgui_utils.h"
#include "../../utils/string_utils.h"


/**
 * @brief Opens the "load" dialog.
 */
void animation_editor::open_load_dialog() {
    //Set up the picker's behavior and data.
    vector<picker_item> file_items;
    for(const auto &a : game.content.global_anim_dbs.list) {
        file_items.push_back(
            picker_item(
                a.second.name,
                "Pack: " + a.second.manifest->pack, "Global animations",
                (void*) a.second.manifest,
                "Internal name: " + a.first + "\n"
                "Path: " +
                game.content.global_anim_dbs.manifest_to_path(*a.second.manifest)
            )
        );
    }
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        if(c == MOB_CATEGORY_NONE) continue;
        mob_category* cat = game.mob_categories.get((MOB_CATEGORY) c);
        for(const auto &a : game.content.mob_anim_dbs.list[c]) {
            mob_type* type = cat->get_type(a.first);
            if(!type) continue;
            file_items.push_back(
                picker_item(
                    type->name,
                    "Pack: " + a.second.manifest->pack, cat->name + " objects",
                    (void*) a.second.manifest,
                    "Internal name: " + a.first + "\n"
                    "Path: " +
                    game.content.mob_anim_dbs.manifest_to_path(
                        *a.second.manifest, cat->folder_name,
                        type->manifest->internal_name
                    )
                )
            );
        }
    }
    load_dialog_picker = picker_info(this);
    load_dialog_picker.items = file_items;
    load_dialog_picker.pick_callback =
        std::bind(
            &animation_editor::pick_anim_db_file, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5
        );
        
    //Open the dialog that will contain the picker and history.
    open_dialog(
        "Load a file or create a new one",
        std::bind(&animation_editor::process_gui_load_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&animation_editor::close_load_dialog, this);
}


/**
 * @brief Opens the "new" dialog.
 */
void animation_editor::open_new_dialog() {
    open_dialog(
        "Create a new animation database",
        std::bind(&animation_editor::process_gui_new_dialog, this)
    );
    dialogs.back()->custom_size = point(400, 0);
}


/**
 * @brief Opens the options dialog.
 *
 */
void animation_editor::open_options_dialog() {
    open_dialog(
        "Options",
        std::bind(&animation_editor::process_gui_options_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&animation_editor::close_options_dialog, this);
}


/**
 * @brief Processes Dear ImGui for this frame.
 */
void animation_editor::process_gui() {
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.win_w, game.win_h));
    ImGui::Begin(
        "Animation editor", nullptr,
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
    ImGui::GetWindowDrawList()->AddCallback(draw_canvas_imgui_callback, nullptr);
    
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
}


/**
 * @brief Processes the Dear ImGui control panel for this frame.
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
    } case EDITOR_STATE_INFO: {
        process_gui_panel_info();
        break;
    } case EDITOR_STATE_TOOLS: {
        process_gui_panel_tools();
        break;
    }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the list of the current hitbox's hazards,
 * as well as the widgets necessary to control it, for this frame.
 */
void animation_editor::process_gui_hitbox_hazards() {
    //Hitbox hazards node.
    if(saveable_tree_node("hitbox", "Hazards")) {
    
        static int selected_hazard_idx = 0;
        
        //Hitbox hazard addition button.
        if(
            ImGui::ImageButton(
                "hitboxAddButton",
                editor_icons[EDITOR_ICON_ADD],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
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
        for(auto &h : game.content.hazards.list) {
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
                selected_hazard_idx = (int) list.size();
                changes_mgr.mark_as_changed();
                set_status(
                    "Added hazard \"" + picked_hazard + "\" to the hitbox."
                );
            }
        }
        
        //Hitbox hazard removal button.
        if(selected_hazard_idx >= 0 && !cur_hitbox->hazards_str.empty()) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "hitboxRemButton",
                    editor_icons[EDITOR_ICON_REMOVE],
                    ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                )
            ) {
                vector<string> list =
                    semicolon_list_to_vector(cur_hitbox->hazards_str);
                if(selected_hazard_idx < (int) list.size()) {
                    string hazard_name = list[selected_hazard_idx];
                    cur_hitbox->hazards_str.clear();
                    for(size_t h = 0; h < list.size(); h++) {
                        if(h == (size_t) selected_hazard_idx) continue;
                        cur_hitbox->hazards_str += list[h] + ";";
                    }
                    if(!cur_hitbox->hazards_str.empty()) {
                        //Delete the trailing semicolon.
                        cur_hitbox->hazards_str.pop_back();
                    }
                    selected_hazard_idx =
                        std::min(
                            selected_hazard_idx, (int) list.size() - 2
                        );
                    changes_mgr.mark_as_changed();
                    set_status(
                        "Removed hazard \"" + hazard_name +
                        "\" from the hitbox."
                    );
                }
            }
            set_tooltip(
                "Remove the selected hazard from the list of "
                "hazards this hitbox has."
            );
        }
        
        //Hitbox hazard list.
        ImGui::ListBox(
            "Hazards", &selected_hazard_idx,
            semicolon_list_to_vector(cur_hitbox->hazards_str),
            4
        );
        set_tooltip(
            "List of hazards this hitbox has."
        );
        
        ImGui::TreePop();
    }
}


/**
 * @brief Processes the "load" dialog for this frame.
 */
void animation_editor::process_gui_load_dialog() {
    //History node.
    process_gui_history(
    [this](const string &path) -> string {
        return get_path_short_name(path);
    },
    [this](const string &path) {
        close_top_dialog();
        load_anim_db_file(path, true);
    }
    );
    
    //New node.
    ImGui::Spacer();
    if(saveable_tree_node("load", "New")) {
        if(ImGui::Button("Create new...", ImVec2(168.0f, 32.0f))) {
            open_new_dialog();
        }
        
        ImGui::TreePop();
    }
    set_tooltip("Creates a new animation database.");
    
    //Load node.
    ImGui::Spacer();
    if(saveable_tree_node("load", "Load")) {
        load_dialog_picker.process();
        
        ImGui::TreePop();
    }
}


/**
 * @brief Processes the Dear ImGui menu bar for this frame.
 */
void animation_editor::process_gui_menu_bar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load file item.
            if(ImGui::MenuItem("Load file...", "Ctrl+L")) {
                load_widget_pos = get_last_widget_pos();
                load_cmd(1.0f);
            }
            set_tooltip(
                "Pick a file to load.",
                "Ctrl + L"
            );
            
            //Reload current file item.
            if(ImGui::MenuItem("Reload current file")) {
                reload_widget_pos = get_last_widget_pos();
                reload_cmd(1.0f);
            }
            set_tooltip(
                "Lose all changes and reload the current file from the disk."
            );
            
            //Save current file item.
            if(ImGui::MenuItem("Save current file", "Ctrl+S")) {
                save_cmd(1.0f);
            }
            set_tooltip(
                "Save the animation database into the file on disk.",
                "Ctrl + S"
            );
            
            //Separator item.
            ImGui::Separator();
            
            //Options menu item.
            if(ImGui::MenuItem("Options...")) {
                open_options_dialog();
            }
            set_tooltip(
                "Open the options menu, so you can tweak your preferences."
            );
            
            //Quit editor item.
            if(ImGui::MenuItem("Quit", "Ctrl+Q")) {
                quit_widget_pos = get_last_widget_pos();
                quit_cmd(1.0f);
            }
            set_tooltip(
                "Quit the animation editor.",
                "Ctrl + Q"
            );
            
            ImGui::EndMenu();
            
        }
        
        //View menu.
        if(ImGui::BeginMenu("View")) {
        
            //Zoom in item.
            if(ImGui::MenuItem("Zoom in", "Plus")) {
                zoom_in_cmd(1.0f);
            }
            set_tooltip(
                "Zooms the camera in a bit.",
                "Plus"
            );
            
            //Zoom out item.
            if(ImGui::MenuItem("Zoom out", "Minus")) {
                zoom_out_cmd(1.0f);
            }
            set_tooltip(
                "Zooms the camera out a bit.",
                "Minus"
            );
            
            //Zoom and position reset item.
            if(ImGui::MenuItem("Zoom/position reset", "0")) {
                zoom_and_pos_reset_cmd(1.0f);
            }
            set_tooltip(
                "Reset the zoom level, and if pressed again,\n"
                "reset the camera position.",
                "0"
            );
            
            //Zoom everything item.
            if(ImGui::MenuItem("Zoom onto everything", "Home")) {
                zoom_everything_cmd(1.0f);
            }
            set_tooltip(
                "Move and zoom the camera so that everything in the animation\n"
                "fits nicely into view.",
                "Home"
            );
            
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
                set_status(state_str + " tooltips.");
                save_options();
            }
            set_tooltip(
                "Whether tooltips should appear when you place your mouse on\n"
                "top of something in the GUI. Like the tooltip you are\n"
                "reading right now."
            );
            
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
                    "check out the tutorial in the manual, located "
                    "in the engine's folder.";
                show_message_box(
                    game.display, "Help", "Animation editor help",
                    help_str.c_str(), nullptr, 0
                );
            }
            set_tooltip(
                "Opens a general help message for this editor."
            );
            
            ImGui::EndMenu();
            
        }
        
        ImGui::EndMenuBar();
        
    }
}


/**
 * @brief Processes the Dear ImGui "new" dialog for this frame.
 */
void animation_editor::process_gui_new_dialog() {
    static string pack;
    static int type = 0;
    static string custom_mob_cat;
    static mob_type* mob_type_ptr = nullptr;
    static string problem;
    static string internal_name;
    static string anim_path;
    static bool must_update = true;
    
    //Pack widgets.
    must_update |= process_gui_new_dialog_pack_widgets(&pack);
    
    //Global animation radio.
    ImGui::Spacer();
    must_update |= ImGui::RadioButton("Global animation", &type, 0);
    
    //Mob type animation radio.
    ImGui::SameLine();
    must_update |= ImGui::RadioButton("Object type", &type, 1);
    
    ImGui::Spacer();
    
    if(type == 0) {
        //Internal name input.
        must_update |= ImGui::InputText("Internal name", &internal_name);
        set_tooltip(
            "Internal name of the new animation database.\n"
            "Remember to keep it simple, type in lowercase, "
            "and use underscores!"
        );
        
        //Small spacer dummy widget.
        ImGui::Dummy(ImVec2(0, 19));
        
    } else {
        //Mob type widgets.
        must_update |=
            process_gui_mob_type_widgets(&custom_mob_cat, &mob_type_ptr);
            
    }
    
    //Check if everything's ok.
    if(must_update) {
        problem.clear();
        if(type == 0) {
            if(internal_name.empty()) {
                problem = "You have to type an internal name first!";
            } else {
                content_manifest temp_man;
                temp_man.internal_name = internal_name;
                temp_man.pack = pack;
                anim_path =
                    game.content.global_anim_dbs.manifest_to_path(temp_man);
                if(file_exists(anim_path)) {
                    problem =
                        "There is already a global animation database\n"
                        "with that internal name in that pack!";
                }
            }
        } else {
            if(!mob_type_ptr) {
                problem = "You have to choose an object type first!";
            } else {
                content_manifest temp_man;
                temp_man.internal_name = FILE_NAMES::MOB_TYPE_ANIMATION;
                temp_man.pack = pack;
                anim_path =
                    game.content.mob_anim_dbs.manifest_to_path(
                        temp_man,
                        mob_type_ptr->category->folder_name,
                        mob_type_ptr->manifest->internal_name
                    );
                if(file_exists(anim_path)) {
                    problem =
                        "There is already an animation database\n"
                        "file for that object type in that pack!";
                }
            }
        }
        must_update = false;
    }
    
    //Create button.
    ImGui::Spacer();
    ImGui::SetupCentering(140);
    if(!problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create animation database", ImVec2(140, 40))) {
        auto really_create = [ = ] () {
            close_top_dialog();
            close_top_dialog(); //Close the load dialog.
            create_anim_db(anim_path);
            pack.clear();
            type = 0;
            custom_mob_cat.clear();
            mob_type_ptr = nullptr;
            problem.clear();
            internal_name.clear();
            anim_path.clear();
            must_update = true;
        };
        
        if(pack == FOLDER_NAMES::BASE_PACK && !game.options.engine_developer) {
            open_base_content_warning_dialog(really_create);
        } else {
            really_create();
        }
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    set_tooltip(
        problem.empty() ? "Create the animation database!" : problem
    );
}


/**
 * @brief Processes the options dialog for this frame.
 */
void animation_editor::process_gui_options_dialog() {
    //Controls node.
    if(saveable_tree_node("options", "Controls")) {
    
        //Middle mouse button pans checkbox.
        ImGui::Checkbox("Use MMB to pan", &game.options.editor_mmb_pan);
        set_tooltip(
            "Use the middle mouse button to pan the camera\n"
            "(and RMB to reset camera/zoom).\n"
            "Default: " +
            b2s(OPTIONS::DEF_EDITOR_MMB_PAN) + "."
        );
        
        //Drag threshold value.
        int drag_threshold = (int) game.options.editor_mouse_drag_threshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Drag threshold", &drag_threshold,
            0.1f, 0, INT_MAX
        );
        set_tooltip(
            "Cursor must move these many pixels to be considered a drag.\n"
            "Default: " + i2s(OPTIONS::DEF_EDITOR_MOUSE_DRAG_THRESHOLD) +
            ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.editor_mouse_drag_threshold = drag_threshold;
        
        ImGui::TreePop();
        
    }
    
    ImGui::Spacer();
    
    process_gui_editor_style();
    
    ImGui::Spacer();
    
    //Misc. node.
    if(saveable_tree_node("options", "Misc.")) {
    
        //Background texture checkbox.
        if(ImGui::Checkbox("Use background texture", &use_bg)) {
            if(!use_bg) {
                if(bg) {
                    al_destroy_bitmap(bg);
                    bg = nullptr;
                }
                game.options.anim_editor_bg_texture.clear();
            }
        }
        set_tooltip(
            "Check this to use a repeating texture on the background\n"
            "of the editor."
        );
        
        if(use_bg) {
            ImGui::Indent();
            
            //Background texture browse button.
            if(ImGui::Button("Browse...", ImVec2(96.0f, 32.0f))) {
                vector<string> f =
                    prompt_file_dialog(
                        FOLDER_PATHS_FROM_ROOT::BASE_PACK + "/" + FOLDER_PATHS_FROM_PACK::TEXTURES,
                        "Please choose a background texture.",
                        "*.*", 0, game.display
                    );
                    
                if(!f.empty() && !f[0].empty()) {
                    game.options.anim_editor_bg_texture = f[0];
                    if(bg) {
                        al_destroy_bitmap(bg);
                        bg = nullptr;
                    }
                    bg =
                        load_bmp(
                            game.options.anim_editor_bg_texture,
                            nullptr, false, false, false
                        );
                }
            }
            set_tooltip(
                "Browse for which texture file to use."
            );
            
            //Background texture name.
            string file_name;
            if(!game.options.anim_editor_bg_texture.empty()) {
                file_name =
                    split(game.options.anim_editor_bg_texture, "/").back();
            }
            ImGui::SameLine();
            ImGui::Text("%s", file_name.c_str());
            
            ImGui::Unindent();
        }
        
        ImGui::TreePop();
        
    }
}


/**
 * @brief Processes the Dear ImGui animation control panel for this frame.
 *
 */
void animation_editor::process_gui_panel_animation() {
    ImGui::BeginChild("animation");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("ANIMATIONS");
    
    //Current animation text.
    size_t cur_anim_idx = INVALID;
    if(cur_anim_i.cur_anim) {
        cur_anim_idx = db.find_animation(cur_anim_i.cur_anim->name);
    }
    ImGui::Text(
        "Current animation: %s / %i",
        (cur_anim_idx == INVALID ? "--" : i2s(cur_anim_idx + 1).c_str()),
        (int) db.animations.size()
    );
    
    //Previous animation button.
    if(
        ImGui::ImageButton(
            "prevAnimButton",
            editor_icons[EDITOR_ICON_PREVIOUS],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.animations.empty()) {
            if(!cur_anim_i.cur_anim) {
                pick_animation(db.animations[0]->name, "", "", nullptr, false);
            } else {
                size_t new_idx =
                    sum_and_wrap(
                        (int) db.find_animation(cur_anim_i.cur_anim->name),
                        -1,
                        (int) db.animations.size()
                    );
                pick_animation(db.animations[new_idx]->name, "", "", nullptr, false);
            }
        }
    }
    set_tooltip(
        "Previous\nanimation."
    );
    
    //Change current animation button.
    string anim_button_name =
        (
            cur_anim_i.cur_anim ?
            cur_anim_i.cur_anim->name :
            NONE_OPTION
        ) + "##anim";
    ImVec2 anim_button_size(
        -(EDITOR::ICON_BMP_SIZE + 16.0f), EDITOR::ICON_BMP_SIZE + 6.0f
    );
    ImGui::SameLine();
    if(ImGui::Button(anim_button_name.c_str(), anim_button_size)) {
        vector<picker_item> anim_names;
        for(size_t a = 0; a < db.animations.size(); a++) {
            ALLEGRO_BITMAP* anim_frame_1 = nullptr;
            if(!db.animations[a]->frames.empty()) {
                size_t s_pos =
                    db.find_sprite(
                        db.animations[a]->frames[0].sprite_name
                    );
                if(s_pos != INVALID) {
                    anim_frame_1 = db.sprites[s_pos]->bitmap;
                }
            }
            anim_names.push_back(
                picker_item(db.animations[a]->name, "", "", anim_frame_1)
            );
        }
        open_picker_dialog(
            "Pick an animation, or create a new one",
            anim_names,
            std::bind(
                &animation_editor::pick_animation, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5
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
            "nextAnimButton",
            editor_icons[EDITOR_ICON_NEXT],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.animations.empty()) {
            if(!cur_anim_i.cur_anim) {
                pick_animation(db.animations[0]->name, "", "", nullptr, false);
            } else {
                size_t new_idx =
                    sum_and_wrap(
                        (int) db.find_animation(cur_anim_i.cur_anim->name),
                        1,
                        (int) db.animations.size()
                    );
                pick_animation(db.animations[new_idx]->name, "", "", nullptr, false);
            }
        }
    }
    set_tooltip(
        "Next\nanimation."
    );
    
    ImGui::Spacer();
    
    if(cur_anim_i.cur_anim) {
    
        //Delete animation button.
        if(
            ImGui::ImageButton(
                "delAnimButton",
                editor_icons[EDITOR_ICON_REMOVE],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            string cur_anim_name = cur_anim_i.cur_anim->name;
            size_t nr = db.find_animation(cur_anim_name);
            db.animations.erase(db.animations.begin() + nr);
            if(db.animations.empty()) {
                cur_anim_i.clear();
            } else {
                nr = std::min(nr, db.animations.size() - 1);
                pick_animation(db.animations[nr]->name, "", "", nullptr, false);
            }
            anim_playing = false;
            changes_mgr.mark_as_changed();
            set_status("Deleted animation \"" + cur_anim_name + "\".");
        }
        set_tooltip(
            "Delete the current animation."
        );
        
    }
    
    if(cur_anim_i.cur_anim) {
    
        if(db.animations.size() > 1) {
        
            //Import animation button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "importAnimButton",
                    editor_icons[EDITOR_ICON_DUPLICATE],
                    ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                )
            ) {
                ImGui::OpenPopup("importAnim");
            }
            set_tooltip(
                "Import the data from another animation."
            );
            
            //Import animation popup.
            vector<string> import_anim_names;
            for(size_t a = 0; a < db.animations.size(); a++) {
                if(db.animations[a] == cur_anim_i.cur_anim) continue;
                import_anim_names.push_back(db.animations[a]->name);
            }
            string picked_anim;
            if(list_popup("importAnim", import_anim_names, &picked_anim)) {
                import_animation_data(picked_anim);
                set_status(
                    "Imported animation data from \"" + picked_anim + "\"."
                );
            }
            
        }
        
        //Rename animation button.
        static string rename_anim_name;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "renameAnimButton",
                editor_icons[EDITOR_ICON_INFO],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            duplicate_string(cur_anim_i.cur_anim->name, rename_anim_name);
            ImGui::OpenPopup("renameAnim");
        }
        set_tooltip(
            "Rename the current animation."
        );
        
        //Rename animation popup.
        if(input_popup("renameAnim", "New name:", &rename_anim_name)) {
            rename_animation(cur_anim_i.cur_anim, rename_anim_name);
        }
        
        //Animation data node.
        if(saveable_tree_node("animation", "Animation data")) {
        
            //Loop frame value.
            int loop_frame = (int) cur_anim_i.cur_anim->loop_frame + 1;
            if(
                ImGui::DragInt(
                    "Loop frame", &loop_frame, 0.1f, 1,
                    cur_anim_i.cur_anim->frames.empty() ?
                    1 :
                    (int) cur_anim_i.cur_anim->frames.size()
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "The animation loops back to this frame when it "
                "reaches the last one.",
                "", WIDGET_EXPLANATION_DRAG
            );
            loop_frame =
                clamp(
                    loop_frame, 1,
                    cur_anim_i.cur_anim->frames.empty() ?
                    1 :
                    cur_anim_i.cur_anim->frames.size()
                );
            cur_anim_i.cur_anim->loop_frame = loop_frame - 1;
            
            //Hit rate slider.
            int hit_rate = cur_anim_i.cur_anim->hit_rate;
            if(ImGui::SliderInt("Hit rate", &hit_rate, 0, 100)) {
                changes_mgr.mark_as_changed();
                cur_anim_i.cur_anim->hit_rate = hit_rate;
            }
            set_tooltip(
                "If this attack can knock back Pikmin, this indicates "
                "the chance that it will hit.\n"
                "0 means it will always miss, 50 means it will hit "
                "half the time, etc.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            //Animation information text.
            ImGui::TextDisabled("(Animation info)");
            string anim_info_str =
                "Total duration: " + f2s(cur_anim_i.cur_anim->get_duration()) + "s";
            if(cur_anim_i.cur_anim->loop_frame != 0) {
                anim_info_str +=
                    "\nLoop segment duration: " +
                    f2s(cur_anim_i.cur_anim->get_loop_duration()) + "s";
            }
            set_tooltip(anim_info_str);
            
            ImGui::TreePop();
        }
        
        //Frame list node.
        ImGui::Spacer();
        if(saveable_tree_node("animation", "Frame list")) {
        
            frame* frame_ptr = nullptr;
            if(
                cur_anim_i.cur_frame_idx == INVALID &&
                !cur_anim_i.cur_anim->frames.empty()
            ) {
                cur_anim_i.cur_frame_idx = 0;
                cur_anim_i.cur_frame_time = 0.0f;
            }
            if(cur_anim_i.valid_frame()) {
                frame_ptr =
                    &(cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx]);
            }
            
            //Current frame text.
            ImGui::Text(
                "Current frame: %s / %i",
                frame_ptr ? i2s(cur_anim_i.cur_frame_idx + 1).c_str() : "--",
                (int) cur_anim_i.cur_anim->frames.size()
            );
            
            if(frame_ptr) {
                //Play/pause button.
                if(
                    ImGui::ImageButton(
                        "playButton",
                        editor_icons[EDITOR_ICON_PLAY_PAUSE],
                        ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                    )
                ) {
                    play_animation_cmd(1.0f);
                }
                set_tooltip(
                    "Play or pause the animation.\n"
                    "Hold Shift to start from the beginning.",
                    "Spacebar"
                );
                
                //Previous frame button.
                ImGui::SameLine();
                if(
                    ImGui::ImageButton(
                        "prevFrameButton",
                        editor_icons[EDITOR_ICON_PREVIOUS],
                        ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                    )
                ) {
                    anim_playing = false;
                    if(!cur_anim_i.cur_anim->frames.empty()) {
                        if(cur_anim_i.cur_frame_idx == INVALID) {
                            cur_anim_i.cur_frame_idx = 0;
                        } else if(cur_anim_i.cur_frame_idx == 0) {
                            cur_anim_i.cur_frame_idx =
                                cur_anim_i.cur_anim->frames.size() - 1;
                        } else {
                            cur_anim_i.cur_frame_idx--;
                        }
                        cur_anim_i.cur_frame_time = 0.0f;
                    }
                }
                set_tooltip(
                    "Previous frame."
                );
                
                //Next frame button.
                ImGui::SameLine();
                if(
                    ImGui::ImageButton(
                        "nextFrameButton",
                        editor_icons[EDITOR_ICON_NEXT],
                        ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                    )
                ) {
                    anim_playing = false;
                    if(!cur_anim_i.cur_anim->frames.empty()) {
                        if(
                            cur_anim_i.cur_frame_idx ==
                            cur_anim_i.cur_anim->frames.size() - 1 ||
                            cur_anim_i.cur_frame_idx == INVALID
                        ) {
                            cur_anim_i.cur_frame_idx = 0;
                        } else {
                            cur_anim_i.cur_frame_idx++;
                        }
                        cur_anim_i.cur_frame_time = 0.0f;
                    }
                }
                set_tooltip(
                    "Next frame."
                );
                
                ImGui::SameLine();
            }
            
            //Add frame button.
            if(
                ImGui::ImageButton(
                    "addFrameButton",
                    editor_icons[EDITOR_ICON_ADD],
                    ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                )
            ) {
                if(
                    cur_anim_i.cur_frame_idx <
                    cur_anim_i.cur_anim->loop_frame
                ) {
                    //Let the loop frame stay the same.
                    cur_anim_i.cur_anim->loop_frame++;
                }
                anim_playing = false;
                if(cur_anim_i.cur_frame_idx != INVALID) {
                    cur_anim_i.cur_frame_idx++;
                    cur_anim_i.cur_frame_time = 0.0f;
                    cur_anim_i.cur_anim->frames.insert(
                        cur_anim_i.cur_anim->frames.begin() +
                        cur_anim_i.cur_frame_idx,
                        frame(
                            cur_anim_i.cur_anim->frames[
                                cur_anim_i.cur_frame_idx - 1
                            ]
                        )
                    );
                } else {
                    cur_anim_i.cur_anim->frames.push_back(frame());
                    cur_anim_i.cur_frame_idx = 0;
                    cur_anim_i.cur_frame_time = 0.0f;
                    set_best_frame_sprite();
                }
                frame_ptr =
                    &(cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx]);
                changes_mgr.mark_as_changed();
                set_status(
                    "Added frame #" + i2s(cur_anim_i.cur_frame_idx + 1) + "."
                );
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
                        "delFrameButton",
                        editor_icons[EDITOR_ICON_REMOVE],
                        ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                    )
                ) {
                    size_t deleted_frame_idx = cur_anim_i.cur_frame_idx;
                    if(cur_anim_i.cur_frame_idx != INVALID) {
                        cur_anim_i.cur_anim->delete_frame(
                            cur_anim_i.cur_frame_idx
                        );
                    }
                    if(cur_anim_i.cur_anim->frames.empty()) {
                        cur_anim_i.cur_frame_idx = INVALID;
                        frame_ptr = nullptr;
                    } else if(
                        cur_anim_i.cur_frame_idx >=
                        cur_anim_i.cur_anim->frames.size()
                    ) {
                        cur_anim_i.cur_frame_idx =
                            cur_anim_i.cur_anim->frames.size() - 1;
                        frame_ptr =
                            &(
                                cur_anim_i.cur_anim->frames[
                                    cur_anim_i.cur_frame_idx
                                ]
                            );
                    }
                    anim_playing = false;
                    cur_anim_i.cur_frame_time = 0.0f;
                    changes_mgr.mark_as_changed();
                    set_status(
                        "Deleted frame #" + i2s(deleted_frame_idx + 1) + "."
                    );
                }
                set_tooltip(
                    "Delete the current frame."
                );
                
            }
            
            if(frame_ptr) {
            
                //Sprite combobox.
                vector<string> sprite_names;
                for(size_t s = 0; s < db.sprites.size(); s++) {
                    sprite_names.push_back(db.sprites[s]->name);
                }
                if(
                    ImGui::Combo(
                        "Sprite", &frame_ptr->sprite_name, sprite_names, 15
                    )
                ) {
                    frame_ptr->sprite_idx =
                        db.find_sprite(frame_ptr->sprite_name);
                    frame_ptr->sprite_ptr =
                        db.sprites[frame_ptr->sprite_idx];
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "The sprite to use for this frame."
                );
                
                //Duration value.
                if(
                    ImGui::DragFloat(
                        "Duration", &frame_ptr->duration, 0.0005, 0.0f, FLT_MAX
                    )
                ) {
                    cur_anim_i.cur_frame_time = 0.0f;
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "How long this frame lasts for, in seconds.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Interpolate checkbox.
                if(
                    ImGui::Checkbox("Interpolate", &frame_ptr->interpolate)
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "If true, the transformation data (sprite translation,\n"
                    "sprite rotation, etc.) on this frame will smoothly\n"
                    "interpolate until it meets the transformation\n"
                    "data of the next frame.\n"
                    "This does not affect the bitmap or hitboxes."
                );
                
                //Signal checkbox.
                bool use_signal = (frame_ptr->signal != INVALID);
                if(ImGui::Checkbox("Signal", &use_signal)) {
                    if(use_signal) {
                        frame_ptr->signal = 0;
                    } else {
                        frame_ptr->signal = INVALID;
                    }
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "Whether a signal event should be sent to the script\n"
                    "when this frame starts."
                );
                
                //Signal value.
                if(use_signal) {
                    ImGui::SameLine();
                    int f_signal = (int) frame_ptr->signal;
                    if(
                        ImGui::DragInt("##signal", &f_signal, 0.1, 0, INT_MAX)
                    ) {
                        changes_mgr.mark_as_changed();
                        frame_ptr->signal = f_signal;
                    }
                    set_tooltip(
                        "Number of the signal.",
                        "", WIDGET_EXPLANATION_DRAG
                    );
                }
                
                if(loaded_mob_type) {
                
                    //Sound checkbox.
                    bool use_sound = (!frame_ptr->sound.empty());
                    if(ImGui::Checkbox("Sound", &use_sound)) {
                        if(use_sound) {
                            frame_ptr->sound = NONE_OPTION;
                        } else {
                            frame_ptr->sound.clear();
                        }
                        changes_mgr.mark_as_changed();
                        db.fill_sound_idx_caches(loaded_mob_type);
                    }
                    set_tooltip(
                        "Whether a sound should play when this frame starts."
                    );
                    
                    if(use_sound) {
                    
                        //Sound combobox.
                        ImGui::SameLine();
                        vector<string> sounds = { NONE_OPTION };
                        for(
                            size_t s = 0;
                            s < loaded_mob_type->sounds.size();
                            s++
                        ) {
                            sounds.push_back(loaded_mob_type->sounds[s].name);
                        }
                        if(
                            ImGui::Combo(
                                "##sound",
                                &frame_ptr->sound,
                                sounds,
                                15
                            )
                        ) {
                            db.fill_sound_idx_caches(loaded_mob_type);
                            changes_mgr.mark_as_changed();
                        }
                        set_tooltip(
                            "Name of the sound in the object's data."
                        );
                    }
                    
                }
                
                //Apply duration to all button.
                ImGui::Spacer();
                if(ImGui::Button("Apply duration to all frames")) {
                    float d =
                        cur_anim_i.cur_anim->frames[
                            cur_anim_i.cur_frame_idx
                        ].duration;
                    for(
                        size_t i = 0;
                        i < cur_anim_i.cur_anim->frames.size();
                        i++
                    ) {
                        cur_anim_i.cur_anim->frames[i].duration = d;
                    }
                    cur_anim_i.cur_frame_time = 0.0f;
                    changes_mgr.mark_as_changed();
                    set_status(
                        "Applied the duration " + f2s(d) + " to all frames."
                    );
                }
            }
            
            ImGui::TreePop();
        }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui body part control panel for this frame.
 */
void animation_editor::process_gui_panel_body_part() {
    ImGui::BeginChild("bodyPart");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("BODY PARTS");
    
    static string new_part_name;
    static int selected_part = 0;
    
    //Add body part button.
    if(
        ImGui::ImageButton(
            "addPartButton",
            editor_icons[EDITOR_ICON_ADD],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
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
            for(size_t b = 0; b < db.body_parts.size(); b++) {
                if(db.body_parts[b]->name == new_part_name) {
                    selected_part = (int) b;
                    already_exists = true;
                }
            }
            if(!already_exists) {
                selected_part = std::max(0, selected_part);
                db.body_parts.insert(
                    db.body_parts.begin() + selected_part +
                    (db.body_parts.empty() ? 0 : 1),
                    new body_part(new_part_name)
                );
                if(db.body_parts.size() == 1) {
                    selected_part = 0;
                } else {
                    selected_part++;
                }
                update_hitboxes();
                changes_mgr.mark_as_changed();
                set_status("Created body part \"" + new_part_name + "\".");
                new_part_name.clear();
            } else {
                set_status(
                    "A body part by the name \"" + new_part_name +
                    "\" already exists!",
                    true
                );
            }
        }
    }
    
    if(!db.body_parts.empty()) {
    
        //Delete body part button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "delPartButton",
                editor_icons[EDITOR_ICON_REMOVE],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            if(selected_part >= 0 && !db.body_parts.empty()) {
                string deleted_part_name =
                    db.body_parts[selected_part]->name;
                delete db.body_parts[selected_part];
                db.body_parts.erase(
                    db.body_parts.begin() + selected_part
                );
                if(db.body_parts.empty()) {
                    selected_part = -1;
                } else if(selected_part > 0) {
                    selected_part--;
                }
                update_hitboxes();
                changes_mgr.mark_as_changed();
                set_status(
                    "Deleted body part \"" + deleted_part_name + "\"."
                );
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
                "renamePartButton",
                editor_icons[EDITOR_ICON_INFO],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            duplicate_string(
                db.body_parts[selected_part]->name, rename_part_name
            );
            ImGui::OpenPopup("renamePart");
        }
        set_tooltip(
            "Rename the current body part."
        );
        
        //Rename body part popup.
        if(input_popup("renamePart", "New name:", &rename_part_name)) {
            rename_body_part(
                db.body_parts[selected_part], rename_part_name
            );
        }
        
        //Body part list.
        if(
            ImGui::BeginChild(
                "partsList", ImVec2(0.0f, 80.0f), ImGuiChildFlags_Border
            )
        ) {
        
            for(size_t p = 0; p < db.body_parts.size(); p++) {
            
                //Body part selectable.
                bool is_selected = (p == (size_t) selected_part);
                ImGui::Selectable(
                    db.body_parts[p]->name.c_str(), &is_selected
                );
                
                if(ImGui::IsItemActive()) {
                    selected_part = (int) p;
                    if(!ImGui::IsItemHovered()) {
                        int p2 =
                            (int) p +
                            (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
                        if(p2 >= 0 && p2 < (int) db.body_parts.size()) {
                            body_part* p_ptr = db.body_parts[p];
                            db.body_parts[p] = db.body_parts[p2];
                            db.body_parts[p2] = p_ptr;
                            ImGui::ResetMouseDragDelta();
                            update_hitboxes();
                            changes_mgr.mark_as_changed();
                        }
                    }
                }
                
            }
            
            ImGui::EndChild();
            
        }
        
    }
    
    if(db.body_parts.size() > 1) {
    
        //Explanation text.
        ImGui::Spacer();
        ImGui::TextWrapped(
            "The higher on the list, the more priority that body "
            "part's hitboxes have when the game checks collisions. "
            "Drag and drop items in the list to sort them."
        );
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui animation database info control panel
 * for this frame.
 */
void animation_editor::process_gui_panel_info() {
    ImGui::BeginChild("info");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("INFO");
    
    //Version input.
    string version = db.version;
    if(ImGui::InputText("Version", &version)) {
        db.version = version;
    }
    set_tooltip(
        "Version of the file, preferably in the \"X.Y.Z\" format. "
        "Optional."
    );
    
    //Maker input.
    string maker = db.maker;
    if(ImGui::InputText("Maker", &maker)) {
        db.maker = maker;
    }
    set_tooltip(
        "Name (or nickname) of who made this file. "
        "Optional."
    );
    
    //Maker notes input.
    string maker_notes = db.maker_notes;
    if(ImGui::InputText("Maker notes", &maker_notes)) {
        db.maker_notes = maker_notes;
    }
    set_tooltip(
        "Extra notes or comments about the file for other makers to see. "
        "Optional."
    );
    
    //Notes input.
    string notes = db.notes;
    if(ImGui::InputText("Notes", &notes)) {
        db.notes = notes;
    }
    set_tooltip(
        "Extra notes or comments of any kind. "
        "Optional."
    );
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui main control panel for this frame.
 */
void animation_editor::process_gui_panel_main() {
    if(manifest.internal_name.empty()) return;
    
    ImGui::BeginChild("main");
    
    //Current file text.
    ImGui::Text(
        "File: %s",
        loaded_mob_type ?
        loaded_mob_type->manifest->internal_name.c_str() :
        manifest.internal_name.c_str()
    );
    string file_tooltip =
        "Pack: " + game.content.packs.list[manifest.pack].name + "\n"
        "File path: " + manifest.path + "\n\n"
        "File state: ";
    if(!changes_mgr.exists_on_disk()) {
        file_tooltip += "Not saved to disk yet!";
    } else if(changes_mgr.has_unsaved_changes()) {
        file_tooltip += "You have unsaved changes.";
    } else {
        file_tooltip += "Everything ok.";
    }
    set_tooltip(file_tooltip);
    
    //Animations button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "animsButton",
            editor_icons[EDITOR_ICON_ANIMATIONS],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE),
            24.0f,
            "Animations"
        )
    ) {
        if(!cur_anim_i.cur_anim && !db.animations.empty()) {
            pick_animation(db.animations[0]->name, "", "", nullptr, false);
        }
        change_state(EDITOR_STATE_ANIMATION);
    }
    set_tooltip(
        "Change the way the animations look like."
    );
    
    //Sprites button.
    if(
        ImGui::ImageButtonAndText(
            "spritesButton",
            editor_icons[EDITOR_ICON_SPRITES],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE),
            24.0f,
            "Sprites"
        )
    ) {
        if(!cur_sprite && !db.sprites.empty()) {
            cur_sprite = db.sprites[0];
        }
        change_state(EDITOR_STATE_SPRITE);
    }
    set_tooltip(
        "Change how each individual sprite looks like."
    );
    
    //Body parts button.
    if(
        ImGui::ImageButtonAndText(
            "partsButton",
            editor_icons[EDITOR_ICON_BODY_PARTS],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE),
            24.0f,
            "Body parts"
        )
    ) {
        change_state(EDITOR_STATE_BODY_PART);
    }
    set_tooltip(
        "Change what body parts exist, and their order."
    );
    
    //Information button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "infoButton",
            editor_icons[EDITOR_ICON_INFO],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE),
            8.0f,
            "Info"
        )
    ) {
        change_state(EDITOR_STATE_INFO);
    }
    set_tooltip(
        "Set the animation database's information here, if you want."
    );
    
    //Tools button.
    if(
        ImGui::ImageButtonAndText(
            "toolsButton",
            editor_icons[EDITOR_ICON_TOOLS],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE),
            8.0f,
            "Tools"
        )
    ) {
        change_state(EDITOR_STATE_TOOLS);
    }
    set_tooltip(
        "Special tools to help with specific tasks."
    );
    
    //Stats node.
    ImGui::Spacer();
    if(saveable_tree_node("main", "Stats")) {
    
        //Animation amount text.
        ImGui::BulletText(
            "Animations: %i", (int) db.animations.size()
        );
        
        //Sprite amount text.
        ImGui::BulletText(
            "Sprites: %i", (int) db.sprites.size()
        );
        
        //Body part amount text.
        ImGui::BulletText(
            "Body parts: %i", (int) db.body_parts.size()
        );
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite control panel for this frame.
 */
void animation_editor::process_gui_panel_sprite() {
    ImGui::BeginChild("sprite");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("SPRITES");
    
    //Current sprite text.
    size_t cur_sprite_idx = INVALID;
    if(cur_sprite) {
        cur_sprite_idx = db.find_sprite(cur_sprite->name);
    }
    ImGui::Text(
        "Current sprite: %s / %i",
        (cur_sprite_idx == INVALID ? "--" : i2s(cur_sprite_idx + 1).c_str()),
        (int) db.sprites.size()
    );
    
    //Previous sprite button.
    if(
        ImGui::ImageButton(
            "prevSpriteButton",
            editor_icons[EDITOR_ICON_PREVIOUS],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.sprites.empty()) {
            if(!cur_sprite) {
                pick_sprite(db.sprites[0]->name, "", "", nullptr, false);
            } else {
                size_t new_idx =
                    sum_and_wrap(
                        (int) db.find_sprite(cur_sprite->name),
                        -1,
                        (int) db.sprites.size()
                    );
                pick_sprite(db.sprites[new_idx]->name, "", "", nullptr, false);
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
        -(EDITOR::ICON_BMP_SIZE + 16.0f), EDITOR::ICON_BMP_SIZE + 6.0f
    );
    ImGui::SameLine();
    if(ImGui::Button(sprite_button_name.c_str(), sprite_button_size)) {
        vector<picker_item> sprite_names;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            sprite_names.push_back(
                picker_item(
                    db.sprites[s]->name,
                    "", "",
                    db.sprites[s]->bitmap
                )
            );
        }
        open_picker_dialog(
            "Pick a sprite, or create a new one",
            sprite_names,
            std::bind(
                &animation_editor::pick_sprite, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5
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
            "nextSpriteButton",
            editor_icons[EDITOR_ICON_NEXT],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.sprites.empty()) {
            if(!cur_sprite) {
                pick_sprite(db.sprites[0]->name, "", "", nullptr, false);
            } else {
                size_t new_idx =
                    sum_and_wrap(
                        (int) db.find_sprite(cur_sprite->name),
                        1,
                        (int) db.sprites.size()
                    );
                pick_sprite(db.sprites[new_idx]->name, "", "", nullptr, false);
            }
        }
    }
    set_tooltip(
        "Next\nsprite."
    );
    
    ImGui::Spacer();
    
    if(cur_sprite) {
        //Delete sprite button.
        if(
            ImGui::ImageButton(
                "delSpriteButton",
                editor_icons[EDITOR_ICON_REMOVE],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            string deleted_sprite_name = cur_sprite->name;
            size_t nr = db.find_sprite(deleted_sprite_name);
            db.delete_sprite(nr);
            cur_anim_i.cur_frame_idx = 0;
            if(db.sprites.empty()) {
                cur_sprite = nullptr;
                cur_hitbox = nullptr;
                cur_hitbox_idx = INVALID;
            } else {
                nr = std::min(nr, db.sprites.size() - 1);
                pick_sprite(db.sprites[nr]->name, "", "", nullptr, false);
            }
            changes_mgr.mark_as_changed();
            set_status("Deleted sprite \"" + deleted_sprite_name + "\".");
        }
        set_tooltip(
            "Delete the current sprite.\n"
            "Any frame that makes use of this sprite\n"
            "will be deleted from its animation."
        );
    }
    
    if(cur_sprite) {
    
        if(db.sprites.size() > 1) {
        
            //Import sprite button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "importSpriteButton",
                    editor_icons[EDITOR_ICON_DUPLICATE],
                    ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                )
            ) {
                ImGui::OpenPopup("importSprite");
            }
            set_tooltip(
                "Import the data from another sprite."
            );
            
            //Import sprite popup.
            vector<string> import_sprite_names;
            for(size_t s = 0; s < db.sprites.size(); s++) {
                if(db.sprites[s] == cur_sprite) continue;
                import_sprite_names.push_back(db.sprites[s]->name);
            }
            string picked_sprite;
            if(
                list_popup("importSprite", import_sprite_names, &picked_sprite)
            ) {
                import_sprite_file_data(picked_sprite);
                import_sprite_transformation_data(picked_sprite);
                import_sprite_hitbox_data(picked_sprite);
                import_sprite_top_data(picked_sprite);
                set_status(
                    "Imported all sprite data from \"" + picked_sprite + "\"."
                );
            }
            
        }
        
        //Rename sprite button.
        static string rename_sprite_name;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "renameSpriteButton",
                editor_icons[EDITOR_ICON_INFO],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            duplicate_string(cur_sprite->name, rename_sprite_name);
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
                "resizeSpriteButton",
                editor_icons[EDITOR_ICON_RESIZE],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
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
        
        if(!db.body_parts.empty()) {
            //Sprite hitboxes button.
            if(ImGui::Button("Hitboxes", mode_buttons_size)) {
                if(cur_sprite && !cur_sprite->hitboxes.empty()) {
                    update_cur_hitbox();
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


/**
 * @brief Processes the Dear ImGui sprite bitmap control panel for this frame.
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
    panel_title("BITMAP");
    
    if(db.sprites.size() > 1) {
    
        //Import bitmap data button.
        if(
            ImGui::ImageButton(
                "importDataButton",
                editor_icons[EDITOR_ICON_DUPLICATE],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteBitmap");
        }
        set_tooltip(
            "Import the bitmap data from another sprite."
        );
        
        //Import bitmap popup.
        vector<string> import_sprite_names;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            if(db.sprites[s] == cur_sprite) continue;
            import_sprite_names.push_back(db.sprites[s]->name);
        }
        string picked_sprite;
        if(
            list_popup(
                "importSpriteBitmap", import_sprite_names, &picked_sprite
            )
        ) {
            import_sprite_file_data(picked_sprite);
            center_camera_on_sprite_bitmap(false);
            set_status(
                "Imported file data from \"" + picked_sprite + "\"."
            );
        }
        
    }
    
    //Browse for spritesheet button.
    ImGui::Spacer();
    if(ImGui::Button("...")) {
        FILE_DIALOG_RESULT result = FILE_DIALOG_RESULT_SUCCESS;
        vector<string> f =
            prompt_file_dialog_locked_to_folder(
                FOLDER_PATHS_FROM_ROOT::BASE_PACK + "/" + FOLDER_PATHS_FROM_PACK::GRAPHICS, //TODO
                "Please choose the bitmap to get the sprites from.",
                "*.png",
                ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                ALLEGRO_FILECHOOSER_PICTURES,
                &result, game.display
            );
            
        switch(result) {
        case FILE_DIALOG_RESULT_WRONG_FOLDER: {
            //File doesn't belong to the folder.
            set_status("The chosen image is not in the graphics folder!", true);
            break;
        } case FILE_DIALOG_RESULT_CANCELED: {
            //User canceled.
            break;
        } case FILE_DIALOG_RESULT_SUCCESS: {
            cur_sprite->set_bitmap(
                f[0], cur_sprite->file_pos, cur_sprite->file_size
            );
            last_spritesheet_used = f[0];
            center_camera_on_sprite_bitmap(true);
            changes_mgr.mark_as_changed();
            set_status("Picked an image successfully.");
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
        last_spritesheet_used = file_name;
        center_camera_on_sprite_bitmap(true);
        changes_mgr.mark_as_changed();
    }
    set_tooltip(
        "File name of the bitmap to use as a spritesheet, in the "
        "Graphics folder. Extension included. e.g. "
        "\"large_fly.png\""
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
        changes_mgr.mark_as_changed();
    }
    set_tooltip(
        "Top-left coordinates.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
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
        changes_mgr.mark_as_changed();
    }
    set_tooltip(
        "Width and height.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Canvas explanation text.
    ImGui::Spacer();
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
            changes_mgr.mark_as_changed();
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite hitboxes control panel for this frame.
 */
void animation_editor::process_gui_panel_sprite_hitboxes() {
    ImGui::BeginChild("spriteHitboxes");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panel_title("HITBOXES");
    
    //Hitbox name text.
    ImGui::Text(
        "Hitbox: %s",
        cur_hitbox ? cur_hitbox->body_part_name.c_str() : NONE_OPTION.c_str()
    );
    
    //Previous hitbox button.
    if(
        ImGui::ImageButton(
            "prevHitboxButton",
            editor_icons[EDITOR_ICON_PREVIOUS],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(cur_sprite->hitboxes.size()) {
            if(!cur_hitbox) {
                cur_hitbox = &cur_sprite->hitboxes[0];
                cur_hitbox_idx = 0;
            } else {
                cur_hitbox_idx =
                    sum_and_wrap(
                        (int) cur_hitbox_idx, -1,
                        (int) cur_sprite->hitboxes.size()
                    );
                cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_idx];
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
            "nextHitboxButton",
            editor_icons[EDITOR_ICON_NEXT],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(cur_sprite->hitboxes.size()) {
            if(cur_hitbox_idx == INVALID) {
                cur_hitbox = &cur_sprite->hitboxes[0];
                cur_hitbox_idx = 0;
            } else {
                cur_hitbox_idx =
                    sum_and_wrap(
                        (int) cur_hitbox_idx, 1,
                        (int) cur_sprite->hitboxes.size()
                    );
                cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_idx];
            }
        }
    }
    set_tooltip(
        "Select the next hitbox."
    );
    
    if(cur_hitbox && db.sprites.size() > 1) {
    
        //Import hitbox data button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "importDataButton",
                editor_icons[EDITOR_ICON_DUPLICATE],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteHitboxes");
        }
        set_tooltip(
            "Import the hitbox data from another sprite."
        );
        
        //Import sprite popup.
        vector<string> import_sprite_names;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            if(db.sprites[s] == cur_sprite) continue;
            import_sprite_names.push_back(db.sprites[s]->name);
        }
        string picked_sprite;
        if(
            list_popup(
                "importSpriteHitboxes", import_sprite_names, &picked_sprite
            )
        ) {
            import_sprite_hitbox_data(picked_sprite);
            set_status(
                "Imported hitbox data from \"" + picked_sprite + "\"."
            );
        }
        
    }
    
    //Side view checkbox.
    ImGui::Spacer();
    ImGui::Checkbox("Use side view", &side_view);
    set_tooltip(
        "Use a side view of the object, so you can adjust hitboxes "
        "horizontally."
    );
    
    if(cur_hitbox) {
        //Hitbox center value.
        if(ImGui::DragFloat2("Center", (float*) &cur_hitbox->pos, 0.05f)) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "X and Y coordinates of the center.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Hitbox radius value.
        if(
            ImGui::DragFloat(
                "Radius", &cur_hitbox->radius, 0.05f, 0.001f, FLT_MAX
            )
        ) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Radius of the hitbox.",
            "", WIDGET_EXPLANATION_DRAG
        );
        cur_hitbox->radius =
            std::max(ANIM_EDITOR::HITBOX_MIN_RADIUS, cur_hitbox->radius);
            
        //Hitbox Z value.
        if(ImGui::DragFloat("Z", &cur_hitbox->z, 0.1f)) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Altitude of the hitbox's bottom.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        if(
            ImGui::DragFloat("Height", &cur_hitbox->height, 0.1f, 0.0f, FLT_MAX)
        ) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Hitbox's height. 0 = spans infinitely vertically.",
            "", WIDGET_EXPLANATION_DRAG
        );
        cur_hitbox->height = std::max(0.0f, cur_hitbox->height);
        
        //Hitbox type text.
        ImGui::Spacer();
        ImGui::Text("Hitbox type:");
        
        //Normal hitbox radio button.
        int type_int = cur_hitbox->type;
        if(ImGui::RadioButton("Normal", &type_int, HITBOX_TYPE_NORMAL)) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Normal hitbox, one that can be damaged."
        );
        
        //Attack hitbox radio button.
        if(ImGui::RadioButton("Attack", &type_int, HITBOX_TYPE_ATTACK)) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Attack hitbox, one that damages opponents."
        );
        
        //Disabled hitbox radio button.
        if(ImGui::RadioButton("Disabled", &type_int, HITBOX_TYPE_DISABLED)) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Disabled hitbox, one that cannot be interacted with."
        );
        cur_hitbox->type = (HITBOX_TYPE) type_int;
        
        ImGui::Indent();
        
        switch(cur_hitbox->type) {
        case HITBOX_TYPE_NORMAL: {
    
            //Defense multiplier value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat("Defense multiplier", &cur_hitbox->value, 0.01)
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Opponent attacks will have their damage divided "
                "by this amount.\n"
                "0 = invulnerable.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Pikmin latch checkbox.
            if(
                ImGui::Checkbox(
                    "Pikmin can latch", &cur_hitbox->can_pikmin_latch
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Can the Pikmin latch on to this hitbox?"
            );
            
            ImGui::Spacer();
            
            //Hazards list.
            process_gui_hitbox_hazards();
            
            break;
        } case HITBOX_TYPE_ATTACK: {
    
            //Power value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat("Power", &cur_hitbox->value, 0.01)
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Attack power, in hit points.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Outward knockback checkbox.
            if(
                ImGui::Checkbox(
                    "Outward knockback", &cur_hitbox->knockback_outward
                )
            ) {
                changes_mgr.mark_as_changed();
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
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "Angle to knock away towards.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
            }
            
            //Knockback strength value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat(
                    "Knockback value", &cur_hitbox->knockback, 0.01
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "How strong the knockback is. 3 is a good value.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Wither chance value.
            int wither_chance_int = cur_hitbox->wither_chance;
            ImGui::SetNextItemWidth(128.0f);
            if(ImGui::SliderInt("Wither chance", &wither_chance_int, 0, 100)) {
                changes_mgr.mark_as_changed();
                cur_hitbox->wither_chance = wither_chance_int;
            }
            set_tooltip(
                "Chance of the attack lowering a Pikmin's maturity by one.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            ImGui::Spacer();
            
            //Hazards list.
            process_gui_hitbox_hazards();
            
            break;
            
        }  case HITBOX_TYPE_DISABLED: {
            break;
        }
        }
        
        ImGui::Unindent();
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite top control panel for this frame.
 */
void animation_editor::process_gui_panel_sprite_top() {
    ImGui::BeginChild("spriteTop");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panel_title("TOP");
    
    if(db.sprites.size() > 1) {
    
        //Import top data button.
        if(
            ImGui::ImageButton(
                "importDataButton",
                editor_icons[EDITOR_ICON_DUPLICATE],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteTop");
        }
        set_tooltip(
            "Import the top data from another sprite."
        );
        
        //Import sprite popup.
        vector<string> import_sprite_names;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            if(db.sprites[s] == cur_sprite) continue;
            import_sprite_names.push_back(db.sprites[s]->name);
        }
        string picked_sprite;
        if(
            list_popup(
                "importSpriteTop", import_sprite_names, &picked_sprite
            )
        ) {
            import_sprite_top_data(picked_sprite);
            set_status(
                "Imported Pikmin top data from \"" + picked_sprite + "\"."
            );
        }
        
    }
    
    //Visible checkbox.
    ImGui::Spacer();
    if(ImGui::Checkbox("Visible", &cur_sprite->top_visible)) {
        changes_mgr.mark_as_changed();
    }
    set_tooltip(
        "Is the top visible in this sprite?"
    );
    
    if(cur_sprite->top_visible) {
    
        //Top center value.
        if(
            ImGui::DragFloat2("Center", (float*) &cur_sprite->top_pos, 0.05f)
        ) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Center coordinates.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Top size value.
        if(
            process_gui_size_widgets(
                "Size", cur_sprite->top_size, 0.01f,
                top_keep_aspect_ratio, false, ANIM_EDITOR::TOP_MIN_SIZE
            )
        ) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Width and height.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
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
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Angle.",
            "", WIDGET_EXPLANATION_SLIDER
        );
        
        //Toggle maturity button.
        ImGui::Spacer();
        if(ImGui::Button("Toggle maturity")) {
            cur_maturity = sum_and_wrap(cur_maturity, 1, N_MATURITIES);
        }
        set_tooltip(
            "View a different maturity top."
        );
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite transform control panel for
 * this frame.
 */
void animation_editor::process_gui_panel_sprite_transform() {
    ImGui::BeginChild("spriteTransform");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panel_title("TRANSFORM");
    
    if(db.sprites.size() > 1) {
    
        //Import transformation data button.
        if(
            ImGui::ImageButton(
                "importDataButton",
                editor_icons[EDITOR_ICON_DUPLICATE],
                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteTransform");
        }
        set_tooltip(
            "Import the transformation data from another sprite."
        );
        
        //Import sprite popup.
        vector<string> import_sprite_names;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            if(db.sprites[s] == cur_sprite) continue;
            import_sprite_names.push_back(db.sprites[s]->name);
        }
        string picked_sprite;
        if(
            list_popup(
                "importSpriteTransform", import_sprite_names, &picked_sprite
            )
        ) {
            import_sprite_transformation_data(picked_sprite);
            set_status(
                "Imported transformation data from \"" + picked_sprite + "\"."
            );
        }
        
    }
    
    //Sprite offset value.
    ImGui::Spacer();
    if(
        ImGui::DragFloat2("Offset", (float*) &cur_sprite->offset, 0.05f)
    ) {
        changes_mgr.mark_as_changed();
    }
    set_tooltip(
        "X and Y offset.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Sprite scale value.
    if(
        process_gui_size_widgets(
            "Scale", cur_sprite->scale,
            0.005f, cur_sprite_keep_aspect_ratio, cur_sprite_keep_area,
            -FLT_MAX
        )
    ) {
        changes_mgr.mark_as_changed();
    }
    set_tooltip(
        "Horizontal and vertical scale.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Sprite flip X button.
    ImGui::Indent();
    if(
        ImGui::Button("Flip X")
    ) {
        cur_sprite->scale.x *= -1.0f;
        changes_mgr.mark_as_changed();
    }
    
    //Sprite flip Y button.
    ImGui::SameLine();
    if(
        ImGui::Button("Flip Y")
    ) {
        cur_sprite->scale.y *= -1.0f;
        changes_mgr.mark_as_changed();
    }
    
    //Keep aspect ratio checkbox.
    if(ImGui::Checkbox("Keep aspect ratio", &cur_sprite_keep_aspect_ratio)) {
        cur_sprite_keep_area = false;
    }
    set_tooltip("Keep the aspect ratio when resizing the sprite.");
    
    //Keep area checkbox.
    if(ImGui::Checkbox("Keep area", &cur_sprite_keep_area)) {
        cur_sprite_keep_aspect_ratio = false;
    };
    ImGui::Unindent();
    set_tooltip(
        "Keeps the same total area when resizing the sprite.\n"
        "Useful for squash and stretch effects."
    );
    
    //Sprite angle value.
    cur_sprite->angle = normalize_angle(cur_sprite->angle);
    if(
        ImGui::SliderAngle("Angle", &cur_sprite->angle, 0.0f, 360.0f, "%.2f")
    ) {
        changes_mgr.mark_as_changed();
    }
    set_tooltip(
        "Angle.",
        "", WIDGET_EXPLANATION_SLIDER
    );
    
    //Sprite tint color.
    if(
        ImGui::ColorEdit4(
            "Tint color", (float*) &cur_sprite->tint,
            ImGuiColorEditFlags_NoInputs
        )
    ) {
        changes_mgr.mark_as_changed();
    }
    set_tooltip("Color to tint it by. White makes it look normal.");
    
    ImGui::Spacer();
    
    if(db.sprites.size() > 1) {
    
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
                for(size_t s = 0; s < db.sprites.size(); s++) {
                    if(cur_sprite == db.sprites[s]) continue;
                    all_sprites.push_back(db.sprites[s]->name);
                }
                static string comparison_sprite_name;
                ImGui::Combo(
                    "Sprite", &comparison_sprite_name, all_sprites, 15
                );
                set_tooltip(
                    "Choose another sprite to serve as a comparison."
                );
                size_t comparison_sprite_idx =
                    db.find_sprite(comparison_sprite_name);
                if(comparison_sprite_idx != INVALID) {
                    comparison_sprite = db.sprites[comparison_sprite_idx];
                } else {
                    comparison_sprite = nullptr;
                }
                
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


/**
 * @brief Processes the Dear ImGui tools control panel for this frame.
 */
void animation_editor::process_gui_panel_tools() {
    ImGui::BeginChild("tools");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("TOOLS");
    
    //Resize everything value.
    static float resize_mult = 1.0f;
    ImGui::SetNextItemWidth(96.0f);
    ImGui::DragFloat("##resizeMult", &resize_mult, 0.01);
    set_tooltip(
        "Resize multiplier.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
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
    set_tooltip(
        "Scales value.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
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


/**
 * @brief Processes the Dear ImGui status bar for this frame.
 */
void animation_editor::process_gui_status_bar() {
    //Status bar text.
    process_gui_status_bar_text();
    
    //Spacer dummy widget.
    ImGui::SameLine();
    float size =
        canvas_separator_x - ImGui::GetItemRectSize().x -
        ANIM_EDITOR::MOUSE_COORDS_TEXT_WIDTH;
    ImGui::Dummy(ImVec2(size, 0));
    
    bool showing_coords = false;
    bool showing_time = false;
    float cur_time = 0.0f;
    
    //Mouse coordinates text.
    if(
        (!is_mouse_in_gui || is_m1_pressed) &&
        !is_cursor_in_timeline() && !anim_playing &&
        state != EDITOR_STATE_SPRITE_BITMAP &&
        (state != EDITOR_STATE_HITBOXES || !side_view)
    ) {
        showing_coords = true;
        ImGui::SameLine();
        ImGui::Text(
            "%s, %s",
            box_string(f2s(game.mouse_cursor.w_pos.x), 7).c_str(),
            box_string(f2s(game.mouse_cursor.w_pos.y), 7).c_str()
        );
    }
    
    if(
        !showing_coords &&
        state == EDITOR_STATE_ANIMATION &&
        cur_anim_i.valid_frame()
    ) {
        if(is_cursor_in_timeline()) {
            cur_time = get_cursor_timeline_time();
        } else {
            cur_time =
                cur_anim_i.cur_anim->get_time(
                    cur_anim_i.cur_frame_idx, cur_anim_i.cur_frame_time
                );
        }
        
        showing_time = true;
    }
    
    //Animation time text.
    if(showing_time) {
        ImGui::SameLine();
        ImGui::Text(
            "%ss",
            box_string(f2s(cur_time), 7).c_str()
        );
    }
    
    
}


/**
 * @brief Processes the Dear ImGui toolbar for this frame.
 */
void animation_editor::process_gui_toolbar() {
    if(manifest.internal_name.empty()) return;
    
    //Quit button.
    if(
        ImGui::ImageButton(
            "quitButton",
            editor_icons[EDITOR_ICON_QUIT],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        quit_widget_pos = get_last_widget_pos();
        quit_cmd(1.0f);
    }
    set_tooltip(
        "Quit the animation editor.",
        "Ctrl + Q"
    );
    
    //Load button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "loadButton",
            editor_icons[EDITOR_ICON_LOAD],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        load_widget_pos = get_last_widget_pos();
        load_cmd(1.0f);
    }
    set_tooltip(
        "Pick a file to load.",
        "Ctrl + L"
    );
    
    //Save button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "saveButton",
            changes_mgr.has_unsaved_changes() ?
            editor_icons[EDITOR_ICON_SAVE_UNSAVED] :
            editor_icons[EDITOR_ICON_SAVE],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        save_cmd(1.0f);
    }
    set_tooltip(
        "Save the animation database into the file on disk.",
        "Ctrl + S"
    );
    
    //Toggle grid button.
    ImGui::SameLine(0, 16);
    if(
        ImGui::ImageButton(
            "gridButton",
            editor_icons[EDITOR_ICON_GRID],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        grid_toggle_cmd(1.0f);
    }
    set_tooltip(
        "Toggle visibility of the grid.",
        "Ctrl + G"
    );
    
    //Toggle hitboxes button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "hitboxesButton",
            editor_icons[EDITOR_ICON_HITBOXES],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        hitboxes_toggle_cmd(1.0f);
    }
    set_tooltip(
        "Toggle visibility of the hitboxes, if any.",
        "Ctrl + H"
    );
    
    //Toggle mob radius button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "mobRadiusButton",
            editor_icons[EDITOR_ICON_MOB_RADIUS],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        mob_radius_toggle_cmd(1.0f);
    }
    set_tooltip(
        "Toggle visibility of the mob's radius, if applicable.",
        "Ctrl + R"
    );
    
    //Toggle leader silhouette button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "silhouetteButton",
            editor_icons[EDITOR_ICON_LEADER_SILHOUETTE],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        leader_silhouette_toggle_cmd(1.0f);
    }
    set_tooltip(
        "Toggle visibility of a leader silhouette.",
        "Ctrl + P"
    );
}
