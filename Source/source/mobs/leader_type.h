/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader type class and leader type-related functions.
 */

#ifndef LEADER_TYPE_INCLUDED
#define LEADER_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../data_file.h"
#include "../misc_structs.h"
#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * A type of leader. The "leader" class is a mob, so the walking Olimar,
 * walking Louie, etc. This leader type is actually the definition of
 * what the leader is like. Maybe this will be clearer:
 * The same way you have enemies and enemy types, you can have more
 * than one leader on the map that is of the same leader type;
 * this means you can have 3 Olimars, if you want.
 * Why would you do that, though?
 */
class leader_type : public mob_type {
public:
    float whistle_range;
    unsigned int punch_strength;
    //Time until the Pikmin is actually popped out of the ground.
    float pluck_delay;
    //When this leader is thrown, multiply the vertical throw strength by this.
    float throw_strength_mult;
    float max_throw_height;
    
    sample_struct sfx_whistle;
    sample_struct sfx_dismiss;
    sample_struct sfx_name_call;
    
    ALLEGRO_BITMAP* bmp_icon; //Standby icon.
    
    leader_type();
    void load_from_file(
        data_node* file, const bool load_resources,
        vector<pair<size_t, string> >* anim_conversions
    );
};

#endif //ifndef LEADER_TYPE_INCLUDED
