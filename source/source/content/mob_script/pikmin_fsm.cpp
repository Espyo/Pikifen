/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin finite state machine logic.
 */

#include <algorithm>

#include "pikmin_fsm.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
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


/**
 * @brief Creates the finite state machine for the Pikmin's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void pikmin_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("seed", PIKMIN_STATE_SEED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::becomeSprout);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::seedLanded);
            efc.changeState("sprout");
        }
    }
    
    efc.newState("sprout", PIKMIN_STATE_SPROUT); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::standStill);
            efc.run(pikmin_fsm::becomeSprout);
            efc.run(pikmin_fsm::sproutScheduleEvol);
        }
        efc.newEvent(MOB_EV_PLUCKED); {
            efc.changeState("plucking");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::sproutEvolve);
            efc.run(pikmin_fsm::sproutScheduleEvol);
        }
    }
    
    efc.newState("plucking", PIKMIN_STATE_PLUCKING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::beginPluck);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("plucking_thrown");
        }
    }
    
    efc.newState("plucking_thrown", PIKMIN_STATE_PLUCKING_THROWN); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::beThrownAfterPluck);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::landAfterPluck);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
    }
    
    efc.newState("leaving_onion", PIKMIN_STATE_LEAVING_ONION); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::leaveOnion);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tickTrackRide);
        }
    }
    
    efc.newState("entering_onion", PIKMIN_STATE_ENTERING_ONION); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::enterOnion);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tickEnteringOnion);
        }
    }
    
    efc.newState("in_group_chasing", PIKMIN_STATE_IN_GROUP_CHASING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startChasingLeader);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.run(pikmin_fsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("in_group_stopped");
        }
        efc.newEvent(MOB_EV_SWARM_STARTED); {
            efc.changeState("swarm_chasing");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::beDismissed);
            efc.changeState("goingToDismissSpot");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("in_group_stopped", PIKMIN_STATE_IN_GROUP_STOPPED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::stopInGroup);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::clearBoredomData);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::beGrabbedByFriend);
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
            efc.run(pikmin_fsm::beDismissed);
            efc.changeState("goingToDismissSpot");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::startBoredomAnim);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::checkBoredomAnimEnd);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("swarm_chasing", PIKMIN_STATE_SWARM_CHASING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::setSwarmReach);
            efc.run(pikmin_fsm::startChasingLeader);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::setIdleTaskReach);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("swarm_stopped");
        }
        efc.newEvent(MOB_EV_SWARM_ENDED); {
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::beDismissed);
            efc.changeState("goingToDismissSpot");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::goToOpponent);
        }
        efc.newEvent(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.changeState("going_to_carriable_object");
        }
        efc.newEvent(MOB_EV_NEAR_TOOL); {
            efc.run(pikmin_fsm::goToTool);
        }
        efc.newEvent(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(pikmin_fsm::goToGroupTask);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("swarm_stopped", PIKMIN_STATE_SWARM_STOPPED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::setSwarmReach);
            efc.run(pikmin_fsm::stopInGroup);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::setIdleTaskReach);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::beGrabbedByFriend);
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
            efc.run(pikmin_fsm::beDismissed);
            efc.changeState("goingToDismissSpot");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::goToOpponent);
        }
        efc.newEvent(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.changeState("going_to_carriable_object");
        }
        efc.newEvent(MOB_EV_NEAR_TOOL); {
            efc.run(pikmin_fsm::goToTool);
        }
        efc.newEvent(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(pikmin_fsm::goToGroupTask);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("grabbed_by_leader", PIKMIN_STATE_GRABBED_BY_LEADER); {
        efc.newEvent(MOB_EV_THROWN); {
            efc.run(pikmin_fsm::beThrown);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_RELEASED); {
            efc.run(pikmin_fsm::beReleased);
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::notifyLeaderRelease);
            efc.run(pikmin_fsm::beReleased);
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::notifyLeaderRelease);
            efc.run(pikmin_fsm::beReleased);
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("thrown", PIKMIN_STATE_THROWN); {
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::land);
            efc.run(pikmin_fsm::setBumpLock);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::checkOutgoingAttack);
            efc.run(pikmin_fsm::landOnMob);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_N); {
            efc.run(pikmin_fsm::landOnMob);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::beThrownByBouncer);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("mob_landing", PIKMIN_STATE_MOB_LANDING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startMobLanding);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finishMobLanding);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::unlatch);
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
    }
    
    efc.newState(
        "goingToDismissSpot", PIKMIN_STATE_GOING_TO_DISMISS_SPOT
    ); {
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::goingToDismissSpot);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::clearTimer);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reachDismissSpot);
            efc.run(pikmin_fsm::setBumpLock);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::reachDismissSpot);
            efc.run(pikmin_fsm::setBumpLock);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::goToOpponent);
        }
        efc.newEvent(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.changeState("going_to_carriable_object");
        }
        efc.newEvent(MOB_EV_NEAR_TOOL); {
            efc.run(pikmin_fsm::goToTool);
        }
        efc.newEvent(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(pikmin_fsm::goToGroupTask);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("idling", PIKMIN_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::becomeIdle);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::clearBoredomData);
            efc.run(pikmin_fsm::stopBeingIdle);
        }
        efc.newEvent(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::goToOpponent);
        }
        efc.newEvent(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.changeState("going_to_carriable_object");
        }
        efc.newEvent(MOB_EV_NEAR_TOOL); {
            efc.run(pikmin_fsm::goToTool);
        }
        efc.newEvent(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(pikmin_fsm::goToGroupTask);
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("riding_track");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::checkLeaderBump);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::startBoredomAnim);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::checkBoredomAnimEnd);
            efc.run(pikmin_fsm::checkShakingAnimEnd);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("called", PIKMIN_STATE_CALLED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::called);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finishCalledAnim);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("going_to_opponent", PIKMIN_STATE_GOING_TO_OPPONENT); {
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::decideAttack);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUS_OFF_REACH); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_FOCUS_DIED); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("circling_opponent", PIKMIN_STATE_CIRCLING_OPPONENT); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::circleOpponent);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::decideAttack);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUS_OFF_REACH); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_FOCUS_DIED); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "going_to_carriable_object", PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::goToCarriableObject);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reachCarriableObject);
            efc.changeState("carrying");
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.run(pikmin_fsm::forgetCarriableObject);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::forgetCarriableObject);
            efc.changeState("sighing");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::forgetCarriableObject);
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::forgetCarriableObject);
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::forgetCarriableObject);
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::forgetCarriableObject);
            efc.run(pikmin_fsm::fallDownPit);
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
            efc.run(pikmin_fsm::forgetTool);
            efc.changeState("sighing");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::forgetTool);
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::forgetTool);
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::forgetTool);
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::forgetTool);
            efc.run(pikmin_fsm::fallDownPit);
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
            efc.run(pikmin_fsm::forgetGroupTask);
            efc.changeState("sighing");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::forgetGroupTask);
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.run(pikmin_fsm::forgetGroupTask);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::forgetGroupTask);
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::forgetGroupTask);
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::forgetGroupTask);
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "going_to_onion", PIKMIN_STATE_GOING_TO_ONION
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::goToOnion);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("entering_onion");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("sighing", PIKMIN_STATE_SIGHING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::standStill);
            efc.run(pikmin_fsm::sigh);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::checkLeaderBump);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("carrying", PIKMIN_STATE_CARRYING); {
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stopCarrying);
            efc.run(pikmin_fsm::standStill);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tickCarrying);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FINISHED_CARRYING); {
            efc.run(pikmin_fsm::finishCarrying);
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("picking_up", PIKMIN_STATE_PICKING_UP); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startPickingUp);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finishPickingUp);
            efc.changeState("idling_h");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
    }
    
    efc.newState("on_group_task", PIKMIN_STATE_ON_GROUP_TASK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::workOnGroupTask);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tickGroupTaskWork);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::forgetGroupTask);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.run(pikmin_fsm::forgetGroupTask);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("returning", PIKMIN_STATE_RETURNING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startReturning);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::standStill);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("attacking_grounded", PIKMIN_STATE_ATTACKING_GROUNDED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::prepareToAttack);
        }
        efc.newEvent(MOB_EV_FOCUS_OFF_REACH); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::rechaseOpponent);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("attacking_latched", PIKMIN_STATE_ATTACKING_LATCHED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::prepareToAttack);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::unlatch);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_FOCUS_DIED); {
            efc.run(pikmin_fsm::loseLatchedMob);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("grabbed_by_enemy", PIKMIN_STATE_GRABBED_BY_ENEMY); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::beGrabbedByEnemy);
        }
        efc.newEvent(MOB_EV_RELEASED); {
            efc.run(pikmin_fsm::beReleased);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_SWALLOWED); {
            efc.run(pikmin_fsm::startDying);
            efc.run(pikmin_fsm::finishDying);
        }
    }
    
    efc.newState("knocked_back", PIKMIN_STATE_KNOCKED_BACK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::beAttacked);
            efc.run(pikmin_fsm::getKnockedBack);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.changeState("knocked_down");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
    }
    
    efc.newState("knocked_down", PIKMIN_STATE_KNOCKED_DOWN); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::standStill);
            efc.run(pikmin_fsm::getKnockedDown);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.changeState("getting_up");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::calledWhileKnockedDown);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("knocked_down_dying");
        }
    }
    
    efc.newState("getting_up", PIKMIN_STATE_GETTING_UP); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startGettingUp);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finishGettingUp);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::calledWhileKnockedDown);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("impact_bounce", PIKMIN_STATE_IMPACT_BOUNCE); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::standStill);
            efc.run(pikmin_fsm::doImpactBounce);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::landAfterImpactBounce);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("impact_lunge", PIKMIN_STATE_IMPACT_LUNGE); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startImpactLunge);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("impact_bounce");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::checkOutgoingAttack);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("helpless", PIKMIN_STATE_HELPLESS); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::notifyLeaderRelease);
            efc.run(pikmin_fsm::beReleased);
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::standStill);
            efc.run(pikmin_fsm::becomeHelpless);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        
        //The logic to lose helplessness is in
        //pikmin::handleStatusEffectLoss();
    }
    
    efc.newState("flailing", PIKMIN_STATE_FLAILING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::standStill);
            efc.run(pikmin_fsm::notifyLeaderRelease);
            efc.run(pikmin_fsm::beReleased);
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::startFlailing);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::standStill);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::flailToLeader);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        
        //The logic to stop flailing is in
        //pikmin::handleStatusEffectLoss();
    }
    
    efc.newState("panicking", PIKMIN_STATE_PANICKING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::standStill);
            efc.run(pikmin_fsm::unlatch);
            efc.run(pikmin_fsm::notifyLeaderRelease);
            efc.run(pikmin_fsm::beReleased);
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::startPanicking);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::panicNewChase);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        
        //The logic to stop panicking is in
        //pikmin::handleStatusEffectLoss();
    }
    
    efc.newState("drinking", PIKMIN_STATE_DRINKING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startDrinking);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::finishDrinking);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("celebrating", PIKMIN_STATE_CELEBRATING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::standStill);
            efc.run(pikmin_fsm::celebrate);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::checkLeaderBump);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("in_group_chasing_h", PIKMIN_STATE_IN_GROUP_CHASING_H); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startChasingLeader);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::releaseTool);
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader_h");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.run(pikmin_fsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("in_group_stopped_h");
        }
        efc.newEvent(MOB_EV_SWARM_STARTED); {
            efc.changeState("swarm_chasing_h");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::beDismissed);
            efc.changeState("going_to_dismiss_spot_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("in_group_stopped_h", PIKMIN_STATE_IN_GROUP_STOPPED_H); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::stopInGroup);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::releaseTool);
            efc.changeState("in_group_stopped");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader_h");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.changeState("in_group_chasing_h");
        }
        efc.newEvent(MOB_EV_SWARM_STARTED); {
            efc.changeState("swarm_chasing_h");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::beDismissed);
            efc.changeState("going_to_dismiss_spot_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("swarm_chasing_h", PIKMIN_STATE_SWARM_CHASING_H); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::setSwarmReach);
            efc.run(pikmin_fsm::startChasingLeader);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::releaseTool);
            efc.changeState("swarm_chasing");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::setIdleTaskReach);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::beGrabbedByFriend);
            efc.changeState("grabbed_by_leader_h");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("swarm_stopped_h");
        }
        efc.newEvent(MOB_EV_SWARM_ENDED); {
            efc.changeState("in_group_chasing_h");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::beDismissed);
            efc.changeState("going_to_dismiss_spot_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("swarm_stopped_h", PIKMIN_STATE_SWARM_STOPPED_H); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::setSwarmReach);
            efc.run(pikmin_fsm::stopInGroup);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::releaseTool);
            efc.changeState("swarm_stopped");
        }
        efc.newEvent(MOB_EV_GO_TO_ONION); {
            efc.changeState("going_to_onion");
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::setIdleTaskReach);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::beGrabbedByFriend);
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
            efc.run(pikmin_fsm::beDismissed);
            efc.changeState("going_to_dismiss_spot_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("grabbed_by_leader_h", PIKMIN_STATE_GRABBED_BY_LEADER_H); {
        efc.newEvent(MOB_EV_THROWN); {
            efc.run(pikmin_fsm::beThrown);
            efc.changeState("thrown_h");
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::releaseTool);
            efc.changeState("grabbed_by_leader");
        }
        efc.newEvent(MOB_EV_RELEASED); {
            efc.changeState("in_group_chasing_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::notifyLeaderRelease);
            efc.run(pikmin_fsm::beReleased);
            efc.run(pikmin_fsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::notifyLeaderRelease);
            efc.run(pikmin_fsm::beReleased);
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("thrown_h", PIKMIN_STATE_THROWN_H); {
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::releaseTool);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::landWhileHolding);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::landOnMobWhileHolding);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_N); {
            efc.run(pikmin_fsm::landOnMobWhileHolding);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::fallDownPit);
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
            efc.run(pikmin_fsm::releaseTool);
            efc.changeState("goingToDismissSpot");
        }
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::goingToDismissSpot);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::clearTimer);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reachDismissSpot);
            efc.run(pikmin_fsm::setBumpLock);
            efc.changeState("idling_h");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::reachDismissSpot);
            efc.changeState("idling_h");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("idling_h", PIKMIN_STATE_IDLING_H); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::becomeIdle);
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::releaseTool);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stopBeingIdle);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::whistledWhileHolding);
        }
        efc.newEvent(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::checkLeaderBump);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::tryHeldItemHotswap);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::releaseTool);
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("called_h", PIKMIN_STATE_CALLED_H); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::called);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finishCalledAnim);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::checkIncomingAttack);
        }
        efc.newEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.changeState("knocked_back");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touchedEatHitbox);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("riding_track", PIKMIN_STATE_RIDING_TRACK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startRidingTrack);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tickTrackRide);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::whistledWhileRiding);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("crushed", PIKMIN_STATE_CRUSHED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::beCrushed);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finishDying);
        }
    }
    
    efc.newState("knocked_down_dying", PIKMIN_STATE_KNOCKED_DOWN_DYING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startKnockedDownDying);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finishDying);
        }
    }
    
    efc.newState("dying", PIKMIN_STATE_DYING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::startDying);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finishDying);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_idx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_PIKMIN_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_PIKMIN_STATES) + " in enum."
    );
}


/**
 * @brief When a Pikmin is hit by an attack and gets knocked back.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::beAttacked(Mob* m, void* info1, void* info2) {
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(info) {
        //Damage.
        float damage = 0;
        float health_before = pik_ptr->health;
        info->mob2->calculateDamage(m, info->h2, info->h1, &damage);
        m->applyAttackDamage(info->mob2, info->h2, info->h1, damage);
        if(pik_ptr->health <= 0.0f && health_before > 0.0f) {
            if(!info->h2->hazards.empty()) {
                game.statistics.pikmin_hazard_deaths++;
            }
        }
        
        //Knockback.
        float knockback = 0;
        float knockback_angle = 0;
        info->mob2->calculateKnockback(
            m, info->h2, info->h1, &knockback, &knockback_angle
        );
        m->applyKnockback(knockback, knockback_angle);
        
        //Withering.
        if(info->h2->wither_chance > 0 && pik_ptr->maturity > 0) {
            unsigned char wither_roll = game.rng.i(0, 100);
            if(wither_roll < info->h2->wither_chance) {
                pik_ptr->increaseMaturity(-1);
            }
        }
        
        //Effects.
        m->doAttackEffects(info->mob2, info->h2, info->h1, damage, knockback);
        
    } else {
        //This can happen, for example, if the Pikmin got told to get knocked
        //back from a bomb rock hotswap. There's no real "hit" in this case
        //so let's just do the basics and let the Pikmin leave the group,
        //change animation, and little else.
    }
    
    //Finish up.
    m->leaveGroup();
    pikmin_fsm::beReleased(m, info1, info2);
    pikmin_fsm::notifyLeaderRelease(m, info1, info2);
    pikmin_fsm::releaseTool(m, nullptr, nullptr);
    m->face(m->angle, nullptr);
}


/**
 * @brief When a Pikmin is crushed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::beCrushed(Mob* m, void* info1, void* info2) {
    pikmin_fsm::startDying(m, info1, info2);
    m->z = m->ground_sector->z;
    m->setAnimation(PIKMIN_ANIM_CRUSHED);
}


/**
 * @brief When a Pikmin is dismissed by its leader.
 *
 * @param m The mob.
 * @param info1 Pointer to the world coordinates to go to.
 * @param info2 Unused.
 */
void pikmin_fsm::beDismissed(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->chase(*((Point*) info1), m->z);
    
    m->setAnimation(PIKMIN_ANIM_IDLING);
    m->playSound(pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_IDLE]);
}


/**
 * @brief When a Pikmin is grabbed by an enemy.
 *
 * @param m The mob.
 * @param info1 Pointer to the enemy.
 * @param info2 Pointer to the hitbox that grabbed.
 */
void pikmin_fsm::beGrabbedByEnemy(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    engineAssert(info2 != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* ene_ptr = (Mob*) info1;
    Hitbox* hbox_ptr = (Hitbox*) info2;
    
    ene_ptr->chomp(pik_ptr, hbox_ptr);
    pik_ptr->is_grabbed_by_enemy = true;
    disableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pik_ptr->leaveGroup();
    
    pik_ptr->setAnimation(PIKMIN_ANIM_FLAILING, START_ANIM_OPTION_RANDOM_TIME);
    m->playSound(pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_CAUGHT]);
    
}


/**
 * @brief When a Pikmin is grabbed by a leader.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::beGrabbedByFriend(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->setAnimation(PIKMIN_ANIM_IDLING);
    m->playSound(pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_HELD]);
}


/**
 * @brief When a Pikmin is gently released by a leader or enemy.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::beReleased(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    ((Pikmin*) m)->is_grabbed_by_enemy = false;
    
    size_t held_sound_idx =
        pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_HELD];
    if(held_sound_idx != INVALID) {
        game.audio.stopAllPlaybacks(
            pik_ptr->type->sounds[held_sound_idx].sample
        );
    }
}


/**
 * @brief When a Pikmin is thrown by a leader.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::beThrown(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    m->setAnimation(PIKMIN_ANIM_THROWN);
    
    size_t held_sound_idx =
        pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_HELD];
    if(held_sound_idx != INVALID) {
        game.audio.stopAllPlaybacks(
            pik_ptr->type->sounds[held_sound_idx].sample
        );
    }
    
    size_t throw_sound_idx =
        pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_THROWN];
    if(throw_sound_idx != INVALID) {
        MobType::Sound* throw_sound =
            &pik_ptr->type->sounds[throw_sound_idx];
        SoundSourceConfig throw_sound_config;
        throw_sound_config.stack_mode = SOUND_STACK_MODE_OVERRIDE;
        game.audio.createMobSoundSource(
            throw_sound->sample,
            m, false, throw_sound_config
        );
    }
    
    ((Pikmin*) m)->startThrowTrail();
}


/**
 * @brief When a Pikmin is thrown after being plucked.
 *
 * @param m The mob.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void pikmin_fsm::beThrownAfterPluck(Mob* m, void* info1, void* info2) {
    float throw_angle = getAngle(m->pos, m->focused_mob->pos);
    m->speed_z = PIKMIN::THROW_VER_SPEED;
    m->speed = angleToCoordinates(throw_angle, PIKMIN::THROW_HOR_SPEED);
    m->face(throw_angle + TAU / 2.0f, nullptr, true);
    
    m->setAnimation(PIKMIN_ANIM_PLUCKING_THROWN);
    ((Pikmin*) m)->startThrowTrail();
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sys_content_names.part_pikmin_pluck_dirt, m
        );
    m->particle_generators.push_back(pg);
}


/**
 * @brief When a Pikmin is thrown by a bouncer mob.
 *
 * @param m The mob.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void pikmin_fsm::beThrownByBouncer(Mob* m, void* info1, void* info2) {
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->setAnimation(PIKMIN_ANIM_THROWN);
    
    ((Pikmin*) m)->startThrowTrail();
}


/**
 * @brief When a Pikmin becomes "helpless".
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::becomeHelpless(Mob* m, void* info1, void* info2) {
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->leaveGroup();
    
    m->setAnimation(PIKMIN_ANIM_IDLING);
}


/**
 * @brief When a Pikmin becomes idling.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::becomeIdle(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    pikmin_fsm::standStill(m, info1, info2);
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
        pik_ptr->chase(
            pik_ptr->pos,
            pik_ptr->ground_sector->z + PIKMIN::FLIER_ABOVE_FLOOR_HEIGHT
        );
    }
    
    m->unfocusFromMob();
    
    m->setAnimation(
        PIKMIN_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    m->setTimer(
        game.rng.f(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin becomes a seed or a sprout.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::becomeSprout(Mob* m, void* info1, void* info2) {
    m->leaveGroup();
    enableFlag(m->flags, MOB_FLAG_INTANGIBLE);
    enableFlag(m->flags, MOB_FLAG_NON_HUNTABLE);
    enableFlag(m->flags, MOB_FLAG_NON_HURTABLE);
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    ((Pikmin*) m)->is_seed_or_sprout = true;
    m->setAnimation(
        PIKMIN_ANIM_SPROUT, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief Makes a Pikmin begin its plucking process.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that is plucking.
 * @param info2 Unused.
 */
void pikmin_fsm::beginPluck(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* lea_ptr = (Mob*) info1;
    
    pik_ptr->focusOnMob(lea_ptr);
    disableFlag(m->flags, MOB_FLAG_NON_HUNTABLE);
    disableFlag(m->flags, MOB_FLAG_NON_HURTABLE);
    disableFlag(m->flags, MOB_FLAG_INTANGIBLE);
    pik_ptr->is_seed_or_sprout = false;
    pikmin_fsm::clearTimer(m, info1, info2); //Clear sprout evolution timer.
    
    pik_ptr->setAnimation(PIKMIN_ANIM_PLUCKING);
}


/**
 * @brief When a Pikmin is called over by a leader, either by being whistled,
 * or touched when idling.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 If not nullptr, then the Pikmin must be silent.
 */
void pikmin_fsm::called(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* caller = (Mob*) info1;
    
    pik_ptr->was_last_hit_dud = false;
    pik_ptr->consecutive_dud_hits = 0;
    pikmin_fsm::standStill(m, info1, info2);
    
    pik_ptr->focusOnMob(caller);
    
    pik_ptr->setAnimation(PIKMIN_ANIM_CALLED);
    if(info2 == nullptr) {
        m->playSound(pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_CALLED]);
    }
}


/**
 * @brief When a Pikmin that is knocked down is called over by a leader,
 * either by being whistled, or touched when idling.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void pikmin_fsm::calledWhileKnockedDown(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* caller = (Mob*) info1;
    
    //Let's use the "temp" variable to specify whether or not a leader
    //already whistled it.
    if(pik_ptr->temp_i == 1) return;
    
    pik_ptr->focusOnMob(caller);
    
    pik_ptr->script_timer.time_left =
        std::max(
            0.01f,
            pik_ptr->script_timer.time_left -
            pik_ptr->pik_type->knocked_down_whistle_bonus
        );
        
    pik_ptr->temp_i = 1;
}


/**
 * @brief When a Pikmin should celebrate.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::celebrate(Mob* m, void* info1, void* info2) {
    if(game.rng.i(0, 1) == 0) {
        m->setAnimation(PIKMIN_ANIM_BACKFLIP);
    } else {
        m->setAnimation(PIKMIN_ANIM_TWIRLING);
    }
}


/**
 * @brief When a Pikmin should check if the animation that ended is a boredom
 * animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::checkBoredomAnimEnd(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(!pik_ptr->in_bored_animation) return;
    m->setAnimation(PIKMIN_ANIM_IDLING);
    pik_ptr->in_bored_animation = false;
    m->setTimer(
        game.rng.f(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin should check the attack it has just received.
 * If the attack is successful, another event is triggered. Otherwise
 * nothing happens.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::checkIncomingAttack(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(pik_ptr->invuln_period.time_left > 0) {
        //The Pikmin cannot be attacked right now.
        return;
    }
    
    if(!pik_ptr->processAttackMiss(info)) {
        //It has been decided that this attack missed.
        return;
    }
    
    float damage = 0;
    if(!info->mob2->calculateDamage(m, info->h2, info->h1, &damage)) {
        //This attack doesn't cause damage.
        return;
    }
    
    //If we got to this point, then greenlight for the attack.
    m->fsm.runEvent(MOB_EV_PIKMIN_DAMAGE_CONFIRMED, info1, info2);
}


/**
 * @brief When a Pikmin should check if the leader bumping it should
 * result in it being added to the group or not.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::checkLeaderBump(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(pik_ptr->bump_lock > 0.0f) {
        pik_ptr->bump_lock = game.config.pikmin.idle_bump_delay;
        return;
    }
    if(
        !pik_ptr->holding.empty() &&
        pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
    ) {
        m->fsm.setState(PIKMIN_STATE_CALLED_H, info1, info2);
    } else {
        m->fsm.setState(PIKMIN_STATE_CALLED, info1, info2);
    }
}


/**
 * @brief When a Pikmin should check the attack it is about to unleash.
 * If it realizes it's doing no damage, it should start considering
 * sighing and giving up.
 *
 * @param m The mob.
 * @param info1 Pointer to the opponent.
 * @param info2 Unused.
 */
void pikmin_fsm::checkOutgoingAttack(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
    float damage = 0;
    bool attack_success =
        pik_ptr->calculateDamage(info->mob2, info->h1, info->h2, &damage);
        
    if(damage == 0 || !attack_success) {
        pik_ptr->was_last_hit_dud = true;
    }
}


/**
 * @brief When a Pikmin should check if the animation that ended is a shaking
 * animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::checkShakingAnimEnd(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(!pik_ptr->in_shaking_animation) return;
    m->setAnimation(PIKMIN_ANIM_IDLING);
    pik_ptr->in_shaking_animation = false;
    m->setTimer(
        game.rng.f(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin has to circle around its opponent.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::circleOpponent(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    m->stopCircling();
    
    float circle_time = game.rng.f(0.0f, 1.0f);
    //Bias the time so that there's a higher chance of picking a close angle,
    //and a lower chance of circling to a distant one. The Pikmin came here
    //to attack, not dance!
    circle_time *= circle_time;
    circle_time += 0.5f;
    m->setTimer(circle_time);
    
    bool go_cw = game.rng.f(0.0f, 1.0f) <= 0.5f;
    m->circleAround(
        m->focused_mob, Point(), m->focused_mob->radius + m->radius, go_cw,
        m->getBaseSpeed(), true
    );
    
    m->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
}


/**
 * @brief When a Pikmin has to clear any data about being bored.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::clearBoredomData(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    pikmin_fsm::clearTimer(m, info1, info2);
    pik_ptr->in_bored_animation = false;
}


/**
 * @brief When a Pikmin has to clear any timer set.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::clearTimer(Mob* m, void* info1, void* info2) {
    m->setTimer(0);
}


/**
 * @brief When the Pikmin reaches an opponent that it was chasing after,
 * and should now decide how to attack it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::decideAttack(Mob* m, void* info1, void* info2) {
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    
    if(m->invuln_period.time_left > 0) {
        //Don't let the Pikmin attack while invulnerable. Otherwise, this can
        //be exploited to let Pikmin vulnerable to a hazard attack the obstacle
        //emitting said hazard.
        return;
    }
    
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pik_ptr->stopChasing();
    pik_ptr->stopCircling();
    
    bool can_circle =
        pik_ptr->fsm.cur_state->id != PIKMIN_STATE_CIRCLING_OPPONENT &&
        m->focused_mob->type->category->id == MOB_CATEGORY_ENEMIES;
        
    switch(pik_ptr->pik_type->attack_method) {
    case PIKMIN_ATTACK_LATCH: {
        //This Pikmin latches on to things and/or smacks with its top.
        Distance d;
        Hitbox* closest_h =
            pik_ptr->focused_mob->getClosestHitbox(
                pik_ptr->pos, HITBOX_TYPE_NORMAL, &d
            );
        float h_z = 0;
        
        if(closest_h) {
            h_z = closest_h->z + pik_ptr->focused_mob->z;
        }
        
        if(
            !closest_h || !closest_h->can_pikmin_latch ||
            h_z > pik_ptr->z + pik_ptr->height ||
            h_z + closest_h->height < pik_ptr->z ||
            d >= closest_h->radius + pik_ptr->radius
        ) {
            //Can't latch to the closest hitbox.
            
            if(
                game.rng.f(0.0f, 1.0f) <=
                PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED &&
                can_circle
            ) {
                //Circle around the opponent a bit before smacking.
                pik_ptr->fsm.setState(PIKMIN_STATE_CIRCLING_OPPONENT);
            } else {
                //Smack.
                pik_ptr->fsm.setState(PIKMIN_STATE_ATTACKING_GROUNDED);
            }
            
        } else {
            //Can latch to the closest hitbox.
            
            if(
                game.rng.f(0, 1) <=
                PIKMIN::CIRCLE_OPPONENT_CHANCE_PRE_LATCH &&
                can_circle
            ) {
                //Circle around the opponent a bit before latching.
                pik_ptr->fsm.setState(PIKMIN_STATE_CIRCLING_OPPONENT);
            } else {
                //Latch on.
                pik_ptr->latch(pik_ptr->focused_mob, closest_h);
                pik_ptr->fsm.setState(PIKMIN_STATE_ATTACKING_LATCHED);
            }
            
        }
        
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {
        //This Pikmin attacks by lunching forward for an impact.
        
        if(
            game.rng.f(0, 1) <=
            PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED &&
            can_circle
        ) {
            //Circle around the opponent a bit before lunging.
            pik_ptr->fsm.setState(PIKMIN_STATE_CIRCLING_OPPONENT);
        } else {
            //Go for the lunge.
            pik_ptr->fsm.setState(PIKMIN_STATE_IMPACT_LUNGE);
        }
        
        break;
        
    }
    }
}


/**
 * @brief When a Pikmin has to bounce back from an impact attack.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::doImpactBounce(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    disableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    float impact_angle = 0.0f;
    float impact_speed = 0.0f;
    
    if(pik_ptr->focused_mob) {
        if(pik_ptr->focused_mob->rectangular_dim.x != 0) {
            impact_angle =
                getAngle(
                    getClosestPointInRotatedRectangle(
                        pik_ptr->pos,
                        pik_ptr->focused_mob->pos,
                        pik_ptr->focused_mob->rectangular_dim,
                        pik_ptr->focused_mob->angle,
                        nullptr
                    ),
                    pik_ptr->pos
                );
        } else {
            impact_angle = getAngle(pik_ptr->focused_mob->pos, pik_ptr->pos);
        }
        impact_speed = 200.0f;
    }
    
    pik_ptr->speed =
        angleToCoordinates(
            impact_angle, impact_speed
        );
    pik_ptr->speed_z = 500.0f;
    pik_ptr->face(impact_angle + TAU / 2.0f, nullptr, true);
    
    pik_ptr->setAnimation(PIKMIN_ANIM_KNOCKED_BACK);
}


/**
 * @brief When a Pikmin must start climbing up an Onion's leg.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::enterOnion(Mob* m, void* info1, void* info2) {
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Onion* oni_ptr = (Onion*) pik_ptr->focused_mob;
    
    disableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    //Set its data to start climbing.
    vector<size_t> checkpoints;
    checkpoints.push_back((pik_ptr->temp_i * 2) + 1);
    checkpoints.push_back(pik_ptr->temp_i * 2);
    
    pik_ptr->track_info = new TrackRideInfo(
        oni_ptr, checkpoints, oni_ptr->oni_type->nest->pikmin_enter_speed
    );
    
    pik_ptr->setAnimation(PIKMIN_ANIM_CLIMBING);
}


/**
 * @brief When a Pikmin falls down a bottomless pit.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::fallDownPit(Mob* m, void* info1, void* info2) {
    m->startDying();
    m->finishDying();
}


/**
 * @brief When a Pikmin finished the animation for when it's called.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finishCalledAnim(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* lea_ptr = pik_ptr->focused_mob;
    
    if(lea_ptr) {
        if(lea_ptr->following_group) {
            //If this leader is following another one,
            //then the new Pikmin should be in the group of that top leader.
            lea_ptr = lea_ptr->following_group;
        }
        lea_ptr->addToGroup(pik_ptr);
        pik_ptr->fsm.setState(
            pik_ptr->holding.empty() ?
            PIKMIN_STATE_IN_GROUP_CHASING :
            PIKMIN_STATE_IN_GROUP_CHASING_H,
            info1, info2);
    } else {
        pik_ptr->fsm.setState(
            pik_ptr->holding.empty() ?
            PIKMIN_STATE_IDLING :
            PIKMIN_STATE_IDLING_H,
            info1, info2
        );
    }
}


/**
 * @brief When a Pikmin successfully finishes carrying an object.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finishCarrying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    engineAssert(pik_ptr->carrying_mob != nullptr, m->printStateHistory());
    
    if(pik_ptr->carrying_mob->carry_info->must_return) {
        //The Pikmin should return somewhere (like a pile).
        pik_ptr->fsm.setState(PIKMIN_STATE_RETURNING, (void*) pik_ptr->carrying_mob);
        
    } else {
        //The Pikmin can just sit and chill.
        pik_ptr->fsm.setState(PIKMIN_STATE_CELEBRATING);
    }
}


/**
 * @brief When a Pikmin finishes drinking the drop it was drinking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finishDrinking(Mob* m, void* info1, void* info2) {
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    Pikmin* pik_ptr = (Pikmin*) m;
    Drop* dro_ptr = (Drop*) m->focused_mob;
    
    switch(dro_ptr->dro_type->effect) {
    case DROP_EFFECT_MATURATE: {
        pik_ptr->increaseMaturity(dro_ptr->dro_type->increase_amount);
        break;
    } case DROP_EFFECT_GIVE_STATUS: {
        pik_ptr->applyStatusEffect(
            dro_ptr->dro_type->status_to_give, false, false
        );
        break;
    } default: {
        break;
    }
    }
    
    m->unfocusFromMob();
}


/**
 * @brief When a Pikmin finishes dying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finishDying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    pik_ptr->finishDying();
}


/**
 * @brief When a Pikmin finishes getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finishGettingUp(Mob* m, void* info1, void* info2) {
    Mob* prev_focused_mob = m->focused_mob;
    
    m->fsm.setState(PIKMIN_STATE_IDLING);
    
    if(prev_focused_mob) {
        if(
            prev_focused_mob->type->category->id == MOB_CATEGORY_LEADERS &&
            !m->canHunt(prev_focused_mob)
        ) {
            m->fsm.runEvent(MOB_EV_WHISTLED, (void*) prev_focused_mob);
            
        } else if(
            m->canHunt(prev_focused_mob)
        ) {
            m->fsm.runEvent(
                MOB_EV_OPPONENT_IN_REACH, (void*) prev_focused_mob, nullptr
            );
            
        }
    }
}


/**
 * @brief When a Pikmin finishes its sequence of landing on another mob.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finishMobLanding(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(!m->focused_mob) {
        //The mob has died or vanished since the Pikmin first landed.
        //Return to idle.
        pik_ptr->fsm.setState(PIKMIN_STATE_IDLING);
        return;
    }
    
    switch(pik_ptr->pik_type->attack_method) {
    case PIKMIN_ATTACK_LATCH: {
        pik_ptr->fsm.setState(PIKMIN_STATE_ATTACKING_LATCHED);
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {
        pik_ptr->fsm.setState(PIKMIN_STATE_IMPACT_BOUNCE);
        break;
        
    }
    }
}


/**
 * @brief When a Pikmin finishes picking some object up to hold it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finishPickingUp(Mob* m, void* info1, void* info2) {
    Tool* too_ptr = (Tool*) (m->focused_mob);
    
    if(!hasFlag(too_ptr->holdability_flags, HOLDABILITY_FLAG_PIKMIN)) {
        m->fsm.setState(PIKMIN_STATE_IDLING);
        return;
    }
    
    m->subgroup_type_ptr =
        game.states.gameplay->subgroup_types.getType(
            SUBGROUP_TYPE_CATEGORY_TOOL, m->focused_mob->type
        );
    m->hold(
        m->focused_mob, INVALID, 4, 0, 0.5f,
        true, HOLD_ROTATION_METHOD_FACE_HOLDER
    );
    m->unfocusFromMob();
}


/**
 * @brief When the Pikmin must move towards the whistle.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void pikmin_fsm::flailToLeader(Mob* m, void* info1, void* info2) {
    Mob* caller = (Mob*) info1;
    m->chase(caller->pos, caller->z);
}


/**
 * @brief When a Pikmin is meant to drop the object it's carrying, or
 * stop chasing the object if it's not carrying it yet, but wants to.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::forgetCarriableObject(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(!pik_ptr->carrying_mob) return;
    
    pik_ptr->carrying_mob->carry_info->spot_info[pik_ptr->temp_i].state =
        CARRY_SPOT_STATE_FREE;
    pik_ptr->carrying_mob->carry_info->spot_info[pik_ptr->temp_i].pik_ptr =
        nullptr;
        
    pik_ptr->carrying_mob = nullptr;
}


/**
 * @brief When a Pikmin is meant to forget a group task object it was going for.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::forgetGroupTask(Mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    GroupTask* tas_ptr = (GroupTask*) (m->focused_mob);
    Pikmin* pik_ptr = (Pikmin*) m;
    tas_ptr->freeUpSpot(pik_ptr);
    m->unfocusFromMob();
}


/**
 * @brief When a Pikmin is meant to forget a tool object it was going for.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::forgetTool(Mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    Tool* too_ptr = (Tool*) (m->focused_mob);
    too_ptr->reserved = nullptr;
    m->unfocusFromMob();
}


/**
 * @brief When a Pikmin gets knocked back.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::getKnockedBack(Mob* m, void* info1, void* info2) {
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->setAnimation(PIKMIN_ANIM_KNOCKED_BACK);
}


/**
 * @brief When a Pikmin gets knocked back and lands on the floor.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::getKnockedDown(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    //Let's use the "temp" variable to specify whether or not a leader
    //already whistled it.
    pik_ptr->temp_i = 0;
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->setTimer(pik_ptr->pik_type->knocked_down_duration);
    
    m->setAnimation(PIKMIN_ANIM_LYING);
}


/**
 * @brief When a Pikmin needs to go towards its spot on a carriable object.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob to carry.
 * @param info2 Unused.
 */
void pikmin_fsm::goToCarriableObject(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Mob* carriable_mob = (Mob*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pik_ptr->carrying_mob = carriable_mob;
    pik_ptr->leaveGroup();
    pik_ptr->stopChasing();
    
    size_t closest_spot = INVALID;
    Distance closest_spot_dist;
    CarrierSpot* closest_spot_ptr = nullptr;
    Point closest_spot_offset;
    
    //If this is the first Pikmin to go to the carriable mob, rotate
    //the points such that 0 faces this Pikmin instead.
    if(
        carriable_mob->carry_info->isEmpty() &&
        carriable_mob->type->custom_carry_spots.empty()
    ) {
        carriable_mob->carry_info->rotatePoints(
            getAngle(carriable_mob->pos, pik_ptr->pos)
        );
    }
    
    for(size_t s = 0; s < carriable_mob->type->max_carriers; s++) {
        CarrierSpot* spot_ptr =
            &carriable_mob->carry_info->spot_info[s];
        if(spot_ptr->state != CARRY_SPOT_STATE_FREE) continue;
        
        Point spot_offset =
            rotatePoint(spot_ptr->pos, carriable_mob->angle);
        Distance d(pik_ptr->pos, carriable_mob->pos + spot_offset);
        
        if(closest_spot == INVALID || d < closest_spot_dist) {
            closest_spot = s;
            closest_spot_dist = d;
            closest_spot_ptr = spot_ptr;
            closest_spot_offset = spot_offset;
        }
    }
    
    if(!closest_spot_ptr) return;
    
    pik_ptr->focusOnMob(carriable_mob);
    pik_ptr->temp_i = closest_spot;
    closest_spot_ptr->state = CARRY_SPOT_STATE_RESERVED;
    closest_spot_ptr->pik_ptr = pik_ptr;
    
    pik_ptr->chase(
        &carriable_mob->pos, &carriable_mob->z,
        closest_spot_offset, 0.0f
    );
    pik_ptr->setTimer(PIKMIN::GOTO_TIMEOUT);
    
    m->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
    
}


/**
 * @brief When a Pikmin needs to go towards a group task mob.
 *
 * @param m The mob.
 * @param info1 Pointer to the group task.
 * @param info2 Unused.
 */
void pikmin_fsm::goToGroupTask(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    GroupTask* tas_ptr = (GroupTask*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(
        !hasFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR) &&
        tas_ptr->tas_type->flying_pikmin_only
    ) {
        //Only flying Pikmin can use this, and this Pikmin doesn't fly.
        return;
    }
    
    GroupTask::GroupTaskSpot* free_spot = tas_ptr->getFreeSpot();
    if(!free_spot) {
        //There are no free spots available. Forget it.
        return;
    }
    
    tas_ptr->reserveSpot(free_spot, pik_ptr);
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pik_ptr->leaveGroup();
    pik_ptr->stopChasing();
    
    m->focusOnMob(tas_ptr);
    
    m->chase(
        &(free_spot->absolute_pos), &tas_ptr->z,
        Point(), tas_ptr->tas_type->spots_z
    );
    pik_ptr->setTimer(PIKMIN::GOTO_TIMEOUT);
    
    m->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
    
    pik_ptr->fsm.setState(PIKMIN_STATE_GOING_TO_GROUP_TASK);
    
}


/**
 * @brief When a Pikmin needs to walk towards an Onion to climb inside.
 *
 * @param m The mob.
 * @param info1 Pointer to the Onion.
 * @param info2 Unused.
 */
void pikmin_fsm::goToOnion(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    PikminNest* nest_ptr = (PikminNest*) info1;
    
    //Pick a leg at random.
    pik_ptr->temp_i =
        game.rng.i(
            0, (int) (nest_ptr->nest_type->leg_body_parts.size() / 2) - 1
        );
    size_t leg_foot_bp_idx =
        nest_ptr->m_ptr->anim.anim_db->findBodyPart(
            nest_ptr->nest_type->leg_body_parts[pik_ptr->temp_i * 2 + 1]
        );
    Point coords =
        nest_ptr->m_ptr->getHitbox(
            leg_foot_bp_idx
        )->getCurPos(nest_ptr->m_ptr->pos, nest_ptr->m_ptr->angle);
        
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    bool aux_b = true; //Needed for a gentle release.
    pikmin_fsm::releaseTool(m, (void*) &aux_b, nullptr);
    
    m->focusOnMob(nest_ptr->m_ptr);
    m->stopChasing();
    m->chase(coords, nest_ptr->m_ptr->z);
    m->leaveGroup();
    
    m->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
}


/**
 * @brief When a Pikmin needs to walk towards an opponent.
 *
 * @param m The mob.
 * @param info1 Pointer to the opponent.
 * @param info2 Unused.
 */
void pikmin_fsm::goToOpponent(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    
    Mob* other_ptr = (Mob*) info1;
    if(!pik_ptr->pik_type->can_fly) {
        //Grounded Pikmin.
        if(other_ptr->type->category->id == MOB_CATEGORY_ENEMIES) {
            Enemy* ene_ptr = (Enemy*) info1;
            if(!ene_ptr->ene_type->allow_ground_attacks) return;
            if(ene_ptr->z > m->z + m->height) return;
        }
    } else {
        //Airborne Pikmin.
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->focusOnMob(other_ptr);
    m->stopChasing();
    
    Point offset = Point();
    float target_distance =
        m->focused_mob->radius + m->radius + PIKMIN::GROUNDED_ATTACK_DIST;
        
    if(m->focused_mob->rectangular_dim.x != 0.0f) {
        bool is_inside = false;
        offset =
            getClosestPointInRotatedRectangle(
                m->pos,
                m->focused_mob->pos,
                m->focused_mob->rectangular_dim,
                m->focused_mob->angle,
                &is_inside
            ) - m->focused_mob->pos;
        target_distance -= m->focused_mob->radius;
    }
    
    m->chase(
        &m->focused_mob->pos, &m->focused_mob->z,
        offset, 0.0f, 0,
        target_distance
    );
    m->leaveGroup();
    
    pik_ptr->was_last_hit_dud = false;
    pik_ptr->consecutive_dud_hits = 0;
    
    m->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
    
    m->fsm.setState(PIKMIN_STATE_GOING_TO_OPPONENT);
}


/**
 * @brief When a Pikmin needs to go towards a tool mob.
 *
 * @param m The mob.
 * @param info1 Pointer to the tool.
 * @param info2 Unused.
 */
void pikmin_fsm::goToTool(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Tool* too_ptr = (Tool*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(too_ptr->reserved && too_ptr->reserved != pik_ptr) {
        //Another Pikmin is already going for it. Ignore it.
        return;
    }
    if(!pik_ptr->pik_type->can_carry_tools) {
        //This Pikmin can't carry tools. Forget it.
        return;
    }
    if(!hasFlag(too_ptr->holdability_flags, HOLDABILITY_FLAG_PIKMIN)) {
        //Can't hold this. Forget it.
        return;
    }
    
    too_ptr->reserved = pik_ptr;
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pik_ptr->leaveGroup();
    pik_ptr->stopChasing();
    
    m->focusOnMob(too_ptr);
    
    m->chase(
        &too_ptr->pos, &too_ptr->z,
        Point(), 0.0f, 0,
        pik_ptr->radius + too_ptr->radius
    );
    pik_ptr->setTimer(PIKMIN::GOTO_TIMEOUT);
    
    m->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
    
    pik_ptr->fsm.setState(PIKMIN_STATE_GOING_TO_TOOL);
    
}


/**
 * @brief When a Pikmin needs to get going to its dismiss spot.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::goingToDismissSpot(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->setTimer(PIKMIN::DISMISS_TIMEOUT);
    
    m->setAnimation(
        m->holding.empty() ? PIKMIN_ANIM_WALKING : PIKMIN_ANIM_CARRYING,
        START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
}


/**
 * @brief When a thrown Pikmin lands.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::land(Mob* m, void* info1, void* info2) {
    pikmin_fsm::standStill(m, nullptr, nullptr);
    
    m->setAnimation(PIKMIN_ANIM_IDLING);
}


/**
 * @brief When a Pikmin being bounced back from an impact attack lands
 * on the ground.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::landAfterImpactBounce(Mob* m, void* info1, void* info2) {
    m->fsm.setState(PIKMIN_STATE_KNOCKED_DOWN);
}


/**
 * @brief When a Pikmin lands after being thrown from a pluck.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::landAfterPluck(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* lea_ptr = pik_ptr->focused_mob;
    
    pik_ptr->setAnimation(PIKMIN_ANIM_IDLING);
    
    if(lea_ptr) {
        if(lea_ptr->following_group) {
            //If this leader is following another one,
            //then the new Pikmin should be in the group of that top leader.
            lea_ptr = lea_ptr->following_group;
        }
        lea_ptr->addToGroup(pik_ptr);
        pik_ptr->fsm.setState(PIKMIN_STATE_IN_GROUP_CHASING, info1, info2);
    } else {
        pik_ptr->fsm.setState(PIKMIN_STATE_IDLING, info1, info2);
    }
}


/**
 * @brief When a thrown Pikmin lands on a mob, to latch on to it.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::landOnMob(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Mob* m2_ptr = info->mob2;
    
    MobEvent* m2_pik_land_ev =
        m2_ptr->fsm.getEvent(MOB_EV_THROWN_PIKMIN_LANDED);
        
    if(m2_pik_land_ev && hasFlag(m->flags, MOB_FLAG_WAS_THROWN)) {
        m2_pik_land_ev->run(m2_ptr, (void*)m);
    }
    
    if(!m->canHurt(m2_ptr)) return;
    
    Hitbox* hbox_ptr = info->h2;
    
    if(
        !hbox_ptr ||
        (
            pik_ptr->pik_type->attack_method == PIKMIN_ATTACK_LATCH &&
            !hbox_ptr->can_pikmin_latch
        )
    ) {
        //No good. Make it bounce back.
        m->speed.x *= -0.3;
        m->speed.y *= -0.3;
        return;
    }
    
    pik_ptr->stopHeightEffect();
    pik_ptr->focused_mob = m2_ptr;
    disableFlag(pik_ptr->flags, MOB_FLAG_WAS_THROWN);
    
    switch(pik_ptr->pik_type->attack_method) {
    case PIKMIN_ATTACK_LATCH: {
        pik_ptr->latch(m2_ptr, hbox_ptr);
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {
        pik_ptr->speed.x = pik_ptr->speed.y = pik_ptr->speed_z = 0;
        break;
        
    }
    }
    
    pik_ptr->fsm.setState(PIKMIN_STATE_MOB_LANDING);
}


/**
 * @brief When a thrown Pikmin lands on a mob, whilst holding something.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::landOnMobWhileHolding(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    engineAssert(!m->holding.empty(), m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Tool* too_ptr = (Tool*) (*m->holding.begin());
    Mob* m2_ptr = info->mob2;
    
    if(!m->canHurt(m2_ptr)) return;
    
    MobEvent* m2_pik_land_ev =
        m2_ptr->fsm.getEvent(MOB_EV_THROWN_PIKMIN_LANDED);
        
    if(m2_pik_land_ev && hasFlag(m->flags, MOB_FLAG_WAS_THROWN)) {
        m2_pik_land_ev->run(m2_ptr, (void*)m);
    }
    
    disableFlag(pik_ptr->flags, MOB_FLAG_WAS_THROWN);
    
    if(too_ptr->too_type->dropped_when_pikmin_lands_on_opponent) {
        pikmin_fsm::releaseTool(m, nullptr, nullptr);
        m->fsm.setState(PIKMIN_STATE_IDLING);
        
        if(too_ptr->too_type->stuck_when_pikmin_lands_on_opponent && info->h2) {
            too_ptr->speed.x = too_ptr->speed.y = too_ptr->speed_z = 0;
            too_ptr->stopHeightEffect();
            
            too_ptr->focused_mob = m2_ptr;
            
            float h_offset_dist;
            float h_offset_angle;
            float v_offset_dist;
            m2_ptr->getHitboxHoldPoint(
                too_ptr, info->h2,
                &h_offset_dist, &h_offset_angle, &v_offset_dist
            );
            m2_ptr->hold(
                too_ptr, info->h2->body_part_idx,
                h_offset_dist, h_offset_angle, v_offset_dist,
                true, HOLD_ROTATION_METHOD_FACE_HOLDER
            );
        }
        
        if(
            too_ptr->too_type->pikmin_returns_after_using &&
            game.states.gameplay->cur_leader_ptr
        ) {
            if(
                !pik_ptr->holding.empty() &&
                pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
            ) {
                m->fsm.setState(
                    PIKMIN_STATE_CALLED_H, game.states.gameplay->cur_leader_ptr
                );
            } else {
                m->fsm.setState(
                    PIKMIN_STATE_CALLED, game.states.gameplay->cur_leader_ptr
                );
            }
        }
    }
}


/**
 * @brief When a thrown Pikmin lands while holding something.
 * Depending on what it is, it might drop it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::landWhileHolding(Mob* m, void* info1, void* info2) {
    engineAssert(!m->holding.empty(), m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Tool* too_ptr = (Tool*) * (m->holding.begin());
    
    pikmin_fsm::standStill(m, nullptr, nullptr);
    
    pik_ptr->is_tool_primed_for_whistle = true;
    
    m->setAnimation(PIKMIN_ANIM_IDLING);
    
    if(too_ptr->too_type->dropped_when_pikmin_lands) {
        pikmin_fsm::releaseTool(m, nullptr, nullptr);
        m->fsm.setState(PIKMIN_STATE_IDLING);
        
        if(
            too_ptr->too_type->pikmin_returns_after_using &&
            game.states.gameplay->cur_leader_ptr
        ) {
            if(
                !pik_ptr->holding.empty() &&
                pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
            ) {
                m->fsm.setState(
                    PIKMIN_STATE_CALLED_H, game.states.gameplay->cur_leader_ptr
                );
            } else {
                m->fsm.setState(
                    PIKMIN_STATE_CALLED, game.states.gameplay->cur_leader_ptr
                );
            }
        }
    } else {
        m->fsm.setState(PIKMIN_STATE_IDLING_H);
    }
}


/**
 * @brief When a Pikmin leaves its Onion because it got called out.
 *
 * @param m The mob.
 * @param info1 Points to the Onion.
 * @param info2 Unused.
 */
void pikmin_fsm::leaveOnion(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->setAnimation(PIKMIN_ANIM_SLIDING);
}


/**
 * @brief When a Pikmin leaves a hazardous sector.
 *
 * @param m The mob.
 * @param info1 Points to the hazard.
 * @param info2 Unused.
 */
void pikmin_fsm::leftHazard(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Hazard* h = (Hazard*) info1;
    if(h->associated_liquid) {
        m->removeParticleGenerator(MOB_PARTICLE_GENERATOR_ID_WAVE_RING);
    }
}


/**
 * @brief When the mob the Pikmin is latched on to disappears.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::loseLatchedMob(Mob* m, void* info1, void* info2) {
    m->stopChasing();
}


/**
 * @brief When a Pikmin notifies the leader that it must gently release it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::notifyLeaderRelease(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = ((Pikmin*) m);
    if(!pik_ptr->following_group) return;
    if(pik_ptr->holder.m != pik_ptr->following_group) return;
    pik_ptr->following_group->fsm.runEvent(MOB_EV_RELEASE_ORDER);
}


/**
 * @brief When a Pikmin needs to decide a new spot to run off to whilst
 * in panicking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::panicNewChase(Mob* m, void* info1, void* info2) {
    m->chase(
        Point(
            m->pos.x + game.rng.f(-1000, 1000),
            m->pos.y + game.rng.f(-1000, 1000)
        ),
        m->z
    );
    m->setTimer(PIKMIN::PANIC_CHASE_INTERVAL);
}


/**
 * @brief When a Pikmin is meant to reel back to unleash an attack.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::prepareToAttack(Mob* m, void* info1, void* info2) {
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    pik_ptr->was_last_hit_dud = false;
    
    if(pik_ptr->focused_mob->rectangular_dim.x != 0.0f) {
        bool is_inside = false;
        Point target =
            getClosestPointInRotatedRectangle(
                m->pos,
                m->focused_mob->pos,
                m->focused_mob->rectangular_dim,
                m->focused_mob->angle,
                &is_inside
            );
        pik_ptr->face(getAngle(m->pos, target), nullptr);
        
    } else {
        pik_ptr->face(0, &pik_ptr->focused_mob->pos);
        
    }
    
    pik_ptr->setAnimation(PIKMIN_ANIM_ATTACKING);
}


/**
 * @brief When a Pikmin reaches its spot on a carriable object.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::reachCarriableObject(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* carriable_mob = pik_ptr->carrying_mob;
    
    Point spot_offset =
        rotatePoint(
            carriable_mob->carry_info->spot_info[pik_ptr->temp_i].pos,
            carriable_mob->angle
        );
    Point final_pos = carriable_mob->pos + spot_offset;
    
    pik_ptr->chase(
        &carriable_mob->pos, &carriable_mob->z,
        spot_offset, 0.0f,
        CHASE_FLAG_TELEPORT |
        CHASE_FLAG_TELEPORTS_CONSTANTLY
    );
    
    pik_ptr->face(getAngle(final_pos, carriable_mob->pos), nullptr);
    
    //Let the carriable mob know that a new Pikmin has grabbed on.
    pik_ptr->carrying_mob->fsm.runEvent(
        MOB_EV_CARRIER_ADDED, (void*) pik_ptr
    );
    
    pik_ptr->in_carry_struggle_animation = false;
    pik_ptr->setAnimation(PIKMIN_ANIM_CARRYING);
}


/**
 * @brief When a Pikmin reaches its dismissal spot.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::reachDismissSpot(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    m->setAnimation(PIKMIN_ANIM_IDLING);
}


/**
 * @brief When a Pikmin that just attacked an opponent needs to walk
 * towards it again.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::rechaseOpponent(Mob* m, void* info1, void* info2) {

    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(pik_ptr->was_last_hit_dud) {
        //Check if the Pikmin's last hits were duds.
        //If so, maybe give up and sigh.
        pik_ptr->consecutive_dud_hits++;
        if(pik_ptr->consecutive_dud_hits >= 4) {
            pik_ptr->consecutive_dud_hits = 0;
            pik_ptr->fsm.setState(PIKMIN_STATE_SIGHING);
            return;
        }
    }
    
    bool can_continue_attacking =
        m->focused_mob &&
        m->focused_mob->health > 0 &&
        Distance(m->pos, m->focused_mob->pos) <=
        (m->radius + m->focused_mob->radius + PIKMIN::GROUNDED_ATTACK_DIST);
        
    if(!can_continue_attacking) {
        //The opponent cannot be chased down. Become idle.
        m->fsm.setState(PIKMIN_STATE_IDLING);
        
    } else if(game.rng.f(0.0f, 1.0f) <= PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED) {
        //Circle around it a bit before attacking from a new angle.
        pik_ptr->fsm.setState(PIKMIN_STATE_CIRCLING_OPPONENT);
        
    } else {
        //If the opponent is alive and within reach, let's stay in this state,
        //and attack some more!
        return;
        
    }
}


/**
 * @brief When a Pikmin is meant to release the tool it is currently holding.
 *
 * @param m The mob.
 * @param info1 If nullptr, release as normal.
 * Otherwise, this is a "gentle" release.
 * @param info2 Unused.
 */
void pikmin_fsm::releaseTool(Mob* m, void* info1, void* info2) {
    if(m->holding.empty()) return;
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* too_ptr = *m->holding.begin();
    
    if(info1) {
        too_ptr->setVar("gentle_release", "true");
    } else {
        too_ptr->setVar("gentle_release", "false");
    }
    pik_ptr->release(too_ptr);
    too_ptr->pos = m->pos;
    too_ptr->speed = Point();
    too_ptr->push_amount = 0.0f;
    m->subgroup_type_ptr =
        game.states.gameplay->subgroup_types.getType(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, pik_ptr->pik_type
        );
    if(m->following_group) {
        m->following_group->group->changeStandbyTypeIfNeeded();
        game.states.gameplay->updateClosestGroupMembers();
    }
}


/**
 * @brief When a Pikmin seed lands on the ground.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::seedLanded(Mob* m, void* info1, void* info2) {
    //Generate the rock particles that come out.
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sys_content_names.part_pikmin_seed_landed, m
        );
    m->particle_generators.push_back(pg);
}


/**
 * @brief When a Pikmin is meant to set its timer for the bump lock.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::setBumpLock(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    pik_ptr->bump_lock = game.config.pikmin.idle_bump_delay;
}


/**
 * @brief When a Pikmin is meant to change "reach" to the idle task reach.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::setIdleTaskReach(Mob* m, void* info1, void* info2) {
    m->near_reach = 0;
    m->updateInteractionSpan();
}


/**
 * @brief When a Pikmin is meant to change "reach" to the swarm reach.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::setSwarmReach(Mob* m, void* info1, void* info2) {
    m->near_reach = 1;
    m->updateInteractionSpan();
}


/**
 * @brief When a Pikmin is meant to sigh.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::sigh(Mob* m, void* info1, void* info2) {
    m->setAnimation(PIKMIN_ANIM_SIGHING);
}


/**
 * @brief Causes a sprout to evolve.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::sproutEvolve(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(pik_ptr->maturity == 0 || pik_ptr->maturity == 1) {
        //Leaf to bud, or bud to flower.
        
        pik_ptr->maturity++;
        
        ParticleGenerator pg =
            standardParticleGenSetup(
                game.sys_content_names.part_sprout_evolution, pik_ptr
            );
        pik_ptr->particle_generators.push_back(pg);
        
    } else {
        //Flower to leaf.
        
        pik_ptr->maturity = 0;
        
        ParticleGenerator pg =
            standardParticleGenSetup(
                game.sys_content_names.part_sprout_regression, pik_ptr
            );
        pik_ptr->particle_generators.push_back(pg);
    }
}


/**
 * @brief Schedules the next evolution for a sprout.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::sproutScheduleEvol(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    m->setTimer(pik_ptr->pik_type->sprout_evolution_time[pik_ptr->maturity]);
}


/**
 * @brief When a Pikmin is meant to stand still in place.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::standStill(Mob* m, void* info1, void* info2) {
    m->stopCircling();
    m->stopFollowingPath();
    m->stopChasing();
    m->stopTurning();
    m->speed.x = m->speed.y = 0;
}


/**
 * @brief When a Pikmin should start a random boredom animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::startBoredomAnim(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    size_t looking_around_anim_idx =
        m->type->anim_db->findAnimation("looking_around");
    size_t sitting_anim_idx =
        m->type->anim_db->findAnimation("sitting");
    size_t lounging_anim_idx =
        m->type->anim_db->findAnimation("lounging");
    vector<size_t> boredom_anims;
    if(looking_around_anim_idx != INVALID) {
        boredom_anims.push_back(looking_around_anim_idx);
    }
    if(sitting_anim_idx != INVALID) {
        boredom_anims.push_back(sitting_anim_idx);
    }
    if(lounging_anim_idx != INVALID) {
        boredom_anims.push_back(lounging_anim_idx);
    }
    
    if(boredom_anims.empty()) return;
    size_t anim_idx = boredom_anims[game.rng.i(0, (int) (boredom_anims.size() - 1))];
    m->setAnimation(anim_idx, START_ANIM_OPTION_NORMAL, false);
    pik_ptr->in_bored_animation = true;
}


/**
 * @brief When a Pikmin needs to start chasing after its leader
 * (or the group spot belonging to the leader).
 *
 * @param m The mob.
 * @param info1 Points to the position struct with the final destination.
 *   If nullptr, the final destination is calculated in this function.
 * @param info2 Unused.
 */
void pikmin_fsm::startChasingLeader(Mob* m, void* info1, void* info2) {
    m->focusOnMob(m->following_group);
    pikmin_fsm::updateInGroupChasing(m, nullptr, nullptr);
    m->setAnimation(
        m->holding.empty() ? PIKMIN_ANIM_WALKING : PIKMIN_ANIM_CARRYING,
        START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
}


/**
 * @brief When a Pikmin starts drinking the drop it touched.
 *
 * @param m The mob.
 * @param info1 Pointer to the drop mob.
 * @param info2 Unused.
 */
void pikmin_fsm::startDrinking(Mob* m, void* info1, void* info2) {
    Mob* drop_ptr = (Mob*) info1;
    m->leaveGroup();
    m->stopChasing();
    m->focusOnMob(drop_ptr);
    m->face(getAngle(m->pos, drop_ptr->pos), nullptr);
    m->setAnimation(PIKMIN_ANIM_DRINKING);
}


/**
 * @brief When a Pikmin starts dying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::startDying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    pik_ptr->startDying();
    
    m->leaveGroup();
    pikmin_fsm::beReleased(m, info1, info2);
    pikmin_fsm::notifyLeaderRelease(m, info1, info2);
    pikmin_fsm::releaseTool(m, nullptr, nullptr);
    m->setAnimation(PIKMIN_ANIM_DYING);
}


/**
 * @brief When a Pikmin is killed after being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::startKnockedDownDying(Mob* m, void* info1, void* info2) {
    pikmin_fsm::startDying(m, info1, info2);
    m->setAnimation(PIKMIN_ANIM_KNOCKED_DOWN_DYING);
}


/**
 * @brief When a Pikmin starts flailing.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::startFlailing(Mob* m, void* info1, void* info2) {
    pikmin_fsm::releaseTool(m, nullptr, nullptr);
    
    //If the Pikmin is following a moveable point, let's change it to
    //a static point. This will make the Pikmin continue to move
    //forward into the water in a straight line.
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    float final_z = 0.0f;
    Point final_pos = m->getChaseTarget(&final_z);
    m->chase(final_pos, final_z);
    
    m->leaveGroup();
    
    //Let the Pikmin continue to swim into the water for a bit
    //before coming to a stop. Otherwise the Pikmin would stop nearly
    //on the edge of the water, and that just looks bad.
    m->setTimer(1.0f);
    
    m->setAnimation(PIKMIN_ANIM_FLAILING, START_ANIM_OPTION_RANDOM_TIME);
}


/**
 * @brief When a Pikmin starts getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::startGettingUp(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->setAnimation(PIKMIN_ANIM_GETTING_UP);
}


/**
 * @brief When a Pikmin starts lunging forward for an impact attack.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::startImpactLunge(Mob* m, void* info1, void* info2) {
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    
    m->chase(&m->focused_mob->pos, &m->focused_mob->z);
    m->setAnimation(PIKMIN_ANIM_ATTACKING);
}


/**
 * @brief When a Pikmin lands on a mob and needs to start its landing animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::startMobLanding(Mob* m, void* info1, void* info2) {
    m->setAnimation(PIKMIN_ANIM_MOB_LANDING);
}


/**
 * @brief When a Pikmin starts panicking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::startPanicking(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    m->leaveGroup();
    pikmin_fsm::panicNewChase(m, info1, info2);
    m->setAnimation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
}


/**
 * @brief When a Pikmin starts picking some object up to hold it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::startPickingUp(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    m->setAnimation(PIKMIN_ANIM_PICKING_UP);
}


/**
 * @brief When a Pikmin must start returning to the carried object's
 * return point.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob that used to be carried.
 * @param info2 Unused.
 */
void pikmin_fsm::startReturning(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* carried_mob = (Mob*) info1;
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    PathFollowSettings settings;
    settings.target_point = carried_mob->carry_info->return_point;
    settings.final_target_distance = carried_mob->carry_info->return_dist;
    
    if(carried_mob->carry_info->destination == CARRY_DESTINATION_LINKED_MOB) {
        //Special case: bridges.
        //Pikmin are meant to carry to the current tip of the bridge,
        //but whereas the start of the bridge is on firm ground, the tip may
        //be above a chasm or water, so the Pikmin might want to take a
        //different path, or be unable to take a path at all.
        //Let's fake the start point to be the start of the bridge,
        //for the sake of path calculations.
        if(
            carried_mob->carry_info->intended_mob->type->category->id ==
            MOB_CATEGORY_BRIDGES
        ) {
            Bridge* bri_ptr = (Bridge*) carried_mob->carry_info->intended_mob;
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FAKED_START);
            settings.faked_start = bri_ptr->getStartPoint();
        }
    }
    
    if(
        pik_ptr->followPath(
            settings, pik_ptr->getBaseSpeed(), pik_ptr->type->acceleration
        )
    ) {
        m->setAnimation(
            PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true,
            m->type->move_speed
        );
    } else {
        pik_ptr->fsm.setState(PIKMIN_STATE_IDLING);
    }
}


/**
 * @brief When a Pikmin starts riding on a track.
 *
 * @param m The mob.
 * @param info1 Points to the track mob.
 * @param info2 Unused.
 */
void pikmin_fsm::startRidingTrack(Mob* m, void* info1, void* info2) {
    Track* tra_ptr = (Track*) info1;
    
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->leaveGroup();
    m->stopChasing();
    m->focusOnMob(tra_ptr);
    m->startHeightEffect();
    
    vector<size_t> checkpoints;
    for(size_t c = 0; c < tra_ptr->type->anim_db->body_parts.size(); c++) {
        checkpoints.push_back(c);
    }
    m->track_info =
        new TrackRideInfo(
        tra_ptr, checkpoints, tra_ptr->tra_type->ride_speed
    );
    
    switch(tra_ptr->tra_type->riding_pose) {
    case TRACK_RIDING_POSE_STOPPED: {
        m->setAnimation(PIKMIN_ANIM_WALKING);
        break;
    } case TRACK_RIDING_POSE_CLIMBING: {
        m->setAnimation(PIKMIN_ANIM_CLIMBING);
        break;
    } case TRACK_RIDING_POSE_SLIDING: {
        m->setAnimation(PIKMIN_ANIM_SLIDING);
        break;
    }
    }
}


/**
 * @brief When a Pikmin must no longer be idling.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stopBeingIdle(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    pik_ptr->bump_lock = 0.0f;
    pik_ptr->in_shaking_animation = false;
}


/**
 * @brief When a Pikmin is no longer in the thrown state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stopBeingThrown(Mob* m, void* info1, void* info2) {
    m->removeParticleGenerator(MOB_PARTICLE_GENERATOR_ID_THROW);
}


/**
 * @brief When a Pikmin is meant to release an object it is carrying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stopCarrying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(!pik_ptr->carrying_mob) return;
    
    pik_ptr->carrying_mob->fsm.runEvent(MOB_EV_CARRIER_REMOVED, (void*) pik_ptr);
    
    pik_ptr->carrying_mob = nullptr;
    pik_ptr->setTimer(0);
}


/**
 * @brief When a Pikmin stands still while in a leader's group.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stopInGroup(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    m->stopChasing();
    m->face(0, &m->following_group->pos);
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->setAnimation(PIKMIN_ANIM_IDLING);
    m->setTimer(
        game.rng.f(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin has to choose its carrying animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tickCarrying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(
        pik_ptr->in_carry_struggle_animation &&
        pik_ptr->carrying_mob->carry_info->is_moving
    ) {
        pik_ptr->in_carry_struggle_animation = false;
        pik_ptr->setAnimation(PIKMIN_ANIM_CARRYING);
    } else if(
        !pik_ptr->in_carry_struggle_animation &&
        !pik_ptr->carrying_mob->carry_info->is_moving
    ) {
        pik_ptr->in_carry_struggle_animation = true;
        pik_ptr->setAnimation(PIKMIN_ANIM_CARRYING_STRUGGLE);
    }
}


/**
 * @brief When a Pikmin has to teleport to its spot in the Onion leg.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tickEnteringOnion(Mob* m, void* info1, void* info2) {
    engineAssert(m->track_info != nullptr, m->printStateHistory());
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    
    if(m->tickTrackRide()) {
        //Finished!
        ((Onion*) m->focused_mob)->nest->storePikmin((Pikmin*) m);
    }
}


/**
 * @brief When a Pikmin has to teleport to its spot in a group task.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tickGroupTaskWork(Mob* m, void* info1, void* info2) {
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    GroupTask* tas_ptr = (GroupTask*) (m->focused_mob);
    Point cur_spot_pos = tas_ptr->getSpotPos(pik_ptr);
    float cur_spot_z = tas_ptr->z + tas_ptr->tas_type->spots_z;
    
    pik_ptr->chase(
        cur_spot_pos, cur_spot_z,
        CHASE_FLAG_TELEPORT |
        CHASE_FLAG_TELEPORTS_CONSTANTLY
    );
    pik_ptr->face(
        tas_ptr->angle + tas_ptr->tas_type->worker_pikmin_angle, nullptr, true
    );
    pik_ptr->stopTurning();
}


/**
 * @brief When a Pikmin has to teleport to its spot in a track it is riding.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tickTrackRide(Mob* m, void* info1, void* info2) {
    engineAssert(m->track_info != nullptr, m->printStateHistory());
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(m->tickTrackRide()) {
        //Finished!
        m->fsm.setState(PIKMIN_STATE_IDLING, nullptr, nullptr);
        if(
            pik_ptr->leader_to_return_to &&
            !pik_ptr->leader_to_return_to->to_delete &&
            pik_ptr->leader_to_return_to->health > 0.0f
        ) {
            if(
                !pik_ptr->holding.empty() &&
                pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
            ) {
                m->fsm.setState(
                    PIKMIN_STATE_CALLED_H, pik_ptr->leader_to_return_to, info2
                );
            } else {
                m->fsm.setState(
                    PIKMIN_STATE_CALLED, pik_ptr->leader_to_return_to, info2
                );
            }
        }
    }
}


/**
 * @brief When a Pikmin touches a "eat" hitbox.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::touchedEatHitbox(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    engineAssert(info2 != nullptr, m->printStateHistory());
    
    if(m->invuln_period.time_left > 0) return;
    if(m->health <= 0) {
        return;
    }
    
    for(size_t s = 0; s < m->statuses.size(); s++) {
        if(m->statuses[s].type->turns_inedible) {
            return;
        }
    }
    
    m->fsm.setState(PIKMIN_STATE_GRABBED_BY_ENEMY, info1, info2);
}


/**
 * @brief When a Pikmin touches a hazard.
 *
 * @param m The mob.
 * @param info1 Pointer to the hazard type.
 * @param info2 Pointer to the hitbox that caused this, if any.
 */
void pikmin_fsm::touchedHazard(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Hazard* haz_ptr = (Hazard*) info1;
    
    if(info2) {
        //This is an attack.
        HitboxInteraction* h_info = (HitboxInteraction*) info2;
        if(!pik_ptr->processAttackMiss(h_info)) {
            //It has been decided that this attack missed.
            return;
        }
    }
    
    if(haz_ptr->associated_liquid) {
        bool already_generating = false;
        for(size_t g = 0; g < m->particle_generators.size(); g++) {
            if(
                m->particle_generators[g].id ==
                MOB_PARTICLE_GENERATOR_ID_WAVE_RING
            ) {
                already_generating = true;
                break;
            }
        }
        
        if(!already_generating) {
            ParticleGenerator pg =
                standardParticleGenSetup(
                    game.sys_content_names.part_wave_ring, m
                );
            pg.follow_z_offset = 1.0f;
            adjustKeyframeInterpolatorValues<float>(
                pg.base_particle.size,
            [ = ] (const float & f) { return f * m->radius; }
            );
            pg.id = MOB_PARTICLE_GENERATOR_ID_WAVE_RING;
            m->particle_generators.push_back(pg);
        }
    }
    
    if(pik_ptr->invuln_period.time_left > 0) return;
    MobType::Vulnerability vuln = pik_ptr->getHazardVulnerability(haz_ptr);
    if(vuln.effect_mult == 0.0f) return;
    
    if(!vuln.status_to_apply || !vuln.status_overrides) {
        for(size_t e = 0; e < haz_ptr->effects.size(); e++) {
            pik_ptr->applyStatusEffect(haz_ptr->effects[e], false, true);
        }
    }
    if(vuln.status_to_apply) {
        pik_ptr->applyStatusEffect(vuln.status_to_apply, false, true);
    }
}


/**
 * @brief When a Pikmin is sprayed.
 *
 * @param m The mob.
 * @param info1 Pointer to the spray type.
 * @param info2 Unused.
 */
void pikmin_fsm::touchedSpray(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    SprayType* s = (SprayType*) info1;
    
    for(size_t e = 0; e < s->effects.size(); e++) {
        m->applyStatusEffect(s->effects[e], false, false);
    }
    
    if(s->buries_pikmin) {
        m->fsm.setState(PIKMIN_STATE_SPROUT, nullptr, nullptr);
    }
}


/**
 * @brief When the Pikmin gets grabbed by an enemy. It should try to swap places
 * with the object that it is holding, instead, if possible.
 * If not, it should drop the object and get grabbed like normal.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tryHeldItemHotswap(Mob* m, void* info1, void* info2) {
    assert(!m->holding.empty());
    
    Tool* too_ptr = (Tool*) * (m->holding.begin());
    if(
        !too_ptr->too_type->can_be_hotswapped &&
        hasFlag(too_ptr->holdability_flags, HOLDABILITY_FLAG_ENEMIES)
    ) {
        //This tool can't be hotswapped... The Pikmin has to get chomped.
        pikmin_fsm::releaseTool(m, nullptr, nullptr);
        m->fsm.setState(PIKMIN_STATE_GRABBED_BY_ENEMY);
        return;
    }
    
    //Start by dropping the tool.
    pikmin_fsm::releaseTool(m, nullptr, nullptr);
    //Receive some invulnerability period to make sure it's not hurt by
    //the same attack.
    m->invuln_period.start();
    //Finally, get knocked back on purpose.
    m->leaveGroup();
    pikmin_fsm::beReleased(m, info1, info2);
    pikmin_fsm::notifyLeaderRelease(m, info1, info2);
    m->fsm.setState(PIKMIN_STATE_KNOCKED_BACK);
}


/**
 * @brief When the Pikmin stops latching on to an enemy.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::unlatch(Mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    m->focused_mob->release(m);
    ((Pikmin*) m)->latched = false;
}


/**
 * @brief When the Pikmin should update its destination when chasing the leader.
 *
 * @param m The mob.
 * @param info1 Points to the position struct with the final destination.
 *   If nullptr, the final destination is calculated in this function.
 * @param info2 Unused.
 */
void pikmin_fsm::updateInGroupChasing(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Point target_pos;
    float target_dist; //Unused dummy value.
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    if(!info1) {
        pik_ptr->getGroupSpotInfo(&target_pos, &target_dist);
    } else {
        target_pos = *((Point*) info1);
    }
    
    m->chase(
        target_pos,
        pik_ptr->following_group->z + PIKMIN::FLIER_ABOVE_FLOOR_HEIGHT
    );
    
}


/**
 * @brief When a Pikmin is whistled over by a leader while holding a tool.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void pikmin_fsm::whistledWhileHolding(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Tool* too_ptr = (Tool*) * (m->holding.begin());
    
    if(
        too_ptr->too_type->dropped_when_pikmin_is_whistled &&
        pik_ptr->is_tool_primed_for_whistle
    ) {
        pikmin_fsm::releaseTool(m, nullptr, nullptr);
    }
    
    pik_ptr->is_tool_primed_for_whistle = false;
    
    if(
        !pik_ptr->holding.empty() &&
        pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
    ) {
        m->fsm.setState(PIKMIN_STATE_CALLED_H, info1, info2);
    } else {
        m->fsm.setState(PIKMIN_STATE_CALLED, info1, info2);
    }
    
}


/**
 * @brief When a Pikmin is whistled over by a leader while riding on a track.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void pikmin_fsm::whistledWhileRiding(Mob* m, void* info1, void* info2) {
    engineAssert(m->track_info, m->printStateHistory());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Track* tra_ptr = (Track*) (m->track_info->m);
    
    if(tra_ptr->tra_type->cancellable_with_whistle) {
        m->stopTrackRide();
        if(
            !pik_ptr->holding.empty() &&
            pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
        ) {
            m->fsm.setState(PIKMIN_STATE_CALLED_H, info1, info2);
        } else {
            m->fsm.setState(PIKMIN_STATE_CALLED, info1, info2);
        }
    }
}


/**
 * @brief When the Pikmin should start working on a group task.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::workOnGroupTask(Mob* m, void* info1, void* info2) {
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    
    GroupTask* tas_ptr = (GroupTask*) (m->focused_mob);
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enableFlag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    tas_ptr->addWorker(pik_ptr);
    
    pik_ptr->stopChasing();
    pik_ptr->face(
        tas_ptr->angle + tas_ptr->tas_type->worker_pikmin_angle,
        nullptr
    );
    
    switch(tas_ptr->tas_type->worker_pikmin_pose) {
    case GROUP_TASK_PIKMIN_POSE_STOPPED: {
        pik_ptr->setAnimation(PIKMIN_ANIM_IDLING);
        break;
    }
    case GROUP_TASK_PIKMIN_POSE_ARMS_OUT: {
        pik_ptr->setAnimation(PIKMIN_ANIM_ARMS_OUT);
        break;
    }
    case GROUP_TASK_PIKMIN_POSE_PUSHING: {
        pik_ptr->setAnimation(PIKMIN_ANIM_PUSHING);
        break;
    }
    case GROUP_TASK_PIKMIN_POSE_CARRYING: {
        pik_ptr->setAnimation(PIKMIN_ANIM_CARRYING);
        break;
    }
    }
}
