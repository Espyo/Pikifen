/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship type class and ship type-related functions.
 */

#include "ship_type.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/ship.h"
#include "../mob_script/ship_fsm.h"


/**
 * @brief Constructs a new ship type object.
 */
ShipType::ShipType() :
    MobType(MOB_CATEGORY_SHIPS) {
    
    nest = new PikminNestType();
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    AreaEditorProp aepPikInside;
    aepPikInside.name = "Pikmin inside";
    aepPikInside.var = "pikmin_inside";
    aepPikInside.type = AEMP_TYPE_TEXT;
    aepPikInside.defValue = "";
    aepPikInside.tooltip =
        "How many Pikmin are inside. "
        "One word per maturity. The first three words are for the first type, "
        "then three more for the second type, and so on. "
        "e.g.: \"8 0 1\" means it has 8 leaf Pikmin inside, and 1 flower.";
    areaEditorProps.push_back(aepPikInside);
    
    ShipFsm::createFsm(this);
}


/**
 * @brief Destroys the ship type object.
 */
ShipType::~ShipType() {
    delete nest;
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
AnimConversionVector ShipType::getAnimConversions() const {
    AnimConversionVector v;
    v.push_back(std::make_pair(SHIP_ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void ShipType::loadCatProperties(DataNode* file) {
    ReaderSetter sRS(file);
    
    sRS.set("can_heal", canHeal);
    sRS.set("control_point_radius", controlPointRadius);
    sRS.set("control_point_offset", controlPointOffset);
    sRS.set("receptacle_offset", receptacleOffset);
    
    nest->loadProperties(file);
    
    for(size_t s = 0; s < sounds.size(); s++) {
        if(sounds[s].name == "beam") {
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
void ShipType::loadCatResources(DataNode* file) {
    //We don't actually need to load any, but we know that if this function
    //is run, then the animations are definitely loaded.
    //Now's a good time to check the leg body parts.
    for(size_t b = 0; b < nest->legBodyParts.size(); b++) {
        if(animDb->findBodyPart(nest->legBodyParts[b]) == INVALID) {
            game.errors.report(
                "The ship type \"" + name + "\" specifies a leg body part "
                "called \"" + nest->legBodyParts[b] + "\", "
                "but no such body part exists!"
            );
        }
    }
}
