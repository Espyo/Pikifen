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

void beAttacked(Mob* m, void* info1, void* info2);
void beCrushed(Mob* m, void* info1, void* info2);
void beDismissed(Mob* m, void* info1, void* info2);
void beGrabbedByEnemy(Mob* m, void* info1, void* info2);
void beGrabbedByFriend(Mob* m, void* info1, void* info2);
void beReleased(Mob* m, void* info1, void* info2);
void beThrown(Mob* m, void* info1, void* info2);
void beThrownAfterPluck(Mob* m, void* info1, void* info2);
void beThrownByBouncer(Mob* m, void* info1, void* info2);
void becomeHelpless(Mob* m, void* info1, void* info2);
void becomeIdle(Mob* m, void* info1, void* info2);
void becomeSprout(Mob* m, void* info1, void* info2);
void beginPluck(Mob* m, void* info1, void* info2);
void called(Mob* m, void* info1, void* info2);
void calledWhileKnockedDown(Mob* m, void* info1, void* info2);
void celebrate(Mob* m, void* info1, void* info2);
void checkBoredomAnimEnd(Mob* m, void* info1, void* info2);
void checkIncomingAttack(Mob* m, void* info1, void* info2);
void checkLeaderBump(Mob* m, void* info1, void* info2);
void checkOutgoingAttack(Mob* m, void* info1, void* info2);
void checkShakingAnimEnd(Mob* m, void* info1, void* info2);
void circleOpponent(Mob* m, void* info1, void* info2);
void clearBoredomData(Mob* m, void* info1, void* info2);
void clearTimer(Mob* m, void* info1, void* info2);
void decideAttack(Mob* m, void* info1, void* info2);
void doImpactBounce(Mob* m, void* info1, void* info2);
void enterOnion(Mob* m, void* info1, void* info2);
void fallDownPit(Mob* m, void* info1, void* info2);
void finishCalledAnim(Mob* m, void* info1, void* info2);
void finishCarrying(Mob* m, void* info1, void* info2);
void finishDrinking(Mob* m, void* info1, void* info2);
void finishDying(Mob* m, void* info1, void* info2);
void finishGettingUp(Mob* m, void* info1, void* info2);
void finishMobLanding(Mob* m, void* info1, void* info2);
void finishPickingUp(Mob* m, void* info1, void* info2);
void flailToLeader(Mob* m, void* info1, void* info2);
void forgetCarriableObject(Mob* m, void* info1, void* info2);
void forgetGroupTask(Mob* m, void* info1, void* info2);
void forgetTool(Mob* m, void* info1, void* info2);
void getKnockedBack(Mob* m, void* info1, void* info2);
void getKnockedDown(Mob* m, void* info1, void* info2);
void goToCarriableObject(Mob* m, void* info1, void* info2);
void goToGroupTask(Mob* m, void* info1, void* info2);
void goToOnion(Mob* m, void* info1, void* info2);
void goToOpponent(Mob* m, void* info1, void* info2);
void goToTool(Mob* m, void* info1, void* info2);
void goingToDismissSpot(Mob* m, void* info1, void* info2);
void land(Mob* m, void* info1, void* info2);
void landAfterImpactBounce(Mob* m, void* info1, void* info2);
void landAfterPluck(Mob* m, void* info1, void* info2);
void landOnMob(Mob* m, void* info1, void* info2);
void landOnMobWhileHolding(Mob* m, void* info1, void* info2);
void landWhileHolding(Mob* m, void* info1, void* info2);
void leaveOnion(Mob* m, void* info1, void* info2);
void leftHazard(Mob* m, void* info1, void* info2);
void loseLatchedMob(Mob* m, void* info1, void* info2);
void notifyLeaderRelease(Mob* m, void* info1, void* info2);
void panicNewChase(Mob* m, void* info1, void* info2);
void prepareToAttack(Mob* m, void* info1, void* info2);
void reachCarriableObject(Mob* m, void* info1, void* info2);
void reachDismissSpot(Mob* m, void* info1, void* info2);
void rechaseOpponent(Mob* m, void* info1, void* info2);
void releaseTool(Mob* m, void* info1, void* info2);
void seedLanded(Mob* m, void* info1, void* info2);
void setBumpLock(Mob* m, void* info1, void* info2);
void setIdleTaskReach(Mob* m, void* info1, void* info2);
void setSwarmReach(Mob* m, void* info1, void* info2);
void sigh(Mob* m, void* info1, void* info2);
void sproutEvolve(Mob* m, void* info1, void* info2);
void sproutScheduleEvol(Mob* m, void* info1, void* info2);
void standStill(Mob* m, void* info1, void* info2);
void startBoredomAnim(Mob* m, void* info1, void* info2);
void startChasingLeader(Mob* m, void* info1, void* info2);
void startDrinking(Mob* m, void* info1, void* info2);
void startDying(Mob* m, void* info1, void* info2);
void startFlailing(Mob* m, void* info1, void* info2);
void startGettingUp(Mob* m, void* info1, void* info2);
void startImpactLunge(Mob* m, void* info1, void* info2);
void startKnockedDownDying(Mob* m, void* info1, void* info2);
void startMobLanding(Mob* m, void* info1, void* info2);
void startPanicking(Mob* m, void* info1, void* info2);
void startPickingUp(Mob* m, void* info1, void* info2);
void startReturning(Mob* m, void* info1, void* info2);
void startRidingTrack(Mob* m, void* info1, void* info2);
void startSeedParticles(Mob* m, void* info1, void* info2);
void stopBeingIdle(Mob* m, void* info1, void* info2);
void stopBeingThrown(Mob* m, void* info1, void* info2);
void stopCarrying(Mob* m, void* info1, void* info2);
void stopInGroup(Mob* m, void* info1, void* info2);
void tickCarrying(Mob* m, void* info1, void* info2);
void tickEnteringOnion(Mob* m, void* info1, void* info2);
void tickGroupTaskWork(Mob* m, void* info1, void* info2);
void tickTrackRide(Mob* m, void* info1, void* info2);
void touchedEatHitbox(Mob* m, void* info1, void* info2);
void touchedHazard(Mob* m, void* info1, void* info2);
void touchedSpray(Mob* m, void* info1, void* info2);
void tryHeldItemHotswap(Mob* m, void* info1, void* info2);
void unlatch(Mob* m, void* info1, void* info2);
void updateInGroupChasing(Mob* m, void* info1, void* info2);
void whistledWhileHolding(Mob* m, void* info1, void* info2);
void whistledWhileRiding(Mob* m, void* info1, void* info2);
void workOnGroupTask(Mob* m, void* info1, void* info2);
}
