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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/mob.h"


/**
 * @brief Constructs a new group task type object.
 */
GroupTaskType::GroupTaskType() :
    MobType(MOB_CATEGORY_GROUP_TASKS) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    
    AreaEditorProp aepPowerGoal;
    aepPowerGoal.name = "Power goal";
    aepPowerGoal.var = "power_goal";
    aepPowerGoal.type = AEMP_TYPE_INT;
    aepPowerGoal.defValue = i2s(powerGoal);
    aepPowerGoal.tooltip = "Pikmin power required for the task's goal.";
    areaEditorProps.push_back(aepPowerGoal);
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void GroupTaskType::loadCatProperties(DataNode* file) {
    ReaderSetter gRS(file);
    
    string contributionMethodStr;
    string workerPikminPoseStr;
    DataNode* contributionMethodNode = nullptr;
    DataNode* workerPikminPoseNode = nullptr;
    
    gRS.set(
        "contribution_method", contributionMethodStr,
        &contributionMethodNode
    );
    gRS.set("flying_pikmin_only", flyingPikminOnly);
    gRS.set("first_row_p1", firstRowP1);
    gRS.set("first_row_p2", firstRowP2);
    gRS.set("interval_between_rows", intervalBetweenRows);
    gRS.set("max_pikmin", maxPikmin);
    gRS.set("pikmin_per_row", pikminPerRow);
    gRS.set("power_goal", powerGoal);
    gRS.set("speed_bonus", speedBonus);
    gRS.set("spots_z", spotsZ);
    gRS.set("worker_pikmin_angle", workerPikminAngle);
    gRS.set(
        "worker_pikmin_pose", workerPikminPoseStr, &workerPikminPoseNode
    );
    
    if(contributionMethodNode) {
        readEnumProp(
            contributionMethodStr,
        (int*) &contributionMethod, {
            "normal",
            "weight",
            "carry_strength",
            "push_strength"
        },
        "contribution type",
        contributionMethodNode
        );
    }
    
    workerPikminAngle = degToRad(workerPikminAngle);
    
    if(workerPikminPoseNode) {
        readEnumProp(
            workerPikminPoseStr,
        (int*) &workerPikminPose, {
            "stopped",
            "arms_out",
            "pushing",
            "carrying",
            "carrying_light"
        },
        "pose",
        workerPikminPoseNode
        );
    }
    
    areaEditorProps.back().defValue = i2s(powerGoal);
}
