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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob/mob.h"


/**
 * @brief Constructs a new group task type object.
 */
GroupTaskType::GroupTaskType() :
    MobType(MOB_CATEGORY_GROUP_TASKS) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    AreaEditorProp aep_power_goal;
    aep_power_goal.name = "Power goal";
    aep_power_goal.var = "power_goal";
    aep_power_goal.type = AEMP_TYPE_INT;
    aep_power_goal.defValue = i2s(powerGoal);
    aep_power_goal.tooltip = "Pikmin power required for the task's goal.";
    areaEditorProps.push_back(aep_power_goal);
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void GroupTaskType::loadCatProperties(DataNode* file) {
    ReaderSetter rs(file);
    
    string contribution_method_str;
    string worker_pikmin_pose_str;
    DataNode* contribution_method_node = nullptr;
    DataNode* worker_pikmin_pose_node = nullptr;
    
    rs.set(
        "contribution_method", contribution_method_str,
        &contribution_method_node
    );
    rs.set("flying_pikmin_only", flyingPikminOnly);
    rs.set("first_row_p1", firstRowP1);
    rs.set("first_row_p2", firstRowP2);
    rs.set("interval_between_rows", intervalBetweenRows);
    rs.set("max_pikmin", maxPikmin);
    rs.set("pikmin_per_row", pikminPerRow);
    rs.set("power_goal", powerGoal);
    rs.set("speed_bonus", speedBonus);
    rs.set("spots_z", spotsZ);
    rs.set("worker_pikmin_angle", workerPikminAngle);
    rs.set(
        "worker_pikmin_pose", worker_pikmin_pose_str, &worker_pikmin_pose_node
    );
    
    if(contribution_method_node) {
        if(contribution_method_str == "normal") {
            contributionMethod = GROUP_TASK_CONTRIBUTION_NORMAL;
        } else if(contribution_method_str == "weight") {
            contributionMethod = GROUP_TASK_CONTRIBUTION_WEIGHT;
        } else if(contribution_method_str == "carry_strength") {
            contributionMethod = GROUP_TASK_CONTRIBUTION_CARRY_STRENGTH;
        } else if(contribution_method_str == "push_strength") {
            contributionMethod = GROUP_TASK_CONTRIBUTION_PUSH_STRENGTH;
        } else {
            game.errors.report(
                "Unknown contribution type \"" +
                contribution_method_str + "\"!", contribution_method_node
            );
        }
    }
    
    workerPikminAngle = degToRad(workerPikminAngle);
    
    if(worker_pikmin_pose_node) {
        if(worker_pikmin_pose_str == "stopped") {
            workerPikminPose = GROUP_TASK_PIKMIN_POSE_STOPPED;
        } else if(worker_pikmin_pose_str == "arms_out") {
            workerPikminPose = GROUP_TASK_PIKMIN_POSE_ARMS_OUT;
        } else if(worker_pikmin_pose_str == "pushing") {
            workerPikminPose = GROUP_TASK_PIKMIN_POSE_PUSHING;
        } else if(worker_pikmin_pose_str == "carrying") {
            workerPikminPose = GROUP_TASK_PIKMIN_POSE_CARRYING;
        } else {
            game.errors.report(
                "Unknown pose \"" + worker_pikmin_pose_str + "\"!",
                worker_pikmin_pose_node
            );
        }
    }
    
    areaEditorProps.back().defValue = i2s(powerGoal);
}
