/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader finite-state machine logic.
 */

#include <algorithm>

#include "leader_fsm.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../game_state/gameplay/gameplay.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob/drop.h"
#include "../mob/leader.h"
#include "../mob/track.h"
#include "../mob_type/leader_type.h"
#include "gen_mob_fsm.h"


using std::unordered_set;


#pragma region FSM


/**
 * @brief Creates the finite-state machine for the leader's logic.
 *
 * @param typ Mob type to create the finite-state machine for.
 */
void LeaderFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", LEADER_STATE_IDLING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::enterIdle);
        }
        efc.newEvent(SCRIPT_EV_ON_LEAVE); {
            efc.run(LeaderFsm::clearBoredomData);
        }
        efc.newEvent(SCRIPT_EV_ON_TICK); {
            efc.run(LeaderFsm::searchSeed);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(LeaderFsm::standStill);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(LeaderFsm::startBoredomAnim);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::checkBoredomAnimEnd);
        }
        efc.newEvent(LEADER_EV_MUST_SEARCH_SEED); {
            efc.run(LeaderFsm::searchSeed);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(LeaderFsm::goPluck);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("called", LEADER_STATE_CALLED); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::called);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::finishCalledAnim);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
    }
    
    efc.newState("active", LEADER_STATE_ACTIVE); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::enterActive);
        }
        efc.newEvent(SCRIPT_EV_ON_LEAVE); {
            efc.run(LeaderFsm::setIsWalkingFalse);
            efc.run(LeaderFsm::setIsTurningFalse);
        }
        efc.newEvent(SCRIPT_EV_ON_TICK); {
            efc.run(LeaderFsm::tickActiveState);
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(LeaderFsm::standStill);
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("idling");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(LeaderFsm::move);
            efc.run(LeaderFsm::setIsWalkingTrue);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(LeaderFsm::standStill);
            efc.run(LeaderFsm::setIsWalkingFalse);
        }
        efc.newEvent(LEADER_EV_HOLDING); {
            efc.run(LeaderFsm::grabMob);
            efc.changeState("holding");
        }
        efc.newEvent(LEADER_EV_START_WHISTLE); {
            efc.changeState("whistling");
        }
        efc.newEvent(LEADER_EV_PUNCH); {
            efc.changeState("punching");
        }
        efc.newEvent(LEADER_EV_DISMISS); {
            efc.changeState("dismissing");
        }
        efc.newEvent(LEADER_EV_SPRAY); {
            efc.changeState("spraying");
        }
        efc.newEvent(LEADER_EV_INVENTORY); {
            efc.run(LeaderFsm::standStill);
            efc.changeState("in_inventory");
        }
        efc.newEvent(LEADER_EV_FALL_ASLEEP); {
            efc.run(LeaderFsm::fallAsleep);
            efc.changeState("sleeping");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(LeaderFsm::goPluck);
            efc.changeState("going_to_pluck");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(MOB_EV_ITCH); {
            efc.changeState("shaking");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("whistling", LEADER_STATE_WHISTLING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::notifyPikminRelease);
            efc.run(LeaderFsm::release);
            efc.run(LeaderFsm::whistle);
        }
        efc.newEvent(SCRIPT_EV_ON_LEAVE); {
            efc.run(LeaderFsm::stopWhistle);
        }
        efc.newEvent(LEADER_EV_STOP_WHISTLE); {
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(LeaderFsm::move);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(LeaderFsm::standStill);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("punching", LEADER_STATE_PUNCHING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::punch);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(LeaderFsm::move);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(LeaderFsm::standStill);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(LeaderFsm::checkPunchDamage);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("holding", LEADER_STATE_HOLDING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::setCorrectStandingAnim);
        }
        efc.newEvent(LEADER_EV_THROW); {
            efc.changeState("throwing");
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(LeaderFsm::notifyPikminRelease);
            efc.run(LeaderFsm::release);
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(LeaderFsm::move);
            efc.run(LeaderFsm::setIsWalkingTrue);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(LeaderFsm::standStill);
            efc.run(LeaderFsm::setIsWalkingFalse);
        }
        efc.newEvent(LEADER_EV_START_WHISTLE); {
            efc.changeState("whistling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::notifyPikminRelease);
            efc.run(LeaderFsm::release);
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.run(LeaderFsm::notifyPikminRelease);
            efc.run(LeaderFsm::release);
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::notifyPikminRelease);
            efc.run(LeaderFsm::release);
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::notifyPikminRelease);
            efc.run(LeaderFsm::release);
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("throwing", LEADER_STATE_THROWING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::doThrow);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(LeaderFsm::move);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(LeaderFsm::standStill);
        }
        efc.newEvent(LEADER_EV_HOLDING); {
            efc.run(LeaderFsm::grabMob);
            efc.changeState("holding");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("dismissing", LEADER_STATE_DISMISSING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::dismiss);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(LeaderFsm::move);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(LeaderFsm::standStill);
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(LEADER_EV_DISMISS); {
            efc.changeState("dismissing");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
    }
    
    efc.newState("spraying", LEADER_STATE_SPRAYING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::spray);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
    }
    
    efc.newState("pain", LEADER_STATE_PAIN); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::setPainAnim);
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("inactive_pain");
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
    }
    
    efc.newState("inactive_pain", LEADER_STATE_INACTIVE_PAIN); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::setPainAnim);
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("pain");
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::beDismissed);
            efc.changeState("idling");
        }
    }
    
    efc.newState("knocked_back", LEADER_STATE_KNOCKED_BACK); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::getKnockedBack);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(LeaderFsm::standStill);
            efc.run(LeaderFsm::getKnockedDown);
            efc.changeState("knocked_down");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState(
        "inactive_knocked_back", LEADER_STATE_INACTIVE_KNOCKED_BACK
    ); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::getKnockedBack);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(LeaderFsm::standStill);
            efc.run(LeaderFsm::getKnockedDown);
            efc.changeState("inactive_knocked_down");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("inactive_thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("knocked_down", LEADER_STATE_KNOCKED_DOWN); {
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("inactive_knocked_down");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::getUpFaster);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.changeState("getting_up");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
    }
    
    efc.newState(
        "inactive_knocked_down", LEADER_STATE_INACTIVE_KNOCKED_DOWN
    ); {
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("knocked_down");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.changeState("inactive_getting_up");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(LeaderFsm::getUpFaster);
            efc.run(LeaderFsm::calledWhileKnockedDown);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("inactive_thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
    }
    
    efc.newState("getting_up", LEADER_STATE_GETTING_UP); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::startGettingUp);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::finishGettingUp);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState(
        "inactive_getting_up", LEADER_STATE_INACTIVE_GETTING_UP
    ); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::startGettingUp);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::finishGettingUp);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(LeaderFsm::calledWhileKnockedDown);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("ko", LEADER_STATE_KO); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::getKod);
        }
    }
    
    efc.newState("in_group_chasing", LEADER_STATE_IN_GROUP_CHASING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::setCorrectStandingAnim);
            efc.run(LeaderFsm::startChasingLeader);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("in_group_stopped");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(LeaderFsm::beDismissed);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.run(LeaderFsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(LeaderFsm::beGrabbedByFriend);
            efc.changeState("held_by_leader");
        }
        efc.newEvent(LEADER_EV_MUST_SEARCH_SEED); {
            efc.run(LeaderFsm::searchSeed);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(LeaderFsm::goPluck);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("inactive_riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("inactive_thrown");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::beDismissed);
            efc.run(LeaderFsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("in_group_stopped", LEADER_STATE_IN_GROUP_STOPPED); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::setCorrectStandingAnim);
            efc.run(LeaderFsm::stopInGroup);
        }
        efc.newEvent(SCRIPT_EV_ON_LEAVE); {
            efc.run(LeaderFsm::clearBoredomData);
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(LeaderFsm::beDismissed);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(LeaderFsm::beGrabbedByFriend);
            efc.changeState("held_by_leader");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(LeaderFsm::startBoredomAnim);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::checkBoredomAnimEnd);
        }
        efc.newEvent(LEADER_EV_MUST_SEARCH_SEED); {
            efc.run(LeaderFsm::searchSeed);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(LeaderFsm::goPluck);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("inactive_riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::beDismissed);
            efc.run(LeaderFsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("going_to_pluck", LEADER_STATE_GOING_TO_PLUCK); {
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(LeaderFsm::startPluck);
            efc.changeState("plucking");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.run(LeaderFsm::signalStopAutoPluck);
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.run(LeaderFsm::beAttacked);
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.changeState("ko");
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("plucking", LEADER_STATE_PLUCKING); {
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::finishPluck);
            efc.changeState("pluck_deciding");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::queueStopAutoPluck);
            efc.run(LeaderFsm::signalStopAutoPluck);
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("inactive_plucking");
        }
    }
    
    efc.newState("pluck_deciding", LEADER_STATE_PLUCK_DECIDING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::decidePluckAction);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(LeaderFsm::goPluck);
            efc.changeState("going_to_pluck");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.changeState("active");
        }
    }
    
    efc.newState(
        "inactive_going_to_pluck", LEADER_STATE_INACTIVE_GOING_TO_PLUCK
    ); {
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(LeaderFsm::startPluck);
            efc.changeState("inactive_plucking");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.changeState("called");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.changeState("ko");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("going_to_pluck");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("inactive_plucking", LEADER_STATE_INACTIVE_PLUCKING); {
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::finishPluck);
            efc.changeState("inactive_pluck_deciding");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(LeaderFsm::joinGroup);
            efc.run(LeaderFsm::queueStopAutoPluck);
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::queueStopAutoPluck);
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("plucking");
        }
    }
    
    efc.newState(
        "inactive_pluck_deciding",
        LEADER_STATE_INACTIVE_PLUCK_DECIDING
    ); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::decidePluckAction);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(LeaderFsm::goPluck);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::stopAutoPluck);
            efc.run(LeaderFsm::idleOrRejoin);
        }
    }
    
    efc.newState("mid_go_here", LEADER_STATE_MID_GO_HERE); {
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("inactive_mid_go_here");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(LeaderFsm::stopGoHere);
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::stopGoHere);
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(LeaderFsm::stopGoHere);
            efc.changeState("ko");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::stopGoHere);
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::stopGoHere);
            efc.run(LeaderFsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::stopGoHere);
            efc.run(LeaderFsm::fallDownPit);
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::stopGoHere);
            efc.changeState("active");
        }
    }
    
    efc.newState("inactive_mid_go_here", LEADER_STATE_INACTIVE_MID_GO_HERE); {
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(LeaderFsm::stopGoHere);
            efc.changeState("called");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("mid_go_here");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(LeaderFsm::stopGoHere);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::stopGoHere);
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(LeaderFsm::stopGoHere);
            efc.changeState("ko");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(LeaderFsm::stopGoHere);
            efc.run(LeaderFsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::stopGoHere);
            efc.run(LeaderFsm::fallDownPit);
        }
    }
    
    efc.newState("in_inventory", LEADER_STATE_IN_INVENTORY); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::standStill);
            efc.run(LeaderFsm::openInventory);
        }
        efc.newEvent(SCRIPT_EV_ON_LEAVE); {
            efc.run(LeaderFsm::closeInventory);
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_SPRAY); {
            efc.changeState("spraying");
        }
        efc.newEvent(LEADER_EV_FALL_ASLEEP); {
            efc.run(LeaderFsm::fallAsleep);
            efc.changeState("sleeping");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
    }
    
    efc.newState("sleeping", LEADER_STATE_SLEEPING); {
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("waking_up");
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("inactive_sleeping");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
            efc.run(LeaderFsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("ko");
        }
    }
    
    efc.newState(
        "inactive_sleeping", LEADER_STATE_INACTIVE_SLEEPING
    ); {
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("inactive_waking_up");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("sleeping");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
            efc.run(LeaderFsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("ko");
        }
    }
    
    efc.newState("waking_up", LEADER_STATE_WAKING_UP); {
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
    }
    
    efc.newState("inactive_waking_up", LEADER_STATE_INACTIVE_WAKING_UP); {
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    
    efc.newState("held_by_leader", LEADER_STATE_HELD); {
        efc.newEvent(SCRIPT_EV_ON_LEAVE); {
            efc.run(LeaderFsm::beReleased);
        }
        efc.newEvent(MOB_EV_THROWN); {
            efc.run(LeaderFsm::beThrown);
            efc.changeState("inactive_thrown");
        }
        efc.newEvent(MOB_EV_RELEASED); {
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("thrown", LEADER_STATE_THROWN); {
        efc.newEvent(SCRIPT_EV_ON_LEAVE); {
            efc.run(LeaderFsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(LeaderFsm::land);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
            efc.changeState("active");
        }
    }
    
    efc.newState("inactive_thrown", LEADER_STATE_INACTIVE_THROWN); {
        efc.newEvent(SCRIPT_EV_ON_LEAVE); {
            efc.run(LeaderFsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(LeaderFsm::land);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(LeaderFsm::beThrownByBouncer);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(LeaderFsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("drinking", LEADER_STATE_DRINKING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::startDrinking);
        }
        efc.newEvent(SCRIPT_EV_ON_LEAVE); {
            efc.run(LeaderFsm::finishDrinking);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(LeaderFsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(LeaderFsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(LeaderFsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("ko");
        }
    }
    
    efc.newState("riding_track", LEADER_STATE_RIDING_TRACK); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::startRidingTrack);
        }
        efc.newEvent(SCRIPT_EV_ON_TICK); {
            efc.run(LeaderFsm::tickTrackRide);
        }
    }
    
    efc.newState(
        "inactive_riding_track", LEADER_STATE_INACTIVE_RIDING_TRACK
    ); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::startRidingTrack);
        }
        efc.newEvent(SCRIPT_EV_ON_TICK); {
            efc.run(LeaderFsm::tickTrackRide);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(LeaderFsm::whistledWhileRiding);
        }
    }
    
    efc.newState("shaking", LEADER_STATE_SHAKING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(LeaderFsm::shake);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::finishShaking);
            efc.changeState("active");
        }
    }
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_LEADER_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_LEADER_STATES) + " in enum."
    );
}


#pragma endregion
#pragma region FSM functions


/**
 * @brief When a leader loses health.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void LeaderFsm::beAttacked(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    float healthBefore = leaPtr->health;
    float offenseMultiplier = 1.0f;
    float defenseMultiplier = 1.0f;
    float damage = 0.0f;
    
    //Initial checks.
    if(leaPtr->invulnPeriod.timeLeft > 0.0f) return;
    if(
        !info->mob2->calculateAttackBasics(
            leaPtr, info->h2, info->h1,
            &offenseMultiplier, &defenseMultiplier
        )
    ) {
        return;
    }
    if(
        !info->mob2->calculateAttackDamage(
            leaPtr, info->h2, info->h1,
            offenseMultiplier, defenseMultiplier, &damage
        )
    ) {
        return;
    }
    
    //Behavior changes.
    leaPtr->stopChasing();
    leaPtr->leaveGroup();
    
    //Damage.
    leaPtr->applyAttackDamage(info->mob2, info->h2, info->h1, damage);
    
    //Knockback.
    bool knockbackExists = false;
    float knockbackStrength = 0.0f;
    float knockbackAngle = 0.0f;
    info->mob2->calculateAttackKnockback(
        leaPtr, info->h2, info->h1, offenseMultiplier, defenseMultiplier,
        &knockbackExists, &knockbackStrength, &knockbackAngle
    );
    if(knockbackExists) {
        leaPtr->applyKnockback(knockbackStrength, knockbackAngle);
    }
    
    //Effects.
    leaPtr->doAttackEffects(
        info->mob2, info->h2, info->h1, damage, knockbackStrength
    );
    if(info->h2->value > 0.0f) {
        leaPtr->healthWheelShaker.shake(1.0f);
    }
    if(healthBefore > 0.0f && leaPtr->health < healthBefore) {
        game.states.gameplay->lastHurtLeaderPos = leaPtr->pos;
        game.statistics.leaderDamageSuffered += healthBefore - leaPtr->health;
    }
    
    //Next state.
    if(knockbackExists) {
        if(knockbackStrength > 0) {
            leaPtr->invulnPeriod.start(LEADER::INVULN_PERIOD_KB);
            if(leaPtr->player) fsm->setState(LEADER_STATE_KNOCKED_BACK);
            else fsm->setState(LEADER_STATE_INACTIVE_KNOCKED_BACK);
        } else {
            leaPtr->invulnPeriod.start(LEADER::INVULN_PERIOD_NORMAL);
            if(leaPtr->player) fsm->setState(LEADER_STATE_PAIN);
            else fsm->setState(LEADER_STATE_INACTIVE_PAIN);
        }
    }
}


/**
 * @brief When a leader is meant to become the active one.
 *
 * @param m The mob.
 * @param info1 Pointer to the player in charge.
 * @param info2 Unused.
 */
void LeaderFsm::becomeActive(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Player* player = (Player*) info1;
    
    if(player->leaderPtr) {
        player->leaderPtr->fsm.runEvent(LEADER_EV_INACTIVATED);
    }
    
    //Normally the player can't swap to leaders that are following another,
    //but some complex cases may allow that (e.g. an inactive leader got
    //whistled by another and then swapped to mid-pluck).
    //Let's swap the group members over.
    if(
        leaPtr->followingGroup &&
        leaPtr->followingGroup->type->category->id == MOB_CATEGORY_LEADERS
    ) {
        Mob* oldLeader = leaPtr->followingGroup;
        leaPtr->leaveGroup();
        oldLeader->fsm.runEvent(MOB_EV_WHISTLED, (void*) leaPtr);
    }
    
    //Update pointers and such.
    size_t newLeaderIdx = player->leaderIdx;
    for(size_t l = 0; l < game.states.gameplay->availableLeaders.size(); l++) {
        if(game.states.gameplay->availableLeaders[l] == leaPtr) {
            newLeaderIdx = l;
            break;
        }
    }
    
    player->leaderPtr = leaPtr;
    player->leaderIdx = newLeaderIdx;
    leaPtr->player = player;
    
    //Check if we're in the middle of loading or of an interlude. If so
    //that probably means it's the first leader at the start of the area.
    //We should probably not play the name call then.
    if(
        !game.states.gameplay->loading &&
        game.states.gameplay->interlude.get() == INTERLUDE_NONE
    ) {
        //Play the name call as a global sound, so that even leaders far away
        //can have their name call play clearly.
        size_t nameCallSoundIdx =
            leaPtr->leaType->soundDataIdxs[LEADER_SOUND_NAME_CALL];
        if(nameCallSoundIdx != INVALID) {
            MobType::Sound* nameCallSound =
                &leaPtr->type->sounds[nameCallSoundIdx];
            game.audio.addNewGlobalSoundSource(
                nameCallSound->sample,
                false, nameCallSound->config
            );
        }
    }
}


/**
 * @brief When a leader stops being the active one.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::becomeInactive(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    leaPtr->player = nullptr;
    leaPtr->stopAutoThrowing();
}


/**
 * @brief When a leader's leader dismisses them.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::beDismissed(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->stopChasing();
    leaPtr->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When a leader is grabbed by another leader.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::beGrabbedByFriend(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When a leader grabbed by another is released.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::beReleased(Fsm* fsm, void* info1, void* info2) {

}


/**
 * @brief When a leader grabbed by another is thrown.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::beThrown(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->startThrowTrail();
    leaPtr->setAnimation(LEADER_ANIM_THROWN);
}


/**
 * @brief When a leader is thrown by a bouncer mob.
 *
 * @param m The mob.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void LeaderFsm::beThrownByBouncer(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    leaPtr->startThrowTrail();
    leaPtr->setAnimation(LEADER_ANIM_THROWN);
    if(!leaPtr->player) {
        leaPtr->leaveGroup();
    }
}


/**
 * @brief When a leader is called and must jump in surprise.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void LeaderFsm::called(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Mob* caller = (Mob*) info1;
    
    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    LeaderFsm::standStill(fsm, info1, info2);
    
    leaPtr->focusOnMob(caller);
    
    leaPtr->setAnimation(LEADER_ANIM_CALLED);
}


/**
 * @brief When a leader that is knocked down is called over by another leader,
 * by whistling them.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void LeaderFsm::calledWhileKnockedDown(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Mob* caller = (Mob*) info1;
    
    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    leaPtr->focusOnMob(caller);
}


/**
 * @brief When a leader should check if the animation that ended is a boredom
 * animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::checkBoredomAnimEnd(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    if(!leaPtr->inBoredAnimation) return;
    leaPtr->setAnimation(LEADER_ANIM_IDLING);
    leaPtr->inBoredAnimation = false;
    leaPtr->setTimer(
        game.rng.f(LEADER::BORED_ANIM_MIN_DELAY, LEADER::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a leader should check how much damage they've caused
 * with their punch.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::checkPunchDamage(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    float offenseMultiplier = 0;
    float defenseMultiplier = 0;
    float damage = 0;
    if(
        info->mob2->health > 0.0f &&
        leaPtr->canHurt(info->mob2) &&
        leaPtr->calculateAttackBasics(
            info->mob2, info->h1, info->h2,
            &offenseMultiplier, &defenseMultiplier
        ) &&
        leaPtr->calculateAttackDamage(
            info->mob2, info->h1, info->h2,
            offenseMultiplier, defenseMultiplier, &damage
        )
    ) {
        game.statistics.punchDamageCaused += damage;
    }
}


/**
 * @brief When a leader has to clear any data about being bored.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::clearBoredomData(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    LeaderFsm::clearTimer(fsm, info1, info2);
    leaPtr->inBoredAnimation = false;
}


/**
 * @brief When a Pikmin has to clear any timer set.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::clearTimer(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->setTimer(0);
}


/**
 * @brief When a leader closes the inventory.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::closeInventory(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    if(!leaPtr->player) return;
    leaPtr->player->inventory->close();
}


/**
 * @brief When a leader must decide what to do next after plucking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::decidePluckAction(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    Distance d;
    Pikmin* newPikmin = nullptr;
    
    if(!leaPtr->queuedPluckCancel) {
        newPikmin =
            getClosestSprout(leaPtr->pos, &d, false);
    }
    
    if(leaPtr->queuedPluckCancel) {
        //It should only signal to stop if it wanted to stop.
        //If there are no more sprouts in range, that doesn't mean the leaders
        //following it can't continue with the sprouts in their range.
        LeaderFsm::signalStopAutoPluck(fsm, info1, info2);
    }
    
    leaPtr->queuedPluckCancel = false;
    
    if(newPikmin && d <= game.config.leaders.nextPluckRange) {
        fsm->runEvent(LEADER_EV_GO_PLUCK, (void*) newPikmin);
    } else {
        fsm->runEvent(LEADER_EV_CANCEL);
    }
}


/**
 * @brief When a leader dismisses the group.
 *
 * @param m The mob.
 * @param info1 If not nullptr, then the dismiss must be silent.
 * @param info2 Unused.
 */
void LeaderFsm::dismiss(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->dismiss(info1 != nullptr);
}


/**
 * @brief When a leader throws the grabbed mob.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::doThrow(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    Mob* heldPtr = leaPtr->getMobHeldInHand();
    if(!heldPtr) return;
    
    heldPtr->fsm.runEvent(MOB_EV_THROWN);
    heldPtr->startHeightEffect();
    
    heldPtr->stopChasing();
    heldPtr->pos = leaPtr->pos;
    heldPtr->z = leaPtr->z;
    
    heldPtr->zCap = leaPtr->throweeMaxZ;
    
    heldPtr->face(leaPtr->throweeAngle, nullptr, true);
    heldPtr->speed = leaPtr->throweeSpeed;
    heldPtr->speedZ = leaPtr->throweeSpeedZ;
    
    enableFlag(heldPtr->flags, MOB_FLAG_WAS_THROWN);
    heldPtr->leaveGroup();
    leaPtr->release(heldPtr);
    
    leaPtr->setAnimation(LEADER_ANIM_THROWING);
    
    if(heldPtr->type->category->id == MOB_CATEGORY_PIKMIN) {
        game.statistics.pikminThrown++;
    }
}


/**
 * @brief When a leader enters the active state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::enterActive(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->setAnimation(
        LEADER_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief When a leader enters the idling state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::enterIdle(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->unfocusFromMob();
    leaPtr->setAnimation(
        LEADER_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    
    leaPtr->setTimer(
        game.rng.f(LEADER::BORED_ANIM_MIN_DELAY, LEADER::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a leader falls asleep.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::fallAsleep(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    LeaderFsm::dismiss(fsm, nullptr, nullptr);
    leaPtr->stopChasing();
    
    leaPtr->setAnimation(LEADER_ANIM_SLEEPING);
    if(leaPtr->leaType->sleepingStatus) {
        leaPtr->applyStatus(leaPtr->leaType->sleepingStatus, false, false);
    }
}


/**
 * @brief When a leader falls down a bottomless pit.
 * This damages and respawns them.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::fallDownPit(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->leaveGroup();
    leaPtr->setHealth(true, true, -0.2);
    leaPtr->invulnPeriod.start(LEADER::INVULN_PERIOD_NORMAL);
    leaPtr->respawn();
}


/**
 * @brief When a leader finished the animation for when it's called.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::finishCalledAnim(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    Mob* caller = leaPtr->focusedMob;
    
    if(leaPtr) {
        LeaderFsm::joinGroup(fsm, (void*) caller, info2);
        fsm->setState(LEADER_STATE_IN_GROUP_CHASING, info1, info2);
    } else {
        fsm->setState(LEADER_STATE_IDLING, info1, info2);
    }
}


/**
 * @brief When a leader finishes drinking the drop it was drinking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::finishDrinking(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    engineAssert(leaPtr->focusedMob != nullptr, fsm->printStateHistory());
    Drop* droPtr = (Drop*) leaPtr->focusedMob;
    
    switch(droPtr->droType->effect) {
    case DROP_EFFECT_INCREASE_SPRAYS: {
        size_t playerTeamIdx = leaPtr->getPlayerTeamIdx();
        if(playerTeamIdx == INVALID) break;
        game.states.gameplay->changeSprayCount(
            &game.states.gameplay->playerTeams[playerTeamIdx],
            droPtr->droType->sprayTypeToIncrease,
            droPtr->droType->increaseAmount
        );
        break;
    } case DROP_EFFECT_GIVE_STATUS: {
        leaPtr->applyStatus(droPtr->droType->statusToGive, false, false, droPtr);
        break;
    } default: {
        break;
    }
    }
    
    leaPtr->unfocusFromMob();
}


/**
 * @brief When a leader finishes getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::finishGettingUp(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    Mob* prevFocusedMob = leaPtr->focusedMob;
    
    if(leaPtr->player) {
        fsm->setState(LEADER_STATE_ACTIVE);
    } else {
        fsm->setState(LEADER_STATE_IDLING);
    }
    
    if(prevFocusedMob) {
        if(
            prevFocusedMob->type->category->id == MOB_CATEGORY_LEADERS &&
            !leaPtr->canHunt(prevFocusedMob)
        ) {
            fsm->runEvent(MOB_EV_WHISTLED, (void*) prevFocusedMob);
        }
    }
}


/**
 * @brief When the leader finishes the animation of the current pluck.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::finishPluck(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    leaPtr->stopChasing();
    leaPtr->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When the leader finishes the shaking animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::finishShaking(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->invulnPeriod.start(LEADER::INVULN_PERIOD_NORMAL);
}


/**
 * @brief When a leader needs gets knocked back.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::getKnockedBack(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->unfocusFromMob();
    leaPtr->setAnimation(LEADER_ANIM_KNOCKED_BACK);
}


/**
 * @brief When a leader gets knocked back and lands on the floor.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::getKnockedDown(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->stopTurning();
    
    //Let's use the "temp" variable to specify whether or not
    //it already received the getting up timer bonus.
    leaPtr->tempI = 0;
    
    leaPtr->setTimer(leaPtr->leaType->knockedDownDuration);
    
    leaPtr->setAnimation(LEADER_ANIM_SLEEPING);
}


/**
 * @brief When a leader gets KO'd (dies).
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::getKod(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    if(game.states.gameplay->unloading) {
        return;
    }
    
    leaPtr->startDying();
    leaPtr->finishDying();
    
    game.states.gameplay->updateAvailableLeaders();
    if(leaPtr->player) {
        changeToNextLeader(leaPtr->player, true, true, true);
    }
    
    LeaderFsm::release(fsm, info1, info2);
    LeaderFsm::dismiss(fsm, info1, info2);
    leaPtr->becomeUncarriable();
    leaPtr->setAnimation(LEADER_ANIM_KO);
    
    game.states.gameplay->lastHurtLeaderPos = leaPtr->pos;
}


/**
 * @brief When a leader must get up faster from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::getUpFaster(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    //Let's use the "temp" variable to specify whether or not
    //it already received the getting up timer bonus.
    if(leaPtr->tempI == 1) return;
    
    fsm->timer.timeLeft =
        std::max(
            0.01f,
            fsm->timer.timeLeft -
            leaPtr->leaType->knockedDownWhistleBonus
        );
    leaPtr->tempI = 1;
}


/**
 * @brief When a leader heads towards a Pikmin with the intent to pluck it.
 * Also signals other leaders in the group to search for other seeds.
 *
 * @param m The mob.
 * @param info1 Pointer to the Pikmin to be plucked.
 * @param info2 Unused.
 */
void LeaderFsm::goPluck(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Pikmin* pikPtr = (Pikmin*) info1;
    
    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    leaPtr->queuedPluckCancel = false;
    
    leaPtr->autoPlucking = true;
    leaPtr->pluckTarget = pikPtr;
    leaPtr->chase(
        &pikPtr->pos, &pikPtr->z,
        Point(), 0.0f,
        CHASE_FLAG_ANY_ANGLE,
        pikPtr->radius + leaPtr->radius
    );
    pikPtr->pluckReserved = true;
    
    //Now for the leaders in the group.
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); l++) {
        Leader* l2Ptr = game.states.gameplay->mobs.leaders[l];
        if(l2Ptr->followingGroup == leaPtr) {
            l2Ptr->fsm.runEvent(LEADER_EV_MUST_SEARCH_SEED);
        }
    }
    
    LeaderFsm::setIsWalkingTrue(fsm, nullptr, nullptr);
}


/**
 * @brief When a leader grabs onto a mob for throwing.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob to grab.
 * @param info2 Unused.
 */
void LeaderFsm::grabMob(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Mob* grabbedMob = (Mob*) info1;

    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    leaPtr->hold(
        grabbedMob, HOLD_TYPE_PURPOSE_HAND, INVALID,
        LEADER::HELD_GROUP_MEMBER_H_DIST, LEADER::HELD_GROUP_MEMBER_ANGLE,
        LEADER::HELD_GROUP_MEMBER_V_DIST,
        false, HOLD_ROTATION_METHOD_FACE_HOLDER
    );
    leaPtr->group->sort(grabbedMob->subgroupTypePtr);
}


/**
 * @brief When a leader must either return to idling, or return to rejoining
 * its leader.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::idleOrRejoin(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    if(leaPtr->followingGroup) {
        fsm->setState(LEADER_STATE_IN_GROUP_CHASING);
    } else {
        fsm->setState(LEADER_STATE_IDLING);
    }
}


/**
 * @brief When a leader joins another leader's group. This transfers
 * their Pikmin.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void LeaderFsm::joinGroup(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Leader* caller = (Leader*) info1;
    
    Mob* topLeader = caller;
    
    if(topLeader->followingGroup) {
        //If this leader is following another one,
        //then the new leader should be in the group of that top leader.
        topLeader = topLeader->followingGroup;
    }
    
    topLeader->addToGroup(leaPtr);
    while(!leaPtr->group->members.empty()) {
        Mob* member = leaPtr->group->members[0];
        member->leaveGroup();
        topLeader->addToGroup(member);
    }
}


/**
 * @brief When a thrown leader lands.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::land(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->stopChasing();
    leaPtr->speed.x = leaPtr->speed.y = 0;
    
    leaPtr->deleteParticleGenerator(MOB_PARTICLE_GENERATOR_ID_THROW);
    
    if(leaPtr->player) {
        fsm->setState(LEADER_STATE_ACTIVE);
    } else {
        fsm->setState(LEADER_STATE_IDLING);
    }
}


/**
 * @brief When a leader leaves a hazardous sector.
 *
 * @param m The mob.
 * @param info1 Points to the hazard.
 * @param info2 Unused.
 */
void LeaderFsm::leftHazard(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Hazard* h = (Hazard*) info1;

    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    if(h->associatedLiquid) {
        leaPtr->deleteParticleGenerator(MOB_PARTICLE_GENERATOR_ID_WAVE_RING);
    }
}


/**
 * @brief When a leader should lose his momentum and stand still.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::loseMomentum(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->stopChasing();
    leaPtr->speed.x = leaPtr->speed.y = 0;
}


/**
 * @brief When a leader begins to move via player control.
 *
 * @param m The mob.
 * @param info1 Pointer to the movement info structure.
 * @param info2 Unused.
 */
void LeaderFsm::move(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    MovementInfo* mov = (MovementInfo*) info1;

    engineAssert(info1 != nullptr, fsm->printStateHistory());

    Point finalCoords;
    float dummyAngle;
    float dummyMagnitude;
    mov->getInfo(
        &finalCoords, &dummyAngle, &dummyMagnitude
    );
    finalCoords *= leaPtr->type->moveSpeed;
    finalCoords += leaPtr->pos;
    leaPtr->chase(
        finalCoords, leaPtr->z, CHASE_FLAG_ANY_ANGLE,
        PATHS::DEF_CHASE_TARGET_DISTANCE,
        leaPtr->getBaseSpeed() * leaPtr->player->leaderSpeedMult
    );
}


/**
 * @brief When a leader notifies the mob it's holding that it will be released.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::notifyPikminRelease(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    Mob* heldPtr = leaPtr->getMobHeldInHand();
    if(!heldPtr) return;
    heldPtr->fsm.runEvent(MOB_EV_RELEASED);
}


/**
 * @brief When a leader opens the inventory.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::openInventory(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    if(!leaPtr->player) return;
    leaPtr->player->inventory->open();
}


/**
 * @brief When a leader punches.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::punch(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->stopTurning();
    leaPtr->setAnimation(LEADER_ANIM_PUNCHING);
}


/**
 * @brief Queues the stopping of the plucking session, for after this
 * pluck's end.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::queueStopAutoPluck(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    leaPtr->queuedPluckCancel = true;
}


/**
 * @brief When a leader gently releases the held mob.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::release(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    Mob* heldPtr = leaPtr->getMobHeldInHand();
    if(!heldPtr) return;

    //Reset the Pikmin's position to match the leader's,
    //so that the leader doesn't release the Pikmin inside a wall behind them.
    heldPtr->pos = leaPtr->pos;
    heldPtr->z = leaPtr->z;
    heldPtr->face(leaPtr->angle + TAU / 2.0f, nullptr, true);
    leaPtr->release(heldPtr);
}


/**
 * @brief When a leader searches for a seed next to them.
 * If found, issues events to go towards the seed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::searchSeed(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    Distance d;
    Pikmin* newPikmin = nullptr;
    if(!leaPtr->queuedPluckCancel) {
        newPikmin =
            getClosestSprout(leaPtr->pos, &d, false);
    }
    
    if(newPikmin && d <= game.config.leaders.nextPluckRange) {
        fsm->runEvent(LEADER_EV_GO_PLUCK, (void*) newPikmin);
    }
}


/**
 * @brief When a leader needs to update its animation to one of the "standing"
 * animations.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setCorrectStandingAnim(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    size_t walkAnimIdx =
        leaPtr->anim.animDb->preNamedConversions[LEADER_ANIM_WALKING];
    size_t idleAnimIdx =
        leaPtr->anim.animDb->preNamedConversions[LEADER_ANIM_IDLING];
    bool mustUseWalkingAnim =
        leaPtr->isActiveWalking || leaPtr->isActiveTurning;
    bool inWalkingAnim =
        leaPtr->anim.curAnim ==
        leaPtr->leaType->animDb->animations[walkAnimIdx];
    bool inIdlingAnim =
        leaPtr->anim.curAnim ==
        leaPtr->leaType->animDb->animations[idleAnimIdx];
        
    if(mustUseWalkingAnim && !inWalkingAnim) {
        leaPtr->setAnimation(LEADER_ANIM_WALKING);
    } else if(!mustUseWalkingAnim && !inIdlingAnim) {
        leaPtr->setAnimation(LEADER_ANIM_IDLING);
    }
}


/**
 * @brief When a leader is no longer turning in place.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setIsTurningFalse(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    if(leaPtr->isActiveTurning) {
        leaPtr->isActiveTurning = false;
        LeaderFsm::trySetCorrectStandingAnim(fsm, info1, info2);
    }
}


/**
 * @brief When a leader starts turning in place.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setIsTurningTrue(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    if(!leaPtr->isActiveTurning) {
        leaPtr->isActiveTurning = true;
        LeaderFsm::trySetCorrectStandingAnim(fsm, info1, info2);
    }
}


/**
 * @brief When a leader is no longer walking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setIsWalkingFalse(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    if(leaPtr->isActiveWalking) {
        leaPtr->isActiveWalking = false;
        LeaderFsm::trySetCorrectStandingAnim(fsm, info1, info2);
    }
}


/**
 * @brief When a leader starts walking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setIsWalkingTrue(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    if(!leaPtr->isActiveWalking) {
        leaPtr->isActiveWalking = true;
        LeaderFsm::trySetCorrectStandingAnim(fsm, info1, info2);
    }
}


/**
 * @brief When a leader needs to change to the knocked back animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setPainAnim(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->setAnimation(LEADER_ANIM_PAIN);
}


/**
 * @brief When the leader must shake latched Pikmin off.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::shake(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    LeaderFsm::standStill(fsm, info1, info2);
    leaPtr->setAnimation(LEADER_ANIM_SHAKING);
}


/**
 * @brief When the leader must signal to their follower leaders to
 * stop plucking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::signalStopAutoPluck(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); l++) {
        Leader* l2Ptr = game.states.gameplay->mobs.leaders[l];
        if(l2Ptr->followingGroup == leaPtr) {
            l2Ptr->fsm.runEvent(LEADER_EV_CANCEL);
        }
    }
}


/**
 * @brief When a leader uses a spray.
 *
 * @param m The mob.
 * @param info1 Pointer to a size_t with the spray's index.
 * @param info2 Unused.
 */
void LeaderFsm::spray(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    size_t sprayIdx = *((size_t*) info1);

    SprayType& sprayTypeRef = *game.config.misc.sprayOrder[sprayIdx];
    
    if(leaPtr->player->team->sprayStats[sprayIdx].nrSprays == 0) {
        fsm->setState(LEADER_STATE_ACTIVE);
        return;
    }
    
    float shootAngle =
        leaPtr->angle + ((sprayTypeRef.angle) ? TAU / 2.0f : 0.0f);
    
    unordered_set<Mob*> affectedMobs;
    if(sprayTypeRef.affectsUser) {
        affectedMobs.insert(leaPtr);
    }
    
    if(sprayTypeRef.group) {
        for(size_t gm = 0; gm < leaPtr->group->members.size(); gm++) {
            Mob* gmPtr = leaPtr->group->members[gm];
            if(
                gmPtr->type->category->id != MOB_CATEGORY_PIKMIN &&
                sprayTypeRef.groupPikminOnly
            ) {
                continue;
            }
            
            affectedMobs.insert(gmPtr);
        }
        //If there is nothing to get sprayed, better not waste it.
        if(affectedMobs.empty())  {
            fsm->setState(LEADER_STATE_ACTIVE);
            return;
        };
        
    } else {
        for(
            size_t am = 0; am < game.states.gameplay->mobs.all.size(); am++
        ) {
            Mob* amPtr = game.states.gameplay->mobs.all[am];
            if(amPtr == leaPtr) continue;
            
            if(
                Distance(leaPtr->pos, amPtr->pos) >
                sprayTypeRef.distanceRange + amPtr->radius
            ) {
                continue;
            }
            
            float angleDiff =
                getAngleSmallestDiff(
                    shootAngle,
                    getAngle(leaPtr->pos, amPtr->pos)
                );
            if(angleDiff > sprayTypeRef.angleRange / 2) continue;
            
            affectedMobs.insert(amPtr);
        }
        
    }
    
    for(auto& am : affectedMobs) {
        am->fsm.runEvent(
            MOB_EV_TOUCHED_SPRAY, (void*) game.config.misc.sprayOrder[sprayIdx]
        );
    }
    
    Point particleSpeedVector =
        rotatePoint(
            Point(sprayTypeRef.distanceRange * 0.8, 0),
            sprayTypeRef.angle
        );
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parSpray, leaPtr
        );
    adjustKeyframeInterpolatorValues<Point>(
        pg.baseParticle.linearSpeed,
    [ = ] (const Point&) { return particleSpeedVector; }
    );
    adjustKeyframeInterpolatorValues<ALLEGRO_COLOR>(
        pg.baseParticle.color,
    [ = ] (const ALLEGRO_COLOR & c) {
        ALLEGRO_COLOR newColor = c;
        newColor.r *= sprayTypeRef.mainColor.r;
        newColor.g *= sprayTypeRef.mainColor.g;
        newColor.b *= sprayTypeRef.mainColor.b;
        newColor.a *= sprayTypeRef.mainColor.a;
        return newColor;
    }
    );
    pg.linearSpeedAngleDeviation = sprayTypeRef.angleRange / 2.0f;
    pg.linearSpeedDeviation.x = sprayTypeRef.distanceRange * 0.4;
    pg.baseParticle.priority = PARTICLE_PRIORITY_HIGH;
    leaPtr->particleGenerators.push_back(pg);
    
    game.states.gameplay->changeSprayCount(leaPtr->player->team, sprayIdx, -1);
    
    leaPtr->stopChasing();
    leaPtr->setAnimation(LEADER_ANIM_SPRAYING);
    
    game.statistics.spraysUsed++;
}


/**
 * @brief When a leader stops moving.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::standStill(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->stopCircling();
    leaPtr->stopFollowingPath();
    leaPtr->stopChasing();
    leaPtr->speed.x = leaPtr->speed.y = 0;
}


/**
 * @brief When a leader should start a random boredom animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::startBoredomAnim(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    size_t lookingAroundAnimIdx =
        leaPtr->type->animDb->findAnimation("looking_around");
    size_t sittingAnimIdx =
        leaPtr->type->animDb->findAnimation("sitting");
    size_t stretchingAnimIdx =
        leaPtr->type->animDb->findAnimation("stretching");
    vector<size_t> boredomAnims;
    if(lookingAroundAnimIdx != INVALID) {
        boredomAnims.push_back(lookingAroundAnimIdx);
    }
    if(sittingAnimIdx != INVALID) {
        boredomAnims.push_back(sittingAnimIdx);
    }
    if(stretchingAnimIdx != INVALID) {
        boredomAnims.push_back(stretchingAnimIdx);
    }
    
    if(boredomAnims.empty()) return;
    size_t animIdx =
        boredomAnims[game.rng.i(0, (int) (boredomAnims.size() - 1))];
    leaPtr->setAnimation(animIdx, START_ANIM_OPTION_NORMAL, false);
    leaPtr->inBoredAnimation = true;
}


/**
 * @brief When a leader must start chasing another.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::startChasingLeader(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->focusOnMob(leaPtr->followingGroup);
    LeaderFsm::updateInGroupChasing(fsm, nullptr, nullptr);
}


/**
 * @brief When a leader starts drinking the drop it touched.
 *
 * @param m The mob.
 * @param info1 Pointer to the drop mob.
 * @param info2 Unused.
 */
void LeaderFsm::startDrinking(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Mob* droPtr = (Mob*) info1;

    leaPtr->leaveGroup();
    leaPtr->stopChasing();
    leaPtr->focusOnMob(droPtr);
    leaPtr->face(getAngle(leaPtr->pos, droPtr->pos), nullptr);
    leaPtr->setAnimation(LEADER_ANIM_DRINKING);
}


/**
 * @brief When a leader starts getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::startGettingUp(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->setAnimation(LEADER_ANIM_GETTING_UP);
}


/**
 * @brief When a leader starts a Go Here walk.
 *
 * @param m The mob.
 * @param info1 Destination point.
 * @param info2 Unused.
 */
void LeaderFsm::startGoHere(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Point destination = *((Point*) info1);
    
    PathFollowSettings settings;
    settings.targetPoint = destination;
    
    float speed = leaPtr->getBaseSpeed();
    for(size_t gm = 0; gm < leaPtr->group->members.size(); gm++) {
        //It can only go as fast as its slowest member.
        speed = std::min(speed, leaPtr->group->members[gm]->getBaseSpeed());
    }
    
    bool success =
        leaPtr->followPath(
            settings, speed, leaPtr->type->acceleration
        );
        
    if(success) {
        fsm->setState(
            leaPtr->player ?
            LEADER_STATE_MID_GO_HERE :
            LEADER_STATE_INACTIVE_MID_GO_HERE
        );
        leaPtr->midGoHere = true;
        LeaderFsm::setIsWalkingTrue(fsm, nullptr, nullptr);
        LeaderFsm::setCorrectStandingAnim(fsm, nullptr, nullptr);
    }
}


/**
 * @brief When a leader grabs on to a sprout and begins plucking it out.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::startPluck(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    engineAssert(leaPtr->pluckTarget != nullptr, fsm->printStateHistory());
    
    leaPtr->pluckTarget->fsm.runEvent(MOB_EV_PLUCKED, (void*) leaPtr);
    leaPtr->pluckTarget->pluckReserved = false;
    leaPtr->pluckTarget = nullptr;
    leaPtr->setAnimation(LEADER_ANIM_PLUCKING);
    LeaderFsm::setIsWalkingFalse(fsm, nullptr, nullptr);
    
}


/**
 * @brief When a leader starts riding on a track.
 *
 * @param m The mob.
 * @param info1 Points to the track mob.
 * @param info2 Unused.
 */
void LeaderFsm::startRidingTrack(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Track* traPtr = (Track*) info1;
    
    LeaderFsm::dismiss(fsm, nullptr, nullptr);
    leaPtr->leaveGroup();
    leaPtr->stopChasing();
    leaPtr->focusOnMob(traPtr);
    leaPtr->startHeightEffect();
    
    vector<size_t> checkpoints;
    for(size_t c = 0; c < traPtr->type->animDb->bodyParts.size(); c++) {
        checkpoints.push_back(c);
    }
    leaPtr->trackInfo =
        new TrackRideInfo(
        traPtr, checkpoints, traPtr->traType->rideSpeed
    );
    
    switch(traPtr->traType->ridingPose) {
    case TRACK_RIDING_POSE_STOPPED: {
        leaPtr->setAnimation(LEADER_ANIM_IDLING);
        break;
    } case TRACK_RIDING_POSE_CLIMBING: {
        leaPtr->setAnimation(LEADER_ANIM_CLIMBING);
        break;
    } case TRACK_RIDING_POSE_SLIDING: {
        leaPtr->setAnimation(LEADER_ANIM_SLIDING);
        break;
    }
    }
}


/**
 * @brief When a leader wakes up.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::startWakingUp(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    leaPtr->setAnimation(LEADER_ANIM_GETTING_UP);
    
    if(leaPtr->leaType->sleepingStatus) {
        for(size_t s = 0; s < leaPtr->statuses.size(); s++) {
            if(leaPtr->statuses[s].type == leaPtr->leaType->sleepingStatus) {
                leaPtr->statuses[s].prevState = leaPtr->statuses[s].state;
                leaPtr->statuses[s].state = STATUS_STATE_TO_DELETE;
            }
        }
    }
}


/**
 * @brief When a leader quits the auto-plucking mindset.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::stopAutoPluck(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    if(leaPtr->pluckTarget) {
        leaPtr->stopChasing();
        leaPtr->pluckTarget->pluckReserved = false;
    }
    leaPtr->autoPlucking = false;
    leaPtr->queuedPluckCancel = false;
    leaPtr->pluckTarget = nullptr;
    leaPtr->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When a leader is no longer in the thrown state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::stopBeingThrown(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    //Delete the throw particle generator.
    leaPtr->deleteParticleGenerator(MOB_PARTICLE_GENERATOR_ID_THROW);
}


/**
 * @brief When a leader stops a Go Here walk.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::stopGoHere(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    leaPtr->stopFollowingPath();
    leaPtr->midGoHere = false;
}


/**
 * @brief When a leader stands still while in another's group.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::stopInGroup(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->stopChasing();
    LeaderFsm::setIsWalkingFalse(fsm, nullptr, nullptr);
    leaPtr->setTimer(
        game.rng.f(LEADER::BORED_ANIM_MIN_DELAY, LEADER::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a leader stops whistling.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::stopWhistle(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->stopWhistling();
}


/**
 * @brief Every tick in the active state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::tickActiveState(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    leaPtr->face(getAngle(leaPtr->pos, leaPtr->player->leaderCursorWorld), nullptr);
    
    bool shouldBeTurning =
        getAngleSmallestDiff(leaPtr->angle, leaPtr->intendedTurnAngle) > TAU / 300.0f;
    if(shouldBeTurning) {
        LeaderFsm::setIsTurningTrue(fsm, info1, info2);
    } else {
        LeaderFsm::setIsTurningFalse(fsm, info1, info2);
    }
}


/**
 * @brief When a leader has to teleport to its spot in a track it is riding.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::tickTrackRide(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    engineAssert(leaPtr->trackInfo != nullptr, fsm->printStateHistory());
    
    if(leaPtr->tickTrackRide()) {
        //Finished!
        if(leaPtr->player) {
            fsm->setState(LEADER_STATE_ACTIVE, nullptr, nullptr);
        } else {
            fsm->setState(LEADER_STATE_IDLING, nullptr, nullptr);
        }
    }
}


/**
 * @brief When a leader touches a hazard.
 *
 * @param m The mob.
 * @param info1 Pointer to the hazard.
 * @param info2 Pointer to the hitbox that caused this, if any.
 */
void LeaderFsm::touchedHazard(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Hazard* hazPtr = (Hazard*) info1;

    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    HitboxInteraction* hitboxInfo = (HitboxInteraction*) info2;
    MobType::Vulnerability vuln = leaPtr->getHazardVulnerability(hazPtr);
    Mob* hitboxMob = nullptr;
    if(hitboxInfo) hitboxMob = hitboxInfo->mob2;
    
    if(!vuln.statusToApply || !vuln.statusOverrides) {
        for(size_t e = 0; e < hazPtr->effects.size(); e++) {
            leaPtr->applyStatus(hazPtr->effects[e], false, true, hitboxMob);
        }
    }
    if(vuln.statusToApply) {
        leaPtr->applyStatus(vuln.statusToApply, false, true, hitboxMob);
    }
    
    if(hazPtr->associatedLiquid) {
        bool alreadyGenerating = false;
        for(size_t g = 0; g < leaPtr->particleGenerators.size(); g++) {
            if(
                leaPtr->particleGenerators[g].id ==
                MOB_PARTICLE_GENERATOR_ID_WAVE_RING
            ) {
                alreadyGenerating = true;
                break;
            }
        }
        
        if(!alreadyGenerating) {
            ParticleGenerator pg =
                standardParticleGenSetup(
                    game.sysContentNames.parWaveRing, leaPtr
                );
            pg.followZOffset = 1.0f;
            adjustKeyframeInterpolatorValues<float>(
                pg.baseParticle.size,
            [ = ] (const float & f) { return f * leaPtr->radius; }
            );
            pg.id = MOB_PARTICLE_GENERATOR_ID_WAVE_RING;
            leaPtr->particleGenerators.push_back(pg);
        }
    }
}


/**
 * @brief When a leader is sprayed.
 *
 * @param m The mob.
 * @param info1 Pointer to the spray type.
 * @param info2 Pointer to the mob that sprayed, if any.
 */
void LeaderFsm::touchedSpray(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    SprayType* s = (SprayType*) info1;
    Mob* sprayer = (Mob*) info2;

    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    for(size_t e = 0; e < s->effects.size(); e++) {
        leaPtr->applyStatus(s->effects[e], false, false, sprayer);
    }
}


/**
 * @brief When a leader tries to update its animation to one of the "standing"
 * ones, if he's not in another animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::trySetCorrectStandingAnim(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    size_t walkAnimIdx =
        leaPtr->anim.animDb->preNamedConversions[LEADER_ANIM_WALKING];
    size_t idleAnimIdx =
        leaPtr->anim.animDb->preNamedConversions[LEADER_ANIM_IDLING];
    if(
        leaPtr->anim.curAnim !=
        leaPtr->anim.animDb->animations[walkAnimIdx] &&
        leaPtr->anim.curAnim !=
        leaPtr->anim.animDb->animations[idleAnimIdx]
    ) {
        //The leader's doing some other animation, so let that happen.
        return;
    }
    
    LeaderFsm::setCorrectStandingAnim(fsm, info1, info2);
}


/**
 * @brief When the leader should update its destination when chasing
 * another leader.
 *
 * @param m The mob.
 * @param info1 Points to the position struct with the final destination.
 *   If nullptr, the final destination is calculated in this function.
 * @param info2 Unused.
 */
void LeaderFsm::updateInGroupChasing(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;

    Point targetPos;
    float targetDist;
    
    leaPtr->getGroupSpotInfo(&targetPos, &targetDist);
    
    leaPtr->chase(
        targetPos, leaPtr->followingGroup->z,
        CHASE_FLAG_ANY_ANGLE, targetDist
    );
    
    LeaderFsm::setIsWalkingTrue(fsm, nullptr, nullptr);
}


/**
 * @brief When a leader begins whistling.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::whistle(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    
    leaPtr->startWhistling();
}


/**
 * @brief When a leader is whistled over by another leader while riding
 * on a track.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void LeaderFsm::whistledWhileRiding(Fsm* fsm, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) fsm->m;
    Track* traPtr = (Track*) (leaPtr->trackInfo->m);

    engineAssert(leaPtr->trackInfo, fsm->printStateHistory());
    
    if(!traPtr->traType->cancellableWithWhistle) {
        return;
    }
    
    leaPtr->stopTrackRide();
    LeaderFsm::joinGroup(fsm, info1, nullptr);
    fsm->setState(LEADER_STATE_IN_GROUP_CHASING);
}


#pragma endregion
