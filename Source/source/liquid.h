/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the liquid class and liquid-related functions.
 */

#ifndef LIQUID_INCLUDED
#define LIQUID_INCLUDED

#include <allegro5/allegro.h>

#include <string>
#include <vector>

#include "animation.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * A liquid type defines how a sector should look to make it look like water.
 * This is considered a "liquid" and not specifically "water" because the
 * engine allows creating other water-like things, like acid, lava, etc.
 * Each have their own color, reflectivity, etc.
 * A hazard can be associated with a liquid. It's the way the
 * engine has to know if a sector is to be shown as a liquid or not.
 */
struct liquid {
    string name;
    ALLEGRO_COLOR main_color;
    animation_database anim_pool;
    animation_instance anim_instance;
    float surface_speed[2];
    unsigned char surface_alpha;
    
    liquid();
};

#endif //ifndef LIQUID_INCLUDED
