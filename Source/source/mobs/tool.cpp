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

/* ----------------------------------------------------------------------------
 * Creates a new tool mob.
 */
tool::tool(
    const point &pos, tool_type* type, const float angle
):
    mob(pos, type, angle),
    too_type(type),
    holdability_flags(0),
    reserved(nullptr) {
    
}
