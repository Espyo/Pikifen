/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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
 */
class enemy_type : public mob_type {
public:
    unsigned char pikmin_seeds;
    float value;
    float revive_speed;
    float regenerate_speed;
    bool is_boss;
    bool drops_corpse;
    
    enemy_type();
    
    void load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions);
};

#endif //ifndef ENEMY_TYPE_INCLUDED
