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
    
    target_type = MOB_TARGET_FLAG_NONE;
    
    pellet_fsm::create_fsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 *
 * @return The vector.
 */
anim_conversion_vector PelletType::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(MOB_TYPE::ANIM_IDLING, "idling"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void PelletType::load_cat_properties(DataNode* file) {
    ReaderSetter rs(file);
    
    string pik_type_str;
    DataNode* pik_type_node = nullptr;
    
    rs.set("match_seeds", match_seeds);
    rs.set("non_match_seeds", non_match_seeds);
    rs.set("number", number);
    rs.set("pikmin_type", pik_type_str, &pik_type_node);
    
    if(
        game.content.mob_types.list.pikmin.find(pik_type_str) ==
        game.content.mob_types.list.pikmin.end()
    ) {
        game.errors.report(
            "Unknown Pikmin type \"" + pik_type_str + "\"!",
            pik_type_node
        );
    } else {
        pik_type = game.content.mob_types.list.pikmin[pik_type_str];
    }
    
    weight = number;
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void PelletType::load_cat_resources(DataNode* file) {
    ReaderSetter rs(file);
    
    string number_image_str;
    DataNode* number_image_node = nullptr;
    
    rs.set("number_image", number_image_str, &number_image_node);
    
    bmp_number = game.content.bitmaps.list.get(number_image_str, number_image_node);
}


/**
 * @brief Unloads resources from memory.
 */
void PelletType::unload_resources() {
    game.content.bitmaps.list.free(bmp_number);
}
