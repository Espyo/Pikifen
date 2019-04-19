/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the decoration finite state machine logic.
 */

#ifndef DECORATION_FSM_INCLUDED
#define DECORATION_FSM_INCLUDED

#include "../mob_types/mob_type.h"


/* ----------------------------------------------------------------------------
 * Functions about the decoration's finite state machine and behavior.
 */
namespace decoration_fsm {
void create_fsm(mob_type* typ);

void become_idle(mob* m, void* info1, void* info2);
void be_bumped(mob* m, void* info1, void* info2);
void check_bump(mob* m, void* info1, void* info2);
};

#endif //ifndef DECORATION_FSM_INCLUDED
