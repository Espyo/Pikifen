/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter finite state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the converter's finite state machine and behavior.
 */
namespace converter_fsm {
void create_fsm(MobType* typ);

void become_idle(Mob* m, void* info1, void* info2);
void bumped(Mob* m, void* info1, void* info2);
void finish_being_bumped(Mob* m, void* info1, void* info2);
void finish_dying(Mob* m, void* info1, void* info2);
void handle_object_touch(Mob* m, void* info1, void* info2);
void handle_pikmin(Mob* m, void* info1, void* info2);
void open(Mob* m, void* info1, void* info2);
void open_or_die(Mob* m, void* info1, void* info2);
void open_or_spit(Mob* m, void* info1, void* info2);
void spew(Mob* m, void* info1, void* info2);
void start_dying(Mob* m, void* info1, void* info2);
}
