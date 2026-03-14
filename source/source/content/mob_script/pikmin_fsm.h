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

void beAttacked(ScriptVM* scriptVM, void* info1, void* info2);
void beCrushed(ScriptVM* scriptVM, void* info1, void* info2);
void beDismissed(ScriptVM* scriptVM, void* info1, void* info2);
void beGrabbedByEnemy(ScriptVM* scriptVM, void* info1, void* info2);
void beGrabbedByFriend(ScriptVM* scriptVM, void* info1, void* info2);
void beReleased(ScriptVM* scriptVM, void* info1, void* info2);
void beThrown(ScriptVM* scriptVM, void* info1, void* info2);
void beThrownAfterPluck(ScriptVM* scriptVM, void* info1, void* info2);
void beThrownByBouncer(ScriptVM* scriptVM, void* info1, void* info2);
void becomeHelpless(ScriptVM* scriptVM, void* info1, void* info2);
void becomeIdle(ScriptVM* scriptVM, void* info1, void* info2);
void becomeSprout(ScriptVM* scriptVM, void* info1, void* info2);
void beginPluck(ScriptVM* scriptVM, void* info1, void* info2);
void called(ScriptVM* scriptVM, void* info1, void* info2);
void calledWhileKnockedDown(ScriptVM* scriptVM, void* info1, void* info2);
void celebrate(ScriptVM* scriptVM, void* info1, void* info2);
void checkBoredomAnimEnd(ScriptVM* scriptVM, void* info1, void* info2);
void checkIncomingAttack(ScriptVM* scriptVM, void* info1, void* info2);
void checkLeaderBump(ScriptVM* scriptVM, void* info1, void* info2);
void checkOutgoingAttack(ScriptVM* scriptVM, void* info1, void* info2);
void checkShakingAnimEnd(ScriptVM* scriptVM, void* info1, void* info2);
void circleOpponent(ScriptVM* scriptVM, void* info1, void* info2);
void clearBoredomData(ScriptVM* scriptVM, void* info1, void* info2);
void clearTimer(ScriptVM* scriptVM, void* info1, void* info2);
void decideAttack(ScriptVM* scriptVM, void* info1, void* info2);
void doImpactBounce(ScriptVM* scriptVM, void* info1, void* info2);
void enterOnion(ScriptVM* scriptVM, void* info1, void* info2);
void fallDownPit(ScriptVM* scriptVM, void* info1, void* info2);
void finishCalledAnim(ScriptVM* scriptVM, void* info1, void* info2);
void finishCarrying(ScriptVM* scriptVM, void* info1, void* info2);
void finishDrinking(ScriptVM* scriptVM, void* info1, void* info2);
void finishDying(ScriptVM* scriptVM, void* info1, void* info2);
void finishGettingUp(ScriptVM* scriptVM, void* info1, void* info2);
void finishMobLanding(ScriptVM* scriptVM, void* info1, void* info2);
void finishPickingUp(ScriptVM* scriptVM, void* info1, void* info2);
void flailToLeader(ScriptVM* scriptVM, void* info1, void* info2);
void forgetCarriableObject(ScriptVM* scriptVM, void* info1, void* info2);
void forgetGroupTask(ScriptVM* scriptVM, void* info1, void* info2);
void forgetTool(ScriptVM* scriptVM, void* info1, void* info2);
void getKnockedBack(ScriptVM* scriptVM, void* info1, void* info2);
void getKnockedDown(ScriptVM* scriptVM, void* info1, void* info2);
void goToCarriableObject(ScriptVM* scriptVM, void* info1, void* info2);
void goToGroupTask(ScriptVM* scriptVM, void* info1, void* info2);
void goToOnion(ScriptVM* scriptVM, void* info1, void* info2);
void goToOpponent(ScriptVM* scriptVM, void* info1, void* info2);
void goToTool(ScriptVM* scriptVM, void* info1, void* info2);
void goingToDismissSpot(ScriptVM* scriptVM, void* info1, void* info2);
void land(ScriptVM* scriptVM, void* info1, void* info2);
void landAfterPluck(ScriptVM* scriptVM, void* info1, void* info2);
void landOnMob(ScriptVM* scriptVM, void* info1, void* info2);
void landOnMobWhileHolding(ScriptVM* scriptVM, void* info1, void* info2);
void landWhileHolding(ScriptVM* scriptVM, void* info1, void* info2);
void leaveOnion(ScriptVM* scriptVM, void* info1, void* info2);
void leftHazard(ScriptVM* scriptVM, void* info1, void* info2);
void loseLatchedMob(ScriptVM* scriptVM, void* info1, void* info2);
void notifyLeaderRelease(ScriptVM* scriptVM, void* info1, void* info2);
void panicNewChase(ScriptVM* scriptVM, void* info1, void* info2);
void prepareToAttack(ScriptVM* scriptVM, void* info1, void* info2);
void reachCarriableObject(ScriptVM* scriptVM, void* info1, void* info2);
void reachDismissSpot(ScriptVM* scriptVM, void* info1, void* info2);
void rechaseOpponent(ScriptVM* scriptVM, void* info1, void* info2);
void releaseTool(ScriptVM* scriptVM, void* info1, void* info2);
void seedLanded(ScriptVM* scriptVM, void* info1, void* info2);
void setBumpLock(ScriptVM* scriptVM, void* info1, void* info2);
void setIdleTaskReach(ScriptVM* scriptVM, void* info1, void* info2);
void setSwarmReach(ScriptVM* scriptVM, void* info1, void* info2);
void sigh(ScriptVM* scriptVM, void* info1, void* info2);
void sproutEvolve(ScriptVM* scriptVM, void* info1, void* info2);
void sproutScheduleEvol(ScriptVM* scriptVM, void* info1, void* info2);
void standStill(ScriptVM* scriptVM, void* info1, void* info2);
void startBoredomAnim(ScriptVM* scriptVM, void* info1, void* info2);
void startChasingLeader(ScriptVM* scriptVM, void* info1, void* info2);
void startDrinking(ScriptVM* scriptVM, void* info1, void* info2);
void startDying(ScriptVM* scriptVM, void* info1, void* info2);
void startFlailing(ScriptVM* scriptVM, void* info1, void* info2);
void startGettingUp(ScriptVM* scriptVM, void* info1, void* info2);
void startImpactLunge(ScriptVM* scriptVM, void* info1, void* info2);
void startKnockedDownDying(ScriptVM* scriptVM, void* info1, void* info2);
void startMobLanding(ScriptVM* scriptVM, void* info1, void* info2);
void startPanicking(ScriptVM* scriptVM, void* info1, void* info2);
void startPickingUp(ScriptVM* scriptVM, void* info1, void* info2);
void startReturning(ScriptVM* scriptVM, void* info1, void* info2);
void startRidingTrack(ScriptVM* scriptVM, void* info1, void* info2);
void startSeedParticles(ScriptVM* scriptVM, void* info1, void* info2);
void stopBeingIdle(ScriptVM* scriptVM, void* info1, void* info2);
void stopBeingThrown(ScriptVM* scriptVM, void* info1, void* info2);
void stopCarrying(ScriptVM* scriptVM, void* info1, void* info2);
void stopInGroup(ScriptVM* scriptVM, void* info1, void* info2);
void tickCarrying(ScriptVM* scriptVM, void* info1, void* info2);
void tickEnteringOnion(ScriptVM* scriptVM, void* info1, void* info2);
void tickGroupTaskWork(ScriptVM* scriptVM, void* info1, void* info2);
void tickTrackRide(ScriptVM* scriptVM, void* info1, void* info2);
void touchedEatHitbox(ScriptVM* scriptVM, void* info1, void* info2);
void touchedHazard(ScriptVM* scriptVM, void* info1, void* info2);
void touchedSpray(ScriptVM* scriptVM, void* info1, void* info2);
void tryHeldItemHotswap(ScriptVM* scriptVM, void* info1, void* info2);
void unlatch(ScriptVM* scriptVM, void* info1, void* info2);
void updateInGroupChasing(ScriptVM* scriptVM, void* info1, void* info2);
void whistledWhileHolding(ScriptVM* scriptVM, void* info1, void* info2);
void whistledWhileRiding(ScriptVM* scriptVM, void* info1, void* info2);
void workOnGroupTask(ScriptVM* scriptVM, void* info1, void* info2);
}
