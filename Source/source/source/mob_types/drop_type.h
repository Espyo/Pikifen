/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drop type class and drop type-related functions.
 */

#ifndef DROP_TYPE_INCLUDED
#define DROP_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../spray_type.h"
#include "../status.h"
#include "../libs/data_file.h"
#include "mob_type.h"


//Drop object animations.
enum DROP_ANIMATIONS {
    //Idling.
    DROP_ANIM_IDLING,
    //Falling.
    DROP_ANIM_FALLING,
    //Landing.
    DROP_ANIM_LANDING,
    //Bumped against.
    DROP_ANIM_BUMPED,
};


//Possible drop consumers.
enum DROP_CONSUMERS {
    //Pikmin.
    DROP_CONSUMER_PIKMIN,
    //Leaders.
    DROP_CONSUMER_LEADERS,
};


//Possible drop consumption effects.
enum DROP_EFFECTS {
    //Maturate a Pikmin.
    DROP_EFFECT_MATURATE,
    //Increase spray amount.
    DROP_EFFECT_INCREASE_SPRAYS,
    //Give a status effect.
    DROP_EFFECT_GIVE_STATUS,
};


//Drop object states.
enum DROP_STATES {
    //Idling.
    DROP_STATE_IDLING,
    //Falling.
    DROP_STATE_FALLING,
    //Landing.
    DROP_STATE_LANDING,
    //Bumped against.
    DROP_STATE_BUMPED,
    
    //Total amount of drop object states.
    N_DROP_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of drop, like a nectar drop, spray drop, etc.
 */
class drop_type : public mob_type {
public:
    //What sorts of mobs can consume this drop.
    DROP_CONSUMERS consumer;
    //Effects upon consumption.
    DROP_EFFECTS effect;
    //How many doses does this drop have? i.e. how many mobs can it serve?
    size_t total_doses;
    //If the consumption effect increases something, this specifies the amount.
    int increase_amount;
    //If it increases a spray type count, this specifies the spray type index.
    size_t spray_type_to_increase;
    //If it gives a status effect, this points to the status type.
    status_type* status_to_give;
    //How quickly it shrinks. Aesthetic only.
    float shrink_speed;
    
    drop_type();
    void load_properties(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
};


#endif //ifndef DROP_TYPE_INCLUDED
