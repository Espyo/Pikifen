/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general particle editor-related functions.
 */

#pragma once

#include <string>

#include "../editor.h"


namespace PARTICLE_EDITOR {
extern const vector<float> GRID_INTERVALS;
extern const float MOUSE_COORDS_TEXT_WIDTH;
extern const string SONG_NAME;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the particle editor.
 */
class particle_editor : public editor {

public:

    //--- Members ---
    
    //Automatically load this file upon boot-up of the editor, if any.
    string auto_load_file;
    
    
    //--- Function declarations ---
    
    particle_editor();
    void do_logic() override;
    void do_drawing() override;
    void load() override;
    void unload() override;
    string get_name() const override;
    void draw_canvas();
    string get_history_option_prefix() const override;
    string get_opened_content_path() const;
    
private:

    //--- Members ---
    
    //Currently loaded particle generator.
    particle_generator loaded_gen;
    
    //Particle manager.
    particle_manager part_mgr;
    
    //Whether to use a background texture, if any.
    ALLEGRO_BITMAP* bg = nullptr;
    
    //Is the grid visible?
    bool grid_visible = true;
    
    //Picker info for the picker in the "load" dialog.
    picker_info load_dialog_picker;
    
    //Position of the load widget.
    point load_widget_pos;
    
    //Is the particle manager currently generating?
    bool mgr_running = false;
    
    //Is the particle generator currently generating?
    bool gen_running = false;
    
    //Offset the generator's angle in the editor by this much.
    float generator_angle_offset = 0.0f;
    
    //Offset the generator's position in the editor by this much.
    point generator_pos_offset;
    
    //Is the leader silhouette visible?
    bool leader_silhouette_visible = false;
    
    //Is the emission shape visible?
    bool emission_shape_visible = false;
    
    //Selected color keyframe.
    size_t selected_color_keyframe = 0;
    
    //Selected size keyframe.
    size_t selected_size_keyframe = 0;
    
    //Selected linear speed keyframe.
    size_t selected_linear_speed_keyframe = 0;
    
    //Selected orbital velocity keyframe.
    size_t selected_oribital_velocity_keyframe = 0;
    
    //Selected outward velocity keyframe.
    size_t selected_outward_velocity_keyframe = 0;
    
    //Position of the reload widget.
    point reload_widget_pos;
    
    //Position of the quit widget.
    point quit_widget_pos;
    
    //Whether to use a background texture.
    bool use_bg = false;
    
    struct {
    
        //Selected pack.
        string pack;
        
        //Internal name of the new particle generator.
        string internal_name = "my_particle_generator";
        
        //Problem found, if any.
        string problem;
        
        //Path to the new generator.
        string part_gen_path;
        
        //Whether the dialog needs updating.
        bool must_update = true;
        
        //Whether we need to focus on the text input widget.
        bool needs_text_focus = true;
        
    } new_dialog;
    
    
    //--- Function declarations ---
    
    void close_load_dialog();
    void close_options_dialog();
    void create_part_gen(const string &part_gen_path);
    string get_file_tooltip(const string &path) const;
    void load_part_gen_file(
        const string &path, const bool should_update_history
    );
    void open_load_dialog();
    void open_new_dialog();
    void open_options_dialog();
    void pick_part_gen_file(
        const string &name, const string &top_cat, const string &sec_cat,
        void* info, bool is_new
    );
    void reload_part_gens();
    void setup_for_new_part_gen_post();
    void setup_for_new_part_gen_pre();
    bool save_part_gen();
    static void draw_canvas_imgui_callback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void grid_interval_decrease_cmd(float input_value);
    void grid_interval_increase_cmd(float input_value);
    void grid_toggle_cmd(float input_value);
    void load_cmd(float input_value);
    void quit_cmd(float input_value);
    void reload_cmd(float input_value);
    void save_cmd(float input_value);
    void zoom_and_pos_reset_cmd(float input_value);
    void zoom_in_cmd(float input_value);
    void zoom_out_cmd(float input_value);
    void clear_particles_cmd(float input_value);
    void emission_shape_toggle_cmd(float input_value);
    void leader_silhouette_toggle_cmd(float input_value);
    void part_gen_playback_toggle_cmd(float input_value);
    void part_mgr_playback_toggle_cmd(float input_value);
    void process_gui();
    void process_gui_control_panel();
    void process_gui_load_dialog();
    void process_gui_menu_bar();
    void process_gui_new_dialog();
    void process_gui_options_dialog();
    void process_gui_panel_generator();
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
    void reset_cam_xy();
    void reset_cam_zoom();
    
};
