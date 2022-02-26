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


/* ----------------------------------------------------------------------------
 * Something that makes up the interactable game world and can be drawn.
 * This contains information about how it should be drawn.
 */
class world_component {
public:
    sector* sector_ptr;
    mob* mob_shadow_ptr;
    mob* mob_limb_ptr;
    mob* mob_ptr;
    particle* particle_ptr;
    
    float z;
    size_t nr;
    world_component();
};


#endif //ifndef WORLD_COMPONENT_INCLUDED
