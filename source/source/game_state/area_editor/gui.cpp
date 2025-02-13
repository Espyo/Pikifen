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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../lib/imgui/imgui_stdlib.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/imgui_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Opens the "load" dialog.
 */
void area_editor::open_load_dialog() {
    reload_areas();
    
    //Set up the picker's behavior and data.
    vector<picker_item> areas;
    
    for(size_t a = 0; a < game.content.areas.list[AREA_TYPE_SIMPLE].size(); a++) {
        area_data* area_ptr = game.content.areas.list[AREA_TYPE_SIMPLE][a];
        content_manifest* man = area_ptr->manifest;
        areas.push_back(
            picker_item(
                area_ptr->name,
                "Pack: " + game.content.packs.list[man->pack].name,
                "Simple", (void*) man,
                get_folder_tooltip(man->path, ""),
                area_ptr->thumbnail.get()
            )
        );
    }
    for(size_t a = 0; a < game.content.areas.list[AREA_TYPE_MISSION].size(); a++) {
        area_data* area_ptr = game.content.areas.list[AREA_TYPE_MISSION][a];
        content_manifest* man = area_ptr->manifest;
        areas.push_back(
            picker_item(
                area_ptr->name,
                "Pack: " + game.content.packs.list[man->pack].name,
                "Mission", (void*) man,
                get_folder_tooltip(man->path, ""),
                area_ptr->thumbnail.get()
            )
        );
    }
    
    load_dialog_picker = picker_info(this);
    load_dialog_picker.items = areas;
    load_dialog_picker.pick_callback =
        std::bind(
            &area_editor::pick_area_folder, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5
        );
        
    //Open the dialog that will contain the picker and history.
    open_dialog(
        "Load an area or create a new one",
        std::bind(&area_editor::process_gui_load_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&area_editor::close_load_dialog, this);
}


/**
 * @brief Opens the "new" dialog.
 */
void area_editor::open_new_dialog() {
    open_dialog(
        "Create a new area",
        std::bind(&area_editor::process_gui_new_dialog, this)
    );
    dialogs.back()->custom_size = point(400, 0);
    dialogs.back()->close_callback = [this] () {
        new_dialog.pack.clear();
        new_dialog.internal_name = "my_area";
        new_dialog.type = AREA_TYPE_SIMPLE;
        new_dialog.area_path.clear();
        new_dialog.last_checked_area_path.clear();
        new_dialog.area_path_exists = false;
    };
}


/**
 * @brief Opens the options dialog.
 */
void area_editor::open_options_dialog() {
    open_dialog(
        "Options",
        std::bind(&area_editor::process_gui_options_dialog, this)
    );
    dialogs.back()->close_callback =
        std::bind(&area_editor::close_options_dialog, this);
}


/**
 * @brief Processes Dear ImGui for this frame.
 */
void area_editor::process_gui() {
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.win_w, game.win_h));
    ImGui::Begin(
        "Area editor", nullptr,
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
void area_editor::process_gui_control_panel() {
    ImGui::BeginChild("panel");
    
    //Basically, just show the correct panel for the current state.
    switch(state) {
    case EDITOR_STATE_MAIN: {
        process_gui_panel_main();
        break;
    } case EDITOR_STATE_INFO: {
        process_gui_panel_info();
        break;
    } case EDITOR_STATE_GAMEPLAY: {
        process_gui_panel_gameplay();
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
    }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui area deletion dialog for this frame.
 */
void area_editor::process_gui_delete_area_dialog() {
    //Explanation text.
    string explanation_str;
    if(!changes_mgr.exists_on_disk()) {
        explanation_str =
            "You have never saved this area to disk, so if you\n"
            "delete, you will only lose your unsaved progress.";
    } else {
        explanation_str =
            "If you delete, you will lose all unsaved progress,\n"
            "and the area's files on the disk will be gone FOREVER!";
    }
    ImGui::SetupCentering(ImGui::CalcTextSize(explanation_str.c_str()).x);
    ImGui::Text("%s", explanation_str.c_str());
    
    //Final warning text.
    string final_warning_str =
        "Are you sure you want to delete the current area?";
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
        delete_current_area();
    }
    ImGui::PopStyleColor(3);
}


/**
 * @brief Processes the Dear ImGui widgets regarding a grading criterion
 * for this frame.
 *
 * @param value_ptr Points to the value of the points value.
 * @param criterion_idx Criterion index.
 * @param widget_label Label for the main value widget.
 * @param tooltip Start of the tooltip for this criterion's value widget.
 */
void area_editor::process_gui_grading_criterion_widgets(
    int* value_ptr, MISSION_SCORE_CRITERIA criterion_idx,
    const string &widget_label, const string &tooltip
) {
    //Main value.
    ImGui::SetNextItemWidth(50);
    int points_int = *value_ptr;
    if(ImGui::DragInt(widget_label.c_str(), &points_int, 0.1f)) {
        register_change("mission grading change");
        *value_ptr = points_int;
    }
    set_tooltip(
        tooltip + "\n"
        "Negative numbers means the player loses points.\n"
        "0 means this criterion doesn't count.",
        "", WIDGET_EXPLANATION_DRAG
    );
    if(*value_ptr != 0) {
        ImGui::Indent();
        
        //Loss on fail checkbox.
        int flags = game.cur_area_data->mission.point_loss_data;
        if(
            ImGui::CheckboxFlags(
                ("0 points on fail##zpof" + i2s(criterion_idx)).c_str(),
                &flags,
                get_idx_bitmask(criterion_idx)
            )
        ) {
            register_change("mission grading change");
            game.cur_area_data->mission.point_loss_data = flags;
        }
        set_tooltip(
            "If checked, the player will receive 0 points for\n"
            "this criterion if they fail the mission."
        );
        
        //Use in HUD checkbox.
        flags = game.cur_area_data->mission.point_hud_data;
        if(
            ImGui::CheckboxFlags(
                ("Use in HUD counter##uihc" + i2s(criterion_idx)).c_str(),
                &flags, get_idx_bitmask(MISSION_SCORE_CRITERIA_PIKMIN_BORN)
            )
        ) {
            register_change("mission grading change");
            game.cur_area_data->mission.point_hud_data = flags;
        }
        set_tooltip(
            "If checked, the HUD item for the score counter will\n"
            "use this criterion in its calculation. If none of\n"
            "the criteria are used for the HUD item, then it\n"
            "won't even show up."
        );
        
        ImGui::Unindent();
    }
}


/**
 * @brief Processes the Dear ImGui widgets regarding a grading medal
 * requirements for this frame.
 *
 * @param requirement_ptr Points to the requirement value for this medal.
 * @param widget_label Label for the value widget.
 * @param widget_min_value Minimum value for the value widget.
 * @param widget_max_value Maximum value for the value widget.
 * @param tooltip Tooltip for the value widget.
 */
void area_editor::process_gui_grading_medal_widgets(
    int* requirement_ptr, const string &widget_label,
    int widget_min_value, int widget_max_value,
    const string &tooltip
) {
    //Requirement value.
    int req = *requirement_ptr;
    ImGui::SetNextItemWidth(90);
    if(
        ImGui::DragInt(
            widget_label.c_str(), &req, 1.0f, widget_min_value, widget_max_value
        )
    ) {
        register_change("mission grading change");
        *requirement_ptr = req;
    }
    set_tooltip(tooltip, "", WIDGET_EXPLANATION_DRAG);
}


/**
 * @brief Processes the Dear ImGui widgets regarding a grading mode
 * for this frame.
 *
 * @param value Internal value for this mode's radio button.
 * @param widget_label Label for the radio widget.
 * @param tooltip Tooltip for the radio widget.
 */
void area_editor::process_gui_grading_mode_widgets(
    int value, const string &widget_label, const string &tooltip
) {
    //Radio button.
    int mode = game.cur_area_data->mission.grading_mode;
    if(ImGui::RadioButton(widget_label.c_str(), &mode, value)) {
        register_change("mission grading change");
        game.cur_area_data->mission.grading_mode =
            (MISSION_GRADING_MODE) mode;
    }
    set_tooltip(tooltip);
}


/**
 * @brief Processes the Dear ImGui "load" dialog for this frame.
 */
void area_editor::process_gui_load_dialog() {
    //History node.
    process_gui_history(
    [this](const string &name) -> string {
        return name;
    },
    [this](const string &path) {
        close_top_dialog();
        load_area_folder(path, false, true);
    },
    [this](const string &path) {
        return get_folder_tooltip(path, "");
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
    set_tooltip("Create a new area.");
    
    //Load node.
    ImGui::Spacer();
    if(saveable_tree_node("load", "Load")) {
        load_dialog_picker.process();
        
        ImGui::TreePop();
    }
}


/**
 * @brief Processes the Dear ImGui "new" dialog for this frame.
 */
void area_editor::process_gui_new_dialog() {
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
        "Internal name of the new area.\n"
        "Remember to keep it simple, type in lowercase, and use underscores!"
    );
    
    //Simple area radio.
    ImGui::Spacer();
    ImGui::RadioButton("Simple area", &new_dialog.type, AREA_TYPE_SIMPLE);
    set_tooltip("Choose this to make your area a simple area.");
    
    //Mission area radio.
    ImGui::SameLine();
    ImGui::RadioButton("Mission", &new_dialog.type, AREA_TYPE_MISSION);
    set_tooltip("Choose this to make your area a mission area.");
    
    //Check if everything's ok.
    content_manifest temp_man;
    temp_man.pack = new_dialog.pack;
    temp_man.internal_name = new_dialog.internal_name;
    new_dialog.area_path =
        game.content.areas.manifest_to_path(
            temp_man, (AREA_TYPE) new_dialog.type
        );
    if(new_dialog.last_checked_area_path != new_dialog.area_path) {
        new_dialog.area_path_exists = folder_exists(new_dialog.area_path);
        new_dialog.last_checked_area_path = new_dialog.area_path;
    }
    
    if(new_dialog.internal_name.empty()) {
        problem = "You have to type an internal name first!";
    } else if(!is_internal_name_good(new_dialog.internal_name)) {
        problem =
            "The internal name should only have lowercase letters,\n"
            "numbers, and underscores!";
    } else if(new_dialog.area_path_exists) {
        problem =
            "There is already an area of that type with\n"
            "that internal name in that pack!";
    }
    
    //Create button.
    ImGui::Spacer();
    ImGui::SetupCentering(100);
    if(!problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create area", ImVec2(100, 40))) {
        hit_create_button = true;
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    set_tooltip(
        problem.empty() ? "Create the area!" : problem
    );
    
    //Creation logic.
    if(hit_create_button) {
        if(!problem.empty()) return;
        auto really_create = [ = ] () {
            create_area(new_dialog.area_path);
            close_top_dialog();
            close_top_dialog(); //Close the load dialog.
        };
        
        if(
            new_dialog.pack == FOLDER_NAMES::BASE_PACK &&
            !game.options.engine_developer
        ) {
            open_base_content_warning_dialog(really_create);
        } else {
            really_create();
        }
    }
}


/**
 * @brief Processes the Dear ImGui menu bar for this frame.
 */
void area_editor::process_gui_menu_bar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load or create area item.
            if(ImGui::MenuItem("Load or create...", "Ctrl+L")) {
                load_widget_pos = get_last_widget_pos();
                load_cmd(1.0f);
            }
            set_tooltip(
                "Pick an area to load, or create a new one.",
                "Ctrl + L"
            );
            
            //Reload current area item.
            if(ImGui::MenuItem("Reload current area")) {
                reload_widget_pos = get_last_widget_pos();
                reload_cmd(1.0f);
            }
            set_tooltip(
                "Lose all changes and reload the current area from the disk."
            );
            
            //Save current area item.
            if(ImGui::MenuItem("Save current area", "Ctrl+S")) {
                save_cmd(1.0f);
            }
            set_tooltip(
                "Save the area into the files on disk.",
                "Ctrl + S"
            );
            
            //Delete current area item.
            if(ImGui::MenuItem("Delete current area")) {
                delete_area_cmd(1.0f);
            }
            set_tooltip(
                "Delete the current area from the disk."
            );
            
            //Quick play item.
            if(ImGui::MenuItem("Quick play", "Ctrl+P")) {
                quick_play_cmd(1.0f);
            }
            set_tooltip(
                "Save, quit, and start playing the area. Leaving will return "
                "to the editor.",
                "Ctrl + P"
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
            
            //Debug menu.
            if(ImGui::BeginMenu("Debug")) {
            
                //Show edge indexes item.
                if(
                    ImGui::MenuItem(
                        "Show edge indexes", "F1", &debug_edge_idxs
                    )
                ) {
                    if(debug_edge_idxs) {
                        set_status("Enabled debug edge index display.");
                    } else {
                        set_status("Disabled debug edge index display.");
                    }
                }
                set_tooltip(
                    "Shows what index each edge is.\n"
                    "Mostly useful for debugging the engine."
                );
                
                //Show sector indexes item.
                if(
                    ImGui::MenuItem(
                        "Show sector indexes", "F2", &debug_sector_idxs
                    )
                ) {
                    if(debug_sector_idxs) {
                        set_status("Enabled debug sector index display.");
                    } else {
                        set_status("Disabled debug sector index display.");
                    }
                }
                set_tooltip(
                    "Shows the sector index on either side of an edge.\n"
                    "Mostly useful for debugging the engine."
                );
                
                //Show vertex indexes item.
                if(
                    ImGui::MenuItem(
                        "Show vertex indexes", "F3", &debug_vertex_idxs
                    )
                ) {
                    if(debug_vertex_idxs) {
                        set_status("Enabled debug vertex index display.");
                    } else {
                        set_status("Disabled debug vertex index display.");
                    }
                }
                set_tooltip(
                    "Shows what index each vertex is.\n"
                    "Mostly useful for debugging the engine."
                );
                
                //Show sector triangulation item.
                if(
                    ImGui::MenuItem(
                        "Show sector triangulation", "F4", &debug_triangulation
                    )
                ) {
                    if(debug_triangulation) {
                        set_status("Enabled debug triangulation display.");
                    } else {
                        set_status("Disabled debug triangulation display.");
                    }
                }
                set_tooltip(
                    "Shows what triangles make up the selected sector.\n"
                    "Mostly useful for debugging the engine."
                );
                
                //Show path indexes item.
                if(
                    ImGui::MenuItem(
                        "Show path indexes", "F5", &debug_path_idxs
                    )
                ) {
                    if(debug_path_idxs) {
                        set_status("Enabled debug path index display.");
                    } else {
                        set_status("Disabled debug path index display.");
                    }
                }
                set_tooltip(
                    "Shows what index each path stop is.\n"
                    "Mostly useful for debugging the engine."
                );
                
                ImGui::EndMenu();
                
            }
            
            //Quit editor item.
            if(ImGui::MenuItem("Quit", "Ctrl+Q")) {
                quit_widget_pos = get_last_widget_pos();
                quit_cmd(1.0f);
            }
            set_tooltip(
                "Quit the area editor.",
                "Ctrl + Q"
            );
            
            ImGui::EndMenu();
            
        }
        
        //Edit menu.
        if(ImGui::BeginMenu("Edit")) {
        
            //Undo item.
            if(ImGui::MenuItem("Undo", "Ctrl+Z")) {
                undo_cmd(1.0f);
            }
            string undo_text;
            if(undo_history.empty()) {
                undo_text = "Nothing to undo.";
            } else {
                undo_text = "Undo: " + undo_history.front().second + ".";
            }
            set_tooltip(
                undo_text,
                "Ctrl + Z"
            );
            
            //Redo item.
            if(ImGui::MenuItem("Redo", "Ctrl+Y")) {
                redo_cmd(1.0f);
            }
            string redo_text;
            if(redo_history.empty()) {
                redo_text =
                    "Nothing to redo.";
            } else {
                redo_text =
                    "Redo: " + redo_history.front().second + ".";
            }
            set_tooltip(
                redo_text,
                "Ctrl + Y"
            );
            
            //Separator.
            ImGui::Separator();
            
            //Copy properties item.
            if(ImGui::MenuItem("Copy properties", "Ctrl+C")) {
                copy_properties_cmd(1.0f);
            }
            set_tooltip(
                "Copies the properties of what you selected, if applicable.",
                "Ctrl + C"
            );
            
            //Paste properties item.
            if(ImGui::MenuItem("Paste properties", "Ctrl+V")) {
                paste_properties_cmd(1.0f);
            }
            set_tooltip(
                "Pastes previously-copied properties onto what you selected, "
                "if applicable.",
                "Ctrl + V"
            );
            
            if(
                state == EDITOR_STATE_LAYOUT &&
                sub_state == EDITOR_SUB_STATE_NONE
            ) {
                //Paste texture item.
                if(ImGui::MenuItem("Paste texture", "Ctrl+T")) {
                    paste_texture_cmd(1.0f);
                }
                set_tooltip(
                    "Pastes a previously-copied sector's texture onto "
                    "the sector you selected.",
                    "Ctrl + T"
                );
            }
            
            //Separator.
            ImGui::Separator();
            
            //Select all item.
            if(ImGui::MenuItem("Select all", "Ctrl+A")) {
                select_all_cmd(1.0f);
            }
            set_tooltip(
                "Selects everything in the current mode, if applicable.",
                "Ctrl + A"
            );
            
            //Delete item.
            if(ImGui::MenuItem("Delete", "Delete")) {
                delete_cmd(1.0f);
            }
            set_tooltip(
                "Deletes the selected things, if applicable.",
                "Delete"
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
                "Move and zoom the camera so that everything in the area\n"
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
                    "To create an area, start by drawing its layout. "
                    "For this, you draw the polygons that make up the "
                    "geometry of the area. These polygons cannot overlap, "
                    "and a polygon whose floor is higher than its neighbor's "
                    "makes a wall. After that, place objects where you want, "
                    "specify the carrying paths, add details, and try it out."
                    "\n\n"
                    "If you need more help on how to use the area editor, "
                    "check out the tutorial in the manual, located "
                    "in the engine's folder.";
                open_help_dialog(help_str, "area.html");
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
 * @brief Processes the Dear ImGui mob script vars for this frame.
 *
 * @param m_ptr Mob to process.
 */
void area_editor::process_gui_mob_script_vars(mob_gen* m_ptr) {
    if(!m_ptr->type) return;
    
    map<string, string> vars_map = get_var_map(m_ptr->vars);
    map<string, string> new_vars_map;
    map<string, bool> vars_in_widgets;
    
    //Start with the properties that apply to all objects.
    
    //Team property.
    string team_value;
    if(vars_map.find("team") != vars_map.end()) {
        team_value = vars_map["team"];
    }
    
    vector<string> team_names;
    team_names.push_back("(Default)");
    for(unsigned char t = 0; t < N_MOB_TEAMS; t++) {
        team_names.push_back(game.team_names[t]);
    }
    
    int team_nr;
    if(team_value.empty()) {
        team_nr = 0;
    } else {
        size_t team_nr_st = string_to_team_nr(team_value);
        if(team_nr_st == INVALID) {
            team_nr = 0;
        } else {
            //0 is reserved in this widget for "default".
            //Increase it by one to get the widget's team index number.
            team_nr = (int) team_nr_st + 1;
        }
    }
    
    if(ImGui::Combo("Team", &team_nr, team_names, 15)) {
        register_change("object script vars change");
        if(team_nr > 0) {
            //0 is reserved in this widget for "default".
            //Decrease it by one to get the real team index number.
            team_nr--;
            team_value = game.team_internal_names[team_nr];
        } else {
            team_value.clear();
        }
    }
    set_tooltip(
        "What sort of team this object belongs to.\n"
        "(Variable name: \"team\".)"
    );
    
    if(!team_value.empty()) new_vars_map["team"] = team_value;
    vars_in_widgets["team"] = true;
    
    //Health property.
    float max_health = m_ptr->type->max_health;
    if(vars_map.find("max_health") != vars_map.end()) {
        max_health = s2f(vars_map["max_health"]);
    }
    float health = max_health;
    if(vars_map.find("health") != vars_map.end()) {
        health = s2f(vars_map["health"]);
    }
    
    if(ImGui::DragFloat("Health", &health, 0.25f, 0.0f, max_health)) {
        register_change("object script vars change");
    }
    set_tooltip(
        "Starting health for this specific object.\n"
        "(Variable name: \"health\".)",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    if(health != max_health) {
        new_vars_map["health"] = f2s(health);
    }
    vars_in_widgets["health"] = true;
    
    //Max health property.
    if(ImGui::DragFloat("Max health", &max_health, 0.25f, 0.0f, FLT_MAX)) {
        register_change("object script vars change");
    }
    set_tooltip(
        "Maximum health for this specific object.\n"
        "The object type's default is " + f2s(m_ptr->type->max_health) + ".\n"
        "(Variable name: \"max_health\".)",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    if(max_health != m_ptr->type->max_health) {
        new_vars_map["max_health"] = f2s(max_health);
    }
    vars_in_widgets["max_health"] = true;
    
    //Now, dynamically create widgets for all properties this mob type has.
    
    for(size_t p = 0; p < m_ptr->type->area_editor_props.size(); p++) {
    
        mob_type::area_editor_prop_t* p_ptr =
            &m_ptr->type->area_editor_props[p];
            
        string value;
        if(vars_map.find(p_ptr->var) == vars_map.end()) {
            value = p_ptr->def_value;
        } else {
            value = vars_map[p_ptr->var];
        }
        
        switch(p_ptr->type) {
        case AEMP_TYPE_TEXT: {
    
            string value_s = value;
            if(ImGui::InputText(p_ptr->name.c_str(), &value_s)) {
                register_change("object script vars change");
                value = value_s;
            }
            
            break;
            
        } case AEMP_TYPE_INT: {
    
            int value_i = s2i(value);
            if(
                ImGui::DragInt(
                    p_ptr->name.c_str(), &value_i, 0.02f,
                    p_ptr->min_value, p_ptr->max_value
                )
            ) {
                register_change("object script vars change");
                value = i2s(value_i);
            }
            
            break;
            
        } case AEMP_TYPE_FLOAT: {
    
            float value_f = s2f(value);
            if(
                ImGui::DragFloat(
                    p_ptr->name.c_str(), &value_f, 0.1f,
                    p_ptr->min_value, p_ptr->max_value
                )
            ) {
                register_change("object script vars change");
                value = f2s(value_f);
            }
            
            break;
            
        } case AEMP_TYPE_BOOL: {
    
            bool value_b = s2b(value);
            if(ImGui::Checkbox(p_ptr->name.c_str(), &value_b)) {
                register_change("object script vars change");
                value = b2s(value_b);
            }
            
            break;
            
        } case AEMP_TYPE_LIST: {
    
            string value_s = value;
            if(ImGui::Combo(p_ptr->name, &value_s, p_ptr->value_list, 15)) {
                register_change("object script vars change");
                value = value_s;
            }
            
            break;
            
        } case AEMP_TYPE_NR_LIST: {
    
            int item_idx = s2i(value);
            if(ImGui::Combo(p_ptr->name, &item_idx, p_ptr->value_list, 15)) {
                register_change("object script vars change");
                value = i2s(item_idx);
            }
            
            break;
            
        }
        }
        
        set_tooltip(
            word_wrap(p_ptr->tooltip, 50) +
            (p_ptr->tooltip.empty() ? "" : "\n") +
            "(Variable name: \"" + p_ptr->var + "\".)",
            "",
            (p_ptr->type == AEMP_TYPE_INT || p_ptr->type == AEMP_TYPE_FLOAT) ?
            WIDGET_EXPLANATION_DRAG :
            WIDGET_EXPLANATION_NONE
        );
        
        if(value != p_ptr->def_value) {
            new_vars_map[p_ptr->var] = value;
        }
        
        vars_in_widgets[p_ptr->var] = true;
        
    }
    
    string other_vars_str;
    for(auto const &v : vars_map) {
        if(!vars_in_widgets[v.first]) {
            other_vars_str += v.first + "=" + v.second + ";";
        }
    }
    
    m_ptr->vars.clear();
    for(auto const &v : new_vars_map) {
        m_ptr->vars += v.first + "=" + v.second + ";";
    }
    m_ptr->vars += other_vars_str;
    
    if(!m_ptr->vars.empty() && m_ptr->vars[m_ptr->vars.size() - 1] == ';') {
        m_ptr->vars.pop_back();
    }
    
    //Finally, a widget for the entire list.
    string mob_vars = m_ptr->vars;
    ImGui::Spacer();
    if(mono_input_text("Full list", &mob_vars)) {
        register_change("object script vars change");
        m_ptr->vars = mob_vars;
    }
    set_tooltip(
        "This is the full list of script variables to use.\n"
        "You can add variables here, though variables in the "
        "wrong format will be removed.\n"
        "Format example: \"sleep=y;jumping=n\"."
    );
}


/**
 * @brief Processes the options dialog for this frame.
 */
void area_editor::process_gui_options_dialog() {
    //Controls node.
    if(saveable_tree_node("options", "Controls")) {
    
        //Snap threshold value.
        int snap_threshold = (int) game.options.area_editor_snap_threshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Snap threshold", &snap_threshold,
            0.1f, 0, INT_MAX
        );
        set_tooltip(
            "Cursor must be these many pixels close\n"
            "to a vertex/edge in order to snap there.\n"
            "Default: " +
            i2s(OPTIONS::DEF_AREA_EDITOR_SNAP_THRESHOLD) + ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.area_editor_snap_threshold = snap_threshold;
        
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
    
    //View node.
    ImGui::Spacer();
    if(saveable_tree_node("options", "View")) {
    
        //Show edge length checkbox.
        ImGui::Checkbox(
            "Show edge length", &game.options.area_editor_show_edge_length
        );
        set_tooltip(
            "Show the length of nearby edges when drawing or moving vertexes.\n"
            "Default: " +
            b2s(OPTIONS::DEF_AREA_EDITOR_SHOW_EDGE_LENGTH) + "."
        );
        
        //Show circular sector info checkbox.
        ImGui::Checkbox(
            "Show circular sector info",
            &game.options.area_editor_show_circular_info
        );
        set_tooltip(
            "Show the radius and number of vertexes of a circular sector\n"
            "when drawing one.\n"
            "Default: " +
            b2s(OPTIONS::DEF_AREA_EDITOR_SHOW_CIRCULAR_INFO) + "."
        );
        
        //Show path link length checkbox.
        ImGui::Checkbox(
            "Show path link length",
            &game.options.area_editor_show_path_link_length
        );
        set_tooltip(
            "Show the length of nearby path links when drawing or\n"
            "moving path stops.\n"
            "Default: " +
            b2s(OPTIONS::DEF_AREA_EDITOR_SHOW_PATH_LINK_LENGTH) + "."
        );
        
        //Show territory checkbox.
        ImGui::Checkbox(
            "Show territory/terrain radius",
            &game.options.area_editor_show_territory
        );
        set_tooltip(
            "Show the territory radius and terrain radius\n"
            "of the selected objects, when applicable.\n"
            "Default: " + b2s(OPTIONS::DEF_AREA_EDITOR_SHOW_TERRITORY) +
            "."
        );
        
        //View mode text.
        int view_mode = game.options.area_editor_view_mode;
        ImGui::Text("View mode:");
        
        ImGui::Indent();
        
        //Textures view mode radio button.
        ImGui::RadioButton("Textures", &view_mode, VIEW_MODE_TEXTURES);
        set_tooltip(
            "Draw textures on the sectors." +
            (string) (
                (
                    OPTIONS::DEF_AREA_EDITOR_VIEW_MODE ==
                    VIEW_MODE_TEXTURES
                ) ?
                "\nThis is the default." :
                ""
            )
        );
        
        //Wireframe view mode radio button.
        ImGui::RadioButton("Wireframe", &view_mode, VIEW_MODE_WIREFRAME);
        set_tooltip(
            "Do not draw sectors, only edges and vertexes.\n"
            "Best for performance." +
            (string) (
                (
                    OPTIONS::DEF_AREA_EDITOR_VIEW_MODE ==
                    VIEW_MODE_WIREFRAME
                ) ?
                "This is the default." :
                ""
            )
        );
        
        //Heightmap view mode radio button.
        ImGui::RadioButton("Heightmap", &view_mode, VIEW_MODE_HEIGHTMAP);
        set_tooltip(
            "Draw sectors as heightmaps. Lighter means taller." +
            (string) (
                (
                    OPTIONS::DEF_AREA_EDITOR_VIEW_MODE ==
                    VIEW_MODE_HEIGHTMAP
                ) ?
                "This is the default." :
                ""
            )
        );
        
        //Brightness view mode radio button.
        ImGui::RadioButton("Brightness", &view_mode, VIEW_MODE_BRIGHTNESS);
        set_tooltip(
            "Draw sectors as solid grays based on their brightness." +
            (string) (
                (
                    OPTIONS::DEF_AREA_EDITOR_VIEW_MODE ==
                    VIEW_MODE_BRIGHTNESS
                ) ?
                "This is the default." :
                ""
            )
        );
        game.options.area_editor_view_mode = (VIEW_MODE) view_mode;
        
        ImGui::Unindent();
        
        ImGui::TreePop();
        
    }
    
    ImGui::Spacer();
    
    process_gui_editor_style();
    
    ImGui::Spacer();
    
    //Misc. node.
    if(saveable_tree_node("options", "Misc.")) {
    
        //Interface mode text.
        ImGui::Text("Interface mode:");
        
        //Basic interface button.
        int interface_mode_i = (int) game.options.area_editor_advanced_mode;
        ImGui::Indent();
        ImGui::RadioButton("Basic", &interface_mode_i, 0);
        set_tooltip(
            "Only shows basic GUI items. Recommended for starters\n"
            "so that the interface isn't overwhelming. See the\n"
            "\"Advanced\" option's description for a list of such items."
        );
        
        //Advanced interface button.
        ImGui::RadioButton("Advanced", &interface_mode_i, 1);
        set_tooltip(
            "Shows and enables some advanced GUI items:\n"
            "- Toolbar buttons (and shortcut keys) to quickly swap "
            "modes with.\n"
            "- Toolbar button to toggle preview mode with."
        );
        ImGui::Unindent();
        game.options.area_editor_advanced_mode = (bool) interface_mode_i;
        
        //Selection transformation checkbox.
        ImGui::Checkbox(
            "Selection transformation", &game.options.area_editor_sel_trans
        );
        set_tooltip(
            "If true, when you select two or more vertexes, some handles\n"
            "will appear, allowing you to scale or rotate them together.\n"
            "Default: " + b2s(OPTIONS::DEF_AREA_EDITOR_SEL_TRANS) + "."
        );
        
        //Grid interval text.
        ImGui::Text(
            "Grid interval: %i", (int) game.options.area_editor_grid_interval
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
            "Default: " + i2s(OPTIONS::DEF_AREA_EDITOR_GRID_INTERVAL) +
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
            "Default: " + i2s(OPTIONS::DEF_AREA_EDITOR_GRID_INTERVAL) +
            ".",
            "Shift + Minus"
        );
        
        //Auto-backup interval value.
        int backup_interval = game.options.area_editor_backup_interval;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Auto-backup interval", &backup_interval, 1, 0, INT_MAX
        );
        set_tooltip(
            "Interval between auto-backup saves, in seconds. 0 = off.\n"
            "Default: " + i2s(OPTIONS::DEF_AREA_EDITOR_BACKUP_INTERVAL) +
            ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.area_editor_backup_interval = backup_interval;
        
        //Undo limit value.
        size_t old_undo_limit = game.options.area_editor_undo_limit;
        int undo_limit = (int) game.options.area_editor_undo_limit;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Undo limit", &undo_limit, 0.1, 0, INT_MAX
        );
        set_tooltip(
            "Maximum number of operations that can be undone. 0 = off.\n"
            "Default: " + i2s(OPTIONS::DEF_AREA_EDITOR_UNDO_LIMIT) + ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.area_editor_undo_limit = undo_limit;
        
        if(game.options.area_editor_undo_limit != old_undo_limit) {
            update_undo_history();
        }
        
        ImGui::Spacer();
        
        ImGui::TreePop();
        
    }
}


/**
 * @brief Processes the Dear ImGui area details control panel for this frame.
 */
void area_editor::process_gui_panel_details() {
    ImGui::BeginChild("details");
    
    if(sub_state == EDITOR_SUB_STATE_NEW_SHADOW) {
    
        //Creation explanation text.
        ImGui::TextWrapped(
            "Use the canvas to place a tree shadow. It'll appear where "
            "you click."
        );
        
        //Creation cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            set_status();
            sub_state = EDITOR_SUB_STATE_NONE;
        }
        set_tooltip(
            "Cancel the creation.",
            "Escape"
        );
        
    } else {
    
        //Back button.
        if(ImGui::Button("Back")) {
            change_state(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panel_title("DETAILS");
        
        //Tree shadows node.
        if(saveable_tree_node("details", "Tree shadows")) {
        
            //New tree shadow button.
            if(
                ImGui::ImageButton(
                    "newShadowButton", editor_icons[EDITOR_ICON_ADD],
                    point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                new_tree_shadow_cmd(1.0f);
            }
            set_tooltip(
                "Start creating a new tree shadow.\n"
                "Click on the canvas where you want the shadow to be.",
                "N"
            );
            
            //Delete shadow button.
            if(selected_shadow) {
                ImGui::SameLine();
                if(
                    ImGui::ImageButton(
                        "delShadowButton", editor_icons[EDITOR_ICON_REMOVE],
                        point(EDITOR::ICON_BMP_SIZE)
                    )
                ) {
                    delete_tree_shadow_cmd(1.0f);
                }
                set_tooltip(
                    "Delete the selected tree shadow.",
                    "Delete"
                );
            }
            
            ImGui::Spacer();
            
            if(selected_shadow) {
            
                //Choose the tree shadow image button.
                if(ImGui::Button("Choose image...")) {
                    open_bitmap_dialog(
                    [this] (const string &bmp) {
                        if(bmp != selected_shadow->bmp_name) {
                            //New image, delete the old one.
                            register_change("tree shadow image change");
                            if(selected_shadow->bitmap != game.bmp_error) {
                                game.content.bitmaps.list.free(
                                    selected_shadow->bmp_name
                                );
                            }
                            selected_shadow->bmp_name = bmp;
                            selected_shadow->bitmap =
                                game.content.bitmaps.list.get(
                                    selected_shadow->bmp_name, nullptr, false
                                );
                        }
                        set_status("Picked a tree shadow image successfully.");
                    },
                    FOLDER_NAMES::TEXTURES
                    );
                }
                set_tooltip(
                    "Choose which texture to use from the game's content."
                );
    
                //Tree shadow image name text.
                ImGui::SameLine();
                mono_text("%s", selected_shadow->bmp_name.c_str());
                
                //Tree shadow center value.
                point shadow_center = selected_shadow->center;
                if(
                    ImGui::DragFloat2("Center", (float*) &shadow_center)
                ) {
                    register_change("tree shadow center change");
                    selected_shadow->center = shadow_center;
                }
                set_tooltip(
                    "Center coordinates of the tree shadow.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Tree shadow size value.
                point shadow_size = selected_shadow->size;
                if(
                    process_gui_size_widgets(
                        "Size", shadow_size,
                        1.0f, selected_shadow_keep_aspect_ratio, false,
                        -FLT_MAX
                    )
                ) {
                    register_change("tree shadow size change");
                    selected_shadow->size = shadow_size;
                };
                set_tooltip(
                    "Width and height of the tree shadow.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Tree shadow aspect ratio checkbox.
                ImGui::Indent();
                ImGui::Checkbox(
                    "Keep aspect ratio",
                    &selected_shadow_keep_aspect_ratio
                );
                ImGui::Unindent();
                set_tooltip("Keep the aspect ratio when resizing the image.");
                
                //Tree shadow angle value.
                float shadow_angle = normalize_angle(selected_shadow->angle);
                if(ImGui::SliderAngle("Angle", &shadow_angle, 0, 360, "%.2f")) {
                    register_change("tree shadow angle change");
                    selected_shadow->angle = shadow_angle;
                }
                set_tooltip(
                    "Angle of the tree shadow.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Tree shadow opacity value.
                int shadow_opacity = selected_shadow->alpha;
                if(ImGui::SliderInt("Opacity", &shadow_opacity, 0, 255)) {
                    register_change("tree shadow opacity change");
                    selected_shadow->alpha = shadow_opacity;
                }
                set_tooltip(
                    "How opaque the tree shadow is.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Tree shadow sway value.
                point shadow_sway = selected_shadow->sway;
                if(ImGui::DragFloat2("Sway", (float*) &shadow_sway, 0.1)) {
                    register_change("tree shadow sway change");
                    selected_shadow->sway = shadow_sway;
                }
                set_tooltip(
                    "Multiply the amount of swaying by this much. 0 means "
                    "no swaying in that direction.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
            } else {
            
                //"No tree shadow selected" text.
                ImGui::TextDisabled("(No tree shadow selected)");
                
            }
            
            ImGui::TreePop();
            
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui edge control panel for this frame.
 */
void area_editor::process_gui_panel_edge() {
    edge* e_ptr = *selected_edges.begin();
    
    //Wall shadow node.
    if(saveable_tree_node("layout", "Wall shadow")) {
    
        //Length/presence text.
        ImGui::Text("Length and presence:");
        
        //Automatic length radio button.
        bool auto_length = (e_ptr->wall_shadow_length == LARGE_FLOAT);
        if(ImGui::RadioButton("Automatic length", auto_length)) {
            if(!auto_length) {
                register_change("edge shadow length change");
                e_ptr->wall_shadow_length = LARGE_FLOAT;
                quick_preview_timer.start();
            }
            auto_length = true;
        }
        set_tooltip(
            "The wall shadow's length will depend "
            "on the height of the wall.\n"
            "If it's too short, the wall shadow will also "
            "automatically disappear."
        );
        
        //Never show radio button.
        bool no_length = (e_ptr->wall_shadow_length == 0.0f);
        if(ImGui::RadioButton("Never show", no_length)) {
            if(!no_length) {
                register_change("edge shadow length change");
                e_ptr->wall_shadow_length = 0.0f;
                quick_preview_timer.start();
            }
            no_length = true;
        }
        set_tooltip(
            "The wall shadow will never appear, no matter what."
        );
        
        //Fixed length radio button.
        bool fixed_length = (!no_length && !auto_length);
        if(ImGui::RadioButton("Fixed length", fixed_length)) {
            if(!fixed_length) {
                register_change("edge shadow length change");
                e_ptr->wall_shadow_length = 30.0f;
                quick_preview_timer.start();
            }
            fixed_length = true;
        }
        set_tooltip(
            "The wall shadow will always appear, and will "
            "have a fixed length regardless of the wall's height."
        );
        
        //Length value.
        if(fixed_length) {
            float length = e_ptr->wall_shadow_length;
            if(
                ImGui::DragFloat(
                    "Length", &length, 0.2f,
                    GEOMETRY::SHADOW_MIN_LENGTH, GEOMETRY::SHADOW_MAX_LENGTH
                )
            ) {
                register_change("edge shadow length change");
                e_ptr->wall_shadow_length = length;
                quick_preview_timer.start();
            }
            set_tooltip(
                "Length of the shadow.",
                "", WIDGET_EXPLANATION_DRAG
            );
        }
        
        //Shadow color.
        ALLEGRO_COLOR color = e_ptr->wall_shadow_color;
        ImGui::Spacer();
        if(
            ImGui::ColorEdit4(
                "Color", (float*) &color,
                ImGuiColorEditFlags_NoInputs
            )
        ) {
            register_change("edge shadow color change");
            e_ptr->wall_shadow_color = color;
            quick_preview_timer.start();
        }
        set_tooltip(
            "Color of the shadow, opacity included. "
            "This is the color\n"
            "closest to the wall, since it becomes more "
            "transparent as it goes out."
        );
        
        ImGui::TreePop();
    }
    
    //Ledge smoothing node.
    ImGui::Spacer();
    if(saveable_tree_node("layout", "Ledge smoothing")) {
    
        //Length value.
        float length = e_ptr->ledge_smoothing_length;
        if(
            ImGui::DragFloat(
                "Length", &length, 0.2f,
                0.0f, GEOMETRY::SMOOTHING_MAX_LENGTH
            )
        ) {
            register_change("edge ledge smoothing length change");
            e_ptr->ledge_smoothing_length = length;
            quick_preview_timer.start();
        }
        set_tooltip(
            "Length of the ledge smoothing effect.\n"
            "Use this to make a ledge leading into a wall look more rounded.\n"
            "0 means there will be no effect.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Smoothing color.
        ALLEGRO_COLOR color = e_ptr->ledge_smoothing_color;
        ImGui::Spacer();
        if(
            ImGui::ColorEdit4(
                "Color", (float*) &color,
                ImGuiColorEditFlags_NoInputs
            )
        ) {
            register_change("edge ledge smoothing color change");
            e_ptr->ledge_smoothing_color = color;
            quick_preview_timer.start();
        }
        set_tooltip(
            "Color of the ledge smoothing effect, opacity included. "
            "This is the color\n"
            "closest to the edge, since it becomes more "
            "transparent as it goes out."
        );
        
        ImGui::TreePop();
    }
    
    homogenize_selected_edges();
    update_all_edge_offset_caches();
    
}


/**
 * @brief Processes the Dear ImGui area gameplay settings control panel for
 * this frame.
 */
void area_editor::process_gui_panel_gameplay() {
    ImGui::BeginChild("gameplay");
    
    switch(sub_state) {
    case EDITOR_SUB_STATE_MISSION_EXIT: {

        //Instructions text.
        ImGui::TextWrapped(
            "Use the handles on the canvas to control where the exit region is."
        );
        
        //Region center text.
        ImGui::Text(
            "Exit region center: %s,%s",
            f2s(game.cur_area_data->mission.goal_exit_center.x).c_str(),
            f2s(game.cur_area_data->mission.goal_exit_center.y).c_str()
        );
        
        //Region center text.
        ImGui::Text(
            "Exit region size: %s x %s",
            f2s(game.cur_area_data->mission.goal_exit_size.x).c_str(),
            f2s(game.cur_area_data->mission.goal_exit_size.y).c_str()
        );
        
        //Finish button.
        if(ImGui::Button("Finish")) {
            sub_state = EDITOR_SUB_STATE_NONE;
        }
        set_tooltip("Click here to finish.");
        
        break;
        
    }
    default: {

        //Back button.
        if(ImGui::Button("Back")) {
            change_state(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panel_title("GAMEPLAY");
        
        //Sprays node.
        ImGui::Spacer();
        if(saveable_tree_node("gameplay", "Starting sprays")) {
        
            map<string, string> spray_strs =
                get_var_map(game.cur_area_data->spray_amounts);
            for(size_t s = 0; s < game.config.spray_order.size(); s++) {
                string spray_internal_name =
                    game.config.spray_order[s]->manifest->internal_name;
                int amount = s2i(spray_strs[spray_internal_name]);
                ImGui::SetNextItemWidth(50);
                if(
                    ImGui::DragInt(
                        game.config.spray_order[s]->name.c_str(), &amount,
                        0.1, 0, INT_MAX
                    )
                ) {
                    register_change("area spray amounts change");
                    spray_strs[spray_internal_name] = i2s(amount);
                    game.cur_area_data->spray_amounts.clear();
                    for(auto const &v : spray_strs) {
                        game.cur_area_data->spray_amounts +=
                            v.first + "=" + v.second + ";";
                    }
                }
                set_tooltip(
                    "Starting amount of spray dosages to give the player.", "",
                    WIDGET_EXPLANATION_DRAG
                );
                
            }
            
            ImGui::TreePop();
        }
        
        ImGui::Spacer();
        
        if(game.cur_area_data->type == AREA_TYPE_MISSION) {
            process_gui_panel_mission();
        }
        
        break;
    }
    
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui area info control panel for this frame.
 */
void area_editor::process_gui_panel_info() {
    ImGui::BeginChild("info");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("INFO");
    
    //General node.
    if(saveable_tree_node("info", "General")) {
    
        //Area name input.
        string name = game.cur_area_data->name;
        if(ImGui::InputText("Name", &name)) {
            register_change("area name change");
            game.cur_area_data->name = name;
        }
        set_tooltip(
            "Name of the area."
        );
        
        //Area subtitle input.
        string subtitle = game.cur_area_data->subtitle;
        if(ImGui::InputText("Subtitle", &subtitle)) {
            register_change("area subtitle change");
            game.cur_area_data->subtitle = subtitle;
        }
        set_tooltip(
            "Subtitle, if any. Appears on the loading screen."
        );
        
        //Area description input.
        string description = game.cur_area_data->description;
        if(ImGui::InputText("Description", &description)) {
            register_change("area description change");
            game.cur_area_data->description = description;
        }
        set_tooltip(
            "A general description about the area, like what the player "
            "does here."
        );
        
        //Add area tags button.
        if(
            ImGui::Button(
                "+",
                ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())
            )
        ) {
            ImGui::OpenPopup("addTags");
        }
        set_tooltip(
            "Add tags from a list of recommended tags.\n"
            "You can still add your own, of course."
        );
        
        //Add area tags popup.
        if(popup("addTags")) {
        
            string new_tag;
            
            //Gameplay tags combo.
            vector<string> gameplay_tags = {
                "Standard",
                "Puzzle",
                "Short and sweet",
                "Exploration",
                "Battle",
                "Challenge",
                "Gimmick",
                "Role-playing",
                "Custom game mode",
            };
            int gameplay_tag_idx = -1;
            if(ImGui::Combo("Gameplay", &gameplay_tag_idx, gameplay_tags, 15)) {
                new_tag = gameplay_tags[gameplay_tag_idx];
            }
            
            //Theme tags combo.
            vector<string> theme_tags = {
                "Autumn",
                "Beach",
                "Cave",
                "Concrete",
                "Desert",
                "Forest",
                "Garden",
                "House",
                "Lakeside",
                "Man-made",
                "Metal",
                "Snow",
                "Swamp",
                "Tiles",
                "Toys",
            };
            int theme_tag_idx = -1;
            if(ImGui::Combo("Theme", &theme_tag_idx, theme_tags, 15)) {
                new_tag = theme_tags[theme_tag_idx];
            }
            
            //Misc. tags combo.
            vector<string> misc_tags = {
                "Art",
                "Technical",
                "Troll",
                "Tutorial",
            };
            int misc_tag_idx = -1;
            if(ImGui::Combo("Misc.", &misc_tag_idx, misc_tags, 15)) {
                new_tag = misc_tags[misc_tag_idx];
            }
            
            if(!new_tag.empty()) {
                register_change("area tags change");
                if(!game.cur_area_data->tags.empty()) {
                    game.cur_area_data->tags += "; ";
                }
                game.cur_area_data->tags += new_tag;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
        
        //Area tags input.
        ImGui::SameLine();
        string tags = game.cur_area_data->tags;
        if(ImGui::InputText("Tags", &tags)) {
            register_change("area tags change");
            game.cur_area_data->tags = tags;
        }
        set_tooltip(
            "Short keywords that describe the area, separated by semicolon.\n"
            "Example: \"Beach; Gimmick; Short and sweet\""
        );
        
        //Difficulty value.
        int difficulty = game.cur_area_data->difficulty;
        vector<string> difficulty_options = {
            "Not specified",
            "1 - Very easy",
            "2 - Easy",
            "3 - Medium",
            "4 - Hard",
            "5 - Very hard"
        };
        if(ImGui::Combo("Difficulty", &difficulty, difficulty_options, 15)) {
            register_change("difficulty change");
            game.cur_area_data->difficulty = difficulty;
        }
        set_tooltip(
            "How hard this area is. This is very subjective, and only\n"
            "serves as a way to tell players if this area is something\n"
            "relaxed and easy (1), or if it's something that only the\n"
            "most experienced Pikmin veterans can handle (5).\n"
            "Or anything in between.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        ImGui::TreePop();
    }
    
    //Ambiance node.
    ImGui::Spacer();
    if(saveable_tree_node("info", "Ambiance")) {
    
        //Preview song button.
        bool valid_song_selected =
            !game.cur_area_data->song_name.empty() &&
            game.cur_area_data->song_name != NONE_OPTION;
        bool previewing =
            !preview_song.empty();
        bool can_preview_selected_song =
            valid_song_selected &&
            preview_song != game.cur_area_data->song_name;
        bool can_stop_previewing =
            previewing &&
            (
                !valid_song_selected ||
                preview_song == game.cur_area_data->song_name
            );
        bool preview_button_valid =
            can_preview_selected_song || can_stop_previewing;
            
        if(!preview_button_valid) ImGui::BeginDisabled();
        
        if(
            ImGui::ImageButton(
                "previewSongButton",
                can_stop_previewing ?
                editor_icons[EDITOR_ICON_STOP] :
                editor_icons[EDITOR_ICON_PLAY],
                point(ImGui::GetTextLineHeight())
            )
        ) {
            if(can_preview_selected_song) {
                preview_song = game.cur_area_data->song_name;
                game.audio.set_current_song(preview_song);
                previewing = true;
            } else if(can_stop_previewing) {
                game.audio.set_current_song(AREA_EDITOR::SONG_NAME, false);
                preview_song.clear();
                previewing = false;
            }
        }
        
        if(!preview_button_valid) ImGui::EndDisabled();
        
        string preview_tooltip_str;
        if(previewing) {
            preview_tooltip_str +=
                "Currently previewing the song \"" +
                game.content.songs.list[preview_song].name +
                "\".\n";
        }
        if(can_preview_selected_song) {
            preview_tooltip_str +=
                "Click here to preview the song \"" +
                game.content.songs.list[game.cur_area_data->song_name].name +
                "\".";
        } else if(can_stop_previewing) {
            preview_tooltip_str +=
                "Click here to stop.";
        } else {
            preview_tooltip_str +=
                "If you select a song, you can click here to preview it.";
        }
        set_tooltip(preview_tooltip_str);
        
        //Music combobox.
        ImGui::SameLine();
        vector<string> song_internals;
        vector<string> song_names;
        song_internals.push_back("");
        song_names.push_back(NONE_OPTION);
        for(auto &s : game.content.songs.list) {
            song_internals.push_back(s.first);
            song_names.push_back(s.second.name);
        }
        string song_name = game.cur_area_data->song_name;
        if(ImGui::Combo("Song", &song_name, song_internals, song_names, 15)) {
            register_change("area song change");
            game.cur_area_data->song_name = song_name;
        }
        set_tooltip(
            "What song to play."
        );
        
        //Area weather combobox.
        vector<string> weather_cond_internals;
        vector<string> weather_cond_names;
        weather_cond_internals.push_back("");
        weather_cond_names.push_back(NONE_OPTION);
        for(auto &w : game.content.weather_conditions.list) {
            weather_cond_internals.push_back(w.first);
            weather_cond_names.push_back(w.second.name);
        }
        string weather_name = game.cur_area_data->weather_name;
        if(
            ImGui::Combo(
                "Weather", &weather_name,
                weather_cond_internals, weather_cond_names, 15
            )
        ) {
            register_change("area weather change");
            game.cur_area_data->weather_name = weather_name;
        }
        set_tooltip(
            "The weather condition to use."
        );
        
        ImGui::Spacer();
        
        bool has_time_limit = false;
        float mission_min = 0;
        if(game.cur_area_data->type == AREA_TYPE_MISSION) {
            if(
                game.cur_area_data->mission.goal == MISSION_GOAL_TIMED_SURVIVAL
            ) {
                has_time_limit = true;
                mission_min =
                    game.cur_area_data->mission.goal_amount / 60.0f;
            } else if(
                has_flag(
                    game.cur_area_data->mission.fail_conditions,
                    get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
                )
            ) {
                has_time_limit = true;
                mission_min =
                    game.cur_area_data->mission.fail_time_limit / 60.0f;
            }
        }
        int day_start_min = (int) game.cur_area_data->day_time_start;
        day_start_min = wrap_float(day_start_min, 0, 60 * 24);
        float day_speed = game.cur_area_data->day_time_speed;
        int day_end_min = (int) (day_start_min + mission_min * day_speed);
        day_end_min = wrap_float(day_end_min, 0, 60 * 24);
        
        //Area day time at start value.
        if(
            ImGui::DragTime2(
                "Start day time", &day_start_min, "h", "m", 23, 59
            )
        ) {
            register_change("day time change");
            game.cur_area_data->day_time_start = day_start_min;
            if(has_time_limit) {
                day_speed =
                    calculate_day_speed(
                        day_start_min, day_end_min, mission_min
                    );
                game.cur_area_data->day_time_speed = day_speed;
            }
        }
        set_tooltip(
            "Point of the (game world) day at which gameplay starts.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        if(has_time_limit) {
            //Area day time at end value.
            if(
                ImGui::DragTime2(
                    "End day time", &day_end_min, "h", "m", 23, 59
                )
            ) {
                register_change("day time change");
                day_speed =
                    calculate_day_speed(
                        day_start_min, day_end_min, mission_min
                    );
                game.cur_area_data->day_time_speed = day_speed;
            }
            set_tooltip(
                "Point of the (game world) day at which gameplay ends.\n"
                "Only applicable in missions with some sort of time limits.\n"
                "Set this to the same as the area start time to make\n"
                "the day time frozen.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
        } else {
        
            //Area day time speed value.
            ImGui::SetNextItemWidth(165);
            if(
                ImGui::DragFloat(
                    "Day time speed", &day_speed, 0.1f, 0.0f, FLT_MAX
                )
            ) {
                register_change("day time change");
                game.cur_area_data->day_time_speed = day_speed;
            }
            set_tooltip(
                "Speed at which the (game world) day passes.\n"
                "60 means 1 game-world-hour goes by in 1 real-world-minute.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
        }
        
        ImGui::TreePop();
    }
    
    //Thumbnail node.
    ImGui::Spacer();
    if(saveable_tree_node("info", "Thumbnail")) {
    
        //Thumbnail browse button.
        if(ImGui::Button("Browse...")) {
            vector<string> f =
                prompt_file_dialog(
                    "",
                    "Please choose an image to copy over and "
                    "use as the thumbnail.",
                    "*.jpg;*.png",
                    ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                    ALLEGRO_FILECHOOSER_PICTURES,
                    game.display
                );
                
            if(!f.empty() && !f[0].empty()) {
                register_change("area thumbnail change");
                remove_thumbnail();
                game.cur_area_data->load_thumbnail(f[0]);
                thumbnail_needs_saving = true;
                thumbnail_backup_needs_saving = true;
            }
            
        }
        set_tooltip(
            "Press the Browse... button to set the area's thumbnail from\n"
            "a file on your disk. When you save the area, the thumbnail\n"
            "gets saved into thumbnail.png in the area's folder, \n"
            "but the original file you selected with the\n"
            "Browse... button will be left untouched."
        );
        
        //Thumbnail remove button.
        if(ImGui::Button("Remove thumbnail")) {
            remove_thumbnail();
            thumbnail_needs_saving = true;
            thumbnail_backup_needs_saving = true;
        }
        set_tooltip(
            "Removes the current thumbnail, if any."
        );
        
        //Current thumbnail text.
        //This needs to come after everything else, because the previous buttons
        //could delete the bitmap after we already told Dear ImGui that it
        //would be drawing it.
        ImGui::Text("Current thumbnail:");
        
        if(!game.cur_area_data->thumbnail) {
            //No thumbnail text.
            ImGui::Text("None");
        } else {
            //Thumbnail image.
            point size =
                resize_to_box_keeping_aspect_ratio(
                    get_bitmap_dimensions(game.cur_area_data->thumbnail.get()),
                    point(200.0f)
                );
            ImGui::Image(game.cur_area_data->thumbnail.get(), size);
        }
        
        ImGui::TreePop();
    }
    
    //Background node.
    ImGui::Spacer();
    if(saveable_tree_node("info", "Background")) {
    
        //Choose background image button.
        if(ImGui::Button("Choose image...")) {
            open_bitmap_dialog(
            [this] (const string &bmp) {
                register_change("area background change");
                game.cur_area_data->bg_bmp_name = bmp;
                set_status("Picked a background image successfully.");
            },
            FOLDER_NAMES::TEXTURES
            );
        }
        set_tooltip(
            "Choose which background image to use from the game's content.\n"
            "This repeating texture can be seen when looking at the void."
        );
    
        //Background image name text.
        ImGui::SameLine();
        mono_text("%s", game.cur_area_data->bg_bmp_name.c_str());
        
        //Background color value.
        ALLEGRO_COLOR bg_color = game.cur_area_data->bg_color;
        if(
            ImGui::ColorEdit4(
                "Void color", (float*) &bg_color,
                ImGuiColorEditFlags_NoInputs
            )
        ) {
            register_change("area background color change");
            game.cur_area_data->bg_color = bg_color;
        }
        set_tooltip(
            "Set the color of the void. If you have a background image,\n"
            "this will appear below it."
        );
        
        //Background distance value.
        float bg_dist = game.cur_area_data->bg_dist;
        if(ImGui::DragFloat("Distance", &bg_dist)) {
            register_change("area background distance change");
            game.cur_area_data->bg_dist = bg_dist;
        }
        set_tooltip(
            "How far away the background texture is. "
            "Affects paralax scrolling.\n"
            "2 is a good value.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Background zoom value.
        float bg_bmp_zoom = game.cur_area_data->bg_bmp_zoom;
        if(ImGui::DragFloat("Zoom", &bg_bmp_zoom, 0.01)) {
            register_change("area background zoom change");
            game.cur_area_data->bg_bmp_zoom = bg_bmp_zoom;
        }
        set_tooltip(
            "Scale the texture by this amount.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        ImGui::TreePop();
    }
    
    //Metadata node.
    ImGui::Spacer();
    if(saveable_tree_node("info", "Metadata")) {
    
        //Maker input.
        string maker = game.cur_area_data->maker;
        if(ImGui::InputText("Maker", &maker)) {
            register_change("area maker change");
            game.cur_area_data->maker = maker;
        }
        set_tooltip(
            "Name (or nickname) of who made this area. "
            "Optional."
        );
        
        //Version input.
        string version = game.cur_area_data->version;
        if(mono_input_text("Version", &version)) {
            register_change("area version change");
            game.cur_area_data->version = version;
        }
        set_tooltip(
            "Version of the area, preferably in the \"X.Y.Z\" format. "
            "Optional."
        );
        
        //Maker notes input.
        string maker_notes = game.cur_area_data->maker_notes;
        if(ImGui::InputText("Maker notes", &maker_notes)) {
            register_change("area maker notes change");
            game.cur_area_data->maker_notes = maker_notes;
        }
        set_tooltip(
            "Extra notes or comments about the area for other makers to see. "
            "Optional."
        );
        
        //Notes input.
        string notes = game.cur_area_data->notes;
        if(ImGui::InputText("Notes", &notes)) {
            register_change("area notes change");
            game.cur_area_data->notes = notes;
        }
        set_tooltip(
            "Extra notes or comments of any kind. "
            "Optional."
        );
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui layout control panel for this frame.
 */
void area_editor::process_gui_panel_layout() {
    ImGui::BeginChild("main");
    
    if(sub_state == EDITOR_SUB_STATE_DRAWING) {
        //Drawing explanation text.
        ImGui::TextWrapped(
            "Use the canvas to draw your layout. Each click places a vertex. "
            "You either draw edges from one edge/vertex to another "
            "edge/vertex, or draw a sector's shape and finish on the "
            "starting vertex."
        );
        
        //Drawing cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            clear_layout_drawing();
            cancel_layout_drawing();
        }
        set_tooltip(
            "Cancel the drawing.",
            "Escape"
        );
        
    } else if(sub_state == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        //Drawing explanation text.
        ImGui::TextWrapped(
            "Use the canvas to draw a circle sector. First, click to choose "
            "the sector's center. Then, choose how large the circle is. "
            "Finally, choose how many edges it'll have."
        );
        
        //Drawing cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            clear_circle_sector();
            cancel_circle_sector();
        }
        set_tooltip(
            "Cancel the drawing.",
            "Escape"
        );
        
    } else if(sub_state == EDITOR_SUB_STATE_QUICK_HEIGHT_SET) {
        //Explanation text.
        ImGui::TextWrapped(
            "Move the cursor up or down to change the sector's height. "
            "Release the key to return to normal."
        );
        
    } else {
    
        //Back button.
        if(ImGui::Button("Back")) {
            change_state(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panel_title("LAYOUT");
        
        //New sector button.
        if(
            ImGui::ImageButton(
                "newSectorButton", editor_icons[EDITOR_ICON_ADD],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            layout_drawing_cmd(1.0f);
        }
        set_tooltip(
            "Start drawing a new sector.\n"
            "Click on the canvas to draw the lines that make up the sector.",
            "N"
        );
        
        //New circle sector button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "newCircleButton", editor_icons[EDITOR_ICON_ADD_CIRCLE_SECTOR],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            circle_sector_cmd(1.0f);
        }
        set_tooltip(
            "Start creating a new circular sector.\n"
            "Click on the canvas to set the center, then radius, then the "
            "number of edges.",
            "C"
        );
        
        //Delete edges button.
        if(!selected_edges.empty()) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "delEdgesButton", editor_icons[EDITOR_ICON_REMOVE],
                    point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                delete_edge_cmd(1.0f);
            }
            set_tooltip(
                "Delete the selected edges.\n"
                "Sectors without any edges left get deleted too.\n"
                "Sectors that would end up with edge gaps also get deleted.\n"
                "If you delete an edge between two sectors,\n"
                "the smallest will merge into the largest.",
                "Delete"
            );
        }
        
        //Selection filter button.
        ALLEGRO_BITMAP* sel_filter_bmp = nullptr;
        string sel_filter_description;
        switch(selection_filter) {
        case SELECTION_FILTER_VERTEXES: {
            sel_filter_bmp = editor_icons[EDITOR_ICON_VERTEXES];
            sel_filter_description = "vertexes only";
            break;
        } case SELECTION_FILTER_EDGES: {
            sel_filter_bmp = editor_icons[EDITOR_ICON_EDGES];
            sel_filter_description = "edges + vertexes";
            break;
        } case SELECTION_FILTER_SECTORS: {
            sel_filter_bmp = editor_icons[EDITOR_ICON_SECTORS];
            sel_filter_description = "sectors + edges + vertexes";
            break;
        } case N_SELECTION_FILTERS: {
            break;
        }
        }
        
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "selFilterButton", sel_filter_bmp,
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            selection_filter_cmd(1.0f);
        }
        set_tooltip(
            "Current selection filter: " + sel_filter_description + ".\n"
            "When selecting things in the canvas, only these will "
            "become selected.",
            "F or Shift + F"
        );
        
        //Clear selection button.
        if(
            !selected_sectors.empty() ||
            !selected_edges.empty() ||
            !selected_vertexes.empty()
        ) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "clearSelButton", editor_icons[EDITOR_ICON_SELECT_NONE],
                    point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                clear_selection();
            }
            set_tooltip(
                "Clear the selection.",
                "Escape"
            );
        }
        
        //Sectors/edges tabs.
        ImGui::Spacer();
        if(ImGui::BeginTabBar("tabTabs")) {
        
            //Sectors tab.
            if(ImGui::BeginTabItem("Sectors")) {
            
                if(layout_mode == LAYOUT_MODE_EDGES) {
                    //If the user homogenized the edges, then
                    //selection_homogenized is true. But the sectors aren't
                    //homogenized, so reset the variable back to false.
                    selection_homogenized = false;
                }
                
                layout_mode = LAYOUT_MODE_SECTORS;
                
                if(selected_sectors.size() == 1 || selection_homogenized) {
                    process_gui_panel_sector();
                    
                } else if(selected_sectors.empty()) {
                
                    //"No sector selected" text.
                    ImGui::TextDisabled("(No sector selected)");
                    
                } else {
                
                    //Non-homogenized sectors warning.
                    ImGui::TextWrapped(
                        "Multiple different sectors selected. "
                        "To make all their properties the same "
                        "and edit them all together, click here:"
                    );
                    
                    //Homogenize sectors button.
                    if(ImGui::Button("Edit all together")) {
                        register_change("sector combining");
                        selection_homogenized = true;
                        homogenize_selected_sectors();
                    }
                }
                
                ImGui::EndTabItem();
            }
            
            //Edges tab.
            if(ImGui::BeginTabItem("Edges", nullptr)) {
            
                layout_mode = LAYOUT_MODE_EDGES;
                
                if(selected_edges.size() == 1 || selection_homogenized) {
                    process_gui_panel_edge();
                    
                } else if(selected_edges.empty()) {
                
                    //"No edge selected" text.
                    ImGui::TextDisabled("(No edge selected)");
                    
                } else {
                
                    //Non-homogenized edges warning.
                    ImGui::TextWrapped(
                        "Multiple different edges selected. "
                        "To make all their properties the same "
                        "and edit them all together, click here:"
                    );
                    
                    //Homogenize edges button.
                    if(ImGui::Button("Edit all together")) {
                        register_change("edge combining");
                        selection_homogenized = true;
                        homogenize_selected_edges();
                    }
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui main control panel for this frame.
 */
void area_editor::process_gui_panel_main() {
    if(manifest.internal_name.empty() || !game.cur_area_data) return;
    
    ImGui::BeginChild("main");
    
    //Current folder header text.
    ImGui::Text("Folder: ");
    
    //Current folder text.
    ImGui::SameLine();
    mono_text("%s", manifest.internal_name.c_str());
    string folder_tooltip =
        get_folder_tooltip(manifest.path, game.cur_area_data->user_data_path) +
        "\n\n"
        "Folder state: ";
    if(!changes_mgr.exists_on_disk()) {
        folder_tooltip += "Not saved to disk yet!";
    } else if(changes_mgr.has_unsaved_changes()) {
        folder_tooltip += "You have unsaved changes.";
    } else {
        folder_tooltip += "Everything ok.";
    }
    set_tooltip(folder_tooltip);
    
    //Layout button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "layoutButton", editor_icons[EDITOR_ICON_SECTORS],
            point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Layout"
        )
    ) {
        change_state(EDITOR_STATE_LAYOUT);
    }
    set_tooltip(
        "Draw sectors (polygons) to create the area's layout."
    );
    
    //Objects button.
    if(
        ImGui::ImageButtonAndText(
            "mobsButton", editor_icons[EDITOR_ICON_MOBS],
            point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Objects"
        )
    ) {
        change_state(EDITOR_STATE_MOBS);
    }
    set_tooltip(
        "Change object settings and placements."
    );
    
    //Paths button.
    if(
        ImGui::ImageButtonAndText(
            "pathsButton", editor_icons[EDITOR_ICON_PATHS],
            point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Paths"
        )
    ) {
        change_state(EDITOR_STATE_PATHS);
    }
    set_tooltip(
        "Draw movement paths, and their stops."
    );
    
    //Details button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "detailsButton", editor_icons[EDITOR_ICON_DETAILS],
            point(EDITOR::ICON_BMP_SIZE),
            12.0f, "Details"
        )
    ) {
        change_state(EDITOR_STATE_DETAILS);
    }
    set_tooltip(
        "Edit misc. details, like tree shadows."
    );
    
    //Area info button.
    if(
        ImGui::ImageButtonAndText(
            "infoButton", editor_icons[EDITOR_ICON_INFO],
            point(EDITOR::ICON_BMP_SIZE),
            12.0f, "Info"
        )
    ) {
        change_state(EDITOR_STATE_INFO);
    }
    set_tooltip(
        "Set the area's name, weather, and other basic information here."
    );
    
    //Area gameplay settings button.
    if(
        ImGui::ImageButtonAndText(
            "gameplayButton", editor_icons[EDITOR_ICON_GAMEPLAY],
            point(EDITOR::ICON_BMP_SIZE),
            12.0f, "Gameplay settings"
        )
    ) {
        change_state(EDITOR_STATE_GAMEPLAY);
    }
    set_tooltip(
        "Specify how the player's gameplay experience in this area will be."
    );
    
    //Review button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "reviewButton", editor_icons[EDITOR_ICON_REVIEW],
            point(EDITOR::ICON_BMP_SIZE),
            8.0f, "Review"
        )
    ) {
        change_state(EDITOR_STATE_REVIEW);
    }
    set_tooltip(
        "Use this to make sure everything is okay with the area."
    );
    
    //Tools button.
    if(
        ImGui::ImageButtonAndText(
            "toolsButton", editor_icons[EDITOR_ICON_TOOLS],
            point(EDITOR::ICON_BMP_SIZE),
            8.0f, "Tools"
        )
    ) {
        change_state(EDITOR_STATE_TOOLS);
    }
    set_tooltip(
        "Special tools to help you make the area."
    );
    
    ImGui::Spacer();
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui mission control panel for this frame.
 */
void area_editor::process_gui_panel_mission() {

    float old_mission_survival_min =
        game.cur_area_data->mission.goal_amount / 60.0f;
    float old_mission_time_limit_min =
        game.cur_area_data->mission.fail_time_limit / 60.0f;
    bool day_duration_needs_update = false;
    
    //Mission goal node.
    if(saveable_tree_node("gameplay", "Mission goal")) {
    
        //Goal combobox.
        vector<string> goals_list;
        for(size_t g = 0; g < game.mission_goals.size(); g++) {
            goals_list.push_back(game.mission_goals[g]->get_name());
        }
        int mission_goal = game.cur_area_data->mission.goal;
        if(ImGui::Combo("Goal", &mission_goal, goals_list, 15)) {
            register_change("mission requirements change");
            game.cur_area_data->mission.goal_mob_idxs.clear();
            game.cur_area_data->mission.goal_amount = 1;
            game.cur_area_data->mission.goal = (MISSION_GOAL) mission_goal;
            if(
                game.cur_area_data->mission.goal ==
                MISSION_GOAL_TIMED_SURVIVAL
            ) {
                day_duration_needs_update = true;
            }
        }
        
        switch(game.cur_area_data->mission.goal) {
        case MISSION_GOAL_END_MANUALLY: {
    
            //Explanation text.
            ImGui::TextWrapped(
                "The player has no real goal. They just play until they have "
                "had enough, at which point they must end from the pause menu."
            );
            
            break;
            
        }
        case MISSION_GOAL_COLLECT_TREASURE: {
    
            process_gui_panel_mission_goal_ct();
            break;
            
        }
        case MISSION_GOAL_BATTLE_ENEMIES: {
    
            process_gui_panel_mission_goal_be();
            break;
            
        }
        case MISSION_GOAL_TIMED_SURVIVAL: {
    
            //Explanation text.
            ImGui::TextWrapped(
                "The player must survive for a certain amount of time."
            );
            
            //Time values.
            ImGui::Spacer();
            int total_seconds =
                (int) game.cur_area_data->mission.goal_amount;
            if(ImGui::DragTime2("Time", &total_seconds)) {
                register_change("mission requirements change");
                total_seconds = std::max(total_seconds, 1);
                game.cur_area_data->mission.goal_amount =
                    (size_t) total_seconds;
                day_duration_needs_update = true;
            }
            set_tooltip(
                "The total survival time.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            break;
            
        }
        case MISSION_GOAL_GET_TO_EXIT: {
    
            process_gui_panel_mission_goal_gte();
            break;
            
        }
        case MISSION_GOAL_GROW_PIKMIN: {
    
            //Explanation text.
            ImGui::TextWrapped(
                "The player must reach or surpass a certain number of "
                "total Pikmin."
            );
            
            //Pikmin amount value.
            ImGui::Spacer();
            int amount =
                (int) game.cur_area_data->mission.goal_amount;
            ImGui::SetNextItemWidth(80);
            if(ImGui::DragInt("Amount", &amount, 0.1f, 1, INT_MAX)) {
                register_change("mission requirements change");
                game.cur_area_data->mission.goal_amount =
                    (size_t) amount;
            }
            set_tooltip(
                "The total Pikmin amount requirement.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            break;
            
        }
        }
        
        ImGui::TreePop();
        
    }
    
    //Mission fail conditions node.
    ImGui::Spacer();
    if(saveable_tree_node("gameplay", "Mission fail conditions")) {
    
        process_gui_panel_mission_fail(&day_duration_needs_update);
        ImGui::TreePop();
    }
    
    //Mission grading node.
    ImGui::Spacer();
    if(saveable_tree_node("gameplay", "Mission grading")) {
    
        process_gui_panel_mission_grading();
        ImGui::TreePop();
        
    }
    
    if(day_duration_needs_update) {
        float day_start_min = game.cur_area_data->day_time_start;
        day_start_min = wrap_float(day_start_min, 0, 60 * 24);
        float day_speed = game.cur_area_data->day_time_speed;
        float old_mission_min = 0;
        size_t mission_seconds = 0;
        if(game.cur_area_data->mission.goal == MISSION_GOAL_TIMED_SURVIVAL) {
            old_mission_min = old_mission_survival_min;
            mission_seconds = game.cur_area_data->mission.goal_amount;
        } else {
            old_mission_min = old_mission_time_limit_min;
            mission_seconds = game.cur_area_data->mission.fail_time_limit;
        }
        float old_day_end_min = day_start_min + old_mission_min * day_speed;
        old_day_end_min = wrap_float(old_day_end_min, 0, 60 * 24);
        mission_seconds = std::max(mission_seconds, (size_t) 1);
        float new_mission_min = mission_seconds / 60.0f;
        game.cur_area_data->day_time_speed =
            calculate_day_speed(
                day_start_min, old_day_end_min, new_mission_min
            );
    }
    
}


/**
 * @brief Processes the Dear ImGui fail conditions part of the
 * mission control panel for this frame.
 *
 * @param day_duration_needs_update The variable that dictates whether the
 * day duration widget data later in the panel needs to be updated.
 */
void area_editor::process_gui_panel_mission_fail(
    bool* day_duration_needs_update
) {
    unsigned int fail_flags =
        (unsigned int) game.cur_area_data->mission.fail_conditions;
    bool fail_flags_changed = false;
    
    //Pause menu end checkbox.
    bool pause_menu_end_is_fail =
        game.cur_area_data->mission.goal != MISSION_GOAL_END_MANUALLY;
    ImGui::BeginDisabled();
    ImGui::CheckboxFlags(
        "End from pause menu",
        &fail_flags,
        get_idx_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
    );
    ImGui::EndDisabled();
    if(pause_menu_end_is_fail) {
        enable_flag(
            game.cur_area_data->mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
        set_tooltip(
            "Since reaching the mission goal automatically ends the\n"
            "mission as a clear, if the player can go to the pause menu\n"
            "and end there, then naturally they haven't reached the\n"
            "goal yet. So this method of ending has to always be a fail."
        );
    } else {
        disable_flag(
            game.cur_area_data->mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
        set_tooltip(
            "The current mission goal is \"end whenever you want\", so\n"
            "ending from the pause menu is the goal, not a fail condition."
        );
    }
    
    //Time limit checkbox.
    if(game.cur_area_data->mission.goal == MISSION_GOAL_TIMED_SURVIVAL) {
        disable_flag(
            fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        );
        disable_flag(
            game.cur_area_data->mission.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        );
        ImGui::BeginDisabled();
    }
    bool time_limit_changed =
        ImGui::CheckboxFlags(
            "Reach the time limit",
            &fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        );
    fail_flags_changed |= time_limit_changed;
    if(
        time_limit_changed &&
        has_flag(
            fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        *day_duration_needs_update = true;
    }
    if(game.cur_area_data->mission.goal == MISSION_GOAL_TIMED_SURVIVAL) {
        ImGui::EndDisabled();
        set_tooltip(
            "The mission's goal is to survive for a certain amount of\n"
            "time, so it doesn't make sense to have a time limit to\n"
            "fail with."
        );
    } else {
        set_tooltip(
            "The mission ends as a fail if the player spends a certain\n"
            "amount of time in the mission."
        );
    }
    
    if(
        has_flag(
            fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        //Time limit values.
        int seconds =
            (int) game.cur_area_data->mission.fail_time_limit;
        ImGui::Indent();
        if(ImGui::DragTime2("Time limit", &seconds)) {
            register_change("mission fail conditions change");
            seconds = std::max(seconds, 1);
            game.cur_area_data->mission.fail_time_limit = (size_t) seconds;
            *day_duration_needs_update = true;
        }
        set_tooltip(
            "Time limit that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    //Reaching too few Pikmin checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Reach too few Pikmin",
            &fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_TOO_FEW_PIKMIN)
        );
    set_tooltip(
        "The mission ends as a fail if the total Pikmin count reaches\n"
        "a certain amount or lower. 0 means this only happens with a\n"
        "total Pikmin extinction. This fail condition isn't forced\n"
        "because the player might still be able to reach the mission\n"
        "goal using leaders. Or because you may want to make a mission\n"
        "with no Pikmin in the first place (like a puzzle stage)."
    );
    
    if(
        has_flag(
            fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_TOO_FEW_PIKMIN)
        )
    ) {
        ImGui::Indent();
        
        //Pikmin amount value.
        int amount =
            (int) game.cur_area_data->mission.fail_too_few_pik_amount;
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Amount##fctfpa", &amount, 0.1f, 0, INT_MAX)) {
            register_change("mission fail conditions change");
            game.cur_area_data->mission.fail_too_few_pik_amount =
                (size_t) amount;
        }
        set_tooltip(
            "Pikmin amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        ImGui::Unindent();
    }
    
    //Reaching too many Pikmin checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Reach too many Pikmin",
            &fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_TOO_MANY_PIKMIN)
        );
    set_tooltip(
        "The mission ends as a fail if the total Pikmin count reaches\n"
        "a certain amount or higher."
    );
    
    if(
        has_flag(
            fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_TOO_MANY_PIKMIN)
        )
    ) {
        ImGui::Indent();
        
        //Pikmin amount value.
        int amount =
            (int) game.cur_area_data->mission.fail_too_many_pik_amount;
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Amount##fctmpa", &amount, 0.1f, 1, INT_MAX)) {
            register_change("mission fail conditions change");
            game.cur_area_data->mission.fail_too_many_pik_amount =
                (size_t) amount;
        }
        set_tooltip(
            "Pikmin amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        ImGui::Unindent();
    }
    
    //Losing Pikmin checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Lose Pikmin",
            &fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_LOSE_PIKMIN)
        );
    set_tooltip(
        "The mission ends as a fail if a certain amount of Pikmin die."
    );
    
    if(
        has_flag(
            fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_LOSE_PIKMIN)
        )
    ) {
        //Pikmin deaths value.
        int amount =
            (int) game.cur_area_data->mission.fail_pik_killed;
        ImGui::Indent();
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Deaths", &amount, 0.1f, 1, INT_MAX)) {
            register_change("mission fail conditions change");
            game.cur_area_data->mission.fail_pik_killed =
                (size_t) amount;
        }
        set_tooltip(
            "Pikmin death amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    //Taking damage checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Take damage",
            &fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_TAKE_DAMAGE)
        );
    set_tooltip(
        "The mission ends as a fail if any leader loses any health."
    );
    
    //Lose leaders checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Lose leaders",
            &fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_LOSE_LEADERS)
        );
    set_tooltip(
        "The mission ends as a fail if a certain amount of leaders get\n"
        "KO'd. This fail condition isn't forced because the\n"
        "player might still be able to reach the mission goal with the\n"
        "Pikmin. Or because you may want to make a really gimmicky\n"
        "automatic mission with no leaders."
    );
    
    if(
        has_flag(
            fail_flags,
            get_idx_bitmask(
                MISSION_FAIL_COND_LOSE_LEADERS
            )
        )
    ) {
        //Leader KOs value.
        int amount =
            (int) game.cur_area_data->mission.fail_leaders_kod;
        ImGui::Indent();
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("KOs", &amount, 0.1f, 1, INT_MAX)) {
            register_change("mission fail conditions change");
            game.cur_area_data->mission.fail_leaders_kod =
                (size_t) amount;
        }
        set_tooltip(
            "Leader KO amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    //Kill enemies checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Kill enemies",
            &fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_KILL_ENEMIES)
        );
    set_tooltip(
        "The mission ends as a fail if a certain amount of\n"
        "enemies get killed."
    );
    
    if(
        has_flag(
            fail_flags,
            get_idx_bitmask(MISSION_FAIL_COND_KILL_ENEMIES)
        )
    ) {
        //Enemy kills value.
        int amount =
            (int) game.cur_area_data->mission.fail_enemies_killed;
        ImGui::Indent();
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Kills", &amount, 0.1f, 1, INT_MAX)) {
            register_change("mission fail conditions change");
            game.cur_area_data->mission.fail_enemies_killed =
                (size_t) amount;
        }
        set_tooltip(
            "Enemy kill amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    if(fail_flags_changed) {
        register_change("mission fail conditions change");
        game.cur_area_data->mission.fail_conditions =
            (bitmask_8_t) fail_flags;
    }
    
    vector<MISSION_FAIL_COND> active_conditions;
    for(size_t c = 0; c < game.mission_fail_conds.size(); c++) {
        if(
            has_flag(
                game.cur_area_data->mission.fail_conditions,
                get_idx_bitmask(c)
            )
        ) {
            active_conditions.push_back((MISSION_FAIL_COND) c);
        }
    }
    
    if(!active_conditions.empty()) {
    
        //Primary HUD condition checkbox.
        ImGui::Spacer();
        bool show_primary =
            game.cur_area_data->mission.fail_hud_primary_cond != INVALID;
        if(ImGui::Checkbox("Show primary HUD element", &show_primary)) {
            register_change("mission fail conditions change");
            game.cur_area_data->mission.fail_hud_primary_cond =
                show_primary ?
                (size_t) active_conditions[0] :
                INVALID;
        }
        set_tooltip(
            "If checked, a large HUD element will appear showing\n"
            "the most important fail condition's information."
        );
        
        if(show_primary) {
            //Primary HUD condition combobox.
            int selected = 0;
            bool found = false;
            vector<string> cond_strings;
            for(size_t c = 0; c < active_conditions.size(); c++) {
                size_t cond_id = active_conditions[c];
                cond_strings.push_back(
                    game.mission_fail_conds[cond_id]->get_name()
                );
                if(
                    cond_id ==
                    game.cur_area_data->mission.fail_hud_primary_cond
                ) {
                    found = true;
                    selected = (int) c;
                }
            }
            if(!found) {
                game.cur_area_data->mission.fail_hud_secondary_cond = 0;
            }
            ImGui::Indent();
            if(
                ImGui::Combo(
                    "Primary condition", &selected, cond_strings, 15
                )
            ) {
                register_change("mission fail conditions change");
                game.cur_area_data->mission.fail_hud_primary_cond =
                    active_conditions[selected];
            }
            set_tooltip(
                "Failure condition to show in the primary HUD element."
            );
            ImGui::Unindent();
        }
        
        //Secondary HUD condition checkbox.
        bool show_secondary =
            game.cur_area_data->mission.fail_hud_secondary_cond != INVALID;
        if(ImGui::Checkbox("Show secondary HUD element", &show_secondary)) {
            register_change("mission fail conditions change");
            game.cur_area_data->mission.fail_hud_secondary_cond =
                show_secondary ?
                (size_t) active_conditions[0] :
                INVALID;
        }
        set_tooltip(
            "If checked, a smaller HUD element will appear showing\n"
            "some other fail condition's information."
        );
        
        if(show_secondary) {
            //Secondary HUD condition combobox.
            bool found = false;
            int selected = 0;
            vector<string> cond_strings;
            for(size_t c = 0; c < active_conditions.size(); c++) {
                size_t cond_id = active_conditions[c];
                cond_strings.push_back(
                    game.mission_fail_conds[cond_id]->get_name()
                );
                if(
                    cond_id ==
                    game.cur_area_data->mission.fail_hud_secondary_cond
                ) {
                    found = true;
                    selected = (int) c;
                }
            }
            if(!found) {
                game.cur_area_data->mission.fail_hud_secondary_cond = 0;
            }
            ImGui::Indent();
            if(
                ImGui::Combo(
                    "Secondary condition", &selected, cond_strings, 15
                )
            ) {
                register_change("mission fail conditions change");
                game.cur_area_data->mission.fail_hud_secondary_cond =
                    active_conditions[selected];
            }
            set_tooltip(
                "Failure condition to show in the secondary HUD element."
            );
            ImGui::Unindent();
        }
        
    } else {
        game.cur_area_data->mission.fail_hud_primary_cond = INVALID;
        game.cur_area_data->mission.fail_hud_secondary_cond = INVALID;
        
    }
}


/**
 * @brief Processes the Dear ImGui battle enemies goal part of the
 * mission control panel for this frame.
 */
void area_editor::process_gui_panel_mission_goal_be() {
    //Explanation text.
    ImGui::TextWrapped(
        "The player must defeat certain enemies, or all of them."
    );
    
    //Enemy requirements text.
    ImGui::Spacer();
    ImGui::Text("Enemy requirements:");
    
    int requires_all_option =
        game.cur_area_data->mission.goal_all_mobs ? 0 : 1;
        
    //All enemies requirement radio button.
    if(ImGui::RadioButton("All", &requires_all_option, 0)) {
        register_change("mission requirements change");
        game.cur_area_data->mission.goal_all_mobs =
            requires_all_option == 0;
    }
    set_tooltip(
        "Require the player to defeat all enemies "
        "in order to reach the goal."
    );
    
    //Specific enemies requirement radio button.
    ImGui::SameLine();
    if(
        ImGui::RadioButton("Specific ones", &requires_all_option, 1)
    ) {
        register_change("mission requirements change");
        game.cur_area_data->mission.goal_all_mobs =
            requires_all_option == 0;
    }
    set_tooltip(
        "Require the player to defeat specific enemies "
        "in order to reach the goal.\n"
        "You must specify which enemies these are."
    );
    
    if(!game.cur_area_data->mission.goal_all_mobs) {
    
        //Start mob selector mode button.
        if(ImGui::Button("Pick enemies...")) {
            change_state(EDITOR_STATE_MOBS);
            sub_state = EDITOR_SUB_STATE_MISSION_MOBS;
        }
        set_tooltip(
            "Click here to start picking which enemies do and\n"
            "do not belong to the required enemy list."
        );
        
    }
    
    //Total objects required text.
    size_t total_required = get_mission_required_mob_count();
    ImGui::Text("Total objects required: %lu", total_required);
}


/**
 * @brief Processes the Dear ImGui collect treasures goal part of the
 * mission control panel for this frame.
 */
void area_editor::process_gui_panel_mission_goal_ct() {
    //Explanation text.
    ImGui::TextWrapped(
        "The player must collect certain treasures, or all of them."
    );
    
    //Treasure requirements text.
    ImGui::Spacer();
    ImGui::Text("Treasure requirements:");
    
    int requires_all_option =
        game.cur_area_data->mission.goal_all_mobs ? 0 : 1;
        
    //All treasures requirement radio button.
    if(ImGui::RadioButton("All", &requires_all_option, 0)) {
        register_change("mission requirements change");
        game.cur_area_data->mission.goal_all_mobs =
            requires_all_option == 0;
    }
    set_tooltip(
        "Require the player to collect all treasures "
        "in order to reach the goal."
    );
    
    //Specific treasures requirement radio button.
    ImGui::SameLine();
    if(
        ImGui::RadioButton("Specific ones", &requires_all_option, 1)
    ) {
        register_change("mission requirements change");
        game.cur_area_data->mission.goal_all_mobs =
            requires_all_option == 0;
    }
    set_tooltip(
        "Require the player to collect specific treasures "
        "in order to reach the goal.\n"
        "You must specify which treasures these are."
    );
    
    if(!game.cur_area_data->mission.goal_all_mobs) {
    
        //Start mob selector mode button.
        if(ImGui::Button("Pick treasures...")) {
            change_state(EDITOR_STATE_MOBS);
            sub_state = EDITOR_SUB_STATE_MISSION_MOBS;
        }
        set_tooltip(
            "Click here to start picking which treasures, piles, and\n"
            "resources do and do not belong to the required\n"
            "treasure list."
        );
        
    }
    
    //Total objects required text.
    size_t total_required = get_mission_required_mob_count();
    ImGui::Text("Total objects required: %lu", total_required);
}


/**
 * @brief Processes the Dear ImGui get to exit goal part of the
 * mission control panel for this frame.
 */
void area_editor::process_gui_panel_mission_goal_gte() {
    //Explanation text.
    ImGui::TextWrapped(
        "The player must get a leader or all of them "
        "to the exit point."
    );
    
    //Start exit region selector mode button.
    ImGui::Spacer();
    if(ImGui::Button("Pick region...")) {
        sub_state = EDITOR_SUB_STATE_MISSION_EXIT;
    }
    set_tooltip(
        "Click here to start picking where the exit region is.\n"
    );
    
    //Region center text.
    ImGui::Text(
        "Exit region center: %s,%s",
        f2s(game.cur_area_data->mission.goal_exit_center.x).c_str(),
        f2s(game.cur_area_data->mission.goal_exit_center.y).c_str()
    );
    
    //Region center text.
    ImGui::Text(
        "Exit region size: %s x %s",
        f2s(game.cur_area_data->mission.goal_exit_size.x).c_str(),
        f2s(game.cur_area_data->mission.goal_exit_size.y).c_str()
    );
    
    //Leader requirements text.
    ImGui::Spacer();
    ImGui::Text("Leader requirements:");
    
    int requires_all_option =
        game.cur_area_data->mission.goal_all_mobs ? 0 : 1;
        
    //All leaders requirement radio button.
    if(ImGui::RadioButton("All", &requires_all_option, 0)) {
        register_change("mission requirements change");
        game.cur_area_data->mission.goal_all_mobs =
            requires_all_option == 0;
    }
    set_tooltip(
        "Require the player to bring all leaders to the exit\n"
        "region in order to reach the mission's goal."
    );
    
    //Specific leaders requirement radio button.
    ImGui::SameLine();
    if(
        ImGui::RadioButton("Specific ones", &requires_all_option, 1)
    ) {
        register_change("mission requirements change");
        game.cur_area_data->mission.goal_all_mobs =
            requires_all_option == 0;
    }
    set_tooltip(
        "Require the player to bring specific leaders to the exit\n"
        "region in order to reach the mission's goal.\n"
        "You must specify which leaders these are."
    );
    
    if(!game.cur_area_data->mission.goal_all_mobs) {
    
        //Start mob selector mode button.
        if(ImGui::Button("Pick leaders...")) {
            change_state(EDITOR_STATE_MOBS);
            sub_state = EDITOR_SUB_STATE_MISSION_MOBS;
        }
        set_tooltip(
            "Click here to start picking which leaders do and\n"
            "do not belong to the required leader list."
        );
        
    }
    
    //Total objects required text.
    size_t total_required = get_mission_required_mob_count();
    ImGui::Text("Total objects required: %lu", total_required);
}


/**
 * @brief Processes the Dear ImGui mission grading part of the
 * mission control panel for this frame.
 */
void area_editor::process_gui_panel_mission_grading() {
    //Grading mode text.
    ImGui::Text("Grading mode:");
    
    //Grading mode widgets.
    process_gui_grading_mode_widgets(
        0, "Points",
        "The player's final grade depends on how many points they\n"
        "got in different criteria."
    );
    
    ImGui::SameLine();
    process_gui_grading_mode_widgets(
        1, "Goal",
        "The player's final grade depends on whether they have reached\n"
        "the mission goal (platinum) or not (nothing)."
    );
    
    ImGui::SameLine();
    process_gui_grading_mode_widgets(
        2, "Participation",
        "The player's final grade depends on whether they have played\n"
        "the mission (platinum) or not (nothing)."
    );
    
    //Grading criterion widgets.
    if(
        game.cur_area_data->mission.grading_mode == MISSION_GRADING_MODE_POINTS
    ) {
    
        ImGui::Spacer();
        process_gui_grading_criterion_widgets(
            &game.cur_area_data->mission.points_per_pikmin_born,
            MISSION_SCORE_CRITERIA_PIKMIN_BORN,
            "Points per Pikmin born",
            "Amount of points that the player receives for each\n"
            "Pikmin born."
        );
        
        process_gui_grading_criterion_widgets(
            &game.cur_area_data->mission.points_per_pikmin_death,
            MISSION_SCORE_CRITERIA_PIKMIN_DEATH,
            "Points per Pikmin death",
            "Amount of points that the player receives for each\n"
            "Pikmin lost."
        );
        
        if(
            has_flag(
                game.cur_area_data->mission.fail_conditions,
                get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
            )
        ) {
            process_gui_grading_criterion_widgets(
                &game.cur_area_data->mission.points_per_sec_left,
                MISSION_SCORE_CRITERIA_SEC_LEFT,
                "Points per second left",
                "Amount of points that the player receives for each\n"
                "second of time left, from the mission's time limit."
            );
        }
        
        process_gui_grading_criterion_widgets(
            &game.cur_area_data->mission.points_per_sec_passed,
            MISSION_SCORE_CRITERIA_SEC_PASSED,
            "Points per second passed",
            "Amount of points that the player receives for each\n"
            "second of time that has passed."
        );
        
        process_gui_grading_criterion_widgets(
            &game.cur_area_data->mission.points_per_treasure_point,
            MISSION_SCORE_CRITERIA_TREASURE_POINTS,
            "Points per treasure point",
            "Amount of points that the player receives for each\n"
            "point gathered from treasures. Different treasures are worth\n"
            "different treasure points."
        );
        
        process_gui_grading_criterion_widgets(
            &game.cur_area_data->mission.points_per_enemy_point,
            MISSION_SCORE_CRITERIA_ENEMY_POINTS,
            "Points per enemy point",
            "Amount of points that the player receives for each\n"
            "enemy point. Different enemies are worth different\n"
            "points."
        );
        
        //Starting score value.
        ImGui::Spacer();
        int starting_points = game.cur_area_data->mission.starting_points;
        ImGui::SetNextItemWidth(60);
        if(ImGui::DragInt("Starting points", &starting_points, 1.0f)) {
            register_change("mission grading change");
            game.cur_area_data->mission.starting_points = starting_points;
        }
        set_tooltip(
            "Starting amount of points. It can be positive or negative.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Medal point requirements text.
        ImGui::Spacer();
        ImGui::Text("Medal point requirements:");
        
        //Medal point requirement widgets.
        process_gui_grading_medal_widgets(
            &game.cur_area_data->mission.bronze_req, "Bronze",
            INT_MIN, game.cur_area_data->mission.silver_req - 1,
            "To get a bronze medal, the player needs at least these\n"
            "many points. Fewer than this, and the player gets no medal."
        );
        
        process_gui_grading_medal_widgets(
            &game.cur_area_data->mission.silver_req, "Silver",
            game.cur_area_data->mission.bronze_req + 1,
            game.cur_area_data->mission.gold_req - 1,
            "To get a silver medal, the player needs at least these\n"
            "many points."
        );
        
        process_gui_grading_medal_widgets(
            &game.cur_area_data->mission.gold_req, "Gold",
            game.cur_area_data->mission.silver_req + 1,
            game.cur_area_data->mission.platinum_req - 1,
            "To get a gold medal, the player needs at least these\n"
            "many points."
        );
        
        process_gui_grading_medal_widgets(
            &game.cur_area_data->mission.platinum_req, "Platinum",
            game.cur_area_data->mission.gold_req + 1, INT_MAX,
            "To get a platinum medal, the player needs at least these\n"
            "many points."
        );
    }
}


/**
 * @brief Processes the Dear ImGui mob control panel for this frame.
 */
void area_editor::process_gui_panel_mob() {

    mob_gen* m_ptr = *selected_mobs.begin();
    
    //Category and type comboboxes.
    string custom_cat_name = "";
    if(m_ptr->type) custom_cat_name = m_ptr->type->custom_category_name;
    mob_type* type = m_ptr->type;
    
    if(process_gui_mob_type_widgets(&custom_cat_name, &type)) {
        register_change("object type change");
        m_ptr->type = type;
        last_mob_custom_cat_name = "";
        if(m_ptr->type) {
            last_mob_custom_cat_name = m_ptr->type->custom_category_name;
        }
        last_mob_type = m_ptr->type;
    }
    
    if(m_ptr->type) {
        //Tips text.
        ImGui::TextDisabled("(%s info & tips)", m_ptr->type->name.c_str());
        string full_str =
            "Internal object category: " + m_ptr->type->category->name + "\n" +
            word_wrap(m_ptr->type->description, 50);
        if(!m_ptr->type->area_editor_tips.empty()) {
            full_str +=
                "\n\n" +
                word_wrap(m_ptr->type->area_editor_tips, 50);
        }
        set_tooltip(full_str);
        
        if(m_ptr->type->area_editor_recommend_links_from) {
            if(m_ptr->links.empty()) {
                //No outgoing links warning.
                ImGui::PushStyleColor(
                    ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.05f, 1.0f)
                );
                ImGui::Text("Warning: no links from this mob!");
                ImGui::PopStyleColor();
                set_tooltip(
                    "Warning: you need to link this object to a different one\n"
                    "in order for it to work as intended!"
                );
            }
        }
        
        if(m_ptr->type->area_editor_recommend_links_to) {
            bool has_links_to = false;
            for(
                size_t m = 0;
                m < game.cur_area_data->mob_generators.size();
                m++
            ) {
                mob_gen* other_m_ptr = game.cur_area_data->mob_generators[m];
                for(size_t l = 0; l < other_m_ptr->links.size(); l++) {
                    if(other_m_ptr->links[l] == m_ptr) {
                        has_links_to = true;
                        break;
                    }
                }
                if(has_links_to) break;
            }
            if(!has_links_to) {
                //No incoming links warning.
                ImGui::PushStyleColor(
                    ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.05f, 1.0f)
                );
                ImGui::Text("Warning: no links to this mob!");
                ImGui::PopStyleColor();
                set_tooltip(
                    "Warning: you need to link a different object to this one\n"
                    "in order for it to work as intended!"
                );
            }
        }
        
        //If the mob type exists, obviously the missing mob type problem is
        //gone, if it was active.
        if(problem_type == EPT_TYPELESS_MOB) {
            clear_problems();
        }
    }
    
    //Object angle value.
    float mob_angle = normalize_angle(m_ptr->angle);
    ImGui::Spacer();
    if(ImGui::SliderAngle("Angle", &mob_angle, 0, 360, "%.2f")) {
        register_change("object angle change");
        m_ptr->angle = mob_angle;
    }
    set_tooltip(
        "Angle that the object is facing.\n"
        "You can also press R in the canvas to "
        "make it face the cursor.",
        "", WIDGET_EXPLANATION_SLIDER
    );
    
    //Object script vars node.
    ImGui::Spacer();
    if(saveable_tree_node("mobs", "Script vars")) {
    
        process_gui_mob_script_vars(m_ptr);
        
        ImGui::TreePop();
        
    }
    
    //Object advanced node.
    ImGui::Spacer();
    if(saveable_tree_node("mobs", "Advanced")) {
    
        if(m_ptr->stored_inside == INVALID) {
            //Store inside another mob button.
            if(ImGui::Button("Store inside...")) {
                sub_state = EDITOR_SUB_STATE_STORE_MOB_INSIDE;
            }
            set_tooltip(
                "If you want to store this object inside another object,\n"
                "click here to choose which object will do the storing.\n"
                "When that object dies, this one pops out."
            );
            
        } else {
        
            //Unstore button.
            if(ImGui::Button("Unstore")) {
                m_ptr->stored_inside = INVALID;
            }
            set_tooltip(
                "This object is currently stored inside another. Click here\n"
                "to unstore it and make it a regular object instead."
            );
        }
        
        //Object link amount text.
        ImGui::Spacer();
        ImGui::Text(
            "%i link%s", (int) m_ptr->links.size(),
            m_ptr->links.size() == 1 ? "" : "s"
        );
        
        //Object new link button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "newLinkButton", editor_icons[EDITOR_ICON_ADD],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            if(sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK) {
                sub_state = EDITOR_SUB_STATE_NONE;
            } else {
                sub_state = EDITOR_SUB_STATE_ADD_MOB_LINK;
            }
        }
        set_tooltip(
            "Start creating a new object link.\n"
            "Click on the other object you want to link to.",
            "Shift+L"
        );
        
        //Object delete link button.
        if(!(*selected_mobs.begin())->links.empty()) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "delLinkButton", editor_icons[EDITOR_ICON_REMOVE],
                    point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                if((*selected_mobs.begin())->links.size() == 1) {
                    register_change("Object link deletion");
                    m_ptr->links.erase(m_ptr->links.begin());
                    m_ptr->link_idxs.erase(m_ptr->link_idxs.begin());
                    homogenize_selected_mobs();
                } else if(sub_state == EDITOR_SUB_STATE_DEL_MOB_LINK) {
                    sub_state = EDITOR_SUB_STATE_NONE;
                } else {
                    sub_state = EDITOR_SUB_STATE_DEL_MOB_LINK;
                }
            }
            set_tooltip(
                "Delete an object link.\n"
                "If there is only one, it gets deleted automatically.\n"
                "Otherwise, you must click on the other object whose\n"
                "link you want to delete, or click the link proper."
            );
        }
        
        ImGui::TreePop();
    }
    
    homogenize_selected_mobs();
    
}


/**
 * @brief Processes the Dear ImGui mobs control panel for this frame.
 */
void area_editor::process_gui_panel_mobs() {
    ImGui::BeginChild("mobs");
    
    if(sub_state == EDITOR_SUB_STATE_NEW_MOB) {
    
        //Creation explanation text.
        ImGui::TextWrapped(
            "Use the canvas to place an object. It'll appear where you click."
        );
        
        //Creation cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            set_status();
            sub_state = EDITOR_SUB_STATE_NONE;
        }
        set_tooltip(
            "Cancel the creation.",
            "Escape"
        );
        
    } else if(sub_state == EDITOR_SUB_STATE_DUPLICATE_MOB) {
    
        //Duplication explanation text.
        ImGui::TextWrapped(
            "Use the canvas to place the new duplicated object(s). "
            "It/they will appear where you click."
        );
        
        //Duplication cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            set_status();
            sub_state = EDITOR_SUB_STATE_NONE;
        }
        set_tooltip(
            "Cancel the duplication.",
            "Escape"
        );
        
    } else if(sub_state == EDITOR_SUB_STATE_STORE_MOB_INSIDE) {
    
        //Storing process explanation text.
        ImGui::TextWrapped(
            "Use the canvas to link to an object. Click on the object you "
            "want this one to be stored inside of."
        );
        
        //Storing process cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            set_status();
            sub_state = EDITOR_SUB_STATE_NONE;
        }
        set_tooltip(
            "Cancel the storing process.",
            "Escape"
        );
        
    } else if(sub_state == EDITOR_SUB_STATE_ADD_MOB_LINK) {
    
        //Link addition explanation text.
        ImGui::TextWrapped(
            "Use the canvas to link to an object. Click on the object you "
            "want this one to link to."
        );
        
        //Link addition cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            set_status();
            sub_state = EDITOR_SUB_STATE_NONE;
        }
        set_tooltip(
            "Cancel the linking.",
            "Escape"
        );
        
    } else if(sub_state == EDITOR_SUB_STATE_DEL_MOB_LINK) {
    
        //Link deletion explanation text.
        ImGui::TextWrapped(
            "Use the canvas to delete an object link. Click on a linked object "
            "or on its link to delete the corresponding link."
        );
        
        //Link deletion cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            set_status();
            sub_state = EDITOR_SUB_STATE_NONE;
        }
        set_tooltip(
            "Cancel the link removal.",
            "Escape"
        );
        
    } else if(sub_state == EDITOR_SUB_STATE_MISSION_MOBS) {
    
        string cat_name =
            game.cur_area_data->mission.goal ==
            MISSION_GOAL_COLLECT_TREASURE ?
            "treasure/pile/resource" :
            game.cur_area_data->mission.goal ==
            MISSION_GOAL_BATTLE_ENEMIES ?
            "enemy" :
            "leader";
            
        //Instructions text.
        ImGui::TextWrapped(
            "Click a %s object to mark or unmark it as a required "
            "object for the mission. Objects flashing yellow are considered "
            "required. Click the finish button when you are done.",
            cat_name.c_str()
        );
        
        //Total objects required text.
        ImGui::Text(
            "Total objects required: %lu",
            game.cur_area_data->mission.goal_mob_idxs.size()
        );
        
        //Finish button.
        if(ImGui::Button("Finish")) {
            change_state(EDITOR_STATE_GAMEPLAY);
        }
        set_tooltip("Click here to finish.");
        
    } else {
    
        //Back button.
        if(ImGui::Button("Back")) {
            change_state(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panel_title("OBJECTS");
        
        //New object button.
        if(
            ImGui::ImageButton(
                "newMobButton", editor_icons[EDITOR_ICON_ADD],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            new_mob_cmd(1.0f);
        }
        set_tooltip(
            "Start creating a new object.\n"
            "Click on the canvas where you want the object to be.",
            "N"
        );
        
        if(!selected_mobs.empty()) {
        
            //Delete object button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "delMobButtonn", editor_icons[EDITOR_ICON_REMOVE],
                    point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                delete_mob_cmd(1.0f);
            }
            set_tooltip(
                "Delete all selected objects.\n",
                "Delete"
            );
            
            //Duplicate object button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "dupMobButton", editor_icons[EDITOR_ICON_DUPLICATE],
                    point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                duplicate_mobs_cmd(1.0f);
            }
            set_tooltip(
                "Start duplicating the selected objects.\n"
                "Click on the canvas where you want the copied objects to be.",
                "Ctrl+D"
            );
            
        }
        
        ImGui::Spacer();
        
        if(selected_mobs.size() == 1 || selection_homogenized) {
        
            process_gui_panel_mob();
            
        } else if(selected_mobs.empty()) {
        
            //"No object selected" text.
            ImGui::TextDisabled("(No object selected)");
            
        } else {
        
            //Non-homogenized objects warning.
            ImGui::TextWrapped(
                "Multiple different objects selected. To make all their "
                "properties the same and edit them all together, click here:"
            );
            
            //Homogenize objects button.
            if(ImGui::Button("Edit all together")) {
                register_change("object combining");
                selection_homogenized = true;
                homogenize_selected_mobs();
            }
        }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui path link control panel for this frame.
 */
void area_editor::process_gui_panel_path_link() {
    path_link* l_ptr = *selected_path_links.begin();
    
    //Type combobox.
    vector<string> link_type_names;
    link_type_names.push_back("Normal");
    link_type_names.push_back("Ledge");
    
    int type_i = l_ptr->type;
    if(ImGui::Combo("Type", &type_i, link_type_names, 15)) {
        register_change("path link type change");
        l_ptr->type = (PATH_LINK_TYPE) type_i;
    }
    set_tooltip(
        "What type of link this is."
    );
    
    homogenize_selected_path_links();
}


/**
 * @brief Processes the Dear ImGui path stop control panel for this frame.
 */
void area_editor::process_gui_panel_path_stop() {
    path_stop* s_ptr = *selected_path_stops.begin();
    
    //Radius value.
    float radius = s_ptr->radius;
    if(ImGui::DragFloat("Radius", &radius, 0.5f, PATHS::MIN_STOP_RADIUS)) {
        radius = std::max(PATHS::MIN_STOP_RADIUS, radius);
        register_change("path stop radius change");
        s_ptr->radius = radius;
        path_preview_timer.start(false);
    }
    set_tooltip(
        "Radius of the stop. Used when mobs want to find the closest\n"
        "start/end stop.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Script use only checkbox.
    int flags_i = s_ptr->flags;
    if(
        ImGui::CheckboxFlags(
            "Script use only",
            &flags_i,
            PATH_STOP_FLAG_SCRIPT_ONLY
        )
    ) {
        register_change("path stop property change");
        s_ptr->flags = flags_i;
    }
    set_tooltip(
        "Can only be used by objects if their script tells them to."
    );
    
    //Light load only checkbox.
    if(
        ImGui::CheckboxFlags(
            "Light load only",
            &flags_i,
            PATH_STOP_FLAG_LIGHT_LOAD_ONLY
        )
    ) {
        register_change("path stop property change");
        s_ptr->flags = flags_i;
    }
    set_tooltip(
        "Can only be used by objects that are not carrying anything, "
        "or by objects that only have a weight of 1."
    );
    
    //Airborne only checkbox.
    if(
        ImGui::CheckboxFlags(
            "Airborne only",
            &flags_i,
            PATH_STOP_FLAG_AIRBORNE_ONLY
        )
    ) {
        register_change("path stop property change");
        s_ptr->flags = flags_i;
    }
    set_tooltip(
        "Can only be used by objects that can fly."
    );
    
    //Label text.
    mono_input_text("Label", &s_ptr->label);
    set_tooltip(
        "If this stop is part of a path that you want\n"
        "to address in a script, write the name here."
    );
    
    homogenize_selected_path_stops();
}


/**
 * @brief Processes the Dear ImGui paths control panel for this frame.
 */
void area_editor::process_gui_panel_paths() {
    ImGui::BeginChild("paths");
    
    if(sub_state == EDITOR_SUB_STATE_PATH_DRAWING) {
    
        //Drawing explanation text.
        ImGui::TextWrapped(
            "Use the canvas to draw path links and path stops. "
            "Each click places a stop and/or connects to a stop. "
            "Use the following widgets the change how new links will be."
        );
        
        //Link settings text.
        ImGui::Spacer();
        ImGui::Text("New path link settings:");
        ImGui::Indent();
        
        int one_way_mode = path_drawing_normals;
        
        //One-way links radio button.
        ImGui::RadioButton("Draw one-way links", &one_way_mode, 0);
        set_tooltip(
            "When drawing, new links drawn will be one-way links.",
            "1"
        );
        
        //Normal links radio button.
        ImGui::RadioButton("Draw normal links", &one_way_mode, 1);
        set_tooltip(
            "When drawing, new links drawn will be normal (two-way) links.",
            "2"
        );
        
        path_drawing_normals = one_way_mode;
        
        //Type combobox.
        vector<string> link_type_names;
        link_type_names.push_back("Normal");
        link_type_names.push_back("Ledge");
        
        int type_i = path_drawing_type;
        if(ImGui::Combo("Type", &type_i, link_type_names, 15)) {
            path_drawing_type = (PATH_LINK_TYPE) type_i;
        }
        set_tooltip(
            "What type of link to draw."
        );
        ImGui::Unindent();
        
        //Stop settings text.
        ImGui::Spacer();
        ImGui::Text("New path stop settings:");
        
        //Script use only checkbox.
        ImGui::Indent();
        int flags_i = path_drawing_flags;
        if(
            ImGui::CheckboxFlags(
                "Script use only",
                &flags_i,
                PATH_STOP_FLAG_SCRIPT_ONLY
            )
        ) {
            path_drawing_flags = flags_i;
        }
        set_tooltip(
            "Can only be used by objects if their script tells them to."
        );
        
        //Light load only checkbox.
        if(
            ImGui::CheckboxFlags(
                "Light load only",
                &flags_i,
                PATH_STOP_FLAG_LIGHT_LOAD_ONLY
            )
        ) {
            path_drawing_flags = flags_i;
        }
        set_tooltip(
            "Can only be used by objects that are not carrying anything, "
            "or by objects that only have a weight of 1."
        );
        
        //Airborne only checkbox.
        if(
            ImGui::CheckboxFlags(
                "Airborne only",
                &flags_i,
                PATH_STOP_FLAG_AIRBORNE_ONLY
            )
        ) {
            path_drawing_flags = flags_i;
        }
        set_tooltip(
            "Can only be used by objects that can fly."
        );
        
        //Label text.
        mono_input_text("Label", &path_drawing_label);
        set_tooltip(
            "If the new stop is part of a path that you want\n"
            "to address in a script, write the name here."
        );
        ImGui::Unindent();
        
        //Drawing stop button.
        ImGui::Spacer();
        if(ImGui::Button("Done", ImVec2(-1.0f, 32.0f))) {
            set_status();
            sub_state = EDITOR_SUB_STATE_NONE;
        }
        set_tooltip(
            "Stop drawing.",
            "Escape"
        );
        
    } else {
    
        //Back button.
        if(ImGui::Button("Back")) {
            change_state(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panel_title("PATHS");
        
        //New path button.
        if(
            ImGui::ImageButton(
                "newPathButton", editor_icons[EDITOR_ICON_ADD],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            new_path_cmd(1.0f);
        }
        set_tooltip(
            "Start drawing a new path.\n"
            "Click on a path stop to start there, or click somewhere empty "
            "to start on a new stop.\n"
            "Then, click a path stop or somewhere empty to create a "
            "link there.",
            "N"
        );
        
        //Delete path button.
        if(!selected_path_links.empty() || !selected_path_stops.empty()) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "delPathButton", editor_icons[EDITOR_ICON_REMOVE],
                    point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                delete_path_cmd(1.0f);
            }
            set_tooltip(
                "Delete all selected path stops and/or path links.\n",
                "Delete"
            );
        }
        
        //Stop properties node.
        ImGui::Spacer();
        if(saveable_tree_node("paths", "Stop properties")) {
        
            bool ok_to_edit =
                (selected_path_stops.size() == 1) || selection_homogenized;
                
            if(selected_path_stops.empty()) {
            
                //"No stop selected" text.
                ImGui::TextDisabled("(No path stop selected)");
                
            } else if(ok_to_edit) {
            
                process_gui_panel_path_stop();
                
            } else {
            
                //Non-homogenized stops warning.
                ImGui::TextWrapped(
                    "Multiple different path stops selected. "
                    "To make all their properties the same and "
                    "edit them all together, click here:"
                );
                
                //Homogenize stops button.
                if(ImGui::Button("Edit all together")) {
                    register_change("path stop combining");
                    selection_homogenized = true;
                    //Unselect path links otherwise those will be considered
                    //homogenized too.
                    selected_path_links.clear();
                    homogenize_selected_path_stops();
                }
            }
            
            
            ImGui::TreePop();
            
        }
        
        //Link properties node.
        ImGui::Spacer();
        if(saveable_tree_node("paths", "Link properties")) {
        
            bool ok_to_edit =
                (selected_path_links.size() == 1) || selection_homogenized;
            if(!ok_to_edit && selected_path_links.size() == 2) {
                auto it = selected_path_links.begin();
                path_link* l1 = *it;
                it++;
                path_link* l2 = *it;
                if(
                    l1->start_ptr == l2->end_ptr &&
                    l1->end_ptr == l2->start_ptr
                ) {
                    //The only things we have selected are a link,
                    //and also the opposite link. As far as the user cares,
                    //this is all just one link that is of the "normal" type.
                    //And if they edit the properties, we want both links to
                    //be edited together.
                    ok_to_edit = true;
                }
            }
            
            if(selected_path_links.empty()) {
            
                //"No link selected" text.
                ImGui::TextDisabled("(No path link selected)");
                
            } else if(ok_to_edit) {
            
                process_gui_panel_path_link();
                
            } else {
            
                //Non-homogenized links warning.
                ImGui::TextWrapped(
                    "Multiple different path links selected. "
                    "To make all their properties the same and "
                    "edit them all together, click here:"
                );
                
                //Homogenize links button.
                if(ImGui::Button("Edit all together")) {
                    register_change("path link combining");
                    selection_homogenized = true;
                    //Unselect path stops otherwise those will be considered
                    //homogenized too.
                    selected_path_stops.clear();
                    homogenize_selected_path_links();
                }
            }
            
            
            ImGui::TreePop();
            
        }
        
        //Path preview node.
        ImGui::Spacer();
        if(saveable_tree_node("paths", "Path preview")) {
        
            //Show preview path checkbox.
            if(ImGui::Checkbox("Show preview path", &show_path_preview)) {
                if(
                    show_path_preview &&
                    path_preview_checkpoints[0].x == LARGE_FLOAT
                ) {
                    //No previous location. Place them on-camera.
                    path_preview_checkpoints[0].x =
                        game.cam.pos.x - AREA_EDITOR::COMFY_DIST;
                    path_preview_checkpoints[0].y =
                        game.cam.pos.y;
                    path_preview_checkpoints[1].x =
                        game.cam.pos.x + AREA_EDITOR::COMFY_DIST;
                    path_preview_checkpoints[1].y =
                        game.cam.pos.y;
                }
                path_preview_dist = calculate_preview_path();
            }
            set_tooltip(
                "Show the path objects will take to travel from point A\n"
                "to point B. These points can be dragged in the canvas.\n"
                "Hazards and obstacles will not be taken into consideration\n"
                "when calculating the preview path."
            );
            
            ImGui::Spacer();
            
            if(show_path_preview) {
            
                unsigned int flags_i = path_preview_settings.flags;
                
                //Is from script checkbox.
                if(
                    ImGui::CheckboxFlags(
                        "Is from script",
                        &flags_i,
                        PATH_FOLLOW_FLAG_SCRIPT_USE
                    )
                ) {
                    path_preview_settings.flags = flags_i;
                    path_preview_dist = calculate_preview_path();
                }
                set_tooltip(
                    "Whether the path preview feature is considered to be\n"
                    "from a script, meaning it can use script-only stops."
                );
                
                //Has light load checkbox.
                if(
                    ImGui::CheckboxFlags(
                        "Has light load",
                        &flags_i,
                        PATH_FOLLOW_FLAG_LIGHT_LOAD
                    )
                ) {
                    path_preview_settings.flags = flags_i;
                    path_preview_dist = calculate_preview_path();
                }
                set_tooltip(
                    "Whether the path preview feature is considered to have\n"
                    "a light load, meaning it can use light load-only stops."
                );
                
                //Is airborne checkbox.
                if(
                    ImGui::CheckboxFlags(
                        "Is airborne",
                        &flags_i,
                        PATH_FOLLOW_FLAG_AIRBORNE
                    )
                ) {
                    path_preview_settings.flags = flags_i;
                    path_preview_dist = calculate_preview_path();
                }
                set_tooltip(
                    "Whether the path preview feature is considered to be\n"
                    "airborne, meaning it can use airborne-only stops\n"
                    "and go up ledges."
                );
                
                //Use stops with this label input.
                if(
                    ImGui::InputText(
                        "Label",
                        &path_preview_settings.label
                    )
                ) {
                    path_preview_dist = calculate_preview_path();
                }
                set_tooltip(
                    "To limit the path preview feature to only use stops with\n"
                    "a given label, write its name here, or leave it empty\n"
                    "for no label enforcement."
                );
                
                string result;
                float total_dist = 0.0f;
                size_t total_nr_stops = 0;
                bool success = false;
                
                if(path_preview_result > 0) {
                    total_dist = path_preview_dist;
                    total_nr_stops = path_preview.size();
                    success = true;
                }
                
                result = path_result_to_string(path_preview_result);
                
                //Path result header text.
                ImGui::Spacer();
                ImGui::Text("Result:");
                
                //Path result text.
                ImGui::BulletText("%s", result.c_str());
                
                //Path total travel distance text.
                if(success) {
                    ImGui::BulletText(
                        "Total travel distance: %f", total_dist
                    );
                } else {
                    ImGui::Text(" ");
                }
                
                //Path total stops visited text.
                if(success) {
                    ImGui::BulletText(
                        "Total stops visited: %lu", total_nr_stops
                    );
                } else {
                    ImGui::Text(" ");
                }
                
            }
            
            ImGui::TreePop();
            
        }
        
        //Path tools node.
        ImGui::Spacer();
        if(saveable_tree_node("paths", "Tools")) {
        
            //Show closest stop checkbox.
            ImGui::Checkbox("Show closest stop", &show_closest_stop);
            set_tooltip(
                "Show the closest stop to the cursor.\n"
                "Useful to know which stop "
                "Pikmin will go to when starting to carry."
            );
            
            //Select stops with label button.
            if(ImGui::Button("Select all stops with label...")) {
                ImGui::OpenPopup("selectStops");
            }
            set_tooltip(
                "Selects all stops that have the specified label.\n"
                "The search is case-sensitive."
            );
            
            //Select stops with label popup.
            static string label_name;
            if(input_popup("selectStops", "Label:", &label_name, true)) {
                select_path_stops_with_label(label_name);
            }
            
            ImGui::TreePop();
            
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui review control panel for this frame.
 */
void area_editor::process_gui_panel_review() {
    ImGui::BeginChild("review");
    
    //Back button.
    if(ImGui::Button("Back")) {
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("REVIEW");
    
    //Problem search node.
    if(saveable_tree_node("review", "Problem search")) {
    
        //Problem search button.
        if(ImGui::Button("Search for problems")) {
            find_problems();
        }
        set_tooltip(
            "Search for problems with the area."
        );
        
        //Problem texts.
        ImGui::Text("Problem found:");
        
        ImGui::Indent();
        if(problem_type == EPT_NONE_YET) {
            ImGui::TextDisabled("Haven't searched yet.");
        } else {
            ImGui::TextWrapped("%s", problem_title.c_str());
        }
        ImGui::Unindent();
        
        if(!problem_description.empty()) {
        
            ImGui::Indent();
            ImGui::TextWrapped("%s", problem_description.c_str());
            ImGui::Unindent();
            
            //Go to problem button.
            if(ImGui::Button("Go to problem")) {
                goto_problem();
            }
            set_tooltip(
                "Focus the camera on the problem found, if applicable."
            );
            
        }
        
        ImGui::TreePop();
        
    }
    
    //Preview node.
    ImGui::Spacer();
    if(saveable_tree_node("review", "Preview")) {
    
        //Area preview checkbox.
        ImGui::Checkbox("Preview area", &preview_mode);
        set_tooltip(
            "Preview how the area will look like, without any of the "
            "area editor's components in the way."
        );
        
        //Tree shadows checkbox.
        if(!preview_mode) {
            ImGui::BeginDisabled();
        }
        ImGui::Indent();
        ImGui::Checkbox("Show tree shadows", &show_shadows);
        ImGui::Unindent();
        if(!preview_mode) {
            ImGui::EndDisabled();
        }
        
        ImGui::TreePop();
        
    }
    
    //Cross-section node.
    ImGui::Spacer();
    if(saveable_tree_node("review", "Cross-section")) {
    
        //Show cross-section checkbox.
        if(ImGui::Checkbox("Show cross-section", &show_cross_section)) {
            if(show_cross_section) {
                cross_section_window_start = canvas_tl;
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
                    game.cam.pos.x - AREA_EDITOR::COMFY_DIST;
                cross_section_checkpoints[0].y =
                    game.cam.pos.y;
                cross_section_checkpoints[1].x =
                    game.cam.pos.x + AREA_EDITOR::COMFY_DIST;
                cross_section_checkpoints[1].y =
                    game.cam.pos.y;
            }
        }
        set_tooltip(
            "Show a 2D cross-section between points A and B."
        );
        
        //Show height grid checkbox.
        if(show_cross_section) {
            ImGui::Indent();
            ImGui::Checkbox("Show height grid", &show_cross_section_grid);
            set_tooltip(
                "Show a height grid in the cross-section window."
            );
            ImGui::Unindent();
        }
        
        ImGui::Spacer();
        
        ImGui::TreePop();
        
    }
    
    //Tools node.
    if(saveable_tree_node("review", "Tools")) {
    
        //Show blocking sectors checkbox.
        ImGui::Checkbox("Show blocking sectors", &show_blocking_sectors);
        set_tooltip(
            "Show which sectors are blocking (red) and which\n"
            "are not (green). Useful to make sure the radar works as\n"
            "intended, and that players can't go or throw out-of-bounds."
        );
        
        //Show height grid checkbox.
        if(show_cross_section) {
            ImGui::Indent();
            ImGui::Checkbox("Show height grid", &show_cross_section_grid);
            set_tooltip(
                "Show a height grid in the cross-section window."
            );
            ImGui::Unindent();
        }
        
        ImGui::Spacer();
        
        ImGui::TreePop();
        
    }
    
    //Stats node.
    if(saveable_tree_node("main", "Stats")) {
    
        //Sector amount text.
        ImGui::BulletText(
            "Sectors: %i", (int) game.cur_area_data->sectors.size()
        );
        
        //Edge amount text.
        ImGui::BulletText(
            "Edges: %i", (int) game.cur_area_data->edges.size()
        );
        
        //Vertex amount text.
        ImGui::BulletText(
            "Vertexes: %i", (int) game.cur_area_data->vertexes.size()
        );
        
        //Object amount text.
        ImGui::BulletText(
            "Objects: %i", (int) game.cur_area_data->mob_generators.size()
        );
        
        //Path stop amount text.
        ImGui::BulletText(
            "Path stops: %i", (int) game.cur_area_data->path_stops.size()
        );
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sector control panel for this frame.
 */
void area_editor::process_gui_panel_sector() {
    sector* s_ptr = *selected_sectors.begin();
    
    //Sector behavior node.
    if(saveable_tree_node("layout", "Behavior")) {
    
        //Sector height value.
        float sector_z = s_ptr->z;
        if(ImGui::DragFloat("Height", &sector_z)) {
            register_change("sector height change");
            s_ptr->z = sector_z;
            update_all_edge_offset_caches();
        }
        if(ImGui::BeginPopupContextItem()) {
            //-50 height selectable.
            if(ImGui::Selectable("-50")) {
                register_change("sector height change");
                s_ptr->z -= 50.0f;
                update_all_edge_offset_caches();
                ImGui::CloseCurrentPopup();
            }
            
            //+50 height selectable.
            if(ImGui::Selectable("+50")) {
                register_change("sector height change");
                s_ptr->z += 50.0f;
                update_all_edge_offset_caches();
                ImGui::CloseCurrentPopup();
            }
            
            //Set to zero selectable.
            if(ImGui::Selectable("Set to 0")) {
                register_change("sector height change");
                s_ptr->z = 0.0f;
                update_all_edge_offset_caches();
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
        set_tooltip(
            "Height of the floor. Positive numbers are higher.\n"
            "Right-click for some shortcuts.\n"
            "You can also hold H in the canvas to set a sector's height\n"
            "by moving the cursor up or down.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Sector hazards node.
        ImGui::Spacer();
        if(saveable_tree_node("layout", "Hazards")) {
        
            static int selected_hazard_idx = 0;
            
            //Sector hazard addition button.
            if(
                ImGui::ImageButton(
                    "addHazardButton", editor_icons[EDITOR_ICON_ADD],
                    point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                ImGui::OpenPopup("addHazard");
            }
            set_tooltip(
                "Add a new hazard to the list of hazards this sector has.\n"
                "Click to open a pop-up for you to choose from."
            );
            
            //Sector hazard addition popup.
            vector<string> all_hazards_list;
            for(auto &h : game.content.hazards.list) {
                all_hazards_list.push_back(h.first);
            }
            string picked_hazard;
            if(
                list_popup(
                    "addHazard", all_hazards_list, &picked_hazard, true
                )
            ) {
                vector<string> list =
                    semicolon_list_to_vector(s_ptr->hazards_str);
                if(
                    std::find(
                        list.begin(), list.end(), picked_hazard
                    ) == list.end()
                ) {
                    register_change("sector hazard addition");
                    if(!s_ptr->hazards_str.empty()) {
                        s_ptr->hazards_str += ";";
                    }
                    s_ptr->hazards_str += picked_hazard;
                    s_ptr->hazards.push_back(&(game.content.hazards.list[picked_hazard]));
                    selected_hazard_idx = (int) list.size();
                    set_status(
                        "Added hazard \"" + picked_hazard +
                        "\" to the sector."
                    );
                }
            }
            
            //Sector hazard removal button.
            if(
                selected_hazard_idx >= 0 &&
                !(*selected_sectors.begin())->hazards_str.empty()
            ) {
                ImGui::SameLine();
                if(
                    ImGui::ImageButton(
                        "remHazardButton", editor_icons[EDITOR_ICON_REMOVE],
                        point(EDITOR::ICON_BMP_SIZE)
                    )
                ) {
                    vector<string> list =
                        semicolon_list_to_vector(s_ptr->hazards_str);
                    if(selected_hazard_idx < (int) list.size()) {
                        register_change("sector hazard removal");
                        string hazard_name = list[selected_hazard_idx];
                        s_ptr->hazards_str.clear();
                        s_ptr->hazards.clear();
                        for(size_t h = 0; h < list.size(); h++) {
                            if(h == (size_t) selected_hazard_idx) continue;
                            s_ptr->hazards_str += list[h] + ";";
                            s_ptr->hazards.push_back(&(game.content.hazards.list[list[h]]));
                        }
                        if(!s_ptr->hazards_str.empty()) {
                            //Delete the trailing semicolon.
                            s_ptr->hazards_str.pop_back();
                        }
                        selected_hazard_idx =
                            std::min(
                                selected_hazard_idx, (int) list.size() - 2
                            );
                        set_status(
                            "Removed hazard \"" + hazard_name +
                            "\" from the sector."
                        );
                    }
                }
                set_tooltip(
                    "Remove the selected hazard from the list of "
                    "hazards this sector has."
                );
            }
            
            //Sector hazard list.
            if(
                !(*selected_sectors.begin())->hazards_str.empty()
            ) {
                mono_list_box(
                    "Hazards", &selected_hazard_idx,
                    semicolon_list_to_vector(s_ptr->hazards_str),
                    4
                );
                set_tooltip(
                    "List of hazards this sector has."
                );
                
                bool sector_hazard_floor = s_ptr->hazard_floor;
                if(ImGui::Checkbox("Floor only", &sector_hazard_floor)) {
                    register_change("sector hazard floor option change");
                    s_ptr->hazard_floor = sector_hazard_floor;
                }
                set_tooltip(
                    "Do the hazards only affects objects on the floor,\n"
                    "or do they affect airborne objects in the sector too?"
                );
            }
            
            ImGui::TreePop();
        }
        
        //Sector advanced behavior node.
        ImGui::Spacer();
        if(saveable_tree_node("layout", "Advanced")) {
        
            //Sector type combobox.
            vector<string> types_list;
            for(
                size_t t = 0; t < game.sector_types.get_nr_of_items(); t++
            ) {
                types_list.push_back(
                    str_to_sentence(game.sector_types.get_name((SECTOR_TYPE) t))
                );
            }
            int sector_type = s_ptr->type;
            if(ImGui::Combo("Type", &sector_type, types_list, 15)) {
                register_change("sector type change");
                s_ptr->type = (SECTOR_TYPE) sector_type;
            }
            set_tooltip(
                "What type of sector this is."
            );
            
            //Sector bottomless pit checkbox.
            bool sector_bottomless_pit = s_ptr->is_bottomless_pit;
            if(ImGui::Checkbox("Bottomless pit", &sector_bottomless_pit)) {
                register_change("sector bottomless pit change");
                s_ptr->is_bottomless_pit = sector_bottomless_pit;
                if(!sector_bottomless_pit) {
                    update_sector_texture(s_ptr, s_ptr->texture_info.bmp_name);
                }
            }
            set_tooltip(
                "Is this sector's floor a bottomless pit?\n"
                "Pikmin die when they fall in, and you can see the void."
            );
            
            ImGui::Spacer();
            
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }
    
    //Sector appearance node.
    ImGui::Spacer();
    if(saveable_tree_node("layout", "Appearance")) {
    
        int texture_type = !s_ptr->fade;
        
        //Sector texture fader radio button.
        ImGui::RadioButton("Texture fader", &texture_type, 0);
        set_tooltip(
            "Makes the surrounding textures fade into each other."
        );
        
        //Sector regular texture radio button.
        ImGui::RadioButton("Regular texture", &texture_type, 1);
        set_tooltip(
            "Makes the sector use a regular texture."
        );
        
        if(s_ptr->fade != (texture_type == 0)) {
            register_change("sector texture type change");
            s_ptr->fade = texture_type == 0;
            if(!s_ptr->fade) {
                update_sector_texture(s_ptr, s_ptr->texture_info.bmp_name);
            }
        }
        
        if(!s_ptr->fade) {
        
            ImGui::Indent();
            
            //Sector texture button.
            if(ImGui::Button("Choose image...")) {
                vector<picker_item> picker_buttons;
                
                picker_buttons.push_back(picker_item("Choose another..."));
                
                for(size_t s = 0; s < texture_suggestions.size(); s++) {
                    picker_buttons.push_back(
                        picker_item(
                            texture_suggestions[s].name,
                            "", "", nullptr,
                            "",
                            texture_suggestions[s].bmp
                        )
                    );
                }
                open_picker_dialog(
                    "Pick a texture",
                    picker_buttons,
                    std::bind(
                        &area_editor::pick_texture, this,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3,
                        std::placeholders::_4,
                        std::placeholders::_5
                    ),
                    "Suggestions:", false, true
                );
            }
            set_tooltip(
                "Pick a texture to use.\n"
                "You can also press T in the canvas to copy\n"
                "the texture of the currently selected sector,\n"
                "and paste it into whatever sector is under the cursor."
            );
            
            //Sector texture name text.
            ImGui::SameLine();
            mono_text("%s", s_ptr->texture_info.bmp_name.c_str());
            
            ImGui::Unindent();
            
        }
        
        //Sector texture effects node.
        ImGui::Spacer();
        if(saveable_tree_node("layout", "Texture effects")) {
        
            //Sector texture offset value.
            point texture_translation = s_ptr->texture_info.translation;
            if(ImGui::DragFloat2("Offset", (float*) &texture_translation)) {
                register_change("sector texture offset change");
                s_ptr->texture_info.translation = texture_translation;
                quick_preview_timer.start();
            }
            set_tooltip(
                "Offset the texture horizontally or vertically "
                "by this much.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Sector texture scale value.
            point texture_scale = s_ptr->texture_info.scale;
            if(ImGui::DragFloat2("Scale", (float*) &texture_scale, 0.01)) {
                register_change("sector texture scale change");
                s_ptr->texture_info.scale = texture_scale;
                quick_preview_timer.start();
            }
            set_tooltip(
                "Scale the texture horizontally or vertically "
                "by this much.\n"
                "The scale's anchor point is at the origin "
                "of the area, at coordinates 0,0.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Sector texture rotation value.
            float texture_rotation = normalize_angle(s_ptr->texture_info.rot);
            if(ImGui::SliderAngle("Angle", &texture_rotation, 0, 360, "%.2f")) {
                register_change("sector texture angle change");
                s_ptr->texture_info.rot = texture_rotation;
                quick_preview_timer.start();
            }
            set_tooltip(
                "Rotate the texture by these many degrees.\n"
                "The rotation's center point is at the origin "
                "of the area, at coordinates 0,0.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            //Sector texture tint value.
            ALLEGRO_COLOR texture_tint = s_ptr->texture_info.tint;
            if(
                ImGui::ColorEdit4(
                    "Tint color", (float*) &texture_tint,
                    ImGuiColorEditFlags_NoInputs
                )
            ) {
                register_change("sector texture tint change");
                s_ptr->texture_info.tint = texture_tint;
                quick_preview_timer.start();
            }
            set_tooltip(
                "Tint the texture with this color. White means no tint."
            );
            
            //On-canvas texture effect editing checkbox.
            bool octee_on =
                sub_state == EDITOR_SUB_STATE_OCTEE;
            if(ImGui::Checkbox("On-canvas editing", &octee_on)) {
                sub_state =
                    octee_on ?
                    EDITOR_SUB_STATE_OCTEE :
                    EDITOR_SUB_STATE_NONE;
            }
            set_tooltip(
                "Enable on-canvas texture effect editing.\n"
                "With this, you can click and drag on the canvas "
                "to adjust the texture,\n"
                "based on whatever mode is currently active."
            );
            
            if(octee_on) {
            
                ImGui::Indent();
                
                int octee_mode_int = (int) octee_mode;
                
                //On-canvas texture effect editing offset radio button.
                ImGui::RadioButton(
                    "Change offset", &octee_mode_int,
                    (int) OCTEE_MODE_OFFSET
                );
                set_tooltip(
                    "Dragging will change the texture's offset.",
                    "1"
                );
                
                //On-canvas texture effect editing scale radio button.
                ImGui::RadioButton(
                    "Change scale", &octee_mode_int,
                    (int) OCTEE_MODE_SCALE
                );
                set_tooltip(
                    "Dragging will change the texture's scale.",
                    "2"
                );
                
                //On-canvas texture effect editing angle radio button.
                ImGui::RadioButton(
                    "Change angle", &octee_mode_int,
                    (int) OCTEE_MODE_ANGLE
                );
                set_tooltip(
                    "Dragging will change the texture's angle.",
                    "3"
                );
                
                octee_mode = (OCTEE_MODE) octee_mode_int;
                
                ImGui::Unindent();
                
            }
            
            ImGui::TreePop();
        }
        
        //Sector mood node.
        ImGui::Spacer();
        if(saveable_tree_node("layout", "Sector mood")) {
        
            //Sector brightness value.
            int sector_brightness = s_ptr->brightness;
            ImGui::SetNextItemWidth(180);
            if(ImGui::SliderInt("Brightness", &sector_brightness, 0, 255)) {
                register_change("sector brightness change");
                s_ptr->brightness = sector_brightness;
            }
            set_tooltip(
                "How bright the sector is. Affects not just the sector's "
                "appearance, but everything inside it.\n"
                "0 is fully dark, 255 is fully lit.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            ImGui::Spacer();
            
            ImGui::TreePop();
        }
        
        ImGui::Spacer();
        
        ImGui::TreePop();
    }
    
    homogenize_selected_sectors();
}


/**
 * @brief Processes the Dear ImGui tools control panel for this frame.
 */
void area_editor::process_gui_panel_tools() {
    ImGui::BeginChild("tools");
    
    //Back button.
    if(ImGui::Button("Back")) {
        save_reference();
        change_state(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panel_title("TOOLS");
    
    //Reference image node.
    if(saveable_tree_node("tools", "Reference image")) {
    
        string old_ref_file_name = reference_file_path;
        
        //Browse for a reference image button.
        if(ImGui::Button("Browse...")) {
            vector<string> f =
                prompt_file_dialog(
                    "",
                    "Please choose the bitmap to use for a reference.",
                    "*.*",
                    ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                    ALLEGRO_FILECHOOSER_PICTURES,
                    game.display
                );
                
            if(!f.empty() && !f[0].empty()) {
                reference_file_path = f[0];
            }
        }
        set_tooltip(
            "Browse for a file on your disk to use."
        );
    
        //Reference image name text.
        string ref_file_name =
            get_path_last_component(reference_file_path);
        ImGui::SameLine();
        mono_text("%s", ref_file_name.c_str());
        set_tooltip("Full path:\n" + reference_file_path);
        
        //Reference image file name input.
        ImGui::SameLine();
        ImGui::InputText("Bitmap", &reference_file_path);
        set_tooltip(
            "File name of the reference image, anywhere on the disk.\n"
            "Extension included. e.g.: \"sketch_2.jpg\""
        );
        
        if(old_ref_file_name != reference_file_path) {
            update_reference();
        }
        
        //Reference center value.
        ImGui::DragFloat2("Center", (float*) &reference_center);
        set_tooltip(
            "Center coordinates.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Reference size value.
        process_gui_size_widgets(
            "Size", reference_size, 1.0f,
            reference_keep_aspect_ratio, false,
            AREA_EDITOR::REFERENCE_MIN_SIZE
        );
        set_tooltip(
            "Width and height.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Reference keep aspect ratio checkbox.
        ImGui::Indent();
        ImGui::Checkbox(
            "Keep aspect ratio",
            &reference_keep_aspect_ratio
        );
        ImGui::Unindent();
        set_tooltip("Keep the aspect ratio when resizing the image.");
        
        //Reference opacity value.
        int opacity = reference_alpha;
        ImGui::SliderInt("Opacity", &opacity, 0, 255);
        reference_alpha = opacity;
        set_tooltip(
            "How opaque it is.",
            "", WIDGET_EXPLANATION_SLIDER
        );
        
        ImGui::TreePop();
        
    }
    
    //Misc. node.
    ImGui::Spacer();
    if(saveable_tree_node("tools", "Misc.")) {
    
        //Load auto-backup button.
        if(ImGui::Button("Load auto-backup")) {
            changes_mgr.ask_if_unsaved(
                point(),
                "loading the auto-backup", "load",
            [this] () {
                bool backup_exists = false;
                if(!manifest.internal_name.empty()) {
                    string file_path =
                        game.cur_area_data->user_data_path + "/" + FILE_NAMES::AREA_GEOMETRY;
                    if(al_filename_exists(file_path.c_str())) {
                        backup_exists = true;
                    }
                }
                
                if(backup_exists) {
                    load_backup();
                } else {
                    set_status("There is no backup available.");
                }
            },
            [this] () { return save_area(false); }
            );
        }
        set_tooltip(
            "Discard all changes made and load the auto-backup, if any exists."
        );
        
        //Resize everything multiplier value.
        static float resize_mults[2] = { 1.0f, 1.0f };
        ImGui::SetNextItemWidth(128.0f);
        ImGui::DragFloat2("##resizeMult", resize_mults, 0.01);
        set_tooltip(
            "Resize multipliers, vertically and horizontally.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Resize everything button.
        ImGui::SameLine();
        if(ImGui::Button("Resize everything")) {
            if(resize_mults[0] == 0.0f || resize_mults[1] == 0.0f) {
                set_status(
                    "Can't resize everything to size 0!",
                    true
                );
            } else if(resize_mults[0] == 1.0f && resize_mults[1] == 1.0f) {
                set_status(
                    "Resizing everything by 1 wouldn't make a difference!",
                    true
                );
            } else {
                register_change("global resize");
                resize_everything(resize_mults);
                set_status(
                    "Resized everything by " + f2s(resize_mults[0]) + ", " +
                    f2s(resize_mults[1]) + "."
                );
                resize_mults[0] = 1.0f;
                resize_mults[1] = 1.0f;
            }
        }
        set_tooltip(
            "Resize everything in the area by the specified multiplier.\n"
            "0.5 will resize everything to half size, 2.0 to double, etc."
        );
        
        ImGui::Spacer();
        
        ImGui::TreePop();
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui status bar for this frame.
 */
void area_editor::process_gui_status_bar() {
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
void area_editor::process_gui_toolbar() {
    if(manifest.internal_name.empty() || !game.cur_area_data) return;
    
    //Quit button.
    if(
        ImGui::ImageButton(
            "quitButton", editor_icons[EDITOR_ICON_QUIT],
            point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        quit_widget_pos = get_last_widget_pos();
        quit_cmd(1.0f);
    }
    set_tooltip(
        "Quit the area editor.",
        "Ctrl + Q"
    );
    
    //Load button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "loadButton", editor_icons[EDITOR_ICON_LOAD],
            point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        load_widget_pos = get_last_widget_pos();
        load_cmd(1.0f);
    }
    set_tooltip(
        "Pick an area to load, or create a new one.",
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
            point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        save_cmd(1.0f);
    }
    set_tooltip(
        "Save the area into the files on disk.",
        "Ctrl + S"
    );
    
    //Play button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "playButton", editor_icons[EDITOR_ICON_PLAY],
            point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        quick_play_cmd(1.0f);
    }
    set_tooltip(
        "Save, quit, and start playing the area. Leaving will return "
        "to the editor.",
        "Ctrl + P"
    );
    
    //Undo button.
    unsigned char undo_opacity = undo_history.empty() ? 50 : 255;
    ImGui::SameLine(0, 16);
    if(
        ImGui::ImageButton(
            "undoButton", editor_icons[EDITOR_ICON_UNDO],
            point(EDITOR::ICON_BMP_SIZE),
            point(0.0f), point(1.0f),
            COLOR_EMPTY, map_alpha(undo_opacity)
        )
    ) {
        undo_cmd(1.0f);
    }
    string undo_text;
    if(undo_history.empty()) {
        undo_text = "Nothing to undo.";
    } else {
        undo_text = "Undo: " + undo_history.front().second + ".";
    }
    set_tooltip(
        undo_text,
        "Ctrl + Z"
    );
    
    //Redo button.
    unsigned redo_opacity = redo_history.empty() ? 50 : 255;
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "redoButton", editor_icons[EDITOR_ICON_UNDO],
            point(EDITOR::ICON_BMP_SIZE),
            point(1.0f, 0.0f), point(0.0f, 1.0f),
            COLOR_EMPTY, map_alpha(redo_opacity)
        )
    ) {
        redo_cmd(1.0f);
    }
    string redo_text;
    if(redo_history.empty()) {
        redo_text =
            "Nothing to redo.";
    } else {
        redo_text =
            "Redo: " + redo_history.front().second + ".";
    }
    set_tooltip(
        redo_text,
        "Ctrl + Y"
    );
    
    if(!reference_file_path.empty()) {
    
        //Reference image toggle button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "refToggleButton", editor_icons[EDITOR_ICON_REFERENCE],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            reference_toggle_cmd(1.0f);
        }
        set_tooltip(
            "Toggle the visibility of the reference image.",
            "Ctrl + R"
        );
        
        //Reference image opacity value.
        int reference_alpha_int = reference_alpha;
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(0.0f, 0.0f));
        ImGui::SetNextItemWidth(48.0f);
        ImGui::SliderInt("##refAlpha", &reference_alpha_int, 0, 255, "");
        set_tooltip(
            "Opacity of the reference image.",
            "", WIDGET_EXPLANATION_SLIDER
        );
        ImGui::EndGroup();
        reference_alpha = reference_alpha_int;
        
    }
    
    //Snap mode button.
    ALLEGRO_BITMAP* snap_mode_bmp = nullptr;
    string snap_mode_description;
    switch(game.options.area_editor_snap_mode) {
    case SNAP_MODE_GRID: {
        snap_mode_bmp = editor_icons[EDITOR_ICON_SNAP_GRID];
        snap_mode_description = "grid. Holding Shift disables snapping.";
        break;
    } case SNAP_MODE_VERTEXES: {
        snap_mode_bmp = editor_icons[EDITOR_ICON_SNAP_VERTEXES];
        snap_mode_description = "vertexes. Holding Shift disables snapping.";
        break;
    } case SNAP_MODE_EDGES: {
        snap_mode_bmp = editor_icons[EDITOR_ICON_SNAP_EDGES];
        snap_mode_description = "edges. Holding Shift disables snapping.";
        break;
    } case SNAP_MODE_NOTHING: {
        snap_mode_bmp = editor_icons[EDITOR_ICON_SNAP_NOTHING];
        snap_mode_description = "off. Holding Shift snaps to grid.";
        break;
    } case N_SNAP_MODES: {
        break;
    }
    }
    
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "snapButton", snap_mode_bmp,
            point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        snap_mode_cmd(1.0f);
    }
    set_tooltip(
        "Current snap mode: " + snap_mode_description,
        "X or Shift + X"
    );
    
    if(game.options.area_editor_advanced_mode) {
    
        //Layout mode button.
        ImGui::SameLine(0, 16);
        if(
            ImGui::ImageButton(
                "layoutButton", editor_icons[EDITOR_ICON_SECTORS],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            change_state(EDITOR_STATE_LAYOUT);
        }
        set_tooltip(
            "Swaps to the layout editing mode.",
            "L"
        );
        
        //Mobs mode button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "mobsButton", editor_icons[EDITOR_ICON_MOBS],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            change_state(EDITOR_STATE_MOBS);
        }
        set_tooltip(
            "Swaps to the objects editing mode.",
            "O"
        );
        
        //Paths mode button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "pathsButton", editor_icons[EDITOR_ICON_PATHS],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            change_state(EDITOR_STATE_PATHS);
        }
        set_tooltip(
            "Swaps to the paths editing mode.",
            "P"
        );
        
        //Details mode button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "detailsButton", editor_icons[EDITOR_ICON_DETAILS],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            change_state(EDITOR_STATE_DETAILS);
        }
        set_tooltip(
            "Swaps to the details editing mode.",
            "D"
        );
        
        //Toggle preview mode button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "previewButton", editor_icons[EDITOR_ICON_REVIEW],
                point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            preview_mode = !preview_mode;
        }
        set_tooltip(
            "Toggles area preview mode. More info in the review panel.",
            "Shift + P"
        );
        
    }
}
