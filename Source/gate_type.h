/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gate type class and gate type-related functions.
 */

#ifndef GATE_TYPE_INCLUDED
#define GATE_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * A type of gate. Brown, white, blue, whatever.
 */
class gate_type : public mob_type {
private:
    void init_script();
    
public:
    gate_type();
};

enum GATE_ANIMATIONS {
    GATE_ANIM_IDLE,
    GATE_ANIM_NOTHING,
};

#endif //ifndef GATE_TYPE_INCLUDED