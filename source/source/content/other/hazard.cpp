/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Hazard class and hazard-related functions.
 */

#include "hazard.h"

#include "../../core/game.h"
#include "../../core/misc_structs.h"
#include "../../util/string_utils.h"


using std::string;
using std::vector;

/**
 * @brief Loads hazard data from a data node.
 *
 * @param node Data node to load from.
 */
void Hazard::loadFromDataNode(DataNode* node) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter rs(node);
    
    string effects_str;
    string liquid_str;
    DataNode* effects_node = nullptr;
    DataNode* liquid_node = nullptr;
    
    rs.set("blocks_paths", blocksPaths);
    rs.set("color", mainColor);
    rs.set("effects", effects_str, &effects_node);
    rs.set("liquid", liquid_str, &liquid_node);
    
    if(effects_node) {
        vector<string> effects_strs = semicolonListToVector(effects_str);
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
                effects.push_back(
                    game.content.statusTypes.list[effect_name]
                );
            }
        }
    }
    
    if(liquid_node) {
        if(game.content.liquids.list.find(liquid_str) == game.content.liquids.list.end()) {
            game.errors.report(
                "Unknown liquid \"" + liquid_str + "\"!",
                liquid_node
            );
        } else {
            associatedLiquid = game.content.liquids.list[liquid_str];
        }
    }
}
