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
        EDITOR_MODE_SPRITE_TRANSFORM,
        EDITOR_MODE_TOP,
        EDITOR_MODE_HISTORY,
        EDITOR_MODE_TOOLS,
    };
    
    enum ANIMATION_EDITOR_PICKER_TYPES {
        ANIMATION_EDITOR_PICKER_ANIMATION,
        ANIMATION_EDITOR_PICKER_SPRITE,
    };
    
    enum LMB_ACTION {
        LMB_ACTION_NONE,
        LMB_ACTION_MOVE,
        LMB_ACTION_RESIZE,
        LMB_ACTION_ROTATE,
    };
    
    static const float  ZOOM_MAX_LEVEL_EDITOR;
    static const float  ZOOM_MIN_LEVEL_EDITOR;
    
    static const string DELETE_ICON;
    static const string DUPLICATE_ICON;
    static const string EXIT_ICON;
    static const string HITBOXES_ICON;
    static const string LOAD_ICON;
    static const string MOVE_LEFT_ICON;
    static const string MOVE_RIGHT_ICON;
    static const string NEW_ICON;
    static const string NEXT_ICON;
    static const string PLAY_PAUSE_ICON;
    static const string PREVIOUS_ICON;
    static const string SAVE_ICON;
    
    
    animation_database   anims;
    bool                 anim_playing;
    bool                 comparison;
    sprite*              comparison_sprite;
    bool                 comparison_blink;
    bool                 comparison_blink_show;
    timer                comparison_blink_timer;
    animation*           cur_anim;
    size_t               cur_body_part_nr;
    size_t               cur_frame_nr;
    float                cur_frame_time;
    //The alpha is calculated using the sine of this value.
    float                cur_hitbox_alpha;
    size_t               cur_hitbox_nr;
    //Current maturity of the Pikmin,
    //used to check the visuals of different Pikmin tops.
    unsigned char        cur_maturity;
    sprite*              cur_sprite;
    string               file_path;
    ALLEGRO_FILECHOOSER* file_dialog;
    //Hitbox being grabbed by the mouse cursor. INVALID = none.
    size_t               grabbing_hitbox;
    bool                 grabbing_hitbox_edge;
    //X world coordinate of the point we're grabbing,
    //or the anchor, when in resize mode.
    point                grabbing_hitbox_point;
    bool                 hitboxes_visible;
    bool                 is_pikmin;
    string               last_file_used;
    //Top bitmaps for the current Pikmin type.
    ALLEGRO_BITMAP*      top_bmp[N_MATURITIES];
    unsigned char        sprite_tra_lmb_action;
    unsigned char        top_lmb_action;
    
    string get_cut_path(const string &p);
    void animation_to_gui();
    void body_part_to_gui();
    void frame_to_gui();
    void hitbox_to_gui();
    void sprite_to_gui();
    void sprite_transform_to_gui();
    void top_to_gui();
    void gui_to_body_part();
    void gui_to_animation();
    void gui_to_frame();
    void gui_to_hitbox();
    void gui_to_sprite();
    void gui_to_sprite_transform();
    void gui_to_top();
    void load_animation_database();
    void open_hitbox_type(unsigned char type);
    void open_picker(const unsigned char type, const bool can_make_new);
    void populate_history();
    void rename_animation();
    void rename_sprite();
    void resize_by_resolution();
    void resize_everything();
    void save_animation_database();
    void update_hitboxes();
    void update_stats();
    
    virtual void hide_all_frames();
    virtual void change_to_right_frame();
    virtual void create_new_from_picker(const string &name);
    virtual void pick(const string &name, const string &category);
    
public:

    animation_editor();
    
    string auto_load_anim;
    
    virtual void do_logic();
    virtual void do_drawing();
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void load();
    virtual void unload();
    virtual void update_transformations();
    
};

#endif //ifndef ANIMATION_EDITOR_INCLUDED
