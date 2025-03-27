/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle editor Dear ImGui logic.
 */

#include "editor.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../lib/imgui/imgui_stdlib.h"
#include "../../util/allegro_utils.h"
#include "../../util/imgui_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Opens the "load" dialog.
 */
void ParticleEditor::open_load_dialog() {
    reload_part_gens();
    
    //Set up the picker's behavior and data.
    vector<PickerItem> file_items;
    for(const auto &g : game.content.particle_gen.list) {
        ContentManifest* man = g.second.manifest;
        file_items.push_back(
            PickerItem(
                g.second.name,
                "Pack: " + game.content.packs.list[man->pack].name, "",
                (void*) man,
                get_file_tooltip(man->path)
            )
        );
    }
    
    load_dialog_picker = Picker(this);
    load_dialog_picker.items = file_items;
    load_dialog_picker.pick_callback =
        std::bind(
            &ParticleEditor::pick_part_gen_file, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5
        );
        
    //Open the dialog that will contain the picker and history.
    open_dialog(
        "Load a particle generator file",
        std::bind(&ParticleEditor::process_gui_load_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&ParticleEditor::close_load_dialog, this);
}


/**
 * @brief Opens the "new" dialog.
 */
void ParticleEditor::open_new_dialog() {
    open_dialog(
        "Create a new particle generator",
        std::bind(&ParticleEditor::process_gui_new_dialog, this)
    );
    dialogs.back()->custom_size = Point(400, 0);
    dialogs.back()->close_callback = [this] () {
        new_dialog.pack.clear();
        new_dialog.internal_name = "my_particle_generator";
        new_dialog.part_gen_path.clear();
        new_dialog.last_checked_part_gen_path.clear();
        new_dialog.part_gen_path_exists = false;
    };
    
}


/**
 * @brief Opens the options dialog.
 */
void ParticleEditor::open_options_dialog() {
    open_dialog(
        "Options",
        std::bind(&ParticleEditor::process_gui_options_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&ParticleEditor::close_options_dialog, this);
}


/**
 * @brief Processes Dear ImGui for this frame.
 */
void ParticleEditor::process_gui() {
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.win_w, game.win_h));
    ImGui::Begin(
        "Particle editor", nullptr,
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
    
    //Process any dialogs.
    process_dialogs();
}


/**
 * @brief Processes the Dear ImGui control panel for this frame.
 */
void ParticleEditor::process_gui_control_panel() {
    if(manifest.internal_name.empty()) return;
    
    ImGui::BeginChild("panel");
    
    //Current file header text.
    ImGui::Text("File: ");
    
    //Current file text.
    ImGui::SameLine();
    mono_text("%s", manifest.internal_name.c_str());
    string file_tooltip =
        get_file_tooltip(manifest.path) + "\n\n"
        "File state: ";
    if(!changes_mgr.exists_on_disk()) {
        file_tooltip += "Not saved to disk yet!";
    } else if(changes_mgr.has_unsaved_changes()) {
        file_tooltip += "You have unsaved changes.";
    } else {
        file_tooltip += "Everything ok.";
    }
    set_tooltip(file_tooltip);
    
    ImGui::Spacer();
    
    //Process the particle generator info.
    process_gui_panel_generator();
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui particle generator deletion dialog
 * for this frame.
 */
void ParticleEditor::process_gui_delete_part_gen_dialog() {
    //Explanation text.
    string explanation_str;
    if(!changes_mgr.exists_on_disk()) {
        explanation_str =
            "You have never saved this particle generator to disk, so if you\n"
            "delete, you will only lose your unsaved progress.";
    } else {
        explanation_str =
            "If you delete, you will lose all unsaved progress, and the\n"
            "particle generator's files on the disk will be gone FOREVER!";
    }
    ImGui::SetupCentering(ImGui::CalcTextSize(explanation_str.c_str()).x);
    ImGui::Text("%s", explanation_str.c_str());
    
    //Final warning text.
    string final_warning_str =
        "Are you sure you want to delete the current particle generator?";
    ImGui::SetupCentering(ImGui::CalcTextSize(final_warning_str.c_str()).x);
    ImGui::TextColored(
        ImVec4(0.8, 0.6, 0.6, 1.0),
        "%s", final_warning_str.c_str()
    );
    
    //Cancel button.
    ImGui::Spacer();
    ImGui::SetupCentering(100 + 100 + 30);
    if(ImGui::Button("Cancel", ImVec2(100, 40))) {
        close_top_dialog();
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
        close_top_dialog();
        delete_current_part_gen();
    }
    ImGui::PopStyleColor(3);
}


/**
 * @brief Processes the "load" dialog for this frame.
 */
void ParticleEditor::process_gui_load_dialog() {
    //History node.
    process_gui_history(
        game.options.particle_editor.history,
    [this](const string &path) -> string {
        return path;
    },
    [this](const string &path) {
        close_top_dialog();
        load_part_gen_file(path, true);
    },
    [this](const string &path) {
        return get_file_tooltip(path);
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
    set_tooltip(
        "Creates a new particle generator."
    );
    
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
void ParticleEditor::process_gui_menu_bar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load file item.
            if(ImGui::MenuItem("Load or create...", "Ctrl+L")) {
                load_widget_pos = get_last_widget_pos();
                load_cmd(1.0f);
            }
            set_tooltip(
                "Pick a particle generator file to load.",
                "Ctrl + L"
            );
            
            //Reload current file item.
            if(ImGui::MenuItem("Reload current particle generator")) {
                reload_widget_pos = get_last_widget_pos();
                reload_cmd(1.0f);
            }
            set_tooltip(
                "Lose all changes and reload the current file from the disk."
            );
            
            //Save file item.
            if(ImGui::MenuItem("Save current particle generator", "Ctrl+S")) {
                save_cmd(1.0f);
            }
            set_tooltip(
                "Save the particle generator into the file on disk.",
                "Ctrl + S"
            );
            
            //Delete current particle generator item.
            if(ImGui::MenuItem("Delete current particle generator")) {
                delete_part_gen_cmd(1.0f);
            }
            set_tooltip(
                "Delete the current particle generator from the disk."
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
                "Quit the particle editor.",
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
            if(ImGui::MenuItem("Reset", "0")) {
                zoom_and_pos_reset_cmd(1.0f);
            }
            set_tooltip(
                "Reset the zoom level and camera position.",
                "0"
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
                    "The particle editor allows you to change how each "
                    "particle generator works. In-game, particle generators "
                    "are responsible for generating particles, and each one "
                    "emits particles differently. Each generator also has "
                    "information about its particles' sizes, colors, movement, "
                    "etc."
                    "\n\n"
                    "If you need more help on how to use the particle editor, "
                    "check out the tutorial in the manual, located "
                    "in the engine's folder.";
                open_help_dialog(help_str, "particle.html");
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
void ParticleEditor::process_gui_new_dialog() {
    string problem;
    bool hit_create_button = false;
    
    //Pack widgets.
    process_gui_new_dialog_pack_widgets(&new_dialog.pack);
    
    //Internal name input.
    ImGui::Spacer();
    ImGui::FocusOnInputText(new_dialog.needs_text_focus);
    if(
        mono_input_text(
            "Internal name", &new_dialog.internal_name,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hit_create_button = true;
    }
    set_tooltip(
        "Internal name of the new particle generator.\n"
        "Remember to keep it simple, type in lowercase, and use underscores!"
    );
    
    //Check if everything's ok.
    ContentManifest temp_man;
    temp_man.pack = new_dialog.pack;
    temp_man.internal_name = new_dialog.internal_name;
    new_dialog.part_gen_path =
        game.content.particle_gen.manifest_to_path(temp_man);
    if(new_dialog.last_checked_part_gen_path != new_dialog.part_gen_path) {
        new_dialog.part_gen_path_exists =
            file_exists(new_dialog.part_gen_path);
        new_dialog.last_checked_part_gen_path = new_dialog.part_gen_path;
    }
    
    if(new_dialog.internal_name.empty()) {
        problem = "You have to type an internal name first!";
    } else if(!is_internal_name_good(new_dialog.internal_name)) {
        problem =
            "The internal name should only have lowercase letters,\n"
            "numbers, and underscores!";
    } else {
        if(new_dialog.part_gen_path_exists) {
            problem =
                "There is already a particle generator with\n"
                "that internal name in that pack!";
        }
    }
    
    //Create button.
    ImGui::Spacer();
    ImGui::SetupCentering(200);
    if(!problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create particle generator", ImVec2(200, 40))) {
        hit_create_button = true;
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    set_tooltip(problem.empty() ? "Create the particle generator!" : problem);
    
    //Creation logic.
    if(hit_create_button) {
        if(!problem.empty()) return;
        auto really_create = [this] () {
            create_part_gen(new_dialog.part_gen_path);
            close_top_dialog();
            close_top_dialog(); //Close the load dialog.
        };
        
        if(
            new_dialog.pack == FOLDER_NAMES::BASE_PACK &&
            !game.options.advanced.engine_dev
        ) {
            open_base_content_warning_dialog(really_create);
        } else {
            really_create();
        }
    }
}


/**
 * @brief Processes the options dialog for this frame.
 */
void ParticleEditor::process_gui_options_dialog() {
    //Controls node.
    if(saveable_tree_node("options", "Controls")) {
    
        //Middle mouse button pans checkbox.
        ImGui::Checkbox("Use MMB to pan", &game.options.editors.mmb_pan);
        set_tooltip(
            "Use the middle mouse button to pan the camera\n"
            "(and RMB to reset camera/zoom).\n"
            "Default: " +
            b2s(OPTIONS::EDITORS_D::MMB_PAN) + "."
        );
        
        //Grid interval text.
        ImGui::Text(
            "Grid interval: %f", game.options.particle_editor.grid_interval
        );
        
        //Increase grid interval button.
        ImGui::SameLine();
        if(
            ImGui::Button(
                "+",
                ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())
            )
        ) {
            grid_interval_increase_cmd(1.0f);
        }
        set_tooltip(
            "Increase the spacing on the grid.\n"
            "Default: " + i2s(OPTIONS::PART_ED_D::GRID_INTERVAL) +
            ".",
            "Shift + Plus"
        );
        
        //Decrease grid interval button.
        ImGui::SameLine();
        if(
            ImGui::Button(
                "-",
                ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())
            )
        ) {
            grid_interval_decrease_cmd(1.0f);
        }
        set_tooltip(
            "Decrease the spacing on the grid.\n"
            "Default: " + i2s(OPTIONS::PART_ED_D::GRID_INTERVAL) +
            ".",
            "Shift + Minus"
        );
        
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
                game.options.particle_editor.bg_path.clear();
            }
        }
        set_tooltip(
            "Check this to use a repeating texture on the background\n"
            "of the editor."
        );
        
        if(use_bg) {
            ImGui::Indent();
            
            //Remove background texture button.
            unsigned char rem_bg_opacity =
                game.options.particle_editor.bg_path.empty() ? 50 : 255;
            if(
                ImGui::ImageButton(
                    "remBgButton", editor_icons[EDITOR_ICON_REMOVE],
                    Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                    COLOR_EMPTY, map_alpha(rem_bg_opacity)
                )
            ) {
                game.options.particle_editor.bg_path.clear();
                if(bg) {
                    al_destroy_bitmap(bg);
                    bg = nullptr;
                }
            }
            set_tooltip(
                "Remove the background image.\n"
                "This does not delete the file on your disk."
            );
            
            //Background texture browse button.
            ImGui::SameLine();
            if(ImGui::Button("Browse...")) {
                vector<string> f =
                    prompt_file_dialog(
                        FOLDER_PATHS_FROM_ROOT::BASE_PACK + "/" +
                        FOLDER_PATHS_FROM_PACK::TEXTURES,
                        "Please choose a background texture.",
                        "*.*", 0, game.display
                    );
                    
                if(!f.empty() && !f[0].empty()) {
                    game.options.particle_editor.bg_path = f[0];
                    if(bg) {
                        al_destroy_bitmap(bg);
                        bg = nullptr;
                    }
                    bg =
                        load_bmp(
                            game.options.particle_editor.bg_path,
                            nullptr, false, false, false
                        );
                }
            }
            set_tooltip(
                "Browse for which texture file on your disk to use."
            );
            
            //Background texture name text.
            string bg_file_name =
                get_path_last_component(game.options.particle_editor.bg_path);
            ImGui::SameLine();
            mono_text("%s", bg_file_name.c_str());
            set_tooltip("Full path:\n" + game.options.particle_editor.bg_path);
            
            ImGui::Unindent();
        }
        
        ImGui::TreePop();
        
    }
}


/**
 * @brief Processes the particle generator panel for this frame.
 */
void ParticleEditor::process_gui_panel_generator() {
    //Particle system text.
    ImGui::Text("Particle system:");
    
    //Particle count text.
    ImGui::Indent();
    ImGui::Text(
        "Particles: %lu / %lu",
        part_mgr.get_count(), game.options.advanced.max_particles
    );
    
    //Play/pause particle system button.
    if(
        ImGui::ImageButton(
            "playSystemButton",
            mgr_running ?
            editor_icons[EDITOR_ICON_STOP] :
            editor_icons[EDITOR_ICON_PLAY],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        part_mgr_playback_toggle_cmd(1.0f);
    }
    set_tooltip(
        "Play or pause the particle system.",
        "Shift + Spacebar"
    );
    
    ImGui::SameLine();
    
    //Clear particles button.
    if(
        ImGui::ImageButton(
            "clearParticlesButton", editor_icons[EDITOR_ICON_REMOVE],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        clear_particles_cmd(1.0f);
    }
    set_tooltip(
        "Delete all existing particles.", "D"
    );
    ImGui::Unindent();
    
    //Particle generator text.
    ImGui::Text("Generator:");
    
    //Play/pause particle generator button.
    ImGui::Indent();
    if(
        ImGui::ImageButton(
            "playGeneratorButton",
            gen_running ?
            editor_icons[EDITOR_ICON_STOP] :
            editor_icons[EDITOR_ICON_PLAY],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        part_gen_playback_toggle_cmd(1.0f);
    }
    set_tooltip(
        loaded_gen.emission.interval == 0.0f ?
        "Emit particles now." :
        "Play or pause the particle generator's emission.",
        "Spacebar"
    );
    
    //Particle generator angle value.
    ImGui::SameLine();
    ImGui::SetNextItemWidth(85);
    ImGui::SliderAngle("Angle", &generator_angle_offset, 0.0f);
    set_tooltip(
        "Rotate the generator's facing angle in the editor by this much.\n"
        "You can move the generator by just dragging the mouse in the canvas.",
        "", WIDGET_EXPLANATION_SLIDER
    );
    ImGui::Unindent();
    
    //Emission node.
    ImGui::Spacer();
    bool open_emission_node =
        saveable_tree_node("generator", "Emission");
    set_tooltip(
        "Everything about how the particle generator emits new particles."
    );
    if(open_emission_node) {
    
        //Basics node.
        bool open_basics_node =
            saveable_tree_node("generatorEmission", "Basics");
        set_tooltip("Edit basic information about emission here.");
        if(open_basics_node) {
        
            //Emit mode text.
            ImGui::Text("Mode:");
            
            //Emit once radio.
            int emit_mode = loaded_gen.emission.interval == 0.0f ? 0 : 1;
            ImGui::SameLine();
            if(ImGui::RadioButton("Once", &emit_mode, 0)) {
                if(loaded_gen.emission.interval != 0.0f) {
                    loaded_gen.emission.interval = 0.0f;
                    loaded_gen.emission.interval_deviation = 0.0f;
                    loaded_gen.restart_timer();
                }
                changes_mgr.mark_as_changed();
            }
            set_tooltip("The particles are created just once.");
            
            //Emit continuously radio.
            ImGui::SameLine();
            if(ImGui::RadioButton("Interval", &emit_mode, 1)) {
                if(loaded_gen.emission.interval == 0.0f) {
                    loaded_gen.emission.interval = 0.01f;
                    loaded_gen.emission.interval_deviation = 0.0f;
                    loaded_gen.restart_timer();
                }
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "The particles are constantly being created\n"
                "over time, with a set interval."
            );
            
            if(emit_mode == 1) {
                //Emission interval value.
                ImGui::Indent();
                ImGui::SetNextItemWidth(85);
                if(
                    ImGui::DragFloat(
                        "##interval", &loaded_gen.emission.interval,
                        0.01f, 0.01f, FLT_MAX
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "How long between particle emissions, in seconds.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Emission interval deviation text.
                ImGui::SameLine();
                ImGui::Text(" +-");
                
                //Emission interval deviation value.
                ImGui::SameLine();
                ImGui::SetNextItemWidth(70);
                if(
                    ImGui::DragFloat(
                        "##intervalDeviation",
                        &loaded_gen.emission.interval_deviation,
                        0.01f, 0.0f, FLT_MAX
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "The emission interval varies randomly up or down "
                    "by this amount.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                ImGui::Unindent();
            }
            
            //Emission number text.
            ImGui::Spacer();
            ImGui::Text("Number:");
            
            //Emission number value.
            int number_int = (int) loaded_gen.emission.number;
            ImGui::Indent();
            ImGui::SetNextItemWidth(85);
            if(
                ImGui::DragInt(
                    "##number", &number_int, 1, 1, (int) game.options.advanced.max_particles
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "How many particles are created per emission.",
                "", WIDGET_EXPLANATION_DRAG
            );
            loaded_gen.emission.number = number_int;
            
            //Emission number deviation text.
            ImGui::SameLine();
            ImGui::Text(" +-");
            
            //Emission number deviation value.
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            int number_dev_int = (int) loaded_gen.emission.number_deviation;
            if(
                ImGui::DragInt(
                    "##numberDeviation",
                    &number_dev_int, 1, 0, (int) game.options.advanced.max_particles
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "The creation amount varies randomly up or down by this "
                "amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            loaded_gen.emission.number_deviation = number_dev_int;
            
            ImGui::Unindent();
            
            ImGui::TreePop();
            
        }
        
        //Shape node.
        ImGui::Spacer();
        bool open_shape_node =
            saveable_tree_node("generatorEmission", "Shape");
        set_tooltip(
            "If you want the particles to appear within a specific shape\n"
            "around the generator, edit these properties."
        );
        if(open_shape_node) {
        
            //Circle emission shape radio.
            int shape = loaded_gen.emission.shape;
            if(
                ImGui::RadioButton(
                    "Circle", &shape, PARTICLE_EMISSION_SHAPE_CIRCLE
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Makes it so particles are created in a circle or \n"
                "ring shape around the origin."
            );
            
            //Rectangle emission shape radio.
            ImGui::SameLine();
            if(
                ImGui::RadioButton(
                    "Rectangle", &shape, PARTICLE_EMISSION_SHAPE_RECTANGLE
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Makes it so particles are created in a rectangle or \n"
                "rectangular ring shape around the origin."
            );
            loaded_gen.emission.shape = (PARTICLE_EMISSION_SHAPE)shape;
            
            ImGui::Indent();
            switch (loaded_gen.emission.shape) {
            case PARTICLE_EMISSION_SHAPE_CIRCLE: {
                //Circle emission inner distance value.
                ImGui::SetNextItemWidth(75);
                if(
                    ImGui::DragFloat(
                        "Inner distance",
                        &loaded_gen.emission.circle_inner_dist,
                        0.1f, 0.0f, FLT_MAX
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "Minimum emission distance for particle creation.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Circle emission outer distance value.
                ImGui::SetNextItemWidth(75);
                if(
                    ImGui::DragFloat(
                        "Outer distance",
                        &loaded_gen.emission.circle_outer_dist,
                        0.1f, 0.0f, FLT_MAX
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "Maximum emission distance for particle creation.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                loaded_gen.emission.circle_inner_dist =
                    std::max(
                        loaded_gen.emission.circle_inner_dist,
                        0.0f
                    );
                loaded_gen.emission.circle_outer_dist =
                    std::max(
                        loaded_gen.emission.circle_inner_dist,
                        loaded_gen.emission.circle_outer_dist
                    );
                    
                //Circle emission arc value.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::SliderAngle(
                        "Arc", &loaded_gen.emission.circle_arc, 0
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "Arc of the circle for particle creation.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Circle emission arc rotation value.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::SliderAngle(
                        "Arc rotation", &loaded_gen.emission.circle_arc_rot,
                        0.0f
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "Rotate the emission arc by these many degrees.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Evenly spread checkbox.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::Checkbox(
                        "Evenly spread", &loaded_gen.emission.evenly_spread
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "Evenly spread the particles throughout the emission\n"
                    "area, instead of placing them randomly."
                );
                
                break;
                
            } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
                //Rectangle emission inner distance values.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::DragFloat2(
                        "Inner distance",
                        (float*) &loaded_gen.emission.rect_inner_dist,
                        0.1f, 0.0f, FLT_MAX
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "Minimum emission distance (X and Y) for particle "
                    "creation.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Rectangle emission outer distance values.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::DragFloat2(
                        "Outer distance",
                        (float*) &loaded_gen.emission.rect_outer_dist,
                        0.1f, 0.0f, FLT_MAX
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "Maximum emission distance (X and Y) for particle "
                    "creation.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                loaded_gen.emission.rect_inner_dist.x =
                    std::max(
                        loaded_gen.emission.rect_inner_dist.x,
                        0.0f
                    );
                loaded_gen.emission.rect_inner_dist.y =
                    std::max(
                        loaded_gen.emission.rect_inner_dist.y,
                        0.0f
                    );
                loaded_gen.emission.rect_outer_dist.x =
                    std::max(
                        loaded_gen.emission.rect_outer_dist.x,
                        loaded_gen.emission.rect_inner_dist.x
                    );
                loaded_gen.emission.rect_outer_dist.y =
                    std::max(
                        loaded_gen.emission.rect_outer_dist.y,
                        loaded_gen.emission.rect_inner_dist.y
                    );
                    
                break;
            }
            }
            ImGui::Unindent();
            
            ImGui::TreePop();
            
        }
        
        ImGui::TreePop();
    }
    
    //Particle appearance node.
    ImGui::Spacer();
    bool open_appearance_node =
        saveable_tree_node("generator", "Particle appearance");
    set_tooltip(
        "Everything about how a particle looks."
    );
    if(open_appearance_node) {
    
        //Image node.
        bool open_image_node =
            saveable_tree_node("generatorAppearance", "Image");
        set_tooltip(
            "Edit information about the image (if any) to draw\n"
            "on a particle here."
        );
        if(open_image_node) {
        
            //Remove bitmap button.
            unsigned char rem_bmp_opacity =
                loaded_gen.base_particle.bmp_name.empty() ? 50 : 255;
            if(
                ImGui::ImageButton(
                    "remBmpButton", editor_icons[EDITOR_ICON_REMOVE],
                    Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                    COLOR_EMPTY, map_alpha(rem_bmp_opacity)
                )
            ) {
                //We can't have living particles with destroyed bitmaps,
                //so clear them all.
                part_mgr.clear();
                loaded_gen.base_particle.set_bitmap("");
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Remove the particles' image.\n"
                "This makes the particles be circles."
            );
            
            //Choose image button.
            ImGui::SameLine();
            if(ImGui::Button("Choose image...")) {
                open_bitmap_dialog(
                [this] (const string &bmp) {
                    //We can't have living particles with destroyed bitmaps,
                    //so clear them all.
                    part_mgr.clear();
                    loaded_gen.base_particle.set_bitmap(bmp);
                    changes_mgr.mark_as_changed();
                    set_status("Picked an image successfully.");
                },
                "."
                );
            }
            set_tooltip("Choose which image to use from the game's content.");
            
            //Image name text.
            ImGui::SameLine();
            mono_text("%s", loaded_gen.base_particle.bmp_name.c_str());
            set_tooltip("Internal name:\n" + loaded_gen.base_particle.bmp_name);
            
            if(loaded_gen.base_particle.bitmap) {
            
                //Image angle text.
                ImGui::Spacer();
                ImGui::Text("Angle:");
                
                //Fixed angle radio.
                int angle_type_int = loaded_gen.base_particle.bmp_angle_type;
                ImGui::SameLine();
                if(
                    ImGui::RadioButton(
                        "Fixed", &angle_type_int,
                        PARTICLE_ANGLE_TYPE_FIXED
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "A particle's image angle is fixed all throughout."
                );
                
                //Direction angle radio.
                ImGui::SameLine();
                if(
                    ImGui::RadioButton(
                        "Direction", &angle_type_int,
                        PARTICLE_ANGLE_TYPE_DIRECTION
                    )
                ) {
                    changes_mgr.mark_as_changed();
                }
                set_tooltip(
                    "A particle's image angle matches the direction it's "
                    "traveling."
                );
                loaded_gen.base_particle.bmp_angle_type =
                    (PARTICLE_ANGLE_TYPE) angle_type_int;
                    
                if(
                    loaded_gen.base_particle.bmp_angle_type ==
                    PARTICLE_ANGLE_TYPE_FIXED
                ) {
                
                    //Image angle value.
                    ImGui::Indent();
                    ImGui::SetNextItemWidth(85);
                    if(
                        ImGui::SliderAngle(
                            "##imgAngle",
                            &loaded_gen.base_particle.bmp_angle, 0.0f
                        )
                    ) {
                        changes_mgr.mark_as_changed();
                    }
                    set_tooltip(
                        "Angle of the image.",
                        "", WIDGET_EXPLANATION_SLIDER
                    );
                    
                    //Image angle deviation text.
                    ImGui::SameLine();
                    ImGui::Text(" +-");
                    
                    //Angle deviation value.
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(70);
                    if(
                        ImGui::SliderAngle(
                            "##imgAngleDev",
                            &loaded_gen.bmp_angle_deviation, 0, 180
                        )
                    ) {
                        changes_mgr.mark_as_changed();
                    }
                    set_tooltip(
                        "A particle's image angle varies randomly up or down\n"
                        "by this amount.",
                        "", WIDGET_EXPLANATION_SLIDER
                    );
                    ImGui::Unindent();
                }
            }
            
            ImGui::TreePop();
            
        }
        
        //Particle color node.
        ImGui::Spacer();
        bool open_color_node =
            saveable_tree_node("generatorAppearance", "Color");
        set_tooltip(
            "Control the color a particle has and how it changes over time "
            "here."
        );
        if(open_color_node) {
        
            //Color keyframe editor.
            if(
                keyframe_editor(
                    "Color", loaded_gen.base_particle.color,
                    selected_color_keyframe
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            
            //Blend mode text.
            ImGui::Spacer();
            ImGui::Text("Blend:");
            
            //Normal blending radio.
            int blend_int = loaded_gen.base_particle.blend_type;
            ImGui::SameLine();
            if(
                ImGui::RadioButton(
                    "Normal", &blend_int, PARTICLE_BLEND_TYPE_NORMAL
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Particles appear on top of other particles like normal."
            );
            
            //Additive blending radio.
            ImGui::SameLine();
            if(
                ImGui::RadioButton(
                    "Additive", &blend_int, PARTICLE_BLEND_TYPE_ADDITIVE
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Particle colors add onto the color of particles underneath\n"
                "them. This makes it so the more particles there are,\n"
                "the brighter the color gets."
            );
            loaded_gen.base_particle.blend_type =
                (PARTICLE_BLEND_TYPE) blend_int;
                
            ImGui::TreePop();
            
        }
        
        //Particle size node.
        ImGui::Spacer();
        bool open_size_node =
            saveable_tree_node("generatorAppearance", "Size");
        set_tooltip(
            "Control a particle's size and how it changes over time here."
        );
        if(open_size_node) {
        
            //Size keyframe editor.
            if(
                keyframe_editor(
                    "Size", loaded_gen.base_particle.size,
                    selected_size_keyframe
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            loaded_gen.base_particle.size.set_keyframe_value(
                selected_size_keyframe,
                std::max(
                    0.0f,
                    loaded_gen.base_particle.size.get_keyframe(
                        selected_size_keyframe
                    ).second
                )
            );
            
            //Size deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(70);
            if(
                ImGui::DragFloat(
                    "Size deviation", &loaded_gen.size_deviation,
                    0.5f, 0.0f, FLT_MAX
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's size varies randomly up or down by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            ImGui::TreePop();
            
        }
        
        ImGui::TreePop();
    }
    
    //Particle behavior node.
    ImGui::Spacer();
    bool open_behavior_node =
        saveable_tree_node("generator", "Particle behavior");
    set_tooltip(
        "Everything about how a particle behaves."
    );
    if(open_behavior_node) {
    
        //Basics node.
        bool open_basics_node =
            saveable_tree_node("generatorBehavior", "Basics");
        set_tooltip(
            "Control how long a particle lasts for, and more, here."
        );
        if(open_basics_node) {
        
            //Duration text.
            ImGui::Text("Duration:");
            
            //Duration value.
            ImGui::SetNextItemWidth(85);
            if(
                ImGui::DragFloat(
                    "##particleDur", &loaded_gen.base_particle.duration,
                    0.01f, 0.01f, FLT_MAX
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "How long each particle lives for, in seconds.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Duration deviation text.
            ImGui::SameLine();
            ImGui::Text(" +-");
            
            //Duration deviation value.
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            if(
                ImGui::DragFloat(
                    "##particleDurDev",
                    &loaded_gen.duration_deviation, 0.01f, 0.0f, FLT_MAX
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's lifespan varies randomly up or down by this "
                "amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Absolute angles checkbox.
            ImGui::Spacer();
            if(
                ImGui::Checkbox(
                    "Absolute angles", &loaded_gen.angles_are_absolute
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "If unchecked, the angles the particles move at are relative\n"
                "to the angle of the object, if the particle generator\n"
                "is attached to an object. If checked, the angles are\n"
                "always the same no matter what."
            );
            
            ImGui::TreePop();
            
        }
        
        //Linear speed node.
        ImGui::Spacer();
        bool open_linear_speed_node =
            saveable_tree_node("generatorBehavior", "Linear speed");
        set_tooltip(
            "Control a particle's linear (simple) X and Y speed here."
        );
        if(open_linear_speed_node) {
        
            //Linear speed keyframe editor.
            if(
                keyframe_editor(
                    "Speed", loaded_gen.base_particle.linear_speed,
                    selected_linear_speed_keyframe
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            
            //Linear speed deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(150);
            if(
                ImGui::DragFloat2(
                    "Speed deviation",
                    (float*) &loaded_gen.linear_speed_deviation,
                    0.5f, 0.0f, FLT_MAX
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's linear speed varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Angle deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(75);
            if(
                ImGui::SliderAngle(
                    "Angle deviation",
                    &loaded_gen.linear_speed_angle_deviation, 0, 180
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's movement angle varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            ImGui::TreePop();
            
        }
        
        //Outwards speed node.
        ImGui::Spacer();
        bool open_outwards_speed_node =
            saveable_tree_node("generatorBehavior", "Outwards speed");
        set_tooltip(
            "Control the speed at which a particle moves out from\n"
            "the center here. Use negative values to make them move\n"
            "towards the center instead."
        );
        if(open_outwards_speed_node) {
        
            //Outwards speed keyframe editor.
            if(
                keyframe_editor(
                    "Speed", loaded_gen.base_particle.outwards_speed,
                    selected_outward_velocity_keyframe
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            
            //Outward speed deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(150);
            if(
                ImGui::DragFloat(
                    "Speed deviation",
                    &loaded_gen.outwards_speed_deviation,
                    0.5f, 0.0f, FLT_MAX
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's outward speed varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            ImGui::TreePop();
        }
        
        //Orbital speed node.
        ImGui::Spacer();
        bool open_orbital_speed_node =
            saveable_tree_node("generatorBehavior", "Orbital speed");
        set_tooltip(
            "Control the speed at which a particle orbits around the center "
            "here."
        );
        if(open_orbital_speed_node) {
        
            //Orbital speed keyframe editor.
            if(
                keyframe_editor(
                    "Speed", loaded_gen.base_particle.orbital_speed,
                    selected_oribital_velocity_keyframe
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            
            //Orbital speed deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(150);
            if(
                ImGui::DragFloat(
                    "Speed deviation",
                    &loaded_gen.orbital_speed_deviation,
                    0.5f, 0.0f, FLT_MAX
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's orbital speed varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            ImGui::TreePop();
        }
        
        //Friction node.
        ImGui::Spacer();
        bool open_friction_node =
            saveable_tree_node("generatorBehavior", "Friction");
        set_tooltip(
            "Control how a particle loses speed here."
        );
        if(open_friction_node) {
        
            //Friction value.
            ImGui::SetNextItemWidth(85);
            if(
                ImGui::DragFloat(
                    "##particleFriction",
                    &loaded_gen.base_particle.friction, 0.1f, -FLT_MAX, FLT_MAX
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Slowing factor applied to a particle.\n"
                "Negative values make it speed up.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Friction deviation text.
            ImGui::SameLine();
            ImGui::Text(" +-");
            
            //Friction deviation value.
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            if(
                ImGui::DragFloat(
                    "##particleFrictionDev",
                    &loaded_gen.friction_deviation, 0.1f, 0.0f, FLT_MAX
                )
            ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's friction varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            ImGui::TreePop();
            
        }
        
        ImGui::TreePop();
        
    }
    
    //Info node.
    ImGui::Spacer();
    bool open_info_node =
        saveable_tree_node("generator", "Info");
    set_tooltip(
        "Optional information about the particle generator."
    );
    if(open_info_node) {
    
        //Name input.
        if(
            ImGui::InputText("Name", &loaded_gen.name)
        ) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Name of this particle generator. Optional."
        );
        
        //Description input.
        if(
            ImGui::InputText("Description", &loaded_gen.description)
        ) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Description of this particle generator. Optional."
        );
        
        //Version input.
        if(
            mono_input_text("Version", &loaded_gen.version)
        ) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Version of the file, preferably in the \"X.Y.Z\" format. "
            "Optional."
        );
        
        //Maker input.
        if(
            ImGui::InputText("Maker", &loaded_gen.maker)
        ) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Name (or nickname) of who made this file. "
            "Optional."
        );
        
        //Maker notes input.
        if(
            ImGui::InputText("Maker notes", &loaded_gen.maker_notes)
        ) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Extra notes or comments about the file for other makers to see. "
            "Optional."
        );
        
        //Notes input.
        if(ImGui::InputText("Notes", &loaded_gen.notes)) {
            changes_mgr.mark_as_changed();
        }
        set_tooltip(
            "Extra notes or comments of any kind. "
            "Optional."
        );
        
        ImGui::TreePop();
        
    }
    
}


/**
 * @brief Processes the Dear ImGui status bar for this frame.
 */
void ParticleEditor::process_gui_status_bar() {
    //Status bar text.
    process_gui_status_bar_text();
    
    //Spacer dummy widget.
    ImGui::SameLine();
    float size =
        canvas_separator_x - ImGui::GetItemRectSize().x -
        EDITOR::MOUSE_COORDS_TEXT_WIDTH;
    ImGui::Dummy(ImVec2(size, 0));
    
    //Mouse coordinates text.
    if(!is_mouse_in_gui || is_m1_pressed) {
        ImGui::SameLine();
        mono_text(
            "%s, %s",
            box_string(f2s(game.mouse_cursor.w_pos.x), 7).c_str(),
            box_string(f2s(game.mouse_cursor.w_pos.y), 7).c_str()
        );
    }
}


/**
 * @brief Processes the Dear ImGui toolbar for this frame.
 */
void ParticleEditor::process_gui_toolbar() {
    //Quit button.
    if(
        ImGui::ImageButton(
            "quitButton", editor_icons[EDITOR_ICON_QUIT],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        quit_widget_pos = get_last_widget_pos();
        quit_cmd(1.0f);
    }
    set_tooltip(
        "Quit the particle editor.",
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
        load_widget_pos = get_last_widget_pos();
        load_cmd(1.0f);
    }
    set_tooltip(
        "Pick a particle generator file to load.",
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
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        save_cmd(1.0f);
    }
    set_tooltip(
        "Save the particle generator into the file on disk.",
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
        grid_toggle_cmd(1.0f);
    }
    set_tooltip(
        "Toggle visibility of the grid.",
        "Ctrl + G"
    );
    
    //Leader silhouette button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "silhouetteButton", editor_icons[EDITOR_ICON_LEADER_SILHOUETTE],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        leader_silhouette_toggle_cmd(1.0f);
    }
    set_tooltip(
        "Toggle visibility of a leader silhouette.",
        "Ctrl + P"
    );
    
    //Emission shape button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "emissionShapeButton", editor_icons[EDITOR_ICON_MOB_RADIUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        emission_shape_toggle_cmd(1.0f);
    }
    set_tooltip(
        "Toggle visibility of the emission shape.",
        "Ctrl + R"
    );
}
