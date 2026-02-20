/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the ship's finite-state machine and behavior.
 */
namespace ShipFsm {
void createFsm(MobType* typ);

void receiveMob(Fsm* fsm, void* info1, void* info2);
void setAnim(Fsm* fsm, void* info1, void* info2);
void startDelivery(Fsm* fsm, void* info1, void* info2);
}
