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

#include "../editor.h"
#include "../../libs/imgui/imgui_impl_allegro5.h"


namespace GUI_EDITOR {
extern const vector<float> GRID_INTERVALS;
extern const float MOUSE_COORDS_TEXT_WIDTH;
extern const string SONG_NAME;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the GUI editor.
 */
class gui_editor : public editor {

public:

    //--- Members ---
    
    //Automatically load this file upon boot-up of the editor, if any.
    string auto_load_file;
    
    
    //--- Function declarations ---
    
    gui_editor();
    void do_logic() override;
    void do_drawing() override;
    void load() override;
    void unload() override;
    string get_name() const override;
    void draw_canvas();
    string get_history_option_prefix() const override;
    string get_opened_file_name() const;
    
private:

    //--- Misc. declarations ---
    
    /**
     * @brief Represents a GUI item.
     */
    struct item {
    
        //--- Members ---
        
        //Its name in the file.
        string name;
        
        //Center coordinates.
        point center;
        
        //Width and height.
        point size;
        
    };
    
    
    //--- Members ---
    
    //Currently selected item, or INVALID for none.
    size_t cur_item = INVALID;
    
    //File name of the file currently being edited.
    string file_name;
    
    //Data node for the contents of this GUI file.
    data_node file_node;
    
    //List of items for the current file.
    vector<item> items;
    
    //Picker info for the picker in the "load" dialog.
    picker_info load_dialog_picker;
    
    //Position of the load widget.
    point load_widget_pos;
    
    //The list of items must focus on the currently selected item.
    bool must_focus_on_cur_item = false;
    
    //Small hack -- does the camera need recentering in process_gui()?
    bool must_recenter_cam = false;
    
    //Position of the reload widget.
    point reload_widget_pos;
    
    //Position of the quit widget.
    point quit_widget_pos;
    
    //The current transformation widget.
    transformation_widget cur_transformation_widget;
    
    
    //--- Function declarations ---
    
    void close_load_dialog();
    void close_options_dialog();
    void load_file(bool should_update_history);
    void open_load_dialog();
    void open_options_dialog();
    void pick_file(
        const string &name, const string &category, bool is_new
    );
    bool save_file();
    point snap_point(const point &p);
    static void draw_canvas_imgui_callback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void grid_interval_decrease_cmd(float input_value);
    void grid_interval_increase_cmd(float input_value);
    void load_cmd(float input_value);
    void quit_cmd(float input_value);
    void reload_cmd(float input_value);
    void save_cmd(float input_value);
    void snap_mode_cmd(float input_value);
    void zoom_and_pos_reset_cmd(float input_value);
    void zoom_in_cmd(float input_value);
    void zoom_out_cmd(float input_value);
    void process_gui();
    void process_gui_control_panel();
    void process_gui_load_dialog();
    void process_gui_menu_bar();
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
