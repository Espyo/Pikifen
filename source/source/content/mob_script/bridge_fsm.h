/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the bridge's finite-state machine and behavior.
 */
namespace BridgeFsm {
void createFsm(MobType* typ);

void checkHealth(ScriptVM* scriptVM, void* info1, void* info2);
void open(ScriptVM* scriptVM, void* info1, void* info2);
void setAnim(ScriptVM* scriptVM, void* info1, void* info2);
void setup(ScriptVM* scriptVM, void* info1, void* info2);
}
