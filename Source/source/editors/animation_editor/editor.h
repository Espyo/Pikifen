/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general animation editor-related functions.
 */

#ifndef ANIMATION_EDITOR_INCLUDED
#define ANIMATION_EDITOR_INCLUDED

#include <string>

#include "../editor.h"
#include "../../imgui/imgui_impl_allegro5.h"


class animation_editor : public editor {
public:

    static const size_t HISTORY_SIZE;
    
    //Automatically load this animation file upon boot-up of the editor, if any.
    string auto_load_anim;
    //History for the last files that were opened.
    vector<string> history;
    
    void update_history(const string &n);
    
    void do_logic();
    void do_drawing();
    void draw_canvas();
    void load();
    void unload();
    virtual string get_name() const;
    
    animation_editor();
    
private:

private:

    enum EDITOR_STATES {
        EDITOR_STATE_MAIN,
        EDITOR_STATE_ANIMATION,
        EDITOR_STATE_SPRITE,
        EDITOR_STATE_BODY_PART,
        EDITOR_STATE_HITBOXES,
        EDITOR_STATE_SPRITE_BITMAP,
        EDITOR_STATE_SPRITE_TRANSFORM,
        EDITOR_STATE_TOP,
        EDITOR_STATE_LOAD,
        EDITOR_STATE_TOOLS,
        EDITOR_STATE_OPTIONS,
    };
    
    static const float KEYBOARD_PAN_AMOUNT;
    static const float ZOOM_MAX_LEVEL_EDITOR;
    static const float ZOOM_MIN_LEVEL_EDITOR;
    
    animation_database        anims;
    bool                      anim_playing;
    bool                      comparison;
    bool                      comparison_above;
    bool                      comparison_blink;
    bool                      comparison_blink_show;
    timer                     comparison_blink_timer;
    sprite*                   comparison_sprite;
    bool                      comparison_tint;
    animation*                cur_anim;
    size_t                    cur_body_part_nr;
    size_t                    cur_frame_nr;
    float                     cur_frame_time;
    hitbox*                   cur_hitbox;
    //The alpha is calculated using the sine of this value.
    float                     cur_hitbox_alpha;
    size_t                    cur_hitbox_nr;
    transformation_controller cur_hitbox_tc;
    //Current maturity of the Pikmin,
    //used to check the visuals of different Pikmin tops.
    unsigned char             cur_maturity;
    sprite*                   cur_sprite;
    transformation_controller cur_sprite_tc;
    string                    file_path;
    bool                      hitboxes_visible;
    string                    last_file_used;
    mob_type*                 loaded_mob_type;
    bool                      mob_radius_visible;
    bool                      origin_visible;
    bool                      pikmin_silhouette_visible;
    point                     pre_sprite_bmp_cam_pos;
    float                     pre_sprite_bmp_cam_zoom;
    bool                      side_view;
    bool                      sprite_bmp_add_mode;
    //Top bitmaps for the current Pikmin type.
    ALLEGRO_BITMAP*           top_bmp[N_MATURITIES];
    transformation_controller top_tc;
    
    //Position of some important widgets.
    point reload_widget_pos;
    point quit_widget_pos;
    
    
    //General functions.
    void center_camera_on_sprite_bitmap();
    void change_state(const EDITOR_STATES new_state);
    static void draw_canvas_imgui_callback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void draw_comparison();
    void draw_side_view_hitbox(
        hitbox* h_ptr, const ALLEGRO_COLOR &color,
        const ALLEGRO_COLOR &outline_color, const float outline_thickness
    );
    void draw_side_view_pikmin_silhouette(const float x_offset);
    void draw_side_view_sprite(sprite* s);
    void draw_top_down_view_hitbox(
        hitbox* h_ptr, const ALLEGRO_COLOR &color,
        const ALLEGRO_COLOR &outline_color, const float outline_thickness
    );
    void draw_top_down_view_mob_radius(mob_type* mt);
    void draw_top_down_view_pikmin_silhouette(const float x_offset);
    void draw_top_down_view_sprite(sprite* s);
    void enter_side_view();
    void exit_side_view();
    string get_cut_path(const string &p) const;
    void import_animation_data(const string &name);
    void import_sprite_file_data(const string &name);
    void import_sprite_hitbox_data(const string &name);
    void import_sprite_top_data(const string &name);
    void import_sprite_transformation_data(const string &name);
    void load_animation_database(const bool should_update_history);
    void pick_animation(const string &name, const bool is_new);
    void pick_sprite(const string &name, const bool is_new);
    void press_quit_button();
    void process_gui();
    void process_gui_control_panel();
    void process_gui_panel_animation();
    void process_gui_panel_body_part();
    void process_gui_panel_load();
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
    void rename_animation();
    void rename_sprite();
    void resize_everything();
    void save_animation_database();
    void set_all_sprite_scales();
    void sprite_bmp_flood_fill(
        ALLEGRO_BITMAP* bmp, bool* selection_pixels, const int x, const int y
    );
    void update_cur_hitbox_tc();
    void update_hitboxes();
    void update_stats();
    
    //Input handler functions.
    void handle_key_char_canvas(const ALLEGRO_EVENT &ev);
    void handle_key_down_anywhere(const ALLEGRO_EVENT &ev);
    void handle_key_down_canvas(const ALLEGRO_EVENT &ev);
    void handle_lmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_lmb_down(const ALLEGRO_EVENT &ev);
    void handle_lmb_drag(const ALLEGRO_EVENT &ev);
    void handle_lmb_up(const ALLEGRO_EVENT &ev);
    void handle_mmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_mmb_down(const ALLEGRO_EVENT &ev);
    void handle_mmb_drag(const ALLEGRO_EVENT &ev);
    void handle_mouse_update(const ALLEGRO_EVENT &ev);
    void handle_mouse_wheel(const ALLEGRO_EVENT &ev);
    void handle_rmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_rmb_down(const ALLEGRO_EVENT &ev);
    void handle_rmb_drag(const ALLEGRO_EVENT &ev);
    void pan_cam(const ALLEGRO_EVENT &ev);
    void reset_cam_xy(const ALLEGRO_EVENT &ev);
    void reset_cam_zoom(const ALLEGRO_EVENT &ev);
};

#endif //ifndef ANIMATION_EDITOR_INCLUDED
