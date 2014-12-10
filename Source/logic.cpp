/*
 * Copyright (c) Andr� 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Main game loop logic.
 */

#include <algorithm>

#include "const.h"
#include "functions.h"
#include "logic.h"
#include "pikmin.h"
#include "vars.h"

void do_logic() {

    /*  ********************************************
      ***  .-.                                .-.  ***
    ***** ( L )          MAIN LOGIC          ( L ) *****
      ***  `-�                                `-�  ***
        ********************************************/
    
    leader* cur_leader_ptr = leaders[cur_leader_nr];
    
    
    /*************************************
    *                               .-.  *
    *   Timer things - aesthetic   ( L ) *
    *                               `-�  *
    **************************************/
    
    //Rotation angle for the glow atop idle Pikmin.
    idle_glow_angle += IDLE_GLOW_SPIN_SPEED * delta_t;
    
    //Camera transitions.
    if(cam_trans_pan_time_left > 0) {
        cam_trans_pan_time_left -= delta_t;
        if(cam_trans_pan_time_left < 0) cam_trans_pan_time_left = 0;
        
        float amount_left = cam_trans_pan_time_left / CAM_TRANSITION_DURATION;
        
        cam_x = cam_trans_pan_initial_x + (cam_trans_pan_final_x - cam_trans_pan_initial_x) * (1 - amount_left);
        cam_y = cam_trans_pan_initial_y + (cam_trans_pan_final_y - cam_trans_pan_initial_y) * (1 - amount_left);
    }
    
    if(cam_trans_zoom_time_left > 0) {
        cam_trans_zoom_time_left -= delta_t;
        if(cam_trans_zoom_time_left < 0) cam_trans_zoom_time_left = 0;
        
        float amount_left = cam_trans_zoom_time_left / CAM_TRANSITION_DURATION;
        
        cam_zoom = cam_trans_zoom_initial_level + (cam_trans_zoom_final_level - cam_trans_zoom_initial_level) * (1 - amount_left);
    }
    
    //"Move group" arrows.
    if(moving_group_intensity) {
        move_group_next_arrow_time -= delta_t;
        if(move_group_next_arrow_time <= 0) {
            move_group_next_arrow_time = MOVE_GROUP_ARROWS_INTERVAL;
            move_group_arrows.push_back(0);
        }
    }
    
    float leader_to_cursor_dis = dist(cur_leader_ptr->x, cur_leader_ptr->y, cursor_x, cursor_y);
    for(size_t a = 0; a < move_group_arrows.size(); ) {
        move_group_arrows[a] += MOVE_GROUP_ARROW_SPEED * delta_t;
        
        float max_dist =
            ((moving_group_intensity > 0) ? max_dist = CURSOR_MAX_DIST* moving_group_intensity : leader_to_cursor_dis);
            
        if(move_group_arrows[a] >= max_dist) {
            move_group_arrows.erase(move_group_arrows.begin() + a);
        } else {
            a++;
        }
    }
    
    //Whistle animations.
    whistle_dot_offset -= WHISTLE_DOT_SPIN_SPEED * delta_t;
    
    if(whistle_fade_time > 0) {
        whistle_fade_time -= delta_t;
        if(whistle_fade_time < 0) whistle_fade_time = 0;
    }
    
    if(whistling) {
        //Create rings.
        whistle_next_ring_time -= delta_t;
        if(whistle_next_ring_time <= 0) {
            whistle_next_ring_time = WHISTLE_RINGS_INTERVAL;
            whistle_rings.push_back(0);
            whistle_ring_colors.push_back(whistle_ring_prev_color);
            whistle_ring_prev_color = (whistle_ring_prev_color + 1) % N_WHISTLE_RING_COLORS;
        }
        
        if(pretty_whistle) {
            whistle_next_dot_time -= delta_t;
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
            
            whistle_dot_radius[d] += WHISTLE_RADIUS_GROWTH_SPEED * delta_t;
            if(whistle_radius > 0 && whistle_dot_radius[d] > cur_leader_ptr->lea_type->whistle_range) {
                whistle_dot_radius[d] = cur_leader_ptr->lea_type->whistle_range;
            } else if(whistle_fade_radius > 0 && whistle_dot_radius[d] > whistle_fade_radius) {
                whistle_dot_radius[d] = whistle_fade_radius;
            }
        }
    }
    
    for(size_t r = 0; r < whistle_rings.size(); ) {
        //Erase rings that go beyond the cursor.
        whistle_rings[r] += WHISTLE_RING_SPEED * delta_t;
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
        signed char addition = dir_mult * SHIP_BEAM_RING_COLOR_SPEED * (i + 1) * delta_t;
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
    sun_meter_sun_angle += SUN_METER_SUN_SPIN_SPEED * delta_t;
    
    //Cursor spin angle and invalidness effect.
    cursor_spin_angle -= CURSOR_SPIN_SPEED * delta_t;
    cursor_invalid_effect += CURSOR_INVALID_EFFECT_SPEED * delta_t;
    
    //Cursor trail.
    if(draw_cursor_trail) {
        if(cursor_save_time > 0) {
            cursor_save_time -= delta_t;
            if(cursor_save_time <= 0) {
                cursor_save_time = CURSOR_SAVE_INTERVAL;
                cursor_spots.push_back(point(mouse_cursor_x, mouse_cursor_y));
                if(cursor_spots.size() > CURSOR_SAVE_N_SPOTS) {
                    cursor_spots.erase(cursor_spots.begin());
                }
            }
        }
    }
    
    //Tree shadow swaying.
    tree_shadow_sway += TREE_SHADOW_SWAY_SPEED * delta_t;
    
    
    if(cur_message.empty()) {
    
        /************************************
        *                              .-.  *
        *   Timer things - gameplay   ( L ) *
        *                              `-�  *
        *************************************/
        
        day_minutes += (day_minutes_per_irl_sec * delta_t);
        if(day_minutes > 60 * 24) day_minutes -= 60 * 24;
        
        if(auto_pluck_input_time > 0) {
            auto_pluck_input_time -= delta_t;
            if(auto_pluck_input_time < 0) auto_pluck_input_time = 0;
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
        
        /********************
        *              ***  *
        *   Whistle   * O * *
        *              ***  *
        ********************/
        
        if(whistling && whistle_radius < cur_leader_ptr->lea_type->whistle_range) {
            whistle_radius += WHISTLE_RADIUS_GROWTH_SPEED * delta_t;
            if(whistle_radius > cur_leader_ptr->lea_type->whistle_range) {
                whistle_radius = cur_leader_ptr->lea_type->whistle_range;
                whistle_max_hold = WHISTLE_MAX_HOLD_TIME;
            }
        }
        
        if(whistle_max_hold > 0) {
            whistle_max_hold -= delta_t;
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
        
            //Tick the mob.
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
                if(typeid(*m_ptr) != typeid(leader)) m_ptr->to_delete = true;
            }
            
            //Mob deletion.
            if(m_ptr->to_delete) {
                delete_mob(m_ptr);
                n_mobs--;
                continue;
            }
            m++;
            
            
        }
        
        
        /*******************************
        *                              *
        *   Mob interactions   () - () *
        *                              *
        *******************************/
        
        
        
        
        //Uh... This is a placeholder, I guess?
        
        
        
        
        
        /******************
        *             /\  *
        *   Pikmin   (@:) *
        *             \/  *
        ******************/
        
        size_t n_pikmin = pikmin_list.size();
        size_t pikmin_ai_start = floor(pikmin_ai_portion * ((double) n_pikmin / N_PIKMIN_AI_PORTIONS));
        size_t pikmin_ai_end = floor((pikmin_ai_portion + 1) * ((double)n_pikmin / N_PIKMIN_AI_PORTIONS));
        pikmin_ai_portion = (pikmin_ai_portion + 1) % N_PIKMIN_AI_PORTIONS;
        for(size_t p = pikmin_ai_start; p < pikmin_ai_end; p++) {
            pikmin* pik_ptr = pikmin_list[p];
            
            bool can_be_called =
                !pik_ptr->following_party &&
                pik_ptr->state != PIKMIN_STATE_BURIED &&
                !pik_ptr->speed_z &&
                !pik_ptr->being_chomped;
            bool whistled = check_dist(pik_ptr->x, pik_ptr->y, cursor_x, cursor_y, whistle_radius) && whistling && pik_ptr->unwhistlable_period == 0;
            bool touched =
                check_dist(pik_ptr->x, pik_ptr->y, cur_leader_ptr->x, cur_leader_ptr->y,
                           pik_ptr->type->radius + cur_leader_ptr->type->radius) &&
                !cur_leader_ptr->carrier_info &&
                cur_leader_ptr->state != MOB_STATE_BEING_DELIVERED &&
                pik_ptr->untouchable_period == 0;
            bool is_busy = (pik_ptr->carrying_mob || pik_ptr->attacking_mob);
            
            if(pik_ptr->dead) {
            
                pik_ptr->to_delete = true;
                particles.push_back(
                    particle(
                        PARTICLE_TYPE_PIKMIN_SPIRIT, bmp_pikmin_spirit, pik_ptr->x, pik_ptr->y,
                        0, -50, 0.5, 0, 2, pik_ptr->pik_type->size, pik_ptr->pik_type->main_color
                    )
                );
                sfx_pikmin_dying.play(0.03, false);
                continue;
                
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
                    if(check_dist(pik_ptr->x, pik_ptr->y, nectars[n]->x, nectars[n]->y, nectars[n]->type->radius + pik_ptr->type->radius)) {
                        if(nectars[n]->amount_left > 0)
                            nectars[n]->amount_left--;
                            
                        pik_ptr->maturity = 2;
                    }
                }
            }
            
            //Latch onto a mob.
            size_t n_mobs = mobs.size();
            if(pik_ptr->was_thrown && !pik_ptr->attacking_mob) {
                for(size_t m = 0; m < n_mobs; m++) {
                
                    mob* mob_ptr = mobs[m];
                    if(mob_ptr->dead) continue;
                    if(!should_attack(pik_ptr, mob_ptr)) continue;
                    if(!check_dist(pik_ptr->x, pik_ptr->y, mob_ptr->x, mob_ptr->y, pik_ptr->type->radius + mob_ptr->type->radius)) continue;
                    if(pik_ptr->z > mob_ptr->z + 100) continue; //ToDo this isn't taking height into account.
                    
                    hitbox_instance* closest_hitbox = get_closest_hitbox(pik_ptr->x, pik_ptr->y, mob_ptr);
                    if(!closest_hitbox) continue;
                    pik_ptr->enemy_hitbox_nr = closest_hitbox->hitbox_nr;
                    pik_ptr->speed_x = pik_ptr->speed_y = pik_ptr->speed_z = 0;
                    
                    float actual_hx, actual_hy;
                    rotate_point(closest_hitbox->x, closest_hitbox->y, mob_ptr->angle, &actual_hx, &actual_hy);
                    actual_hx += mob_ptr->x; actual_hy += mob_ptr->y;
                    
                    float x_dif = pik_ptr->x - actual_hx;
                    float y_dif = pik_ptr->y - actual_hy;
                    coordinates_to_angle(x_dif, y_dif, &pik_ptr->enemy_hitbox_angle, &pik_ptr->enemy_hitbox_dist);
                    pik_ptr->enemy_hitbox_angle -= mob_ptr->angle; //Relative to 0 degrees.
                    pik_ptr->enemy_hitbox_dist /= closest_hitbox->radius; //Distance in units to distance in percentage.
                    
                    pik_ptr->attacking_mob = mob_ptr;
                    pik_ptr->state = PIKMIN_STATE_ATTACKING_MOB;
                    pik_ptr->latched = true;
                    pik_ptr->was_thrown = false;
                    pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
                    pik_ptr->anim.change(PIKMIN_ANIM_ATTACK, true, true, false);
                    
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
                (pik_ptr->following_party == cur_leader_ptr && moving_group_intensity)
            ) {
                for(size_t m = 0; m < n_mobs; m++) {
                
                    mob* mob_ptr = mobs[m];
                    
                    if(mob_ptr->dead) continue;
                    if(!should_attack(pik_ptr, mob_ptr)) continue;
                    if(!check_dist(pik_ptr->x, pik_ptr->y, mob_ptr->x, mob_ptr->y, pik_ptr->type->radius + mob_ptr->type->radius + task_range)) continue;
                    
                    hitbox_instance* closest_hitbox = get_closest_hitbox(pik_ptr->x, pik_ptr->y, mob_ptr);
                    if(!closest_hitbox) continue;
                    pik_ptr->enemy_hitbox_nr = closest_hitbox->hitbox_nr;
                    
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
                (pik_ptr->following_party == cur_leader_ptr && moving_group_intensity)
            ) {
                for(size_t m = 0; m < n_mobs; m++) {
                
                    mob* mob_ptr = mobs[m];
                    
                    if(!mob_ptr->carrier_info) continue;
                    if(mob_ptr->carrier_info->current_n_carriers == mob_ptr->carrier_info->max_carriers) continue; //No more room.
                    if(mob_ptr->state == MOB_STATE_BEING_DELIVERED) continue;
                    
                    if(check_dist(pik_ptr->x, pik_ptr->y, mob_ptr->x, mob_ptr->y, pik_ptr->type->radius + mob_ptr->type->radius + task_range)) {
                        pik_ptr->wants_to_carry = mob_ptr;
                        remove_from_party(pik_ptr);
                        pik_ptr->set_state(PIKMIN_STATE_MOVING_TO_CARRY_SPOT);
                        
                        //ToDo remove this random cycle and replace with something more optimal.
                        bool valid_spot = false;
                        unsigned int spot = 0;
                        while(!valid_spot) {
                            spot = randomi(0, mob_ptr->carrier_info->max_carriers - 1);
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
                        true, &pik_ptr->carrying_mob->z
                    );
                    
                    pik_ptr->carrying_mob->carrier_info->current_carrying_strength += pik_ptr->pik_type->carry_strength;
                    
                    if(pik_ptr->carrying_mob->carrier_info->current_carrying_strength >= pik_ptr->carrying_mob->type->weight) {
                        start_carrying(pik_ptr->carrying_mob, pik_ptr, NULL);
                    }
                    
                    pik_ptr->unwhistlable_period = 0;
                    sfx_pikmin_carrying_grab.play(0.03, false);
                }
            }
            
            //Fighting an enemy.
            if(pik_ptr->attacking_mob) {
                hitbox_instance* h_ptr = get_hitbox_instance(pik_ptr->attacking_mob, pik_ptr->enemy_hitbox_nr);
                if(h_ptr) {
                    float actual_hx, actual_hy;
                    rotate_point(h_ptr->x, h_ptr->y, pik_ptr->attacking_mob->angle, &actual_hx, &actual_hy);
                    actual_hx += pik_ptr->attacking_mob->x; actual_hy += pik_ptr->attacking_mob->y;
                    
                    if(pik_ptr->latched || pik_ptr->being_chomped) {
                        float final_px, final_py;
                        angle_to_coordinates(
                            pik_ptr->enemy_hitbox_angle + pik_ptr->attacking_mob->angle,
                            pik_ptr->enemy_hitbox_dist * h_ptr->radius,
                            &final_px, &final_py);
                        final_px += actual_hx; final_py += actual_hy;
                        
                        pik_ptr->set_target(final_px, final_py, NULL, NULL, true);
                        pik_ptr->face(atan2(pik_ptr->attacking_mob->y - pik_ptr->y, pik_ptr->attacking_mob->x - pik_ptr->x));
                        if(pik_ptr->attack_time == 0) pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
                        pik_ptr->anim.change(PIKMIN_ANIM_ATTACK, true, true, false);
                        
                    } else {
                        if(check_dist(pik_ptr->x, pik_ptr->y, actual_hx, actual_hy, pik_ptr->type->radius + h_ptr->radius + PIKMIN_MIN_ATTACK_RANGE)) {
                            pik_ptr->remove_target(true);
                            pik_ptr->face(atan2(actual_hy - pik_ptr->y, actual_hx - pik_ptr->x));
                            if(pik_ptr->attack_time == 0) pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
                        } else {
                            pik_ptr->set_target(actual_hx, actual_hy, NULL, NULL, false);
                            pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
                        }
                    }
                }
                
                if(!pik_ptr->being_chomped) pik_ptr->attack_time -= delta_t* N_PIKMIN_AI_PORTIONS;
                if(pik_ptr->attack_time <= 0 && pik_ptr->knockdown_period == 0) {
                    pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
                    attack(pik_ptr, pik_ptr->attacking_mob, true, pik_ptr->pik_type->attack_power, 0, 0, 0, 0);
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
                    pik_ptr->being_chomped = false;
                }
            }
            
            //Touching a mob's hitboxes.
            for(size_t m = 0; m < mobs.size(); m++) {
                if(typeid(*mobs[m]) == typeid(pikmin)) continue;
                mob* m_ptr = mobs[m];
                frame* f_ptr = m_ptr->anim.get_frame();
                if(f_ptr == NULL) continue;
                
                for(size_t h = 0; h < f_ptr->hitbox_instances.size(); h++) {
                    hitbox_instance* hi_ptr = &f_ptr->hitbox_instances[h];
                    float s = sin(m_ptr->angle);
                    float c = cos(m_ptr->angle);
                    float h_x = m_ptr->x + (hi_ptr->x * c - hi_ptr->y * s);
                    float h_y = m_ptr->y + (hi_ptr->x * s + hi_ptr->y * c);
                    
                    if(check_dist(pik_ptr->x, pik_ptr->y, h_x, h_y, pik_ptr->type->radius + hi_ptr->radius)) {
                        hitbox* h_ptr = hi_ptr->hitbox_ptr;
                        size_t h_nr = hi_ptr->hitbox_nr;
                        
                        if(h_ptr->type == HITBOX_TYPE_ATTACK) {
                            float knockback_angle;
                            if(h_ptr->knockback_outward) {
                                knockback_angle = atan2(pik_ptr->y - h_y, pik_ptr->x - h_x);
                            } else {
                                knockback_angle = h_ptr->knockback_angle;
                            }
                            pik_ptr->latched = false;
                            pik_ptr->anim.change(PIKMIN_ANIM_LYING, true, false, false);
                            attack(m_ptr, pik_ptr, false, h_ptr->multiplier, knockback_angle, h_ptr->knockback, 1, 1);
                        }
                        
                        if(find(m_ptr->chomp_hitboxes.begin(), m_ptr->chomp_hitboxes.end(), hi_ptr->hitbox_nr) != m_ptr->chomp_hitboxes.end() && !pik_ptr->being_chomped) {
                            if(m_ptr->chomping_pikmin.size() >= m_ptr->type->chomp_max_victims) continue;
                            
                            float x_dif = pik_ptr->x - h_x;
                            float y_dif = pik_ptr->y - h_y;
                            coordinates_to_angle(x_dif, y_dif, &pik_ptr->enemy_hitbox_angle, &pik_ptr->enemy_hitbox_dist);
                            pik_ptr->enemy_hitbox_angle -= m_ptr->angle;  //Relative to 0 degrees.
                            pik_ptr->enemy_hitbox_dist /= hi_ptr->radius; //Distance in units to distance in percentage.
                            
                            pik_ptr->latched = false;
                            pik_ptr->enemy_hitbox_nr = h_nr;
                            pik_ptr->attacking_mob = m_ptr;
                            pik_ptr->being_chomped = true;
                            sfx_pikmin_caught.play(0.06, false);
                            
                            m_ptr->events_queued[MOB_EVENT_ATTACK_HIT] = 1;
                            m_ptr->events_queued[MOB_EVENT_ATTACK_MISS] = 0;
                            m_ptr->chomping_pikmin.push_back(pik_ptr);
                            focus_mob(m_ptr, pik_ptr, true, false);
                        }
                    }
                }
            }
            
            if(pik_ptr->carrying_mob) {
                pik_ptr->face(atan2(pik_ptr->carrying_mob->y - pik_ptr->y, pik_ptr->carrying_mob->x - pik_ptr->x));
            }
            
            if(pik_ptr->anim.is_anim(PIKMIN_ANIM_LYING, true) && pik_ptr->knockdown_period == 0) {
                pik_ptr->anim.change(PIKMIN_ANIM_GET_UP, true, false, false); //ToDo this isn't working. This instruction runs, but the animation never changes.
            } else if(pik_ptr->state == PIKMIN_STATE_BURIED) {
                pik_ptr->anim.change(PIKMIN_ANIM_BURROWED, true, true, true);
            } else if(pik_ptr->being_chomped) {
                pik_ptr->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
            } else if(pik_ptr->speed_z == 0 && pik_ptr->attack_time == 0) {
                if(cur_leader_ptr->holding_pikmin != pik_ptr && (pik_ptr->speed_x != 0 || pik_ptr->speed_y != 0)) {
                    pik_ptr->anim.change(PIKMIN_ANIM_WALK, true, true, true);
                } else {
                    pik_ptr->anim.change(PIKMIN_ANIM_IDLE, true, true, true);
                }
            }
            
        }
        
        
        /********************
        *              .-.  *
        *   Leaders   (*:O) *
        *              `-�  *
        ********************/
        
        if(cur_leader_ptr->holding_pikmin) {
            cur_leader_ptr->holding_pikmin->x = cur_leader_ptr->x + cos(cur_leader_ptr->angle + M_PI) * cur_leader_ptr->type->radius;
            cur_leader_ptr->holding_pikmin->y = cur_leader_ptr->y + sin(cur_leader_ptr->angle + M_PI) * cur_leader_ptr->type->radius;
            cur_leader_ptr->holding_pikmin->angle = cur_leader_ptr->angle;
        }
        
        //Current leader movement.
        if(!cur_leader_ptr->auto_pluck_mode && !cur_leader_ptr->auto_pluck_pikmin && !cur_leader_ptr->carrier_info) {
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
            leader* l_ptr = leaders[l];
            if(whistling) {
                if(l != cur_leader_nr) {
                    if(check_dist(l_ptr->x, l_ptr->y, cursor_x, cursor_y, whistle_radius)) {
                    
                        stop_auto_pluck(l_ptr);
                        
                        if(!l_ptr->following_party && !l_ptr->was_thrown) {
                            //Leader got whistled.
                            add_to_party(cur_leader_ptr, l_ptr);
                            
                            size_t n_party_members = l_ptr->party->members.size();
                            for(size_t m = 0; m < n_party_members; m++) {
                                mob* member = l_ptr->party->members[0];
                                remove_from_party(member);
                                add_to_party(cur_leader_ptr, member);
                            }
                        }
                    }
                }
            }
            
            if(l_ptr->following_party && !l_ptr->auto_pluck_mode) {
                l_ptr->set_target(
                    0,
                    0,
                    &l_ptr->following_party->party->party_center_x,
                    &l_ptr->following_party->party->party_center_y,
                    false);
                    
            } else {
            
                if(l_ptr->auto_pluck_pikmin && l_ptr->reached_destination) {
                
                    if(l_ptr->pluck_time == -1) {
                        l_ptr->pluck_time = l_ptr->lea_type->pluck_delay;
                        l_ptr->anim.change(LEADER_ANIM_PLUCK, true, false, false);
                    }
                    
                    if(l_ptr->pluck_time > 0) {
                        l_ptr->pluck_time -= delta_t;
                        
                    } else {
                    
                        leader* new_pikmin_leader = l_ptr;
                        if(l_ptr->following_party) {
                            if(typeid(*l_ptr->following_party) == typeid(leader)) {
                                //If this leader is following another one, then the new Pikmin should be in the party of that top leader.
                                new_pikmin_leader = (leader*) l_ptr->following_party;
                            }
                        }
                        
                        pluck_pikmin(new_pikmin_leader, l_ptr->auto_pluck_pikmin, l_ptr);
                        l_ptr->auto_pluck_pikmin = NULL;
                    }
                }
                
                if(l_ptr->auto_pluck_mode) {
                    if(!l_ptr->auto_pluck_pikmin) {
                        float d;
                        pikmin* new_pikmin = get_closest_buried_pikmin(l_ptr->x, l_ptr->y, &d, true);
                        
                        if(new_pikmin && d <= AUTO_PLUCK_MAX_RADIUS) {
                            go_pluck(l_ptr, new_pikmin);
                        } else { //No more buried Pikmin, or none nearby. Give up.
                            stop_auto_pluck(l_ptr);
                        }
                    }
                }
            }
            
            if(!l_ptr->carrier_info && !whistling) {
                if(l_ptr->speed_x != 0 || l_ptr->speed_y != 0) {
                    l_ptr->anim.change(LEADER_ANIM_WALK, true, true, true);
                } else {
                    l_ptr->anim.change(LEADER_ANIM_IDLE, true, true, true);
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
        *             `-�   *
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
        
        if(!cur_leader_ptr->auto_pluck_mode && cur_leader_ptr->pluck_time == -1 && !cur_leader_ptr->carrier_info) {
            cursor_angle = atan2(cursor_y - cur_leader_ptr->y, cursor_x - cur_leader_ptr->x);
            cur_leader_ptr->face(cursor_angle);
        }
        
        leader_to_cursor_dis = dist(cur_leader_ptr->x, cur_leader_ptr->y, cursor_x, cursor_y);
        if(leader_to_cursor_dis > CURSOR_MAX_DIST) {
            //ToDo with an analog stick, if the cursor is being moved, it's considered off-limit a lot more than it should.
            //Cursor goes beyond the range limit.
            cursor_x = cur_leader_ptr->x + (cos(cursor_angle) * CURSOR_MAX_DIST);
            cursor_y = cur_leader_ptr->y + (sin(cursor_angle) * CURSOR_MAX_DIST);
            
            if(mouse_cursor_speed_x != 0 || mouse_cursor_speed_y != 0) {
                //If we're speeding the mouse cursor (via analog stick), don't let it go beyond the edges.
                mouse_cursor_x = cursor_x;
                mouse_cursor_y = cursor_y;
                al_transform_coordinates(&world_to_screen_transform, &mouse_cursor_x, &mouse_cursor_y);
            }
        }
        
        
        /**************************
        *                    /  / *
        *   Percipitation     / / *
        *                   /  /  *
        **************************/
        
        if(cur_area_map.weather_condition.percipitation_type != PERCIPITATION_TYPE_NONE) {
            percipitation_time_left -= delta_t;
            if(percipitation_time_left <= 0) {
                percipitation_time_left = cur_area_map.weather_condition.percipitation_frequency.get_random_number();
                percipitation.push_back(point(0, 0));
            }
            
            for(size_t p = 0; p < percipitation.size();) {
                percipitation[p].y += cur_area_map.weather_condition.percipitation_speed.get_random_number() * delta_t;
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
        
        throw_particle_timer -= delta_t;
        if(throw_particle_timer <= 0) {
            throw_particle_timer = THROW_PARTICLE_INTERVAL;
            
            for(size_t l = 0; l < n_leaders; l++) {
                if(leaders[l]->was_thrown)
                    particles.push_back(
                        particle(
                            PARTICLE_TYPE_CIRCLE, NULL, leaders[l]->x, leaders[l]->y, 0, 0, 0, 0, 0.6, leaders[l]->type->radius, change_alpha(leaders[l]->type->main_color, 128)
                        )
                    );
            }
            
            for(size_t p = 0; p < n_pikmin; p++) {
                if(pikmin_list[p]->was_thrown)
                    particles.push_back(
                        particle(
                            PARTICLE_TYPE_CIRCLE, NULL, pikmin_list[p]->x, pikmin_list[p]->y, 0, 0, 0, 0, 0.6, pikmin_list[p]->type->radius, change_alpha(pikmin_list[p]->type->main_color, 128)
                        )
                    );
            }
        }
        
    } else { //Displaying a message.
    
        if(cur_message_char < cur_message_stopping_chars[cur_message_section + 1]) {
            cur_message_char_time -= delta_t;
            if(cur_message_char_time <= 0) {
                cur_message_char_time = MESSAGE_CHAR_INTERVAL;
                cur_message_char++;
            }
        }
        
    }
}