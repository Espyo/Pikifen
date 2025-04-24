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
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    ConverterFsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 */
AnimConversionVector ConverterType::getAnimConversions() const {
    AnimConversionVector v;
    
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
    ReaderSetter cRS(file);
    
    string pikminTypesStr;
    string typeAnimSuffixesStr;
    DataNode* pikminTypesNode = nullptr;
    DataNode* typeAnimSuffixesNode = nullptr;
    
    cRS.set("auto_conversion_timeout", autoConversionTimeout);
    cRS.set("available_pikmin_types", pikminTypesStr, &pikminTypesNode);
    cRS.set("buffer_size", bufferSize);
    cRS.set("pikmin_per_conversion", pikminPerConversion);
    cRS.set("same_type_counts_for_output", sameTypeCountsForOutput);
    cRS.set("total_input_pikmin", totalInputPikmin);
    cRS.set(
        "type_animation_suffixes", typeAnimSuffixesStr, &typeAnimSuffixesNode
    );
    cRS.set("type_change_interval", typeChangeInterval);
    
    MobCategory* pikCat = game.mobCategories.get(MOB_CATEGORY_PIKMIN);
    vector<string> pikminTypesStrs =
        semicolonListToVector(pikminTypesStr);
        
    for(size_t t = 0; t < pikminTypesStrs.size(); t++) {
        MobType* typePtr = pikCat->getType(pikminTypesStrs[t]);
        
        if(typePtr) {
            availablePikminTypes.push_back((PikminType*) typePtr);
        } else {
            game.errors.report(
                "Unknown Pikmin type \"" + pikminTypesStrs[t] + "\"!",
                pikminTypesNode
            );
        }
    }
    
    animationGroupSuffixes =
        semicolonListToVector(typeAnimSuffixesStr);
        
    if(availablePikminTypes.size() == 1 && animationGroupSuffixes.empty()) {
        //Let's make life easier. This is a one-type converter,
        //so no need to involve suffixes.
        animationGroupSuffixes.push_back("");
    }
    
    if(availablePikminTypes.empty()) {
        game.errors.report(
            "A converter needs to have at least one available Pikmin type! "
            "Please fill in the \"available_pikmin_types\" property.",
            file
        );
    }
    
    if(animationGroupSuffixes.size() != availablePikminTypes.size()) {
        game.errors.report(
            "The number of animation type suffixes needs to match the "
            "number of available Pikmin types! Did you forget an animation "
            "suffix or a Pikmin type?",
            typeAnimSuffixesNode
        );
    }
}
