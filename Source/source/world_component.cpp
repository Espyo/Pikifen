/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * World component class.
 */

#include "world_component.h"

/* ----------------------------------------------------------------------------
 * Initializes a world component struct.
 */
world_component::world_component() :
    mob_ptr(nullptr),
    mob_shadow_ptr(nullptr),
    mob_limb_ptr(nullptr),
    sector_ptr(nullptr) {
}
