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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/resource.h"
#include "../mob_script/resource_fsm.h"


/**
 * @brief Constructs a new resource type object.
 */
ResourceType::ResourceType() :
    MobType(MOB_CATEGORY_RESOURCES) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    ResourceFsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
AnimConversionVector ResourceType::getAnimConversions() const {
    AnimConversionVector v;
    v.push_back(std::make_pair(RESOURCE_ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void ResourceType::loadCatProperties(DataNode* file) {
    ReaderSetter rRS(file);
    
    string carryingDestinationStr;
    string deliveryResultStr;
    string sprayToConcoctStr;
    DataNode* carryingDestinationNode = nullptr;
    DataNode* deliveryResultNode = nullptr;
    DataNode* sprayToConcoctNode = nullptr;
    
    rRS.set(
        "carrying_destination", carryingDestinationStr,
        &carryingDestinationNode
    );
    rRS.set("damage_mob_amount", damageMobAmount);
    rRS.set("delivery_result", deliveryResultStr, &deliveryResultNode);
    rRS.set("point_amount", pointAmount);
    rRS.set("return_to_pile_on_vanish", returnToPileOnVanish);
    rRS.set("spray_to_concoct", sprayToConcoctStr, &sprayToConcoctNode);
    rRS.set("vanish_delay", vanishDelay);
    rRS.set("vanish_on_drop", vanishOnDrop);
    
    if(carryingDestinationNode) {
        readEnumProp(
            carryDestinationINames, carryingDestinationStr,
            &carryingDestination,
            "carrying destination", carryingDestinationNode
        );
    }
    
    if(deliveryResultNode) {
        readEnumProp(
            resourceDeliveryResultINames, deliveryResultStr,
            &deliveryResult, "delivery result", deliveryResultNode
        );
    }
    
    if(deliveryResult == RESOURCE_DELIVERY_RESULT_INCREASE_INGREDIENTS) {
        for(size_t s = 0; s < game.config.misc.sprayOrder.size(); s++) {
            if(
                game.config.misc.sprayOrder[s]->manifest->internalName ==
                sprayToConcoctStr
            ) {
                sprayToConcoct = s;
                break;
            }
        }
        if(sprayToConcoct == INVALID) {
            game.errors.report(
                "Unknown spray type \"" + sprayToConcoctStr + "\"!",
                sprayToConcoctNode
            );
        }
    }
}
