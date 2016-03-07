/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gate finite state machine logic.
 */

#ifndef GATE_FSM_INCLUDED
#define GATE_FSM_INCLUDED

#include "mob.h"

namespace gate_fsm {
void open(mob* m, void* info1, void* info2);
void take_damage(mob* m, void* info1, void* info2);
void set_anim(mob* m, void* info1, void* info2);
};

#endif //ifndef GATE_FSM_INCLUDED
