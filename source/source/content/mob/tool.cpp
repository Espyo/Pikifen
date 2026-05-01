/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Tool class and tool-related functions.
 */

#include "tool.h"


/**
 * @brief Constructs a new tool object.
 *
 * @param center Starting center coordinates.
 * @param type Tool type this mob belongs to.
 * @param angle Starting angle.
 */
Tool::Tool(
    const Point& center, ToolType* type, float angle
):
    Mob(center, type, angle),
    tooType(type) {
    
    team = MOB_TEAM_NONE;
}
