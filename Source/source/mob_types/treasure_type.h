/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the treasure type class and treasure type-related functions.
 */

#ifndef TREASURE_TYPE_INCLUDED
#define TREASURE_TYPE_INCLUDED

#include "../data_file.h"
#include "mob_type.h"

enum TREASURE_STATES {
    TREASURE_STATE_IDLE_WAITING,
    TREASURE_STATE_IDLE_MOVING,
    TREASURE_STATE_IDLE_STUCK,
    TREASURE_STATE_BEING_DELIVERED,
    
    N_TREASURE_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of treasure.
 * Although uncommon, there can be several
 * treasures of the same type at once.
 * Like the "small red marble" treasure
 * type in Pikmin 2; you can see multiple
 * treasures of that type in some
 * Challenge Mode levels.
 */
class treasure_type : public mob_type {
public:

    treasure_type();
    ~treasure_type();
    anim_conversion_vector get_anim_conversions();
};

#endif //ifndef TREASURE_TYPE_INCLUDED
