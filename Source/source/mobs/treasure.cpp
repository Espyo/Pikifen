/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure class and treasure-related functions.
 */

#include "treasure.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../utils/string_utils.h"
#include "ship.h"


/* ----------------------------------------------------------------------------
 * Creates a treasure.
 * pos:
 *   Starting coordinates.
 * type:
 *   Treasure type this mob belongs to.
 * angle:
 *   Starting angle.
 */
treasure::treasure(const point &pos, treasure_type* type, const float angle) :
    mob(pos, type, angle),
    tre_type(type) {
    
    become_carriable(CARRY_DESTINATION_SHIP);
    
    set_animation(ANIM_IDLING, true, START_ANIMATION_RANDOM_TIME_ON_SPAWN);
    
}
