/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin finite state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the Pikmin's finite state machine and behavior.
 */
namespace pikmin_fsm {
void create_fsm(MobType* typ);

void be_attacked(              Mob* m, void* info1, void* info2);
void be_crushed(               Mob* m, void* info1, void* info2);
void be_dismissed(             Mob* m, void* info1, void* info2);
void be_grabbed_by_enemy(      Mob* m, void* info1, void* info2);
void be_grabbed_by_friend(     Mob* m, void* info1, void* info2);
void be_released(              Mob* m, void* info1, void* info2);
void be_thrown(                Mob* m, void* info1, void* info2);
void be_thrown_after_pluck(    Mob* m, void* info1, void* info2);
void be_thrown_by_bouncer(     Mob* m, void* info1, void* info2);
void become_helpless(          Mob* m, void* info1, void* info2);
void become_idle(              Mob* m, void* info1, void* info2);
void become_sprout(            Mob* m, void* info1, void* info2);
void begin_pluck(              Mob* m, void* info1, void* info2);
void called(                   Mob* m, void* info1, void* info2);
void called_while_knocked_down(Mob* m, void* info1, void* info2);
void celebrate(                Mob* m, void* info1, void* info2);
void check_boredom_anim_end(   Mob* m, void* info1, void* info2);
void check_incoming_attack(    Mob* m, void* info1, void* info2);
void check_leader_bump(        Mob* m, void* info1, void* info2);
void check_outgoing_attack(    Mob* m, void* info1, void* info2);
void check_shaking_anim_end(   Mob* m, void* info1, void* info2);
void circle_opponent(          Mob* m, void* info1, void* info2);
void clear_boredom_data(       Mob* m, void* info1, void* info2);
void clear_timer(              Mob* m, void* info1, void* info2);
void decide_attack(            Mob* m, void* info1, void* info2);
void do_impact_bounce(         Mob* m, void* info1, void* info2);
void enter_onion(              Mob* m, void* info1, void* info2);
void fall_down_pit(            Mob* m, void* info1, void* info2);
void finish_called_anim(       Mob* m, void* info1, void* info2);
void finish_carrying(          Mob* m, void* info1, void* info2);
void finish_drinking(          Mob* m, void* info1, void* info2);
void finish_dying(             Mob* m, void* info1, void* info2);
void finish_getting_up(        Mob* m, void* info1, void* info2);
void finish_mob_landing(       Mob* m, void* info1, void* info2);
void finish_picking_up(        Mob* m, void* info1, void* info2);
void flail_to_leader(          Mob* m, void* info1, void* info2);
void forget_carriable_object(  Mob* m, void* info1, void* info2);
void forget_group_task(        Mob* m, void* info1, void* info2);
void forget_tool(              Mob* m, void* info1, void* info2);
void get_knocked_back(         Mob* m, void* info1, void* info2);
void get_knocked_down(         Mob* m, void* info1, void* info2);
void go_to_carriable_object(   Mob* m, void* info1, void* info2);
void go_to_group_task(         Mob* m, void* info1, void* info2);
void go_to_onion(              Mob* m, void* info1, void* info2);
void go_to_opponent(           Mob* m, void* info1, void* info2);
void go_to_tool(               Mob* m, void* info1, void* info2);
void going_to_dismiss_spot(    Mob* m, void* info1, void* info2);
void land(                     Mob* m, void* info1, void* info2);
void land_after_impact_bounce( Mob* m, void* info1, void* info2);
void land_after_pluck(         Mob* m, void* info1, void* info2);
void land_on_mob(              Mob* m, void* info1, void* info2);
void land_on_mob_while_holding(Mob* m, void* info1, void* info2);
void land_while_holding(       Mob* m, void* info1, void* info2);
void leave_onion(              Mob* m, void* info1, void* info2);
void left_hazard(              Mob* m, void* info1, void* info2);
void lose_latched_mob(         Mob* m, void* info1, void* info2);
void notify_leader_release(    Mob* m, void* info1, void* info2);
void panic_new_chase(          Mob* m, void* info1, void* info2);
void prepare_to_attack(        Mob* m, void* info1, void* info2);
void reach_carriable_object(   Mob* m, void* info1, void* info2);
void reach_dismiss_spot(       Mob* m, void* info1, void* info2);
void rechase_opponent(         Mob* m, void* info1, void* info2);
void release_tool(             Mob* m, void* info1, void* info2);
void seed_landed(              Mob* m, void* info1, void* info2);
void set_bump_lock(            Mob* m, void* info1, void* info2);
void set_idle_task_reach(      Mob* m, void* info1, void* info2);
void set_swarm_reach(          Mob* m, void* info1, void* info2);
void sigh(                     Mob* m, void* info1, void* info2);
void sprout_evolve(            Mob* m, void* info1, void* info2);
void sprout_schedule_evol(     Mob* m, void* info1, void* info2);
void stand_still(              Mob* m, void* info1, void* info2);
void start_boredom_anim(       Mob* m, void* info1, void* info2);
void start_chasing_leader(     Mob* m, void* info1, void* info2);
void start_drinking(           Mob* m, void* info1, void* info2);
void start_dying(              Mob* m, void* info1, void* info2);
void start_flailing(           Mob* m, void* info1, void* info2);
void start_getting_up(         Mob* m, void* info1, void* info2);
void start_impact_lunge(       Mob* m, void* info1, void* info2);
void start_knocked_down_dying( Mob* m, void* info1, void* info2);
void start_mob_landing(        Mob* m, void* info1, void* info2);
void start_panicking(          Mob* m, void* info1, void* info2);
void start_picking_up(         Mob* m, void* info1, void* info2);
void start_returning(          Mob* m, void* info1, void* info2);
void start_riding_track(       Mob* m, void* info1, void* info2);
void stop_being_idle(          Mob* m, void* info1, void* info2);
void stop_being_thrown(        Mob* m, void* info1, void* info2);
void stop_carrying(            Mob* m, void* info1, void* info2);
void stop_in_group(            Mob* m, void* info1, void* info2);
void tick_carrying(            Mob* m, void* info1, void* info2);
void tick_entering_onion(      Mob* m, void* info1, void* info2);
void tick_group_task_work(     Mob* m, void* info1, void* info2);
void tick_track_ride(          Mob* m, void* info1, void* info2);
void touched_eat_hitbox(       Mob* m, void* info1, void* info2);
void touched_hazard(           Mob* m, void* info1, void* info2);
void touched_spray(            Mob* m, void* info1, void* info2);
void try_held_item_hotswap(    Mob* m, void* info1, void* info2);
void unlatch(                  Mob* m, void* info1, void* info2);
void update_in_group_chasing(  Mob* m, void* info1, void* info2);
void whistled_while_holding(   Mob* m, void* info1, void* info2);
void whistled_while_riding(    Mob* m, void* info1, void* info2);
void work_on_group_task(       Mob* m, void* info1, void* info2);
}
