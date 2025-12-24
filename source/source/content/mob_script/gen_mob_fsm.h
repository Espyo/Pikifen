/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the generic mob finite-state machine logic.
 */

#pragma once

#include "../mob/mob.h"


/**
 * @brief Functions about the generic mob's finite-state machine and behavior.
 */
namespace GenMobFsm {
void beAttacked(Mob* m, void* info1, void* info2);
void carryBecomeStuck(Mob* m, void* info1, void* info2);
void carryBeginMove(Mob* m, void* info1, void* info2);
void carryGetPath(Mob* m, void* info1, void* info2);
void carryReachDestination(Mob* m, void* info1, void* info2);
void carryStopBeingStuck(Mob* m, void* info1, void* info2);
void carryStopMove(Mob* m, void* info1, void* info2);
void fallDownPit(Mob* m, void* info1, void* info2);
void goToDyingState(Mob* m, void* info1, void* info2);
void handleCarrierAdded(Mob* m, void* info1, void* info2);
void handleCarrierRemoved(Mob* m, void* info1, void* info2);
void handleDelivery(Mob* m, void* info1, void* info2);
void loseMomentum(Mob* m, void* info1, void* info2);
void startBeingDelivered(Mob* m, void* info1, void* info2);
void touchHazard(Mob* m, void* info1, void* info2);
void touchSpray(Mob* m, void* info1, void* info2);
}
