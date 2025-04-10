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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../lib/imgui/imgui_stdlib.h"
#include "../../util/allegro_utils.h"
#include "../../util/imgui_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Opens the "load" dialog.
 */
void AnimationEditor::openLoadDialog() {
    reloadAnimDbs();
    
    //Set up the picker's behavior and data.
    vector<PickerItem> file_items;
    for(const auto &a : game.content.global_anim_dbs.list) {
        file_items.push_back(
            PickerItem(
                a.second.name,
                "Pack: " + game.content.packs.list[a.second.manifest->pack].name,
                "Global animations",
                (void*) a.second.manifest,
                getFileTooltip(a.second.manifest->path)
            )
        );
    }
    for(size_t c = 0; c < custom_cat_types.size(); c++) {
        for(size_t a = 0; a < custom_cat_types[c].size(); a++) {
            MobType* mt_ptr = custom_cat_types[c][a];
            if(!mt_ptr) continue;
            if(!mt_ptr->manifest) continue;
            auto &cat_anim_dbs =
                game.content.mob_anim_dbs.list[mt_ptr->category->id];
            auto mt_cat_anim_it =
                cat_anim_dbs.find(mt_ptr->manifest->internal_name);
            if(mt_cat_anim_it == cat_anim_dbs.end()) continue;
            
            ContentManifest* man_ptr =
                mt_cat_anim_it->second.manifest;
            file_items.push_back(
                PickerItem(
                    mt_ptr->name,
                    "Pack: " + game.content.packs.list[man_ptr->pack].name,
                    mt_ptr->custom_category_name + " objects",
                    (void*) man_ptr,
                    getFileTooltip(man_ptr->path)
                )
            );
        }
    }
    load_dialog_picker = Picker(this);
    load_dialog_picker.items = file_items;
    load_dialog_picker.pick_callback =
        std::bind(
            &AnimationEditor::pickAnimDbFile, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5
        );
        
    //Open the dialog that will contain the picker and history.
    openDialog(
        "Load an animation database or create a new one",
        std::bind(&AnimationEditor::processGuiLoadDialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&AnimationEditor::closeLoadDialog, this);
}


/**
 * @brief Opens the "new" dialog.
 */
void AnimationEditor::openNewDialog() {
    openDialog(
        "Create a new animation database",
        std::bind(&AnimationEditor::processGuiNewDialog, this)
    );
    dialogs.back()->custom_size = Point(400, 0);
    dialogs.back()->close_callback = [this] () {
        new_dialog.pack.clear();
        new_dialog.type = 0;
        new_dialog.custom_mob_cat.clear();
        new_dialog.mob_type_ptr = nullptr;
        new_dialog.internal_name = "my_animation";
        new_dialog.last_checked_anim_path.clear();
        new_dialog.anim_path.clear();
        new_dialog.anim_path_exists = false;
    };
}


/**
 * @brief Opens the options dialog.
 *
 */
void AnimationEditor::openOptionsDialog() {
    openDialog(
        "Options",
        std::bind(&AnimationEditor::processGuiOptionsDialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&AnimationEditor::closeOptionsDialog, this);
}


/**
 * @brief Processes Dear ImGui for this frame.
 */
void AnimationEditor::processGui() {
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
    processGuiMenuBar();
    
    //The two main columns that split the canvas (+ toolbar + status bar)
    //and control panel.
    ImGui::Columns(2, "colMain");
    
    //Do the toolbar.
    processGuiToolbar();
    
    //Draw the canvas now.
    ImGui::BeginChild("canvas", ImVec2(0, -EDITOR::STATUS_BAR_HEIGHT));
    ImGui::EndChild();
    is_mouse_in_gui =
        !ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    ImVec2 tl = ImGui::GetItemRectMin();
    canvas_tl.x = tl.x;
    canvas_tl.y = tl.y;
    ImVec2 br = ImGui::GetItemRectMax();
    canvas_br.x = br.x;
    canvas_br.y = br.y;
    ImGui::GetWindowDrawList()->AddCallback(drawCanvasDearImGuiCallback, nullptr);
    
    //Status bar.
    processGuiStatusBar();
    
    //Set up the separator for the control panel.
    ImGui::NextColumn();
    
    if(canvas_separator_x == -1) {
        canvas_separator_x = game.win_w * 0.675;
        ImGui::SetColumnWidth(0, canvas_separator_x);
    } else {
        canvas_separator_x = ImGui::GetColumnOffset(1);
    }
    
    //Do the control panel now.
    processGuiControlPanel();
    ImGui::NextColumn();
    
    //Finish the main window.
    ImGui::Columns(1);
    ImGui::End();
    
    //Process any dialogs.
    processDialogs();
}


/**
 * @brief Processes the Dear ImGui control panel for this frame.
 */
void AnimationEditor::processGuiControlPanel() {
    ImGui::BeginChild("panel");
    
    //Basically, just show the correct panel for the current state.
    switch(state) {
    case EDITOR_STATE_MAIN: {
        processGuiPanelMain();
        break;
    } case EDITOR_STATE_ANIMATION: {
        processGuiPanelAnimation();
        break;
    } case EDITOR_STATE_SPRITE: {
        processGuiPanelSprite();
        break;
    } case EDITOR_STATE_BODY_PART: {
        processGuiPanelBodyPart();
        break;
    } case EDITOR_STATE_HITBOXES: {
        processGuiPanelSpriteHitboxes();
        break;
    } case EDITOR_STATE_SPRITE_BITMAP: {
        processGuiPanelSpriteBitmap();
        break;
    } case EDITOR_STATE_SPRITE_TRANSFORM: {
        processGuiPanelSpriteTransform();
        break;
    } case EDITOR_STATE_TOP: {
        processGuiPanelSpriteTop();
        break;
    } case EDITOR_STATE_INFO: {
        processGuiPanelInfo();
        break;
    } case EDITOR_STATE_TOOLS: {
        processGuiPanelTools();
        break;
    }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui animation database deletion dialog
 * for this frame.
 */
void AnimationEditor::processGuiDeleteAnimDbDialog() {
    //Explanation text.
    string explanation_str;
    if(!changes_mgr.existsOnDisk()) {
        explanation_str =
            "You have never saved this animation database to disk, so if you\n"
            "delete, you will only lose your unsaved progress.";
    } else {
        explanation_str =
            "If you delete, you will lose all unsaved progress, and the\n"
            "animation database's files on the disk will be gone FOREVER!";
    }
    ImGui::SetupCentering(ImGui::CalcTextSize(explanation_str.c_str()).x);
    ImGui::Text("%s", explanation_str.c_str());
    
    //Final warning text.
    string final_warning_str =
        "Are you sure you want to delete the current animation database?";
    ImGui::SetupCentering(ImGui::CalcTextSize(final_warning_str.c_str()).x);
    ImGui::TextColored(
        ImVec4(0.8, 0.6, 0.6, 1.0),
        "%s", final_warning_str.c_str()
    );
    
    //Cancel button.
    ImGui::Spacer();
    ImGui::SetupCentering(100 + 100 + 30);
    if(ImGui::Button("Cancel", ImVec2(100, 40))) {
        closeTopDialog();
    }
    
    //Delete button.
    ImGui::SameLine(0.0f, 30);
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImVec4(0.3, 0.1, 0.1, 1.0)
    );
    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered, ImVec4(0.5, 0.1, 0.1, 1.0)
    );
    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive, ImVec4(0.4, 0.1, 0.1, 1.0)
    );
    if(ImGui::Button("Delete", ImVec2(100, 40))) {
        closeTopDialog();
        deleteCurrentAnimDb();
    }
    ImGui::PopStyleColor(3);
}


/**
 * @brief Processes the list of the current hitbox's hazards,
 * as well as the widgets necessary to control it, for this frame.
 */
void AnimationEditor::processGuiHitboxHazards() {
    //Hitbox hazards node.
    if(saveableTreeNode("hitbox", "Hazards")) {
    
        static int selected_hazard_idx = 0;
        vector<string> hazard_inames =
            semicolonListToVector(cur_hitbox->hazards_str);
        if(
            processGuiHazardManagementWidgets(
                hazard_inames, selected_hazard_idx
            )
        ) {
            changes_mgr.markAsChanged();
            cur_hitbox->hazards_str = join(hazard_inames, ";");
        }
        setTooltip("List of hazards this hitbox has.");
        
        ImGui::TreePop();
    }
}


/**
 * @brief Processes the "load" dialog for this frame.
 */
void AnimationEditor::processGuiLoadDialog() {
    //History node.
    processGuiHistory(
        game.options.anim_editor.history,
    [this](const string &path) -> string {
        return path;
    },
    [this](const string &path) {
        closeTopDialog();
        loadAnimDbFile(path, true);
    },
    [this](const string &path) {
        return getFileTooltip(path);
    }
    );
    
    //New node.
    ImGui::Spacer();
    if(saveableTreeNode("load", "New")) {
        if(ImGui::Button("Create new...", ImVec2(168.0f, 32.0f))) {
            openNewDialog();
        }
        
        ImGui::TreePop();
    }
    setTooltip("Creates a new animation database.");
    
    //Load node.
    ImGui::Spacer();
    if(saveableTreeNode("load", "Load")) {
        load_dialog_picker.process();
        
        ImGui::TreePop();
    }
}


/**
 * @brief Processes the Dear ImGui menu bar for this frame.
 */
void AnimationEditor::processGuiMenuBar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load file item.
            if(ImGui::MenuItem("Load or create...", "Ctrl+L")) {
                load_widget_pos = getLastWidgetPost();
                loadCmd(1.0f);
            }
            setTooltip(
                "Pick a file to load.",
                "Ctrl + L"
            );
            
            //Reload current file item.
            if(ImGui::MenuItem("Reload current animation database")) {
                reload_widget_pos = getLastWidgetPost();
                reloadCmd(1.0f);
            }
            setTooltip(
                "Lose all changes and reload the current file from the disk."
            );
            
            //Save current file item.
            if(ImGui::MenuItem("Save current animation database", "Ctrl+S")) {
                saveCmd(1.0f);
            }
            setTooltip(
                "Save the animation database into the file on disk.",
                "Ctrl + S"
            );
            
            //Delete current animation database item.
            if(ImGui::MenuItem("Delete current animation database")) {
                deleteAnimDbCmd(1.0f);
            }
            setTooltip(
                "Delete the current animation database from the disk."
            );
            
            //Separator item.
            ImGui::Separator();
            
            //Options menu item.
            if(ImGui::MenuItem("Options...")) {
                openOptionsDialog();
            }
            setTooltip(
                "Open the options menu, so you can tweak your preferences."
            );
            
            //Quit editor item.
            if(ImGui::MenuItem("Quit", "Ctrl+Q")) {
                quit_widget_pos = getLastWidgetPost();
                quitCmd(1.0f);
            }
            setTooltip(
                "Quit the animation editor.",
                "Ctrl + Q"
            );
            
            ImGui::EndMenu();
            
        }
        
        //View menu.
        if(ImGui::BeginMenu("View")) {
        
            //Zoom in item.
            if(ImGui::MenuItem("Zoom in", "Plus")) {
                zoomInCmd(1.0f);
            }
            setTooltip(
                "Zooms the camera in a bit.",
                "Plus"
            );
            
            //Zoom out item.
            if(ImGui::MenuItem("Zoom out", "Minus")) {
                zoomOutCmd(1.0f);
            }
            setTooltip(
                "Zooms the camera out a bit.",
                "Minus"
            );
            
            //Zoom and position reset item.
            if(ImGui::MenuItem("Zoom/position reset", "0")) {
                zoomAndPosResetCmd(1.0f);
            }
            setTooltip(
                "Reset the zoom level, and if pressed again,\n"
                "reset the camera position.",
                "0"
            );
            
            //Zoom everything item.
            if(ImGui::MenuItem("Zoom onto everything", "Home")) {
                zoomEverythingCmd(1.0f);
            }
            setTooltip(
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
                    "Show tooltips", "", &game.options.editors.show_tooltips
                )
            ) {
                string state_str =
                    game.options.editors.show_tooltips ? "Enabled" : "Disabled";
                setStatus(state_str + " tooltips.");
                saveOptions();
            }
            setTooltip(
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
                openHelpDialog(help_str, "animation.html");
            }
            setTooltip(
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
void AnimationEditor::processGuiNewDialog() {
    string problem;
    bool hit_create_button = false;
    
    //Pack widgets.
    processGuiNewDialogPackWidgets(&new_dialog.pack);
    
    //Global animation radio.
    ImGui::Spacer();
    ImGui::RadioButton("Global animation", &new_dialog.type, 0);
    
    //Mob type animation radio.
    ImGui::SameLine();
    ImGui::RadioButton("Object type", &new_dialog.type, 1);
    
    ImGui::Spacer();
    
    if(new_dialog.type == 0) {
        //Internal name input.
        ImGui::FocusOnInputText(new_dialog.needs_text_focus);
        if(
            monoInputText(
                "Internal name", &new_dialog.internal_name,
                ImGuiInputTextFlags_EnterReturnsTrue
            )
        ) {
            hit_create_button = true;
        }
        setTooltip(
            "Internal name of the new animation database.\n"
            "Remember to keep it simple, type in lowercase, "
            "and use underscores!"
        );
        
        //Small spacer dummy widget.
        ImGui::Dummy(ImVec2(0, 19));
        
    } else {
        //Mob type widgets.
        processGuiMobTypeWidgets(
            &new_dialog.custom_mob_cat, &new_dialog.mob_type_ptr,
            new_dialog.pack
        );
        
    }
    
    //Check if everything's ok.
    if(new_dialog.type == 0) {
        ContentManifest temp_man;
        temp_man.internal_name = new_dialog.internal_name;
        temp_man.pack = new_dialog.pack;
        new_dialog.anim_path =
            game.content.global_anim_dbs.manifestToPath(temp_man);
    } else {
        ContentManifest temp_man;
        temp_man.internal_name = FILE_NAMES::MOB_TYPE_ANIMATION;
        temp_man.pack = new_dialog.pack;
        if(new_dialog.mob_type_ptr) {
            new_dialog.anim_path =
                game.content.mob_anim_dbs.manifestToPath(
                    temp_man,
                    new_dialog.mob_type_ptr->category->folder_name,
                    new_dialog.mob_type_ptr->manifest->internal_name
                );
        }
    }
    if(new_dialog.last_checked_anim_path != new_dialog.anim_path) {
        new_dialog.anim_path_exists = fileExists(new_dialog.anim_path);
        new_dialog.last_checked_anim_path = new_dialog.anim_path;
    }
    
    if(new_dialog.type == 0) {
        if(new_dialog.internal_name.empty()) {
            problem = "You have to type an internal name first!";
        } else if(!isInternalNameGood(new_dialog.internal_name)) {
            problem =
                "The internal name should only have lowercase letters,\n"
                "numbers, and underscores!";
        } else {
            if(new_dialog.anim_path_exists) {
                problem =
                    "There is already a global animation database\n"
                    "with that internal name in that pack!";
            }
        }
    } else {
        if(!new_dialog.mob_type_ptr) {
            problem = "You have to choose an object type first!";
        } else {
            if(new_dialog.anim_path_exists) {
                problem =
                    "There is already an animation database\n"
                    "file for that object type in that pack!";
            }
        }
    }
    
    //Create button.
    ImGui::Spacer();
    ImGui::SetupCentering(200);
    if(!problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create animation database", ImVec2(200, 40))) {
        hit_create_button = true;
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    setTooltip(problem.empty() ? "Create the animation database!" : problem);
    
    //Creation logic.
    if(hit_create_button) {
        if(!problem.empty()) return;
        auto really_create = [this] () {
            closeTopDialog();
            closeTopDialog(); //Close the load dialog.
            createAnimDb(new_dialog.anim_path);
        };
        
        if(
            new_dialog.pack == FOLDER_NAMES::BASE_PACK &&
            !game.options.advanced.engine_dev
        ) {
            openBaseContentWarningDialog(really_create);
        } else {
            really_create();
        }
    }
}


/**
 * @brief Processes the options dialog for this frame.
 */
void AnimationEditor::processGuiOptionsDialog() {
    //Controls node.
    if(saveableTreeNode("options", "Controls")) {
    
        //Middle mouse button pans checkbox.
        ImGui::Checkbox("Use MMB to pan", &game.options.editors.mmb_pan);
        setTooltip(
            "Use the middle mouse button to pan the camera\n"
            "(and RMB to reset camera/zoom).\n"
            "Default: " +
            b2s(OPTIONS::EDITORS_D::MMB_PAN) + "."
        );
        
        //Drag threshold value.
        int drag_threshold = (int) game.options.editors.mouse_drag_threshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Drag threshold", &drag_threshold,
            0.1f, 0, INT_MAX
        );
        setTooltip(
            "Cursor must move these many pixels to be considered a drag.\n"
            "Default: " + i2s(OPTIONS::EDITORS_D::MOUSE_DRAG_THRESHOLD) +
            ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.editors.mouse_drag_threshold = drag_threshold;
        
        ImGui::TreePop();
        
    }
    
    ImGui::Spacer();
    
    processGuiEditorStyle();
    
    ImGui::Spacer();
    
    //Misc. node.
    if(saveableTreeNode("options", "Misc.")) {
    
        //Background texture checkbox.
        if(ImGui::Checkbox("Use background texture", &use_bg)) {
            if(!use_bg) {
                if(bg) {
                    al_destroy_bitmap(bg);
                    bg = nullptr;
                }
                game.options.anim_editor.bg_path.clear();
            }
        }
        setTooltip(
            "Check this to use a repeating texture on the background\n"
            "of the editor."
        );
        
        if(use_bg) {
            ImGui::Indent();
            
            //Remove background texture button.
            unsigned char rem_bg_opacity =
                game.options.anim_editor.bg_path.empty() ? 50 : 255;
            if(
                ImGui::ImageButton(
                    "remBgButton", editor_icons[EDITOR_ICON_REMOVE],
                    Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                    COLOR_EMPTY, mapAlpha(rem_bg_opacity)
                )
            ) {
                game.options.anim_editor.bg_path.clear();
                if(bg) {
                    al_destroy_bitmap(bg);
                    bg = nullptr;
                }
            }
            setTooltip(
                "Remove the background image.\n"
                "This does not delete the file on your disk."
            );
            
            //Background texture browse button.
            ImGui::SameLine();
            if(ImGui::Button("Browse...")) {
                vector<string> f =
                    promptFileDialog(
                        FOLDER_PATHS_FROM_ROOT::BASE_PACK + "/" +
                        FOLDER_PATHS_FROM_PACK::TEXTURES,
                        "Please choose a background texture.",
                        "*.*", 0, game.display
                    );
                    
                if(!f.empty() && !f[0].empty()) {
                    game.options.anim_editor.bg_path = f[0];
                    if(bg) {
                        al_destroy_bitmap(bg);
                        bg = nullptr;
                    }
                    bg =
                        loadBmp(
                            game.options.anim_editor.bg_path,
                            nullptr, false, false, false
                        );
                }
            }
            setTooltip(
                "Browse for which texture file on your disk to use."
            );
            
            //Background texture name text.
            string file_name =
                getPathLastComponent(game.options.anim_editor.bg_path);
            ImGui::SameLine();
            monoText("%s", file_name.c_str());
            setTooltip("Full path:\n" + game.options.anim_editor.bg_path);
            
            ImGui::Unindent();
        }
        
        ImGui::TreePop();
        
    }
}


/**
 * @brief Processes the Dear ImGui animation control panel for this frame.
 */
void AnimationEditor::processGuiPanelAnimation() {
    ImGui::BeginChild("animation");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("ANIMATIONS");
    
    processGuiPanelAnimationHeader();
    
    if(cur_anim_i.cur_anim) {
    
        //Animation data node.
        if(saveableTreeNode("animation", "Animation data")) {
            processGuiPanelAnimationData();
            ImGui::TreePop();
        }
        
        //Frames node.
        ImGui::Spacer();
        if(saveableTreeNode("animation", "Frames")) {
            Frame* frame_ptr = nullptr;
            if(
                cur_anim_i.cur_frame_idx == INVALID &&
                !cur_anim_i.cur_anim->frames.empty()
            ) {
                cur_anim_i.cur_frame_idx = 0;
                cur_anim_i.cur_frame_time = 0.0f;
            }
            if(cur_anim_i.validFrame()) {
                frame_ptr =
                    &(cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx]);
            }
            
            processGuiPanelFrameHeader(frame_ptr);
            if(frame_ptr) {
                processGuiPanelFrame(frame_ptr);
            }
            
            ImGui::TreePop();
        }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui animation control panel's animation
 * data for this frame.
 */
void AnimationEditor::processGuiPanelAnimationData() {
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
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "The animation loops back to this frame when it "
        "reaches the last one.",
        "", WIDGET_EXPLANATION_DRAG
    );
    loop_frame =
        std::clamp(
            loop_frame, 1,
            cur_anim_i.cur_anim->frames.empty() ?
            1 :
            (int) cur_anim_i.cur_anim->frames.size()
        );
    cur_anim_i.cur_anim->loop_frame = loop_frame - 1;
    
    //Hit rate slider.
    int hit_rate = cur_anim_i.cur_anim->hit_rate;
    if(ImGui::SliderInt("Hit rate", &hit_rate, 0, 100)) {
        changes_mgr.markAsChanged();
        cur_anim_i.cur_anim->hit_rate = hit_rate;
    }
    setTooltip(
        "If this attack can knock back Pikmin, this indicates "
        "the chance that it will hit.\n"
        "0 means it will always miss, 50 means it will hit "
        "half the time, etc.",
        "", WIDGET_EXPLANATION_SLIDER
    );
    
    //Animation information text.
    ImGui::TextDisabled("(Animation info)");
    string anim_info_str =
        "Total duration: " + f2s(cur_anim_i.cur_anim->getDuration()) + "s";
    if(cur_anim_i.cur_anim->loop_frame != 0) {
        anim_info_str +=
            "\nLoop segment duration: " +
            f2s(cur_anim_i.cur_anim->getLoopDuration()) + "s";
    }
    setTooltip(anim_info_str);
}


/**
 * @brief Processes the Dear ImGui animation control panel's animation
 * header for this frame.
 */
void AnimationEditor::processGuiPanelAnimationHeader() {
    //Current animation text.
    size_t cur_anim_idx = INVALID;
    if(cur_anim_i.cur_anim) {
        cur_anim_idx = db.findAnimation(cur_anim_i.cur_anim->name);
    }
    ImGui::Text(
        "Current animation: %s / %i",
        (cur_anim_idx == INVALID ? "--" : i2s(cur_anim_idx + 1).c_str()),
        (int) db.animations.size()
    );
    
    //Previous animation button.
    if(
        ImGui::ImageButton(
            "prevAnimButton", editor_icons[EDITOR_ICON_PREVIOUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.animations.empty()) {
            if(!cur_anim_i.cur_anim) {
                pickAnimation(db.animations[0]->name, "", "", nullptr, false);
            } else {
                size_t new_idx =
                    sumAndWrap(
                        (int) db.findAnimation(cur_anim_i.cur_anim->name),
                        -1,
                        (int) db.animations.size()
                    );
                pickAnimation(db.animations[new_idx]->name, "", "", nullptr, false);
            }
        }
    }
    setTooltip(
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
    if(monoButton(anim_button_name.c_str(), anim_button_size)) {
        vector<PickerItem> anim_names;
        for(size_t a = 0; a < db.animations.size(); a++) {
            ALLEGRO_BITMAP* anim_frame_1 = nullptr;
            if(!db.animations[a]->frames.empty()) {
                size_t s_pos =
                    db.findSprite(
                        db.animations[a]->frames[0].sprite_name
                    );
                if(s_pos != INVALID) {
                    anim_frame_1 = db.sprites[s_pos]->bitmap;
                }
            }
            anim_names.push_back(
                PickerItem(db.animations[a]->name, "", "", nullptr, "", anim_frame_1)
            );
        }
        openPickerDialog(
            "Pick an animation, or create a new one",
            anim_names,
            std::bind(
                &AnimationEditor::pickAnimation, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5
            ),
            "", true, true
        );
    }
    setTooltip(
        "Pick an animation, or create a new one."
    );
    
    //Next animation button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "nextAnimButton", editor_icons[EDITOR_ICON_NEXT],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.animations.empty()) {
            if(!cur_anim_i.cur_anim) {
                pickAnimation(db.animations[0]->name, "", "", nullptr, false);
            } else {
                size_t new_idx =
                    sumAndWrap(
                        (int) db.findAnimation(cur_anim_i.cur_anim->name),
                        1,
                        (int) db.animations.size()
                    );
                pickAnimation(db.animations[new_idx]->name, "", "", nullptr, false);
            }
        }
    }
    setTooltip(
        "Next\nanimation."
    );
    
    ImGui::Spacer();
    
    if(cur_anim_i.cur_anim) {
    
        //Delete animation button.
        if(
            ImGui::ImageButton(
                "delAnimButton", editor_icons[EDITOR_ICON_REMOVE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            string cur_anim_name = cur_anim_i.cur_anim->name;
            size_t nr = db.findAnimation(cur_anim_name);
            db.animations.erase(db.animations.begin() + nr);
            if(db.animations.empty()) {
                cur_anim_i.clear();
            } else {
                nr = std::min(nr, db.animations.size() - 1);
                pickAnimation(db.animations[nr]->name, "", "", nullptr, false);
            }
            anim_playing = false;
            changes_mgr.markAsChanged();
            setStatus("Deleted animation \"" + cur_anim_name + "\".");
        }
        setTooltip(
            "Delete the current animation."
        );
        
    }
    
    if(cur_anim_i.cur_anim) {
    
        if(db.animations.size() > 1) {
        
            //Import animation button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "importAnimButton", editor_icons[EDITOR_ICON_DUPLICATE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                ImGui::OpenPopup("importAnim");
            }
            setTooltip(
                "Import the data from another animation."
            );
            
            //Import animation popup.
            vector<string> import_anim_names;
            for(size_t a = 0; a < db.animations.size(); a++) {
                if(db.animations[a] == cur_anim_i.cur_anim) continue;
                import_anim_names.push_back(db.animations[a]->name);
            }
            string picked_anim;
            if(
                listPopup("importAnim", import_anim_names, &picked_anim, true)
            ) {
                importAnimationData(picked_anim);
                setStatus(
                    "Imported animation data from \"" + picked_anim + "\"."
                );
            }
            
        }
        
        //Rename animation button.
        static string rename_anim_name;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "renameAnimButton", editor_icons[EDITOR_ICON_INFO],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            duplicateString(cur_anim_i.cur_anim->name, rename_anim_name);
            openInputPopup("renameAnim");
        }
        setTooltip(
            "Rename the current animation."
        );
        
        //Rename animation popup.
        if(processGuiInputPopup("renameAnim", "New name:", &rename_anim_name, true)) {
            renameAnimation(cur_anim_i.cur_anim, rename_anim_name);
        }
    }
}


/**
 * @brief Processes the Dear ImGui body part control panel for this frame.
 */
void AnimationEditor::processGuiPanelBodyPart() {
    ImGui::BeginChild("bodyPart");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("BODY PARTS");
    
    static string new_part_name;
    static int selected_part = 0;
    
    //Add body part button.
    if(
        ImGui::ImageButton(
            "addPartButton", editor_icons[EDITOR_ICON_ADD],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        new_part_name.clear();
        openInputPopup("newPartName");
    }
    setTooltip(
        "Create a new body part."
        "It will be placed after the currently selected body part."
    );
    
    //Add body part popup.
    if(
        processGuiInputPopup(
            "newPartName", "New body part's name:", &new_part_name, true
        )
    ) {
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
                    new BodyPart(new_part_name)
                );
                if(db.body_parts.size() == 1) {
                    selected_part = 0;
                } else {
                    selected_part++;
                }
                updateHitboxes();
                changes_mgr.markAsChanged();
                setStatus("Created body part \"" + new_part_name + "\".");
                new_part_name.clear();
            } else {
                setStatus(
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
                "delPartButton", editor_icons[EDITOR_ICON_REMOVE],
                Point(EDITOR::ICON_BMP_SIZE)
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
                updateHitboxes();
                changes_mgr.markAsChanged();
                setStatus(
                    "Deleted body part \"" + deleted_part_name + "\"."
                );
            }
        }
        setTooltip(
            "Delete the currently selected body part from the list."
        );
        
        //Rename body part button.
        static string rename_part_name;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "renamePartButton", editor_icons[EDITOR_ICON_INFO],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            duplicateString(
                db.body_parts[selected_part]->name, rename_part_name
            );
            openInputPopup("renamePart");
        }
        setTooltip(
            "Rename the current body part."
        );
        
        //Rename body part popup.
        if(processGuiInputPopup("renamePart", "New name:", &rename_part_name, true)) {
            renameBodyPart(
                db.body_parts[selected_part], rename_part_name
            );
        }
        
        //Body part list.
        if(
            ImGui::BeginChild(
                "partsList", ImVec2(0.0f, 80.0f), ImGuiChildFlags_Borders
            )
        ) {
        
            for(size_t p = 0; p < db.body_parts.size(); p++) {
            
                //Body part selectable.
                bool is_selected = (p == (size_t) selected_part);
                monoSelectable(
                    db.body_parts[p]->name.c_str(), &is_selected
                );
                
                if(ImGui::IsItemActive()) {
                    selected_part = (int) p;
                    if(!ImGui::IsItemHovered()) {
                        int p2 =
                            (int) p +
                            (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
                        if(p2 >= 0 && p2 < (int) db.body_parts.size()) {
                            BodyPart* p_ptr = db.body_parts[p];
                            db.body_parts[p] = db.body_parts[p2];
                            db.body_parts[p2] = p_ptr;
                            ImGui::ResetMouseDragDelta();
                            updateHitboxes();
                            changes_mgr.markAsChanged();
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
 * @brief Processes the Dear ImGui animation control panel's frame info for this
 * frame.
 *
 * @param frame_ptr Pointer to the currently selected frame.
 */
void AnimationEditor::processGuiPanelFrame(Frame* &frame_ptr) {
    //Sprite combobox.
    vector<string> sprite_names;
    for(size_t s = 0; s < db.sprites.size(); s++) {
        sprite_names.push_back(db.sprites[s]->name);
    }
    if(
        monoCombo(
            "Sprite", &frame_ptr->sprite_name, sprite_names, 15
        )
    ) {
        frame_ptr->sprite_idx =
            db.findSprite(frame_ptr->sprite_name);
        frame_ptr->sprite_ptr =
            db.sprites[frame_ptr->sprite_idx];
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "The sprite to use for this frame."
    );
    
    //Duration value.
    if(
        ImGui::DragFloat(
            "Duration", &frame_ptr->duration, 0.0005, 0.0f, FLT_MAX
        )
    ) {
        cur_anim_i.cur_frame_time = 0.0f;
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "How long this frame lasts for, in seconds.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Interpolate checkbox.
    if(
        ImGui::Checkbox("Interpolate", &frame_ptr->interpolate)
    ) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
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
        changes_mgr.markAsChanged();
    }
    setTooltip(
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
            changes_mgr.markAsChanged();
            frame_ptr->signal = f_signal;
        }
        setTooltip(
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
            changes_mgr.markAsChanged();
            db.fillSoundIdxCaches(loaded_mob_type);
        }
        setTooltip(
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
                monoCombo(
                    "##sound",
                    &frame_ptr->sound,
                    sounds,
                    15
                )
            ) {
                db.fillSoundIdxCaches(loaded_mob_type);
                changes_mgr.markAsChanged();
            }
            setTooltip(
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
        changes_mgr.markAsChanged();
        setStatus(
            "Applied the duration " + f2s(d) + " to all frames."
        );
    }
}


/**
 * @brief Processes the Dear ImGui animation control panel's frame
 * header for this frame.
 */
void AnimationEditor::processGuiPanelFrameHeader(
    Frame* &frame_ptr
) {
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
                "playButton", editor_icons[EDITOR_ICON_PLAY_PAUSE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            if(is_shift_pressed) {
                restartAnimCmd(1.0f);
            } else {
                playPauseAnimCmd(1.0f);
            }
        }
        if(ImGui::BeginPopupContextItem()) {
            //From the beginning selectable.
            if(ImGui::Selectable("From the beginning")) {
                restartAnimCmd(1.0f);
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
        setTooltip(
            "Play or pause the animation.\n"
            "Hold Shift to start from the beginning.\n"
            "Right click for more options.",
            "Spacebar"
        );
        
        //Previous frame button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "prevFrameButton", editor_icons[EDITOR_ICON_PREVIOUS],
                Point(EDITOR::ICON_BMP_SIZE)
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
        setTooltip(
            "Previous frame."
        );
        
        //Next frame button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "nextFrameButton", editor_icons[EDITOR_ICON_NEXT],
                Point(EDITOR::ICON_BMP_SIZE)
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
        setTooltip(
            "Next frame."
        );
        
        ImGui::SameLine();
    }
    
    //Add frame button.
    if(
        ImGui::ImageButton(
            "addFrameButton", editor_icons[EDITOR_ICON_ADD],
            Point(EDITOR::ICON_BMP_SIZE)
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
                Frame(
                    cur_anim_i.cur_anim->frames[
                        cur_anim_i.cur_frame_idx - 1
                    ]
                )
            );
        } else {
            cur_anim_i.cur_anim->frames.push_back(Frame());
            cur_anim_i.cur_frame_idx = 0;
            cur_anim_i.cur_frame_time = 0.0f;
            setBestFrameSprite();
        }
        frame_ptr =
            &(cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx]);
        changes_mgr.markAsChanged();
        setStatus(
            "Added frame #" + i2s(cur_anim_i.cur_frame_idx + 1) + "."
        );
    }
    setTooltip(
        "Add a new frame after the curret one, by copying "
        "data from the current one."
    );
    
    if(frame_ptr) {
    
        //Delete frame button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "delFrameButton", editor_icons[EDITOR_ICON_REMOVE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            size_t deleted_frame_idx = cur_anim_i.cur_frame_idx;
            if(cur_anim_i.cur_frame_idx != INVALID) {
                cur_anim_i.cur_anim->deleteFrame(
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
            changes_mgr.markAsChanged();
            setStatus(
                "Deleted frame #" + i2s(deleted_frame_idx + 1) + "."
            );
        }
        setTooltip(
            "Delete the current frame."
        );
        
    }
}


/**
 * @brief Processes the Dear ImGui animation database info control panel
 * for this frame.
 */
void AnimationEditor::processGuiPanelInfo() {
    ImGui::BeginChild("info");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("INFO");
    
    //Name input.
    if(ImGui::InputText("Name", &db.name)) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "Name of this animation. Optional."
    );
    
    //Description input.
    if(ImGui::InputText("Description", &db.description)) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "Description of this animation. Optional."
    );
    
    //Version input.
    if(monoInputText("Version", &db.version)) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "Version of the file, preferably in the \"X.Y.Z\" format. "
        "Optional."
    );
    
    //Maker input.
    if(ImGui::InputText("Maker", &db.maker)) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "Name (or nickname) of who made this file. "
        "Optional."
    );
    
    //Maker notes input.
    if(ImGui::InputText("Maker notes", &db.maker_notes)) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "Extra notes or comments about the file for other makers to see. "
        "Optional."
    );
    
    //Notes input.
    if(ImGui::InputText("Notes", &db.notes)) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "Extra notes or comments of any kind. "
        "Optional."
    );
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui main control panel for this frame.
 */
void AnimationEditor::processGuiPanelMain() {
    if(manifest.internal_name.empty()) return;
    
    ImGui::BeginChild("main");
    
    //Current file header text.
    ImGui::Text("File: ");
    
    //Current file text.
    ImGui::SameLine();
    monoText(
        "%s",
        loaded_mob_type ?
        loaded_mob_type->manifest->internal_name.c_str() :
        manifest.internal_name.c_str()
    );
    string file_tooltip =
        getFileTooltip(manifest.path) + "\n\n"
        "File state: ";
    if(!changes_mgr.existsOnDisk()) {
        file_tooltip += "Not saved to disk yet!";
    } else if(changes_mgr.hasUnsavedChanges()) {
        file_tooltip += "You have unsaved changes.";
    } else {
        file_tooltip += "Everything ok.";
    }
    setTooltip(file_tooltip);
    
    //Animations button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "animsButton", editor_icons[EDITOR_ICON_ANIMATIONS],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Animations"
        )
    ) {
        if(!cur_anim_i.cur_anim && !db.animations.empty()) {
            pickAnimation(db.animations[0]->name, "", "", nullptr, false);
        }
        changeState(EDITOR_STATE_ANIMATION);
    }
    setTooltip(
        "Change the way the animations look like."
    );
    
    //Sprites button.
    if(
        ImGui::ImageButtonAndText(
            "spritesButton", editor_icons[EDITOR_ICON_SPRITES],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Sprites"
        )
    ) {
        if(!cur_sprite && !db.sprites.empty()) {
            cur_sprite = db.sprites[0];
        }
        changeState(EDITOR_STATE_SPRITE);
    }
    setTooltip(
        "Change how each individual sprite looks like."
    );
    
    //Body parts button.
    if(
        ImGui::ImageButtonAndText(
            "partsButton", editor_icons[EDITOR_ICON_BODY_PARTS],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Body parts"
        )
    ) {
        changeState(EDITOR_STATE_BODY_PART);
    }
    setTooltip(
        "Change what body parts exist, and their order."
    );
    
    //Information button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "infoButton", editor_icons[EDITOR_ICON_INFO],
            Point(EDITOR::ICON_BMP_SIZE),
            8.0f, "Info"
        )
    ) {
        changeState(EDITOR_STATE_INFO);
    }
    setTooltip(
        "Set the animation database's information here, if you want."
    );
    
    //Tools button.
    if(
        ImGui::ImageButtonAndText(
            "toolsButton", editor_icons[EDITOR_ICON_TOOLS],
            Point(EDITOR::ICON_BMP_SIZE),
            8.0f, "Tools"
        )
    ) {
        changeState(EDITOR_STATE_TOOLS);
    }
    setTooltip(
        "Special tools to help with specific tasks."
    );
    
    //Stats node.
    ImGui::Spacer();
    if(saveableTreeNode("main", "Stats")) {
    
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
void AnimationEditor::processGuiPanelSprite() {
    ImGui::BeginChild("sprite");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("SPRITES");
    
    //Current sprite text.
    size_t cur_sprite_idx = INVALID;
    if(cur_sprite) {
        cur_sprite_idx = db.findSprite(cur_sprite->name);
    }
    ImGui::Text(
        "Current sprite: %s / %i",
        (cur_sprite_idx == INVALID ? "--" : i2s(cur_sprite_idx + 1).c_str()),
        (int) db.sprites.size()
    );
    
    //Previous sprite button.
    if(
        ImGui::ImageButton(
            "prevSpriteButton", editor_icons[EDITOR_ICON_PREVIOUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.sprites.empty()) {
            if(!cur_sprite) {
                pickSprite(db.sprites[0]->name, "", "", nullptr, false);
            } else {
                size_t new_idx =
                    sumAndWrap(
                        (int) db.findSprite(cur_sprite->name),
                        -1,
                        (int) db.sprites.size()
                    );
                pickSprite(db.sprites[new_idx]->name, "", "", nullptr, false);
            }
        }
    }
    setTooltip(
        "Previous\nsprite."
    );
    
    //Change current sprite button.
    string sprite_button_name =
        (cur_sprite ? cur_sprite->name : NONE_OPTION) + "##sprite";
    ImVec2 sprite_button_size(
        -(EDITOR::ICON_BMP_SIZE + 16.0f), EDITOR::ICON_BMP_SIZE + 6.0f
    );
    ImGui::SameLine();
    if(monoButton(sprite_button_name.c_str(), sprite_button_size)) {
        vector<PickerItem> sprite_names;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            sprite_names.push_back(
                PickerItem(
                    db.sprites[s]->name,
                    "", "", nullptr, "",
                    db.sprites[s]->bitmap
                )
            );
        }
        openPickerDialog(
            "Pick a sprite, or create a new one",
            sprite_names,
            std::bind(
                &AnimationEditor::pickSprite, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5
            ),
            "", true, true
        );
    }
    setTooltip(
        "Pick a sprite, or create a new one."
    );
    
    //Next sprite button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "nextSpriteButton", editor_icons[EDITOR_ICON_NEXT],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.sprites.empty()) {
            if(!cur_sprite) {
                pickSprite(db.sprites[0]->name, "", "", nullptr, false);
            } else {
                size_t new_idx =
                    sumAndWrap(
                        (int) db.findSprite(cur_sprite->name),
                        1,
                        (int) db.sprites.size()
                    );
                pickSprite(db.sprites[new_idx]->name, "", "", nullptr, false);
            }
        }
    }
    setTooltip(
        "Next\nsprite."
    );
    
    ImGui::Spacer();
    
    if(cur_sprite) {
        //Delete sprite button.
        if(
            ImGui::ImageButton(
                "delSpriteButton", editor_icons[EDITOR_ICON_REMOVE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            string deleted_sprite_name = cur_sprite->name;
            size_t nr = db.findSprite(deleted_sprite_name);
            db.deleteSprite(nr);
            cur_anim_i.cur_frame_idx = 0;
            if(db.sprites.empty()) {
                cur_sprite = nullptr;
                cur_hitbox = nullptr;
                cur_hitbox_idx = INVALID;
            } else {
                nr = std::min(nr, db.sprites.size() - 1);
                pickSprite(db.sprites[nr]->name, "", "", nullptr, false);
            }
            changes_mgr.markAsChanged();
            setStatus("Deleted sprite \"" + deleted_sprite_name + "\".");
        }
        setTooltip(
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
                    "importSpriteButton", editor_icons[EDITOR_ICON_DUPLICATE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                ImGui::OpenPopup("importSprite");
            }
            setTooltip(
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
                listPopup(
                    "importSprite", import_sprite_names, &picked_sprite, true
                )
            ) {
                importSpriteBmpData(picked_sprite);
                importSpriteTransformationData(picked_sprite);
                importSpriteHitboxData(picked_sprite);
                importSpriteTopData(picked_sprite);
                setStatus(
                    "Imported all sprite data from \"" + picked_sprite + "\"."
                );
            }
            
        }
        
        //Rename sprite button.
        static string rename_sprite_name;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "renameSpriteButton", editor_icons[EDITOR_ICON_INFO],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            duplicateString(cur_sprite->name, rename_sprite_name);
            openInputPopup("renameSprite");
        }
        setTooltip(
            "Rename the current sprite."
        );
        
        //Rename sprite popup.
        if(processGuiInputPopup("renameSprite", "New name:", &rename_sprite_name, true)) {
            renameSprite(cur_sprite, rename_sprite_name);
        }
        
        //Resize sprite button.
        static string resize_sprite_mult;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "resizeSpriteButton", editor_icons[EDITOR_ICON_RESIZE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            resize_sprite_mult = "1.0";
            openInputPopup("resizeSprite");
        }
        setTooltip(
            "Resize the current sprite."
        );
        
        //Resize sprite popup.
        if(processGuiInputPopup("resizeSprite", "Resize by:", &resize_sprite_mult)) {
            resizeSprite(cur_sprite, s2f(resize_sprite_mult));
        }
        
        ImVec2 mode_buttons_size(-1.0f, 24.0f);
        
        //Sprite bitmap button.
        if(ImGui::Button("Bitmap", mode_buttons_size)) {
            pre_sprite_bmp_cam_pos = game.cam.target_pos;
            pre_sprite_bmp_cam_zoom = game.cam.target_zoom;
            centerCameraOnSpriteBitmap(true);
            changeState(EDITOR_STATE_SPRITE_BITMAP);
        }
        setTooltip(
            "Pick what part of an image makes up this sprite."
        );
        
        if(cur_sprite->bitmap) {
            //Sprite transformation button.
            if(ImGui::Button("Transformation", mode_buttons_size)) {
                changeState(EDITOR_STATE_SPRITE_TRANSFORM);
            }
            setTooltip(
                "Offset, scale, or rotate the sprite's image."
            );
        }
        
        if(!db.body_parts.empty()) {
            //Sprite hitboxes button.
            if(ImGui::Button("Hitboxes", mode_buttons_size)) {
                if(cur_sprite && !cur_sprite->hitboxes.empty()) {
                    updateCurHitbox();
                    changeState(EDITOR_STATE_HITBOXES);
                }
            }
            setTooltip(
                "Edit this sprite's hitboxes."
            );
        }
        
        if(
            loaded_mob_type &&
            loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
        ) {
        
            //Sprite Pikmin top button.
            if(ImGui::Button("Pikmin top", mode_buttons_size)) {
                changeState(EDITOR_STATE_TOP);
            }
            setTooltip(
                "Edit the Pikmin's top (maturity) for this sprite."
            );
            
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite bitmap control panel for this frame.
 */
void AnimationEditor::processGuiPanelSpriteBitmap() {
    ImGui::BeginChild("spriteBitmap");
    
    //Back button.
    if(ImGui::Button("Back")) {
        game.cam.setPos(pre_sprite_bmp_cam_pos);
        game.cam.set_zoom(pre_sprite_bmp_cam_zoom);
        changeState(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panelTitle("BITMAP");
    
    if(db.sprites.size() > 1) {
    
        //Import bitmap data button.
        if(
            ImGui::ImageButton(
                "importDataButton", editor_icons[EDITOR_ICON_DUPLICATE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteBitmap");
        }
        setTooltip(
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
            listPopup(
                "importSpriteBitmap", import_sprite_names, &picked_sprite, true
            )
        ) {
            importSpriteBmpData(picked_sprite);
            centerCameraOnSpriteBitmap(false);
            setStatus(
                "Imported file data from \"" + picked_sprite + "\"."
            );
        }
        
    }
    
    //Choose spritesheet image button.
    ImGui::Spacer();
    if(ImGui::Button("Choose image...")) {
        openBitmapDialog(
        [this] (const string &bmp) {
            cur_sprite->setBitmap(
                bmp, cur_sprite->bmp_pos, cur_sprite->bmp_size
            );
            last_spritesheet_used = bmp;
            centerCameraOnSpriteBitmap(true);
            changes_mgr.markAsChanged();
            setStatus("Picked a spritesheet image successfully.");
        },
        "."
        );
    }
    setTooltip("Choose which spritesheet to use from the game's content.");
    
    //Spritesheet image name text.
    ImGui::SameLine();
    monoText("%s", cur_sprite->bmp_name.c_str());
    setTooltip("Internal name:\n" + cur_sprite->bmp_name);
    
    //Sprite top-left coordinates value.
    int top_left[2] =
    { (int) cur_sprite->bmp_pos.x, (int) cur_sprite->bmp_pos.y };
    if(
        ImGui::DragInt2(
            "Top-left", top_left, 0.05f, 0.0f, INT_MAX
        )
    ) {
        cur_sprite->setBitmap(
            cur_sprite->bmp_name,
            Point(top_left[0], top_left[1]), cur_sprite->bmp_size
        );
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "Top-left coordinates.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Sprite size value.
    int size[2] =
    { (int) cur_sprite->bmp_size.x, (int) cur_sprite->bmp_size.y };
    if(
        ImGui::DragInt2(
            "Size", size, 0.05f, 0.0f, INT_MAX
        )
    ) {
        cur_sprite->setBitmap(
            cur_sprite->bmp_name,
            cur_sprite->bmp_pos, Point(size[0], size[1])
        );
        changes_mgr.markAsChanged();
    }
    setTooltip(
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
    setTooltip(
        "Add to the existing selection instead of replacing it."
    );
    
    if(
        cur_sprite->bmp_pos.x != 0.0f ||
        cur_sprite->bmp_pos.y != 0.0f ||
        cur_sprite->bmp_size.x != 0.0f ||
        cur_sprite->bmp_size.y != 0.0f
    ) {
    
        //Clear selection button.
        if(
            ImGui::Button("Clear selection")
        ) {
            cur_sprite->bmp_pos = Point();
            cur_sprite->bmp_size = Point();
            cur_sprite->setBitmap(
                cur_sprite->bmp_name, cur_sprite->bmp_pos, cur_sprite->bmp_size
            );
            changes_mgr.markAsChanged();
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite hitboxes control panel for this frame.
 */
void AnimationEditor::processGuiPanelSpriteHitboxes() {
    ImGui::BeginChild("spriteHitboxes");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panelTitle("HITBOXES");
    
    //Hitbox name header text.
    ImGui::Text("Hitbox: ");
    
    //Hitbox name text.
    ImGui::SameLine();
    monoText(
        "%s",
        cur_hitbox ? cur_hitbox->body_part_name.c_str() : NONE_OPTION.c_str()
    );
    
    //Previous hitbox button.
    if(
        ImGui::ImageButton(
            "prevHitboxButton", editor_icons[EDITOR_ICON_PREVIOUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(cur_sprite->hitboxes.size()) {
            if(!cur_hitbox) {
                cur_hitbox = &cur_sprite->hitboxes[0];
                cur_hitbox_idx = 0;
            } else {
                cur_hitbox_idx =
                    sumAndWrap(
                        (int) cur_hitbox_idx, -1,
                        (int) cur_sprite->hitboxes.size()
                    );
                cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_idx];
            }
        }
    }
    setTooltip(
        "Select the previous hitbox."
    );
    
    //Next hitbox button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "nextHitboxButton", editor_icons[EDITOR_ICON_NEXT],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(cur_sprite->hitboxes.size()) {
            if(cur_hitbox_idx == INVALID) {
                cur_hitbox = &cur_sprite->hitboxes[0];
                cur_hitbox_idx = 0;
            } else {
                cur_hitbox_idx =
                    sumAndWrap(
                        (int) cur_hitbox_idx, 1,
                        (int) cur_sprite->hitboxes.size()
                    );
                cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_idx];
            }
        }
    }
    setTooltip(
        "Select the next hitbox."
    );
    
    if(cur_hitbox && db.sprites.size() > 1) {
    
        //Import hitbox data button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "importDataButton", editor_icons[EDITOR_ICON_DUPLICATE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteHitboxes");
        }
        setTooltip(
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
            listPopup(
                "importSpriteHitboxes", import_sprite_names,
                &picked_sprite, true
            )
        ) {
            importSpriteHitboxData(picked_sprite);
            setStatus(
                "Imported hitbox data from \"" + picked_sprite + "\"."
            );
        }
        
    }
    
    //Side view checkbox.
    ImGui::Spacer();
    ImGui::Checkbox("Use side view", &side_view);
    setTooltip(
        "Use a side view of the object, so you can adjust hitboxes "
        "horizontally."
    );
    
    if(cur_hitbox) {
        //Hitbox center value.
        if(ImGui::DragFloat2("Center", (float*) &cur_hitbox->pos, 0.05f)) {
            changes_mgr.markAsChanged();
        }
        setTooltip(
            "X and Y coordinates of the center.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Hitbox radius value.
        if(
            ImGui::DragFloat(
                "Radius", &cur_hitbox->radius, 0.05f, 0.001f, FLT_MAX
            )
        ) {
            changes_mgr.markAsChanged();
        }
        setTooltip(
            "Radius of the hitbox.",
            "", WIDGET_EXPLANATION_DRAG
        );
        cur_hitbox->radius =
            std::max(ANIM_EDITOR::HITBOX_MIN_RADIUS, cur_hitbox->radius);
            
        //Hitbox Z value.
        if(ImGui::DragFloat("Z", &cur_hitbox->z, 0.1f)) {
            changes_mgr.markAsChanged();
        }
        setTooltip(
            "Altitude of the hitbox's bottom.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        if(
            ImGui::DragFloat("Height", &cur_hitbox->height, 0.1f, 0.0f, FLT_MAX)
        ) {
            changes_mgr.markAsChanged();
        }
        setTooltip(
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
            changes_mgr.markAsChanged();
        }
        setTooltip(
            "Normal hitbox, one that can be damaged."
        );
        
        //Attack hitbox radio button.
        if(ImGui::RadioButton("Attack", &type_int, HITBOX_TYPE_ATTACK)) {
            changes_mgr.markAsChanged();
        }
        setTooltip(
            "Attack hitbox, one that damages opponents."
        );
        
        //Disabled hitbox radio button.
        if(ImGui::RadioButton("Disabled", &type_int, HITBOX_TYPE_DISABLED)) {
            changes_mgr.markAsChanged();
        }
        setTooltip(
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
                changes_mgr.markAsChanged();
            }
            setTooltip(
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
                changes_mgr.markAsChanged();
            }
            setTooltip(
                "Can the Pikmin latch on to this hitbox?"
            );
            
            ImGui::Spacer();
            
            //Hazards list.
            processGuiHitboxHazards();
            
            break;
        } case HITBOX_TYPE_ATTACK: {
    
            //Power value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat("Power", &cur_hitbox->value, 0.01)
            ) {
                changes_mgr.markAsChanged();
            }
            setTooltip(
                "Attack power, in hit points.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Outward knockback checkbox.
            if(
                ImGui::Checkbox(
                    "Outward knockback", &cur_hitbox->knockback_outward
                )
            ) {
                changes_mgr.markAsChanged();
            }
            setTooltip(
                "If true, opponents are knocked away from the hitbox's center."
            );
            
            //Knockback angle value.
            if(!cur_hitbox->knockback_outward) {
                cur_hitbox->knockback_angle =
                    normalizeAngle(cur_hitbox->knockback_angle);
                ImGui::SetNextItemWidth(128.0f);
                if(
                    ImGui::SliderAngle(
                        "Knockback angle", &cur_hitbox->knockback_angle,
                        0.0f, 360.0f, "%.2f"
                    )
                ) {
                    changes_mgr.markAsChanged();
                }
                setTooltip(
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
                changes_mgr.markAsChanged();
            }
            setTooltip(
                "How strong the knockback is. 3 is a good value.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Wither chance value.
            int wither_chance_int = cur_hitbox->wither_chance;
            ImGui::SetNextItemWidth(128.0f);
            if(ImGui::SliderInt("Wither chance", &wither_chance_int, 0, 100)) {
                changes_mgr.markAsChanged();
                cur_hitbox->wither_chance = wither_chance_int;
            }
            setTooltip(
                "Chance of the attack lowering a Pikmin's maturity by one.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            ImGui::Spacer();
            
            //Hazards list.
            processGuiHitboxHazards();
            
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
void AnimationEditor::processGuiPanelSpriteTop() {
    ImGui::BeginChild("spriteTop");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panelTitle("TOP");
    
    if(db.sprites.size() > 1) {
    
        //Import top data button.
        if(
            ImGui::ImageButton(
                "importDataButton", editor_icons[EDITOR_ICON_DUPLICATE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteTop");
        }
        setTooltip(
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
            listPopup(
                "importSpriteTop", import_sprite_names, &picked_sprite, true
            )
        ) {
            importSpriteTopData(picked_sprite);
            setStatus(
                "Imported Pikmin top data from \"" + picked_sprite + "\"."
            );
        }
        
    }
    
    //Visible checkbox.
    ImGui::Spacer();
    if(ImGui::Checkbox("Visible", &cur_sprite->top_visible)) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "Is the top visible in this sprite?"
    );
    
    if(cur_sprite->top_visible) {
    
        //Top center value.
        if(
            ImGui::DragFloat2("Center", (float*) &cur_sprite->top_pos, 0.05f)
        ) {
            changes_mgr.markAsChanged();
        }
        setTooltip(
            "Center coordinates.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Top size value.
        if(
            processGuiSizeWidgets(
                "Size", cur_sprite->top_size, 0.01f,
                top_keep_aspect_ratio, false, ANIM_EDITOR::TOP_MIN_SIZE
            )
        ) {
            changes_mgr.markAsChanged();
        }
        setTooltip(
            "Width and height.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Keep aspect ratio checkbox.
        ImGui::Indent();
        ImGui::Checkbox("Keep aspect ratio", &top_keep_aspect_ratio);
        ImGui::Unindent();
        setTooltip("Keep the aspect ratio when resizing the top.");
        
        
        //Top angle value.
        cur_sprite->top_angle = normalizeAngle(cur_sprite->top_angle);
        if(
            ImGui::SliderAngle(
                "Angle", &cur_sprite->top_angle, 0.0f, 360.0f, "%.2f"
            )
        ) {
            changes_mgr.markAsChanged();
        }
        setTooltip(
            "Angle.",
            "", WIDGET_EXPLANATION_SLIDER
        );
        
        //Toggle maturity button.
        ImGui::Spacer();
        if(ImGui::Button("Toggle maturity")) {
            cur_maturity = sumAndWrap(cur_maturity, 1, N_MATURITIES);
        }
        setTooltip(
            "View a different maturity top."
        );
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite transform control panel for
 * this frame.
 */
void AnimationEditor::processGuiPanelSpriteTransform() {
    ImGui::BeginChild("spriteTransform");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panelTitle("TRANSFORM");
    
    if(db.sprites.size() > 1) {
    
        //Import transformation data button.
        if(
            ImGui::ImageButton(
                "importDataButton", editor_icons[EDITOR_ICON_DUPLICATE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteTransform");
        }
        setTooltip(
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
            listPopup(
                "importSpriteTransform", import_sprite_names,
                &picked_sprite, true
            )
        ) {
            importSpriteTransformationData(picked_sprite);
            setStatus(
                "Imported transformation data from \"" + picked_sprite + "\"."
            );
        }
        
    }
    
    //Sprite offset value.
    ImGui::Spacer();
    if(
        ImGui::DragFloat2("Offset", (float*) &cur_sprite->offset, 0.05f)
    ) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "X and Y offset.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Sprite scale value.
    if(
        processGuiSizeWidgets(
            "Scale", cur_sprite->scale,
            0.005f, cur_sprite_keep_aspect_ratio, cur_sprite_keep_area,
            -FLT_MAX
        )
    ) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
        "Horizontal and vertical scale.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Sprite flip X button.
    ImGui::Indent();
    if(
        ImGui::Button("Flip X")
    ) {
        cur_sprite->scale.x *= -1.0f;
        changes_mgr.markAsChanged();
    }
    
    //Sprite flip Y button.
    ImGui::SameLine();
    if(
        ImGui::Button("Flip Y")
    ) {
        cur_sprite->scale.y *= -1.0f;
        changes_mgr.markAsChanged();
    }
    
    //Keep aspect ratio checkbox.
    if(ImGui::Checkbox("Keep aspect ratio", &cur_sprite_keep_aspect_ratio)) {
        cur_sprite_keep_area = false;
    }
    setTooltip("Keep the aspect ratio when resizing the sprite.");
    
    //Keep area checkbox.
    if(ImGui::Checkbox("Keep area", &cur_sprite_keep_area)) {
        cur_sprite_keep_aspect_ratio = false;
    };
    ImGui::Unindent();
    setTooltip(
        "Keeps the same total area when resizing the sprite.\n"
        "Useful for squash and stretch effects."
    );
    
    //Sprite angle value.
    cur_sprite->angle = normalizeAngle(cur_sprite->angle);
    if(
        ImGui::SliderAngle("Angle", &cur_sprite->angle, 0.0f, 360.0f, "%.2f")
    ) {
        changes_mgr.markAsChanged();
    }
    setTooltip(
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
        changes_mgr.markAsChanged();
    }
    setTooltip("Color to tint it by. White makes it look normal.");
    
    ImGui::Spacer();
    
    if(db.sprites.size() > 1) {
    
        //Comparison sprite node.
        if(saveableTreeNode("transformation", "Comparison sprite")) {
        
            //Use comparison checkbox.
            ImGui::Checkbox("Use comparison", &comparison);
            setTooltip(
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
                monoCombo(
                    "Sprite", &comparison_sprite_name, all_sprites, 15
                );
                setTooltip(
                    "Choose another sprite to serve as a comparison."
                );
                size_t comparison_sprite_idx =
                    db.findSprite(comparison_sprite_name);
                if(comparison_sprite_idx != INVALID) {
                    comparison_sprite = db.sprites[comparison_sprite_idx];
                } else {
                    comparison_sprite = nullptr;
                }
                
                //Comparison blinks checkbox.
                ImGui::Checkbox("Blink comparison", &comparison_blink);
                setTooltip(
                    "Blink the comparison in and out?"
                );
                
                //Comparison above checkbox.
                ImGui::Checkbox("Comparison above", &comparison_above);
                setTooltip(
                    "Should the comparison appear above or below the working "
                    "sprite?"
                );
                
                //Tint both checkbox.
                ImGui::Checkbox("Tint both", &comparison_tint);
                setTooltip(
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
void AnimationEditor::processGuiPanelTools() {
    ImGui::BeginChild("tools");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("TOOLS");
    
    //Resize everything value.
    static float resize_mult = 1.0f;
    ImGui::SetNextItemWidth(96.0f);
    ImGui::DragFloat("##resizeMult", &resize_mult, 0.01);
    setTooltip(
        "Resize multiplier.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Resize everything button.
    ImGui::SameLine();
    if(ImGui::Button("Resize everything")) {
        resizeEverything(resize_mult);
        resize_mult = 1.0f;
    }
    setTooltip(
        "Resize everything by the given multiplier.\n"
        "0.5 resizes everyting to half size, 2.0 to double, etc."
    );
    
    //Set sprite scales value.
    static float scales_value = 1.0f;
    ImGui::SetNextItemWidth(96.0f);
    ImGui::DragFloat("##scalesValue", &scales_value, 0.01);
    setTooltip(
        "Scales value.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Set sprite scales button.
    ImGui::SameLine();
    if(ImGui::Button("Set all scales")) {
        setAllSpriteScales(scales_value);
    }
    setTooltip(
        "Set the X/Y scales of all sprites to the given value."
    );
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui status bar for this frame.
 */
void AnimationEditor::processGuiStatusBar() {
    //Status bar text.
    processGuiStatusBarText();
    
    //Spacer dummy widget.
    ImGui::SameLine();
    float size =
        canvas_separator_x - ImGui::GetItemRectSize().x -
        EDITOR::MOUSE_COORDS_TEXT_WIDTH;
    ImGui::Dummy(ImVec2(size, 0));
    
    bool showing_coords = false;
    bool showing_time = false;
    float cur_time = 0.0f;
    
    //Mouse coordinates text.
    if(
        (!is_mouse_in_gui || is_m1_pressed) &&
        !isCursorInTimeline() && !anim_playing &&
        state != EDITOR_STATE_SPRITE_BITMAP &&
        (state != EDITOR_STATE_HITBOXES || !side_view)
    ) {
        showing_coords = true;
        ImGui::SameLine();
        monoText(
            "%s, %s",
            boxString(f2s(game.mouse_cursor.w_pos.x), 7).c_str(),
            boxString(f2s(game.mouse_cursor.w_pos.y), 7).c_str()
        );
    }
    
    if(
        !showing_coords &&
        state == EDITOR_STATE_ANIMATION &&
        cur_anim_i.validFrame()
    ) {
        if(isCursorInTimeline()) {
            cur_time = getCursorTimelineTime();
        } else {
            cur_time =
                cur_anim_i.cur_anim->getTime(
                    cur_anim_i.cur_frame_idx, cur_anim_i.cur_frame_time
                );
        }
        
        showing_time = true;
    }
    
    //Animation time text.
    if(showing_time) {
        ImGui::SameLine();
        monoText(
            "%ss",
            boxString(f2s(cur_time), 7).c_str()
        );
    }
    
    
}


/**
 * @brief Processes the Dear ImGui toolbar for this frame.
 */
void AnimationEditor::processGuiToolbar() {
    if(manifest.internal_name.empty()) return;
    
    //Quit button.
    if(
        ImGui::ImageButton(
            "quitButton", editor_icons[EDITOR_ICON_QUIT],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        quit_widget_pos = getLastWidgetPost();
        quitCmd(1.0f);
    }
    setTooltip(
        "Quit the animation editor.",
        "Ctrl + Q"
    );
    
    //Load button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "loadButton", editor_icons[EDITOR_ICON_LOAD],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        load_widget_pos = getLastWidgetPost();
        loadCmd(1.0f);
    }
    setTooltip(
        "Pick a file to load.",
        "Ctrl + L"
    );
    
    //Save button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "saveButton",
            changes_mgr.hasUnsavedChanges() ?
            editor_icons[EDITOR_ICON_SAVE_UNSAVED] :
            editor_icons[EDITOR_ICON_SAVE],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        saveCmd(1.0f);
    }
    setTooltip(
        "Save the animation database into the file on disk.",
        "Ctrl + S"
    );
    
    //Toggle grid button.
    ImGui::SameLine(0, 16);
    if(
        ImGui::ImageButton(
            "gridButton", editor_icons[EDITOR_ICON_GRID],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        gridToggleCmd(1.0f);
    }
    setTooltip(
        "Toggle visibility of the grid.",
        "Ctrl + G"
    );
    
    //Toggle hitboxes button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "hitboxesButton", editor_icons[EDITOR_ICON_HITBOXES],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        hitboxesToggleCmd(1.0f);
    }
    setTooltip(
        "Toggle visibility of the hitboxes, if any.",
        "Ctrl + H"
    );
    
    //Toggle mob radius button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "mobRadiusButton", editor_icons[EDITOR_ICON_MOB_RADIUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        mobRadiusToggleCmd(1.0f);
    }
    setTooltip(
        "Toggle visibility of the mob's radius, if applicable.",
        "Ctrl + R"
    );
    
    //Toggle leader silhouette button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "silhouetteButton", editor_icons[EDITOR_ICON_LEADER_SILHOUETTE],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        leaderSilhouetteToggleCmd(1.0f);
    }
    setTooltip(
        "Toggle visibility of a leader silhouette.",
        "Ctrl + P"
    );
}
