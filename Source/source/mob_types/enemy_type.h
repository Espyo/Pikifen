/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the enemy type class and enemy type-related functions.
 */

#ifndef ENEMY_TYPE_INCLUDED
#define ENEMY_TYPE_INCLUDED

#include <string>
#include <vector>

#include "../data_file.h"
#include "mob_type.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * A type of enemy. A species, if you will.
 * Red Bulborb, Orange Bulborb, Cloaking Burrow-nit, etc.
 */
class enemy_type : public mob_type {
public:
    size_t pikmin_seeds;
    bool drops_corpse;
    bool allow_ground_attacks;
    
    enemy_type();
    ~enemy_type();
    
    void load_properties(data_node* file);
};

#endif //ifndef ENEMY_TYPE_INCLUDED
