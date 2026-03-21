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
void beAttacked(ScriptVM* scriptVM, void* info1, void* info2);
void carryBecomeStuck(ScriptVM* scriptVM, void* info1, void* info2);
void carryBeginMove(ScriptVM* scriptVM, void* info1, void* info2);
void carryGetPath(ScriptVM* scriptVM, void* info1, void* info2);
void carryReachDestination(ScriptVM* scriptVM, void* info1, void* info2);
void carryStopBeingStuck(ScriptVM* scriptVM, void* info1, void* info2);
void carryStopMove(ScriptVM* scriptVM, void* info1, void* info2);
void fallDownPit(ScriptVM* scriptVM, void* info1, void* info2);
void goToDyingState(ScriptVM* scriptVM, void* info1, void* info2);
void handleCarrierAdded(ScriptVM* scriptVM, void* info1, void* info2);
void handleCarrierRemoved(ScriptVM* scriptVM, void* info1, void* info2);
void handleDelivery(ScriptVM* scriptVM, void* info1, void* info2);
void loseMomentum(ScriptVM* scriptVM, void* info1, void* info2);
void startBeingDelivered(ScriptVM* scriptVM, void* info1, void* info2);
void touchHazard(ScriptVM* scriptVM, void* info1, void* info2);
void touchSpray(ScriptVM* scriptVM, void* info1, void* info2);
}
