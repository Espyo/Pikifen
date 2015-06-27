/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the animation editor-related functions.
 */

#ifndef ANIMATION_EDITOR_INCLUDED
#define ANIMATION_EDITOR_INCLUDED

#include <string>

#include "animation.h"
#include "hitbox.h"
#include "LAFI/gui.h"
#include "LAFI/widget.h"

using namespace std;

namespace animation_editor {
enum ANIMATION_EDITOR_PICKER_TYPES {
    ANIMATION_EDITOR_PICKER_ANIMATION,
    ANIMATION_EDITOR_PICKER_FRAME_INSTANCE,
    ANIMATION_EDITOR_PICKER_FRAME,
    ANIMATION_EDITOR_PICKER_HITBOX_INSTANCE,
    ANIMATION_EDITOR_PICKER_HITBOX,
    ANIMATION_EDITOR_PICKER_OBJECT, // Make sure this is the last one.
};



void do_logic();
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
void handle_controls(ALLEGRO_EVENT ev);
void load();
void load_animation_set();
void open_hitbox_type(unsigned char type);
void open_picker(unsigned char type, bool can_make_new);
void pick(string name, unsigned char type);
void save_animation_set();
void update_hitboxes();
void update_stats();

extern animation_set    anims;
extern bool             anim_playing;
extern animation*       cur_anim;
extern frame*           cur_frame;
extern size_t           cur_frame_instance_nr;
extern float            cur_frame_time;
extern float            cur_hitbox_alpha;  // The alpha is calculated using the sine of this value.
extern size_t           cur_hitbox_instance_nr;
extern size_t           cur_hitbox_nr;
extern string           file_name;
extern size_t           grabbing_hitbox;   // Hitbox being grabbed by the mouse cursor. string::npos = none.
extern bool             grabbing_hitbox_edge;
extern float            grabbing_hitbox_x; // X world coordinate of the point we're grabbing, or the anchor, when in resize mode.
extern float            grabbing_hitbox_y;
extern lafi::gui*       gui;
extern bool             hitboxes_visible;
extern bool             holding_m1;
extern bool             holding_m2;
extern unsigned char    maturity; // Current maturity of the Pikmin, used to check the visuals of different Pikmin tops.
extern unsigned char    mob_type_list; // Use MOB_TYPE_*.
extern unsigned char    mode;
extern float            new_hitbox_corner_x; // FLT_MAX = none.
extern float            new_hitbox_corner_y;
extern string           object_name;
extern unsigned char    sec_mode; // Secondary/sub mode.
extern ALLEGRO_BITMAP*  top_bmp[3]; // Top bitmaps for the current Pikmin type.
extern lafi::widget*    wum; // Widget under mouse.


}

#endif // ifndef ANIMATION_EDITOR_INCLUDED
