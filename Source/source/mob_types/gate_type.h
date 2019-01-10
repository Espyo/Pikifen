/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gate type class and gate type-related functions.
 */

#ifndef GATE_TYPE_INCLUDED
#define GATE_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../data_file.h"
#include "mob_type.h"

enum GATE_ANIMATIONS {
    GATE_ANIM_IDLING,
    GATE_ANIM_DESTROYED,
};


/* ----------------------------------------------------------------------------
 * A type of gate. Brown, white, blue, whatever.
 */
class gate_type : public mob_type {
public:
    gate_type();
    ~gate_type();
    anim_conversion_vector get_anim_conversions();
};

#endif //ifndef GATE_TYPE_INCLUDED
