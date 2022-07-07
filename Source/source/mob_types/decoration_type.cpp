/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Decoration type class and decoration type-related functions.
 */

#include "decoration_type.h"

#include "../mob_fsms/decoration_fsm.h"
#include "../mob_script.h"
#include "../mobs/decoration.h"


/* ----------------------------------------------------------------------------
 * Creates a type of decoration.
 */
decoration_type::decoration_type() :
    mob_type(MOB_CATEGORY_DECORATIONS),
    tint_random_maximum(COLOR_EMPTY),
    scale_random_variation(0.0f),
    rotation_random_variation(0.0f),
    random_animation_delay(false) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    decoration_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector decoration_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(DECORATION_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(DECORATION_ANIM_BUMPED, "bumped"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void decoration_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("random_animation_delay", random_animation_delay);
    rs.set("rotation_random_variation", rotation_random_variation);
    rs.set("scale_random_variation", scale_random_variation);
    rs.set("tint_random_maximum", tint_random_maximum);
    
    rotation_random_variation = deg_to_rad(rotation_random_variation);
}
