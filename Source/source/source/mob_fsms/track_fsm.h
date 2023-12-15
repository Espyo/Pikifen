/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the track finite state machine logic.
 */

#ifndef TRACK_FSM_INCLUDED
#define TRACK_FSM_INCLUDED

#include "../mob_types/mob_type.h"


/* ----------------------------------------------------------------------------
 * Functions about the track's finite state machine and behavior.
 */
namespace track_fsm {
void create_fsm(mob_type* typ);

void on_touched(mob* m, void* info1, void* info2);
void spawn(mob* m, void* info1, void* info2);
}


#endif //ifndef TRACK_FSM_INCLUDED
