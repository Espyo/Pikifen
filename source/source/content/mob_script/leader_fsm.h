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

void beAttacked(Fsm* fsm, void* info1, void* info2);
void beDismissed(Fsm* fsm, void* info1, void* info2);
void beGrabbedByFriend(Fsm* fsm, void* info1, void* info2);
void beReleased(Fsm* fsm, void* info1, void* info2);
void beThrown(Fsm* fsm, void* info1, void* info2);
void beThrownByBouncer(Fsm* fsm, void* info1, void* info2);
void becomeActive(Fsm* fsm, void* info1, void* info2);
void becomeInactive(Fsm* fsm, void* info1, void* info2);
void called(Fsm* fsm, void* info1, void* info2);
void calledWhileKnockedDown(Fsm* fsm, void* info1, void* info2);
void checkBoredomAnimEnd(Fsm* fsm, void* info1, void* info2);
void checkPunchDamage(Fsm* fsm, void* info1, void* info2);
void clearBoredomData(Fsm* fsm, void* info1, void* info2);
void clearTimer(Fsm* fsm, void* info1, void* info2);
void closeInventory(Fsm* fsm, void* info1, void* info2);
void decidePluckAction(Fsm* fsm, void* info1, void* info2);
void dismiss(Fsm* fsm, void* info1, void* info2);
void doThrow(Fsm* fsm, void* info1, void* info2);
void enterActive(Fsm* fsm, void* info1, void* info2);
void enterIdle(Fsm* fsm, void* info1, void* info2);
void fallAsleep(Fsm* fsm, void* info1, void* info2);
void fallDownPit(Fsm* fsm, void* info1, void* info2);
void finishCalledAnim(Fsm* fsm, void* info1, void* info2);
void finishDrinking(Fsm* fsm, void* info1, void* info2);
void finishGettingUp(Fsm* fsm, void* info1, void* info2);
void finishPluck(Fsm* fsm, void* info1, void* info2);
void finishShaking(Fsm* fsm, void* info1, void* info2);
void getKnockedBack(Fsm* fsm, void* info1, void* info2);
void getKnockedDown(Fsm* fsm, void* info1, void* info2);
void getUpFaster(Fsm* fsm, void* info1, void* info2);
void getKod(Fsm* fsm, void* info1, void* info2);
void goPluck(Fsm* fsm, void* info1, void* info2);
void grabMob(Fsm* fsm, void* info1, void* info2);
void idleOrRejoin(Fsm* fsm, void* info1, void* info2);
void joinGroup(Fsm* fsm, void* info1, void* info2);
void land(Fsm* fsm, void* info1, void* info2);
void leftHazard(Fsm* fsm, void* info1, void* info2);
void loseMomentum(Fsm* fsm, void* info1, void* info2);
void move(Fsm* fsm, void* info1, void* info2);
void notifyPikminRelease(Fsm* fsm, void* info1, void* info2);
void openInventory(Fsm* fsm, void* info1, void* info2);
void punch(Fsm* fsm, void* info1, void* info2);
void queueStopAutoPluck(Fsm* fsm, void* info1, void* info2);
void release(Fsm* fsm, void* info1, void* info2);
void searchSeed(Fsm* fsm, void* info1, void* info2);
void setCorrectStandingAnim(Fsm* fsm, void* info1, void* info2);
void setIsTurningFalse(Fsm* fsm, void* info1, void* info2);
void setIsTurningTrue(Fsm* fsm, void* info1, void* info2);
void setIsWalkingFalse(Fsm* fsm, void* info1, void* info2);
void setIsWalkingTrue(Fsm* fsm, void* info1, void* info2);
void setPainAnim(Fsm* fsm, void* info1, void* info2);
void shake(Fsm* fsm, void* info1, void* info2);
void signalStopAutoPluck(Fsm* fsm, void* info1, void* info2);
void spray(Fsm* fsm, void* info1, void* info2);
void standStill(Fsm* fsm, void* info1, void* info2);
void startBoredomAnim(Fsm* fsm, void* info1, void* info2);
void startChasingLeader(Fsm* fsm, void* info1, void* info2);
void startDrinking(Fsm* fsm, void* info1, void* info2);
void startGettingUp(Fsm* fsm, void* info1, void* info2);
void startGoHere(Fsm* fsm, void* info1, void* info2);
void startPluck(Fsm* fsm, void* info1, void* info2);
void startRidingTrack(Fsm* fsm, void* info1, void* info2);
void startWakingUp(Fsm* fsm, void* info1, void* info2);
void stopAutoPluck(Fsm* fsm, void* info1, void* info2);
void stopBeingThrown(Fsm* fsm, void* info1, void* info2);
void stopGoHere(Fsm* fsm, void* info1, void* info2);
void stopInGroup(Fsm* fsm, void* info1, void* info2);
void stopWhistle(Fsm* fsm, void* info1, void* info2);
void tickActiveState(Fsm* fsm, void* info1, void* info2);
void tickTrackRide(Fsm* fsm, void* info1, void* info2);
void touchedHazard(Fsm* fsm, void* info1, void* info2);
void touchedSpray(Fsm* fsm, void* info1, void* info2);
void trySetCorrectStandingAnim(Fsm* fsm, void* info1, void* info2);
void updateInGroupChasing(Fsm* fsm, void* info1, void* info2);
void whistle(Fsm* fsm, void* info1, void* info2);
void whistledWhileRiding(Fsm* fsm, void* info1, void* info2);
}
