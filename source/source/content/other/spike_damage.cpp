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
void SpikeDamageType::load_from_data_node(DataNode* node) {
    //Content metadata.
    load_metadata_from_data_node(node);
    
    //Standard data.
    ReaderSetter rs(node);
    
    string particle_generator_name;
    string status_name;
    DataNode* damage_node = nullptr;
    DataNode* particle_generator_node = nullptr;
    DataNode* status_name_node = nullptr;
    
    rs.set("damage", damage, &damage_node);
    rs.set("ingestion_only", ingestion_only);
    rs.set("is_damage_ratio", is_damage_ratio);
    rs.set("status_to_apply", status_name, &status_name_node);
    rs.set(
        "particle_generator", particle_generator_name,
        &particle_generator_node
    );
    
    if(particle_generator_node) {
        if(
            game.content.particle_gen.list.find(particle_generator_name) ==
            game.content.particle_gen.list.end()
        ) {
            game.errors.report(
                "Unknown particle generator \"" +
                particle_generator_name + "\"!", particle_generator_node
            );
        } else {
            particle_gen =
                &game.content.particle_gen.list[particle_generator_name];
            particle_offset_pos =
                s2p(
                    node->getChildByName("particle_offset")->value,
                    &particle_offset_z
                );
        }
    }
    
    if(status_name_node) {
        auto s = game.content.status_types.list.find(status_name);
        if(s != game.content.status_types.list.end()) {
            status_to_apply = s->second;
        } else {
            game.errors.report(
                "Unknown status type \"" + status_name + "\"!",
                status_name_node
            );
        }
    }
}
