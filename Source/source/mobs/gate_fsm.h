/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gate finite state machine logic.
 */

#ifndef GATE_FSM_INCLUDED
#define GATE_FSM_INCLUDED

#include "mob.h"
#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * Functions about the gate's finite state machine and behavior.
 */
namespace gate_fsm {
void create_fsm(mob_type* typ);

void open(mob* m, void* info1, void* info2);
void set_anim(mob* m, void* info1, void* info2);
};

#endif //ifndef GATE_FSM_INCLUDED
