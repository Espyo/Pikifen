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
    
    rs.set("body_color", bodyColor);
    rs.set("shine_color", shineColor);
    rs.set("radar_color", radarColor);
    rs.set("shine_min_threshold", shineMinThreshold);
    rs.set("shine_max_threshold", shineMaxThreshold);
    rs.set("distortion_amount", distortionAmount);
    rs.set("animation_speed", animSpeed);
}
