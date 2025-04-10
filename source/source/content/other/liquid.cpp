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

#include "../../core/game.h"
#include "../../core/load.h"
#include "../../core/misc_structs.h"
#include "../../util/string_utils.h"


using std::string;
using std::vector;

/**
 * @brief Loads liquid data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void Liquid::loadFromDataNode(DataNode* node, CONTENT_LOAD_LEVEL level) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter rs(node);
    string animation_str;
    
    rs.set("body_color", body_color);
    rs.set("shine_color", shine_color);
    rs.set("radar_color", radar_color);
    rs.set("shine_min_threshold", shine_min_threshold);
    rs.set("shine_max_threshold", shine_max_threshold);
    rs.set("distortion_amount", distortion_amount);
    rs.set("animation_speed", anim_speed);
}
