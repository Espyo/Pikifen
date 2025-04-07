/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader finite state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the leader's finite state machine and behavior.
 */
namespace leader_fsm {
void create_fsm(MobType* typ);

void be_attacked(              Mob* m, void* info1, void* info2);
void be_dismissed(             Mob* m, void* info1, void* info2);
void be_grabbed_by_friend(     Mob* m, void* info1, void* info2);
void be_released(              Mob* m, void* info1, void* info2);
void be_thrown(                Mob* m, void* info1, void* info2);
void be_thrown_by_bouncer(     Mob* m, void* info1, void* info2);
void become_active(            Mob* m, void* info1, void* info2);
void become_inactive(          Mob* m, void* info1, void* info2);
void called(                   Mob* m, void* info1, void* info2);
void called_while_knocked_down(Mob* m, void* info1, void* info2);
void clear_boredom_data(       Mob* m, void* info1, void* info2);
void check_boredom_anim_end(   Mob* m, void* info1, void* info2);
void check_punch_damage(       Mob* m, void* info1, void* info2);
void clear_timer(              Mob* m, void* info1, void* info2);
void die(                      Mob* m, void* info1, void* info2);
void dismiss(                  Mob* m, void* info1, void* info2);
void decide_pluck_action(      Mob* m, void* info1, void* info2);
void do_throw(                 Mob* m, void* info1, void* info2);
void enter_active(             Mob* m, void* info1, void* info2);
void enter_idle(               Mob* m, void* info1, void* info2);
void fall_asleep(              Mob* m, void* info1, void* info2);
void fall_down_pit(            Mob* m, void* info1, void* info2);
void finish_called_anim(       Mob* m, void* info1, void* info2);
void finish_pluck(             Mob* m, void* info1, void* info2);
void finish_getting_up(        Mob* m, void* info1, void* info2);
void finish_drinking(          Mob* m, void* info1, void* info2);
void get_knocked_back(         Mob* m, void* info1, void* info2);
void get_knocked_down(         Mob* m, void* info1, void* info2);
void get_up_faster(            Mob* m, void* info1, void* info2);
void go_pluck(                 Mob* m, void* info1, void* info2);
void grab_mob(                 Mob* m, void* info1, void* info2);
void idle_or_rejoin(           Mob* m, void* info1, void* info2);
void join_group(               Mob* m, void* info1, void* info2);
void land(                     Mob* m, void* info1, void* info2);
void left_hazard(              Mob* m, void* info1, void* info2);
void lose_momentum(            Mob* m, void* info1, void* info2);
void move(                     Mob* m, void* info1, void* info2);
void notify_pikmin_release(    Mob* m, void* info1, void* info2);
void punch(                    Mob* m, void* info1, void* info2);
void queue_stop_auto_pluck(    Mob* m, void* info1, void* info2);
void release(                  Mob* m, void* info1, void* info2);
void search_seed(              Mob* m, void* info1, void* info2);
void set_correct_active_anim(  Mob* m, void* info1, void* info2);
void set_is_turning_false(     Mob* m, void* info1, void* info2);
void set_is_turning_true(      Mob* m, void* info1, void* info2);
void set_is_walking_false(     Mob* m, void* info1, void* info2);
void set_is_walking_true(      Mob* m, void* info1, void* info2);
void set_pain_anim(            Mob* m, void* info1, void* info2);
void signal_stop_auto_pluck(   Mob* m, void* info1, void* info2);
void spray(                    Mob* m, void* info1, void* info2);
void stand_still(              Mob* m, void* info1, void* info2);
void start_boredom_anim(       Mob* m, void* info1, void* info2);
void start_chasing_leader(     Mob* m, void* info1, void* info2);
void start_drinking(           Mob* m, void* info1, void* info2);
void start_getting_up(         Mob* m, void* info1, void* info2);
void start_go_here(            Mob* m, void* info1, void* info2);
void start_pluck(              Mob* m, void* info1, void* info2);
void start_riding_track(       Mob* m, void* info1, void* info2);
void start_waking_up(          Mob* m, void* info1, void* info2);
void stop_auto_pluck(          Mob* m, void* info1, void* info2);
void stop_being_thrown(        Mob* m, void* info1, void* info2);
void stop_go_here(             Mob* m, void* info1, void* info2);
void stop_in_group(            Mob* m, void* info1, void* info2);
void stop_whistle(             Mob* m, void* info1, void* info2);
void tick_active_state(        Mob* m, void* info1, void* info2);
void tick_track_ride(          Mob* m, void* info1, void* info2);
void touched_hazard(           Mob* m, void* info1, void* info2);
void touched_spray(            Mob* m, void* info1, void* info2);
void update_in_group_chasing(  Mob* m, void* info1, void* info2);
void whistle(                  Mob* m, void* info1, void* info2);
void whistled_while_riding(    Mob* m, void* info1, void* info2);
}
