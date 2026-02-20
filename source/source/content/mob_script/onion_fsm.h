/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the Onion's finite-state machine and behavior.
 */
namespace OnionFsm {
void createFsm(MobType* typ);

void checkStartGenerating(Fsm* fsm, void* info1, void* info2);
void checkStopGenerating(Fsm* fsm, void* info1, void* info2);
void receiveMob(Fsm* fsm, void* info1, void* info2);
void startDelivery(Fsm* fsm, void* info1, void* info2);
void startGenerating(Fsm* fsm, void* info1, void* info2);
void startIdling(Fsm* fsm, void* info1, void* info2);
void stopGenerating(Fsm* fsm, void* info1, void* info2);
}
