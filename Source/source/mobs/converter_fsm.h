/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter finite state machine logic.
 */

#ifndef CONVERTER_FSM_INCLUDED
#define CONVERTER_FSM_INCLUDED

#include "mob.h"
#include "../mob_types/mob_type.h"


/* ----------------------------------------------------------------------------
 * Functions about the converter's finite state machine and behavior.
 */
namespace converter_fsm {
void create_fsm(mob_type* typ);

};

#endif //ifndef CONVERTER_FSM_INCLUDED
