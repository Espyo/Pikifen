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
    mob_type(MOB_CATEGORY_ONIONS) {
    
    nest = new pikmin_nest_type_struct();
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    area_editor_prop_struct aep_pik_inside;
    aep_pik_inside.name = "Pikmin inside";
    aep_pik_inside.var = "pikmin_inside";
    aep_pik_inside.type = AEMP_TEXT;
    aep_pik_inside.def_value = "";
    aep_pik_inside.tooltip =
        "How many Pikmin are inside. One word per maturity.\n"
        "The first three words are for the first type, "
        "then three more for the second type, and so on. "
        "e.g.: \"8 0 1\" means it has 8 leaf Pikmin inside, and 1 flower.";
    area_editor_props.push_back(aep_pik_inside);
    
    onion_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Destroys a type of Onion.
 */
onion_type::~onion_type() {
    delete nest;
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector onion_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(MOB_TYPE::ANIM_IDLING, "idling"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void onion_type::load_properties(data_node* file) {
    nest->load_properties(file);
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 * file:
 *   File to read from.
 */
void onion_type::load_resources(data_node* file) {
    //We don't actually need to load any, but we know that if this function
    //is run, then the animations are definitely loaded.
    //Now's a good time to check the leg body parts.
    for(size_t b = 0; b < nest->leg_body_parts.size(); ++b) {
        if(anims.find_body_part(nest->leg_body_parts[b]) == INVALID) {
            game.errors.report(
                "The Onion type \"" + name + "\" specifies a leg body part "
                "called \"" + nest->leg_body_parts[b] + "\", "
                "but no such body part exists!"
            );
        }
    }
}
