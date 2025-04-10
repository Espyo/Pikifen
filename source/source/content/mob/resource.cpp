/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource class and resource-related functions.
 */

#include "resource.h"

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"


/**
 * @brief Constructs a new resource object.
 *
 * @param pos Starting coordinates.
 * @param type Resource type this mob belongs to.
 * @param angle Starting angle.
 */
Resource::Resource(const Point &pos, ResourceType* type, float angle) :
    Mob(pos, type, angle),
    res_type(type),
    origin_pile(nullptr) {
    
    becomeCarriable(res_type->carrying_destination);
}
