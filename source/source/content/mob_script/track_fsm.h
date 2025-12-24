/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the track finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the track's finite-state machine and behavior.
 */
namespace TrackFsm {
void createFsm(MobType* typ);

void onTouched(Mob* m, void* info1, void* info2);
void spawn(Mob* m, void* info1, void* info2);
}
