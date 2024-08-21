/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * GUI editor Dear ImGui logic.
 */

#include "editor.h"

#include "../../functions.h"
#include "../../game.h"
#include "../../utils/allegro_utils.h"
#include "../../utils/string_utils.h"
#include "../../libs/imgui/imgui_stdlib.h"

/**
 * @brief Opens the "load" dialog.
 */
void particle_editor::open_load_dialog() {
    //Set up the picker's behavior and data.
    vector<string> files = folder_to_vector(PARTICLE_GENERATORS_FOLDER_PATH, false);
    vector<picker_item> file_items;
    for(size_t f = 0; f < files.size(); ++f) {
        file_items.push_back(picker_item(files[f]));
    }
    load_dialog_picker = picker_info(this);
    load_dialog_picker.can_make_new = false;
    load_dialog_picker.items = file_items;
    load_dialog_picker.pick_callback =
        std::bind(
            &particle_editor::pick_file, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3
        );
        
    //Open the dialog that will contain the picker and history.
    open_dialog(
        "Load a Particle file",
        std::bind(&particle_editor::process_gui_load_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&particle_editor::close_load_dialog, this);
}


/**
 * @brief Opens the options dialog.
 */
void particle_editor::open_options_dialog() {
    open_dialog(
        "Options",
        std::bind(&particle_editor::process_gui_options_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&particle_editor::close_options_dialog, this);
}


/**
 * @brief Processes Dear ImGui for this frame.
 */
void particle_editor::process_gui() {
    //Initial setup.
    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();
    
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
    
    //Small hack. Recenter the camera, if necessary.
    if(must_recenter_cam) {
        reset_cam(true);
        must_recenter_cam = false;
    }
    
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
    process_dialogs();
    
    //Finishing setup.
    ImGui::EndFrame();
}


/**
 * @brief Processes the Dear ImGui control panel for this frame.
 */
void particle_editor::process_gui_control_panel() {
    ImGui::BeginChild("panel");
    
    //Current file text.
    ImGui::Text("Current file: %s", file_name.c_str());

    process_gui_panel_item();
    
    ImGui::EndChild();
}


/**
 * @brief Processes the "load" dialog for this frame.
 */
void particle_editor::process_gui_load_dialog() {
    //History node.
    process_gui_history(
    [this](const string &name) -> string {
        return name;
    },
    [this](const string &name) {
        file_name = name;
        load_particle_generator(true);
        close_top_dialog();
    }
    );
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //List node.
    if(saveable_tree_node("load", "Full list")) {
        load_dialog_picker.process();
        
        ImGui::TreePop();
    }
}


/**
 * @brief Processes the Dear ImGui menu bar for this frame.
 */
void particle_editor::process_gui_menu_bar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load file item.
            if(ImGui::MenuItem("Load file...", "Ctrl+L")) {
                load_widget_pos = get_last_widget_pos();
                load_cmd(1.0f);
            }
            set_tooltip(
                "Pick a GUI file to load.",
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
            
            //Save file item.
            if(ImGui::MenuItem("Save file", "Ctrl+S")) {
                save_cmd(1.0f);
            }
            set_tooltip(
                "Save the GUI into the file on disk.",
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
                "Quit the GUI editor.",
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
                    "This editor allows you to change where each item "
                    "in a graphical user interface is, and how big it is. "
                    "It works both for the gameplay HUD and any menu's items. "
                    "In the canvas you can find the \"game window\", but in "
                    "reality, it's just some square. This is because the "
                    "coordinates you work in go from 0% to 100%, instead of "
                    "using a real screen size, since the player can choose "
                    "whatever screen size they want. In addition, for the sake "
                    "of simplicity, the editor won't show what each GUI item "
                    "looks like. So you will have to use your imagination to "
                    "visualize how everything will really look in-game."
                    "\n\n"
                    "If you need more help on how to use the GUI editor, "
                    "check out the tutorial in the manual, located "
                    "in the engine's folder.";
                show_message_box(
                    game.display, "Help", "GUI editor help",
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
 * @brief Processes the options dialog for this frame.
 */
void particle_editor::process_gui_options_dialog() {
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
        
        //Grid interval text.
        ImGui::Text(
            "Grid interval: %f", game.options.particle_editor_grid_interval
        );
        
        //Increase grid interval button.
        ImGui::SameLine();
        if(ImGui::Button("+")) {
            grid_interval_increase_cmd(1.0f);
        }
        set_tooltip(
            "Increase the spacing on the grid.\n"
            "Default: " + i2s(OPTIONS::DEF_PARTICLE_EDITOR_GRID_INTERVAL) +
            ".",
            "Shift + Plus"
        );
        
        //Decrease grid interval button.
        ImGui::SameLine();
        if(ImGui::Button("-")) {
            grid_interval_decrease_cmd(1.0f);
        }
        set_tooltip(
            "Decrease the spacing on the grid.\n"
            "Default: " + i2s(OPTIONS::DEF_PARTICLE_EDITOR_GRID_INTERVAL) +
            ".",
            "Shift + Minus"
        );
        
        ImGui::TreePop();
        
    }
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    process_gui_editor_style();
}


/**
 * @brief Processes the GUI item info panel for this frame.
 */
void particle_editor::process_gui_panel_item() {
    if(!loaded_content_yet)
        return;
    //Play/pause button.
    if (
        ImGui::ImageButton(
            "playButton",
            generator_running ? editor_icons[EDITOR_ICON_STOP] : editor_icons[EDITOR_ICON_PLAY],
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
        ) {
        particle_playback_toggle_cmd(1.0f);
    }
    set_tooltip(
        "Play or pause the particle system.",
        "Spacebar"
    );
    
    if (ImGui::BeginTabBar("particleTabs")) {
        if (ImGui::BeginTabItem("Emission")) {

            ImGui::Dummy(ImVec2(0, 4));

            //Emission Interval value.
            if (
                ImGui::DragFloat(
                    "Emission Interval", &loaded_gen.emission_interval, 0.0005, 0.0f, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "How long between particle emissions, in seconds.",
                "", WIDGET_EXPLANATION_DRAG
            );

            //Number value.
            int number = (int)loaded_gen.number;
            if (
                ImGui::DragInt(
                    "Number", &number, 1, 1, game.options.max_particles
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "How many particles are emitted per interval.",
                "", WIDGET_EXPLANATION_DRAG
            );
            loaded_gen.number = number;

            ImGui::Indent();
            //Number Deviation value.
            ImGui::SetNextItemWidth(75);
            int number_dev = (int)loaded_gen.number_deviation;
            if (
                ImGui::DragInt(
                    "Number deviation", &number_dev, 1, 0, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "The amount of particles emitted is changed by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            loaded_gen.number_deviation = number_dev;
            ImGui::Unindent();

            //Postion deviation value.
            if (
                ImGui::DragFloat2(
                    "Position deviation", (float*)&loaded_gen.pos_deviation, 0.5f, -FLT_MAX, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's position can vary by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visuals", nullptr)) {

            ImGui::Dummy(ImVec2(0, 4));

            //Remove bitmap button.
            if (
                ImGui::ImageButton(
                    "removeBitmap",
                    editor_icons[EDITOR_ICON_REMOVE],
                    ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                )
                ) {
                loaded_gen.base_particle.set_bitmap("");
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Remove the current bitmap"
            );

            ImGui::SameLine();
            //Browse for bitmap button.
            if (ImGui::Button("...")) {
                FILE_DIALOG_RESULT result = FILE_DIALOG_RESULT_SUCCESS;
                vector<string> f =
                    prompt_file_dialog_locked_to_folder(
                        GRAPHICS_FOLDER_PATH,
                        "Please choose the bitmap to get the sprites from.",
                        "*.png",
                        ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                        ALLEGRO_FILECHOOSER_PICTURES,
                        &result, game.display
                    );

                switch (result) {
                case FILE_DIALOG_RESULT_WRONG_FOLDER: {
                    //File doesn't belong to the folder.
                    set_status("The chosen image is not in the graphics folder!", true);
                    break;
                } case FILE_DIALOG_RESULT_CANCELED: {
                    //User canceled.
                    break;
                } case FILE_DIALOG_RESULT_SUCCESS: {
                    loaded_gen.base_particle.set_bitmap(f[0]);
                    set_status("Picked an image successfully.");
                    changes_mgr.mark_as_changed();
                    break;
                }
                }
            }
            set_tooltip("Browse for a spritesheet file to use.");

            //Spritesheet file name input.
            string file_name = loaded_gen.base_particle.file;
            ImGui::SameLine();
            if (ImGui::InputText("File", &file_name)) {
                loaded_gen.base_particle.set_bitmap(file_name);
                set_status("Picked an image successfully.");
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "File name of the bitmap to use as a spritesheet, in the "
                "Graphics folder. Extension included. e.g. "
                "\"Large_Fly.png\""
            );

            if(saveable_tree_node("particleColors", "Color")) {
                for (size_t c = 0; c < loaded_gen.base_particle.color.keyframe_count(); c++) {
                    if (c != 0) {
                        string btnLabel = "colorDeleteButton" + i2s(c);
                        if (
                            ImGui::ImageButton(
                                btnLabel.c_str(),
                                editor_icons[EDITOR_ICON_REMOVE],
                                ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                            )
                            ) {
                            loaded_gen.base_particle.color.remove(c);
                            continue;
                        }
                        set_tooltip(
                            "Add a new color to the list of colors.\n"
                            "Click to open a pop-up for you to choose from."
                        );
                        ImGui::SameLine();
                    }
                    ALLEGRO_COLOR particle_color = loaded_gen.base_particle.color.get_keyframe(c).second;
                    string colorName = "Tint " + i2s(c + 1);
                    if (
                        ImGui::ColorEdit4(
                            colorName.c_str(), (float*)&particle_color,
                            ImGuiColorEditFlags_NoInputs
                        )
                        ) {
                        changes_mgr.mark_as_changed();
                        loaded_gen.base_particle.color.set_keyframe_value(c, particle_color);
                    }
                    set_tooltip(
                        "Particle's tint."
                    );
                    if(c == 0)
                        continue;
                    ImGui::SameLine();
                    float time = loaded_gen.base_particle.color.get_keyframe(c).first;
                    string timeName = "Time " + i2s(c + 1);
                    if (
                        ImGui::DragFloat(
                            timeName.c_str(), &time, 0.01f, 0, 1
                        )
                        ) {
                        changes_mgr.mark_as_changed();
                        loaded_gen.base_particle.color.set_keyframe_time(c, time);
                    }
                    set_tooltip(
                        "Inital particle size.",
                        "", WIDGET_EXPLANATION_DRAG
                    );
                }
                //Hitbox hazard addition button.
                if (
                    ImGui::ImageButton(
                        "colorAddButton",
                        editor_icons[EDITOR_ICON_ADD],
                        ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
                    )
                    ) {
                    loaded_gen.base_particle.color.add(1, COLOR_WHITE);
                }
                set_tooltip(
                    "Add a new color to the list of colors.\n"
                    "Click to open a pop-up for you to choose from."
                );
                ImGui::TreePop();
            }

            //Size value.
            if (
                ImGui::DragFloat(
                    "Size", &loaded_gen.base_particle.size, 0.01f, 0.01f, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Inital particle size.",
                "", WIDGET_EXPLANATION_DRAG
            );
            ImGui::Indent();
            //Size grow speed value.
            ImGui::SetNextItemWidth(75);
            if (
                ImGui::DragFloat(
                    "Grow Speed", &loaded_gen.base_particle.size_grow_speed, 0.01f, -FLT_MAX, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Increase size by this much per second.",
                "", WIDGET_EXPLANATION_DRAG
            );

            //Size deviation value.
            ImGui::SetNextItemWidth(75);
            if (
                ImGui::DragFloat(
                    "Size deviation", &loaded_gen.size_deviation, 0.01f, 0, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's size can vary by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            ImGui::Unindent();

            //Duration value.
            if (
                ImGui::DragFloat(
                    "Duration", &loaded_gen.base_particle.duration, 0.01f, 0.01f, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "How long each particle persists, in seconds.",
                "", WIDGET_EXPLANATION_DRAG
            );

            ImGui::Indent();
            //Duration deviation value.
            ImGui::SetNextItemWidth(75);
            if (
                ImGui::DragFloat(
                    "Duration deviation", &loaded_gen.duration_deviation, 0.01f, 0, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's lifespan can vary by this amount of seconds.",
                "", WIDGET_EXPLANATION_DRAG
            );
            ImGui::Unindent();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Motion", nullptr)) {

            ImGui::Dummy(ImVec2(0, 4));

            //Friction value.
            if (
                ImGui::DragFloat(
                    "Friction", &loaded_gen.base_particle.friction, 0.1f, -FLT_MAX, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Slowing factor applied to particles.",
                "", WIDGET_EXPLANATION_DRAG
            );

            ImGui::Indent();
            //Friction deviation value.
            ImGui::SetNextItemWidth(75);
            if (
                ImGui::DragFloat(
                    "Friction deviation", &loaded_gen.friction_deviation, 0.1f, 0, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's friciton can vary by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            ImGui::Unindent();

            //Gravity value.
            if (
                ImGui::DragFloat(
                    "Gravity", &loaded_gen.base_particle.gravity, 1, -FLT_MAX, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Downards speed applied to particles.",
                "", WIDGET_EXPLANATION_DRAG
            );

            ImGui::Indent();
            //Gravity deviation value.
            ImGui::SetNextItemWidth(75);
            if (
                ImGui::DragFloat(
                    "Gravity deviation", &loaded_gen.gravity_deviation, 0.5f, 0, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's gravity can vary by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            ImGui::Unindent();

            //Speed value.
            if (
                ImGui::DragFloat2(
                    "Speed", (float*)&loaded_gen.base_particle.speed, 1, -FLT_MAX, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Inital particle speed.",
                "", WIDGET_EXPLANATION_DRAG
            );

            ImGui::Indent();
            //Speed deviation value.
            ImGui::SetNextItemWidth(150);
            if (
                ImGui::DragFloat2(
                    "Speed deviation", (float*)&loaded_gen.speed_deviation, 0.01f, 0, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "A particle's speed can vary by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            ImGui::Unindent();

            //Angle value.
            float angle = rad_to_deg(loaded_gen.angle);
            if (
                ImGui::DragFloat(
                    "Angle", &angle, 1, -FLT_MAX, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "The angle a particle is emitted at.",
                "", WIDGET_EXPLANATION_DRAG
            );
            if (angle < 0)
                angle += 360;
            loaded_gen.angle = deg_to_rad(fmodf(angle, 360));

            ImGui::Indent();
            //Angle deviation value.
            float angle_deviation = rad_to_deg(loaded_gen.angle_deviation);
            ImGui::SetNextItemWidth(75);
            if (
                ImGui::DragFloat(
                    "Angle deviation", &angle_deviation, 1, 0, 360
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "The angle a particle is emitted at can vary by this much.",
                "", WIDGET_EXPLANATION_DRAG
            );
            loaded_gen.angle_deviation = deg_to_rad(angle_deviation);
            ImGui::Unindent();


            //Total speed value.
            if (
                ImGui::DragFloat(
                    "Total speed", &loaded_gen.total_speed, 1, 0, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "The speed a particle is emitted at.",
                "", WIDGET_EXPLANATION_DRAG
            );

            ImGui::Indent();
            //Total speed deviation value.
            ImGui::SetNextItemWidth(75);
            if (
                ImGui::DragFloat(
                    "Speed deviation", &loaded_gen.total_speed_deviation, 0.5f, 0, FLT_MAX
                )
                ) {
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "The speed a particle is emitted at can vary by this much.",
                "", WIDGET_EXPLANATION_DRAG
            );
            ImGui::Unindent();

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}


/**
 * @brief Processes the Dear ImGui status bar for this frame.
 */
void particle_editor::process_gui_status_bar() {
    //Status bar text.
    process_gui_status_bar_text();
    
    //Spacer dummy widget.
    ImGui::SameLine();
    float size =
        canvas_separator_x - ImGui::GetItemRectSize().x -
        PARTICLE_EDITOR::MOUSE_COORDS_TEXT_WIDTH;
    ImGui::Dummy(ImVec2(size, 0));
    
    //Mouse coordinates text.
    if(!is_mouse_in_gui || is_m1_pressed) {
        ImGui::SameLine();
        ImGui::Text(
            "%s, %s",
            box_string(f2s(game.mouse_cursor.w_pos.x), 7).c_str(),
            box_string(f2s(game.mouse_cursor.w_pos.y), 7).c_str()
        );
    }
}


/**
 * @brief Processes the Dear ImGui toolbar for this frame.
 */
void particle_editor::process_gui_toolbar() {
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
        "Quit the GUI editor.",
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
        "Pick a GUI file to load.",
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
        "Save the GUI into the file on disk.",
        "Ctrl + S"
    );

    ImGui::SameLine(0, 16);
    if (
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
