/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin type class and Pikmin type-related functions.
 */

#include "pikmin_type.h"

#include "const.h"
#include "leader.h"
#include "mob_script.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a Pikmin type.
 */
pikmin_type::pikmin_type() {
    attack_attribute = 0;
    carry_strength = 1;
    attack_power = 1;
    weight = 1;
    carry_speed = 1;
    attack_interval = 0.8;
    size = DEF_PIKMIN_SIZE;
    throw_height_mult = 1.0;
    has_onion = true;
    can_dig = false;
    can_fly = false;
    can_swim = false;
    can_latch = true;
    can_carry_bomb_rocks = false;
    bmp_top[0] = NULL;
    bmp_top[1] = NULL;
    bmp_top[2] = NULL;
    bmp_icon[0] = NULL;
    bmp_icon[1] = NULL;
    bmp_icon[2] = NULL;
    
    init_script();
}


void pikmin_type::init_script() {

    easy_fsm_creator efc;
    efc.new_state("buried", PIKMIN_STATE_BURIED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin::become_buried);
        }
        efc.new_event(MOB_EVENT_PLUCKED); {
            efc.run_function(pikmin::be_plucked);
            efc.change_state("in_group_chasing");
        }
    }
    efc.new_state("in_group_chasing", PIKMIN_STATE_IN_GROUP_CHASING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin::chase_leader);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run_function(pikmin::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EVENT_LEADER_IS_NEAR); {
            efc.change_state("in_group_stopped");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run_function(pikmin::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("in_group_stopped", PIKMIN_STATE_IN_GROUP_STOPPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin::stop_in_group);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run_function(pikmin::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EVENT_LEADER_IS_FAR); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run_function(pikmin::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("grabbed_by_leader", PIKMIN_STATE_GRABBED_BY_LEADER); {
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run_function(pikmin::be_released);
        }
        efc.new_event(MOB_EVENT_THROWN); {
            efc.run_function(pikmin::be_thrown);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EVENT_RELEASED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("thrown", PIKMIN_STATE_THROWN); {
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run_function(pikmin::land);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_A_N); {
            efc.run_function(pikmin::land_on_mob);
            efc.change_state("attacking_latched");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("going_to_dismiss_spot", PIKMIN_STATE_GOING_TO_DISMISS_SPOT); {
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run_function(pikmin::reach_dismiss_spot);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("idle", PIKMIN_STATE_IDLE); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin::become_idle);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run_function(pikmin::stop_being_idle);
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run_function(pikmin::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EVENT_NEAR_OPPONENT); {
            efc.run_function(pikmin::go_to_opponent);
            efc.change_state("going_to_opponent");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_TOUCHED_LEADER); {
            efc.run_function(pikmin::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("going_to_carriable_object", PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT); {
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run_function(pikmin::grab_carriable_object);
            efc.change_state("carrying");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin::forget_about_carrying);
            efc.run_function(pikmin::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_LOST_FOCUSED_MOB); {
            efc.run_function(pikmin::forget_about_carrying);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE); {
            efc.run_function(pikmin::forget_about_carrying);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("going_to_opponent", PIKMIN_STATE_GOING_TO_OPPONENT); {
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("attacking_grounded");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_LOST_FOCUSED_MOB); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_DIED); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("attacking_grounded", PIKMIN_STATE_ATTACKING_GROUNDED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin::prepare_to_attack);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run_function(pikmin::tick_attacking_grounded);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run_function(pikmin::rechase_opponent);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("attacking_latched", PIKMIN_STATE_ATTACKING_LATCHED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin::prepare_to_attack);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run_function(pikmin::tick_latched);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_DIED); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("grabbed_by_enemy", PIKMIN_STATE_GRABBED_BY_ENEMY); {
        efc.new_event(MOB_EVENT_RELEASED); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run_function(pikmin::tick_grabbed_by_enemy);
        }
    }
    efc.new_state("knocked_back", PIKMIN_STATE_KNOCKED_BACK); {
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    efc.new_state("carrying", PIKMIN_STATE_CARRYING); {
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run_function(pikmin::forget_about_carrying);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_FINISHED_CARRYING); {
            efc.run_function(pikmin::finish_carrying);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_LOST_FOCUSED_MOB); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    
    states = efc.finish();
    first_state_nr = fix_states(states, "idle");
    
    first_state_nr = 0;
    for(size_t s = 0; s < states.size(); s++) {
        if(states[s]->name == "idle") {
            first_state_nr = s;
            break;
        }
    }
}
