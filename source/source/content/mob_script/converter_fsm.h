/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the converter's finite-state machine and behavior.
 */
namespace ConverterFsm {
void createFsm(MobType* typ);

void becomeIdle(Fsm* fsm, void* info1, void* info2);
void bumped(Fsm* fsm, void* info1, void* info2);
void finishBeingBumped(Fsm* fsm, void* info1, void* info2);
void finishDying(Fsm* fsm, void* info1, void* info2);
void handleObjectTouch(Fsm* fsm, void* info1, void* info2);
void handlePikmin(Fsm* fsm, void* info1, void* info2);
void open(Fsm* fsm, void* info1, void* info2);
void openOrDie(Fsm* fsm, void* info1, void* info2);
void openOrSpit(Fsm* fsm, void* info1, void* info2);
void spit(Fsm* fsm, void* info1, void* info2);
void startDying(Fsm* fsm, void* info1, void* info2);
}
