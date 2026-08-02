/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile type class and pile type-related functions.
 */

#include "pile_type.hpp"

#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../../util/string_utils.hpp"
#include "../mob/pile.hpp"
#include "../mob_script/pile_fsm.hpp"


/**
 * @brief Constructs a new pile type object.
 */
PileType::PileType() :
    MobType(MOB_CATEGORY_PILES) {
    
    targetType = MOB_TARGET_FLAG_PIKMIN_OBSTACLE;
    
    AreaEditorProp aepAmount;
    aepAmount.name = "Amount";
    aepAmount.var = "amount";
    aepAmount.type = AEMP_TYPE_TEXT;
    aepAmount.defValue = "";
    aepAmount.tooltip =
        "How many resources this pile starts with, or leave empty for the max.";
    areaEditorProps.push_back(aepAmount);
    
    PileFsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
AnimConversionVector PileType::getAnimConversions() const {
    AnimConversionVector v;
    
    v.push_back(std::make_pair(PILE_ANIM_IDLING, "idling"));
    
    return getAnimConversionsWithGroups(v, N_PILE_ANIMS);
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void PileType::loadCatProperties(DataNode* file) {
    ReaderSetter pRS(file);
    
    string contentsStr;
    string sizeAnimSuffixesStr;
    bool contentsFromSpawnData = false;
    DataNode* contentsNode = nullptr;
    
    pRS.set("auto_shrink_smallest_radius", autoShrinkSmallestRadius);
    pRS.set("can_drop_multiple", canDropMultiple);
    pRS.set("contents", contentsStr, &contentsNode);
    pRS.set("contents_from_spawn_data", contentsFromSpawnData);
    pRS.set("delete_when_finished", deleteWhenFinished);
    pRS.set("health_per_resource", healthPerResource);
    pRS.set("hide_when_empty", hideWhenEmpty);
    pRS.set("max_amount", maxAmount);
    pRS.set("recharge_amount", rechargeAmount);
    pRS.set("recharge_interval", rechargeInterval);
    pRS.set("show_amount", showAmount);
    pRS.set("size_animation_suffixes", sizeAnimSuffixesStr);
    
    if(contentsFromSpawnData) {
        forIdx(s, spawns) {
            if(spawns[s].name == "contents") {
                contentsSpawnIdx = s;
                break;
            }
        }
        
        if(contentsSpawnIdx == INVALID) {
            game.errors.report(
                "The contents to spawn should come from a \"contents\" spawn "
                "block, but no such block exists!", file
            );
        }
        
    } else {
        auto resType = game.content.mobTypes.list.resource.find(contentsStr);
        if(resType != game.content.mobTypes.list.resource.end()) {
            contentsResource = resType->second;
        } else {
            game.errors.report(
                "Unknown resource type \"" + contentsStr + "\"!", contentsNode
            );
        }
        
    }
    
    animationGroupSuffixes =
        semicolonListToVector(sizeAnimSuffixesStr);
        
    if(animationGroupSuffixes.empty()) {
        //Let's make life easier. If no suffixes were given, then create an
        //implied one.
        animationGroupSuffixes.push_back("");
    }
    
    maxHealth = healthPerResource * maxAmount;
}
