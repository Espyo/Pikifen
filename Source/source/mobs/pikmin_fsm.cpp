/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin finite state machine logic.
 */

#include "../functions.h"
#include "mob_fsm.h"
#include "pikmin.h"
#include "pikmin_fsm.h"
#include "../vars.h"

void pikmin_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("buried", PIKMIN_STATE_BURIED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin_fsm::become_buried);
        }
        efc.new_event(MOB_EVENT_PLUCKED); {
            efc.run_function(pikmin_fsm::begin_pluck);
            efc.change_state("plucked");
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run_function(pikmin_fsm::stand_still);
        }
    }
    
    efc.new_state("plucked", PIKMIN_STATE_PLUCKING); {
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run_function(pikmin_fsm::end_pluck);
            efc.change_state("in_group_chasing");
        }
    }
    
    efc.new_state("in_group_chasing", PIKMIN_STATE_IN_GROUP_CHASING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin_fsm::chase_leader);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run_function(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("in_group_stopped");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_STARTED); {
            efc.change_state("group_move_chasing");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run_function(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("in_group_stopped", PIKMIN_STATE_IN_GROUP_STOPPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin_fsm::stop_in_group);
        }
        efc.new_event(MOB_EVENT_GRABBED_BY_FRIEND); {
            efc.run_function(pikmin_fsm::be_grabbed_by_friend);
            efc.change_state("grabbed_by_leader");
        }
        efc.new_event(MOB_EVENT_SPOT_IS_FAR); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_STARTED); {
            efc.change_state("group_move_chasing");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run_function(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("group_move_chasing", PIKMIN_STATE_GROUP_MOVE_CHASING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin_fsm::chase_leader);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run_function(pikmin_fsm::chase_leader);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("group_move_stopped");
        }
        efc.new_event(MOB_EVENT_GROUP_MOVE_ENDED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_DISMISSED); {
            efc.run_function(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_NEAR_OPPONENT); {
            efc.run_function(pikmin_fsm::go_to_opponent);
            efc.change_state("going_to_opponent");
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run_function(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("group_move_stopped", PIKMIN_STATE_GROUP_MOVE_STOPPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin_fsm::stop_in_group);
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
            efc.run_function(pikmin_fsm::be_dismissed);
            efc.change_state("going_to_dismiss_spot");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_NEAR_OPPONENT); {
            efc.run_function(pikmin_fsm::go_to_opponent);
            efc.change_state("going_to_opponent");
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run_function(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("grabbed_by_leader", PIKMIN_STATE_GRABBED_BY_LEADER); {
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run_function(pikmin_fsm::be_released);
        }
        efc.new_event(MOB_EVENT_THROWN); {
            efc.run_function(pikmin_fsm::be_thrown);
            efc.change_state("thrown");
        }
        efc.new_event(MOB_EVENT_RELEASED); {
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("thrown", PIKMIN_STATE_THROWN); {
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run_function(pikmin_fsm::land);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_A_N); {
            efc.run_function(pikmin_fsm::land_on_mob);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("going_to_dismiss_spot", PIKMIN_STATE_GOING_TO_DISMISS_SPOT); {
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run_function(pikmin_fsm::reach_dismiss_spot);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_NEAR_OPPONENT); {
            efc.run_function(pikmin_fsm::go_to_opponent);
            efc.change_state("going_to_opponent");
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run_function(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("idle", PIKMIN_STATE_IDLE); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin_fsm::become_idle);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run_function(pikmin_fsm::stop_being_idle);
        }
        efc.new_event(MOB_EVENT_NEAR_OPPONENT); {
            efc.run_function(pikmin_fsm::go_to_opponent);
            efc.change_state("going_to_opponent");
        }
        efc.new_event(MOB_EVENT_NEAR_CARRIABLE_OBJECT); {
            efc.run_function(pikmin_fsm::go_to_carriable_object);
            efc.change_state("going_to_carriable_object");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_TOUCHED_LEADER); {
            efc.run_function(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("going_to_opponent", PIKMIN_STATE_GOING_TO_OPPONENT); {
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.change_state("attacking_grounded");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_LOST_FOCUSED_MOB); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_DIED); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("going_to_carriable_object", PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT); {
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run_function(pikmin_fsm::reach_carriable_object);
            efc.change_state("carrying");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE); {
            efc.run_function(pikmin_fsm::forget_carriable_object);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run_function(pikmin_fsm::forget_carriable_object);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("carrying", PIKMIN_STATE_CARRYING); {
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run_function(pikmin_fsm::stop_carrying);
            efc.run_function(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_FINISHED_CARRYING); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_LOST_FOCUSED_MOB); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("attacking_grounded", PIKMIN_STATE_ATTACKING_GROUNDED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin_fsm::prepare_to_attack);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run_function(pikmin_fsm::tick_attacking_grounded);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run_function(pikmin_fsm::rechase_opponent);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
    }
    
    efc.new_state("attacking_latched", PIKMIN_STATE_ATTACKING_LATCHED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(pikmin_fsm::prepare_to_attack);
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run_function(pikmin_fsm::tick_latched);
        }
        efc.new_event(MOB_EVENT_WHISTLED); {
            efc.run_function(pikmin_fsm::called);
            efc.change_state("in_group_chasing");
        }
        efc.new_event(MOB_EVENT_FOCUSED_MOB_DIED); {
            efc.run_function(pikmin_fsm::lose_latched_mob);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(pikmin_fsm::get_knocked_down);
            efc.change_state("knocked_back");
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("grabbed_by_enemy", PIKMIN_STATE_GRABBED_BY_ENEMY); {
        efc.new_event(MOB_EVENT_RELEASED); {
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_ON_TICK); {
            efc.run_function(pikmin_fsm::tick_grabbed_by_enemy);
        }
    }
    
    efc.new_state("knocked_back", PIKMIN_STATE_KNOCKED_BACK); {
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.run_function(pikmin_fsm::stand_still);
            efc.change_state("idle");
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.run_function(pikmin_fsm::stand_still);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_EAT); {
            efc.run_function(pikmin_fsm::be_grabbed_by_enemy);
            efc.change_state("grabbed_by_enemy");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run_function(pikmin_fsm::fall_down_pit);
        }
    }
    
    efc.new_state("celebrating", PIKMIN_STATE_CELEBRATING); {
        //TODO
    }
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idle");
    
    if(typ->states.size() != N_PIKMIN_STATES) {
        error_log(
            "ENGINE WARNING: Number of Pikmin states on the FSM (" + i2s(typ->states.size()) +
            ") and the enum (" + i2s(N_PIKMIN_STATES) + ") do not match."
        );
    }
}


void pikmin_fsm::become_buried(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_BURROWED);
}

void pikmin_fsm::begin_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    mob* lea = (mob*) info1;
    
    if(lea->following_group) {
        if(typeid(*lea->following_group) == typeid(leader)) {
            //If this leader is following another one, then the new Pikmin should be in the group of that top leader.
            lea = lea->following_group;
        }
    }
    
    pik->set_animation(PIKMIN_ANIM_PLUCKING);
    add_to_group(lea, pik);
}

void pikmin_fsm::end_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    pik->set_animation(PIKMIN_ANIM_IDLE);
    sfx_pikmin_plucked.play(0, false);
    sfx_pikmin_pluck.play(0, false);
}

void pikmin_fsm::be_grabbed_by_friend(mob* m, void* info1, void* info2) {
    sfx_pikmin_held.play(0, false);
    m->set_animation(PIKMIN_ANIM_IDLE);
}

void pikmin_fsm::be_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* mob_ptr = (mob*) info1;
    hitbox_instance* hi_ptr = (hitbox_instance*) info2;
    
    pik_ptr->set_connected_hitbox_info(hi_ptr, mob_ptr);
    
    pik_ptr->focused_mob = mob_ptr;
    
    sfx_pikmin_caught.play(0.2, 0);
    pik_ptr->set_animation(PIKMIN_ANIM_IDLE);
    remove_from_group(pik_ptr);
}

void pikmin_fsm::be_dismissed(mob* m, void* info1, void* info2) {
    m->chase(
        *(float*) info1,
        *(float*) info2,
        NULL,
        NULL,
        false
    );
    sfx_pikmin_idle.play(0, false);
    
    m->set_animation(PIKMIN_ANIM_IDLE);
}

void pikmin_fsm::reach_dismiss_spot(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_IDLE);
}

void pikmin_fsm::become_idle(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_IDLE);
    unfocus_mob(m);
}

void pikmin_fsm::be_thrown(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    sfx_pikmin_held.stop();
    sfx_pikmin_thrown.stop();
    sfx_pikmin_thrown.play(0, false);
    m->set_animation(PIKMIN_ANIM_THROWN);
}

void pikmin_fsm::be_released(mob* m, void* info1, void* info2) {

}

void pikmin_fsm::land(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_IDLE);
    pikmin_fsm::stand_still(m, NULL, NULL);
}

void pikmin_fsm::stand_still(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->speed_x = m->speed_y = 0;
}

void pikmin_fsm::called(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    
    pik->attack_time = 0;
    add_to_group(cur_leader_ptr, pik);
    sfx_pikmin_called.play(0.03, false);
}

void pikmin_fsm::get_knocked_down(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    float knockback = 0;
    float knockback_angle = 0;
    
    calculate_knockback(info->mob2, m, info->hi2, info->hi1, &knockback, &knockback_angle);
    apply_knockback(m, knockback, knockback_angle);
    
    m->set_animation(PIKMIN_ANIM_LYING);
    
    remove_from_group(m);
}

void pikmin_fsm::go_to_opponent(mob* m, void* info1, void* info2) {
    focus_mob(m, (mob*) info1);
    m->stop_chasing();
    m->chase(
        0, 0,
        &m->focused_mob->x, &m->focused_mob->y,
        false, nullptr, false,
        m->focused_mob->type->radius + m->type->radius
    );
    m->set_animation(PIKMIN_ANIM_WALK);
    remove_from_group(m);
}

void pikmin_fsm::rechase_opponent(mob* m, void* info1, void* info2) {
    if(
        m->focused_mob &&
        m->focused_mob->health > 0 &&
        dist(m->x, m->y, m->focused_mob->x, m->focused_mob->y) <=
        (m->type->radius + m->focused_mob->type->radius + m->type->near_radius)
    ) {
        return;
    }
    
    m->fsm.set_state(PIKMIN_STATE_IDLE);
}

void pikmin_fsm::go_to_carriable_object(mob* m, void* info1, void* info2) {
    mob* carriable_mob = (mob*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    pik_ptr->carrying_mob = carriable_mob;
    pik_ptr->stop_chasing();
    
    size_t closest_spot = INVALID;
    dist closest_spot_dist;
    carrier_spot_struct* closest_spot_ptr;
    
    for(size_t s = 0; s < carriable_mob->type->max_carriers; ++s) {
        carrier_spot_struct* s_ptr = &carriable_mob->carry_info->spot_info[s];
        if(s_ptr->state != CARRY_SPOT_FREE) continue;
        
        dist d(
            pik_ptr->x,
            pik_ptr->y,
            carriable_mob->x + s_ptr->x,
            carriable_mob->y + s_ptr->y
        );
        if(closest_spot == INVALID || d < closest_spot_dist) {
            closest_spot = s;
            closest_spot_dist = d;
            closest_spot_ptr = s_ptr;
        }
    }
    
    pik_ptr->carrying_spot = closest_spot;
    closest_spot_ptr->state = CARRY_SPOT_RESERVED;
    closest_spot_ptr->pik_ptr = pik_ptr;
    
    pik_ptr->carrying_mob->fsm.run_event(MOB_EVENT_CARRY_WAIT_UP);
    
    pik_ptr->chase(
        closest_spot_ptr->x,
        closest_spot_ptr->y,
        &carriable_mob->x,
        &carriable_mob->y,
        false, nullptr, false,
        pik_ptr->type->radius * 1.2
    );
    pik_ptr->set_animation(PIKMIN_ANIM_WALK);
    remove_from_group(pik_ptr);
    
    pik_ptr->set_timer(PIKMIN_GOTO_TIMEOUT);
}

void pikmin_fsm::reach_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* carriable_mob = pik_ptr->carrying_mob;
    
    pik_ptr->set_animation(PIKMIN_ANIM_GRAB, true);
    
    float final_x = carriable_mob->x + carriable_mob->carry_info->spot_info[pik_ptr->carrying_spot].x;
    float final_y = carriable_mob->y + carriable_mob->carry_info->spot_info[pik_ptr->carrying_spot].y;
    
    pik_ptr->chase(
        carriable_mob->carry_info->spot_info[pik_ptr->carrying_spot].x,
        carriable_mob->carry_info->spot_info[pik_ptr->carrying_spot].y,
        &carriable_mob->x, &carriable_mob->y,
        true, &carriable_mob->z
    );
    
    pik_ptr->face(atan2(carriable_mob->y - final_y, carriable_mob->x - final_x));
    
    pik_ptr->set_animation(PIKMIN_ANIM_CARRY);
    
    //Let the carriable mob know that a new Pikmin has grabbed on.
    pik_ptr->carrying_mob->fsm.run_event(MOB_EVENT_CARRY_KEEP_GOING);
    pik_ptr->carrying_mob->fsm.run_event(
        MOB_EVENT_CARRIER_ADDED, (void*) pik_ptr
    );
    
}


void pikmin_fsm::forget_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    
    p->carrying_mob->fsm.run_event(MOB_EVENT_CARRY_KEEP_GOING);
    
    p->carrying_mob = NULL;
    p->set_timer(0);
}


void pikmin_fsm::stop_carrying(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(!p->carrying_mob) return;
    
    p->carrying_mob->fsm.run_event(MOB_EVENT_CARRIER_REMOVED, (void*) p);
    
    p->carrying_mob = NULL;
    p->set_timer(0);
}


void pikmin_fsm::prepare_to_attack(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    p->set_animation(PIKMIN_ANIM_ATTACK);
    ((pikmin*) p)->attack_time = p->pik_type->attack_interval;
}


void pikmin_fsm::land_on_mob(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    
    mob* mob_ptr = info->mob2;
    hitbox_instance* hi_ptr = info->hi2;
    
    if(!hi_ptr || !hi_ptr->can_pikmin_latch) {
        //No good for latching on. Make it act like it landed on the ground.
        m->fsm.run_event(MOB_EVENT_LANDED);
        return;
    }
    
    pik_ptr->connected_hitbox_nr = hi_ptr->hitbox_nr;
    pik_ptr->speed_x = pik_ptr->speed_y = pik_ptr->speed_z = 0;
    
    pik_ptr->focused_mob = mob_ptr;
    pik_ptr->set_connected_hitbox_info(hi_ptr, mob_ptr);
    
    pik_ptr->was_thrown = false;
    
    pik_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_LATCHED);
    
}

void pikmin_fsm::lose_latched_mob(mob* m, void* info1, void* info2) {
    m->stop_chasing();
}

void pikmin_fsm::tick_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->focused_mob) return;
    
    pik_ptr->teleport_to_connected_hitbox();
}

void pikmin_fsm::tick_latched(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->focused_mob) return;
    
    pik_ptr->teleport_to_connected_hitbox();
    
    pik_ptr->attack_time -= delta_t;
    
    if(pik_ptr->attack_time <= 0) {
        pik_ptr->do_attack(pik_ptr->focused_mob, get_hitbox_instance(pik_ptr->focused_mob, pik_ptr->connected_hitbox_nr));
    }
}

void pikmin_fsm::tick_attacking_grounded(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    pik_ptr->attack_time -= delta_t;
    
    if(!pik_ptr->focused_mob || pik_ptr->focused_mob->dead) {
        return;
    }
    if(pik_ptr->attack_time <= 0) {
        if(!(
                    (pik_ptr->focused_mob->z > pik_ptr->z + pik_ptr->type->height) ||
                    (pik_ptr->focused_mob->z + pik_ptr->focused_mob->type->height < pik_ptr->z)
                )) {
            pik_ptr->do_attack(
                pik_ptr->focused_mob,
                get_hitbox_instance(pik_ptr->focused_mob, pik_ptr->connected_hitbox_nr)
            );
        }
        pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
    }
    
    pik_ptr->face(atan2(pik_ptr->focused_mob->y - pik_ptr->y, pik_ptr->focused_mob->x - pik_ptr->x));
}

void pikmin_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->health = 0;
}

void pikmin_fsm::chase_leader(mob* m, void* info1, void* info2) {
    m->chase(
        m->group_spot_x, m->group_spot_y,
        &m->following_group->group->group_center_x, &m->following_group->group->group_center_y,
        false
    );
    m->set_animation(PIKMIN_ANIM_WALK);
    focus_mob(m, m->following_group);
}

void pikmin_fsm::stop_being_idle(mob* m, void* info1, void* info2) {

}

void pikmin_fsm::stop_in_group(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->set_animation(PIKMIN_ANIM_IDLE);
}
