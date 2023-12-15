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

#include "../libs/data_file.h"
#include "mob_type.h"
#include "pikmin_type.h"


//Pellet object states.
enum PELLET_STATES {
    //Idle, waiting to move.
    PELLET_STATE_IDLE_WAITING,
    //Idle, moving.
    PELLET_STATE_IDLE_MOVING,
    //Idle, stuck.
    PELLET_STATE_IDLE_STUCK,
    //Idle, being thrown.
    PELLET_STATE_IDLE_THROWN,
    //Being delivered.
    PELLET_STATE_BEING_DELIVERED,
    
    //Total amount of pellet object states.
    N_PELLET_STATES,
};


/* ----------------------------------------------------------------------------
 * A pellet type. Contains info on
 * how many seeds the Onion should receive,
 * depending on whether it matches the Pikmin
 * type or not.
 */
class pellet_type : public mob_type {
public:
    //Type of Pikmin this pellet relates to.
    pikmin_type* pik_type;
    //Number on the pellet, and hence, its weight.
    size_t number;
    //Number of seeds given out if the pellet's taken to a matching Onion.
    size_t match_seeds;
    //Number of seeds given out if the pellet's taken to a non-matching Onion.
    size_t non_match_seeds;
    //Bitmap to use to represent the number on the pellet.
    ALLEGRO_BITMAP* bmp_number;
    
    pellet_type();
    void load_properties(data_node* file) override;
    void load_resources(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
    void unload_resources() override;
    
};


#endif //ifndef PELLET_TYPE_INCLUDED
