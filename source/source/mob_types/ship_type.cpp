/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship type class and ship type-related functions.
 */

#include "ship_type.h"

#include "../functions.h"
#include "../game.h"
#include "../mob_fsms/ship_fsm.h"
#include "../mobs/ship.h"
#include "../utils/string_utils.h"


/**
 * @brief Constructs a new ship type object.
 */
ship_type::ship_type() :
    mob_type(MOB_CATEGORY_SHIPS) {
    
    nest = new pikmin_nest_type_t();
    
    target_type = MOB_TARGET_FLAG_NONE;
    
    area_editor_prop_t aep_pik_inside;
    aep_pik_inside.name = "Pikmin inside";
    aep_pik_inside.var = "pikmin_inside";
    aep_pik_inside.type = AEMP_TYPE_TEXT;
    aep_pik_inside.def_value = "";
    aep_pik_inside.tooltip =
        "How many Pikmin are inside. "
        "One word per maturity. The first three words are for the first type, "
        "then three more for the second type, and so on. "
        "e.g.: \"8 0 1\" means it has 8 leaf Pikmin inside, and 1 flower.";
    area_editor_props.push_back(aep_pik_inside);
    
    ship_fsm::create_fsm(this);
}


/**
 * @brief Destroys the ship type object.
 */
ship_type::~ship_type() {
    delete nest;
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
anim_conversion_vector ship_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(SHIP_ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void ship_type::load_cat_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("can_heal", can_heal);
    rs.set("control_point_radius", control_point_radius);
    rs.set("control_point_offset", control_point_offset);
    rs.set("receptacle_offset", receptacle_offset);
    
    nest->load_properties(file);
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void ship_type::load_cat_resources(data_node* file) {
    //We don't actually need to load any, but we know that if this function
    //is run, then the animations are definitely loaded.
    //Now's a good time to check the leg body parts.
    for(size_t b = 0; b < nest->leg_body_parts.size(); b++) {
        if(anim_db->find_body_part(nest->leg_body_parts[b]) == INVALID) {
            game.errors.report(
                "The ship type \"" + name + "\" specifies a leg body part "
                "called \"" + nest->leg_body_parts[b] + "\", "
                "but no such body part exists!"
            );
        }
    }
}
