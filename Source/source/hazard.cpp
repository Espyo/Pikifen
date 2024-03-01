/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Hazard class and hazard-related functions.
 */

#include "hazard.h"

#include "const.h"


/**
 * @brief Constructs a new hazard object.
 */
hazard::hazard() :
    main_color(COLOR_EMPTY),
    associated_liquid(nullptr) {
    
}
