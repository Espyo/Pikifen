/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship finite state machine logic.
 */

#ifndef SHIP_FSM_INCLUDED
#define SHIP_FSM_INCLUDED

#include "../mob_types/mob_type.h"

/* ----------------------------------------------------------------------------
 * Functions about the ship's finite state machine and behavior.
 */
namespace ship_fsm {
void create_fsm(mob_type* typ);

void receive_mob(mob* m, void* info1, void* info2);
void set_anim(mob* m, void* info1, void* info2);
}

#endif //ifndef SHIP_FSM_INCLUDED
