/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the generic mob finite state machine logic.
 */

#ifndef GEN_MOB_FSM_INCLUDED
#define GEN_MOB_FSM_INCLUDED

#include "../mobs/mob.h"

/* ----------------------------------------------------------------------------
 * Functions about the generic mob's finite state machine and behavior.
 */
namespace gen_mob_fsm {
void be_attacked(mob* m, void* info1, void* info2);
void carry_become_stuck(mob* m, void* info1, void* info2);
void carry_begin_move(mob* m, void* info1, void* info2);
void carry_get_path(mob* m, void* info1, void* info2);
void carry_reach_destination(mob* m, void* info1, void* info2);
void carry_stop_being_stuck(mob* m, void* info1, void* info2);
void carry_stop_move(mob* m, void* info1, void* info2);
void die(mob* m, void* info1, void* info2);
void fall_down_pit(mob* m, void* info1, void* info2);
void handle_carrier_added(mob* m, void* info1, void* info2);
void handle_carrier_removed(mob* m, void* info1, void* info2);
void handle_delivery(mob* m, void* info1, void* info2);
void start_being_delivered(mob* m, void* info1, void* info2);
void touch_hazard(mob* m, void* info1, void* info2);
void touch_spray(mob* m, void* info1, void* info2);
}

#endif //ifndef GEN_MOB_FSM_INCLUDED
