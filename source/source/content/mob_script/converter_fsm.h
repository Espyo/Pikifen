/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the converter's finite-state machine and behavior.
 */
namespace ConverterFsm {
void createFsm(MobType* typ);

void becomeIdle(ScriptVM* scriptVM, void* info1, void* info2);
void bumped(ScriptVM* scriptVM, void* info1, void* info2);
void finishBeingBumped(ScriptVM* scriptVM, void* info1, void* info2);
void finishDying(ScriptVM* scriptVM, void* info1, void* info2);
void handleObjectTouch(ScriptVM* scriptVM, void* info1, void* info2);
void handlePikmin(ScriptVM* scriptVM, void* info1, void* info2);
void open(ScriptVM* scriptVM, void* info1, void* info2);
void openOrDie(ScriptVM* scriptVM, void* info1, void* info2);
void openOrSpit(ScriptVM* scriptVM, void* info1, void* info2);
void spit(ScriptVM* scriptVM, void* info1, void* info2);
void startDying(ScriptVM* scriptVM, void* info1, void* info2);
}
