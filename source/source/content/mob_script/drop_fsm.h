/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drop finite state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the drop's finite state machine and behavior.
 */
namespace drop_fsm {
void createFsm(MobType* typ);

void land(Mob* m, void* info1, void* info2);
void onTouched(Mob* m, void* info1, void* info2);
void setBumpedAnim(Mob* m, void* info1, void* info2);
void setFallingAnim(Mob* m, void* info1, void* info2);
void setIdlingAnim(Mob* m, void* info1, void* info2);
}
