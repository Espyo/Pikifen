/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bouncer finite state machine logic.
 */

#ifndef BOUNCER_FSM_INCLUDED
#define BOUNCER_FSM_INCLUDED

#include "../mob_types/mob_type.h"


/* ----------------------------------------------------------------------------
 * Functions about the bouncer's finite state machine and behavior.
 */
namespace bouncer_fsm {
void create_fsm(mob_type* typ);

void handle_mob(mob* m, void* info1, void* info2);
void set_bouncing_animation(mob* m, void* info1, void* info2);
void set_idling_animation(mob* m, void* info1, void* info2);
};

#endif //ifndef BOUNCER_FSM_INCLUDED
