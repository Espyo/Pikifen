/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bouncer finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the bouncer's finite-state machine and behavior.
 */
namespace BouncerFsm {
void createFsm(MobType* typ);

void handleMob(Mob* m, void* info1, void* info2);
void setBouncingAnimation(Mob* m, void* info1, void* info2);
void setIdlingAnimation(Mob* m, void* info1, void* info2);
}
