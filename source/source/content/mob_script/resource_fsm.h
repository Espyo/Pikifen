/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the resource finite state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the resource's finite state machine and behavior.
 */
namespace resource_fsm {
void create_fsm(MobType* typ);

void handle_delivery(Mob* m, void* info1, void* info2);
void handle_dropped(Mob* m, void* info1, void* info2);
void handle_reach_destination(Mob* m, void* info1, void* info2);
void handle_start_moving(Mob* m, void* info1, void* info2);
void lose_momentum(Mob* m, void* info1, void* info2);
void start_being_delivered(Mob* m, void* info1, void* info2);
void start_waiting(Mob* m, void* info1, void* info2);
void vanish(Mob* m, void* info1, void* info2);
}
