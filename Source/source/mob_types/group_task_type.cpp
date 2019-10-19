/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Group task type class and group task type-related functions.
 */

#include "group_task_type.h"

#include "../mobs/mob.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a new type of group task mob.
 */
group_task_type::group_task_type() :
    mob_type(MOB_CATEGORY_GROUP_TASKS),
    pikmin_goal(10),
    max_pikmin(20),
    interval_between_rows(10.0f),
    pikmin_per_row(10) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
}

group_task_type::~group_task_type() { }


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void group_task_type::load_parameters(data_node* file) {
    reader_setter rs(file);
    
    rs.set("pikmin_goal", pikmin_goal);
    rs.set("max_pikmin", max_pikmin);
    rs.set("first_row_p1", first_row_p1);
    rs.set("first_row_p2", first_row_p2);
    rs.set("interval_between_rows", interval_between_rows);
    rs.set("pikmin_per_row", pikmin_per_row);
}
