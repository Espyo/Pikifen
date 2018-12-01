/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pile finite state machine logic.
 */

#ifndef PILE_FSM_INCLUDED
#define PILE_FSM_INCLUDED

#include "../mob_types/mob_type.h"

/* ----------------------------------------------------------------------------
 * Functions about the pile's finite state machine and behavior.
 */
namespace pile_fsm {
void create_fsm(mob_type* typ);
void be_attacked(mob* m, void* info1, void* info2);
void become_idle(mob* m, void* info1, void* info2);
}

#endif //ifndef PILE_FSM_INCLUDED
