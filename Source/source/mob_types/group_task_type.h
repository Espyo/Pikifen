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
    size_t power_goal;
    size_t max_pikmin;
    point first_row_p1;
    point first_row_p2;
    float spots_z;
    float interval_between_rows;
    size_t pikmin_per_row;
    float worker_pikmin_angle;
    GROUP_TASK_PIKMIN_POSES worker_pikmin_pose;
    GROUP_TASK_CONTRIBUTION_METHODS contribution_method;
    float speed_bonus;
    bool flying_pikmin_only;
    
    group_task_type();
    
    void load_properties(data_node* file);
};


#endif //ifndef GROUP_TASK_TYPE_INCLUDED
