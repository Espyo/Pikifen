/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader finite state machine logic.
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


/**
 * @brief Creates the finite state machine for the leader's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void LeaderFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", LEADER_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(LeaderFsm::enterIdle);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(LeaderFsm::clearBoredomData);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
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
            efc.changeState("dying");
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
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(LeaderFsm::called);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(LeaderFsm::finishCalledAnim);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
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
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(LeaderFsm::enterActive);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(LeaderFsm::setIsWalkingFalse);
            efc.run(LeaderFsm::setIsTurningFalse);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
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
        efc.newEvent(LEADER_EV_LIE_DOWN); {
            efc.run(LeaderFsm::fallAsleep);
            efc.changeState("sleeping_waiting");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(LeaderFsm::goPluck);
            efc.changeState("going_to_pluck");
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
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(LeaderFsm::notifyPikminRelease);
            efc.run(LeaderFsm::release);
            efc.run(LeaderFsm::whistle);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
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
            efc.changeState("dying");
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
        efc.newEvent(MOB_EV_ON_ENTER); {
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
            efc.changeState("dying");
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
            efc.changeState("dying");
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
        efc.newEvent(MOB_EV_ON_ENTER); {
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
            efc.changeState("dying");
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
        efc.newEvent(MOB_EV_ON_ENTER); {
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
            efc.changeState("dying");
        }
    }
    
    efc.newState("spraying", LEADER_STATE_SPRAYING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
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
            efc.changeState("dying");
        }
    }
    
    efc.newState("pain", LEADER_STATE_PAIN); {
        efc.newEvent(MOB_EV_ON_ENTER); {
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
        efc.newEvent(MOB_EV_ON_ENTER); {
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
        efc.newEvent(MOB_EV_ON_ENTER); {
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
        efc.newEvent(MOB_EV_ON_ENTER); {
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
            efc.changeState("dying");
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
            efc.changeState("dying");
        }
    }
    
    efc.newState("getting_up", LEADER_STATE_GETTING_UP); {
        efc.newEvent(MOB_EV_ON_ENTER); {
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
        efc.newEvent(MOB_EV_ON_ENTER); {
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
    
    efc.newState("dying", LEADER_STATE_DYING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(LeaderFsm::die);
        }
    }
    
    efc.newState("in_group_chasing", LEADER_STATE_IN_GROUP_CHASING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
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
            efc.changeState("dying");
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
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(LeaderFsm::stopInGroup);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
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
            efc.changeState("dying");
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
            efc.changeState("dying");
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
        efc.newEvent(MOB_EV_ON_ENTER); {
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
            efc.changeState("dying");
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
        efc.newEvent(MOB_EV_ON_ENTER); {
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
            efc.changeState("dying");
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
            efc.changeState("dying");
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
    
    efc.newState("sleeping_waiting", LEADER_STATE_SLEEPING_WAITING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("waking_up");
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("inactive_sleeping_waiting");
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
            efc.changeState("dying");
        }
    }
    
    efc.newState("sleeping_moving", LEADER_STATE_SLEEPING_MOVING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.changeState("sleeping_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("sleeping_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(GenMobFsm::carryReachDestination);
        }
        efc.newEvent(MOB_EV_CARRY_DELIVERED); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("waking_up");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("waking_up");
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
            efc.run(LeaderFsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("dying");
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
    
    efc.newState("sleeping_stuck", LEADER_STATE_SLEEPING_STUCK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBecomeStuck);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.changeState("sleeping_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("waking_up");
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(LeaderFsm::becomeInactive);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
            efc.run(LeaderFsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("dying");
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
    
    efc.newState(
        "inactive_sleeping_waiting", LEADER_STATE_INACTIVE_SLEEPING_WAITING
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("inactive_waking_up");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("sleeping_waiting");
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
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "inactive_sleeping_moving", LEADER_STATE_INACTIVE_SLEEPING_MOVING
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.changeState("inactive_sleeping_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("inactive_sleeping_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(GenMobFsm::carryReachDestination);
        }
        efc.newEvent(MOB_EV_CARRY_DELIVERED); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("inactive_waking_up");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("inactive_waking_up");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
            efc.run(LeaderFsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("dying");
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
            efc.run(LeaderFsm::startWakingUp);
            efc.run(LeaderFsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState(
        "inactive_sleeping_stuck", LEADER_STATE_INACTIVE_SLEEPING_STUCK
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBecomeStuck);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.changeState("inactive_sleeping_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("inactive_waking_up");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(LeaderFsm::becomeActive);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(LeaderFsm::beAttacked);
            efc.run(LeaderFsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(LeaderFsm::startWakingUp);
            efc.changeState("dying");
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
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(LeaderFsm::startWakingUp);
            efc.run(LeaderFsm::fallDownPit);
            efc.changeState("idling");
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
        efc.newEvent(MOB_EV_ON_LEAVE); {
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
            efc.changeState("dying");
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
        efc.newEvent(MOB_EV_ON_LEAVE); {
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
        efc.newEvent(MOB_EV_ON_LEAVE); {
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
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(LeaderFsm::startDrinking);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
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
            efc.changeState("dying");
        }
    }
    
    efc.newState("riding_track", LEADER_STATE_RIDING_TRACK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(LeaderFsm::startRidingTrack);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(LeaderFsm::tickTrackRide);
        }
    }
    
    efc.newState(
        "inactive_riding_track", LEADER_STATE_INACTIVE_RIDING_TRACK
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(LeaderFsm::startRidingTrack);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(LeaderFsm::tickTrackRide);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(LeaderFsm::whistledWhileRiding);
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


/**
 * @brief When a leader loses health.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void LeaderFsm::beAttacked(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* leaPtr = (Leader*) m;
    
    if(m->invulnPeriod.timeLeft > 0.0f) return;
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    float damage = 0;
    float healthBefore = m->health;
    if(!info->mob2->calculateDamage(m, info->h2, info->h1, &damage)) {
        return;
    }
    
    m->applyAttackDamage(info->mob2, info->h2, info->h1, damage);
    
    m->stopChasing();
    
    float knockback = 0;
    float knockbackAngle = 0;
    info->mob2->calculateKnockback(
        m, info->h2, info->h1, &knockback, &knockbackAngle
    );
    m->applyKnockback(knockback, knockbackAngle);
    
    m->leaveGroup();
    
    m->doAttackEffects(info->mob2, info->h2, info->h1, damage, knockback);
    
    if(knockback > 0) {
        m->invulnPeriod.start(LEADER::INVULN_PERIOD_KB);
        if(leaPtr->player) m->fsm.setState(LEADER_STATE_KNOCKED_BACK);
        else m->fsm.setState(LEADER_STATE_INACTIVE_KNOCKED_BACK);
    } else {
        m->invulnPeriod.start(LEADER::INVULN_PERIOD_NORMAL);
        if(leaPtr->player) m->fsm.setState(LEADER_STATE_PAIN);
        else m->fsm.setState(LEADER_STATE_INACTIVE_PAIN);
    }
    
    game.states.gameplay->lastHurtLeaderPos = m->pos;
    if(healthBefore > 0.0f && m->health < healthBefore) {
        game.statistics.leaderDamageSuffered += healthBefore - m->health;
    }
}


/**
 * @brief When a leader is meant to become the active one.
 *
 * @param m The mob.
 * @param info1 Pointer to the player in charge.
 * @param info2 Unused.
 */
void LeaderFsm::becomeActive(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
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
        game.states.gameplay->curInterlude == INTERLUDE_NONE
    ) {
        //Play the name call as a global sound, so that even leaders far away
        //can have their name call play clearly.
        size_t nameCallSoundIdx =
            leaPtr->leaType->soundDataIdxs[LEADER_SOUND_NAME_CALL];
        if(nameCallSoundIdx != INVALID) {
            MobType::Sound* nameCallSound =
                &m->type->sounds[nameCallSoundIdx];
            game.audio.createGlobalSoundSource(
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
void LeaderFsm::becomeInactive(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
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
void LeaderFsm::beDismissed(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    m->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When a leader is grabbed by another leader.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::beGrabbedByFriend(Mob* m, void* info1, void* info2) {
    m->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When a leader grabbed by another is released.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::beReleased(Mob* m, void* info1, void* info2) {

}


/**
 * @brief When a leader grabbed by another is thrown.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::beThrown(Mob* m, void* info1, void* info2) {
    ((Leader*) m)->startThrowTrail();
}


/**
 * @brief When a leader is thrown by a bouncer mob.
 *
 * @param m The mob.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void LeaderFsm::beThrownByBouncer(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    leaPtr->startThrowTrail();
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
void LeaderFsm::called(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* leaPtr = (Leader*) m;
    Mob* caller = (Mob*) info1;
    
    LeaderFsm::standStill(m, info1, info2);
    
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
void LeaderFsm::calledWhileKnockedDown(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* leaPtr = (Leader*) m;
    Mob* caller = (Mob*) info1;
    
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
void LeaderFsm::checkBoredomAnimEnd(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    if(!leaPtr->inBoredAnimation) return;
    m->setAnimation(LEADER_ANIM_IDLING);
    leaPtr->inBoredAnimation = false;
    m->setTimer(
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
void LeaderFsm::checkPunchDamage(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    float damage = 0;
    if(
        info->mob2->health > 0.0f &&
        m->canHurt(info->mob2) &&
        m->calculateDamage(info->mob2, info->h1, info->h2, &damage)
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
void LeaderFsm::clearBoredomData(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    LeaderFsm::clearTimer(m, info1, info2);
    leaPtr->inBoredAnimation = false;
}


/**
 * @brief When a Pikmin has to clear any timer set.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::clearTimer(Mob* m, void* info1, void* info2) {
    m->setTimer(0);
}


/**
 * @brief When a leader must decide what to do next after plucking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::decidePluckAction(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    
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
        LeaderFsm::signalStopAutoPluck(m, info1, info2);
    }
    
    leaPtr->queuedPluckCancel = false;
    
    if(newPikmin && d <= game.config.leaders.nextPluckRange) {
        leaPtr->fsm.runEvent(LEADER_EV_GO_PLUCK, (void*) newPikmin);
    } else {
        leaPtr->fsm.runEvent(LEADER_EV_CANCEL);
    }
}


/**
 * @brief When a leader dies.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::die(Mob* m, void* info1, void* info2) {
    if(game.states.gameplay->unloading) {
        return;
    }
    
    Leader* leaPtr = (Leader*) m;
    
    m->startDying();
    m->finishDying();
    
    game.states.gameplay->updateAvailableLeaders();
    if(leaPtr->player) {
        changeToNextLeader(leaPtr->player, true, true, true);
    }
    
    LeaderFsm::release(m, info1, info2);
    LeaderFsm::dismiss(m, info1, info2);
    m->becomeUncarriable();
    m->setAnimation(LEADER_ANIM_KO);
    
    game.states.gameplay->lastHurtLeaderPos = m->pos;
}


/**
 * @brief When a leader dismisses the group.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::dismiss(Mob* m, void* info1, void* info2) {
    ((Leader*) m)->dismiss();
}


/**
 * @brief When a leader throws the grabbed mob.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::doThrow(Mob* m, void* info1, void* info2) {
    engineAssert(!m->holding.empty(), m->printStateHistory());
    
    Leader* leaPtr = (Leader*) m;
    Mob* holdingPtr = leaPtr->holding[0];
    
    engineAssert(holdingPtr != nullptr, m->printStateHistory());
    
    holdingPtr->fsm.runEvent(MOB_EV_THROWN);
    holdingPtr->startHeightEffect();
    
    holdingPtr->stopChasing();
    holdingPtr->pos = leaPtr->pos;
    holdingPtr->z = leaPtr->z;
    
    holdingPtr->zCap = leaPtr->throweeMaxZ;
    
    holdingPtr->face(leaPtr->throweeAngle, nullptr, true);
    holdingPtr->speed = leaPtr->throweeSpeed;
    holdingPtr->speedZ = leaPtr->throweeSpeedZ;
    
    enableFlag(holdingPtr->flags, MOB_FLAG_WAS_THROWN);
    holdingPtr->leaveGroup();
    leaPtr->release(holdingPtr);
    
    leaPtr->setAnimation(LEADER_ANIM_THROWING);
    
    if(holdingPtr->type->category->id == MOB_CATEGORY_PIKMIN) {
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
void LeaderFsm::enterActive(Mob* m, void* info1, void* info2) {
    ((Leader*) m)->isInWalkingAnim = false;
    m->setAnimation(
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
void LeaderFsm::enterIdle(Mob* m, void* info1, void* info2) {
    m->unfocusFromMob();
    m->setAnimation(
        LEADER_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    
    m->setTimer(
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
void LeaderFsm::fallAsleep(Mob* m, void* info1, void* info2) {
    LeaderFsm::dismiss(m, nullptr, nullptr);
    m->stopChasing();
    
    m->becomeCarriable(CARRY_DESTINATION_SHIP_NO_ONION);
    
    m->setAnimation(LEADER_ANIM_LYING);
}


/**
 * @brief When a leader falls down a bottomless pit.
 * This damages and respawns them.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::fallDownPit(Mob* m, void* info1, void* info2) {
    m->leaveGroup();
    m->setHealth(true, true, -0.2);
    m->invulnPeriod.start(LEADER::INVULN_PERIOD_NORMAL);
    m->respawn();
}


/**
 * @brief When a leader finished the animation for when it's called.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::finishCalledAnim(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    Mob* caller = leaPtr->focusedMob;
    
    if(leaPtr) {
        LeaderFsm::joinGroup(m, (void*) caller, info2);
        leaPtr->fsm.setState(LEADER_STATE_IN_GROUP_CHASING, info1, info2);
    } else {
        leaPtr->fsm.setState(LEADER_STATE_IDLING, info1, info2);
    }
}


/**
 * @brief When a leader finishes drinking the drop it was drinking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::finishDrinking(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    engineAssert(m->focusedMob != nullptr, m->printStateHistory());
    Drop* droPtr = (Drop*) m->focusedMob;
    
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
        m->applyStatusEffect(droPtr->droType->statusToGive, false, false);
        break;
    } default: {
        break;
    }
    }
    
    m->unfocusFromMob();
}


/**
 * @brief When a leader finishes getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::finishGettingUp(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    Mob* prevFocusedMob = m->focusedMob;
    
    if(leaPtr->player) {
        m->fsm.setState(LEADER_STATE_ACTIVE);
    } else {
        m->fsm.setState(LEADER_STATE_IDLING);
    }
    
    if(prevFocusedMob) {
        if(
            prevFocusedMob->type->category->id == MOB_CATEGORY_LEADERS &&
            !m->canHunt(prevFocusedMob)
        ) {
            m->fsm.runEvent(MOB_EV_WHISTLED, (void*) prevFocusedMob);
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
void LeaderFsm::finishPluck(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    leaPtr->stopChasing();
    leaPtr->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When a leader needs gets knocked back.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::getKnockedBack(Mob* m, void* info1, void* info2) {
    m->unfocusFromMob();
    m->setAnimation(LEADER_ANIM_KNOCKED_BACK);
}


/**
 * @brief When a leader gets knocked back and lands on the floor.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::getKnockedDown(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    
    leaPtr->stopTurning();
    
    //Let's use the "temp" variable to specify whether or not
    //it already received the getting up timer bonus.
    leaPtr->tempI = 0;
    
    m->setTimer(leaPtr->leaType->knockedDownDuration);
    
    m->setAnimation(LEADER_ANIM_LYING);
}


/**
 * @brief When a leader must get up faster from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::getUpFaster(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    
    //Let's use the "temp" variable to specify whether or not
    //it already received the getting up timer bonus.
    if(leaPtr->tempI == 1) return;
    
    leaPtr->scriptTimer.timeLeft =
        std::max(
            0.01f,
            leaPtr->scriptTimer.timeLeft -
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
void LeaderFsm::goPluck(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* leaPtr = (Leader*) m;
    Pikmin* pikPtr = (Pikmin*) info1;
    
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
    
    LeaderFsm::setIsWalkingTrue(m, nullptr, nullptr);
}


/**
 * @brief When a leader grabs onto a mob for throwing.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob to grab.
 * @param info2 Unused.
 */
void LeaderFsm::grabMob(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* leaPtr = (Leader*) m;
    Mob* grabbedMob = (Mob*) info1;
    leaPtr->hold(
        grabbedMob, INVALID,
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
void LeaderFsm::idleOrRejoin(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    
    if(leaPtr->followingGroup) {
        leaPtr->fsm.setState(LEADER_STATE_IN_GROUP_CHASING);
    } else {
        leaPtr->fsm.setState(LEADER_STATE_IDLING);
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
void LeaderFsm::joinGroup(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
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
void LeaderFsm::land(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    m->stopChasing();
    m->speed.x = m->speed.y = 0;
    
    m->removeParticleGenerator(MOB_PARTICLE_GENERATOR_ID_THROW);
    
    if(leaPtr->player) {
        m->fsm.setState(LEADER_STATE_ACTIVE);
    } else {
        m->fsm.setState(LEADER_STATE_IDLING);
    }
}


/**
 * @brief When a leader leaves a hazardous sector.
 *
 * @param m The mob.
 * @param info1 Points to the hazard.
 * @param info2 Unused.
 */
void LeaderFsm::leftHazard(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Hazard* h = (Hazard*) info1;
    if(h->associatedLiquid) {
        m->removeParticleGenerator(MOB_PARTICLE_GENERATOR_ID_WAVE_RING);
    }
}


/**
 * @brief When a leader should lose his momentum and stand still.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::loseMomentum(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    m->speed.x = m->speed.y = 0;
}


/**
 * @brief When a leader begins to move via player control.
 *
 * @param m The mob.
 * @param info1 Pointer to the movement info structure.
 * @param info2 Unused.
 */
void LeaderFsm::move(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* leaPtr = (Leader*) m;
    MovementInfo* mov = (MovementInfo*) info1;
    Point finalCoords;
    float dummyAngle;
    float dummyMagnitude;
    mov->getInfo(
        &finalCoords, &dummyAngle, &dummyMagnitude
    );
    finalCoords *= leaPtr->type->moveSpeed;
    finalCoords += leaPtr->pos;
    leaPtr->chase(finalCoords, leaPtr->z, CHASE_FLAG_ANY_ANGLE);
}


/**
 * @brief When a leader notifies the mob it's holding that it will be released.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::notifyPikminRelease(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    if(leaPtr->holding.empty()) return;
    leaPtr->holding[0]->fsm.runEvent(MOB_EV_RELEASED);
}


/**
 * @brief When a leader punches.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::punch(Mob* m, void* info1, void* info2) {
    m->stopTurning();
    m->setAnimation(LEADER_ANIM_PUNCHING);
}


/**
 * @brief Queues the stopping of the plucking session, for after this
 * pluck's end.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::queueStopAutoPluck(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    leaPtr->queuedPluckCancel = true;
}


/**
 * @brief When a leader gently releases the held mob.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::release(Mob* m, void* info1, void* info2) {
    if(m->holding.empty()) return;
    //Reset the Pikmin's position to match the leader's,
    //so that the leader doesn't release the Pikmin inside a wall behind them.
    m->holding[0]->pos = m->pos;
    m->holding[0]->z = m->z;
    m->holding[0]->face(m->angle + TAU / 2.0f, nullptr, true);
    m->release(m->holding[0]);
}


/**
 * @brief When a leader searches for a seed next to them.
 * If found, issues events to go towards the seed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::searchSeed(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    
    Distance d;
    Pikmin* newPikmin = nullptr;
    if(!leaPtr->queuedPluckCancel) {
        newPikmin =
            getClosestSprout(leaPtr->pos, &d, false);
    }
    
    if(newPikmin && d <= game.config.leaders.nextPluckRange) {
        leaPtr->fsm.runEvent(LEADER_EV_GO_PLUCK, (void*) newPikmin);
    }
}


/**
 * @brief When a leader needs to update its animation in the active state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setCorrectActiveAnim(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    bool mustUseWalkingAnim =
        leaPtr->isActiveWalking || leaPtr->isActiveTurning;
        
    if(mustUseWalkingAnim && !leaPtr->isInWalkingAnim) {
        leaPtr->isInWalkingAnim = true;
        leaPtr->setAnimation(LEADER_ANIM_WALKING);
    } else if(!mustUseWalkingAnim && leaPtr->isInWalkingAnim) {
        leaPtr->isInWalkingAnim = false;
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
void LeaderFsm::setIsTurningFalse(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    if(leaPtr->isActiveTurning) {
        leaPtr->isActiveTurning = false;
        LeaderFsm::setCorrectActiveAnim(m, info1, info2);
    }
}


/**
 * @brief When a leader starts turning in place.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setIsTurningTrue(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    if(!leaPtr->isActiveTurning) {
        leaPtr->isActiveTurning = true;
        LeaderFsm::setCorrectActiveAnim(m, info1, info2);
    }
}


/**
 * @brief When a leader is no longer walking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setIsWalkingFalse(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    if(leaPtr->isActiveWalking) {
        leaPtr->isActiveWalking = false;
        LeaderFsm::setCorrectActiveAnim(m, info1, info2);
    }
}


/**
 * @brief When a leader starts walking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setIsWalkingTrue(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    if(!leaPtr->isActiveWalking) {
        leaPtr->isActiveWalking = true;
        LeaderFsm::setCorrectActiveAnim(m, info1, info2);
    }
}


/**
 * @brief When a leader needs to change to the knocked back animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::setPainAnim(Mob* m, void* info1, void* info2) {
    m->setAnimation(LEADER_ANIM_PAIN);
}


/**
 * @brief When the leader must signal to their follower leaders to
 * stop plucking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::signalStopAutoPluck(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
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
void LeaderFsm::spray(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    size_t sprayIdx = *((size_t*) info1);
    SprayType& sprayTypeRef = *game.config.misc.sprayOrder[sprayIdx];
    
    if(leaPtr->player->team->sprayStats[sprayIdx].nrSprays == 0) {
        m->fsm.setState(LEADER_STATE_ACTIVE);
        return;
    }
    
    float cursorAngle =
        getAngle(m->pos, leaPtr->player->leaderCursorWorld);
    float shootAngle =
        cursorAngle + ((sprayTypeRef.angle) ? TAU / 2 : 0);
        
    unordered_set<Mob*> affectedMobs;
    
    if(sprayTypeRef.affectsUser) {
        affectedMobs.insert(m);
    }
    
    if(sprayTypeRef.group) {
        for(size_t gm = 0; gm < m->group->members.size(); gm++) {
            Mob* gmPtr = m->group->members[gm];
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
            m->fsm.setState(LEADER_STATE_ACTIVE);
            return;
        };
        
    } else {
        for(
            size_t am = 0; am < game.states.gameplay->mobs.all.size(); am++
        ) {
            Mob* amPtr = game.states.gameplay->mobs.all[am];
            if(amPtr == m) continue;
            
            if(
                Distance(m->pos, amPtr->pos) >
                sprayTypeRef.distanceRange + amPtr->radius
            ) {
                continue;
            }
            
            float angleDiff =
                getAngleSmallestDiff(
                    shootAngle,
                    getAngle(m->pos, amPtr->pos)
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
            game.sysContentNames.parSpray, m
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
    m->particleGenerators.push_back(pg);
    
    game.states.gameplay->changeSprayCount(leaPtr->player->team, sprayIdx, -1);
    
    m->stopChasing();
    m->setAnimation(LEADER_ANIM_SPRAYING);
    
    game.statistics.spraysUsed++;
}


/**
 * @brief When a leader stops moving.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::standStill(Mob* m, void* info1, void* info2) {
    m->stopCircling();
    m->stopFollowingPath();
    m->stopChasing();
    m->speed.x = m->speed.y = 0;
}


/**
 * @brief When a leader should start a random boredom animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::startBoredomAnim(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    
    size_t lookingAroundAnimIdx =
        m->type->animDb->findAnimation("looking_around");
    size_t sittingAnimIdx =
        m->type->animDb->findAnimation("sitting");
    size_t stretchingAnimIdx =
        m->type->animDb->findAnimation("stretching");
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
    m->setAnimation(animIdx, START_ANIM_OPTION_NORMAL, false);
    leaPtr->inBoredAnimation = true;
}


/**
 * @brief When a leader must start chasing another.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::startChasingLeader(Mob* m, void* info1, void* info2) {
    m->focusOnMob(m->followingGroup);
    LeaderFsm::updateInGroupChasing(m, nullptr, nullptr);
}


/**
 * @brief When a leader starts drinking the drop it touched.
 *
 * @param m The mob.
 * @param info1 Pointer to the drop mob.
 * @param info2 Unused.
 */
void LeaderFsm::startDrinking(Mob* m, void* info1, void* info2) {
    Mob* droPtr = (Mob*) info1;
    m->leaveGroup();
    m->stopChasing();
    m->focusOnMob(droPtr);
    m->face(getAngle(m->pos, droPtr->pos), nullptr);
    m->setAnimation(LEADER_ANIM_DRINKING);
}


/**
 * @brief When a leader starts getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::startGettingUp(Mob* m, void* info1, void* info2) {
    m->setAnimation(LEADER_ANIM_GETTING_UP);
}


/**
 * @brief When a leader starts a Go Here walk.
 *
 * @param m The mob.
 * @param info1 Destination point.
 * @param info2 Unused.
 */
void LeaderFsm::startGoHere(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
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
        leaPtr->fsm.setState(
            leaPtr->player ?
            LEADER_STATE_MID_GO_HERE :
            LEADER_STATE_INACTIVE_MID_GO_HERE
        );
        leaPtr->midGoHere = true;
        LeaderFsm::setIsWalkingTrue(m, nullptr, nullptr);
    }
}


/**
 * @brief When a leader grabs on to a sprout and begins plucking it out.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::startPluck(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    engineAssert(leaPtr->pluckTarget != nullptr, m->printStateHistory());
    
    leaPtr->pluckTarget->fsm.runEvent(MOB_EV_PLUCKED, (void*) leaPtr);
    leaPtr->pluckTarget->pluckReserved = false;
    leaPtr->pluckTarget = nullptr;
    leaPtr->setAnimation(LEADER_ANIM_PLUCKING);
}


/**
 * @brief When a leader starts riding on a track.
 *
 * @param m The mob.
 * @param info1 Points to the track mob.
 * @param info2 Unused.
 */
void LeaderFsm::startRidingTrack(Mob* m, void* info1, void* info2) {
    Track* traPtr = (Track*) info1;
    
    LeaderFsm::dismiss(m, nullptr, nullptr);
    m->leaveGroup();
    m->stopChasing();
    m->focusOnMob(traPtr);
    m->startHeightEffect();
    
    vector<size_t> checkpoints;
    for(size_t c = 0; c < traPtr->type->animDb->bodyParts.size(); c++) {
        checkpoints.push_back(c);
    }
    m->trackInfo =
        new TrackRideInfo(
        traPtr, checkpoints, traPtr->traType->rideSpeed
    );
    
    switch(traPtr->traType->ridingPose) {
    case TRACK_RIDING_POSE_STOPPED: {
        m->setAnimation(LEADER_ANIM_WALKING);
        break;
    } case TRACK_RIDING_POSE_CLIMBING: {
        m->setAnimation(LEADER_ANIM_WALKING); //TODO
        break;
    } case TRACK_RIDING_POSE_SLIDING: {
        m->setAnimation(LEADER_ANIM_WALKING); //TODO
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
void LeaderFsm::startWakingUp(Mob* m, void* info1, void* info2) {
    m->becomeUncarriable();
    delete m->deliveryInfo;
    m->deliveryInfo = nullptr;
    m->setAnimation(LEADER_ANIM_GETTING_UP);
}


/**
 * @brief When a leader quits the auto-plucking mindset.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::stopAutoPluck(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
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
void LeaderFsm::stopBeingThrown(Mob* m, void* info1, void* info2) {
    //Remove the throw particle generator.
    m->removeParticleGenerator(MOB_PARTICLE_GENERATOR_ID_THROW);
}


/**
 * @brief When a leader stops a Go Here walk.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::stopGoHere(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
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
void LeaderFsm::stopInGroup(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    LeaderFsm::setIsWalkingFalse(m, nullptr, nullptr);
    m->setTimer(
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
void LeaderFsm::stopWhistle(Mob* m, void* info1, void* info2) {
    ((Leader*) m)->stopWhistling();
}


/**
 * @brief Every tick in the active state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::tickActiveState(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    m->face(getAngle(m->pos, leaPtr->player->leaderCursorWorld), nullptr);
    
    bool shouldBeTurning =
        getAngleSmallestDiff(m->angle, m->intendedTurnAngle) > TAU / 300.0f;
    if(shouldBeTurning) {
        LeaderFsm::setIsTurningTrue(m, info1, info2);
    } else {
        LeaderFsm::setIsTurningFalse(m, info1, info2);
    }
}


/**
 * @brief When a leader has to teleport to its spot in a track it is riding.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::tickTrackRide(Mob* m, void* info1, void* info2) {
    engineAssert(m->trackInfo != nullptr, m->printStateHistory());
    
    Leader* leaPtr = (Leader*) m;
    if(m->tickTrackRide()) {
        //Finished!
        if(leaPtr->player) {
            m->fsm.setState(LEADER_STATE_ACTIVE, nullptr, nullptr);
        } else {
            m->fsm.setState(LEADER_STATE_IDLING, nullptr, nullptr);
        }
    }
}


/**
 * @brief When a leader touches a hazard.
 *
 * @param m The mob.
 * @param info1 Pointer to the hazard.
 * @param info2 Unused.
 */
void LeaderFsm::touchedHazard(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* leaPtr = (Leader*) m;
    Hazard* hazPtr = (Hazard*) info1;
    MobType::Vulnerability vuln = m->getHazardVulnerability(hazPtr);
    
    if(!vuln.statusToApply || !vuln.statusOverrides) {
        for(size_t e = 0; e < hazPtr->effects.size(); e++) {
            leaPtr->applyStatusEffect(hazPtr->effects[e], false, true);
        }
    }
    if(vuln.statusToApply) {
        leaPtr->applyStatusEffect(vuln.statusToApply, false, true);
    }
    
    if(hazPtr->associatedLiquid) {
        bool alreadyGenerating = false;
        for(size_t g = 0; g < m->particleGenerators.size(); g++) {
            if(
                m->particleGenerators[g].id ==
                MOB_PARTICLE_GENERATOR_ID_WAVE_RING
            ) {
                alreadyGenerating = true;
                break;
            }
        }
        
        if(!alreadyGenerating) {
            ParticleGenerator pg =
                standardParticleGenSetup(
                    game.sysContentNames.parWaveRing, m
                );
            pg.followZOffset = 1.0f;
            adjustKeyframeInterpolatorValues<float>(
                pg.baseParticle.size,
            [ = ] (const float & f) { return f * m->radius; }
            );
            pg.id = MOB_PARTICLE_GENERATOR_ID_WAVE_RING;
            m->particleGenerators.push_back(pg);
        }
    }
}


/**
 * @brief When a leader is sprayed.
 *
 * @param m The mob.
 * @param info1 Pointer to the spray type.
 * @param info2 Unused.
 */
void LeaderFsm::touchedSpray(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* l = (Leader*) m;
    SprayType* s = (SprayType*) info1;
    
    for(size_t e = 0; e < s->effects.size(); e++) {
        l->applyStatusEffect(s->effects[e], false, false);
    }
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
void LeaderFsm::updateInGroupChasing(Mob* m, void* info1, void* info2) {
    Leader* leaPtr = (Leader*) m;
    Point targetPos;
    float targetDist;
    
    leaPtr->getGroupSpotInfo(&targetPos, &targetDist);
    
    m->chase(
        targetPos, leaPtr->followingGroup->z,
        CHASE_FLAG_ANY_ANGLE, targetDist
    );
    
    LeaderFsm::setIsWalkingTrue(m, nullptr, nullptr);
}


/**
 * @brief When a leader begins whistling.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void LeaderFsm::whistle(Mob* m, void* info1, void* info2) {
    ((Leader*) m)->startWhistling();
}


/**
 * @brief When a leader is whistled over by another leader while riding
 * on a track.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void LeaderFsm::whistledWhileRiding(Mob* m, void* info1, void* info2) {
    engineAssert(m->trackInfo, m->printStateHistory());
    
    Track* traPtr = (Track*) (m->trackInfo->m);
    
    if(!traPtr->traType->cancellableWithWhistle) {
        return;
    }
    
    m->stopTrackRide();
    LeaderFsm::joinGroup(m, info1, nullptr);
    m->fsm.setState(LEADER_STATE_IN_GROUP_CHASING);
}
