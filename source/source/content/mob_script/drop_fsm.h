/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drop finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the drop's finite-state machine and behavior.
 */
namespace DropFsm {
void createFsm(MobType* typ);

void land(ScriptVM* scriptVM, void* info1, void* info2);
void onTouched(ScriptVM* scriptVM, void* info1, void* info2);
void setBumpedAnim(ScriptVM* scriptVM, void* info1, void* info2);
void setFallingAnim(ScriptVM* scriptVM, void* info1, void* info2);
void setIdlingAnim(ScriptVM* scriptVM, void* info1, void* info2);
}
