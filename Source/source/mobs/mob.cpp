/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob class and mob-related functions.
 */

#include <algorithm>

#include "../const.h"
#include "../drawing.h"
#include "../functions.h"
#include "mob.h"
#include "pikmin.h"
#include "ship.h"
#include "../status.h"
#include "../vars.h"


size_t next_mob_id = 0;


/* ----------------------------------------------------------------------------
 * Creates a mob of no particular type.
 */
mob::mob(
    const float x, const float y, mob_type* type,
    const float angle, const string &vars
) :
    x(x),
    y(y),
    type(type),
    angle(angle),
    intended_angle(angle),
    anim(&type->anims),
    to_delete(false),
    reached_destination(false),
    speed_x(0),
    speed_y(0),
    speed_z(0),
    home_x(x),
    home_y(y),
    gravity_mult(1.0f),
    push_amount(0),
    push_angle(0),
    tangible(true),
    hide(false),
    health(type->max_health),
    invuln_period(0),
    team(MOB_TEAM_DECORATION),
    chasing(false),
    chase_teleport(false),
    chase_offs_x(x),
    chase_offs_y(y),
    chase_teleport_z(nullptr),
    chase_orig_x(nullptr),
    chase_orig_y(nullptr),
    chase_speed(-1),
    carrying_target(nullptr),
    cur_path_stop_nr(INVALID),
    focused_mob(nullptr),
    fsm(this),
    first_state_set(false),
    dead(false),
    big_damage_ev_queued(false),
    following_group(nullptr),
    was_thrown(false),
    group(nullptr),
    group_spot_x(0),
    group_spot_y(0),
    carry_info(nullptr),
    acceleration(0),
    speed(0),
    chase_free_move(false),
    chase_target_dist(0),
    on_hazard(nullptr),
    chomp_max(0),
    script_timer(0),
    id(next_mob_id) {
    
    next_mob_id++;
    
    sector* sec = get_sector(x, y, nullptr, true);
    z = sec->z;
    ground_sector = sec;
    center_sector = sec;
    
    fsm.set_state(type->first_state_nr);
}


/* ----------------------------------------------------------------------------
 * Makes the mob follow a game tick.
 * This basically calls sub-tickers.
 * Think of it this way: when you want to go somewhere,
 * you first think about rotating your body to face that
 * point, and then think about moving your legs.
 * Then, the actual physics go into place, your nerves
 * send signals to the muscles, and gravity, intertia, etc.
 * take over the rest, to make you move.
 */
void mob::tick() {
    tick_brain();
    tick_physics();
    tick_misc_logic();
    tick_script();
    tick_animation();
    tick_class_specifics();
}


/* ----------------------------------------------------------------------------
 * Ticks one game frame into the mob's animations.
 */
void mob::tick_animation() {
    float mult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); ++s) {
        mult *= this->statuses[s].type->anim_speed_multiplier;
    }
    
    bool finished_anim = anim.tick(delta_t * mult);
    
    if(finished_anim) {
        fsm.run_event(MOB_EVENT_ANIMATION_END);
    }
}


/* ----------------------------------------------------------------------------
 * Ticks the mob's brain for the next frame.
 * This has nothing to do with the mob's individual script.
 * This is related to mob-global things, like
 * thinking about where to move next and such.
 */
void mob::tick_brain() {
    //Chasing a target.
    if(chasing && !chase_teleport && speed_z == 0) {
    
        //Calculate where the target is.
        float final_target_x, final_target_y;
        get_chase_target(&final_target_x, &final_target_y);
        
        if(!chase_teleport) {
        
            if(
                !(fabs(final_target_x - x) < chase_target_dist &&
                  fabs(final_target_y - y) < chase_target_dist)
            ) {
                //If it still hasn't reached its target
                //(or close enough to the target),
                //time to make it think about how to get there.
                
                //Let the mob think about facing the actual target.
                face(atan2(final_target_y - y, final_target_x - x));
                
            } else {
                //Reached the location. The mob should now think
                //about stopping.
                
                chase_speed = 0;
                reached_destination = true;
                fsm.run_event(MOB_EVENT_REACHED_DESTINATION);
            }
            
        }
    }
}


/* ----------------------------------------------------------------------------
 * Performs some logic code for this game frame.
 */
void mob::tick_misc_logic() {
    //Other things.
    if(group) {
        float group_center_mx = 0, group_center_my = 0;
        move_point(
            group->group_center_x, group->group_center_y,
            x, y,
            type->move_speed,
            get_leader_to_group_center_dist(this),
            &group_center_mx, &group_center_my, NULL, NULL
        );
        group->group_center_x += group_center_mx * delta_t;
        group->group_center_y += group_center_my * delta_t;
    }
    
    invuln_period.tick(delta_t);
    
    for(size_t s = 0; s < this->statuses.size(); ++s) {
        statuses[s].tick(delta_t);
        health +=
            type->max_health * statuses[s].type->health_change_ratio * delta_t;
    }
    delete_old_status_effects();
    
    for(size_t g = 0; g < particle_generators.size(); ++g) {
        particle_generators[g].tick(delta_t, particles);
    }
}


/* ----------------------------------------------------------------------------
 * Ticks the mob's actual physics procedures:
 * falling because of gravity, moving forward, etc.
 */
void mob::tick_physics() {
    //Movement.
    bool finished_moving = false;
    bool doing_slide = false;
    
    float new_x = x, new_y = y, new_z = z;
    sector* new_ground_sector = ground_sector;
    sector* new_center_sector = center_sector;
    float pre_move_ground_z = ground_sector->z;
    
    float move_speed_x = speed_x;
    float move_speed_y = speed_y;
    
    float radius_to_use = type->radius;
    
    //Change the facing angle to the angle the mob wants to face.
    if(angle > M_PI)  angle -= M_PI * 2;
    if(angle < -M_PI) angle += M_PI * 2;
    if(intended_angle > M_PI)  intended_angle -= M_PI * 2;
    if(intended_angle < -M_PI) intended_angle += M_PI * 2;
    
    float angle_dif = intended_angle - angle;
    if(angle_dif > M_PI)  angle_dif -= M_PI * 2;
    if(angle_dif < -M_PI) angle_dif += M_PI * 2;
    
    float movement_speed_mult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); ++s) {
        movement_speed_mult *= this->statuses[s].type->speed_multiplier;
    }
    
    angle +=
        sign(angle_dif) * min(
            (double) (type->rotation_speed * movement_speed_mult * delta_t),
            (double) fabs(angle_dif)
        );
        
    if(chasing) {
        float final_target_x, final_target_y;
        get_chase_target(&final_target_x, &final_target_y);
        
        if(chase_teleport) {
            sector* sec =
                get_sector(final_target_x, final_target_y, NULL, true);
            if(!sec) {
                //No sector, invalid teleport. No move.
                return;
                
            } else {
                if(chase_teleport_z) {
                    ground_sector = sec;
                    z = *chase_teleport_z;
                }
                speed_x = speed_y = speed_z = 0;
                x = final_target_x;
                y = final_target_y;
                finished_moving = true;
            }
            
        } else {
        
            //Make it go to the direction it wants.
            float d = dist(x, y, final_target_x, final_target_y).to_float();
            
            float move_amount =
                min(
                    (double) (d / delta_t),
                    (double) chase_speed * movement_speed_mult
                );
                
            bool can_free_move = chase_free_move || d <= 10.0;
            
            float movement_angle =
                can_free_move ?
                atan2(final_target_y - y, final_target_x - x) :
                angle;
                
            move_speed_x = cos(movement_angle) * move_amount;
            move_speed_y = sin(movement_angle) * move_amount;
        }
    }
    
    
    //If another mob is pushing it.
    if(push_amount != 0.0f) {
        //Overly-aggressive pushing results in going through walls.
        //Let's place a cap.
        push_amount =
            min(push_amount, (float) ((type->radius / delta_t) - chase_speed));
        move_speed_x +=
            cos(push_angle) * (push_amount + MOB_PUSH_EXTRA_AMOUNT);
        move_speed_y +=
            sin(push_angle) * (push_amount + MOB_PUSH_EXTRA_AMOUNT);
    }
    
    push_amount = 0;
    
    
    //Try placing it in the place it should be at, judging
    //from the movement speed.
    while(!finished_moving) {
    
        if(move_speed_x == 0 && move_speed_y == 0) break;
        
        //Start by checking sector collisions.
        //For this, we will only check if the mob is intersecting
        //with any edge. With this, we trust that mobs can't go so fast
        //that they're fully on one side of an edge in one frame,
        //and the other side on the next frame.
        //It's pretty naive...but it works!
        bool successful_move = true;
        
        new_x = x + delta_t * move_speed_x;
        new_y = y + delta_t * move_speed_y;
        new_z = z;
        new_ground_sector = ground_sector;
        set<edge*> intersecting_edges;
        
        //Get the sector the mob will be on.
        sector* new_center_sector = get_sector(new_x, new_y, NULL, true);
        sector* step_sector = new_center_sector;
        
        if(!new_center_sector) {
            //Out of bounds. No movement.
            break;
        } else {
            new_ground_sector = new_center_sector;
        }
        
        //Quick panic handler: if it's under the ground, pop it out.
        if(z < new_center_sector->z) {
            z = new_center_sector->z;
        }
        
        //Before checking the edges, let's consult the blockmap and look at
        //the edges in the same block the mob is on.
        //This way, we won't check for edges that are really far away.
        //Use the bounding box to know which blockmap blocks the mob will be on.
        size_t bx1 = cur_area_data.bmap.get_col(new_x - radius_to_use);
        size_t bx2 = cur_area_data.bmap.get_col(new_x + radius_to_use);
        size_t by1 = cur_area_data.bmap.get_row(new_y - radius_to_use);
        size_t by2 = cur_area_data.bmap.get_row(new_y + radius_to_use);
        
        if(
            bx1 == INVALID || bx2 == INVALID ||
            by1 == INVALID || by2 == INVALID
        ) {
            //Somehow out of bounds. No movement.
            break;
        }
        
        float move_angle;
        float move_speed;
        coordinates_to_angle(
            move_speed_x, move_speed_y, &move_angle, &move_speed
        );
        
        //Angle to slide towards.
        float slide_angle = move_angle;
        //Difference between the movement angle and the slide.
        float slide_angle_dif = 0;
        
        edge* e_ptr = NULL;
        
        //Go through the blocks, to find intersections, and set up some things.
        for(size_t bx = bx1; bx <= bx2; ++bx) {
            for(size_t by = by1; by <= by2; ++by) {
            
                vector<edge*>* edges = &cur_area_data.bmap.edges[bx][by];
                
                for(size_t e = 0; e < edges->size(); ++e) {
                
                    e_ptr = (*edges)[e];
                    bool is_edge_blocking = false;
                    
                    if(
                        !circle_intersects_line(
                            new_x, new_y, radius_to_use,
                            e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y,
                            e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y,
                            NULL, NULL
                        )
                    ) {
                        continue;
                    }
                    
                    if(e_ptr->sectors[0] && e_ptr->sectors[1]) {
                    
                        if(
                            e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING ||
                            e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
                        ) {
                            is_edge_blocking = true;
                        }
                        
                        if(!is_edge_blocking) {
                            if(
                                e_ptr->sectors[0]->z < z &&
                                e_ptr->sectors[1]->z < z
                            ) {
                                //An edge whose sectors are below the mob?
                                //No collision here.
                                continue;
                            }
                            if(e_ptr->sectors[0]->z == e_ptr->sectors[1]->z) {
                                //No difference in floor height = no wall.
                                //Ignore this.
                                continue;
                            }
                        }
                        
                        sector* tallest_sector; //Tallest of the two.
                        if(
                            e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING
                        ) {
                            tallest_sector = e_ptr->sectors[1];
                            
                        } else if(
                            e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
                        ) {
                            tallest_sector = e_ptr->sectors[0];
                            
                        } else {
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
                            tallest_sector->z <= z + SECTOR_STEP &&
                            tallest_sector->z > step_sector->z
                        ) {
                            step_sector = tallest_sector;
                        }
                        
                        //Add this edge to the list of intersections, then.
                        intersecting_edges.insert(e_ptr);
                        
                    } else {
                    
                        //If we're on the edge of out-of-bounds geometry,
                        //block entirely.
                        successful_move = false;
                        break;
                        
                    }
                    
                }
                
                if(!successful_move) break;
            }
            
            if(!successful_move) break;
        }
        
        if(!successful_move) break;
        
        if(step_sector->z > new_ground_sector->z) {
            new_ground_sector = step_sector;
        }
        
        if(z < step_sector->z) new_z = step_sector->z;
        
        //Check wall angles and heights to check which of these edges
        //really are wall collisions.
        for(
            auto e = intersecting_edges.begin();
            e != intersecting_edges.end(); e++
        ) {
        
            e_ptr = *e;
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
            
            //If both floors of this edge are above the mob...
            //then what does that mean? That the mob is under the ground?
            //Nonsense! Throw this edge away!
            //It's a false positive, and the only
            //way for it to get caught is if it's behind a more logical
            //edge that we actually did collide against.
            if(e_ptr->sectors[0] && e_ptr->sectors[1]) {
                if(
                    (
                        e_ptr->sectors[0]->z > new_z ||
                        e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING
                    ) &&
                    (
                        e_ptr->sectors[1]->z > new_z ||
                        e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
                    )
                ) {
                    continue;
                }
            }
            
            //Ok, there's obviously been a collision, so let's work out what
            //wall the mob will slide on.
            
            //The wall's normal is the direction the wall is facing.
            //i.e. the direction from the top floor to the bottom floor.
            //We know which side of an edge is which sector because of
            //the vertexes. Imagine you're in first person view,
            //following the edge as a line on the ground.
            //You start on vertex 0 and face vertex 1.
            //Sector 0 will always be on your left.
            if(!doing_slide) {
            
                float wall_normal;
                float wall_angle =
                    atan2(
                        e_ptr->vertexes[1]->y - e_ptr->vertexes[0]->y,
                        e_ptr->vertexes[1]->x - e_ptr->vertexes[0]->x
                    );
                    
                if(wall_sector == 0) {
                    wall_normal = normalize_angle(wall_angle + M_PI_2);
                } else {
                    wall_normal = normalize_angle(wall_angle - M_PI_2);
                }
                
                float nd = get_angle_cw_dif(wall_normal, move_angle);
                if(nd < M_PI_2 || nd > M_PI + M_PI_2) {
                    //If the difference between the movement and the wall's
                    //normal is this, that means we came FROM the wall.
                    //No way! There has to be an edge that makes more sense.
                    continue;
                }
                
                //If we were to slide on this edge, this would be
                //the slide angle.
                float tentative_slide_angle;
                if(nd < M_PI) {
                    //Coming in from the "left" of the normal. Slide right.
                    tentative_slide_angle = wall_normal + M_PI_2;
                } else {
                    //Coming in from the "right" of the normal. Slide left.
                    tentative_slide_angle = wall_normal - M_PI_2;
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
        }
        
        //If the mob is just slamming against the wall head-on, perpendicularly,
        //then forget any idea about sliding.
        //It'd just be awkwardly walking in place.
        if(!successful_move && slide_angle_dif > M_PI_2 - 0.05) {
            doing_slide = true;
        }
        
        
        //We're done here. If the move was unobstructed, good, go there.
        //If not, we'll use the info we gathered before to calculate sliding,
        //and try again.
        
        if(successful_move) {
            //Good news, the mob can move to this new spot freely.
            x = new_x;
            y = new_y;
            z = new_z;
            ground_sector = new_ground_sector;
            center_sector = new_center_sector;
            finished_moving = true;
            
        } else {
        
            //Try sliding.
            if(doing_slide) {
                //We already tried sliding, and we still hit something...
                //Let's just stop completely. This mob can't go forward.
                speed_x = 0;
                speed_y = 0;
                finished_moving = true;
                
            } else {
            
                doing_slide = true;
                //To limit the speed, we should use a cross-product of the
                //movement and slide vectors.
                //But nuts to that, this is just as nice, and a lot simpler!
                move_speed *= 1 - (slide_angle_dif / M_PI);
                angle_to_coordinates(
                    slide_angle, move_speed, &move_speed_x, &move_speed_y
                );
                
            }
            
        }
        
    }
    
    
    //Vertical movement.
    
    //If the current ground is one step (or less) below
    //the previous ground, just instantly go down the step.
    if(
        pre_move_ground_z - ground_sector->z <= SECTOR_STEP &&
        z == pre_move_ground_z
    ) {
        z = ground_sector->z;
    }
    
    //Landing on a bottomless pit or hazardous floor.
    hazard* new_on_hazard = NULL;
    z += delta_t * speed_z;
    if(z <= ground_sector->z) {
        z = ground_sector->z;
        speed_z = 0;
        was_thrown = false;
        fsm.run_event(MOB_EVENT_LANDED);
        if(ground_sector->type == SECTOR_TYPE_BOTTOMLESS_PIT) {
            fsm.run_event(MOB_EVENT_BOTTOMLESS_PIT);
        }
        
        for(size_t h = 0; h < ground_sector->hazards.size(); ++h) {
            fsm.run_event(
                MOB_EVENT_TOUCHED_HAZARD,
                (void*) ground_sector->hazards[h]
            );
            new_on_hazard = ground_sector->hazards[h];
        }
    }
    
    //Gravity.
    if(gravity_mult > 0) {
        if(z > ground_sector->z) {
            speed_z += delta_t * gravity_mult * GRAVITY_ADDER;
        }
    } else {
        speed_z += delta_t * gravity_mult * GRAVITY_ADDER;
    }
    
    //On a sector that has a hazard, not on the floor.
    if(z > ground_sector->z && !ground_sector->hazard_floor) {
        for(size_t h = 0; h < ground_sector->hazards.size(); ++h) {
            fsm.run_event(
                MOB_EVENT_TOUCHED_HAZARD,
                (void*) ground_sector->hazards[h]
            );
            new_on_hazard = ground_sector->hazards[h];
        }
    }
    
    if(new_on_hazard != on_hazard && on_hazard != NULL) {
        fsm.run_event(
            MOB_EVENT_LEFT_HAZARD,
            (void*) on_hazard
        );
    }
    on_hazard = new_on_hazard;
}


/* ----------------------------------------------------------------------------
 * Checks general events in the mob's script for this frame.
 */
void mob::tick_script() {
    if(!first_state_set) {
        fsm.set_state(type->first_state_nr);
        first_state_set = true;
    }
    
    //Health regeneration.
    health += type->health_regen * delta_t;
    health = min(health, type->max_health);
    
    if(!fsm.cur_state) return;
    
    //Timer events.
    mob_event* timer_ev = q_get_event(this, MOB_EVENT_TIMER);
    if(timer_ev && script_timer.duration > 0) {
        if(script_timer.time_left > 0) {
            script_timer.tick(delta_t);
            if(script_timer.time_left == 0.0f) {
                timer_ev->run(this);
            }
        }
    }
    
    //Has it reached its home?
    mob_event* reach_dest_ev = q_get_event(this, MOB_EVENT_REACHED_DESTINATION);
    if(reach_dest_ev && reached_destination) {
        reach_dest_ev->run(this);
    }
    
    //Is it dead?
    if(health <= 0 && type->max_health != 0) {
        dead = true;
        fsm.run_event(MOB_EVENT_DEATH, this);
    }
    
    //Big damage.
    mob_event* big_damage_ev = q_get_event(this, MOB_EVENT_BIG_DAMAGE);
    if(big_damage_ev && big_damage_ev_queued) {
        big_damage_ev->run(this);
        big_damage_ev_queued = false;
    }
}


/* ----------------------------------------------------------------------------
 * Code specific for each class. Meant to be overwritten by the child classes.
 */
void mob::tick_class_specifics() {
}


/* ----------------------------------------------------------------------------
 * Returns the actual location of the movement target.
 */
void mob::get_chase_target(float* x, float* y) {
    *x = chase_offs_x;
    *y = chase_offs_y;
    if(chase_orig_x) *x += *chase_orig_x;
    if(chase_orig_y) *y += *chase_orig_y;
}


/* ----------------------------------------------------------------------------
 * Sets a target for the mob to follow.
 * offs_*:          Coordinates of the target, relative to either the
   * world origin, or another point, specified in the next parameters.
 * orig_*:          Pointers to changing coordinates. If NULL, it is
   * the world origin. Use this to make the mob follow another mob
   * wherever they go, for instance.
 * teleport:        If true, the mob teleports to that spot,
   * instead of walking to it.
 * teleport_z:      Teleports to this Z coordinate, too.
 * free_move:       If true, the mob can go to a direction they're not facing.
 * target_distance: Distance from the target in which the mob is
   * considered as being there.
 * speed:           Speed at which to go to the target. -1 uses the mob's speed.
 */
void mob::chase(
    const float offs_x, const float offs_y,
    float* orig_x, float* orig_y,
    const bool teleport, float* teleport_z,
    const bool free_move, const float target_distance, const float speed
) {

    this->chase_offs_x = offs_x; this->chase_offs_y = offs_y;
    this->chase_orig_x = orig_x; this->chase_orig_y = orig_y;
    this->chase_teleport = teleport;
    this->chase_teleport_z = teleport_z;
    this->chase_free_move = free_move;
    this->chase_target_dist = target_distance;
    this->chase_speed = (speed == -1 ? get_base_speed() : speed);
    
    chasing = true;
    reached_destination = false;
}


/* ----------------------------------------------------------------------------
 * Makes a mob not follow any target any more.
 */
void mob::stop_chasing() {
    chasing = false;
    reached_destination = false;
    chase_teleport_z = NULL;
    
    speed_x = 0;
    speed_y = 0;
}


/* ----------------------------------------------------------------------------
 * Makes the mob eat some of the enemies it has chomped on.
 * nr: Number of captured enemies to swallow.
   * 0:       Release all of them.
 */
void mob::eat(const size_t nr) {

    if(nr == 0) {
        for(size_t p = 0; p < chomping_pikmin.size(); ++p) {
            chomping_pikmin[p]->fsm.run_event(MOB_EVENT_RELEASED);
        }
        chomping_pikmin.clear();
        return;
    }
    
    size_t total = min(nr, chomping_pikmin.size());
    
    for(size_t p = 0; p < total; ++p) {
        chomping_pikmin[p]->health = 0;
        chomping_pikmin[p]->dead = true;
        chomping_pikmin[p]->fsm.run_event(MOB_EVENT_EATEN);
    }
    chomping_pikmin.clear();
}


/* ----------------------------------------------------------------------------
 * Makes a mob gradually face a new angle.
 */
void mob::face(const float new_angle) {
    if(carry_info) return; //If it's being carried, it shouldn't rotate.
    intended_angle = new_angle;
}


/* ----------------------------------------------------------------------------
 * Removes all particle generators with the given ID.
 */
void mob::remove_particle_generator(const int id) {
    for(size_t g = 0; g < particle_generators.size();) {
        if(particle_generators[g].id == id) {
            particle_generators.erase(particle_generators.begin() + g);
        } else {
            ++g;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Sets the mob's animation.
 * nr: Animation number; it's the animation instance number from the pool.
 */
void mob::set_animation(const size_t nr, const bool pre_named) {
    if(nr >= type->anims.animations.size()) return;
    
    size_t final_nr;
    if(pre_named) {
        if(anim.anim_pool->pre_named_conversions.size() <= nr) return;
        final_nr = anim.anim_pool->pre_named_conversions[nr];
    } else {
        final_nr = nr;
    }
    
    if(final_nr == INVALID) return;
    
    animation* new_anim = anim.anim_pool->animations[final_nr];
    anim.anim = new_anim;
    anim.start();
}


/* ----------------------------------------------------------------------------
 * Changes a mob's health, relatively or absolutely.
 * rel:    Change is relative to the current value
   * (i.e. add or subtract from current health)
 * amount: Health amount.
 */
void mob::set_health(const bool rel, const float amount) {
    unsigned short base_nr = 0;
    if(rel) base_nr = health;
    
    health = max(0.0f, (float) (base_nr + amount));
}


/* ----------------------------------------------------------------------------
 * Changes the timer's time and interval.
 * time: New time.
 */
void mob::set_timer(const float time) {
    script_timer.duration = time;
    script_timer.start();
}


/* ----------------------------------------------------------------------------
 * Sets a script variable's value.
 * name:  The variable's name
 * value: The variable's new value.
 */
void mob::set_var(const string &name, const string &value) {
    vars[name] = value;
}


/* ----------------------------------------------------------------------------
 * Sets up stuff for the beginning of the mob's death process.
 */
void mob::start_dying() {
    health = 0;
    if(typeid(*this) == typeid(enemy)) {
        particle p(PARTICLE_TYPE_BITMAP, x, y, 64, 1.5, PARTICLE_PRIORITY_LOW);
        p.bitmap = bmp_sparkle;
        p.color = al_map_rgb(255, 192, 192);
        particle_generator pg(0, p, 25);
        pg.number_deviation = 5;
        pg.angle = 0;
        pg.angle_deviation = M_PI;
        pg.speed = 100;
        pg.speed_deviation = 40;
        pg.duration_deviation = 0.5;
        pg.emit(particles);
    }
}


/* ----------------------------------------------------------------------------
 * Sets up stuff for the end of the mob's dying process.
 */
void mob::finish_dying() {
    if(typeid(*this) == typeid(enemy)) {
        enemy* e_ptr = (enemy*) this;
        if(e_ptr->ene_type->drops_corpse) {
            become_carriable(false);
            e_ptr->fsm.set_state(ENEMY_EXTRA_STATE_CARRIABLE_WAITING);
        }
        particle par(
            PARTICLE_TYPE_ENEMY_SPIRIT, x, y,
            64, 2, PARTICLE_PRIORITY_MEDIUM
        );
        par.bitmap = bmp_enemy_spirit;
        par.speed_x = 0;
        par.speed_y = -50;
        par.friction = 0.5;
        par.gravity = 0;
        par.color = al_map_rgb(255, 192, 255);
        particles.add(par);
    }
}


/* ----------------------------------------------------------------------------
 * Applies a status effect's effects.
 */
void mob::apply_status_effect(status_type* s, const bool refill) {
    if(!can_receive_status(s)) return;
    
    //Check if the mob is already under this status.
    for(size_t ms = 0; ms < this->statuses.size(); ++ms) {
        if(this->statuses[ms].type == s) {
            //Already exists. Can we refill its duration?
            
            if(refill && s->auto_remove_time > 0.0f) {
                this->statuses[ms].time_left = s->auto_remove_time;
            }
            
            return;
        }
    }
    
    //This status is not already inflicted. Let's do so.
    this->statuses.push_back(status(s));
    if(s->causes_panic) {
        receive_panic_from_status();
    }
    if(s->causes_flailing) {
        receive_flailing_from_status();
    }
    change_maturity_amount_from_status(s->maturity_change_amount);
    if(s->generates_particles) {
        particle_generator pg = *s->particle_gen;
        pg.follow_x = &this->x;
        pg.follow_y = &this->y;
        pg.reset();
        particle_generators.push_back(pg);
    }
}


/* ----------------------------------------------------------------------------
 * Deletes all status effects asking to be deleted.
 */
void mob::delete_old_status_effects() {
    for(size_t s = 0; s < this->statuses.size(); ) {
        if(statuses[s].to_delete) {
            if(statuses[s].type->causes_panic) {
                lose_panic_from_status();
            }
            if(statuses[s].type->generates_particles) {
                remove_particle_generator(statuses[s].type->particle_gen->id);
            }
            this->statuses.erase(this->statuses.begin() + s);
        } else {
            ++s;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns the average tint color for all non-white-tint status effects.
 */
ALLEGRO_COLOR mob::get_status_tint_color() {
    size_t n_tints = 0;
    ALLEGRO_COLOR ret = al_map_rgba(0, 0, 0, 0);
    size_t n_statuses = this->statuses.size();
    for(size_t s = 0; s < n_statuses; ++s) {
        status_type* t = this->statuses[s].type;
        if(
            t->tint.r == 1.0f &&
            t->tint.g == 1.0f &&
            t->tint.b == 1.0f &&
            t->tint.a == 1.0f
        ) {
            continue;
        }
        
        ret.r += t->tint.r;
        ret.g += t->tint.g;
        ret.b += t->tint.b;
        ret.a += t->tint.a;
        n_tints++;
    }
    
    if(n_tints == 0) {
        return al_map_rgb(255, 255, 255);
    } else {
        ret.r /= n_statuses;
        ret.g /= n_statuses;
        ret.b /= n_statuses;
        ret.a /= n_statuses;
        return ret;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the current frame of an animation of one of the status effects
 * that the mob is under.
 * bmp_scale: Returns the mob size's scale to apply to the image.
 */
ALLEGRO_BITMAP* mob::get_status_bitmap(float* bmp_scale) {
    *bmp_scale = 0.0f;
    for(size_t s = 0; s < this->statuses.size(); ++s) {
        status_type* t = this->statuses[s].type;
        if(t->animation_name.empty()) continue;
        frame* f = t->anim_instance.get_frame();
        if(!f) return NULL;
        *bmp_scale = t->animation_mob_scale;
        return f->bitmap;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the base speed for this mob.
 * This is overwritten by some child classes.
 */
float mob::get_base_speed() {
    return this->type->move_speed;
}


bool mob::can_receive_status(status_type* s) { return false; };
void mob::receive_flailing_from_status() {}
void mob::receive_panic_from_status() {}
void mob::lose_panic_from_status() {}
void mob::change_maturity_amount_from_status(const int amount) {}


mob::~mob() {}


/* ----------------------------------------------------------------------------
 * Creates a structure with info about a carrying spot.
 */
carrier_spot_struct::carrier_spot_struct(const float x, const float y) :
    state(CARRY_SPOT_FREE),
    x(x),
    y(y),
    pik_ptr(NULL) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a structure with info about carrying.
 * m:             The mob this info belongs to.
 * max_carriers:  The maximum number of carrier Pikmin.
 * carry_to_ship: If true, this mob is delivered to a ship. Otherwise, an Onion.
 */
carry_info_struct::carry_info_struct(mob* m, const bool carry_to_ship) :
    m(m),
    carry_to_ship(carry_to_ship),
    cur_carrying_strength(0),
    cur_n_carriers(0),
    final_destination_x(0),
    final_destination_y(0),
    obstacle_ptr(nullptr),
    go_straight(false),
    stuck_state(0),
    is_moving(false) {
    
    float pikmin_radius = 16;
    //Let's assume all Pikmin are the same radius. Or at least very close.
    if(!pikmin_types.empty()) {
        pikmin_radius = pikmin_types.begin()->second->radius;
    }
    
    for(size_t c = 0; c < m->type->max_carriers; ++c) {
        float angle = (M_PI * 2) / m->type->max_carriers * c;
        float x = cos(angle) * (m->type->radius + pikmin_radius);
        float y = sin(angle) * (m->type->radius + pikmin_radius);
        spot_info.push_back(carrier_spot_struct(x, y));
    }
}


/* ----------------------------------------------------------------------------
 * Returns the speed at which the object should move, given the carrier Pikmin.
 */
float carry_info_struct::get_speed() {
    float max_speed = 0;
    
    //Begin by obtaining the average walking speed of the carriers.
    for(size_t s = 0; s < spot_info.size(); ++s) {
        carrier_spot_struct* s_ptr = &spot_info[s];
        
        if(s_ptr->state != CARRY_SPOT_USED) continue;
        
        pikmin* p_ptr = (pikmin*) s_ptr->pik_ptr;
        max_speed += p_ptr->get_base_speed();
    }
    max_speed /= cur_n_carriers;
    
    //If the object has all carriers, the Pikmin move as fast
    //as possible, which looks bad, since they're not jogging,
    //they're carrying. Let's add a penalty for the weight...
    max_speed *= (1 - carrying_speed_weight_mult * m->type->weight);
    //...and a global carrying speed penalty.
    max_speed *= carrying_speed_max_mult;
    
    //The closer the mob is to having full carriers,
    //the closer to the max speed we get.
    //The speed goes from carrying_speed_base_mult (0 carriers)
    //to max_speed (all carriers).
    return max_speed * (
               carrying_speed_base_mult +
               (cur_n_carriers / (float) spot_info.size()) *
               (1 - carrying_speed_base_mult)
           );
}


/* ----------------------------------------------------------------------------
 * Returns true if all spots are reserved. False otherwise.
 */
bool carry_info_struct::is_full() {
    for(size_t s = 0; s < spot_info.size(); ++s) {
        if(spot_info[s].state == CARRY_SPOT_FREE) return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Deletes a carrier info structure.
 */
carry_info_struct::~carry_info_struct() {
    //TODO
}


/* ----------------------------------------------------------------------------
 * Adds a mob to another mob's group.
 */
void add_to_group(mob* group_leader, mob* new_member) {
    //If it's already following, never mind.
    if(new_member->following_group == group_leader) return;
    
    new_member->following_group = group_leader;
    group_leader->group->members.push_back(new_member);
    
    //Find a spot.
    if(group_leader->group) {
        if(group_leader->group->group_spots) {
            group_leader->group->group_spots->add(new_member);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Applies the knockback values to a mob.
 */
void apply_knockback(
    mob* m, const float knockback, const float knockback_angle
) {
    if(knockback != 0) {
        m->stop_chasing();
        m->speed_x = cos(knockback_angle) * knockback * MOB_KNOCKBACK_H_POWER;
        m->speed_y = sin(knockback_angle) * knockback * MOB_KNOCKBACK_H_POWER;
        m->speed_z = MOB_KNOCKBACK_V_POWER;
    }
}


/* ----------------------------------------------------------------------------
 * Calculates how much damage an attack will cause.
 * attacker:     the attacking mob.
 * victim:       the mob that'll take the damage.
 * attacker_h:   the hitbox of the attacker mob, if any.
 * victim_h:     the hitbox of the victim mob, if any.
 */
float calculate_damage(
    mob* attacker, mob* victim, hitbox_instance* attacker_h,
    hitbox_instance* victim_h
) {
    float attacker_offense = 0;
    float defense_multiplier = 1;
    
    if(victim_h && victim_h->type != HITBOX_TYPE_NORMAL) {
        //This hitbox can't be damaged! Abort!
        return 0;
    }
    
    if(attacker_h) {
        attacker_offense = attacker_h->multiplier;
        
    } else {
        if(typeid(*attacker) == typeid(pikmin)) {
            pikmin* pik_ptr = (pikmin*) attacker;
            attacker_offense =
                pik_ptr->pik_type->attack_power *
                (1 + pik_ptr->maturity * maturity_power_mult);
        }
    }
    
    if(victim_h) {
        defense_multiplier = victim_h->multiplier;
    }
    
    for(size_t s = 0; s < attacker->statuses.size(); ++s) {
        attacker_offense *= attacker->statuses[s].type->attack_multiplier;
    }
    for(size_t s = 0; s < victim->statuses.size(); ++s) {
        defense_multiplier *= victim->statuses[s].type->defense_multiplier;
    }
    
    return attacker_offense * (1.0 / defense_multiplier);
    
}


/* ----------------------------------------------------------------------------
 * Calculates how much knockback an attack will cause.
 * attacker:   the attacking mob.
 * victim:     the mob that'll take the damage.
 * attacker_h: the hitbox of the attacker mob, if any.
 * victim_h:   the hitbox of the victim mob, if any.
 * knockback:  the variable to return the knockback amount to.
 * angle:      the variable to return the angle of the knockback to.
 */
void calculate_knockback(
    mob* attacker, mob* victim, hitbox_instance* attacker_h,
    hitbox_instance* victim_h, float* knockback, float* angle
) {
    if(attacker_h) {
        *knockback = attacker_h->knockback;
        if(attacker_h->knockback_outward) {
            *angle += atan2(victim->y - attacker->y, victim->x - attacker->x);
        } else {
            *angle += attacker_h->knockback_angle;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Causes a mob to damage another via hitboxes.
 * attacker:     the attacking mob.
 * victim:       the mob that'll take the damage.
 * attacker_h:   the hitbox of the attacker mob, if any.
 * victim_h:     the hitbox of the victim mob, if any.
 * total_damage: the variable to return the total caused damage to, if any.
 */
void cause_hitbox_damage(
    mob* attacker, mob* victim, hitbox_instance* attacker_h,
    hitbox_instance* victim_h, float* total_damage
) {
    float attacker_offense = 0;
    float defense_multiplier = 1;
    float knockback = 0;
    float knockback_angle = attacker->angle;
    
    if(attacker_h) {
        attacker_offense = attacker_h->multiplier;
        knockback = attacker_h->knockback;
        if(attacker_h->knockback_outward) {
            knockback_angle +=
                atan2(victim->y - attacker->y, victim->x - attacker->x);
        } else {
            knockback_angle += attacker_h->knockback_angle;
        }
        
    } else {
        if(typeid(*attacker) == typeid(pikmin)) {
            attacker_offense =
                ((pikmin*) attacker)->maturity *
                ((pikmin*) attacker)->pik_type->attack_power *
                maturity_power_mult;
        }
    }
    
    if(victim_h) {
        defense_multiplier = victim_h->multiplier;
    }
    
    float damage = attacker_offense * (1.0 / defense_multiplier);
    
    if(total_damage) *total_damage = damage;
    
    //Cause the damage and the knockback.
    victim->health -= damage;
    if(knockback != 0) {
        victim->stop_chasing();
        victim->speed_x =
            cos(knockback_angle) * knockback * MOB_KNOCKBACK_H_POWER;
        victim->speed_y =
            sin(knockback_angle) * knockback * MOB_KNOCKBACK_H_POWER;
        victim->speed_z =
            MOB_KNOCKBACK_V_POWER;
    }
    
    //Script stuff.
    victim->fsm.run_event(MOB_EVENT_DAMAGE, victim);
    
    //If before taking damage, the interval was dividable X times,
    //and after it's only dividable by Y (X>Y), an interval was crossed.
    if(
        victim->type->big_damage_interval > 0 &&
        victim->health != victim->type->max_health
    ) {
        if(
            floor((victim->health + damage) /
                  victim->type->big_damage_interval) >
            floor(victim->health / victim->type->big_damage_interval)
        ) {
            victim->big_damage_ev_queued = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a mob, adding it to the corresponding vectors.
 */
void create_mob(mob* m) {
    mobs.push_back(m);
    
    if(typeid(*m) == typeid(pikmin)) {
        pikmin_list.push_back((pikmin*) m);
        
    } else if(typeid(*m) == typeid(leader)) {
        leaders.push_back((leader*) m);
        
    } else if(typeid(*m) == typeid(onion)) {
        onions.push_back((onion*) m);
        
    } else if(typeid(*m) == typeid(nectar)) {
        nectars.push_back((nectar*) m);
        
    } else if(typeid(*m) == typeid(pellet)) {
        pellets.push_back((pellet*) m);
        
    } else if(typeid(*m) == typeid(ship)) {
        ships.push_back((ship*) m);
        
    } else if(typeid(*m) == typeid(treasure)) {
        treasures.push_back((treasure*) m);
        
    } else if(typeid(*m) == typeid(info_spot)) {
        info_spots.push_back((info_spot*) m);
        
    } else if(typeid(*m) == typeid(enemy)) {
        enemies.push_back((enemy*) m);
        
    } else if(typeid(*m) == typeid(gate)) {
        gates.push_back((gate*) m);
        
    } else if(typeid(*m) == typeid(bridge)) {
        bridges.push_back((bridge*) m);
        
    }
}


/* ----------------------------------------------------------------------------
 * Deletes a mob from the relevant vectors.
 * It's always removed from the vector of mobs, but it's
 * also removed from the vector of Pikmin if it's a Pikmin,
 * leaders if it's a leader, etc.
 */
void delete_mob(mob* m) {
    remove_from_group(m);
    
    mobs.erase(find(mobs.begin(), mobs.end(), m));
    
    if(typeid(*m) == typeid(pikmin)) {
        pikmin* p_ptr = (pikmin*) m;
        pikmin_list.erase(find(pikmin_list.begin(), pikmin_list.end(), p_ptr));
        
    } else if(typeid(*m) == typeid(leader)) {
        leaders.erase(find(leaders.begin(), leaders.end(), (leader*) m));
        
    } else if(typeid(*m) == typeid(onion)) {
        onions.erase(find(onions.begin(), onions.end(), (onion*) m));
        
    } else if(typeid(*m) == typeid(nectar)) {
        nectars.erase(find(nectars.begin(), nectars.end(), (nectar*) m));
        
    } else if(typeid(*m) == typeid(pellet)) {
        pellets.erase(find(pellets.begin(), pellets.end(), (pellet*) m));
        
    } else if(typeid(*m) == typeid(ship)) {
        ships.erase(find(ships.begin(), ships.end(), (ship*) m));
        
    } else if(typeid(*m) == typeid(treasure)) {
        treasures.erase(
            find(treasures.begin(), treasures.end(), (treasure*) m)
        );
        
    } else if(typeid(*m) == typeid(info_spot)) {
        info_spots.erase(
            find(info_spots.begin(), info_spots.end(), (info_spot*) m)
        );
        
    } else if(typeid(*m) == typeid(enemy)) {
        enemies.erase(find(enemies.begin(), enemies.end(), (enemy*) m));
        
    } else if(typeid(*m) == typeid(gate)) {
        gates.erase(find(gates.begin(), gates.end(), (gate*) m));
        
    } else {
        log_error(
            "ENGINE WARNING: Ran delete_mob() with a bad mob, of type \"" +
            m->type->name + "\", x = " + f2s(m->x) +
            ", y = " + f2s(m->y) + "!"
        );
        
    }
    
    delete m;
}


/* ----------------------------------------------------------------------------
 * Makes m1 focus on m2.
 */
void focus_mob(mob* m1, mob* m2) {
    unfocus_mob(m1);
    
    m1->focused_mob = m2;
}


/* ----------------------------------------------------------------------------
 * Returns the closest hitbox to a point, belonging to a mob's current frame
 * of animation and position.
 * x, y: Point.
 * m:    The mob.
 */
hitbox_instance* get_closest_hitbox(const float x, const float y, mob* m) {
    frame* f = m->anim.get_frame();
    if(!f) return NULL;
    hitbox_instance* closest_hitbox = NULL;
    float closest_hitbox_dist = 0;
    
    for(size_t h = 0; h < f->hitbox_instances.size(); ++h) {
        hitbox_instance* h_ptr = &f->hitbox_instances[h];
        float hx, hy;
        rotate_point(h_ptr->x, h_ptr->y, m->angle, &hx, &hy);
        float d = dist(x - m->x, y - m->y, hx, hy).to_float() - h_ptr->radius;
        if(h == 0 || d < closest_hitbox_dist) {
            closest_hitbox_dist = d;
            closest_hitbox = h_ptr;
        }
    }
    
    return closest_hitbox;
}


/* ----------------------------------------------------------------------------
 * Returns the hitbox instance in the current animation with
 * the specified number.
 */
hitbox_instance* get_hitbox_instance(mob* m, const size_t nr) {
    frame* f = m->anim.get_frame();
    if(!f) return NULL;
    if(f->hitbox_instances.empty()) return NULL;
    return &f->hitbox_instances[nr];
}


/* ----------------------------------------------------------------------------
 * Removes a mob from its leader's group.
 */
void remove_from_group(mob* member) {
    if(!member->following_group) return;
    
    member->following_group->group->members.erase(
        find(
            member->following_group->group->members.begin(),
            member->following_group->group->members.end(),
            member
        )
    );
    
    if(member->following_group->group->group_spots) {
        member->following_group->group->group_spots->remove(member);
    }
    
    member->following_group = NULL;
}


/* ----------------------------------------------------------------------------
 * Should m1 attack m2? Teams are used to decide this.
 */
bool should_attack(mob* m1, mob* m2) {
    if(m1->team == m2->team) return false;
    if(m2->team == MOB_TEAM_DECORATION) return false;
    if(m1->team == MOB_TEAM_NONE) return true;
    if(m2->team == MOB_TEAM_OBSTACLE) {
        if(typeid(*m1) == typeid(pikmin)) return true;
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Makes m1 lose focus on its current mob.
 */
void unfocus_mob(mob* m1) {
    m1->focused_mob = nullptr;
}


/* ----------------------------------------------------------------------------
 * Sets up data for a mob to become carriable.
 */
void mob::become_carriable(const bool to_ship) {
    carry_info = new carry_info_struct(this, to_ship);
}


/* ----------------------------------------------------------------------------
 * Sets up data for a mob to stop being carriable.
 */
void mob::become_uncarriable() {
    if(!carry_info) return;
    
    for(size_t p = 0; p < carry_info->spot_info.size(); ++p) {
        if(carry_info->spot_info[p].state != CARRY_SPOT_FREE) {
            carry_info->spot_info[p].pik_ptr->fsm.run_event(
                MOB_EVENT_FOCUSED_MOB_UNCARRIABLE
            );
        }
    }
    
    stop_chasing();
    
    delete carry_info;
    carry_info = NULL;
}


/* ----------------------------------------------------------------------------
 * Checks the carrying destination again.
 */
void mob::recalculate_carrying_destination(mob* m, void* info1, void* info2) {
    m->calculate_carrying_destination(NULL, NULL);
}


/* ----------------------------------------------------------------------------
 * Updates carrying data, begins moving if needed, etc.
 * added:   The Pikmin that got added, if any.
 * removed: The Pikmin that got removed, if any.
 */
void mob::calculate_carrying_destination(mob* added, mob* removed) {
    if(!carry_info) return;
    
    carry_info->stuck_state = 0;
    
    //For starters, check if this is to be carried to the ship.
    //Get that out of the way if so.
    if(carry_info->carry_to_ship) {
    
        ship* closest_ship = NULL;
        dist closest_ship_dist;
        
        for(size_t s = 0; s < ships.size(); ++s) {
            ship* s_ptr = ships[s];
            dist d(
                x, y,
                s_ptr->beam_final_x, s_ptr->beam_final_y
            );
            
            if(!closest_ship || d < closest_ship_dist) {
                closest_ship = s_ptr;
                closest_ship_dist = d;
            }
        }
        
        if(closest_ship) {
            carry_info->final_destination_x = closest_ship->beam_final_x;
            carry_info->final_destination_y = closest_ship->beam_final_y;
            carrying_target = closest_ship;
            
        } else {
            carrying_target = NULL;
            carry_info->stuck_state = 1;
            return;
        }
        
        return;
    }
    
    //If it's meant for an Onion, we need to decide which Onion, based on
    //the Pikmin. Buckle up, because it's not as easy as it might seem.
    
    //How many of each Pikmin type are carrying.
    map<pikmin_type*, unsigned> type_quantity;
    //The Pikmin type with the most carriers.
    vector<pikmin_type*> majority_types;
    unordered_set<pikmin_type*> available_onions;
    
    //First, check what Onions even are available.
    for(size_t o = 0; o < onions.size(); o++) {
        onion* o_ptr = onions[o];
        if(o_ptr->activated) {
            available_onions.insert(o_ptr->oni_type->pik_type);
        }
    }
    
    if(available_onions.empty()) {
        //No Onions?! Well...make the Pikmin stuck.
        carrying_target = NULL;
        carry_info->stuck_state = 1;
        return;
    }
    
    //Count how many of each type there are carrying.
    for(size_t p = 0; p < type->max_carriers; ++p) {
        pikmin* pik_ptr = NULL;
        
        if(carry_info->spot_info[p].state != CARRY_SPOT_USED) continue;
        
        pik_ptr = (pikmin*) carry_info->spot_info[p].pik_ptr;
        
        //If it doesn't have an Onion, it won't even count.
        if(available_onions.find(pik_ptr->pik_type) == available_onions.end()) {
            continue;
        }
        
        type_quantity[pik_ptr->pik_type]++;
    }
    
    //Then figure out what are the majority types.
    unsigned most = 0;
    for(auto t = type_quantity.begin(); t != type_quantity.end(); ++t) {
        if(t->second > most) {
            most = t->second;
            majority_types.clear();
        }
        if(t->second == most) majority_types.push_back(t->first);
    }
    
    //If we ended up with no candidates, pick a type at random,
    //out of all possible types.
    if(majority_types.empty()) {
        for(
            auto t = available_onions.begin();
            t != available_onions.end(); ++t
        ) {
            majority_types.push_back(*t);
        }
    }
    
    pikmin_type* decided_type = NULL;
    
    //Now let's pick an Onion from the candidates.
    if(majority_types.size() == 1) {
        //If there's only one possible type to pick, pick it.
        decided_type = *majority_types.begin();
        
    } else {
        //If there's a tie, let's take a careful look.
        bool new_tie = false;
        
        //Is the Pikmin that just joined part of the majority types?
        //If so, that means this Pikmin just created a NEW tie!
        //So let's pick a random Onion again.
        if(added) {
            for(size_t mt = 0; mt < majority_types.size(); ++mt) {
                if(added->type == majority_types[mt]) {
                    new_tie = true;
                    break;
                }
            }
        }
        
        //If a Pikmin left, check if it is related to the majority types.
        //If not, then a new tie wasn't made, no worries.
        //If it was related, a new tie was created.
        if(removed) {
            new_tie = false;
            for(size_t mt = 0; mt < majority_types.size(); ++mt) {
                if(removed->type == majority_types[mt]) {
                    new_tie = true;
                    break;
                }
            }
        }
        
        //Check if the previously decided type belongs to one of the majorities.
        //If so, it can be chosen again, but if not, it cannot.
        bool can_continue = false;
        for(size_t mt = 0; mt < majority_types.size(); ++mt) {
            if(majority_types[mt] == decided_type) {
                can_continue = true;
                break;
            }
        }
        if(!can_continue) decided_type = NULL;
        
        //If the Pikmin that just joined is not a part of the majorities,
        //then it had no impact on the existing ties.
        //Go with the Onion that had been decided before.
        if(new_tie || !decided_type) {
            //TODO make this cycle instead of being picked randomly.
            decided_type =
                majority_types[randomi(0, majority_types.size() - 1)];
        }
    }
    
    
    //Figure out where that type's Onion is.
    size_t onion_nr = 0;
    for(; onion_nr < onions.size(); ++onion_nr) {
        if(onions[onion_nr]->oni_type->pik_type == decided_type) {
            break;
        }
    }
    
    //Finally, set the destination data.
    carry_info->final_destination_x = onions[onion_nr]->x;
    carry_info->final_destination_y = onions[onion_nr]->y;
    carrying_target = onions[onion_nr];
}


/* ----------------------------------------------------------------------------
 * Draws the mob. This can be overwritten by child classes.
 */
void mob::draw() {

    frame* f_ptr = anim.get_frame();
    
    if(!f_ptr) return;
    
    float draw_x, draw_y;
    float draw_w, draw_h;
    get_sprite_center(this, f_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, f_ptr, &draw_w, &draw_h);
    
    ALLEGRO_COLOR tint = get_status_tint_color();
    float brightness = get_sprite_brightness(this) / 255.0;
    tint.r *= brightness;
    tint.g *= brightness;
    tint.b *= brightness;
    tint.a *= brightness;
    
    draw_sprite(
        f_ptr->bitmap,
        draw_x, draw_y,
        draw_w, draw_h,
        angle,
        tint
    );
    
}

/* ----------------------------------------------------------------------------
 * Returns where a sprite's center should be, for normal mob drawing routines.
 */
void mob::get_sprite_center(mob* m, frame* f, float* x, float* y) {
    float c = cos(m->angle), s = sin(m->angle);
    *x = m->x + c * f->offs_x + c * f->offs_y;
    *y = m->y - s * f->offs_y + s * f->offs_x;
    
}

/* ----------------------------------------------------------------------------
 * Returns what a sprite's dimensions should be,
 * for normal mob drawing routines.
 * m: the mob.
 * f: the frame.
 * w: variable to return the width to.
 * h: variable to return the height to.
 * scale: variable to return the scale used to. Optional.
 */
void mob::get_sprite_dimensions(
    mob* m, frame* f, float* w, float* h, float* scale
) {
    *w = f->game_w;
    *h = f->game_h;
    float sucking_mult = 1.0;
    float height_mult = 1 + m->z * 0.0001;
    
    float final_scale = sucking_mult * height_mult;
    if(scale) *scale = final_scale;
    
    *w = *w * final_scale;
    *h = *h * final_scale;
}

/* ----------------------------------------------------------------------------
 * Returns what a sprite's lighting should be, for normal mob drawing routines.
 */
float mob::get_sprite_brightness(mob* m) {
    return m->center_sector->brightness;
}
