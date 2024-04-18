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


/**
 * @brief Constructs a new treasure type object.
 */
treasure_type::treasure_type() :
    mob_type(MOB_CATEGORY_TREASURES) {
    
    target_type = MOB_TARGET_FLAG_NONE;
    
    treasure_fsm::create_fsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
anim_conversion_vector treasure_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(MOB_TYPE::ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void treasure_type::load_cat_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("points", points);
}
