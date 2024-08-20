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
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Process the list of items.
    process_gui_panel_items();
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    //Process the currently selected item.
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
            "Grid interval: %f", game.options.gui_editor_grid_interval
        );
        
        //Increase grid interval button.
        ImGui::SameLine();
        if(ImGui::Button("+")) {
            grid_interval_increase_cmd(1.0f);
        }
        set_tooltip(
            "Increase the spacing on the grid.\n"
            "Default: " + i2s(OPTIONS::DEF_GUI_EDITOR_GRID_INTERVAL) +
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
            "Default: " + i2s(OPTIONS::DEF_GUI_EDITOR_GRID_INTERVAL) +
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
    //TODO: remove???
    /*
    if(cur_item == INVALID) return;
    
    item* cur_item_ptr = &items[cur_item];
    
    if(cur_item_ptr->size.x == 0.0f) return;
    
    //Item's name text.
    ImGui::Text("Item \"%s\" data:", cur_item_ptr->name.c_str());
    
    //Center values.
    if(
        ImGui::DragFloat2("Center", (float*) &cur_item_ptr->center, 0.10f)
    ) {
        changes_mgr.mark_as_changed();
    }
    set_tooltip(
        "Center coordinates of the item. e.g. 32,100 is 32% of the\n"
        "width horizontally and very bottom vertically.",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    //Size values.
    if(
        process_gui_size_widgets(
            "Size", cur_item_ptr->size, 0.10f, false, false, 0.10f
        )
    ) {
        changes_mgr.mark_as_changed();
    }
    set_tooltip(
        "Width and height of the item. e.g. 40,90 is 40% of the screen width,\n"
        "and 90% of the screen height.",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    //Spacer dummy widget.
    ImGui::Dummy(ImVec2(0, 16));
    
    point top_left(
        cur_item_ptr->center.x - cur_item_ptr->size.x / 2.0f,
        cur_item_ptr->center.y - cur_item_ptr->size.y / 2.0f
    );
    point bottom_right(
        cur_item_ptr->center.x + cur_item_ptr->size.x / 2.0f,
        cur_item_ptr->center.y + cur_item_ptr->size.y / 2.0f
    );
    bool update_from_corners = false;
    
    //Top-left coordinates values.
    if(ImGui::DragFloat2("Top-left", (float*) &top_left, 0.10f)) {
        update_from_corners = true;
    }
    
    //Bottom-right coordinates values.
    if(ImGui::DragFloat2("Bottom-right", (float*) &bottom_right, 0.10f)) {
        update_from_corners = true;
    }
    
    if(update_from_corners) {
        point new_size(
            bottom_right.x - top_left.x,
            bottom_right.y - top_left.y
        );
        if(new_size.x > 0.0f && new_size.y > 0.0f) {
            point new_center(
                (top_left.x + bottom_right.x) / 2.0f,
                (top_left.y + bottom_right.y) / 2.0f
            );
            cur_item_ptr->center = new_center;
            cur_item_ptr->size = new_size;
        }
        changes_mgr.mark_as_changed();
    }
    */
}


/**
 * @brief Processes the GUI item list panel for this frame.
 */
void particle_editor::process_gui_panel_items() {
    //TODO: remove
    /*
    //Items text.
    ImGui::Text("Items:");
    
    //Item list.
    if(
        ImGui::BeginChild(
            "itemsList", ImVec2(0.0f, 300.0f), ImGuiChildFlags_Border
        )
    ) {
        for(size_t i = 0; i < items.size(); ++i) {
        
            //Item checkbox.
            bool visible = items[i].size.x != 0.0f;
            if(
                ImGui::Checkbox(("##v" + items[i].name).c_str(), &visible)
            ) {
                if(visible) {
                    items[i].center.x = 50.0f;
                    items[i].center.y = 50.0f;
                    items[i].size.x = 10.0f;
                    items[i].size.y = 10.0f;
                } else {
                    items[i].center.x = 0.0f;
                    items[i].center.y = 0.0f;
                    items[i].size.x = 0.0f;
                    items[i].size.y = 0.0f;
                }
                changes_mgr.mark_as_changed();
            }
            set_tooltip(
                "Whether this item is visible in-game or not."
            );
            
            //Separator text.
            ImGui::SameLine();
            ImGui::Text("  ");
            
            //Item selectable.
            bool selected = cur_item == i;
            ImGui::SameLine();
            if(
                ImGui::Selectable(items[i].name.c_str(), &selected)
            ) {
                cur_item = i;
            }
            
            if(must_focus_on_cur_item && selected) {
                ImGui::SetScrollHereY(0.5f);
                must_focus_on_cur_item = false;
            }
            
        }
        ImGui::EndChild();
    }
    */
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
        GUI_EDITOR::MOUSE_COORDS_TEXT_WIDTH;
    ImGui::Dummy(ImVec2(size, 0));
    
    //Mouse coordinates text.
    if(!is_mouse_in_gui || is_m1_pressed) {
        ImGui::SameLine();
        ImGui::Text(
            "%s, %s",
            box_string(f2s(game.mouse_cursor.w_pos.x), 7, "%").c_str(),
            box_string(f2s(game.mouse_cursor.w_pos.y), 7, "%").c_str()
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
    
    //Snap mode button.
    ALLEGRO_BITMAP* snap_mode_bmp = nullptr;
    string snap_mode_description;
    if(game.options.gui_editor_snap) {
        snap_mode_bmp = editor_icons[EDITOR_ICON_SNAP_GRID];
        snap_mode_description = "grid. Holding Shift disables snapping.";
    } else {
        snap_mode_bmp = editor_icons[EDITOR_ICON_SNAP_NOTHING];
        snap_mode_description = "nothing. Holding Shift snaps to grid.";
    }
    
    ImGui::SameLine(0, 16);
    if(
        ImGui::ImageButton(
            "snapButton",
            snap_mode_bmp,
            ImVec2(EDITOR::ICON_BMP_SIZE, EDITOR::ICON_BMP_SIZE)
        )
    ) {
        snap_mode_cmd(1.0f);
    }
    set_tooltip(
        "Current snap mode: " + snap_mode_description,
        "X"
    );
}
