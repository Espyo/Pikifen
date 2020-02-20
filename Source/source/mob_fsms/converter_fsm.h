/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter finite state machine logic.
 */

#ifndef CONVERTER_FSM_INCLUDED
#define CONVERTER_FSM_INCLUDED

#include "../mob_types/mob_type.h"


/* ----------------------------------------------------------------------------
 * Functions about the converter's finite state machine and behavior.
 */
namespace converter_fsm {
void create_fsm(mob_type* typ);

void become_idle(mob* m, void* info1, void* info2);
void bumped(mob* m, void* info1, void* info2);
void finish_being_bumped(mob* m, void* info1, void* info2);
void finish_dying(mob* m, void* info1, void* info2);
void handle_object_touch(mob* m, void* info1, void* info2);
void handle_pikmin(mob* m, void* info1, void* info2);
void open(mob* m, void* info1, void* info2);
void open_or_die(mob* m, void* info1, void* info2);
void open_or_spit(mob* m, void* info1, void* info2);
void spew(mob* m, void* info1, void* info2);
void start_dying(mob* m, void* info1, void* info2);
};

#endif //ifndef CONVERTER_FSM_INCLUDED
