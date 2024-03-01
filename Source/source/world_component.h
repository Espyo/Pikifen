/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the world component class.
 */

#ifndef WORLD_COMPONENT_INCLUDED
#define WORLD_COMPONENT_INCLUDED

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
    sector* sector_ptr;
    
    //If it's a mob shadow, this points to its mob.
    mob* mob_shadow_ptr;
    
    //If it's a mob limb, this points to its mob.
    mob* mob_limb_ptr;
    
    //If it's a mob, this points to it.
    mob* mob_ptr;
    
    //If it's a particle, this points to it.
    particle* particle_ptr;
    
    //Its Z coordinate.
    float z;
    
    //Index number in the list of world components. Used for sorting.
    size_t nr;
    

    //--- Function declarations ---
    
    world_component();
    
};


#endif //ifndef WORLD_COMPONENT_INCLUDED
