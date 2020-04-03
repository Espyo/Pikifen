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

#include "../functions.h"
#include "../mobs/mob.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a new type of group task mob.
 */
group_task_type::group_task_type() :
    mob_type(MOB_CATEGORY_GROUP_TASKS),
    power_goal(10),
    max_pikmin(20),
    interval_between_rows(10.0f),
    pikmin_per_row(10),
    worker_pikmin_angle(0),
    worker_pikmin_pose(GROUP_TASK_PIKMIN_POSE_STOPPED),
    contribution_method(GROUP_TASK_CONTRIBUTION_NORMAL),
    speed_bonus(1.0f) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 */
void group_task_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string contribution_method_str;
    string worker_pikmin_pose_str;
    data_node* contribution_method_node;
    data_node* worker_pikmin_pose_node;
    
    rs.set(
        "contribution_method", contribution_method_str,
        &contribution_method_node
    );
    rs.set("first_row_p1", first_row_p1);
    rs.set("first_row_p2", first_row_p2);
    rs.set("interval_between_rows", interval_between_rows);
    rs.set("max_pikmin", max_pikmin);
    rs.set("pikmin_per_row", pikmin_per_row);
    rs.set("power_goal", power_goal);
    rs.set("speed_bonus", speed_bonus);
    rs.set("worker_pikmin_angle", worker_pikmin_angle);
    rs.set(
        "worker_pikmin_pose", worker_pikmin_pose_str, &worker_pikmin_pose_node
    );
    
    if(contribution_method_node) {
        if(contribution_method_str == "normal") {
            contribution_method = GROUP_TASK_CONTRIBUTION_NORMAL;
        } else if(contribution_method_str == "weight") {
            contribution_method = GROUP_TASK_CONTRIBUTION_WEIGHT;
        } else if(contribution_method_str == "carry_strength") {
            contribution_method = GROUP_TASK_CONTRIBUTION_CARRY_STRENGTH;
        } else if(contribution_method_str == "push_strength") {
            contribution_method = GROUP_TASK_CONTRIBUTION_PUSH_STRENGTH;
        } else {
            log_error(
                "Unknown contribution type \"" +
                contribution_method_str + "\"!", contribution_method_node
            );
        }
    }
    
    worker_pikmin_angle = deg_to_rad(worker_pikmin_angle);
    
    if(worker_pikmin_pose_node) {
        if(worker_pikmin_pose_str == "stopped") {
            worker_pikmin_pose = GROUP_TASK_PIKMIN_POSE_STOPPED;
        } else if(worker_pikmin_pose_str == "arms_stretched") {
            worker_pikmin_pose = GROUP_TASK_PIKMIN_POSE_ARMS_STRETCHED;
        } else if(worker_pikmin_pose_str == "pushing") {
            worker_pikmin_pose = GROUP_TASK_PIKMIN_POSE_PUSHING;
        } else {
            log_error(
                "Unknown pose \"" + worker_pikmin_pose_str + "\"!",
                worker_pikmin_pose_node
            );
        }
    }
}
