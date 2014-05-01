#include <algorithm>

#include "const.h"
#include "functions.h"
#include "mob.h"
#include "pikmin.h"
#include "vars.h"

mob::mob(const float x, const float y, const float z, mob_type* t, sector* sec) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->sec = sec;
    
    type = t;
    anim = animation_instance(&t->anims);
    
    to_delete = false;
    reached_destination = false;
    
    speed_x = speed_y = speed_z = 0;
    home_x = x; home_y = y;
    angle = intended_angle = 0;
    affected_by_gravity = true;
    
    health = t->max_health;
    invuln_period = 0;
    knockdown_period = 0;
    team = MOB_TEAM_NONE;
    
    go_to_target = false;
    gtt_instant = false;
    target_x = x;
    target_y = y;
    target_rel_x = NULL;
    target_rel_y = NULL;
    
    focused_prey = NULL;
    for(unsigned char e = 0; e < N_MOB_EVENTS; e++) events_queued[e] = 0;
    events_queued[MOB_EVENT_SPAWN] = 1;
    
    timer = timer_interval = 0;
    script_wait = 0;
    script_wait_event = NULL;
    dead = false;
    state = MOB_STATE_IDLE;
    time_in_state = 0;
    
    following_party = NULL;
    was_thrown = false;
    unwhistlable_period = 0;
    untouchable_period = 0;
    party = NULL;
    
    carrier_info = NULL;
}

void mob::tick() {
    //Movement.
    bool was_airborne = z > sec->floors[0].z;
    x += delta_t* speed_x;
    y += delta_t* speed_y;
    z += delta_t* speed_z;
    
    if(z <= sec->floors[0].z) {
        z = sec->floors[0].z;
        if(was_airborne) {
            speed_x = 0;
            speed_y = 0;
            speed_z = 0;
            was_thrown = false;
        }
    }
    
    //Gravity.
    if(z > sec->floors[0].z && affected_by_gravity) {
        speed_z += delta_t* (GRAVITY_ADDER);
    }
    
    //Chasing a target.
    if(go_to_target && ((speed_z == 0 && knockdown_period == 0) || gtt_instant)) {
        float final_target_x = target_x, final_target_y = target_y;
        if(target_rel_x) final_target_x += *target_rel_x;
        if(target_rel_y) final_target_y += *target_rel_y;
        
        if(gtt_instant) {
        
            x = final_target_x;
            y = final_target_y;
            speed_x = speed_y = 0;
            
        } else if(x != final_target_x || y != final_target_y) {
            float new_angle = angle;
            move_point(x, y, final_target_x, final_target_y, get_base_speed(), 0.001, &speed_x, &speed_y, &new_angle, &reached_destination);
            if(!reached_destination) {
                //Only face the way the mob wants to go if it's still going. Otherwise, let other code turn them whichever way it wants.
                face(new_angle);
            }
        } else reached_destination = true;
    }
    
    //ToDo collisions
    
    //Other things.
    if(unwhistlable_period > 0) {
        unwhistlable_period -= delta_t;
        unwhistlable_period = max(unwhistlable_period, 0);
    }
    if(untouchable_period > 0) {
        untouchable_period -= delta_t;
        untouchable_period = max(untouchable_period, 0);
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
        for(size_t m = 0; m < n_members; m++) {
            party->members[m]->face(atan2(y - party->members[m]->y, x - party->members[m]->x));
        }
    }
    
    time_in_state += delta_t;
    
    if(invuln_period > 0) {
        invuln_period -= delta_t;
        invuln_period = max(invuln_period, 0);
    }
    
    if(speed_z == 0) {
        if(knockdown_period > 0) {
            knockdown_period -= delta_t;
            knockdown_period = max(knockdown_period, 0);
        }
    }
    
    
    //Change the facing angle to the angle the mob wants to face.
    if(angle > M_PI)  angle -= M_PI * 2;
    if(angle < -M_PI) angle += M_PI * 2;
    if(intended_angle > M_PI)  intended_angle -= M_PI * 2;
    if(intended_angle < -M_PI) intended_angle += M_PI * 2;
    
    float angle_dif = intended_angle - angle;
    if(angle_dif > M_PI)  angle_dif -= M_PI * 2;
    if(angle_dif < -M_PI) angle_dif += M_PI * 2;
    
    angle += sign(angle_dif) * min(type->rotation_speed * delta_t, fabs(angle_dif));
    
    //Scripts.
    if(
        get_mob_event(this, MOB_EVENT_SEE_PREY, true) ||
        get_mob_event(this, MOB_EVENT_LOSE_PREY, true) ||
        get_mob_event(this, MOB_EVENT_NEAR_PREY, true)
    ) {
        mob* actual_prey = NULL;
        if(focused_prey) if(!focused_prey->dead) actual_prey = focused_prey;
        
        if(actual_prey) {
            float d = dist(x, y, actual_prey->x, actual_prey->y);
            
            //Prey is near.
            if(d <= type->near_radius && script_wait == 0) {
                focused_prey_near = true;
                events_queued[MOB_EVENT_SEE_PREY] = 0;
                events_queued[MOB_EVENT_LOSE_PREY] = 0;
                events_queued[MOB_EVENT_NEAR_PREY] = 2;
            }
            
            //Prey is suddenly out of sight.
            if(d > type->sight_radius) {
                unfocus_mob(this, actual_prey, true);
                
            } else {
            
                //Prey was near, but is now far.
                if(focused_prey_near) {
                    if( d > type->near_radius) {
                        focused_prey_near = false;
                        events_queued[MOB_EVENT_NEAR_PREY] = 0;
                        events_queued[MOB_EVENT_LOSE_PREY] = 0;
                        events_queued[MOB_EVENT_SEE_PREY] = 1;
                    }
                }
            }
            
        } else {
        
            //Find a Pikmin.
            if(!actual_prey) {
                size_t n_pikmin = pikmin_list.size();
                for(size_t p = 0; p < n_pikmin; p++) {
                    pikmin* pik_ptr = pikmin_list[p];
                    if(pik_ptr->dead) continue;
                    
                    float d = dist(x, y, pik_ptr->x, pik_ptr->y);
                    if(d <= type->sight_radius) {
                        focus_mob(this, pik_ptr, d < type->near_radius, true);
                        break;
                    }
                }
            }
            
            if(!focused_prey) {
                //Try the captains now.
                size_t n_leaders = leaders.size();
                for(size_t l = 0; l < n_leaders; l++) {
                    leader* leader_ptr = leaders[l];
                    if(leader_ptr->dead) continue;
                    
                    float d = dist(x, y, leader_ptr->x, leader_ptr->y);
                    if(d <= type->sight_radius) {
                        focus_mob(this, leader_ptr, d < type->near_radius, true);
                        break;
                    }
                }
            }
        }
        
    }
    
    if(get_mob_event(this, MOB_EVENT_TIMER, true)) {
        if(timer > 0 && timer_interval > 0) {
            timer -= delta_t;
            if(timer <= 0) {
                timer = timer_interval;
                events_queued[MOB_EVENT_TIMER] = 1;
            }
        }
    }
    
    if(get_mob_event(this, MOB_EVENT_REACH_HOME, true)) {
        if(reached_destination && target_code == MOB_TARGET_HOME) {
            target_code = MOB_TARGET_NONE;
            events_queued[MOB_EVENT_REACH_HOME] = 1;
        }
    }
    
    if(!dead && health <= 0) {
        dead = true;
        if(get_mob_event(this, MOB_EVENT_DEATH, true)) {
            events_queued[MOB_EVENT_DEATH] = 1;
        }
    }
    
    //Actuall run the scripts, if possible.
    bool ran_event = false;
    for(unsigned char e = 0; e < N_MOB_EVENTS; e++) {
        if(events_queued[e] == 1) {
            mob_event* ev_ptr = get_mob_event(this, e);
            if(ev_ptr) {
                ev_ptr->run(this, 0);
                ran_event = true;
                events_queued[e] = 0;
            }
        }
    }
    if(!ran_event) { //Try the low priority ones now.
        for(unsigned char e = 0; e < N_MOB_EVENTS; e++) {
            if(events_queued[e] == 2) {
                mob_event* ev_ptr = get_mob_event(this, e);
                if(ev_ptr) {
                    ev_ptr->run(this, 0);
                    events_queued[e] = 0;
                }
            }
        }
    } else {
        for(unsigned char e = 0; e < N_MOB_EVENTS; e++) {
            if(events_queued[e] == 2) events_queued[e] = 0;
        }
    }
    
    if(script_wait > 0) {
        script_wait -= delta_t;
        if(script_wait <= 0) {
            script_wait = 0;
            
            script_wait_event->run(this, script_wait_action); //Continue the waiting event.
        }
    }
    
    //Animation.
    bool finished_anim = anim.tick(delta_t);
    
    if(script_wait == -1 && finished_anim) { //Waiting for the animation to end.
        script_wait = 0;
        script_wait_event->run(this, script_wait_action); //Continue the waiting event.
    }
}

void mob::set_target(float target_x, float target_y, float* target_rel_x, float* target_rel_y, bool instant) {
    this->target_x = target_x; this->target_y = target_y;
    this->target_rel_x = target_rel_x; this->target_rel_y = target_rel_y;
    this->gtt_instant = instant;
    
    go_to_target = true;
    reached_destination = false;
    target_code = MOB_TARGET_NONE;
}

void mob::remove_target(bool stop) {
    go_to_target = false;
    reached_destination = false;
    
    if(stop) {
        speed_x = 0;
        speed_y = 0;
    }
}

void mob::face(float new_angle) {
    intended_angle = new_angle;
}

void mob::set_state(unsigned char new_state) {
    state = new_state;
    time_in_state = 0;
}

float mob::get_base_speed() {
    return this->type->move_speed;
}

mob::~mob() {}

carrier_info_struct::carrier_info_struct(mob* m, unsigned int max_carriers, bool carry_to_ship) {
    this->max_carriers = max_carriers;
    this->carry_to_ship = carry_to_ship;
    
    current_carrying_strength = 0;
    current_n_carriers = 0;
    decided_type = NULL;
    
    for(size_t c = 0; c < max_carriers; c++) {
        carrier_spots.push_back(NULL);
        float angle = (M_PI * 2) / max_carriers * c;
        carrier_spots_x.push_back(cos(angle) * m->type->size * 0.5);
        carrier_spots_y.push_back(sin(angle) * m->type->size * 0.5);
    }
}

carrier_info_struct::~carrier_info_struct() {
    for(size_t s = 0; s < max_carriers; s++) {
        if(carrier_spots[s]) {
            if(typeid(*carrier_spots[s]) == typeid(pikmin)) {
                drop_mob((pikmin*) carrier_spots[s]);
            }
        }
    }
}
