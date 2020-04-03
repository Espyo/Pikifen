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

#include "../utils/string_utils.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates a new type of tool mob.
 */
tool_type::tool_type() :
    mob_type(MOB_CATEGORY_TOOLS),
    bmp_icon(NULL),
    can_be_hotswapped(true),
    dropped_when_pikmin_is_whistled(false),
    dropped_when_pikmin_lands(true),
    dropped_when_pikmin_lands_on_opponent(false),
    stuck_when_pikmin_lands_on_opponent(false),
    pikmin_returns_after_using(true) {
    
    target_type = MOB_TARGET_TYPE_FRAGILE;
    hurtable_targets =
        MOB_TARGET_TYPE_PLAYER |
        MOB_TARGET_TYPE_ENEMY |
        MOB_TARGET_TYPE_WEAK_PLAIN_OBSTACLE |
        MOB_TARGET_TYPE_STRONG_PLAIN_OBSTACLE |
        MOB_TARGET_TYPE_EXPLODABLE |
        MOB_TARGET_TYPE_EXPLODABLE_PIKMIN_OBSTACLE;
        
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 */
void tool_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
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


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void tool_type::load_resources(data_node* file) {
    reader_setter rs(file);
    
    string icon_str;
    data_node* icon_node;
    
    rs.set("icon", icon_str, &icon_node);
    
    bmp_icon = bitmaps.get(icon_str, icon_node);
}
