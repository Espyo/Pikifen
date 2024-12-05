/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Liquid class and liquid-related functions.
 */

#include "liquid.h"

#include "game.h"
#include "load.h"
#include "misc_structs.h"
#include "utils/string_utils.h"


using std::string;
using std::vector;

/**
 * @brief Loads liquid data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void liquid::load_from_data_node(data_node* node, CONTENT_LOAD_LEVEL level) {
    //Content metadata.
    load_metadata_from_data_node(node);
    
    //Standard data.
    reader_setter rs(node);
    string animation_str;
    data_node* animation_node = nullptr;
    
    rs.set("animation", animation_str, &animation_node);
    rs.set("color", main_color);
    rs.set("radar_color", radar_color);
    rs.set("surface_1_speed", surface_speed[0]);
    rs.set("surface_2_speed", surface_speed[1]);
    rs.set("surface_alpha", surface_alpha);
    
    auto it = game.content.global_anim_dbs.list.find(animation_str);
    if(it != game.content.global_anim_dbs.list.end()) {
        anim.init_to_first_anim(&game.content.global_anim_dbs.list[animation_str]);
    } else {
        game.errors.report(
            "Unknown animation \"" + animation_str + "\"!",
            animation_node
        );
    }
}
