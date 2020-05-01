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

#include "../../game.h"
#include "../../imgui/imgui_impl_allegro5.h"


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
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground
    );
    
    //The menu bar.
    process_gui_menu_bar();
    
    //The two main columns that split the canvas and control panel.
    ImGui::Columns(2, "main");
    ImGui::NextColumn();
    
    if(canvas_separator_x == -1) {
        canvas_separator_x = game.win_w * 0.675;
        ImGui::SetColumnWidth(0, canvas_separator_x);
    } else {
        canvas_separator_x = ImGui::GetColumnOffset(1);
    }
    update_canvas_coordinates();
    
    //Do the control panel.
    process_gui_control_panel();
    
    //Finish the main window.
    ImGui::End();
    
    //TODO left here for debugging puporses.
    //ImGui::ShowDemoWindow();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui control panel for this frame.
 */
void area_editor::process_gui_control_panel() {
    ImGui::BeginChild("cp", ImVec2(200, 0));
    
    ImGui::Text("Editing area AAAAA");
    if(ImGui::Button("Quit")) {
        leave();
    }
    ImGui::SameLine();
    ImGui::Button("Load");
    ImGui::SameLine();
    ImGui::Button("Save");
    
    ImGui::EndChild();
}


/* ----------------------------------------------------------------------------
 * Processes the ImGui menu bar for this frame.
 */
void area_editor::process_gui_menu_bar() {
    if(ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Editor")) {
            ImGui::MenuItem("Quit"); //TODO
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}
