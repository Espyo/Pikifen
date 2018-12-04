/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the resource finite state machine logic.
 */

#ifndef RESOURCE_FSM_INCLUDED
#define RESOURCE_FSM_INCLUDED

#include "../mob_types/mob_type.h"

/* ----------------------------------------------------------------------------
 * Functions about the resource's finite state machine and behavior.
 */
namespace resource_fsm {
void create_fsm(mob_type* typ);

void handle_delivery(mob* m, void* info1, void* info2);
void handle_dropped(mob* m, void* info1, void* info2);
void handle_start_moving(mob* m, void* info1, void* info2);
void lose_momentum(mob* m, void* info1, void* info2);
void start_waiting(mob* m, void* info1, void* info2);
void vanish(mob* m, void* info1, void* info2);
}

#endif //ifndef RESOURCE_FSM_INCLUDED
