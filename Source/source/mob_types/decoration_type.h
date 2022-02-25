/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the decoration type class and decoration type-related functions.
 */

#ifndef DECORATION_TYPE_INCLUDED
#define DECORATION_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../utils/data_file.h"
#include "mob_type.h"


//Decoration object animations.
enum DECORATION_ANIMATIONS {
    //Idling.
    DECORATION_ANIM_IDLING,
    //Bumped against.
    DECORATION_ANIM_BUMPED,
};

//Decoration object states.
enum DECORATION_STATES {
    //Idling.
    DECORATION_STATE_IDLING,
    //Bumped against.
    DECORATION_STATE_BUMPED,
    
    //Total amount of decoration object states.
    N_DECORATION_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of decoration.
 */
class decoration_type : public mob_type {
public:
    ALLEGRO_COLOR tint_random_maximum;
    float scale_random_variation;
    float rotation_random_variation;
    bool random_animation_delay;
    
    decoration_type();
    void load_properties(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
};


#endif //ifndef DECORATION_TYPE_INCLUDED
