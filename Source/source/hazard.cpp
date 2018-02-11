/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Hazard class and hazard-related functions.
 */

#include "hazard.h"

/* ----------------------------------------------------------------------------
 * Creates a hazard.
 */
hazard::hazard() :
    main_color(al_map_rgba(0, 0, 0, 0)),
    associated_liquid(nullptr) {
    
}
