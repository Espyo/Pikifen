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


/* ----------------------------------------------------------------------------
 * A mob that requires multiple Pikmin to work together in order to clear.
 */
class group_task : public mob {
public:
    group_task_type* tas_type;
    
    group_task(const point &pos, group_task_type* type, const float angle);
};

#endif //ifndef GROUP_TASK_INCLUDED
