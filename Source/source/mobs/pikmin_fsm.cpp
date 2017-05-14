/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin finite state machine logic.
 */

#include "../functions.h"
#include "../hazard.h"
#include "mob_fsm.h"
#include "pikmin.h"
#include "pikmin_fsm.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the Pikmin's logic.
 */
void pikmin_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("buried", PIKMIN_STATE_BURIED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::become_buried);
        }
        efc.new_event(MOB_EVENT_PLUCKED); {
            efc.run(pikmin_fsm::begin_pluck);
            efc.change_state("plucking");
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(pikmin_fsm::stand_still);
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
            efc.run(pikmin_fsm::chase_leader);
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
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
    
    efc.new_state("in_group_stopped", PIKMIN_STATE_IN_GROUP_STOPPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(pikmin_fsm::face_leader);
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
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
            efc.run(pikmin_fsm::chase_leader);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(pikmin_fsm::set_idle_task_reach);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(pikmin_fsm::chase_leader);
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
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
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
    
    efc.new_state("group_move_stopped", PIKMIN_STATE_GROUP_MOVE_STOPPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::set_group_move_reach);
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(pikmin_fsm::face_leader);
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
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
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
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::land_on_mob);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
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
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
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
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_FOCUS_OFF_REACH); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
        efc.new_event(MOB_EVENT_FRAME_SIGNAL); {
            efc.run(pikmin_fsm::do_grounded_attack);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run(pikmin_fsm::rechase_opponent);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
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
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    
    efc.new_state("attacking_latched", PIKMIN_STATE_ATTACKING_LATCHED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pikmin_fsm::prepare_to_attack);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(pikmin_fsm::tick_latched);
        }
        efc.new_event(MOB_EVENT_FRAME_SIGNAL); {
            efc.run(pikmin_fsm::do_latched_attack);
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
            efc.run(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
        efc.new_event(MOB_EVENT_RELEASED); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run(pikmin_fsm::tick_grabbed_by_enemy);
        }
    }
    
    efc.new_state("knocked_back", PIKMIN_STATE_KNOCKED_BACK); {
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run(pikmin_fsm::stand_still);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
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
    
    efc.new_state("celebrating", PIKMIN_STATE_CELEBRATING); {
    }
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    assert(typ->states.size() == N_PIKMIN_STATES);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin becomes buried.
 */
void pikmin_fsm::become_buried(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_BURIED);
    m->unpushable = true;
}


/* ----------------------------------------------------------------------------
 * Makes a Pikmin begin its plucking process.
 * info1: Pointer to the leader that is plucking.
 */
void pikmin_fsm::begin_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    mob* lea = (mob*) info1;
    
    if(lea->following_group) {
        //If this leader is following another one,
        //then the new Pikmin should be in the group of that top leader.
        lea = lea->following_group;
    }
    add_to_group(lea, pik);
    
    pik->set_animation(PIKMIN_ANIM_PLUCKING);
    m->unpushable = false;
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
 * info1: Pointer to the mob.
 * info2: Pointer to the hitbox that grabbed.
 */
void pikmin_fsm::be_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* mob_ptr = (mob*) info1;
    hitbox* h_ptr = (hitbox*) info2;
    
    pik_ptr->set_connected_hitbox_info(h_ptr, mob_ptr);
    
    pik_ptr->focused_mob = mob_ptr;
    
    sfx_pikmin_caught.play(0.2, 0);
    pik_ptr->set_animation(PIKMIN_ANIM_IDLING);
    remove_from_group(pik_ptr);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is dismissed by its leader.
 * info1: Pointer to the world coordinates to go to.
 */
void pikmin_fsm::be_dismissed(mob* m, void* info1, void* info2) {
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
    m->set_animation(PIKMIN_ANIM_IDLING);
    pikmin_fsm::stand_still(m, NULL, NULL);
    remove_from_group(m);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin becomes idling.
 */
void pikmin_fsm::become_idle(mob* m, void* info1, void* info2) {
    pikmin_fsm::stand_still(m, info1, info2);
    m->set_animation(PIKMIN_ANIM_IDLING);
    unfocus_mob(m);
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
        PARTICLE_TYPE_CIRCLE, m->pos,
        m->type->radius, 0.6, PARTICLE_PRIORITY_LOW
    );
    throw_p.size_grow_speed = -5;
    throw_p.color = change_alpha(m->type->main_color, 128);
    particle_generator pg(THROW_PARTICLE_INTERVAL, throw_p, 1);
    pg.follow = &m->pos;
    pg.id = MOB_PARTICLE_GENERATOR_THROW;
    m->particle_generators.push_back(pg);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is gently released by a leader.
 */
void pikmin_fsm::be_released(mob* m, void* info1, void* info2) {
}


/* ----------------------------------------------------------------------------
 * When a Pikmin notifies the leader that it must gently release it.
 */
void pikmin_fsm::notify_leader_release(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = ((pikmin*) m);
    if(!pik_ptr->following_group) return;
    if(((leader*) (pik_ptr->following_group))->holding_pikmin != m) return;
    pik_ptr->following_group->fsm.run_event(LEADER_EVENT_RELEASE);
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands.
 */
void pikmin_fsm::land(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_IDLING);
    
    pikmin_fsm::stand_still(m, NULL, NULL);
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
    
    add_to_group(cur_leader_ptr, pik);
    sfx_pikmin_called.play(0.03, false);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is hit by an attack and gets knocked back.
 * info1: Pointer to the hitbox touch information structure.
 */
void pikmin_fsm::get_knocked_down(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    float knockback = 0;
    float knockback_angle = 0;
    
    calculate_knockback(
        info->mob2, m, info->h2, info->h1, &knockback, &knockback_angle
    );
    apply_knockback(m, knockback, knockback_angle);
    
    m->set_animation(PIKMIN_ANIM_LYING);
    
    remove_from_group(m);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to walk towards an opponent.
 * info1: Pointer to the opponent.
 */
void pikmin_fsm::go_to_opponent(mob* m, void* info1, void* info2) {
    mob* o_ptr = (mob*) info1;
    if(o_ptr->type->category->id == MOB_CATEGORY_ENEMIES) {
        if(!((enemy*) info1)->ene_type->allow_ground_attacks) return;
    }
    
    focus_mob(m, (mob*) info1);
    m->stop_chasing();
    m->chase(
        point(),
        &m->focused_mob->pos,
        false, nullptr, false,
        m->focused_mob->type->radius + m->type->radius + GROUNDED_ATTACK_DIST
    );
    m->set_animation(PIKMIN_ANIM_WALKING);
    remove_from_group(m);
    
    m->fsm.set_state(PIKMIN_STATE_GOING_TO_OPPONENT);
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
 * When a Pikmin needs to go towards its spot on a carriable object.
 * info1: Pointer to the mob to carry.
 */
void pikmin_fsm::go_to_carriable_object(mob* m, void* info1, void* info2) {
    mob* carriable_mob = (mob*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    pik_ptr->carrying_mob = carriable_mob;
    pik_ptr->stop_chasing();
    
    size_t closest_spot = INVALID;
    dist closest_spot_dist;
    carrier_spot_struct* closest_spot_ptr = NULL;
    
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
    
    pik_ptr->focused_mob = carriable_mob;
    pik_ptr->carrying_spot = closest_spot;
    closest_spot_ptr->state = CARRY_SPOT_RESERVED;
    closest_spot_ptr->pik_ptr = pik_ptr;
    
    pik_ptr->chase(
        closest_spot_ptr->pos, &carriable_mob->pos,
        false, nullptr, false,
        pik_ptr->type->radius * 1.2
    );
    pik_ptr->set_animation(PIKMIN_ANIM_WALKING);
    remove_from_group(pik_ptr);
    
    pik_ptr->set_timer(PIKMIN_GOTO_TIMEOUT);
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
    
    pik_ptr->face(get_angle(final_pos, carriable_mob->pos));
    
    pik_ptr->set_animation(PIKMIN_ANIM_CARRYING);
    
    //Let the carriable mob know that a new Pikmin has grabbed on.
    pik_ptr->carrying_mob->fsm.run_event(
        MOB_EVENT_CARRIER_ADDED, (void*) pik_ptr
    );
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to drop the object it's carrying, or
 * stop chasing the object if it's not carrying it yet, but wants to.
 */
void pikmin_fsm::forget_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    
    p->carrying_mob->carry_info->spot_info[p->carrying_spot].state =
        CARRY_SPOT_FREE;
    p->carrying_mob->carry_info->spot_info[p->carrying_spot].pik_ptr =
        NULL;
        
    p->carrying_mob = NULL;
    p->set_timer(0);
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
    pikmin* p = (pikmin*) m;
    p->set_animation(PIKMIN_ANIM_ATTACKING);
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands on a mob, to latch on to it.
 * info1: Pointer to the hitbox touch information structure.
 */
void pikmin_fsm::land_on_mob(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    
    mob* mob_ptr = info->mob2;
    hitbox* h_ptr = info->h2;
    
    if(!h_ptr || !h_ptr->can_pikmin_latch) {
        //No good for latching on. Make it act like it landed on the ground.
        m->fsm.run_event(MOB_EVENT_LANDED);
        return;
    }
    
    pik_ptr->connected_hitbox_nr = h_ptr->body_part_index;
    pik_ptr->speed.x = pik_ptr->speed.y = pik_ptr->speed_z = 0;
    
    pik_ptr->focused_mob = mob_ptr;
    pik_ptr->set_connected_hitbox_info(h_ptr, mob_ptr);
    
    pik_ptr->was_thrown = false;
    
    pik_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_LATCHED);
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin leaves a hazardous sector.
 * info1: Points to the hazard.
 */
void pikmin_fsm::left_hazard(mob* m, void* info1, void* info2) {
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
 * When a frame has passed while the Pikmin is being grabbed by an enemy.
 */
void pikmin_fsm::tick_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->focused_mob) return;
    
    pik_ptr->teleport_to_connected_hitbox();
}


/* ----------------------------------------------------------------------------
 * When a frame has passed while the Pikmin is latched on to an enemy.
 */
void pikmin_fsm::tick_latched(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->focused_mob) return;
    
    pik_ptr->teleport_to_connected_hitbox();
}


/* ----------------------------------------------------------------------------
 * When a frame has passed while a Pikmin is attacking on the ground.
 */
void pikmin_fsm::tick_attacking_grounded(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    if(!pik_ptr->focused_mob || pik_ptr->focused_mob->dead) {
        return;
    }
    
    pik_ptr->face(get_angle(pik_ptr->pos, pik_ptr->focused_mob->pos));
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to turn towards its leader.
 */
void pikmin_fsm::face_leader(mob* m, void* info1, void* info2) {
    m->face(get_angle(m->pos, m->following_group->pos));
}


/* ----------------------------------------------------------------------------
 * When a Pikmin falls down a bottomless pit.
 */
void pikmin_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->health = 0;
    m->dead = true;
}


/* ----------------------------------------------------------------------------
 * Makes the Pikmin do the actual attack in the grounded attacking animation,
 * if possible.
 * info1: Points to the signal received by the frame of the attack animation.
 */
void pikmin_fsm::do_grounded_attack(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(
        !(
            (
                p->focused_mob->z > p->z + p->type->height
            ) ||
            (
                p->focused_mob->z + p->focused_mob->type->height <
                p->z
            )
        )
    ) {
        p->do_attack(
            p->focused_mob,
            get_closest_hitbox(
                p->pos,
                p->focused_mob,
                HITBOX_TYPE_NORMAL
            )
        );
    }
}


/* ----------------------------------------------------------------------------
 * Makes the Pikmin do the actual attack in the latched attacking animation,
 * if possible.
 * info1: Points to the signal received by the frame of the attack animation.
 */
void pikmin_fsm::do_latched_attack(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    p->do_attack(
        p->focused_mob,
        get_hitbox(
            p->focused_mob, p->connected_hitbox_nr
        )
    );
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to chase after its leader (or the group spot
 * belonging to the leader).
 * info1: Points to the position struct with the final destination.
 *   If NULL, the final destination is calculated here.
 */
void pikmin_fsm::chase_leader(mob* m, void* info1, void* info2) {
    pikmin_fsm::update_in_group_chasing(m, info1, info2);
    focus_mob(m, m->following_group);
    m->set_animation(PIKMIN_ANIM_WALKING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts flailing.
 */
void pikmin_fsm::start_flailing(mob* m, void* info1, void* info2) {
    //If the Pikmin is following a moveable point, let's change it to
    //a static point. This will make the Pikmin continue to move
    //forward into the water in a straight line.
    point final_pos = m->get_chase_target();
    m->chase(final_pos, NULL, false);
    
    remove_from_group(m);
    
    //Let the Pikmin continue to swim into the water for a bit
    //before coming to a stop. Otherwise the Pikmin would stop nearly
    //on the edge of the water, and that just looks bad.
    m->set_timer(1.0f);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin touches an enemy's eat hitbox, but first has
 * to check if it is edible, since it's in the special "disabled" state.
 * info1: Points to the hitbox.
 */
void pikmin_fsm::check_disabled_edible(mob* m, void* info1, void* info2) {
    if(m->disabled_state_flags & DISABLED_STATE_FLAG_INEDIBLE) return;
    pikmin_fsm::be_grabbed_by_enemy(m, info1, info2);
    m->fsm.set_state(PIKMIN_STATE_GRABBED_BY_ENEMY, info1, info2);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin checks if it's no longer meant to be flailing.
 * info1: Points to the hazard that the Pikmin left.
 */
void pikmin_fsm::check_remove_flailing(mob* m, void* info1, void* info2) {
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
 * When the Pikmin must move towards the whistle.
 */
void pikmin_fsm::flail_to_whistle(mob* m, void* info1, void* info2) {
    m->chase(cur_leader_ptr->pos, NULL, false, NULL, true);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts panicking.
 */
void pikmin_fsm::start_panicking(mob* m, void* info1, void* info2) {
    remove_from_group(m);
    pikmin_fsm::panic_new_chase(m, info1, info2);
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
}


/* ----------------------------------------------------------------------------
 * When a Pikmin touches a hazard.
 * info1: Pointer to the hazard type.
 */
void pikmin_fsm::touched_hazard(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    hazard* h = (hazard*) info1;
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
                PARTICLE_TYPE_BITMAP, m->pos,
                0, 1, PARTICLE_PRIORITY_LOW
            );
            p.bitmap = bmp_wave_ring;
            p.size_grow_speed = m->type->radius * 4;
            p.before_mobs = true;
            particle_generator pg(0.3, p, 1);
            pg.follow = &m->pos;
            pg.id = MOB_PARTICLE_GENERATOR_WAVE_RING;
            m->particle_generators.push_back(pg);
        }
    }
    
    for(size_t r = 0; r < p->pik_type->resistances.size(); ++r) {
        if(p->pik_type->resistances[r] == h) return; //Immune!
    }
    if(p->invuln_period.time_left > 0) return;
    
    for(size_t e = 0; e < h->effects.size(); ++e) {
        p->apply_status_effect(h->effects[e], false);
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is sprayed.
 * info1: Pointer to the spray type.
 */
void pikmin_fsm::touched_spray(mob* m, void* info1, void* info2) {
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); ++e) {
        m->apply_status_effect(s->effects[e], false);
    }
}


/* ----------------------------------------------------------------------------
 * When the Pikmin should try to latch on whilst grounded.
 * If it fails, it just tries a grounded attack.
 */
void pikmin_fsm::try_latching(mob* m, void* info1, void* info2) {
    pikmin* p_ptr = (pikmin*) m;
    dist d;
    hitbox* closest_h = NULL;
    
    p_ptr->stop_chasing();
    
    if(!p_ptr->focused_mob->type->is_obstacle) {
        closest_h =
            get_closest_hitbox(
                p_ptr->pos, p_ptr->focused_mob,
                HITBOX_TYPE_NORMAL, &d
            );
    }
    
    if(
        !closest_h || !closest_h->can_pikmin_latch ||
        d >= closest_h->radius + p_ptr->type->radius
    ) {
        //Can't latch. Let's just do a grounded attack instead.
        p_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_GROUNDED);
        
    } else {
        //Go for a latch.
        hitbox_touch_info hti(p_ptr->focused_mob, NULL, closest_h);
        pikmin_fsm::land_on_mob(m, &hti, NULL);
        
    }
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
