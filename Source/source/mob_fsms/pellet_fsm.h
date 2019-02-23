/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pellet finite state machine logic.
 */

#ifndef PELLET_FSM_INCLUDED
#define PELLET_FSM_INCLUDED

#include "../mob_types/mob_type.h"

/* ----------------------------------------------------------------------------
 * Functions about the Onion's finite state machine and behavior.
 */
namespace pellet_fsm {
void create_fsm(mob_type* typ);

void stand_still(mob* m, void* info1, void* info2);
}

#endif //ifndef PELLET_FSM_INCLUDED
