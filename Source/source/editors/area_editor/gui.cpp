/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor Dear ImGui logic.
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
 * Shows the area picker, if possible.
 */
void area_editor::open_area_picker() {
    if(!check_new_unsaved_changes()) {
        vector<picker_item> areas;
        vector<string> folders =
            folder_to_vector(AREAS_FOLDER_PATH, true);
            
        for(size_t f = 0; f < folders.size(); ++f) {
            areas.push_back(picker_item(folders[f]));
        }
        picker.set(
            areas, "Pick an area, or create a new one",
            std::bind(
                &area_editor::pick_area, this,
                std::placeholders::_1,
                std::placeholders::_2
            ),
            "", true
        );
    }
}


/* ----------------------------------------------------------------------------
 * Processes ImGui for this frame.
 */
void area_editor::process_gui() {
    //Initial setup.
    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();
    
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.win_w, game.win_h));
    ImGui::Begin(
        "Area editor", NULL,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse// | ImGuiWindowFlags_NoBackground
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
    bool picker_was_open = picker.is_open;
    picker.process();
    if(picker.is_open != picker_was_open) {
        user_closed_picker = true;
    }
    
    //TODO left here for debugging puporses.
    if(show_imgui_demo) ImGui::ShowDemoWindow(&show_imgui_demo);
    
    //Finishing setup.
    ImGui::EndFrame();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui control panel for this frame.
 */
void area_editor::process_gui_control_panel() {
    ImGui::BeginChild("panel");
    
    switch(state) {
    case EDITOR_STATE_MAIN: {
        process_gui_panel_main();
        break;
    } case EDITOR_STATE_INFO: {
        process_gui_panel_info();
        break;
    } case EDITOR_STATE_LAYOUT: {
        process_gui_panel_layout();
        break;
    } case EDITOR_STATE_MOBS: {
        process_gui_panel_mobs();
        break;
    } case EDITOR_STATE_PATHS: {
        process_gui_panel_paths();
        break;
    } case EDITOR_STATE_DETAILS: {
        process_gui_panel_details();
        break;
    } case EDITOR_STATE_REVIEW: {
        process_gui_panel_review();
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
void area_editor::process_gui_menu_bar() {
    if(ImGui::BeginMenuBar()) {
    
        if(ImGui::BeginMenu("Editor")) {
        
            if(ImGui::MenuItem("Load or create area...")) {
                open_area_picker();
            }
            
            if(ImGui::MenuItem("Show demo")) {
                show_imgui_demo = true;
            }
            
            if(ImGui::MenuItem("Quit")) {
                if(!check_new_unsaved_changes()) {
                    quick_play_area.clear();
                    leave();
                }
            }
            
            ImGui::EndMenu();
            
        }
        
        if(ImGui::BeginMenu("Help")) {
        
            if(ImGui::MenuItem("Help")) {
                string help_str =
                    "To create an area, start by drawing its layout. "
                    "For this, you draw the polygons that make up the "
                    "geometry of the area. These polygons cannot overlap, "
                    "and a polygon whose floor is higher than its neighbor's "
                    "makes a wall. After that, place objects where you want, "
                    "specify the carrying paths, add details, and try it out."
                    "\n\n"
                    "If you need more help on how to use the area editor, "
                    "check out the tutorial on\n" + AREA_EDITOR_TUTORIAL_URL;
                show_message_box(
                    game.display, "Help", "Area editor help",
                    help_str.c_str(), NULL, 0
                );
            }
            
            ImGui::EndMenu();
            
        }
        
        ImGui::EndMenuBar();
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui area details control panel for this frame.
 */
void area_editor::process_gui_panel_details() {
    ImGui::BeginChild("info");
    
    if(ImGui::Button("Back")) {
        sub_state = EDITOR_SUB_STATE_NONE;
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::TreeNode("Tree shadows")) {
    
        if(ImGui::Button("New")) {
            if(sub_state == EDITOR_SUB_STATE_NEW_SHADOW) {
                sub_state = EDITOR_SUB_STATE_NONE;
            } else {
                sub_state = EDITOR_SUB_STATE_NEW_SHADOW;
            }
        }
        
        if(selected_shadow) {
            ImGui::SameLine();
            if(ImGui::Button("Delete")) {
                if(!selected_shadow) {
                    status_text = "You have to select shadows to delete!";
                } else {
                    register_change("tree shadow deletion");
                    for(
                        size_t s = 0;
                        s < game.cur_area_data.tree_shadows.size();
                        ++s
                    ) {
                        if(
                            game.cur_area_data.tree_shadows[s] ==
                            selected_shadow
                        ) {
                            game.cur_area_data.tree_shadows.erase(
                                game.cur_area_data.tree_shadows.begin() + s
                            );
                            delete selected_shadow;
                            selected_shadow = NULL;
                            break;
                        }
                    }
                }
            }
        }
        
        if(selected_shadow) {
        
            ImGui::Button("...");
            
            ImGui::SameLine();
            ImGui::InputText("Bitmap", &selected_shadow->file_name);
            
            if(
                ImGui::DragFloat2("Center", (float*) &selected_shadow->center)
            ) {
                selected_shadow_transformation.set_center(
                    selected_shadow->center
                );
            }
            
            point old_size = selected_shadow->size;
            if(
                ImGui::DragFloat2("Size", (float*) &selected_shadow->size)
            ) {
                if(selected_shadow_transformation.keep_aspect_ratio) {
                    float ratio = old_size.x / old_size.y;
                    if(selected_shadow->size.x != old_size.x) {
                        selected_shadow->size.y =
                            selected_shadow->size.x / ratio;
                    } else {
                        selected_shadow->size.x =
                            selected_shadow->size.y * ratio;
                    }
                }
                selected_shadow_transformation.set_size(selected_shadow->size);
            }
            
            ImGui::Checkbox(
                "Keep aspect ratio",
                &selected_shadow_transformation.keep_aspect_ratio
            );
            
            if(ImGui::SliderAngle("Angle", &selected_shadow->angle, 0, 360)) {
                selected_shadow_transformation.set_angle(
                    selected_shadow->angle
                );
            }
            
            int opacity = selected_shadow->alpha;
            ImGui::SliderInt("Opacity", &opacity, 0, 255);
            selected_shadow->alpha = opacity;
            
            ImGui::DragFloat2(
                "Sway", (float*) &selected_shadow->sway, 0.1
            );
            
        }
        
        ImGui::TreePop();
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui area info control panel for this frame.
 */
void area_editor::process_gui_panel_info() {
    ImGui::BeginChild("info");
    
    if(ImGui::Button("Back")) {
        sub_state = EDITOR_SUB_STATE_NONE;
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::TreeNode("General")) {
    
        ImGui::InputText("Name", &game.cur_area_data.name);
        
        ImGui::InputText("Subtitle", &game.cur_area_data.subtitle);
        
        vector<string> weather_conditions;
        for(auto w : game.weather_conditions) {
            weather_conditions.push_back(w.first);
        }
        ImGui::Combo(
            "Weather", &game.cur_area_data.weather_name, weather_conditions
        );
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
    }
    
    if(ImGui::TreeNode("Background")) {
    
        if(ImGui::Button("...")) {
            FILE_DIALOG_RESULTS result = FILE_DIALOG_RES_SUCCESS;
            vector<string> f =
                prompt_file_dialog_locked_to_folder(
                    TEXTURES_FOLDER_PATH,
                    "Please choose the texture to use for the background.",
                    "*.*",
                    ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                    ALLEGRO_FILECHOOSER_PICTURES,
                    &result
                );
                
            switch(result) {
            case FILE_DIALOG_RES_WRONG_FOLDER: {
                //File doesn't belong to the folder.
                status_text = "The chosen image is not in the textures folder!";
                break;
            } case FILE_DIALOG_RES_CANCELED: {
                //User canceled.
                break;
            } case FILE_DIALOG_RES_SUCCESS: {
                game.cur_area_data.bg_bmp_file_name = f[0];
                break;
            }
            }
        }
        
        ImGui::SameLine();
        ImGui::InputText("Bitmap", &game.cur_area_data.bg_bmp_file_name);
        
        ImGui::ColorEdit4(
            "Color", (float*) &game.cur_area_data.bg_color,
            ImGuiColorEditFlags_NoInputs
        );
        
        ImGui::DragFloat("Distance", &game.cur_area_data.bg_dist);
        
        ImGui::DragFloat("Zoom", &game.cur_area_data.bg_bmp_zoom);
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
    }
    
    if(ImGui::TreeNode("Metadata")) {
    
        ImGui::InputText("Creator", &game.cur_area_data.creator);
        ImGui::InputText("Version", &game.cur_area_data.version);
        ImGui::InputText("Notes", &game.cur_area_data.notes);
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
    }
    
    if(ImGui::TreeNode("Gameplay")) {
        ImGui::InputText("Sprays", &game.cur_area_data.spray_amounts);
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui layout control panel for this frame.
 */
void area_editor::process_gui_panel_layout() {
    ImGui::BeginChild("main");
    
    if(ImGui::Button("Back")) {
        clear_selection();
        sub_state = EDITOR_SUB_STATE_NONE;
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::Button("New")) {
        clear_layout_drawing();
        if(sub_state == EDITOR_SUB_STATE_DRAWING) {
            cancel_layout_drawing();
        } else {
            sub_state = EDITOR_SUB_STATE_DRAWING;
        }
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Circle")) {
        clear_circle_sector();
        if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
            cancel_circle_sector();
        } else {
            sub_state = EDITOR_SUB_STATE_CIRCLE_SECTOR;
        }
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Delete")) {
        if(selected_sectors.empty()) {
            status_text = "You have to select sectors to delete!";
        } else {
            area_data* prepared_state = prepare_state();
            if(!remove_isolated_sectors()) {
                status_text = "Some of the sectors are not isolated!";
                forget_prepared_state(prepared_state);
            } else {
                status_text = "Deleted sectors.";
                clear_selection();
                register_change("sector removal", prepared_state);
            }
        }
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Sel filter")) {
        clear_selection();
        selection_filter =
            sum_and_wrap(selection_filter, 1, N_SELECTION_FILTERS);
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Sel none")) {
        clear_selection();
    }
    
    ImGui::Dummy(ImVec2(0, 16));
    
    sector* s_ptr = NULL;
    if(selected_sectors.size() == 1 || selection_homogenized) {
        s_ptr = *selected_sectors.begin();
        
        if(ImGui::TreeNode("Sector behavior")) {
        
            ImGui::DragFloat("Height", &s_ptr->z);
            
            ImGui::Dummy(ImVec2(0, 16));
            
            if(ImGui::TreeNode("Hazards")) {
            
                vector<string> all_hazards_list;
                for(auto h : game.hazards) {
                    all_hazards_list.push_back(h.first);
                }
                
                static string new_hazard_selected_name;
                static int selected_hazard_nr = 0;
                
                ImGui::Combo(
                    "##hazards", &new_hazard_selected_name, all_hazards_list
                );
                
                ImGui::SameLine();
                if(ImGui::Button("+")) {
                    sector* s_ptr = *selected_sectors.begin();
                    vector<string> list =
                        semicolon_list_to_vector(s_ptr->hazards_str);
                    if(
                        !new_hazard_selected_name.empty() &&
                        std::find(
                            list.begin(), list.end(), new_hazard_selected_name
                        ) == list.end()
                    ) {
                        register_change("hazard addition");
                        if(!s_ptr->hazards_str.empty()) {
                            s_ptr->hazards_str += ";";
                        }
                        s_ptr->hazards_str += new_hazard_selected_name;
                        homogenize_selected_sectors();
                        selected_hazard_nr = list.size();
                    }
                }
                
                ImGui::SameLine();
                if(ImGui::Button("-")) {
                    sector* s_ptr = *selected_sectors.begin();
                    vector<string> list =
                        semicolon_list_to_vector(s_ptr->hazards_str);
                    if(
                        selected_hazard_nr >= 0 &&
                        selected_hazard_nr < list.size()
                    ) {
                        register_change("hazard removal");
                        s_ptr->hazards_str.clear();
                        for(size_t h = 0; h < list.size(); ++h) {
                            if(h == selected_hazard_nr) continue;
                            s_ptr->hazards_str += list[h] + ";";
                        }
                        if(!s_ptr->hazards_str.empty()) {
                            //Remove the trailing semicolon.
                            s_ptr->hazards_str.pop_back();
                        }
                        selected_hazard_nr =
                            std::min(selected_hazard_nr, (int) list.size() - 2);
                        homogenize_selected_sectors();
                    }
                }
                
                ImGui::ListBox(
                    "Hazards", &selected_hazard_nr,
                    semicolon_list_to_vector(s_ptr->hazards_str),
                    4
                );
                
                ImGui::Checkbox("Floor only", &s_ptr->hazard_floor);
                
                ImGui::Dummy(ImVec2(0, 16));
                
                ImGui::TreePop();
            }
            
            if(ImGui::TreeNode("Advanced")) {
            
                vector<string> types_list;
                for(
                    size_t t = 0; t < game.sector_types.get_nr_of_types(); ++t
                ) {
                    types_list.push_back(game.sector_types.get_name(t));
                }
                int type = s_ptr->type;
                ImGui::Combo("Type", &type, types_list);
                s_ptr->type = type;
                
                if(
                    s_ptr->type == SECTOR_TYPE_BRIDGE ||
                    s_ptr->type == SECTOR_TYPE_BRIDGE_RAIL
                ) {
                
                    float bridge_height = s2f(s_ptr->tag);
                    ImGui::SetNextItemWidth(96.0f);
                    ImGui::DragFloat("Bridge height", &bridge_height);
                    s_ptr->tag = f2s(bridge_height);
                    
                }
                
                ImGui::Checkbox("Bottomless pit", &s_ptr->is_bottomless_pit);
                
                ImGui::Dummy(ImVec2(0, 16));
                
                ImGui::TreePop();
            }
            
            ImGui::Dummy(ImVec2(0, 16));
            
            ImGui::TreePop();
        }
        
        if(ImGui::TreeNode("Sector appearance")) {
        
            int texture_type = !s_ptr->fade;
            
            ImGui::RadioButton("Texture fader", &texture_type, 0);
            
            ImGui::RadioButton("Regular texture", &texture_type, 1);
            
            s_ptr->fade = texture_type == 0;
            
            if(!s_ptr->fade) {
            
                ImGui::Indent();
                
                if(ImGui::Button("Change")) {
                    vector<picker_item> suggestions;
                    
                    for(size_t s = 0; s < texture_suggestions.size(); ++s) {
                        suggestions.push_back(
                            picker_item(
                                texture_suggestions[s].name,
                                "",
                                texture_suggestions[s].bmp
                            )
                        );
                    }
                    picker.set(
                        suggestions, "Pick a texture",
                        std::bind(
                            &area_editor::pick_texture, this,
                            std::placeholders::_1,
                            std::placeholders::_2
                        ),
                        "Suggestions:"
                    );
                }
                
                ImGui::SameLine();
                ImGui::Text("%s", s_ptr->texture_info.file_name.c_str());
                
                ImGui::Unindent();
                
            }
            
            ImGui::Dummy(ImVec2(0, 16));
            
            if(ImGui::TreeNode("Texture effects")) {
            
                ImGui::DragFloat2(
                    "Offset", (float*) &s_ptr->texture_info.translation
                );
                
                ImGui::DragFloat2(
                    "Scale", (float*) &s_ptr->texture_info.scale, 0.01
                );
                
                ImGui::SliderAngle("Angle", &s_ptr->texture_info.rot, 0, 360);
                
                ImGui::ColorEdit4(
                    "Tint color", (float*) &s_ptr->texture_info.tint,
                    ImGuiColorEditFlags_NoInputs
                );
                
                bool octee_on =
                    sub_state == EDITOR_SUB_STATE_OCTEE;
                if(ImGui::Checkbox("On-canvas editing", &octee_on)) {
                    sub_state =
                        octee_on ?
                        EDITOR_SUB_STATE_OCTEE :
                        EDITOR_SUB_STATE_NONE;
                }
                
                if(octee_on) {
                
                    ImGui::Indent();
                    
                    int octee_mode_int = (int) octee_mode;
                    
                    ImGui::RadioButton(
                        "Change offset", &octee_mode_int,
                        (int) OCTEE_MODE_OFFSET
                    );
                    
                    ImGui::RadioButton(
                        "Change scale", &octee_mode_int,
                        (int) OCTEE_MODE_SCALE
                    );
                    
                    ImGui::RadioButton(
                        "Change angle", &octee_mode_int,
                        (int) OCTEE_MODE_ANGLE
                    );
                    
                    octee_mode = (OCTEE_MODES) octee_mode_int;
                    
                    ImGui::Unindent();
                    
                }
                
                ImGui::Dummy(ImVec2(0, 16));
                
                ImGui::TreePop();
            }
            
            if(ImGui::TreeNode("Sector mood")) {
            
                int sector_brightness = s_ptr->brightness;
                ImGui::SetNextItemWidth(180);
                ImGui::SliderInt("Brightness", &sector_brightness, 0, 255);
                s_ptr->brightness = sector_brightness;
                
                ImGui::Checkbox(
                    "Always cast shadow", &s_ptr->always_cast_shadow
                );
                
                ImGui::Dummy(ImVec2(0, 16));
                
                ImGui::TreePop();
            }
            
            ImGui::Dummy(ImVec2(0, 16));
            
            ImGui::TreePop();
        }
        
        homogenize_selected_sectors();
        
    } else if(selected_sectors.empty()) {
    
        ImGui::Text("No sector selected.");
        
    } else {
    
        ImGui::TextWrapped(
            "Multiple different sectors selected. To make all their properties "
            "the same and edit them all together, click here:"
        );
        
        if(ImGui::Button("Edit all together")) {
            register_change("sector combining");
            selection_homogenized = true;
            homogenize_selected_sectors();
        }
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui main control panel for this frame.
 */
void area_editor::process_gui_panel_main() {
    ImGui::BeginChild("main");
    
    ImGui::Text("Area: %s", cur_area_name.c_str());
    
    ImGui::Dummy(ImVec2(0, 16));
    
    if(ImGui::Button("Info")) {
        state = EDITOR_STATE_INFO;
    }
    
    if(ImGui::Button("Layout")) {
        state = EDITOR_STATE_LAYOUT;
    }
    
    if(ImGui::Button("Objects")) {
        state = EDITOR_STATE_MOBS;
    }
    
    if(ImGui::Button("Paths")) {
        state = EDITOR_STATE_PATHS;
    }
    
    if(ImGui::Button("Details")) {
        state = EDITOR_STATE_DETAILS;
    }
    
    if(ImGui::Button("Review")) {
        clear_problems();
        state = EDITOR_STATE_REVIEW;
    }
    
    if(ImGui::Button("Tools")) {
        update_backup_status();
        state = EDITOR_STATE_TOOLS;
    }
    
    if(ImGui::Button("Options")) {
        state = EDITOR_STATE_OPTIONS;
    }
    
    ImGui::Dummy(ImVec2(0, 16));
    
    if(ImGui::TreeNode("Stats")) {
        ImGui::Text(
            "Sectors: %i", (int) game.cur_area_data.sectors.size()
        );
        
        ImGui::Text(
            "Vertexes: %i", (int) game.cur_area_data.vertexes.size()
        );
        
        ImGui::Text(
            "Objects: %i", (int) game.cur_area_data.mob_generators.size()
        );
        
        ImGui::Text(
            "Path stops: %i", (int) game.cur_area_data.path_stops.size()
        );
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui mobs control panel for this frame.
 */
void area_editor::process_gui_panel_mobs() {
    ImGui::BeginChild("mobs");
    
    if(ImGui::Button("Back")) {
        sub_state = EDITOR_SUB_STATE_NONE;
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::Button("New")) {
        if(sub_state == EDITOR_SUB_STATE_NEW_MOB) {
            sub_state = EDITOR_SUB_STATE_NONE;
        } else {
            clear_selection();
            sub_state = EDITOR_SUB_STATE_NEW_MOB;
        }
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Delete")) {
        delete_selected_mobs();
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Duplicate")) {
        if(selected_mobs.empty()) {
            status_text = "You have to select mobs to duplicate!";
        } else if(sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB) {
            sub_state = EDITOR_SUB_STATE_NONE;
        } else {
            sub_state = EDITOR_SUB_STATE_DUPLICATE_MOB;
        }
    }
    
    ImGui::Dummy(ImVec2(0, 16));
    
    mob_gen* m_ptr = NULL;
    if(selected_mobs.size() == 1 || selection_homogenized) {
        m_ptr = *selected_mobs.begin();
        
        if(!m_ptr->category) {
            m_ptr->category = game.mob_categories.get(MOB_CATEGORY_NONE);
        }
        
        vector<string> categories;
        for(size_t c = 0; c < N_MOB_CATEGORIES; ++c) {
            categories.push_back(game.mob_categories.get(c)->plural_name);
        }
        int selected_category_nr = m_ptr->category->id;
        
        if(ImGui::Combo("Category", &selected_category_nr, categories)) {
            m_ptr->category = game.mob_categories.get(selected_category_nr);
            
            vector<string> type_names;
            m_ptr->category->get_type_names(type_names);
            
            m_ptr->type = NULL;
            if(!type_names.empty()) {
                m_ptr->type = m_ptr->category->get_type(type_names[0]);
            }
        }
        
        if(m_ptr->category->id != MOB_CATEGORY_NONE) {
        
            vector<string> types;
            m_ptr->category->get_type_names(types);
            for(size_t t = 0; t < types.size(); ) {
                mob_type* t_ptr = m_ptr->category->get_type(types[t]);
                if(t_ptr->appears_in_area_editor) {
                    ++t;
                } else {
                    types.erase(types.begin() + t);
                }
            }
            
            string selected_type_name;
            if(m_ptr->type) {
                selected_type_name = m_ptr->type->name;
            }
            if(ImGui::Combo("Type", &selected_type_name, types)) {
                m_ptr->type = m_ptr->category->get_type(selected_type_name);
            }
        }
        
        ImGui::SliderAngle("Angle", &m_ptr->angle, 0, 360);
        
        if(ImGui::TreeNode("Advanced")) {
        
            ImGui::InputText("Script vars", &m_ptr->vars);
            
            ImGui::Text(
                "%i link%s", (int) m_ptr->links.size(),
                m_ptr->links.size() == 1 ? "" : "s"
            );
            
            ImGui::SameLine();
            ImGui::Button("New");
            
            ImGui::SameLine();
            ImGui::Button("Delete");
            
            ImGui::TreePop();
        }
        
        homogenize_selected_mobs();
        
    } else if(selected_mobs.empty()) {
    
        ImGui::Text("No object selected.");
        
    } else {
    
        ImGui::TextWrapped(
            "Multiple different objects selected. To make all their properties "
            "the same and edit them all together, click here:"
        );
        
        if(ImGui::Button("Edit all together")) {
            register_change("object combining");
            selection_homogenized = true;
            homogenize_selected_mobs();
        }
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui options control panel for this frame.
 */
void area_editor::process_gui_panel_options() {
    ImGui::BeginChild("options");
    
    if(ImGui::Button("Save and go back")) {
        state = EDITOR_STATE_MAIN;
        save_options();
    }
    
    if(ImGui::TreeNode("Controls")) {
    
        int snap_threshold = (int) game.options.area_editor_snap_threshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Snap threshold", &snap_threshold,
            1, 0, 9999
        );
        game.options.area_editor_snap_threshold = snap_threshold;
        
        ImGui::Checkbox("Use MMB to pan", &game.options.editor_mmb_pan);
        
        int drag_threshold = (int) game.options.editor_mouse_drag_threshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Drag threshold", &drag_threshold,
            1, 0, 9999
        );
        game.options.editor_mouse_drag_threshold = drag_threshold;
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
        
    }
    
    if(ImGui::TreeNode("View")) {
    
        ImGui::Checkbox(
            "Show edge length", &game.options.area_editor_show_edge_length
        );
        
        ImGui::Checkbox(
            "Show territory", &game.options.area_editor_show_territory
        );
        
        int view_mode = game.options.area_editor_view_mode;
        ImGui::Text("View mode:");
        
        ImGui::RadioButton("Textures", &view_mode, VIEW_MODE_TEXTURES);
        
        ImGui::RadioButton("Wireframe", &view_mode, VIEW_MODE_WIREFRAME);
        
        ImGui::RadioButton("Heightmap", &view_mode, VIEW_MODE_HEIGHTMAP);
        
        ImGui::RadioButton("Brightness", &view_mode, VIEW_MODE_BRIGHTNESS);
        game.options.area_editor_view_mode = view_mode;
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
        
    }
    
    if(ImGui::TreeNode("Misc.")) {
    
        ImGui::Text(
            "Grid interval: %i", (int) game.options.area_editor_grid_interval
        );
        
        ImGui::SameLine();
        if(ImGui::Button("+")) {
            game.options.area_editor_grid_interval =
                std::min(
                    game.options.area_editor_grid_interval * 2.0f,
                    MAX_GRID_INTERVAL
                );
        }
        
        ImGui::SameLine();
        if(ImGui::Button("-")) {
            game.options.area_editor_grid_interval =
                std::max(
                    game.options.area_editor_grid_interval * 0.5f,
                    MIN_GRID_INTERVAL
                );
        }
        
        int backup_interval = game.options.area_editor_backup_interval;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Auto-backup interval", &backup_interval, 1, 0, 9999
        );
        game.options.area_editor_backup_interval = backup_interval;
        
        int undo_limit = game.options.area_editor_undo_limit;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Undo limit", &undo_limit, 1, 0, 9999
        );
        game.options.area_editor_undo_limit = undo_limit;
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui paths control panel for this frame.
 */
void area_editor::process_gui_panel_paths() {
    ImGui::BeginChild("paths");
    
    if(ImGui::Button("Back")) {
        sub_state = EDITOR_SUB_STATE_NONE;
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::Button("Draw")) {
        if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
            sub_state = EDITOR_SUB_STATE_NONE;
        } else {
            path_drawing_stop_1 = NULL;
            sub_state = EDITOR_SUB_STATE_PATH_DRAWING;
        }
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Delete")) {
        delete_selected_path_elements();
    }
    
    ImGui::Dummy(ImVec2(0, 16));
    
    ImGui::Text("Drawing mode:");
    
    int one_way_mode = path_drawing_normals;
    
    ImGui::RadioButton("One-way links", &one_way_mode, 0);
    
    ImGui::RadioButton("Normal links", &one_way_mode, 1);
    
    path_drawing_normals = one_way_mode;
    
    if(ImGui::TreeNode("Tools")) {
    
        ImGui::Checkbox("Show closest stop", &show_closest_stop);
        
        if(ImGui::Checkbox("Show calculated path", &show_path_preview)) {
            if(
                show_path_preview &&
                path_preview_checkpoints[0].x == LARGE_FLOAT
            ) {
                //No previous location. Place them on-camera.
                path_preview_checkpoints[0].x =
                    game.cam.pos.x - COMFY_DIST;
                path_preview_checkpoints[0].y =
                    game.cam.pos.y;
                path_preview_checkpoints[1].x =
                    game.cam.pos.x + COMFY_DIST;
                path_preview_checkpoints[1].y =
                    game.cam.pos.y;
            }
            path_preview_dist = calculate_preview_path();
        }
        
        if(show_path_preview) {
            ImGui::Text("Total distance: %f", path_preview_dist);
        }
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui review control panel for this frame.
 */
void area_editor::process_gui_panel_review() {
    ImGui::BeginChild("review");
    
    if(ImGui::Button("Back")) {
        sub_state = EDITOR_SUB_STATE_NONE;
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::TreeNode("Problem search")) {
    
        if(sub_state != EDITOR_SUB_STATE_TEXTURE_VIEW) {
        
            if(ImGui::Button("Search for problems")) {
                find_problems();
            }
            
            ImGui::Text("Problem found:");
            
            ImGui::TextWrapped("%s", problem_title.c_str());
            
            if(!problem_description.empty()) {
            
                ImGui::TextWrapped("%s", problem_description.c_str());
                
                if(ImGui::Button("Go to problem")) {
                    goto_problem();
                }
                
            }
            
        } else {
        
            ImGui::TextWrapped("Not available during area preview mode.");
            
        }
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
        
    }
    
    if(ImGui::TreeNode("Preview")) {
    
        static bool see_textures;
        if(ImGui::Checkbox("Preview area", &see_textures)) {
            clear_problems();
            if(see_textures) {
                sub_state = EDITOR_SUB_STATE_TEXTURE_VIEW;
            } else {
                sub_state = EDITOR_SUB_STATE_NONE;
            }
        }
        
        ImGui::Checkbox("Show tree shadows", &show_shadows);
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
        
    }
    
    if(ImGui::TreeNode("Cross-section")) {
    
        if(ImGui::Checkbox("Show cross-section", &show_cross_section)) {
            if(show_cross_section) {
                cross_section_window_start =
                    point(0.0f, 0.0f);
                cross_section_window_end =
                    point(canvas_br.x * 0.5, canvas_br.y * 0.5);
                cross_section_z_window_start =
                    point(
                        cross_section_window_end.x,
                        cross_section_window_start.y
                    );
                cross_section_z_window_end =
                    point(
                        cross_section_window_end.x + 48,
                        cross_section_window_end.y
                    );
            }
            
            if(
                show_cross_section &&
                cross_section_checkpoints[0].x == LARGE_FLOAT
            ) {
                //No previous location. Place them on-camera.
                cross_section_checkpoints[0].x =
                    game.cam.pos.x - COMFY_DIST;
                cross_section_checkpoints[0].y =
                    game.cam.pos.y;
                cross_section_checkpoints[1].x =
                    game.cam.pos.x + COMFY_DIST;
                cross_section_checkpoints[1].y =
                    game.cam.pos.y;
            }
        }
        
        ImGui::Checkbox("Show height grid", &show_cross_section_grid);
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui tools control panel for this frame.
 */
void area_editor::process_gui_panel_tools() {
    ImGui::BeginChild("tools");
    
    if(ImGui::Button("Back")) {
        sub_state = EDITOR_SUB_STATE_NONE;
        state = EDITOR_STATE_MAIN;
        save_reference();
    }
    
    if(ImGui::TreeNode("Reference image")) {
    
        string old_ref_file_name = reference_file_name;
        
        if(ImGui::Button("...")) {
            vector<string> f =
                prompt_file_dialog(
                    "",
                    "Please choose the bitmap to use for a reference.",
                    "*.*",
                    ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                    ALLEGRO_FILECHOOSER_PICTURES
                );
                
            if(!f.empty() && !f[0].empty()) {
                reference_file_name = f[0];
            }
        }
        
        ImGui::SameLine();
        ImGui::InputText("Bitmap", &reference_file_name);
        
        if(old_ref_file_name != reference_file_name) {
            update_reference();
        }
        
        point reference_center = reference_transformation.get_center();
        if(
            ImGui::DragFloat2("Center", (float*) &reference_center)
        ) {
            reference_transformation.set_center(
                reference_center
            );
        }
        
        point old_size = reference_transformation.get_size();
        point reference_size = old_size;
        if(
            ImGui::DragFloat2("Size", (float*) &reference_size)
        ) {
            if(reference_transformation.keep_aspect_ratio) {
                float ratio = old_size.x / old_size.y;
                if(reference_size.x != old_size.x) {
                    reference_size.y =
                        reference_size.x / ratio;
                } else {
                    reference_size.x =
                        reference_size.y * ratio;
                }
            }
            reference_transformation.set_size(reference_size);
        }
        
        ImGui::Checkbox(
            "Keep aspect ratio",
            &reference_transformation.keep_aspect_ratio
        );
        
        int opacity = reference_alpha;
        ImGui::SliderInt("Opacity", &opacity, 0, 255);
        reference_alpha = opacity;
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
        
    }
    
    if(ImGui::TreeNode("Misc.")) {
    
        if(ImGui::Button("Load auto-backup")) {
            if(can_load_backup) {
                if(!check_new_unsaved_changes()) {
                    load_backup();
                }
            }
        }
        
        static float resize_mult = 1.0f;
        ImGui::DragFloat("##resizeMult", &resize_mult, 0.01);
        
        ImGui::SameLine();
        if(ImGui::Button("Resize everything")) {
            if(resize_mult != 0.0f) {
                register_change("global resize");
                resize_everything(resize_mult);
                status_text = "Resized successfully.";
                resize_mult = 1.0f;
            } else {
                status_text = "Can't resize everything to size 0!";
            }
        }
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui status bar for this frame.
 */
void area_editor::process_gui_status_bar() {
    ImGui::Text("%s", status_text.c_str());
    
    ImGui::SameLine();
    float size = canvas_separator_x - ImGui::GetItemRectSize().x - 150.0f;
    ImGui::Dummy(ImVec2(size, 0));
    
    ImGui::SameLine();
    ImGui::Text(
        "%s, %s",
        box_string(f2s(game.mouse_cursor_w.x), 7).c_str(),
        box_string(f2s(game.mouse_cursor_w.y), 7).c_str()
    );
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui toolbar for this frame.
 */
void area_editor::process_gui_toolbar() {
    if(ImGui::Button("Quit")) {
        if(!check_new_unsaved_changes()) {
            leave();
        }
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Reload")) {
        if(can_reload) {
            if(!check_new_unsaved_changes()) {
                load_area(false);
            }
        }
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Save")) {
        save_area(false);
        clear_selection();
        state = EDITOR_STATE_MAIN;
        made_new_changes = false;
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Test")) {
        if(!save_area(false)) return;
        quick_play_area = cur_area_name;
        quick_play_cam_pos = game.cam.pos;
        quick_play_cam_z = game.cam.zoom;
        leave();
    }
    
    ImGui::SameLine(0, 16);
    if(ImGui::Button("Undo")) {
        undo();
    }
    
    if(!reference_file_name.empty()) {
    
        ImGui::SameLine();
        if(ImGui::Button("Ref")) {
            show_reference = !show_reference;
        }
        
        int reference_alpha_int = reference_alpha;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(48.0f);
        ImGui::SliderInt("##refAlpha", &reference_alpha_int, 0, 255, "");
        reference_alpha = reference_alpha_int;
        
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Snap")) {
        if(!is_shift_pressed) {
            snap_mode = sum_and_wrap(snap_mode, 1, N_SNAP_MODES);
        } else {
            snap_mode = sum_and_wrap(snap_mode, -1, N_SNAP_MODES);
        }
    }
}
