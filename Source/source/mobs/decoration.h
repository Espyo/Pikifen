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

enum DECORATION_STATES {
    DECORATION_STATE_IDLING,
    DECORATION_STATE_BUMPED,
    
    N_DECORATION_STATES,
};


/* ----------------------------------------------------------------------------
 * A decoration mob, like a plant. They don't do much other than be pretty
 * and get bumped occasionally.
 */
class decoration : public mob {
public:
    decoration_type* dec_type;
    
    ALLEGRO_COLOR individual_tint;
    float individual_scale;
    float individual_rotation;
    bool has_done_first_animation;
    
    decoration(const point &pos, decoration_type* dec_type, const float angle);
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
};

#endif //ifndef DECORATION_INCLUDED
