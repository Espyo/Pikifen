/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the decoration finite state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the decoration's finite state machine and behavior.
 */
namespace DecorationFsm {
void createFsm(MobType* typ);

void beBumped(Mob* m, void* info1, void* info2);
void becomeIdle(Mob* m, void* info1, void* info2);
void checkBump(Mob* m, void* info1, void* info2);
}
