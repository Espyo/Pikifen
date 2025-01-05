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
    
    rs.set("body_color", body_color);
    rs.set("shine_color", shine_color);
    rs.set("radar_color", radar_color);
    rs.set("shine_amount", shine_amount);
    rs.set("distortion_amount", distortion_amount);
    rs.set("animation_speed", anim_speed);
}
