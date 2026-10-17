/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet type class and pellet type-related functions.
 */

#include "pellet_type.hpp"

#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../../util/string_utils.hpp"
#include "../mob_script/gen_mob_fsm.hpp"
#include "../mob_script/pellet_fsm.hpp"


/**
 * @brief Constructs a new pellet type object.
 */
PelletType::PelletType() :
    MobType(MOB_CATEGORY_PELLETS) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    PelletFsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
AnimConversionVector PelletType::getAnimConversions() const {
    AnimConversionVector v;
    v.push_back(std::make_pair(MOB_TYPE::ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void PelletType::loadCatProperties(DataNode* file) {
    ReaderSetter pRS(file);
    
    string pikTypeStr;
    DataNode* pikTypeNode = nullptr;
    
    //DEPRECATED in 1.2.0 by "match_nutrients".
    deprecatedReaderSetter(
        pRS, "match_seeds", "use \"match_nutrients\"", matchNutrients
    );
    //DEPRECATED in 1.2.0 by "non_match_nutrients".
    deprecatedReaderSetter(
        pRS, "non_match_seeds", "use \"non_match_nutrients\"", nonMatchNutrients
    );
    
    pRS.set("draw_number", drawNumber);
    pRS.set("match_nutrients", matchNutrients);
    pRS.set("non_match_nutrients", nonMatchNutrients);
    pRS.set("number", number);
    pRS.set("nutrient_family", nutrientFamily);
    pRS.set("pikmin_type", pikTypeStr, &pikTypeNode);
    
    if(!isInMap(game.content.mobTypes.list.pikmin, pikTypeStr)) {
        game.errors.report(
            "Unknown Pikmin type \"" + pikTypeStr + "\"!",
            pikTypeNode
        );
    } else {
        pikType = game.content.mobTypes.list.pikmin[pikTypeStr];
    }
    
    weight = number;
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void PelletType::loadCatResources(DataNode* file) {
    ReaderSetter pRS(file);
    
    string numberImageStr;
    DataNode* numberImageNode = nullptr;
    
    pRS.set("number_image", numberImageStr, &numberImageNode);
    
    bmpNumber = game.content.bitmaps.list.get(numberImageStr, numberImageNode);
}


/**
 * @brief Unloads resources from memory.
 */
void PelletType::unloadResources() {
    game.content.bitmaps.list.free(bmpNumber);
}
