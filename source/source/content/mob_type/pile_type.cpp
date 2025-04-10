/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile type class and pile type-related functions.
 */

#include "pile_type.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob_script/pile_fsm.h"
#include "../mob/pile.h"


/**
 * @brief Constructs a new pile type object.
 */
PileType::PileType() :
    MobType(MOB_CATEGORY_PILES) {
    
    target_type = MOB_TARGET_FLAG_PIKMIN_OBSTACLE;
    
    AreaEditorProp aep_amount;
    aep_amount.name = "Amount";
    aep_amount.var = "amount";
    aep_amount.type = AEMP_TYPE_TEXT;
    aep_amount.def_value = "";
    aep_amount.tooltip =
        "How many resources this pile starts with, or leave empty for the max.";
    area_editor_props.push_back(aep_amount);
    
    pile_fsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
anim_conversion_vector PileType::getAnimConversions() const {
    anim_conversion_vector v;
    
    v.push_back(std::make_pair(PILE_ANIM_IDLING, "idling"));
    
    return getAnimConversionsWithGroups(v, N_PILE_ANIMS);
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void PileType::loadCatProperties(DataNode* file) {
    ReaderSetter rs(file);
    
    string contents_str;
    string size_animation_suffixes_str;
    DataNode* contents_node = nullptr;
    
    rs.set("auto_shrink_smallest_radius", auto_shrink_smallest_radius);
    rs.set("can_drop_multiple", can_drop_multiple);
    rs.set("contents", contents_str, &contents_node);
    rs.set("delete_when_finished", delete_when_finished);
    rs.set("health_per_resource", health_per_resource);
    rs.set("hide_when_empty", hide_when_empty);
    rs.set("max_amount", max_amount);
    rs.set("recharge_amount", recharge_amount);
    rs.set("recharge_interval", recharge_interval);
    rs.set("show_amount", show_amount);
    rs.set("size_animation_suffixes", size_animation_suffixes_str);
    
    auto res_type = game.content.mob_types.list.resource.find(contents_str);
    if(res_type != game.content.mob_types.list.resource.end()) {
        contents = res_type->second;
    } else {
        game.errors.report(
            "Unknown resource type \"" + contents_str + "\"!", contents_node
        );
    }
    
    animation_group_suffixes =
        semicolonListToVector(size_animation_suffixes_str);
        
    if(animation_group_suffixes.empty()) {
        //Let's make life easier. If no suffixes were given, then create an
        //implied one.
        animation_group_suffixes.push_back("");
    }
    
    max_health = health_per_resource * max_amount;
}
