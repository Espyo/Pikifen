/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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


using std::string;
using std::vector;


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
    //Name of the hazard.
    string name;
    //Color that best represents this hazard.
    ALLEGRO_COLOR main_color;
    //Status effects for mobs that interact with this hazard.
    vector<status_type*> effects;
    //If it's got an associated liquid, this points to it.
    liquid* associated_liquid;
    
    hazard();
};


#endif //ifndef HAZARD_INCLUDED
