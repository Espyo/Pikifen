/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the leader's finite-state machine and behavior.
 */
namespace LeaderFsm {
void createFsm(MobType* typ);

void beAttacked(ScriptVM* scriptVM, void* info1, void* info2);
void beDismissed(ScriptVM* scriptVM, void* info1, void* info2);
void beGrabbedByFriend(ScriptVM* scriptVM, void* info1, void* info2);
void beReleased(ScriptVM* scriptVM, void* info1, void* info2);
void beThrown(ScriptVM* scriptVM, void* info1, void* info2);
void beThrownByBouncer(ScriptVM* scriptVM, void* info1, void* info2);
void becomeActive(ScriptVM* scriptVM, void* info1, void* info2);
void becomeInactive(ScriptVM* scriptVM, void* info1, void* info2);
void called(ScriptVM* scriptVM, void* info1, void* info2);
void calledWhileKnockedDown(ScriptVM* scriptVM, void* info1, void* info2);
void checkBoredomAnimEnd(ScriptVM* scriptVM, void* info1, void* info2);
void checkPunchDamage(ScriptVM* scriptVM, void* info1, void* info2);
void clearBoredomData(ScriptVM* scriptVM, void* info1, void* info2);
void clearTimer(ScriptVM* scriptVM, void* info1, void* info2);
void closeInventory(ScriptVM* scriptVM, void* info1, void* info2);
void decidePluckAction(ScriptVM* scriptVM, void* info1, void* info2);
void dismiss(ScriptVM* scriptVM, void* info1, void* info2);
void doThrow(ScriptVM* scriptVM, void* info1, void* info2);
void enterActive(ScriptVM* scriptVM, void* info1, void* info2);
void enterIdle(ScriptVM* scriptVM, void* info1, void* info2);
void fallAsleep(ScriptVM* scriptVM, void* info1, void* info2);
void fallDownPit(ScriptVM* scriptVM, void* info1, void* info2);
void finishCalledAnim(ScriptVM* scriptVM, void* info1, void* info2);
void finishDrinking(ScriptVM* scriptVM, void* info1, void* info2);
void finishGettingUp(ScriptVM* scriptVM, void* info1, void* info2);
void finishPluck(ScriptVM* scriptVM, void* info1, void* info2);
void finishShaking(ScriptVM* scriptVM, void* info1, void* info2);
void getKnockedBack(ScriptVM* scriptVM, void* info1, void* info2);
void getKnockedDown(ScriptVM* scriptVM, void* info1, void* info2);
void getUpFaster(ScriptVM* scriptVM, void* info1, void* info2);
void getKod(ScriptVM* scriptVM, void* info1, void* info2);
void goPluck(ScriptVM* scriptVM, void* info1, void* info2);
void grabMob(ScriptVM* scriptVM, void* info1, void* info2);
void idleOrRejoin(ScriptVM* scriptVM, void* info1, void* info2);
void joinGroup(ScriptVM* scriptVM, void* info1, void* info2);
void land(ScriptVM* scriptVM, void* info1, void* info2);
void leftHazard(ScriptVM* scriptVM, void* info1, void* info2);
void loseMomentum(ScriptVM* scriptVM, void* info1, void* info2);
void move(ScriptVM* scriptVM, void* info1, void* info2);
void notifyPikminRelease(ScriptVM* scriptVM, void* info1, void* info2);
void openInventory(ScriptVM* scriptVM, void* info1, void* info2);
void punch(ScriptVM* scriptVM, void* info1, void* info2);
void queueStopAutoPluck(ScriptVM* scriptVM, void* info1, void* info2);
void release(ScriptVM* scriptVM, void* info1, void* info2);
void searchSeed(ScriptVM* scriptVM, void* info1, void* info2);
void setCorrectStandingAnim(ScriptVM* scriptVM, void* info1, void* info2);
void setIsTurningFalse(ScriptVM* scriptVM, void* info1, void* info2);
void setIsTurningTrue(ScriptVM* scriptVM, void* info1, void* info2);
void setIsWalkingFalse(ScriptVM* scriptVM, void* info1, void* info2);
void setIsWalkingTrue(ScriptVM* scriptVM, void* info1, void* info2);
void setPainAnim(ScriptVM* scriptVM, void* info1, void* info2);
void shake(ScriptVM* scriptVM, void* info1, void* info2);
void signalStopAutoPluck(ScriptVM* scriptVM, void* info1, void* info2);
void spray(ScriptVM* scriptVM, void* info1, void* info2);
void standStill(ScriptVM* scriptVM, void* info1, void* info2);
void startBoredomAnim(ScriptVM* scriptVM, void* info1, void* info2);
void startChasingLeader(ScriptVM* scriptVM, void* info1, void* info2);
void startDrinking(ScriptVM* scriptVM, void* info1, void* info2);
void startGettingUp(ScriptVM* scriptVM, void* info1, void* info2);
void startGoHere(ScriptVM* scriptVM, void* info1, void* info2);
void startPluck(ScriptVM* scriptVM, void* info1, void* info2);
void startRidingTrack(ScriptVM* scriptVM, void* info1, void* info2);
void startWakingUp(ScriptVM* scriptVM, void* info1, void* info2);
void stopAutoPluck(ScriptVM* scriptVM, void* info1, void* info2);
void stopBeingThrown(ScriptVM* scriptVM, void* info1, void* info2);
void stopGoHere(ScriptVM* scriptVM, void* info1, void* info2);
void stopInGroup(ScriptVM* scriptVM, void* info1, void* info2);
void stopWhistle(ScriptVM* scriptVM, void* info1, void* info2);
void tickActiveState(ScriptVM* scriptVM, void* info1, void* info2);
void tickTrackRide(ScriptVM* scriptVM, void* info1, void* info2);
void touchedHazard(ScriptVM* scriptVM, void* info1, void* info2);
void touchedSpray(ScriptVM* scriptVM, void* info1, void* info2);
void trySetCorrectStandingAnim(ScriptVM* scriptVM, void* info1, void* info2);
void updateInGroupChasing(ScriptVM* scriptVM, void* info1, void* info2);
void whistle(ScriptVM* scriptVM, void* info1, void* info2);
void whistledWhileRiding(ScriptVM* scriptVM, void* info1, void* info2);
}
