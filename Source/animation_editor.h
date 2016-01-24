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
    animation*           cur_anim;
    frame*               cur_frame;
    size_t               cur_frame_instance_nr;
    float                cur_frame_time;
    float                cur_hitbox_alpha;  //The alpha is calculated using the sine of this value.
    size_t               cur_hitbox_instance_nr;
    size_t               cur_hitbox_nr;
    string               file_path;
    ALLEGRO_FILECHOOSER* file_dialog;
    size_t               grabbing_hitbox;   //Hitbox being grabbed by the mouse cursor. string::npos = none.
    bool                 grabbing_hitbox_edge;
    float                grabbing_hitbox_x; //X world coordinate of the point we're grabbing, or the anchor, when in resize mode.
    float                grabbing_hitbox_y;
    lafi::gui*           gui;
    bool                 hitboxes_visible;
    bool                 holding_m1;
    bool                 holding_m2;
    bool                 is_pikmin;
    unsigned char        maturity; //Current maturity of the Pikmin, used to check the visuals of different Pikmin tops.
    unsigned char        mode;
    float                new_hitbox_corner_x; //FLT_MAX = none.
    float                new_hitbox_corner_y;
    unsigned char        sec_mode; //Secondary/sub mode.
    ALLEGRO_BITMAP*      top_bmp[3]; //Top bitmaps for the current Pikmin type.
    lafi::widget*        wum; //Widget under mouse.
    
    void leave();
    void gui_load_animation();
    void gui_load_frame();
    void gui_load_frame_instance();
    void gui_load_hitbox();
    void gui_load_hitbox_instance();
    void gui_load_top();
    void gui_save_animation();
    void gui_save_frame();
    void gui_save_frame_instance();
    void gui_save_hitbox();
    void gui_save_hitbox_instance();
    void gui_save_top();
    void load_animation_pool();
    void open_hitbox_type(unsigned char type);
    void open_picker(unsigned char type, bool can_make_new);
    void pick(string name, unsigned char type);
    void save_animation_pool();
    void update_hitboxes();
    void update_stats();
    
public:

    animation_editor();
    
    virtual void do_logic();
    virtual void do_drawing();
    virtual void handle_controls(ALLEGRO_EVENT ev);
    virtual void load();
    virtual void unload();
    
};

#endif //ifndef ANIMATION_EDITOR_INCLUDED
