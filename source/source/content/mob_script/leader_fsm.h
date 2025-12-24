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

void beAttacked(Mob* m, void* info1, void* info2);
void beDismissed(Mob* m, void* info1, void* info2);
void beGrabbedByFriend(Mob* m, void* info1, void* info2);
void beReleased(Mob* m, void* info1, void* info2);
void beThrown(Mob* m, void* info1, void* info2);
void beThrownByBouncer(Mob* m, void* info1, void* info2);
void becomeActive(Mob* m, void* info1, void* info2);
void becomeInactive(Mob* m, void* info1, void* info2);
void called(Mob* m, void* info1, void* info2);
void calledWhileKnockedDown(Mob* m, void* info1, void* info2);
void checkBoredomAnimEnd(Mob* m, void* info1, void* info2);
void checkPunchDamage(Mob* m, void* info1, void* info2);
void clearBoredomData(Mob* m, void* info1, void* info2);
void clearTimer(Mob* m, void* info1, void* info2);
void closeInventory(Mob* m, void* info1, void* info2);
void decidePluckAction(Mob* m, void* info1, void* info2);
void dismiss(Mob* m, void* info1, void* info2);
void doThrow(Mob* m, void* info1, void* info2);
void enterActive(Mob* m, void* info1, void* info2);
void enterIdle(Mob* m, void* info1, void* info2);
void fallAsleep(Mob* m, void* info1, void* info2);
void fallDownPit(Mob* m, void* info1, void* info2);
void finishCalledAnim(Mob* m, void* info1, void* info2);
void finishDrinking(Mob* m, void* info1, void* info2);
void finishGettingUp(Mob* m, void* info1, void* info2);
void finishPluck(Mob* m, void* info1, void* info2);
void getKnockedBack(Mob* m, void* info1, void* info2);
void getKnockedDown(Mob* m, void* info1, void* info2);
void getUpFaster(Mob* m, void* info1, void* info2);
void getKod(Mob* m, void* info1, void* info2);
void goPluck(Mob* m, void* info1, void* info2);
void grabMob(Mob* m, void* info1, void* info2);
void idleOrRejoin(Mob* m, void* info1, void* info2);
void joinGroup(Mob* m, void* info1, void* info2);
void land(Mob* m, void* info1, void* info2);
void leftHazard(Mob* m, void* info1, void* info2);
void loseMomentum(Mob* m, void* info1, void* info2);
void move(Mob* m, void* info1, void* info2);
void notifyPikminRelease(Mob* m, void* info1, void* info2);
void openInventory(Mob* m, void* info1, void* info2);
void punch(Mob* m, void* info1, void* info2);
void queueStopAutoPluck(Mob* m, void* info1, void* info2);
void release(Mob* m, void* info1, void* info2);
void searchSeed(Mob* m, void* info1, void* info2);
void setCorrectStandingAnim(Mob* m, void* info1, void* info2);
void setIsTurningFalse(Mob* m, void* info1, void* info2);
void setIsTurningTrue(Mob* m, void* info1, void* info2);
void setIsWalkingFalse(Mob* m, void* info1, void* info2);
void setIsWalkingTrue(Mob* m, void* info1, void* info2);
void setPainAnim(Mob* m, void* info1, void* info2);
void signalStopAutoPluck(Mob* m, void* info1, void* info2);
void spray(Mob* m, void* info1, void* info2);
void standStill(Mob* m, void* info1, void* info2);
void startBoredomAnim(Mob* m, void* info1, void* info2);
void startChasingLeader(Mob* m, void* info1, void* info2);
void startDrinking(Mob* m, void* info1, void* info2);
void startGettingUp(Mob* m, void* info1, void* info2);
void startGoHere(Mob* m, void* info1, void* info2);
void startPluck(Mob* m, void* info1, void* info2);
void startRidingTrack(Mob* m, void* info1, void* info2);
void startWakingUp(Mob* m, void* info1, void* info2);
void stopAutoPluck(Mob* m, void* info1, void* info2);
void stopBeingThrown(Mob* m, void* info1, void* info2);
void stopGoHere(Mob* m, void* info1, void* info2);
void stopInGroup(Mob* m, void* info1, void* info2);
void stopWhistle(Mob* m, void* info1, void* info2);
void tickActiveState(Mob* m, void* info1, void* info2);
void tickTrackRide(Mob* m, void* info1, void* info2);
void touchedHazard(Mob* m, void* info1, void* info2);
void touchedSpray(Mob* m, void* info1, void* info2);
void trySetCorrectStandingAnim(Mob* m, void* info1, void* info2);
void updateInGroupChasing(Mob* m, void* info1, void* info2);
void whistle(Mob* m, void* info1, void* info2);
void whistledWhileRiding(Mob* m, void* info1, void* info2);
}
