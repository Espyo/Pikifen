#include "const.h"
#include "functions.h"
#include "vars.h"

void do_logic() {

    /*  ********************************************
      ***  .-.                                .-.  ***
    ***** ( L )          MAIN LOGIC          ( L ) *****
      ***  `-´                                `-´  ***
        ********************************************/
    
    leader* cur_leader_ptr = leaders[cur_leader_nr];
    
    
    /*************************************
    *                               .-.  *
    *   Timer things - aesthetic   ( L ) *
    *                               `-´  *
    **************************************/
    
    idle_glow_angle += IDLE_GLOW_SPIN_SPEED / game_fps;
    
    //Camera transitions.
    if(cam_trans_pan_time_left > 0) {
        cam_trans_pan_time_left -= 1.0 / game_fps;
        if(cam_trans_pan_time_left < 0) cam_trans_pan_time_left = 0;
        
        float amount_left = cam_trans_pan_time_left / CAM_TRANSITION_DURATION;
        
        cam_x = cam_trans_pan_initial_x + (cam_trans_pan_final_x - cam_trans_pan_initial_x) * (1 - amount_left);
        cam_y = cam_trans_pan_initial_y + (cam_trans_pan_final_y - cam_trans_pan_initial_y) * (1 - amount_left);
    }
    
    if(cam_trans_zoom_time_left > 0) {
        cam_trans_zoom_time_left -= 1.0 / game_fps;
        if(cam_trans_zoom_time_left < 0) cam_trans_zoom_time_left = 0;
        
        float amount_left = cam_trans_zoom_time_left / CAM_TRANSITION_DURATION;
        
        cam_zoom = cam_trans_zoom_initial_level + (cam_trans_zoom_final_level - cam_trans_zoom_initial_level) * (1 - amount_left);
    }
    
    //"Move group" arrows.
    if(moving_group_intensity) {
        move_group_next_arrow_time -= 1.0 / game_fps;
        if(move_group_next_arrow_time <= 0) {
            move_group_next_arrow_time = MOVE_GROUP_ARROWS_INTERVAL;
            move_group_arrows.push_back(0);
        }
    }
    
    float leader_to_cursor_dis = dist(cur_leader_ptr->x, cur_leader_ptr->y, cursor_x, cursor_y);
    for(size_t a = 0; a < move_group_arrows.size(); ) {
        move_group_arrows[a] += MOVE_GROUP_ARROW_SPEED * (1.0 / game_fps);
        
        float max_dist =
            ((moving_group_intensity > 0) ? max_dist = CURSOR_MAX_DIST* moving_group_intensity : leader_to_cursor_dis);
            
        if(move_group_arrows[a] >= max_dist) {
            move_group_arrows.erase(move_group_arrows.begin() + a);
        } else {
            a++;
        }
    }
    
    //Whistle animations.
    whistle_dot_offset -= WHISTLE_DOT_SPIN_SPEED / game_fps;
    
    if(whistle_fade_time > 0) {
        whistle_fade_time -= 1.0 / game_fps;
        if(whistle_fade_time < 0) whistle_fade_time = 0;
    }
    
    if(whistling) {
        //Create rings.
        whistle_next_ring_time -= 1.0 / game_fps;
        if(whistle_next_ring_time <= 0) {
            whistle_next_ring_time = WHISTLE_RINGS_INTERVAL;
            whistle_rings.push_back(0);
            whistle_ring_colors.push_back(whistle_ring_prev_color);
            whistle_ring_prev_color = (whistle_ring_prev_color + 1) % N_WHISTLE_RING_COLORS;
        }
        
        if(pretty_whistle) {
            whistle_next_dot_time -= 1.0 / game_fps;
            if(whistle_next_dot_time <= 0) {
                whistle_next_dot_time = WHISTLE_DOT_INTERVAL;
                unsigned char dot = 255;
                for(unsigned char d = 0; d < 6; d++) { //Find WHAT dot to add.
                    if(whistle_dot_radius[d] == -1) { dot = d; break;}
                }
                
                if(dot != 255) whistle_dot_radius[dot] = 0;
            }
        }
        
        for(unsigned char d = 0; d < 6; d++) {
            if(whistle_dot_radius[d] == -1) continue;
            
            whistle_dot_radius[d] += WHISTLE_RADIUS_GROWTH_SPEED / game_fps;
            if(whistle_radius > 0 && whistle_dot_radius[d] > cur_leader_ptr->lea_type->whistle_range) {
                whistle_dot_radius[d] = cur_leader_ptr->lea_type->whistle_range;
            } else if(whistle_fade_radius > 0 && whistle_dot_radius[d] > whistle_fade_radius) {
                whistle_dot_radius[d] = whistle_fade_radius;
            }
        }
    }
    
    for(size_t r = 0; r < whistle_rings.size(); ) {
        //Erase rings that go beyond the cursor.
        whistle_rings[r] += WHISTLE_RING_SPEED / game_fps;
        if(whistle_rings[r] >= leader_to_cursor_dis) {
            whistle_rings.erase(whistle_rings.begin() + r);
            whistle_ring_colors.erase(whistle_ring_colors.begin() + r);
        } else {
            r++;
        }
    }
    
    //Ship beam ring.
    //The way this works is that the three color components are saved.
    //Each frame, we increase them or decrease them (if it reaches 255, set it to decrease, if 0, set it to increase).
    //Each index increases/decreases at a different speed, with red being the slowest and blue the fastest.
    for(unsigned char i = 0; i < 3; i++) {
        float dir_mult = (ship_beam_ring_color_up[i]) ? 1.0 : -1.0;
        signed char addition = dir_mult * SHIP_BEAM_RING_COLOR_SPEED * (i + 1) * (1.0 / game_fps);
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
    
    //Sun meter.
    sun_meter_sun_angle += SUN_METER_SUN_SPIN_SPEED / game_fps;
    
    //Cursor spin angle and invalidness effect.
    cursor_spin_angle -= CURSOR_SPIN_SPEED / game_fps;
    cursor_invalid_effect += CURSOR_INVALID_EFFECT_SPEED / game_fps;
    
    //Cursor trail.
    if(cursor_save_time > 0) {
        cursor_save_time -= 1.0 / game_fps;
        if(cursor_save_time <= 0) {
            cursor_save_time = CURSOR_SAVE_INTERVAL;
            cursor_spots.push_back(point(mouse_cursor_x, mouse_cursor_y));
            if(cursor_spots.size() > CURSOR_SAVE_N_SPOTS) {
                cursor_spots.erase(cursor_spots.begin());
            }
        }
    }
    
    
    //Tick all particles.
    size_t n_particles = particles.size();
    for(size_t p = 0; p < n_particles; ) {
        if(!particles[p].tick()) {
            particles.erase(particles.begin() + p);
            n_particles--;
        } else {
            p++;
        }
    }
    
    
    if(cur_message.size() == 0) {
    
        /************************************
        *                              .-.  *
        *   Timer things - gameplay   ( L ) *
        *                              `-´  *
        *************************************/
        
        day_minutes += (day_minutes_per_irl_sec / game_fps);
        if(day_minutes > 60 * 24) day_minutes -= 60 * 24;
        
        if(auto_pluck_input_time > 0) {
            auto_pluck_input_time -= (1.0 / game_fps);
            if(auto_pluck_input_time < 0) auto_pluck_input_time = 0;
        }
        
        /********************
        *              ***  *
        *   Whistle   * O * *
        *              ***  *
        ********************/
        
        if(whistling && whistle_radius < cur_leader_ptr->lea_type->whistle_range) {
            whistle_radius += WHISTLE_RADIUS_GROWTH_SPEED / game_fps;
            if(whistle_radius > cur_leader_ptr->lea_type->whistle_range) {
                whistle_radius = cur_leader_ptr->lea_type->whistle_range;
                whistle_max_hold = WHISTLE_MAX_HOLD_TIME;
            }
        }
        
        if(whistle_max_hold > 0) {
            whistle_max_hold -= 1.0 / game_fps;
            if(whistle_max_hold <= 0) {
                stop_whistling();
            }
        }
        
        
        /*****************
        *                *
        *   Mobs   ()--> *
        *                *
        ******************/
        
        size_t n_mobs = mobs.size();
        for(size_t m = 0; m < n_mobs;) {
            mob* m_ptr = mobs[m];
            m_ptr->tick();
            
            if(m_ptr->carrier_info) {
                if(m_ptr->state == MOB_STATE_BEING_CARRIED && m_ptr->reached_destination && m_ptr->carrier_info->decided_type) {
                    m_ptr->set_state(MOB_STATE_BEING_DELIVERED);
                    sfx_pikmin_carrying.stop();
                }
            }
            
            if(m_ptr->state == MOB_STATE_BEING_DELIVERED && m_ptr->time_in_state >= DELIVERY_SUCK_TIME) {
                if(m_ptr->carrier_info->carry_to_ship) {
                    //Find ship.
                    //ToDo.
                    
                } else {
                    //Find Onion.
                    size_t n_onions = onions.size();
                    size_t o = 0;
                    for(; o < n_onions; o++) {
                        if(onions[o]->oni_type->pik_type == m_ptr->carrier_info->decided_type) break;
                    }
                    
                    if(typeid(*m_ptr) == typeid(pellet)) {
                        pellet* p_ptr = (pellet*) m_ptr;
                        if(p_ptr->pel_type->pik_type == p_ptr->carrier_info->decided_type) {
                            give_pikmin_to_onion(onions[o], p_ptr->pel_type->match_seeds);
                        } else {
                            give_pikmin_to_onion(onions[o], p_ptr->pel_type->non_match_seeds);
                        }
                        
                    } else if(typeid(*m_ptr) == typeid(enemy)) {
                        enemy* e_ptr = (enemy*) m_ptr;
                        give_pikmin_to_onion(onions[o], e_ptr->ene_type->pikmin_seeds);
                        
                    }
                }
                
                random_particle_explosion(
                    PARTICLE_TYPE_BITMAP, bmp_smoke,
                    m_ptr->x, m_ptr->y,
                    60, 80, 10, 20,
                    1, 2, 24, 24, al_map_rgb(255, 255, 255)
                );
                
                make_uncarriable(m_ptr);
                m_ptr->to_delete = true;
            }
            
            //Mob deletion.
            if(m_ptr->to_delete) {
                delete_mob(m_ptr);
                n_mobs--;
                continue;
            }
            m++;
            
            
        }
        
        
        /******************
        *             /\  *
        *   Pikmin   (@:) *
        *             \/  *
        ******************/
        
        size_t n_pikmin = pikmin_list.size();
        for(size_t p = 0; p < n_pikmin; p++) {
            pikmin* pik_ptr = pikmin_list[p];
            
            bool can_be_called =
                !pik_ptr->following_party &&
                pik_ptr->state != PIKMIN_STATE_BURIED &&
                !pik_ptr->speed_z &&
                !pik_ptr->uncallable_period;
            bool whistled = (dist(pik_ptr->x, pik_ptr->y, cursor_x, cursor_y) <= whistle_radius && whistling);
            bool touched =
                dist(pik_ptr->x, pik_ptr->y, cur_leader_ptr->x, cur_leader_ptr->y) <=
                pik_ptr->type->size * 0.5 + cur_leader_ptr->type->size * 0.5 &&
                !cur_leader_ptr->carrier_info;
            bool is_busy = (pik_ptr->carrying_mob || pik_ptr->attacking_mob);
            
            if(pik_ptr->health <= 0 && !pik_ptr->dead) {
                pik_ptr->dead = true;
                pik_ptr->to_delete = true;
                particles.push_back(
                    particle(
                        PARTICLE_TYPE_PIKMIN_SPIRIT, bmp_pikmin_spirit, pik_ptr->x, pik_ptr->y,
                        0, -50, 0.5, 0, 2, pik_ptr->pik_type->size, pik_ptr->pik_type->main_color
                    )
                );
                sfx_pikmin_dying.play(0.03, false);
                
            }
            
            if(can_be_called && (whistled || (touched && !is_busy))) {
            
                //Pikmin got whistled or touched.
                drop_mob(pik_ptr);
                pik_ptr->attacking_mob = NULL;
                pik_ptr->attack_time = 0;
                add_to_party(cur_leader_ptr, pik_ptr);
                sfx_pikmin_called.play(0.03, false);
                
                pik_ptr->attacking_mob = NULL;
                pik_ptr->set_state(PIKMIN_STATE_IN_GROUP);
                
            }
            
            //Touching nectar.
            size_t n_nectars = nectars.size();
            if(
                !pik_ptr->carrying_mob &&
                !pik_ptr->attacking_mob &&
                pik_ptr->state != PIKMIN_STATE_BURIED &&
                !pik_ptr->speed_z &&
                pik_ptr->maturity != 2
            ) {
                for(size_t n = 0; n < n_nectars; n++) {
                    if(dist(pik_ptr->x, pik_ptr->y, nectars[n]->x, nectars[n]->y) <= nectars[n]->type->size * 0.5 + pik_ptr->type->size * 0.5) {
                        if(nectars[n]->amount_left > 0)
                            nectars[n]->amount_left--;
                            
                        pik_ptr->maturity = 2;
                    }
                }
            }
            
            //Latch onto a mob.
            size_t n_mobs = mobs.size();
            if(pik_ptr->was_thrown) {
                for(size_t m = 0; m < n_mobs; m++) {
                
                    mob* mob_ptr = mobs[m];
                    if(mob_ptr->dead) continue;
                    if(!should_attack(pik_ptr, mob_ptr)) continue;
                    if(dist(pik_ptr->x, pik_ptr->y, mob_ptr->x, mob_ptr->y) > pik_ptr->type->size * 0.5 + mob_ptr->type->size * 0.5) continue;
                    
                    hitbox_instance* closest_hitbox = get_closest_hitbox(pik_ptr->x, pik_ptr->y, mob_ptr);
                    pik_ptr->attacking_hitbox_name = closest_hitbox->hitbox_name;
                    
                    float actual_hx, actual_hy;
                    rotate_point(closest_hitbox->x, closest_hitbox->y, mob_ptr->angle, &actual_hx, &actual_hy);
                    actual_hx += mob_ptr->x; actual_hy += mob_ptr->y;
                    
                    //ToDo there should be a way to optimize this.
                    float x_dif = pik_ptr->x - actual_hx;
                    float y_dif = pik_ptr->y - actual_hy;
                    rotate_point(x_dif, y_dif, -mob_ptr->angle, &pik_ptr->attacking_hitbox_x, &pik_ptr->attacking_hitbox_y);
                    
                    pik_ptr->attacking_mob = mob_ptr;
                    pik_ptr->state = PIKMIN_STATE_ATTACKING_MOB;
                    pik_ptr->latched = true;
                    
                    pik_ptr->set_target(0, 0, &mob_ptr->x, &mob_ptr->y, true);
                }
            }
            
            //Finding a mob to fight.
            if(
                (!pik_ptr->following_party &&
                 !pik_ptr->carrying_mob &&
                 !pik_ptr->wants_to_carry &&
                 !pik_ptr->attacking_mob &&
                 pik_ptr->state != PIKMIN_STATE_BURIED &&
                 !pik_ptr->speed_z) ||
                (pik_ptr->following_party && moving_group_intensity)
            ) {
                for(size_t m = 0; m < n_mobs; m++) {
                
                    mob* mob_ptr = mobs[m];
                    
                    if(mob_ptr->dead) continue;
                    if(!should_attack(pik_ptr, mob_ptr)) continue;
                    if(dist(pik_ptr->x, pik_ptr->y, mob_ptr->x, mob_ptr->y) > pik_ptr->type->size * 0.5 + mob_ptr->type->size * 0.5 + PIKMIN_MIN_TASK_RANGE) continue;
                    
                    hitbox_instance* closest_hitbox = get_closest_hitbox(pik_ptr->x, pik_ptr->y, mob_ptr);
                    pik_ptr->attacking_hitbox_name = closest_hitbox->hitbox_name;
                    
                    pik_ptr->attacking_mob = mob_ptr;
                    pik_ptr->latched = false;
                    remove_from_party(pik_ptr);
                    pik_ptr->state = PIKMIN_STATE_ATTACKING_MOB;
                }
            }
            
            
            //Finding a mob to carry.
            if(
                (!pik_ptr->following_party &&
                 !pik_ptr->carrying_mob &&
                 !pik_ptr->wants_to_carry &&
                 !pik_ptr->attacking_mob &&
                 pik_ptr->state != PIKMIN_STATE_BURIED &&
                 !pik_ptr->speed_z) ||
                (pik_ptr->following_party && moving_group_intensity)
            ) {
                for(size_t m = 0; m < n_mobs; m++) {
                
                    mob* mob_ptr = mobs[m];
                    
                    if(!mob_ptr->carrier_info) continue;
                    if(mob_ptr->carrier_info->current_n_carriers == mob_ptr->carrier_info->max_carriers) continue; //No more room.
                    if(mob_ptr->state == MOB_STATE_BEING_DELIVERED) continue;
                    
                    if(dist(pik_ptr->x, pik_ptr->y, mob_ptr->x, mob_ptr->y) <= pik_ptr->type->size * 0.5 + mob_ptr->type->size * 0.5 + PIKMIN_MIN_TASK_RANGE) {
                        pik_ptr->wants_to_carry = mob_ptr;
                        remove_from_party(pik_ptr);
                        pik_ptr->set_state(PIKMIN_STATE_MOVING_TO_CARRY_SPOT);
                        
                        //ToDo remove this random cycle and replace with something more optimal.
                        bool valid_spot = false;
                        unsigned int spot = 0;
                        while(!valid_spot) {
                            spot = random(0, mob_ptr->carrier_info->max_carriers - 1);
                            valid_spot = !mob_ptr->carrier_info->carrier_spots[spot];
                        }
                        
                        pik_ptr->set_target(
                            mob_ptr->carrier_info->carrier_spots_x[spot],
                            mob_ptr->carrier_info->carrier_spots_y[spot],
                            &mob_ptr->x,
                            &mob_ptr->y,
                            false
                        );
                        
                        mob_ptr->carrier_info->carrier_spots[spot] = pik_ptr;
                        mob_ptr->carrier_info->current_n_carriers++;
                        
                        pik_ptr->carrying_spot = spot;
                        
                        break;
                    }
                }
            }
            
            //Reaching to the mob carry spot.
            if(pik_ptr->state == PIKMIN_STATE_MOVING_TO_CARRY_SPOT && pik_ptr->reached_destination) {
                pik_ptr->set_state(PIKMIN_STATE_CARRYING);
                pik_ptr->carrying_mob = pik_ptr->wants_to_carry;
                pik_ptr->wants_to_carry = NULL;
                
                if(pik_ptr->carrying_mob->state != MOB_STATE_BEING_DELIVERED) {
                
                    pik_ptr->set_target(
                        pik_ptr->carrying_mob->carrier_info->carrier_spots_x[pik_ptr->carrying_spot],
                        pik_ptr->carrying_mob->carrier_info->carrier_spots_y[pik_ptr->carrying_spot],
                        &pik_ptr->carrying_mob->x,
                        &pik_ptr->carrying_mob->y,
                        true
                    );
                    
                    pik_ptr->carrying_mob->carrier_info->current_carrying_strength += pik_ptr->pik_type->carry_strength;
                    
                    if(pik_ptr->carrying_mob->carrier_info->current_carrying_strength >= pik_ptr->carrying_mob->type->weight) {
                        start_carrying(pik_ptr->carrying_mob, pik_ptr, NULL);
                    }
                    
                    pik_ptr->uncallable_period = 0;
                    sfx_pikmin_carrying_grab.play(0.03, false);
                }
            }
            
            //Fighting an enemy.
            if(pik_ptr->attacking_mob) {
                hitbox_instance* h_ptr = get_hitbox(pik_ptr->attacking_mob, pik_ptr->attacking_hitbox_name);
                if(h_ptr) {
                    float actual_hx, actual_hy;
                    rotate_point(h_ptr->x, h_ptr->y, pik_ptr->attacking_mob->angle, &actual_hx, &actual_hy);
                    actual_hx += pik_ptr->attacking_mob->x; actual_hy += pik_ptr->attacking_mob->y;
                    
                    if(pik_ptr->latched) {
                        float final_px, final_py;
                        rotate_point(pik_ptr->attacking_hitbox_x, pik_ptr->attacking_hitbox_y, pik_ptr->attacking_mob->angle, &final_px, &final_py);
                        final_px += actual_hx; final_py += actual_hy;
                        
                        pik_ptr->set_target(final_px, final_py, NULL, NULL, true);
                        pik_ptr->face(atan2(pik_ptr->attacking_mob->y - pik_ptr->y, pik_ptr->attacking_mob->x - pik_ptr->x));
                        if(pik_ptr->attack_time == 0) pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
                        
                    } else {
                        if(dist(pik_ptr->x, pik_ptr->y, actual_hx, actual_hy) <= pik_ptr->type->size * 0.5 + h_ptr->radius + PIKMIN_MIN_ATTACK_RANGE) {
                            pik_ptr->remove_target(true);
                            pik_ptr->face(atan2(actual_hy - pik_ptr->y, actual_hx - pik_ptr->x));
                            if(pik_ptr->attack_time == 0) pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
                        } else {
                            pik_ptr->set_target(actual_hx, actual_hy, NULL, NULL, false);
                            pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
                        }
                    }
                }
                
                pik_ptr->attack_time -= 1.0 / game_fps;
                if(pik_ptr->attack_time <= 0) {
                    pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
                    attack(pik_ptr, pik_ptr->attacking_mob, true, pik_ptr->pik_type->attack_power);
                    sfx_attack.play(0.06, false);
                    sfx_pikmin_attack.play(0.06, false);
                    particles.push_back(
                        particle(
                            PARTICLE_TYPE_SMACK, bmp_smack,
                            pik_ptr->x, pik_ptr->y,
                            0, 0, 0, 0,
                            SMACK_PARTICLE_DUR,
                            64,
                            al_map_rgb(255, 160, 128)
                        )
                    );
                }
                
                if(pik_ptr->attacking_mob->dead) {
                    pik_ptr->state = PIKMIN_STATE_CELEBRATING;
                    pik_ptr->remove_target(true);
                    pik_ptr->attacking_mob = NULL;
                }
            }
            
            //Touching mobs.
            for(size_t m = 0; m < mobs.size(); m++) {
                if(typeid(*mobs[m]) == typeid(pikmin)) continue;
                mob* m_ptr = mobs[m];
                frame* f_ptr = m_ptr->anim.get_frame();
                if(f_ptr == NULL) continue; //ToDo report
                
                for(size_t h = 0; h < f_ptr->hitbox_instances.size(); h++) {
                    hitbox_instance* h_ptr = &f_ptr->hitbox_instances[h];
                    float s = sin(m_ptr->angle);
                    float c = cos(m_ptr->angle);
                    float h_x = m_ptr->x + (h_ptr->x * c - h_ptr->y * s);
                    float h_y = m_ptr->y + (h_ptr->x * s + h_ptr->y * c);
                    
                    if(dist(pik_ptr->x, pik_ptr->y, h_x, h_y) <= pik_ptr->type->size / 2 + h_ptr->radius) {
                        if(m_ptr->type->anim.hitboxes[h_ptr->hitbox_name].type == HITBOX_TYPE_ATTACK) {
                            pik_ptr->health = 0;
                        }
                    }
                }
            }
            
            if(pik_ptr->carrying_mob) {
                pik_ptr->face(atan2(pik_ptr->carrying_mob->y - pik_ptr->y, pik_ptr->carrying_mob->x - pik_ptr->x));
            }
            
        }
        
        
        /********************
        *              .-.  *
        *   Leaders   (*:O) *
        *              `-´  *
        ********************/
        
        if(cur_leader_ptr->holding_pikmin) {
            cur_leader_ptr->holding_pikmin->x = cur_leader_ptr->x + cos(cur_leader_ptr->angle + M_PI) * cur_leader_ptr->type->size / 2;
            cur_leader_ptr->holding_pikmin->y = cur_leader_ptr->y + sin(cur_leader_ptr->angle + M_PI) * cur_leader_ptr->type->size / 2;
        }
        
        //Current leader movement.
        if(!cur_leader_ptr->auto_pluck_mode) {
            float leader_move_intensity = dist(0, 0, leader_move_x, leader_move_y);
            if(leader_move_intensity < 0.75) leader_move_intensity = 0;
            if(leader_move_intensity > 1) leader_move_intensity = 1;
            if(leader_move_intensity == 0)
                cur_leader_ptr->remove_target(true);
            else
                cur_leader_ptr->set_target(
                    cur_leader_ptr->x + leader_move_x * cur_leader_ptr->type->move_speed,
                    cur_leader_ptr->y + leader_move_y * cur_leader_ptr->type->move_speed,
                    NULL, NULL, false);
        }
        
        size_t n_leaders = leaders.size();
        for(size_t l = 0; l < n_leaders; l++) {
            if(whistling) {
                if(l != cur_leader_nr) {
                    if(
                        dist(leaders[l]->x, leaders[l]->y, cursor_x, cursor_y) <= whistle_radius &&
                        !leaders[l]->following_party &&
                        !leaders[l]->was_thrown) {
                        //Leader got whistled.
                        add_to_party(cur_leader_ptr, leaders[l]);
                        leaders[l]->auto_pluck_mode = false;
                        
                        size_t n_party_members = leaders[l]->party->members.size();
                        for(size_t m = 0; m < n_party_members; m++) {
                            mob* member = leaders[l]->party->members[0];
                            remove_from_party(member);
                            add_to_party(cur_leader_ptr, member);
                        }
                    }
                }
            }
            
            if(leaders[l]->following_party && !leaders[l]->auto_pluck_mode) {
                leaders[l]->set_target(
                    0,
                    0,
                    &leaders[l]->following_party->party->party_center_x,
                    &leaders[l]->following_party->party->party_center_y,
                    false);
            } else {
                if(leaders[l]->auto_pluck_mode) {
                    if(leaders[l]->auto_pluck_pikmin && leaders[l]->reached_destination) {
                    
                        leader* new_pikmin_leader = leaders[l];
                        if(leaders[l]->following_party) {
                            if(typeid(*leaders[l]->following_party) == typeid(leader)) {
                                //If this leader is following another one, then the new Pikmin should be a part of that top leader.
                                new_pikmin_leader = (leader*) leaders[l]->following_party;
                            }
                        }
                        
                        //Reached the Pikmin we want to pluck. Pluck it and find a new one.
                        pluck_pikmin(new_pikmin_leader, leaders[l]->auto_pluck_pikmin);
                        leaders[l]->auto_pluck_pikmin = NULL;
                    }
                    
                    if(!leaders[l]->auto_pluck_pikmin) {
                        float d;
                        pikmin* new_pikmin = get_closest_buried_pikmin(leaders[l]->x, leaders[l]->y, &d, true);
                        
                        if(new_pikmin && d <= AUTO_PLUCK_MAX_RADIUS) {
                            leaders[l]->auto_pluck_pikmin = new_pikmin;
                            new_pikmin->pluck_reserved = true;
                            leaders[l]->set_target(new_pikmin->x, new_pikmin->y, NULL, NULL, false);
                        } else { //No more buried Pikmin, or none nearby. Give up.
                            leaders[l]->auto_pluck_mode = false;
                            leaders[l]->remove_target(true);
                        }
                    }
                } else {
                    if(leaders[l]->auto_pluck_pikmin) {
                        //Cleanup.
                        leaders[l]->auto_pluck_pikmin->pluck_reserved = false;
                        leaders[l]->auto_pluck_pikmin = NULL;
                        leaders[l]->remove_target(true);
                    }
                }
            }
            
        }
        
        if(cam_trans_pan_time_left > 0) {
            cam_trans_pan_final_x = cur_leader_ptr->x;
            cam_trans_pan_final_y = cur_leader_ptr->y;
        } else {
            cam_x = cur_leader_ptr->x;
            cam_y = cur_leader_ptr->y;
        }
        
        
        /***********************************
        *                             ***  *
        *   Current leader's group   ****O *
        *                             ***  *
        ************************************/
        
        float closest_distance = 0;
        size_t n_members = cur_leader_ptr->party->members.size();
        closest_party_member = cur_leader_ptr->holding_pikmin;
        
        if(n_members > 0 && !closest_party_member) {
        
            for(size_t m = 0; m < n_members; m++) {
                float d = dist(cur_leader_ptr->x, cur_leader_ptr->y, cur_leader_ptr->party->members[m]->x, cur_leader_ptr->party->members[m]->y);
                if(m == 0 || d < closest_distance) {
                    closest_distance = d;
                    closest_party_member = cur_leader_ptr->party->members[m];
                }
            }
            
            if(closest_distance > MIN_GRAB_RANGE) {
                closest_party_member = NULL;
            }
        }
        
        if(moving_group_to_cursor) {
            moving_group_angle = cursor_angle;
            moving_group_intensity = leader_to_cursor_dis / CURSOR_MAX_DIST;
        } else if(moving_group_pos_x != 0 || moving_group_pos_y != 0) {
            coordinates_to_angle(
                moving_group_pos_x, moving_group_pos_y,
                &moving_group_angle, &moving_group_intensity);
            if(moving_group_intensity > 1) moving_group_intensity = 1;
        } else {
            moving_group_intensity = 0;
        }
        
        if(moving_group_intensity) {
            cur_leader_ptr->party->party_center_x = cur_leader_ptr->x + cos(moving_group_angle) * moving_group_intensity * CURSOR_MAX_DIST;
            cur_leader_ptr->party->party_center_y = cur_leader_ptr->y + sin(moving_group_angle) * moving_group_intensity * CURSOR_MAX_DIST;
        } else if(prev_moving_group_intensity != 0) {
            float d = get_leader_to_group_center_dist(cur_leader_ptr);
            cur_leader_ptr->party->party_center_x = cur_leader_ptr->x + cos(moving_group_angle) * d;
            cur_leader_ptr->party->party_center_y = cur_leader_ptr->y + sin(moving_group_angle) * d;
        }
        prev_moving_group_intensity = moving_group_intensity;
        
        
        /********************
        *             .-.   *
        *   Cursor   ( = )> *
        *             `-´   *
        ********************/
        
        mouse_cursor_x += mouse_cursor_speed_x;
        mouse_cursor_y += mouse_cursor_speed_y;
        
        float mcx = mouse_cursor_x, mcy = mouse_cursor_y;
        ALLEGRO_TRANSFORM world_to_screen_transform = get_world_to_screen_transform();
        ALLEGRO_TRANSFORM screen_to_world_transform = world_to_screen_transform;
        al_invert_transform(&screen_to_world_transform);
        al_transform_coordinates(&screen_to_world_transform, &mcx, &mcy);
        cursor_x = mcx;
        cursor_y = mcy;
        
        if(!cur_leader_ptr->auto_pluck_mode) {
            cursor_angle = atan2(cursor_y - cur_leader_ptr->y, cursor_x - cur_leader_ptr->x);
            cur_leader_ptr->face(cursor_angle);
        }
        
        leader_to_cursor_dis = dist(cur_leader_ptr->x, cur_leader_ptr->y, cursor_x, cursor_y);
        if(leader_to_cursor_dis > CURSOR_MAX_DIST) {
            //ToDo with an analog stick, if the cursor is being moved, it's considered off-limit a lot more than it should.
            //Cursor goes beyond the range limit.
            cursor_x = cur_leader_ptr->x + (cos(cursor_angle) * CURSOR_MAX_DIST);
            cursor_y = cur_leader_ptr->y + (sin(cursor_angle) * CURSOR_MAX_DIST);
            mouse_cursor_valid = false;
            
            if(mouse_cursor_speed_x != 0 || mouse_cursor_speed_y != 0) {
                //If we're speeding the mouse cursor (via analog stick), don't let it go beyond the edges.
                mouse_cursor_x = cursor_x;
                mouse_cursor_y = cursor_y;
                al_transform_coordinates(&world_to_screen_transform, &mouse_cursor_x, &mouse_cursor_y);
            }
        } else {
            mouse_cursor_valid = true;
        }
        
        
        /**************************
        *                    /  / *
        *   Percipitation     / / *
        *                   /  /  *
        **************************/
        
        if(cur_weather.percipitation_type != PERCIPITATION_TYPE_NONE) {
            percipitation_time_left -= (1.0 / game_fps);
            if(percipitation_time_left <= 0) {
                percipitation_time_left = cur_weather.percipitation_frequency.get_random_number();
                percipitation.push_back(point(0, 0));
            }
            
            for(size_t p = 0; p < percipitation.size();) {
                percipitation[p].y += cur_weather.percipitation_speed.get_random_number() / game_fps;
                if(percipitation[p].y > scr_h) {
                    percipitation.erase(percipitation.begin() + p);
                } else {
                    p++;
                }
            }
        }
        
        
        /**********************
        *                 *   *
        *   Particles   *   * *
        *                ***  *
        **********************/
        
        throw_particle_timer -= 1.0 / game_fps;
        if(throw_particle_timer <= 0) {
            throw_particle_timer = THROW_PARTICLE_INTERVAL;
            
            for(size_t l = 0; l < n_leaders; l++) {
                if(leaders[l]->was_thrown)
                    particles.push_back(
                        particle(
                            PARTICLE_TYPE_CIRCLE, NULL, leaders[l]->x, leaders[l]->y, 0, 0, 0, 0, 0.6, leaders[l]->type->size * 0.5, change_alpha(leaders[l]->type->main_color, 128)
                        )
                    );
            }
            
            for(size_t p = 0; p < n_pikmin; p++) {
                if(pikmin_list[p]->was_thrown)
                    particles.push_back(
                        particle(
                            PARTICLE_TYPE_CIRCLE, NULL, pikmin_list[p]->x, pikmin_list[p]->y, 0, 0, 0, 0, 0.6, pikmin_list[p]->type->size * 0.5, change_alpha(pikmin_list[p]->type->main_color, 128)
                        )
                    );
            }
        }
        
    } else { //Displaying a message.
    
        if(cur_message_char < cur_message_stopping_chars[cur_message_section + 1]) {
            cur_message_char_time -= 1.0 / game_fps;
            if(cur_message_char_time <= 0) {
                cur_message_char_time = MESSAGE_CHAR_INTERVAL;
                cur_message_char++;
            }
        }
        
    }
}