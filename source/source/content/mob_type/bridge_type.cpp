/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge type class and bridge type-related functions.
 */

#include "bridge_type.h"

#include "../../core/game.h"
#include "../mob_script/bridge_fsm.h"
#include "../mob/bridge.h"
#include "../other/mob_script.h"


/**
 * @brief Constructs a new bridge type object.
 */
BridgeType::BridgeType() :
    MobType(MOB_CATEGORY_BRIDGES) {
    
    radius = 32;
    maxHealth = 2000;
    pushable = false;
    pushes = false;
    canBlockPaths = true;
    targetType = MOB_TARGET_FLAG_PIKMIN_OBSTACLE;
    startingTeam = MOB_TEAM_OBSTACLE;
    
    areaEditorTips =
        "Link this object to another object, so that "
        "you can specify where the bridge ends. "
        "A \"Dummy\" object works perfectly for this.\n"
        "Changing the bridge's health changes how much of it is pre-built. "
        "Changing its max health changes how long Pikmin "
        "work on it for, or how many fragments are needed.";
        
    AreaEditorProp aep_chunks;
    aep_chunks.name = "Chunks";
    aep_chunks.var = "chunks";
    aep_chunks.type = AEMP_TYPE_INT;
    aep_chunks.defValue = "10";
    aep_chunks.minValue = 1;
    aep_chunks.maxValue = 50;
    aep_chunks.tooltip =
        "How many chunks it's divided by. "
        "If the bridge goes up or down, it may need "
        "more chunks in order to allow enough steps.";
    areaEditorProps.push_back(aep_chunks);
    
    bridge_fsm::createFsm(this);
}


/**
 * @brief Returns the vector of animation conversions.
 */
anim_conversion_vector BridgeType::getAnimConversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(BRIDGE_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(BRIDGE_ANIM_DESTROYED, "destroyed"));
    return v;
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void BridgeType::loadCatProperties(DataNode* file) {
    ReaderSetter rs(file);
    
    rs.set("rail_width", railWidth);
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void BridgeType::loadCatResources(DataNode* file) {
    ReaderSetter rs(file);
    
    rs.set("main_texture", mainTextureBmpName);
    rs.set("left_rail_texture", leftRailTextureBmpName);
    rs.set("right_rail_texture", rightRailTextureBmpName);
    
    if(!mainTextureBmpName.empty()) {
        bmpMainTexture = game.content.bitmaps.list.get(mainTextureBmpName);
    }
    if(!leftRailTextureBmpName.empty()) {
        bmpLeftRailTexture =
            game.content.bitmaps.list.get(leftRailTextureBmpName);
    }
    if(!rightRailTextureBmpName.empty()) {
        bmpRightRailTexture =
            game.content.bitmaps.list.get(rightRailTextureBmpName);
    }
}


/**
 * @brief Unloads resources from memory.
 */
void BridgeType::unloadResources() {
    game.content.bitmaps.list.free(mainTextureBmpName);
    game.content.bitmaps.list.free(leftRailTextureBmpName);
    game.content.bitmaps.list.free(rightRailTextureBmpName);
}
