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
    ReaderSetter hRS(node);
    
    string effectsStr;
    string liquidStr;
    DataNode* effectsNode = nullptr;
    DataNode* liquidNode = nullptr;
    
    hRS.set("blocks_paths", blocksPaths);
    hRS.set("color", mainColor);
    hRS.set("effects", effectsStr, &effectsNode);
    hRS.set("liquid", liquidStr, &liquidNode);
    
    if(effectsNode) {
        vector<string> effectsStrs = semicolonListToVector(effectsStr);
        for(size_t e = 0; e < effectsStrs.size(); e++) {
            string effectName = effectsStrs[e];
            if(
                game.content.statusTypes.list.find(effectName) ==
                game.content.statusTypes.list.end()
            ) {
                game.errors.report(
                    "Unknown status effect \"" + effectName + "\"!",
                    effectsNode
                );
            } else {
                effects.push_back(
                    game.content.statusTypes.list[effectName]
                );
            }
        }
    }
    
    if(liquidNode) {
        if(game.content.liquids.list.find(liquidStr) == game.content.liquids.list.end()) {
            game.errors.report(
                "Unknown liquid \"" + liquidStr + "\"!",
                liquidNode
            );
        } else {
            associatedLiquid = game.content.liquids.list[liquidStr];
        }
    }
}
