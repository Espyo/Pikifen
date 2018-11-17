/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drop type class and drop type-related functions.
 */

#ifndef DROP_TYPE_INCLUDED
#define DROP_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../data_file.h"
#include "mob_type.h"
#include "../spray_type.h"
#include "../status.h"

enum DROP_ANIMATIONS {
    DROP_ANIM_IDLING,
    DROP_ANIM_FALLING,
    DROP_ANIM_LANDING,
    DROP_ANIM_TOUCHED,
};

enum DROP_CONSUMERS {
    DROP_CONSUMER_LEADERS,
    DROP_CONSUMER_PIKMIN,
};

enum DROP_EFFECT {
    DROP_EFFECT_MATURATE,
    DROP_EFFECT_INCREASE_SPRAYS,
    DROP_EFFECT_GIVE_STATUS,
};


/* ----------------------------------------------------------------------------
 * A type of drop, like a nectar drop, spray drop, etc.
 */
class drop_type : public mob_type {
public:
    unsigned char consumer;
    unsigned char effect;
    size_t total_doses;
    int increase_amount;
    spray_type* spray_type_to_increase;
    status* status_to_give;
    
    drop_type();
    ~drop_type();
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions();
    void unload_resources();
};

#endif //ifndef DROP_TYPE_INCLUDED
