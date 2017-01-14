/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the generic mob finite state machine logic.
 */

#ifndef GEN_MOB_FSM_INCLUDED
#define GEN_MOB_FSM_INCLUDED

#include "mob.h"

/* ----------------------------------------------------------------------------
 * Functions about the generic mob's finite state machine and behavior.
 */
namespace gen_mob_fsm {
const float CARRYING_STUCK_SWAY_AMOUNT = 20.0f;
const float CARRYING_STUCK_SPEED_MULT = 0.3f;

void touch_hazard(mob* m, void* info1, void* info2);
void touch_spray(mob* m, void* info1, void* info2);
void lose_health(mob* m, void* info1, void* info2);
void handle_carrier_added(mob* m, void* info1, void* info2);
void handle_carrier_removed(mob* m, void* info1, void* info2);
void carry_begin_move(mob* m, void* info1, void* info2);
void carry_stop_move(mob* m, void* info1, void* info2);
void check_carry_begin(mob* m, void* info1, void* info2);
void check_carry_stop(mob* m, void* info1, void* info2);
void set_next_target(mob* m, void* info1, void* info2);
void start_being_delivered(mob* m, void* info1, void* info2);
void handle_delivery(mob* m, void* info1, void* info2);
}

#endif //ifndef GEN_MOB_FSM_INCLUDED
