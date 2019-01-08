/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor GUI setup and logic.
 */

#include "editor.h"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_allegro5.h"
#include "../../vars.h"

using namespace std;


/* ----------------------------------------------------------------------------
 * Processes ImGui for this frame.
 */
void area_editor_imgui::process_gui() {
    //TODO
    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(scr_w, scr_h));
    ImGui::Begin("Main", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);

    if(ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Editor")) {
            ImGui::MenuItem("Quit");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::Columns(2, "main");

    ImGui::NextColumn();

    ImGui::BeginChild("cp", ImVec2(200, 0));

    ImGui::Text("Editing area AAAAA");
    if(ImGui::Button("Quit")) {
        exit(0);
    }
    ImGui::SameLine();
    ImGui::Button("Reload");
    ImGui::SameLine();
    ImGui::Button("Save");

    ImGui::EndChild();

    ImGui::End();
    
    ImGui::ShowDemoWindow(NULL);
}
