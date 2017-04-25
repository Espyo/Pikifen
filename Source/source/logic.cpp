/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Main game loop logic.
 */

#include <algorithm>

#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "mobs/pikmin.h"
#include "vars.h"

const float CAMERA_SMOOTHNESS_MULT = 4.5f;

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
    
    //Camera movement.
    cam_pos.x +=
        (cam_final_pos.x - cam_pos.x) * (CAMERA_SMOOTHNESS_MULT * delta_t);
    cam_pos.y +=
        (cam_final_pos.y - cam_pos.y) * (CAMERA_SMOOTHNESS_MULT * delta_t);
    cam_zoom +=
        (cam_final_zoom - cam_zoom) * (CAMERA_SMOOTHNESS_MULT * delta_t);
        
    //"Move group" arrows.
    if(group_move_intensity) {
        group_move_next_arrow_timer.tick(delta_t);
    }
    
    dist leader_to_cursor_dist(cur_leader_ptr->pos, leader_cursor_w);
    for(size_t a = 0; a < group_move_arrows.size(); ) {
        group_move_arrows[a] += GROUP_MOVE_ARROW_SPEED * delta_t;
        
        dist max_dist =
            (group_move_intensity > 0) ?
            cursor_max_dist * group_move_intensity :
            leader_to_cursor_dist;
            
        if(max_dist < group_move_arrows[a]) {
            group_move_arrows.erase(group_move_arrows.begin() + a);
        } else {
            a++;
        }
    }
    
    whistle_fade_timer.tick(delta_t);
    
    if(whistling) {
        //Create rings.
        whistle_next_ring_timer.tick(delta_t);
        
        if(pretty_whistle) {
            whistle_next_dot_timer.tick(delta_t);
        }
        
        for(unsigned char d = 0; d < 6; ++d) {
            if(whistle_dot_radius[d] == -1) continue;
            
            whistle_dot_radius[d] += whistle_growth_speed * delta_t;
            if(
                whistle_radius > 0 &&
                whistle_dot_radius[d] > cur_leader_ptr->lea_type->whistle_range
            ) {
                whistle_dot_radius[d] = cur_leader_ptr->lea_type->whistle_range;
                
            } else if(
                whistle_fade_radius > 0 &&
                whistle_dot_radius[d] > whistle_fade_radius
            ) {
                whistle_dot_radius[d] = whistle_fade_radius;
            }
        }
    }
    
    for(size_t r = 0; r < whistle_rings.size(); ) {
        //Erase rings that go beyond the cursor.
        whistle_rings[r] += WHISTLE_RING_SPEED * delta_t;
        if(leader_to_cursor_dist < whistle_rings[r]) {
            whistle_rings.erase(whistle_rings.begin() + r);
            whistle_ring_colors.erase(whistle_ring_colors.begin() + r);
        } else {
            r++;
        }
    }
    
    //Ship beam ring.
    //The way this works is that the three color components are saved.
    //Each frame, we increase them or decrease them
    //(if it reaches 255, set it to decrease, if 0, set it to increase).
    //Each index increases/decreases at a different speed,
    //with red being the slowest and blue the fastest.
    for(unsigned char i = 0; i < 3; ++i) {
        float dir_mult = (ship_beam_ring_color_up[i]) ? 1.0 : -1.0;
        signed char addition =
            dir_mult * SHIP_BEAM_RING_COLOR_SPEED * (i + 1) * delta_t;
        if(ship_beam_ring_color[i] + addition >= 255) {
            ship_beam_ring_color[i] = 255;
            ship_beam_ring_color_up[i] = false;
        } else if(ship_beam_ring_color[i] + addition <= 0) {
            ship_beam_ring_color[i] = 0;
            ship_beam_ring_color_up[i] = true;
        } else {
            ship_beam_ring_color[i] += addition;
        }
    }
    
    //Cursor spin angle and invalidness effect.
    //TODO this goes unused.
    cursor_invalid_effect += CURSOR_INVALID_EFFECT_SPEED * delta_t;
    
    //Cursor trail.
    if(draw_cursor_trail) {
        cursor_save_timer.tick(delta_t);
    }
    
    //Cursor being above or below the leader.
    //TODO check this only one out of every three frames or something.
    cursor_height_diff_light = 0;
    sector* cursor_sector =
        get_sector(leader_cursor_w, NULL, true);
    if(cursor_sector) {
        cursor_height_diff_light =
            (cursor_sector->z - cur_leader_ptr->z) * 0.0033;
        cursor_height_diff_light =
            max(cursor_height_diff_light, -0.33f);
        cursor_height_diff_light =
            min(cursor_height_diff_light, 0.33f);
    }
    
    //Specific animations.
    spark_animation.instance.tick(delta_t);
    
    //Area title fade.
    area_title_fade_timer.tick(delta_t);
    
    //Fade.
    fade_mgr.tick(delta_t);
    
    
}


/* ----------------------------------------------------------------------------
 * Ticks the logic of gameplay-related things.
 */
void gameplay::do_gameplay_logic() {

    game_states[cur_game_state_nr]->update_transformations();
    
    if(cur_message.empty()) {
    
        /************************************
        *                              .-.  *
        *   Timer things - gameplay   ( L ) *
        *                              `-´  *
        *************************************/
        
        day_minutes += (day_minutes_per_irl_sec * delta_t);
        if(day_minutes > 60 * 24) day_minutes -= 60 * 24;
        
        area_time_passed += delta_t;
        
        //Tick all particles.
        particles.tick_all(delta_t);
        
        //Ticks all status effect animations.
        for(auto s = status_types.begin(); s != status_types.end(); ++s) {
            s->second.anim_instance.tick(delta_t);
        }
        
        
        /********************
        *              ***  *
        *   Whistle   * O * *
        *              ***  *
        ********************/
        
        if(
            whistling &&
            whistle_radius < cur_leader_ptr->lea_type->whistle_range
        ) {
            whistle_radius += whistle_growth_speed * delta_t;
            if(whistle_radius > cur_leader_ptr->lea_type->whistle_range) {
                whistle_radius = cur_leader_ptr->lea_type->whistle_range;
            }
        }
        
        
        /*****************
        *                *
        *   Mobs   ()--> *
        *                *
        ******************/
        
        size_t n_mobs = mobs.size();
        for(size_t m = 0; m < n_mobs;) {
        
            //Tick the mob.
            mob* m_ptr = mobs[m];
            m_ptr->tick();
            
            if(m_ptr->fsm.cur_state) {
                process_mob(m_ptr, m);
            }
            
            //Mob deletion.
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
        
        if(cur_leader_ptr->holding_pikmin) {
            cur_leader_ptr->holding_pikmin->pos.x =
                cur_leader_ptr->pos.x +
                cos(cur_leader_ptr->angle + M_PI) *
                cur_leader_ptr->type->radius;
            cur_leader_ptr->holding_pikmin->pos.y =
                cur_leader_ptr->pos.y +
                sin(cur_leader_ptr->angle + M_PI) *
                cur_leader_ptr->type->radius;
            cur_leader_ptr->holding_pikmin->z =
                cur_leader_ptr->z;
            cur_leader_ptr->holding_pikmin->angle =
                cur_leader_ptr->angle;
        }
        
        //Current leader movement.
        float leader_move_intensity = leader_movement.get_intensity();
        if(leader_move_intensity < 0.75) leader_move_intensity = 0;
        if(leader_move_intensity > 1) leader_move_intensity = 1;
        if(leader_move_intensity == 0) {
            cur_leader_ptr->fsm.run_event(
                LEADER_EVENT_MOVE_END, (void*) &leader_movement
            );
        } else {
            cur_leader_ptr->fsm.run_event(
                LEADER_EVENT_MOVE_START, (void*) &leader_movement
            );
        }
        
        cam_final_pos = cur_leader_ptr->pos;
        
        
        /***********************************
        *                             ***  *
        *   Current leader's group   ****O *
        *                             ***  *
        ************************************/
        
        size_t n_members = cur_leader_ptr->group->members.size();
        closest_group_member = cur_leader_ptr->holding_pikmin;
        closest_group_member_distant = false;
        
        if(n_members > 0 && !closest_group_member) {
        
            update_closest_group_member();
        }
        
        point group_move_coords = group_movement.get_coords();
        
        if(group_move_cursor) {
            group_move_angle = cursor_angle;
            float leader_to_cursor_dist =
                dist(cur_leader_ptr->pos, leader_cursor_w).to_float();
            group_move_intensity =
                leader_to_cursor_dist / cursor_max_dist;
        } else if(group_move_coords.x != 0 || group_move_coords.y != 0) {
            coordinates_to_angle(
                group_move_coords,
                &group_move_angle, &group_move_intensity
            );
            if(group_move_intensity > 1) group_move_intensity = 1;
        } else {
            group_move_intensity = 0;
        }
        
        
        /********************
        *             .-.   *
        *   Cursor   ( = )> *
        *             `-´   *
        ********************/
        
        point mouse_cursor_speed =
            cursor_movement.get_coords() * delta_t * MOUSE_CURSOR_MOVE_SPEED;
            
        mouse_cursor_s += mouse_cursor_speed;
        
        mouse_cursor_w = mouse_cursor_s;
        al_transform_coordinates(
            &screen_to_world_transform,
            &mouse_cursor_w.x, &mouse_cursor_w.y
        );
        leader_cursor_w = mouse_cursor_w;
        
        cursor_angle =
            get_angle(cur_leader_ptr->pos, leader_cursor_w);
        if(cur_leader_ptr->fsm.cur_state->id == LEADER_STATE_ACTIVE) {
            //TODO move this to the FSM.
            cur_leader_ptr->face(cursor_angle);
        }
        
        dist leader_to_cursor_dist(cur_leader_ptr->pos, leader_cursor_w);
        if(leader_to_cursor_dist > cursor_max_dist) {
            //TODO with an analog stick, if the cursor is being moved,
            //it's considered off-limit a lot more than it should.
            
            //Cursor goes beyond the range limit.
            leader_cursor_w.x =
                cur_leader_ptr->pos.x + (cos(cursor_angle) * cursor_max_dist);
            leader_cursor_w.y =
                cur_leader_ptr->pos.y + (sin(cursor_angle) * cursor_max_dist);
                
            if(mouse_cursor_speed.x != 0 || mouse_cursor_speed.y != 0) {
                //If we're speeding the mouse cursor (via analog stick),
                //don't let it go beyond the edges.
                mouse_cursor_w = leader_cursor_w;
                mouse_cursor_s = mouse_cursor_w;
                al_transform_coordinates(
                    &world_to_screen_transform,
                    &mouse_cursor_s.x, &mouse_cursor_s.y
                );
            }
        }
        
        leader_cursor_s = leader_cursor_w;
        al_transform_coordinates(
            &world_to_screen_transform,
            &leader_cursor_s.x, &leader_cursor_s.y
        );
        
        
        
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
        for(auto l = liquids.begin(); l != liquids.end(); ++l) {
            l->second.anim_instance.tick(delta_t);
        }
        
    } else { //Displaying a message.
    
        if(
            cur_message_char <
            cur_message_stopping_chars[cur_message_section + 1]
        ) {
            if(cur_message_char_timer.duration == 0.0f) {
                size_t stopping_char =
                    cur_message_stopping_chars[cur_message_section + 1];
                //Display everything right away.
                cur_message_char = stopping_char;
            } else {
                cur_message_char_timer.tick(delta_t);
            }
        }
        
    }
    
    //Print framerate.
    if(show_framerate) {
        framerate_update_timer.tick(delta_t);
        print_info(
            "Average:  " + i2s(framerate_counter) + " FPS\n\n"
            "Now:      " + f2s(1.0 / delta_t) + " FPS\n"
            "Intended: " + i2s(game_fps) + " FPS"
        );
    }
    
    //Print info on a mob.
    if(dev_tool_info_lock) {
        string name_str =
            box_string("Mob: " + dev_tool_info_lock->type->name + ".", 30);
        string coords_str =
            box_string(
                "Coords: " +
                box_string(f2s(dev_tool_info_lock->pos.x), 6) + " " +
                box_string(f2s(dev_tool_info_lock->pos.y), 6) + " " +
                box_string(f2s(dev_tool_info_lock->z), 6) + ".",
                30
            );
        string stateh_str =
            "State hist.: " +
            (
                dev_tool_info_lock->fsm.cur_state ?
                dev_tool_info_lock->fsm.cur_state->name :
                "(None!)"
            );
        for(unsigned char p = 0; p < N_PREV_STATES; ++p) {
            stateh_str +=
                ", " + dev_tool_info_lock->fsm.prev_state_names[p];
        }
        stateh_str += ".";
        string anim_str =
            box_string(
                "Animation: " +
                (dev_tool_info_lock->anim.cur_anim ?
                 dev_tool_info_lock->anim.cur_anim->name :
                 "(None!)") +
                ".",
                60
            );
        string health_str =
            box_string("Health: " + f2s(dev_tool_info_lock->health) + ".", 30);
        string timer_str =
            box_string(
                "Timer: " +
                f2s(dev_tool_info_lock->script_timer.time_left) + ".",
                30
            );
            
        string vars_str = "Vars: ";
        if(!dev_tool_info_lock->vars.empty()) {
            for(
                auto v = dev_tool_info_lock->vars.begin();
                v != dev_tool_info_lock->vars.end(); ++v
            ) {
                vars_str += v->first + "=" + v->second + "; ";
            }
            vars_str.erase(vars_str.size() - 2, 2);
            vars_str += ".";
        } else {
            vars_str += "(None).";
        }
        
        print_info(
            name_str + coords_str + "\n" +
            stateh_str + "\n" +
            health_str + timer_str + "\n" +
            anim_str + "\n" +
            vars_str
        );
    }
    
    //Print mouse coordinates.
    if(dev_tool_show_mouse_coords) {
        print_info(
            "Mouse coordinates: " + f2s(mouse_cursor_w.x) +
            ", " + f2s(mouse_cursor_w.y) + "."
        );
    }
    
    info_print_timer.tick(delta_t);
    
    ready_for_input = true;
    
}


/* ----------------------------------------------------------------------------
 * Handles the logic required to tick a specific mob and its interactions
 * with other mobs.
 */
void gameplay::process_mob(mob* m_ptr, size_t m) {
    /********************************
     *                              *
     *   Mob interactions   () - () *
     *                              *
     ********************************/
    //Interactions between this mob and the others.
    size_t n_mobs = mobs.size();
    for(size_t m2 = 0; m2 < n_mobs; ++m2) {
        if(m == m2) continue;
        
        mob* m2_ptr = mobs[m2];
        dist d(m_ptr->pos, m2_ptr->pos);
        
        //Check if mob 1 should be pushed by mob 2.
        if(
            m2_ptr->type->pushes &&
            m2_ptr->tangible &&
            m_ptr->type->pushable && !m_ptr->unpushable &&
            m2_ptr->z < m_ptr->z + m_ptr->type->height &&
            m2_ptr->z + m2_ptr->type->height > m_ptr->z &&
            d <= m_ptr->type->radius + m2_ptr->type->radius
        ) {
            float d_amount =
                fabs(
                    d.to_float() - m_ptr->type->radius -
                    m2_ptr->type->radius
                );
            //If the mob is inside the other,
            //it needs to be pushed out.
            if(d_amount > m_ptr->push_amount) {
                m_ptr->push_amount = d_amount / delta_t;
                m_ptr->push_angle = get_angle(m2_ptr->pos, m_ptr->pos);
            }
        }
        
        if(!m2_ptr->dead) {
            //Check "see"s.
            mob_event* see_op_ev =
                q_get_event(
                    m_ptr, MOB_EVENT_SEEN_OPPONENT
                );
            mob_event* see_ob_ev =
                q_get_event(
                    m_ptr, MOB_EVENT_SEEN_OBJECT
                );
            if(see_op_ev || see_ob_ev) {
            
                if(d <= m_ptr->type->sight_radius) {
                    if(see_ob_ev) {
                        see_ob_ev->run(m_ptr, (void*) m2_ptr);
                    }
                    if(see_op_ev && should_attack(m_ptr, m2_ptr)) {
                        see_op_ev->run(m_ptr, (void*) m2_ptr);
                    }
                }
                
            }
            
            //Check "near"s.
            mob_event* near_op_ev =
                q_get_event(m_ptr, MOB_EVENT_NEAR_OPPONENT);
            mob_event* near_ob_ev =
                q_get_event(m_ptr, MOB_EVENT_NEAR_OBJECT);
            if(near_op_ev || near_ob_ev) {
            
                if(
                    d <=
                    (
                        m_ptr->type->radius +
                        m2_ptr->type->radius +
                        m_ptr->type->near_radius
                    )
                ) {
                    if(near_ob_ev) {
                        near_ob_ev->run(m_ptr, (void*) m2_ptr);
                    }
                    if(
                        near_op_ev &&
                        should_attack(m_ptr, m2_ptr) &&
                        !m2_ptr->dead
                    ) {
                        near_op_ev->run(m_ptr, (void*) m2_ptr);
                    }
                }
                
            }
            
            //Check if it's facing.
            mob_event* facing_op_ev =
                q_get_event(m_ptr, MOB_EVENT_FACING_OPPONENT);
            mob_event* facing_ob_ev =
                q_get_event(m_ptr, MOB_EVENT_FACING_OBJECT);
            if(facing_op_ev || facing_ob_ev) {
            
                float angle_dif =
                    get_angle_smallest_dif(
                        m_ptr->angle,
                        get_angle(m_ptr->pos, m2_ptr->pos)
                    );
                if(
                    d <=
                    (
                        m_ptr->type->radius +
                        m2_ptr->type->radius +
                        m_ptr->type->near_radius
                    ) &&
                    angle_dif <= (m_ptr->type->near_angle / 2.0)
                ) {
                
                    if(facing_ob_ev) {
                        facing_ob_ev->run(m_ptr, (void*) m2_ptr);
                    }
                    if(
                        facing_op_ev &&
                        should_attack(m_ptr, m2_ptr)
                    ) {
                        facing_op_ev->run(m_ptr, (void*) m2_ptr);
                    }
                }
                
            }
        }
        
        //Check touches. This does not use hitboxes,
        //only the object radii.
        mob_event* touch_op_ev =
            q_get_event(m_ptr, MOB_EVENT_TOUCHED_OPPONENT);
        mob_event* touch_le_ev =
            q_get_event(m_ptr, MOB_EVENT_TOUCHED_LEADER);
        mob_event* touch_ob_ev =
            q_get_event(m_ptr, MOB_EVENT_TOUCHED_OBJECT);
        mob_event* pik_land_ev =
            q_get_event(m_ptr, MOB_EVENT_PIKMIN_LANDED);
        if(
            touch_op_ev || touch_le_ev ||
            touch_ob_ev || pik_land_ev
        ) {
        
            bool z_touch;
            if(
                m_ptr->type->height == 0 ||
                m2_ptr->type->height == 0
            ) {
                z_touch = true;
            } else {
                z_touch =
                    !(
                        (
                            m2_ptr->z >
                            m_ptr->z + m_ptr->type->height
                        ) || (
                            m2_ptr->z + m2_ptr->type->height <
                            m2_ptr->z
                        )
                    );
            }
            
            if(
                z_touch &&
                m2_ptr->tangible &&
                d <= (m_ptr->type->radius + m2_ptr->type->radius)
            ) {
                if(touch_ob_ev) {
                    touch_ob_ev->run(m_ptr, (void*) m2_ptr);
                }
                if(touch_op_ev && should_attack(m_ptr, m2_ptr)) {
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
            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_A_N);
        mob_event* hitbox_touch_na_ev =
            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_N_A);
        mob_event* hitbox_touch_eat_ev =
            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_EAT);
        mob_event* hitbox_touch_haz_ev =
            q_get_event(m_ptr, MOB_EVENT_TOUCHED_HAZARD);
        if(hitbox_touch_an_ev || hitbox_touch_na_ev || hitbox_touch_eat_ev) {
        
            sprite* s1_ptr = m_ptr->anim.get_cur_sprite();
            sprite* s2_ptr = m2_ptr->anim.get_cur_sprite();
            
            bool m1_is_hitbox = false;
            vector<hazard*> m1_resistances;
            
            if(m_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
                m1_is_hitbox = true;
                m1_resistances = ((pikmin*) m_ptr)->pik_type->resistances;
            } else if(m_ptr->type->category->id == MOB_CATEGORY_LEADERS) {
                m1_is_hitbox = true;
            }
            
            bool m1_has_hitboxes =
                s1_ptr && (!s1_ptr->hitboxes.empty() || m1_is_hitbox);
            bool m2_has_hitboxes =
                s2_ptr && !s2_ptr->hitboxes.empty();
                
            //If they're so far away the hitboxes can't touch, just skip
            //the check. Also, if neither have hitboxes up, never mind.
            if(
                m1_has_hitboxes && m2_has_hitboxes &&
                d < s1_ptr->hitbox_span + s2_ptr->hitbox_span
            ) {
            
                bool reported_an_ev = false;
                bool reported_na_ev = false;
                bool reported_eat_ev = false;
                bool reported_haz_ev = false;
                
                float m1_angle_sin = 0;
                float m1_angle_cos = 0;
                if(!m1_is_hitbox) {
                    m1_angle_sin = sin(m_ptr->angle);
                    m1_angle_cos = cos(m_ptr->angle);
                }
                float m2_angle_sin = sin(m2_ptr->angle);
                float m2_angle_cos = cos(m2_ptr->angle);
                
                //For all of mob 2's hitboxes, check for collisions.
                for(
                    size_t h2 = 0;
                    h2 < s2_ptr->hitboxes.size(); ++h2
                ) {
                    hitbox* h2_ptr = &s2_ptr->hitboxes[h2];
                    if(h2_ptr->type == HITBOX_TYPE_DISABLED) continue;
                    
                    //Hazard resistance check.
                    if(
                        !h2_ptr->hazards.empty() &&
                        is_resistant_to_hazards(
                            m1_resistances, h2_ptr->hazards
                        )
                    ) {
                        continue;
                    }
                    
                    //Get mob 2's real hitbox location.
                    point m2_h_pos(
                        m2_ptr->pos.x + (
                            h2_ptr->pos.x * m2_angle_cos -
                            h2_ptr->pos.y * m2_angle_sin
                        ),
                        m2_ptr->pos.y + (
                            h2_ptr->pos.x * m2_angle_sin +
                            h2_ptr->pos.y * m2_angle_cos
                        )
                    );
                    float m2_h_z = m2_ptr->z + h2_ptr->z;
                    
                    if(m1_is_hitbox) {
                        //Just check if the entire Pikmin/leader
                        //touched mob 2's hitbox.
                        
                        bool z_collision;
                        if(h2_ptr->height == 0) {
                            //Always hits vertically.
                            //Imagine the hitbox is infinitely high.
                            z_collision = true;
                        } else {
                            z_collision =
                                !(
                                    (m2_h_z > m_ptr->z) ||
                                    (m2_h_z + h2_ptr->height < m_ptr->z)
                                );
                        }
                        
                        if(
                            z_collision &&
                            dist(m_ptr->pos, m2_h_pos) <
                            (m_ptr->type->radius + h2_ptr->radius)
                        ) {
                            //Collision!
                            if(
                                hitbox_touch_eat_ev && !reported_eat_ev &&
                                m2_ptr->chomping_pikmin.size() <
                                m2_ptr->chomp_max &&
                                find(
                                    m2_ptr->chomp_hitboxes.begin(),
                                    m2_ptr->chomp_hitboxes.end(),
                                    h2_ptr->body_part_index
                                ) !=
                                m2_ptr->chomp_hitboxes.end()
                            ) {
                                hitbox_touch_eat_ev->run(
                                    m_ptr, (void*) m2_ptr, (void*) h2_ptr
                                );
                                m2_ptr->chomping_pikmin.push_back(m_ptr);
                                reported_eat_ev = true;
                            }
                            
                            if(
                                !reported_haz_ev &&
                                h2_ptr->type == HITBOX_TYPE_ATTACK &&
                                hitbox_touch_haz_ev &&
                                !h2_ptr->hazards.empty()
                            ) {
                                for(
                                    size_t h = 0;
                                    h < h2_ptr->hazards.size(); ++h
                                ) {
                                    hitbox_touch_haz_ev->run(
                                        m_ptr,
                                        (void*) h2_ptr->hazards[h]
                                    );
                                }
                                reported_haz_ev = true;
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
                            
                            if(
                                h2_ptr->type == HITBOX_TYPE_ATTACK &&
                                hitbox_touch_na_ev && !reported_na_ev &&
                                //Check if the events have been sent
                                //already. This way, Pikmin aren't
                                //knocked back AND eaten.
                                !reported_eat_ev &&
                                !reported_haz_ev &&
                                !disable_attack_status
                            ) {
                                hitbox_touch_info ev_info =
                                    hitbox_touch_info(m2_ptr, NULL, h2_ptr);
                                hitbox_touch_na_ev->run(
                                    m_ptr, (void*) &ev_info
                                );
                                reported_na_ev = true;
                                
                            } else if(
                                h2_ptr->type == HITBOX_TYPE_NORMAL &&
                                hitbox_touch_an_ev && !reported_an_ev &&
                                !reported_haz_ev
                            ) {
                                hitbox_touch_info ev_info =
                                    hitbox_touch_info(m2_ptr, NULL, h2_ptr);
                                hitbox_touch_an_ev->run(
                                    m_ptr, (void*) &ev_info
                                );
                                reported_an_ev = true;
                            }
                        }
                        
                    } else {
                        //Check if any hitbox touched mob 2's hitbox.
                        
                        for(
                            size_t h1 = 0;
                            h1 < s1_ptr->hitboxes.size(); ++h1
                        ) {
                        
                            hitbox* h1_ptr =
                                &s1_ptr->hitboxes[h1];
                            if(h1_ptr->type == HITBOX_TYPE_DISABLED) {
                                continue;
                            }
                            
                            //Get mob 1's real hitbox location.
                            point m1_h_pos(
                                m_ptr->pos.x + (
                                    h1_ptr->pos.x * m1_angle_cos -
                                    h1_ptr->pos.y * m1_angle_sin
                                ),
                                m_ptr->pos.y + (
                                    h1_ptr->pos.x * m1_angle_sin +
                                    h1_ptr->pos.y * m1_angle_cos
                                )
                            );
                            float m1_h_z =
                                m_ptr->z + h1_ptr->z;
                                
                            bool z_collision;
                            if(h1_ptr->height == 0 || h2_ptr->height == 0) {
                                z_collision = true;
                            } else {
                                z_collision =
                                    !(
                                        (
                                            m2_h_z >
                                            m1_h_z + h1_ptr->height
                                        ) || (
                                            m2_h_z + h2_ptr->height <
                                            m1_h_z
                                        )
                                    );
                            }
                            
                            if(
                                z_collision &&
                                dist(m1_h_pos, m2_h_pos) <
                                (h1_ptr->radius + h2_ptr->radius)
                            ) {
                                //Collision!
                                if(
                                    hitbox_touch_eat_ev &&
                                    !reported_eat_ev &&
                                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                                    m2_ptr->chomping_pikmin.size() <
                                    m2_ptr->chomp_max &&
                                    find(
                                        m2_ptr->chomp_hitboxes.begin(),
                                        m2_ptr->chomp_hitboxes.end(),
                                        h2_ptr->body_part_index
                                    ) !=
                                    m2_ptr->chomp_hitboxes.end()
                                ) {
                                    hitbox_touch_eat_ev->run(
                                        m_ptr,
                                        (void*) m2_ptr,
                                        (void*) h2_ptr
                                    );
                                    m2_ptr->chomping_pikmin.push_back(
                                        m_ptr
                                    );
                                    reported_eat_ev = true;
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
                                
                                if(
                                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                                    h2_ptr->type == HITBOX_TYPE_ATTACK &&
                                    hitbox_touch_na_ev && !reported_na_ev &&
                                    !disable_attack_status
                                ) {
                                    hitbox_touch_info ev_info =
                                        hitbox_touch_info(
                                            m2_ptr, h1_ptr, h2_ptr
                                        );
                                    hitbox_touch_na_ev->run(
                                        m_ptr, (void*) &ev_info
                                    );
                                    reported_na_ev = true;
                                    
                                } else if(
                                    h1_ptr->type == HITBOX_TYPE_ATTACK &&
                                    h2_ptr->type == HITBOX_TYPE_NORMAL &&
                                    hitbox_touch_an_ev && !reported_an_ev &&
                                    !reported_haz_ev
                                ) {
                                    hitbox_touch_info ev_info =
                                        hitbox_touch_info(
                                            m2_ptr, h1_ptr, h2_ptr
                                        );
                                    hitbox_touch_an_ev->run(
                                        m_ptr, (void*) &ev_info
                                    );
                                    reported_an_ev = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        //Find a carriable mob to grab.
        mob_event* near_carriable_object_ev =
            q_get_event(m_ptr, MOB_EVENT_NEAR_CARRIABLE_OBJECT);
        if(near_carriable_object_ev) {
        
            if(
                m2_ptr->carry_info &&
                !m2_ptr->carry_info->is_full() &&
                d <=
                m_ptr->type->radius + m2_ptr->type->radius + task_range(m_ptr)
            ) {
            
                near_carriable_object_ev->run(m_ptr, (void*) m2_ptr);
                
            }
            
        }
    }
    
    //Check if it got whistled.
    mob_event* whistled_ev = q_get_event(m_ptr, MOB_EVENT_WHISTLED);
    if(whistling && whistled_ev) {
        if(
            dist(m_ptr->pos, leader_cursor_w) <=
            whistle_radius
        ) {
            whistled_ev->run(m_ptr);
        }
    }
    
    //Following a leader.
    if(m_ptr->following_group) {
        mob_event* spot_near_ev = q_get_event(m_ptr, MOB_EVENT_SPOT_IS_NEAR);
        mob_event* spot_far_ev =  q_get_event(m_ptr, MOB_EVENT_SPOT_IS_FAR);
        
        if(spot_near_ev || spot_far_ev) {
            point final_pos =
                m_ptr->following_group->group->anchor +
                m_ptr->following_group->group->get_spot_offset(
                    m_ptr->group_spot_index
                );
            dist d(m_ptr->pos, final_pos);
            if(spot_far_ev && d >= 5) {
                spot_far_ev->run(m_ptr, (void*) &final_pos);
            } else if(spot_near_ev && d < 5) {
                spot_near_ev->run(m_ptr);
            }
        }
    }
    
    //Focused on a mob.
    if(m_ptr->focused_mob) {
    
        dist d(m_ptr->pos, m_ptr->focused_mob->pos);
        if(m_ptr->focused_mob->dead) {
            m_ptr->fsm.run_event(MOB_EVENT_FOCUSED_MOB_DIED);
        }
        
        //We have to recheck if the focused mob is not NULL, because
        //sending MOB_EVENT_FOCUSED_MOB_DIED could've set this to NULL.
        if(m_ptr->focused_mob) {
            if(d > (m_ptr->type->sight_radius * 1.1f)) {
                m_ptr->fsm.run_event(MOB_EVENT_LOST_FOCUSED_MOB);
            }
        }
        
        if(m_ptr->focused_mob) {
            if(!m_ptr->focused_mob->carry_info) {
                m_ptr->fsm.run_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE);
            }
        }
        
    }
    
    //Far away from home.
    mob_event* far_from_home_ev = q_get_event(m_ptr, MOB_EVENT_FAR_FROM_HOME);
    if(far_from_home_ev) {
        dist d(m_ptr->pos, m_ptr->home);
        if(d >= m_ptr->type->territory_radius) {
            far_from_home_ev->run(m_ptr);
        }
    }
    
    //Check its mouth.
    if(m_ptr->chomping_pikmin.empty()) {
        m_ptr->fsm.run_event(MOB_EVENT_MOUTH_EMPTY);
    } else {
        m_ptr->fsm.run_event(MOB_EVENT_MOUTH_OCCUPIED);
    }
    
    //Being carried, but has an obstacle.
    if(m_ptr->carry_info) {
        if(m_ptr->carry_info->obstacle_ptr) {
            if(m_ptr->carry_info->obstacle_ptr->health == 0) {
                m_ptr->fsm.run_event(MOB_EVENT_CARRY_BEGIN_MOVE);
            }
        }
    }
    
    //Tick event.
    m_ptr->fsm.run_event(MOB_EVENT_ON_TICK);
}
