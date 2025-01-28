/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the world component class.
 */

#pragma once

#include <cstdio>


struct sector;
class mob;
struct particle;


/**
 * @brief Something that makes up the interactable game world and can be drawn.
 * This contains information about how it should be drawn.
 */
class world_component {

public:

    //--- Members ---
    
    //If it's a sector, this points to it.
    sector* sector_ptr = nullptr;
    
    //If it's a mob shadow, this points to its mob.
    mob* mob_shadow_ptr = nullptr;
    
    //If it's a mob limb, this points to its mob.
    mob* mob_limb_ptr = nullptr;
    
    //If it's a mob, this points to it.
    mob* mob_ptr = nullptr;
    
    //If it's a particle, this points to it.
    particle* particle_ptr = nullptr;
    
    //Its Z coordinate.
    float z = 0.0f;
    
    //Index in the list of world components. Used for sorting.
    size_t idx = 0;
    
};
