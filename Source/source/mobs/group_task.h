/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the group task class and group task-related functions.
 */

#ifndef GROUP_TASK_INCLUDED
#define GROUP_TASK_INCLUDED

#include <vector>

#include "../mob_types/group_task_type.h"
#include "mob.h"
#include "pikmin.h"


/* ----------------------------------------------------------------------------
 * A mob that requires multiple Pikmin to work together in order to clear.
 */
class group_task : public mob {
private:
    //Combined Pikmin power put into the task right now. Cached for performance.
    float power;
    //Has it already run the "task finished" code?
    bool ran_task_finished_code;
    
    void update_spot_absolute_positions();
    
public:
    group_task_type* tas_type;
    
    struct group_task_spot {
        point relative_pos;
        point absolute_pos;
        unsigned char state;
        pikmin* pikmin_here;
        group_task_spot(const point &pos);
    };
    
    vector<group_task_spot> spots;
    
    group_task(const point &pos, group_task_type* type, const float angle);
    void add_worker(pikmin* who);
    void finish_task();
    void free_up_spot(pikmin* whose);
    group_task_spot* get_free_spot();
    float get_power();
    point get_spot_pos(pikmin* whose);
    void reserve_spot(group_task_spot* spot, pikmin* who);
    virtual void tick_class_specifics();
};

#endif //ifndef GROUP_TASK_INCLUDED
