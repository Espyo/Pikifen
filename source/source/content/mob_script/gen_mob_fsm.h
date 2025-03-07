/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the generic mob finite state machine logic.
 */

#pragma once

#include "../mob/mob.h"


/**
 * @brief Functions about the generic mob's finite state machine and behavior.
 */
namespace gen_mob_fsm {
void be_attacked(Mob* m, void* info1, void* info2);
void carry_become_stuck(Mob* m, void* info1, void* info2);
void carry_begin_move(Mob* m, void* info1, void* info2);
void carry_get_path(Mob* m, void* info1, void* info2);
void carry_reach_destination(Mob* m, void* info1, void* info2);
void carry_stop_being_stuck(Mob* m, void* info1, void* info2);
void carry_stop_move(Mob* m, void* info1, void* info2);
void fall_down_pit(Mob* m, void* info1, void* info2);
void go_to_dying_state(Mob* m, void* info1, void* info2);
void handle_carrier_added(Mob* m, void* info1, void* info2);
void handle_carrier_removed(Mob* m, void* info1, void* info2);
void handle_delivery(Mob* m, void* info1, void* info2);
void lose_momentum(Mob* m, void* info1, void* info2);
void start_being_delivered(Mob* m, void* info1, void* info2);
void touch_hazard(Mob* m, void* info1, void* info2);
void touch_spray(Mob* m, void* info1, void* info2);
}
