/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin finite state machine logic.
 */

#ifndef PIKMIN_FSM_INCLUDED
#define PIKMIN_FSM_INCLUDED

#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * Functions about the Pikmin's finite state machine and behavior.
 */
namespace pikmin_fsm {
void create_fsm(mob_type* typ);

void be_dismissed(           mob* m, void* info1, void* info2);
void be_grabbed_by_enemy(    mob* m, void* info1, void* info2);
void be_grabbed_by_friend(   mob* m, void* info1, void* info2);
void be_released(            mob* m, void* info1, void* info2);
void be_thrown(              mob* m, void* info1, void* info2);
void become_buried(          mob* m, void* info1, void* info2);
void become_disabled(        mob* m, void* info1, void* info2);
void become_idle(            mob* m, void* info1, void* info2);
void begin_pluck(            mob* m, void* info1, void* info2);
void buried_evolve(          mob* m, void* info1, void* info2);
void buried_schedule_evol(   mob* m, void* info1, void* info2);
void called(                 mob* m, void* info1, void* info2);
void chase_leader(           mob* m, void* info1, void* info2);
void check_disabled_edible(  mob* m, void* info1, void* info2);
void check_remove_flailing(  mob* m, void* info1, void* info2);
void clear_timer(            mob* m, void* info1, void* info2);
void do_grounded_attack(     mob* m, void* info1, void* info2);
void do_latched_attack(      mob* m, void* info1, void* info2);
void end_pluck(              mob* m, void* info1, void* info2);
void face_leader(            mob* m, void* info1, void* info2);
void fall_down_pit(          mob* m, void* info1, void* info2);
void flail_to_whistle(       mob* m, void* info1, void* info2);
void forget_carriable_object(mob* m, void* info1, void* info2);
void get_knocked_down(       mob* m, void* info1, void* info2);
void get_up(                 mob* m, void* info1, void* info2);
void go_to_carriable_object( mob* m, void* info1, void* info2);
void go_to_opponent(         mob* m, void* info1, void* info2);
void going_to_dismiss_spot(  mob* m, void* info1, void* info2);
void land(                   mob* m, void* info1, void* info2);
void land_on_mob(            mob* m, void* info1, void* info2);
void left_hazard(            mob* m, void* info1, void* info2);
void lose_latched_mob(       mob* m, void* info1, void* info2);
void notify_leader_release(  mob* m, void* info1, void* info2);
void panic_new_chase(        mob* m, void* info1, void* info2);
void prepare_to_attack(      mob* m, void* info1, void* info2);
void reach_carriable_object( mob* m, void* info1, void* info2);
void reach_dismiss_spot(     mob* m, void* info1, void* info2);
void rechase_opponent(       mob* m, void* info1, void* info2);
void remove_disabled(        mob* m, void* info1, void* info2);
void remove_panic(           mob* m, void* info1, void* info2);
void seed_landed(            mob* m, void* info1, void* info2);
void set_group_move_reach(   mob* m, void* info1, void* info2);
void set_idle_task_reach(    mob* m, void* info1, void* info2);
void sigh(                   mob* m, void* info1, void* info2);
void start_flailing(         mob* m, void* info1, void* info2);
void start_panicking(        mob* m, void* info1, void* info2);
void stand_still(            mob* m, void* info1, void* info2);
void stop_being_idle(        mob* m, void* info1, void* info2);
void stop_being_thrown(      mob* m, void* info1, void* info2);
void stop_carrying(          mob* m, void* info1, void* info2);
void stop_in_group(          mob* m, void* info1, void* info2);
void tick_attacking_grounded(mob* m, void* info1, void* info2);
void tick_grabbed_by_enemy(  mob* m, void* info1, void* info2);
void tick_latched(           mob* m, void* info1, void* info2);
void touched_hazard(         mob* m, void* info1, void* info2);
void touched_spray(          mob* m, void* info1, void* info2);
void try_latching(           mob* m, void* info1, void* info2);
void update_in_group_chasing(mob* m, void* info1, void* info2);
}

#endif //ifndef PIKMIN_FSM_INCLUDED
