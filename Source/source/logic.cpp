/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
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
#include "logic.h"
#include "mobs/pikmin.h"
#include "vars.h"

const float CAMERA_SMOOTHNESS_MULT = 4.5f;

/* ----------------------------------------------------------------------------
 * Ticks the logic of aesthetic things. If the game is paused, these can
 * be frozen in place without any negative impact.
 */
void do_aesthetic_logic() {

    /*************************************
    *                               .-.  *
    *   Timer things - aesthetic   ( L ) *
    *                               `-´  *
    **************************************/
    
    //Camera movement.
    cam_x += (cam_final_x - cam_x) * (CAMERA_SMOOTHNESS_MULT * delta_t);
    cam_y += (cam_final_y - cam_y) * (CAMERA_SMOOTHNESS_MULT * delta_t);
    cam_zoom +=
        (cam_final_zoom - cam_zoom) * (CAMERA_SMOOTHNESS_MULT * delta_t);
        
    //"Move group" arrows.
    if(group_move_intensity) {
        group_move_next_arrow_timer.tick(delta_t);
    }
    
    dist leader_to_cursor_dis(
        cur_leader_ptr->x, cur_leader_ptr->y, cursor_x, cursor_y
    );
    for(size_t a = 0; a < group_move_arrows.size(); ) {
        group_move_arrows[a] += GROUP_MOVE_ARROW_SPEED * delta_t;
        
        dist max_dist =
            (group_move_intensity > 0) ?
            cursor_max_dist * group_move_intensity :
            leader_to_cursor_dis;
            
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
        if(leader_to_cursor_dis < whistle_rings[r]) {
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
    sector* cursor_sector = get_sector(cursor_x, cursor_y, NULL, true);
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
void do_gameplay_logic() {

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
            cur_leader_ptr->holding_pikmin->x =
                cur_leader_ptr->x +
                cos(cur_leader_ptr->angle + M_PI) *
                cur_leader_ptr->type->radius;
            cur_leader_ptr->holding_pikmin->y =
                cur_leader_ptr->y +
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
        
        cam_final_x = cur_leader_ptr->x;
        cam_final_y = cur_leader_ptr->y;
        
        
        /***********************************
        *                             ***  *
        *   Current leader's group   ****O *
        *                             ***  *
        ************************************/
        
        dist closest_distance = 0;
        size_t n_members = cur_leader_ptr->group->members.size();
        closest_group_member = cur_leader_ptr->holding_pikmin;
        
        if(n_members > 0 && !closest_group_member) {
        
            for(size_t m = 0; m < n_members; ++m) {
                dist d(
                    cur_leader_ptr->x, cur_leader_ptr->y,
                    cur_leader_ptr->group->members[m]->x,
                    cur_leader_ptr->group->members[m]->y
                );
                if(m == 0 || d < closest_distance) {
                    closest_distance = d;
                    closest_group_member = cur_leader_ptr->group->members[m];
                }
            }
            
            if(closest_distance > pikmin_grab_range) {
                closest_group_member = NULL;
            }
        }
        
        float group_move_x = group_movement.get_x();
        float group_move_y = group_movement.get_y();
        
        if(group_move_cursor) {
            group_move_angle = cursor_angle;
            dist leader_to_cursor_dis(
                cur_leader_ptr->x, cur_leader_ptr->y, cursor_x, cursor_y
            );
            group_move_intensity =
                leader_to_cursor_dis.to_float() / cursor_max_dist;
        } else if(group_move_x != 0 || group_move_y != 0) {
            coordinates_to_angle(
                group_move_x, group_move_y,
                &group_move_angle, &group_move_intensity
            );
            if(group_move_intensity > 1) group_move_intensity = 1;
        } else {
            group_move_intensity = 0;
        }
        
        if(group_move_intensity) {
            cur_leader_ptr->group->group_center_x =
                cur_leader_ptr->x + cos(group_move_angle) *
                group_move_intensity * cursor_max_dist;
            cur_leader_ptr->group->group_center_y =
                cur_leader_ptr->y + sin(group_move_angle) *
                group_move_intensity * cursor_max_dist;
        } else if(prev_group_move_intensity != 0) {
            float d = get_leader_to_group_center_dist(cur_leader_ptr);
            cur_leader_ptr->group->group_center_x =
                cur_leader_ptr->x + cos(group_move_angle) * d;
            cur_leader_ptr->group->group_center_y =
                cur_leader_ptr->y + sin(group_move_angle) * d;
        }
        prev_group_move_intensity = group_move_intensity;
        
        
        /********************
        *             .-.   *
        *   Cursor   ( = )> *
        *             `-´   *
        ********************/
        
        float mouse_cursor_speed_x =
            delta_t * MOUSE_CURSOR_MOVE_SPEED * cursor_movement.get_x();
        float mouse_cursor_speed_y =
            delta_t * MOUSE_CURSOR_MOVE_SPEED * cursor_movement.get_y();
            
        mouse_cursor_x += mouse_cursor_speed_x;
        mouse_cursor_y += mouse_cursor_speed_y;
        
        float mcx = mouse_cursor_x, mcy = mouse_cursor_y;
        ALLEGRO_TRANSFORM world_to_screen_transform =
            get_world_to_screen_transform();
        ALLEGRO_TRANSFORM screen_to_world_transform =
            world_to_screen_transform;
        al_invert_transform(&screen_to_world_transform);
        al_transform_coordinates(&screen_to_world_transform, &mcx, &mcy);
        cursor_x = mcx;
        cursor_y = mcy;
        
        cursor_angle =
            atan2(cursor_y - cur_leader_ptr->y, cursor_x - cur_leader_ptr->x);
        if(cur_leader_ptr->fsm.cur_state->id == LEADER_STATE_ACTIVE) {
            cur_leader_ptr->face(cursor_angle);
        }
        
        dist leader_to_cursor_dis =
            dist(cur_leader_ptr->x, cur_leader_ptr->y, cursor_x, cursor_y);
        if(leader_to_cursor_dis > cursor_max_dist) {
            //TODO with an analog stick, if the cursor is being moved,
            //it's considered off-limit a lot more than it should.
            
            //Cursor goes beyond the range limit.
            cursor_x =
                cur_leader_ptr->x + (cos(cursor_angle) * cursor_max_dist);
            cursor_y =
                cur_leader_ptr->y + (sin(cursor_angle) * cursor_max_dist);
                
            if(mouse_cursor_speed_x != 0 || mouse_cursor_speed_y != 0) {
                //If we're speeding the mouse cursor (via analog stick),
                //don't let it go beyond the edges.
                mouse_cursor_x = cursor_x;
                mouse_cursor_y = cursor_y;
                al_transform_coordinates(
                    &world_to_screen_transform,
                    &mouse_cursor_x, &mouse_cursor_y
                );
            }
        }
        
        
        /**************************
        *                    /  / *
        *   Percipitation     / / *
        *                   /  /  *
        **************************/
        
        /*
        if(
            cur_area_data.weather_condition.percipitation_type !=
            PERCIPITATION_TYPE_NONE
        ) {
            percipitation_timer.tick(delta_t);
            if(percipitation_timer.ticked) {
                percipitation_timer = timer(
                    cur_area_data.weather_condition.
                    percipitation_frequency.get_random_number()
                );
                percipitation_timer.start();
                percipitation.push_back(point(0, 0));
            }
        
            for(size_t p = 0; p < percipitation.size();) {
                percipitation[p].y +=
                    cur_area_data.weather_condition.
                    percipitation_speed.get_random_number() * delta_t;
                if(percipitation[p].y > scr_h) {
                    percipitation.erase(percipitation.begin() + p);
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
                box_string(f2s(dev_tool_info_lock->x), 6) + " " +
                box_string(f2s(dev_tool_info_lock->y), 6) + " " +
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
                (dev_tool_info_lock->anim.anim ?
                 dev_tool_info_lock->anim.anim->name :
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
        float mx, my;
        get_mouse_cursor_coordinates(&mx, &my);
        print_info("Mouse coordinates: " + f2s(mx) + ", " + f2s(my) + ".");
    }
    
    info_print_timer.tick(delta_t);
    
    ready_for_input = true;
    
}


/* ----------------------------------------------------------------------------
 * Handles the logic required to tick a specific mob and its interactions
 * with other mobs.
 */
void process_mob(mob* m_ptr, size_t m) {
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
        dist d(m_ptr->x, m_ptr->y, m2_ptr->x, m2_ptr->y);
        
        //Check if mob 1 should be pushed by mob 2.
        if(
            m2_ptr->type->pushes &&
            m2_ptr->tangible &&
            m_ptr->type->pushable &&
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
                m_ptr->push_angle =
                    atan2(
                        m_ptr->y - m2_ptr->y,
                        m_ptr->x - m2_ptr->x
                    );
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
                        atan2(
                            m2_ptr->y - m_ptr->y,
                            m2_ptr->x - m_ptr->x
                        )
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
                    typeid(*m2_ptr) == typeid(pikmin)
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
        
            frame* f1_ptr = m_ptr->anim.get_frame();
            frame* f2_ptr = m2_ptr->anim.get_frame();
            
            bool m1_is_hitbox = false;
            vector<hazard*> m1_resistances;
            
            if(typeid(*m_ptr) == typeid(pikmin)) {
                m1_is_hitbox = true;
                m1_resistances = ((pikmin*) m_ptr)->pik_type->resistances;
            } else if(typeid(*m_ptr) == typeid(leader)) {
                m1_is_hitbox = true;
            }
            
            bool m1_has_hitboxes =
                f1_ptr && (!f1_ptr->hitbox_instances.empty() || m1_is_hitbox);
            bool m2_has_hitboxes =
                f2_ptr && !f2_ptr->hitbox_instances.empty();
                
            //If they're so far away the hitboxes can't touch, just skip
            //the check. Also, if neither have hitboxes up, never mind.
            if(
                m1_has_hitboxes && m2_has_hitboxes &&
                d < f1_ptr->hitbox_span + f2_ptr->hitbox_span
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
                    h2 < f2_ptr->hitbox_instances.size(); ++h2
                ) {
                    hitbox_instance* h2_ptr = &f2_ptr->hitbox_instances[h2];
                    if(h2_ptr->type == HITBOX_TYPE_DISABLED) continue;
                    
                    //Hazard resistance check.
                    if(!h2_ptr->hazards.empty()) {
                        size_t n_resistances = 0;
                        for(size_t h = 0; h < h2_ptr->hazards.size(); ++h) {
                            for(
                                size_t r = 0;
                                r < m1_resistances.size(); ++r
                            ) {
                                if(
                                    h2_ptr->hazards[h] ==
                                    m1_resistances[r]
                                ) {
                                    n_resistances++;
                                    break;
                                }
                            }
                        }
                        if(n_resistances == h2_ptr->hazards.size()) {
                            //The mob can resist all
                            //of this hitbox's hazards!
                            continue;
                        }
                    }
                    
                    //Get mob 2's real hitbox location.
                    float m2_h_x =
                        m2_ptr->x + (
                            h2_ptr->x * m2_angle_cos -
                            h2_ptr->y * m2_angle_sin
                        );
                    float m2_h_y =
                        m2_ptr->y + (
                            h2_ptr->x * m2_angle_sin +
                            h2_ptr->y * m2_angle_cos
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
                            dist(m_ptr->x, m_ptr->y, m2_h_x, m2_h_y) <
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
                                    h2_ptr->hitbox_nr
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
                            
                            if(
                                h2_ptr->type == HITBOX_TYPE_ATTACK &&
                                hitbox_touch_na_ev && !reported_na_ev &&
                                //Check if the events have been sent
                                //already. This way, Pikmin aren't
                                //knocked back AND eaten.
                                !reported_eat_ev &&
                                !reported_haz_ev
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
                            h1 < f1_ptr->hitbox_instances.size(); ++h1
                        ) {
                        
                            hitbox_instance* h1_ptr =
                                &f1_ptr->hitbox_instances[h1];
                            if(h1_ptr->type == HITBOX_TYPE_DISABLED) {
                                continue;
                            }
                            
                            //Get mob 1's real hitbox location.
                            float m1_h_x =
                                m_ptr->x + (
                                    h1_ptr->x * m1_angle_cos -
                                    h1_ptr->y * m1_angle_sin
                                );
                            float m1_h_y =
                                m_ptr->y + (
                                    h1_ptr->x * m1_angle_sin +
                                    h1_ptr->y * m1_angle_cos
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
                                dist(m1_h_x, m1_h_y, m2_h_x, m2_h_y) <
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
                                        h2_ptr->hitbox_nr
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
                                
                                if(
                                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                                    h2_ptr->type == HITBOX_TYPE_ATTACK &&
                                    hitbox_touch_na_ev && !reported_na_ev
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
        if(dist(m_ptr->x, m_ptr->y, cursor_x, cursor_y) <= whistle_radius) {
            whistled_ev->run(m_ptr);
        }
    }
    
    //Following a leader.
    if(m_ptr->following_group) {
        mob_event* spot_near_ev = q_get_event(m_ptr, MOB_EVENT_SPOT_IS_NEAR);
        mob_event* spot_far_ev =  q_get_event(m_ptr, MOB_EVENT_SPOT_IS_FAR);
        
        if(spot_near_ev || spot_far_ev) {
            dist d(
                m_ptr->x, m_ptr->y,
                m_ptr->following_group->group->group_center_x +
                m_ptr->group_spot_x,
                m_ptr->following_group->group->group_center_y +
                m_ptr->group_spot_y
            );
            if(spot_far_ev && d >= 5) {
                spot_far_ev->run(m_ptr);
            } else if(spot_near_ev && d < 5) {
                spot_near_ev->run(m_ptr);
            }
        }
    }
    
    //Focused on a mob.
    if(m_ptr->focused_mob) {
    
        dist d(
            m_ptr->x, m_ptr->y, m_ptr->focused_mob->x, m_ptr->focused_mob->y
        );
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
        dist d(m_ptr->x, m_ptr->y, m_ptr->home_x, m_ptr->home_y);
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
