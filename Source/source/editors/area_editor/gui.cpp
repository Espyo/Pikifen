/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor Dear ImGui logic.
 */

#include "editor.h"

#include "../../functions.h"
#include "../../game.h"
#include "../../imgui/imgui_impl_allegro5.h"
#include "../../imgui/imgui_stdlib.h"
#include "../../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Processes ImGui for this frame.
 */
void area_editor::process_gui() {
    //TODO everything!
    
    //Initial setup.
    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();
    
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.win_w, game.win_h));
    ImGui::Begin(
        "Main", NULL,
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
        //TODO
        break;
    } case EDITOR_STATE_REVIEW: {
        //TODO
        break;
    } case EDITOR_STATE_TOOLS: {
        //TODO
        break;
    } case EDITOR_STATE_OPTIONS: {
        //TODO
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
            if(ImGui::MenuItem("Show demo")) {
                show_imgui_demo = true;
            }
            if(ImGui::MenuItem("Quit")) {
                //TODO check if there are unsaved changes.
                quick_play_area.clear();
                leave();
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
 * Processes the ImGui area info control panel for this frame.
 */
void area_editor::process_gui_panel_info() {
    ImGui::BeginChild("info");
    
    if(ImGui::Button("Back")) {
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::TreeNode("General")) {
    
        ImGui::InputText("Name", &game.cur_area_data.name);
        
        ImGui::InputText("Subtitle", &game.cur_area_data.subtitle);
        
        //TODO
        static int temp = 0;
        ImGui::Combo("Weather", &temp, "a\0b\0c\0");
        
        ImGui::Dummy(ImVec2(0, 16));
        
        ImGui::TreePop();
    }
    
    if(ImGui::TreeNode("Background")) {
    
        //TODO
        ImGui::Button("...##bg");
        
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
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::Button("New")) {
        //TODO
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Circle")) {
        //TODO
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Delete")) {
        //TODO
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Sel filter")) {
        //TODO
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Sel none")) {
        //TODO
    }
    
    ImGui::Dummy(ImVec2(0, 16));
    
    sector* s_ptr = NULL;
    if(!selected_sectors.empty()) {
        s_ptr = *selected_sectors.begin();
    }
    
    //TODO logic to select multiple at once.
    
    if(s_ptr) {
    
        if(ImGui::TreeNode("Sector behavior")) {
        
            ImGui::DragFloat("Height", &s_ptr->z);
            
            ImGui::Dummy(ImVec2(0, 16));
            
            if(ImGui::TreeNode("Hazards")) {
            
                //TODO
                
                ImGui::Checkbox("Floor only", &s_ptr->hazard_floor);
                
                ImGui::Dummy(ImVec2(0, 16));
                
                ImGui::TreePop();
            }
            
            if(ImGui::TreeNode("Advanced")) {
            
                //TODO
                int temp = 0;
                ImGui::Combo("Type", &temp, "Normal\0stuff\0");
                
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
            //TODO texture chooser.
            
            s_ptr->fade = texture_type == 0;
            
            ImGui::Dummy(ImVec2(0, 16));
            
            if(ImGui::TreeNode("Texture effects")) {
            
                ImGui::DragFloat2(
                    "Offset", (float*) &s_ptr->texture_info.translation
                );
                
                ImGui::DragFloat2(
                    "Scale", (float*) &s_ptr->texture_info.scale, 0.01
                );
                
                ImGui::SliderAngle("Angle", &s_ptr->texture_info.rot, 0, 359);
                
                ImGui::ColorEdit4(
                    "Tint color", (float*) &s_ptr->texture_info.tint,
                    ImGuiColorEditFlags_NoInputs
                );
                
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
        
    } else {
    
        ImGui::Text("No sector selected.");
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui main control panel for this frame.
 */
void area_editor::process_gui_panel_main() {
    ImGui::BeginChild("main");
    
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
        state = EDITOR_STATE_REVIEW;
    }
    
    if(ImGui::Button("Tools")) {
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
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::Button("New")) {
        //TODO
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Delete")) {
        //TODO
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Duplicate")) {
        //TODO
    }
    
    ImGui::Dummy(ImVec2(0, 16));
    
    mob_gen* m_ptr = NULL;
    if(!selected_mobs.empty()) {
        m_ptr = *selected_mobs.begin();
    }
    
    //TODO logic to select multiple at once.
    
    if(m_ptr) {
    
        string cat_name = (m_ptr->category ? m_ptr->category->plural_name : "");
        ImGui::Text("Category: %s", cat_name.c_str());
        
        //TODO mob type
        
        ImGui::SliderAngle("Angle", &m_ptr->angle, 0, 359);
        
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
        
    } else {
    
        ImGui::Text("No object selected.");
        
    }
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui paths control panel for this frame.
 */
void area_editor::process_gui_panel_paths() {
    ImGui::BeginChild("paths");
    
    if(ImGui::Button("Back")) {
        state = EDITOR_STATE_MAIN;
    }
    
    if(ImGui::Button("New")) {
        //TODO
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Delete")) {
        //TODO
    }
    
    ImGui::Dummy(ImVec2(0, 16));
    
    ImGui::Text("Drawing mode:");
    
    int one_way_mode = path_drawing_normals;
    
    ImGui::RadioButton("One-way links", &one_way_mode, 0);
    
    ImGui::RadioButton("Normal links", &one_way_mode, 1);
    
    path_drawing_normals = one_way_mode;
    
    if(ImGui::TreeNode("Tools")) {
    
        ImGui::Checkbox("Show closest stop", &show_closest_stop);
        
        ImGui::Checkbox("Show calculated path", &show_path_preview);
        
        if(show_path_preview) {
            ImGui::Text("Total distance: %f", 0.0f); //TODO
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
    ImGui::Text("");
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui toolbar for this frame.
 */
void area_editor::process_gui_toolbar() {
    if(ImGui::Button("Quit")) {
        leave();
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Reload")) {
        //TODO check if there are unsaved changes.
        load_area(false);
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Save")) {
        save_area(false);
        clear_selection();
        state = EDITOR_STATE_MAIN;
        //TODO change_to_right_frame();
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
    
    ImGui::SameLine();
    if(ImGui::Button("Ref")) {
        show_reference = !show_reference;
    }
    
    ImGui::SameLine();
    static int temp; //TODO;
    ImGui::VSliderInt("##refAlpha", ImVec2(12, 20), &temp, 0, 255, "");
    
    ImGui::SameLine();
    if(ImGui::Button("Snap")) {
        if(!is_shift_pressed) {
            snap_mode = sum_and_wrap(snap_mode, 1, N_SNAP_MODES);
        } else {
            snap_mode = sum_and_wrap(snap_mode, -1, N_SNAP_MODES);
        }
    }
}
