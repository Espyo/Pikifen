/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Decoration type class and decoration type-related functions.
 */

#include "decoration_type.h"

#include "../mob_script/decoration_fsm.h"
#include "../mob/decoration.h"
#include "../other/mob_script.h"


/**
 * @brief Constructs a new decoration type object.
 */
DecorationType::DecorationType() :
    MobType(MOB_CATEGORY_DECORATIONS) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    AreaEditorProp aepRandomAnimDelay;
    aepRandomAnimDelay.name = "Random animation delay";
    aepRandomAnimDelay.var = "random_animation_delay";
    aepRandomAnimDelay.type = AEMP_TYPE_BOOL;
    aepRandomAnimDelay.defValue = "true";
    aepRandomAnimDelay.tooltip =
        "If this decoration type can have a random animation delay,\n"
        "this property makes this decoration use it or not.";
    areaEditorProps.push_back(aepRandomAnimDelay);
    
    AreaEditorProp aepRandomTint;
    aepRandomTint.name = "Random tint";
    aepRandomTint.var = "random_tint";
    aepRandomTint.type = AEMP_TYPE_BOOL;
    aepRandomTint.defValue = "true";
    aepRandomTint.tooltip =
        "If this decoration type can have a random color tint,\n"
        "this property makes this decoration use it or not.";
    areaEditorProps.push_back(aepRandomTint);
    
    AreaEditorProp aepRandomScale;
    aepRandomScale.name = "Random scale";
    aepRandomScale.var = "random_scale";
    aepRandomScale.type = AEMP_TYPE_BOOL;
    aepRandomScale.defValue = "true";
    aepRandomScale.tooltip =
        "If this decoration type can have a random scale,\n"
        "this property makes this decoration use it or not.";
    areaEditorProps.push_back(aepRandomScale);
    
    AreaEditorProp aepRandomRotation;
    aepRandomRotation.name = "Random rotation";
    aepRandomRotation.var = "random_rotation";
    aepRandomRotation.type = AEMP_TYPE_BOOL;
    aepRandomRotation.defValue = "true";
    aepRandomRotation.tooltip =
        "If this decoration type can have a random scale,\n"
        "this property makes this decoration use it or not.";
    areaEditorProps.push_back(aepRandomRotation);
    
    blackoutRadius = 0.0f;
    
    DecorationFsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 */
AnimConversionVector DecorationType::getAnimConversions() const {
    AnimConversionVector v;
    v.push_back(std::make_pair(DECORATION_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(DECORATION_ANIM_BUMPED, "bumped"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void DecorationType::loadCatProperties(DataNode* file) {
    ReaderSetter dRS(file);
    
    dRS.set("random_animation_delay", randomAnimationDelay);
    dRS.set("rotation_random_variation", rotationRandomVariation);
    dRS.set("scale_random_variation", scaleRandomVariation);
    dRS.set("tint_random_maximum", tintRandomMaximum);
    
    rotationRandomVariation = degToRad(rotationRandomVariation);
}
