/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drop finite state machine logic.
 */

#ifndef DROP_FSM_INCLUDED
#define DROP_FSM_INCLUDED

#include "../mob_types/mob_type.h"


/**
 * @brief Functions about the drop's finite state machine and behavior.
 */
namespace drop_fsm {
void create_fsm(mob_type* typ);

void land(mob* m, void* info1, void* info2);
void on_touched(mob* m, void* info1, void* info2);
void set_bumped_anim(mob* m, void* info1, void* info2);
void set_falling_anim(mob* m, void* info1, void* info2);
void set_idling_anim(mob* m, void* info1, void* info2);
}


#endif //ifndef DROP_FSM_INCLUDED
