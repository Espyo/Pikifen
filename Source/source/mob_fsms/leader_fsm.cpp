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

#include "../functions.h"
#include "../game.h"
#include "../game_states/gameplay/gameplay.h"
#include "../mob_types/leader_type.h"
#include "../mobs/drop.h"
#include "../mobs/leader.h"
#include "../mobs/track.h"
#include "../utils/string_utils.h"
#include "gen_mob_fsm.h"


using std::unordered_set;


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the leader's logic.
 * typ:
 *   Mob type to create the finite state machine for.
 */
void leader_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", LEADER_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::enter_idle);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(leader_fsm::search_seed);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::join_group);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::become_active);
            efc.change_state("active");
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(leader_fsm::stop);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(LEADER_EV_MUST_SEARCH_SEED); {
            efc.run(leader_fsm::search_seed);
        }
        efc.new_event(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::go_pluck);
            efc.change_state("inactive_going_to_pluck");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("active", LEADER_STATE_ACTIVE); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::enter_active);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(leader_fsm::tick_active_state);
        }
        efc.new_event(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::stop);
            efc.run(leader_fsm::become_inactive);
            efc.change_state("idling");
        }
        efc.new_event(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
            efc.run(leader_fsm::set_walk_anim);
        }
        efc.new_event(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::stop);
            efc.run(leader_fsm::set_stop_anim);
        }
        efc.new_event(LEADER_EV_HOLDING); {
            efc.run(leader_fsm::grab_mob);
            efc.change_state("holding");
        }
        efc.new_event(LEADER_EV_START_WHISTLE); {
            efc.change_state("whistling");
        }
        efc.new_event(LEADER_EV_PUNCH); {
            efc.change_state("punching");
        }
        efc.new_event(LEADER_EV_DISMISS); {
            efc.change_state("dismissing");
        }
        efc.new_event(LEADER_EV_SPRAY); {
            efc.change_state("spraying");
        }
        efc.new_event(LEADER_EV_LIE_DOWN); {
            efc.run(leader_fsm::fall_asleep);
            efc.change_state("sleeping_waiting");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::go_pluck);
            efc.change_state("going_to_pluck");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EV_TOUCHED_TRACK); {
            efc.change_state("riding_track");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("whistling", LEADER_STATE_WHISTLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::whistle);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::stop_whistle);
        }
        efc.new_event(LEADER_EV_STOP_WHISTLE); {
            efc.change_state("active");
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.change_state("active");
        }
        efc.new_event(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.new_event(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::stop);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("punching", LEADER_STATE_PUNCHING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::punch);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("active");
        }
        efc.new_event(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.new_event(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::stop);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("holding", LEADER_STATE_HOLDING); {
        efc.new_event(LEADER_EV_THROW); {
            efc.run(leader_fsm::do_throw);
            efc.change_state("active");
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.change_state("active");
        }
        efc.new_event(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
            efc.run(leader_fsm::set_walk_anim);
        }
        efc.new_event(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::stop);
            efc.run(leader_fsm::set_stop_anim);
        }
        efc.new_event(LEADER_EV_START_WHISTLE); {
            efc.change_state("whistling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_DROP); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("dismissing", LEADER_STATE_DISMISSING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::dismiss);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("active");
        }
        efc.new_event(LEADER_EV_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.new_event(LEADER_EV_MOVE_END); {
            efc.run(leader_fsm::stop);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
    }
    
    efc.new_state("spraying", LEADER_STATE_SPRAYING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::spray);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("active");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
    }
    
    efc.new_state("pain", LEADER_STATE_PAIN); {
        efc.new_event(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::become_inactive);
            efc.change_state("inactive_pain");
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("active");
        }
    }
    
    efc.new_state("inactive_pain", LEADER_STATE_INACTIVE_PAIN); {
        efc.new_event(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::become_active);
            efc.change_state("pain");
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::be_dismissed);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("knocked_back", LEADER_STATE_KNOCKED_BACK); {
        efc.new_event(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::become_inactive);
            efc.change_state("inactive_knocked_back");
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(leader_fsm::lose_momentum);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("active");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "inactive_knocked_back", LEADER_STATE_INACTIVE_KNOCKED_BACK
    ); {
        efc.new_event(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::become_active);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(leader_fsm::lose_momentum);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("dying", LEADER_STATE_DYING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::die);
        }
    }
    
    efc.new_state("in_group_chasing", LEADER_STATE_IN_GROUP_CHASING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::chase_leader);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("in_group_stopped");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(leader_fsm::be_dismissed);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(leader_fsm::be_grabbed_by_friend);
            efc.change_state("held_by_leader");
        }
        efc.new_event(LEADER_EV_MUST_SEARCH_SEED); {
            efc.run(leader_fsm::search_seed);
        }
        efc.new_event(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::go_pluck);
            efc.change_state("inactive_going_to_pluck");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_TRACK); {
            efc.change_state("inactive_riding_track");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::be_dismissed);
            efc.run(leader_fsm::fall_down_pit);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("in_group_stopped", LEADER_STATE_IN_GROUP_STOPPED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::stop_in_group);
        }
        efc.new_event(MOB_EV_SPOT_IS_FAR); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(leader_fsm::be_dismissed);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(leader_fsm::be_grabbed_by_friend);
            efc.change_state("held_by_leader");
        }
        efc.new_event(LEADER_EV_MUST_SEARCH_SEED); {
            efc.run(leader_fsm::search_seed);
        }
        efc.new_event(LEADER_EV_GO_PLUCK); {
            efc.run(leader_fsm::go_pluck);
            efc.change_state("inactive_going_to_pluck");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_TRACK); {
            efc.change_state("inactive_riding_track");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::be_dismissed);
            efc.run(leader_fsm::fall_down_pit);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("going_to_pluck", LEADER_STATE_GOING_TO_PLUCK); {
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(leader_fsm::start_pluck);
            efc.change_state("plucking");
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.run(leader_fsm::signal_stop_auto_pluck);
            efc.change_state("active");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.run(leader_fsm::be_attacked);
            efc.change_state("active");
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.change_state("dying");
        }
        efc.new_event(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::become_inactive);
            efc.change_state("inactive_going_to_pluck");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("plucking", LEADER_STATE_PLUCKING); {
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::finish_current_pluck);
            efc.run(leader_fsm::search_seed);
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::queue_stop_auto_pluck);
            efc.run(leader_fsm::signal_stop_auto_pluck);
        }
        efc.new_event(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::become_inactive);
            efc.change_state("inactive_plucking");
        }
    }
    
    efc.new_state(
        "inactive_going_to_pluck", LEADER_STATE_INACTIVE_GOING_TO_PLUCK
    ); {
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(leader_fsm::start_pluck);
            efc.change_state("inactive_plucking");
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::join_group);
            efc.run(leader_fsm::stop_auto_pluck);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.change_state("dying");
        }
        efc.new_event(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::become_active);
            efc.change_state("going_to_pluck");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("inactive_plucking", LEADER_STATE_INACTIVE_PLUCKING); {
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(leader_fsm::finish_current_pluck);
            efc.run(leader_fsm::search_seed);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::join_group);
            efc.run(leader_fsm::queue_stop_auto_pluck);
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::queue_stop_auto_pluck);
        }
        efc.new_event(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::become_active);
            efc.change_state("plucking");
        }
    }
    
    efc.new_state("sleeping_waiting", LEADER_STATE_SLEEPING_WAITING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("sleeping_moving");
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("waking_up");
        }
        efc.new_event(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::become_inactive);
            efc.change_state("inactive_sleeping_waiting");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
    }
    
    efc.new_state("sleeping_moving", LEADER_STATE_SLEEPING_MOVING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.change_state("sleeping_waiting");
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_PATH_BLOCKED); {
            efc.change_state("sleeping_stuck");
        }
        efc.new_event(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carry_reach_destination);
        }
        efc.new_event(MOB_EV_CARRY_DELIVERED); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("waking_up");
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("waking_up");
        }
        efc.new_event(LEADER_EV_INACTIVATED); {
            efc.run(leader_fsm::become_inactive);
            efc.change_state("inactive_sleeping_moving");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("sleeping_stuck", LEADER_STATE_SLEEPING_STUCK); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_become_stuck);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.change_state("sleeping_waiting");
        }
        efc.new_event(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("sleeping_moving");
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("waking_up");
        }
        efc.new_event(LEADER_EV_INACTIVATED); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(leader_fsm::become_inactive);
            efc.change_state("inactive_sleeping_moving");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "inactive_sleeping_waiting", LEADER_STATE_INACTIVE_SLEEPING_WAITING
    ); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("inactive_sleeping_moving");
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("inactive_waking_up");
        }
        efc.new_event(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::become_active);
            efc.change_state("sleeping_waiting");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(leader_fsm::start_waking_up);
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
    }
    
    efc.new_state(
        "inactive_sleeping_moving", LEADER_STATE_INACTIVE_SLEEPING_MOVING
    ); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.change_state("inactive_sleeping_waiting");
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_PATH_BLOCKED); {
            efc.change_state("inactive_sleeping_stuck");
        }
        efc.new_event(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carry_reach_destination);
        }
        efc.new_event(MOB_EV_CARRY_DELIVERED); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("inactive_waking_up");
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("inactive_waking_up");
        }
        efc.new_event(LEADER_EV_ACTIVATED); {
            efc.run(leader_fsm::become_active);
            efc.change_state("sleeping_moving");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::start_waking_up);
            efc.run(leader_fsm::fall_down_pit);
            efc.change_state("idling");
        }
    }
    
    efc.new_state(
        "inactive_sleeping_stuck", LEADER_STATE_INACTIVE_SLEEPING_STUCK
    ); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_become_stuck);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.change_state("inactive_sleeping_waiting");
        }
        efc.new_event(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("sleeping_moving");
        }
        efc.new_event(LEADER_EV_CANCEL); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("inactive_waking_up");
        }
        efc.new_event(LEADER_EV_ACTIVATED); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(leader_fsm::become_active);
            efc.change_state("sleeping_moving");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(leader_fsm::start_waking_up);
            efc.run(leader_fsm::fall_down_pit);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("waking_up", LEADER_STATE_WAKING_UP); {
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("active");
        }
    }
    
    efc.new_state("inactive_waking_up", LEADER_STATE_INACTIVE_WAKING_UP); {
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
    }
    
    efc.new_state("held_by_leader", LEADER_STATE_HELD); {
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::be_released);
        }
        efc.new_event(MOB_EV_THROWN); {
            efc.run(leader_fsm::be_thrown);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_RELEASED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("thrown", LEADER_STATE_THROWN); {
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::stop_being_thrown);
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(leader_fsm::land);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(leader_fsm::be_thrown_by_bouncer);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("drinking", LEADER_STATE_DRINKING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::start_drinking);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(leader_fsm::finish_drinking);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("active");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.change_state("dying");
        }
    }
    
    efc.new_state("riding_track", LEADER_STATE_RIDING_TRACK); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::start_riding_track);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(leader_fsm::tick_track_ride);
        }
    }
    
    efc.new_state(
        "inactive_riding_track", LEADER_STATE_INACTIVE_RIDING_TRACK
    ); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(leader_fsm::start_riding_track);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(leader_fsm::tick_track_ride);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(leader_fsm::whistled_while_riding);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_LEADER_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_LEADER_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When a leader loses health.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the hitbox touch information structure.
 * info2:
 *   Unused.
 */
void leader_fsm::be_attacked(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    leader* l_ptr = (leader*) m;
    
    if(m->invuln_period.time_left > 0.0f) return;
    m->invuln_period.start();
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    
    float damage = 0;
    if(!info->mob2->calculate_damage(m, info->h2, info->h1, &damage)) {
        return;
    }
    
    m->apply_attack_damage(info->mob2, info->h2, info->h1, damage);
    
    m->stop_chasing();
    
    float knockback = 0;
    float knockback_angle = 0;
    info->mob2->calculate_knockback(
        m, info->h2, info->h1, &knockback, &knockback_angle
    );
    m->apply_knockback(knockback, knockback_angle);
    
    m->leave_group();
    
    m->do_attack_effects(info->mob2, info->h2, info->h1, damage, knockback);
    
    if(knockback > 0) {
        m->set_animation(LEADER_ANIM_KNOCKED_DOWN);
        
        if(l_ptr->active) m->fsm.set_state(LEADER_STATE_KNOCKED_BACK);
        else m->fsm.set_state(LEADER_STATE_INACTIVE_KNOCKED_BACK);
        
    } else {
        m->set_animation(LEADER_ANIM_PAIN);
        
        if(l_ptr->active) m->fsm.set_state(LEADER_STATE_PAIN);
        else m->fsm.set_state(LEADER_STATE_INACTIVE_PAIN);
        
    }
}


/* ----------------------------------------------------------------------------
 * When a leader's leader dismisses them.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::be_dismissed(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader is grabbed by another leader.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::be_grabbed_by_friend(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader grabbed by another is released.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::be_released(mob* m, void* info1, void* info2) {

}


/* ----------------------------------------------------------------------------
 * When a leader grabbed by another is thrown.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::be_thrown(mob* m, void* info1, void* info2) {
    ((leader*) m)->start_throw_trail();
}


/* ----------------------------------------------------------------------------
 * When a leader is thrown by a bouncer mob.
 * m:
 *   The mob.
 * info1:
 *   Points to the bouncer mob.
 * info2:
 *   Unused.
 */
void leader_fsm::be_thrown_by_bouncer(mob* m, void* info1, void* info2) {
    ((leader*) m)->start_throw_trail();
}


/* ----------------------------------------------------------------------------
 * When a leader is meant to become the active one.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::become_active(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    game.states.gameplay->cur_leader_ptr->fsm.run_event(
        LEADER_EV_INACTIVATED
    );
    
    size_t new_leader_nr = game.states.gameplay->cur_leader_nr;
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); ++l) {
        if(game.states.gameplay->mobs.leaders[l] == l_ptr) {
            new_leader_nr = l;
            break;
        }
    }
    
    game.states.gameplay->cur_leader_ptr = l_ptr;
    game.states.gameplay->cur_leader_nr = new_leader_nr;
    l_ptr->active = true;
    
    l_ptr->lea_type->sfx_name_call.play(0, false);
}


/* ----------------------------------------------------------------------------
 * When a leader stops being the active one.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::become_inactive(mob* m, void* info1, void* info2) {
    ((leader*) m)->active = false;
}


/* ----------------------------------------------------------------------------
 * When a leader must chase another.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::chase_leader(mob* m, void* info1, void* info2) {
    group_info_struct* leader_group_ptr = m->following_group->group;
    engine_assert(leader_group_ptr != NULL, m->print_state_history());
    
    float distance =
        m->following_group->type->radius +
        m->type->radius + game.config.standard_pikmin_radius;
        
    for(size_t me = 0; me < leader_group_ptr->members.size(); ++me) {
        mob* member_ptr = leader_group_ptr->members[me];
        if(member_ptr == m) {
            break;
        } else if(member_ptr->subgroup_type_ptr == m->subgroup_type_ptr) {
            //If this member is also a leader,
            //then that means the current leader should stick behind.
            distance +=
                member_ptr->type->radius * 2 + GROUP_SPOT_INTERVAL;
        }
    }
    
    m->chase(
        point(), &m->following_group->pos, false, NULL,
        true, distance
    );
    m->set_animation(LEADER_ANIM_WALKING);
    m->focus_on_mob(m->following_group);
}


/* ----------------------------------------------------------------------------
 * When a leader dies.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::die(mob* m, void* info1, void* info2) {
    size_t living_leaders = 0;
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); ++l) {
        if(game.states.gameplay->mobs.leaders[l]->health > 0) {
            living_leaders++;
        }
    }
    if(living_leaders == 0) {
        game.states.results->can_continue = false;
        game.states.results->leader_ko = true;
        game.states.gameplay->leave(gameplay_state::LEAVE_TO_FINISH);
    } else if(game.states.gameplay->cur_leader_ptr == m) {
        change_to_next_leader(true, true);
    }
    
    leader_fsm::release(m, info1, info2);
    leader_fsm::dismiss(m, info1, info2);
    m->stop_chasing();
    m->become_uncarriable();
    m->set_animation(LEADER_ANIM_LYING);
}


/* ----------------------------------------------------------------------------
 * When a leader dismisses the group.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::dismiss(mob* m, void* info1, void* info2) {
    ((leader*) m)->dismiss();
}


/* ----------------------------------------------------------------------------
 * When a leader throws the grabbed mob.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::do_throw(mob* m, void* info1, void* info2) {
    engine_assert(!m->holding.empty(), m->print_state_history());
    
    leader* leader_ptr = (leader*) m;
    mob* holding_ptr = leader_ptr->holding[0];
    
    engine_assert(holding_ptr != NULL, m->print_state_history());
    
    holding_ptr->fsm.run_event(MOB_EV_THROWN);
    holding_ptr->start_height_effect();
    
    holding_ptr->stop_chasing();
    holding_ptr->pos = leader_ptr->pos;
    holding_ptr->z = leader_ptr->z;
    
    float angle;
    float target_z;
    if(game.states.gameplay->leader_cursor_mob) {
        target_z =
            game.states.gameplay->leader_cursor_mob->z +
            game.states.gameplay->leader_cursor_mob->height;
    } else if(game.states.gameplay->leader_cursor_sector) {
        target_z = game.states.gameplay->leader_cursor_sector->z;
    } else {
        target_z = m->z;
    }
    
    float max_height;
    switch (holding_ptr->type->category->id) {
    case MOB_CATEGORY_PIKMIN: {
        max_height = ((pikmin*) holding_ptr)->pik_type->max_throw_height;
        break;
    } case MOB_CATEGORY_LEADERS: {
        max_height = ((leader*) holding_ptr)->lea_type->max_throw_height;
        break;
    } default: {
        max_height = (target_z - leader_ptr->z) * 1.2f;
        break;
    }
    }
    
    if(max_height < target_z) {
        //Can't reach! Just do a convincing throw that is sure to fail.
        //Limiting the "target" Z makes it so the horizontal velocity isn't
        //so wild.
        target_z = max_height * 0.75;
    }
    
    holding_ptr->calculate_throw(
        game.states.gameplay->leader_cursor_w,
        target_z,
        max_height,
        &holding_ptr->speed,
        &holding_ptr->speed_z,
        &angle
    );
    
    holding_ptr->z_cap = m->z + max_height;
    
    holding_ptr->angle = angle;
    holding_ptr->angle_cos = cos(angle);
    holding_ptr->angle_sin = sin(angle);
    holding_ptr->face(angle, NULL);
    
    holding_ptr->was_thrown = true;
    holding_ptr->leave_group();
    leader_ptr->release(holding_ptr);
    
    game.sys_assets.sfx_throw.stop();
    game.sys_assets.sfx_throw.play(0, false);
    leader_ptr->set_animation(LEADER_ANIM_THROWING);
}


/* ----------------------------------------------------------------------------
 * When a leader enters the active state.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::enter_active(mob* m, void* info1, void* info2) {
    ((leader*) m)->is_in_walking_anim = false;
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader enters the idling state.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::enter_idle(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader falls asleep.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::fall_asleep(mob* m, void* info1, void* info2) {
    leader_fsm::dismiss(m, NULL, NULL);
    m->stop_chasing();
    
    m->become_carriable(CARRY_DESTINATION_ONION);
    
    m->set_animation(LEADER_ANIM_LYING);
}


/* ----------------------------------------------------------------------------
 * When a leader falls down a bottomless pit.
 * This damages and respawns them.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->leave_group();
    m->set_health(true, true, -0.2);
    m->respawn();
}


/* ----------------------------------------------------------------------------
 * When the leader finishes the animation of the current pluck.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::finish_current_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    l_ptr->stop_chasing();
    l_ptr->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader finishes drinking the drop it was drinking.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::finish_drinking(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    drop* d_ptr = (drop*) m->focused_mob;
    
    switch(d_ptr->dro_type->effect) {
    case DROP_EFFECT_INCREASE_SPRAYS: {
        spray_stats_struct* stats =
            &game.states.gameplay->spray_stats[
        d_ptr->dro_type->spray_type_to_increase
        ];
        stats->nr_sprays =
            std::max(
                (long long) stats->nr_sprays + d_ptr->dro_type->increase_amount,
                (long long) 0
            );
        break;
    } case DROP_EFFECT_GIVE_STATUS: {
        m->apply_status_effect(
            d_ptr->dro_type->status_to_give, true, false
        );
        break;
    } default: {
        break;
    }
    }
    
    m->unfocus_from_mob();
}


/* ----------------------------------------------------------------------------
 * When a leader heads towards a Pikmin with the intent to pluck it.
 * Also signals other leaders in the group to search for other seeds.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the Pikmin to be plucked.
 * info2:
 *   Unused.
 */
void leader_fsm::go_pluck(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    leader* lea_ptr = (leader*) m;
    pikmin* pik_ptr = (pikmin*) info1;
    
    lea_ptr->queued_pluck_cancel = false;
    
    lea_ptr->auto_plucking = true;
    lea_ptr->pluck_target = pik_ptr;
    lea_ptr->chase(
        pik_ptr->pos, NULL,
        false, nullptr, true,
        pik_ptr->type->radius + lea_ptr->type->radius
    );
    pik_ptr->pluck_reserved = true;
    
    //Now for the leaders in the group.
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); ++l) {
        leader* l2_ptr = game.states.gameplay->mobs.leaders[l];
        if(l2_ptr->following_group == lea_ptr) {
            l2_ptr->auto_plucking = true;
            l2_ptr->fsm.run_event(LEADER_EV_MUST_SEARCH_SEED);
        }
    }
    
    lea_ptr->set_animation(LEADER_ANIM_WALKING);
}


/* ----------------------------------------------------------------------------
 * When a leader grabs onto a mob for throwing.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the mob to grab.
 * info2:
 *   Unused.
 */
void leader_fsm::grab_mob(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    leader* l_ptr = (leader*) m;
    mob* grabbed_mob = (mob*) info1;
    l_ptr->hold(
        grabbed_mob, INVALID, LEADER_HELD_MOB_DIST, LEADER_HELD_MOB_ANGLE,
        false, true
    );
    l_ptr->group->sort(grabbed_mob->subgroup_type_ptr);
}


/* ----------------------------------------------------------------------------
 * When a leader joins another leader's group. This transfers their Pikmin.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::join_group(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    
    game.states.gameplay->cur_leader_ptr->add_to_group(l_ptr);
    while(!l_ptr->group->members.empty()) {
        mob* member = l_ptr->group->members[0];
        member->leave_group();
        game.states.gameplay->cur_leader_ptr->add_to_group(member);
    }
}


/* ----------------------------------------------------------------------------
 * When a thrown leader lands.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::land(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->speed.x = m->speed.y = 0;
    
    m->remove_particle_generator(MOB_PARTICLE_GENERATOR_THROW);
    
    if(m == game.states.gameplay->cur_leader_ptr) {
        m->fsm.set_state(LEADER_STATE_ACTIVE);
    } else {
        m->fsm.set_state(LEADER_STATE_IDLING);
    }
}


/* ----------------------------------------------------------------------------
 * When a leader leaves a hazardous sector.
 * m:
 *   The mob.
 * info1:
 *   Points to the hazard.
 * info2:
 *   Unused.
 */
void leader_fsm::left_hazard(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hazard* h = (hazard*) info1;
    if(h->associated_liquid) {
        m->remove_particle_generator(MOB_PARTICLE_GENERATOR_WAVE_RING);
    }
}


/* ----------------------------------------------------------------------------
 * When a leader should lose his momentum and stand still.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::lose_momentum(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->speed.x = m->speed.y = 0;
}


/* ----------------------------------------------------------------------------
 * When a leader begins to move via player control.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the movement info structure.
 * info2:
 *   Unused.
 */
void leader_fsm::move(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    leader* l_ptr = (leader*) m;
    movement_struct* mov = (movement_struct*) info1;
    point final_coords;
    float dummy_angle;
    float dummy_magnitude;
    mov->get_clean_info(
        &final_coords, &dummy_angle, &dummy_magnitude
    );
    final_coords *= l_ptr->type->move_speed;
    final_coords += l_ptr->pos;
    l_ptr->chase(final_coords, NULL, false, NULL, true);
}


/* ----------------------------------------------------------------------------
 * When a leader notifies the mob it's holding that it will be released.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::notify_pikmin_release(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(l_ptr->holding.empty()) return;
    l_ptr->holding[0]->fsm.run_event(MOB_EV_RELEASED);
}


/* ----------------------------------------------------------------------------
 * When a leader punches.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::punch(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_PUNCHING);
    m->stop_turning();
}


/* ----------------------------------------------------------------------------
 * Queues the stopping of the plucking session, for after this pluck's end.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::queue_stop_auto_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    l_ptr->queued_pluck_cancel = true;
}


/* ----------------------------------------------------------------------------
 * When a leader gently releases the held mob.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::release(mob* m, void* info1, void* info2) {
    if(m->holding.empty()) return;
    m->release(m->holding[0]);
}


/* ----------------------------------------------------------------------------
 * When a leader searches for a seed next to them.
 * If found, issues events to go towards the seed.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::search_seed(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    
    dist d;
    pikmin* new_pikmin = NULL;
    if(!l_ptr->queued_pluck_cancel) {
        new_pikmin =
            get_closest_sprout(l_ptr->pos, &d, false);
    } else {
        leader_fsm::stop_auto_pluck(m, NULL, NULL);
    }
    
    if(l_ptr->active) {
        l_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    } else {
        if(l_ptr->following_group) {
            l_ptr->fsm.set_state(LEADER_STATE_IN_GROUP_CHASING);
        } else {
            l_ptr->fsm.set_state(LEADER_STATE_IDLING);
        }
    }
    
    if(new_pikmin && d <= game.config.next_pluck_range) {
        l_ptr->fsm.run_event(LEADER_EV_GO_PLUCK, (void*) new_pikmin);
        l_ptr->queued_pluck_cancel = false;
    } else {
        leader_fsm::stop_auto_pluck(m, NULL, NULL);
    }
}


/* ----------------------------------------------------------------------------
 * When a leader needs to change to the idling animation.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::set_stop_anim(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(l_ptr->is_in_walking_anim) {
        l_ptr->set_animation(LEADER_ANIM_IDLING);
        l_ptr->is_in_walking_anim = false;
    }
}


/* ----------------------------------------------------------------------------
 * When a leader needs to change to the walking animation.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::set_walk_anim(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(!l_ptr->is_in_walking_anim) {
        l_ptr->set_animation(LEADER_ANIM_WALKING);
        l_ptr->is_in_walking_anim = true;
    }
}


/* ----------------------------------------------------------------------------
 * When the leader must signal to their follower leaders to stop plucking.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::signal_stop_auto_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); ++l) {
        leader* l2_ptr = game.states.gameplay->mobs.leaders[l];
        if(l2_ptr->following_group == l_ptr) {
            l2_ptr->fsm.run_event(LEADER_EV_CANCEL);
        }
    }
}


/* ----------------------------------------------------------------------------
 * When a leader uses a spray.
 * m:
 *   The mob.
 * info1:
 *   Pointer to a size_t with the spray's ID number.
 * info2:
 *   Unused.
 */
void leader_fsm::spray(mob* m, void* info1, void* info2) {
    size_t spray_nr = *((size_t*) info1);
    
    if(game.states.gameplay->spray_stats[spray_nr].nr_sprays == 0) {
        m->fsm.set_state(LEADER_STATE_ACTIVE);
        return;
    }
    
    float cursor_angle =
        get_angle(m->pos, game.states.gameplay->leader_cursor_w);
    float shoot_angle =
        cursor_angle + ((game.spray_types[spray_nr].angle) ? TAU / 2 : 0);
        
    unordered_set<mob*> affected_mobs;
    if(game.spray_types[spray_nr].group) {
        for(size_t gm = 0; gm < m->group->members.size(); ++gm) {
            if(
                m->group->members[gm]->type->category->id ==
                MOB_CATEGORY_PIKMIN
            ) {
                affected_mobs.insert(m->group->members[gm]);
            }
        }
        //If there is nothing to get sprayed, better not waste it.
        if(affected_mobs.empty())  {
            m->fsm.set_state(LEADER_STATE_ACTIVE);
            return;
        };
        
    } else {
        for(
            size_t am = 0; am < game.states.gameplay->mobs.all.size(); ++am
        ) {
            mob* am_ptr = game.states.gameplay->mobs.all[am];
            if(am_ptr == m) continue;
            
            if(
                dist(m->pos, am_ptr->pos) >
                game.spray_types[spray_nr].distance_range + am_ptr->type->radius
            ) {
                continue;
            }
            
            float angle_dif =
                get_angle_smallest_dif(
                    shoot_angle,
                    get_angle(m->pos, am_ptr->pos)
                );
            if(angle_dif > game.spray_types[spray_nr].angle_range / 2) continue;
            
            affected_mobs.insert(am_ptr);
        }
        
    }
    
    for(auto am : affected_mobs) {
        am->fsm.run_event(
            MOB_EV_TOUCHED_SPRAY, (void*) &game.spray_types[spray_nr]
        );
    }
    
    particle p(
        PARTICLE_TYPE_BITMAP, m->pos, m->z + m->height,
        52, 3.5, PARTICLE_PRIORITY_MEDIUM
    );
    p.bitmap = game.sys_assets.bmp_smoke;
    p.friction = 1;
    p.color = game.spray_types[spray_nr].main_color;
    particle_generator pg(0, p, 32);
    pg.angle = shoot_angle;
    pg.angle_deviation = game.spray_types[spray_nr].angle_range / 2.0f;
    pg.total_speed = game.spray_types[spray_nr].distance_range * 0.8;
    pg.total_speed_deviation = game.spray_types[spray_nr].distance_range * 0.4;
    pg.size_deviation = 0.5;
    pg.emit(game.states.gameplay->particles);
    
    game.states.gameplay->spray_stats[spray_nr].nr_sprays--;
    
    m->stop_chasing();
    m->set_animation(LEADER_ANIM_SPRAYING);
}


/* ----------------------------------------------------------------------------
 * When a leader starts drinking the drop it touched.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the drop mob.
 * info2:
 *   Unused.
 */
void leader_fsm::start_drinking(mob* m, void* info1, void* info2) {
    mob* drop_ptr = (mob*) info1;
    m->leave_group();
    m->stop_chasing();
    m->focus_on_mob(drop_ptr);
    m->set_animation(LEADER_ANIM_DRINKING);
    m->face(get_angle(m->pos, drop_ptr->pos), NULL);
}


/* ----------------------------------------------------------------------------
 * When a leader grabs on to a sprout and begins plucking it out.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::start_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    engine_assert(l_ptr->pluck_target != NULL, m->print_state_history());
    
    l_ptr->pluck_target->fsm.run_event(MOB_EV_PLUCKED, (void*) l_ptr);
    l_ptr->pluck_target->pluck_reserved = false;
    l_ptr->pluck_target = nullptr;
    l_ptr->set_animation(LEADER_ANIM_PLUCKING);
}


/* ----------------------------------------------------------------------------
 * When a leader starts riding on a track.
 * m:
 *   The mob.
 * info1:
 *   Points to the track mob.
 * info2:
 *   Unused.
 */
void leader_fsm::start_riding_track(mob* m, void* info1, void* info2) {
    track* tra_ptr = (track*) info1;
    
    leader_fsm::dismiss(m, NULL, NULL);
    m->leave_group();
    m->stop_chasing();
    m->focus_on_mob(tra_ptr);
    m->start_height_effect();
    
    switch(tra_ptr->tra_type->riding_pose) {
    case TRACK_RIDING_POSE_STOPPED: {
        m->set_animation(LEADER_ANIM_WALKING);
        break;
    } case TRACK_RIDING_POSE_CLIMBING: {
        m->set_animation(LEADER_ANIM_WALKING);
        break;
    } case TRACK_RIDING_POSE_SLIDING: {
        m->set_animation(LEADER_ANIM_WALKING);
        break;
    }
    }
    
    vector<size_t> checkpoints;
    for(size_t c = 0; c < tra_ptr->type->anims.body_parts.size() - 1; ++c) {
        checkpoints.push_back(c);
    }
    m->track_info =
        new track_info_struct(
        tra_ptr, checkpoints, tra_ptr->tra_type->ride_speed
    );
}


/* ----------------------------------------------------------------------------
 * When a leader wakes up.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::start_waking_up(mob* m, void* info1, void* info2) {
    m->become_uncarriable();
    m->set_animation(LEADER_ANIM_GETTING_UP);
}


/* ----------------------------------------------------------------------------
 * When a leader stops moving.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::stop(mob* m, void* info1, void* info2) {
    m->stop_chasing();
}


/* ----------------------------------------------------------------------------
 * When a leader quits the auto-plucking mindset.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::stop_auto_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(l_ptr->pluck_target) {
        l_ptr->stop_chasing();
        l_ptr->pluck_target->pluck_reserved = false;
    }
    l_ptr->auto_plucking = false;
    l_ptr->queued_pluck_cancel = false;
    l_ptr->pluck_target = NULL;
    l_ptr->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader is no longer in the thrown state.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::stop_being_thrown(mob* m, void* info1, void* info2) {
    //Remove the throw particle generator.
    m->remove_particle_generator(MOB_PARTICLE_GENERATOR_THROW);
}


/* ----------------------------------------------------------------------------
 * When a leader stands still while in another's group.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::stop_in_group(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader stops whistling.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::stop_whistle(mob* m, void* info1, void* info2) {
    ((leader*) m)->stop_whistling();
}


/* ----------------------------------------------------------------------------
 * Every tick in the active state.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::tick_active_state(mob* m, void* info1, void* info2) {
    m->face(get_angle(m->pos, game.states.gameplay->leader_cursor_w), NULL);
}


/* ----------------------------------------------------------------------------
 * When a leader has to teleport to its spot in a track it is riding.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::tick_track_ride(mob* m, void* info1, void* info2) {
    engine_assert(m->track_info != NULL, m->print_state_history());
    
    if(m->tick_track_ride()) {
        //Finished!
        if(((leader*) m)->active) {
            m->fsm.set_state(LEADER_STATE_ACTIVE, NULL, NULL);
        } else {
            m->fsm.set_state(LEADER_STATE_IDLING, NULL, NULL);
        }
    }
}


/* ----------------------------------------------------------------------------
 * When a leader touches a hazard.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the hazard.
 * info2:
 *   Unused.
 */
void leader_fsm::touched_hazard(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    leader* l = (leader*) m;
    hazard* h = (hazard*) info1;
    
    for(size_t e = 0; e < h->effects.size(); ++e) {
        l->apply_status_effect(h->effects[e], false, false);
    }
    
    if(h->associated_liquid) {
        bool already_generating = false;
        for(size_t g = 0; g < m->particle_generators.size(); ++g) {
            if(
                m->particle_generators[g].id ==
                MOB_PARTICLE_GENERATOR_WAVE_RING
            ) {
                already_generating = true;
                break;
            }
        }
        
        if(!already_generating) {
            particle p(
                PARTICLE_TYPE_BITMAP, m->pos, m->z,
                0, 1, PARTICLE_PRIORITY_LOW
            );
            p.bitmap = game.sys_assets.bmp_wave_ring;
            p.size_grow_speed = m->type->radius * 4;
            particle_generator pg(0.3, p, 1);
            pg.follow_mob = m;
            pg.id = MOB_PARTICLE_GENERATOR_WAVE_RING;
            m->particle_generators.push_back(pg);
        }
    }
}


/* ----------------------------------------------------------------------------
 * When a leader is sprayed.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the spray type.
 * info2:
 *   Unused.
 */
void leader_fsm::touched_spray(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    leader* l = (leader*) m;
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); ++e) {
        l->apply_status_effect(s->effects[e], false, false);
    }
}


/* ----------------------------------------------------------------------------
 * When a leader begins whistling.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::whistle(mob* m, void* info1, void* info2) {
    ((leader*) m)->start_whistling();
}


/* ----------------------------------------------------------------------------
 * When a leader is whistled over by another leader while riding on a track.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void leader_fsm::whistled_while_riding(mob* m, void* info1, void* info2) {
    engine_assert(m->track_info, m->print_state_history());
    
    track* tra_ptr = (track*) (m->track_info->m);
    
    if(!tra_ptr->tra_type->cancellable_with_whistle) {
        return;
    }
    
    m->stop_track_ride();
    leader_fsm::join_group(m, NULL, NULL);
    m->fsm.set_state(LEADER_STATE_IN_GROUP_CHASING);
}
