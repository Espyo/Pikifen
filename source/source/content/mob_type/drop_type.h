/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drop type class and drop type-related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include "../../lib/data_file/data_file.h"
#include "../../util/enum_utils.h"
#include "../../util/general_utils.h"
#include "../other/spray_type.h"
#include "../other/status.h"
#include "mob_type.h"


//Drop object animations.
enum DROP_ANIM {

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
enum DROP_CONSUMER {

    //Pikmin.
    DROP_CONSUMER_PIKMIN,
    
    //Leaders.
    DROP_CONSUMER_LEADERS,
    
};


//Drop consumer enum naming (internal names).
buildEnumNames(dropConsumerINames, DROP_CONSUMER)({
    { DROP_CONSUMER_PIKMIN, "pikmin" },
    { DROP_CONSUMER_LEADERS, "leaders" },
});


//Possible drop consumption effects.
enum DROP_EFFECT {

    //Maturate a Pikmin.
    DROP_EFFECT_MATURATE,
    
    //Increase spray amount.
    DROP_EFFECT_INCREASE_SPRAYS,
    
    //Give a status effect.
    DROP_EFFECT_GIVE_STATUS,
    
};


//Drop effect enum naming (internal names).
buildEnumNames(dropEffectINames, DROP_EFFECT)({
    { DROP_EFFECT_MATURATE, "maturate" },
    { DROP_EFFECT_INCREASE_SPRAYS, "increase_sprays" },
    { DROP_EFFECT_GIVE_STATUS, "give_status" },
});


//Drop object states.
enum DROP_STATE {

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


/**
 * @brief A type of drop, like a nectar drop, spray drop, etc.
 */
class DropType : public MobType {

public:

    //--- Public members ---
    
    //What sorts of mobs can consume this drop.
    DROP_CONSUMER consumer = DROP_CONSUMER_PIKMIN;
    
    //Effects upon consumption.
    DROP_EFFECT effect = DROP_EFFECT_MATURATE;
    
    //How many doses does this drop have? i.e. how many mobs can it serve?
    size_t totalDoses = 1;
    
    //If the consumption effect increases something, this specifies the amount.
    int increaseAmount = 2;
    
    //If it increases a spray type count, this specifies the spray type index.
    size_t sprayTypeToIncrease = INVALID;
    
    //If it gives a status effect, this points to the status type.
    StatusType* statusToGive = nullptr;
    
    //How quickly it shrinks. Aesthetic only.
    float shrinkSpeed = 40.0f;
    
    
    //--- Public function declarations ---
    
    DropType();
    void loadCatProperties(DataNode* file) override;
    AnimConversionVector getAnimConversions() const override;
    
};
