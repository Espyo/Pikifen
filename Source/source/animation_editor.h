/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the animation editor-related functions.
 */

#ifndef ANIMATION_EDITOR_INCLUDED
#define ANIMATION_EDITOR_INCLUDED

#include <string>

#include <allegro5/allegro_native_dialog.h>

#include "animation.h"
#include "game_state.h"
#include "hitbox.h"
#include "LAFI/gui.h"
#include "LAFI/widget.h"
#include "misc_structs.h"

using namespace std;

class animation_editor : public game_state {
private:

    enum ANIMATION_EDITOR_PICKER_TYPES {
        ANIMATION_EDITOR_PICKER_ANIMATION,
        ANIMATION_EDITOR_PICKER_FRAME_INSTANCE,
        ANIMATION_EDITOR_PICKER_FRAME,
        ANIMATION_EDITOR_PICKER_HITBOX_INSTANCE,
        ANIMATION_EDITOR_PICKER_HITBOX,
    };
    
    animation_pool       anims;
    bool                 anim_playing;
    bool                 comparison;
    frame*               comparison_frame;
    bool                 comparison_blink;
    bool                 comparison_blink_show;
    timer                comparison_blink_timer;
    animation*           cur_anim;
    frame*               cur_frame;
    size_t               cur_frame_instance_nr;
    float                cur_frame_time;
    //The alpha is calculated using the sine of this value.
    float                cur_hitbox_alpha;
    size_t               cur_hitbox_instance_nr;
    size_t               cur_hitbox_nr;
    string               file_path;
    ALLEGRO_FILECHOOSER* file_dialog;
    bool                 frame_offset_with_mouse;
    //Hitbox being grabbed by the mouse cursor. INVALID = none.
    size_t               grabbing_hitbox;
    bool                 grabbing_hitbox_edge;
    //X world coordinate of the point we're grabbing,
    //or the anchor, when in resize mode.
    float                grabbing_hitbox_x;
    float                grabbing_hitbox_y;
    lafi::gui*           gui;
    bool                 hitboxes_visible;
    bool                 holding_m1;
    bool                 holding_m2;
    bool                 is_pikmin;
    string               last_file_used;
    bool                 made_changes;
    //Current maturity of the Pikmin,
    //used to check the visuals of different Pikmin tops.
    unsigned char        maturity;
    unsigned char        mode;
    //Hitbox corner coordinates. FLT_MAX = none.
    float                new_hitbox_corner_x;
    float                new_hitbox_corner_y;
    //Secondary/sub mode.
    unsigned char        sec_mode;
    //Top bitmaps for the current Pikmin type.
    ALLEGRO_BITMAP*      top_bmp[3];
    //Widget under mouse.
    lafi::widget*        wum;
    
    void close_changes_warning();
    string get_cut_path(const string &p);
    void gui_load_animation();
    void gui_load_frame();
    void gui_load_frame_instance();
    void gui_load_frame_offset();
    void gui_load_hitbox();
    void gui_load_hitbox_instance();
    void gui_load_top();
    void gui_save_animation();
    void gui_save_frame();
    void gui_save_frame_instance();
    void gui_save_frame_offset();
    void gui_save_hitbox();
    void gui_save_hitbox_instance();
    void gui_save_top();
    void leave();
    void load_animation_pool();
    void open_hitbox_type(unsigned char type);
    void open_picker(unsigned char type, bool can_make_new);
    void pick(string name, unsigned char type);
    void populate_history();
    void resize_everything();
    void save_animation_pool();
    void show_changes_warning();
    void update_hitboxes();
    void update_stats();
    
public:

    animation_editor();
    
    string auto_load_anim;
    
    virtual void do_logic();
    virtual void do_drawing();
    virtual void handle_controls(ALLEGRO_EVENT ev);
    virtual void load();
    virtual void unload();
    
};

#endif //ifndef ANIMATION_EDITOR_INCLUDED
