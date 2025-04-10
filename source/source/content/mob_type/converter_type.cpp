/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter type class and converter type-related functions.
 */

#include "converter_type.h"

#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob_script/converter_fsm.h"


/**
 * @brief Constructs a new converter type object.
 */
ConverterType::ConverterType() :
    MobType(MOB_CATEGORY_CONVERTERS) {
    
    target_type = MOB_TARGET_FLAG_NONE;
    
    converter_fsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 */
anim_conversion_vector ConverterType::getAnimConversions() const {
    anim_conversion_vector v;
    
    v.push_back(std::make_pair(CONVERTER_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(CONVERTER_ANIM_BUMPED, "bumped"));
    v.push_back(std::make_pair(CONVERTER_ANIM_CLOSING, "closing"));
    v.push_back(std::make_pair(CONVERTER_ANIM_SPITTING, "spitting"));
    v.push_back(std::make_pair(CONVERTER_ANIM_OPENING, "opening"));
    v.push_back(std::make_pair(CONVERTER_ANIM_DYING, "dying"));
    
    return getAnimConversionsWithGroups(v, N_CONVERTER_ANIMS);
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void ConverterType::loadCatProperties(DataNode* file) {
    ReaderSetter rs(file);
    
    string pikmin_types_str;
    string type_animation_suffixes_str;
    DataNode* pikmin_types_node = nullptr;
    DataNode* type_animation_suffixes_node = nullptr;
    
    rs.set("auto_conversion_timeout", auto_conversion_timeout);
    rs.set("available_pikmin_types", pikmin_types_str, &pikmin_types_node);
    rs.set("buffer_size", buffer_size);
    rs.set("pikmin_per_conversion", pikmin_per_conversion);
    rs.set("same_type_counts_for_output", same_type_counts_for_output);
    rs.set("total_input_pikmin", total_input_pikmin);
    rs.set(
        "type_animation_suffixes", type_animation_suffixes_str,
        &type_animation_suffixes_node
    );
    rs.set("type_change_interval", type_change_interval);
    
    MobCategory* pik_cat = game.mob_categories.get(MOB_CATEGORY_PIKMIN);
    vector<string> pikmin_types_strs =
        semicolonListToVector(pikmin_types_str);
        
    for(size_t t = 0; t < pikmin_types_strs.size(); t++) {
        MobType* type_ptr = pik_cat->getType(pikmin_types_strs[t]);
        
        if(type_ptr) {
            available_pikmin_types.push_back((PikminType*) type_ptr);
        } else {
            game.errors.report(
                "Unknown Pikmin type \"" + pikmin_types_strs[t] + "\"!",
                pikmin_types_node
            );
        }
    }
    
    animation_group_suffixes =
        semicolonListToVector(type_animation_suffixes_str);
        
    if(available_pikmin_types.size() == 1 && animation_group_suffixes.empty()) {
        //Let's make life easier. This is a one-type converter,
        //so no need to involve suffixes.
        animation_group_suffixes.push_back("");
    }
    
    if(available_pikmin_types.empty()) {
        game.errors.report(
            "A converter needs to have at least one available Pikmin type! "
            "Please fill in the \"available_pikmin_types\" property.",
            file
        );
    }
    
    if(animation_group_suffixes.size() != available_pikmin_types.size()) {
        game.errors.report(
            "The number of animation type suffixes needs to match the "
            "number of available Pikmin types! Did you forget an animation "
            "suffix or a Pikmin type?",
            type_animation_suffixes_node
        );
    }
}
