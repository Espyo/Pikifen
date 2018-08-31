/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader finite state machine logic.
 */

#include "../functions.h"
#include "../gameplay.h"
#include "leader.h"
#include "leader_fsm.h"
#include "leader_type.h"
#include "mob_fsm.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the leader's logic.
 */
void leader_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", LEADER_STATE_IDLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::enter_idle);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(leader_fsm::inactive_search_seed);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(leader_fsm::join_group);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(LEADER_EVENT_FOCUSED); {
            efc.run(leader_fsm::focus);
            efc.change_state("active");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::inactive_be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(LEADER_EVENT_INACTIVE_SEARCH_SEED); {
            efc.run(leader_fsm::inactive_search_seed);
        }
        efc.new_event(LEADER_EVENT_GO_PLUCK); {
            efc.run(leader_fsm::go_pluck);
            efc.change_state("inactive_going_to_pluck");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("active", LEADER_STATE_ACTIVE); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::enter_active);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(leader_fsm::tick_active_state);
        }
        efc.new_event(LEADER_EVENT_UNFOCUSED); {
            efc.run(leader_fsm::stop);
            efc.run(leader_fsm::unfocus);
            efc.change_state("idling");
        }
        efc.new_event(LEADER_EVENT_MOVE_START); {
            efc.run(leader_fsm::move);
            efc.run(leader_fsm::set_walk_anim);
        }
        efc.new_event(LEADER_EVENT_MOVE_END); {
            efc.run(leader_fsm::stop);
            efc.run(leader_fsm::set_stop_anim);
        }
        efc.new_event(LEADER_EVENT_HOLDING); {
            efc.run(leader_fsm::grab_mob);
            efc.change_state("holding");
        }
        efc.new_event(LEADER_EVENT_START_WHISTLE); {
            efc.change_state("whistling");
        }
        efc.new_event(LEADER_EVENT_PUNCH); {
            efc.change_state("punching");
        }
        efc.new_event(LEADER_EVENT_DISMISS); {
            efc.change_state("dismissing");
        }
        efc.new_event(LEADER_EVENT_SPRAY); {
            efc.change_state("spraying");
        }
        efc.new_event(LEADER_EVENT_LIE_DOWN); {
            efc.run(leader_fsm::fall_asleep);
            efc.change_state("sleeping_waiting");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(LEADER_EVENT_GO_PLUCK); {
            efc.run(leader_fsm::go_pluck);
            efc.change_state("going_to_pluck");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("whistling", LEADER_STATE_WHISTLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::whistle);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(leader_fsm::stop_whistle);
        }
        efc.new_event(LEADER_EVENT_STOP_WHISTLE); {
            efc.change_state("active");
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.change_state("active");
        }
        efc.new_event(LEADER_EVENT_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.new_event(LEADER_EVENT_MOVE_END); {
            efc.run(leader_fsm::stop);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("punching", LEADER_STATE_PUNCHING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::punch);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("active");
        }
        efc.new_event(LEADER_EVENT_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.new_event(LEADER_EVENT_MOVE_END); {
            efc.run(leader_fsm::stop);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("holding", LEADER_STATE_HOLDING); {
        efc.new_event(LEADER_EVENT_THROW); {
            efc.run(leader_fsm::do_throw);
            efc.change_state("active");
        }
        efc.new_event(LEADER_EVENT_RELEASE); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.change_state("active");
        }
        efc.new_event(LEADER_EVENT_MOVE_START); {
            efc.run(leader_fsm::move);
            efc.run(leader_fsm::set_walk_anim);
        }
        efc.new_event(LEADER_EVENT_MOVE_END); {
            efc.run(leader_fsm::stop);
            efc.run(leader_fsm::set_stop_anim);
        }
        efc.new_event(LEADER_EVENT_START_WHISTLE); {
            efc.change_state("whistling");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::notify_pikmin_release);
            efc.run(leader_fsm::release);
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("dismissing", LEADER_STATE_DISMISSING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::dismiss);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("active");
        }
        efc.new_event(LEADER_EVENT_MOVE_START); {
            efc.run(leader_fsm::move);
        }
        efc.new_event(LEADER_EVENT_MOVE_END); {
            efc.run(leader_fsm::stop);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
    }
    
    efc.new_state("spraying", LEADER_STATE_SPRAYING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::spray);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("active");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
    }
    
    efc.new_state("pain", LEADER_STATE_PAIN); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::suffer_pain);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("active");
        }
    }
    
    efc.new_state("inactive_pain", LEADER_STATE_INACTIVE_PAIN); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::suffer_pain);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run(leader_fsm::be_dismissed);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("knocked_back", LEADER_STATE_KNOCKED_BACK); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::get_knocked_back);
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(leader_fsm::lose_momentum);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("active");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "inactive_knocked_back", LEADER_STATE_INACTIVE_KNOCKED_BACK
    ); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::get_knocked_back);
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(leader_fsm::lose_momentum);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("dying", LEADER_STATE_DYING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::die);
        }
    }
    
    efc.new_state("in_group_chasing", LEADER_STATE_IN_GROUP_CHASING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::chase_leader);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("in_group_stopped");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(leader_fsm::be_dismissed);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(leader_fsm::be_grabbed_by_friend);
            efc.change_state("held_by_leader");
        }
        efc.new_event(LEADER_EVENT_INACTIVE_SEARCH_SEED); {
            efc.run(leader_fsm::inactive_search_seed);
        }
        efc.new_event(LEADER_EVENT_GO_PLUCK); {
            efc.run(leader_fsm::go_pluck);
            efc.change_state("inactive_going_to_pluck");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::inactive_be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("in_group_stopped", LEADER_STATE_IN_GROUP_STOPPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(leader_fsm::stop_in_group);
        }
        efc.new_event(MOB_EVENT_SPOT_IS_FAR); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(leader_fsm::be_dismissed);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(leader_fsm::be_grabbed_by_friend);
            efc.change_state("held_by_leader");
        }
        efc.new_event(LEADER_EVENT_INACTIVE_SEARCH_SEED); {
            efc.run(leader_fsm::inactive_search_seed);
        }
        efc.new_event(LEADER_EVENT_GO_PLUCK); {
            efc.run(leader_fsm::go_pluck);
            efc.change_state("inactive_going_to_pluck");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::inactive_be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("going_to_pluck", LEADER_STATE_GOING_TO_PLUCK); {
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(leader_fsm::start_pluck);
            efc.change_state("plucking");
        }
        efc.new_event(LEADER_EVENT_CANCEL); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.run(leader_fsm::signal_stop_auto_pluck);
            efc.change_state("active");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.run(leader_fsm::be_attacked);
            efc.change_state("active");
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.change_state("dying");
        }
        efc.new_event(LEADER_EVENT_UNFOCUSED); {
            efc.run(leader_fsm::unfocus);
            efc.change_state("inactive_going_to_pluck");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("plucking", LEADER_STATE_PLUCKING); {
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run(leader_fsm::finish_current_pluck);
            efc.run(leader_fsm::search_seed);
        }
        efc.new_event(LEADER_EVENT_CANCEL); {
            efc.run(leader_fsm::queue_stop_auto_pluck);
            efc.run(leader_fsm::signal_stop_auto_pluck);
        }
        efc.new_event(LEADER_EVENT_UNFOCUSED); {
            efc.run(leader_fsm::unfocus);
            efc.change_state("inactive_plucking");
        }
    }
    
    efc.new_state(
        "inactive_going_to_pluck", LEADER_STATE_INACTIVE_GOING_TO_PLUCK
    ); {
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(leader_fsm::start_pluck);
            efc.change_state("inactive_plucking");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(leader_fsm::join_group);
            efc.run(leader_fsm::stop_auto_pluck);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(LEADER_EVENT_CANCEL); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.run(leader_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run(leader_fsm::stop_auto_pluck);
            efc.change_state("dying");
        }
        efc.new_event(LEADER_EVENT_FOCUSED); {
            efc.run(leader_fsm::focus);
            efc.change_state("going_to_pluck");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("inactive_plucking", LEADER_STATE_INACTIVE_PLUCKING); {
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run(leader_fsm::finish_current_pluck);
            efc.run(leader_fsm::inactive_search_seed);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(leader_fsm::join_group);
            efc.run(leader_fsm::queue_stop_auto_pluck);
        }
        efc.new_event(LEADER_EVENT_CANCEL); {
            efc.run(leader_fsm::queue_stop_auto_pluck);
        }
        efc.new_event(LEADER_EVENT_FOCUSED); {
            efc.run(leader_fsm::focus);
            efc.change_state("plucking");
        }
    }
    
    efc.new_state("sleeping_waiting", LEADER_STATE_SLEEPING_WAITING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("sleeping_moving");
        }
        efc.new_event(LEADER_EVENT_CANCEL); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("waking_up");
        }
        efc.new_event(LEADER_EVENT_UNFOCUSED); {
            efc.run(leader_fsm::unfocus);
            efc.change_state("inactive_sleeping_waiting");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
    }
    
    efc.new_state("sleeping_moving", LEADER_STATE_SLEEPING_MOVING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_begin);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.change_state("sleeping_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_begin_move);
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("waking_up");
        }
        efc.new_event(LEADER_EVENT_CANCEL); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("waking_up");
        }
        efc.new_event(LEADER_EVENT_UNFOCUSED); {
            efc.run(leader_fsm::unfocus);
            efc.change_state("inactive_sleeping_moving");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "inactive_sleeping_waiting", LEADER_STATE_INACTIVE_SLEEPING_WAITING
    ); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("inactive_sleeping_moving");
        }
        efc.new_event(LEADER_EVENT_CANCEL); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("inactive_waking_up");
        }
        efc.new_event(LEADER_EVENT_FOCUSED); {
            efc.run(leader_fsm::focus);
            efc.change_state("sleeping_waiting");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
    }
    
    efc.new_state(
        "inactive_sleeping_moving", LEADER_STATE_INACTIVE_SLEEPING_MOVING
    ); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_begin);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.change_state("inactive_sleeping_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_begin_move);
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("inactive_waking_up");
        }
        efc.new_event(LEADER_EVENT_CANCEL); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("inactive_waking_up");
        }
        efc.new_event(LEADER_EVENT_FOCUSED); {
            efc.run(leader_fsm::focus);
            efc.change_state("sleeping_moving");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::be_attacked);
            efc.run(leader_fsm::start_waking_up);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run(leader_fsm::start_waking_up);
            efc.change_state("dying");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("waking_up", LEADER_STATE_WAKING_UP); {
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("active");
        }
    }
    
    efc.new_state("inactive_waking_up", LEADER_STATE_INACTIVE_WAKING_UP); {
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("idling");
        }
    }
    
    efc.new_state("held_by_leader", LEADER_STATE_HELD); {
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(leader_fsm::be_released);
        }
        efc.new_event(MOB_EVENT_THROWN); {
            efc.run(leader_fsm::be_thrown);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EVENT_RELEASED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(leader_fsm::inactive_be_attacked);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.change_state("dying");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("thrown", LEADER_STATE_THROWN); {
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(leader_fsm::stop_being_thrown);
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(leader_fsm::land);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(leader_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(leader_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(leader_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(leader_fsm::fall_down_pit);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    assert(typ->states.size() == N_LEADER_STATES);
}


/* ----------------------------------------------------------------------------
 * When a leader begins whistling.
 */
void leader_fsm::whistle(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    
    l_ptr->lea_type->sfx_whistle.play(0, false);
    
    for(unsigned char d = 0; d < 6; ++d) whistle_dot_radius[d] = -1;
    whistle_fade_timer.start();
    whistle_fade_radius = 0;
    whistling = true;
    l_ptr->lea_type->sfx_whistle.play(0, false);
    l_ptr->set_animation(LEADER_ANIM_WHISTLING);
    l_ptr->script_timer.start(2.5f);
}


/* ----------------------------------------------------------------------------
 * When a leader stops whistling.
 */
void leader_fsm::stop_whistle(mob* m, void* info1, void* info2) {
    if(!whistling) return;
    
    ((leader*) m)->lea_type->sfx_whistle.stop();
    
    whistle_fade_timer.start();
    whistle_fade_radius = whistle_radius;
    
    whistling = false;
    whistle_radius = 0;
    
}


/* ----------------------------------------------------------------------------
 * When a leader joins another leader's group. This transfers their Pikmin.
 */
void leader_fsm::join_group(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    
    cur_leader_ptr->add_to_group(l_ptr);
    size_t n_group_members = l_ptr->group->members.size();
    for(size_t m = 0; m < n_group_members; ++m) {
        mob* member = l_ptr->group->members[0];
        member->leave_group();
        cur_leader_ptr->add_to_group(member);
    }
}


/* ----------------------------------------------------------------------------
 * When a leader falls down a bottomless pit.
 * This damages and respawns them.
 */
void leader_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->leave_group();
    m->set_health(true, true, -0.2);
    m->respawn();
}


/* ----------------------------------------------------------------------------
 * When a leader is meant to become the active one.
 */
void leader_fsm::focus(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    cur_leader_ptr->fsm.run_event(LEADER_EVENT_UNFOCUSED);
    
    size_t new_leader_nr = cur_leader_nr;
    for(size_t l = 0; l < leaders.size(); ++l) {
        if(leaders[l] == l_ptr) {
            new_leader_nr = l;
            break;
        }
    }
    
    cur_leader_ptr = l_ptr;
    cur_leader_nr = new_leader_nr;
    
    l_ptr->lea_type->sfx_name_call.play(0, false);
}


/* ----------------------------------------------------------------------------
 * When a leader enters the idling state.
 */
void leader_fsm::enter_idle(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader enters the active state.
 */
void leader_fsm::enter_active(mob* m, void* info1, void* info2) {
    ((leader*) m)->is_in_walking_anim = false;
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader stops being the active one.
 */
void leader_fsm::unfocus(mob* m, void* info1, void* info2) {

}


/* ----------------------------------------------------------------------------
 * Every tick in the active state.
 */
void leader_fsm::tick_active_state(mob* m, void* info1, void* info2) {
    m->face(cursor_angle, NULL);
}


/* ----------------------------------------------------------------------------
 * When a leader touches a hazard.
 * info1: Pointer to the hazard.
 */
void leader_fsm::touched_hazard(mob* m, void* info1, void* info2) {
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
            p.bitmap = bmp_wave_ring;
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
 * info1: Pointer to the spray type.
 */
void leader_fsm::touched_spray(mob* m, void* info1, void* info2) {
    leader* l = (leader*) m;
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); ++e) {
        l->apply_status_effect(s->effects[e], false, false);
    }
}


/* ----------------------------------------------------------------------------
 * When a leader begins to move via player control.
 * info1: Pointer to the movement info structure.
 */
void leader_fsm::move(mob* m, void* info1, void* info2) {
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
 * When a leader stops moving.
 */
void leader_fsm::stop(mob* m, void* info1, void* info2) {
    m->stop_chasing();
}


/* ----------------------------------------------------------------------------
 * When a leader needs to change to the walking animation.
 */
void leader_fsm::set_walk_anim(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(!l_ptr->is_in_walking_anim) {
        l_ptr->set_animation(LEADER_ANIM_WALKING);
        l_ptr->is_in_walking_anim = true;
    }
}


/* ----------------------------------------------------------------------------
 * When a leader needs to change to the idling animation.
 */
void leader_fsm::set_stop_anim(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(l_ptr->is_in_walking_anim) {
        l_ptr->set_animation(LEADER_ANIM_IDLING);
        l_ptr->is_in_walking_anim = false;
    }
}


/* ----------------------------------------------------------------------------
 * When a leader grabs onto a mob for throwing.
 * info1: Pointer to the mob.
 */
void leader_fsm::grab_mob(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    l_ptr->hold(
        (mob*) info1, INVALID, LEADER_HELD_MOB_DIST, LEADER_HELD_MOB_ANGLE,
        false
    );
    l_ptr->group->sort(m->subgroup_type_ptr);
}


/* ----------------------------------------------------------------------------
 * When a leader throws the grabbed mob.
 */
void leader_fsm::do_throw(mob* m, void* info1, void* info2) {
    leader* leader_ptr = (leader*) m;
    mob* holding_ptr = leader_ptr->holding[0];
    
    holding_ptr->fsm.run_event(MOB_EVENT_THROWN);
    
    holding_ptr->pos = leader_ptr->pos;
    holding_ptr->z = leader_ptr->z;
    
    float angle, mag;
    coordinates_to_angle(
        leader_cursor_w - leader_ptr->pos,
        &angle, &mag
    );
    
    float throw_height_mult = 1.0;
    if(holding_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
        throw_height_mult =
            ((pikmin*) holding_ptr)->pik_type->throw_strength_mult;
    } else if(holding_ptr->type->category->id == MOB_CATEGORY_LEADERS) {
        throw_height_mult =
            ((leader*) holding_ptr)->lea_type->throw_strength_mult;
    }
    
    //Regular Pikmin are thrown about 288.88 units high.
    holding_ptr->speed.x =
        cos(angle) * mag * THROW_DISTANCE_MULTIPLIER *
        (1.0 / (THROW_STRENGTH_MULTIPLIER * throw_height_mult));
    holding_ptr->speed.y =
        sin(angle) * mag * THROW_DISTANCE_MULTIPLIER *
        (1.0 / (THROW_STRENGTH_MULTIPLIER * throw_height_mult));
    holding_ptr->speed_z = get_throw_z_speed(throw_height_mult);
    
    holding_ptr->z_cap =
        m->z + get_max_throw_height(holding_ptr->speed_z);
        
    holding_ptr->angle = angle;
    holding_ptr->face(angle, NULL);
    
    holding_ptr->was_thrown = true;
    
    holding_ptr->leave_group();
    leader_ptr->release(holding_ptr);
    
    sfx_throw.stop();
    sfx_throw.play(0, false);
    leader_ptr->set_animation(LEADER_ANIM_THROWING);
}


/* ----------------------------------------------------------------------------
 * When a leader notifies the mob it's holding that it will be released.
 */
void leader_fsm::notify_pikmin_release(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(l_ptr->holding.empty()) return;
    l_ptr->holding[0]->fsm.run_event(MOB_EVENT_RELEASED);
}


/* ----------------------------------------------------------------------------
 * When a leader punches.
 */
void leader_fsm::punch(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_PUNCHING);
}


/* ----------------------------------------------------------------------------
 * Queues the stopping of the plucking session, for after this pluck's end.
 */
void leader_fsm::queue_stop_auto_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    l_ptr->queued_pluck_cancel = true;
}


/* ----------------------------------------------------------------------------
 * When a leader gently releases the held mob.
 */
void leader_fsm::release(mob* m, void* info1, void* info2) {
    if(m->holding.empty()) return;
    m->release(m->holding[0]);
}


/* ----------------------------------------------------------------------------
 * When a leader dismisses the group.
 */
void leader_fsm::dismiss(mob* m, void* info1, void* info2) {
    ((leader*) m)->dismiss();
}


/* ----------------------------------------------------------------------------
 * When a leader uses a spray.
 * info1: Pointer to a size_t with the spray's ID number.
 */
void leader_fsm::spray(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    size_t spray_nr = *((size_t*) info1);
    
    if(spray_amounts[spray_nr] == 0) return;
    
    float shoot_angle =
        cursor_angle + ((spray_types[spray_nr].angle) ? TAU / 2 : 0);
        
    unordered_set<mob*> affected_mobs;
    if(spray_types[spray_nr].group) {
        for(size_t gm = 0; gm < m->group->members.size(); ++gm) {
            if(
                m->group->members[gm]->type->category->id ==
                MOB_CATEGORY_PIKMIN
            ) {
                affected_mobs.insert(m->group->members[gm]);
            }
        }
        //If there is nothing to get sprayed, better not waste it.
        if(affected_mobs.empty()) return;
        
    } else {
        for(size_t am = 0; am < mobs.size(); ++am) {
            mob* am_ptr = mobs[am];
            if(am_ptr == m) continue;
            
            if(
                dist(m->pos, am_ptr->pos) >
                spray_types[spray_nr].distance_range + am_ptr->type->radius
            ) {
                continue;
            }
            
            float angle_dif =
                get_angle_smallest_dif(
                    shoot_angle,
                    get_angle(m->pos, am_ptr->pos)
                );
            if(angle_dif > spray_types[spray_nr].angle_range / 2) continue;
            
            affected_mobs.insert(am_ptr);
        }
        
    }
    
    for(auto am = affected_mobs.begin(); am != affected_mobs.end(); ++am) {
        (*am)->fsm.run_event(
            MOB_EVENT_TOUCHED_SPRAY, (void*) &spray_types[spray_nr]
        );
    }
    
    particle p(
        PARTICLE_TYPE_BITMAP, m->pos, m->z + m->type->height,
        52, 3.5, PARTICLE_PRIORITY_MEDIUM
    );
    p.bitmap = bmp_smoke;
    p.friction = 1;
    p.color = spray_types[spray_nr].main_color;
    particle_generator pg(0, p, 32);
    pg.angle = shoot_angle;
    pg.angle_deviation = spray_types[spray_nr].angle_range / 2.0f;
    pg.total_speed = spray_types[spray_nr].distance_range * 0.8;
    pg.total_speed_deviation = spray_types[spray_nr].distance_range * 0.4;
    pg.size_deviation = 0.5;
    pg.emit(particles);
    
    spray_amounts[spray_nr]--;
    
    m->set_animation(LEADER_ANIM_SPRAYING);
}


/* ----------------------------------------------------------------------------
 * When a leader leaves a hazardous sector.
 * info1: Points to the hazard.
 */
void leader_fsm::left_hazard(mob* m, void* info1, void* info2) {
    hazard* h = (hazard*) info1;
    if(h->associated_liquid) {
        m->remove_particle_generator(MOB_PARTICLE_GENERATOR_WAVE_RING);
    }
}


/* ----------------------------------------------------------------------------
 * When a leader loses health.
 * info1: Pointer to the hitbox touch information structure.
 * info2: If not NULL, that means this leader is inactive.
 */
void leader_fsm::be_attacked(mob* m, void* info1, void* info2) {
    if(m->invuln_period.time_left > 0.0f) return;
    m->invuln_period.start();
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    
    if(!info->mob2->attack(m, info->h2, info->h1, NULL)) return;
    
    float knockback = 0;
    float knockback_angle = 0;
    calculate_knockback(
        info->mob2, m, info->h2, info->h1, &knockback, &knockback_angle
    );
    m->apply_knockback(knockback, knockback_angle);
    
    //If info2 has a value, then this leader is inactive.
    if(knockback > 0) {
        if(info2)
            m->fsm.set_state(LEADER_STATE_INACTIVE_KNOCKED_BACK);
        else
            m->fsm.set_state(LEADER_STATE_KNOCKED_BACK);
    } else {
        if(info2)
            m->fsm.set_state(LEADER_STATE_INACTIVE_PAIN);
        else
            m->fsm.set_state(LEADER_STATE_PAIN);
    }
}


/* ----------------------------------------------------------------------------
 * When a leader should lose his momentum and stand still.
 */
void leader_fsm::lose_momentum(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->speed.x = m->speed.y = 0;
}


/* ----------------------------------------------------------------------------
 * When an inactive leader loses health.
 * info1: Pointer to the hitbox touch information structure.
 */
void leader_fsm::inactive_be_attacked(mob* m, void* info1, void* info2) {
    int a = 0;
    leader_fsm::be_attacked(m, info1, &a);
    //We need to send the function a value so it knows
    //it's an inactive leader.
}


/* ----------------------------------------------------------------------------
 * When a leader dies.
 */
void leader_fsm::die(mob* m, void* info1, void* info2) {
    //TODO TEMP.
    size_t living_leaders = 0;
    for(size_t l = 0; l < leaders.size(); ++l) {
        if(leaders[l]->health > 0) living_leaders++;
    }
    if(living_leaders == 0) {
        fade_mgr.start_fade(
            false,
        []() {
            ((gameplay*) game_states[GAME_STATE_GAME])->leave();
        }
        );
    } else if(cur_leader_ptr == m) {
        change_to_next_leader(true, true);
    }
    
    m->stop_chasing();
    m->become_uncarriable();
    m->set_animation(LEADER_ANIM_LYING);
}


/* ----------------------------------------------------------------------------
 * When a leader reels back in pain from being hurt.
 */
void leader_fsm::suffer_pain(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_PAIN);
    m->stop_chasing();
}


/* ----------------------------------------------------------------------------
 * When a leader lies down from being knocked back.
 */
void leader_fsm::get_knocked_back(mob* m, void* info1, void* info2) {
    m->leave_group();
    m->set_animation(LEADER_ANIM_KNOCKED_DOWN);
}


/* ----------------------------------------------------------------------------
 * When a leader falls asleep.
 */
void leader_fsm::fall_asleep(mob* m, void* info1, void* info2) {
    leader_fsm::dismiss(m, NULL, NULL);
    m->stop_chasing();
    
    m->become_carriable(false);
    
    m->set_animation(LEADER_ANIM_LYING);
}


/* ----------------------------------------------------------------------------
 * When a leader wakes up.
 */
void leader_fsm::start_waking_up(mob* m, void* info1, void* info2) {
    m->become_uncarriable();
    m->set_animation(LEADER_ANIM_GETTING_UP);
}


/* ----------------------------------------------------------------------------
 * When a leader must chase another.
 */
void leader_fsm::chase_leader(mob* m, void* info1, void* info2) {
    group_info* leader_group_ptr = m->following_group->group;
    float distance =
        m->following_group->type->radius +
        m->type->radius + standard_pikmin_radius;
        
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
 * When a leader is no longer in the thrown state.
 */
void leader_fsm::stop_being_thrown(mob* m, void* info1, void* info2) {
    //Remove the throw particle generator.
    m->remove_particle_generator(MOB_PARTICLE_GENERATOR_THROW);
}


/* ----------------------------------------------------------------------------
 * When a leader stands still while in another's group.
 */
void leader_fsm::stop_in_group(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader's leader dismisses them.
 */
void leader_fsm::be_dismissed(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader heads towards a Pikmin with the intent to pluck it.
 * Also signals other leaders in the group to search for other seeds.
 * info1: Pointer to the Pikmin to be plucked.
 */
void leader_fsm::go_pluck(mob* m, void* info1, void* info2) {
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
    for(size_t l = 0; l < leaders.size(); ++l) {
        if(leaders[l]->following_group == lea_ptr) {
            leaders[l]->auto_plucking = true;
            leaders[l]->fsm.run_event(LEADER_EVENT_INACTIVE_SEARCH_SEED);
        }
    }
    
    lea_ptr->set_animation(LEADER_ANIM_WALKING);
}


/* ----------------------------------------------------------------------------
 * When a leader grabs on to a sprout and begins plucking it out.
 */
void leader_fsm::start_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    l_ptr->pluck_target->fsm.run_event(MOB_EVENT_PLUCKED, (void*) l_ptr);
    l_ptr->pluck_target->pluck_reserved = false;
    l_ptr->pluck_target = nullptr;
    l_ptr->set_animation(LEADER_ANIM_PLUCKING);
}


/* ----------------------------------------------------------------------------
 * When a leader quits the auto-plucking mindset.
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
 * When the leader must signal to their follower leaders to stop plucking.
 */
void leader_fsm::signal_stop_auto_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    for(size_t l = 0; l < leaders.size(); ++l) {
        if(leaders[l]->following_group == l_ptr) {
            leaders[l]->fsm.run_event(LEADER_EVENT_CANCEL);
        }
    }
}


/* ----------------------------------------------------------------------------
 * When the leader finishes the animation of the current pluck.
 */
void leader_fsm::finish_current_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    l_ptr->stop_chasing();
    l_ptr->set_animation(LEADER_ANIM_IDLING);
}

/* ----------------------------------------------------------------------------
 * When a leader searches for a seed next to them.
 * If found, issues events to go towards the seed.
 * info1: If not NULL, this leader is inactive.
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
    
    if(!info1) {
        //Active leader.
        l_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    } else {
        //Inactive leader.
        if(l_ptr->following_group)
            l_ptr->fsm.set_state(LEADER_STATE_IN_GROUP_CHASING);
        else
            l_ptr->fsm.set_state(LEADER_STATE_IDLING);
    }
    
    if(new_pikmin && d <= next_pluck_range) {
        l_ptr->fsm.run_event(LEADER_EVENT_GO_PLUCK, (void*) new_pikmin);
        l_ptr->queued_pluck_cancel = false;
    } else {
        leader_fsm::stop_auto_pluck(m, NULL, NULL);
    }
}


/* ----------------------------------------------------------------------------
 * When an inactive leader searches for a seed next to them.
 * This just calls search_seed().
 */
void leader_fsm::inactive_search_seed(mob* m, void* info1, void* info2) {
    int a = 0; //Dummy value.
    leader_fsm::search_seed(m, &a, NULL);
}


/* ----------------------------------------------------------------------------
 * When a leader is grabbed by another leader.
 */
void leader_fsm::be_grabbed_by_friend(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a leader grabbed by another is released.
 */
void leader_fsm::be_released(mob* m, void* info1, void* info2) {

}


/* ----------------------------------------------------------------------------
 * When a leader grabbed by another is thrown.
 */
void leader_fsm::be_thrown(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    
    particle throw_p(
        PARTICLE_TYPE_CIRCLE, m->pos, m->z,
        m->type->radius, 0.6, PARTICLE_PRIORITY_LOW
    );
    throw_p.size_grow_speed = -5;
    throw_p.color = change_alpha(m->type->main_color, 128);
    particle_generator pg(THROW_PARTICLE_INTERVAL, throw_p, 1);
    pg.follow_mob = m;
    pg.id = MOB_PARTICLE_GENERATOR_THROW;
    m->particle_generators.push_back(pg);
}


/* ----------------------------------------------------------------------------
 * When a thrown leader lands.
 */
void leader_fsm::land(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->speed.x = m->speed.y = 0;
    
    m->remove_particle_generator(MOB_PARTICLE_GENERATOR_THROW);
}
