/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the info spot class and info spot-related functions.
 */

#ifndef INFO_SPOT_INCLUDED
#define INFO_SPOT_INCLUDED

#include <string>

#include "mob.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * An info spot is basically like a sign post:
 * it's scattered around the map and contains info.
 * It's not a thing in any Pikmin game, but I figured
 * it could be useful.
 * When the player approaches it, its text appears.
 * If the text is too big, the player can "attack" near it,
 * in order to open a message box, where more info fits.
 */
class info_spot : public mob {
public:
    string text;
    //If true, clicking while near this info spot
    //opens a message box with the full text.
    bool opens_box;
    unsigned int text_w; //Used instead of calculating the width every time.
    
    info_spot(
        const point &pos, const float angle, const string &vars
    );
    virtual void draw(sprite_effect_manager* effect_manager = NULL);
};

#endif //ifndef INFO_SPOT_INCLUDED
