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
    ImGui::BeginChild("canvas", ImVec2(0, -16));
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
    ImGui::BeginChild("cp", ImVec2(200, 0));
    
    ImGui::Text("Editing area AAAAA");
    if(ImGui::Button("Load")) {
        //TODO
        cur_area_name = "Play";
        load_area(false);
    }
    ImGui::SameLine();
    ImGui::Button("Save");
    
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
