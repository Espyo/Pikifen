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
void pikmin_fsm::create_fsm(MobType* typ) {
    EasyFsmCreator efc;
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_SWALLOWED); {
            efc.run(pikmin_fsm::start_dying);
            efc.run(pikmin_fsm::finish_dying);
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
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.run(pikmin_fsm::be_thrown_by_bouncer);
            efc.change_state("thrown");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("knocked_down_dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
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
        efc.new_event(MOB_EV_ZERO_HEALTH); {
            efc.change_state("dying");
        }
    }
    
    efc.new_state("crushed", PIKMIN_STATE_CRUSHED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::be_crushed);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finish_dying);
        }
    }
    
    efc.new_state("knocked_down_dying", PIKMIN_STATE_KNOCKED_DOWN_DYING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_knocked_down_dying);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finish_dying);
        }
    }
    
    efc.new_state("dying", PIKMIN_STATE_DYING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(pikmin_fsm::start_dying);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.run(pikmin_fsm::finish_dying);
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
void pikmin_fsm::be_attacked(Mob* m, void* info1, void* info2) {
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
            unsigned char wither_roll = game.rng.i(0, 100);
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
void pikmin_fsm::be_crushed(Mob* m, void* info1, void* info2) {
    pikmin_fsm::start_dying(m, info1, info2);
    m->z = m->ground_sector->z;
    m->set_animation(PIKMIN_ANIM_CRUSHED);
}


/**
 * @brief When a Pikmin is dismissed by its leader.
 *
 * @param m The mob.
 * @param info1 Pointer to the world coordinates to go to.
 * @param info2 Unused.
 */
void pikmin_fsm::be_dismissed(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    engine_assert(info1 != nullptr, m->print_state_history());
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->chase(*((Point*) info1), m->z);
    
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
void pikmin_fsm::be_grabbed_by_enemy(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    engine_assert(info2 != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* ene_ptr = (Mob*) info1;
    Hitbox* hbox_ptr = (Hitbox*) info2;
    
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
void pikmin_fsm::be_grabbed_by_friend(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
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
void pikmin_fsm::be_released(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    ((Pikmin*) m)->is_grabbed_by_enemy = false;
    
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
void pikmin_fsm::be_thrown(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
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
        MobType::Sound* throw_sound =
            &pik_ptr->type->sounds[throw_sound_idx];
        SoundSourceConfig throw_sound_config;
        throw_sound_config.stack_mode = SOUND_STACK_MODE_OVERRIDE;
        game.audio.create_mob_sound_source(
            throw_sound->sample,
            m, false, throw_sound_config
        );
    }
    
    ((Pikmin*) m)->start_throw_trail();
}


/**
 * @brief When a Pikmin is thrown after being plucked.
 *
 * @param m The mob.
 * @param info1 Points to the bouncer mob.
 * @param info2 Unused.
 */
void pikmin_fsm::be_thrown_after_pluck(Mob* m, void* info1, void* info2) {
    float throw_angle = get_angle(m->pos, m->focused_mob->pos);
    m->speed_z = PIKMIN::THROW_VER_SPEED;
    m->speed = angle_to_coordinates(throw_angle, PIKMIN::THROW_HOR_SPEED);
    m->face(throw_angle + TAU / 2.0f, nullptr, true);
    
    m->set_animation(PIKMIN_ANIM_PLUCKING_THROWN);
    ((Pikmin*) m)->start_throw_trail();
    
    ParticleGenerator pg =
        standard_particle_gen_setup(
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
void pikmin_fsm::be_thrown_by_bouncer(Mob* m, void* info1, void* info2) {
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->set_animation(PIKMIN_ANIM_THROWN);
    
    ((Pikmin*) m)->start_throw_trail();
}


/**
 * @brief When a Pikmin becomes "helpless".
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::become_helpless(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::become_idle(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::become_sprout(Mob* m, void* info1, void* info2) {
    m->leave_group();
    enable_flag(m->flags, MOB_FLAG_INTANGIBLE);
    enable_flag(m->flags, MOB_FLAG_NON_HUNTABLE);
    enable_flag(m->flags, MOB_FLAG_NON_HURTABLE);
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    ((Pikmin*) m)->is_seed_or_sprout = true;
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
void pikmin_fsm::begin_pluck(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* lea_ptr = (Mob*) info1;
    
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
void pikmin_fsm::called(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* caller = (Mob*) info1;
    
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
void pikmin_fsm::called_while_knocked_down(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* caller = (Mob*) info1;
    
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
void pikmin_fsm::celebrate(Mob* m, void* info1, void* info2) {
    if(game.rng.i(0, 1) == 0) {
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
void pikmin_fsm::check_boredom_anim_end(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(!pik_ptr->in_bored_animation) return;
    m->set_animation(PIKMIN_ANIM_IDLING);
    pik_ptr->in_bored_animation = false;
    m->set_timer(
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
void pikmin_fsm::check_incoming_attack(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::check_leader_bump(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
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
void pikmin_fsm::check_outgoing_attack(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::check_shaking_anim_end(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(!pik_ptr->in_shaking_animation) return;
    m->set_animation(PIKMIN_ANIM_IDLING);
    pik_ptr->in_shaking_animation = false;
    m->set_timer(
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
void pikmin_fsm::circle_opponent(Mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->stop_circling();
    
    float circle_time = game.rng.f(0.0f, 1.0f);
    //Bias the time so that there's a higher chance of picking a close angle,
    //and a lower chance of circling to a distant one. The Pikmin came here
    //to attack, not dance!
    circle_time *= circle_time;
    circle_time += 0.5f;
    m->set_timer(circle_time);
    
    bool go_cw = game.rng.f(0.0f, 1.0f) <= 0.5f;
    m->circle_around(
        m->focused_mob, Point(), m->focused_mob->radius + m->radius, go_cw,
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
void pikmin_fsm::clear_boredom_data(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
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
void pikmin_fsm::clear_timer(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::decide_attack(Mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    if(m->invuln_period.time_left > 0) {
        //Don't let the Pikmin attack while invulnerable. Otherwise, this can
        //be exploited to let Pikmin vulnerable to a hazard attack the obstacle
        //emitting said hazard.
        return;
    }
    
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
        Distance d;
        Hitbox* closest_h =
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
                game.rng.f(0.0f, 1.0f) <=
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
                game.rng.f(0, 1) <=
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
            game.rng.f(0, 1) <=
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
 * @brief When a Pikmin has to bounce back from an impact attack.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::do_impact_bounce(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::enter_onion(Mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Onion* oni_ptr = (Onion*) pik_ptr->focused_mob;
    
    disable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    //Set its data to start climbing.
    vector<size_t> checkpoints;
    checkpoints.push_back((pik_ptr->temp_i * 2) + 1);
    checkpoints.push_back(pik_ptr->temp_i * 2);
    
    pik_ptr->track_info = new TrackRideInfo(
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
void pikmin_fsm::fall_down_pit(Mob* m, void* info1, void* info2) {
    m->start_dying();
    m->finish_dying();
}


/**
 * @brief When a Pikmin finished the animation for when it's called.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finish_called_anim(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* lea_ptr = pik_ptr->focused_mob;
    
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
void pikmin_fsm::finish_carrying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    engine_assert(pik_ptr->carrying_mob != nullptr, m->print_state_history());
    
    if(pik_ptr->carrying_mob->carry_info->must_return) {
        //The Pikmin should return somewhere (like a pile).
        pik_ptr->fsm.set_state(PIKMIN_STATE_RETURNING, (void*) pik_ptr->carrying_mob);
        
    } else {
        //The Pikmin can just sit and chill.
        pik_ptr->fsm.set_state(PIKMIN_STATE_CELEBRATING);
    }
}


/**
 * @brief When a Pikmin finishes drinking the drop it was drinking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finish_drinking(Mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    Pikmin* pik_ptr = (Pikmin*) m;
    Drop* dro_ptr = (Drop*) m->focused_mob;
    
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
 * @brief When a Pikmin finishes dying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finish_dying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    pik_ptr->finish_dying();
}


/**
 * @brief When a Pikmin finishes getting up from being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::finish_getting_up(Mob* m, void* info1, void* info2) {
    Mob* prev_focused_mob = m->focused_mob;
    
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
void pikmin_fsm::finish_mob_landing(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::finish_picking_up(Mob* m, void* info1, void* info2) {
    Tool* too_ptr = (Tool*) (m->focused_mob);
    
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
void pikmin_fsm::flail_to_leader(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::forget_carriable_object(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::forget_group_task(Mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    GroupTask* tas_ptr = (GroupTask*) (m->focused_mob);
    Pikmin* pik_ptr = (Pikmin*) m;
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
void pikmin_fsm::forget_tool(Mob* m, void* info1, void* info2) {
    if(!m->focused_mob) return;
    
    Tool* too_ptr = (Tool*) (m->focused_mob);
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
void pikmin_fsm::get_knocked_back(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::get_knocked_down(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::go_to_carriable_object(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Mob* carriable_mob = (Mob*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    pik_ptr->carrying_mob = carriable_mob;
    pik_ptr->leave_group();
    pik_ptr->stop_chasing();
    
    size_t closest_spot = INVALID;
    Distance closest_spot_dist;
    CarrierSpot* closest_spot_ptr = nullptr;
    Point closest_spot_offset;
    
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
        CarrierSpot* spot_ptr =
            &carriable_mob->carry_info->spot_info[s];
        if(spot_ptr->state != CARRY_SPOT_STATE_FREE) continue;
        
        Point spot_offset =
            rotate_point(spot_ptr->pos, carriable_mob->angle);
        Distance d(pik_ptr->pos, carriable_mob->pos + spot_offset);
        
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
void pikmin_fsm::go_to_group_task(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    GroupTask* tas_ptr = (GroupTask*) info1;
    Pikmin* pik_ptr = (Pikmin*) m;
    
    if(
        !has_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR) &&
        tas_ptr->tas_type->flying_pikmin_only
    ) {
        //Only flying Pikmin can use this, and this Pikmin doesn't fly.
        return;
    }
    
    GroupTask::GroupTaskSpot* free_spot = tas_ptr->get_free_spot();
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
        Point(), tas_ptr->tas_type->spots_z
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
void pikmin_fsm::go_to_onion(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    PikminNest* nest_ptr = (PikminNest*) info1;
    
    //Pick a leg at random.
    pik_ptr->temp_i =
        game.rng.i(
            0, (int) (nest_ptr->nest_type->leg_body_parts.size() / 2) - 1
        );
    size_t leg_foot_bp_idx =
        nest_ptr->m_ptr->anim.anim_db->find_body_part(
            nest_ptr->nest_type->leg_body_parts[pik_ptr->temp_i * 2 + 1]
        );
    Point coords =
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
void pikmin_fsm::go_to_opponent(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    
    Mob* other_ptr = (Mob*) info1;
    if(other_ptr->type->category->id == MOB_CATEGORY_ENEMIES) {
        Enemy* ene_ptr = (Enemy*) info1;
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
    
    Point offset = Point();
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
void pikmin_fsm::go_to_tool(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
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
        Point(), 0.0f, 0,
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
void pikmin_fsm::going_to_dismiss_spot(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::land(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::land_after_impact_bounce(Mob* m, void* info1, void* info2) {
    m->fsm.set_state(PIKMIN_STATE_KNOCKED_DOWN);
}


/**
 * @brief When a Pikmin lands after being thrown from a pluck.
 *
 * @param m The mob.
 * @param info1 Pointer to the hitbox touch information structure.
 * @param info2 Unused.
 */
void pikmin_fsm::land_after_pluck(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* lea_ptr = pik_ptr->focused_mob;
    
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
void pikmin_fsm::land_on_mob(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Mob* m2_ptr = info->mob2;
    
    MobEvent* m2_pik_land_ev =
        m2_ptr->fsm.get_event(MOB_EV_THROWN_PIKMIN_LANDED);
        
    if(m2_pik_land_ev && has_flag(m->flags, MOB_FLAG_WAS_THROWN)) {
        m2_pik_land_ev->run(m2_ptr, (void*)m);
    }
    
    if(!m->can_hurt(m2_ptr)) return;
    
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
void pikmin_fsm::land_on_mob_while_holding(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    engine_assert(!m->holding.empty(), m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    HitboxInteraction* info = (HitboxInteraction*) info1;
    Tool* too_ptr = (Tool*) (*m->holding.begin());
    Mob* m2_ptr = info->mob2;
    
    if(!m->can_hurt(m2_ptr)) return;
    
    MobEvent* m2_pik_land_ev =
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
void pikmin_fsm::land_while_holding(Mob* m, void* info1, void* info2) {
    engine_assert(!m->holding.empty(), m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Tool* too_ptr = (Tool*) * (m->holding.begin());
    
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
void pikmin_fsm::leave_onion(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::left_hazard(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Hazard* h = (Hazard*) info1;
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
void pikmin_fsm::lose_latched_mob(Mob* m, void* info1, void* info2) {
    m->stop_chasing();
}


/**
 * @brief When a Pikmin notifies the leader that it must gently release it.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::notify_leader_release(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = ((Pikmin*) m);
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
void pikmin_fsm::panic_new_chase(Mob* m, void* info1, void* info2) {
    m->chase(
        Point(
            m->pos.x + game.rng.f(-1000, 1000),
            m->pos.y + game.rng.f(-1000, 1000)
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
void pikmin_fsm::prepare_to_attack(Mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    pik_ptr->was_last_hit_dud = false;
    
    if(pik_ptr->focused_mob->rectangular_dim.x != 0.0f) {
        bool is_inside = false;
        Point target =
            get_closest_point_in_rotated_rectangle(
                m->pos,
                m->focused_mob->pos,
                m->focused_mob->rectangular_dim,
                m->focused_mob->angle,
                &is_inside
            );
        pik_ptr->face(get_angle(m->pos, target), nullptr);
        
    } else {
        pik_ptr->face(0, &pik_ptr->focused_mob->pos);
        
    }
    
    pik_ptr->set_animation(PIKMIN_ANIM_ATTACKING);
}


/**
 * @brief When a Pikmin reaches its spot on a carriable object.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::reach_carriable_object(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* carriable_mob = pik_ptr->carrying_mob;
    
    Point spot_offset =
        rotate_point(
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
void pikmin_fsm::reach_dismiss_spot(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::rechase_opponent(Mob* m, void* info1, void* info2) {

    Pikmin* pik_ptr = (Pikmin*) m;
    
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
        Distance(m->pos, m->focused_mob->pos) <=
        (m->radius + m->focused_mob->radius + PIKMIN::GROUNDED_ATTACK_DIST);
        
    if(!can_continue_attacking) {
        //The opponent cannot be chased down. Become idle.
        m->fsm.set_state(PIKMIN_STATE_IDLING);
        
    } else if(game.rng.f(0.0f, 1.0f) <= PIKMIN::CIRCLE_OPPONENT_CHANCE_GROUNDED) {
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
void pikmin_fsm::release_tool(Mob* m, void* info1, void* info2) {
    if(m->holding.empty()) return;
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* too_ptr = *m->holding.begin();
    
    if(info1) {
        too_ptr->set_var("gentle_release", "true");
    } else {
        too_ptr->set_var("gentle_release", "false");
    }
    pik_ptr->release(too_ptr);
    too_ptr->pos = m->pos;
    too_ptr->speed = Point();
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
void pikmin_fsm::seed_landed(Mob* m, void* info1, void* info2) {
    //Generate the rock particles that come out.
    ParticleGenerator pg =
        standard_particle_gen_setup(
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
void pikmin_fsm::set_bump_lock(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    pik_ptr->bump_lock = game.config.idle_bump_delay;
}


/**
 * @brief When a Pikmin is meant to change "reach" to the idle task reach.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::set_idle_task_reach(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::set_swarm_reach(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::sigh(Mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_SIGHING);
}


/**
 * @brief Causes a sprout to evolve.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::sprout_evolve(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(pik_ptr->maturity == 0 || pik_ptr->maturity == 1) {
        //Leaf to bud, or bud to flower.
        
        pik_ptr->maturity++;
        
        ParticleGenerator pg =
            standard_particle_gen_setup(
                game.sys_content_names.part_sprout_evolution, pik_ptr
            );
        pik_ptr->particle_generators.push_back(pg);
        
    } else {
        //Flower to leaf.
        
        pik_ptr->maturity = 0;
        
        ParticleGenerator pg =
            standard_particle_gen_setup(
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
void pikmin_fsm::sprout_schedule_evol(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    m->set_timer(pik_ptr->pik_type->sprout_evolution_time[pik_ptr->maturity]);
}


/**
 * @brief When a Pikmin is meant to stand still in place.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stand_still(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::start_boredom_anim(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
    size_t anim_idx = boredom_anims[game.rng.i(0, (int) (boredom_anims.size() - 1))];
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
void pikmin_fsm::start_chasing_leader(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::start_drinking(Mob* m, void* info1, void* info2) {
    Mob* drop_ptr = (Mob*) info1;
    m->leave_group();
    m->stop_chasing();
    m->focus_on_mob(drop_ptr);
    m->face(get_angle(m->pos, drop_ptr->pos), nullptr);
    m->set_animation(PIKMIN_ANIM_DRINKING);
}


/**
 * @brief When a Pikmin starts dying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_dying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    pik_ptr->start_dying();
    
    m->leave_group();
    pikmin_fsm::be_released(m, info1, info2);
    pikmin_fsm::notify_leader_release(m, info1, info2);
    pikmin_fsm::release_tool(m, nullptr, nullptr);
    m->set_animation(PIKMIN_ANIM_DYING);
}


/**
 * @brief When a Pikmin is killed after being knocked down.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_knocked_down_dying(Mob* m, void* info1, void* info2) {
    pikmin_fsm::start_dying(m, info1, info2);
    m->set_animation(PIKMIN_ANIM_KNOCKED_DOWN_DYING);
}


/**
 * @brief When a Pikmin starts flailing.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_flailing(Mob* m, void* info1, void* info2) {
    pikmin_fsm::release_tool(m, nullptr, nullptr);
    
    //If the Pikmin is following a moveable point, let's change it to
    //a static point. This will make the Pikmin continue to move
    //forward into the water in a straight line.
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    float final_z = 0.0f;
    Point final_pos = m->get_chase_target(&final_z);
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
void pikmin_fsm::start_getting_up(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::start_impact_lunge(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::start_mob_landing(Mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_MOB_LANDING);
}


/**
 * @brief When a Pikmin starts panicking.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::start_panicking(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
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
void pikmin_fsm::start_picking_up(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::start_returning(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Mob* carried_mob = (Mob*) info1;
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
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
void pikmin_fsm::start_riding_track(Mob* m, void* info1, void* info2) {
    Track* tra_ptr = (Track*) info1;
    
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
        new TrackRideInfo(
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
void pikmin_fsm::stop_being_idle(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::stop_being_thrown(Mob* m, void* info1, void* info2) {
    m->remove_particle_generator(MOB_PARTICLE_GENERATOR_ID_THROW);
}


/**
 * @brief When a Pikmin is meant to release an object it is carrying.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stop_carrying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    if(!pik_ptr->carrying_mob) return;
    
    pik_ptr->carrying_mob->fsm.run_event(MOB_EV_CARRIER_REMOVED, (void*) pik_ptr);
    
    pik_ptr->carrying_mob = nullptr;
    pik_ptr->set_timer(0);
}


/**
 * @brief When a Pikmin stands still while in a leader's group.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::stop_in_group(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
    m->stop_chasing();
    m->face(0, &m->following_group->pos);
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    m->set_animation(PIKMIN_ANIM_IDLING);
    m->set_timer(
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
void pikmin_fsm::tick_carrying(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::tick_entering_onion(Mob* m, void* info1, void* info2) {
    engine_assert(m->track_info != nullptr, m->print_state_history());
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    if(m->tick_track_ride()) {
        //Finished!
        ((Onion*) m->focused_mob)->nest->store_pikmin((Pikmin*) m);
    }
}


/**
 * @brief When a Pikmin has to teleport to its spot in a group task.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void pikmin_fsm::tick_group_task_work(Mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    GroupTask* tas_ptr = (GroupTask*) (m->focused_mob);
    Point cur_spot_pos = tas_ptr->get_spot_pos(pik_ptr);
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
void pikmin_fsm::tick_track_ride(Mob* m, void* info1, void* info2) {
    engine_assert(m->track_info != nullptr, m->print_state_history());
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
void pikmin_fsm::touched_eat_hitbox(Mob* m, void* info1, void* info2) {
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
void pikmin_fsm::touched_hazard(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Hazard* haz_ptr = (Hazard*) info1;
    
    if(info2) {
        //This is an attack.
        HitboxInteraction* h_info = (HitboxInteraction*) info2;
        if(!pik_ptr->process_attack_miss(h_info)) {
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
                standard_particle_gen_setup(
                    game.sys_content_names.part_wave_ring, m
                );
            pg.follow_z_offset = 1.0f;
            adjust_keyframe_interpolator_values<float>(
                pg.base_particle.size,
            [ = ] (const float & f) { return f * m->radius; }
            );
            pg.id = MOB_PARTICLE_GENERATOR_ID_WAVE_RING;
            m->particle_generators.push_back(pg);
        }
    }
    
    if(pik_ptr->invuln_period.time_left > 0) return;
    MobType::Vulnerability vuln = pik_ptr->get_hazard_vulnerability(haz_ptr);
    if(vuln.effect_mult == 0.0f) return;
    
    if(!vuln.status_to_apply || !vuln.status_overrides) {
        for(size_t e = 0; e < haz_ptr->effects.size(); e++) {
            pik_ptr->apply_status_effect(haz_ptr->effects[e], false, true);
        }
    }
    if(vuln.status_to_apply) {
        pik_ptr->apply_status_effect(vuln.status_to_apply, false, true);
    }
}


/**
 * @brief When a Pikmin is sprayed.
 *
 * @param m The mob.
 * @param info1 Pointer to the spray type.
 * @param info2 Unused.
 */
void pikmin_fsm::touched_spray(Mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    SprayType* s = (SprayType*) info1;
    
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
void pikmin_fsm::try_held_item_hotswap(Mob* m, void* info1, void* info2) {
    assert(!m->holding.empty());
    
    Tool* too_ptr = (Tool*) * (m->holding.begin());
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
void pikmin_fsm::update_in_group_chasing(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Point target_pos;
    float target_dist; //Unused dummy value.
    
    if(pik_ptr->pik_type->can_fly) {
        enable_flag(pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
    
    if(!info1) {
        pik_ptr->get_group_spot_info(&target_pos, &target_dist);
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
void pikmin_fsm::whistled_while_holding(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) m;
    Tool* too_ptr = (Tool*) * (m->holding.begin());
    
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
void pikmin_fsm::whistled_while_riding(Mob* m, void* info1, void* info2) {
    engine_assert(m->track_info, m->print_state_history());
    
    Pikmin* pik_ptr = (Pikmin*) m;
    Track* tra_ptr = (Track*) (m->track_info->m);
    
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
void pikmin_fsm::work_on_group_task(Mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    GroupTask* tas_ptr = (GroupTask*) (m->focused_mob);
    Pikmin* pik_ptr = (Pikmin*) m;
    
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
