/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the group task type class and group task type-related functions.
 */

#pragma once

#include "mob_type.h"


//Poses that Pikmin working on a group task can take.
enum GROUP_TASK_PIKMIN_POSE {

    //Stopped.
    GROUP_TASK_PIKMIN_POSE_STOPPED,
    
    //Arms stretched out sideways.
    GROUP_TASK_PIKMIN_POSE_ARMS_OUT,
    
    //Pushing forward.
    GROUP_TASK_PIKMIN_POSE_PUSHING,
    
    //Carrying.
    GROUP_TASK_PIKMIN_POSE_CARRYING,
    
};


//Methods by which a Pikmin can contribute to a group task.
enum GROUP_TASK_CONTRIBUTION {

    //Each Pikmin contributes by 1.
    GROUP_TASK_CONTRIBUTION_NORMAL,
    
    //Each Pikmin contributes with its weight.
    GROUP_TASK_CONTRIBUTION_WEIGHT,
    
    //Each Pikmin contributes with its carrying strength.
    GROUP_TASK_CONTRIBUTION_CARRY_STRENGTH,
    
    //Each Pikmin contributes with its pushing strength.
    GROUP_TASK_CONTRIBUTION_PUSH_STRENGTH,
    
};


/**
 * @brief A type of group task mob. This can be a pushable box,
 * liftable gate, etc.
 */
class GroupTaskType : public MobType {

public:

    //--- Members ---
    
    //Default power requirement in order to reach the goal.
    size_t power_goal = 10;
    
    //Maximum amount of Pikmin that can work.
    size_t max_pikmin = 20;
    
    //First point of the first row of workers.
    Point first_row_p1;
    
    //Second point of the first row of workers.
    Point first_row_p2;
    
    //Z coordinate of the contributor spots.
    float spots_z = 0.0f;
    
    //Interval between each row of workers.
    float interval_between_rows = 10.0f;
    
    //How many Pikmin spots per row of workers.
    size_t pikmin_per_row = 10;
    
    //What (relative) angle the Pikmin should face when working.
    float worker_pikmin_angle = 0.0f;
    
    //Pose that worker Pikmin should take.
    GROUP_TASK_PIKMIN_POSE worker_pikmin_pose = GROUP_TASK_PIKMIN_POSE_STOPPED;
    
    //How each worker Pikmin contributes to the power.
    GROUP_TASK_CONTRIBUTION contribution_method = GROUP_TASK_CONTRIBUTION_NORMAL;
    
    //How much to increase the mob's speed relative to the current power.
    float speed_bonus = 1.0f;
    
    //If true, only flying Pikmin can work on it.
    bool flying_pikmin_only = false;
    
    
    //--- Function declarations ---
    
    GroupTaskType();
    void load_cat_properties(DataNode* file) override;
    
};
