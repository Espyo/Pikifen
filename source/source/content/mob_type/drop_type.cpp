/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop type class and drop type-related functions.
 */

#include "drop_type.hpp"

#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../mob_script/drop_fsm.hpp"


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
            dropConsumerINames, consumerStr, &consumer,
            "consumer", consumerNode
        );
    }
    
    if(effectNode) {
        readEnumProp(
            dropEffectINames, effectStr, &effect,
            "drop effect", effectNode
        );
    }
    
    if(effect == DROP_EFFECT_INCREASE_SPRAYS) {
        forIdx(s, game.config.misc.sprayOrder) {
            if(
                game.config.misc.sprayOrder[s]->manifest->internalName ==
                sprayNameStr
            ) {
                sprayTypeToIncrease = s;
                break;
            }
        }
        if(sprayTypeToIncrease == INVALID) {
            game.log.write(
                LOG_LEVEL_ERROR,
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
            game.log.write(
                LOG_LEVEL_ERROR,
                "Unknown status type \"" + statusNameStr + "\"!",
                statusNameNode
            );
        }
    }
    
    if(totalDoses == 0) {
        game.log.write(
            LOG_LEVEL_ERROR,
            "The number of total doses cannot be zero!", totalDosesNode
        );
    }
    
    shrinkSpeed /= 100.0f;
}
