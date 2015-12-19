/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the hazard class and hazard-related functions.
 */

#ifndef HAZARD_INCLUDED
#define HAZARD_INCLUDED

#include <allegro5/allegro.h>

#include <string>

using namespace std;

/* ----------------------------------------------------------------------------
 * An hazard is the likes of fire, water, electricty, crushing, etc.
 * Pikmin can be vulnerable or invulnerable to these.
 * Most of the time, hazards are elements (of nature), but
 * this is not necessarily the case. A hazard is just an abstract danger.
 */
class hazard {
public:
    ALLEGRO_COLOR main_color;
};

#endif //ifndef HAZARD_INCLUDED
