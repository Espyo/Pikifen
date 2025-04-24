/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Tool type class and tool type-related functions.
 */

#include "tool_type.h"

#include "../../core/game.h"
#include "../../util/string_utils.h"


/**
 * @brief Constructs a new tool type object.
 */
ToolType::ToolType() :
    MobType(MOB_CATEGORY_TOOLS) {
    
    targetType = MOB_TARGET_FLAG_FRAGILE;
    hurtableTargets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_WEAK_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_STRONG_PLAIN_OBSTACLE |
        MOB_TARGET_FLAG_EXPLODABLE |
        MOB_TARGET_FLAG_EXPLODABLE_PIKMIN_OBSTACLE;
        
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void ToolType::loadCatProperties(DataNode* file) {
    ReaderSetter tRS(file);
    
    tRS.set("dropped_when_pikmin_is_whistled", droppedWhenPikminIsWhistled);
    tRS.set("dropped_when_pikmin_lands", droppedWhenPikminLands);
    tRS.set(
        "dropped_when_pikmin_lands_on_opponent",
        droppedWhenPikminLandsOnOpponent
    );
    tRS.set("pikmin_returns_after_using", pikminReturnsAfterUsing);
    tRS.set(
        "stuck_when_pikmin_lands_on_opponent",
        stuckWhenPikminLandsOnOpponent
    );
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void ToolType::loadCatResources(DataNode* file) {
    ReaderSetter tRS(file);
    
    string iconStr;
    DataNode* iconNode = nullptr;
    
    tRS.set("icon", iconStr, &iconNode);
    
    bmpIcon = game.content.bitmaps.list.get(iconStr, iconNode);
}
