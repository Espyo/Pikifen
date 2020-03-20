/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Logic about mob movement, gravity, wall collision, etc.
 */

#include "mobs/mob.h"

#include "vars.h"

const float MOB_PUSH_THROTTLE_TIMEOUT = 1.0f;

/* ----------------------------------------------------------------------------
 * Ticks the mob's actual physics procedures:
 * falling because of gravity, moving forward, etc.
 */
void mob::tick_physics(const float delta_t) {
    if(!ground_sector) {
        //Object is placed out of bounds.
        return;
    }
    
    //Initial setup.
    float move_speed_mult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); ++s) {
        move_speed_mult *= this->statuses[s].type->speed_multiplier;
    }
    
    point pre_move_pos = pos;
    point move_speed = speed;
    bool touched_wall = false;
    float pre_move_ground_z = ground_sector->z;
    
    //Rotation logic.
    tick_rotation_physics(delta_t, move_speed_mult);
    
    //What type of horizontal movement is this?
    H_MOVE_RESULTS h_mov_type =
        get_physics_horizontal_movement(delta_t, move_speed_mult, &move_speed);
        
    if(h_mov_type == H_MOVE_FAIL) {
        return;
    } else if (h_mov_type == H_MOVE_OK) {
        //Horizontal movement time!
        tick_horizontal_movement_physics(
            delta_t, move_speed, &touched_wall
        );
    }
    
    //Vertical movement.
    tick_vertical_movement_physics(delta_t, pre_move_ground_z);
    
    //Walk on top of another mob, if possible.
    tick_walkable_riding_physics(delta_t);
    
    //Final setup.
    push_amount = 0;
    
    if(touched_wall) {
        fsm.run_event(MOB_EV_TOUCHED_WALL);
    }
    
    if(type->walkable) {
        walkable_moved = (pos - pre_move_pos) / delta_t;
    }
}


/* ----------------------------------------------------------------------------
 * Returns which walkable mob this mob should be considered to be on top of.
 * Returns NULL if none is found.
 */
mob* mob::get_mob_to_walk_on() {
    for(size_t m = 0; m < mobs.size(); ++m) {
        mob* m_ptr = mobs[m];
        if(!m_ptr->type->walkable) {
            continue;
        }
        if(m_ptr == this) {
            continue;
        }
        if(z < m_ptr->z + m_ptr->height - SECTOR_STEP) {
            continue;
        }
        if(z > m_ptr->z + m_ptr->height) {
            continue;
        }
        
        //Check if they collide on X+Y.
        if(
            type->rectangular_dim.x != 0 &&
            m_ptr->type->rectangular_dim.x != 0
        ) {
            //Rectangle vs rectangle.
            //Not supported.
            continue;
        } else if(type->rectangular_dim.x != 0) {
            //Rectangle vs circle.
            if(
                !circle_intersects_rectangle(
                    m_ptr->pos, m_ptr->type->radius,
                    pos, type->rectangular_dim,
                    angle
                )
            ) {
                continue;
            }
        } else if(m_ptr->type->rectangular_dim.x != 0) {
            //Circle vs rectangle.
            if(
                !circle_intersects_rectangle(
                    pos, type->radius,
                    m_ptr->pos, m_ptr->type->rectangular_dim,
                    m_ptr->angle
                )
            ) {
                continue;
            }
        } else {
            //Circle vs circle.
            if(
                dist(pos, m_ptr->pos) >
                (type->radius + m_ptr->type->radius)
            ) {
                continue;
            }
        }
        return m_ptr;
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Calculates which edges the mob is intersecting with for horizontal movement
 * physics logic.
 * Returns H_MOVE_OK if everything is okay, H_MOVE_FAIL if movement is
 * impossible.
 * new_pos:            Position to check.
 * intersecting_edges: List of edges it is intersecting with.
 */
H_MOVE_RESULTS mob::get_movement_edge_intersections(
    const point &new_pos, vector<edge*>* intersecting_edges
) {
    //Before checking the edges, let's consult the blockmap and look at
    //the edges in the same blocks the mob is on.
    //This way, we won't check for edges that are really far away.
    //Use the bounding box to know which blockmap blocks the mob will be on.
    size_t bx1 = cur_area_data.bmap.get_col(new_pos.x - type->radius);
    size_t bx2 = cur_area_data.bmap.get_col(new_pos.x + type->radius);
    size_t by1 = cur_area_data.bmap.get_row(new_pos.y - type->radius);
    size_t by2 = cur_area_data.bmap.get_row(new_pos.y + type->radius);
    
    if(
        bx1 == INVALID || bx2 == INVALID ||
        by1 == INVALID || by2 == INVALID
    ) {
        //Somehow out of bounds. No movement.
        return H_MOVE_FAIL;
    }
    
    set<edge*> candidate_edges;
    
    //Go through the blocks, and get a list of all edges to check against.
    for(size_t bx = bx1; bx <= bx2; ++bx) {
        for(size_t by = by1; by <= by2; ++by) {
        
            vector<edge*>* edges = &cur_area_data.bmap.edges[bx][by];
            
            for(size_t e = 0; e < edges->size(); ++e) {
                candidate_edges.insert(edges->operator[](e));
            }
        }
    }
    
    //Go through each edge, and figure out if it is a valid wall for our mob.
    for(auto e_ptr : candidate_edges) {
    
        bool is_edge_blocking = false;
        
        if(
            !circle_intersects_line(
                new_pos, type->radius,
                point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
                point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y),
                NULL, NULL
            )
        ) {
            //No intersection? Well, obviously this one doesn't count.
            continue;
        }
        
        if(!e_ptr->sectors[0] || !e_ptr->sectors[1]) {
            //If we're on the edge of out-of-bounds geometry,
            //block entirely.
            return H_MOVE_FAIL;
        }
        
        for(unsigned char s = 0; s < 2; ++s) {
            if(e_ptr->sectors[s]->type == SECTOR_TYPE_BLOCKING) {
                is_edge_blocking = true;
                break;
            }
        }
        
        if(!is_edge_blocking) {
            if(e_ptr->sectors[0]->z == e_ptr->sectors[1]->z) {
                //No difference in floor height = no wall.
                //Ignore this.
                continue;
            }
            if(
                e_ptr->sectors[0]->z < z &&
                e_ptr->sectors[1]->z < z
            ) {
                //An edge whose sectors are below the mob?
                //No collision here.
                continue;
            }
        }
        
        if(
            e_ptr->sectors[0]->z > z &&
            e_ptr->sectors[1]->z > z
        ) {
            //If both floors of this edge are above the mob...
            //then what does that mean? That the mob is under the ground?
            //Nonsense! Throw this edge away!
            //It's a false positive, and it's likely behind a more logical
            //edge that we actually did collide against.
            continue;
        }
        
        if(
            e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING &&
            e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
        ) {
            //Same logic as the previous check.
            continue;
        }
        
        //Add this edge to the list of intersections, then.
        intersecting_edges->push_back(e_ptr);
    }
    
    return H_MOVE_OK;
}


//If a mob is this close to the destination, it can move without tank controls.
const float FREE_MOVE_THRESHOLD = 10.0f;

/* ----------------------------------------------------------------------------
 * Calculates how much the mob is going to move horizontally, for the purposes
 * of movement physics calculation.
 * Returns H_MOVE_OK on normal movement, H_MOVE_TELEPORTED if the mob's X
 * and Y have been set and movement logic can be skipped, and H_MOVE_FAIL if
 * movement is entirely impossible this frame.
 * delta_t:         How many seconds to tick the logic by.
 * move_speed_mult: Movement speed is multiplied by this.
 * move_speed:      The calculated move speed is placed in this struct.
 */
H_MOVE_RESULTS mob::get_physics_horizontal_movement(
    const float delta_t, const float move_speed_mult, point* move_speed
) {
    //Held by another mob.
    if(holder.m) {
        point final_pos = holder.get_final_pos(&z);
        z += 1.0f; //Added visibility for latched Pikmin.
        speed_z = 0;
        chase(final_pos, NULL, true);
    }
    
    //Chasing.
    if(chase_info.is_chasing) {
        point final_target_pos = get_chase_target();
        
        if(chase_info.teleport) {
            sector* sec =
                get_sector(final_target_pos, NULL, true);
                
            if(!sec) {
                //No sector, invalid teleport. No move.
                return H_MOVE_FAIL;
            }
            
            if(chase_info.teleport_z) {
                z = *chase_info.teleport_z;
            }
            ground_sector = sec;
            center_sector = sec;
            speed.x = speed.y = 0;
            pos = final_target_pos;
            return H_MOVE_TELEPORTED;
            
        } else {
        
            //Make it go to the direction it wants.
            float d = dist(pos, final_target_pos).to_float();
            
            float move_amount =
                min(
                    (double) (d / delta_t),
                    (double) chase_info.speed * move_speed_mult
                );
                
            bool can_free_move =
                chase_info.free_move || d <= FREE_MOVE_THRESHOLD;
                
            float movement_angle =
                can_free_move ?
                get_angle(pos, final_target_pos) :
                angle;
                
            move_speed->x = cos(movement_angle) * move_amount;
            move_speed->y = sin(movement_angle) * move_amount;
        }
        
    }
    
    //If another mob is pushing it.
    if(push_amount != 0.0f) {
        //Overly-aggressive pushing results in going through walls.
        //Let's place a cap.
        push_amount =
            min(push_amount, (float) (type->radius / delta_t) * 4);
            
        //If the mob spawned recently, throttle its push. This avoids a bundle
        //of recently-spawned objects from pushing each other with insane force.
        //Setting the amount to 0 means it'll use the push provided by
        //MOB_PUSH_EXTRA_AMOUNT exclusively.
        if(time_alive < MOB_PUSH_THROTTLE_TIMEOUT) {
            push_amount = 0;
        }
        
        move_speed->x +=
            cos(push_angle) * (push_amount + MOB_PUSH_EXTRA_AMOUNT);
        move_speed->y +=
            sin(push_angle) * (push_amount + MOB_PUSH_EXTRA_AMOUNT);
    }
    
    //Scrolling floors.
    if(
        (ground_sector->scroll.x != 0 || ground_sector->scroll.y != 0) &&
        z <= ground_sector->z
    ) {
        (*move_speed) += ground_sector->scroll;
    }
    
    //On top of a mob.
    if(standing_on_mob) {
        (*move_speed) += standing_on_mob->walkable_moved;
    }
    
    return H_MOVE_OK;
}


/* ----------------------------------------------------------------------------
 * Calculates the angle at which the mob should slide against this wall,
 * for the purposes of movement physics calculations.
 * Returns H_MOVE_OK on success, H_MOVE_FAIL if the mob can't
 * slide against this wall.
 * e_ptr:       Pointer to the edge in question.
 * wall_sector: Number of the sector that actually makes a wall (i.e. the
 *   highest).
 * move_angle:  Angle at which the mob is going to move.
 * slide_angle: Holds the calculated slide angle.
 */
H_MOVE_RESULTS mob::get_wall_slide_angle(
    edge* e_ptr, unsigned char wall_sector, const float move_angle,
    float* slide_angle
) {
    //The wall's normal is the direction the wall is facing.
    //i.e. the direction from the top floor to the bottom floor.
    //We know which side of an edge is which sector because of
    //the vertexes. Imagine you're in first person view,
    //following the edge as a line on the ground.
    //You start on vertex 0 and face vertex 1.
    //Sector 0 will always be on your left.
    
    float wall_normal;
    float wall_angle =
        get_angle(
            point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
            point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y)
        );
        
    if(wall_sector == 0) {
        wall_normal = normalize_angle(wall_angle + TAU / 4);
    } else {
        wall_normal = normalize_angle(wall_angle - TAU / 4);
    }
    
    float nd = get_angle_cw_dif(wall_normal, move_angle);
    if(nd < TAU * 0.25 || nd > TAU * 0.75) {
        //If the difference between the movement and the wall's
        //normal is this, that means we came FROM the wall.
        //No way! There has to be an edge that makes more sense.
        return H_MOVE_FAIL;
    }
    
    //If we were to slide on this edge, this would be
    //the slide angle.
    float tentative_slide_angle;
    if(nd < TAU / 2) {
        //Coming in from the "left" of the normal. Slide right.
        *slide_angle = wall_normal + TAU / 4;
    } else {
        //Coming in from the "right" of the normal. Slide left.
        *slide_angle = wall_normal - TAU / 4;
    }
    
    return H_MOVE_OK;
}


/* ----------------------------------------------------------------------------
 * Ticks physics logic regarding the mob's horizontal movement.
 * delta_t:              How many seconds to tick the logic by.
 * attempted_move_speed: Movement speed to calculate with.
 * touched_wall:         Holds whether or not the mob touched a wall in this
 *   move.
 */
void mob::tick_horizontal_movement_physics(
    const float delta_t, const point &attempted_move_speed,
    bool* touched_wall
) {
    if(attempted_move_speed.x == 0 && attempted_move_speed.y == 0) {
        //No movement. Nothing to do here.
        return;
    }
    
    //Setup.
    bool finished_moving = false;
    bool doing_slide = false;
    
    point new_pos = pos;
    float new_z = z;
    sector* new_ground_sector = ground_sector;
    
    point move_speed = attempted_move_speed;
    
    //Try placing it in the place it should be at, judging
    //from the movement speed.
    while(!finished_moving) {
    
        //Start by checking sector collisions.
        //For this, we will only check if the mob is intersecting
        //with any edge. With this, we trust that mobs can't go so fast
        //that they're fully on one side of an edge in one frame,
        //and the other side on the next frame.
        //It's pretty naive...but it works!
        bool successful_move = true;
        
        new_pos.x = pos.x + delta_t* move_speed.x;
        new_pos.y = pos.y + delta_t* move_speed.y;
        new_z = z;
        new_ground_sector = ground_sector;
        
        //Get the sector the mob will be on.
        sector* new_center_sector = get_sector(new_pos, NULL, true);
        sector* step_sector = new_center_sector;
        
        if(!new_center_sector) {
            //Out of bounds. No movement.
            return;
        } else {
            new_ground_sector = new_center_sector;
        }
        
        if(z < new_center_sector->z) {
            //If it'd end up under the ground, refuse the move.
            return;
        }
        
        //Get all edges it collides against in this new position.
        vector<edge*> intersecting_edges;
        if(
            get_movement_edge_intersections(new_pos, &intersecting_edges) ==
            H_MOVE_FAIL
        ) {
            return;
        }
        
        //For every sector in the new position, let's figure out
        //the ground sector, and also a stepping sector, if possible.
        for(size_t e = 0; e < intersecting_edges.size(); ++e) {
            edge* e_ptr = intersecting_edges[e];
            sector* tallest_sector = ground_sector; //Tallest of the two.
            if(
                e_ptr->sectors[0]->type != SECTOR_TYPE_BLOCKING &&
                e_ptr->sectors[1]->type != SECTOR_TYPE_BLOCKING
            ) {
                if(e_ptr->sectors[0]->z > e_ptr->sectors[1]->z) {
                    tallest_sector = e_ptr->sectors[0];
                } else {
                    tallest_sector = e_ptr->sectors[1];
                }
            }
            
            if(
                tallest_sector->z > new_ground_sector->z &&
                tallest_sector->z <= z
            ) {
                new_ground_sector = tallest_sector;
            }
            
            //Check if it can go up this step.
            //It can go up this step if the floor is within
            //stepping distance of the mob's current Z,
            //and if this step is larger than any step
            //encountered of all edges crossed.
            if(
                !was_thrown &&
                tallest_sector->z <= z + SECTOR_STEP &&
                tallest_sector->z > step_sector->z
            ) {
                step_sector = tallest_sector;
            }
        }
        
        //Mosey on up to the step sector, if any.
        if(step_sector->z > new_ground_sector->z) {
            new_ground_sector = step_sector;
        }
        if(z < step_sector->z) new_z = step_sector->z;
        
        //Figure out sliding logic now, if needed.
        float move_angle;
        float total_move_speed;
        coordinates_to_angle(
            move_speed, &move_angle, &total_move_speed
        );
        
        //Angle to slide towards.
        float slide_angle = move_angle;
        //Difference between the movement angle and the slide.
        float slide_angle_dif = 0;
        
        //Check the sector heights of the intersecting edges to figure out
        //which are really walls, and how to slide against them.
        for(size_t e = 0; e < intersecting_edges.size(); ++e) {
            edge* e_ptr = intersecting_edges[e];
            bool is_edge_wall = false;
            unsigned char wall_sector = 0;
            
            for(unsigned char s = 0; s < 2; s++) {
                if(e_ptr->sectors[s]->type == SECTOR_TYPE_BLOCKING) {
                    is_edge_wall = true;
                    wall_sector = s;
                }
            }
            
            if(!is_edge_wall) {
                for(unsigned char s = 0; s < 2; s++) {
                    if(e_ptr->sectors[s]->z > new_z) {
                        is_edge_wall = true;
                        wall_sector = s;
                    }
                }
            }
            
            //This isn't a wall... Get out of here, faker.
            if(!is_edge_wall) continue;
            
            //Ok, there's obviously been a collision, so let's work out what
            //wall the mob will slide on.
            
            if(!doing_slide) {
                float tentative_slide_angle;
                if(
                    get_wall_slide_angle(
                        e_ptr, wall_sector, move_angle, &tentative_slide_angle
                    ) == H_MOVE_FAIL
                ) {
                    continue;
                }
                
                float sd =
                    get_angle_smallest_dif(move_angle, tentative_slide_angle);
                if(sd > slide_angle_dif) {
                    slide_angle_dif = sd;
                    slide_angle = tentative_slide_angle;
                }
            }
            
            //By the way, if we got to this point, that means there are real
            //collisions happening. Let's mark this move as unsuccessful.
            successful_move = false;
            (*touched_wall) = true;
        }
        
        //If the mob is just slamming against the wall head-on, perpendicularly,
        //then forget any idea about sliding.
        //It'd just be awkwardly walking in place.
        //Reset its horizontal position, but keep calculations for
        //everything else.
        if(!successful_move && slide_angle_dif > TAU / 4 - 0.05) {
            new_pos = pos;
            successful_move = true;
        }
        
        
        //We're done checking. If the move was unobstructed, good, go there.
        //If not, we'll use the info we gathered before to calculate sliding,
        //and try again.
        
        if(successful_move) {
            //Good news, the mob can be placed in this new spot freely.
            pos = new_pos;
            z = new_z;
            ground_sector = new_ground_sector;
            center_sector = new_center_sector;
            finished_moving = true;
            
        } else {
        
            //Try sliding.
            if(doing_slide) {
                //We already tried sliding, and we still hit something...
                //Let's just stop completely. This mob can't go forward.
                finished_moving = true;
                
            } else {
                doing_slide = true;
                //To limit the speed, we should use a cross-product of the
                //movement and slide vectors.
                //But nuts to that, this is just as nice, and a lot simpler!
                total_move_speed *= 1 - (slide_angle_dif / TAU / 2);
                move_speed =
                    angle_to_coordinates(slide_angle, total_move_speed);
                    
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Ticks physics logic regarding the mob rotating.
 * delta_t:         How many seconds to tick the logic by.
 * move_speed_mult: Movement speed is multiplied by this.
 */
void mob::tick_rotation_physics(
    const float delta_t, const float move_speed_mult
) {
    //Change the facing angle to the angle the mob wants to face.
    if(angle > TAU / 2)  angle -= TAU;
    if(angle < -TAU / 2) angle += TAU;
    if(intended_turn_pos) {
        intended_turn_angle = get_angle(pos, *intended_turn_pos);
    }
    if(intended_turn_angle > TAU / 2)  intended_turn_angle -= TAU;
    if(intended_turn_angle < -TAU / 2) intended_turn_angle += TAU;
    
    float angle_dif = intended_turn_angle - angle;
    if(angle_dif > TAU / 2)  angle_dif -= TAU;
    if(angle_dif < -TAU / 2) angle_dif += TAU;
    
    angle +=
        sign(angle_dif) * min(
            (double) (type->rotation_speed * move_speed_mult * delta_t),
            (double) fabs(angle_dif)
        );
        
    if(holder.m) {
        if(holder.rotation_method == HOLD_ROTATION_METHOD_FACE_HOLDER) {
            float dummy;
            point final_pos = holder.get_final_pos(&dummy);
            angle = get_angle(final_pos, holder.m->pos);
            stop_turning();
        } else if(holder.rotation_method == HOLD_ROTATION_METHOD_COPY_HOLDER) {
            angle = holder.m->angle;
            stop_turning();
        }
    }
    
    angle_cos = cos(angle);
    angle_sin = sin(angle);
}


/* ----------------------------------------------------------------------------
 * Ticks physics logic regarding the mob's vertical movement.
 * delta_t:           How many seconds to tick the logic by.
 * pre_move_ground_z: Z of the floor before horizontal movement started.
 */
void mob::tick_vertical_movement_physics(
    const float delta_t, const float pre_move_ground_z
) {
    z += delta_t* speed_z;
    
    if(!standing_on_mob) {
        //If the current ground is one step (or less) below
        //the previous ground, just instantly go down the step.
        if(
            pre_move_ground_z - ground_sector->z <= SECTOR_STEP &&
            z == pre_move_ground_z
        ) {
            z = ground_sector->z;
        }
    }
    
    //Gravity.
    speed_z += delta_t* gravity_mult * GRAVITY_ADDER;
    
    //Landing.
    hazard* new_on_hazard = NULL;
    if(speed_z <= 0) {
        if(standing_on_mob) {
            z = standing_on_mob->z + standing_on_mob->height;
            speed_z = 0;
            was_thrown = false;
            fsm.run_event(MOB_EV_LANDED);
            stop_height_effect();
            
        } else if(z <= ground_sector->z) {
            z = ground_sector->z;
            speed_z = 0;
            was_thrown = false;
            fsm.run_event(MOB_EV_LANDED);
            stop_height_effect();
            
            if(ground_sector->is_bottomless_pit) {
                fsm.run_event(MOB_EV_BOTTOMLESS_PIT);
            }
            
            for(size_t h = 0; h < ground_sector->hazards.size(); ++h) {
                fsm.run_event(
                    MOB_EV_TOUCHED_HAZARD,
                    (void*) ground_sector->hazards[h]
                );
                new_on_hazard = ground_sector->hazards[h];
            }
        }
    }
    
    //Due to framerate imperfections, thrown Pikmin/leaders can reach higher
    //than intended. z_cap forces a cap. FLT_MAX = no cap.
    if(speed_z <= 0) {
        z_cap = FLT_MAX;
    } else if(z_cap < FLT_MAX) {
        z = min(z, z_cap);
    }
    
    //On a sector that has a hazard that is not on the floor.
    if(z > ground_sector->z && !ground_sector->hazard_floor) {
        for(size_t h = 0; h < ground_sector->hazards.size(); ++h) {
            fsm.run_event(
                MOB_EV_TOUCHED_HAZARD,
                (void*) ground_sector->hazards[h]
            );
            new_on_hazard = ground_sector->hazards[h];
        }
    }
    
    if(new_on_hazard != on_hazard && on_hazard != NULL) {
        fsm.run_event(
            MOB_EV_LEFT_HAZARD,
            (void*) on_hazard
        );
    }
    on_hazard = new_on_hazard;
    
    //Quick panic check: if it's somehow inside the ground, pop it out.
    z = max(z, ground_sector->z);
}


/* ----------------------------------------------------------------------------
 * Ticks physics logic regarding landing on top of a walkable mob.
 * delta_t: How many seconds to tick the logic by.
 */
void mob::tick_walkable_riding_physics(const float delta_t) {
    mob* rider_added_ev_mob = NULL;
    mob* rider_removed_ev_mob = NULL;
    mob* new_standing_on_mob = NULL;
    
    //Check which mob it is on top of, if any.
    new_standing_on_mob = get_mob_to_walk_on();
    
    if(new_standing_on_mob) {
        z = new_standing_on_mob->z + new_standing_on_mob->height;
    }
    
    if(new_standing_on_mob != standing_on_mob) {
        if(standing_on_mob) {
            rider_removed_ev_mob = standing_on_mob;
        }
        if(new_standing_on_mob) {
            rider_added_ev_mob = new_standing_on_mob;
        }
    }
    
    standing_on_mob = new_standing_on_mob;
    
    if(rider_removed_ev_mob) {
        rider_removed_ev_mob->fsm.run_event(
            MOB_EV_RIDER_REMOVED, (void*) this
        );
        if(type->weight != 0.0f) {
            rider_removed_ev_mob->fsm.run_event(
                MOB_EV_WEIGHT_REMOVED, (void*) this
            );
        }
    }
    if(rider_added_ev_mob) {
        rider_added_ev_mob->fsm.run_event(
            MOB_EV_RIDER_ADDED, (void*) this
        );
        if(type->weight != 0.0f) {
            rider_added_ev_mob->fsm.run_event(
                MOB_EV_WEIGHT_ADDED, (void*) this
            );
        }
    }
}
