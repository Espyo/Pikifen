/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion finite state machine logic.
 */

#ifndef ONION_FSM_INCLUDED
#define ONION_FSM_INCLUDED

#include "../mob_types/mob_type.h"

/* ----------------------------------------------------------------------------
 * Functions about the Onion's finite state machine and behavior.
 */
namespace onion_fsm {
void create_fsm(mob_type* typ);

void receive_mob(mob* m, void* info1, void* info2);
}

#endif //ifndef ONION_FSM_INCLUDED
