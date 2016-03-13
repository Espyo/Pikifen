/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pellet type class and pellet type-related functions.
 */

#ifndef PELLET_TYPE_INCLUDED
#define PELLET_TYPE_INCLUDED

#include "../data_file.h"
#include "mob_type.h"
#include "pikmin_type.h"

/* ----------------------------------------------------------------------------
 * A pellet type. Contains info on
 * how many seeds the Onion should receive,
 * depending on whether it matches the Pikmin
 * type or not.
 */
class pellet_type : public mob_type {
public:
    pikmin_type* pik_type;
    unsigned number; //Number on the pellet, and hence, its weight.
    unsigned match_seeds; //Number of seeds given out if the pellet's taken to a matching Onion.
    unsigned non_match_seeds; //Number of seeds given out if the pellet's taken to a non-matching Onion.
    ALLEGRO_BITMAP* bmp_number;
    
    pellet_type();
    void load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions);
};

#endif //ifndef PELLET_TYPE_INCLUDED
