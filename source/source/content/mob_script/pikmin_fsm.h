/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin finite-state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the Pikmin's finite-state machine and behavior.
 */
namespace PikminFsm {
void createFsm(MobType* typ);

void beAttacked(Fsm* fsm, void* info1, void* info2);
void beCrushed(Fsm* fsm, void* info1, void* info2);
void beDismissed(Fsm* fsm, void* info1, void* info2);
void beGrabbedByEnemy(Fsm* fsm, void* info1, void* info2);
void beGrabbedByFriend(Fsm* fsm, void* info1, void* info2);
void beReleased(Fsm* fsm, void* info1, void* info2);
void beThrown(Fsm* fsm, void* info1, void* info2);
void beThrownAfterPluck(Fsm* fsm, void* info1, void* info2);
void beThrownByBouncer(Fsm* fsm, void* info1, void* info2);
void becomeHelpless(Fsm* fsm, void* info1, void* info2);
void becomeIdle(Fsm* fsm, void* info1, void* info2);
void becomeSprout(Fsm* fsm, void* info1, void* info2);
void beginPluck(Fsm* fsm, void* info1, void* info2);
void called(Fsm* fsm, void* info1, void* info2);
void calledWhileKnockedDown(Fsm* fsm, void* info1, void* info2);
void celebrate(Fsm* fsm, void* info1, void* info2);
void checkBoredomAnimEnd(Fsm* fsm, void* info1, void* info2);
void checkIncomingAttack(Fsm* fsm, void* info1, void* info2);
void checkLeaderBump(Fsm* fsm, void* info1, void* info2);
void checkOutgoingAttack(Fsm* fsm, void* info1, void* info2);
void checkShakingAnimEnd(Fsm* fsm, void* info1, void* info2);
void circleOpponent(Fsm* fsm, void* info1, void* info2);
void clearBoredomData(Fsm* fsm, void* info1, void* info2);
void clearTimer(Fsm* fsm, void* info1, void* info2);
void decideAttack(Fsm* fsm, void* info1, void* info2);
void doImpactBounce(Fsm* fsm, void* info1, void* info2);
void enterOnion(Fsm* fsm, void* info1, void* info2);
void fallDownPit(Fsm* fsm, void* info1, void* info2);
void finishCalledAnim(Fsm* fsm, void* info1, void* info2);
void finishCarrying(Fsm* fsm, void* info1, void* info2);
void finishDrinking(Fsm* fsm, void* info1, void* info2);
void finishDying(Fsm* fsm, void* info1, void* info2);
void finishGettingUp(Fsm* fsm, void* info1, void* info2);
void finishMobLanding(Fsm* fsm, void* info1, void* info2);
void finishPickingUp(Fsm* fsm, void* info1, void* info2);
void flailToLeader(Fsm* fsm, void* info1, void* info2);
void forgetCarriableObject(Fsm* fsm, void* info1, void* info2);
void forgetGroupTask(Fsm* fsm, void* info1, void* info2);
void forgetTool(Fsm* fsm, void* info1, void* info2);
void getKnockedBack(Fsm* fsm, void* info1, void* info2);
void getKnockedDown(Fsm* fsm, void* info1, void* info2);
void goToCarriableObject(Fsm* fsm, void* info1, void* info2);
void goToGroupTask(Fsm* fsm, void* info1, void* info2);
void goToOnion(Fsm* fsm, void* info1, void* info2);
void goToOpponent(Fsm* fsm, void* info1, void* info2);
void goToTool(Fsm* fsm, void* info1, void* info2);
void goingToDismissSpot(Fsm* fsm, void* info1, void* info2);
void land(Fsm* fsm, void* info1, void* info2);
void landAfterPluck(Fsm* fsm, void* info1, void* info2);
void landOnMob(Fsm* fsm, void* info1, void* info2);
void landOnMobWhileHolding(Fsm* fsm, void* info1, void* info2);
void landWhileHolding(Fsm* fsm, void* info1, void* info2);
void leaveOnion(Fsm* fsm, void* info1, void* info2);
void leftHazard(Fsm* fsm, void* info1, void* info2);
void loseLatchedMob(Fsm* fsm, void* info1, void* info2);
void notifyLeaderRelease(Fsm* fsm, void* info1, void* info2);
void panicNewChase(Fsm* fsm, void* info1, void* info2);
void prepareToAttack(Fsm* fsm, void* info1, void* info2);
void reachCarriableObject(Fsm* fsm, void* info1, void* info2);
void reachDismissSpot(Fsm* fsm, void* info1, void* info2);
void rechaseOpponent(Fsm* fsm, void* info1, void* info2);
void releaseTool(Fsm* fsm, void* info1, void* info2);
void seedLanded(Fsm* fsm, void* info1, void* info2);
void setBumpLock(Fsm* fsm, void* info1, void* info2);
void setIdleTaskReach(Fsm* fsm, void* info1, void* info2);
void setSwarmReach(Fsm* fsm, void* info1, void* info2);
void sigh(Fsm* fsm, void* info1, void* info2);
void sproutEvolve(Fsm* fsm, void* info1, void* info2);
void sproutScheduleEvol(Fsm* fsm, void* info1, void* info2);
void standStill(Fsm* fsm, void* info1, void* info2);
void startBoredomAnim(Fsm* fsm, void* info1, void* info2);
void startChasingLeader(Fsm* fsm, void* info1, void* info2);
void startDrinking(Fsm* fsm, void* info1, void* info2);
void startDying(Fsm* fsm, void* info1, void* info2);
void startFlailing(Fsm* fsm, void* info1, void* info2);
void startGettingUp(Fsm* fsm, void* info1, void* info2);
void startImpactLunge(Fsm* fsm, void* info1, void* info2);
void startKnockedDownDying(Fsm* fsm, void* info1, void* info2);
void startMobLanding(Fsm* fsm, void* info1, void* info2);
void startPanicking(Fsm* fsm, void* info1, void* info2);
void startPickingUp(Fsm* fsm, void* info1, void* info2);
void startReturning(Fsm* fsm, void* info1, void* info2);
void startRidingTrack(Fsm* fsm, void* info1, void* info2);
void startSeedParticles(Fsm* fsm, void* info1, void* info2);
void stopBeingIdle(Fsm* fsm, void* info1, void* info2);
void stopBeingThrown(Fsm* fsm, void* info1, void* info2);
void stopCarrying(Fsm* fsm, void* info1, void* info2);
void stopInGroup(Fsm* fsm, void* info1, void* info2);
void tickCarrying(Fsm* fsm, void* info1, void* info2);
void tickEnteringOnion(Fsm* fsm, void* info1, void* info2);
void tickGroupTaskWork(Fsm* fsm, void* info1, void* info2);
void tickTrackRide(Fsm* fsm, void* info1, void* info2);
void touchedEatHitbox(Fsm* fsm, void* info1, void* info2);
void touchedHazard(Fsm* fsm, void* info1, void* info2);
void touchedSpray(Fsm* fsm, void* info1, void* info2);
void tryHeldItemHotswap(Fsm* fsm, void* info1, void* info2);
void unlatch(Fsm* fsm, void* info1, void* info2);
void updateInGroupChasing(Fsm* fsm, void* info1, void* info2);
void whistledWhileHolding(Fsm* fsm, void* info1, void* info2);
void whistledWhileRiding(Fsm* fsm, void* info1, void* info2);
void workOnGroupTask(Fsm* fsm, void* info1, void* info2);
}
