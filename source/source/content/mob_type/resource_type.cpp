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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob_script/resource_fsm.h"
#include "../mob/resource.h"


/**
 * @brief Constructs a new resource type object.
 */
ResourceType::ResourceType() :
    MobType(MOB_CATEGORY_RESOURCES) {
    
    target_type = MOB_TARGET_FLAG_NONE;
    
    resource_fsm::create_fsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
anim_conversion_vector ResourceType::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(RESOURCE_ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void ResourceType::load_cat_properties(DataNode* file) {
    ReaderSetter rs(file);
    
    string carrying_destination_str;
    string delivery_result_str;
    string spray_to_concoct_str;
    DataNode* carrying_destination_node = nullptr;
    DataNode* delivery_result_node = nullptr;
    DataNode* spray_to_concoct_node = nullptr;
    
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
    } else if(carrying_destination_str == "linked_mob_matching_type") {
        carrying_destination = CARRY_DESTINATION_LINKED_MOB_MATCHING_TYPE;
    } else {
        game.errors.report(
            "Unknown carrying destination \"" +
            carrying_destination_str + "\"!", carrying_destination_node
        );
    }
    
    if(delivery_result_str == "damage_mob") {
        delivery_result = RESOURCE_DELIVERY_RESULT_DAMAGE_MOB;
    } else if(delivery_result_str == "increase_ingredients") {
        delivery_result = RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS;
    } else if(delivery_result_str == "add_points") {
        delivery_result = RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS;
    } else if(delivery_result_str == "stay") {
        delivery_result = RESOURCE_DELIVERY_RESULT_STAY;
    } else {
        game.errors.report(
            "Unknown delivery result \"" + delivery_result_str + "\"!",
            delivery_result_node
        );
    }
    
    if(delivery_result == RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS) {
        for(size_t s = 0; s < game.config.misc.spray_order.size(); s++) {
            if(
                game.config.misc.spray_order[s]->manifest->internal_name ==
                spray_to_concoct_str
            ) {
                spray_to_concoct = s;
                break;
            }
        }
        if(spray_to_concoct == INVALID) {
            game.errors.report(
                "Unknown spray type \"" + spray_to_concoct_str + "\"!",
                spray_to_concoct_node
            );
        }
    }
}
