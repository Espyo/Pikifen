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

    enum EDITOR_MODES {
        EDITOR_MODE_MAIN,
        EDITOR_MODE_ANIMATION,
        EDITOR_MODE_SPRITE,
        EDITOR_MODE_BODY_PART,
        EDITOR_MODE_HITBOXES,
        EDITOR_MODE_SPRITE_BITMAP,
        EDITOR_MODE_SPRITE_TRANSFORM,
        EDITOR_MODE_TOP,
        EDITOR_MODE_HISTORY,
        EDITOR_MODE_TOOLS,
    };
    
    enum ANIMATION_EDITOR_PICKER_TYPES {
        ANIMATION_EDITOR_PICKER_MOB_TYPES,
        ANIMATION_EDITOR_PICKER_GLOBAL_ANIMS,
        ANIMATION_EDITOR_PICKER_ANIMATION,
        ANIMATION_EDITOR_PICKER_SPRITE,
    };
    
    enum PICKER_DISAMBIGS {
        PICKER_DISAMBIG_LOAD,
        PICKER_DISAMBIG_IMPORT,
        PICKER_DISAMBIG_COMPARISON,
    };
    
    static const float ZOOM_MAX_LEVEL_EDITOR;
    static const float ZOOM_MIN_LEVEL_EDITOR;
    
    
    static const string ICON_DELETE;
    static const string ICON_DUPLICATE;
    static const string ICON_EXIT;
    static const string ICON_HITBOXES;
    static const string ICON_INFO;
    static const string ICON_LOAD;
    static const string ICON_MOVE_LEFT;
    static const string ICON_MOVE_RIGHT;
    static const string ICON_NEW;
    static const string ICON_NEXT;
    static const string ICON_PLAY_PAUSE;
    static const string ICON_PREVIOUS;
    static const string ICON_SAVE;
    
    //GUI widgets.
    lafi::frame* frm_main;
    lafi::frame* frm_object;
    lafi::frame* frm_history;
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
    lafi::frame* frm_bottom;
    
    
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
    //The alpha is calculated using the sine of this value.
    float                     cur_hitbox_alpha;
    size_t                    cur_hitbox_nr;
    //Current maturity of the Pikmin,
    //used to check the visuals of different Pikmin tops.
    unsigned char             cur_maturity;
    sprite*                   cur_sprite;
    transformation_controller cur_sprite_tc;
    string                    file_path;
    //Hitbox being grabbed by the mouse cursor. INVALID = none.
    size_t                    grabbing_hitbox;
    bool                      grabbing_hitbox_edge;
    //X world coordinate of the point we're grabbing,
    //or the anchor, when in resize mode.
    point                     grabbing_hitbox_point;
    bool                      hitboxes_visible;
    string                    last_file_used;
    mob_type*                 loaded_mob_type;
    //Disambiguation for the exact kind of picker. Use PICKER_DISAMBIG_*.
    unsigned char             picker_disambig;
    //Top bitmaps for the current Pikmin type.
    ALLEGRO_BITMAP*           top_bmp[N_MATURITIES];
    transformation_controller top_tc;
    
    //General functions.
    void cur_sprite_tc_to_gui();
    void draw_comparison();
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
    void resize_by_resolution();
    void resize_everything();
    void save_animation_database();
    void sprite_bmp_flood_fill(
        ALLEGRO_BITMAP* bmp, bool* selection_pixels,
        const int x, const int y, const int bmp_w, const int bmp_h
    );
    void top_tc_to_gui();
    void update_hitboxes();
    void update_stats();
    
    //Input handler functions.
    virtual void handle_key_down(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_double_click(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_down(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_drag(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_up(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_double_click(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_down(const ALLEGRO_EVENT &ev);
    virtual void handle_mouse_update(const ALLEGRO_EVENT &ev);
    virtual void handle_mouse_wheel(const ALLEGRO_EVENT &ev);
    virtual void handle_rmb_drag(const ALLEGRO_EVENT &ev);
    
    //GUI functions.
    void animation_to_gui();
    void body_part_to_gui();
    void frame_to_gui();
    void hitbox_to_gui();
    void sprite_to_gui();
    void sprite_bmp_to_gui();
    void sprite_transform_to_gui();
    void top_to_gui();
    void gui_to_body_part();
    void gui_to_animation();
    void gui_to_frame();
    void gui_to_hitbox();
    void gui_to_sprite();
    void gui_to_sprite_bmp();
    void gui_to_sprite_transform();
    void gui_to_top();
    
    virtual void hide_all_frames();
    virtual void change_to_right_frame();
    virtual void create_new_from_picker(const string &name);
    virtual void pick(const string &name, const string &category);
    
public:

    animation_editor();
    
    string auto_load_anim;
    
    virtual void do_logic();
    virtual void do_drawing();
    virtual void load();
    virtual void unload();
    virtual void update_transformations();
    
};

#endif //ifndef ANIMATION_EDITOR_INCLUDED
