/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet type class and pellet type-related functions.
 */

#include "pellet_type.h"

#include "../functions.h"
#include "../game.h"
#include "../mob_fsms/gen_mob_fsm.h"
#include "../mob_fsms/pellet_fsm.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a type of pellet.
 */
pellet_type::pellet_type() :
    mob_type(MOB_CATEGORY_PELLETS),
    pik_type(nullptr),
    number(0),
    match_seeds(0),
    non_match_seeds(0),
    bmp_number(nullptr) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    pellet_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector pellet_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(MOB_TYPE::ANIM_IDLING, "idling"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void pellet_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string pik_type_str;
    data_node* pik_type_node = NULL;
    
    rs.set("match_seeds", match_seeds);
    rs.set("non_match_seeds", non_match_seeds);
    rs.set("number", number);
    rs.set("pikmin_type", pik_type_str, &pik_type_node);
    
    if(
        game.mob_types.pikmin.find(pik_type_str) ==
        game.mob_types.pikmin.end()
    ) {
        log_error(
            "Unknown Pikmin type \"" + pik_type_str + "\"!",
            pik_type_node
        );
    } else {
        pik_type = game.mob_types.pikmin[pik_type_str];
    }
    
    weight = number;
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 * file:
 *   File to read from.
 */
void pellet_type::load_resources(data_node* file) {
    reader_setter rs(file);
    
    string number_image_str;
    data_node* number_image_node = NULL;
    
    rs.set("number_image", number_image_str, &number_image_node);
    
    bmp_number = game.bitmaps.get(number_image_str, number_image_node);
}


/* ----------------------------------------------------------------------------
 * Unloads resources from memory.
 */
void pellet_type::unload_resources() {
    game.bitmaps.detach(bmp_number);
}
