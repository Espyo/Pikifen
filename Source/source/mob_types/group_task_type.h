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

enum GROUP_TASK_PIKMIN_POSES {
    GROUP_TASK_PIKMIN_POSE_STOPPED,
    GROUP_TASK_PIKMIN_POSE_ARMS_STRETCHED,
};


/* ----------------------------------------------------------------------------
 * A type of group task mob. This can be a pushable box, liftable gate, etc.
 */
class group_task_type : public mob_type {
public:
    size_t pikmin_goal;
    size_t max_pikmin;
    point first_row_p1;
    point first_row_p2;
    float interval_between_rows;
    size_t pikmin_per_row;
    float worker_pikmin_angle;
    size_t worker_pikmin_pose;
    
    group_task_type();
    ~group_task_type();
    
    void load_parameters(data_node* file);
};

#endif //ifndef GROUP_TASK_TYPE_INCLUDED
