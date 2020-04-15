/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion type class and Onion type-related functions.
 */

#include "onion_type.h"

#include "../functions.h"
#include "../game.h"
#include "../mob_fsms/onion_fsm.h"
#include "../mobs/onion.h"

/* ----------------------------------------------------------------------------
 * Creates a type of Onion.
 */
onion_type::onion_type() :
    mob_type(MOB_CATEGORY_ONIONS),
    pik_type(NULL) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    onion_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector onion_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(std::make_pair(ANIM_IDLING, "idling"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 */
void onion_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string pik_type_str;
    data_node* pik_type_node;
    
    rs.set("pikmin_type", pik_type_str, &pik_type_node);
    
    if(game.mob_types.pikmin.find(pik_type_str) == game.mob_types.pikmin.end()) {
        log_error(
            "Unknown Pikmin type \"" + pik_type_str + "\"!",
            pik_type_node
        );
    }
    pik_type = game.mob_types.pikmin[pik_type_str];
}
