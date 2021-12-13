/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the decoration class and decoration-related functions.
 */

#ifndef DECORATION_INCLUDED
#define DECORATION_INCLUDED

#include "../mob_types/decoration_type.h"
#include "mob.h"


/* ----------------------------------------------------------------------------
 * A decoration mob, like a plant. They don't do much other than be pretty
 * and get bumped occasionally.
 */
class decoration : public mob {
public:
    //What type of decoration it is.
    decoration_type* dec_type;
    
    //This decoration's random tint, if any.
    ALLEGRO_COLOR individual_tint;
    //This decoration's random scale, if any.
    float individual_scale;
    //This decoration's random rotation, if any.
    float individual_rotation;
    
    //Constructor.
    decoration(const point &pos, decoration_type* dec_type, const float angle);
    
    //Mob drawing routine.
    virtual void draw_mob();
};


#endif //ifndef DECORATION_INCLUDED
