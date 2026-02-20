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

void handleDelivery(Fsm* fsm, void* info1, void* info2);
void handleDropped(Fsm* fsm, void* info1, void* info2);
void handleReachDestination(Fsm* fsm, void* info1, void* info2);
void handleStartMoving(Fsm* fsm, void* info1, void* info2);
void loseMomentum(Fsm* fsm, void* info1, void* info2);
void startBeingDelivered(Fsm* fsm, void* info1, void* info2);
void startWaiting(Fsm* fsm, void* info1, void* info2);
void vanish(Fsm* fsm, void* info1, void* info2);
}
