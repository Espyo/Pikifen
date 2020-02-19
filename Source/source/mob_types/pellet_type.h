/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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
    //Number on the pellet, and hence, its weight.
    size_t number;
    //Number of seeds given out if the pellet's taken to a matching Onion.
    size_t match_seeds;
    //Number of seeds given out if the pellet's taken to a non-matching Onion.
    size_t non_match_seeds;
    ALLEGRO_BITMAP* bmp_number;
    
    pellet_type();
    ~pellet_type();
    void load_parameters(data_node* file);
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions();
    void unload_resources();
    
};

#endif //ifndef PELLET_TYPE_INCLUDED
