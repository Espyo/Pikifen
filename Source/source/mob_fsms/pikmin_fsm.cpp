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
#include "../game.h"
#include "../hazard.h"
#include "../mobs/drop.h"
#include "../mobs/group_task.h"
#include "../mobs/pikmin.h"
#include "../mobs/tool.h"
#include "../mobs/track.h"
#include "../utils/string_utils.h"
#include "gen_mob_fsm.h"


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the Pikmin's logic.
 * typ:
 *   Mob type to create the finite state machine for.
 */
void pikmin_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("seed", PIKMIN_STATE_SEED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::become_sprout);
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::seed_landed);
            efc.run(pikmin_fsm::stand_still);
            efc.change_state("sprout");
        }
    }
    
    efc.new_state("sprout", PIKMIN_STATE_SPROUT); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::become_sprout);
            efc.run(pikmin_fsm::sprout_schedule_evol);
        }
        efc.new_event(MOB_EV_PLUCKED); {
            efc.run(pikmin_fsm::begin_pluck);
            efc.change_state("plucking");
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::sprout_evolve);
            efc.run(pikmin_fsm::sprout_schedule_evol);
        }
    }
    
    efc.new_state("plucking", PIKMIN_STATE_PLUCKING); {
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::end_pluck);
            efc.change_state("in_group_chasing");
        }
    }
    
    efc.new_state("leaving_onion", PIKMIN_STATE_LEAVING_ONION); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::leave_onion);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tick_track_ride);
        }
    }
    
    efc.new_state("entering_onion", PIKMIN_STATE_ENTERING_ONION); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::enter_onion);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tick_entering_onion);
        }
    }
    
    efc.new_state("in_group_chasing", PIKMIN_STATE_IN_GROUP_CHASING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_chasing_leader);
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EV_GO_TO_ONION); {
            efc.change_state("going_to_onion");
        }
        efc.new_event(MOB_EV_SPOT_IS_FAR); {
            efc.run(pikmin_fsm::update_in_group_chasing);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("in_group_stopped");
        }
        efc.new_event(MOB_EV_SWARM_STARTED); {
            efc.change_state("swarm_chasing");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("in_group_stopped", PIKMIN_STATE_IN_GROUP_STOPPED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EV_GO_TO_ONION); {
            efc.change_state("going_to_onion");
        }
        efc.new_event(MOB_EV_SPOT_IS_FAR); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_SWARM_STARTED); {
            efc.change_state("swarm_chasing");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("swarm_chasing", PIKMIN_STATE_SWARM_CHASING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::set_swarm_reach);
            efc.run(pikmin_fsm::start_chasing_leader);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::set_idle_task_reach);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::update_in_group_chasing);
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("swarm_stopped");
        }
        efc.new_event(MOB_EV_SWARM_ENDED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EV_NEAR_TOOL); {
            efc.run(pikmin_fsm::go_to_tool);
        }
        efc.new_event(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(pikmin_fsm::go_to_group_task);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EV_TOUCHED_TRACK); {
            efc.change_state("riding_track");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("swarm_stopped", PIKMIN_STATE_SWARM_STOPPED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::set_swarm_reach);
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::set_idle_task_reach);
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("swarm_stopped");
        }
        efc.new_event(MOB_EV_SPOT_IS_FAR); {
            efc.change_state("swarm_chasing");
        }
        efc.new_event(MOB_EV_SWARM_ENDED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EV_NEAR_TOOL); {
            efc.run(pikmin_fsm::go_to_tool);
        }
        efc.new_event(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(pikmin_fsm::go_to_group_task);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("grabbed_by_leader", PIKMIN_STATE_GRABBED_BY_LEADER); {
        efc.new_event(MOB_EV_THROWN); {
            efc.run(pikmin_fsm::be_thrown);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_RELEASED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("thrown", PIKMIN_STATE_THROWN); {
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_being_thrown);
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::land);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::check_outgoing_attack);
            efc.run(pikmin_fsm::land_on_mob);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_N); {
            efc.run(pikmin_fsm::land_on_mob);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::be_thrown_by_bouncer);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_dismiss_spot", PIKMIN_STATE_GOING_TO_DISMISS_SPOT
    ); {
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::going_to_dismiss_spot);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::clear_timer);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EV_NEAR_TOOL); {
            efc.run(pikmin_fsm::go_to_tool);
        }
        efc.new_event(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(pikmin_fsm::go_to_group_task);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EV_TOUCHED_TRACK); {
            efc.change_state("riding_track");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("idling", PIKMIN_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::become_idle);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_being_idle);
        }
        efc.new_event(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EV_NEAR_CARRIABLE_OBJECT); {
            efc.run(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EV_NEAR_TOOL); {
            efc.run(pikmin_fsm::go_to_tool);
        }
        efc.new_event(MOB_EV_NEAR_GROUP_TASK); {
            efc.run(pikmin_fsm::go_to_group_task);
        }
        efc.new_event(MOB_EV_TOUCHED_TRACK); {
            efc.change_state("riding_track");
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_TOUCHED_DROP); {
            efc.change_state("drinking");
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("going_to_opponent", PIKMIN_STATE_GOING_TO_OPPONENT); {
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::attack_reached_opponent);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_FOCUS_OFF_REACH); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_FOCUS_DIED); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_carriable_object", PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT
    ); {
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reach_carriable_object);
            efc.change_state("carrying");
        }
        efc.new_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.change_state("sighing");
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::forget_carriable_object);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_tool", PIKMIN_STATE_GOING_TO_TOOL
    ); {
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("picking_up");
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::forget_tool);
            efc.change_state("sighing");
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::forget_tool);
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::forget_tool);
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::forget_tool);
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::forget_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_group_task", PIKMIN_STATE_GOING_TO_GROUP_TASK
    ); {
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("on_group_task");
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::forget_group_task);
            efc.change_state("sighing");
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::forget_group_task);
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.run(pikmin_fsm::forget_group_task);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::forget_group_task);
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::forget_group_task);
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::forget_group_task);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_onion", PIKMIN_STATE_GOING_TO_ONION
    ); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::go_to_onion);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("entering_onion");
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("sighing", PIKMIN_STATE_SIGHING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::stand_still);
            efc.run(pikmin_fsm::sigh);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
    }
    
    efc.new_state("carrying", PIKMIN_STATE_CARRYING); {
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_carrying);
            efc.run(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_FINISHED_CARRYING); {
            efc.run(pikmin_fsm::finish_carrying);
        }
        efc.new_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("picking_up", PIKMIN_STATE_PICKING_UP); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_picking_up);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finish_picking_up);
            efc.change_state("idling_h");
        }
    }
    
    efc.new_state("on_group_task", PIKMIN_STATE_ON_GROUP_TASK); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::work_on_group_task);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tick_group_task_work);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::forget_group_task);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.run(pikmin_fsm::forget_group_task);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::check_outgoing_attack);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
    }
    
    efc.new_state("returning", PIKMIN_STATE_RETURNING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_returning);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("attacking_grounded", PIKMIN_STATE_ATTACKING_GROUNDED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::prepare_to_attack);
        }
        efc.new_event(MOB_EV_FOCUS_OFF_REACH); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::rechase_opponent);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::check_outgoing_attack);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
    }
    
    efc.new_state("attacking_latched", PIKMIN_STATE_ATTACKING_LATCHED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::prepare_to_attack);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::unlatch);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_FOCUS_DIED); {
            efc.run(pikmin_fsm::lose_latched_mob);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::check_outgoing_attack);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("grabbed_by_enemy", PIKMIN_STATE_GRABBED_BY_ENEMY); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::be_grabbed_by_enemy);
        }
        efc.new_event(MOB_EV_RELEASED); {
            efc.run(pikmin_fsm::be_released);
            efc.change_state("idling");
        }
    }
    
    efc.new_state("knocked_back", PIKMIN_STATE_KNOCKED_BACK); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::get_knocked_back);
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.change_state("knocked_down");
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("knocked_down", PIKMIN_STATE_KNOCKED_DOWN); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::stand_still);
            efc.run(pikmin_fsm::get_knocked_down);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.change_state("getting_up");
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called_while_knocked_down);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("getting_up", PIKMIN_STATE_GETTING_UP); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_getting_up);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finish_getting_up);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called_while_knocked_down);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("impact_bounce", PIKMIN_STATE_IMPACT_BOUNCE); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::setup_impact_bounce);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::do_impact_bounce);
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::land_after_impact_bounce);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("impact_lunge", PIKMIN_STATE_IMPACT_LUNGE); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_impact_lunge);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::stand_still);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::stand_still);
            efc.run(pikmin_fsm::check_outgoing_attack);
            efc.run(pikmin_fsm::land_on_mob);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("helpless", PIKMIN_STATE_HELPLESS); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::stand_still);
            efc.run(pikmin_fsm::become_helpless);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
        
        //The logic to lose helplessness is in
        //pikmin::handle_status_effect_loss();
    }
    
    efc.new_state("flailing", PIKMIN_STATE_FLAILING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::start_flailing);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::flail_to_whistle);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
        
        //The logic to stop flailing is in
        //pikmin::handle_status_effect_loss();
    }
    
    efc.new_state("panicking", PIKMIN_STATE_PANICKING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::start_panicking);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::panic_new_chase);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
        }
        
        //The logic to stop panicking
        //is in pikmin::handle_status_effect_loss();
    }
    
    efc.new_state("drinking", PIKMIN_STATE_DRINKING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_drinking);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::finish_drinking);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
    }
    
    efc.new_state("celebrating", PIKMIN_STATE_CELEBRATING); {
    }
    
    efc.new_state("in_group_chasing_h", PIKMIN_STATE_IN_GROUP_CHASING_H); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_chasing_leader);
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_GO_TO_ONION); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("going_to_onion");
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader_h");
        }
        efc.new_event(MOB_EV_SPOT_IS_FAR); {
            efc.run(pikmin_fsm::update_in_group_chasing);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("in_group_stopped_h");
        }
        efc.new_event(MOB_EV_SWARM_STARTED); {
            efc.change_state("swarm_chasing_h");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot_h");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("in_group_stopped_h", PIKMIN_STATE_IN_GROUP_STOPPED_H); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("in_group_stopped");
        }
        efc.new_event(MOB_EV_GO_TO_ONION); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("going_to_onion");
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader_h");
        }
        efc.new_event(MOB_EV_SPOT_IS_FAR); {
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EV_SWARM_STARTED); {
            efc.change_state("swarm_chasing_h");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot_h");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("swarm_chasing_h", PIKMIN_STATE_SWARM_CHASING_H); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::set_swarm_reach);
            efc.run(pikmin_fsm::start_chasing_leader);
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("swarm_chasing");
        }
        efc.new_event(MOB_EV_GO_TO_ONION); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("going_to_onion");
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::set_idle_task_reach);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::update_in_group_chasing);
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader_h");
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("swarm_stopped_h");
        }
        efc.new_event(MOB_EV_SWARM_ENDED); {
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot_h");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("swarm_stopped_h", PIKMIN_STATE_SWARM_STOPPED_H); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::set_swarm_reach);
            efc.run(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("swarm_stopped");
        }
        efc.new_event(MOB_EV_GO_TO_ONION); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("going_to_onion");
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::set_idle_task_reach);
        }
        efc.new_event(MOB_EV_GRABBED_BY_FRIEND); {
            efc.run(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader_h");
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("swarm_stopped_h");
        }
        efc.new_event(MOB_EV_SPOT_IS_FAR); {
            efc.change_state("swarm_chasing_h");
        }
        efc.new_event(MOB_EV_SWARM_ENDED); {
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EV_DISMISSED); {
            efc.run(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot_h");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("grabbed_by_leader_h", PIKMIN_STATE_GRABBED_BY_LEADER_H); {
        efc.new_event(MOB_EV_THROWN); {
            efc.run(pikmin_fsm::be_thrown);
            efc.change_state("thrown_h");
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EV_RELEASED); {
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("thrown_h", PIKMIN_STATE_THROWN_H); {
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_being_thrown);
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::land_while_holding);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::land_on_mob_while_holding);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_N); {
            efc.run(pikmin_fsm::land_on_mob_while_holding);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state(
        "going_to_dismiss_spot_h", PIKMIN_STATE_GOING_TO_DISMISS_SPOT_H
    ); {
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::going_to_dismiss_spot);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::clear_timer);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.change_state("idling_h");
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.change_state("idling_h");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("idling_h", PIKMIN_STATE_IDLING_H); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::become_idle);
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_being_idle);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::whistled_while_holding);
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::called);
            efc.change_state("in_group_chasing_h");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::be_attacked);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::try_held_item_hotswap);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_TOUCHED_SPRAY); {
            efc.run(pikmin_fsm::touched_spray);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("riding_track", PIKMIN_STATE_RIDING_TRACK); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_riding_track);
        }
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tick_track_ride);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::whistled_while_riding);
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
 * When the Pikmin reaches an opponent that it was chasing after,
 * and should now attack it. If it's a Pikmin that can latch, it will try so,
 * and if it fails, it just tries a grounded attack.
 * If it's a Pikmin that attacks via impact, it lunges.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::attack_reached_opponent(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    if(m->invuln_period.time_left > 0) {
        //Don't let the Pikmin attack while invulnerable. Otherwise, this can
        //be exploited to let Pikmin vulnerable to a hazard attack the obstacle
        //emitting said hazard.
        return;
    }
    
    pikmin* p_ptr = (pikmin*) m;
    
    p_ptr->stop_chasing();
    
    switch(p_ptr->pik_type->attack_method) {
    case PIKMIN_ATTACK_LATCH: {
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
            h_z > p_ptr->z + p_ptr->height ||
            h_z + closest_h->height < p_ptr->z ||
            d >= closest_h->radius + p_ptr->type->radius
        ) {
            //Can't latch. Let's just do a grounded attack instead.
            p_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_GROUNDED);
            
        } else {
            //Go for a latch instead.
            hitbox_interaction hti(p_ptr->focused_mob, NULL, closest_h);
            pikmin_fsm::land_on_mob(m, &hti, NULL);
            
        }
        
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {

        p_ptr->fsm.set_state(PIKMIN_STATE_IMPACT_LUNGE);
        
        break;
        
    }
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is hit by an attack and gets knocked back.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the hitbox touch information structure.
 * info2:
 *   Unused.
 */
void pikmin_fsm::be_attacked(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    pikmin* p_ptr = (pikmin*) m;
    
    //Damage.
    float damage = 0;
    info->mob2->calculate_damage(m, info->h2, info->h1, &damage);
    m->apply_attack_damage(info->mob2, info->h2, info->h1, damage);
    
    //Knockback.
    float knockback = 0;
    float knockback_angle = 0;
    info->mob2->calculate_knockback(
        m, info->h2, info->h1, &knockback, &knockback_angle
    );
    m->apply_knockback(knockback, knockback_angle);
    
    //Withering.
    if(info->h2->wither_chance > 0 && p_ptr->maturity > 0) {
        unsigned char wither_roll = randomi(0, 100);
        if(wither_roll < info->h2->wither_chance) {
            p_ptr->increase_maturity(-1);
        }
    }
    
    //Finish up.
    m->leave_group();
    
    m->do_attack_effects(info->mob2, info->h2, info->h1, damage, knockback);
    
    pikmin_fsm::be_released(m, info1, info2);
    pikmin_fsm::notify_leader_release(m, info1, info2);
    pikmin_fsm::release_tool(m, info1, info2);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is dismissed by its leader.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the world coordinates to go to.
 * info2:
 *   Unused.
 */
void pikmin_fsm::be_dismissed(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    m->chase(
        *((point*) info1),
        NULL,
        false
    );
    game.sys_assets.sfx_pikmin_idle.play(0, false);
    
    m->set_animation(PIKMIN_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is grabbed by an enemy.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the enemy.
 * info2:
 *   Pointer to the hitbox that grabbed.
 */
void pikmin_fsm::be_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    engine_assert(info2 != NULL, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    mob* ene_ptr = (mob*) info1;
    hitbox* h_ptr = (hitbox*) info2;
    
    ene_ptr->chomp(pik_ptr, h_ptr);
    
    game.sys_assets.sfx_pikmin_caught.play(0.2, 0);
    pik_ptr->set_animation(PIKMIN_ANIM_IDLING);
    pik_ptr->leave_group();
    pik_ptr->is_grabbed_by_enemy = true;
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is grabbed by a leader.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::be_grabbed_by_friend(mob* m, void* info1, void* info2) {
    game.sys_assets.sfx_pikmin_held.play(0, false);
    m->set_animation(PIKMIN_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is gently released by a leader or enemy.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::be_released(mob* m, void* info1, void* info2) {
    ((pikmin*) m)->is_grabbed_by_enemy = false;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is thrown by a leader.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::be_thrown(mob* m, void* info1, void* info2) {
    game.sys_assets.sfx_pikmin_held.stop();
    game.sys_assets.sfx_pikmin_thrown.stop();
    game.sys_assets.sfx_pikmin_thrown.play(0, false);
    
    m->set_animation(PIKMIN_ANIM_THROWN);
    
    ((pikmin*) m)->start_throw_trail();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is thrown by a bouncer mob.
 * m:
 *   The mob.
 * info1:
 *   Points to the bouncer mob.
 * info2:
 *   Unused.
 */
void pikmin_fsm::be_thrown_by_bouncer(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_THROWN);
    
    ((pikmin*) m)->start_throw_trail();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin becomes "helpless".
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::become_helpless(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_IDLING);
    m->leave_group();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin becomes idling.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::become_idle(mob* m, void* info1, void* info2) {
    pikmin_fsm::stand_still(m, info1, info2);
    m->set_animation(PIKMIN_ANIM_IDLING);
    m->unfocus_from_mob();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin becomes a seed or a sprout.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
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
 * m:
 *   The mob.
 * info1:
 *   Pointer to the leader that is plucking.
 * info2:
 *   Unused.
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
 * When a Pikmin is called over by a leader, either by being whistled,
 * or touched when idling.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the leader that called.
 * info2:
 *   Unused.
 */
void pikmin_fsm::called(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    pikmin* pik = (pikmin*) m;
    leader* caller = (leader*) info1;
    
    pik->was_last_hit_dud = false;
    pik->consecutive_dud_hits = 0;
    
    caller->add_to_group(pik);
    game.sys_assets.sfx_pikmin_called.play(0.03, false);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin that is knocked down is called over by a leader,
 * either by being whistled, or touched when idling.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the leader that called.
 * info2:
 *   Unused.
 */
void pikmin_fsm::called_while_knocked_down(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    pikmin* pik = (pikmin*) m;
    leader* caller = (leader*) info1;
    
    //Let's use the "temp" variable to specify whether or not a leader
    //already whistled it.
    if(pik->temp_i == 1) return;
    
    pik->focus_on_mob(caller);
    
    pik->script_timer.time_left =
        std::max(
            0.01f,
            pik->script_timer.time_left - PIKMIN_KNOCKED_DOWN_WHISTLE_BONUS
        );
        
    pik->temp_i = 1;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin should check the attack it has just received.
 * If the attack is successful, another event is triggered. Otherwise
 * nothing happens.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the hitbox touch information structure.
 * info2:
 *   Unused.
 */
void pikmin_fsm::check_incoming_attack(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    pikmin* p_ptr = (pikmin*) m;
    
    if(p_ptr->invuln_period.time_left > 0) {
        //The Pikmin cannot be attacked right now.
        return;
    }
    
    if(!p_ptr->process_attack_miss(info)) {
        //It has been decided that this attack missed.
        return;
    }
    
    float damage = 0;
    if(!info->mob2->calculate_damage(m, info->h2, info->h1, &damage)) {
        //This attack doesn't cause damage.
        return;
    }
    
    //If we got to this point, then greenlight for the attack.
    m->fsm.run_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED, info1, info2);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin should check the attack it is about to unleash.
 * If it realizes it's doing no damage, it should start considering
 * sighing and giving up.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the opponent.
 * info2:
 *   Unused.
 */
void pikmin_fsm::check_outgoing_attack(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    pikmin* p_ptr = (pikmin*) m;
    
    float damage = 0;
    bool attack_success =
        p_ptr->calculate_damage(info->mob2, info->h1, info->h2, &damage);
    if(damage == 0 || !attack_success) {
    
        p_ptr->was_last_hit_dud = true;
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin has to clear any timer set.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::clear_timer(mob* m, void* info1, void* info2) {
    m->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin has to bounce back from an impact attack.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::do_impact_bounce(mob* m, void* info1, void* info2) {
    pikmin* p_ptr = (pikmin*) m;
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    float impact_angle = get_angle(p_ptr->focused_mob->pos, p_ptr->pos);
    float impact_speed = 200.0f;
    p_ptr->speed =
        angle_to_coordinates(
            impact_angle, impact_speed
        );
    p_ptr->speed_z = 500.0f;
    
    p_ptr->set_animation(PIKMIN_ANIM_KNOCKED_BACK);
}


/* ----------------------------------------------------------------------------
 * Makes a Pikmin finish its plucking process.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::end_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    pik->set_animation(PIKMIN_ANIM_IDLING);
    game.sys_assets.sfx_pikmin_plucked.play(0, false);
    game.sys_assets.sfx_pluck.play(0, false);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin must start climbing up an Onion's leg.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::enter_onion(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    pikmin* p_ptr = (pikmin*) m;
    onion* o_ptr = (onion*) p_ptr->focused_mob;
    
    //Set its data to start climbing.
    p_ptr->set_animation(PIKMIN_ANIM_WALKING); //TODO
    
    vector<size_t> checkpoints;
    checkpoints.push_back((p_ptr->temp_i * 2) + 1);
    checkpoints.push_back(p_ptr->temp_i * 2);
    
    p_ptr->track_info = new track_info_struct(
        o_ptr, checkpoints, o_ptr->oni_type->nest->pikmin_enter_speed
    );
}


/* ----------------------------------------------------------------------------
 * When a Pikmin falls down a bottomless pit.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->set_health(false, false, 0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin successfully finishes carrying an object.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
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
 * When a Pikmin finishes drinking the drop it was drinking.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::finish_drinking(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    pikmin* p_ptr = (pikmin*) m;
    drop* d_ptr = (drop*) m->focused_mob;
    
    switch(d_ptr->dro_type->effect) {
    case DROP_EFFECT_MATURATE: {
        p_ptr->increase_maturity(d_ptr->dro_type->increase_amount);
        break;
    } case DROP_EFFECT_GIVE_STATUS: {
        p_ptr->apply_status_effect(
            d_ptr->dro_type->status_to_give, true, false
        );
        break;
    }
    }
    
    m->unfocus_from_mob();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin finishes picking some object up to hold it.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::finish_picking_up(mob* m, void* info1, void* info2) {
    tool* too_ptr = (tool*) (m->focused_mob);
    
    if(!(too_ptr->holdability_flags & HOLDABLE_BY_PIKMIN)) {
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        return;
    }
    
    m->subgroup_type_ptr =
        game.states.gameplay->subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_TOOL, m->focused_mob->type
        );
    m->hold(m->focused_mob, INVALID, 4, 0, true, true);
    m->unfocus_from_mob();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin finishes getting up from being knocked down.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::finish_getting_up(mob* m, void* info1, void* info2) {
    mob* prev_focused_mob = m->focused_mob;
    
    m->fsm.set_state(PIKMIN_STATE_IDLING);
    
    if(prev_focused_mob) {
        if(
            prev_focused_mob->type->category->id == MOB_CATEGORY_LEADERS &&
            !m->can_hunt(prev_focused_mob)
        ) {
            m->fsm.run_event(MOB_EV_WHISTLED, (void*) prev_focused_mob);
            
        } else if(
            m->can_hunt(prev_focused_mob)
        ) {
            m->fsm.run_event(
                MOB_EV_OPPONENT_IN_REACH, (void*) prev_focused_mob, NULL
            );
            
        }
    }
}


/* ----------------------------------------------------------------------------
 * When the Pikmin must move towards the whistle.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the leader that called.
 * info2:
 *   Unused.
 */
void pikmin_fsm::flail_to_whistle(mob* m, void* info1, void* info2) {
    leader* caller = (leader*) info1;
    m->chase(
        caller->pos, NULL, false, NULL, true
    );
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to drop the object it's carrying, or
 * stop chasing the object if it's not carrying it yet, but wants to.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::forget_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(!p->carrying_mob) return;
    
    p->carrying_mob->carry_info->spot_info[p->temp_i].state =
        CARRY_SPOT_FREE;
    p->carrying_mob->carry_info->spot_info[p->temp_i].pik_ptr =
        NULL;
        
    p->carrying_mob = NULL;
    p->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to forget a group task object it was going for.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::forget_group_task(mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    group_task* tas_ptr = (group_task*) (m->focused_mob);
    pikmin* pik_ptr = (pikmin*) m;
    tas_ptr->free_up_spot(pik_ptr);
    m->unfocus_from_mob();
    m->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to forget a tool object it was going for.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::forget_tool(mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    tool* too_ptr = (tool*) (m->focused_mob);
    too_ptr->reserved = NULL;
    m->unfocus_from_mob();
    m->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin gets knocked back.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::get_knocked_back(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_KNOCKED_BACK);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin gets knocked back and lands on the floor.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::get_knocked_down(mob* m, void* info1, void* info2) {
    pikmin* p_ptr = (pikmin*) m;
    
    //Let's use the "temp" variable to specify whether or not a leader
    //already whistled it.
    p_ptr->temp_i = 0;
    
    m->set_animation(PIKMIN_ANIM_LYING);
    m->set_timer(PIKMIN_KNOCKED_DOWN_DURATION);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to go towards its spot on a carriable object.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the mob to carry.
 * info2:
 *   Unused.
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
    pik_ptr->temp_i = closest_spot;
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
 * When a Pikmin needs to go towards a group task mob.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the group task.
 * info2:
 *   Unused.
 */
void pikmin_fsm::go_to_group_task(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    group_task* tas_ptr = (group_task*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    group_task::group_task_spot* free_spot = tas_ptr->get_free_spot();
    if(!free_spot) {
        //There are no free spots available. Forget it.
        return;
    }
    
    tas_ptr->reserve_spot(free_spot, pik_ptr);
    
    pik_ptr->leave_group();
    pik_ptr->stop_chasing();
    
    m->focus_on_mob(tas_ptr);
    
    m->chase(
        point(), &(free_spot->absolute_pos),
        false, nullptr, false
    );
    
    pik_ptr->set_animation(PIKMIN_ANIM_WALKING);
    pik_ptr->set_timer(PIKMIN_GOTO_TIMEOUT);
    pik_ptr->fsm.set_state(PIKMIN_STATE_GOING_TO_GROUP_TASK);
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to walk towards an Onion to climb inside.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the Onion.
 * info2:
 *   Unused.
 */
void pikmin_fsm::go_to_onion(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    pikmin* p_ptr = (pikmin*) m;
    pikmin_nest_struct* n_ptr = (pikmin_nest_struct*) info1;
    
    //Pick a leg at random.
    p_ptr->temp_i =
        randomi(0, (n_ptr->nest_type->leg_body_parts.size() / 2) - 1);
    size_t leg_foot_bp_idx =
        n_ptr->m_ptr->anim.anim_db->find_body_part(
            n_ptr->nest_type->leg_body_parts[p_ptr->temp_i * 2 + 1]
        );
    point coords =
        n_ptr->m_ptr->get_hitbox(
            leg_foot_bp_idx
        )->get_cur_pos(n_ptr->m_ptr->pos, n_ptr->m_ptr->angle);
        
    m->focus_on_mob(n_ptr->m_ptr);
    m->stop_chasing();
    m->chase(coords, NULL, false);
    m->set_animation(PIKMIN_ANIM_WALKING);
    m->leave_group();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to walk towards an opponent.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the opponent.
 * info2:
 *   Unused.
 */
void pikmin_fsm::go_to_opponent(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    pikmin* p_ptr = (pikmin*) m;
    
    mob* o_ptr = (mob*) info1;
    if(o_ptr->type->category->id == MOB_CATEGORY_ENEMIES) {
        enemy* e_ptr = (enemy*) info1;
        if(!e_ptr->ene_type->allow_ground_attacks) return;
        if(e_ptr->z > m->z + m->height) return;
    }
    
    m->focus_on_mob(o_ptr);
    m->stop_chasing();
    m->chase(
        point(),
        &m->focused_mob->pos,
        false, nullptr, false,
        m->focused_mob->type->radius + m->type->radius + GROUNDED_ATTACK_DIST
    );
    m->set_animation(PIKMIN_ANIM_WALKING);
    m->leave_group();
    
    p_ptr->was_last_hit_dud = false;
    p_ptr->consecutive_dud_hits = 0;
    
    m->fsm.set_state(PIKMIN_STATE_GOING_TO_OPPONENT);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to go towards a tool mob.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the tool.
 * info2:
 *   Unused.
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


const float PIKMIN_DISMISS_TIMEOUT = 4.0f;

/* ----------------------------------------------------------------------------
 * When a Pikmin needs to get going to its dismiss spot.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::going_to_dismiss_spot(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_WALKING);
    
    m->set_timer(PIKMIN_DISMISS_TIMEOUT);
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::land(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_IDLING);
    
    pikmin_fsm::stand_still(m, NULL, NULL);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin being bounced back from an impact attack lands on the ground.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the hitbox touch information structure.
 * info2:
 *   Unused.
 */
void pikmin_fsm::land_after_impact_bounce(mob* m, void* info1, void* info2) {
    if(m->script_timer.time_left > 0.0f) {
        //We haven't actually landed yet. Never mind.
        return;
    }
    
    m->fsm.set_state(PIKMIN_STATE_KNOCKED_DOWN);
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands on a mob, to latch on to it.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the hitbox touch information structure.
 * info2:
 *   Unused.
 */
void pikmin_fsm::land_on_mob(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    hitbox_interaction* info = (hitbox_interaction*) info1;
    mob* m2_ptr = info->mob2;
    
    if(!m->can_hurt(m2_ptr)) return;
    
    mob_event* m2_pik_land_ev =
        m2_ptr->fsm.get_event(MOB_EV_THROWN_PIKMIN_LANDED);
        
    if(m2_pik_land_ev && m->was_thrown) {
        m2_pik_land_ev->run(m2_ptr, (void*)m);
    }
    
    hitbox* h_ptr = info->h2;
    
    if(
        !h_ptr ||
        (
            pik_ptr->pik_type->attack_method == PIKMIN_ATTACK_LATCH &&
            !h_ptr->can_pikmin_latch
        )
    ) {
        //No good. Make it bounce back.
        m->speed.x *= -0.3;
        m->speed.y *= -0.3;
        return;
    }
    
    pik_ptr->stop_height_effect();
    pik_ptr->focused_mob = m2_ptr;
    pik_ptr->was_thrown = false;
    
    switch(pik_ptr->pik_type->attack_method) {
    case PIKMIN_ATTACK_LATCH: {
        pik_ptr->speed.x = pik_ptr->speed.y = pik_ptr->speed_z = 0;
        
        float h_offset_dist;
        float h_offset_angle;
        m2_ptr->get_hitbox_hold_point(
            pik_ptr, h_ptr, &h_offset_dist, &h_offset_angle
        );
        m2_ptr->hold(
            pik_ptr, h_ptr->body_part_index, h_offset_dist, h_offset_angle,
            true, true
        );
        
        pik_ptr->latched = true;
        
        pik_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_LATCHED);
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {
        pik_ptr->fsm.set_state(PIKMIN_STATE_IMPACT_BOUNCE);
        break;
        
    }
    }
    
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands on a mob, whilst holding something.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the hitbox touch information structure.
 * info2:
 *   Unused.
 */
void pikmin_fsm::land_on_mob_while_holding(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    engine_assert(!m->holding.empty(), m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    hitbox_interaction* info = (hitbox_interaction*) info1;
    tool* too_ptr = (tool*) (*m->holding.begin());
    mob* m2_ptr = info->mob2;
    
    if(!m->can_hurt(m2_ptr)) return;
    
    mob_event* m2_pik_land_ev =
        m2_ptr->fsm.get_event(MOB_EV_THROWN_PIKMIN_LANDED);
        
    if (m2_pik_land_ev && m->was_thrown) {
        m2_pik_land_ev->run(m2_ptr, (void*)m);
    }
    
    pik_ptr->was_thrown = false;
    
    if(too_ptr->too_type->dropped_when_pikmin_lands_on_opponent) {
        pikmin_fsm::release_tool(m, info1, info2);
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        
        if(too_ptr->too_type->stuck_when_pikmin_lands_on_opponent && info->h2) {
            too_ptr->speed.x = too_ptr->speed.y = too_ptr->speed_z = 0;
            too_ptr->stop_height_effect();
            
            too_ptr->focused_mob = m2_ptr;
            
            float h_offset_dist;
            float h_offset_angle;
            m2_ptr->get_hitbox_hold_point(
                too_ptr, info->h2, &h_offset_dist, &h_offset_angle
            );
            m2_ptr->hold(
                too_ptr, info->h2->body_part_index,
                h_offset_dist, h_offset_angle, true, true
            );
        }
        
        if(too_ptr->too_type->pikmin_returns_after_using) {
            pikmin_fsm::called(m, game.states.gameplay->cur_leader_ptr, NULL);
            m->fsm.set_state(PIKMIN_STATE_IN_GROUP_CHASING);
        }
    }
}


/* ----------------------------------------------------------------------------
 * When a thrown Pikmin lands while holding something.
 * Depending on what it is, it might drop it.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
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
            pikmin_fsm::called(m, game.states.gameplay->cur_leader_ptr, NULL);
            m->fsm.set_state(PIKMIN_STATE_IN_GROUP_CHASING);
        }
    } else {
        m->fsm.set_state(PIKMIN_STATE_IDLING_H);
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin leaves its Onion because it got called out.
 * m:
 *   The mob.
 * info1:
 *   Points to the Onion.
 * info2:
 *   Unused.
 */
void pikmin_fsm::leave_onion(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    m->set_animation(PIKMIN_ANIM_SLIDING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin leaves a hazardous sector.
 * m:
 *   The mob.
 * info1:
 *   Points to the hazard.
 * info2:
 *   Unused.
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
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::lose_latched_mob(mob* m, void* info1, void* info2) {
    m->stop_chasing();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin notifies the leader that it must gently release it.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::notify_leader_release(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = ((pikmin*) m);
    if(!pik_ptr->following_group) return;
    if(pik_ptr->holder.m != pik_ptr->following_group) return;
    pik_ptr->following_group->fsm.run_event(MOB_EV_RELEASE_ORDER);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to decide a new spot to run off to whilst
 * in panicking.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
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
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::prepare_to_attack(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    pikmin* p = (pikmin*) m;
    p->set_animation(PIKMIN_ANIM_ATTACKING);
    p->face(0, &p->focused_mob->pos);
    p->was_last_hit_dud = false;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin reaches its spot on a carriable object.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::reach_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* carriable_mob = pik_ptr->carrying_mob;
    
    pik_ptr->set_animation(PIKMIN_ANIM_GRABBING, true);
    
    point final_pos =
        carriable_mob->pos +
        carriable_mob->carry_info->spot_info[pik_ptr->temp_i].pos;
        
    pik_ptr->chase(
        carriable_mob->carry_info->spot_info[pik_ptr->temp_i].pos,
        &carriable_mob->pos,
        true, &carriable_mob->z
    );
    
    pik_ptr->face(get_angle(final_pos, carriable_mob->pos), NULL);
    
    pik_ptr->set_animation(PIKMIN_ANIM_CARRYING);
    
    //Let the carriable mob know that a new Pikmin has grabbed on.
    pik_ptr->carrying_mob->fsm.run_event(
        MOB_EV_CARRIER_ADDED, (void*) pik_ptr
    );
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin reaches its dismissal spot.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::reach_dismiss_spot(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin that just attacked an opponent needs to walk
 * towards it again.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::rechase_opponent(mob* m, void* info1, void* info2) {

    pikmin* p_ptr = (pikmin*) m;
    
    if(p_ptr->was_last_hit_dud) {
        //Check if the Pikmin's last hits were duds.
        //If so, maybe give up and sigh.
        p_ptr->consecutive_dud_hits++;
        if(p_ptr->consecutive_dud_hits >= 4) {
            p_ptr->consecutive_dud_hits = 0;
            p_ptr->fsm.set_state(PIKMIN_STATE_SIGHING);
            return;
        }
    }
    
    if(
        m->focused_mob &&
        m->focused_mob->health > 0 &&
        dist(m->pos, m->focused_mob->pos) <=
        (m->type->radius + m->focused_mob->type->radius + GROUNDED_ATTACK_DIST)
    ) {
        //If the opponent is alive and within reach, let's stay in this state,
        //and attack some more!
        return;
    }
    
    //The opponent cannot be chased down. Become idle.
    m->fsm.set_state(PIKMIN_STATE_IDLING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to release the tool it is currently holding.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
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
        game.states.gameplay->subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, p_ptr->pik_type
        );
    if(m->following_group) {
        m->following_group->group->change_standby_type_if_needed();
        game.states.gameplay->update_closest_group_member();
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin seed lands on the ground.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::seed_landed(mob* m, void* info1, void* info2) {
    //Generate the rock particles that come out.
    particle pa(
        PARTICLE_TYPE_BITMAP, m->pos, m->z + m->height,
        4, 1, PARTICLE_PRIORITY_LOW
    );
    pa.bitmap = game.sys_assets.bmp_rock;
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
    pg.emit(game.states.gameplay->particles);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to to change "reach" to the idle task reach.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::set_idle_task_reach(mob* m, void* info1, void* info2) {
    m->near_reach = 0;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to to change "reach" to the swarm reach.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::set_swarm_reach(mob* m, void* info1, void* info2) {
    m->near_reach = 1;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin has to setup the timer in order to start the impact bounce
 * process later.
 * The reason we can't start the process right as the state is entered is
 * because the Pikmin changes animation and position too quickly for the mob
 * it impacted against to register the attack hitbox collision.
 * We have to delay that for one frame, and setting a timer with a tiny time
 * will do the trick.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::setup_impact_bounce(mob* m, void* info1, void* info2) {
    m->set_timer(0.000001f);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to sigh.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::sigh(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_SIGHING);
}


/* ----------------------------------------------------------------------------
 * Causes a sprout to evolve.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::sprout_evolve(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(p->maturity == 0 || p->maturity == 1) {
        //Leaf to bud, or bud to flower.
        
        p->maturity++;
        
        //Generate a burst of particles to symbolize the maturation.
        particle pa(
            PARTICLE_TYPE_BITMAP, m->pos, m->z + m->height,
            16, 1, PARTICLE_PRIORITY_LOW
        );
        pa.bitmap = game.sys_assets.bmp_sparkle;
        pa.color = al_map_rgb(255, 255, 255);
        particle_generator pg(0, pa, 8);
        pg.number_deviation = 1;
        pg.size_deviation = 8;
        pg.angle = 0;
        pg.angle_deviation = TAU / 2;
        pg.total_speed = 40;
        pg.total_speed_deviation = 10;
        pg.duration_deviation = 0.25;
        pg.emit(game.states.gameplay->particles);
        
    } else {
        //Flower to leaf.
        
        p->maturity = 0;
        
        //Generate a dribble of particles to symbolize the regression.
        particle pa(
            PARTICLE_TYPE_BITMAP, m->pos, m->z + m->height,
            16, 1, PARTICLE_PRIORITY_LOW
        );
        pa.bitmap = game.sys_assets.bmp_sparkle;
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
        pg.emit(game.states.gameplay->particles);
    }
}


/* ----------------------------------------------------------------------------
 * Schedules the next evolution for a sprout.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::sprout_schedule_evol(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    m->set_timer(p->pik_type->sprout_evolution_time[p->maturity]);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to stand still in place.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::stand_still(mob* m, void* info1, void* info2) {
    m->stop_circling();
    m->stop_following_path();
    m->stop_chasing();
    m->stop_turning();
    m->speed.x = m->speed.y = 0;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin needs to start chasing after its leader (or the group spot
 * belonging to the leader).
 * m:
 *   The mob.
 * info1:
 *   Points to the position struct with the final destination.
 *   If NULL, the final destination is calculated in this function.
 * info2:
 *   Unused.
 */
void pikmin_fsm::start_chasing_leader(mob* m, void* info1, void* info2) {
    m->focus_on_mob(m->following_group);
    m->set_animation(PIKMIN_ANIM_WALKING);
    pikmin_fsm::update_in_group_chasing(m, NULL, NULL);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts drinking the drop it touched.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the drop mob.
 * info2:
 *   Unused.
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
 * When a Pikmin starts flailing.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
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
 * When a Pikmin starts lunging forward for an impact attack.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::start_impact_lunge(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    m->chase(point(), &m->focused_mob->pos, false);
    m->set_animation(PIKMIN_ANIM_ATTACKING);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts panicking.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::start_panicking(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_WALKING);
    m->leave_group();
    pikmin_fsm::panic_new_chase(m, info1, info2);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts getting up from being knocked down.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::start_getting_up(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_GETTING_UP);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin starts picking some object up to hold it.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::start_picking_up(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_PICKING_UP);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin must start returning to the carried object's return point.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the mob that used to be carried.
 * info2:
 *   Unused.
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
 * When a Pikmin starts riding on a track.
 * m:
 *   The mob.
 * info1:
 *   Points to the track mob.
 * info2:
 *   Unused.
 */
void pikmin_fsm::start_riding_track(mob* m, void* info1, void* info2) {
    track* tra_ptr = (track*) info1;
    
    m->leave_group();
    m->stop_chasing();
    m->focus_on_mob(tra_ptr);
    m->start_height_effect();
    
    switch(tra_ptr->tra_type->riding_pose) {
    case TRACK_RIDING_POSE_STOPPED: {
        m->set_animation(PIKMIN_ANIM_WALKING);
        break;
    } case TRACK_RIDING_POSE_CLIMBING: {
        m->set_animation(PIKMIN_ANIM_WALKING);
        break;
    } case TRACK_RIDING_POSE_SLIDING: {
        m->set_animation(PIKMIN_ANIM_SLIDING);
        break;
    }
    }
    
    vector<size_t> checkpoints;
    for(size_t c = 0; c < tra_ptr->type->anims.body_parts.size(); ++c) {
        checkpoints.push_back(c);
    }
    m->track_info =
        new track_info_struct(
        tra_ptr, checkpoints, tra_ptr->tra_type->ride_speed
    );
}


/* ----------------------------------------------------------------------------
 * When a Pikmin must no longer be idling.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::stop_being_idle(mob* m, void* info1, void* info2) {

}


/* ----------------------------------------------------------------------------
 * When a Pikmin is no longer in the thrown state.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::stop_being_thrown(mob* m, void* info1, void* info2) {
    m->remove_particle_generator(MOB_PARTICLE_GENERATOR_THROW);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is meant to release an object it is carrying.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::stop_carrying(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(!p->carrying_mob) return;
    
    p->carrying_mob->fsm.run_event(MOB_EV_CARRIER_REMOVED, (void*) p);
    
    p->carrying_mob = NULL;
    p->set_timer(0);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin stands still while in a leader's group.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::stop_in_group(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_IDLING);
    m->face(0, &m->following_group->pos);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin has to teleport to its spot in the Onion leg.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::tick_entering_onion(mob* m, void* info1, void* info2) {
    engine_assert(m->track_info != NULL, m->print_state_history());
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    if(m->tick_track_ride()) {
        //Finished!
        ((onion*) m->focused_mob)->nest->store_pikmin((pikmin*) m);
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin has to teleport to its spot in a group task.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::tick_group_task_work(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    group_task* tas_ptr = (group_task*) (m->focused_mob);
    point cur_spot_pos = tas_ptr->get_spot_pos(pik_ptr);
    
    pik_ptr->chase(
        cur_spot_pos,
        NULL,
        true,
        &tas_ptr->z
    );
    pik_ptr->angle = tas_ptr->angle + tas_ptr->tas_type->worker_pikmin_angle;
    pik_ptr->intended_turn_angle = pik_ptr->angle;
    pik_ptr->stop_turning();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin has to teleport to its spot in a track it is riding.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::tick_track_ride(mob* m, void* info1, void* info2) {
    engine_assert(m->track_info != NULL, m->print_state_history());
    pikmin* pik_ptr = (pikmin*) m;
    
    if(m->tick_track_ride()) {
        //Finished!
        m->fsm.set_state(PIKMIN_STATE_IDLING, NULL, NULL);
        if(pik_ptr->leader_to_return_to) {
            pikmin_fsm::called(m, pik_ptr->leader_to_return_to, NULL);
            m->fsm.set_state(PIKMIN_STATE_IN_GROUP_CHASING);
        }
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin touches a "eat" hitbox.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::touched_eat_hitbox(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    engine_assert(info2 != NULL, m->print_state_history());
    
    if(m->invuln_period.time_left > 0) return;
    if(m->health <= 0) {
        return;
    }
    
    for(size_t s = 0; s < m->statuses.size(); ++s) {
        if(m->statuses[s].type->turns_inedible) {
            return;
        }
    }
    
    m->fsm.set_state(PIKMIN_STATE_GRABBED_BY_ENEMY, info1, info2);
}


/* ----------------------------------------------------------------------------
 * When a Pikmin touches a hazard.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the hazard type.
 * info2:
 *   Pointer to the hitbox that caused this, if any.
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
            particle par(
                PARTICLE_TYPE_BITMAP, m->pos, m->z,
                0, 1, PARTICLE_PRIORITY_LOW
            );
            par.bitmap = game.sys_assets.bmp_wave_ring;
            par.size_grow_speed = m->type->radius * 4;
            particle_generator pg(0.3, par, 1);
            pg.follow_mob = m;
            pg.id = MOB_PARTICLE_GENERATOR_WAVE_RING;
            m->particle_generators.push_back(pg);
        }
    }
    
    if(p->invuln_period.time_left > 0) return;
    mob_type::vulnerability_struct vuln = p->get_hazard_vulnerability(h);
    if(vuln.damage_mult == 0.0f) return;
    
    if(!vuln.status_to_apply || !vuln.status_overrides) {
        for(size_t e = 0; e < h->effects.size(); ++e) {
            p->apply_status_effect(h->effects[e], false, false);
        }
    }
    if(vuln.status_to_apply) {
        p->apply_status_effect(vuln.status_to_apply, true, false);
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is sprayed.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the spray type.
 * info2:
 *   Unused.
 */
void pikmin_fsm::touched_spray(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); ++e) {
        m->apply_status_effect(s->effects[e], false, false);
    }
    
    if(s->buries_pikmin) {
        m->fsm.set_state(PIKMIN_STATE_SPROUT, NULL, NULL);
    }
}


/* ----------------------------------------------------------------------------
 * When the Pikmin gets grabbed by an enemy. It should try to swap places
 * with the object that it is holding, instead, if possible.
 * If not, it should drop the object and get grabbed like normal.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
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
 * When the Pikmin stops latching on to an enemy.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::unlatch(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    m->focused_mob->release(m);
    ((pikmin*) m)->latched = false;
}


/* ----------------------------------------------------------------------------
 * When the Pikmin should update its destination when chasing the leader.
 * m:
 *   The mob.
 * info1:
 *   Points to the position struct with the final destination.
 *   If NULL, the final destination is calculated in this function.
 * info2:
 *   Unused.
 */
void pikmin_fsm::update_in_group_chasing(mob* m, void* info1, void* info2) {
    pikmin* p_ptr = (pikmin*) m;
    point target_pos;
    float target_dist; //Unused dummy value.
    
    if(!info1) {
        p_ptr->get_group_spot_info(&target_pos, &target_dist);
    } else {
        target_pos = *((point*) info1);
    }
    
    m->chase(target_pos, NULL, false);
    
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is whistled over by a leader while holding a tool.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the leader that called.
 * info2:
 *   Unused.
 */
void pikmin_fsm::whistled_while_holding(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    tool* too_ptr = (tool*) * (m->holding.begin());
    
    if(
        too_ptr->too_type->dropped_when_pikmin_is_whistled &&
        pik_ptr->is_tool_primed_for_whistle
    ) {
        pikmin_fsm::release_tool(m, info1, info2);
    }
    
    pik_ptr->is_tool_primed_for_whistle = false;
}


/* ----------------------------------------------------------------------------
 * When a Pikmin is whistled over by a leader while riding on a track.
 * m:
 *   The mob.
 * info1:
 *   Pointer to the leader that called.
 * info2:
 *   Unused.
 */
void pikmin_fsm::whistled_while_riding(mob* m, void* info1, void* info2) {
    engine_assert(m->track_info, m->print_state_history());
    
    track* tra_ptr = (track*) (m->track_info->m);
    
    if(tra_ptr->tra_type->cancellable_with_whistle) {
        m->stop_track_ride();
        pikmin_fsm::called(m, info1, NULL);
        m->fsm.set_state(PIKMIN_STATE_IN_GROUP_CHASING);
    }
}


/* ----------------------------------------------------------------------------
 * When the Pikmin should start working on a group task.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pikmin_fsm::work_on_group_task(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    group_task* tas_ptr = (group_task*) (m->focused_mob);
    pikmin* pik_ptr = (pikmin*) m;
    
    tas_ptr->add_worker(pik_ptr);
    
    pik_ptr->stop_chasing();
    pik_ptr->face(
        tas_ptr->angle + tas_ptr->tas_type->worker_pikmin_angle,
        NULL
    );
    
    if(
        tas_ptr->tas_type->worker_pikmin_pose ==
        GROUP_TASK_PIKMIN_POSE_STOPPED
    ) {
        pik_ptr->set_animation(PIKMIN_ANIM_IDLING);
    }
}
