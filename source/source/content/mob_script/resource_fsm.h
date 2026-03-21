/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the resource finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the resource's finite-state machine and behavior.
 */
namespace ResourceFsm {
void createFsm(MobType* typ);

void handleDelivery(ScriptVM* scriptVM, void* info1, void* info2);
void handleDropped(ScriptVM* scriptVM, void* info1, void* info2);
void handleReachDestination(ScriptVM* scriptVM, void* info1, void* info2);
void handleStartMoving(ScriptVM* scriptVM, void* info1, void* info2);
void loseMomentum(ScriptVM* scriptVM, void* info1, void* info2);
void startBeingDelivered(ScriptVM* scriptVM, void* info1, void* info2);
void startWaiting(ScriptVM* scriptVM, void* info1, void* info2);
void vanish(ScriptVM* scriptVM, void* info1, void* info2);
}
