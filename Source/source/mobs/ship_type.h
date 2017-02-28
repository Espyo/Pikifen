/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship type class and ship type-related functions.
 */

#ifndef SHIP_TYPE_INCLUDED
#define SHIP_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../data_file.h"
#include "../misc_structs.h"
#include "mob_type.h"

enum SHIP_ANIMATIONS {
    SHIP_ANIM_IDLING,
};

/* ----------------------------------------------------------------------------
 * A type of ship (Hocotate ship, research pod, golden HS, S.S. Drake, etc.).
 */
class ship_type : public mob_type {
public:

    bool can_heal;
    point beam_offset;
    float beam_radius;
    
    ship_type();
    void load_from_file(
        data_node* file, const bool load_resources,
        vector<pair<size_t, string> >* anim_conversions
    );
};

#endif //ifndef SHIP_TYPE_INCLUDED
