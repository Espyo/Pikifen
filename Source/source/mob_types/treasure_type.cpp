/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure type class and treasure type-related functions.
 */

#include "treasure_type.h"

#include "../functions.h"
#include "../mob_fsms/gen_mob_fsm.h"
#include "../mob_fsms/treasure_fsm.h"
#include "../mobs/mob.h"
#include "../mobs/treasure.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a type of treasure.
 */
treasure_type::treasure_type() :
    mob_type(MOB_CATEGORY_TREASURES) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    treasure_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector treasure_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(ANIM_IDLING, "idling"));
    return v;
}
