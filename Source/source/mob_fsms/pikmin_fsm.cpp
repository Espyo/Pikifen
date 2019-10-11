/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin finite state machine logic.
 */

#include "pikmin_fsm.h"

#include "../functions.h"
#include "../hazard.h"
#include "../mobs/pikmin.h"
#include "../utils/string_utils.h"
#include "../vars.h"
#include "gen_mob_fsm.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the Pikmin's logic.
 */
void pikmin_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("seed", PIKMIN_STATE_SEED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::become_sprout);
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(pikmin_fsm::seed_landed);
            efc.run(pikmin_fsm::stand_still);
            efc.change_state("sprout");
        }
    }
    
    efc.new_state("sprout", PIKMIN_STATE_SPROUT); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::become_sprout);
            efc.run(pikmin_fsm::sprout_schedule_evol);
        }
        efc.new_event(MOB_EVENT_PLUCKED); {
            efc.run(pikmin_fsm::begin_pluck);
            efc.change_state("plucking");
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(pikmin_fsm::sprout_evolve);
            efc.run(pikmin_fsm::sprout_schedule_evol);
        }
    }
    
    efc.new_state("plucking", PIKMIN_STATE_PLUCKING); {
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run(pikmin_fsm::end_pluck);
            efc.change_state("in_group_chasing");
        }
    }
    
    efc.new_state("in_group_chasing", PIKMIN_STATE_IN_GROUP_CHASING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::start_chasing_leader);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EVENT_SPOT_IS_FAR); {
            efc.run(pikmin_fsm::update_in_group_chasing);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("in_group_stopped");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_STARTED); {
            efc.change_state("group_move_chasing");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("in_group_stopped", PIKMIN_STATE_IN_GROUP_STOPPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EVENT_SPOT_IS_FAR); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_STARTED); {
            efc.change_state("group_move_chasing");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("group_move_chasing", PIKMIN_STATE_GROUP_MOVE_CHASING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::set_group_move_reach);
            efc.run(pikmin_fsm::start_chasing_leader);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::set_idle_task_reach);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(pikmin_fsm::update_in_group_chasing);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("group_move_stopped");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_ENDED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EVENT_NEAR_TOOL); {
            efc.run(pikmin_fsm::go_to_tool);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("group_move_stopped", PIKMIN_STATE_GROUP_MOVE_STOPPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::set_group_move_reach);
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::set_idle_task_reach);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("group_move_stopped");
        }
        efc.new_event(MOB_EVENT_SPOT_IS_FAR); {
            efc.change_state("group_move_chasing");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_ENDED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EVENT_NEAR_TOOL); {
            efc.run(pikmin_fsm::go_to_tool);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("grabbed_by_leader", PIKMIN_STATE_GRABBED_BY_LEADER); {
        efc.new_event(MOB_EVENT_THROWN); {
            efc.run(pikmin_fsm::be_thrown);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EVENT_RELEASED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("thrown", PIKMIN_STATE_THROWN); {
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_being_thrown);
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(pikmin_fsm::land);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N); {
            efc.run(pikmin_fsm::land_on_mob);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_dismiss_spot", PIKMIN_STATE_GOING_TO_DISMISS_SPOT
    ); {
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::going_to_dismiss_spot);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::clear_timer);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EVENT_NEAR_TOOL); {
            efc.run(pikmin_fsm::go_to_tool);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("idling", PIKMIN_STATE_IDLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::become_idle);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_being_idle);
        }
        efc.new_event(MOB_EVENT_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EVENT_NEAR_TOOL); {
            efc.run(pikmin_fsm::go_to_tool);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("going_to_opponent", PIKMIN_STATE_GOING_TO_OPPONENT); {
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::try_latching);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_FOCUS_OFF_REACH); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_FOCUS_DIED); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_carriable_object", PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT
    ); {
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reach_carriable_object);
            efc.change_state("carrying");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.change_state("sighing");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_tool", PIKMIN_STATE_GOING_TO_TOOL
    ); {
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("picking_up");
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(pikmin_fsm::forget_tool);
            efc.change_state("sighing");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::forget_tool);
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::forget_tool);
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::forget_tool);
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::forget_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("sighing", PIKMIN_STATE_SIGHING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::stand_still);
            efc.run(pikmin_fsm::sigh);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
    }
    
    efc.new_state("carrying", PIKMIN_STATE_CARRYING); {
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_carrying);
            efc.run(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_FINISHED_CARRYING); {
            efc.run(pikmin_fsm::finish_carrying);
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("picking_up", PIKMIN_STATE_PICKING_UP); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::start_picking_up);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run(pikmin_fsm::finish_picking_up);
            efc.change_state("idling_h");
        }
    }
    
    efc.new_state("returning", PIKMIN_STATE_RETURNING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::start_returning);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("attacking_grounded", PIKMIN_STATE_ATTACKING_GROUNDED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::prepare_to_attack);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(pikmin_fsm::tick_attacking_grounded);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run(pikmin_fsm::rechase_opponent);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
    }
    
    efc.new_state("attacking_latched", PIKMIN_STATE_ATTACKING_LATCHED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::prepare_to_attack);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::unlatch);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_FOCUS_DIED); {
            efc.run(pikmin_fsm::lose_latched_mob);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("grabbed_by_enemy", PIKMIN_STATE_GRABBED_BY_ENEMY); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
        }
        efc.new_event(MOB_EVENT_RELEASED); {
            efc.run(pikmin_fsm::be_released);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("knocked_back", PIKMIN_STATE_KNOCKED_BACK); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::get_knocked_back);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run(pikmin_fsm::get_up);
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("disabled", PIKMIN_STATE_DISABLED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::become_disabled);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::remove_disabled);
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::check_disabled_edible);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("flailing", PIKMIN_STATE_FLAILING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::start_flailing);
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
            efc.run(pikmin_fsm::check_remove_flailing);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::flail_to_whistle);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("panicking", PIKMIN_STATE_PANICKING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::start_panicking);
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(pikmin_fsm::panic_new_chase);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::remove_panic);
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("drinking", PIKMIN_STATE_DRINKING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::start_drinking);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::finish_drinking);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
    }
    
    efc.new_state("celebrating", PIKMIN_STATE_CELEBRATING); {
    }
    
    efc.new_state("in_group_chasing_h", PIKMIN_STATE_IN_GROUP_CHASING_H); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::start_chasing_leader);
        }
        efc.new_event(MOB_EVENT_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader_h");
        }
        efc.new_event(MOB_EVENT_SPOT_IS_FAR); {
            efc.run(pikmin_fsm::update_in_group_chasing);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("in_group_stopped_h");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_STARTED); {
            efc.change_state("group_move_chasing_h");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot_h");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("in_group_stopped_h", PIKMIN_STATE_IN_GROUP_STOPPED_H); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EVENT_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("in_group_stopped");
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader_h");
        }
        efc.new_event(MOB_EVENT_SPOT_IS_FAR); {
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_STARTED); {
            efc.change_state("group_move_chasing_h");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot_h");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("group_move_chasing_h", PIKMIN_STATE_GROUP_MOVE_CHASING_H); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::set_group_move_reach);
            efc.run(pikmin_fsm::start_chasing_leader);
        }
        efc.new_event(MOB_EVENT_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("group_move_chasing");
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::set_idle_task_reach);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(pikmin_fsm::update_in_group_chasing);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader_h");
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("group_move_stopped_h");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_ENDED); {
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot_h");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("group_move_stopped_h", PIKMIN_STATE_GROUP_MOVE_STOPPED_H); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::set_group_move_reach);
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EVENT_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("group_move_stopped");
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::set_idle_task_reach);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader_h");
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("group_move_stopped_h");
        }
        efc.new_event(MOB_EVENT_SPOT_IS_FAR); {
            efc.change_state("group_move_chasing_h");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_ENDED); {
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot_h");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("grabbed_by_leader_h", PIKMIN_STATE_GRABBED_BY_LEADER_H); {
        efc.new_event(MOB_EVENT_THROWN); {
            efc.run(pikmin_fsm::be_thrown);
            efc.change_state("thrown_h");
        }
        efc.new_event(MOB_EVENT_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EVENT_RELEASED); {
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("thrown_h", PIKMIN_STATE_THROWN_H); {
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_being_thrown);
        }
        efc.new_event(MOB_EVENT_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(pikmin_fsm::land_while_holding);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N); {
            efc.run(pikmin_fsm::land_on_mob_while_holding);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_dismiss_spot_h", PIKMIN_STATE_GOING_TO_DISMISS_SPOT_H
    ); {
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EVENT_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::going_to_dismiss_spot);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::clear_timer);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.change_state("idling_h");
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.change_state("idling_h");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("idling_h", PIKMIN_STATE_IDLING_H); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::become_idle);
        }
        efc.new_event(MOB_EVENT_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_being_idle);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called_while_holding);
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EVENT_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::be_attacked);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EVENT_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EVENT_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EVENT_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_PIKMIN_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_PIKMIN_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When a Pikmin becomes a seed or a sprout.
 */
void pikmin_fsm::become_sprout(mob* m, void* info1, void* info2) {
    m->leave_group();
    m->set_animation(PIKMIN_ANIM_SPROUT);
    m->unpushable = true;
    m->is_huntable = false;
    ((pikmin*) m)->is_seed_or_sprout = true;
}


/* ----------------------------------------------------------------------------
 * Makes a Pikmin begin its plucking process.
 * info1: Pointer to the leader that is plucking.
 */
void pikmin_fsm::begin_pluck(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    pikmin* pik = (pikmin*) m;
    mob* lea = (mob*) info1;
    
    if(lea->following_group) {
        //If this leader is following another one,
        //then the new Pikmin should be in the group of that top leader.
        lea = lea->following_group;
    }
    lea->add_to_group(pik);
    
    pik->set_animation(PIKMIN_ANIM_PLUCKING);
    m->is_huntable = true;
    m->unpushable = false;
    pik->is_seed_or_sprout = false;
    m->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * Causes a sprout to evolve.
 */
void pikmin_fsm::sprout_evolve(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(p->maturity == 0 || p->maturity == 1) {
        //Leaf to bud, or bud to flower.
        
        p->maturity++;
        
        //Generate a burst of particles to symbolize the maturation.
        particle pa(
            PARTICLE_TYPE_BITMAP, m->pos, m->z + m->type->height,
            16, 1, PARTICLE_PRIORITY_LOW
        );
        pa.bitmap = bmp_sparkle;
        pa.color = al_map_rgb(255, 255, 255);
        particle_generator pg(0, pa, 8);
        pg.number_deviation = 1;
        pg.size_deviation = 8;
        pg.angle = 0;
        pg.angle_deviation = TAU / 2;
        pg.total_speed = 40;
        pg.total_speed_deviation = 10;
        pg.duration_deviation = 0.25;
        pg.emit(particles);
        //TODO play a sound.
        
    } else {
        //Flower to leaf.
        
        p->maturity = 0;
        
        //Generate a dribble of particles to symbolize the regression.
        particle pa(
            PARTICLE_TYPE_BITMAP, m->pos, m->z + m->type->height,
            16, 1, PARTICLE_PRIORITY_LOW
        );
        pa.bitmap = bmp_sparkle;
        pa.color = al_map_rgb(255, 224, 224);
        pa.gravity = 300;
        particle_generator pg(0, pa, 8);
        pg.number_deviation = 1;
        pg.size_deviation = 8;
        pg.angle = 0;
        pg.angle_deviation = TAU / 2;
        pg.total_speed = 50;
        pg.total_speed_deviation = 10;
        pg.duration_deviation = 0.25;
        pg.emit(particles);
        //TODO play a sound.
    }
}


/* ----------------------------------------------------------------------------
 * Schedules the next evolution for a sprout.
 */
void pikmin_fsm::sprout_schedule_evol(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    m->set_timer(p->pik_type->sprout_evolution_time[p->maturity]);
}


/* ----------------------------------------------------------------------------
 * Makes a Pikmin finish its plucking process.
 */
void pikmin_fsm::end_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    pik->set_animation(PIKMIN_ANIM_IDLING);
    sfx_pikmin_plucked.play(0, false);
    sfx_pluck.play(0, false);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is grabbed by a leader.
 */
void pikmin_fsm::be_grabbed_by_friend(mob* m, void* info1, void* info2) {
    sfx_pikmin_held.play(0, false);
    m->set_animation(PIKMIN_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is grabbed by an enemy.
 * info1: Pointer to the enemy.
 * info2: Pointer to the hitbox that grabbed.
 */
void pikmin_fsm::be_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    engine_assert(info2 != NULL, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    mob* ene_ptr = (mob*) info1;
    hitbox* h_ptr = (hitbox*) info2;
    
    ene_ptr->chomp(pik_ptr, h_ptr);
    
    sfx_pikmin_caught.play(0.2, 0);
    pik_ptr->set_animation(PIKMIN_ANIM_IDLING);
    pik_ptr->leave_group();
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is dismissed by its leader.
 * info1: Pointer to the world coordinates to go to.
 */
void pikmin_fsm::be_dismissed(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    m->chase(
        *((point*) info1),
        NULL,
        false
    );
    sfx_pikmin_idle.play(0, false);
    
    m->set_animation(PIKMIN_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin reaches its dismissal spot.
 */
void pikmin_fsm::reach_dismiss_spot(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin becomes "disabled".
 */
void pikmin_fsm::become_disabled(mob* m, void* info1, void* info2) {
    pikmin_fsm::release_tool(m, info1, info2);
    pikmin_fsm::notify_leader_release(m, info1, info2);
    m->set_animation(PIKMIN_ANIM_IDLING);
    pikmin_fsm::stand_still(m, NULL, NULL);
    m->leave_group();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin becomes idling.
 */
void pikmin_fsm::become_idle(mob* m, void* info1, void* info2) {
    pikmin_fsm::stand_still(m, info1, info2);
    m->set_animation(PIKMIN_ANIM_IDLING);
    m->unfocus_from_mob();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is thrown by a leader.
 */
void pikmin_fsm::be_thrown(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    sfx_pikmin_held.stop();
    sfx_pikmin_thrown.stop();
    sfx_pikmin_thrown.play(0, false);
    m->set_animation(PIKMIN_ANIM_THROWN);
    
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
 * When a Pikmin is gently released by a leader or enemy.
 */
void pikmin_fsm::be_released(mob* m, void* info1, void* info2) {

}


/* ----------------------------------------------------------------------------
 * When a Pikmin notifies the leader that it must gently release it.
 */
void pikmin_fsm::notify_leader_release(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = ((pikmin*) m);
    if(!pik_ptr->following_group) return;
    if(pik_ptr->holder.m != pik_ptr->following_group) return;
    pik_ptr->following_group->fsm.run_event(MOB_EVENT_RELEASE_ORDER);
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands.
 */
void pikmin_fsm::land(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_IDLING);
    
    pikmin_fsm::stand_still(m, NULL, NULL);
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands while holding something.
 * Depending on what it is, it might drop it.
 */
void pikmin_fsm::land_while_holding(mob* m, void* info1, void* info2) {
    engine_assert(!m->holding.empty(), m->print_state_history());
    
    tool* too_ptr = (tool*) * (m->holding.begin());
    
    m->set_animation(PIKMIN_ANIM_IDLING);
    
    pikmin_fsm::stand_still(m, NULL, NULL);
    
    ((pikmin*) m)->is_tool_primed_for_whistle = true;
    
    if(too_ptr->too_type->dropped_when_pikmin_lands) {
        pikmin_fsm::release_tool(m, info1, info2);
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        
        if(too_ptr->too_type->pikmin_returns_after_using) {
            pikmin_fsm::called(m, NULL, NULL);
            m->fsm.set_state(PIKMIN_STATE_IN_GROUP_CHASING);
        }
    } else {
        m->fsm.set_state(PIKMIN_STATE_IDLING_H);
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to release the tool it is currently holding.
 */
void pikmin_fsm::release_tool(mob* m, void* info1, void* info2) {
    if(m->holding.empty()) return;
    pikmin* p_ptr = (pikmin*) m;
    mob* t_ptr = *m->holding.begin();
    
    m->release(t_ptr);
    t_ptr->pos = m->pos;
    t_ptr->speed = point();
    t_ptr->push_amount = 0.0f;
    m->subgroup_type_ptr =
        subgroup_types.get_type(SUBGROUP_TYPE_CATEGORY_PIKMIN, p_ptr->pik_type);
    if(m->following_group) {
        m->following_group->group->change_standby_type_if_needed();
        update_closest_group_member();
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to stop being disabled.
 */
void pikmin_fsm::remove_disabled(mob* m, void* info1, void* info2) {
    m->invuln_period.start();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to stop panicking.
 */
void pikmin_fsm::remove_panic(mob* m, void* info1, void* info2) {
    m->invuln_period.start();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin seed lands on the ground.
 */
void pikmin_fsm::seed_landed(mob* m, void* info1, void* info2) {
    //Generate the rock particles that come out.
    particle pa(
        PARTICLE_TYPE_BITMAP, m->pos, m->z + m->type->height,
        4, 1, PARTICLE_PRIORITY_LOW
    );
    pa.bitmap = bmp_rock;
    pa.color = al_map_rgb(160, 80, 32);
    pa.gravity = 50;
    particle_generator pg(0, pa, 8);
    pg.number_deviation = 1;
    pg.size_deviation = 2;
    pg.angle = 0;
    pg.angle_deviation = TAU / 2;
    pg.total_speed = 50;
    pg.total_speed_deviation = 10;
    pg.duration_deviation = 0.25;
    pg.emit(particles);
    //TODO play a sound.
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to to change "reach" to the group move reach.
 */
void pikmin_fsm::set_group_move_reach(mob* m, void* info1, void* info2) {
    m->near_reach = 1;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to to change "reach" to the idle task reach.
 */
void pikmin_fsm::set_idle_task_reach(mob* m, void* info1, void* info2) {
    m->near_reach = 0;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to sigh.
 */
void pikmin_fsm::sigh(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_SIGHING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to stand still in place.
 */
void pikmin_fsm::stand_still(mob* m, void* info1, void* info2) {
    m->stop_circling();
    m->stop_following_path();
    m->stop_chasing();
    m->speed.x = m->speed.y = 0;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is called over by a leader, either by being whistled,
 * or touched when idling.
 */
void pikmin_fsm::called(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    
    for(size_t s = 0; s < m->statuses.size(); ++s) {
        if(m->statuses[s].type->removable_with_whistle) {
            m->statuses[s].to_delete = true;
        }
    }
    m->delete_old_status_effects();
    
    cur_leader_ptr->add_to_group(pik);
    sfx_pikmin_called.play(0.03, false);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is called over by a leader, either by being whistled,
 * or touched when idling, but while the Pikmin is holding a tool.
 */
void pikmin_fsm::called_while_holding(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    tool* too_ptr = (tool*) * (m->holding.begin());
    
    if(
        too_ptr->too_type->dropped_when_pikmin_is_whistled &&
        pik_ptr->is_tool_primed_for_whistle &&
        whistling
    ) {
        //Since this event can be called when the Pikmin is bumped, we must add
        //a check to only release the tool if it's a real whistle. Checking
        //if the leader is whistling is a roundabout way... but it works.
        pikmin_fsm::release_tool(m, info1, info2);
    }
    
    pik_ptr->is_tool_primed_for_whistle = false;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to walk towards an opponent.
 * info1: Pointer to the opponent.
 */
void pikmin_fsm::go_to_opponent(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    mob* o_ptr = (mob*) info1;
    if(o_ptr->type->category->id == MOB_CATEGORY_ENEMIES) {
        enemy* e_ptr = (enemy*) info1;
        if(!e_ptr->ene_type->allow_ground_attacks) return;
        if(e_ptr->z > m->z + m->type->height) return;
    }
    
    m->focus_on_mob((mob*) info1);
    m->stop_chasing();
    m->chase(
        point(),
        &m->focused_mob->pos,
        false, nullptr, false,
        m->focused_mob->type->radius + m->type->radius + GROUNDED_ATTACK_DIST
    );
    m->set_animation(PIKMIN_ANIM_WALKING);
    m->leave_group();
    
    m->fsm.set_state(PIKMIN_STATE_GOING_TO_OPPONENT);
}


const float PIKMIN_DISMISS_TIMEOUT = 4.0f;

/* ----------------------------------------------------------------------------
 * When a Pikmin needs to get going to its dismiss spot.
 */
void pikmin_fsm::going_to_dismiss_spot(mob* m, void* info1, void* info2) {
    m->set_timer(PIKMIN_DISMISS_TIMEOUT);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin that just attacked an opponent needs to walk
 * towards it again.
 */
void pikmin_fsm::rechase_opponent(mob* m, void* info1, void* info2) {
    if(
        m->focused_mob &&
        m->focused_mob->health > 0 &&
        dist(m->pos, m->focused_mob->pos) <=
        (m->type->radius + m->focused_mob->type->radius + GROUNDED_ATTACK_DIST)
    ) {
        return;
    }
    
    m->fsm.set_state(PIKMIN_STATE_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is hit by an attack and gets knocked back.
 * info1: Pointer to the hitbox touch information structure.
 */
void pikmin_fsm::be_attacked(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    pikmin* p_ptr = (pikmin*) m;
    
    if(p_ptr->invuln_period.time_left > 0) return;
    
    if(!p_ptr->process_attack_miss(info)) {
        //It has been decided that this attack missed.
        return;
    }
    
    if(!info->mob2->attack(p_ptr, info->h2, info->h1, NULL)) {
        return;
    }
    
    float knockback = 0;
    float knockback_angle = 0;
    calculate_knockback(
        info->mob2, m, info->h2, info->h1, &knockback, &knockback_angle
    );
    m->apply_knockback(knockback, knockback_angle);
    
    //Withering.
    if(info->h2->wither_chance > 0 && p_ptr->maturity > 0) {
        unsigned char wither_roll = randomi(0, 100);
        if(wither_roll < info->h2->wither_chance) {
            p_ptr->increase_maturity(-1);
        }
    }
    
    m->leave_group();
    
    pikmin_fsm::be_released(m, info1, info2);
    pikmin_fsm::notify_leader_release(m, info1, info2);
    pikmin_fsm::release_tool(m, info1, info2);
    m->fsm.set_state(PIKMIN_STATE_KNOCKED_BACK);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin gets knocked back.
 */
void pikmin_fsm::get_knocked_back(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_LYING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin gets up from being knocked down.
 */
void pikmin_fsm::get_up(mob* m, void* info1, void* info2) {
    mob* prev_focused_mob = m->focused_mob;
    m->fsm.set_state(PIKMIN_STATE_IDLING);
    if(
        prev_focused_mob &&
        m->can_hunt(prev_focused_mob)
    ) {
        m->fsm.run_event(
            MOB_EVENT_OPPONENT_IN_REACH, (void*) prev_focused_mob, NULL
        );
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to go towards its spot on a carriable object.
 * info1: Pointer to the mob to carry.
 */
void pikmin_fsm::go_to_carriable_object(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    mob* carriable_mob = (mob*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    pik_ptr->carrying_mob = carriable_mob;
    pik_ptr->leave_group();
    pik_ptr->stop_chasing();
    
    size_t closest_spot = INVALID;
    dist closest_spot_dist;
    carrier_spot_struct* closest_spot_ptr = NULL;
    
    //If this is the first Pikmin to go to the carriable mob, rotate
    //the points such that 0 faces this Pikmin instead.
    if(carriable_mob->carry_info->is_empty()) {
        carriable_mob->carry_info->rotate_points(
            get_angle(carriable_mob->pos, pik_ptr->pos)
        );
    }
    
    for(size_t s = 0; s < carriable_mob->type->max_carriers; ++s) {
        carrier_spot_struct* s_ptr = &carriable_mob->carry_info->spot_info[s];
        if(s_ptr->state != CARRY_SPOT_FREE) continue;
        
        dist d(
            pik_ptr->pos, carriable_mob->pos + s_ptr->pos
        );
        if(closest_spot == INVALID || d < closest_spot_dist) {
            closest_spot = s;
            closest_spot_dist = d;
            closest_spot_ptr = s_ptr;
        }
    }
    
    pik_ptr->focus_on_mob(carriable_mob);
    pik_ptr->carrying_spot = closest_spot;
    closest_spot_ptr->state = CARRY_SPOT_RESERVED;
    closest_spot_ptr->pik_ptr = pik_ptr;
    
    pik_ptr->chase(
        closest_spot_ptr->pos, &carriable_mob->pos,
        false, nullptr, false
    );
    pik_ptr->set_animation(PIKMIN_ANIM_WALKING);
    
    pik_ptr->set_timer(PIKMIN_GOTO_TIMEOUT);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to go towards a tool mob.
 * info1: Pointer to the tool.
 */
void pikmin_fsm::go_to_tool(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    tool* too_ptr = (tool*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    if(too_ptr->reserved && too_ptr->reserved != pik_ptr) {
        //Another Pikmin is already going for it. Ignore it.
        return;
    }
    if(!pik_ptr->pik_type->can_carry_tools) {
        //This Pikmin can't carry tools. Forget it.
        return;
    }
    if(!(too_ptr->holdability_flags & HOLDABLE_BY_PIKMIN)) {
        //Can't hold this. Forget it.
        return;
    }
    
    too_ptr->reserved = pik_ptr;
    
    pik_ptr->leave_group();
    pik_ptr->stop_chasing();
    
    m->focus_on_mob(too_ptr);
    
    m->chase(
        point(), &too_ptr->pos,
        false, nullptr, false,
        pik_ptr->type->radius + too_ptr->type->radius
    );
    
    pik_ptr->set_animation(PIKMIN_ANIM_WALKING);
    pik_ptr->set_timer(PIKMIN_GOTO_TIMEOUT);
    pik_ptr->fsm.set_state(PIKMIN_STATE_GOING_TO_TOOL);
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin reaches its spot on a carriable object.
 */
void pikmin_fsm::reach_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* carriable_mob = pik_ptr->carrying_mob;
    
    pik_ptr->set_animation(PIKMIN_ANIM_GRABBING, true);
    
    point final_pos =
        carriable_mob->pos +
        carriable_mob->carry_info->spot_info[pik_ptr->carrying_spot].pos;
        
    pik_ptr->chase(
        carriable_mob->carry_info->spot_info[pik_ptr->carrying_spot].pos,
        &carriable_mob->pos,
        true, &carriable_mob->z
    );
    
    pik_ptr->face(get_angle(final_pos, carriable_mob->pos), NULL);
    
    pik_ptr->set_animation(PIKMIN_ANIM_CARRYING);
    
    //Let the carriable mob know that a new Pikmin has grabbed on.
    pik_ptr->carrying_mob->fsm.run_event(
        MOB_EVENT_CARRIER_ADDED, (void*) pik_ptr
    );
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin finishes drinking the drop it was drinking.
 */
void pikmin_fsm::finish_drinking(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    pikmin* p_ptr = (pikmin*) m;
    drop* d_ptr = (drop*) m->focused_mob;
    
    if(d_ptr->dro_type->effect == DROP_EFFECT_MATURATE) {
        p_ptr->increase_maturity(d_ptr->dro_type->increase_amount);
        
    } else if(d_ptr->dro_type->effect == DROP_EFFECT_GIVE_STATUS) {
        p_ptr->apply_status_effect(
            d_ptr->dro_type->status_to_give, true, false
        );
    }
    
    m->unfocus_from_mob();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin successfully finishes carrying an object.
 */
void pikmin_fsm::finish_carrying(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    engine_assert(p->carrying_mob != NULL, m->print_state_history());
    
    if(p->carrying_mob->carry_info->must_return) {
        //The Pikmin should return somewhere (like a pile).
        p->fsm.set_state(PIKMIN_STATE_RETURNING, (void*) p->carrying_mob);
        
    } else {
        //The Pikmin can just sit and chill.
        p->fsm.set_state(PIKMIN_STATE_IDLING);
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to drop the object it's carrying, or
 * stop chasing the object if it's not carrying it yet, but wants to.
 */
void pikmin_fsm::forget_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    engine_assert(p->carrying_mob != NULL, m->print_state_history());
    
    p->carrying_mob->carry_info->spot_info[p->carrying_spot].state =
        CARRY_SPOT_FREE;
    p->carrying_mob->carry_info->spot_info[p->carrying_spot].pik_ptr =
        NULL;
        
    p->carrying_mob = NULL;
    p->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to forget a tool object it was going for.
 */
void pikmin_fsm::forget_tool(mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    tool* too_ptr = (tool*) (m->focused_mob);
    too_ptr->reserved = NULL;
    m->unfocus_from_mob();
    m->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to release an object it is carrying.
 */
void pikmin_fsm::stop_carrying(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(!p->carrying_mob) return;
    
    p->carrying_mob->fsm.run_event(MOB_EVENT_CARRIER_REMOVED, (void*) p);
    
    p->carrying_mob = NULL;
    p->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to decide a new spot to run off to whilst
 * in panicking.
 */
void pikmin_fsm::panic_new_chase(mob* m, void* info1, void* info2) {
    m->chase(
        point(
            m->pos.x + randomf(-1000, 1000),
            m->pos.y + randomf(-1000, 1000)
        ),
        NULL, false
    );
    m->set_timer(PIKMIN_PANIC_CHASE_INTERVAL);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to reel back to unleash an attack.
 */
void pikmin_fsm::prepare_to_attack(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    pikmin* p = (pikmin*) m;
    p->set_animation(PIKMIN_ANIM_ATTACKING);
    p->face(0, &p->focused_mob->pos);
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands on a mob, to latch on to it.
 * info1: Pointer to the hitbox touch information structure.
 */
void pikmin_fsm::land_on_mob(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    hitbox_interaction* info = (hitbox_interaction*) info1;
    mob* mob_ptr = info->mob2;
    
    if(!m->can_hurt(mob_ptr)) return;
    
    hitbox* h_ptr = info->h2;
    
    if(!h_ptr || !h_ptr->can_pikmin_latch) {
        //No good for latching on. Make it bounce back.
        m->speed.x *= -0.3;
        m->speed.y *= -0.3;
        return;
    }
    
    pik_ptr->speed.x = pik_ptr->speed.y = pik_ptr->speed_z = 0;
    pik_ptr->stop_height_effect();
    
    pik_ptr->focused_mob = mob_ptr;
    
    float h_offset_dist;
    float h_offset_angle;
    mob_ptr->get_hitbox_hold_point(
        pik_ptr, h_ptr, &h_offset_dist, &h_offset_angle
    );
    mob_ptr->hold(
        pik_ptr, h_ptr->body_part_index, h_offset_dist, h_offset_angle, true
    );
    
    pik_ptr->was_thrown = false;
    pik_ptr->latched = true;
    
    pik_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_LATCHED);
    
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands on a mob, whilst holding something.
 * info1: Pointer to the hitbox touch information structure.
 */
void pikmin_fsm::land_on_mob_while_holding(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    engine_assert(!m->holding.empty(), m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    hitbox_interaction* info = (hitbox_interaction*) info1;
    tool* too_ptr = (tool*) (*m->holding.begin());
    
    if(!m->can_hurt(info->mob2)) return;
    
    pik_ptr->was_thrown = false;
    
    if(too_ptr->too_type->dropped_when_pikmin_lands_on_opponent) {
        pikmin_fsm::release_tool(m, info1, info2);
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        
        if(too_ptr->too_type->stuck_when_pikmin_lands_on_opponent && info->h2) {
            too_ptr->speed.x = too_ptr->speed.y = too_ptr->speed_z = 0;
            too_ptr->stop_height_effect();
            
            too_ptr->focused_mob = info->mob2;
            
            float h_offset_dist;
            float h_offset_angle;
            info->mob2->get_hitbox_hold_point(
                too_ptr, info->h2, &h_offset_dist, &h_offset_angle
            );
            info->mob2->hold(
                too_ptr, info->h2->body_part_index,
                h_offset_dist, h_offset_angle, true
            );
        }
        
        if(too_ptr->too_type->pikmin_returns_after_using) {
            pikmin_fsm::called(m, NULL, NULL);
            m->fsm.set_state(PIKMIN_STATE_IN_GROUP_CHASING);
        }
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin leaves a hazardous sector.
 * info1: Points to the hazard.
 */
void pikmin_fsm::left_hazard(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hazard* h = (hazard*) info1;
    if(h->associated_liquid) {
        m->remove_particle_generator(MOB_PARTICLE_GENERATOR_WAVE_RING);
    }
}


/* ----------------------------------------------------------------------------
 * When the mob the Pikmin is latched on to disappears.
 */
void pikmin_fsm::lose_latched_mob(mob* m, void* info1, void* info2) {
    m->stop_chasing();
}


/* ----------------------------------------------------------------------------
 * When a frame has passed while a Pikmin is attacking on the ground.
 */
void pikmin_fsm::tick_attacking_grounded(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    if(!pik_ptr->focused_mob || pik_ptr->focused_mob->health <= 0) {
        return;
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin falls down a bottomless pit.
 */
void pikmin_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->set_health(false, false, 0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to start chasing after its leader (or the group spot
 * belonging to the leader).
 * info1: Points to the position struct with the final destination.
 *   If NULL, the final destination is calculated here.
 */
void pikmin_fsm::start_chasing_leader(mob* m, void* info1, void* info2) {
    m->focus_on_mob(m->following_group);
    m->set_animation(PIKMIN_ANIM_WALKING);
    pikmin_fsm::update_in_group_chasing(m, NULL, NULL);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts flailing.
 */
void pikmin_fsm::start_flailing(mob* m, void* info1, void* info2) {
    pikmin_fsm::release_tool(m, info1, info2);
    
    //If the Pikmin is following a moveable point, let's change it to
    //a static point. This will make the Pikmin continue to move
    //forward into the water in a straight line.
    point final_pos = m->get_chase_target();
    m->chase(final_pos, NULL, false);
    
    m->leave_group();
    
    //Let the Pikmin continue to swim into the water for a bit
    //before coming to a stop. Otherwise the Pikmin would stop nearly
    //on the edge of the water, and that just looks bad.
    m->set_timer(1.0f);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts picking some object up to hold it.
 */
void pikmin_fsm::start_picking_up(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_PICKING_UP);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin finishes picking some object up to hold it.
 */
void pikmin_fsm::finish_picking_up(mob* m, void* info1, void* info2) {
    tool* too_ptr = (tool*) (m->focused_mob);
    
    if(!(too_ptr->holdability_flags & HOLDABLE_BY_PIKMIN)) {
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        return;
    }
    
    m->subgroup_type_ptr =
        subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_TOOL, m->focused_mob->type
        );
    m->hold(m->focused_mob, INVALID, 4, 0, true);
    m->unfocus_from_mob();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin touches an enemy's eat hitbox, but first has
 * to check if it is edible, since it's in the special "disabled" state.
 * info1: Points to the hitbox.
 */
void pikmin_fsm::check_disabled_edible(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    if(m->disabled_state_flags & DISABLED_STATE_FLAG_INEDIBLE) return;
    pikmin_fsm::be_grabbed_by_enemy(m, info1, info2);
    m->fsm.set_state(PIKMIN_STATE_GRABBED_BY_ENEMY, info1, info2);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin checks if it's no longer meant to be flailing.
 * info1: Points to the hazard that the Pikmin left.
 */
void pikmin_fsm::check_remove_flailing(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hazard* h_ptr = (hazard*) info1;
    
    for(size_t s = 0; s < m->statuses.size(); ++s) {
        for(size_t e = 0; e < h_ptr->effects.size(); ++e) {
            if(
                m->statuses[s].type == h_ptr->effects[e] &&
                h_ptr->effects[e]->causes_flailing
            ) {
            
                m->statuses[s].to_delete = true;
                m->fsm.set_state(PIKMIN_STATE_IDLING);
                pikmin_fsm::stand_still(m, NULL, NULL);
                
            }
        }
    }
    
    //Let's piggyback this check to also remove liquid wave ring particles.
    if(h_ptr->associated_liquid) {
        m->remove_particle_generator(MOB_PARTICLE_GENERATOR_WAVE_RING);
    }
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin has to clear any timer set.
 */
void pikmin_fsm::clear_timer(mob* m, void* info1, void* info2) {
    m->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When the Pikmin must move towards the whistle.
 */
void pikmin_fsm::flail_to_whistle(mob* m, void* info1, void* info2) {
    m->chase(cur_leader_ptr->pos, NULL, false, NULL, true);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts drinking the drop it touched.
 * info1: Pointer to the drop mob.
 */
void pikmin_fsm::start_drinking(mob* m, void* info1, void* info2) {
    mob* drop_ptr = (mob*) info1;
    m->leave_group();
    m->stop_chasing();
    m->focus_on_mob(drop_ptr);
    m->set_animation(PIKMIN_ANIM_DRINKING);
    m->face(get_angle(m->pos, drop_ptr->pos), NULL);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts panicking.
 */
void pikmin_fsm::start_panicking(mob* m, void* info1, void* info2) {
    pikmin_fsm::release_tool(m, info1, info2);
    m->set_animation(PIKMIN_ANIM_WALKING);
    m->leave_group();
    pikmin_fsm::panic_new_chase(m, info1, info2);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin must start returning to the carried object's return point.
 * info1: Pointer to the mob that used to be carried.
 */
void pikmin_fsm::start_returning(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    pikmin* p_ptr = (pikmin*) m;
    mob* carried_mob = (mob*) info1;
    
    if(
        p_ptr->follow_path(
            carried_mob->carry_info->return_point,
            false,
            p_ptr->get_base_speed(),
            carried_mob->carry_info->return_dist
        )
    ) {
        p_ptr->set_animation(PIKMIN_ANIM_WALKING);
    } else {
        p_ptr->fsm.set_state(PIKMIN_STATE_IDLING);
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin must no longer be idling.
 */
void pikmin_fsm::stop_being_idle(mob* m, void* info1, void* info2) {

}


/* ----------------------------------------------------------------------------
 * When a Pikmin is no longer in the thrown state.
 */
void pikmin_fsm::stop_being_thrown(mob* m, void* info1, void* info2) {
    m->remove_particle_generator(MOB_PARTICLE_GENERATOR_THROW);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin stands still while in a leader's group.
 */
void pikmin_fsm::stop_in_group(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_IDLING);
    m->face(0, &m->following_group->pos);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin touches a "eat" hitbox.
 */
void pikmin_fsm::touched_eat_hitbox(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    engine_assert(info2 != NULL, m->print_state_history());
    
    if(m->invuln_period.time_left > 0) return;
    
    m->fsm.set_state(PIKMIN_STATE_GRABBED_BY_ENEMY, info1, info2);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin touches a hazard.
 * info1: Pointer to the hazard type.
 * info2: Pointer to the hitbox that caused this, if any.
 */
void pikmin_fsm::touched_hazard(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    pikmin* p = (pikmin*) m;
    hazard* h = (hazard*) info1;
    
    if(info2) {
        //This is an attack.
        hitbox_interaction* h_info = (hitbox_interaction*) info2;
        if(!p->process_attack_miss(h_info)) {
            //It has been decided that this attack missed.
            return;
        }
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
    
    if(p->get_hazard_vulnerability(h) == 0.0f) return;
    if(p->invuln_period.time_left > 0) return;
    
    for(size_t e = 0; e < h->effects.size(); ++e) {
        p->apply_status_effect(h->effects[e], false, false);
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is sprayed.
 * info1: Pointer to the spray type.
 */
void pikmin_fsm::touched_spray(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); ++e) {
        m->apply_status_effect(s->effects[e], false, false);
    }
    
    if(s->buries_pikmin) {
        //TODO make sure this only buries opposing Pikmin.
        m->fsm.set_state(PIKMIN_STATE_SPROUT, NULL, NULL);
    }
}


/* ----------------------------------------------------------------------------
 * When the Pikmin gets grabbed by an enemy. It should try to swap places
 * with the object that it is holding, instead, if possible.
 * If not, it should drop the object and get grabbed like normal.
 */
void pikmin_fsm::try_held_item_hotswap(mob* m, void* info1, void* info2) {
    assert(!m->holding.empty());
    
    tool* too_ptr = (tool*) * (m->holding.begin());
    if(
        !too_ptr->too_type->can_be_hotswapped &&
        (too_ptr->holdability_flags & HOLDABLE_BY_ENEMIES)
    ) {
        //This tool can't be hotswapped... The Pikmin has to get chomped.
        pikmin_fsm::release_tool(m, info1, info2);
        m->fsm.set_state(PIKMIN_STATE_GRABBED_BY_ENEMY);
        return;
    }
    
    //Start by dropping the tool.
    pikmin_fsm::release_tool(m, info1, info2);
    //Receive some invulnerability period to make sure it's not hurt by
    //the same attack.
    m->invuln_period.start();
    //Finally, get knocked back on purpose.
    m->leave_group();
    pikmin_fsm::be_released(m, info1, info2);
    pikmin_fsm::notify_leader_release(m, info1, info2);
    m->fsm.set_state(PIKMIN_STATE_KNOCKED_BACK);
}


/* ----------------------------------------------------------------------------
 * When the Pikmin should try to latch on whilst grounded.
 * If it fails, it just tries a grounded attack.
 */
void pikmin_fsm::try_latching(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    pikmin* p_ptr = (pikmin*) m;
    
    p_ptr->stop_chasing();
    
    dist d;
    hitbox* closest_h =
        p_ptr->focused_mob->get_closest_hitbox(
            p_ptr->pos, HITBOX_TYPE_NORMAL, &d
        );
    float h_z = 0;
    
    if(closest_h) {
        h_z = closest_h->z + p_ptr->focused_mob->z;
    }
    
    if(
        !closest_h || !closest_h->can_pikmin_latch ||
        h_z > p_ptr->z + p_ptr->type->height ||
        h_z + closest_h->height < p_ptr->z ||
        d >= closest_h->radius + p_ptr->type->radius
    ) {
        //Can't latch. Let's just do a grounded attack instead.
        p_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_GROUNDED);
        
    } else {
        //Go for a latch.
        hitbox_interaction hti(p_ptr->focused_mob, NULL, closest_h);
        pikmin_fsm::land_on_mob(m, &hti, NULL);
        
    }
}


/* ----------------------------------------------------------------------------
 * When the Pikmin stops latching on to an enemy.
 */
void pikmin_fsm::unlatch(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    m->focused_mob->release(m);
    ((pikmin*) m)->latched = false;
}


/* ----------------------------------------------------------------------------
 * When the Pikmin should update its destination when chasing the leader.
 * info1: Points to the position struct with the final destination.
 *   If NULL, the final destination is calculated here.
 */
void pikmin_fsm::update_in_group_chasing(mob* m, void* info1, void* info2) {
    point pos;
    
    if(!info1) {
        pos =
            m->following_group->group->anchor +
            m->following_group->group->get_spot_offset(m->group_spot_index);
    } else {
        pos = *((point*) info1);
    }
    
    m->chase(pos, NULL, false);
    
}
