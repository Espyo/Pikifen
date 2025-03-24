/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general GUI editor-related functions.
 */

#pragma once

#include <string>

#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../editor.h"


namespace GUI_EDITOR {
extern const vector<float> GRID_INTERVALS;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the GUI editor.
 */
class GuiEditor : public Editor {

public:

    //--- Members ---
    
    //Automatically load this file upon boot-up of the editor, if any.
    string auto_load_file;
    
    
    //--- Function declarations ---
    
    GuiEditor();
    void do_logic() override;
    void do_drawing() override;
    void load() override;
    void unload() override;
    string get_name() const override;
    void draw_canvas();
    string get_opened_content_path() const;
    
private:

    //--- Misc. declarations ---
    
    /**
     * @brief Represents a GUI item.
     */
    struct Item {
    
        //--- Members ---
        
        //Its name in the file.
        string name;
        
        //Center coordinates.
        Point center;
        
        //Width and height.
        Point size;
        
    };
    
    
    //--- Members ---
    
    //Currently selected item, or INVALID for none.
    size_t cur_item = INVALID;
    
    //Data node for the contents of the current GUI definition.
    DataNode file_node;
    
    //List of items for the current GUI definition.
    vector<Item> items;
    
    //Picker info for the picker in the "load" dialog.
    Picker load_dialog_picker;
    
    //Position of the load widget.
    Point load_widget_pos;
    
    //The list of items must focus on the currently selected item.
    bool must_focus_on_cur_item = false;
    
    //Small hack -- does the camera need recentering in process_gui()?
    bool must_recenter_cam = false;
    
    //Position of the reload widget.
    Point reload_widget_pos;
    
    //Position of the quit widget.
    Point quit_widget_pos;
    
    //The current transformation widget.
    TransformationWidget cur_transformation_widget;
    
    struct {
    
        //Selected pack.
        string pack;
        
        //Internal name of the new GUI definition.
        string internal_name;
        
        //Problem found, if any.
        string problem;
        
        //Path to the new GUI definition.
        string def_path;
        
        //Whether the dialog needs updating.
        bool must_update = true;
        
    } new_dialog;
    
    
    //--- Function declarations ---
    
    void close_load_dialog();
    void close_options_dialog();
    void create_gui_def(const string &internal_name, const string &pack);
    void delete_current_gui_def();
    void load_gui_def_file(const string &path, bool should_update_history);
    void open_load_dialog();
    void open_new_dialog();
    void open_options_dialog();
    void pick_gui_def_file(
        const string &name, const string &top_cat, const string &sec_cat, void* info, bool is_new
    );
    bool save_gui_def();
    void setup_for_new_gui_def();
    Point snap_point(const Point &p);
    static void draw_canvas_imgui_callback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    string get_file_tooltip(const string &path) const;
    void grid_interval_decrease_cmd(float input_value);
    void grid_interval_increase_cmd(float input_value);
    void delete_gui_def_cmd(float input_value);
    void load_cmd(float input_value);
    void quit_cmd(float input_value);
    void reload_cmd(float input_value);
    void reload_gui_defs();
    void save_cmd(float input_value);
    void snap_mode_cmd(float input_value);
    void zoom_and_pos_reset_cmd(float input_value);
    void zoom_in_cmd(float input_value);
    void zoom_out_cmd(float input_value);
    void process_gui();
    void process_gui_control_panel();
    void process_gui_delete_gui_def_dialog();
    void process_gui_load_dialog();
    void process_gui_menu_bar();
    void process_gui_new_dialog();
    void process_gui_options_dialog();
    void process_gui_panel_item();
    void process_gui_panel_items();
    void process_gui_status_bar();
    void process_gui_toolbar();
    void handle_key_char_canvas(const ALLEGRO_EVENT &ev) override;
    void handle_key_down_anywhere(const ALLEGRO_EVENT &ev) override;
    void handle_key_down_canvas(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_double_click(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_down(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_drag(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_up(const ALLEGRO_EVENT &ev) override;
    void handle_mmb_down(const ALLEGRO_EVENT &ev) override;
    void handle_mmb_drag(const ALLEGRO_EVENT &ev) override;
    void handle_mouse_update(const ALLEGRO_EVENT &ev) override;
    void handle_mouse_wheel(const ALLEGRO_EVENT &ev) override;
    void handle_rmb_down(const ALLEGRO_EVENT &ev) override;
    void handle_rmb_drag(const ALLEGRO_EVENT &ev) override;
    void pan_cam(const ALLEGRO_EVENT &ev);
    void reset_cam(bool instantaneous);
    
};
