/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the hazard class and hazard-related functions.
 */

#ifndef HAZARD_INCLUDED
#define HAZARD_INCLUDED

#include <allegro5/allegro.h>

#include <string>
#include <vector>

using namespace std;

struct liquid;
struct status_type;

/* ----------------------------------------------------------------------------
 * An hazard is the likes of fire, water, electricty, crushing, etc.
 * Pikmin can be vulnerable or invulnerable to these.
 * Most of the time, hazards are elements (of nature), but
 * this is not necessarily the case. A hazard is just an abstract danger,
 * not an object that emits said danger.
 */
struct hazard {
    string name;
    ALLEGRO_COLOR main_color;
    vector<status_type*> effects;
    liquid* associated_liquid;
    
    hazard();
};

#endif //ifndef HAZARD_INCLUDED
