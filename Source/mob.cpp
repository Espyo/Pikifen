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

#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "mob.h"
#include "pikmin.h"
#include "ship.h"
#include "vars.h"


size_t next_mob_id = 0;


/* ----------------------------------------------------------------------------
 * Creates a mob.
 */
mob::mob(const float x, const float y, mob_type* type, const float angle, const string &vars) :
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
    affected_by_gravity(true),
    push_amount(0),
    push_angle(0),
    health(type->max_health),
    invuln_period(0),
    team(MOB_TEAM_DECORATION),
    go_to_target(false),
    gtt_instant(false),
    target_x(x),
    target_y(y),
    target_z(nullptr),
    target_rel_x(nullptr),
    target_rel_y(nullptr),
    focused_mob(nullptr),
    fsm(this),
    first_state_set(false),
    dead(false),
    delivery_time(DELIVERY_SUCK_TIME + 1.0f),
    big_damage_ev_queued(false),
    following_party(nullptr),
    was_thrown(false),
    unwhistlable_period(0),
    untouchable_period(0),
    party(nullptr),
    party_spot_x(0),
    party_spot_y(0),
    carrier_info(nullptr),
    move_speed_mult(0),
    acceleration(0),
    speed(0),
    gtt_free_move(false),
    target_distance(0),
    chomp_max(0),
    script_timer(0),
    id(next_mob_id) {
    
    next_mob_id++;
    
    sector* sec = get_sector(x, y, nullptr, true);
    z = sec->z;
    ground_z = sec->z;
    lighting = sec->brightness;
    
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
}


/* ----------------------------------------------------------------------------
 * Ticks one game frame into the mob's animations.
 */
void mob::tick_animation() {
    bool finished_anim = anim.tick(delta_t);
    
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
    if(go_to_target && !gtt_instant && speed_z == 0) {
    
        //Calculate where the target is.
        float final_target_x, final_target_y;
        get_final_target(&final_target_x, &final_target_y);
        
        if(!gtt_instant) {
        
            if(
                !(fabs(final_target_x - x) < target_distance &&
                  fabs(final_target_y - y) < target_distance)
            ) {
                //If it still hasn't reached its target (or close enough to the target),
                //time to make it think about how to get there.
                
                //Let the mob think about facing the actual target.
                face(atan2(final_target_y - y, final_target_x - x));
                //Let the mob think about moving forward.
                speed = type->move_speed;
                
            } else {
                //Reached the location. The mob should now think
                //about stopping.
                
                speed = 0;
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
    if(unwhistlable_period > 0) {
        unwhistlable_period -= delta_t;
        unwhistlable_period = max(unwhistlable_period, 0.0f);
    }
    if(untouchable_period > 0) {
        untouchable_period -= delta_t;
        untouchable_period = max(untouchable_period, 0.0f);
    }
    
    if(delivery_time <= DELIVERY_SUCK_TIME) {
        delivery_time -= delta_t;
        delivery_time = max(delivery_time, 0.0f);
    }
    
    if(party) {
        float party_center_mx = 0, party_center_my = 0;
        move_point(
            party->party_center_x, party->party_center_y,
            x, y,
            type->move_speed,
            get_leader_to_group_center_dist(this),
            &party_center_mx, &party_center_my, NULL, NULL
        );
        party->party_center_x += party_center_mx * delta_t;
        party->party_center_y += party_center_my * delta_t;
        
        size_t n_members = party->members.size();
        for(size_t m = 0; m < n_members; ++m) {
            party->members[m]->face(atan2(y - party->members[m]->y, x - party->members[m]->x));
        }
    }
    
    invuln_period.tick(delta_t);
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
    float new_ground_z = ground_z;
    unsigned char new_lighting = lighting;
    float pre_move_ground_z = ground_z;
    
    float move_speed_x = speed_x;
    float move_speed_y = speed_y;
    
    //Change the facing angle to the angle the mob wants to face.
    if(angle > M_PI)  angle -= M_PI * 2;
    if(angle < -M_PI) angle += M_PI * 2;
    if(intended_angle > M_PI)  intended_angle -= M_PI * 2;
    if(intended_angle < -M_PI) intended_angle += M_PI * 2;
    
    float angle_dif = intended_angle - angle;
    if(angle_dif > M_PI)  angle_dif -= M_PI * 2;
    if(angle_dif < -M_PI) angle_dif += M_PI * 2;
    
    angle += sign(angle_dif) * min((double) (type->rotation_speed * delta_t), (double) fabs(angle_dif));
    
    if(go_to_target) {
        //If the mob is meant to teleport somewhere,
        //let's just do so.
        float final_target_x, final_target_y;
        get_final_target(&final_target_x, &final_target_y);
        
        if(gtt_instant) {
            sector* sec = get_sector(final_target_x, final_target_y, NULL, true);
            if(!sec) {
                //No sector, invalid teleport. No move.
                return;
                
            } else {
                if(target_z) {
                    ground_z = sec->z;
                    z = *target_z;
                }
                speed_x = speed_y = speed_z = 0;
                x = final_target_x;
                y = final_target_y;
                finished_moving = true;
            }
            
        } else {
        
            //Make it go to the direction it wants.
            float d = dist(x, y, final_target_x, final_target_y).to_float();
            float move_amount = min((double) (d / delta_t), (double) speed);
            
            bool can_free_move = gtt_free_move || move_amount <= 10.0;
            
            float movement_angle = can_free_move ?
                                   atan2(final_target_y - y, final_target_x - x) :
                                   angle;
                                   
            move_speed_x = cos(movement_angle) * move_amount;
            move_speed_y = sin(movement_angle) * move_amount;
        }
    }
    
    
    //If another mob is pushing it.
    if(push_amount != 0.0f) {
        move_speed_x += cos(push_angle) * push_amount;
        move_speed_y += sin(push_angle) * push_amount;
        
        push_amount = 0;
    }
    
    
    //Try placing it in the place it should be at, judging
    //from the movement speed.
    while(!finished_moving) {
    
        if(move_speed_x == 0 && move_speed_y == 0) break;
        
        //Start by checking sector collisions.
        //For this, we will only check if the mob is intersecting with any edge.
        //With this, we trust that mobs can't go so fast that they're fully on one
        //side of an edge in one frame, and the other side on the next frame.
        //It's pretty naive... but it works!
        bool successful_move = true;
        
        new_x = x + delta_t* move_speed_x;
        new_y = y + delta_t* move_speed_y;
        new_z = z;
        new_ground_z = ground_z;
        new_lighting = lighting;
        set<edge*> intersecting_edges;
        
        //Get the sector the mob is currently on.
        sector* base_sector = get_sector(new_x, new_y, NULL, true);
        
        if(!base_sector) {
            //Out of bounds. No movement.
            break;
        } else {
            new_ground_z = base_sector->z;
            new_lighting = base_sector->brightness;
        }
        
        //Quick panic handler: if it's under the ground, pop it out.
        if(z < base_sector->z) {
            z = base_sector->z;
        }
        
        //Before checking the edges, let's consult the blockmap and look at
        //the edges in the same block the mob is on.
        //This way, we won't check for edges that are really far away.
        //Use the bounding box to know which blockmap blocks the mob will be on.
        size_t bx1 = cur_area_data.bmap.get_col(new_x - type->radius);
        size_t bx2 = cur_area_data.bmap.get_col(new_x + type->radius);
        size_t by1 = cur_area_data.bmap.get_row(new_y - type->radius);
        size_t by2 = cur_area_data.bmap.get_row(new_y + type->radius);
        
        if(
            bx1 == string::npos || bx2 == string::npos ||
            by1 == string::npos || by2 == string::npos
        ) {
            //Somehow out of bounds. No movement.
            break;
        }
        
        float move_angle;
        float move_speed;
        coordinates_to_angle(move_speed_x, move_speed_y, &move_angle, &move_speed);
        
        float slide_angle = move_angle; //Angle to slide towards.
        float slide_angle_dif = 0;      //Difference between the movement angle and the slide.
        float step_z = new_ground_z;    //Height of the step, if any.
        float tallest_z_below_mob = new_ground_z; //Tallest sector floor below the mob.
        
        edge* e_ptr = NULL;
        
        //Go through the blocks, to find intersections, and set up some things.
        for(size_t bx = bx1; bx <= bx2; ++bx) {
            for(size_t by = by1; by <= by2; ++by) {
            
                vector<edge*>* edges = &cur_area_data.bmap.edges[bx][by];
                
                for(size_t e = 0; e < edges->size(); ++e) {
                
                    e_ptr = (*edges)[e];
                    bool is_edge_wall = false;
                    
                    if(
                        !circle_intersects_line(
                            new_x, new_y, type->radius,
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
                            is_edge_wall = true;
                        }
                        
                        if(e_ptr->sectors[0]->z == e_ptr->sectors[1]->z && !is_edge_wall) {
                            //No difference in floor height = no wall. Ignore this.
                            continue;
                        }
                        
                        float tallest_z; //Tallest of the two sectors.
                        if(e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING) {
                            tallest_z = e_ptr->sectors[1]->z;
                        } else if(e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING) {
                            tallest_z = e_ptr->sectors[0]->z;
                        } else {
                            tallest_z = max(e_ptr->sectors[0]->z, e_ptr->sectors[1]->z);
                        }
                        
                        if(tallest_z > tallest_z_below_mob && tallest_z <= z) {
                            tallest_z_below_mob = tallest_z;
                        }
                        
                        if(tallest_z < z && !is_edge_wall) {
                            //An edge whose sectors are below the mob? No collision here.
                            continue;
                        }
                        
                        //Check if it can go up this step.
                        //It can go up this step if the floor is within stepping distance
                        //of the mob's current Z, and if this step is larger
                        //than any step encountered of all edges crossed.
                        if(
                            tallest_z <= new_ground_z + SECTOR_STEP &&
                            tallest_z > step_z
                        ) {
                            step_z = tallest_z;
                        }
                        
                        //Add this edge to the list of intersections, then.
                        intersecting_edges.insert(e_ptr);
                        
                    } else {
                    
                        //If we're on the edge of out-of-bounds geometry, block entirely.
                        successful_move = false;
                        break;
                        
                    }
                    
                }
                
                if(!successful_move) break;
            }
            
            if(!successful_move) break;
        }
        
        if(!successful_move) break;
        
        if(step_z > tallest_z_below_mob) tallest_z_below_mob = step_z;
        new_ground_z = tallest_z_below_mob;
        
        if(z < step_z) new_z = step_z;
        
        //Check wall angles and heights to check which of these edges really are wall collisions.
        for(auto e = intersecting_edges.begin(); e != intersecting_edges.end(); e++) {
        
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
            //Nonsense! Throw this edge away! It's a false positive, and the only
            //way for it to get caught is if it's behind a more logical
            //edge that we actually did collide against.
            if(e_ptr->sectors[0] && e_ptr->sectors[1]) {
                if(
                    (e_ptr->sectors[0]->z > new_z || e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING) &&
                    (e_ptr->sectors[1]->z > new_z || e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING)
                ) {
                    continue;
                }
            }
            
            //Ok, there's obviously been a collision, so let's work out what
            //wall the mob will slide on.
            
            //The wall's normal is the direction the wall is facing.
            //i.e. the direction from the top floor to the bottom floor.
            //We know which side of an edge is which sector because of the vertexes.
            //Imagine you're in first person view, following the edge as a line on the ground.
            //You start on vertex 0 and face vertex 1. Sector 0 will always be on your left.
            if(!doing_slide) {
            
                float wall_normal;
                float wall_angle = atan2(e_ptr->vertexes[1]->y - e_ptr->vertexes[0]->y, e_ptr->vertexes[1]->x - e_ptr->vertexes[0]->x);
                
                if(wall_sector == 0) {
                    wall_normal = normalize_angle(wall_angle + M_PI_2);
                } else {
                    wall_normal = normalize_angle(wall_angle - M_PI_2);
                }
                
                float nd = get_angle_cw_dif(wall_normal, move_angle);
                if(nd < M_PI_2 || nd > M_PI + M_PI_2) {
                    //If the difference between the movement and the wall's normal is this,
                    //that means we came FROM the wall. No way! There has to be an edge
                    //that makes more sense.
                    continue;
                }
                
                //If we were to slide on this edge, this would be the slide angle.
                float tentative_slide_angle;
                if(nd < M_PI) {
                    //Coming in from the "left" of the normal. Slide right.
                    tentative_slide_angle = wall_normal + M_PI_2;
                } else {
                    //Coming in from the "right" of the normal. Slide left.
                    tentative_slide_angle = wall_normal - M_PI_2;
                }
                
                float sd = get_angle_smallest_dif(move_angle, tentative_slide_angle);
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
        //then forget any idea about sliding. It'd just be awkwardly walking in place.
        if(!successful_move && slide_angle_dif > M_PI_2 - 0.05) {
            doing_slide = true;
        }
        
        
        //We're done here. If the move was unobstructed, good, go there.
        //If not, we'll use the info we gathered before to calculate sliding, and try again.
        
        if(successful_move) {
            //Good news, the mob can move to this new spot freely.
            x = new_x;
            y = new_y;
            z = new_z;
            ground_z = new_ground_z;
            lighting = new_lighting;
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
                //To limit the speed, we should use a cross-product of the movement and slide vectors.
                //But nuts to that, this is just as nice, and a lot simpler!
                move_speed *= 1 - (slide_angle_dif / M_PI);
                angle_to_coordinates(slide_angle, move_speed, &move_speed_x, &move_speed_y);
                
            }
            
        }
        
    }
    
    
    //Vertical movement.
    
    //If the current ground is one step (or less) below
    //the previous ground, just instantly go down the step.
    if(pre_move_ground_z - ground_z <= SECTOR_STEP && z == pre_move_ground_z) {
        z = ground_z;
    }
    
    bool was_airborne = z > ground_z;
    z += delta_t* speed_z;
    if(z <= ground_z) {
        z = ground_z;
        if(was_airborne) {
            speed_z = 0;
            was_thrown = false;
            fsm.run_event(MOB_EVENT_LANDED);
            if(get_sector(x, y, NULL, true)->type == SECTOR_TYPE_BOTTOMLESS_PIT){
                fsm.run_event(MOB_EVENT_BOTTOMLESS_PIT);
            }
        }
    }
    
    //Gravity.
    if(z > ground_z && affected_by_gravity) {
        speed_z += delta_t* (GRAVITY_ADDER);
    }
}


/* ----------------------------------------------------------------------------
 * Checks general events in the mob's script for this frame.
 */
void mob::tick_script() {
    if(!first_state_set) {
        fsm.set_state(type->first_state_nr);
        first_state_set = true;
    }
    
    //Timer events.
    mob_event* timer_ev = fsm.get_event(MOB_EVENT_TIMER);
    if(timer_ev && script_timer.interval > 0) {
        script_timer.tick(delta_t);
        if(script_timer.is_over) {
            script_timer.start();
            timer_ev->run(this);
        }
    }
    
    //Has it reached its home?
    mob_event* reach_dest_ev = fsm.get_event(MOB_EVENT_REACHED_DESTINATION);
    if(reach_dest_ev && reached_destination) {
        reach_dest_ev->run(this);
    }
    
    //Is it dead?
    if(health <= 0 && type->max_health != 0) {
        dead = true;
        fsm.run_event(MOB_EVENT_DEATH, this);
    }
}



/* ----------------------------------------------------------------------------
 * Returns the actual location of the movement target.
 */
void mob::get_final_target(float* x, float* y) {
    *x = target_x;
    *y = target_y;
    if(target_rel_x) *x += *target_rel_x;
    if(target_rel_y) *y += *target_rel_y;
}


/* ----------------------------------------------------------------------------
 * Sets a target for the mob to follow.
 * target_*:     Coordinates of the target, relative to either the world origin,
   * or another point, specified in the next parameters.
 * target_rel_*: Pointers to moving coordinates. If NULL, it's the world origin.
   * Use this to make the mob follow another mob wherever they go, for instance.
 * instant:      If true, the mob teleports to that spot, instead of walking to it.
 * target_z:     Teleports to this Z coordinate, too.
 * free_move:    If true, the mob can go to a direction they're not facing.
 * target_distance: Distance from the target in which the mob is considered as being there.
 */
void mob::set_target(float target_x, float target_y, float* target_rel_x, float* target_rel_y, bool instant, float* target_z, bool free_move, float target_distance) {
    this->target_x = target_x; this->target_y = target_y;
    this->target_rel_x = target_rel_x; this->target_rel_y = target_rel_y;
    this->gtt_instant = instant;
    this->target_z = target_z;
    this->gtt_free_move = free_move;
    this->target_distance = target_distance;
    
    go_to_target = true;
    reached_destination = false;
}


/* ----------------------------------------------------------------------------
 * Makes a mob not follow any target.
 */
void mob::remove_target() {
    go_to_target = false;
    reached_destination = false;
    target_z = NULL;
    
    speed_x = 0;
    speed_y = 0;
}


/* ----------------------------------------------------------------------------
 * Makes the mob eat some of the enemies it has chomped on.
 * nr: Number of captured enemies to swallow.
   * 0:            Release all of them.
   * string::npos: Eat all of them.
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
        chomping_pikmin[p]->fsm.run_event(MOB_EVENT_EATEN);
    }
    chomping_pikmin.clear();
}


/* ----------------------------------------------------------------------------
 * Makes a mob gradually face a new angle.
 */
void mob::face(float new_angle) {
    if(carrier_info) return; //If it's being carried, it shouldn't rotate.
    intended_angle = new_angle;
}


/* ----------------------------------------------------------------------------
 * Sets the mob's animation.
 * nr: Animation number; it's the animation instance number from the pool.
 */
void mob::set_animation(const size_t nr, bool pre_named) {
    if(nr >= type->anims.animations.size()) return;
    
    size_t final_nr;
    if(pre_named) {
        if(anim.anim_pool->pre_named_conversions.size() <= nr) return;
        final_nr = anim.anim_pool->pre_named_conversions[nr];
    } else {
        final_nr = nr;
    }
    
    if(final_nr == string::npos) return;
    
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
    script_timer.interval = time;
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
        random_particle_explosion(PARTICLE_TYPE_BITMAP, bmp_sparkle, x, y, 100, 140, 20, 40, 1, 2, 64, 64, al_map_rgb(255, 192, 192));
    }
}


/* ----------------------------------------------------------------------------
 * Sets up stuff for the end of the mob's dying process.
 */
void mob::finish_dying() {
    if(typeid(*this) == typeid(enemy)) {
        enemy* e_ptr = (enemy*) this;
        if(e_ptr->ene_type->drops_corpse) {
            carrier_info = new carrier_info_struct(this, e_ptr->ene_type->max_carriers, false);
        }
        particles.push_back(
            particle(
                PARTICLE_TYPE_ENEMY_SPIRIT, bmp_enemy_spirit, x, y,
                0, -50, 0.5, 0, 2, 64, al_map_rgb(255, 192, 255)
            )
        );
    }
}


/* ----------------------------------------------------------------------------
 * Returns the base speed for this mob.
 * This is overwritten by some child classes.
 */
float mob::get_base_speed() {
    return this->type->move_speed;
}


mob::~mob() {}


/* ----------------------------------------------------------------------------
 * Creates a structure with info about carrying.
 * m:             The mob this info belongs to.
 * max_carriers:  The maximum number of carrier Pikmin.
 * carry_to_ship: If true, this mob is delivered to a ship. Otherwise, an Onion.
 */
carrier_info_struct::carrier_info_struct(mob* m, unsigned int max_carriers, bool carry_to_ship) :
    max_carriers(max_carriers),
    carry_to_ship(carry_to_ship),
    current_carrying_strength(0),
    current_n_carriers(0),
    decided_type(nullptr) {
    
    for(size_t c = 0; c < max_carriers; ++c) {
        carrier_spots.push_back(NULL);
        float angle = (M_PI * 2) / max_carriers * c;
        carrier_spots_x.push_back(cos(angle) * m->type->radius);
        carrier_spots_y.push_back(sin(angle) * m->type->radius);
    }
}


/* ----------------------------------------------------------------------------
 * Deletes a carrier info structure.
 * Makes all carrying Pikmin drop it in the process.
 */
carrier_info_struct::~carrier_info_struct() {
}


/* ----------------------------------------------------------------------------
 * Adds a mob to another mob's party.
 */
void add_to_party(mob* party_leader, mob* new_member) {
    if(new_member->following_party == party_leader) return; //Already following, never mind.
    
    new_member->following_party = party_leader;
    party_leader->party->members.push_back(new_member);
    
    //Find a spot.
    if(party_leader->party) {
        if(party_leader->party->party_spots) {
            party_leader->party->party_spots->add(new_member);
            
            //TODO remove.
            /*
            new_member->set_target(
                spot_x, spot_y,
                &party_leader->party->party_center_x, &party_leader->party->party_center_y,
                false
            );*/
        }
    }
    
    make_uncarriable(new_member);
}


/* ----------------------------------------------------------------------------
 * Applies the knockback values to a mob.
 */
void apply_knockback(mob* m, const float knockback, const float knockback_angle) {
    if(knockback != 0) {
        //TODO make these not be magic numbers.
        m->remove_target();
        m->speed_x = cos(knockback_angle) * knockback * 130;
        m->speed_y = sin(knockback_angle) * knockback * 130;
        m->speed_z = 200;
    }
}


/* ----------------------------------------------------------------------------
 * Calculates how much damage an attack will cause.
 * attacker:     the attacking mob.
 * victim:       the mob that'll take the damage.
 * attacker_h:   the hitbox of the attacker mob, if any.
 * victim_h:     the hitbox of the victim mob, if any.
 */
float calculate_damage(mob* attacker, mob* victim, hitbox_instance* attacker_h, hitbox_instance* victim_h) {
    float attacker_offense = 0;
    float defense_multiplier = 1;
    
    if(attacker_h) {
        attacker_offense = attacker_h->multiplier;
        
    } else {
        if(typeid(*attacker) == typeid(pikmin)) {
            pikmin* pik_ptr = (pikmin*) attacker;
            attacker_offense = pik_ptr->pik_type->attack_power * (1 + pik_ptr->maturity * MATURITY_POWER_MULT);
        }
    }
    
    if(victim_h) {
        defense_multiplier = victim_h->multiplier;
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
    mob* attacker, mob* victim, hitbox_instance* attacker_h, hitbox_instance* victim_h,
    float* knockback, float* angle
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
void cause_hitbox_damage(mob* attacker, mob* victim, hitbox_instance* attacker_h, hitbox_instance* victim_h, float* total_damage) {
    float attacker_offense = 0;
    float defense_multiplier = 1;
    float knockback = 0;
    float knockback_angle = attacker->angle;
    
    if(attacker_h) {
        attacker_offense = attacker_h->multiplier;
        knockback = attacker_h->knockback;
        if(attacker_h->knockback_outward) {
            knockback_angle += atan2(victim->y - attacker->y, victim->x - attacker->x);
        } else {
            knockback_angle += attacker_h->knockback_angle;
        }
        
    } else {
        if(typeid(*attacker) == typeid(pikmin)) {
            attacker_offense = ((pikmin*) attacker)->maturity * ((pikmin*) attacker)->pik_type->attack_power * MATURITY_POWER_MULT;
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
        //TODO make these not be magic numbers.
        victim->remove_target();
        victim->speed_x = cos(knockback_angle) * knockback * 130;
        victim->speed_y = sin(knockback_angle) * knockback * 130;
        victim->speed_z = 200;
    }
    
    //Script stuff.
    victim->fsm.run_event(MOB_EVENT_DAMAGE, victim);
    
    //If before taking damage, the interval was dividable X times, and after it's only dividable by Y (X>Y), an interval was crossed.
    if(victim->type->big_damage_interval > 0 && victim->health != victim->type->max_health) {
        if(
            floor((victim->health + damage) / victim->type->big_damage_interval) >
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
    remove_from_party(m);
    
    mobs.erase(find(mobs.begin(), mobs.end(), m));
    
    if(typeid(*m) == typeid(pikmin)) {
        pikmin* p_ptr = (pikmin*) m;
        pikmin::forget_about_carrying(m, NULL, NULL);
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
        treasures.erase(find(treasures.begin(), treasures.end(), (treasure*) m));
        
    } else if(typeid(*m) == typeid(info_spot)) {
        info_spots.erase(find(info_spots.begin(), info_spots.end(), (info_spot*) m));
        
    } else if(typeid(*m) == typeid(enemy)) {
        enemies.erase(find(enemies.begin(), enemies.end(), (enemy*) m));
        
    } else if(typeid(*m) == typeid(gate)) {
        gates.erase(find(gates.begin(), gates.end(), (gate*) m));
        
    } else {
        //TODO warn somehow.
        
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
 * Returns the closest hitbox to a point, belonging to a mob's current frame of animation and position.
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
 * Returns the hitbox instance in the current animation with the specified number.
 */
hitbox_instance* get_hitbox_instance(mob* m, const size_t nr) {
    frame* f = m->anim.get_frame();
    if(!f) return NULL;
    if(f->hitbox_instances.empty()) return NULL;
    return &f->hitbox_instances[nr];
}


/* ----------------------------------------------------------------------------
 * Makes a mob impossible to be carried, and makes the Pikmin carrying it drop it.
 */
void make_uncarriable(mob* m) {
    if(!m->carrier_info) return;
    
    for(size_t p = 0; p < m->carrier_info->carrier_spots.size(); ++p) {
        mob* p_ptr = m->carrier_info->carrier_spots[p];
        if(!p_ptr) continue;
        p_ptr->fsm.run_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE);
    }
    
    delete m->carrier_info;
    m->carrier_info = NULL;
}


/* ----------------------------------------------------------------------------
 * Removes a mob from its leader's party.
 */
void remove_from_party(mob* member) {
    if(!member->following_party) return;
    
    member->following_party->party->members.erase(find(
                member->following_party->party->members.begin(),
                member->following_party->party->members.end(),
                member));
                
    if(member->following_party->party->party_spots) {
        member->following_party->party->party_spots->remove(member);
    }
    
    member->following_party = NULL;
    member->unwhistlable_period = UNWHISTLABLE_PERIOD;
    member->untouchable_period = UNTOUCHABLE_PERIOD;
}


/* ----------------------------------------------------------------------------
 * Should m1 attack m2? Teams are used to decide this.
 */
bool should_attack(mob* m1, mob* m2) {
    if(m2->team == MOB_TEAM_DECORATION) return false;
    if(m1->team == MOB_TEAM_NONE) return true;
    if(m1->team == m2->team) return false;
    if(typeid(*m1) == typeid(pikmin) && m2->team == MOB_TEAM_OBSTACLE) return true;
    return true;
}


/* ----------------------------------------------------------------------------
 * Makes m1 lose focus on its current mob.
 */
void unfocus_mob(mob* m1) {
    m1->focused_mob = nullptr;
}


/* ----------------------------------------------------------------------------
 * Event handler that makes the mob lose health by being damaged by another.
 */
void mob::lose_health(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    if(!should_attack(m, info->mob2)) return;
    
    float damage = 0;
    
    damage = calculate_damage(info->mob2, m, info->hi2, info->hi1);
    m->health -= damage;
    
    m->fsm.run_event(MOB_EVENT_DAMAGE, info->mob2);
    
    //If before taking damage, the interval was dividable X times, and after it's only dividable by Y (X>Y), an interval was crossed.
    if(
        m->type->big_damage_interval > 0 &&
        m->health != m->type->max_health
    ) {
        if(
            floor((m->health + damage) / m->type->big_damage_interval) >
            floor(m->health / m->type->big_damage_interval)
        ) {
            m->big_damage_ev_queued = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Draws the mob. This should be overwritten by child classes.
 */
void mob::draw() {

    frame* f_ptr = anim.get_frame();
    
    if(!f_ptr) return;
    
    float draw_x, draw_y;
    float draw_w, draw_h;
    get_sprite_center(this, f_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, f_ptr, &draw_w, &draw_h);
    
    draw_sprite(
        f_ptr->bitmap,
        draw_x, draw_y,
        draw_w, draw_h,
        angle,
        map_gray(get_sprite_lighting(this))
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
 * Returns what a sprite's dimensions should be, for normal mob drawing routines.
 * m: the mob.
 * f: the frame.
 * w: variable to return the width to.
 * h: variable to return the height to.
 * scale: variable to return the scale used to. Optional.
 */
void mob::get_sprite_dimensions(mob* m, frame* f, float* w, float* h, float* scale) {
    *w = f->game_w;
    *h = f->game_h;
    float sucking_mult = 1.0;
    float height_mult = 1 + m->z * 0.0001;
    if(m->delivery_time < DELIVERY_SUCK_TIME) {
        sucking_mult = (m->delivery_time / DELIVERY_SUCK_TIME);
        sucking_mult = max(sucking_mult, 0.0f);
    }
    
    float final_scale = sucking_mult * height_mult;
    if(scale) *scale = final_scale;
    
    *w = *w * final_scale;
    *h = *h * final_scale;
}

/* ----------------------------------------------------------------------------
 * Returns what a sprite's lighting should be, for normal mob drawing routines.
 */
float mob::get_sprite_lighting(mob* m) {
    return m->lighting;
}
