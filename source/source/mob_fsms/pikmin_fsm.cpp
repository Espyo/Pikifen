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

#include "../functions.h"
#include "../game.h"
#include "../hazard.h"
#include "../mobs/bridge.h"
#include "../mobs/drop.h"
#include "../mobs/group_task.h"
#include "../mobs/pikmin.h"
#include "../mobs/tool.h"
#include "../mobs/track.h"
#include "../utils/general_utils.h"
#include "../utils/string_utils.h"
#include "gen_mob_fsm.h"


/**
 * @brief Creates the finite state machine for the Pikmin's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void pikmin_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("seed", PIKMIN_STATE_SEED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::become_sprout);
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::seed_landed);
            efc.change_state("sprout");
        }
    }
    
    efc.new_state("sprout", PIKMIN_STATE_SPROUT); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::stand_still);
            efc.run(pikmin_fsm::become_sprout);
            efc.run(pikmin_fsm::sprout_schedule_evol);
        }
        efc.new_event(MOB_EV_PLUCKED); {
            efc.change_state("plucking");
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::sprout_evolve);
            efc.run(pikmin_fsm::sprout_schedule_evol);
        }
    }
    
    efc.new_state("plucking", PIKMIN_STATE_PLUCKING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::begin_pluck);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("plucking_thrown");
        }
    }
    
    efc.new_state("plucking_thrown", PIKMIN_STATE_PLUCKING_THROWN); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::be_thrown_after_pluck);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::stop_being_thrown);
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(pikmin_fsm::land_after_pluck);
        }
        efc.new_event(MOB_EV_TOUCHED_HAZARD); {
            efc.run(pikmin_fsm::touched_hazard);
        }
        efc.new_event(MOB_EV_LEFT_HAZARD); {
            efc.run(pikmin_fsm::left_hazard);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(pikmin_fsm::fall_down_pit);
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
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::clear_boredom_data);
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
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::start_boredom_anim);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::check_boredom_anim_end);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EV_NEAR_CARRIABLE_OBJECT); {
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
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_EAT); {
            efc.run(pikmin_fsm::touched_eat_hitbox);
        }
        efc.new_event(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EV_NEAR_CARRIABLE_OBJECT); {
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
            efc.run(pikmin_fsm::be_released);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
            efc.run(pikmin_fsm::set_bump_lock);
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
    
    efc.new_state("mob_landing", PIKMIN_STATE_MOB_LANDING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_mob_landing);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finish_mob_landing);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_A_N); {
            efc.run(pikmin_fsm::check_outgoing_attack);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::unlatch);
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
    
    efc.new_state(
        "going_to_dismiss_spot", PIKMIN_STATE_GOING_TO_DISMISS_SPOT
    ); {
        efc.new_event(MOB_EV_WHISTLED); {
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::going_to_dismiss_spot);
        }
        efc.new_event(MOB_EV_ON_LEAVE); {
            efc.run(pikmin_fsm::clear_timer);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.run(pikmin_fsm::set_bump_lock);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::reach_dismiss_spot);
            efc.run(pikmin_fsm::set_bump_lock);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EV_NEAR_CARRIABLE_OBJECT); {
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
            efc.run(pikmin_fsm::clear_boredom_data);
            efc.run(pikmin_fsm::stop_being_idle);
        }
        efc.new_event(MOB_EV_OPPONENT_IN_REACH); {
            efc.run(pikmin_fsm::go_to_opponent);
        }
        efc.new_event(MOB_EV_NEAR_CARRIABLE_OBJECT); {
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
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::check_leader_bump);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::start_boredom_anim);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::check_boredom_anim_end);
            efc.run(pikmin_fsm::check_shaking_anim_end);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
    
    efc.new_state("called", PIKMIN_STATE_CALLED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::called);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finish_called_anim);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
    
    efc.new_state("going_to_opponent", PIKMIN_STATE_GOING_TO_OPPONENT); {
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(pikmin_fsm::decide_attack);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.change_state("called");
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
    
    efc.new_state("circling_opponent", PIKMIN_STATE_CIRCLING_OPPONENT); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::circle_opponent);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::decide_attack);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.change_state("called");
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
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::go_to_carriable_object);
        }
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
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::forget_carriable_object);
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
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
            efc.run(pikmin_fsm::forget_tool);
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
            efc.change_state("called");
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
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::check_leader_bump);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
        efc.new_event(MOB_EV_ON_TICK); {
            efc.run(pikmin_fsm::tick_carrying);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.change_state("called");
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
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE); {
            efc.run(pikmin_fsm::forget_group_task);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_FOCUS_DIED); {
            efc.run(pikmin_fsm::lose_latched_mob);
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
            efc.run(pikmin_fsm::be_attacked);
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
            efc.run(pikmin_fsm::stand_still);
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
            efc.change_state("impact_bounce");
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
            efc.run(pikmin_fsm::stand_still);
            efc.run(pikmin_fsm::notify_leader_release);
            efc.run(pikmin_fsm::be_released);
            efc.run(pikmin_fsm::release_tool);
            efc.run(pikmin_fsm::start_flailing);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.run(pikmin_fsm::flail_to_leader);
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
            efc.run(pikmin_fsm::stand_still);
            efc.run(pikmin_fsm::unlatch);
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
        
        //The logic to stop panicking is in
        //pikmin::handle_status_effect_loss();
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
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::stand_still);
            efc.run(pikmin_fsm::celebrate);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
        efc.new_event(MOB_EV_WHISTLED); {
            efc.change_state("called");
        }
        efc.new_event(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::check_leader_bump);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
    
    efc.new_state("in_group_chasing_h", PIKMIN_STATE_IN_GROUP_CHASING_H); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_chasing_leader);
        }
        efc.new_event(MOB_EV_RELEASE_ORDER); {
            efc.run(pikmin_fsm::release_tool);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EV_GO_TO_ONION); {
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
            efc.change_state("called_h");
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
            efc.run(pikmin_fsm::set_bump_lock);
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
        }
        efc.new_event(MOB_EV_TOUCHED_ACTIVE_LEADER); {
            efc.run(pikmin_fsm::check_leader_bump);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
    
    efc.new_state("called_h", PIKMIN_STATE_CALLED_H); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::called);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finish_called_anim);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(pikmin_fsm::check_incoming_attack);
        }
        efc.new_event(MOB_EV_PIKMIN_DAMAGE_CONFIRMED); {
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
    
    efc.new_state("crushed", PIKMIN_STATE_CRUSHED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::be_crushed);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::die);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_idx = fix_states(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
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
void pikmin_fsm::be_attacked(mob* m, void* info1, void* info2) {
    hitbox_interaction* info = (hitbox_interaction*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    if(info) {
        //Damage.
        float damage = 0;
        float health_before = pik_ptr->health;
        info->mob2->calculate_damage(m, info->h2, info->h1, &damage);
        m->apply_attack_damage(info->mob2, info->h2, info->h1, damage);
        if(pik_ptr->health <= 0.0f && health_before > 0.0f) {
            if(!info->h2->hazards.empty()) {
                game.statistics.pikmin_hazard_deaths++;
            }
        }
        
        //Knockback.
        float knockback = 0;
        float knockback_angle = 0;
        info->mob2->calculate_knockback(
            m, info->h2, info->h1, &knockback, &knockback_angle
        );
        m->apply_knockback(knockback, knockback_angle);
        
        //Withering.
        if(info->h2->wither_chance > 0 && pik_ptr->maturity > 0) {
            unsigned char wither_roll = randomi(0, 100);
            if(wither_roll < info->h2->wither_chance) {
                pik_ptr->increase_maturity(-1);
            }
        }
        
        //Effects.
        m->do_attack_effects(info->mob2, info->h2, info->h1, damage, knockback);
        
    } else {
        //This can happen, for example, if the Pikmin got told to get knocked
        //back from a bomb rock hotswap. There's no real "hit" in this case
        //so let's just do the basics and let the Pikmin leave the group,
        //change animation, and little else.
    }
    
    //Finish up.
    m->leave_group();
    pikmin_fsm::be_released(m, info1, info2);
    pikmin_fsm::notify_leader_release(m, info1, info2);
    pikmin_fsm::release_tool(m, nullptr, nullptr);
    m->face(m->angle, nullptr);
}


/**
 * @brief When a Pikmin is crushed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::be_crushed(mob* m, void* info1, void* info2) {
    m->z = m->ground_sector->z;
    m->leave_group();
    pikmin_fsm::be_released(m, info1, info2);
    pikmin_fsm::notify_leader_release(m, info1, info2);
    pikmin_fsm::release_tool(m, nullptr, nullptr);
    enable_flag(m->flags, MOB_FLAG_INTANGIBLE);
    m->set_animation(PIKMIN_ANIM_CRUSHED);
}


/**
 * @brief When a Pikmin is dismissed by its leader.
 *
 * @param m The mob.
 * @param info1 Pointer to the world coordinates to go to.
 * @param info2 Unused.
 */
void pikmin_fsm::be_dismissed(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    engine_assert(info1 != nullptr, m->print_state_history());
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->chase(*((point*) info1), m->z);
    
    m->set_animation(PIKMIN_ANIM_IDLING);
    m->play_sound(pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_IDLE]);
}


/**
 * @brief When a Pikmin is grabbed by an enemy.
 *
 * @param m The mob.
 * @param info1 Pointer to the enemy.
 * @param info2 Pointer to the hitbox that grabbed.
 */
void pikmin_fsm::be_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    engine_assert(info2 != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    mob* ene_ptr = (mob*) info1;
    hitbox* hbox_ptr = (hitbox*) info2;
    
    ene_ptr->chomp(pik_ptr, hbox_ptr);
    pik_ptr->is_grabbed_by_enemy = true;
    disable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    pik_ptr->leave_group();
    
    pik_ptr->set_animation(PIKMIN_ANIM_FLAILING, START_ANIM_OPTION_RANDOM_TIME);
    m->play_sound(pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_CAUGHT]);
    
}


/**
 * @brief When a Pikmin is grabbed by a leader.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::be_grabbed_by_friend(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->set_animation(PIKMIN_ANIM_IDLING);
    m->play_sound(pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_HELD]);
}


/**
 * @brief When a Pikmin is gently released by a leader or enemy.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::be_released(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    ((pikmin*) m)->is_grabbed_by_enemy = false;
    
    size_t held_sound_idx =
        pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_HELD];
    if(held_sound_idx != INVALID) {
        game.audio.stop_all_playbacks(
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
void pikmin_fsm::be_thrown(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    m->set_animation(PIKMIN_ANIM_THROWN);
    
    size_t held_sound_idx =
        pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_HELD];
    if(held_sound_idx != INVALID) {
        game.audio.stop_all_playbacks(
            pik_ptr->type->sounds[held_sound_idx].sample
        );
    }
    
    size_t throw_sound_idx =
        pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_THROWN];
    if(throw_sound_idx != INVALID) {
        mob_type::sound_t* throw_sound =
            &pik_ptr->type->sounds[throw_sound_idx];
        sound_source_config_t throw_sound_config;
        throw_sound_config.stack_mode = SOUND_STACK_MODE_OVERRIDE;
        game.audio.create_mob_sound_source(
            throw_sound->sample,
            m,
            throw_sound_config
        );
    }
    
    ((pikmin*) m)->start_throw_trail();
}


/**
 * @brief When a Pikmin is thrown after being plucked.
 *
 * @param m The mob.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void pikmin_fsm::be_thrown_after_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    float throw_angle = get_angle(m->pos, m->focused_mob->pos);
    m->speed_z = PIKMIN::THROW_VER_SPEED;
    m->speed = angle_to_coordinates(throw_angle, PIKMIN::THROW_HOR_SPEED);
    m->face(throw_angle + TAU / 2.0f, nullptr, true);
    
    m->set_animation(PIKMIN_ANIM_PLUCKING_THROWN);
    m->play_sound(pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_PLUCKED]);
    game.audio.create_world_pos_sound_source(
        game.sys_assets.sound_pluck,
        m->pos
    );
    ((pikmin*) m)->start_throw_trail();
    
    particle par(
        m->pos, m->z + m->get_drawing_height() + 1.0,
        12, 0.5, PARTICLE_PRIORITY_MEDIUM
    );
    par.color.set_keyframe_value(0, al_map_rgb(172, 164, 134));
    par.color.add(1, al_map_rgba(172, 164, 134, 0));
    par.outwards_speed = keyframe_interpolator<float>(70);
    par.linear_speed.add(1, point(0, 35));
    particle_generator pg(0, par, 12);
    pg.emission.number_deviation = 5;
    pg.outwards_speed_deviation = 10;
    pg.duration_deviation = 0.3;
    pg.emit(game.states.gameplay->particles);
}


/**
 * @brief When a Pikmin is thrown by a bouncer mob.
 *
 * @param m The mob.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void pikmin_fsm::be_thrown_by_bouncer(mob* m, void* info1, void* info2) {
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->set_animation(PIKMIN_ANIM_THROWN);
    
    ((pikmin*) m)->start_throw_trail();
}


/**
 * @brief When a Pikmin becomes "helpless".
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::become_helpless(mob* m, void* info1, void* info2) {
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->leave_group();
    
    m->set_animation(PIKMIN_ANIM_IDLING);
}


/**
 * @brief When a Pikmin becomes idling.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::become_idle(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    pikmin_fsm::stand_still(m, info1, info2);
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
        pik_ptr->chase(
            pik_ptr->pos,
            pik_ptr->ground_sector->z + PIKMIN::FLIER_ABOVE_FLOOR_HEIGHT
        );
    }
    
    m->unfocus_from_mob();
    
    m->set_animation(
        PIKMIN_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    m->set_timer(
        randomf(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin becomes a seed or a sprout.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::become_sprout(mob* m, void* info1, void* info2) {
    m->leave_group();
    enable_flag(m->flags, MOB_FLAG_INTANGIBLE);
    enable_flag(m->flags, MOB_FLAG_NON_HUNTABLE);
    enable_flag(m->flags, MOB_FLAG_NON_HURTABLE);
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    ((pikmin*) m)->is_seed_or_sprout = true;
    m->set_animation(
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
void pikmin_fsm::begin_pluck(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    mob* lea_ptr = (mob*) info1;
    
    pik_ptr->focus_on_mob(lea_ptr);
    disable_flag(m->flags, MOB_FLAG_NON_HUNTABLE);
    disable_flag(m->flags, MOB_FLAG_NON_HURTABLE);
    disable_flag(m->flags, MOB_FLAG_INTANGIBLE);
    pik_ptr->is_seed_or_sprout = false;
    pikmin_fsm::clear_timer(m, info1, info2); //Clear sprout evolution timer.
    
    pik_ptr->set_animation(PIKMIN_ANIM_PLUCKING);
}


/**
 * @brief When a Pikmin is called over by a leader, either by being whistled,
 * or touched when idling.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 If not nullptr, then the Pikmin must be silent.
 */
void pikmin_fsm::called(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    mob* caller = (mob*) info1;
    
    pik_ptr->was_last_hit_dud = false;
    pik_ptr->consecutive_dud_hits = 0;
    pikmin_fsm::stand_still(m, info1, info2);
    
    pik_ptr->focus_on_mob(caller);
    
    pik_ptr->set_animation(PIKMIN_ANIM_CALLED);
    if(info2 == nullptr) {
        m->play_sound(pik_ptr->pik_type->sound_data_idxs[PIKMIN_SOUND_CALLED]);
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
void pikmin_fsm::called_while_knocked_down(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    mob* caller = (mob*) info1;
    
    //Let's use the "temp" variable to specify whether or not a leader
    //already whistled it.
    if(pik_ptr->temp_i == 1) return;
    
    pik_ptr->focus_on_mob(caller);
    
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
void pikmin_fsm::celebrate(mob* m, void* info1, void* info2) {
    if(randomi(0, 1) == 0) {
        m->set_animation(PIKMIN_ANIM_BACKFLIP);
    } else {
        m->set_animation(PIKMIN_ANIM_TWIRLING);
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
void pikmin_fsm::check_boredom_anim_end(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->in_bored_animation) return;
    m->set_animation(PIKMIN_ANIM_IDLING);
    pik_ptr->in_bored_animation = false;
    m->set_timer(
        randomf(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
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
void pikmin_fsm::check_incoming_attack(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    if(pik_ptr->invuln_period.time_left > 0) {
        //The Pikmin cannot be attacked right now.
        return;
    }
    
    if(!pik_ptr->process_attack_miss(info)) {
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


/**
 * @brief When a Pikmin should check if the leader bumping it should
 * result in it being added to the group or not.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::check_leader_bump(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(pik_ptr->bump_lock > 0.0f) {
        pik_ptr->bump_lock = game.config.idle_bump_delay;
        return;
    }
    if(
        !pik_ptr->holding.empty() &&
        pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
    ) {
        m->fsm.set_state(PIKMIN_STATE_CALLED_H, info1, info2);
    } else {
        m->fsm.set_state(PIKMIN_STATE_CALLED, info1, info2);
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
void pikmin_fsm::check_outgoing_attack(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    float damage = 0;
    bool attack_success =
        pik_ptr->calculate_damage(info->mob2, info->h1, info->h2, &damage);
        
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
void pikmin_fsm::check_shaking_anim_end(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->in_shaking_animation) return;
    m->set_animation(PIKMIN_ANIM_IDLING);
    pik_ptr->in_shaking_animation = false;
    m->set_timer(
        randomf(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin has to circle around its opponent.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::circle_opponent(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->stop_circling();
    
    float circle_time = randomf(0.0f, 1.0f);
    //Bias the time so that there's a higher chance of picking a close angle,
    //and a lower chance of circling to a distant one. The Pikmin came here
    //to attack, not dance!
    circle_time *= circle_time;
    circle_time += 0.5f;
    m->set_timer(circle_time);
    
    bool go_cw = randomf(0.0f, 1.0f) <= 0.5f;
    m->circle_around(
        m->focused_mob, point(), m->focused_mob->radius + m->radius, go_cw,
        m->get_base_speed(), true
    );
    
    m->set_animation(
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
void pikmin_fsm::clear_boredom_data(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    pikmin_fsm::clear_timer(m, info1, info2);
    pik_ptr->in_bored_animation = false;
}


/**
 * @brief When a Pikmin has to clear any timer set.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::clear_timer(mob* m, void* info1, void* info2) {
    m->set_timer(0);
}


/**
 * @brief When the Pikmin reaches an opponent that it was chasing after,
 * and should now decide how to attack it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::decide_attack(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    if(m->invuln_period.time_left > 0) {
        //Don't let the Pikmin attack while invulnerable. Otherwise, this can
        //be exploited to let Pikmin vulnerable to a hazard attack the obstacle
        //emitting said hazard.
        return;
    }
    
    pikmin* pik_ptr = (pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pik_ptr->stop_chasing();
    pik_ptr->stop_circling();
    
    bool can_circle =
        pik_ptr->fsm.cur_state->id != PIKMIN_STATE_CIRCLING_OPPONENT &&
        m->focused_mob->type->category->id == MOB_CATEGORY_ENEMIES;
        
    switch(pik_ptr->pik_type->attack_method) {
    case PIKMIN_ATTACK_LATCH: {
        //This Pikmin latches on to things and/or smacks with its top.
        dist d;
        hitbox* closest_h =
            pik_ptr->focused_mob->get_closest_hitbox(
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
                randomf(0.0f, 1.0f) <=
                PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED &&
                can_circle
            ) {
                //Circle around the opponent a bit before smacking.
                pik_ptr->fsm.set_state(PIKMIN_STATE_CIRCLING_OPPONENT);
            } else {
                //Smack.
                pik_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_GROUNDED);
            }
            
        } else {
            //Can latch to the closest hitbox.
            
            if(
                randomf(0, 1) <=
                PIKMIN::CIRCLE_OPPONENT_CHANCE_PRE_LATCH &&
                can_circle
            ) {
                //Circle around the opponent a bit before latching.
                pik_ptr->fsm.set_state(PIKMIN_STATE_CIRCLING_OPPONENT);
            } else {
                //Latch on.
                pik_ptr->latch(pik_ptr->focused_mob, closest_h);
                pik_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_LATCHED);
            }
            
        }
        
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {
        //This Pikmin attacks by lunching forward for an impact.
        
        if(
            randomf(0, 1) <=
            PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED &&
            can_circle
        ) {
            //Circle around the opponent a bit before lunging.
            pik_ptr->fsm.set_state(PIKMIN_STATE_CIRCLING_OPPONENT);
        } else {
            //Go for the lunge.
            pik_ptr->fsm.set_state(PIKMIN_STATE_IMPACT_LUNGE);
        }
        
        break;
        
    }
    }
}


/**
 * @brief When a Pikmin has to die right now.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::die(mob* m, void* info1, void* info2) {
    m->set_health(false, false, 0);
}


/**
 * @brief When a Pikmin has to bounce back from an impact attack.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::do_impact_bounce(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    disable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    float impact_angle = 0.0f;
    float impact_speed = 0.0f;
    
    if(pik_ptr->focused_mob) {
        if(pik_ptr->focused_mob->rectangular_dim.x != 0) {
            impact_angle =
                get_angle(
                    get_closest_point_in_rotated_rectangle(
                        pik_ptr->pos,
                        pik_ptr->focused_mob->pos,
                        pik_ptr->focused_mob->rectangular_dim,
                        pik_ptr->focused_mob->angle,
                        nullptr
                    ),
                    pik_ptr->pos
                );
        } else {
            impact_angle = get_angle(pik_ptr->focused_mob->pos, pik_ptr->pos);
        }
        impact_speed = 200.0f;
    }
    
    pik_ptr->speed =
        angle_to_coordinates(
            impact_angle, impact_speed
        );
    pik_ptr->speed_z = 500.0f;
    pik_ptr->face(impact_angle + TAU / 2.0f, nullptr, true);
    
    pik_ptr->set_animation(PIKMIN_ANIM_KNOCKED_BACK);
}


/**
 * @brief When a Pikmin must start climbing up an Onion's leg.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::enter_onion(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    onion* oni_ptr = (onion*) pik_ptr->focused_mob;
    
    disable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    //Set its data to start climbing.
    vector<size_t> checkpoints;
    checkpoints.push_back((pik_ptr->temp_i * 2) + 1);
    checkpoints.push_back(pik_ptr->temp_i * 2);
    
    pik_ptr->track_info = new track_t(
        oni_ptr, checkpoints, oni_ptr->oni_type->nest->pikmin_enter_speed
    );
    
    pik_ptr->set_animation(PIKMIN_ANIM_CLIMBING);
}


/**
 * @brief When a Pikmin falls down a bottomless pit.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->set_health(false, false, 0);
}


/**
 * @brief When a Pikmin finished the animation for when it's called.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finish_called_anim(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* lea_ptr = pik_ptr->focused_mob;
    
    if(lea_ptr) {
        if(lea_ptr->following_group) {
            //If this leader is following another one,
            //then the new Pikmin should be in the group of that top leader.
            lea_ptr = lea_ptr->following_group;
        }
        lea_ptr->add_to_group(pik_ptr);
        pik_ptr->fsm.set_state(
            pik_ptr->holding.empty() ?
            PIKMIN_STATE_IN_GROUP_CHASING :
            PIKMIN_STATE_IN_GROUP_CHASING_H,
            info1, info2);
    } else {
        pik_ptr->fsm.set_state(
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
void pikmin_fsm::finish_carrying(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    engine_assert(p->carrying_mob != nullptr, m->print_state_history());
    
    if(p->carrying_mob->carry_info->must_return) {
        //The Pikmin should return somewhere (like a pile).
        p->fsm.set_state(PIKMIN_STATE_RETURNING, (void*) p->carrying_mob);
        
    } else {
        //The Pikmin can just sit and chill.
        p->fsm.set_state(PIKMIN_STATE_CELEBRATING);
    }
}


/**
 * @brief When a Pikmin finishes drinking the drop it was drinking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finish_drinking(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    pikmin* pik_ptr = (pikmin*) m;
    drop* dro_ptr = (drop*) m->focused_mob;
    
    switch(dro_ptr->dro_type->effect) {
    case DROP_EFFECT_MATURATE: {
        pik_ptr->increase_maturity(dro_ptr->dro_type->increase_amount);
        break;
    } case DROP_EFFECT_GIVE_STATUS: {
        pik_ptr->apply_status_effect(
            dro_ptr->dro_type->status_to_give, false, false
        );
        break;
    } default: {
        break;
    }
    }
    
    m->unfocus_from_mob();
}


/**
 * @brief When a Pikmin finishes getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
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
void pikmin_fsm::finish_mob_landing(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    if(!m->focused_mob) {
        //The mob has died or vanished since the Pikmin first landed.
        //Return to idle.
        pik_ptr->fsm.set_state(PIKMIN_STATE_IDLING);
        return;
    }
    
    switch(pik_ptr->pik_type->attack_method) {
    case PIKMIN_ATTACK_LATCH: {
        pik_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_LATCHED);
        break;
        
    }
    case PIKMIN_ATTACK_IMPACT: {
        pik_ptr->fsm.set_state(PIKMIN_STATE_IMPACT_BOUNCE);
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
void pikmin_fsm::finish_picking_up(mob* m, void* info1, void* info2) {
    tool* too_ptr = (tool*) (m->focused_mob);
    
    if(!has_flag(too_ptr->holdability_flags, HOLDABILITY_FLAG_PIKMIN)) {
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        return;
    }
    
    m->subgroup_type_ptr =
        game.states.gameplay->subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_TOOL, m->focused_mob->type
        );
    m->hold(
        m->focused_mob, INVALID, 4, 0, 0.5f,
        true, HOLD_ROTATION_METHOD_FACE_HOLDER
    );
    m->unfocus_from_mob();
}


/**
 * @brief When the Pikmin must move towards the whistle.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void pikmin_fsm::flail_to_leader(mob* m, void* info1, void* info2) {
    mob* caller = (mob*) info1;
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
void pikmin_fsm::forget_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(!p->carrying_mob) return;
    
    p->carrying_mob->carry_info->spot_info[p->temp_i].state =
        CARRY_SPOT_STATE_FREE;
    p->carrying_mob->carry_info->spot_info[p->temp_i].pik_ptr =
        nullptr;
        
    p->carrying_mob = nullptr;
}


/**
 * @brief When a Pikmin is meant to forget a group task object it was going for.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::forget_group_task(mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    group_task* tas_ptr = (group_task*) (m->focused_mob);
    pikmin* pik_ptr = (pikmin*) m;
    tas_ptr->free_up_spot(pik_ptr);
    m->unfocus_from_mob();
}


/**
 * @brief When a Pikmin is meant to forget a tool object it was going for.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::forget_tool(mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    tool* too_ptr = (tool*) (m->focused_mob);
    too_ptr->reserved = nullptr;
    m->unfocus_from_mob();
}


/**
 * @brief When a Pikmin gets knocked back.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::get_knocked_back(mob* m, void* info1, void* info2) {
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->set_animation(PIKMIN_ANIM_KNOCKED_BACK);
}


/**
 * @brief When a Pikmin gets knocked back and lands on the floor.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::get_knocked_down(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    //Let's use the "temp" variable to specify whether or not a leader
    //already whistled it.
    pik_ptr->temp_i = 0;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->set_timer(pik_ptr->pik_type->knocked_down_duration);
    
    m->set_animation(PIKMIN_ANIM_LYING);
}


/**
 * @brief When a Pikmin needs to go towards its spot on a carriable object.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob to carry.
 * @param info2 Unused.
 */
void pikmin_fsm::go_to_carriable_object(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    mob* carriable_mob = (mob*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pik_ptr->carrying_mob = carriable_mob;
    pik_ptr->leave_group();
    pik_ptr->stop_chasing();
    
    size_t closest_spot = INVALID;
    dist closest_spot_dist;
    carrier_spot_t* closest_spot_ptr = nullptr;
    point closest_spot_offset;
    
    //If this is the first Pikmin to go to the carriable mob, rotate
    //the points such that 0 faces this Pikmin instead.
    if(
        carriable_mob->carry_info->is_empty() &&
        carriable_mob->type->custom_carry_spots.empty()
    ) {
        carriable_mob->carry_info->rotate_points(
            get_angle(carriable_mob->pos, pik_ptr->pos)
        );
    }
    
    for(size_t s = 0; s < carriable_mob->type->max_carriers; s++) {
        carrier_spot_t* spot_ptr =
            &carriable_mob->carry_info->spot_info[s];
        if(spot_ptr->state != CARRY_SPOT_STATE_FREE) continue;
        
        point spot_offset =
            rotate_point(spot_ptr->pos, carriable_mob->angle);
        dist d(pik_ptr->pos, carriable_mob->pos + spot_offset);
        
        if(closest_spot == INVALID || d < closest_spot_dist) {
            closest_spot = s;
            closest_spot_dist = d;
            closest_spot_ptr = spot_ptr;
            closest_spot_offset = spot_offset;
        }
    }
    
    if(!closest_spot_ptr) return;
    
    pik_ptr->focus_on_mob(carriable_mob);
    pik_ptr->temp_i = closest_spot;
    closest_spot_ptr->state = CARRY_SPOT_STATE_RESERVED;
    closest_spot_ptr->pik_ptr = pik_ptr;
    
    pik_ptr->chase(
        &carriable_mob->pos, &carriable_mob->z,
        closest_spot_offset, 0.0f
    );
    pik_ptr->set_timer(PIKMIN::GOTO_TIMEOUT);
    
    m->set_animation(
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
void pikmin_fsm::go_to_group_task(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    group_task* tas_ptr = (group_task*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    if(
        !has_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR) &&
        tas_ptr->tas_type->flying_pikmin_only
    ) {
        //Only flying Pikmin can use this, and this Pikmin doesn't fly.
        return;
    }
    
    group_task::group_task_spot* free_spot = tas_ptr->get_free_spot();
    if(!free_spot) {
        //There are no free spots available. Forget it.
        return;
    }
    
    tas_ptr->reserve_spot(free_spot, pik_ptr);
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pik_ptr->leave_group();
    pik_ptr->stop_chasing();
    
    m->focus_on_mob(tas_ptr);
    
    m->chase(
        &(free_spot->absolute_pos), &tas_ptr->z,
        point(), tas_ptr->tas_type->spots_z
    );
    pik_ptr->set_timer(PIKMIN::GOTO_TIMEOUT);
    
    m->set_animation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
    
    pik_ptr->fsm.set_state(PIKMIN_STATE_GOING_TO_GROUP_TASK);
    
}


/**
 * @brief When a Pikmin needs to walk towards an Onion to climb inside.
 *
 * @param m The mob.
 * @param info1 Pointer to the Onion.
 * @param info2 Unused.
 */
void pikmin_fsm::go_to_onion(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    pikmin_nest_t* nest_ptr = (pikmin_nest_t*) info1;
    
    //Pick a leg at random.
    pik_ptr->temp_i =
        randomi(
            0, (int) (nest_ptr->nest_type->leg_body_parts.size() / 2) - 1
        );
    size_t leg_foot_bp_idx =
        nest_ptr->m_ptr->anim.anim_db->find_body_part(
            nest_ptr->nest_type->leg_body_parts[pik_ptr->temp_i * 2 + 1]
        );
    point coords =
        nest_ptr->m_ptr->get_hitbox(
            leg_foot_bp_idx
        )->get_cur_pos(nest_ptr->m_ptr->pos, nest_ptr->m_ptr->angle);
        
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    bool aux_b = true; //Needed for a gentle release.
    pikmin_fsm::release_tool(m, (void*) &aux_b, nullptr);
    
    m->focus_on_mob(nest_ptr->m_ptr);
    m->stop_chasing();
    m->chase(coords, nest_ptr->m_ptr->z);
    m->leave_group();
    
    m->set_animation(
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
void pikmin_fsm::go_to_opponent(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    
    mob* other_ptr = (mob*) info1;
    if(other_ptr->type->category->id == MOB_CATEGORY_ENEMIES) {
        enemy* ene_ptr = (enemy*) info1;
        if(
            !ene_ptr->ene_type->allow_ground_attacks &&
            !pik_ptr->pik_type->can_fly
        ) return;
        if(ene_ptr->z > m->z + m->height) return;
    }
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->focus_on_mob(other_ptr);
    m->stop_chasing();
    
    point offset = point();
    float target_distance =
        m->focused_mob->radius + m->radius + PIKMIN::GROUNDED_ATTACK_DIST;
        
    if(m->focused_mob->rectangular_dim.x != 0.0f) {
        bool is_inside = false;
        offset =
            get_closest_point_in_rotated_rectangle(
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
    m->leave_group();
    
    pik_ptr->was_last_hit_dud = false;
    pik_ptr->consecutive_dud_hits = 0;
    
    m->set_animation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
    
    m->fsm.set_state(PIKMIN_STATE_GOING_TO_OPPONENT);
}


/**
 * @brief When a Pikmin needs to go towards a tool mob.
 *
 * @param m The mob.
 * @param info1 Pointer to the tool.
 * @param info2 Unused.
 */
void pikmin_fsm::go_to_tool(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
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
    if(!has_flag(too_ptr->holdability_flags, HOLDABILITY_FLAG_PIKMIN)) {
        //Can't hold this. Forget it.
        return;
    }
    
    too_ptr->reserved = pik_ptr;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pik_ptr->leave_group();
    pik_ptr->stop_chasing();
    
    m->focus_on_mob(too_ptr);
    
    m->chase(
        &too_ptr->pos, &too_ptr->z,
        point(), 0.0f, 0,
        pik_ptr->radius + too_ptr->radius
    );
    pik_ptr->set_timer(PIKMIN::GOTO_TIMEOUT);
    
    m->set_animation(
        PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true, m->type->move_speed
    );
    
    pik_ptr->fsm.set_state(PIKMIN_STATE_GOING_TO_TOOL);
    
}


/**
 * @brief When a Pikmin needs to get going to its dismiss spot.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::going_to_dismiss_spot(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->set_timer(PIKMIN::DISMISS_TIMEOUT);
    
    m->set_animation(
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
void pikmin_fsm::land(mob* m, void* info1, void* info2) {
    pikmin_fsm::stand_still(m, nullptr, nullptr);
    
    m->set_animation(PIKMIN_ANIM_IDLING);
}


/**
 * @brief When a Pikmin being bounced back from an impact attack lands
 * on the ground.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::land_after_impact_bounce(mob* m, void* info1, void* info2) {
    m->fsm.set_state(PIKMIN_STATE_KNOCKED_DOWN);
}


/**
 * @brief When a Pikmin lands after being thrown from a pluck.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::land_after_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* lea_ptr = pik_ptr->focused_mob;
    
    pik_ptr->set_animation(PIKMIN_ANIM_IDLING);
    
    if(lea_ptr) {
        if(lea_ptr->following_group) {
            //If this leader is following another one,
            //then the new Pikmin should be in the group of that top leader.
            lea_ptr = lea_ptr->following_group;
        }
        lea_ptr->add_to_group(pik_ptr);
        pik_ptr->fsm.set_state(PIKMIN_STATE_IN_GROUP_CHASING, info1, info2);
    } else {
        pik_ptr->fsm.set_state(PIKMIN_STATE_IDLING, info1, info2);
    }
}


/**
 * @brief When a thrown Pikmin lands on a mob, to latch on to it.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::land_on_mob(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    hitbox_interaction* info = (hitbox_interaction*) info1;
    mob* m2_ptr = info->mob2;
    
    mob_event* m2_pik_land_ev =
        m2_ptr->fsm.get_event(MOB_EV_THROWN_PIKMIN_LANDED);
        
    if(m2_pik_land_ev && has_flag(m->flags, MOB_FLAG_WAS_THROWN)) {
        m2_pik_land_ev->run(m2_ptr, (void*)m);
    }
    
    if(!m->can_hurt(m2_ptr)) return;
    
    hitbox* hbox_ptr = info->h2;
    
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
    
    pik_ptr->stop_height_effect();
    pik_ptr->focused_mob = m2_ptr;
    disable_flag(pik_ptr->flags, MOB_FLAG_WAS_THROWN);
    
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
    
    pik_ptr->fsm.set_state(PIKMIN_STATE_MOB_LANDING);
}


/**
 * @brief When a thrown Pikmin lands on a mob, whilst holding something.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::land_on_mob_while_holding(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    engine_assert(!m->holding.empty(), m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    hitbox_interaction* info = (hitbox_interaction*) info1;
    tool* too_ptr = (tool*) (*m->holding.begin());
    mob* m2_ptr = info->mob2;
    
    if(!m->can_hurt(m2_ptr)) return;
    
    mob_event* m2_pik_land_ev =
        m2_ptr->fsm.get_event(MOB_EV_THROWN_PIKMIN_LANDED);
        
    if(m2_pik_land_ev && has_flag(m->flags, MOB_FLAG_WAS_THROWN)) {
        m2_pik_land_ev->run(m2_ptr, (void*)m);
    }
    
    disable_flag(pik_ptr->flags, MOB_FLAG_WAS_THROWN);
    
    if(too_ptr->too_type->dropped_when_pikmin_lands_on_opponent) {
        pikmin_fsm::release_tool(m, nullptr, nullptr);
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        
        if(too_ptr->too_type->stuck_when_pikmin_lands_on_opponent && info->h2) {
            too_ptr->speed.x = too_ptr->speed.y = too_ptr->speed_z = 0;
            too_ptr->stop_height_effect();
            
            too_ptr->focused_mob = m2_ptr;
            
            float h_offset_dist;
            float h_offset_angle;
            float v_offset_dist;
            m2_ptr->get_hitbox_hold_point(
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
                m->fsm.set_state(
                    PIKMIN_STATE_CALLED_H, game.states.gameplay->cur_leader_ptr
                );
            } else {
                m->fsm.set_state(
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
void pikmin_fsm::land_while_holding(mob* m, void* info1, void* info2) {
    engine_assert(!m->holding.empty(), m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    tool* too_ptr = (tool*) * (m->holding.begin());
    
    pikmin_fsm::stand_still(m, nullptr, nullptr);
    
    pik_ptr->is_tool_primed_for_whistle = true;
    
    m->set_animation(PIKMIN_ANIM_IDLING);
    
    if(too_ptr->too_type->dropped_when_pikmin_lands) {
        pikmin_fsm::release_tool(m, nullptr, nullptr);
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        
        if(
            too_ptr->too_type->pikmin_returns_after_using &&
            game.states.gameplay->cur_leader_ptr
        ) {
            if(
                !pik_ptr->holding.empty() &&
                pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
            ) {
                m->fsm.set_state(
                    PIKMIN_STATE_CALLED_H, game.states.gameplay->cur_leader_ptr
                );
            } else {
                m->fsm.set_state(
                    PIKMIN_STATE_CALLED, game.states.gameplay->cur_leader_ptr
                );
            }
        }
    } else {
        m->fsm.set_state(PIKMIN_STATE_IDLING_H);
    }
}


/**
 * @brief When a Pikmin leaves its Onion because it got called out.
 *
 * @param m The mob.
 * @param info1 Points to the Onion.
 * @param info2 Unused.
 */
void pikmin_fsm::leave_onion(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->set_animation(PIKMIN_ANIM_SLIDING);
}


/**
 * @brief When a Pikmin leaves a hazardous sector.
 *
 * @param m The mob.
 * @param info1 Points to the hazard.
 * @param info2 Unused.
 */
void pikmin_fsm::left_hazard(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    hazard* h = (hazard*) info1;
    if(h->associated_liquid) {
        m->remove_particle_generator(MOB_PARTICLE_GENERATOR_ID_WAVE_RING);
    }
}


/**
 * @brief When the mob the Pikmin is latched on to disappears.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::lose_latched_mob(mob* m, void* info1, void* info2) {
    m->stop_chasing();
}


/**
 * @brief When a Pikmin notifies the leader that it must gently release it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::notify_leader_release(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = ((pikmin*) m);
    if(!pik_ptr->following_group) return;
    if(pik_ptr->holder.m != pik_ptr->following_group) return;
    pik_ptr->following_group->fsm.run_event(MOB_EV_RELEASE_ORDER);
}


/**
 * @brief When a Pikmin needs to decide a new spot to run off to whilst
 * in panicking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::panic_new_chase(mob* m, void* info1, void* info2) {
    m->chase(
        point(
            m->pos.x + randomf(-1000, 1000),
            m->pos.y + randomf(-1000, 1000)
        ),
        m->z
    );
    m->set_timer(PIKMIN::PANIC_CHASE_INTERVAL);
}


/**
 * @brief When a Pikmin is meant to reel back to unleash an attack.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::prepare_to_attack(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    pikmin* p = (pikmin*) m;
    p->was_last_hit_dud = false;
    
    if(p->focused_mob->rectangular_dim.x != 0.0f) {
        bool is_inside = false;
        point target =
            get_closest_point_in_rotated_rectangle(
                m->pos,
                m->focused_mob->pos,
                m->focused_mob->rectangular_dim,
                m->focused_mob->angle,
                &is_inside
            );
        p->face(get_angle(m->pos, target), nullptr);
        
    } else {
        p->face(0, &p->focused_mob->pos);
        
    }
    
    p->set_animation(PIKMIN_ANIM_ATTACKING);
}


/**
 * @brief When a Pikmin reaches its spot on a carriable object.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::reach_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* carriable_mob = pik_ptr->carrying_mob;
    
    point spot_offset =
        rotate_point(
            carriable_mob->carry_info->spot_info[pik_ptr->temp_i].pos,
            carriable_mob->angle
        );
    point final_pos = carriable_mob->pos + spot_offset;
    
    pik_ptr->chase(
        &carriable_mob->pos, &carriable_mob->z,
        spot_offset, 0.0f,
        CHASE_FLAG_TELEPORT |
        CHASE_FLAG_TELEPORTS_CONSTANTLY
    );
    
    pik_ptr->face(get_angle(final_pos, carriable_mob->pos), nullptr);
    
    //Let the carriable mob know that a new Pikmin has grabbed on.
    pik_ptr->carrying_mob->fsm.run_event(
        MOB_EV_CARRIER_ADDED, (void*) pik_ptr
    );
    
    pik_ptr->in_carry_struggle_animation = false;
    pik_ptr->set_animation(PIKMIN_ANIM_CARRYING);
}


/**
 * @brief When a Pikmin reaches its dismissal spot.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::reach_dismiss_spot(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_IDLING);
}


/**
 * @brief When a Pikmin that just attacked an opponent needs to walk
 * towards it again.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::rechase_opponent(mob* m, void* info1, void* info2) {

    pikmin* pik_ptr = (pikmin*) m;
    
    if(pik_ptr->was_last_hit_dud) {
        //Check if the Pikmin's last hits were duds.
        //If so, maybe give up and sigh.
        pik_ptr->consecutive_dud_hits++;
        if(pik_ptr->consecutive_dud_hits >= 4) {
            pik_ptr->consecutive_dud_hits = 0;
            pik_ptr->fsm.set_state(PIKMIN_STATE_SIGHING);
            return;
        }
    }
    
    bool can_continue_attacking =
        m->focused_mob &&
        m->focused_mob->health > 0 &&
        dist(m->pos, m->focused_mob->pos) <=
        (m->radius + m->focused_mob->radius + PIKMIN::GROUNDED_ATTACK_DIST);
        
    if(!can_continue_attacking) {
        //The opponent cannot be chased down. Become idle.
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        
    } else if(randomf(0.0f, 1.0f) <= PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED) {
        //Circle around it a bit before attacking from a new angle.
        pik_ptr->fsm.set_state(PIKMIN_STATE_CIRCLING_OPPONENT);
        
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
void pikmin_fsm::release_tool(mob* m, void* info1, void* info2) {
    if(m->holding.empty()) return;
    pikmin* pik_ptr = (pikmin*) m;
    mob* too_ptr = *m->holding.begin();
    
    if(info1) {
        too_ptr->set_var("gentle_release", "true");
    } else {
        too_ptr->set_var("gentle_release", "false");
    }
    pik_ptr->release(too_ptr);
    too_ptr->pos = m->pos;
    too_ptr->speed = point();
    too_ptr->push_amount = 0.0f;
    m->subgroup_type_ptr =
        game.states.gameplay->subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, pik_ptr->pik_type
        );
    if(m->following_group) {
        m->following_group->group->change_standby_type_if_needed();
        game.states.gameplay->update_closest_group_members();
    }
}


/**
 * @brief When a Pikmin seed lands on the ground.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::seed_landed(mob* m, void* info1, void* info2) {
    //Generate the rock particles that come out.
    particle pa(
        m->pos, m->z + m->get_drawing_height(),
        4, 1, PARTICLE_PRIORITY_LOW
    );
    pa.bitmap = game.sys_assets.bmp_rock;
    pa.color.set_keyframe_value(0, al_map_rgb(160, 80, 32));
    pa.color.add(1, al_map_rgba(160, 80, 32, 0));
    pa.outwards_speed = keyframe_interpolator<float>(50);
    pa.linear_speed.add(1, point(0, 50));
    particle_generator pg(0, pa, 8);
    pg.emission.number_deviation = 1;
    pg.size_deviation = 2;
    pg.outwards_speed_deviation = 10;
    pg.duration_deviation = 0.25;
    pg.emit(game.states.gameplay->particles);
}


/**
 * @brief When a Pikmin is meant to set its timer for the bump lock.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::set_bump_lock(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    pik_ptr->bump_lock = game.config.idle_bump_delay;
}


/**
 * @brief When a Pikmin is meant to change "reach" to the idle task reach.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::set_idle_task_reach(mob* m, void* info1, void* info2) {
    m->near_reach = 0;
    m->update_interaction_span();
}


/**
 * @brief When a Pikmin is meant to change "reach" to the swarm reach.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::set_swarm_reach(mob* m, void* info1, void* info2) {
    m->near_reach = 1;
    m->update_interaction_span();
}


/**
 * @brief When a Pikmin is meant to sigh.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::sigh(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_SIGHING);
}


/**
 * @brief Causes a sprout to evolve.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::sprout_evolve(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(p->maturity == 0 || p->maturity == 1) {
        //Leaf to bud, or bud to flower.
        
        p->maturity++;
        
        //Generate a burst of particles to symbolize the maturation.
        particle pa(
            m->pos, m->z + m->get_drawing_height(),
            16, 1, PARTICLE_PRIORITY_LOW
        );
        pa.bitmap = game.sys_assets.bmp_sparkle;
        pa.color.add(1, change_alpha(COLOR_WHITE, 0));
        pa.outwards_speed = keyframe_interpolator<float>(50);
        particle_generator pg(0, pa, 8);
        pg.emission.number_deviation = 1;
        pg.size_deviation = 8;
        pg.linear_speed_deviation.x = 10;
        pg.duration_deviation = 0.25;
        pg.emit(game.states.gameplay->particles);
        
    } else {
        //Flower to leaf.
        
        p->maturity = 0;
        
        //Generate a dribble of particles to symbolize the regression.
        particle pa(
            m->pos, m->z + m->get_drawing_height(),
            16, 1, PARTICLE_PRIORITY_LOW
        );
        pa.bitmap = game.sys_assets.bmp_sparkle;
        pa.color.set_keyframe_value(0, al_map_rgb(255, 224, 224));
        pa.color.add(1, al_map_rgb(255, 224, 224));
        pa.outwards_speed = keyframe_interpolator<float>(50);
        pa.linear_speed.add(1, point(0, 300));
        particle_generator pg(0, pa, 8);
        pg.emission.number_deviation = 1;
        pg.size_deviation = 8;
        pg.outwards_speed_deviation = 10;
        pg.duration_deviation = 0.25;
        pg.emit(game.states.gameplay->particles);
    }
}


/**
 * @brief Schedules the next evolution for a sprout.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::sprout_schedule_evol(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    m->set_timer(p->pik_type->sprout_evolution_time[p->maturity]);
}


/**
 * @brief When a Pikmin is meant to stand still in place.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stand_still(mob* m, void* info1, void* info2) {
    m->stop_circling();
    m->stop_following_path();
    m->stop_chasing();
    m->stop_turning();
    m->speed.x = m->speed.y = 0;
}


/**
 * @brief When a Pikmin should start a random boredom animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_boredom_anim(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    size_t looking_around_anim_idx =
        m->type->anim_db->find_animation("looking_around");
    size_t sitting_anim_idx =
        m->type->anim_db->find_animation("sitting");
    size_t lounging_anim_idx =
        m->type->anim_db->find_animation("lounging");
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
    size_t anim_idx = boredom_anims[randomi(0, boredom_anims.size() - 1)];
    m->set_animation(anim_idx, START_ANIM_OPTION_NORMAL, false);
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
void pikmin_fsm::start_chasing_leader(mob* m, void* info1, void* info2) {
    m->focus_on_mob(m->following_group);
    pikmin_fsm::update_in_group_chasing(m, nullptr, nullptr);
    m->set_animation(
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
void pikmin_fsm::start_drinking(mob* m, void* info1, void* info2) {
    mob* drop_ptr = (mob*) info1;
    m->leave_group();
    m->stop_chasing();
    m->focus_on_mob(drop_ptr);
    m->face(get_angle(m->pos, drop_ptr->pos), nullptr);
    m->set_animation(PIKMIN_ANIM_DRINKING);
}


/**
 * @brief When a Pikmin starts flailing.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_flailing(mob* m, void* info1, void* info2) {
    pikmin_fsm::release_tool(m, nullptr, nullptr);
    
    //If the Pikmin is following a moveable point, let's change it to
    //a static point. This will make the Pikmin continue to move
    //forward into the water in a straight line.
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    float final_z = 0.0f;
    point final_pos = m->get_chase_target(&final_z);
    m->chase(final_pos, final_z);
    
    m->leave_group();
    
    //Let the Pikmin continue to swim into the water for a bit
    //before coming to a stop. Otherwise the Pikmin would stop nearly
    //on the edge of the water, and that just looks bad.
    m->set_timer(1.0f);
    
    m->set_animation(PIKMIN_ANIM_FLAILING, START_ANIM_OPTION_RANDOM_TIME);
}


/**
 * @brief When a Pikmin starts getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_getting_up(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->set_animation(PIKMIN_ANIM_GETTING_UP);
}


/**
 * @brief When a Pikmin starts lunging forward for an impact attack.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_impact_lunge(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    m->chase(&m->focused_mob->pos, &m->focused_mob->z);
    m->set_animation(PIKMIN_ANIM_ATTACKING);
}


/**
 * @brief When a Pikmin lands on a mob and needs to start its landing animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_mob_landing(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_MOB_LANDING);
}


/**
 * @brief When a Pikmin starts panicking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_panicking(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    m->leave_group();
    pikmin_fsm::panic_new_chase(m, info1, info2);
    m->set_animation(
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
void pikmin_fsm::start_picking_up(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_PICKING_UP);
}


/**
 * @brief When a Pikmin must start returning to the carried object's
 * return point.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob that used to be carried.
 * @param info2 Unused.
 */
void pikmin_fsm::start_returning(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    mob* carried_mob = (mob*) info1;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    path_follow_settings settings;
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
            bridge* bri_ptr = (bridge*) carried_mob->carry_info->intended_mob;
            enable_flag(settings.flags, PATH_FOLLOW_FLAG_FAKED_START);
            settings.faked_start = bri_ptr->get_start_point();
        }
    }
    
    if(
        pik_ptr->follow_path(
            settings, pik_ptr->get_base_speed(), pik_ptr->type->acceleration
        )
    ) {
        m->set_animation(
            PIKMIN_ANIM_WALKING, START_ANIM_OPTION_NORMAL, true,
            m->type->move_speed
        );
    } else {
        pik_ptr->fsm.set_state(PIKMIN_STATE_IDLING);
    }
}


/**
 * @brief When a Pikmin starts riding on a track.
 *
 * @param m The mob.
 * @param info1 Points to the track mob.
 * @param info2 Unused.
 */
void pikmin_fsm::start_riding_track(mob* m, void* info1, void* info2) {
    track* tra_ptr = (track*) info1;
    
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->leave_group();
    m->stop_chasing();
    m->focus_on_mob(tra_ptr);
    m->start_height_effect();
    
    vector<size_t> checkpoints;
    for(size_t c = 0; c < tra_ptr->type->anim_db->body_parts.size(); c++) {
        checkpoints.push_back(c);
    }
    m->track_info =
        new track_t(
        tra_ptr, checkpoints, tra_ptr->tra_type->ride_speed
    );
    
    switch(tra_ptr->tra_type->riding_pose) {
    case TRACK_RIDING_POSE_STOPPED: {
        m->set_animation(PIKMIN_ANIM_WALKING);
        break;
    } case TRACK_RIDING_POSE_CLIMBING: {
        m->set_animation(PIKMIN_ANIM_CLIMBING);
        break;
    } case TRACK_RIDING_POSE_SLIDING: {
        m->set_animation(PIKMIN_ANIM_SLIDING);
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
void pikmin_fsm::stop_being_idle(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
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
void pikmin_fsm::stop_being_thrown(mob* m, void* info1, void* info2) {
    m->remove_particle_generator(MOB_PARTICLE_GENERATOR_ID_THROW);
}


/**
 * @brief When a Pikmin is meant to release an object it is carrying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stop_carrying(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(!p->carrying_mob) return;
    
    p->carrying_mob->fsm.run_event(MOB_EV_CARRIER_REMOVED, (void*) p);
    
    p->carrying_mob = nullptr;
    p->set_timer(0);
}


/**
 * @brief When a Pikmin stands still while in a leader's group.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stop_in_group(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    m->stop_chasing();
    m->face(0, &m->following_group->pos);
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->set_animation(PIKMIN_ANIM_IDLING);
    m->set_timer(
        randomf(PIKMIN::BORED_ANIM_MIN_DELAY, PIKMIN::BORED_ANIM_MAX_DELAY)
    );
}


/**
 * @brief When a Pikmin has to choose its carrying animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tick_carrying(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    if(
        pik_ptr->in_carry_struggle_animation &&
        pik_ptr->carrying_mob->carry_info->is_moving
    ) {
        pik_ptr->in_carry_struggle_animation = false;
        pik_ptr->set_animation(PIKMIN_ANIM_CARRYING);
    } else if(
        !pik_ptr->in_carry_struggle_animation &&
        !pik_ptr->carrying_mob->carry_info->is_moving
    ) {
        pik_ptr->in_carry_struggle_animation = true;
        pik_ptr->set_animation(PIKMIN_ANIM_CARRYING_STRUGGLE);
    }
}


/**
 * @brief When a Pikmin has to teleport to its spot in the Onion leg.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tick_entering_onion(mob* m, void* info1, void* info2) {
    engine_assert(m->track_info != nullptr, m->print_state_history());
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    if(m->tick_track_ride()) {
        //Finished!
        ((onion*) m->focused_mob)->nest->store_pikmin((pikmin*) m);
    }
}


/**
 * @brief When a Pikmin has to teleport to its spot in a group task.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tick_group_task_work(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    group_task* tas_ptr = (group_task*) (m->focused_mob);
    point cur_spot_pos = tas_ptr->get_spot_pos(pik_ptr);
    float cur_spot_z = tas_ptr->z + tas_ptr->tas_type->spots_z;
    
    pik_ptr->chase(
        cur_spot_pos, cur_spot_z,
        CHASE_FLAG_TELEPORT |
        CHASE_FLAG_TELEPORTS_CONSTANTLY
    );
    pik_ptr->face(
        tas_ptr->angle + tas_ptr->tas_type->worker_pikmin_angle, nullptr, true
    );
    pik_ptr->stop_turning();
}


/**
 * @brief When a Pikmin has to teleport to its spot in a track it is riding.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tick_track_ride(mob* m, void* info1, void* info2) {
    engine_assert(m->track_info != nullptr, m->print_state_history());
    pikmin* pik_ptr = (pikmin*) m;
    
    if(m->tick_track_ride()) {
        //Finished!
        m->fsm.set_state(PIKMIN_STATE_IDLING, nullptr, nullptr);
        if(
            pik_ptr->leader_to_return_to &&
            !pik_ptr->leader_to_return_to->to_delete &&
            pik_ptr->leader_to_return_to->health > 0.0f
        ) {
            if(
                !pik_ptr->holding.empty() &&
                pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
            ) {
                m->fsm.set_state(
                    PIKMIN_STATE_CALLED_H, pik_ptr->leader_to_return_to, info2
                );
            } else {
                m->fsm.set_state(
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
void pikmin_fsm::touched_eat_hitbox(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    engine_assert(info2 != nullptr, m->print_state_history());
    
    if(m->invuln_period.time_left > 0) return;
    if(m->health <= 0) {
        return;
    }
    
    for(size_t s = 0; s < m->statuses.size(); s++) {
        if(m->statuses[s].type->turns_inedible) {
            return;
        }
    }
    
    m->fsm.set_state(PIKMIN_STATE_GRABBED_BY_ENEMY, info1, info2);
}


/**
 * @brief When a Pikmin touches a hazard.
 *
 * @param m The mob.
 * @param info1 Pointer to the hazard type.
 * @param info2 Pointer to the hitbox that caused this, if any.
 */
void pikmin_fsm::touched_hazard(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
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
            particle par(
                m->pos, m->z,
                0, 1, PARTICLE_PRIORITY_LOW
            );
            par.bitmap = game.sys_assets.bmp_wave_ring;
            par.size.add(1, m->radius * 4);
            par.color.add(1, change_alpha(COLOR_WHITE, 0));
            particle_generator pg(0.3, par, 1);
            pg.follow_mob = m;
            pg.id = MOB_PARTICLE_GENERATOR_ID_WAVE_RING;
            m->particle_generators.push_back(pg);
        }
    }
    
    if(p->invuln_period.time_left > 0) return;
    mob_type::vulnerability_t vuln = p->get_hazard_vulnerability(h);
    if(vuln.damage_mult == 0.0f) return;
    
    if(!vuln.status_to_apply || !vuln.status_overrides) {
        for(size_t e = 0; e < h->effects.size(); e++) {
            p->apply_status_effect(h->effects[e], false, true);
        }
    }
    if(vuln.status_to_apply) {
        p->apply_status_effect(vuln.status_to_apply, false, true);
    }
}


/**
 * @brief When a Pikmin is sprayed.
 *
 * @param m The mob.
 * @param info1 Pointer to the spray type.
 * @param info2 Unused.
 */
void pikmin_fsm::touched_spray(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); e++) {
        m->apply_status_effect(s->effects[e], false, false);
    }
    
    if(s->buries_pikmin) {
        m->fsm.set_state(PIKMIN_STATE_SPROUT, nullptr, nullptr);
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
void pikmin_fsm::try_held_item_hotswap(mob* m, void* info1, void* info2) {
    assert(!m->holding.empty());
    
    tool* too_ptr = (tool*) * (m->holding.begin());
    if(
        !too_ptr->too_type->can_be_hotswapped &&
        has_flag(too_ptr->holdability_flags, HOLDABILITY_FLAG_ENEMIES)
    ) {
        //This tool can't be hotswapped... The Pikmin has to get chomped.
        pikmin_fsm::release_tool(m, nullptr, nullptr);
        m->fsm.set_state(PIKMIN_STATE_GRABBED_BY_ENEMY);
        return;
    }
    
    //Start by dropping the tool.
    pikmin_fsm::release_tool(m, nullptr, nullptr);
    //Receive some invulnerability period to make sure it's not hurt by
    //the same attack.
    m->invuln_period.start();
    //Finally, get knocked back on purpose.
    m->leave_group();
    pikmin_fsm::be_released(m, info1, info2);
    pikmin_fsm::notify_leader_release(m, info1, info2);
    m->fsm.set_state(PIKMIN_STATE_KNOCKED_BACK);
}


/**
 * @brief When the Pikmin stops latching on to an enemy.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::unlatch(mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    m->focused_mob->release(m);
    ((pikmin*) m)->latched = false;
}


/**
 * @brief When the Pikmin should update its destination when chasing the leader.
 *
 * @param m The mob.
 * @param info1 Points to the position struct with the final destination.
 *   If nullptr, the final destination is calculated in this function.
 * @param info2 Unused.
 */
void pikmin_fsm::update_in_group_chasing(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    point target_pos;
    float target_dist; //Unused dummy value.
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    if(!info1) {
        pik_ptr->get_group_spot_info(&target_pos, &target_dist);
    } else {
        target_pos = *((point*) info1);
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
void pikmin_fsm::whistled_while_holding(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    tool* too_ptr = (tool*) * (m->holding.begin());
    
    if(
        too_ptr->too_type->dropped_when_pikmin_is_whistled &&
        pik_ptr->is_tool_primed_for_whistle
    ) {
        pikmin_fsm::release_tool(m, nullptr, nullptr);
    }
    
    pik_ptr->is_tool_primed_for_whistle = false;
    
    if(
        !pik_ptr->holding.empty() &&
        pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
    ) {
        m->fsm.set_state(PIKMIN_STATE_CALLED_H, info1, info2);
    } else {
        m->fsm.set_state(PIKMIN_STATE_CALLED, info1, info2);
    }
    
}


/**
 * @brief When a Pikmin is whistled over by a leader while riding on a track.
 *
 * @param m The mob.
 * @param info1 Pointer to the leader that called.
 * @param info2 Unused.
 */
void pikmin_fsm::whistled_while_riding(mob* m, void* info1, void* info2) {
    engine_assert(m->track_info, m->print_state_history());
    
    pikmin* pik_ptr = (pikmin*) m;
    track* tra_ptr = (track*) (m->track_info->m);
    
    if(tra_ptr->tra_type->cancellable_with_whistle) {
        m->stop_track_ride();
        if(
            !pik_ptr->holding.empty() &&
            pik_ptr->holding[0]->type->category->id == MOB_CATEGORY_TOOLS
        ) {
            m->fsm.set_state(PIKMIN_STATE_CALLED_H, info1, info2);
        } else {
            m->fsm.set_state(PIKMIN_STATE_CALLED, info1, info2);
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
void pikmin_fsm::work_on_group_task(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    group_task* tas_ptr = (group_task*) (m->focused_mob);
    pikmin* pik_ptr = (pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    tas_ptr->add_worker(pik_ptr);
    
    pik_ptr->stop_chasing();
    pik_ptr->face(
        tas_ptr->angle + tas_ptr->tas_type->worker_pikmin_angle,
        nullptr
    );
    
    switch(tas_ptr->tas_type->worker_pikmin_pose) {
    case GROUP_TASK_PIKMIN_POSE_STOPPED: {
        pik_ptr->set_animation(PIKMIN_ANIM_IDLING);
        break;
    }
    case GROUP_TASK_PIKMIN_POSE_ARMS_OUT: {
        pik_ptr->set_animation(PIKMIN_ANIM_ARMS_OUT);
        break;
    }
    case GROUP_TASK_PIKMIN_POSE_PUSHING: {
        pik_ptr->set_animation(PIKMIN_ANIM_PUSHING);
        break;
    }
    case GROUP_TASK_PIKMIN_POSE_CARRYING: {
        pik_ptr->set_animation(PIKMIN_ANIM_CARRYING);
        break;
    }
    }
}
