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

#include "../mob_types/mob_type.h"


/**
 * @brief Functions about the Onion's finite state machine and behavior.
 */
namespace onion_fsm {
void create_fsm(mob_type* typ);

void check_start_generating(mob* m, void* info1, void* info2);
void check_stop_generating(mob* m, void* info1, void* info2);
void receive_mob(mob* m, void* info1, void* info2);
void start_generating(mob* m, void* info1, void* info2);
void start_idling(mob* m, void* info1, void* info2);
void stop_generating(mob* m, void* info1, void* info2);
}
