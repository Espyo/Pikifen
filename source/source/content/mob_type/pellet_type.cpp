/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet type class and pellet type-related functions.
 */

#include "pellet_type.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../mob_script/gen_mob_fsm.h"
#include "../mob_script/pellet_fsm.h"
#include "../../util/string_utils.h"


/**
 * @brief Constructs a new pellet type object.
 */
PelletType::PelletType() :
    MobType(MOB_CATEGORY_PELLETS) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    pellet_fsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
anim_conversion_vector PelletType::getAnimConversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(MOB_TYPE::ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void PelletType::loadCatProperties(DataNode* file) {
    ReaderSetter rs(file);
    
    string pik_type_str;
    DataNode* pik_type_node = nullptr;
    
    rs.set("match_seeds", matchSeeds);
    rs.set("non_match_seeds", nonMatchSeeds);
    rs.set("number", number);
    rs.set("pikmin_type", pik_type_str, &pik_type_node);
    
    if(
        game.content.mobTypes.list.pikmin.find(pik_type_str) ==
        game.content.mobTypes.list.pikmin.end()
    ) {
        game.errors.report(
            "Unknown Pikmin type \"" + pik_type_str + "\"!",
            pik_type_node
        );
    } else {
        pikType = game.content.mobTypes.list.pikmin[pik_type_str];
    }
    
    weight = number;
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void PelletType::loadCatResources(DataNode* file) {
    ReaderSetter rs(file);
    
    string number_image_str;
    DataNode* number_image_node = nullptr;
    
    rs.set("number_image", number_image_str, &number_image_node);
    
    bmpNumber = game.content.bitmaps.list.get(number_image_str, number_image_node);
}


/**
 * @brief Unloads resources from memory.
 */
void PelletType::unloadResources() {
    game.content.bitmaps.list.free(bmpNumber);
}
