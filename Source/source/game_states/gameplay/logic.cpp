/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Main gameplay logic.
 */

#include <algorithm>

#include "gameplay.h"

#include "../../const.h"
#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../mobs/group_task.h"
#include "../../mobs/pikmin.h"
#include "../../mobs/tool.h"
#include "../../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Ticks the logic of aesthetic things regarding the leader.
 * If the game is paused, these can be frozen in place without
 * any negative impact.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void gameplay_state::do_aesthetic_leader_logic(const float delta_t) {
    if(!cur_leader_ptr) return;
    
    //Swarming arrows.
    if(swarm_magnitude) {
        cur_leader_ptr->swarm_next_arrow_timer.tick(delta_t);
    }
    
    dist leader_to_cursor_dist(cur_leader_ptr->pos, leader_cursor_w);
    for(size_t a = 0; a < cur_leader_ptr->swarm_arrows.size(); ) {
        cur_leader_ptr->swarm_arrows[a] +=
            GAMEPLAY::SWARM_ARROW_SPEED * delta_t;
            
        dist max_dist =
            (swarm_magnitude > 0) ?
            game.config.cursor_max_dist * swarm_magnitude :
            leader_to_cursor_dist;
            
        if(max_dist < cur_leader_ptr->swarm_arrows[a]) {
            cur_leader_ptr->swarm_arrows.erase(
                cur_leader_ptr->swarm_arrows.begin() + a
            );
        } else {
            a++;
        }
    }
    
    //Whistle.
    float whistle_dist;
    point whistle_pos;
    
    if(leader_to_cursor_dist > game.config.whistle_max_dist) {
        whistle_dist = game.config.whistle_max_dist;
        float whistle_angle =
            get_angle(cur_leader_ptr->pos, leader_cursor_w);
        whistle_pos = angle_to_coordinates(whistle_angle, whistle_dist);
        whistle_pos += cur_leader_ptr->pos;
    } else {
        whistle_dist = leader_to_cursor_dist.to_float();
        whistle_pos = leader_cursor_w;
    }
    
    whistle.tick(
        delta_t, whistle_pos,
        cur_leader_ptr->lea_type->whistle_range, whistle_dist
    );
    
    //Where the cursor is.
    cursor_height_diff_light = 0;
    
    if(leader_to_cursor_dist > game.config.throw_max_dist) {
        float throw_angle =
            get_angle(cur_leader_ptr->pos, leader_cursor_w);
        throw_dest =
            angle_to_coordinates(throw_angle, game.config.throw_max_dist);
        throw_dest += cur_leader_ptr->pos;
    } else {
        throw_dest = leader_cursor_w;
    }
    
    throw_dest_mob = NULL;
    for(size_t m = 0; m < mobs.all.size(); ++m) {
        mob* m_ptr = mobs.all[m];
        if(!bbox_check(throw_dest, m_ptr->pos, m_ptr->max_span)) {
            //Too far away; of course the cursor isn't on it.
            continue;
        }
        if(!m_ptr->type->pushable && !m_ptr->type->walkable) {
            //If it doesn't push and can't be walked on, there's probably
            //nothing really for the Pikmin to land on top of.
            continue;
        }
        if(
            throw_dest_mob &&
            m_ptr->z + m_ptr->height <
            throw_dest_mob->z + throw_dest_mob->height
        ) {
            //If this mob is lower than the previous known "under cursor" mob,
            //then forget it.
            continue;
        }
        if(!m_ptr->is_point_on(throw_dest)) {
            //The cursor is not really on top of this mob.
            continue;
        }
        
        throw_dest_mob = m_ptr;
    }
    
    leader_cursor_sector =
        get_sector(leader_cursor_w, NULL, true);
        
    throw_dest_sector =
        get_sector(throw_dest, NULL, true);
        
    if(leader_cursor_sector) {
        cursor_height_diff_light =
            (leader_cursor_sector->z - cur_leader_ptr->z) * 0.001;
        cursor_height_diff_light =
            clamp(cursor_height_diff_light, -0.1f, 0.1f);
    }
    
}


/* ----------------------------------------------------------------------------
 * Ticks the logic of aesthetic things. If the game is paused, these can
 * be frozen in place without any negative impact.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void gameplay_state::do_aesthetic_logic(const float delta_t) {
    //Leader stuff.
    do_aesthetic_leader_logic(delta_t);
    
    //Specific animations.
    game.sys_assets.spark_animation.instance.tick(delta_t);
}


/* ----------------------------------------------------------------------------
 * Ticks the logic of leader gameplay-related things.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void gameplay_state::do_gameplay_leader_logic(const float delta_t) {
    if(!cur_leader_ptr) return;
    
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Logic -- Current leader");
    }
    
    if(cur_leader_ptr->to_delete) {
        game.states.gameplay->update_available_leaders();
        change_to_next_leader(true, true, true);
    }
    
    /********************
    *              ***  *
    *   Whistle   * O * *
    *              ***  *
    ********************/
    
    if(
        whistle.whistling &&
        whistle.radius < cur_leader_ptr->lea_type->whistle_range
    ) {
        whistle.radius += game.config.whistle_growth_speed * delta_t;
        if(whistle.radius > cur_leader_ptr->lea_type->whistle_range) {
            whistle.radius = cur_leader_ptr->lea_type->whistle_range;
        }
    }
    
    //Current leader movement.
    point dummy_coords;
    float dummy_angle;
    float leader_move_magnitude;
    leader_movement.get_info(
        &dummy_coords, &dummy_angle, &leader_move_magnitude
    );
    if(leader_move_magnitude < 0.75) {
        cur_leader_ptr->fsm.run_event(
            LEADER_EV_MOVE_END, (void*) &leader_movement
        );
    } else {
        cur_leader_ptr->fsm.run_event(
            LEADER_EV_MOVE_START, (void*) &leader_movement
        );
    }
    
    if(cur_interlude == INTERLUDE_NONE) {
        //Adjust the camera position.
        float leader_weight = 1.0f;
        float cursor_weight = game.options.cursor_cam_weight;
        float group_weight = 0.0f;
        
        point group_center = cur_leader_ptr->pos;
        if(!cur_leader_ptr->group->members.empty()) {
            point tl = cur_leader_ptr->group->members[0]->pos;
            point br = cur_leader_ptr->group->members[0]->pos;
            for(size_t m = 1; m < cur_leader_ptr->group->members.size(); ++m) {
                mob* member = cur_leader_ptr->group->members[m];
                tl.x = std::min(tl.x, member->pos.x);
                tl.y = std::min(tl.y, member->pos.y);
                br.x = std::max(tl.x, member->pos.x);
                br.y = std::max(tl.y, member->pos.y);
            }
            group_center.x = (tl.x + br.x) / 2.0f;
            group_center.y = (tl.y + br.y) / 2.0f;
            group_weight = 0.1f;
        }
        
        float weight_sums = leader_weight + cursor_weight + group_weight;
        if(weight_sums == 0.0f) weight_sums = 0.01f;
        leader_weight /= weight_sums;
        cursor_weight /= weight_sums;
        group_weight /= weight_sums;
        
        game.cam.target_pos =
            cur_leader_ptr->pos * leader_weight +
            leader_cursor_w * cursor_weight +
            group_center * group_weight;
    }
    
    //Check what to show on the notification, if anything.
    notification.set_enabled(false);
    
    bool notification_done = false;
    
    //Lying down stop notification.
    if(
        !notification_done &&
        cur_leader_ptr->carry_info
    ) {
        notification.set_enabled(true);
        notification.set_contents(
            game.controls.find_bind(PLAYER_ACTION_WHISTLE).input,
            "Get up",
            point(
                cur_leader_ptr->pos.x,
                cur_leader_ptr->pos.y - cur_leader_ptr->radius
            )
        );
        notification_done = true;
    }
    
    //Auto-throw stop notification.
    if(
        !notification_done &&
        cur_leader_ptr->auto_throwing &&
        game.options.auto_throw_mode == AUTO_THROW_TOGGLE
    ) {
        notification.set_enabled(true);
        notification.set_contents(
            game.controls.find_bind(PLAYER_ACTION_THROW).input,
            "Stop throwing",
            point(
                cur_leader_ptr->pos.x,
                cur_leader_ptr->pos.y - cur_leader_ptr->radius
            )
        );
        notification_done = true;
    }
    
    //Pluck stop notification.
    if(
        !notification_done &&
        cur_leader_ptr->auto_plucking
    ) {
        notification.set_enabled(true);
        notification.set_contents(
            game.controls.find_bind(PLAYER_ACTION_WHISTLE).input,
            "Stop",
            point(
                cur_leader_ptr->pos.x,
                cur_leader_ptr->pos.y - cur_leader_ptr->radius
            )
        );
        notification_done = true;
    }
    
    if(!cur_leader_ptr->auto_plucking) {
        dist closest_d = 0;
        dist d = 0;
        
        //Ship healing notification.
        close_to_ship_to_heal = NULL;
        for(size_t s = 0; s < mobs.ships.size(); ++s) {
            ship* s_ptr = mobs.ships[s];
            d = dist(cur_leader_ptr->pos, s_ptr->pos);
            if(!s_ptr->is_leader_on_cp(cur_leader_ptr)) {
                continue;
            }
            if(cur_leader_ptr->health == cur_leader_ptr->max_health) {
                continue;
            }
            if(!s_ptr->shi_type->can_heal) {
                continue;
            }
            if(d < closest_d || !close_to_ship_to_heal) {
                close_to_ship_to_heal = s_ptr;
                closest_d = d;
                notification.set_enabled(true);
                notification.set_contents(
                    game.controls.find_bind(PLAYER_ACTION_THROW).input,
                    "Repair suit",
                    point(
                        close_to_ship_to_heal->pos.x,
                        close_to_ship_to_heal->pos.y -
                        close_to_ship_to_heal->radius
                    )
                );
                notification_done = true;
            }
        }
        
        //Interactable mob notification.
        closest_d = 0;
        d = 0;
        close_to_interactable_to_use = NULL;
        if(!notification_done) {
            for(size_t i = 0; i < mobs.interactables.size(); ++i) {
                d = dist(cur_leader_ptr->pos, mobs.interactables[i]->pos);
                if(d > mobs.interactables[i]->int_type->trigger_range) {
                    continue;
                }
                if(d < closest_d || !close_to_interactable_to_use) {
                    close_to_interactable_to_use = mobs.interactables[i];
                    closest_d = d;
                    notification.set_enabled(true);
                    notification.set_contents(
                        game.controls.find_bind(PLAYER_ACTION_THROW).input,
                        close_to_interactable_to_use->int_type->prompt_text,
                        point(
                            close_to_interactable_to_use->pos.x,
                            close_to_interactable_to_use->pos.y -
                            close_to_interactable_to_use->radius
                        )
                    );
                    notification_done = true;
                }
            }
        }
        
        //Pikmin pluck notification.
        closest_d = 0;
        d = 0;
        close_to_pikmin_to_pluck = NULL;
        if(!notification_done) {
            pikmin* p = get_closest_sprout(cur_leader_ptr->pos, &d, false);
            if(p && d <= game.config.pluck_range) {
                close_to_pikmin_to_pluck = p;
                notification.set_enabled(true);
                notification.set_contents(
                    game.controls.find_bind(PLAYER_ACTION_THROW).input,
                    "Pluck",
                    point(
                        p->pos.x,
                        p->pos.y -
                        p->radius
                    )
                );
                notification_done = true;
            }
        }
        
        //Nest open notification.
        closest_d = 0;
        d = 0;
        close_to_nest_to_open = NULL;
        if(!notification_done) {
            for(size_t o = 0; o < mobs.onions.size(); ++o) {
                d = dist(cur_leader_ptr->pos, mobs.onions[o]->pos);
                if(d > game.config.onion_open_range) continue;
                if(d < closest_d || !close_to_nest_to_open) {
                    close_to_nest_to_open = mobs.onions[o]->nest;
                    closest_d = d;
                    notification.set_enabled(true);
                    notification.set_contents(
                        game.controls.find_bind(PLAYER_ACTION_THROW).input,
                        "Check",
                        point(
                            close_to_nest_to_open->m_ptr->pos.x,
                            close_to_nest_to_open->m_ptr->pos.y -
                            close_to_nest_to_open->m_ptr->radius
                        )
                    );
                    notification_done = true;
                }
            }
            for(size_t s = 0; s < mobs.ships.size(); ++s) {
                d = dist(cur_leader_ptr->pos, mobs.ships[s]->pos);
                if(!mobs.ships[s]->is_leader_on_cp(cur_leader_ptr)) {
                    continue;
                }
                if(mobs.ships[s]->shi_type->nest->pik_types.empty()) {
                    continue;
                }
                if(d < closest_d || !close_to_nest_to_open) {
                    close_to_nest_to_open = mobs.ships[s]->nest;
                    closest_d = d;
                    notification.set_enabled(true);
                    notification.set_contents(
                        game.controls.find_bind(PLAYER_ACTION_THROW).input,
                        "Check",
                        point(
                            close_to_nest_to_open->m_ptr->pos.x,
                            close_to_nest_to_open->m_ptr->pos.y -
                            close_to_nest_to_open->m_ptr->radius
                        )
                    );
                    notification_done = true;
                }
            }
        }
    }
    
    notification.tick(delta_t);
    
    /********************
    *             .-.   *
    *   Cursor   ( = )> *
    *             `-´   *
    ********************/
    
    point mouse_cursor_speed;
    float dummy_magnitude;
    cursor_movement.get_info(
        &mouse_cursor_speed, &dummy_angle, &dummy_magnitude
    );
    mouse_cursor_speed =
        mouse_cursor_speed * delta_t* game.options.cursor_speed;
        
    leader_cursor_w = game.mouse_cursor.w_pos;
    
    float cursor_angle = get_angle(cur_leader_ptr->pos, leader_cursor_w);
    
    dist leader_to_cursor_dist(cur_leader_ptr->pos, leader_cursor_w);
    if(leader_to_cursor_dist > game.config.cursor_max_dist) {
        //Cursor goes beyond the range limit.
        leader_cursor_w.x =
            cur_leader_ptr->pos.x +
            (cos(cursor_angle) * game.config.cursor_max_dist);
        leader_cursor_w.y =
            cur_leader_ptr->pos.y +
            (sin(cursor_angle) * game.config.cursor_max_dist);
            
        if(mouse_cursor_speed.x != 0 || mouse_cursor_speed.y != 0) {
            //If we're speeding the mouse cursor (via analog stick),
            //don't let it go beyond the edges.
            game.mouse_cursor.w_pos = leader_cursor_w;
            game.mouse_cursor.s_pos = game.mouse_cursor.w_pos;
            al_transform_coordinates(
                &game.world_to_screen_transform,
                &game.mouse_cursor.s_pos.x, &game.mouse_cursor.s_pos.y
            );
        }
    }
    
    leader_cursor_s = leader_cursor_w;
    al_transform_coordinates(
        &game.world_to_screen_transform,
        &leader_cursor_s.x, &leader_cursor_s.y
    );
    
    
    /***********************************
    *                             ***  *
    *   Current leader's group   ****O *
    *                             ***  *
    ************************************/
    
    update_closest_group_members();
    if(!cur_leader_ptr->holding.empty()) {
        closest_group_member[BUBBLE_CURRENT] = cur_leader_ptr->holding[0];
    }
    
    float old_swarm_magnitude = swarm_magnitude;
    point swarm_coords;
    float new_swarm_angle;
    swarm_movement.get_info(
        &swarm_coords, &new_swarm_angle, &swarm_magnitude
    );
    if(swarm_magnitude > 0) {
        //This stops arrows that were fading away to the left from
        //turning to angle 0 because the magnitude reached 0.
        swarm_angle = new_swarm_angle;
    }
    
    if(swarm_cursor) {
        swarm_angle = cursor_angle;
        leader_to_cursor_dist = dist(cur_leader_ptr->pos, leader_cursor_w);
        swarm_magnitude =
            leader_to_cursor_dist.to_float() / game.config.cursor_max_dist;
    }
    
    if(old_swarm_magnitude != swarm_magnitude) {
        if(swarm_magnitude != 0) {
            cur_leader_ptr->signal_swarm_start();
        } else {
            cur_leader_ptr->signal_swarm_end();
        }
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
}


/* ----------------------------------------------------------------------------
 * Ticks the logic of gameplay-related things.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void gameplay_state::do_gameplay_logic(const float delta_t) {

    //Camera movement.
    if(!cur_leader_ptr) {
        //If there's no leader being controlled, might as well move the camera.
        point coords;
        float dummy_angle;
        float dummy_magnitude;
        leader_movement.get_info(&coords, &dummy_angle, &dummy_magnitude);
        game.cam.target_pos = game.cam.pos + (coords * 120.0f / game.cam.zoom);
    }
    
    game.cam.tick(delta_t);
    
    update_transformations();
    
    game.cam.update_box();
    
    if(!msg_box) {
    
        /************************************
        *                              .-.  *
        *   Timer things - gameplay   ( L ) *
        *                              `-´  *
        *************************************/
        
        //Mouse cursor.
        point mouse_cursor_speed;
        float dummy_angle;
        float dummy_magnitude;
        cursor_movement.get_info(
            &mouse_cursor_speed, &dummy_angle, &dummy_magnitude
        );
        mouse_cursor_speed =
            mouse_cursor_speed * delta_t* game.options.cursor_speed;
            
        game.mouse_cursor.s_pos += mouse_cursor_speed;
        
        game.mouse_cursor.w_pos = game.mouse_cursor.s_pos;
        al_transform_coordinates(
            &game.screen_to_world_transform,
            &game.mouse_cursor.w_pos.x, &game.mouse_cursor.w_pos.y
        );
        
        area_time_passed += delta_t;
        if(cur_interlude == INTERLUDE_NONE) {
            gameplay_time_passed += delta_t;
            day_minutes +=
                (game.cur_area_data.day_time_speed * delta_t / 60.0f);
            if(day_minutes > 60 * 24) {
                day_minutes -= 60 * 24;
            }
        }
        
        //Tick all particles.
        if(game.perf_mon) {
            game.perf_mon->start_measurement("Logic -- Particles");
        }
        
        particles.tick_all(delta_t);
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
        }
        
        //Tick all status effect animations.
        for(auto &s : game.status_types) {
            s.second->overlay_anim_instance.tick(delta_t);
        }
        
        /*******************
        *             +--+ *
        *   Sectors   |  | *
        *             +--+ *
        ********************/
        if(game.perf_mon) {
            game.perf_mon->start_measurement("Logic -- Sector animation");
        }
        
        for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
            sector* s_ptr = game.cur_area_data.sectors[s];
            
            if(s_ptr->draining_liquid) {
            
                s_ptr->liquid_drain_left -= delta_t;
                
                if(s_ptr->liquid_drain_left <= 0) {
                
                    for(size_t h = 0; h < s_ptr->hazards.size();) {
                        if(s_ptr->hazards[h]->associated_liquid) {
                            s_ptr->hazards.erase(s_ptr->hazards.begin() + h);
                            path_mgr.handle_sector_hazard_change(s_ptr);
                        } else {
                            ++h;
                        }
                    }
                    
                    s_ptr->liquid_drain_left = 0;
                    s_ptr->draining_liquid = false;
                    
                    unordered_set<vertex*> sector_vertexes;
                    for(size_t e = 0; e < s_ptr->edges.size(); ++e) {
                        sector_vertexes.insert(s_ptr->edges[e]->vertexes[0]);
                        sector_vertexes.insert(s_ptr->edges[e]->vertexes[1]);
                    }
                    update_offset_effect_caches(
                        game.liquid_limit_effect_caches,
                        sector_vertexes,
                        does_edge_have_liquid_limit,
                        get_liquid_limit_length,
                        get_liquid_limit_color
                    );
                }
            }
            
            if(s_ptr->scroll.x != 0 || s_ptr->scroll.y != 0) {
                s_ptr->texture_info.translation += s_ptr->scroll * delta_t;
            }
        }
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
        }
        
        
        /*****************
        *                *
        *   Mobs   ()--> *
        *                *
        ******************/
        
        size_t old_nr_living_leaders = nr_living_leaders;
        //Some setup to calculate how far the leader walks.
        leader* old_leader = cur_leader_ptr;
        point old_leader_pos;
        bool old_leader_was_walking = false;
        if(cur_leader_ptr) {
            old_leader_pos = cur_leader_ptr->pos;
            old_leader_was_walking =
                cur_leader_ptr->active &&
                !has_flag(
                    cur_leader_ptr->chase_info.flags,
                    CHASE_FLAG_TELEPORT
                ) &&
                !has_flag(
                    cur_leader_ptr->chase_info.flags,
                    CHASE_FLAG_TELEPORTS_CONSTANTLY
                ) &&
                cur_leader_ptr->chase_info.state == CHASE_STATE_CHASING;
        }
        
        size_t n_mobs = mobs.all.size();
        for(size_t m = 0; m < n_mobs; ++m) {
            //Tick the mob.
            mob* m_ptr = mobs.all[m];
            m_ptr->tick(delta_t);
            if(!m_ptr->is_stored_inside_mob()) {
                process_mob_interactions(m_ptr, m);
            }
        }
        
        for(size_t m = 0; m < n_mobs;) {
            //Mob deletion.
            mob* m_ptr = mobs.all[m];
            if(m_ptr->to_delete) {
                delete_mob(m_ptr);
                n_mobs--;
                continue;
            }
            m++;
        }
        
        do_gameplay_leader_logic(delta_t);
        
        if(
            cur_leader_ptr && cur_leader_ptr == old_leader &&
            old_leader_was_walking
        ) {
            //This more or less tells us how far the leader walked in this
            //frame. It's not perfect, since it will also count the leader
            //getting pushed and knocked back whilst in the chasing state.
            //It also won't count the movement if the active leader changed
            //midway through.
            //But those are rare cases that don't really affect much in the
            //grand scheme of things, and don't really matter for a fun stat.
            game.statistics.distance_walked +=
                dist(old_leader_pos, cur_leader_ptr->pos).to_float();
        }
        
        nr_living_leaders = 0;
        for(size_t l = 0; l < mobs.leaders.size(); ++l) {
            if(mobs.leaders[l]->health > 0.0f) {
                nr_living_leaders++;
            }
        }
        if(nr_living_leaders < old_nr_living_leaders) {
            game.statistics.leader_kos +=
                old_nr_living_leaders - nr_living_leaders;
        }
        leaders_kod = starting_nr_of_leaders - nr_living_leaders;
        
        
        /**************************
        *                    /  / *
        *   Precipitation     / / *
        *                   /  /  *
        **************************/
        
        /*
        if(
            cur_area_data.weather_condition.precipitation_type !=
            PRECIPITATION_TYPE_NONE
        ) {
            precipitation_timer.tick(delta_t);
            if(precipitation_timer.ticked) {
                precipitation_timer = timer(
                    cur_area_data.weather_condition.
                    precipitation_frequency.get_random_number()
                );
                precipitation_timer.start();
                precipitation.push_back(point(0, 0));
            }
        
            for(size_t p = 0; p < precipitation.size();) {
                precipitation[p].y +=
                    cur_area_data.weather_condition.
                    precipitation_speed.get_random_number() * delta_t;
                if(precipitation[p].y > scr_h) {
                    precipitation.erase(precipitation.begin() + p);
                } else {
                    p++;
                }
            }
        }
        */
        
        
        /********************
        *             ~ ~ ~ *
        *   Liquids    ~ ~  *
        *             ~ ~ ~ *
        ********************/
        for(auto &l : game.liquids) {
            l.second->anim_instance.tick(delta_t);
        }
        
        
        /******************
        *             ___ *
        *   Mission   \ / *
        *              O  *
        *******************/
        if(
            game.cur_area_data.type == AREA_TYPE_MISSION &&
            game.cur_area_data.mission.goal == MISSION_GOAL_GET_TO_EXIT
        ) {
            cur_leaders_in_mission_exit = 0;
            for(size_t l = 0; l < mobs.leaders.size(); ++l) {
                mob* l_ptr = mobs.leaders[l];
                if(
                    std::find(
                        mission_remaining_mob_ids.begin(),
                        mission_remaining_mob_ids.end(),
                        mobs.leaders[l]->id
                    ) ==
                    mission_remaining_mob_ids.end()
                ) {
                    //Not a required leader.
                    continue;
                }
                if(
                    fabs(
                        l_ptr->pos.x -
                        game.cur_area_data.mission.goal_exit_center.x
                    ) <=
                    game.cur_area_data.mission.goal_exit_size.x / 2.0f &&
                    fabs(
                        l_ptr->pos.y -
                        game.cur_area_data.mission.goal_exit_center.y
                    ) <=
                    game.cur_area_data.mission.goal_exit_size.y / 2.0f
                ) {
                    cur_leaders_in_mission_exit++;
                }
            }
        }
        
        float real_goal_ratio = 0.0f;
        int goal_cur_amount =
            game.mission_goals[game.cur_area_data.mission.goal]->get_cur_amount(
                this
            );
        int goal_req_amount =
            game.mission_goals[game.cur_area_data.mission.goal]->get_req_amount(
                this
            );
        if(goal_req_amount != 0.0f) {
            real_goal_ratio = goal_cur_amount / (float) goal_req_amount;
        }
        goal_indicator_ratio +=
            (real_goal_ratio - goal_indicator_ratio) *
            (HUD::GOAL_INDICATOR_SMOOTHNESS_MULT * delta_t);
            
        if(game.cur_area_data.mission.fail_hud_primary_cond != INVALID) {
            float real_fail_ratio = 0.0f;
            int fail_cur_amount =
                game.mission_fail_conds[
                    game.cur_area_data.mission.fail_hud_primary_cond
                ]->get_cur_amount(this);
            int fail_req_amount =
                game.mission_fail_conds[
                    game.cur_area_data.mission.fail_hud_primary_cond
                ]->get_req_amount(this);
            if(fail_req_amount != 0.0f) {
                real_fail_ratio = fail_cur_amount / (float) fail_req_amount;
            }
            fail_1_indicator_ratio +=
                (real_fail_ratio - fail_1_indicator_ratio) *
                (HUD::GOAL_INDICATOR_SMOOTHNESS_MULT * delta_t);
        }
        
        if(game.cur_area_data.mission.fail_hud_secondary_cond != INVALID) {
            float real_fail_ratio = 0.0f;
            int fail_cur_amount =
                game.mission_fail_conds[
                    game.cur_area_data.mission.fail_hud_secondary_cond
                ]->get_cur_amount(this);
            int fail_req_amount =
                game.mission_fail_conds[
                    game.cur_area_data.mission.fail_hud_secondary_cond
                ]->get_req_amount(this);
            if(fail_req_amount != 0.0f) {
                real_fail_ratio = fail_cur_amount / (float) fail_req_amount;
            }
            fail_2_indicator_ratio +=
                (real_fail_ratio - fail_2_indicator_ratio) *
                (HUD::GOAL_INDICATOR_SMOOTHNESS_MULT * delta_t);
        }
        
        if(game.cur_area_data.type == AREA_TYPE_MISSION) {
            if(cur_interlude == INTERLUDE_NONE) {
                if(is_mission_clear_met()) {
                    end_mission(true);
                } else if(is_mission_fail_met(&mission_fail_reason)) {
                    end_mission(false);
                }
            }
            //Reset the positions of the last mission-end-related things,
            //since if they didn't get used in end_mission, then they
            //may be stale from here on.
            last_enemy_killed_pos = point(LARGE_FLOAT, LARGE_FLOAT);
            last_hurt_leader_pos = point(LARGE_FLOAT, LARGE_FLOAT);
            last_pikmin_born_pos = point(LARGE_FLOAT, LARGE_FLOAT);
            last_pikmin_death_pos = point(LARGE_FLOAT, LARGE_FLOAT);
            last_ship_that_got_treasure_pos = point(LARGE_FLOAT, LARGE_FLOAT);
            
            mission_score = game.cur_area_data.mission.starting_points;
            for(size_t c = 0; c < game.mission_score_criteria.size(); ++c) {
                if(
                    !has_flag(
                        game.cur_area_data.mission.point_hud_data,
                        get_index_bitmask(c)
                    )
                ) {
                    continue;
                }
                mission_score_criterion* c_ptr =
                    game.mission_score_criteria[c];
                int c_score =
                    c_ptr->get_score(this, &game.cur_area_data.mission);
                mission_score += c_score;
            }
            if(mission_score != old_mission_score) {
                mission_score_cur_text->start_juice_animation(
                    gui_item::JUICE_TYPE_GROW_TEXT_HIGH
                );
                old_mission_score = mission_score;
            }
            
            score_indicator +=
                (mission_score - score_indicator) *
                (HUD::SCORE_INDICATOR_SMOOTHNESS_MULT * delta_t);
                
            int goal_cur =
                game.mission_goals[game.cur_area_data.mission.goal]->
                get_cur_amount(game.states.gameplay);
            if(goal_cur != old_mission_goal_cur) {
                mission_goal_cur_text->start_juice_animation(
                    gui_item::JUICE_TYPE_GROW_TEXT_HIGH
                );
                old_mission_goal_cur = goal_cur;
            }
            
            if(
                game.cur_area_data.mission.fail_hud_primary_cond !=
                INVALID
            ) {
                size_t cond =
                    game.cur_area_data.mission.fail_hud_primary_cond;
                int fail_1_cur =
                    game.mission_fail_conds[cond]->get_cur_amount(
                        game.states.gameplay
                    );
                if(fail_1_cur != old_mission_fail_1_cur) {
                    mission_fail_1_cur_text->start_juice_animation(
                        gui_item::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    old_mission_fail_1_cur = fail_1_cur;
                }
            }
            if(
                game.cur_area_data.mission.fail_hud_secondary_cond !=
                INVALID
            ) {
                size_t cond =
                    game.cur_area_data.mission.fail_hud_secondary_cond;
                int fail_2_cur =
                    game.mission_fail_conds[cond]->get_cur_amount(
                        game.states.gameplay
                    );
                if(fail_2_cur != old_mission_fail_2_cur) {
                    mission_fail_2_cur_text->start_juice_animation(
                        gui_item::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    old_mission_fail_2_cur = fail_2_cur;
                }
            }
            
        }
        
    } else { //Displaying a message.
    
        msg_box->tick(delta_t);
        if(msg_box->to_delete) {
            start_message("", NULL);
        }
        
    }
    
    replay_timer.tick(delta_t);
    
    if(!ready_for_input) {
        ready_for_input = true;
        is_input_allowed = true;
    }
    
}


/* ----------------------------------------------------------------------------
 * Ticks the logic of in-game menu-related things.
 */
void gameplay_state::do_menu_logic() {
    if(onion_menu) {
        if(!onion_menu->to_delete) {
            onion_menu->tick(game.delta_t);
        } else {
            delete onion_menu;
            onion_menu = NULL;
            paused = false;
            game.audio.handle_world_unpause();
        }
    } else if(pause_menu) {
        if(!pause_menu->to_delete) {
            pause_menu->tick(game.delta_t);
        } else {
            delete pause_menu;
            pause_menu = NULL;
            paused = false;
            game.audio.handle_world_unpause();
        }
    }
    
    hud->tick(game.delta_t);
    
    //Process and print framerate and system info.
    if(game.show_system_info) {
    
        //Make sure that speed changes don't affect the FPS calculation.
        double real_delta_t = game.delta_t;
        if (game.maker_tools.change_speed) {
            real_delta_t /= game.maker_tools.change_speed_mult;
        }
        
        game.framerate_history.push_back(1.0f / real_delta_t);
        if(game.framerate_history.size() > GAME::FRAMERATE_HISTORY_SIZE) {
            game.framerate_history.erase(game.framerate_history.begin());
        }
        
        game.framerate_last_avg_point++;
        
        float sample_avg;
        
        if(game.framerate_last_avg_point >= GAME::FRAMERATE_AVG_SAMPLE_SIZE) {
            //Let's get an average, using FRAMERATE_AVG_SAMPLE_SIZE frames.
            //If we can fit a sample of this size using the most recent
            //unsampled frames, then use those. Otherwise, keep using the last
            //block, which starts at framerate_last_avg_point.
            //This makes it so the average stays the same for a bit of time,
            //so the player can actually read it.
            if(
                game.framerate_last_avg_point >
                GAME::FRAMERATE_AVG_SAMPLE_SIZE * 2
            ) {
                game.framerate_last_avg_point =
                    GAME::FRAMERATE_AVG_SAMPLE_SIZE;
            }
            float sample_avg_sum = 0;
            size_t sample_avg_point_count = 0;
            size_t sample_size =
                std::min(
                    (size_t) GAME::FRAMERATE_AVG_SAMPLE_SIZE,
                    game.framerate_history.size()
                );
                
            for(size_t f = 0; f < sample_size; ++f) {
                size_t idx =
                    game.framerate_history.size() -
                    game.framerate_last_avg_point + f;
                sample_avg_sum += game.framerate_history[idx];
                sample_avg_point_count++;
            }
            
            sample_avg = sample_avg_sum / (float) sample_avg_point_count;
            
        } else {
            //If there are less than FRAMERATE_AVG_SAMPLE_SIZE frames in
            //the history, the average will change every frame until we get
            //that. This defeats the purpose of a smoothly-updating number,
            //so until that requirement is filled, let's stick to the oldest
            //record.
            sample_avg = game.framerate_history[0];
            
        }
        
        string fps_str =
            box_string(f2s(sample_avg), 12, " avg, ") +
            box_string(f2s(1.0f / real_delta_t), 12, " now, ") +
            i2s(game.options.target_fps) + " intended";
        string n_mobs_str =
            box_string(i2s(mobs.all.size()), 7);
        string n_particles_str =
            box_string(i2s(particles.get_count()), 7);
        string resolution_str =
            i2s(game.win_w) + "x" + i2s(game.win_h);
        string area_v_str =
            game.cur_area_data.version.empty() ?
            "-" :
            game.cur_area_data.version;
        string area_maker_str =
            game.cur_area_data.maker.empty() ?
            "-" :
            game.cur_area_data.maker;
        string game_v_str =
            game.config.version.empty() ? "-" : game.config.version;
            
        print_info(
            "FPS: " + fps_str +
            "\n"
            "Mobs: " + n_mobs_str + " Particles: " + n_particles_str +
            "\n"
            "Resolution: " + resolution_str +
            "\n"
            "Area version " + area_v_str + ", by " + area_maker_str +
            "\n"
            "Pikifen version " + get_engine_version_string() +
            ", game version " + game_v_str,
            1.0f, 1.0f
        );
        
    } else {
        game.framerate_last_avg_point = 0;
        game.framerate_history.clear();
    }
    
    //Print info on a mob.
    if(game.maker_tools.info_lock) {
        string name_str =
            box_string(game.maker_tools.info_lock->type->name, 26);
        string coords_str =
            box_string(
                box_string(f2s(game.maker_tools.info_lock->pos.x), 8, " ") +
                box_string(f2s(game.maker_tools.info_lock->pos.y), 8, " ") +
                box_string(f2s(game.maker_tools.info_lock->z), 7),
                23
            );
        string state_h_str =
            (
                game.maker_tools.info_lock->fsm.cur_state ?
                game.maker_tools.info_lock->fsm.cur_state->name :
                "(None!)"
            );
        for(unsigned char p = 0; p < STATE_HISTORY_SIZE; ++p) {
            state_h_str +=
                " " + game.maker_tools.info_lock->fsm.prev_state_names[p];
        }
        string anim_str =
            game.maker_tools.info_lock->anim.cur_anim ?
            game.maker_tools.info_lock->anim.cur_anim->name :
            "(None!)";
        string health_str =
            box_string(
                box_string(f2s(game.maker_tools.info_lock->health), 6) +
                " / " +
                box_string(
                    f2s(game.maker_tools.info_lock->max_health), 6
                ),
                23
            );
        string timer_str =
            f2s(game.maker_tools.info_lock->script_timer.time_left);
        string vars_str;
        if(!game.maker_tools.info_lock->vars.empty()) {
            for(
                auto v = game.maker_tools.info_lock->vars.begin();
                v != game.maker_tools.info_lock->vars.end(); ++v
            ) {
                vars_str += v->first + "=" + v->second + "; ";
            }
            vars_str.erase(vars_str.size() - 2, 2);
        } else {
            vars_str = "(None)";
        }
        
        print_info(
            "Mob: " + name_str +
            "Coords: " + coords_str +
            "\n"
            "Last states: " + state_h_str +
            "\n"
            "Animation: " + anim_str +
            "\n"
            "Health: " + health_str + " Timer: " + timer_str +
            "\n"
            "Vars: " + vars_str,
            5.0f, 3.0f
        );
    }
    
    //Print path info.
    if(game.maker_tools.info_lock && game.maker_tools.path_info) {
        if(game.maker_tools.info_lock->path_info) {
        
            path_info_struct* path = game.maker_tools.info_lock->path_info;
            string result_str = path_result_to_string(path->result);
            
            string stops_str =
                box_string(i2s(path->cur_path_stop_nr + 1), 3) +
                "/" +
                box_string(i2s(path->path.size()), 3);
                
            string settings_str;
            auto flags = path->settings.flags;
            if(has_flag(flags, PATH_FOLLOW_FLAG_CAN_CONTINUE)) {
                settings_str += "can continue, ";
            }
            if(has_flag(flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES)) {
                settings_str += "ignore obstacles, ";
            }
            if(has_flag(flags, PATH_FOLLOW_FLAG_FOLLOW_MOB)) {
                settings_str += "follow mob, ";
            }
            if(has_flag(flags, PATH_FOLLOW_FLAG_FAKED_START)) {
                settings_str += "faked start, ";
            }
            if(has_flag(flags, PATH_FOLLOW_FLAG_FAKED_END)) {
                settings_str += "faked end, ";
            }
            if(has_flag(flags, PATH_FOLLOW_FLAG_SCRIPT_USE)) {
                settings_str += "script, ";
            }
            if(has_flag(flags, PATH_FOLLOW_FLAG_LIGHT_LOAD)) {
                settings_str += "light load, ";
            }
            if(has_flag(flags, PATH_FOLLOW_FLAG_AIRBORNE)) {
                settings_str += "airborne, ";
            }
            if(settings_str.size() > 2) {
                //Remove the extra comma and space.
                settings_str.pop_back();
                settings_str.pop_back();
            } else {
                settings_str = "none";
            }
            
            string block_str = path_block_reason_to_string(path->block_reason);
            
            print_info(
                "Path calculation result: " + result_str +
                "\n" +
                "Heading to stop " + stops_str +
                "\n" +
                "Settings: " + settings_str +
                "\n" +
                "Block reason: " + block_str,
                5.0f, 3.0f
            );
            
        } else {
        
            print_info("Mob is not following any path.", 5.0f, 3.0f);
            
        }
    }
    
    //Print mouse coordinates.
    if(game.maker_tools.geometry_info) {
        sector* mouse_sector =
            get_sector(game.mouse_cursor.w_pos, NULL, true);
            
        string coords_str =
            box_string(f2s(game.mouse_cursor.w_pos.x), 6) + " " +
            box_string(f2s(game.mouse_cursor.w_pos.y), 6);
        string blockmap_str =
            box_string(
                i2s(game.cur_area_data.bmap.get_col(game.mouse_cursor.w_pos.x)),
                5, " "
            ) +
            i2s(game.cur_area_data.bmap.get_row(game.mouse_cursor.w_pos.y));
        string sector_z_str, sector_light_str, sector_tex_str;
        if(mouse_sector) {
            sector_z_str =
                box_string(f2s(mouse_sector->z), 6);
            sector_light_str =
                box_string(i2s(mouse_sector->brightness), 3);
            sector_tex_str =
                mouse_sector->texture_info.file_name;
        }
        
        string str =
            "Mouse coords: " + coords_str +
            "\n"
            "Blockmap under mouse: " + blockmap_str +
            "\n"
            "Sector under mouse: ";
            
        if(mouse_sector) {
            str +=
                "\n"
                "  Z: " + sector_z_str + " Light: " + sector_light_str +
                "\n"
                "  Texture: " + sector_tex_str;
        } else {
            str += "None";
        }
        
        print_info(str, 1.0f, 1.0f);
    }
    
    game.maker_tools.info_print_timer.tick(game.delta_t);
    
    //Big message.
    if(cur_big_msg != BIG_MESSAGE_NONE) {
        big_msg_time += game.delta_t;
    }
    
    switch(cur_big_msg) {
    case BIG_MESSAGE_NONE: {
        break;
    } case BIG_MESSAGE_READY: {
        if(big_msg_time >= GAMEPLAY::BIG_MSG_READY_DUR) {
            cur_big_msg = BIG_MESSAGE_GO;
            big_msg_time = 0.0f;
        }
        break;
    } case BIG_MESSAGE_GO: {
        if(big_msg_time >= GAMEPLAY::BIG_MSG_GO_DUR) {
            cur_big_msg = BIG_MESSAGE_NONE;
        }
        break;
    } case BIG_MESSAGE_MISSION_CLEAR: {
        if(big_msg_time >= GAMEPLAY::BIG_MSG_MISSION_CLEAR_DUR) {
            cur_big_msg = BIG_MESSAGE_NONE;
        }
        break;
    } case BIG_MESSAGE_MISSION_FAILED: {
        if(big_msg_time >= GAMEPLAY::BIG_MSG_MISSION_FAILED_DUR) {
            cur_big_msg = BIG_MESSAGE_NONE;
        }
        break;
    }
    }
    
    //Interlude.
    if(cur_interlude != INTERLUDE_NONE) {
        interlude_time += game.delta_t;
    }
    
    switch(cur_interlude) {
    case INTERLUDE_NONE: {
        break;
    } case INTERLUDE_READY: {
        if(interlude_time >= GAMEPLAY::BIG_MSG_READY_DUR) {
            cur_interlude = INTERLUDE_NONE;
            delta_t_mult = 1.0f;
            hud->gui.start_animation(
                GUI_MANAGER_ANIM_OUT_TO_IN,
                GAMEPLAY::AREA_INTRO_HUD_MOVE_TIME
            );
        }
        break;
    } case INTERLUDE_MISSION_END: {
        if(interlude_time >= GAMEPLAY::BIG_MSG_MISSION_CLEAR_DUR) {
            cur_interlude = INTERLUDE_NONE;
            delta_t_mult = 1.0f;
            leave(LEAVE_TO_END);
        }
        break;
    }
    }
    
    //Area title fade.
    area_title_fade_timer.tick(game.delta_t);
    
    //Fade.
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Checks if the mission goal has been met.
 */
bool gameplay_state::is_mission_clear_met() {
    return game.mission_goals[game.cur_area_data.mission.goal]->is_met(this);
}


/* ----------------------------------------------------------------------------
 * Checks if a mission fail condition has been met.
 * reason:
 *   The reason gets returned here, if any.
 */
bool gameplay_state::is_mission_fail_met(MISSION_FAIL_CONDITIONS* reason) {
    for(size_t f = 0; f < game.mission_fail_conds.size(); ++f) {
        if(
            has_flag(
                game.cur_area_data.mission.fail_conditions,
                get_index_bitmask(f)
            )
        ) {
            if(game.mission_fail_conds[f]->is_met(this)) {
                *reason = (MISSION_FAIL_CONDITIONS) f;
                return true;
            }
        }
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Handles the logic required to tick a specific mob and its interactions
 * with other mobs.
 * m_ptr:
 *   Mob to process.
 * m:
 *   Index of the mob.
 */
void gameplay_state::process_mob_interactions(mob* m_ptr, size_t m) {
    vector<pending_intermob_event> pending_intermob_events;
    mob_state* state_before = m_ptr->fsm.cur_state;
    
    size_t n_mobs = mobs.all.size();
    for(size_t m2 = 0; m2 < n_mobs; ++m2) {
        if(m == m2) continue;
        
        mob* m2_ptr = mobs.all[m2];
        if(m2_ptr->to_delete) continue;
        if(m2_ptr->is_stored_inside_mob()) continue;
        
        dist d(m_ptr->pos, m2_ptr->pos);

        if (d > m_ptr->max_interaction_radius + m2_ptr->max_span)
            continue;

        if(game.perf_mon) {
            game.perf_mon->start_measurement("Objects -- Touching others");
        }

        if(d <= m_ptr->max_span + m2_ptr->max_span) {
            //Only check if their radii or hitboxes
            //can (theoretically) reach each other.
            process_mob_touches(m_ptr, m2_ptr, m, m2, d);
            
        }
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
            game.perf_mon->start_measurement("Objects -- Reaches");
        }
        
        if(
            m2_ptr->health != 0 && m_ptr->near_reach != INVALID &&
            !m2_ptr->has_invisibility_status
        ) {
            process_mob_reaches(
                m_ptr, m2_ptr, m, m2, d, pending_intermob_events
            );
        }
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
            game.perf_mon->start_measurement("Objects -- Misc. interactions");
        }
        
        process_mob_misc_interactions(
            m_ptr, m2_ptr, m, m2, d, pending_intermob_events
        );
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
        }
    }
    
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Objects -- Interaction results");
    }
    
    //Check the pending inter-mob events.
    sort(
        pending_intermob_events.begin(), pending_intermob_events.end(),
    [m_ptr] (pending_intermob_event e1, pending_intermob_event e2) -> bool {
        return
        (
            e1.d.to_float() -
            (m_ptr->radius + e1.mob_ptr->radius)
        ) < (
            e2.d.to_float() -
            (m_ptr->radius + e2.mob_ptr->radius)
        );
    }
    );
    
    for(size_t e = 0; e < pending_intermob_events.size(); ++e) {
        if(m_ptr->fsm.cur_state != state_before) {
            //We can't go on, since the new state might not even have the
            //event, and the reaches could've also changed.
            break;
        }
        if(!pending_intermob_events[e].event_ptr) continue;
        pending_intermob_events[e].event_ptr->run(
            m_ptr, (void*) pending_intermob_events[e].mob_ptr
        );
        
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the logic between m_ptr and m2_ptr regarding miscellaneous things.
 * m_ptr:
 *   Mob that's being processed.
 * m2_ptr:
 *   Check against this mob.
 * m:
 *   Index of the mob being processed.
 * m2:
 *   Index of the mob to check against.
 * d:
 *   Distance between the two.
 * pending_intermob_events:
 *   Vector of events to be processed.
 */
void gameplay_state::process_mob_misc_interactions(
    mob* m_ptr, mob* m2_ptr, const size_t m, const size_t m2, dist &d,
    vector<pending_intermob_event> &pending_intermob_events
) {
    //Find a carriable mob to grab.
    mob_event* nco_event =
        m_ptr->fsm.get_event(MOB_EV_NEAR_CARRIABLE_OBJECT);
    if(
        nco_event &&
        m2_ptr->carry_info &&
        !m2_ptr->carry_info->is_full()
    ) {
        dist d_between = m_ptr->get_distance_between(m2_ptr, &d);
        if(d_between <= task_range(m_ptr)) {
            pending_intermob_events.push_back(
                pending_intermob_event(d_between, nco_event, m2_ptr)
            );
        }
    }
    
    //Find a tool mob.
    mob_event* nto_event =
        m_ptr->fsm.get_event(MOB_EV_NEAR_TOOL);
    if(
        nto_event &&
        typeid(*m2_ptr) == typeid(tool)
    ) {
        dist d_between = m_ptr->get_distance_between(m2_ptr, &d);
        if(d_between <= task_range(m_ptr)) {
            tool* too_ptr = (tool*) m2_ptr;
            if(too_ptr->reserved && too_ptr->reserved != m_ptr) {
                //Another Pikmin is already going for it. Ignore it.
            } else {
                pending_intermob_events.push_back(
                    pending_intermob_event(d_between, nto_event, m2_ptr)
                );
            }
        }
    }
    
    //Find a group task mob.
    mob_event* ngto_event =
        m_ptr->fsm.get_event(MOB_EV_NEAR_GROUP_TASK);
    if(
        ngto_event &&
        m2_ptr->health > 0 &&
        typeid(*m2_ptr) == typeid(group_task)
    ) {
        dist d_between = m_ptr->get_distance_between(m2_ptr, &d);
        if(d_between <= task_range(m_ptr)) {
            group_task* tas_ptr = (group_task*) m2_ptr;
            group_task::group_task_spot* free_spot = tas_ptr->get_free_spot();
            if(!free_spot) {
                //There are no free spots here. Ignore it.
            } else {
                pending_intermob_events.push_back(
                    pending_intermob_event(d_between, ngto_event, m2_ptr)
                );
            }
        }
        
    }
    
    //"Bumped" by the active leader being nearby.
    mob_event* touch_le_ev =
        m_ptr->fsm.get_event(MOB_EV_TOUCHED_ACTIVE_LEADER);
    if(
        touch_le_ev &&
        m2_ptr == cur_leader_ptr &&
        //Small hack. This way,
        //Pikmin don't get bumped by leaders that are,
        //for instance, lying down.
        m2_ptr->fsm.cur_state->id == LEADER_STATE_ACTIVE &&
        d <= game.config.idle_bump_range
    ) {
        touch_le_ev->run(m_ptr, (void*) m2_ptr);
    }
}


/* ----------------------------------------------------------------------------
 * Handles the logic between m_ptr and m2_ptr regarding everything involving
 * one being in the other's reach.
 * m_ptr:
 *   Mob that's being processed.
 * m2_ptr:
 *   Check against this mob.
 * m:
 *   Index of the mob being processed.
 * m2:
 *   Index of the mob to check against.
 * d:
 *   Distance between the two.
 * pending_intermob_events:
 *   Vector of events to be processed.
 */
void gameplay_state::process_mob_reaches(
    mob* m_ptr, mob* m2_ptr, const size_t m, const size_t m2, dist &d,
    vector<pending_intermob_event> &pending_intermob_events
) {
    //Check reaches.
    mob_event* obir_ev =
        m_ptr->fsm.get_event(MOB_EV_OBJECT_IN_REACH);
    mob_event* opir_ev =
        m_ptr->fsm.get_event(MOB_EV_OPPONENT_IN_REACH);
        
    if(!obir_ev && !opir_ev) return;
    
    mob_type::reach_struct* r_ptr =
        &m_ptr->type->reaches[m_ptr->near_reach];
        
    dist d_between = m_ptr->get_distance_between(m2_ptr, &d);
    float face_diff =
        get_angle_smallest_dif(
            m_ptr->angle,
            get_angle(m_ptr->pos, m2_ptr->pos)
        );
        
    bool in_reach =
        (
            d_between <= r_ptr->radius_1 &&
            face_diff <= r_ptr->angle_1 / 2.0
        );
    if(!in_reach) {
        in_reach =
            (
                d_between <= r_ptr->radius_2 &&
                face_diff <= r_ptr->angle_2 / 2.0
            );
    }
    
    if(in_reach) {
        if(obir_ev) {
            pending_intermob_events.push_back(
                pending_intermob_event(
                    d_between, obir_ev, m2_ptr
                )
            );
        }
        if(opir_ev && m_ptr->can_hunt(m2_ptr)) {
            pending_intermob_events.push_back(
                pending_intermob_event(
                    d_between, opir_ev, m2_ptr
                )
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the logic between m_ptr and m2_ptr regarding everything involving
 * one touching the other.
 * m_ptr:
 *   Mob that's being processed.
 * m2_ptr:
 *   Check against this mob.
 * m:
 *   Index of the mob being processed.
 * m2:
 *   Index of the mob to check against.
 * d:
 *   Distance between the two.
 */
void gameplay_state::process_mob_touches(
    mob* m_ptr, mob* m2_ptr, const size_t m, const size_t m2, dist &d
) {
    //Check if mob 1 should be pushed by mob 2.
    bool both_idle_pikmin =
        m_ptr->type->category->id == MOB_CATEGORY_PIKMIN &&
        m2_ptr->type->category->id == MOB_CATEGORY_PIKMIN &&
        (
            ((pikmin*) m_ptr)->fsm.cur_state->id == PIKMIN_STATE_IDLING ||
            ((pikmin*) m_ptr)->fsm.cur_state->id == PIKMIN_STATE_IDLING_H
        ) && (
            ((pikmin*) m2_ptr)->fsm.cur_state->id == PIKMIN_STATE_IDLING ||
            ((pikmin*) m2_ptr)->fsm.cur_state->id == PIKMIN_STATE_IDLING_H
        );
    bool ok_to_push = true;
    if(
        has_flag(m_ptr->flags, MOB_FLAG_INTANGIBLE) ||
        has_flag(m2_ptr->flags, MOB_FLAG_INTANGIBLE)
    ) {
        ok_to_push = false;
    } else if(!m_ptr->type->pushable) {
        ok_to_push = false;
    } else if(has_flag(m_ptr->flags, MOB_FLAG_UNPUSHABLE)) {
        ok_to_push = false;
    } else if(m_ptr->standing_on_mob == m2_ptr) {
        ok_to_push = false;
    }
    
    if(
        ok_to_push &&
        (m2_ptr->type->pushes || both_idle_pikmin) && (
            (
                m2_ptr->z < m_ptr->z + m_ptr->height &&
                m2_ptr->z + m2_ptr->height > m_ptr->z
            ) || (
                m_ptr->height == 0
            ) || (
                m2_ptr->height == 0
            )
        ) && !(
            //If they are both being carried by Pikmin, one of them
            //shouldn't push, otherwise the Pikmin
            //can get stuck in a deadlock.
            m_ptr->carry_info && m_ptr->carry_info->is_moving &&
            m2_ptr->carry_info && m2_ptr->carry_info->is_moving &&
            m < m2
        )
    ) {
        float push_amount = 0;
        float push_angle = 0;
        
        if(m2_ptr->type->pushes_with_hitboxes) {
            //Push with the hitboxes.
            
            sprite* s2_ptr = m2_ptr->get_cur_sprite();
            
            for(size_t h = 0; h < s2_ptr->hitboxes.size(); ++h) {
                hitbox* h_ptr = &s2_ptr->hitboxes[h];
                if(h_ptr->type == HITBOX_TYPE_DISABLED) continue;
                point h_pos(
                    m2_ptr->pos.x + (
                        h_ptr->pos.x * m2_ptr->angle_cos -
                        h_ptr->pos.y * m2_ptr->angle_sin
                    ),
                    m2_ptr->pos.y + (
                        h_ptr->pos.x * m2_ptr->angle_sin +
                        h_ptr->pos.y * m2_ptr->angle_cos
                    )
                );
                //It's more optimized to get the hitbox position here
                //instead of calling hitbox::get_cur_pos because
                //we already know the sine and cosine, so they don't
                //need to be re-calculated.
                
                dist hd(m_ptr->pos, h_pos);
                if(hd < m_ptr->radius + h_ptr->radius) {
                    float p =
                        fabs(
                            hd.to_float() - m_ptr->radius -
                            h_ptr->radius
                        );
                    if(push_amount == 0 || p > push_amount) {
                        push_amount = p;
                        push_angle = get_angle(h_pos, m_ptr->pos);
                    }
                }
            }
            
        } else {
            bool xy_collision = false;
            float temp_push_amount = 0;
            float temp_push_angle = 0;
            if(
                m_ptr->rectangular_dim.x != 0 &&
                m2_ptr->rectangular_dim.x != 0
            ) {
                //Rectangle vs rectangle.
                xy_collision =
                    rectangles_intersect(
                        m_ptr->pos, m_ptr->rectangular_dim, m_ptr->angle,
                        m2_ptr->pos, m2_ptr->rectangular_dim, m2_ptr->angle,
                        &temp_push_amount, &temp_push_angle
                    );
            } else if(m_ptr->rectangular_dim.x != 0) {
                //Rectangle vs circle.
                xy_collision =
                    circle_intersects_rectangle(
                        m2_ptr->pos, m2_ptr->radius,
                        m_ptr->pos, m_ptr->rectangular_dim,
                        m_ptr->angle, &temp_push_amount, &temp_push_angle
                    );
                temp_push_angle += TAU / 2.0f;
            } else if(m2_ptr->rectangular_dim.x != 0) {
                //Circle vs rectangle.
                xy_collision =
                    circle_intersects_rectangle(
                        m_ptr->pos, m_ptr->radius,
                        m2_ptr->pos, m2_ptr->rectangular_dim,
                        m2_ptr->angle, &temp_push_amount, &temp_push_angle
                    );
            } else {
                //Circle vs circle.
                xy_collision =
                    d <= (m_ptr->radius + m2_ptr->radius);
                if(xy_collision) {
                    //Only bother calculating if there's a collision.
                    temp_push_amount =
                        fabs(
                            d.to_float() - m_ptr->radius -
                            m2_ptr->radius
                        );
                    temp_push_angle = get_angle(m2_ptr->pos, m_ptr->pos);
                }
            }
            
            if(xy_collision) {
                push_amount = temp_push_amount;
                if(m2_ptr->type->pushes_softly) {
                    push_amount =
                        std::min(
                            push_amount,
                            (float) (MOB::PUSH_SOFTLY_AMOUNT * game.delta_t)
                        );
                }
                push_angle = temp_push_angle;
                if(both_idle_pikmin) {
                    //Lower the push.
                    //Basically, make PUSH_EXTRA_AMOUNT do all the work.
                    push_amount = 0.1f;
                    //Deviate the angle slightly. This way, if two Pikmin
                    //are in the same spot, they don't drag each other forever.
                    push_angle += 0.1f * (m > m2);
                } else if(
                    m_ptr->time_alive < MOB::PUSH_THROTTLE_TIMEOUT ||
                    m2_ptr->time_alive < MOB::PUSH_THROTTLE_TIMEOUT
                ) {
                    //If either the pushed mob or the pusher mob spawned
                    //recently, then throttle the push. This avoids stuff like
                    //an enemy spoil pushing said enemy with insane force.
                    //Especially if there are multiple spoils.
                    //Setting the amount to 0.1 means it'll only really use the
                    //push provided by MOB_PUSH_EXTRA_AMOUNT.
                    float time_factor =
                        std::min(m_ptr->time_alive, m2_ptr->time_alive);
                    push_amount *=
                        time_factor /
                        MOB::PUSH_THROTTLE_TIMEOUT *
                        MOB::PUSH_THROTTLE_FACTOR;
                        
                }
            }
        }
        
        //If the mob is inside the other,
        //it needs to be pushed out.
        if((push_amount / game.delta_t) > m_ptr->push_amount) {
            m_ptr->push_amount = push_amount / game.delta_t;
            m_ptr->push_angle = push_angle;
        }
    }
    
    
    //Check touches. This does not use hitboxes,
    //only the object radii (or rectangular width/height).
    mob_event* touch_op_ev =
        m_ptr->fsm.get_event(MOB_EV_TOUCHED_OPPONENT);
    mob_event* touch_ob_ev =
        m_ptr->fsm.get_event(MOB_EV_TOUCHED_OBJECT);
    if(touch_op_ev || touch_ob_ev) {
    
        bool z_touch;
        if(
            m_ptr->height == 0 ||
            m2_ptr->height == 0
        ) {
            z_touch = true;
        } else {
            z_touch =
                !(
                    (m2_ptr->z > m_ptr->z + m_ptr->height) ||
                    (m2_ptr->z + m2_ptr->height < m_ptr->z)
                );
        }
        
        bool xy_collision = false;
        if(
            m_ptr->rectangular_dim.x != 0 &&
            m2_ptr->rectangular_dim.x != 0
        ) {
            //Rectangle vs rectangle.
            xy_collision =
                rectangles_intersect(
                    m_ptr->pos, m_ptr->rectangular_dim, m_ptr->angle,
                    m2_ptr->pos, m2_ptr->rectangular_dim, m2_ptr->angle
                );
        } else if(m_ptr->rectangular_dim.x != 0) {
            //Rectangle vs circle.
            xy_collision =
                circle_intersects_rectangle(
                    m2_ptr->pos, m2_ptr->radius,
                    m_ptr->pos, m_ptr->rectangular_dim,
                    m_ptr->angle
                );
        } else if(m2_ptr->rectangular_dim.x != 0) {
            //Circle vs rectangle.
            xy_collision =
                circle_intersects_rectangle(
                    m_ptr->pos, m_ptr->radius,
                    m2_ptr->pos, m2_ptr->rectangular_dim,
                    m2_ptr->angle
                );
        } else {
            //Circle vs circle.
            xy_collision =
                d <= (m_ptr->radius + m2_ptr->radius);
        }
        
        if(
            z_touch && !has_flag(m2_ptr->flags, MOB_FLAG_INTANGIBLE) &&
            xy_collision
        ) {
            if(touch_ob_ev) {
                touch_ob_ev->run(m_ptr, (void*) m2_ptr);
            }
            if(touch_op_ev && m_ptr->can_hunt(m2_ptr)) {
                touch_op_ev->run(m_ptr, (void*) m2_ptr);
            }
        }
        
    }
    
    //Check hitbox touches.
    mob_event* hitbox_touch_an_ev =
        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_A_N);
    mob_event* hitbox_touch_na_ev =
        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_N_A);
    mob_event* hitbox_touch_nn_ev =
        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_N_N);
    mob_event* hitbox_touch_eat_ev =
        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_EAT);
    mob_event* hitbox_touch_haz_ev =
        m_ptr->fsm.get_event(MOB_EV_TOUCHED_HAZARD);
        
    sprite* s1_ptr = m_ptr->get_cur_sprite();
    sprite* s2_ptr = m2_ptr->get_cur_sprite();
    
    if(
        (
            hitbox_touch_an_ev || hitbox_touch_na_ev || hitbox_touch_nn_ev ||
            hitbox_touch_eat_ev
        ) &&
        s1_ptr && s2_ptr &&
        !s1_ptr->hitboxes.empty() && !s2_ptr->hitboxes.empty()
    ) {
    
        bool reported_an_ev = false;
        bool reported_na_ev = false;
        bool reported_nn_ev = false;
        bool reported_eat_ev = false;
        bool reported_haz_ev = false;
        
        for(size_t h1 = 0; h1 < s1_ptr->hitboxes.size(); ++h1) {
        
            hitbox* h1_ptr = &s1_ptr->hitboxes[h1];
            if(h1_ptr->type == HITBOX_TYPE_DISABLED) continue;
            
            for(size_t h2 = 0; h2 < s2_ptr->hitboxes.size(); ++h2) {
                hitbox* h2_ptr = &s2_ptr->hitboxes[h2];
                if(h2_ptr->type == HITBOX_TYPE_DISABLED) continue;
                
                //Get the real hitbox locations.
                point m1_h_pos =
                    h1_ptr->get_cur_pos(
                        m_ptr->pos, m_ptr->angle_cos, m_ptr->angle_sin
                    );
                point m2_h_pos =
                    h2_ptr->get_cur_pos(
                        m2_ptr->pos, m2_ptr->angle_cos, m2_ptr->angle_sin
                    );
                float m1_h_z = m_ptr->z + h1_ptr->z;
                float m2_h_z = m2_ptr->z + h2_ptr->z;
                
                bool collided = false;
                
                if(
                    (
                        m_ptr->holder.m == m2_ptr &&
                        m_ptr->holder.hitbox_nr == h2
                    ) || (
                        m2_ptr->holder.m == m_ptr &&
                        m2_ptr->holder.hitbox_nr == h1
                    )
                ) {
                    //Mobs held by a hitbox are obviously touching it.
                    collided = true;
                }
                
                if(!collided) {
                    bool z_collision;
                    if(h1_ptr->height == 0 || h2_ptr->height == 0) {
                        z_collision = true;
                    } else {
                        z_collision =
                            !(
                                (m2_h_z > m1_h_z + h1_ptr->height) ||
                                (m2_h_z + h2_ptr->height < m1_h_z)
                            );
                    }
                    
                    if(
                        z_collision &&
                        dist(m1_h_pos, m2_h_pos) <
                        (h1_ptr->radius + h2_ptr->radius)
                    ) {
                        collided = true;
                    }
                }
                
                if(!collided) continue;
                
                //Collision confirmed!
                
                if(
                    hitbox_touch_an_ev && !reported_an_ev &&
                    h1_ptr->type == HITBOX_TYPE_ATTACK &&
                    h2_ptr->type == HITBOX_TYPE_NORMAL
                ) {
                    hitbox_interaction ev_info =
                        hitbox_interaction(
                            m2_ptr, h1_ptr, h2_ptr
                        );
                        
                    hitbox_touch_an_ev->run(
                        m_ptr, (void*) &ev_info
                    );
                    reported_an_ev = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitbox_touch_eat_ev =
                        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_EAT);
                    hitbox_touch_haz_ev =
                        m_ptr->fsm.get_event(MOB_EV_TOUCHED_HAZARD);
                    hitbox_touch_na_ev =
                        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_N_A);
                    hitbox_touch_nn_ev =
                        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_N_N);
                }
                
                if(
                    hitbox_touch_nn_ev && !reported_nn_ev &&
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    h2_ptr->type == HITBOX_TYPE_NORMAL
                ) {
                    hitbox_interaction ev_info =
                        hitbox_interaction(
                            m2_ptr, h1_ptr, h2_ptr
                        );
                        
                    hitbox_touch_nn_ev->run(
                        m_ptr, (void*) &ev_info
                    );
                    reported_nn_ev = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitbox_touch_eat_ev =
                        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_EAT);
                    hitbox_touch_haz_ev =
                        m_ptr->fsm.get_event(MOB_EV_TOUCHED_HAZARD);
                    hitbox_touch_na_ev =
                        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_N_A);
                    hitbox_touch_an_ev =
                        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_A_N);
                }
                
                if(
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    h2_ptr->type == HITBOX_TYPE_ATTACK
                ) {
                    //Confirmed damage.
                    
                    //Hazard resistance check.
                    if(
                        !h2_ptr->hazards.empty() &&
                        m_ptr->is_resistant_to_hazards(h2_ptr->hazards)
                    ) {
                        continue;
                    }
                    
                    //Should this mob even attack this other mob?
                    if(!m2_ptr->can_hurt(m_ptr)) {
                        continue;
                    }
                }
                
                //Check if m2 is under any status effect
                //that disables attacks.
                bool disable_attack_status = false;
                for(size_t s = 0; s < m2_ptr->statuses.size(); ++s) {
                    if(m2_ptr->statuses[s].type->disables_attack) {
                        disable_attack_status = true;
                        break;
                    }
                }
                
                //First, the "touched eat hitbox" event.
                if(
                    hitbox_touch_eat_ev &&
                    !reported_eat_ev &&
                    !disable_attack_status &&
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    m2_ptr->chomping_mobs.size() <
                    m2_ptr->chomp_max &&
                    find(
                        m2_ptr->chomp_body_parts.begin(),
                        m2_ptr->chomp_body_parts.end(),
                        h2_ptr->body_part_index
                    ) !=
                    m2_ptr->chomp_body_parts.end()
                ) {
                    hitbox_touch_eat_ev->run(
                        m_ptr,
                        (void*) m2_ptr,
                        (void*) h2_ptr
                    );
                    reported_eat_ev = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitbox_touch_haz_ev =
                        m_ptr->fsm.get_event(MOB_EV_TOUCHED_HAZARD);
                    hitbox_touch_na_ev =
                        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_N_A);
                }
                
                //"Touched hazard" event.
                if(
                    hitbox_touch_haz_ev &&
                    !reported_haz_ev &&
                    !disable_attack_status &&
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    h2_ptr->type == HITBOX_TYPE_ATTACK &&
                    !h2_ptr->hazards.empty()
                ) {
                    for(
                        size_t h = 0;
                        h < h2_ptr->hazards.size(); ++h
                    ) {
                        hitbox_interaction ev_info =
                            hitbox_interaction(
                                m2_ptr, h1_ptr, h2_ptr
                            );
                        hitbox_touch_haz_ev->run(
                            m_ptr,
                            (void*) h2_ptr->hazards[h],
                            (void*) &ev_info
                        );
                    }
                    reported_haz_ev = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitbox_touch_na_ev =
                        m_ptr->fsm.get_event(MOB_EV_HITBOX_TOUCH_N_A);
                }
                
                //"Normal hitbox touched attack hitbox" event.
                if(
                    hitbox_touch_na_ev &&
                    !reported_na_ev &&
                    !disable_attack_status &&
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    h2_ptr->type == HITBOX_TYPE_ATTACK
                ) {
                    hitbox_interaction ev_info =
                        hitbox_interaction(
                            m2_ptr, h1_ptr, h2_ptr
                        );
                    hitbox_touch_na_ev->run(
                        m_ptr, (void*) &ev_info
                    );
                    reported_na_ev = true;
                    
                }
            }
        }
    }
}
