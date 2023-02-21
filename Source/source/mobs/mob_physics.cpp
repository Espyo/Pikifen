/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Logic about mob movement, gravity, wall collision, etc.
 */

#include <algorithm>

#include "mob.h"

#include "../functions.h"
#include "../game.h"


using std::set;


/* ----------------------------------------------------------------------------
 * Returns which walkable mob this mob should be considered to be on top of.
 * Returns NULL if none is found.
 */
mob* mob::get_mob_to_walk_on() const {
    //Can't walk on anything if it's moving upwards.
    if(speed_z > 0.0f) return NULL;
    
    mob* best_candidate = NULL;
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
        mob* m_ptr = game.states.gameplay->mobs.all[m];
        if(!m_ptr->type->walkable) {
            continue;
        }
        if(m_ptr == this) {
            continue;
        }
        if(fabs(z - (m_ptr->z + m_ptr->height)) > GEOMETRY::STEP_HEIGHT) {
            continue;
        }
        if(best_candidate && m_ptr->z <= best_candidate->z) {
            continue;
        }
        
        //Check if they collide on X+Y.
        if(
            rectangular_dim.x != 0 &&
            m_ptr->rectangular_dim.x != 0
        ) {
            //Rectangle vs rectangle.
            if(
                !rectangles_intersect(
                    pos, rectangular_dim, angle,
                    m_ptr->pos, m_ptr->rectangular_dim, m_ptr->angle
                )
            ) {
                continue;
            }
        } else if(rectangular_dim.x != 0) {
            //Rectangle vs circle.
            if(
                !circle_intersects_rectangle(
                    m_ptr->pos, m_ptr->radius,
                    pos, rectangular_dim,
                    angle
                )
            ) {
                continue;
            }
        } else if(m_ptr->rectangular_dim.x != 0) {
            //Circle vs rectangle.
            if(
                !circle_intersects_rectangle(
                    pos, radius,
                    m_ptr->pos, m_ptr->rectangular_dim,
                    m_ptr->angle
                )
            ) {
                continue;
            }
        } else {
            //Circle vs circle.
            if(
                dist(pos, m_ptr->pos) >
                (radius + m_ptr->radius)
            ) {
                continue;
            }
        }
        best_candidate = m_ptr;
    }
    return best_candidate;
}


/* ----------------------------------------------------------------------------
 * Calculates which edges the mob is intersecting with for horizontal movement
 * physics logic.
 * Returns H_MOVE_OK if everything is okay, H_MOVE_FAIL if movement is
 * impossible.
 * new_pos:
 *   Position to check.
 * intersecting_edges:
 *   List of edges it is intersecting with.
 */
H_MOVE_RESULTS mob::get_movement_edge_intersections(
    const point &new_pos, vector<edge*>* intersecting_edges
) const {
    //Before checking the edges, let's consult the blockmap and look at
    //the edges in the same blocks the mob is on.
    //This way, we won't check for edges that are really far away.
    //Use the bounding box to know which blockmap blocks the mob will be on.
    set<edge*> candidate_edges;
    //Use the terrain radius if the mob is moving about and alive.
    //Otherwise if it's a corpse, it can use the regular radius.
    float radius_to_use =
        (type->terrain_radius < 0 || health <= 0) ?
        radius :
        type->terrain_radius;
        
    if(
        !game.cur_area_data.bmap.get_edges_in_region(
            new_pos - radius_to_use,
            new_pos + radius_to_use,
            candidate_edges
        )
    ) {
        //Somehow out of bounds. No movement.
        return H_MOVE_FAIL;
    }
    
    //Go through each edge, and figure out if it is a valid wall for our mob.
    for(auto e_ptr : candidate_edges) {
    
        bool is_edge_blocking = false;
        
        if(
            !circle_intersects_line_seg(
                new_pos, radius_to_use,
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


/* ----------------------------------------------------------------------------
 * Calculates how much the mob is going to move horizontally, for the purposes
 * of movement physics calculation.
 * Returns H_MOVE_OK on normal movement, H_MOVE_TELEPORTED if the mob's X
 * and Y have been set and movement logic can be skipped, and H_MOVE_FAIL if
 * movement is entirely impossible this frame.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 * move_speed_mult:
 *   Movement speed is multiplied by this.
 * move_speed:
 *   The calculated move speed is placed in this struct.
 */
H_MOVE_RESULTS mob::get_physics_horizontal_movement(
    const float delta_t, const float move_speed_mult, point* move_speed
) {
    //Held by another mob.
    if(holder.m) {
        point final_pos = holder.get_final_pos(&z);
        speed_z = 0;
        chase(final_pos, z, CHASE_FLAG_TELEPORT);
    }
    
    //Chasing.
    if(chase_info.state == CHASE_STATE_CHASING) {
        point final_target_pos = get_chase_target();
        
        if(has_flag(chase_info.flags, CHASE_FLAG_TELEPORT)) {
        
            sector* sec =
                get_sector(final_target_pos, NULL, true);
                
            if(!sec) {
                //No sector, invalid teleport. No move.
                return H_MOVE_FAIL;
            }
            
            z = chase_info.offset_z;
            if(chase_info.orig_z) {
                z += *chase_info.orig_z;
            }
            
            ground_sector = sec;
            center_sector = sec;
            speed.x = speed.y = 0;
            pos = final_target_pos;
            
            if(!has_flag(chase_info.flags, CHASE_FLAG_TELEPORTS_CONSTANTLY)) {
                chase_info.state = CHASE_STATE_FINISHED;
            }
            return H_MOVE_TELEPORTED;
            
        } else {
        
            //Make it go to the direction it wants.
            float d = dist(pos, final_target_pos).to_float();
            
            chase_info.cur_speed +=
                chase_info.acceleration * delta_t;
            chase_info.cur_speed =
                std::min(chase_info.cur_speed, chase_info.max_speed);
                
            float move_amount =
                std::min(
                    (double) (d / delta_t),
                    (double) chase_info.cur_speed * move_speed_mult
                );
                
            bool can_free_move =
                has_flag(chase_info.flags, CHASE_FLAG_ANY_ANGLE) ||
                d <= MOB::FREE_MOVE_THRESHOLD;
                
            float movement_angle =
                can_free_move ?
                get_angle(pos, final_target_pos) :
                angle;
                
            move_speed->x = cos(movement_angle) * move_amount;
            move_speed->y = sin(movement_angle) * move_amount;
        }
        
    } else {
        chase_info.acceleration = 0.0f;
        chase_info.cur_speed = 0.0f;
        chase_info.max_speed = 0.0f;
        
    }
    
    //If another mob is pushing it.
    if(push_amount != 0.0f) {
        //Overly-aggressive pushing results in going through walls.
        //Let's place a cap.
        push_amount =
            std::min(push_amount, (float) (radius / delta_t) * 4);
            
        move_speed->x +=
            cos(push_angle) * (push_amount + MOB::PUSH_EXTRA_AMOUNT);
        move_speed->y +=
            sin(push_angle) * (push_amount + MOB::PUSH_EXTRA_AMOUNT);
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
 * e_ptr:
 *   Pointer to the edge in question.
 * wall_sector:
 *   Number of the sector that actually makes a wall (i.e. the highest).
 * move_angle:
 *   Angle at which the mob is going to move.
 * slide_angle:
 *   Holds the calculated slide angle.
 */
H_MOVE_RESULTS mob::get_wall_slide_angle(
    edge* e_ptr, unsigned char wall_sector, const float move_angle,
    float* slide_angle
) const {
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
 * delta_t:
 *   How long the frame's tick is, in seconds.
 * attempted_move_speed:
 *   Movement speed to calculate with.
 * touched_wall:
 *   Holds whether or not the mob touched a wall in this move.
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
        float new_z = z;
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
                !has_flag(flags, MOB_FLAG_WAS_THROWN) &&
                tallest_sector->z <= z + GEOMETRY::STEP_HEIGHT &&
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
 * Ticks the mob's actual physics procedures:
 * falling because of gravity, moving forward, etc.
 * delta_t:
 *   How long the frame's tick is, in seconds.
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
        
    switch (h_mov_type) {
    case H_MOVE_FAIL: {
        return;
    } case H_MOVE_TELEPORTED: {
        break;
    } case H_MOVE_OK: {
        //Horizontal movement time!
        tick_horizontal_movement_physics(
            delta_t, move_speed, &touched_wall
        );
        break;
    }
    }
    
    //Vertical movement.
    tick_vertical_movement_physics(
        delta_t, pre_move_ground_z, h_mov_type == H_MOVE_TELEPORTED
    );
    
    //Walk on top of another mob, if possible.
    if(type->can_walk_on_others) tick_walkable_riding_physics(delta_t);
    
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
 * Ticks physics logic regarding the mob rotating.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 * move_speed_mult:
 *   Movement speed is multiplied by this.
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
        sign(angle_dif) * std::min(
            (double) (type->rotation_speed * move_speed_mult * delta_t),
            (double) fabs(angle_dif)
        );
        
    if(holder.m) {
        switch(holder.rotation_method) {
        case HOLD_ROTATION_METHOD_FACE_HOLDER: {
            float dummy;
            point final_pos = holder.get_final_pos(&dummy);
            angle = get_angle(final_pos, holder.m->pos);
            stop_turning();
            break;
        } case HOLD_ROTATION_METHOD_COPY_HOLDER: {
            angle = holder.m->angle;
            stop_turning();
            break;
        } default: {
            break;
        }
        }
    }
    
    angle_cos = cos(angle);
    angle_sin = sin(angle);
}


/* ----------------------------------------------------------------------------
 * Ticks physics logic regarding the mob's vertical movement.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 * pre_move_ground_z:
 *   Z of the floor before horizontal movement started.
 * was_teleport:
 *   Did the mob just teleport previously in the movement logic?
 */
void mob::tick_vertical_movement_physics(
    const float delta_t, const float pre_move_ground_z,
    const bool was_teleport
) {
    bool apply_gravity = true;
    float old_speed_z = speed_z;
    speed_z = 0.0f;
    
    if(!standing_on_mob) {
        //If the current ground is one step (or less) below
        //the previous ground, just instantly go down the step.
        if(
            pre_move_ground_z - ground_sector->z <= GEOMETRY::STEP_HEIGHT &&
            z == pre_move_ground_z
        ) {
            z = ground_sector->z;
        }
    }
    
    //Vertical chasing.
    if(
        chase_info.state == CHASE_STATE_CHASING &&
        has_flag(flags, MOB_FLAG_CAN_MOVE_MIDAIR) &&
        !has_flag(chase_info.flags, CHASE_FLAG_TELEPORT)
    ) {
        apply_gravity = false;
        
        float target_z = chase_info.offset_z;
        if(chase_info.orig_z) target_z += *chase_info.orig_z;
        float diff_z = fabs(target_z - z);
        
        speed_z =
            std::min((float) (diff_z / delta_t), chase_info.cur_speed);
        if(target_z < z) {
            speed_z = -speed_z;
        }
    }
    
    //Gravity.
    if(
        apply_gravity && !has_flag(flags, MOB_FLAG_CAN_MOVE_MIDAIR) &&
        !holder.m && !was_teleport
    ) {
        speed_z = old_speed_z + delta_t* gravity_mult * MOB::GRAVITY_ADDER;
    }
    
    //Apply the change in Z.
    z += speed_z * delta_t;
    
    //Landing.
    hazard* new_on_hazard = NULL;
    if(speed_z <= 0) {
        if(standing_on_mob) {
            z = standing_on_mob->z + standing_on_mob->height;
            speed_z = 0;
            disable_flag(flags, MOB_FLAG_WAS_THROWN);
            fsm.run_event(MOB_EV_LANDED);
            stop_height_effect();
            
        } else if(z <= ground_sector->z) {
            z = ground_sector->z;
            speed_z = 0;
            disable_flag(flags, MOB_FLAG_WAS_THROWN);
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
        z = std::min(z, z_cap);
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
        
        for(size_t s = 0; s < statuses.size(); ++s) {
            if(statuses[s].type->remove_on_hazard_leave) {
                statuses[s].to_delete = true;
            }
        }
        delete_old_status_effects();
    }
    on_hazard = new_on_hazard;
    
    //Quick panic check: if it's somehow inside the ground, pop it out.
    z = std::max(z, ground_sector->z);
}


/* ----------------------------------------------------------------------------
 * Ticks physics logic regarding landing on top of a walkable mob.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void mob::tick_walkable_riding_physics(const float delta_t) {
    mob* rider_added_ev_mob = NULL;
    mob* rider_removed_ev_mob = NULL;
    mob* new_standing_on_mob = get_mob_to_walk_on();
    
    //Check which mob it is on top of, if any.
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
