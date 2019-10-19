/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Group task class and group task-related functions.
 */

#include "group_task.h"

/* ----------------------------------------------------------------------------
 * Creates a new group task mob.
 */
group_task::group_task(
    const point &pos, group_task_type* type, const float angle
):
    mob(pos, type, angle),
    tas_type(type) {
    
}
