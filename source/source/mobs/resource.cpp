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

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"


/**
 * @brief Constructs a new resource object.
 *
 * @param pos Starting coordinates.
 * @param type Resource type this mob belongs to.
 * @param angle Starting angle.
 */
resource::resource(const point &pos, resource_type* type, float angle) :
    mob(pos, type, angle),
    res_type(type),
    origin_pile(nullptr) {
    
    become_carriable(res_type->carrying_destination);
}
