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
    ReaderSetter sRS(node);
    
    string effectsStr;
    string iconStr;
    DataNode* effectsNode = nullptr;
    DataNode* iconNode = nullptr;
    
    sRS.set("effects", effectsStr, &effectsNode);
    sRS.set("icon", iconStr, &iconNode);
    sRS.set("group", group);
    sRS.set("group_pikmin_only", groupPikminOnly);
    sRS.set("affects_user", affectsUser);
    sRS.set("angle", angle);
    sRS.set("distance_range", distanceRange);
    sRS.set("angle_range", angleRange);
    sRS.set("color", mainColor);
    sRS.set("ingredients_needed", ingredientsNeeded);
    sRS.set("buries_pikmin", buriesPikmin);
    
    if(effectsNode) {
        vector<string> effectsStrs =
            semicolonListToVector(effectsNode->value);
        for(size_t e = 0; e < effectsStrs.size(); e++) {
            string effectName = effectsStrs[e];
            if(!isInMap(game.content.statusTypes.list, effectName)) {
                game.errors.report(
                    "Unknown status effect \"" + effectName + "\"!",
                    effectsNode
                );
            } else {
                effects.push_back(game.content.statusTypes.list[effectName]);
            }
        }
    }
    
    angle = degToRad(angle);
    angleRange = degToRad(angleRange);
    
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        bmpSpray = game.content.bitmaps.list.get(iconStr, iconNode);
    }
}
