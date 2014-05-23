/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the area editor-related functions.
 */

#ifndef AREA_EDITOR_INCLUDED
#define AREA_EDITOR_INCLUDED

#include <allegro5/allegro.h>

namespace area_editor {

void change_to_right_frame();
void do_logic();
void handle_controls(ALLEGRO_EVENT ev);
void load();
void load_bg_to_gui();
void save_bg_from_gui();
float snap_to_grid(const float c);

};

#endif //ifndef AREA_EDITOR_INCLUDED