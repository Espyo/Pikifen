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
    ReaderSetter lRS(node);
    
    string animation_str;
    
    lRS.set("body_color", bodyColor);
    lRS.set("shine_color", shineColor);
    lRS.set("radar_color", radarColor);
    lRS.set("shine_min_threshold", shineMinThreshold);
    lRS.set("shine_max_threshold", shineMaxThreshold);
    lRS.set("distortion_amount", distortionAmount);
    lRS.set("animation_speed", animSpeed);
}
