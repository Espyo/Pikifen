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

using namespace std;

namespace animation_editor {
enum ANIMATION_EDITOR_PICKER_TYPES {
    ANIMATION_EDITOR_PICKER_ANIMATION,
    ANIMATION_EDITOR_PICKER_FRAME_INSTANCE,
    ANIMATION_EDITOR_PICKER_FRAME,
    ANIMATION_EDITOR_PICKER_HITBOX_INSTANCE,
    ANIMATION_EDITOR_PICKER_HITBOX,
    ANIMATION_EDITOR_PICKER_OBJECT, //Make sure this is the last one.
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
void update_stats();
}

#endif //ifndef ANIMATION_EDITOR_INCLUDED
