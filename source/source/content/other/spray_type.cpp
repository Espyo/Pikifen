/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Spray type class and spray type-related functions.
 */

#include "spray_type.h"

#include "../../core/game.h"
#include "../../core/misc_structs.h"
#include "../../util/string_utils.h"


/**
 * @brief Loads spray type data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void SprayType::load_from_data_node(
    DataNode* node, CONTENT_LOAD_LEVEL level
) {
    //Content metadata.
    load_metadata_from_data_node(node);
    
    //Standard data.
    ReaderSetter rs(node);
    
    string effects_str;
    string icon_str;
    DataNode* effects_node = nullptr;
    DataNode* icon_node = nullptr;
    
    rs.set("effects", effects_str, &effects_node);
    rs.set("icon", icon_str, &icon_node);
    rs.set("group", group);
    rs.set("group_pikmin_only", group_pikmin_only);
    rs.set("affects_user", affects_user);
    rs.set("angle", angle);
    rs.set("distance_range", distance_range);
    rs.set("angle_range", angle_range);
    rs.set("color", main_color);
    rs.set("ingredients_needed", ingredients_needed);
    rs.set("buries_pikmin", buries_pikmin);
    
    if(effects_node) {
        vector<string> effects_strs =
            semicolon_list_to_vector(effects_node->value);
        for(size_t e = 0; e < effects_strs.size(); e++) {
            string effect_name = effects_strs[e];
            if(
                game.content.status_types.list.find(effect_name) ==
                game.content.status_types.list.end()
            ) {
                game.errors.report(
                    "Unknown status effect \"" + effect_name + "\"!",
                    effects_node
                );
            } else {
                effects.push_back(game.content.status_types.list[effect_name]);
            }
        }
    }
    
    angle = deg_to_rad(angle);
    angle_range = deg_to_rad(angle_range);
    
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        bmp_spray = game.content.bitmaps.list.get(icon_str, icon_node);
    }
}
