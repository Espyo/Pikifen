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

void becomeIdle(Mob* m, void* info1, void* info2);
void bumped(Mob* m, void* info1, void* info2);
void finishBeingBumped(Mob* m, void* info1, void* info2);
void finishDying(Mob* m, void* info1, void* info2);
void handleObjectTouch(Mob* m, void* info1, void* info2);
void handlePikmin(Mob* m, void* info1, void* info2);
void open(Mob* m, void* info1, void* info2);
void openOrDie(Mob* m, void* info1, void* info2);
void openOrSpit(Mob* m, void* info1, void* info2);
void spit(Mob* m, void* info1, void* info2);
void startDying(Mob* m, void* info1, void* info2);
}
