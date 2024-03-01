/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader finite state machine logic.
 */

#ifndef LEADER_FSM_INCLUDED
#define LEADER_FSM_INCLUDED

#include "../mob_types/mob_type.h"


/**
 * @brief Functions about the leader's finite state machine and behavior.
 */
namespace leader_fsm {
void create_fsm(mob_type* typ);

void be_attacked(            mob* m, void* info1, void* info2);
void be_dismissed(           mob* m, void* info1, void* info2);
void be_grabbed_by_friend(   mob* m, void* info1, void* info2);
void be_released(            mob* m, void* info1, void* info2);
void be_thrown(              mob* m, void* info1, void* info2);
void be_thrown_by_bouncer(   mob* m, void* info1, void* info2);
void become_active(          mob* m, void* info1, void* info2);
void become_inactive(        mob* m, void* info1, void* info2);
void check_punch_damage(     mob* m, void* info1, void* info2);
void die(                    mob* m, void* info1, void* info2);
void dismiss(                mob* m, void* info1, void* info2);
void decide_pluck_action(    mob* m, void* info1, void* info2);
void do_throw(               mob* m, void* info1, void* info2);
void enter_active(           mob* m, void* info1, void* info2);
void enter_idle(             mob* m, void* info1, void* info2);
void fall_asleep(            mob* m, void* info1, void* info2);
void fall_down_pit(          mob* m, void* info1, void* info2);
void finish_pluck(           mob* m, void* info1, void* info2);
void finish_drinking(        mob* m, void* info1, void* info2);
void go_pluck(               mob* m, void* info1, void* info2);
void grab_mob(               mob* m, void* info1, void* info2);
void hazard_pikmin_share(    mob* m, void* info1, void* info2);
void idle_or_rejoin(         mob* m, void* info1, void* info2);
void join_group(             mob* m, void* info1, void* info2);
void land(                   mob* m, void* info1, void* info2);
void left_hazard(            mob* m, void* info1, void* info2);
void lose_momentum(          mob* m, void* info1, void* info2);
void move(                   mob* m, void* info1, void* info2);
void notify_pikmin_release(  mob* m, void* info1, void* info2);
void punch(                  mob* m, void* info1, void* info2);
void queue_stop_auto_pluck(  mob* m, void* info1, void* info2);
void release(                mob* m, void* info1, void* info2);
void search_seed(            mob* m, void* info1, void* info2);
void set_knocked_back_anim(  mob* m, void* info1, void* info2);
void set_pain_anim(          mob* m, void* info1, void* info2);
void set_stop_anim(          mob* m, void* info1, void* info2);
void set_walk_anim(          mob* m, void* info1, void* info2);
void signal_stop_auto_pluck( mob* m, void* info1, void* info2);
void spray(                  mob* m, void* info1, void* info2);
void start_chasing_leader(   mob* m, void* info1, void* info2);
void start_drinking(         mob* m, void* info1, void* info2);
void start_go_here(          mob* m, void* info1, void* info2);
void start_pluck(            mob* m, void* info1, void* info2);
void start_riding_track(     mob* m, void* info1, void* info2);
void start_waking_up(        mob* m, void* info1, void* info2);
void stop(                   mob* m, void* info1, void* info2);
void stop_auto_pluck(        mob* m, void* info1, void* info2);
void stop_being_thrown(      mob* m, void* info1, void* info2);
void stop_go_here(           mob* m, void* info1, void* info2);
void stop_in_group(          mob* m, void* info1, void* info2);
void stop_whistle(           mob* m, void* info1, void* info2);
void tick_active_state(      mob* m, void* info1, void* info2);
void tick_track_ride(        mob* m, void* info1, void* info2);
void touched_hazard(         mob* m, void* info1, void* info2);
void touched_spray(          mob* m, void* info1, void* info2);
void update_in_group_chasing(mob* m, void* info1, void* info2);
void whistle(                mob* m, void* info1, void* info2);
void whistled_while_riding(  mob* m, void* info1, void* info2);
}


#endif //ifndef LEADER_FSM_INCLUDED
