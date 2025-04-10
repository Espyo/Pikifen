/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the decoration type class and decoration type-related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include "../../lib/data_file/data_file.h"
#include "../../util/drawing_utils.h"
#include "mob_type.h"


//Decoration object animations.
enum DECORATION_ANIM {

    //Idling.
    DECORATION_ANIM_IDLING,
    
    //Bumped against.
    DECORATION_ANIM_BUMPED,
    
};


//Decoration object states.
enum DECORATION_STATE {

    //Idling.
    DECORATION_STATE_IDLING,
    
    //Bumped against.
    DECORATION_STATE_BUMPED,
    
    //Total amount of decoration object states.
    N_DECORATION_STATES,
    
};


/**
 * @brief A type of decoration.
 */
class DecorationType : public MobType {

public:

    //--- Members ---
    
    //Maximum amount it can deviate the tint by, for every color component.
    ALLEGRO_COLOR tint_random_maximum = COLOR_EMPTY;
    
    //Maximum amount it can deviate the scale by.
    float scale_random_variation = 0.0f;
    
    //Maximum amount it can deviate the rotation by.
    float rotation_random_variation = 0.0f;
    
    //Should it skip to a random point of the animation when it starts?
    bool random_animation_delay = false;
    
    
    //--- Function declarations ---
    
    DecorationType();
    void loadCatProperties(DataNode* file) override;
    anim_conversion_vector getAnimConversions() const override;
    
};
