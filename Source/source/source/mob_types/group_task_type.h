/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the group task type class and group task type-related functions.
 */

#ifndef GROUP_TASK_TYPE_INCLUDED
#define GROUP_TASK_TYPE_INCLUDED

#include "mob_type.h"


//Poses that Pikmin working on a group task can take.
enum GROUP_TASK_PIKMIN_POSES {
    //Stopped.
    GROUP_TASK_PIKMIN_POSE_STOPPED,
    //Arms stretched sideways.
    GROUP_TASK_PIKMIN_POSE_ARMS_STRETCHED,
    //Pushing forward.
    GROUP_TASK_PIKMIN_POSE_PUSHING,
    //Carrying.
    GROUP_TASK_PIKMIN_POSE_CARRYING,
};


//Methods by which a Pikmin can contribute to a group task.
enum GROUP_TASK_CONTRIBUTION_METHODS {
    //Each Pikmin contributes by 1.
    GROUP_TASK_CONTRIBUTION_NORMAL,
    //Each Pikmin contributes with its weight.
    GROUP_TASK_CONTRIBUTION_WEIGHT,
    //Each Pikmin contributes with its carrying strength.
    GROUP_TASK_CONTRIBUTION_CARRY_STRENGTH,
    //Each Pikmin contributes with its pushing strength.
    GROUP_TASK_CONTRIBUTION_PUSH_STRENGTH,
};


/* ----------------------------------------------------------------------------
 * A type of group task mob. This can be a pushable box, liftable gate, etc.
 */
class group_task_type : public mob_type {
public:
    //Default power requirement in order to reach the goal.
    size_t power_goal;
    //Maximum amount of Pikmin that can work.
    size_t max_pikmin;
    //First point of the first row of workers.
    point first_row_p1;
    //Second point of the first row of workers.
    point first_row_p2;
    //Z coordinate of the contributor spots.
    float spots_z;
    //Interval between each row of workers.
    float interval_between_rows;
    //How many Pikmin spots per row of workers.
    size_t pikmin_per_row;
    //What (relative) angle the Pikmin should face when working.
    float worker_pikmin_angle;
    //Pose that worker Pikmin should take.
    GROUP_TASK_PIKMIN_POSES worker_pikmin_pose;
    //How each worker Pikmin contributes to the power.
    GROUP_TASK_CONTRIBUTION_METHODS contribution_method;
    //How much to increase the mob's speed relative to the current power.
    float speed_bonus;
    //If true, only flying Pikmin can work on it.
    bool flying_pikmin_only;
    
    group_task_type();
    
    void load_properties(data_node* file) override;
};


#endif //ifndef GROUP_TASK_TYPE_INCLUDED
