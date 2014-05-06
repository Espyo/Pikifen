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
void do_area_editor_logic();
void handle_area_editor_controls(ALLEGRO_EVENT ev);
};

#endif //ifndef AREA_EDITOR_INCLUDED