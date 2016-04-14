/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the spray type and spray type-related functions.
 */

#ifndef SPRAY_TYPE_INCLUDED
#define SPRAY_TYPE_INCLUDED

#include <vector>

#include <allegro5/allegro.h>

#include "status.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * A spray type. It decides how the spray behaves,
 * what status effect it causes, and some other values.
 */
class spray_type {
public:
    string name;
    vector<status_type*> effects; //What the spray does.
    bool to_party;                //True: applied to the entire party. False: applied in a specified range.
    float angle;                  //If applied outside of the party, this is the angle of shooting.
    float distance_range;         //If applied outside of the party, this is the distance range.
    float angle_range;            //If applied outside of the party, this is the angle range.
    
    ALLEGRO_COLOR main_color;
    ALLEGRO_BITMAP* bmp_spray; //Bitmap for the spray count.
    
    size_t berries_needed; //How many berries are needed in order to concot a new spray. 0 means there are no berries for this spray type.
    
    spray_type();
};

#endif //ifndef SPRAY_TYPE_INCLUDED
