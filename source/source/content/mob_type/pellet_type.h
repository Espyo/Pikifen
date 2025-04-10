/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pellet type class and pellet type-related functions.
 */

#pragma once

#include "../../lib/data_file/data_file.h"
#include "mob_type.h"
#include "pikmin_type.h"


//Pellet object states.
enum PELLET_STATE {

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


/**
 * @brief A pellet type.
 *
 * Contains info on how many seeds the Onion should receive,
 * depending on whether it matches the Pikmin type or not.
 */
class PelletType : public MobType {

public:

    //--- Members ---
    
    //Type of Pikmin this pellet relates to.
    PikminType* pik_type = nullptr;
    
    //Number on the pellet, and hence, its weight.
    size_t number = 0;
    
    //Number of seeds given out if the pellet's taken to a matching Onion.
    size_t match_seeds = 0;
    
    //Number of seeds given out if the pellet's taken to a non-matching Onion.
    size_t non_match_seeds = 0;
    
    //Bitmap to use to represent the number on the pellet.
    ALLEGRO_BITMAP* bmp_number = nullptr;
    
    
    //--- Function declarations ---
    
    PelletType();
    void loadCatProperties(DataNode* file) override;
    void loadCatResources(DataNode* file) override;
    anim_conversion_vector getAnimConversions() const override;
    void unloadResources() override;
    
};
