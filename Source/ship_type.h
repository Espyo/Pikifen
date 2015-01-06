/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship type class and ship type-related functions.
 */

#ifndef SHIP_TYPE_INCLUDED
#define SHIP_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "misc_structs.h"
#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * A type of ship (Hocotate Ship, research pod, golden HS, golden RP, ...).
 */
class ship_type : public mob_type {
public:

    bool can_heal;
    
    ship_type();
};

#endif //ifndef SHIP_TYPE_INCLUDED
