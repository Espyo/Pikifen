/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge finite state machine logic.
 */

#ifndef BRIDGE_FSM_INCLUDED
#define BRIDGE_FSM_INCLUDED

#include "../mob_types/mob_type.h"


/* ----------------------------------------------------------------------------
 * Functions about the bridge's finite state machine and behavior.
 */
namespace bridge_fsm {
void create_fsm(mob_type* typ);

void check_health(mob* m, void* info1, void* info2);
void open(mob* m, void* info1, void* info2);
void set_anim(mob* m, void* info1, void* info2);
void setup(mob* m, void* info1, void* info2);
}


#endif //ifndef BRIDGE_FSM_INCLUDED
