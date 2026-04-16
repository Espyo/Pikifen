/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin finite-state machine logic.
 */

#include <algorithm>

#include "pikmin_fsm.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob/bridge.h"
#include "../mob/drop.h"
#include "../mob/group_task.h"
#include "../mob/pikmin.h"
#include "../mob/tool.h"
#include "../mob/track.h"
#include "../other/hazard.h"
#include "gen_mob_fsm.h"


#pragma region FSM


/**
 * @brief Creates the finite-state machine for the Pikmin's logic.
 *
 * @param typ Mob type to create the finite-state machine for.
 */
void PikminFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("seed", PIKMIN_STATE_SEED); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::becomeSprout);
            efc.run(PikminFsm::startSeedParticles);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(PikminFsm::seedLanded);
            efc.changeState("sprout");
        }
    }
    
    efc.newState("sprout", PIKMIN_STATE_SPROUT); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::becomeSprout);
            efc.run(PikminFsm::sproutScheduleEvol);
        }
        efc.newEvent(MOB_EV_PLUCKED); {
            efc.changeState("plucking");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::sproutEvolve);
            efc.run(PikminFsm::sproutScheduleEvol);
        }
    }
    
    efc.newState("plucking", PIKMIN_STATE_PLUCKING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::beginPluck);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("plucking_thrown");
        }
    }
    
    efc.newState("plucking_thrown", PIKMIN_STATE_PLUCKING_THROWN); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::beThrownAfterPluck);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(PikminFsm::landAfterPluck);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
    }
    
    efc.newState("leaving_onion", PIKMIN_STATE_LEAVING_ONION); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::leaveOnion);
        }
        efc.newEvent(FSM_EV_ON_TICK); {
            efc.run(PikminFsm::tickTrackRide);
        }
    }
    
    efc.newState("entering_onion", PIKMIN_STATE_ENTERING_ONION); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::enterOnion);
        }
        efc.newEvent(FSM_EV_ON_TICK); {
            efc.run(PikminFsm::tickEnteringOnion);
        }
    }
    
    efc.newState("in_group_chasing", PIKMIN_STATE_IN_GROUP_CHASING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::startChasingLeader);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(PikminFsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.run(PikminFsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("in_group_stopped");
        }
        efc.newEvent(MOB_EV_SWARM_STARTED); {
            efc.changeState("swarm_chasing");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(PikminFsm::beDismissed);
            efc.changeState("going_to_dismiss_spot");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(PikminFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("in_group_stopped", PIKMIN_STATE_IN_GROUP_STOPPED); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::stopInGroup);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::clearBoredomData);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(PikminFsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_SWARM_STARTED); {
            efc.changeState("swarm_chasing");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(PikminFsm::beDismissed);
            efc.changeState("going_to_dismiss_spot");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::startBoredomAnim);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::checkBoredomAnimEnd);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("swarm_chasing", PIKMIN_STATE_SWARM_CHASING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::setSwarmReach);
            efc.run(PikminFsm::startChasingLeader);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::setIdleTaskReach);
        }
        efc.newEvent(FSM_EV_ON_TICK); {
            efc.run(PikminFsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(PikminFsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("swarm_stopped");
        }
        efc.newEvent(MOB_EV_SWARM_ENDED); {
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(PikminFsm::beDismissed);
            efc.changeState("going_to_dismiss_spot");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(PikminFsm::goToOpponent);
        }
        efc.newEvent(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.changeState("going_to_carriable_object");
        }
        efc.newEvent(MOB_EV_NEAR_TOOL); {
            efc.run(PikminFsm::goToTool);
        }
        efc.newEvent(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(PikminFsm::goToGroupTask);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(PikminFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("swarm_stopped", PIKMIN_STATE_SWARM_STOPPED); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::setSwarmReach);
            efc.run(PikminFsm::stopInGroup);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::setIdleTaskReach);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(PikminFsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("swarm_stopped");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.changeState("swarm_chasing");
        }
        efc.newEvent(MOB_EV_SWARM_ENDED); {
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(PikminFsm::beDismissed);
            efc.changeState("going_to_dismiss_spot");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(PikminFsm::goToOpponent);
        }
        efc.newEvent(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.changeState("going_to_carriable_object");
        }
        efc.newEvent(MOB_EV_NEAR_TOOL); {
            efc.run(PikminFsm::goToTool);
        }
        efc.newEvent(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(PikminFsm::goToGroupTask);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("grabbed_by_leader", PIKMIN_STATE_GRABBED_BY_LEADER); {
        efc.newEvent(MOB_EV_THROWN); {
            efc.run(PikminFsm::beThrown);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_RELEASED); {
            efc.run(PikminFsm::beReleased);
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::notifyLeaderRelease);
            efc.run(PikminFsm::beReleased);
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::notifyLeaderRelease);
            efc.run(PikminFsm::beReleased);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("thrown", PIKMIN_STATE_THROWN); {
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(PikminFsm::land);
            efc.run(PikminFsm::setBumpLock);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(PikminFsm::checkOutgoingAttack);
            efc.run(PikminFsm::landOnMob);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_N); {
            efc.run(PikminFsm::landOnMob);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(PikminFsm::beThrownByBouncer);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("mob_landing", PIKMIN_STATE_MOB_LANDING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::startMobLanding);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::finishMobLanding);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(PikminFsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
    }
    
    efc.newState(
        "going_to_dismiss_spot", PIKMIN_STATE_GOING_TO_DISMISS_SPOT
    ); {
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::goingToDismissSpot);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::clearTimer);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(PikminFsm::reachDismissSpot);
            efc.run(PikminFsm::setBumpLock);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::reachDismissSpot);
            efc.run(PikminFsm::setBumpLock);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(PikminFsm::goToOpponent);
        }
        efc.newEvent(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.changeState("going_to_carriable_object");
        }
        efc.newEvent(MOB_EV_NEAR_TOOL); {
            efc.run(PikminFsm::goToTool);
        }
        efc.newEvent(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(PikminFsm::goToGroupTask);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(PikminFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("idling", PIKMIN_STATE_IDLING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::becomeIdle);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::clearBoredomData);
            efc.run(PikminFsm::stopBeingIdle);
        }
        efc.newEvent(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(PikminFsm::goToOpponent);
        }
        efc.newEvent(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.changeState("going_to_carriable_object");
        }
        efc.newEvent(MOB_EV_NEAR_TOOL); {
            efc.run(PikminFsm::goToTool);
        }
        efc.newEvent(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(PikminFsm::goToGroupTask);
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("riding_track");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(PikminFsm::checkLeaderBump);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::startBoredomAnim);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::checkBoredomAnimEnd);
            efc.run(PikminFsm::checkShakingAnimEnd);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(PikminFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("called", PIKMIN_STATE_CALLED); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::called);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::finishCalledAnim);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("going_to_opponent", PIKMIN_STATE_GOING_TO_OPPONENT); {
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(PikminFsm::decideAttack);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_FOCUS_OFF_REACH); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_FOCUS_DIED); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("circling_opponent", PIKMIN_STATE_CIRCLING_OPPONENT); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::circleOpponent);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::decideAttack);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_FOCUS_OFF_REACH); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_FOCUS_DIED); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "going_to_carriable_object", PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT
    ); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::goToCarriableObject);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(PikminFsm::reachCarriableObject);
            efc.changeState("carrying");
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.run(PikminFsm::forgetCarriableObject);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::forgetCarriableObject);
            efc.changeState("sighing");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(PikminFsm::forgetCarriableObject);
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::forgetCarriableObject);
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::forgetCarriableObject);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "going_to_tool", PIKMIN_STATE_GOING_TO_TOOL
    ); {
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("picking_up");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::forgetTool);
            efc.changeState("sighing");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(PikminFsm::forgetTool);
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::forgetTool);
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::forgetTool);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "going_to_group_task", PIKMIN_STATE_GOING_TO_GROUP_TASK
    ); {
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("on_group_task");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::forgetGroupTask);
            efc.changeState("sighing");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(PikminFsm::forgetGroupTask);
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.run(PikminFsm::forgetGroupTask);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::forgetGroupTask);
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::forgetGroupTask);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "going_to_onion", PIKMIN_STATE_GOING_TO_ONION
    ); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::goToOnion);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("entering_onion");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("sighing", PIKMIN_STATE_SIGHING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::sigh);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(PikminFsm::checkLeaderBump);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("carrying", PIKMIN_STATE_CARRYING); {
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::stopCarrying);
            efc.run(PikminFsm::standStill);
        }
        efc.newEvent(FSM_EV_ON_TICK); {
            efc.run(PikminFsm::tickCarrying);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FINISHED_TASK); {
            efc.run(PikminFsm::finishCarrying);
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("picking_up", PIKMIN_STATE_PICKING_UP); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::startPickingUp);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::finishPickingUp);
            efc.changeState("idling_h");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
    }
    
    efc.newState("on_group_task", PIKMIN_STATE_ON_GROUP_TASK); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::workOnGroupTask);
        }
        efc.newEvent(FSM_EV_ON_TICK); {
            efc.run(PikminFsm::tickGroupTaskWork);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::forgetGroupTask);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_FINISHED_TASK); {
            efc.changeState("celebrating");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(PikminFsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("returning", PIKMIN_STATE_RETURNING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::startReturning);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::standStill);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("attacking_grounded", PIKMIN_STATE_ATTACKING_GROUNDED); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::prepareToAttack);
        }
        efc.newEvent(MOB_EV_FOCUS_OFF_REACH); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::rechaseOpponent);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(PikminFsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("attacking_latched", PIKMIN_STATE_ATTACKING_LATCHED); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::prepareToAttack);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::unlatch);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUS_DIED); {
            efc.run(PikminFsm::loseLatchedMob);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(PikminFsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("grabbed_by_enemy", PIKMIN_STATE_GRABBED_BY_ENEMY); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::beGrabbedByEnemy);
        }
        efc.newEvent(MOB_EV_RELEASED); {
            efc.run(PikminFsm::beReleased);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_SWALLOWED); {
            efc.run(PikminFsm::startDying);
            efc.run(PikminFsm::finishDying);
        }
    }
    
    efc.newState("knocked_back", PIKMIN_STATE_KNOCKED_BACK); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::getKnockedBack);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.changeState("knocked_down");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(PikminFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
    }
    
    efc.newState("knocked_down", PIKMIN_STATE_KNOCKED_DOWN); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::getKnockedDown);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.changeState("getting_up");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(PikminFsm::calledWhileKnockedDown);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("knocked_down_dying");
        }
    }
    
    efc.newState("getting_up", PIKMIN_STATE_GETTING_UP); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::startGettingUp);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::finishGettingUp);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(PikminFsm::calledWhileKnockedDown);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("impact_bounce", PIKMIN_STATE_IMPACT_BOUNCE); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::doImpactBounce);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.changeState("knocked_down");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("impact_lunge", PIKMIN_STATE_IMPACT_LUNGE); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::startImpactLunge);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("impact_bounce");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(PikminFsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("helpless", PIKMIN_STATE_HELPLESS); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::notifyLeaderRelease);
            efc.run(PikminFsm::beReleased);
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::becomeHelpless);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        
        //The logic to lose helplessness is in
        //pikmin::handleStatusEffectLoss();
    }
    
    efc.newState("flailing", PIKMIN_STATE_FLAILING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::notifyLeaderRelease);
            efc.run(PikminFsm::beReleased);
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::startFlailing);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::standStill);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(PikminFsm::flailToLeader);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        
        //The logic to stop flailing is in
        //pikmin::handleStatusEffectLoss();
    }
    
    efc.newState("panicking", PIKMIN_STATE_PANICKING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::unlatch);
            efc.run(PikminFsm::notifyLeaderRelease);
            efc.run(PikminFsm::beReleased);
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::startPanicking);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::panicNewChase);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        
        //The logic to stop panicking is in
        //pikmin::handleStatusEffectLoss();
    }
    
    efc.newState("drinking", PIKMIN_STATE_DRINKING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::startDrinking);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::finishDrinking);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("celebrating", PIKMIN_STATE_CELEBRATING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::celebrate);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(PikminFsm::checkLeaderBump);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("in_group_chasing_h", PIKMIN_STATE_IN_GROUP_CHASING_H); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::startChasingLeader);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(PikminFsm::releaseTool);
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(PikminFsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader_h");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.run(PikminFsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("in_group_stopped_h");
        }
        efc.newEvent(MOB_EV_SWARM_STARTED); {
            efc.changeState("swarm_chasing_h");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(PikminFsm::beDismissed);
            efc.changeState("going_to_dismiss_spot_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("in_group_stopped_h", PIKMIN_STATE_IN_GROUP_STOPPED_H); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::stopInGroup);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(PikminFsm::releaseTool);
            efc.changeState("in_group_stopped");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(PikminFsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader_h");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.changeState("in_group_chasing_h");
        }
        efc.newEvent(MOB_EV_SWARM_STARTED); {
            efc.changeState("swarm_chasing_h");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(PikminFsm::beDismissed);
            efc.changeState("going_to_dismiss_spot_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("swarm_chasing_h", PIKMIN_STATE_SWARM_CHASING_H); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::setSwarmReach);
            efc.run(PikminFsm::startChasingLeader);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(PikminFsm::releaseTool);
            efc.changeState("swarm_chasing");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::setIdleTaskReach);
        }
        efc.newEvent(FSM_EV_ON_TICK); {
            efc.run(PikminFsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(PikminFsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader_h");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("swarm_stopped_h");
        }
        efc.newEvent(MOB_EV_SWARM_ENDED); {
            efc.changeState("in_group_chasing_h");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(PikminFsm::beDismissed);
            efc.changeState("going_to_dismiss_spot_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("swarm_stopped_h", PIKMIN_STATE_SWARM_STOPPED_H); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::setSwarmReach);
            efc.run(PikminFsm::stopInGroup);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(PikminFsm::releaseTool);
            efc.changeState("swarm_stopped");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::setIdleTaskReach);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(PikminFsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader_h");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("swarm_stopped_h");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.changeState("swarm_chasing_h");
        }
        efc.newEvent(MOB_EV_SWARM_ENDED); {
            efc.changeState("in_group_chasing_h");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(PikminFsm::beDismissed);
            efc.changeState("going_to_dismiss_spot_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("grabbed_by_leader_h", PIKMIN_STATE_GRABBED_BY_LEADER_H); {
        efc.newEvent(MOB_EV_THROWN); {
            efc.run(PikminFsm::beThrown);
            efc.changeState("thrown_h");
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(PikminFsm::releaseTool);
            efc.changeState("grabbed_by_leader");
        }
        efc.newEvent(MOB_EV_RELEASED); {
            efc.changeState("in_group_chasing_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::notifyLeaderRelease);
            efc.run(PikminFsm::beReleased);
            efc.run(PikminFsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::notifyLeaderRelease);
            efc.run(PikminFsm::beReleased);
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("thrown_h", PIKMIN_STATE_THROWN_H); {
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(PikminFsm::releaseTool);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(PikminFsm::landWhileHolding);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(PikminFsm::landOnMobWhileHolding);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_N); {
            efc.run(PikminFsm::landOnMobWhileHolding);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "going_to_dismiss_spot_h", PIKMIN_STATE_GOING_TO_DISMISS_SPOT_H
    ); {
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called_h");
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(PikminFsm::releaseTool);
            efc.changeState("going_to_dismiss_spot");
        }
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::goingToDismissSpot);
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::clearTimer);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(PikminFsm::reachDismissSpot);
            efc.run(PikminFsm::setBumpLock);
            efc.changeState("idling_h");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(PikminFsm::reachDismissSpot);
            efc.changeState("idling_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("idling_h", PIKMIN_STATE_IDLING_H); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::becomeIdle);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(PikminFsm::releaseTool);
            efc.changeState("idling");
        }
        efc.newEvent(FSM_EV_ON_LEAVE); {
            efc.run(PikminFsm::stopBeingIdle);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(PikminFsm::whistledWhileHolding);
        }
        efc.newEvent(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(PikminFsm::checkLeaderBump);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::releaseTool);
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("called_h", PIKMIN_STATE_CALLED_H); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::called);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::finishCalledAnim);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(PikminFsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(PikminFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(PikminFsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(PikminFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(PikminFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(PikminFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(PikminFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("riding_track", PIKMIN_STATE_RIDING_TRACK); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::startRidingTrack);
        }
        efc.newEvent(FSM_EV_ON_TICK); {
            efc.run(PikminFsm::tickTrackRide);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(PikminFsm::whistledWhileRiding);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("crushed", PIKMIN_STATE_CRUSHED); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::beCrushed);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::finishDying);
        }
    }
    
    efc.newState("knocked_down_dying", PIKMIN_STATE_KNOCKED_DOWN_DYING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::startKnockedDownDying);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::finishDying);
        }
    }
    
    efc.newState("dying", PIKMIN_STATE_DYING); {
        efc.newEvent(FSM_EV_ON_ENTER); {
            efc.run(PikminFsm::standStill);
            efc.run(PikminFsm::startDying);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(PikminFsm::finishDying);
        }
    }
    
    typ->scriptDef.fsm.states = efc.finish();
    typ->scriptDef.fsm.compileStates();
    typ->scriptDef.fsm.setFirstState("idling");
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->scriptDef.fsm.states.size() == N_PIKMIN_STATES,
        i2s(typ->scriptDef.fsm.states.size()) + " registered, " +
        i2s(N_PIKMIN_STATES) + " in enum."
    );
}


#pragma endregion
#pragma region FSM functions


/**
 * @brief When a Pikmin is hit by an attack and gets knocked back.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void PikminFsm::beAttacked(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    bool knockbackExists = false;
    
    if(info) {
        //Damage.
        float healthBefore = pikPtr->health;
        float offenseMultiplier = 0;
        float defenseMultiplier = 0;
        float damage = 0;
        bool validAttack = false;
        
        validAttack =
            info->mob2->calculateAttackBasics(
                pikPtr, info->h2, info->h1,
                &offenseMultiplier, &defenseMultiplier
            );
        if(validAttack) {
            validAttack =
                info->mob2->calculateAttackDamage(
                    pikPtr, info->h2, info->h1,
                    offenseMultiplier, defenseMultiplier, &damage
                );
        }
        if(validAttack) {
            pikPtr->applyAttackDamage(info->mob2, info->h2, info->h1, damage);
        }
        
        if(pikPtr->health <= 0.0f && healthBefore > 0.0f) {
            if(info->h2->hazard) {
                game.statistics.pikminHazardDeaths++;
            }
        }
        
        //Knockback.
        float knockbackStrength = 0.0f;
        float knockbackAngle = 0.0f;
        info->mob2->calculateAttackKnockback(
            pikPtr, info->h2, info->h1,
            offenseMultiplier, defenseMultiplier,
            &knockbackExists, &knockbackStrength, &knockbackAngle
        );
        if(knockbackExists) {
            pikPtr->applyKnockback(knockbackStrength, knockbackAngle);
        }
        
        //Withering.
        if(info->h2->witherChance > 0 && pikPtr->maturity > 0) {
            unsigned char witherRoll = game.rng.i(0, 100);
            if(witherRoll < info->h2->witherChance) {
                pikPtr->increaseMaturity(-1);
            }
        }
        
        //Effects.
        pikPtr->doAttackEffects(
            info->mob2, info->h2, info->h1, damage, knockbackStrength
        );
        
    } else {
        //This can happen, for example, if the Pikmin got told to get knocked
        //back from a bomb rock hotswap. There's no real "hit" in this case
        //so let's just do the basics and let the Pikmin leave the group,
        //change animation, and little else.
    }
    
    //Finish up.
    if(knockbackExists) {
        PikminFsm::forgetGroupTask(scriptVM, info1, info2);
        PikminFsm::unlatch(scriptVM, info1, info2);
        PikminFsm::forgetCarriableObject(scriptVM, info1, info2);
        PikminFsm::forgetTool(scriptVM, info1, info2);
        pikPtr->leaveGroup();
        PikminFsm::beReleased(scriptVM, info1, info2);
        PikminFsm::notifyLeaderRelease(scriptVM, info1, info2);
        PikminFsm::releaseTool(scriptVM, nullptr, nullptr);
        pikPtr->face(pikPtr->angle, nullptr);
        scriptVM->fsm.setState(PIKMIN_STATE_KNOCKED_BACK, info1, info2);
    }
}


/**
 * @brief When a Pikmin becomes "helpless".
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::becomeHelpless(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pikPtr->leaveGroup();
    pikPtr->setAnimation(PIKMIN_ANIM_IDLING);
}


/**
 * @brief When a Pikmin becomes idling.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::becomeIdle(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    PikminFsm::standStill(scriptVM, info1, info2);
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
        pikPtr->chase(
            pikPtr->pos,
            pikPtr->groundSector->z + PIKMIN::FLIER_ABOVE_FLOOR_HEIGHT
        );
    }
    
    scriptVM->unfocusFromMob();
    
    pikPtr->setAnimation(
        PIKMIN_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME, true
    );
    scriptVM->setTimer(
        game.rng.f(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin becomes a seed or a sprout.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::becomeSprout(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->leaveGroup();
    enableFlag(pikPtr->flags, MOB_FLAG_INTANGIBLE);
    enableFlag(pikPtr->flags, MOB_FLAG_NON_HUNTABLE);
    enableFlag(pikPtr->flags, MOB_FLAG_NON_HURTABLE);
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pikPtr->isSeedOrSprout = true;
    pikPtr->setAnimation(
        PIKMIN_ANIM_SPROUT, START_ANIM_OPTION_RANDOM_TIME, true
    );
}


/**
 * @brief When a Pikmin is crushed.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::beCrushed(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    PikminFsm::startDying(scriptVM, info1, info2);
    pikPtr->z = pikPtr->groundSector->z;
    pikPtr->setAnimation(PIKMIN_ANIM_CRUSHED);
}


/**
 * @brief When a Pikmin is dismissed by its leader.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the world coordinates to go to.
 * @param info2 Unused.
 */
void PikminFsm::beDismissed(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pikPtr->chase(*((Point*) info1), pikPtr->z);
    
    pikPtr->playSound(pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_IDLE]);
}


/**
 * @brief Makes a Pikmin begin its plucking process.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the leader that is plucking.
 * @param info2 Unused.
 */
void PikminFsm::beginPluck(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Mob* leaPtr = (Mob*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    scriptVM->focusOnMob(leaPtr);
    disableFlag(pikPtr->flags, MOB_FLAG_NON_HUNTABLE);
    disableFlag(pikPtr->flags, MOB_FLAG_NON_HURTABLE);
    disableFlag(pikPtr->flags, MOB_FLAG_INTANGIBLE);
    pikPtr->isSeedOrSprout = false;
    
    //Clear sprout evolution timer.
    PikminFsm::clearTimer(scriptVM, info1, info2);
    
    pikPtr->setAnimation(PIKMIN_ANIM_PLUCKING);
}


/**
 * @brief When a Pikmin is grabbed by an enemy.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the enemy.
 * @param info2 Pointer to the hitbox that grabbed.
 */
void PikminFsm::beGrabbedByEnemy(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Mob* enePtr = (Mob*) info1;
    Hitbox* hboxPtr = (Hitbox*) info2;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    engineAssert(info2 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    enePtr->chomp(pikPtr, hboxPtr);
    pikPtr->isGrabbedByEnemy = true;
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pikPtr->leaveGroup();
    
    pikPtr->setAnimation(PIKMIN_ANIM_FLAILING, START_ANIM_OPTION_RANDOM_TIME);
    pikPtr->playSound(pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_CAUGHT]);
}


/**
 * @brief When a Pikmin is grabbed by a leader.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::beGrabbedByFriend(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pikPtr->setAnimation(PIKMIN_ANIM_IDLING);
    pikPtr->playSound(pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_HELD]);
}


/**
 * @brief When a Pikmin is gently released by a leader or enemy.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::beReleased(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->isGrabbedByEnemy = false;
    
    size_t heldSoundIdx =
        pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_HELD];
    if(heldSoundIdx != INVALID) {
        game.audio.stopAllPlaybacks(
            pikPtr->type->sounds[heldSoundIdx].sample
        );
    }
}


/**
 * @brief When a Pikmin is thrown by a leader.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::beThrown(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    pikPtr->setAnimation(PIKMIN_ANIM_THROWN);
    
    size_t heldSoundIdx =
        pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_HELD];
    if(heldSoundIdx != INVALID) {
        game.audio.stopAllPlaybacks(
            pikPtr->type->sounds[heldSoundIdx].sample
        );
    }
    
    size_t throwSoundIdx =
        pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_THROWN];
    if(throwSoundIdx != INVALID) {
        MobType::Sound* throwSound =
            &pikPtr->type->sounds[throwSoundIdx];
        game.audio.addNewMobSoundSource(
            throwSound->sample,
            pikPtr, false, { .stackMode = SOUND_STACK_MODE_OVERRIDE }
        );
    }
    
    pikPtr->startThrowTrail();
}


/**
 * @brief When a Pikmin is thrown after being plucked.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void PikminFsm::beThrownAfterPluck(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    float throwAngle = getAngle(pikPtr->pos, scriptVM->focusedMob->pos);
    pikPtr->speedZ = PIKMIN::THROW_VER_SPEED;
    pikPtr->speed = angleToCoordinates(throwAngle, PIKMIN::THROW_HOR_SPEED);
    pikPtr->face(throwAngle + TAU / 2.0f, nullptr, true);
    
    pikPtr->setAnimation(PIKMIN_ANIM_PLUCKING_THROWN);
    pikPtr->startThrowTrail();
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parPikminPluckDirt, pikPtr
        );
    pikPtr->particleGenerators.push_back(pg);
}


/**
 * @brief When a Pikmin is thrown by a bouncer mob.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void PikminFsm::beThrownByBouncer(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pikPtr->setAnimation(PIKMIN_ANIM_THROWN);
    
    pikPtr->startThrowTrail();
}


/**
 * @brief When a Pikmin is called over by a leader, either by being whistled,
 * or touched when idling.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the leader that called.
 * @param info2 If not nullptr, then the Pikmin must be silent.
 */
void PikminFsm::called(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Mob* caller = (Mob*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    pikPtr->wasLastHitDing = false;
    pikPtr->consecutiveDings = 0;
    PikminFsm::standStill(scriptVM, info1, info2);
    
    scriptVM->focusOnMob(caller);
    
    pikPtr->setAnimation(PIKMIN_ANIM_CALLED);
    if(info2 == nullptr) {
        pikPtr->playSound(pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_CALLED]);
    }
}


/**
 * @brief When a Pikmin that is knocked down is called over by a leader,
 * either by being whistled, or touched when idling.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void PikminFsm::calledWhileKnockedDown(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Mob* caller = (Mob*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    //Let's use the "temp" variable to specify whether or not a leader
    //already whistled it.
    if(pikPtr->tempI == 1) return;
    
    scriptVM->focusOnMob(caller);
    
    scriptVM->timer.timeLeft =
        std::max(
            0.01f,
            scriptVM->timer.timeLeft -
            pikPtr->pikType->knockedDownWhistleBonus
        );
        
    pikPtr->tempI = 1;
}


/**
 * @brief When a Pikmin should celebrate.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::celebrate(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(game.rng.i(0, 1) == 0) {
        pikPtr->setAnimation(PIKMIN_ANIM_BACKFLIP);
    } else {
        pikPtr->setAnimation(PIKMIN_ANIM_TWIRLING);
    }
}


/**
 * @brief When a Pikmin should check if the animation that ended is a boredom
 * animation.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::checkBoredomAnimEnd(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(!pikPtr->inBoredAnimation) return;
    pikPtr->setAnimation(PIKMIN_ANIM_IDLING);
    pikPtr->inBoredAnimation = false;
    scriptVM->setTimer(
        game.rng.f(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin should check the attack it has just received.
 * If the attack is successful, another event is triggered. Otherwise
 * nothing happens.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void PikminFsm::checkIncomingAttack(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    if(pikPtr->invulnPeriod.timeLeft > 0) {
        //The Pikmin cannot be attacked right now.
        return;
    }
    
    if(!pikPtr->processAttackMiss(info)) {
        //It has been decided that this attack missed.
        return;
    }
    
    float offenseMultiplier = 0;
    float defenseMultiplier = 0;
    float damage = 0;
    if(
        !info->mob2->calculateAttackBasics(
            pikPtr, info->h2, info->h1,
            &offenseMultiplier, &defenseMultiplier
        )
    ) {
        //This attack doesn't work.
        return;
    }
    
    if(
        !info->mob2->calculateAttackDamage(
            pikPtr, info->h2, info->h1,
            offenseMultiplier, defenseMultiplier, &damage
        )
    ) {
        //This attack doesn't cause damage.
        return;
    }
    
    //If we got to this point, then green light for the attack.
    scriptVM->fsm.runEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED, info1, info2);
}


/**
 * @brief When a Pikmin should check if the leader bumping it should
 * result in it being added to the group or not.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the leader that called.
 * @param info2 If not nullptr, then the Pikmin must be silent.
 */
void PikminFsm::checkLeaderBump(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(game.options.misc.pikminBumpDist >= 0.0f && pikPtr->bumpLock > 0.0f) {
        pikPtr->bumpLock = game.config.pikmin.idleBumpDelay;
        return;
    }
    if(pikPtr->getMobHeldInHand()) {
        scriptVM->fsm.setState(PIKMIN_STATE_CALLED_H, info1, info2);
    } else {
        scriptVM->fsm.setState(PIKMIN_STATE_CALLED, info1, info2);
    }
}


/**
 * @brief When a Pikmin should check the attack it is about to unleash.
 * If it realizes it's doing no damage, it should start considering
 * sighing and giving up.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the opponent.
 * @param info2 Unused.
 */
void PikminFsm::checkOutgoingAttack(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    float offenseMultiplier = 0;
    float defenseMultiplier = 0;
    float damage = 0;
    bool attackSuccess =
        pikPtr->calculateAttackBasics(
            info->mob2, info->h1, info->h2,
            &offenseMultiplier, &defenseMultiplier
        );
        
    if(attackSuccess) {
        attackSuccess =
            pikPtr->calculateAttackDamage(
                info->mob2, info->h1, info->h2,
                offenseMultiplier, defenseMultiplier, &damage
            );
    }
    
    if(damage == 0 || !attackSuccess) {
        pikPtr->wasLastHitDing = true;
    } else {
        pikPtr->wasLastHitDing = false;
        pikPtr->consecutiveDings = 0;
    }
}


/**
 * @brief When a Pikmin should check if the animation that ended is a shaking
 * animation.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::checkShakingAnimEnd(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(!pikPtr->inShakingAnimation) return;
    pikPtr->setAnimation(PIKMIN_ANIM_IDLING);
    pikPtr->inShakingAnimation = false;
    scriptVM->setTimer(
        game.rng.f(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin has to circle around its opponent.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::circleOpponent(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->stopChasing();
    pikPtr->stopCircling();
    
    float circleTime = game.rng.f(0.0f, 1.0f);
    //Bias the time so that there's a higher chance of picking a close angle,
    //and a lower chance of circling to a distant one. The Pikmin came here
    //to attack, not dance!
    circleTime *= circleTime;
    circleTime += 0.5f;
    scriptVM->setTimer(circleTime);
    
    bool goCw = game.rng.f(0.0f, 1.0f) <= 0.5f;
    pikPtr->circleAround(
        scriptVM->focusedMob, Point(),
        scriptVM->focusedMob->radius + pikPtr->radius, goCw,
        pikPtr->getBaseSpeed(), true
    );
    
    pikPtr->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_RANDOM_TIME,
        true, pikPtr->type->moveSpeed
    );
}


/**
 * @brief When a Pikmin has to clear any data about being bored.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::clearBoredomData(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    PikminFsm::clearTimer(scriptVM, info1, info2);
    pikPtr->inBoredAnimation = false;
}


/**
 * @brief When a Pikmin has to clear any timer set.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::clearTimer(ScriptVM* scriptVM, void* info1, void* info2) {
    scriptVM->setTimer(0);
}


/**
 * @brief When the Pikmin reaches an opponent that it was chasing after,
 * and should now decide how to attack it.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::decideAttack(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(!scriptVM->focusedMob) return;
    
    if(pikPtr->invulnPeriod.timeLeft > 0) {
        //Don't let the Pikmin attack while invulnerable. Otherwise, this can
        //be exploited to let Pikmin vulnerable to a hazard attack the obstacle
        //emitting said hazard.
        return;
    }
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pikPtr->stopChasing();
    pikPtr->stopCircling();
    
    bool canCircle =
        scriptVM->fsm.curState->id != PIKMIN_STATE_CIRCLING_OPPONENT &&
        scriptVM->focusedMob->type->category->id == MOB_CATEGORY_ENEMIES;
        
    switch(pikPtr->pikType->attackMethod) {
    case PIKMIN_ATTACK_LATCH: {
        //This Pikmin latches on to things and/or smacks with its top.
        Distance d;
        Hitbox* closestH =
            scriptVM->focusedMob->getClosestHitbox(
                pikPtr->pos, HITBOX_TYPE_NORMAL, &d
            );
        float hZ = 0;
        
        if(closestH) {
            hZ = closestH->z + scriptVM->focusedMob->z;
        }
        
        if(
            !closestH ||
            closestH->surfaceType != HITBOX_SURFACE_TYPE_LATCHABLE ||
            hZ > pikPtr->z + pikPtr->height ||
            hZ + closestH->height < pikPtr->z ||
            d >= closestH->radius + pikPtr->radius
        ) {
            //Can't latch to the closest hitbox.
            
            if(
                game.rng.f(0.0f, 1.0f) <=
                PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED &&
                canCircle
            ) {
                //Circle around the opponent a bit before smacking.
                scriptVM->fsm.setState(PIKMIN_STATE_CIRCLING_OPPONENT);
            } else {
                //Smack.
                scriptVM->fsm.setState(PIKMIN_STATE_ATTACKING_GROUNDED);
            }
            
        } else {
            //Can latch to the closest hitbox.
            
            if(
                game.rng.f(0, 1) <=
                PIKMIN::CIRCLE_OPPONENT_CHANCE_PRE_LATCH &&
                canCircle
            ) {
                //Circle around the opponent a bit before latching.
                scriptVM->fsm.setState(PIKMIN_STATE_CIRCLING_OPPONENT);
            } else {
                //Latch on.
                pikPtr->latch(scriptVM->focusedMob, closestH);
                scriptVM->fsm.setState(PIKMIN_STATE_ATTACKING_LATCHED);
            }
            
        }
        
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {
        //This Pikmin attacks by lunching forward for an impact.
        
        if(
            game.rng.f(0, 1) <=
            PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED &&
            canCircle
        ) {
            //Circle around the opponent a bit before lunging.
            scriptVM->fsm.setState(PIKMIN_STATE_CIRCLING_OPPONENT);
        } else {
            //Go for the lunge.
            scriptVM->fsm.setState(PIKMIN_STATE_IMPACT_LUNGE);
        }
        
        break;
        
    }
    }
}


/**
 * @brief When a Pikmin has to bounce back from an impact attack.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::doImpactBounce(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    float impactAngle = 0.0f;
    float impactSpeed = 0.0f;
    
    if(scriptVM->focusedMob) {
        if(scriptVM->focusedMob->rectangularDim.x != 0) {
            impactAngle =
                getAngle(
                    getClosestPointInRotatedRectangle(
                        pikPtr->pos,
                        scriptVM->focusedMob->pos,
                        scriptVM->focusedMob->rectangularDim,
                        scriptVM->focusedMob->angle,
                        nullptr
                    ),
                    pikPtr->pos
                );
        } else {
            impactAngle = getAngle(scriptVM->focusedMob->pos, pikPtr->pos);
        }
        impactSpeed = 200.0f;
    }
    
    pikPtr->speed =
        angleToCoordinates(
            impactAngle, impactSpeed
        );
    pikPtr->speedZ = 500.0f;
    pikPtr->face(impactAngle + TAU / 2.0f, nullptr, true);
    
    pikPtr->setAnimation(PIKMIN_ANIM_BOUNCED_BACK);
}


/**
 * @brief When a Pikmin must start climbing up an Onion's leg.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::enterOnion(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Onion* oniPtr = (Onion*) scriptVM->focusedMob;
    
    engineAssert(
        scriptVM->focusedMob != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    //Set its data to start climbing.
    vector<size_t> checkpoints;
    checkpoints.push_back((pikPtr->tempI * 2) + 1);
    checkpoints.push_back(pikPtr->tempI * 2);
    
    pikPtr->trackInfo = new TrackRideInfo(
        oniPtr, checkpoints, oniPtr->oniType->nest->pikminEnterSpeed
    );
    
    pikPtr->setAnimation(PIKMIN_ANIM_CLIMBING, START_ANIM_OPTION_RANDOM_TIME);
}


/**
 * @brief When a Pikmin falls down a bottomless pit.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::fallDownPit(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->startDying();
    pikPtr->finishDying();
}


/**
 * @brief When a Pikmin finished the animation for when it's called.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::finishCalledAnim(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Mob* leaPtr = scriptVM->focusedMob;
    bool pikIsHolding = pikPtr->getMobHeldInHand();
    
    if(leaPtr) {
        if(leaPtr->followingGroup) {
            //If this leader is following another one,
            //then the new Pikmin should be in the group of that top leader.
            leaPtr = leaPtr->followingGroup;
        }
        if(leaPtr->addToGroup(pikPtr)) {
            scriptVM->fsm.setState(
                pikIsHolding ?
                PIKMIN_STATE_IN_GROUP_CHASING_H :
                PIKMIN_STATE_IN_GROUP_CHASING,
                info1, info2);
            return;
        }
    }
    scriptVM->fsm.setState(
        pikIsHolding ?
        PIKMIN_STATE_IDLING_H :
        PIKMIN_STATE_IDLING,
        info1, info2
    );
}


/**
 * @brief When a Pikmin successfully finishes carrying an object.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::finishCarrying(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(
        pikPtr->carryingMob != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    if(pikPtr->carryingMob->carryInfo->mustReturn) {
        //The Pikmin should return somewhere (like a pile).
        scriptVM->fsm.setState(
            PIKMIN_STATE_RETURNING, (void*) pikPtr->carryingMob
        );
        
    } else {
        //The Pikmin can just sit and chill.
        scriptVM->fsm.setState(PIKMIN_STATE_CELEBRATING);
    }
}


/**
 * @brief When a Pikmin finishes drinking the drop it was drinking.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::finishDrinking(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(
        scriptVM->focusedMob != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    Drop* droPtr = (Drop*) scriptVM->focusedMob;
    
    switch(droPtr->droType->effect) {
    case DROP_EFFECT_MATURATE: {
        pikPtr->increaseMaturity(droPtr->droType->increaseAmount);
        break;
    } case DROP_EFFECT_GIVE_STATUS: {
        pikPtr->applyStatus(
            droPtr->droType->statusToGive, false, false, droPtr
        );
        break;
    } default: {
        break;
    }
    }
    
    scriptVM->unfocusFromMob();
}


/**
 * @brief When a Pikmin finishes dying.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::finishDying(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->finishDying();
}


/**
 * @brief When a Pikmin finishes getting up from being knocked down.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::finishGettingUp(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Mob* prevFocusedMob = scriptVM->focusedMob;
    
    scriptVM->fsm.setState(PIKMIN_STATE_IDLING);
    
    if(prevFocusedMob) {
        if(
            prevFocusedMob->type->category->id == MOB_CATEGORY_LEADERS &&
            !pikPtr->canHunt(prevFocusedMob)
        ) {
            scriptVM->fsm.runEvent(MOB_EV_WHISTLED, (void*) prevFocusedMob);
            
        } else if(
            pikPtr->canHunt(prevFocusedMob)
        ) {
            scriptVM->fsm.runEvent(
                MOB_EV_OPPONENT_IN_REACH, (void*) prevFocusedMob, nullptr
            );
            
        }
    }
}


/**
 * @brief When a Pikmin finishes its sequence of landing on another mob.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::finishMobLanding(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(!scriptVM->focusedMob) {
        //The mob has died or vanished since the Pikmin first landed.
        //Return to idle.
        scriptVM->fsm.setState(PIKMIN_STATE_IDLING);
        return;
    }
    
    switch(pikPtr->pikType->attackMethod) {
    case PIKMIN_ATTACK_LATCH: {
        scriptVM->fsm.setState(PIKMIN_STATE_ATTACKING_LATCHED);
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {
        scriptVM->fsm.setState(PIKMIN_STATE_IMPACT_BOUNCE);
        break;
        
    }
    }
}


/**
 * @brief When a Pikmin finishes picking some object up to hold it.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::finishPickingUp(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Tool* tooPtr = (Tool*) (scriptVM->focusedMob);
    
    if(!hasFlag(tooPtr->holdabilityFlags, HOLDABILITY_FLAG_PIKMIN)) {
        scriptVM->fsm.setState(PIKMIN_STATE_IDLING);
        return;
    }
    
    pikPtr->subgroupTypePtr =
        game.states.gameplay->subgroupTypes.getType(
            SUBGROUP_TYPE_CATEGORY_TOOL, tooPtr->type
        );
    pikPtr->hold(
        tooPtr, HOLD_TYPE_PURPOSE_HAND, INVALID, 4, 0, 0.5f,
        true, HOLD_ROTATION_METHOD_FACE_HOLDER
    );
    scriptVM->unfocusFromMob();
}


/**
 * @brief When the Pikmin must move towards the whistle.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void PikminFsm::flailToLeader(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Mob* caller = (Mob*) info1;
    
    pikPtr->chase(caller->pos, caller->z);
}


/**
 * @brief When a Pikmin is meant to drop the object it's carrying, or
 * stop chasing the object if it's not carrying it yet, but wants to.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::forgetCarriableObject(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(!pikPtr->carryingMob) return;
    
    pikPtr->carryingMob->carryInfo->spotInfo[pikPtr->tempI].state =
        CARRY_SPOT_STATE_FREE;
    pikPtr->carryingMob->carryInfo->spotInfo[pikPtr->tempI].pikPtr =
        nullptr;
        
    pikPtr->carryingMob = nullptr;
}


/**
 * @brief When a Pikmin is meant to forget a group task object it was going for.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::forgetGroupTask(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(!scriptVM->focusedMob) return;
    if(scriptVM->focusedMob->type->category->id != MOB_CATEGORY_GROUP_TASKS) {
        return;
    }
    
    GroupTask* tasPtr = (GroupTask*) (scriptVM->focusedMob);
    tasPtr->freeUpSpot(pikPtr);
    scriptVM->unfocusFromMob();
}


/**
 * @brief When a Pikmin is meant to forget a tool object it was going for.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::forgetTool(ScriptVM* scriptVM, void* info1, void* info2) {
    if(!scriptVM->focusedMob) return;
    
    Tool* tooPtr = (Tool*) (scriptVM->focusedMob);
    tooPtr->reserved = nullptr;
    scriptVM->unfocusFromMob();
}


/**
 * @brief When a Pikmin gets knocked back.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::getKnockedBack(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pikPtr->setAnimation(PIKMIN_ANIM_KNOCKED_BACK);
}


/**
 * @brief When a Pikmin gets knocked back and lands on the floor.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::getKnockedDown(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    //Let's use the "temp" variable to specify whether or not a leader
    //already whistled it.
    pikPtr->tempI = 0;
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    scriptVM->setTimer(pikPtr->pikType->knockedDownDuration);
    
    pikPtr->setAnimation(PIKMIN_ANIM_LYING);
}


/**
 * @brief When a Pikmin needs to get going to its dismiss spot.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::goingToDismissSpot(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    scriptVM->setTimer(PIKMIN::DISMISS_TIMEOUT);
    
    pikPtr->setAnimation(
        pikPtr->getMobHeldInHand() ?
        PIKMIN_ANIM_CARRYING_LIGHT :
        PIKMIN_ANIM_WALKING,
        START_ANIM_OPTION_RANDOM_TIME, true, pikPtr->type->moveSpeed
    );
}


/**
 * @brief When a Pikmin needs to go towards its spot on a carriable object.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the mob to carry.
 * @param info2 Unused.
 */
void PikminFsm::goToCarriableObject(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Mob* carriableMob = (Mob*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pikPtr->carryingMob = carriableMob;
    pikPtr->leaveGroup();
    pikPtr->stopChasing();
    
    size_t closestSpot = INVALID;
    Distance closestSpotDist;
    CarrierSpot* closestSpotPtr = nullptr;
    Point closestSpotOffset;
    
    //If this is the first Pikmin to go to the carriable mob, rotate
    //the points such that 0 faces this Pikmin instead.
    if(
        carriableMob->carryInfo->isEmpty() &&
        carriableMob->type->customCarrySpots.empty()
    ) {
        carriableMob->carryInfo->rotatePoints(
            getAngle(carriableMob->pos, pikPtr->pos)
        );
    }
    
    for(size_t s = 0; s < carriableMob->type->maxCarriers; s++) {
        CarrierSpot* spotPtr =
            &carriableMob->carryInfo->spotInfo[s];
        if(spotPtr->state != CARRY_SPOT_STATE_FREE) continue;
        
        Point spotOffset =
            rotatePoint(spotPtr->pos, carriableMob->angle);
        Distance d(pikPtr->pos, carriableMob->pos + spotOffset);
        
        if(closestSpot == INVALID || d < closestSpotDist) {
            closestSpot = s;
            closestSpotDist = d;
            closestSpotPtr = spotPtr;
            closestSpotOffset = spotOffset;
        }
    }
    
    if(!closestSpotPtr) return;
    
    scriptVM->focusOnMob(carriableMob);
    pikPtr->tempI = closestSpot;
    closestSpotPtr->state = CARRY_SPOT_STATE_RESERVED;
    closestSpotPtr->pikPtr = pikPtr;
    
    pikPtr->chase(
        &carriableMob->pos, &carriableMob->z,
        closestSpotOffset, 0.0f
    );
    scriptVM->setTimer(PIKMIN::GOTO_TIMEOUT);
    
    pikPtr->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_RANDOM_TIME,
        true, pikPtr->type->moveSpeed
    );
    
}


/**
 * @brief When a Pikmin needs to go towards a group task mob.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the group task.
 * @param info2 Unused.
 */
void PikminFsm::goToGroupTask(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    GroupTask* tasPtr = (GroupTask*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    if(
        !hasFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR) &&
        tasPtr->tasType->flyingPikminOnly
    ) {
        //Only flying Pikmin can use this, and this Pikmin doesn't fly.
        return;
    }
    
    GroupTask::GroupTaskSpot* freeSpot = tasPtr->getFreeSpot();
    if(!freeSpot) {
        //There are no free spots available. Forget it.
        return;
    }
    
    tasPtr->reserveSpot(freeSpot, pikPtr);
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pikPtr->leaveGroup();
    pikPtr->stopChasing();
    
    scriptVM->focusOnMob(tasPtr);
    
    pikPtr->chase(
        &(freeSpot->absolutePos), &tasPtr->z,
        Point(), tasPtr->tasType->spotsZ
    );
    scriptVM->setTimer(PIKMIN::GOTO_TIMEOUT);
    
    pikPtr->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_RANDOM_TIME,
        true, pikPtr->type->moveSpeed
    );
    
    scriptVM->fsm.setState(PIKMIN_STATE_GOING_TO_GROUP_TASK);
    
}


/**
 * @brief When a Pikmin needs to walk towards an Onion to climb inside.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the Onion.
 * @param info2 Unused.
 */
void PikminFsm::goToOnion(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    PikminNest* nestPtr = (PikminNest*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    //Pick a leg at random.
    pikPtr->tempI =
        game.rng.i(
            0, (int) (nestPtr->nestType->legBodyParts.size() / 2) - 1
        );
    size_t legFootBPIdx =
        nestPtr->mPtr->anim.animDb->findBodyPart(
            nestPtr->nestType->legBodyParts[pikPtr->tempI * 2 + 1]
        );
    Point coords =
        nestPtr->mPtr->getHitbox(
            legFootBPIdx
        )->getCurPos(nestPtr->mPtr->pos, nestPtr->mPtr->angle);
        
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    bool auxB = true; //Needed for a gentle release.
    PikminFsm::releaseTool(scriptVM, (void*) &auxB, nullptr);
    
    scriptVM->focusOnMob(nestPtr->mPtr);
    pikPtr->stopChasing();
    pikPtr->chase(coords, nestPtr->mPtr->z);
    pikPtr->leaveGroup();
    
    pikPtr->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_RANDOM_TIME,
        true, pikPtr->type->moveSpeed
    );
}


/**
 * @brief When a Pikmin needs to walk towards an opponent.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the opponent.
 * @param info2 Unused.
 */
void PikminFsm::goToOpponent(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Mob* otherPtr = (Mob*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    if(!pikPtr->pikType->canFly) {
        //Grounded Pikmin.
        if(otherPtr->type->category->id == MOB_CATEGORY_ENEMIES) {
            Enemy* enePtr = (Enemy*) info1;
            if(!enePtr->eneType->allowGroundAttacks) return;
            if(enePtr->z > pikPtr->z + pikPtr->height) return;
        }
    } else {
        //Airborne Pikmin.
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    scriptVM->focusOnMob(otherPtr);
    pikPtr->stopChasing();
    
    Point offset = Point();
    float targetDist =
        scriptVM->focusedMob->radius +
        pikPtr->radius + PIKMIN::GROUNDED_ATTACK_DIST;
        
    if(scriptVM->focusedMob->rectangularDim.x != 0.0f) {
        bool isInside = false;
        offset =
            getClosestPointInRotatedRectangle(
                pikPtr->pos,
                scriptVM->focusedMob->pos,
                scriptVM->focusedMob->rectangularDim,
                scriptVM->focusedMob->angle,
                &isInside
            ) - scriptVM->focusedMob->pos;
        targetDist -= scriptVM->focusedMob->radius;
    }
    
    pikPtr->chase(
        &scriptVM->focusedMob->pos, &scriptVM->focusedMob->z,
        offset, 0.0f, 0,
        targetDist
    );
    pikPtr->leaveGroup();
    
    pikPtr->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_RANDOM_TIME,
        true, pikPtr->type->moveSpeed
    );
    
    scriptVM->fsm.setState(PIKMIN_STATE_GOING_TO_OPPONENT);
}


/**
 * @brief When a Pikmin needs to go towards a tool mob.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the tool.
 * @param info2 Unused.
 */
void PikminFsm::goToTool(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Tool* tooPtr = (Tool*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    if(tooPtr->reserved && tooPtr->reserved != pikPtr) {
        //Another Pikmin is already going for it. Ignore it.
        return;
    }
    if(!pikPtr->pikType->canCarryTools) {
        //This Pikmin can't carry tools. Forget it.
        return;
    }
    if(!hasFlag(tooPtr->holdabilityFlags, HOLDABILITY_FLAG_PIKMIN)) {
        //Can't hold this. Forget it.
        return;
    }
    
    tooPtr->reserved = pikPtr;
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pikPtr->leaveGroup();
    pikPtr->stopChasing();
    
    scriptVM->focusOnMob(tooPtr);
    
    pikPtr->chase(
        &tooPtr->pos, &tooPtr->z,
        Point(), 0.0f, 0,
        pikPtr->radius + tooPtr->radius
    );
    scriptVM->setTimer(PIKMIN::GOTO_TIMEOUT);
    
    pikPtr->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_RANDOM_TIME,
        true, pikPtr->type->moveSpeed
    );
    
    scriptVM->fsm.setState(PIKMIN_STATE_GOING_TO_TOOL);
    
}


/**
 * @brief When a thrown Pikmin lands.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::land(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    PikminFsm::standStill(scriptVM, nullptr, nullptr);
    pikPtr->setAnimation(PIKMIN_ANIM_IDLING);
}


/**
 * @brief When a Pikmin lands after being thrown from a pluck.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void PikminFsm::landAfterPluck(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Mob* leaPtr = scriptVM->focusedMob;
    
    pikPtr->setAnimation(PIKMIN_ANIM_IDLING);
    
    if(leaPtr) {
        if(leaPtr->followingGroup) {
            //If this leader is following another one,
            //then the new Pikmin should be in the group of that top leader.
            leaPtr = leaPtr->followingGroup;
        }
        leaPtr->addToGroup(pikPtr);
        scriptVM->fsm.setState(PIKMIN_STATE_IN_GROUP_CHASING, info1, info2);
    } else {
        scriptVM->fsm.setState(PIKMIN_STATE_IDLING, info1, info2);
    }
}


/**
 * @brief When a thrown Pikmin lands on a mob, to latch on to it.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void PikminFsm::landOnMob(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    Mob* m2Ptr = info->mob2;
    
    FsmEventDef* m2PikLandEv =
        m2Ptr->scriptVM.fsm.getEvent(MOB_EV_THROWN_PIKMIN_LANDED);
        
    if(m2PikLandEv && hasFlag(pikPtr->flags, MOB_FLAG_WAS_THROWN)) {
        m2PikLandEv->run(&m2Ptr->scriptVM, (void*) pikPtr);
    }
    
    if(!pikPtr->canHurt(m2Ptr)) return;
    
    Hitbox* hboxPtr = info->h2;
    
    if(
        !hboxPtr ||
        (
            pikPtr->pikType->attackMethod == PIKMIN_ATTACK_LATCH &&
            hboxPtr->surfaceType != HITBOX_SURFACE_TYPE_LATCHABLE
        )
    ) {
        if(hboxPtr->surfaceType == HITBOX_SURFACE_TYPE_BOUNCY) {
            //No good. Make it bounce back.
            float throwAngle = getAngle(pikPtr->speed);
            throwAngle += TAU / 2.0f;
            pikPtr->speed = angleToCoordinates(throwAngle, 200.0f);
        } else {
            //No good. Make it stop.
            pikPtr->speed.x = 0.0f;
            pikPtr->speed.y = 0.0f;
        }
        return;
    }
    
    pikPtr->stopHeightEffect();
    scriptVM->focusedMob = m2Ptr;
    disableFlag(pikPtr->flags, MOB_FLAG_WAS_THROWN);
    
    switch(pikPtr->pikType->attackMethod) {
    case PIKMIN_ATTACK_LATCH: {
        pikPtr->latch(m2Ptr, hboxPtr);
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {
        pikPtr->speed.x = pikPtr->speed.y = pikPtr->speedZ = 0;
        break;
        
    }
    }
    
    scriptVM->fsm.setState(PIKMIN_STATE_MOB_LANDING);
}


/**
 * @brief When a thrown Pikmin lands on a mob, whilst holding something.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void PikminFsm::landOnMobWhileHolding(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    Mob* m2Ptr = info->mob2;
    Tool* tooPtr = (Tool*) pikPtr->getMobHeldInHand();
    
    if(!tooPtr) return;
    if(!pikPtr->canHurt(m2Ptr)) return;
    
    FsmEventDef* m2PikLandEv =
        m2Ptr->scriptVM.fsm.getEvent(MOB_EV_THROWN_PIKMIN_LANDED);
        
    if(m2PikLandEv && hasFlag(pikPtr->flags, MOB_FLAG_WAS_THROWN)) {
        m2PikLandEv->run(&m2Ptr->scriptVM, (void*) pikPtr);
    }
    
    disableFlag(pikPtr->flags, MOB_FLAG_WAS_THROWN);
    
    if(tooPtr->tooType->droppedWhenPikminLandsOnOpponent) {
        PikminFsm::releaseTool(scriptVM, nullptr, nullptr);
        scriptVM->fsm.setState(PIKMIN_STATE_IDLING);
        
        if(tooPtr->tooType->stuckWhenPikminLandsOnOpponent && info->h2) {
            tooPtr->speed.x = tooPtr->speed.y = tooPtr->speedZ = 0;
            tooPtr->stopHeightEffect();
            
            tooPtr->scriptVM.focusedMob = m2Ptr;
            
            float hOffsetDist;
            float hOffsetAngle;
            float vOffsetDist;
            m2Ptr->getHitboxHoldPoint(
                tooPtr, info->h2,
                &hOffsetDist, &hOffsetAngle, &vOffsetDist
            );
            m2Ptr->hold(
                tooPtr, HOLD_TYPE_LATCH, info->h2->bodyPartIdx,
                hOffsetDist, hOffsetAngle, vOffsetDist,
                true, HOLD_ROTATION_METHOD_FACE_HOLDER
            );
        }
        
        Distance closestLeaderDist;
        Leader* closestLeader = nullptr;
        forIdx(l, game.states.gameplay->mobs.leaders) {
            Leader* lPtr = game.states.gameplay->mobs.leaders[l];
            if(!lPtr->player) continue;
            if(!lPtr->isViableLeader(pikPtr)) continue;
            Distance d(pikPtr->pos, lPtr->pos);
            if(!closestLeader || d < closestLeaderDist) {
                closestLeaderDist = d;
                closestLeader = lPtr;
            }
        }
        
        if(tooPtr->tooType->pikminReturnsAfterUsing && closestLeader) {
            if(pikPtr->getMobHeldInHand()) {
                scriptVM->fsm.setState(PIKMIN_STATE_CALLED_H, closestLeader);
            } else {
                scriptVM->fsm.setState(PIKMIN_STATE_CALLED, closestLeader);
            }
        }
    }
}


/**
 * @brief When a thrown Pikmin lands while holding something.
 * Depending on what it is, it might drop it.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::landWhileHolding(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Tool* tooPtr = (Tool*) pikPtr->getMobHeldInHand();
    
    if(!tooPtr) return;
    
    PikminFsm::standStill(scriptVM, nullptr, nullptr);
    
    pikPtr->isToolPrimedForWhistle = true;
    
    pikPtr->setAnimation(PIKMIN_ANIM_IDLING);
    
    if(tooPtr->tooType->droppedWhenPikminLands) {
        PikminFsm::releaseTool(scriptVM, nullptr, nullptr);
        scriptVM->fsm.setState(PIKMIN_STATE_IDLING);
        
        Distance closestLeaderDist;
        Leader* closestLeader = nullptr;
        forIdx(l, game.states.gameplay->mobs.leaders) {
            Leader* lPtr = game.states.gameplay->mobs.leaders[l];
            if(!lPtr->player) continue;
            if(!lPtr->isViableLeader(pikPtr)) continue;
            Distance d(pikPtr->pos, lPtr->pos);
            if(!closestLeader || d < closestLeaderDist) {
                closestLeaderDist = d;
                closestLeader = lPtr;
            }
        }
        
        if(tooPtr->tooType->pikminReturnsAfterUsing && closestLeader) {
            if(pikPtr->getMobHeldInHand()) {
                scriptVM->fsm.setState(PIKMIN_STATE_CALLED_H, closestLeader);
            } else {
                scriptVM->fsm.setState(PIKMIN_STATE_CALLED, closestLeader);
            }
        }
    } else {
        scriptVM->fsm.setState(PIKMIN_STATE_IDLING_H);
    }
}


/**
 * @brief When a Pikmin leaves its Onion because it got called out.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Points to the Onion.
 * @param info2 Unused.
 */
void PikminFsm::leaveOnion(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pikPtr->setAnimation(PIKMIN_ANIM_SLIDING, START_ANIM_OPTION_RANDOM_TIME);
}


/**
 * @brief When a Pikmin leaves a hazardous sector.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Points to the hazard.
 * @param info2 Unused.
 */
void PikminFsm::leftHazard(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Hazard* h = (Hazard*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    if(h->associatedLiquid) {
        pikPtr->deleteParticleGenerator(MOB_PARTICLE_GENERATOR_ID_WAVE_RING);
    }
}


/**
 * @brief When the mob the Pikmin is latched on to disappears.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::loseLatchedMob(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->stopChasing();
}


/**
 * @brief When a Pikmin notifies the leader that it must gently release it.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::notifyLeaderRelease(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(!pikPtr->followingGroup) return;
    if(pikPtr->holder.m != pikPtr->followingGroup) return;
    pikPtr->followingGroup->scriptVM.fsm.runEvent(MOB_EV_RELEASE_ORDER);
}


/**
 * @brief When a Pikmin needs to decide a new spot to run off to whilst
 * in panicking.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::panicNewChase(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->chase(
        Point(
            pikPtr->pos.x + game.rng.f(-1000, 1000),
            pikPtr->pos.y + game.rng.f(-1000, 1000)
        ),
        pikPtr->z
    );
    scriptVM->setTimer(PIKMIN::PANIC_CHASE_INTERVAL);
}


/**
 * @brief When a Pikmin is meant to reel back to unleash an attack.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::prepareToAttack(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(
        scriptVM->focusedMob != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    if(scriptVM->focusedMob->rectangularDim.x != 0.0f) {
        bool isInside = false;
        Point target =
            getClosestPointInRotatedRectangle(
                pikPtr->pos,
                scriptVM->focusedMob->pos,
                scriptVM->focusedMob->rectangularDim,
                scriptVM->focusedMob->angle,
                &isInside
            );
        pikPtr->face(getAngle(pikPtr->pos, target), nullptr);
        
    } else {
        pikPtr->face(0, &scriptVM->focusedMob->pos);
        
    }
    
    pikPtr->setAnimation(PIKMIN_ANIM_ATTACKING);
}


/**
 * @brief When a Pikmin reaches its spot on a carriable object.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::reachCarriableObject(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Mob* carriableMob = pikPtr->carryingMob;
    
    Point spotOffset =
        rotatePoint(
            carriableMob->carryInfo->spotInfo[pikPtr->tempI].pos,
            carriableMob->angle
        );
    Point finalPos = carriableMob->pos + spotOffset;
    
    pikPtr->chase(
        &carriableMob->pos, &carriableMob->z,
        spotOffset, 0.0f,
        CHASE_FLAG_TELEPORT |
        CHASE_FLAG_TELEPORTS_CONSTANTLY
    );
    
    pikPtr->face(getAngle(finalPos, carriableMob->pos), nullptr);
    
    //Let the carriable mob know that a new Pikmin has grabbed on.
    pikPtr->carryingMob->scriptVM.fsm.runEvent(
        MOB_EV_CARRIER_ADDED, (void*) pikPtr
    );
    
    pikPtr->inCarryStruggleAnimation = false;
    pikPtr->setAnimation(PIKMIN_ANIM_CARRYING);
    pikPtr->playSound(
        pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_CARRYING_GRAB]
    );
}


/**
 * @brief When a Pikmin reaches its dismissal spot.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::reachDismissSpot(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->stopChasing();
    pikPtr->setAnimation(PIKMIN_ANIM_IDLING);
}


/**
 * @brief When a Pikmin that just attacked an opponent needs to walk
 * towards it again.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::rechaseOpponent(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(pikPtr->wasLastHitDing) {
        //Check if the Pikmin's last hits were dings.
        //If so, maybe give up and sigh.
        pikPtr->consecutiveDings++;
        if(pikPtr->consecutiveDings >= 4) {
            pikPtr->wasLastHitDing = false;
            pikPtr->consecutiveDings = 0;
            scriptVM->fsm.setState(PIKMIN_STATE_SIGHING);
            return;
        }
    }
    
    bool canContinueAttacking =
        scriptVM->focusedMob &&
        scriptVM->focusedMob->health > 0 &&
        Distance(pikPtr->pos, scriptVM->focusedMob->pos) <=
        (
            pikPtr->radius + scriptVM->focusedMob->radius +
            PIKMIN::GROUNDED_ATTACK_DIST
        );
        
    if(!canContinueAttacking) {
        //The opponent cannot be chased down. Become idle.
        scriptVM->fsm.setState(PIKMIN_STATE_IDLING);
        
    } else if(
        game.rng.f(0.0f, 1.0f) <= PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED
    ) {
        //Circle around it a bit before attacking from a new angle.
        scriptVM->fsm.setState(PIKMIN_STATE_CIRCLING_OPPONENT);
        
    } else {
        //If the opponent is alive and within reach, let's stay in this state,
        //and attack some more!
        return;
        
    }
}


/**
 * @brief When a Pikmin is meant to release the tool it is currently holding.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 If nullptr, release as normal.
 * Otherwise, this is a "gentle" release.
 * @param info2 Unused.
 */
void PikminFsm::releaseTool(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Mob* tooPtr = pikPtr->getMobHeldInHand();
    
    if(!tooPtr) return;
    
    if(info1) {
        tooPtr->scriptVM.setVar("gentle_release", "true");
    } else {
        tooPtr->scriptVM.setVar("gentle_release", "false");
    }
    pikPtr->release(tooPtr);
    tooPtr->pos = pikPtr->pos;
    tooPtr->speed = Point();
    tooPtr->pushAmount = 0.0f;
    pikPtr->subgroupTypePtr =
        game.states.gameplay->subgroupTypes.getType(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, pikPtr->pikType
        );
    if(pikPtr->followingGroup) {
        pikPtr->followingGroup->group->changeStandbyTypeIfNeeded();
        if(pikPtr->followingGroup->type->category->id == MOB_CATEGORY_LEADERS) {
            Leader* leaPtr = (Leader*) pikPtr->followingGroup;
            if(leaPtr->player) {
                game.states.gameplay->updateClosestGroupMembers(leaPtr->player);
            }
        }
    }
}


/**
 * @brief When a Pikmin seed lands on the ground.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::seedLanded(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    //Clear the seed sparkles.
    pikPtr->particleGenerators.clear();
    
    //Generate the rock particles that come out.
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parPikminSeedLanded, pikPtr
        );
    pikPtr->particleGenerators.push_back(pg);
    
    //Play the sound.
    pikPtr->playSound(
        pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_SEED_LANDING]
    );
}


/**
 * @brief When a Pikmin is meant to set its timer for the bump lock.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::setBumpLock(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->bumpLock = game.config.pikmin.idleBumpDelay;
}


/**
 * @brief When a Pikmin is meant to change "reach" to the idle task reach.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::setIdleTaskReach(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->nearReach = 0;
    pikPtr->updateInteractionSpan();
}


/**
 * @brief When a Pikmin is meant to change "reach" to the swarm reach.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::setSwarmReach(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->nearReach = 1;
    pikPtr->updateInteractionSpan();
}


/**
 * @brief When a Pikmin is meant to sigh.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::sigh(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->setAnimation(PIKMIN_ANIM_SIGHING);
}


/**
 * @brief Causes a sprout to evolve.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::sproutEvolve(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(pikPtr->maturity == 0 || pikPtr->maturity == 1) {
        //Leaf to bud, or bud to flower.
        pikPtr->increaseMaturity(1);
    } else {
        //Flower to leaf.
        pikPtr->increaseMaturity(-2);
    }
}


/**
 * @brief Schedules the next evolution for a sprout.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::sproutScheduleEvol(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    scriptVM->setTimer(pikPtr->pikType->sproutEvolutionTime[pikPtr->maturity]);
}


/**
 * @brief When a Pikmin is meant to stand still in place.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::standStill(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->stopCircling();
    pikPtr->stopFollowingPath();
    pikPtr->stopChasing();
    pikPtr->stopTurning();
    pikPtr->speed.x = pikPtr->speed.y = 0;
}


/**
 * @brief When a Pikmin should start a random boredom animation.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startBoredomAnim(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    size_t lookingAroundAnimIdx =
        pikPtr->type->animDb->findAnimation("looking_around");
    size_t sittingAnimIdx =
        pikPtr->type->animDb->findAnimation("sitting");
    size_t loungingAnimIdx =
        pikPtr->type->animDb->findAnimation("lounging");
    vector<size_t> boredomAnims;
    if(lookingAroundAnimIdx != INVALID) {
        boredomAnims.push_back(lookingAroundAnimIdx);
    }
    if(sittingAnimIdx != INVALID) {
        boredomAnims.push_back(sittingAnimIdx);
    }
    if(loungingAnimIdx != INVALID) {
        boredomAnims.push_back(loungingAnimIdx);
    }
    
    if(boredomAnims.empty()) return;
    size_t animIdx =
        boredomAnims[game.rng.i(0, (int) (boredomAnims.size() - 1))];
    pikPtr->setAnimation(animIdx, START_ANIM_OPTION_NORMAL, false);
    pikPtr->inBoredAnimation = true;
}


/**
 * @brief When a Pikmin needs to start chasing after its leader
 * (or the group spot belonging to the leader).
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Points to the position struct with the final destination.
 *   If nullptr, the final destination is calculated in this function.
 * @param info2 Unused.
 */
void PikminFsm::startChasingLeader(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    scriptVM->focusOnMob(pikPtr->followingGroup);
    PikminFsm::updateInGroupChasing(scriptVM, nullptr, nullptr);
    pikPtr->setAnimation(
        pikPtr->getMobHeldInHand() ?
        PIKMIN_ANIM_CARRYING_LIGHT :
        PIKMIN_ANIM_WALKING,
        START_ANIM_OPTION_RANDOM_TIME, true, pikPtr->type->moveSpeed
    );
}


/**
 * @brief When a Pikmin starts drinking the drop it touched.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the drop mob.
 * @param info2 Unused.
 */
void PikminFsm::startDrinking(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Mob* droPtr = (Mob*) info1;
    
    pikPtr->leaveGroup();
    pikPtr->stopChasing();
    scriptVM->focusOnMob(droPtr);
    pikPtr->face(getAngle(pikPtr->pos, droPtr->pos), nullptr);
    pikPtr->setAnimation(PIKMIN_ANIM_DRINKING);
}


/**
 * @brief When a Pikmin starts dying.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startDying(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->startDying();
    
    pikPtr->leaveGroup();
    PikminFsm::beReleased(scriptVM, info1, info2);
    PikminFsm::notifyLeaderRelease(scriptVM, info1, info2);
    PikminFsm::releaseTool(scriptVM, nullptr, nullptr);
    pikPtr->setAnimation(PIKMIN_ANIM_DYING);
}


/**
 * @brief When a Pikmin starts flailing.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startFlailing(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    PikminFsm::releaseTool(scriptVM, nullptr, nullptr);
    
    //If the Pikmin is following a moveable point, let's change it to
    //a static point. This will make the Pikmin continue to move
    //forward into the water in a straight line.
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    float finalZ = 0.0f;
    Point finalPos = pikPtr->getChaseTarget(&finalZ);
    pikPtr->chase(finalPos, finalZ);
    
    pikPtr->leaveGroup();
    
    //Let the Pikmin continue to swim into the water for a bit
    //before coming to a stop. Otherwise the Pikmin would stop nearly
    //on the edge of the water, and that just looks bad.
    scriptVM->setTimer(1.0f);
    
    pikPtr->setAnimation(PIKMIN_ANIM_FLAILING, START_ANIM_OPTION_RANDOM_TIME);
    pikPtr->playSound(pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_SUFFERING]);
}


/**
 * @brief When a Pikmin starts getting up from being knocked down.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startGettingUp(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pikPtr->setAnimation(PIKMIN_ANIM_GETTING_UP);
}


/**
 * @brief When a Pikmin starts lunging forward for an impact attack.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startImpactLunge(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(
        scriptVM->focusedMob != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    pikPtr->chase(&scriptVM->focusedMob->pos, &scriptVM->focusedMob->z);
    pikPtr->setAnimation(PIKMIN_ANIM_ATTACKING);
}


/**
 * @brief When a Pikmin is killed after being knocked down.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startKnockedDownDying(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    PikminFsm::startDying(scriptVM, info1, info2);
    pikPtr->setAnimation(PIKMIN_ANIM_KNOCKED_DOWN_DYING);
}


/**
 * @brief When a Pikmin lands on a mob and needs to start its landing animation.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startMobLanding(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->setAnimation(PIKMIN_ANIM_MOB_LANDING);
}


/**
 * @brief When a Pikmin starts panicking.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startPanicking(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    pikPtr->leaveGroup();
    PikminFsm::panicNewChase(scriptVM, info1, info2);
    pikPtr->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_RANDOM_TIME,
        true, pikPtr->type->moveSpeed
    );
    pikPtr->playSound(pikPtr->pikType->soundDataIdxs[PIKMIN_SOUND_SUFFERING]);
}


/**
 * @brief When a Pikmin starts picking some object up to hold it.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startPickingUp(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->stopChasing();
    pikPtr->setAnimation(PIKMIN_ANIM_PICKING_UP);
}


/**
 * @brief When a Pikmin must start returning to the carried object's
 * return point.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the mob that used to be carried.
 * @param info2 Unused.
 */
void PikminFsm::startReturning(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Mob* carriedMob = (Mob*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    PathFollowSettings settings;
    settings.targetPoint = carriedMob->carryInfo->returnPoint;
    settings.finalTargetDistance = carriedMob->carryInfo->returnDist;
    
    if(carriedMob->carryInfo->destination == CARRY_DESTINATION_LINKED_MOB) {
        //Special case: bridges.
        //Pikmin are meant to carry to the current tip of the bridge,
        //but whereas the start of the bridge is on firm ground, the tip may
        //be above a chasm or water, so the Pikmin might want to take a
        //different path, or be unable to take a path at all.
        //Let's fake the start point to be the start of the bridge,
        //for the sake of path calculations.
        if(
            carriedMob->carryInfo->intendedMob->type->category->id ==
            MOB_CATEGORY_BRIDGES
        ) {
            Bridge* briPtr = (Bridge*) carriedMob->carryInfo->intendedMob;
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FAKED_START);
            settings.fakedStart = briPtr->getStartPoint();
        }
    }
    
    if(
        pikPtr->followPath(
            settings, pikPtr->getBaseSpeed(), pikPtr->type->acceleration
        )
    ) {
        pikPtr->setAnimation(
            PIKMIN_ANIM_WALKING, START_ANIM_OPTION_RANDOM_TIME, true,
            pikPtr->type->moveSpeed
        );
    } else {
        scriptVM->fsm.setState(PIKMIN_STATE_IDLING);
    }
}


/**
 * @brief When a Pikmin starts riding on a track.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Points to the track mob.
 * @param info2 Unused.
 */
void PikminFsm::startRidingTrack(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Track* traPtr = (Track*) info1;
    
    disableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pikPtr->leaveGroup();
    pikPtr->stopChasing();
    scriptVM->focusOnMob(traPtr);
    pikPtr->startHeightEffect();
    
    vector<size_t> checkpoints;
    forIdx(c, traPtr->type->animDb->bodyParts) {
        checkpoints.push_back(c);
    }
    pikPtr->trackInfo =
        new TrackRideInfo(
        traPtr, checkpoints, traPtr->traType->rideSpeed
    );
    
    switch(traPtr->traType->ridingPose) {
    case TRACK_RIDING_POSE_STOPPED: {
        pikPtr->setAnimation(
            PIKMIN_ANIM_WALKING, START_ANIM_OPTION_RANDOM_TIME
        );
        break;
    } case TRACK_RIDING_POSE_CLIMBING: {
        pikPtr->setAnimation(
            PIKMIN_ANIM_CLIMBING, START_ANIM_OPTION_RANDOM_TIME
        );
        break;
    } case TRACK_RIDING_POSE_SLIDING: {
        pikPtr->setAnimation(
            PIKMIN_ANIM_SLIDING, START_ANIM_OPTION_RANDOM_TIME
        );
        break;
    }
    }
}


/**
 * @brief When a Pikmin must start emitting seed particles.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::startSeedParticles(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parPikminSeed, pikPtr
        );
    adjustKeyframeInterpolatorValues<ALLEGRO_COLOR>(
        pg.baseParticle.color,
    [ = ] (const ALLEGRO_COLOR & c) {
        ALLEGRO_COLOR newColor = c;
        newColor.r *= pikPtr->type->mainColor.r;
        newColor.g *= pikPtr->type->mainColor.g;
        newColor.b *= pikPtr->type->mainColor.b;
        newColor.a *= pikPtr->type->mainColor.a;
        return newColor;
    }
    );
    pikPtr->particleGenerators.push_back(pg);
}


/**
 * @brief When a Pikmin must no longer be idling.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::stopBeingIdle(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->bumpLock = 0.0f;
    pikPtr->inShakingAnimation = false;
}


/**
 * @brief When a Pikmin is no longer in the thrown state.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::stopBeingThrown(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->deleteParticleGenerator(MOB_PARTICLE_GENERATOR_ID_THROW);
}


/**
 * @brief When a Pikmin is meant to release an object it is carrying.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::stopCarrying(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(!pikPtr->carryingMob) return;
    
    pikPtr->carryingMob->scriptVM.fsm.runEvent(
        MOB_EV_CARRIER_REMOVED, (void*) pikPtr
    );
    
    pikPtr->carryingMob = nullptr;
    scriptVM->setTimer(0);
}


/**
 * @brief When a Pikmin stands still while in a leader's group.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::stopInGroup(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    pikPtr->stopChasing();
    pikPtr->face(0, &pikPtr->followingGroup->pos);
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pikPtr->setAnimation(PIKMIN_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME);
    scriptVM->setTimer(
        game.rng.f(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin has to choose its carrying animation.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::tickCarrying(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(
        pikPtr->inCarryStruggleAnimation &&
        pikPtr->carryingMob->carryInfo->isMoving
    ) {
        pikPtr->inCarryStruggleAnimation = false;
        pikPtr->setAnimation(
            PIKMIN_ANIM_CARRYING, START_ANIM_OPTION_RANDOM_TIME
        );
        
    } else if(
        !pikPtr->inCarryStruggleAnimation &&
        !pikPtr->carryingMob->carryInfo->isMoving
    ) {
        pikPtr->inCarryStruggleAnimation = true;
        pikPtr->setAnimation(
            PIKMIN_ANIM_CARRYING_STRUGGLE, START_ANIM_OPTION_RANDOM_TIME
        );
    }
}


/**
 * @brief When a Pikmin has to teleport to its spot in the Onion leg.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::tickEnteringOnion(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(
        pikPtr->trackInfo != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    engineAssert(
        scriptVM->focusedMob != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    if(pikPtr->tickTrackRide()) {
        //Finished!
        ((Onion*) scriptVM->focusedMob)->nest->storePikmin(pikPtr);
    }
}


/**
 * @brief When a Pikmin has to teleport to its spot in a group task.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::tickGroupTaskWork(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(
        scriptVM->focusedMob != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    GroupTask* tasPtr = (GroupTask*) (scriptVM->focusedMob);
    Point curSpotPos = tasPtr->getSpotPos(pikPtr);
    float curSpotZ = tasPtr->z + tasPtr->tasType->spotsZ;
    
    pikPtr->chase(
        curSpotPos, curSpotZ,
        CHASE_FLAG_TELEPORT |
        CHASE_FLAG_TELEPORTS_CONSTANTLY
    );
    pikPtr->face(
        tasPtr->angle + tasPtr->tasType->workerPikminAngle, nullptr, true
    );
    pikPtr->stopTurning();
}


/**
 * @brief When a Pikmin has to teleport to its spot in a track it is riding.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::tickTrackRide(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(
        pikPtr->trackInfo != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    if(pikPtr->tickTrackRide()) {
        //Finished!
        scriptVM->fsm.setState(PIKMIN_STATE_IDLING, nullptr, nullptr);
        if(
            pikPtr->leaderToReturnTo &&
            pikPtr->leaderToReturnTo->isViableLeader(pikPtr)
        ) {
            if(pikPtr->getMobHeldInHand()) {
                scriptVM->fsm.setState(
                    PIKMIN_STATE_CALLED_H, pikPtr->leaderToReturnTo, info2
                );
            } else {
                scriptVM->fsm.setState(
                    PIKMIN_STATE_CALLED, pikPtr->leaderToReturnTo, info2
                );
            }
        }
    }
}


/**
 * @brief When a Pikmin touches a "eat" hitbox.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::touchedEatHitbox(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    engineAssert(info2 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    if(pikPtr->invulnPeriod.timeLeft > 0) return;
    if(pikPtr->health <= 0) {
        return;
    }
    
    forIdx(s, pikPtr->statuses) {
        if(pikPtr->statuses[s].state != STATUS_STATE_ACTIVE) continue;
        if(pikPtr->statuses[s].type->turnsInedible) {
            return;
        }
    }
    
    scriptVM->fsm.setState(PIKMIN_STATE_GRABBED_BY_ENEMY, info1, info2);
}


/**
 * @brief When a Pikmin touches a hazard.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the hazard type.
 * @param info2 Pointer to the hitbox that caused this, if any.
 */
void PikminFsm::touchedHazard(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    Hazard* hazPtr = (Hazard*) info1;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    Mob* hitboxMob = nullptr;
    
    if(info2) {
        //This is an attack.
        HitboxInteraction* hInfo = (HitboxInteraction*) info2;
        hitboxMob = hInfo->mob2;
        if(!pikPtr->processAttackMiss(hInfo)) {
            //It has been decided that this attack missed.
            return;
        }
    }
    
    if(hazPtr->associatedLiquid) {
        bool alreadyGenerating = false;
        forIdx(g, pikPtr->particleGenerators) {
            if(
                pikPtr->particleGenerators[g].id ==
                MOB_PARTICLE_GENERATOR_ID_WAVE_RING
            ) {
                alreadyGenerating = true;
                break;
            }
        }
        
        if(!alreadyGenerating) {
            ParticleGenerator pg =
                standardParticleGenSetup(
                    game.sysContentNames.parWaveRing, pikPtr
                );
            pg.followZOffset = 1.0f;
            adjustKeyframeInterpolatorValues<float>(
                pg.baseParticle.size,
            [ = ] (const float & f) { return f * pikPtr->radius; }
            );
            pg.id = MOB_PARTICLE_GENERATOR_ID_WAVE_RING;
            pikPtr->particleGenerators.push_back(pg);
        }
    }
    
    if(pikPtr->invulnPeriod.timeLeft > 0) return;
    MobType::Vulnerability vuln = pikPtr->getHazardVulnerability(hazPtr);
    if(vuln.effectMult == 0.0f) return;
    
    if(!vuln.statusToApply || !vuln.statusOverrides) {
        forIdx(e, hazPtr->effects) {
            pikPtr->applyStatus(hazPtr->effects[e], false, true, hitboxMob);
        }
    }
    if(vuln.statusToApply) {
        pikPtr->applyStatus(vuln.statusToApply, false, true, hitboxMob);
    }
}


/**
 * @brief When a Pikmin is sprayed.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the spray type.
 * @param info2 Pointer to the mob that sprayed, if any.
 */
void PikminFsm::touchedSpray(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    SprayType* s = (SprayType*) info1;
    Mob* sprayer = (Mob*) info2;
    
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    forIdx(e, s->effects) {
        pikPtr->applyStatus(s->effects[e], false, false, sprayer);
    }
    
    if(s->buriesPikmin) {
        scriptVM->fsm.setState(PIKMIN_STATE_SPROUT, nullptr, nullptr);
    }
}


/**
 * @brief When the Pikmin gets grabbed by an enemy. It should try to swap places
 * with the object that it is holding, instead, if possible.
 * If not, it should drop the object and get grabbed like normal.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::tryHeldItemHotswap(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Tool* tooPtr = (Tool*) pikPtr->getMobHeldInHand();
    if(!tooPtr) return;
    
    if(
        !tooPtr->tooType->canBeHotswapped &&
        hasFlag(tooPtr->holdabilityFlags, HOLDABILITY_FLAG_ENEMIES)
    ) {
        //This tool can't be hotswapped... The Pikmin has to get chomped.
        PikminFsm::releaseTool(scriptVM, nullptr, nullptr);
        scriptVM->fsm.setState(PIKMIN_STATE_GRABBED_BY_ENEMY);
        return;
    }
    
    //Start by dropping the tool.
    PikminFsm::releaseTool(scriptVM, nullptr, nullptr);
    //Receive some invulnerability period to make sure it's not hurt by
    //the same attack.
    pikPtr->invulnPeriod.start();
    //Finally, get knocked back on purpose.
    pikPtr->leaveGroup();
    PikminFsm::beReleased(scriptVM, info1, info2);
    PikminFsm::notifyLeaderRelease(scriptVM, info1, info2);
    scriptVM->fsm.setState(PIKMIN_STATE_KNOCKED_BACK);
}


/**
 * @brief When the Pikmin stops latching on to an enemy.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::unlatch(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    if(!scriptVM->focusedMob) return;
    
    scriptVM->focusedMob->release(pikPtr);
    pikPtr->latched = false;
}


/**
 * @brief When the Pikmin should update its destination when chasing the leader.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Points to the position struct with the final destination.
 *   If nullptr, the final destination is calculated in this function.
 * @param info2 Unused.
 */
void PikminFsm::updateInGroupChasing(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Point targetPos;
    float targetDist; //Unused dummy value.
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    if(!info1) {
        pikPtr->getGroupSpotInfo(&targetPos, &targetDist);
    } else {
        targetPos = *((Point*) info1);
    }
    
    float targetZ = pikPtr->followingGroup->z;
    if(pikPtr->pikType->canFly) {
        targetZ += PIKMIN::FLIER_ABOVE_FLOOR_HEIGHT;
    }
    
    pikPtr->chase(targetPos, targetZ);
    
}


/**
 * @brief When a Pikmin is whistled over by a leader while holding a tool.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void PikminFsm::whistledWhileHolding(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    Tool* tooPtr = (Tool*) pikPtr->getMobHeldInHand();
    
    if(!tooPtr) return;
    
    if(
        tooPtr->tooType->droppedWhenPikminIsWhistled &&
        pikPtr->isToolPrimedForWhistle
    ) {
        PikminFsm::releaseTool(scriptVM, nullptr, nullptr);
    }
    
    pikPtr->isToolPrimedForWhistle = false;
    
    if(pikPtr->getMobHeldInHand()) {
        scriptVM->fsm.setState(PIKMIN_STATE_CALLED_H, info1, info2);
    } else {
        scriptVM->fsm.setState(PIKMIN_STATE_CALLED, info1, info2);
    }
    
}


/**
 * @brief When a Pikmin is whistled over by a leader while riding on a track.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void PikminFsm::whistledWhileRiding(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(pikPtr->trackInfo, scriptVM->fsm.getStateHistoryStr());
    
    Track* traPtr = (Track*) (pikPtr->trackInfo->m);
    
    if(traPtr->traType->cancellableWithWhistle) {
        pikPtr->stopTrackRide();
        if(pikPtr->getMobHeldInHand()) {
            scriptVM->fsm.setState(PIKMIN_STATE_CALLED_H, info1, info2);
        } else {
            scriptVM->fsm.setState(PIKMIN_STATE_CALLED, info1, info2);
        }
    }
}


/**
 * @brief When the Pikmin should start working on a group task.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void PikminFsm::workOnGroupTask(ScriptVM* scriptVM, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) scriptVM->mob;
    
    engineAssert(
        scriptVM->focusedMob != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    GroupTask* tasPtr = (GroupTask*) (scriptVM->focusedMob);
    
    if(pikPtr->pikType->canFly) {
        enableFlag(pikPtr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    tasPtr->addWorker(pikPtr);
    
    pikPtr->stopChasing();
    pikPtr->face(
        tasPtr->angle + tasPtr->tasType->workerPikminAngle,
        nullptr
    );
    
    switch(tasPtr->tasType->workerPikminPose) {
    case GROUP_TASK_PIKMIN_POSE_STOPPED: {
        pikPtr->setAnimation(
            PIKMIN_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME
        );
        break;
    } case GROUP_TASK_PIKMIN_POSE_ARMS_OUT: {
        pikPtr->setAnimation(
            PIKMIN_ANIM_ARMS_OUT, START_ANIM_OPTION_RANDOM_TIME
        );
        break;
    } case GROUP_TASK_PIKMIN_POSE_PUSHING: {
        pikPtr->setAnimation(
            PIKMIN_ANIM_PUSHING, START_ANIM_OPTION_RANDOM_TIME
        );
        break;
    } case GROUP_TASK_PIKMIN_POSE_CARRYING: {
        pikPtr->setAnimation(
            PIKMIN_ANIM_CARRYING, START_ANIM_OPTION_RANDOM_TIME
        );
        break;
    } case GROUP_TASK_PIKMIN_POSE_CARRYING_LIGHT: {
        pikPtr->setAnimation(
            PIKMIN_ANIM_CARRYING_LIGHT, START_ANIM_OPTION_RANDOM_TIME
        );
        break;
    }
    }
}


#pragma endregion
