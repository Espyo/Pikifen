/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the element class and element-related functions.
 */

#ifndef ELEMENT_INCLUDED
#define ELEMENT_INCLUDED

#include <allegro5/allegro.h>

#include <string>

using namespace std;

/* ----------------------------------------------------------------------------
 * An element is the likes of fire, water, electricty, etc.
 * Pikmin can be vulnerable or invulnerable to these.
 */
class element {
public:
    ALLEGRO_COLOR main_color;
};

#endif // ifndef ELEMENT_INCLUDED
