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
void create_fsm(MobType* typ);

void land(Mob* m, void* info1, void* info2);
void on_touched(Mob* m, void* info1, void* info2);
void set_bumped_anim(Mob* m, void* info1, void* info2);
void set_falling_anim(Mob* m, void* info1, void* info2);
void set_idling_anim(Mob* m, void* info1, void* info2);
}
