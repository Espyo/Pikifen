/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion finite state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the Onion's finite state machine and behavior.
 */
namespace onion_fsm {
void create_fsm(MobType* typ);

void check_start_generating(Mob* m, void* info1, void* info2);
void check_stop_generating(Mob* m, void* info1, void* info2);
void receive_mob(Mob* m, void* info1, void* info2);
void start_generating(Mob* m, void* info1, void* info2);
void start_idling(Mob* m, void* info1, void* info2);
void stop_generating(Mob* m, void* info1, void* info2);
}
