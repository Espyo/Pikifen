/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource type class and resource type-related functions.
 */

#include "resource_type.h"

#include "../functions.h"
#include "../game.h"
#include "../mob_fsms/resource_fsm.h"
#include "../mobs/resource.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a type of resource.
 */
resource_type::resource_type() :
    mob_type(MOB_CATEGORY_RESOURCES),
    vanish_on_drop(false),
    return_to_pile_on_vanish(false),
    vanish_delay(0.0f),
    carrying_destination(CARRY_DESTINATION_SHIP),
    delivery_result(RESOURCE_DELIVERY_RESULT_ADD_POINTS),
    damage_mob_amount(1.0f),
    spray_to_concoct(INVALID),
    point_amount(1.0f) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    resource_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector resource_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(RESOURCE_ANIM_IDLING, "idling"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void resource_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string carrying_destination_str;
    string delivery_result_str;
    string spray_to_concoct_str;
    data_node* carrying_destination_node = NULL;
    data_node* delivery_result_node = NULL;
    data_node* spray_to_concoct_node = NULL;
    
    rs.set(
        "carrying_destination", carrying_destination_str,
        &carrying_destination_node
    );
    rs.set("damage_mob_amount", damage_mob_amount);
    rs.set("delivery_result", delivery_result_str, &delivery_result_node);
    rs.set("point_amount", point_amount);
    rs.set("return_to_pile_on_vanish", return_to_pile_on_vanish);
    rs.set("spray_to_concoct", spray_to_concoct_str, &spray_to_concoct_node);
    rs.set("vanish_delay", vanish_delay);
    rs.set("vanish_on_drop", vanish_on_drop);
    
    if(carrying_destination_str == "ship") {
        carrying_destination = CARRY_DESTINATION_SHIP;
    } else if(carrying_destination_str == "linked_mob") {
        carrying_destination = CARRY_DESTINATION_LINKED_MOB;
    } else {
        log_error(
            "Unknown carrying destination \"" +
            carrying_destination_str + "\"!", carrying_destination_node
        );
    }
    
    if(delivery_result_str == "damage_mob") {
        delivery_result = RESOURCE_DELIVERY_RESULT_DAMAGE_MOB;
    } else if(delivery_result_str == "increase_ingredients") {
        delivery_result = RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS;
    } else if(delivery_result_str == "add_points") {
        delivery_result = RESOURCE_DELIVERY_RESULT_ADD_POINTS;
    } else {
        log_error(
            "Unknown delivery result \"" + delivery_result_str + "\"!",
            delivery_result_node
        );
    }
    
    if(delivery_result == RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS) {
        for(size_t s = 0; s < game.spray_types.size(); ++s) {
            if(game.spray_types[s].name == spray_to_concoct_str) {
                spray_to_concoct = s;
                break;
            }
        }
        if(spray_to_concoct == INVALID) {
            log_error(
                "Unknown spray type \"" + spray_to_concoct_str + "\"!",
                spray_to_concoct_node
            );
        }
    }
}
