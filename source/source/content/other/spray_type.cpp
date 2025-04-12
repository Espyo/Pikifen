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
void SprayType::loadFromDataNode(
    DataNode* node, CONTENT_LOAD_LEVEL level
) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter rs(node);
    
    string effects_str;
    string icon_str;
    DataNode* effects_node = nullptr;
    DataNode* icon_node = nullptr;
    
    rs.set("effects", effects_str, &effects_node);
    rs.set("icon", icon_str, &icon_node);
    rs.set("group", group);
    rs.set("group_pikmin_only", groupPikminOnly);
    rs.set("affects_user", affectsUser);
    rs.set("angle", angle);
    rs.set("distance_range", distanceRange);
    rs.set("angle_range", angleRange);
    rs.set("color", mainColor);
    rs.set("ingredients_needed", ingredientsNeeded);
    rs.set("buries_pikmin", buriesPikmin);
    
    if(effects_node) {
        vector<string> effects_strs =
            semicolonListToVector(effects_node->value);
        for(size_t e = 0; e < effects_strs.size(); e++) {
            string effect_name = effects_strs[e];
            if(
                game.content.statusTypes.list.find(effect_name) ==
                game.content.statusTypes.list.end()
            ) {
                game.errors.report(
                    "Unknown status effect \"" + effect_name + "\"!",
                    effects_node
                );
            } else {
                effects.push_back(game.content.statusTypes.list[effect_name]);
            }
        }
    }
    
    angle = degToRad(angle);
    angleRange = degToRad(angleRange);
    
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        bmpSpray = game.content.bitmaps.list.get(icon_str, icon_node);
    }
}
