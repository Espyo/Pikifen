/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general animation editor-related functions.
 */

#ifndef ANIMATION_EDITOR_INCLUDED
#define ANIMATION_EDITOR_INCLUDED

#include <string>

#include <allegro5/allegro_native_dialog.h>

#include "editor.h"
#include "../animation.h"
#include "../game_state.h"
#include "../hitbox.h"
#include "../LAFI/gui.h"
#include "../LAFI/widget.h"
#include "../misc_structs.h"

using namespace std;

class animation_editor : public editor {
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
    
    enum ANIMATION_EDITOR_PICKER_TYPES {
        PICKER_LOAD_MOB_TYPE,
        PICKER_LOAD_GLOBAL_ANIM,
        PICKER_EDIT_ANIMATION,
        PICKER_SET_FRAME_SPRITE,
        PICKER_EDIT_SPRITE,
        PICKER_IMPORT_SPRITE,
        PICKER_IMPORT_SPRITE_BITMAP,
        PICKER_IMPORT_SPRITE_TRANSFORMATION,
        PICKER_IMPORT_SPRITE_HITBOXES,
        PICKER_IMPORT_SPRITE_TOP,
        PICKER_COMPARE_SPRITE,
        PICKER_RENAME_ANIMATION,
        PICKER_RENAME_SPRITE,
    };
    
    static const float ZOOM_MAX_LEVEL_EDITOR;
    static const float ZOOM_MIN_LEVEL_EDITOR;
    
    
    //GUI widgets.
    lafi::frame* frm_main;
    lafi::frame* frm_object;
    lafi::frame* frm_load;
    lafi::frame* frm_anims;
    lafi::frame* frm_anim;
    lafi::frame* frm_frame;
    lafi::frame* frm_sprites;
    lafi::frame* frm_sprite;
    lafi::frame* frm_sprite_bmp;
    lafi::frame* frm_sprite_tra;
    lafi::frame* frm_sprite_comp;
    lafi::frame* frm_hitboxes;
    lafi::frame* frm_hitbox;
    lafi::frame* frm_normal_h;
    lafi::frame* frm_attack_h;
    lafi::frame* frm_top;
    lafi::frame* frm_body_parts;
    lafi::frame* frm_body_part;
    lafi::frame* frm_tools;
    lafi::frame* frm_options;
    lafi::style* faded_style;
    lafi::style* gui_style;
    
    
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
    bool                      side_view;
    //Top bitmaps for the current Pikmin type.
    ALLEGRO_BITMAP*           top_bmp[N_MATURITIES];
    transformation_controller top_tc;
    
    //General functions.
    ALLEGRO_BITMAP* create_hitbox_bitmap();
    void cur_hitbox_tc_to_gui();
    void cur_sprite_tc_to_gui();
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
    string get_cut_path(const string &p);
    void import_sprite_file_data(const string &name);
    void import_sprite_hitbox_data(const string &name);
    void import_sprite_top_data(const string &name);
    void import_sprite_transformation_data(const string &name);
    void load_animation_database(const bool update_history);
    void open_hitbox_type(unsigned char type);
    void open_picker(const unsigned char type, const bool can_make_new);
    void populate_history();
    void pick_sprite(const string &name);
    void rename_animation();
    void rename_sprite();
    void resize_everything();
    void save_animation_database();
    void set_all_sprite_scales();
    void sprite_bmp_flood_fill(
        ALLEGRO_BITMAP* bmp, bool* selection_pixels,
        const int x, const int y, const int bmp_w, const int bmp_h
    );
    void top_tc_to_gui();
    void update_cur_hitbox_tc();
    void update_hitboxes();
    void update_stats();
    
    //Input handler functions.
    void handle_key_char(const ALLEGRO_EVENT &ev);
    void handle_key_down(const ALLEGRO_EVENT &ev);
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
    
    //GUI functions.
    void animation_to_gui();
    void body_part_to_gui();
    void frame_to_gui();
    void hitbox_to_gui();
    void options_to_gui();
    void sprite_to_gui();
    void sprite_bmp_to_gui();
    void sprite_transform_to_gui();
    void top_to_gui();
    void gui_to_body_part();
    void gui_to_animation();
    void gui_to_frame();
    void gui_to_hitbox();
    void gui_to_options();
    void gui_to_sprite();
    void gui_to_sprite_bmp();
    void gui_to_sprite_transform();
    void gui_to_top();
    
    void hide_all_frames();
    void change_to_right_frame();
    void create_new_from_picker(
        const size_t pipcker_id, const string &name
    );
    void pick(
        const size_t picker_id, const string &name, const string &category
    );
    
public:

    animation_editor();
    
    string auto_load_anim;
    
    void do_logic();
    void do_drawing();
    void load();
    void unload();
    
};

#endif //ifndef ANIMATION_EDITOR_INCLUDED
