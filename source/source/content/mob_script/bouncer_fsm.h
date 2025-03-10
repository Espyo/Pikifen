/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bouncer finite state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the bouncer's finite state machine and behavior.
 */
namespace bouncer_fsm {
void create_fsm(MobType* typ);

void handle_mob(Mob* m, void* info1, void* info2);
void set_bouncing_animation(Mob* m, void* info1, void* info2);
void set_idling_animation(Mob* m, void* info1, void* info2);
}
