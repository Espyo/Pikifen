/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Main game loop logic.
 */

#include <algorithm>

#include "gameplay.h"

#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "mobs/group_task.h"
#include "mobs/pikmin.h"
#include "mobs/tool.h"
#include "utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Ticks the logic of aesthetic things. If the game is paused, these can
 * be frozen in place without any negative impact.
 */
void gameplay::do_aesthetic_logic() {

    /*************************************
    *                               .-.  *
    *   Timer things - aesthetic   ( L ) *
    *                               `-´  *
    **************************************/
    
    //Swarming arrows.
    if(swarm_magnitude) {
        swarm_next_arrow_timer.tick(game.delta_t);
    }
    
    dist leader_to_cursor_dist(cur_leader_ptr->pos, leader_cursor_w);
    for(size_t a = 0; a < swarm_arrows.size(); ) {
        swarm_arrows[a] += SWARM_ARROW_SPEED * game.delta_t;
        
        dist max_dist =
            (swarm_magnitude > 0) ?
            game.config.cursor_max_dist * swarm_magnitude :
            leader_to_cursor_dist;
            
        if(max_dist < swarm_arrows[a]) {
            swarm_arrows.erase(swarm_arrows.begin() + a);
        } else {
            a++;
        }
    }
    
    whistle.tick(
        game.delta_t,
        cur_leader_ptr->lea_type->whistle_range, leader_to_cursor_dist
    );
    
    //Cursor trail.
    if(game.options.draw_cursor_trail) {
        cursor_save_timer.tick(game.delta_t);
    }
    
    //Where the cursor is.
    cursor_height_diff_light = 0;
    
    leader_cursor_mob = NULL;
    for(size_t m = 0; m < mobs.all.size(); ++m) {
        mob* m_ptr = mobs.all[m];
        if(!bbox_check(leader_cursor_w, m_ptr->pos, m_ptr->type->max_span)) {
            //Too far away; of course the cursor isn't on it.
            continue;
        }
        if(
            leader_cursor_mob &&
            m_ptr->z + m_ptr->height <
            leader_cursor_mob->z + leader_cursor_mob->height
        ) {
            //If this mob is lower than the previous known "under cursor" mob,
            //then forget it.
            continue;
        }
        if(dist(leader_cursor_w, m_ptr->pos) > m_ptr->type->radius) {
            //The cursor is not really on top of this mob.
            continue;
        }
        leader_cursor_mob = m_ptr;
    }
    
    leader_cursor_sector =
        get_sector(leader_cursor_w, NULL, true);
        
    if(leader_cursor_sector) {
        cursor_height_diff_light =
            (leader_cursor_sector->z - cur_leader_ptr->z) * 0.0033;
        cursor_height_diff_light =
            clamp(cursor_height_diff_light, -0.33f, 0.33f);
    }
    
    //Whether the held Pikmin can reach the cursor.
    throw_can_reach_cursor = true;
    if(!cur_leader_ptr->holding.empty()) {
        mob* held_mob = cur_leader_ptr->holding[0];
        
        if(
            !leader_cursor_sector ||
            leader_cursor_sector->type == SECTOR_TYPE_BLOCKING
        ) {
            throw_can_reach_cursor = false;
            
        } else {
            float max_throw_z = 0;
            size_t cat = held_mob->type->category->id;
            switch(cat) {
            case MOB_CATEGORY_PIKMIN: {
                max_throw_z =
                    ((pikmin*) held_mob)->pik_type->max_throw_height;
                break;
            } case MOB_CATEGORY_LEADERS: {
                max_throw_z =
                    ((leader*) held_mob)->lea_type->max_throw_height;
                break;
            }
            }
            
            if(max_throw_z > 0) {
                throw_can_reach_cursor =
                    leader_cursor_sector->z <= cur_leader_ptr->z + max_throw_z;
            }
        }
    }
    
    
    //Specific animations.
    game.sys_assets.spark_animation.instance.tick(game.delta_t);
    
    //Area title fade.
    area_title_fade_timer.tick(game.delta_t);
    
    //Fade.
    game.fade_mgr.tick(game.delta_t);
    
    
}


/* ----------------------------------------------------------------------------
 * Ticks the logic of gameplay-related things.
 */
void gameplay::do_gameplay_logic() {

    //Camera movement.
    game.cam.tick(game.delta_t);
    
    update_transformations();
    
    game.cam.update_box();
    
    if(!msg_box) {
    
        /************************************
        *                              .-.  *
        *   Timer things - gameplay   ( L ) *
        *                              `-´  *
        *************************************/
        
        day_minutes += (game.config.day_minutes_per_irl_sec * game.delta_t);
        if(day_minutes > 60 * 24) day_minutes -= 60 * 24;
        
        area_time_passed += game.delta_t;
        
        if(game.perf_mon) {
            game.perf_mon->start_measurement("Logic -- Particles");
        }
        
        //Tick all particles.
        particles.tick_all(game.delta_t);
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
        }
        
        //Ticks all status effect animations.
        for(auto &s : game.status_types) {
            s.second->anim_instance.tick(game.delta_t);
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
            
                s_ptr->liquid_drain_left -= game.delta_t;
                
                if(s_ptr->liquid_drain_left <= 0) {
                
                    for(size_t h = 0; h < s_ptr->hazards.size();) {
                        if(s_ptr->hazards[h]->associated_liquid) {
                            s_ptr->hazards.erase(s_ptr->hazards.begin() + h);
                        } else {
                            ++h;
                        }
                    }
                    
                    s_ptr->liquid_drain_left = 0;
                    s_ptr->draining_liquid = false;
                }
            }
            
            if(s_ptr->scroll.x != 0 || s_ptr->scroll.y != 0) {
                s_ptr->texture_info.translation += s_ptr->scroll * game.delta_t;
            }
        }
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
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
            whistle.radius += game.config.whistle_growth_speed * game.delta_t;
            if(whistle.radius > cur_leader_ptr->lea_type->whistle_range) {
                whistle.radius = cur_leader_ptr->lea_type->whistle_range;
            }
        }
        
        
        /*****************
        *                *
        *   Mobs   ()--> *
        *                *
        ******************/
        size_t n_mobs = mobs.all.size();
        for(size_t m = 0; m < n_mobs; ++m) {
            //Tick the mob.
            mob* m_ptr = mobs.all[m];
            m_ptr->tick(game.delta_t);
            
            if(m_ptr->fsm.cur_state) {
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
        
        
        /*******************
        *             .-.  *
        *   Leader   (*:O) *
        *             `-´  *
        *******************/
        if(game.perf_mon) {
            game.perf_mon->start_measurement("Logic -- Current leader");
        }
        
        //Current leader movement.
        point dummy_coords;
        float dummy_angle;
        float leader_move_magnitude;
        leader_movement.get_clean_info(
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
        
        game.cam.target_pos = cur_leader_ptr->pos;
        
        //Check proximity with certain key things.
        if(!cur_leader_ptr->auto_plucking) {
            dist closest_d = 0;
            dist d = 0;
            bool done = false;
            
            close_to_ship_to_heal = NULL;
            for(size_t s = 0; s < mobs.ships.size(); ++s) {
                ship* s_ptr = mobs.ships[s];
                d = dist(cur_leader_ptr->pos, s_ptr->pos);
                if(!s_ptr->is_leader_under_beam(cur_leader_ptr)) {
                    continue;
                }
                if(cur_leader_ptr->health == cur_leader_ptr->type->max_health) {
                    continue;
                }
                if(!s_ptr->shi_type->can_heal) {
                    continue;
                }
                if(d < closest_d || !close_to_ship_to_heal) {
                    close_to_ship_to_heal = s_ptr;
                    closest_d = d;
                    done = true;
                }
            }
            
            closest_d = 0;
            d = 0;
            close_to_pikmin_to_pluck = NULL;
            if(!done) {
                pikmin* p = get_closest_sprout(cur_leader_ptr->pos, &d, false);
                if(p && d <= game.config.pluck_range) {
                    close_to_pikmin_to_pluck = p;
                    done = true;
                }
            }
            
            closest_d = 0;
            d = 0;
            close_to_onion_to_open = NULL;
            if(!done) {
                for(size_t o = 0; o < mobs.onions.size(); ++o) {
                    d = dist(cur_leader_ptr->pos, mobs.onions[o]->pos);
                    if(d > game.config.onion_open_range) continue;
                    if(d < closest_d || !close_to_onion_to_open) {
                        close_to_onion_to_open = mobs.onions[o];
                        closest_d = d;
                        done = true;
                    }
                }
            }
            
            closest_d = 0;
            d = 0;
            close_to_interactable_to_use = NULL;
            if(!done) {
                for(size_t i = 0; i < mobs.interactables.size(); ++i) {
                    d = dist(cur_leader_ptr->pos, mobs.interactables[i]->pos);
                    if(d > mobs.interactables[i]->int_type->trigger_range) continue;
                    if(d < closest_d || !close_to_interactable_to_use) {
                        close_to_interactable_to_use = mobs.interactables[i];
                        closest_d = d;
                        done = true;
                    }
                }
            }
        }
        
        
        /********************
        *             .-.   *
        *   Cursor   ( = )> *
        *             `-´   *
        ********************/
        
        point mouse_cursor_speed;
        float dummy_magnitude;
        cursor_movement.get_clean_info(
            &mouse_cursor_speed, &dummy_angle, &dummy_magnitude
        );
        mouse_cursor_speed =
            mouse_cursor_speed * game.delta_t* MOUSE_CURSOR_MOVE_SPEED;
            
        game.mouse_cursor_s += mouse_cursor_speed;
        
        game.mouse_cursor_w = game.mouse_cursor_s;
        al_transform_coordinates(
            &game.screen_to_world_transform,
            &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
        );
        leader_cursor_w = game.mouse_cursor_w;
        
        float cursor_angle = get_angle(cur_leader_ptr->pos, leader_cursor_w);
        
        dist leader_to_cursor_dist(cur_leader_ptr->pos, leader_cursor_w);
        if(leader_to_cursor_dist > game.config.cursor_max_dist) {
            //Cursor goes beyond the range limit.
            leader_cursor_w.x =
                cur_leader_ptr->pos.x + (cos(cursor_angle) * game.config.cursor_max_dist);
            leader_cursor_w.y =
                cur_leader_ptr->pos.y + (sin(cursor_angle) * game.config.cursor_max_dist);
                
            if(mouse_cursor_speed.x != 0 || mouse_cursor_speed.y != 0) {
                //If we're speeding the mouse cursor (via analog stick),
                //don't let it go beyond the edges.
                game.mouse_cursor_w = leader_cursor_w;
                game.mouse_cursor_s = game.mouse_cursor_w;
                al_transform_coordinates(
                    &game.world_to_screen_transform,
                    &game.mouse_cursor_s.x, &game.mouse_cursor_s.y
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
        
        size_t n_members = cur_leader_ptr->group->members.size();
        closest_group_member = NULL;
        if(!cur_leader_ptr->holding.empty()) {
            closest_group_member = cur_leader_ptr->holding[0];
        }
        closest_group_member_distant = false;
        
        if(n_members > 0 && !closest_group_member) {
        
            update_closest_group_member();
        }
        
        float old_swarm_magnitude = swarm_magnitude;
        point swarm_coords;
        float new_swarm_angle;
        swarm_movement.get_clean_info(
            &swarm_coords, &new_swarm_angle, &swarm_magnitude
        );
        if(swarm_magnitude > 0) {
            //This stops arrows that were fading away to the left from
            //turning to angle 0 because the magnitude reached 0.
            swarm_angle = new_swarm_angle;
        }
        
        if(swarm_cursor) {
            swarm_angle = cursor_angle;
            float leader_to_cursor_dist =
                dist(cur_leader_ptr->pos, leader_cursor_w).to_float();
            swarm_magnitude =
                leader_to_cursor_dist / game.config.cursor_max_dist;
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
            l.second->anim_instance.tick(game.delta_t);
        }
        
    } else { //Displaying a message.
    
        msg_box->tick(game.delta_t);
    }
    
    hud_items.tick(game.delta_t);
    
    replay_timer.tick(game.delta_t);
    
    //Process and print framerate and system info.
    if(game.show_system_info) {
    
        game.framerate_history.push_back(1.0f / game.delta_t);
        if(game.framerate_history.size() > FRAMERATE_HISTORY_SIZE) {
            game.framerate_history.erase(game.framerate_history.begin());
        }
        
        game.framerate_last_avg_point++;
        
        float sample_avg;
        
        if(game.framerate_last_avg_point >= FRAMERATE_AVG_SAMPLE_SIZE) {
            //Let's get an average, using FRAMERATE_AVG_SAMPLE_SIZE frames.
            //If we can fit a sample of this size using the most recent
            //unsampled frames, then use those. Otherwise, keep using the last
            //block, which starts at framerate_last_avg_point.
            //This makes it so the average stays the same for a bit of time,
            //so the player can actually read it.
            if(game.framerate_last_avg_point > FRAMERATE_AVG_SAMPLE_SIZE * 2) {
                game.framerate_last_avg_point = FRAMERATE_AVG_SAMPLE_SIZE;
            }
            float sample_avg_sum = 0;
            size_t sample_avg_point_count = 0;
            size_t sample_size =
                std::min(
                    (size_t) FRAMERATE_AVG_SAMPLE_SIZE,
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
            box_string(f2s(1.0f / game.delta_t), 12, " now, ") +
            i2s(game.options.target_fps) + " intended";
        string n_mobs_str =
            box_string(i2s(mobs.all.size()), 7);
        string n_particles_str =
            box_string(i2s(particles.get_count()), 7);
        string resolution_str =
            i2s(game.win_w) + "x" + i2s(game.win_h);
        string area_v_str =
            game.cur_area_data.version;
        string area_maker_str =
            game.cur_area_data.maker;
        string engine_v_str =
            i2s(VERSION_MAJOR) + "." +
            i2s(VERSION_MINOR) + "." +
            i2s(VERSION_REV);
        string game_v_str =
            game.config.version;
            
        print_info(
            "FPS: " + fps_str +
            "\n"
            "Mobs: " + n_mobs_str + " Particles: " + n_particles_str +
            "\n"
            "Resolution: " + resolution_str +
            "\n"
            "Area version " + area_v_str + ", by " + area_maker_str +
            "\n"
            "Pikifen version " + engine_v_str +
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
                    f2s(game.maker_tools.info_lock->type->max_health), 6
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
    
    //Print mouse coordinates.
    if(game.maker_tools.geometry_info) {
        sector* mouse_sector =
            get_sector(game.mouse_cursor_w, NULL, true);
            
        string coords_str =
            box_string(f2s(game.mouse_cursor_w.x), 6) + " " +
            box_string(f2s(game.mouse_cursor_w.y), 6);
        string blockmap_str =
            box_string(
                i2s(game.cur_area_data.bmap.get_col(game.mouse_cursor_w.x)),
                5, " "
            ) +
            i2s(game.cur_area_data.bmap.get_row(game.mouse_cursor_w.y));
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
    
    if(!ready_for_input) {
        ready_for_input = true;
        is_input_allowed = true;
    }
    
}


/* ----------------------------------------------------------------------------
 * Handles the logic required to tick a specific mob and its interactions
 * with other mobs.
 */
void gameplay::process_mob_interactions(mob* m_ptr, size_t m) {
    vector<pending_intermob_event> pending_intermob_events;
    mob_state* state_before = m_ptr->fsm.cur_state;
    
    size_t n_mobs = mobs.all.size();
    for(size_t m2 = 0; m2 < n_mobs; ++m2) {
        if(m == m2) continue;
        
        mob* m2_ptr = mobs.all[m2];
        if(m2_ptr->to_delete) continue;
        
        dist d(m_ptr->pos, m2_ptr->pos);
        
        if(game.perf_mon) {
            game.perf_mon->start_measurement("Objects -- Touching others");
        }
        
        if(d <= m_ptr->type->max_span + m2_ptr->type->max_span) {
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
            (m_ptr->type->radius + e1.mob_ptr->type->radius)
        ) < (
            e2.d.to_float() -
            (m_ptr->type->radius + e2.mob_ptr->type->radius)
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
 * m_ptr:                   Mob that's being processed.
 * m2_ptr:                  Check against this mob.
 * m:                       Index of the mob being processed.
 * m2:                      Index of the mob to check against.
 * d:                       Distance between the two.
 * pending_intermob_events: Vector of events to be processed.
 */
void gameplay::process_mob_misc_interactions(
    mob* m_ptr, mob* m2_ptr, const size_t m, const size_t m2, dist &d,
    vector<pending_intermob_event> &pending_intermob_events
) {
    //Find a carriable mob to grab.
    mob_event* nco_event =
        q_get_event(m_ptr, MOB_EV_NEAR_CARRIABLE_OBJECT);
    if(
        nco_event &&
        m2_ptr->carry_info &&
        !m2_ptr->carry_info->is_full() &&
        d <=
        m_ptr->type->radius + m2_ptr->type->radius + task_range(m_ptr)
    ) {
    
        pending_intermob_events.push_back(
            pending_intermob_event(
                d, nco_event, m2_ptr
            )
        );
        
    }
    
    //Find a tool mob.
    mob_event* nto_event =
        q_get_event(m_ptr, MOB_EV_NEAR_TOOL);
    if(
        nto_event &&
        d <=
        m_ptr->type->radius + m2_ptr->type->radius + task_range(m_ptr) &&
        typeid(*m2_ptr) == typeid(tool)
    ) {
        tool* too_ptr = (tool*) m2_ptr;
        if(too_ptr->reserved && too_ptr->reserved != m_ptr) {
            //Another Pikmin is already going for it. Ignore it.
        } else {
            pending_intermob_events.push_back(
                pending_intermob_event(d, nto_event, m2_ptr)
            );
        }
    }
    
    //Find a group task mob.
    mob_event* ngto_event =
        q_get_event(m_ptr, MOB_EV_NEAR_GROUP_TASK);
    if(
        ngto_event &&
        m2_ptr->health > 0 &&
        d <=
        m_ptr->type->radius + m2_ptr->type->radius + task_range(m_ptr) &&
        typeid(*m2_ptr) == typeid(group_task)
    ) {
        group_task* tas_ptr = (group_task*) m2_ptr;
        group_task::group_task_spot* free_spot = tas_ptr->get_free_spot();
        if(!free_spot) {
            //There are no free spots here. Ignore it.
        } else {
            pending_intermob_events.push_back(
                pending_intermob_event(d, ngto_event, m2_ptr)
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the logic between m_ptr and m2_ptr regarding everything involving
 * one being in the other's reach.
 * m_ptr:                   Mob that's being processed.
 * m2_ptr:                  Check against this mob.
 * m:                       Index of the mob being processed.
 * m2:                      Index of the mob to check against.
 * d:                       Distance between the two.
 * pending_intermob_events: Vector of events to be processed.
 */
void gameplay::process_mob_reaches(
    mob* m_ptr, mob* m2_ptr, const size_t m, const size_t m2, dist &d,
    vector<pending_intermob_event> &pending_intermob_events
) {
    //Check reaches.
    mob_event* obir_ev =
        q_get_event(m_ptr, MOB_EV_OBJECT_IN_REACH);
    mob_event* opir_ev =
        q_get_event(m_ptr, MOB_EV_OPPONENT_IN_REACH);
        
    mob_type::reach_struct* r_ptr =
        &m_ptr->type->reaches[m_ptr->near_reach];
        
    if(!obir_ev && !opir_ev) return;
    
    float face_diff =
        get_angle_smallest_dif(
            m_ptr->angle,
            get_angle(m_ptr->pos, m2_ptr->pos)
        );
        
    if(
        (
            d <= r_ptr->radius_1 +
            (m_ptr->type->radius + m2_ptr->type->radius) &&
            face_diff <= r_ptr->angle_1 / 2.0
        ) || (
            d <= r_ptr->radius_2 +
            (m_ptr->type->radius + m2_ptr->type->radius) &&
            face_diff <= r_ptr->angle_2 / 2.0
        )
        
    ) {
        if(obir_ev) {
            pending_intermob_events.push_back(
                pending_intermob_event(
                    d, obir_ev, m2_ptr
                )
            );
        }
        if(opir_ev && m_ptr->can_hunt(m2_ptr)) {
            pending_intermob_events.push_back(
                pending_intermob_event(
                    d, opir_ev, m2_ptr
                )
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles the logic between m_ptr and m2_ptr regarding everything involving
 * one touching the other.
 * m_ptr:        Mob that's being processed.
 * m2_ptr:       Check against this mob.
 * m:            Index of the mob being processed.
 * m2:           Index of the mob to check against.
 * d:            Distance between the two.
 */
void gameplay::process_mob_touches(
    mob* m_ptr, mob* m2_ptr, const size_t m, const size_t m2, dist &d
) {
    //Check if mob 1 should be pushed by mob 2.
    if(
        m2_ptr->type->pushes &&
        m2_ptr->tangible &&
        m_ptr->type->pushable && !m_ptr->unpushable &&
        m_ptr->standing_on_mob != m2_ptr &&
        (
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
            
            sprite* s2_ptr = m2_ptr->anim.get_cur_sprite();
            
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
                if(hd < m_ptr->type->radius + h_ptr->radius) {
                    float p =
                        fabs(
                            hd.to_float() - m_ptr->type->radius -
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
                m_ptr->type->rectangular_dim.x != 0 &&
                m2_ptr->type->rectangular_dim.x != 0
            ) {
                //Rectangle vs rectangle.
                //Not supported.
            } else if(m_ptr->type->rectangular_dim.x != 0) {
                //Rectangle vs circle.
                xy_collision =
                    circle_intersects_rectangle(
                        m2_ptr->pos, m2_ptr->type->radius,
                        m_ptr->pos, m_ptr->type->rectangular_dim,
                        m_ptr->angle, &temp_push_amount, &temp_push_angle
                    );
            } else if(m2_ptr->type->rectangular_dim.x != 0) {
                //Circle vs rectangle.
                xy_collision =
                    circle_intersects_rectangle(
                        m_ptr->pos, m_ptr->type->radius,
                        m2_ptr->pos, m2_ptr->type->rectangular_dim,
                        m2_ptr->angle, &temp_push_amount, &temp_push_angle
                    );
            } else {
                //Circle vs circle.
                xy_collision =
                    d <= (m_ptr->type->radius + m2_ptr->type->radius);
                temp_push_amount =
                    fabs(
                        d.to_float() - m_ptr->type->radius -
                        m2_ptr->type->radius
                    );
                temp_push_angle = get_angle(m2_ptr->pos, m_ptr->pos);
            }
            
            if(xy_collision) {
                push_amount = temp_push_amount;
                push_angle = temp_push_angle;
            }
        }
        
        //If the mob is inside the other,
        //it needs to be pushed out.
        if(push_amount > m_ptr->push_amount) {
            m_ptr->push_amount = push_amount / game.delta_t;
            m_ptr->push_angle = push_angle;
        }
    }
    
    
    //Check touches. This does not use hitboxes,
    //only the object radii (or rectangular width/height).
    mob_event* touch_op_ev =
        q_get_event(m_ptr, MOB_EV_TOUCHED_OPPONENT);
    mob_event* touch_le_ev =
        q_get_event(m_ptr, MOB_EV_TOUCHED_ACTIVE_LEADER);
    mob_event* touch_ob_ev =
        q_get_event(m_ptr, MOB_EV_TOUCHED_OBJECT);
    mob_event* pik_land_ev =
        q_get_event(m_ptr, MOB_EV_THROWN_PIKMIN_LANDED);
    if(
        touch_op_ev || touch_le_ev ||
        touch_ob_ev || pik_land_ev
    ) {
    
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
            m_ptr->type->rectangular_dim.x != 0 &&
            m2_ptr->type->rectangular_dim.x != 0
        ) {
            //Rectangle vs rectangle.
            //Not supported.
            xy_collision = false;
        } else if(m_ptr->type->rectangular_dim.x != 0) {
            //Rectangle vs circle.
            xy_collision =
                circle_intersects_rectangle(
                    m2_ptr->pos, m2_ptr->type->radius,
                    m_ptr->pos, m_ptr->type->rectangular_dim,
                    m_ptr->angle
                );
        } else if(m2_ptr->type->rectangular_dim.x != 0) {
            //Circle vs rectangle.
            xy_collision =
                circle_intersects_rectangle(
                    m_ptr->pos, m_ptr->type->radius,
                    m2_ptr->pos, m2_ptr->type->rectangular_dim,
                    m2_ptr->angle
                );
        } else {
            //Circle vs circle.
            xy_collision =
                d <= (m_ptr->type->radius + m2_ptr->type->radius);
        }
        
        if(z_touch && m2_ptr->tangible && xy_collision) {
            if(touch_ob_ev) {
                touch_ob_ev->run(m_ptr, (void*) m2_ptr);
            }
            if(touch_op_ev && m_ptr->can_hunt(m2_ptr)) {
                touch_op_ev->run(m_ptr, (void*) m2_ptr);
            }
            if(
                pik_land_ev &&
                m2_ptr->was_thrown &&
                m2_ptr->type->category->id == MOB_CATEGORY_PIKMIN
            ) {
                pik_land_ev->run(m_ptr, (void*) m2_ptr);
            }
            if(
                touch_le_ev && m2_ptr == cur_leader_ptr &&
                //Small hack. This way,
                //Pikmin don't get bumped by leaders that are,
                //for instance, lying down.
                m2_ptr->fsm.cur_state->id == LEADER_STATE_ACTIVE
            ) {
                touch_le_ev->run(m_ptr, (void*) m2_ptr);
            }
        }
        
    }
    
    //Check hitbox touches.
    mob_event* hitbox_touch_an_ev =
        q_get_event(m_ptr, MOB_EV_HITBOX_TOUCH_A_N);
    mob_event* hitbox_touch_na_ev =
        q_get_event(m_ptr, MOB_EV_HITBOX_TOUCH_N_A);
    mob_event* hitbox_touch_eat_ev =
        q_get_event(m_ptr, MOB_EV_HITBOX_TOUCH_EAT);
    mob_event* hitbox_touch_haz_ev =
        q_get_event(m_ptr, MOB_EV_TOUCHED_HAZARD);
        
    sprite* s1_ptr = m_ptr->anim.get_cur_sprite();
    sprite* s2_ptr = m2_ptr->anim.get_cur_sprite();
    
    if(
        (hitbox_touch_an_ev || hitbox_touch_na_ev || hitbox_touch_eat_ev) &&
        s1_ptr && s2_ptr &&
        !s1_ptr->hitboxes.empty() && !s2_ptr->hitboxes.empty()
    ) {
    
        bool reported_an_ev = false;
        bool reported_na_ev = false;
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
                    hitbox_touch_an_ev &&
                    !reported_an_ev &&
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
                        q_get_event(m_ptr, MOB_EV_HITBOX_TOUCH_EAT);
                    hitbox_touch_haz_ev =
                        q_get_event(m_ptr, MOB_EV_TOUCHED_HAZARD);
                    hitbox_touch_na_ev =
                        q_get_event(m_ptr, MOB_EV_HITBOX_TOUCH_N_A);
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
                        q_get_event(m_ptr, MOB_EV_TOUCHED_HAZARD);
                    hitbox_touch_na_ev =
                        q_get_event(m_ptr, MOB_EV_HITBOX_TOUCH_N_A);
                }
                
                //"Touched hazard" event.
                if(
                    hitbox_touch_haz_ev &&
                    !reported_haz_ev &&
                    !disable_attack_status &&
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
                        q_get_event(m_ptr, MOB_EV_HITBOX_TOUCH_N_A);
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
