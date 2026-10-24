/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Hazard class and hazard-related functions.
 */

#include "hazard.hpp"

#include "../../core/game.hpp"
#include "../../core/misc_structs.hpp"
#include "../../util/string_utils.hpp"


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
        forIdx(e, effectsStrs) {
            string effectName = effectsStrs[e];
            if(!isInMap(game.content.statusTypes.list, effectName)) {
                game.log.write(
                    LOG_LEVEL_ERROR,
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
        if(!isInMap(game.content.liquids.list, liquidStr)) {
            game.log.write(
                LOG_LEVEL_ERROR,
                "Unknown liquid \"" + liquidStr + "\"!",
                liquidNode
            );
        } else {
            associatedLiquid = game.content.liquids.list[liquidStr];
        }
    }
}
