/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion type class and Onion type-related functions.
 */

#include "onion_type.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../mob/onion.h"
#include "../mob_script/onion_fsm.h"


/**
 * @brief Constructs a new Onion type object.
 */
OnionType::OnionType() :
    MobType(MOB_CATEGORY_ONIONS) {
    
    nest = new PikminNestType();
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    AreaEditorProp aepPikInside;
    aepPikInside.name = "Pikmin inside";
    aepPikInside.var = "pikmin_inside";
    aepPikInside.type = AEMP_TYPE_TEXT;
    aepPikInside.defValue = "";
    aepPikInside.tooltip =
        "How many Pikmin are inside. One word per maturity.\n"
        "The first three words are for the first type, "
        "then three more for the second type, and so on. "
        "e.g.: \"8 0 1\" means it has 8 leaf Pikmin inside, and 1 flower.";
    areaEditorProps.push_back(aepPikInside);
    
    OnionFsm::createFsm(this);
}


/**
 * @brief Destroys the Onion type object.
 */
OnionType::~OnionType() {
    delete nest;
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
AnimConversionVector OnionType::getAnimConversions() const {
    AnimConversionVector v;
    v.push_back(std::make_pair(ONION_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(ONION_ANIM_GENERATING, "generating"));
    v.push_back(
        std::make_pair(ONION_ANIM_STOPPING_GENERATION, "stopping_generation")
    );
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void OnionType::loadCatProperties(DataNode* file) {
    nest->loadProperties(file, this);
    nest->createColormap();
    
    for(size_t s = 0; s < sounds.size(); s++) {
        if(sounds[s].name == "pop") {
            soundPopIdx = s;
        } else if(sounds[s].name == "beam") {
            soundBeamIdx = s;
        } else if(sounds[s].name == "reception") {
            soundReceptionIdx = s;
        }
    }
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void OnionType::loadCatResources(DataNode* file) {
    //We don't actually need to load any, but we know that if this function
    //is run, then the animations are definitely loaded.
    //Now's a good time to check the leg body parts.
    for(size_t b = 0; b < nest->legBodyParts.size(); b++) {
        if(animDb->findBodyPart(nest->legBodyParts[b]) == INVALID) {
            game.errors.report(
                "The Onion type \"" + name + "\" specifies a leg body part "
                "called \"" + nest->legBodyParts[b] + "\", "
                "but no such body part exists!"
            );
        }
    }
}
