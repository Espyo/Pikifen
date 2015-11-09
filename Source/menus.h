/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the menus.
 */

#ifndef MENUS_INCLUDED
#define MENUS_INCLUDED

#include <vector>

#include <allegro5/allegro.h>

#include "menu_widgets.h"

using namespace std;

namespace main_menu {

extern ALLEGRO_BITMAP* bmp_menu_bg;
extern menu_widget* selected_widget;
extern vector<menu_widget*> menu_widgets;
extern size_t new_game_state;

void load();
void unload();
void handle_controls(ALLEGRO_EVENT ev);
void do_logic();
void set_selected(menu_widget* widget);

}

#endif //ifndef MENUS_INCLUDED
