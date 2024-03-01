/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * World component class.
 */

#include "world_component.h"


/**
 * @brief Constructs a new world component object.
 */
world_component::world_component() :
    sector_ptr(nullptr),
    mob_shadow_ptr(nullptr),
    mob_limb_ptr(nullptr),
    mob_ptr(nullptr),
    particle_ptr(nullptr),
    z(0.0f),
    nr(0) {
}
