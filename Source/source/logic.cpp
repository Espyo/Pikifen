/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
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
#include "gameplay.h"
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
    
    //"Move group" arrows.
    if(group_move_magnitude) {
        group_move_next_arrow_timer.tick(delta_t);
    }
    
    dist leader_to_cursor_dist(cur_leader_ptr->pos, leader_cursor_w);
    for(size_t a = 0; a < group_move_arrows.size(); ) {
        group_move_arrows[a] += GROUP_MOVE_ARROW_SPEED * delta_t;
        
        dist max_dist =
            (group_move_magnitude > 0) ?
            cursor_max_dist * group_move_magnitude :
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
            clamp(cursor_height_diff_light, -0.33f, 0.33f);
    }
    
    //Whether the held Pikmin can reach the cursor.
    throw_can_reach_cursor = true;
    if(cur_leader_ptr->holding_pikmin) {
    
        if(!cursor_sector || cursor_sector->type == SECTOR_TYPE_BLOCKING) {
            throw_can_reach_cursor = false;
            
        } else {
            float max_throw_z = 0;
            size_t cat = cur_leader_ptr->holding_pikmin->type->category->id;
            if(cat == MOB_CATEGORY_PIKMIN) {
                max_throw_z =
                    (
                        (pikmin*) cur_leader_ptr->holding_pikmin
                    )->pik_type->max_throw_height;
            } else if(cat == MOB_CATEGORY_LEADERS) {
                max_throw_z =
                    (
                        (leader*) cur_leader_ptr->holding_pikmin
                    )->lea_type->max_throw_height;
            }
            
            if(max_throw_z > 0) {
                throw_can_reach_cursor =
                    cursor_sector->z < cur_leader_ptr->z + max_throw_z;
            }
        }
    }
    
    
    //Specific animations.
    spark_animation.instance.tick(delta_t);
    
    //Area title fade.
    area_title_fade_timer.tick(delta_t);
    
    //Fade.
    fade_mgr.tick(delta_t);
    
    
}


const float CAMERA_BOX_MARGIN = 128.0f;

/* ----------------------------------------------------------------------------
 * Ticks the logic of gameplay-related things.
 */
void gameplay::do_gameplay_logic() {

    //Camera movement.
    cam_pos.x +=
        (cam_final_pos.x - cam_pos.x) * (CAMERA_SMOOTHNESS_MULT * delta_t);
    cam_pos.y +=
        (cam_final_pos.y - cam_pos.y) * (CAMERA_SMOOTHNESS_MULT * delta_t);
    cam_zoom +=
        (cam_final_zoom - cam_zoom) * (CAMERA_SMOOTHNESS_MULT * delta_t);
        
    game_states[cur_game_state_nr]->update_transformations();
    
    //Set the camera bounding box.
    cam_box[0] = point(0, 0);
    cam_box[1] = point(scr_w, scr_h);
    al_transform_coordinates(
        &screen_to_world_transform,
        &cam_box[0].x,
        &cam_box[0].y
    );
    al_transform_coordinates(
        &screen_to_world_transform,
        &cam_box[1].x,
        &cam_box[1].y
    );
    cam_box[0].x -= CAMERA_BOX_MARGIN;
    cam_box[0].y -= CAMERA_BOX_MARGIN;
    cam_box[1].x += CAMERA_BOX_MARGIN;
    cam_box[1].y += CAMERA_BOX_MARGIN;
    
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
        for(size_t m = 0; m < n_mobs; ++m) {
            //Tick the mob.
            mob* m_ptr = mobs[m];
            m_ptr->tick();
            
            if(m_ptr->fsm.cur_state) {
                process_mob_interactions(m_ptr, m);
            }
        }
        
        for(size_t m = 0; m < n_mobs;) {
            //Mob deletion.
            mob* m_ptr = mobs[m];
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
        //TODO move this logic to the leader class once
        //multiplayer logic is implemented.
        
        //Current leader movement.
        point dummy_coords;
        float dummy_angle;
        float leader_move_magnitude;
        leader_movement.get_clean_info(
            &dummy_coords, &dummy_angle, &leader_move_magnitude
        );
        if(leader_move_magnitude < 0.75) {
            cur_leader_ptr->fsm.run_event(
                LEADER_EVENT_MOVE_END, (void*) &leader_movement
            );
        } else {
            cur_leader_ptr->fsm.run_event(
                LEADER_EVENT_MOVE_START, (void*) &leader_movement
            );
        }
        
        cam_final_pos = cur_leader_ptr->pos;
        
        //Check proximity with certain key things.
        if(!cur_leader_ptr->auto_plucking) {
            dist closest_d = 0;
            dist d = 0;
            bool done = false;
            
            close_to_ship_to_heal = NULL;
            for(size_t s = 0; s < ships.size(); ++s) {
                ship* s_ptr = ships[s];
                d = dist(cur_leader_ptr->pos, s_ptr->pos);
                if(!s_ptr->is_leader_under_ring(cur_leader_ptr)) {
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
                if(p && d <= pluck_range) {
                    close_to_pikmin_to_pluck = p;
                    done = true;
                }
            }
            
            closest_d = 0;
            d = 0;
            close_to_onion_to_open = NULL;
            if(!done) {
                for(size_t o = 0; o < onions.size(); ++o) {
                    d = dist(cur_leader_ptr->pos, onions[o]->pos);
                    if(d > onion_open_range) continue;
                    if(d < closest_d || !close_to_onion_to_open) {
                        close_to_onion_to_open = onions[o];
                        closest_d = d;
                        done = true;
                    }
                }
            }
            
            closest_d = 0;
            d = 0;
            close_to_spot_to_read = NULL;
            if(!done) {
                for(size_t i = 0; i < info_spots.size(); ++i) {
                    d = dist(cur_leader_ptr->pos, info_spots[i]->pos);
                    if(d > info_spot_trigger_range) continue;
                    if(d < closest_d || !close_to_spot_to_read) {
                        close_to_spot_to_read = info_spots[i];
                        closest_d = d;
                        done = true;
                    }
                }
            }
        }
        
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
        
        float old_group_move_magnitude = group_move_magnitude;
        point group_move_coords;
        float new_group_move_angle;
        group_movement.get_clean_info(
            &group_move_coords, &new_group_move_angle, &group_move_magnitude
        );
        if(group_move_magnitude > 0) {
            //This stops arrows that were fading away to the left from
            //turning to angle 0 because the magnitude reached 0.
            group_move_angle = new_group_move_angle;
        }
        
        if(group_move_cursor) {
            group_move_angle = cursor_angle;
            float leader_to_cursor_dist =
                dist(cur_leader_ptr->pos, leader_cursor_w).to_float();
            group_move_magnitude =
                leader_to_cursor_dist / cursor_max_dist;
        }
        
        if(old_group_move_magnitude != group_move_magnitude) {
            if(group_move_magnitude != 0) {
                cur_leader_ptr->signal_group_move_start();
            } else {
                cur_leader_ptr->signal_group_move_end();
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
            mouse_cursor_speed * delta_t* MOUSE_CURSOR_MOVE_SPEED;
            
        mouse_cursor_s += mouse_cursor_speed;
        
        mouse_cursor_w = mouse_cursor_s;
        al_transform_coordinates(
            &screen_to_world_transform,
            &mouse_cursor_w.x, &mouse_cursor_w.y
        );
        leader_cursor_w = mouse_cursor_w;
        
        cursor_angle =
            get_angle(cur_leader_ptr->pos, leader_cursor_w);
            
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
    
    hud_items.tick(delta_t);
    replay_timer.tick(delta_t);
    
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
    if(creator_tool_info_lock) {
        string name_str =
            box_string("Mob: " + creator_tool_info_lock->type->name, 30);
        string coords_str =
            box_string(
                "Coords: " +
                box_string(f2s(creator_tool_info_lock->pos.x), 7) + " " +
                box_string(f2s(creator_tool_info_lock->pos.y), 7) + " " +
                box_string(f2s(creator_tool_info_lock->z), 7),
                30
            );
        string stateh_str =
            "State hist.: " +
            (
                creator_tool_info_lock->fsm.cur_state ?
                creator_tool_info_lock->fsm.cur_state->name :
                "(None!)"
            );
        for(unsigned char p = 0; p < STATE_HISTORY_SIZE; ++p) {
            stateh_str +=
                ", " + creator_tool_info_lock->fsm.prev_state_names[p];
        }
        string anim_str =
            box_string(
                "Animation: " +
                (creator_tool_info_lock->anim.cur_anim ?
                 creator_tool_info_lock->anim.cur_anim->name :
                 "(None!)"),
                60
            );
        string health_str =
            box_string(
                "Health: " +
                f2s(creator_tool_info_lock->health) +
                " / " +
                f2s(creator_tool_info_lock->type->max_health),
                30
            );
        string timer_str =
            box_string(
                "Timer: " +
                f2s(creator_tool_info_lock->script_timer.time_left),
                30
            );
        string vars_str = "Vars: ";
        if(!creator_tool_info_lock->vars.empty()) {
            for(
                auto v = creator_tool_info_lock->vars.begin();
                v != creator_tool_info_lock->vars.end(); ++v
            ) {
                vars_str += v->first + "=" + v->second + "; ";
            }
            vars_str.erase(vars_str.size() - 2, 2);
        } else {
            vars_str += "(None)";
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
    if(creator_tool_geometry_info) {
        sector* mouse_sector =
            get_sector(mouse_cursor_w, NULL, true);
        string str =
            "Mouse coordinates: " + f2s(mouse_cursor_w.x) +
            ", " + f2s(mouse_cursor_w.y) + "\n"
            "Sector under mouse: " +
            (mouse_sector ? "" : "None") + "\n";
        if(mouse_sector) {
            str +=
                "  Z: " + f2s(mouse_sector->z) + "\n"
                "  Texture: " +
                mouse_sector->texture_info.file_name;
        }
        print_info(str);
    }
    
    info_print_timer.tick(delta_t);
    
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
    /********************************
     *                              *
     *   Mob interactions   () - () *
     *                              *
     ********************************/
    //Interactions between this mob and the others.
    
    //We want the mob to follow inter-mob events from the closest mob to
    //the one farthest away. As such, let's catch all viable mobs and their
    //distances, save it in a vector, and then go through it in order.
    struct pending_intermob_event {
        dist d;
        mob_event* event_ptr;
        mob* mob_ptr;
        pending_intermob_event(
            const dist &d, mob_event* event_ptr, mob* mob_ptr
        ) :
            d(d),
            event_ptr(event_ptr),
            mob_ptr(mob_ptr) { }
    };
    
    vector<pending_intermob_event> pending_intermob_events;
    
    size_t n_mobs = mobs.size();
    for(size_t m2 = 0; m2 < n_mobs; ++m2) {
        if(m == m2) continue;
        
        mob* m2_ptr = mobs[m2];
        float m1_angle_sin = sin(m_ptr->angle);
        float m1_angle_cos = cos(m_ptr->angle);
        float m2_angle_sin = sin(m2_ptr->angle);
        float m2_angle_cos = cos(m2_ptr->angle);
        dist d(m_ptr->pos, m2_ptr->pos);
        float face_diff =
            get_angle_smallest_dif(
                m_ptr->angle,
                get_angle(m_ptr->pos, m2_ptr->pos)
            );
            
        //Check touches. This does not use hitboxes,
        //only the object radii.
        mob_event* touch_op_ev =
            q_get_event(m_ptr, MOB_EVENT_TOUCHED_OPPONENT);
        mob_event* touch_le_ev =
            q_get_event(m_ptr, MOB_EVENT_TOUCHED_ACTIVE_LEADER);
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
                        (m2_ptr->z > m_ptr->z + m_ptr->type->height) ||
                        (m2_ptr->z + m2_ptr->type->height < m2_ptr->z)
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
                if(touch_op_ev && m_ptr->should_attack(m2_ptr)) {
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
        
        //Check if mob 1 should be pushed by mob 2.
        if(
            m2_ptr->type->pushes &&
            m2_ptr->tangible &&
            m_ptr->type->pushable && !m_ptr->unpushable &&
            (
                (
                    m2_ptr->z < m_ptr->z + m_ptr->type->height &&
                    m2_ptr->z + m2_ptr->type->height > m_ptr->z
                ) || (
                    m_ptr->type->height == 0
                ) || (
                    m2_ptr->type->height == 0
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
                
                if(d <= m_ptr->type->radius + s2_ptr->hitbox_span) {
                    for(size_t h = 0; h < s2_ptr->hitboxes.size(); ++h) {
                        hitbox* h_ptr = &s2_ptr->hitboxes[h];
                        if(h_ptr->type == HITBOX_TYPE_DISABLED) continue;
                        point h_pos(
                            m2_ptr->pos.x + (
                                h_ptr->pos.x * m2_angle_cos -
                                h_ptr->pos.y * m2_angle_sin
                            ),
                            m2_ptr->pos.y + (
                                h_ptr->pos.x * m2_angle_sin +
                                h_ptr->pos.y * m2_angle_cos
                            )
                        );
                        
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
                }
                
            } else if(d <= m_ptr->type->radius + m2_ptr->type->radius) {
                //Push with the object radius.
                
                push_amount =
                    fabs(
                        d.to_float() - m_ptr->type->radius -
                        m2_ptr->type->radius
                    );
                push_angle = get_angle(m2_ptr->pos, m_ptr->pos);
                
            }
            
            //If the mob is inside the other,
            //it needs to be pushed out.
            if(push_amount > m_ptr->push_amount) {
                m_ptr->push_amount = push_amount / delta_t;
                m_ptr->push_angle = push_angle;
            }
        }
        
        //Check hitbox touches.
        mob_event* hitbox_touch_n_ev =
            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_N);
        mob_event* hitbox_touch_na_ev =
            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_N_A);
        mob_event* hitbox_touch_eat_ev =
            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_EAT);
        mob_event* hitbox_touch_haz_ev =
            q_get_event(m_ptr, MOB_EVENT_TOUCHED_HAZARD);
            
        sprite* s1_ptr = m_ptr->anim.get_cur_sprite();
        sprite* s2_ptr = m2_ptr->anim.get_cur_sprite();
        
        if(
            (hitbox_touch_n_ev || hitbox_touch_na_ev || hitbox_touch_eat_ev) &&
            s1_ptr && s2_ptr &&
            !s1_ptr->hitboxes.empty() && !s2_ptr->hitboxes.empty() &&
            d < s1_ptr->hitbox_span + s2_ptr->hitbox_span
        ) {
        
            bool reported_n_ev = false;
            bool reported_na_ev = false;
            bool reported_eat_ev = false;
            bool reported_haz_ev = false;
            
            for(size_t h1 = 0; h1 < s1_ptr->hitboxes.size(); ++h1) {
            
                hitbox* h1_ptr = &s1_ptr->hitboxes[h1];
                if(h1_ptr->type == HITBOX_TYPE_DISABLED) continue;
                
                for(size_t h2 = 0; h2 < s2_ptr->hitboxes.size(); ++h2) {
                    hitbox* h2_ptr = &s2_ptr->hitboxes[h2];
                    if(h2_ptr->type == HITBOX_TYPE_DISABLED) continue;
                    
                    //Check if m2 is under any status effect
                    //that disables attacks.
                    bool disable_attack_status = false;
                    for(
                        size_t s = 0;
                        s < m2_ptr->statuses.size(); ++s
                    ) {
                        if(m2_ptr->statuses[s].type->disables_attack) {
                            disable_attack_status = true;
                            break;
                        }
                    }
                    
                    //Get the real hitbox locations.
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
                    float m1_h_z = m_ptr->z + h1_ptr->z;
                    float m2_h_z = m2_ptr->z + h2_ptr->z;
                    
                    bool collided = false;
                    
                    if(m2_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
                        //Pikmin latched on to a hitbox are obviously
                        //touching it.
                        pikmin* p_ptr = (pikmin*) m2_ptr;
                        if(
                            m2_ptr->focused_mob == m_ptr &&
                            p_ptr->connected_hitbox_nr == h1
                        ) {
                            collided = true;
                        }
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
                        hitbox_touch_n_ev &&
                        !reported_n_ev &&
                        h2_ptr->type == HITBOX_TYPE_NORMAL
                    ) {
                        hitbox_interaction ev_info =
                            hitbox_interaction(
                                m2_ptr, h1_ptr, h2_ptr
                            );
                        hitbox_touch_n_ev->run(
                            m_ptr, (void*) &ev_info
                        );
                        reported_n_ev = true;
                        
                        //Re-fetch the other events, since this event
                        //could have triggered a state change.
                        hitbox_touch_eat_ev =
                            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_EAT);
                        hitbox_touch_haz_ev =
                            q_get_event(m_ptr, MOB_EVENT_TOUCHED_HAZARD);
                        hitbox_touch_na_ev =
                            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_N_A);
                    }
                    
                    if(
                        h1_ptr->type == HITBOX_TYPE_NORMAL &&
                        h2_ptr->type == HITBOX_TYPE_ATTACK
                    ) {
                        //Confirmed damage.
                        
                        //Check if the attack already hit this mob.
                        bool already_hit = false;
                        for(
                            size_t ho = 0;
                            ho < m2_ptr->hit_opponents.size();
                            ++ho
                        ) {
                            if(m2_ptr->hit_opponents[ho] == m_ptr) {
                                already_hit = true;
                                break;
                            }
                        }
                        if(already_hit) {
                            continue;
                        } else {
                            m2_ptr->hit_opponents.push_back(m_ptr);
                        }
                        
                        //Hazard resistance check.
                        if(
                            !h2_ptr->hazards.empty() &&
                            m_ptr->is_resistant_to_hazards(h2_ptr->hazards)
                        ) {
                            continue;
                        }
                        
                        //Should this mob even attack this other mob?
                        if(!m2_ptr->should_attack(m_ptr)) {
                            continue;
                        }
                    }
                    
                    //First, the "touched eat hitbox" event.
                    if(
                        hitbox_touch_eat_ev &&
                        !reported_eat_ev &&
                        !disable_attack_status &&
                        h1_ptr->type == HITBOX_TYPE_NORMAL &&
                        m2_ptr->chomping_pikmin.size() <
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
                            q_get_event(m_ptr, MOB_EVENT_TOUCHED_HAZARD);
                        hitbox_touch_na_ev =
                            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_N_A);
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
                            hitbox_touch_haz_ev->run(
                                m_ptr,
                                (void*) h2_ptr->hazards[h]
                            );
                        }
                        reported_haz_ev = true;
                        
                        //Re-fetch the other events, since this event
                        //could have triggered a state change.
                        hitbox_touch_na_ev =
                            q_get_event(m_ptr, MOB_EVENT_HITBOX_TOUCH_N_A);
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
        
        if(m2_ptr->health > 0 && m_ptr->near_reach != INVALID) {
            //Check reaches.
            
            mob_event* obir_ev =
                q_get_event(m_ptr, MOB_EVENT_OBJECT_IN_REACH);
            mob_event* opir_ev =
                q_get_event(m_ptr, MOB_EVENT_OPPONENT_IN_REACH);
                
            mob_type::reach_struct* r_ptr =
                &m_ptr->type->reaches[m_ptr->near_reach];
            if(obir_ev || opir_ev) {
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
                    if(opir_ev && m_ptr->should_attack(m2_ptr)) {
                        pending_intermob_events.push_back(
                            pending_intermob_event(
                                d, opir_ev, m2_ptr
                            )
                        );
                    }
                }
            }
            
        }
        
        //Find a carriable mob to grab.
        mob_event* nco_event =
            q_get_event(m_ptr, MOB_EVENT_NEAR_CARRIABLE_OBJECT);
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
    
    mob_state* state_before = m_ptr->fsm.cur_state;
    for(size_t e = 0; e < pending_intermob_events.size(); ++e) {
        if(!pending_intermob_events[e].event_ptr) continue;
        pending_intermob_events[e].event_ptr->run(
            m_ptr, (void*) pending_intermob_events[e].mob_ptr
        );
        if(m_ptr->fsm.cur_state != state_before) {
            //We can't go on, since the new state might not even have the
            //event, and the reaches could've also changed.
            break;
        }
    }
    
}
