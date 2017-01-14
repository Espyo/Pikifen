/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the treasure finite state machine logic.
 */

#ifndef TREASURE_FSM_INCLUDED
#define TREASURE_FSM_INCLUDED

#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * Functions about the treasure's finite state machine and behavior.
 */
namespace treasure_fsm {
void create_fsm(mob_type* typ);

void handle_delivery(mob* m, void* info1, void* info2);
}

#endif //ifndef TREASURE_FSM_INCLUDED
