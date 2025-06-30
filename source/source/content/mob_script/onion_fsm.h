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
namespace OnionFsm {
void createFsm(MobType* typ);

void checkStartGenerating(Mob* m, void* info1, void* info2);
void checkStopGenerating(Mob* m, void* info1, void* info2);
void receiveMob(Mob* m, void* info1, void* info2);
void startDelivery(Mob* m, void* info1, void* info2);
void startGenerating(Mob* m, void* info1, void* info2);
void startIdling(Mob* m, void* info1, void* info2);
void stopGenerating(Mob* m, void* info1, void* info2);
}
