/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general animation editor-related functions.
 */

#pragma once

#include <string>

#include "../editor.h"
#include "../../libs/imgui/imgui_impl_allegro5.h"
#include "../../utils/general_utils.h"


namespace ANIM_EDITOR {
extern const float FLOOD_FILL_ALPHA_THRESHOLD;
extern const float GRID_INTERVAL;
extern const float HITBOX_MIN_RADIUS;
extern const float KEYBOARD_PAN_AMOUNT;
extern const float MOUSE_COORDS_TEXT_WIDTH;
extern const string SONG_NAME;
extern const size_t TIMELINE_HEADER_HEIGHT;
extern const size_t TIMELINE_HEIGHT;
extern const size_t TIMELINE_LOOP_TRI_SIZE;
extern const size_t TIMELINE_PADDING;
extern const float TOP_MIN_SIZE;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the animation editor.
 */
class animation_editor : public editor {

public:

    //--- Members ---
    
    //Automatically load this animation file upon boot-up of the editor, if any.
    string auto_load_file;
    
    
    //--- Function declarations ---
    
    animation_editor();
    void do_logic() override;
    void do_drawing() override;
    void load() override;
    void unload() override;
    string get_name() const override;
    string get_name_for_history() const;
    void draw_canvas();
    string get_history_option_prefix() const override;
    string get_opened_content_path() const;
    
private:

    //--- Misc. declarations ---
    
    //Editor states.
    enum EDITOR_STATE {
    
        //Main menu.
        EDITOR_STATE_MAIN,
        
        //Animation editing.
        EDITOR_STATE_ANIMATION,
        
        //Sprite editing.
        EDITOR_STATE_SPRITE,
        
        //Body part editing.
        EDITOR_STATE_BODY_PART,
        
        //Hitbox editing.
        EDITOR_STATE_HITBOXES,
        
        //Sprite bitmap editing.
        EDITOR_STATE_SPRITE_BITMAP,
        
        //Sprite transformations editing.
        EDITOR_STATE_SPRITE_TRANSFORM,
        
        //Top editing.
        EDITOR_STATE_TOP,
        
        //Info.
        EDITOR_STATE_INFO,
        
        //Tools.
        EDITOR_STATE_TOOLS,
        
    };
    
    
    //--- Members ---
    
    //Currently loaded animation database.
    animation_database db;
    
    //Is the current animation playing?
    bool anim_playing = false;
    
    //Whether to use a background texture, if any.
    ALLEGRO_BITMAP* bg = nullptr;
    
    //Is the sprite comparison mode on?
    bool comparison = false;
    
    //Is the comparison sprite above the working sprite?
    bool comparison_above = true;
    
    //Is the comparison sprite meant to blink?
    bool comparison_blink = true;
    
    //Is the blinking comparison sprite currently visible?
    bool comparison_blink_show = true;
    
    //Time left until the blinking comparison sprite's visibility is swapped.
    timer comparison_blink_timer;
    
    //Comparison sprite to use in sprite comparison mode.
    sprite* comparison_sprite = nullptr;
    
    //Is the comparison sprite mode tinting the sprites?
    bool comparison_tint = true;
    
    //Animation instance, for when the user is editing animations.
    animation_instance cur_anim_i;
    
    //Current hitbox.
    hitbox* cur_hitbox = nullptr;
    
    //The alpha is calculated using the sine of this value.
    float cur_hitbox_alpha = 0.0f;
    
    //Index number of the current hitbox.
    size_t cur_hitbox_idx = INVALID;
    
    //Current maturity to display on the Pikmin's top.
    unsigned char cur_maturity = 0;
    
    //Current sprite, for when the user is editing sprites.
    sprite* cur_sprite = nullptr;
    
    //Keep the aspect ratio when resizing the current sprite?
    bool cur_sprite_keep_aspect_ratio = true;
    
    //Keep the total area when resizing the current sprite?
    bool cur_sprite_keep_area = false;
    
    //The current transformation widget.
    transformation_widget cur_transformation_widget;
    
    //Is the grid visible?
    bool grid_visible = true;
    
    //Are the hitboxes currently visible?
    bool hitboxes_visible = true;
    
    //Last file used as for a spritesheet.
    string last_spritesheet_used;
    
    //Picker info for the picker in the "load" dialog.
    picker_info load_dialog_picker;
    
    //Mob type of the currently loaded animation database, if any.
    mob_type* loaded_mob_type = nullptr;
    
    //Is the mob radius visible?
    bool mob_radius_visible = false;
    
    //Is the leader silhouette visible?
    bool leader_silhouette_visible = false;
    
    //Before entering the sprite bitmap state, this was the camera position.
    point pre_sprite_bmp_cam_pos;
    
    //Before entering the sprite bitmap state, this was the camera zoom.
    float pre_sprite_bmp_cam_zoom = 1.0f;
    
    //Is side view on?
    bool side_view = false;
    
    //Is the add mode on in the sprite bitmap state?
    bool sprite_bmp_add_mode = false;
    
    //Top bitmaps for the current Pikmin type.
    ALLEGRO_BITMAP* top_bmp[N_MATURITIES] = { nullptr, nullptr, nullptr };
    
    //Keep the aspect ratio when resizing the Pikmin top?
    bool top_keep_aspect_ratio = true;
    
    //Whether to use a background texture.
    bool use_bg = false;
    
    //Position of the load widget.
    point load_widget_pos;
    
    //Position of the reload widget.
    point reload_widget_pos;
    
    //Position of the quit widget.
    point quit_widget_pos;
    
    //Info about the "new" dialog.
    struct {
    
        //Selected pack.
        string pack;
        
        //Selected animation database type.
        int type = 0;
        
        //Selected custom mob category, when picking a mob type.
        string custom_mob_cat;
        
        //Selected mob type, when picking a mob type.
        mob_type* mob_type_ptr = nullptr;
        
        //Problem found, if any.
        string problem;
        
        //Internal name of the new animation database.
        string internal_name = "my_animation";
        
        //Path to the new animation database.
        string anim_path;
        
        //Whether the dialog needs updating.
        bool must_update = true;
        
    } new_dialog;
    
    
    //--- Function declarations ---
    
    void center_camera_on_sprite_bitmap(bool instant);
    void change_state(const EDITOR_STATE new_state);
    void close_load_dialog();
    void close_options_dialog();
    void create_anim_db(const string &path);
    float get_cursor_timeline_time();
    string get_file_tooltip(const string &path) const;
    void handle_lmb_drag_in_timeline();
    void import_animation_data(const string &name);
    void import_sprite_file_data(const string &name);
    void import_sprite_hitbox_data(const string &name);
    void import_sprite_top_data(const string &name);
    void import_sprite_transformation_data(const string &name);
    bool is_cursor_in_timeline();
    void load_anim_db_file(
        const string &path, bool should_update_history
    );
    void pick_anim_db_file(
        const string &name, const string &top_cat, const string &sec_cat,
        void* info, bool is_new
    );
    void play_sound(size_t sound_idx);
    void reload_anim_dbs();
    void rename_animation(animation* anim, const string &new_name);
    void rename_body_part(body_part* part, const string &new_name);
    void rename_sprite(sprite* spr, const string &new_name);
    void resize_everything(float mult);
    void resize_sprite(sprite* s, float mult);
    bool save_anim_db();
    void setup_for_new_anim_db_post();
    void setup_for_new_anim_db_pre();
    void set_all_sprite_scales(float scale);
    void set_best_frame_sprite();
    void sprite_bmp_flood_fill(
        ALLEGRO_BITMAP* bmp, bool* selection_pixels, int x, int y
    );
    void update_cur_hitbox();
    void update_hitboxes();
    static void draw_canvas_imgui_callback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void draw_comparison();
    void draw_side_view_hitbox(
        hitbox* h_ptr, const ALLEGRO_COLOR &color,
        const ALLEGRO_COLOR &outline_color, float outline_thickness
    );
    void draw_side_view_leader_silhouette(float x_offset);
    void draw_side_view_sprite(const sprite* s);
    void draw_timeline();
    void draw_top_down_view_hitbox(
        hitbox* h_ptr, const ALLEGRO_COLOR &color,
        const ALLEGRO_COLOR &outline_color, float outline_thickness
    );
    void draw_top_down_view_leader_silhouette(float x_offset);
    void draw_top_down_view_mob_radius(mob_type* mt);
    void draw_top_down_view_sprite(sprite* s);
    void open_load_dialog();
    void open_new_dialog();
    void open_options_dialog();
    void pick_animation(
        const string &name, const string &top_cat, const string &sec_cat,
        void* info, bool is_new
    );
    void pick_sprite(
        const string &name, const string &top_cat, const string &sec_cat,
        void* info, bool is_new
    );
    void grid_toggle_cmd(float input_value);
    void hitboxes_toggle_cmd(float input_value);
    void leader_silhouette_toggle_cmd(float input_value);
    void load_cmd(float input_value);
    void mob_radius_toggle_cmd(float input_value);
    void play_animation_cmd(float input_value);
    void quit_cmd(float input_value);
    void reload_cmd(float input_value);
    void save_cmd(float input_value);
    void zoom_and_pos_reset_cmd(float input_value);
    void zoom_everything_cmd(float input_value);
    void zoom_in_cmd(float input_value);
    void zoom_out_cmd(float input_value);
    void process_gui();
    void process_gui_control_panel();
    void process_gui_hitbox_hazards();
    void process_gui_load_dialog();
    void process_gui_new_dialog();
    void process_gui_options_dialog();
    void process_gui_panel_animation();
    void process_gui_panel_body_part();
    void process_gui_panel_info();
    void process_gui_panel_main();
    void process_gui_panel_options();
    void process_gui_panel_sprite();
    void process_gui_panel_sprite_bitmap();
    void process_gui_panel_sprite_hitboxes();
    void process_gui_panel_sprite_top();
    void process_gui_panel_sprite_transform();
    void process_gui_panel_tools();
    void process_gui_menu_bar();
    void process_gui_status_bar();
    void process_gui_toolbar();
    void handle_key_char_canvas(const ALLEGRO_EVENT &ev) override;
    void handle_key_down_anywhere(const ALLEGRO_EVENT &ev) override;
    void handle_key_down_canvas(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_double_click(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_down(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_drag(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_up(const ALLEGRO_EVENT &ev) override;
    void handle_mmb_double_click(const ALLEGRO_EVENT &ev) override;
    void handle_mmb_down(const ALLEGRO_EVENT &ev) override;
    void handle_mmb_drag(const ALLEGRO_EVENT &ev) override;
    void handle_mouse_update(const ALLEGRO_EVENT &ev) override;
    void handle_mouse_wheel(const ALLEGRO_EVENT &ev) override;
    void handle_rmb_double_click(const ALLEGRO_EVENT &ev) override;
    void handle_rmb_down(const ALLEGRO_EVENT &ev) override;
    void handle_rmb_drag(const ALLEGRO_EVENT &ev) override;
    void pan_cam(const ALLEGRO_EVENT &ev);
    void reset_cam_xy();
    void reset_cam_zoom();
    
};
