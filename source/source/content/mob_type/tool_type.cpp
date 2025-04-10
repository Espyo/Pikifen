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
    
    target_type = MOB_TARGET_FLAG_FRAGILE;
    hurtable_targets =
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
    ReaderSetter rs(file);
    
    rs.set("dropped_when_pikmin_is_whistled", dropped_when_pikmin_is_whistled);
    rs.set("dropped_when_pikmin_lands", dropped_when_pikmin_lands);
    rs.set(
        "dropped_when_pikmin_lands_on_opponent",
        dropped_when_pikmin_lands_on_opponent
    );
    rs.set("pikmin_returns_after_using", pikmin_returns_after_using);
    rs.set(
        "stuck_when_pikmin_lands_on_opponent",
        stuck_when_pikmin_lands_on_opponent
    );
}


/**
 * @brief Loads resources into memory.
 *
 * @param file File to read from.
 */
void ToolType::loadCatResources(DataNode* file) {
    ReaderSetter rs(file);
    
    string icon_str;
    DataNode* icon_node = nullptr;
    
    rs.set("icon", icon_str, &icon_node);
    
    bmp_icon = game.content.bitmaps.list.get(icon_str, icon_node);
}
