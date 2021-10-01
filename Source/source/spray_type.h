/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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


using std::string;
using std::vector;


/* ----------------------------------------------------------------------------
 * A spray type. It decides how the spray behaves,
 * what status effect it causes, and some other values.
 */
class spray_type {
public:
    string name;
    //What the spray does.
    vector<status_type*> effects;
    //True: applied to the entire group. False: applied in a specified range.
    bool group;
    //Does it only apply to Pikmin in the group, or leaders too?
    bool group_pikmin_only;
    //Apply the spray to its user as well.
    bool affects_user;
    //If applied outside of the group, this is the angle of shooting.
    float angle;
    //If applied outside of the group, this is the distance range.
    float distance_range;
    //If applied outside of the group, this is the angle range.
    float angle_range;
    
    ALLEGRO_COLOR main_color;
    //Bitmap for the spray count.
    ALLEGRO_BITMAP* bmp_spray;
    
    //How many ingredients are needed in order to concot a new spray.
    //0 means there are no ingredients for this spray type.
    size_t ingredients_needed;
    
    //Extra effects.
    bool buries_pikmin;
    
    spray_type();
};


#endif //ifndef SPRAY_TYPE_INCLUDED
