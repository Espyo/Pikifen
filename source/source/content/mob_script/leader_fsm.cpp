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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../game_state/gameplay/gameplay.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob_type/leader_type.h"
#include "../mob/drop.h"
#include "../mob/leader.h"
#include "../mob/track.h"
#include "gen_mob_fsm.h"


using std::unordered_set;


/**
 * @brief Creates the finite state machine for the leader's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void leader_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", LEADER_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::enterIdle);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::clearBoredomData);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(leader_fsm::searchSeed);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.changeState("called");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::becomeActive);
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(leader_fsm::standStill);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(leader_fsm::startBoredomAnim);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::checkBoredomAnimEnd);
        }
        efc.newEvent(LEADER_EV_MUST_SEARCH_SEED); {
            efc.run(leader_fsm::searchSeed);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::goPluck);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("called", LEADER_STATE_CALLED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::called);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::finishCalledAnim);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
    }
    
    efc.newState("active", LEADER_STATE_ACTIVE); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::enterActive);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::setIsWalkingFalse);
            efc.run(leader_fsm::setIsTurningFalse);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(leader_fsm::tickActiveState);
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::standStill);
            efc.run(leader_fsm::becomeInactive);
            efc.changeState("idling");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
            efc.run(leader_fsm::setIsWalkingTrue);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::standStill);
            efc.run(leader_fsm::setIsWalkingFalse);
        }
        efc.newEvent(LEADER_EV_HOLDING); {
            efc.run(leader_fsm::grabMob);
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
            efc.run(leader_fsm::fallAsleep);
            efc.changeState("sleeping_waiting");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::goPluck);
            efc.changeState("going_to_pluck");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("whistling", LEADER_STATE_WHISTLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::notifyPikminRelease);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::whistle);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::stopWhistle);
        }
        efc.newEvent(LEADER_EV_STOP_WHISTLE); {
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::standStill);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("punching", LEADER_STATE_PUNCHING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::punch);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::standStill);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(leader_fsm::checkPunchDamage);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("holding", LEADER_STATE_HOLDING); {
        efc.newEvent(LEADER_EV_THROW); {
            efc.changeState("throwing");
        }
        efc.newEvent(MOB_EV_RELEASE_ORDER); {
            efc.run(leader_fsm::notifyPikminRelease);
            efc.run(leader_fsm::release);
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
            efc.run(leader_fsm::setIsWalkingTrue);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::standStill);
            efc.run(leader_fsm::setIsWalkingFalse);
        }
        efc.newEvent(LEADER_EV_START_WHISTLE); {
            efc.changeState("whistling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::notifyPikminRelease);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.run(leader_fsm::notifyPikminRelease);
            efc.run(leader_fsm::release);
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::notifyPikminRelease);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::notifyPikminRelease);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("throwing", LEADER_STATE_THROWING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::doThrow);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::standStill);
        }
        efc.newEvent(LEADER_EV_HOLDING); {
            efc.run(leader_fsm::grabMob);
            efc.changeState("holding");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("dismissing", LEADER_STATE_DISMISSING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::dismiss);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.newEvent(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::standStill);
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_DROP); {
            efc.changeState("drinking");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("spraying", LEADER_STATE_SPRAYING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::spray);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("pain", LEADER_STATE_PAIN); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::setPainAnim);
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::becomeInactive);
            efc.changeState("inactive_pain");
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
    }
    
    efc.newState("inactive_pain", LEADER_STATE_INACTIVE_PAIN); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::setPainAnim);
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::becomeActive);
            efc.changeState("pain");
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::beDismissed);
            efc.changeState("idling");
        }
    }
    
    efc.newState("knocked_back", LEADER_STATE_KNOCKED_BACK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::getKnockedBack);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(leader_fsm::standStill);
            efc.run(leader_fsm::getKnockedDown);
            efc.changeState("knocked_down");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState(
        "inactive_knocked_back", LEADER_STATE_INACTIVE_KNOCKED_BACK
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::getKnockedBack);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(leader_fsm::standStill);
            efc.run(leader_fsm::getKnockedDown);
            efc.changeState("inactive_knocked_down");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("inactive_thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("knocked_down", LEADER_STATE_KNOCKED_DOWN); {
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::becomeInactive);
            efc.changeState("inactive_knocked_down");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::getUpFaster);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.changeState("getting_up");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "inactive_knocked_down", LEADER_STATE_INACTIVE_KNOCKED_DOWN
    ); {
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::becomeActive);
            efc.changeState("knocked_down");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.changeState("inactive_getting_up");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::getUpFaster);
            efc.run(leader_fsm::calledWhileKnockedDown);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("inactive_thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("getting_up", LEADER_STATE_GETTING_UP); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::startGettingUp);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::finishGettingUp);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState(
        "inactive_getting_up", LEADER_STATE_INACTIVE_GETTING_UP
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::startGettingUp);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::finishGettingUp);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::calledWhileKnockedDown);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("dying", LEADER_STATE_DYING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::die);
        }
    }
    
    efc.newState("in_group_chasing", LEADER_STATE_IN_GROUP_CHASING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::startChasingLeader);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.changeState("in_group_stopped");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(leader_fsm::beDismissed);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.run(leader_fsm::updateInGroupChasing);
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(leader_fsm::beGrabbedByFriend);
            efc.changeState("held_by_leader");
        }
        efc.newEvent(LEADER_EV_MUST_SEARCH_SEED); {
            efc.run(leader_fsm::searchSeed);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::goPluck);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("inactive_riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("inactive_thrown");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::beDismissed);
            efc.run(leader_fsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("in_group_stopped", LEADER_STATE_IN_GROUP_STOPPED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::stopInGroup);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::clearBoredomData);
        }
        efc.newEvent(MOB_EV_SPOT_IS_FAR); {
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_DISMISSED); {
            efc.run(leader_fsm::beDismissed);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(leader_fsm::beGrabbedByFriend);
            efc.changeState("held_by_leader");
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(leader_fsm::startBoredomAnim);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::checkBoredomAnimEnd);
        }
        efc.newEvent(LEADER_EV_MUST_SEARCH_SEED); {
            efc.run(leader_fsm::searchSeed);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::goPluck);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TOUCHED_TRACK); {
            efc.changeState("inactive_riding_track");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::beDismissed);
            efc.run(leader_fsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("going_to_pluck", LEADER_STATE_GOING_TO_PLUCK); {
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(leader_fsm::startPluck);
            efc.changeState("plucking");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.run(leader_fsm::signalStopAutoPluck);
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.run(leader_fsm::beAttacked);
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.changeState("dying");
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::becomeInactive);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("plucking", LEADER_STATE_PLUCKING); {
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::finishPluck);
            efc.changeState("pluck_deciding");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::queueStopAutoPluck);
            efc.run(leader_fsm::signalStopAutoPluck);
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::becomeInactive);
            efc.changeState("inactive_plucking");
        }
    }
    
    efc.newState("pluck_deciding", LEADER_STATE_PLUCK_DECIDING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::decidePluckAction);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::goPluck);
            efc.changeState("going_to_pluck");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.changeState("active");
        }
    }
    
    efc.newState(
        "inactive_going_to_pluck", LEADER_STATE_INACTIVE_GOING_TO_PLUCK
    ); {
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(leader_fsm::startPluck);
            efc.changeState("inactive_plucking");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.changeState("called");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.changeState("dying");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::becomeActive);
            efc.changeState("going_to_pluck");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("inactive_plucking", LEADER_STATE_INACTIVE_PLUCKING); {
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::finishPluck);
            efc.changeState("inactive_pluck_deciding");
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::joinGroup);
            efc.run(leader_fsm::queueStopAutoPluck);
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::queueStopAutoPluck);
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::becomeActive);
            efc.changeState("plucking");
        }
    }
    
    efc.newState(
        "inactive_pluck_deciding",
        LEADER_STATE_INACTIVE_PLUCK_DECIDING
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::decidePluckAction);
        }
        efc.newEvent(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::goPluck);
            efc.changeState("inactive_going_to_pluck");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::stopAutoPluck);
            efc.run(leader_fsm::idleOrRejoin);
        }
    }
    
    efc.newState("mid_go_here", LEADER_STATE_MID_GO_HERE); {
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::becomeInactive);
            efc.changeState("inactive_mid_go_here");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(leader_fsm::stopGoHere);
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::stopGoHere);
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(leader_fsm::stopGoHere);
            efc.changeState("dying");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::stopGoHere);
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::stopGoHere);
            efc.run(leader_fsm::beThrownByBouncer);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::stopGoHere);
            efc.run(leader_fsm::fallDownPit);
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::stopGoHere);
            efc.changeState("active");
        }
    }
    
    efc.newState("inactive_mid_go_here", LEADER_STATE_INACTIVE_MID_GO_HERE); {
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::stopGoHere);
            efc.changeState("called");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::becomeActive);
            efc.changeState("mid_go_here");
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(leader_fsm::stopGoHere);
            efc.changeState("idling");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::stopGoHere);
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(leader_fsm::stopGoHere);
            efc.changeState("dying");
        }
        efc.newEvent(LEADER_EV_GO_HERE); {
            efc.run(leader_fsm::stopGoHere);
            efc.run(leader_fsm::startGoHere);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::stopGoHere);
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("sleeping_waiting", LEADER_STATE_SLEEPING_WAITING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("waking_up");
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::becomeInactive);
            efc.changeState("inactive_sleeping_waiting");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
            efc.run(leader_fsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("dying");
        }
    }
    
    efc.newState("sleeping_moving", LEADER_STATE_SLEEPING_MOVING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.changeState("sleeping_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("sleeping_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carryReachDestination);
        }
        efc.newEvent(MOB_EV_CARRY_DELIVERED); {
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("waking_up");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("waking_up");
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::becomeInactive);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
            efc.run(leader_fsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState("sleeping_stuck", LEADER_STATE_SLEEPING_STUCK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carryBecomeStuck);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.changeState("sleeping_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("waking_up");
        }
        efc.newEvent(LEADER_EV_INACTIVATED); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(leader_fsm::becomeInactive);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
            efc.run(leader_fsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
        }
    }
    
    efc.newState(
        "inactive_sleeping_waiting", LEADER_STATE_INACTIVE_SLEEPING_WAITING
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("inactive_waking_up");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::becomeActive);
            efc.changeState("sleeping_waiting");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
            efc.run(leader_fsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(leader_fsm::startWakingUp);
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("dying");
        }
    }
    
    efc.newState(
        "inactive_sleeping_moving", LEADER_STATE_INACTIVE_SLEEPING_MOVING
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.changeState("inactive_sleeping_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("inactive_sleeping_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carryReachDestination);
        }
        efc.newEvent(MOB_EV_CARRY_DELIVERED); {
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("inactive_waking_up");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("inactive_waking_up");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::becomeActive);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
            efc.run(leader_fsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::startWakingUp);
            efc.run(leader_fsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState(
        "inactive_sleeping_stuck", LEADER_STATE_INACTIVE_SLEEPING_STUCK
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carryBecomeStuck);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.changeState("inactive_sleeping_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("inactive_sleeping_moving");
        }
        efc.newEvent(LEADER_EV_CANCEL); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("inactive_waking_up");
        }
        efc.newEvent(LEADER_EV_ACTIVATED); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(leader_fsm::becomeActive);
            efc.changeState("sleeping_moving");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
            efc.run(leader_fsm::startWakingUp);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(leader_fsm::startWakingUp);
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(leader_fsm::startWakingUp);
            efc.run(leader_fsm::fallDownPit);
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
            efc.run(leader_fsm::beReleased);
        }
        efc.newEvent(MOB_EV_THROWN); {
            efc.run(leader_fsm::beThrown);
            efc.changeState("thrown");
        }
        efc.newEvent(MOB_EV_RELEASED); {
            efc.changeState("in_group_chasing");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("thrown", LEADER_STATE_THROWN); {
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(leader_fsm::land);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
            efc.changeState("active");
        }
    }
    
    efc.newState("inactive_thrown", LEADER_STATE_INACTIVE_THROWN); {
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::stopBeingThrown);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(leader_fsm::land);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::beThrownByBouncer);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fallDownPit);
            efc.changeState("idling");
        }
    }
    
    efc.newState("drinking", LEADER_STATE_DRINKING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::startDrinking);
        }
        efc.newEvent(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::finishDrinking);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("active");
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::beAttacked);
        }
        efc.newEvent(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touchedHazard);
        }
        efc.newEvent(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::leftHazard);
        }
        efc.newEvent(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touchedSpray);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.changeState("dying");
        }
    }
    
    efc.newState("riding_track", LEADER_STATE_RIDING_TRACK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::startRidingTrack);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(leader_fsm::tickTrackRide);
        }
    }
    
    efc.newState(
        "inactive_riding_track", LEADER_STATE_INACTIVE_RIDING_TRACK
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::startRidingTrack);
        }
        efc.newEvent(MOB_EV_ON_TICK); {
            efc.run(leader_fsm::tickTrackRide);
        }
        efc.newEvent(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::whistledWhileRiding);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_idx = fixStates(typ->states, "idling", typ);
    
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
void leader_fsm::beAttacked(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* lea_ptr = (Leader*) m;
    
    if(m->invuln_period.time_left > 0.0f) return;
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    float damage = 0;
    float health_before = m->health;
    if(!info->mob2->calculateDamage(m, info->h2, info->h1, &damage)) {
        return;
    }
    
    m->applyAttackDamage(info->mob2, info->h2, info->h1, damage);
    
    m->stopChasing();
    
    float knockback = 0;
    float knockback_angle = 0;
    info->mob2->calculateKnockback(
        m, info->h2, info->h1, &knockback, &knockback_angle
    );
    m->applyKnockback(knockback, knockback_angle);
    
    m->leaveGroup();
    
    m->doAttackEffects(info->mob2, info->h2, info->h1, damage, knockback);
    
    if(knockback > 0) {
        m->invuln_period.start(LEADER::INVULN_PERIOD_KB);
        if(lea_ptr->active) m->fsm.setState(LEADER_STATE_KNOCKED_BACK);
        else m->fsm.setState(LEADER_STATE_INACTIVE_KNOCKED_BACK);
    } else {
        m->invuln_period.start(LEADER::INVULN_PERIOD_NORMAL);
        if(lea_ptr->active) m->fsm.setState(LEADER_STATE_PAIN);
        else m->fsm.setState(LEADER_STATE_INACTIVE_PAIN);
    }
    
    game.states.gameplay->last_hurt_leader_pos = m->pos;
    if(health_before > 0.0f && m->health < health_before) {
        game.statistics.leader_damage_suffered += health_before - m->health;
    }
}


/**
 * @brief When a leader's leader dismisses them.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::beDismissed(Mob* m, void* info1, void* info2) {
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
void leader_fsm::beGrabbedByFriend(Mob* m, void* info1, void* info2) {
    m->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When a leader grabbed by another is released.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::beReleased(Mob* m, void* info1, void* info2) {

}


/**
 * @brief When a leader grabbed by another is thrown.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::beThrown(Mob* m, void* info1, void* info2) {
    ((Leader*) m)->startThrowTrail();
}


/**
 * @brief When a leader is thrown by a bouncer mob.
 *
 * @param m The mob.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void leader_fsm::beThrownByBouncer(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    lea_ptr->startThrowTrail();
    if(!lea_ptr->active) {
        lea_ptr->leaveGroup();
    }
}


/**
 * @brief When a leader is meant to become the active one.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::becomeActive(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    
    if(game.states.gameplay->cur_leader_ptr) {
        game.states.gameplay->cur_leader_ptr->fsm.runEvent(
            LEADER_EV_INACTIVATED
        );
    }
    
    //Normally the player can't swap to leaders that are following another,
    //but some complex cases may allow that (e.g. an inactive leader got
    //whistled by another and then swapped to mid-pluck).
    //Let's swap the group members over.
    if(
        lea_ptr->following_group &&
        lea_ptr->following_group->type->category->id == MOB_CATEGORY_LEADERS
    ) {
        Mob* old_leader = lea_ptr->following_group;
        lea_ptr->leaveGroup();
        old_leader->fsm.runEvent(MOB_EV_WHISTLED, (void*) lea_ptr);
    }
    
    //Update pointers and such.
    size_t new_leader_idx = game.states.gameplay->cur_leader_idx;
    for(size_t l = 0; l < game.states.gameplay->available_leaders.size(); l++) {
        if(game.states.gameplay->available_leaders[l] == lea_ptr) {
            new_leader_idx = l;
            break;
        }
    }
    
    game.states.gameplay->cur_leader_ptr = lea_ptr;
    game.states.gameplay->cur_leader_idx = new_leader_idx;
    lea_ptr->active = true;
    
    //Check if we're in the middle of loading or of an interlude. If so
    //that probably means it's the first leader at the start of the area.
    //We should probably not play the name call then.
    if(
        !game.states.gameplay->loading &&
        game.states.gameplay->cur_interlude == INTERLUDE_NONE
    ) {
        //Play the name call as a global sound, so that even leaders far away
        //can have their name call play clearly.
        size_t name_call_sound_idx =
            lea_ptr->lea_type->sound_data_idxs[LEADER_SOUND_NAME_CALL];
        if(name_call_sound_idx != INVALID) {
            MobType::Sound* name_call_sound =
                &m->type->sounds[name_call_sound_idx];
            game.audio.createGlobalSoundSource(
                name_call_sound->sample,
                false, name_call_sound->config
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
void leader_fsm::becomeInactive(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    lea_ptr->active = false;
    lea_ptr->stopAutoThrowing();
}


/**
 * @brief When a leader is called and must jump in surprise.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void leader_fsm::called(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* lea_ptr = (Leader*) m;
    Mob* caller = (Mob*) info1;
    
    leader_fsm::standStill(m, info1, info2);
    
    lea_ptr->focusOnMob(caller);
    
    lea_ptr->setAnimation(LEADER_ANIM_CALLED);
}


/**
 * @brief When a leader that is knocked down is called over by another leader,
 * by whistling them.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void leader_fsm::calledWhileKnockedDown(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* lea_ptr = (Leader*) m;
    Mob* caller = (Mob*) info1;
    
    lea_ptr->focusOnMob(caller);
}


/**
 * @brief When a leader should check if the animation that ended is a boredom
 * animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::checkBoredomAnimEnd(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    if(!lea_ptr->in_bored_animation) return;
    m->setAnimation(LEADER_ANIM_IDLING);
    lea_ptr->in_bored_animation = false;
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
void leader_fsm::checkPunchDamage(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    float damage = 0;
    if(
        info->mob2->health > 0.0f &&
        m->canHurt(info->mob2) &&
        m->calculateDamage(info->mob2, info->h1, info->h2, &damage)
    ) {
        game.statistics.punch_damage_caused += damage;
    }
}


/**
 * @brief When a leader has to clear any data about being bored.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::clearBoredomData(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    leader_fsm::clearTimer(m, info1, info2);
    lea_ptr->in_bored_animation = false;
}


/**
 * @brief When a Pikmin has to clear any timer set.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::clearTimer(Mob* m, void* info1, void* info2) {
    m->setTimer(0);
}


/**
 * @brief When a leader must decide what to do next after plucking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::decidePluckAction(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    
    Distance d;
    Pikmin* new_pikmin = nullptr;
    
    if(!lea_ptr->queued_pluck_cancel) {
        new_pikmin =
            getClosestSprout(lea_ptr->pos, &d, false);
    }
    
    if(lea_ptr->queued_pluck_cancel) {
        //It should only signal to stop if it wanted to stop.
        //If there are no more sprouts in range, that doesn't mean the leaders
        //following it can't continue with the sprouts in their range.
        leader_fsm::signalStopAutoPluck(m, info1, info2);
    }
    
    lea_ptr->queued_pluck_cancel = false;
    
    if(new_pikmin && d <= game.config.leaders.next_pluck_range) {
        lea_ptr->fsm.runEvent(LEADER_EV_GO_PLUCK, (void*) new_pikmin);
    } else {
        lea_ptr->fsm.runEvent(LEADER_EV_CANCEL);
    }
}


/**
 * @brief When a leader dies.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::die(Mob* m, void* info1, void* info2) {
    if(game.states.gameplay->unloading) {
        return;
    }
    
    m->startDying();
    m->finishDying();
    
    game.states.gameplay->updateAvailableLeaders();
    if(m == game.states.gameplay->cur_leader_ptr) {
        changeToNextLeader(true, true, true);
    }
    
    leader_fsm::release(m, info1, info2);
    leader_fsm::dismiss(m, info1, info2);
    m->becomeUncarriable();
    m->setAnimation(LEADER_ANIM_KO);
    
    game.states.gameplay->last_hurt_leader_pos = m->pos;
}


/**
 * @brief When a leader dismisses the group.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::dismiss(Mob* m, void* info1, void* info2) {
    ((Leader*) m)->dismiss();
}


/**
 * @brief When a leader throws the grabbed mob.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::doThrow(Mob* m, void* info1, void* info2) {
    engineAssert(!m->holding.empty(), m->printStateHistory());
    
    Leader* leader_ptr = (Leader*) m;
    Mob* holding_ptr = leader_ptr->holding[0];
    
    engineAssert(holding_ptr != nullptr, m->printStateHistory());
    
    holding_ptr->fsm.runEvent(MOB_EV_THROWN);
    holding_ptr->startHeightEffect();
    
    holding_ptr->stopChasing();
    holding_ptr->pos = leader_ptr->pos;
    holding_ptr->z = leader_ptr->z;
    
    holding_ptr->z_cap = leader_ptr->throwee_max_z;
    
    holding_ptr->face(leader_ptr->throwee_angle, nullptr, true);
    holding_ptr->speed = leader_ptr->throwee_speed;
    holding_ptr->speed_z = leader_ptr->throwee_speed_z;
    
    enableFlag(holding_ptr->flags, MOB_FLAG_WAS_THROWN);
    holding_ptr->leaveGroup();
    leader_ptr->release(holding_ptr);
    
    leader_ptr->setAnimation(LEADER_ANIM_THROWING);
    
    if(holding_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
        game.statistics.pikmin_thrown++;
    }
}


/**
 * @brief When a leader enters the active state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::enterActive(Mob* m, void* info1, void* info2) {
    ((Leader*) m)->is_in_walking_anim = false;
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
void leader_fsm::enterIdle(Mob* m, void* info1, void* info2) {
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
void leader_fsm::fallAsleep(Mob* m, void* info1, void* info2) {
    leader_fsm::dismiss(m, nullptr, nullptr);
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
void leader_fsm::fallDownPit(Mob* m, void* info1, void* info2) {
    m->leaveGroup();
    m->setHealth(true, true, -0.2);
    m->invuln_period.start(LEADER::INVULN_PERIOD_NORMAL);
    m->respawn();
}


/**
 * @brief When a leader finished the animation for when it's called.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::finishCalledAnim(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    Mob* caller = lea_ptr->focused_mob;
    
    if(lea_ptr) {
        leader_fsm::joinGroup(m, (void*) caller, info2);
        lea_ptr->fsm.setState(LEADER_STATE_IN_GROUP_CHASING, info1, info2);
    } else {
        lea_ptr->fsm.setState(LEADER_STATE_IDLING, info1, info2);
    }
}


/**
 * @brief When a leader finishes drinking the drop it was drinking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::finishDrinking(Mob* m, void* info1, void* info2) {
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    Drop* dro_ptr = (Drop*) m->focused_mob;
    
    switch(dro_ptr->dro_type->effect) {
    case DROP_EFFECT_INCREASE_SPRAYS: {
        game.states.gameplay->changeSprayCount(
            dro_ptr->dro_type->spray_type_to_increase,
            dro_ptr->dro_type->increase_amount
        );
        break;
    } case DROP_EFFECT_GIVE_STATUS: {
        m->applyStatusEffect(dro_ptr->dro_type->status_to_give, false, false);
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
void leader_fsm::finishGettingUp(Mob* m, void* info1, void* info2) {
    Mob* prev_focused_mob = m->focused_mob;
    
    if(m == game.states.gameplay->cur_leader_ptr) {
        m->fsm.setState(LEADER_STATE_ACTIVE);
    } else {
        m->fsm.setState(LEADER_STATE_IDLING);
    }
    
    if(prev_focused_mob) {
        if(
            prev_focused_mob->type->category->id == MOB_CATEGORY_LEADERS &&
            !m->canHunt(prev_focused_mob)
        ) {
            m->fsm.runEvent(MOB_EV_WHISTLED, (void*) prev_focused_mob);
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
void leader_fsm::finishPluck(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    lea_ptr->stopChasing();
    lea_ptr->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When a leader needs gets knocked back.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::getKnockedBack(Mob* m, void* info1, void* info2) {
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
void leader_fsm::getKnockedDown(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    
    lea_ptr->stopTurning();
    
    //Let's use the "temp" variable to specify whether or not
    //it already received the getting up timer bonus.
    lea_ptr->temp_i = 0;
    
    m->setTimer(lea_ptr->lea_type->knocked_down_duration);
    
    m->setAnimation(LEADER_ANIM_LYING);
}


/**
 * @brief When a leader must get up faster from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::getUpFaster(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    
    //Let's use the "temp" variable to specify whether or not
    //it already received the getting up timer bonus.
    if(lea_ptr->temp_i == 1) return;
    
    lea_ptr->script_timer.time_left =
        std::max(
            0.01f,
            lea_ptr->script_timer.time_left -
            lea_ptr->lea_type->knocked_down_whistle_bonus
        );
    lea_ptr->temp_i = 1;
}


/**
 * @brief When a leader heads towards a Pikmin with the intent to pluck it.
 * Also signals other leaders in the group to search for other seeds.
 *
 * @param m The mob.
 * @param info1 Pointer to the Pikmin to be plucked.
 * @param info2 Unused.
 */
void leader_fsm::goPluck(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* lea_ptr = (Leader*) m;
    Pikmin* pik_ptr = (Pikmin*) info1;
    
    lea_ptr->queued_pluck_cancel = false;
    
    lea_ptr->auto_plucking = true;
    lea_ptr->pluck_target = pik_ptr;
    lea_ptr->chase(
        &pik_ptr->pos, &pik_ptr->z,
        Point(), 0.0f,
        CHASE_FLAG_ANY_ANGLE,
        pik_ptr->radius + lea_ptr->radius
    );
    pik_ptr->pluck_reserved = true;
    
    //Now for the leaders in the group.
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); l++) {
        Leader* l2_ptr = game.states.gameplay->mobs.leaders[l];
        if(l2_ptr->following_group == lea_ptr) {
            l2_ptr->fsm.runEvent(LEADER_EV_MUST_SEARCH_SEED);
        }
    }
    
    leader_fsm::setIsWalkingTrue(m, nullptr, nullptr);
}


/**
 * @brief When a leader grabs onto a mob for throwing.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob to grab.
 * @param info2 Unused.
 */
void leader_fsm::grabMob(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* lea_ptr = (Leader*) m;
    Mob* grabbed_mob = (Mob*) info1;
    lea_ptr->hold(
        grabbed_mob, INVALID,
        LEADER::HELD_GROUP_MEMBER_H_DIST, LEADER::HELD_GROUP_MEMBER_ANGLE,
        LEADER::HELD_GROUP_MEMBER_V_DIST,
        false, HOLD_ROTATION_METHOD_FACE_HOLDER
    );
    lea_ptr->group->sort(grabbed_mob->subgroup_type_ptr);
}


/**
 * @brief When a leader must either return to idling, or return to rejoining
 * its leader.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::idleOrRejoin(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    
    if(lea_ptr->following_group) {
        lea_ptr->fsm.setState(LEADER_STATE_IN_GROUP_CHASING);
    } else {
        lea_ptr->fsm.setState(LEADER_STATE_IDLING);
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
void leader_fsm::joinGroup(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    Leader* caller = (Leader*) info1;
    Mob* top_leader = caller;
    
    if(top_leader->following_group) {
        //If this leader is following another one,
        //then the new leader should be in the group of that top leader.
        top_leader = top_leader->following_group;
    }
    
    top_leader->addToGroup(lea_ptr);
    while(!lea_ptr->group->members.empty()) {
        Mob* member = lea_ptr->group->members[0];
        member->leaveGroup();
        top_leader->addToGroup(member);
    }
}


/**
 * @brief When a thrown leader lands.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::land(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    m->speed.x = m->speed.y = 0;
    
    m->removeParticleGenerator(MOB_PARTICLE_GENERATOR_ID_THROW);
    
    if(m == game.states.gameplay->cur_leader_ptr) {
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
void leader_fsm::leftHazard(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Hazard* h = (Hazard*) info1;
    if(h->associated_liquid) {
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
void leader_fsm::loseMomentum(Mob* m, void* info1, void* info2) {
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
void leader_fsm::move(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* lea_ptr = (Leader*) m;
    MovementInfo* mov = (MovementInfo*) info1;
    Point final_coords;
    float dummy_angle;
    float dummy_magnitude;
    mov->getInfo(
        &final_coords, &dummy_angle, &dummy_magnitude
    );
    final_coords *= lea_ptr->type->move_speed;
    final_coords += lea_ptr->pos;
    lea_ptr->chase(final_coords, lea_ptr->z, CHASE_FLAG_ANY_ANGLE);
}


/**
 * @brief When a leader notifies the mob it's holding that it will be released.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::notifyPikminRelease(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    if(lea_ptr->holding.empty()) return;
    lea_ptr->holding[0]->fsm.runEvent(MOB_EV_RELEASED);
}


/**
 * @brief When a leader punches.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::punch(Mob* m, void* info1, void* info2) {
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
void leader_fsm::queueStopAutoPluck(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    lea_ptr->queued_pluck_cancel = true;
}


/**
 * @brief When a leader gently releases the held mob.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::release(Mob* m, void* info1, void* info2) {
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
void leader_fsm::searchSeed(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    
    Distance d;
    Pikmin* new_pikmin = nullptr;
    if(!lea_ptr->queued_pluck_cancel) {
        new_pikmin =
            getClosestSprout(lea_ptr->pos, &d, false);
    }
    
    if(new_pikmin && d <= game.config.leaders.next_pluck_range) {
        lea_ptr->fsm.runEvent(LEADER_EV_GO_PLUCK, (void*) new_pikmin);
    }
}


/**
 * @brief When a leader needs to update its animation in the active state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::setCorrectActiveAnim(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    bool must_use_walking_anim =
        lea_ptr->is_active_walking || lea_ptr->is_active_turning;
        
    if(must_use_walking_anim && !lea_ptr->is_in_walking_anim) {
        lea_ptr->is_in_walking_anim = true;
        lea_ptr->setAnimation(LEADER_ANIM_WALKING);
    } else if(!must_use_walking_anim && lea_ptr->is_in_walking_anim) {
        lea_ptr->is_in_walking_anim = false;
        lea_ptr->setAnimation(LEADER_ANIM_IDLING);
    }
}


/**
 * @brief When a leader is no longer turning in place.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::setIsTurningFalse(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    if(lea_ptr->is_active_turning) {
        lea_ptr->is_active_turning = false;
        leader_fsm::setCorrectActiveAnim(m, info1, info2);
    }
}


/**
 * @brief When a leader starts turning in place.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::setIsTurningTrue(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    if(!lea_ptr->is_active_turning) {
        lea_ptr->is_active_turning = true;
        leader_fsm::setCorrectActiveAnim(m, info1, info2);
    }
}


/**
 * @brief When a leader is no longer walking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::setIsWalkingFalse(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    if(lea_ptr->is_active_walking) {
        lea_ptr->is_active_walking = false;
        leader_fsm::setCorrectActiveAnim(m, info1, info2);
    }
}


/**
 * @brief When a leader starts walking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::setIsWalkingTrue(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    if(!lea_ptr->is_active_walking) {
        lea_ptr->is_active_walking = true;
        leader_fsm::setCorrectActiveAnim(m, info1, info2);
    }
}


/**
 * @brief When a leader needs to change to the knocked back animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::setPainAnim(Mob* m, void* info1, void* info2) {
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
void leader_fsm::signalStopAutoPluck(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); l++) {
        Leader* l2_ptr = game.states.gameplay->mobs.leaders[l];
        if(l2_ptr->following_group == lea_ptr) {
            l2_ptr->fsm.runEvent(LEADER_EV_CANCEL);
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
void leader_fsm::spray(Mob* m, void* info1, void* info2) {
    size_t spray_idx = *((size_t*) info1);
    SprayType &spray_type_ref = *game.config.misc.spray_order[spray_idx];
    
    if(game.states.gameplay->spray_stats[spray_idx].nr_sprays == 0) {
        m->fsm.setState(LEADER_STATE_ACTIVE);
        return;
    }
    
    float cursor_angle =
        getAngle(m->pos, game.states.gameplay->leader_cursor_w);
    float shoot_angle =
        cursor_angle + ((spray_type_ref.angle) ? TAU / 2 : 0);
        
    unordered_set<Mob*> affected_mobs;
    
    if(spray_type_ref.affects_user) {
        affected_mobs.insert(m);
    }
    
    if(spray_type_ref.group) {
        for(size_t gm = 0; gm < m->group->members.size(); gm++) {
            Mob* gm_ptr = m->group->members[gm];
            if(
                gm_ptr->type->category->id != MOB_CATEGORY_PIKMIN &&
                spray_type_ref.group_pikmin_only
            ) {
                continue;
            }
            
            affected_mobs.insert(gm_ptr);
        }
        //If there is nothing to get sprayed, better not waste it.
        if(affected_mobs.empty())  {
            m->fsm.setState(LEADER_STATE_ACTIVE);
            return;
        };
        
    } else {
        for(
            size_t am = 0; am < game.states.gameplay->mobs.all.size(); am++
        ) {
            Mob* am_ptr = game.states.gameplay->mobs.all[am];
            if(am_ptr == m) continue;
            
            if(
                Distance(m->pos, am_ptr->pos) >
                spray_type_ref.distance_range + am_ptr->radius
            ) {
                continue;
            }
            
            float angle_dif =
                getAngleSmallestDiff(
                    shoot_angle,
                    getAngle(m->pos, am_ptr->pos)
                );
            if(angle_dif > spray_type_ref.angle_range / 2) continue;
            
            affected_mobs.insert(am_ptr);
        }
        
    }
    
    for(auto &am : affected_mobs) {
        am->fsm.runEvent(
            MOB_EV_TOUCHED_SPRAY, (void*) game.config.misc.spray_order[spray_idx]
        );
    }
    
    Point particle_speed_vector =
        rotatePoint(
            Point(spray_type_ref.distance_range * 0.8, 0),
            spray_type_ref.angle
        );
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sys_content_names.part_spray, m
        );
    adjustKeyframeInterpolatorValues<Point>(
        pg.base_particle.linear_speed,
    [ = ] (const Point &) { return particle_speed_vector; }
    );
    adjustKeyframeInterpolatorValues<ALLEGRO_COLOR>(
        pg.base_particle.color,
    [ = ] (const ALLEGRO_COLOR & c) {
        ALLEGRO_COLOR new_c = c;
        new_c.r *= spray_type_ref.main_color.r;
        new_c.g *= spray_type_ref.main_color.g;
        new_c.b *= spray_type_ref.main_color.b;
        new_c.a *= spray_type_ref.main_color.a;
        return new_c;
    }
    );
    pg.linear_speed_angle_deviation = spray_type_ref.angle_range / 2.0f;
    pg.linear_speed_deviation.x = spray_type_ref.distance_range * 0.4;
    pg.base_particle.priority = PARTICLE_PRIORITY_HIGH;
    m->particle_generators.push_back(pg);
    
    game.states.gameplay->changeSprayCount(spray_idx, -1);
    
    m->stopChasing();
    m->setAnimation(LEADER_ANIM_SPRAYING);
    
    game.statistics.sprays_used++;
}


/**
 * @brief When a leader stops moving.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::standStill(Mob* m, void* info1, void* info2) {
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
void leader_fsm::startBoredomAnim(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    
    size_t looking_around_anim_idx =
        m->type->anim_db->findAnimation("looking_around");
    size_t sitting_anim_idx =
        m->type->anim_db->findAnimation("sitting");
    size_t stretching_anim_idx =
        m->type->anim_db->findAnimation("stretching");
    vector<size_t> boredom_anims;
    if(looking_around_anim_idx != INVALID) {
        boredom_anims.push_back(looking_around_anim_idx);
    }
    if(sitting_anim_idx != INVALID) {
        boredom_anims.push_back(sitting_anim_idx);
    }
    if(stretching_anim_idx != INVALID) {
        boredom_anims.push_back(stretching_anim_idx);
    }
    
    if(boredom_anims.empty()) return;
    size_t anim_idx = boredom_anims[game.rng.i(0, (int) (boredom_anims.size() - 1))];
    m->setAnimation(anim_idx, START_ANIM_OPTION_NORMAL, false);
    lea_ptr->in_bored_animation = true;
}


/**
 * @brief When a leader must start chasing another.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::startChasingLeader(Mob* m, void* info1, void* info2) {
    m->focusOnMob(m->following_group);
    leader_fsm::updateInGroupChasing(m, nullptr, nullptr);
}


/**
 * @brief When a leader starts drinking the drop it touched.
 *
 * @param m The mob.
 * @param info1 Pointer to the drop mob.
 * @param info2 Unused.
 */
void leader_fsm::startDrinking(Mob* m, void* info1, void* info2) {
    Mob* drop_ptr = (Mob*) info1;
    m->leaveGroup();
    m->stopChasing();
    m->focusOnMob(drop_ptr);
    m->face(getAngle(m->pos, drop_ptr->pos), nullptr);
    m->setAnimation(LEADER_ANIM_DRINKING);
}


/**
 * @brief When a leader starts getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::startGettingUp(Mob* m, void* info1, void* info2) {
    m->setAnimation(LEADER_ANIM_GETTING_UP);
}


/**
 * @brief When a leader starts a Go Here walk.
 *
 * @param m The mob.
 * @param info1 Destination point.
 * @param info2 Unused.
 */
void leader_fsm::startGoHere(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    Point destination = *((Point*) info1);
    
    PathFollowSettings settings;
    settings.target_point = destination;
    
    float speed = lea_ptr->getBaseSpeed();
    for(size_t gm = 0; gm < lea_ptr->group->members.size(); gm++) {
        //It can only go as fast as its slowest member.
        speed = std::min(speed, lea_ptr->group->members[gm]->getBaseSpeed());
    }
    
    bool success =
        lea_ptr->followPath(
            settings, speed, lea_ptr->type->acceleration
        );
        
    if(success) {
        lea_ptr->fsm.setState(
            lea_ptr->active ?
            LEADER_STATE_MID_GO_HERE :
            LEADER_STATE_INACTIVE_MID_GO_HERE
        );
        lea_ptr->mid_go_here = true;
        leader_fsm::setIsWalkingTrue(m, nullptr, nullptr);
    }
}


/**
 * @brief When a leader grabs on to a sprout and begins plucking it out.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::startPluck(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    engineAssert(lea_ptr->pluck_target != nullptr, m->printStateHistory());
    
    lea_ptr->pluck_target->fsm.runEvent(MOB_EV_PLUCKED, (void*) lea_ptr);
    lea_ptr->pluck_target->pluck_reserved = false;
    lea_ptr->pluck_target = nullptr;
    lea_ptr->setAnimation(LEADER_ANIM_PLUCKING);
}


/**
 * @brief When a leader starts riding on a track.
 *
 * @param m The mob.
 * @param info1 Points to the track mob.
 * @param info2 Unused.
 */
void leader_fsm::startRidingTrack(Mob* m, void* info1, void* info2) {
    Track* tra_ptr = (Track*) info1;
    
    leader_fsm::dismiss(m, nullptr, nullptr);
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
void leader_fsm::startWakingUp(Mob* m, void* info1, void* info2) {
    m->becomeUncarriable();
    delete m->delivery_info;
    m->delivery_info = nullptr;
    m->setAnimation(LEADER_ANIM_GETTING_UP);
}


/**
 * @brief When a leader quits the auto-plucking mindset.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::stopAutoPluck(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    if(lea_ptr->pluck_target) {
        lea_ptr->stopChasing();
        lea_ptr->pluck_target->pluck_reserved = false;
    }
    lea_ptr->auto_plucking = false;
    lea_ptr->queued_pluck_cancel = false;
    lea_ptr->pluck_target = nullptr;
    lea_ptr->setAnimation(LEADER_ANIM_IDLING);
}


/**
 * @brief When a leader is no longer in the thrown state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::stopBeingThrown(Mob* m, void* info1, void* info2) {
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
void leader_fsm::stopGoHere(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    lea_ptr->stopFollowingPath();
    lea_ptr->mid_go_here = false;
}


/**
 * @brief When a leader stands still while in another's group.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::stopInGroup(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    leader_fsm::setIsWalkingFalse(m, nullptr, nullptr);
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
void leader_fsm::stopWhistle(Mob* m, void* info1, void* info2) {
    ((Leader*) m)->stopWhistling();
}


/**
 * @brief Every tick in the active state.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::tickActiveState(Mob* m, void* info1, void* info2) {
    m->face(getAngle(m->pos, game.states.gameplay->leader_cursor_w), nullptr);
    
    bool should_be_turning =
        getAngleSmallestDiff(m->angle, m->intended_turn_angle) > TAU / 300.0f;
    if(should_be_turning) {
        leader_fsm::setIsTurningTrue(m, info1, info2);
    } else {
        leader_fsm::setIsTurningFalse(m, info1, info2);
    }
}


/**
 * @brief When a leader has to teleport to its spot in a track it is riding.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::tickTrackRide(Mob* m, void* info1, void* info2) {
    engineAssert(m->track_info != nullptr, m->printStateHistory());
    
    if(m->tickTrackRide()) {
        //Finished!
        if(((Leader*) m)->active) {
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
void leader_fsm::touchedHazard(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Leader* l = (Leader*) m;
    Hazard* h = (Hazard*) info1;
    MobType::Vulnerability vuln = m->getHazardVulnerability(h);
    
    if(!vuln.status_to_apply || !vuln.status_overrides) {
        for(size_t e = 0; e < h->effects.size(); e++) {
            l->applyStatusEffect(h->effects[e], false, true);
        }
    }
    if(vuln.status_to_apply) {
        l->applyStatusEffect(vuln.status_to_apply, false, true);
    }
    
    if(h->associated_liquid) {
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
}


/**
 * @brief When a leader is sprayed.
 *
 * @param m The mob.
 * @param info1 Pointer to the spray type.
 * @param info2 Unused.
 */
void leader_fsm::touchedSpray(Mob* m, void* info1, void* info2) {
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
void leader_fsm::updateInGroupChasing(Mob* m, void* info1, void* info2) {
    Leader* lea_ptr = (Leader*) m;
    Point target_pos;
    float target_dist;
    
    lea_ptr->getGroupSpotInfo(&target_pos, &target_dist);
    
    m->chase(
        target_pos, lea_ptr->following_group->z,
        CHASE_FLAG_ANY_ANGLE, target_dist
    );
    
    leader_fsm::setIsWalkingTrue(m, nullptr, nullptr);
}


/**
 * @brief When a leader begins whistling.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void leader_fsm::whistle(Mob* m, void* info1, void* info2) {
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
void leader_fsm::whistledWhileRiding(Mob* m, void* info1, void* info2) {
    engineAssert(m->track_info, m->printStateHistory());
    
    Track* tra_ptr = (Track*) (m->track_info->m);
    
    if(!tra_ptr->tra_type->cancellable_with_whistle) {
        return;
    }
    
    m->stopTrackRide();
    leader_fsm::joinGroup(m, info1, nullptr);
    m->fsm.setState(LEADER_STATE_IN_GROUP_CHASING);
}
