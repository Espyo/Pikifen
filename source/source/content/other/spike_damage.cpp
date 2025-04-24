/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Spike damage class and spike damage-related functions.
 */

#include "spike_damage.h"

#include "../../core/game.h"
#include "../../core/misc_structs.h"


/**
 * @brief Loads spike damage type data from a data node.
 *
 * @param node Data node to load from.
 */
void SpikeDamageType::loadFromDataNode(DataNode* node) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter sRS(node);
    
    string particleGeneratorName;
    string statusName;
    DataNode* damageNode = nullptr;
    DataNode* particleGeneratorNode = nullptr;
    DataNode* statusNameNode = nullptr;
    
    sRS.set("damage", damage, &damageNode);
    sRS.set("ingestion_only", ingestionOnly);
    sRS.set("is_damage_ratio", isDamageRatio);
    sRS.set("status_to_apply", statusName, &statusNameNode);
    sRS.set(
        "particle_generator", particleGeneratorName,
        &particleGeneratorNode
    );
    
    if(particleGeneratorNode) {
        if(
            game.content.particleGens.list.find(particleGeneratorName) ==
            game.content.particleGens.list.end()
        ) {
            game.errors.report(
                "Unknown particle generator \"" +
                particleGeneratorName + "\"!", particleGeneratorNode
            );
        } else {
            particleGen =
                &game.content.particleGens.list[particleGeneratorName];
            particleOffsetPos =
                s2p(
                    node->getChildByName("particle_offset")->value,
                    &particleOffsetZ
                );
        }
    }
    
    if(statusNameNode) {
        auto s = game.content.statusTypes.list.find(statusName);
        if(s != game.content.statusTypes.list.end()) {
            statusToApply = s->second;
        } else {
            game.errors.report(
                "Unknown status type \"" + statusName + "\"!",
                statusNameNode
            );
        }
    }
}
