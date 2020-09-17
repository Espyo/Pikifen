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
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    area_editor_prop_struct aep_pik_inside;
    aep_pik_inside.name = "Pikmin inside";
    aep_pik_inside.var = "pikmin_inside";
    aep_pik_inside.type = AEMP_TEXT;
    aep_pik_inside.def_value = "";
    aep_pik_inside.tooltip =
        "How many Pikmin are inside.\n"
        "One word per maturity. The first three words are for the first type,\n"
        "then three more for the second type, and so on.\n"
        "e.g.: \"8 0 1\" means it has 8 leaf Pikmin inside, and 1 flower.";
    area_editor_props.push_back(aep_pik_inside);
    
    onion_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector onion_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(ANIM_IDLING, "idling"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void onion_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string pik_types_str;
    data_node* pik_types_node = NULL;
    
    rs.set("pikmin_types", pik_types_str, &pik_types_node);
    
    vector<string> pik_types_strs = semicolon_list_to_vector(pik_types_str);
    for(size_t t = 0; t < pik_types_strs.size(); ++t) {
        string &str = pik_types_strs[t];
        if(
            game.mob_types.pikmin.find(str) ==
            game.mob_types.pikmin.end()
        ) {
            log_error(
                "Unknown Pikmin type \"" + str + "\"!",
                pik_types_node
            );
        } else {
            pik_types.push_back(game.mob_types.pikmin[str]);
        }
    }
}
