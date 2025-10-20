/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop type class and drop type-related functions.
 */

#include "drop_type.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../mob_script/drop_fsm.h"


/**
 * @brief Constructs a new drop type object.
 */
DropType::DropType() :
    MobType(MOB_CATEGORY_DROPS) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    height = 8.0f;
    
    DropFsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 */
AnimConversionVector DropType::getAnimConversions() const {
    AnimConversionVector v;
    v.push_back(std::make_pair(DROP_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(DROP_ANIM_FALLING, "falling"));
    v.push_back(std::make_pair(DROP_ANIM_LANDING, "landing"));
    v.push_back(std::make_pair(DROP_ANIM_BUMPED, "bumped"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void DropType::loadCatProperties(DataNode* file) {
    ReaderSetter dRS(file);
    
    string consumerStr;
    string effectStr;
    string sprayNameStr;
    string statusNameStr;
    DataNode* consumerNode = nullptr;
    DataNode* effectNode = nullptr;
    DataNode* sprayNameNode = nullptr;
    DataNode* statusNameNode = nullptr;
    DataNode* totalDosesNode = nullptr;
    
    dRS.set("consumer", consumerStr, &consumerNode);
    dRS.set("effect", effectStr, &effectNode);
    dRS.set("increase_amount", increaseAmount);
    dRS.set("shrink_speed", shrinkSpeed);
    dRS.set("spray_type_to_increase", sprayNameStr, &sprayNameNode);
    dRS.set("status_to_give", statusNameStr, &statusNameNode);
    dRS.set("total_doses", totalDoses, &totalDosesNode);
    
    if(consumerNode) {
        readEnumProp(
            consumerStr,
        (int*) &consumer, {
            "pikmin",
            "leaders"
        },
        "consumer",
        consumerNode
        );
    }
    
    if(effectNode) {
        readEnumProp(
            effectStr,
        (int*) &effect, {
            "maturate",
            "increase_sprays",
            "give_status",
        },
        "drop effect",
        effectNode
        );
    }
    
    if(effect == DROP_EFFECT_INCREASE_SPRAYS) {
        for(size_t s = 0; s < game.config.misc.sprayOrder.size(); s++) {
            if(
                game.config.misc.sprayOrder[s]->manifest->internalName ==
                sprayNameStr
            ) {
                sprayTypeToIncrease = s;
                break;
            }
        }
        if(sprayTypeToIncrease == INVALID) {
            game.errors.report(
                "Unknown spray type \"" + sprayNameStr + "\"!",
                sprayNameNode
            );
        }
    }
    
    if(statusNameNode) {
        auto s = game.content.statusTypes.list.find(statusNameStr);
        if(s != game.content.statusTypes.list.end()) {
            statusToGive = s->second;
        } else {
            game.errors.report(
                "Unknown status type \"" + statusNameStr + "\"!",
                statusNameNode
            );
        }
    }
    
    if(totalDoses == 0) {
        game.errors.report(
            "The number of total doses cannot be zero!", totalDosesNode
        );
    }
    
    shrinkSpeed /= 100.0f;
}
