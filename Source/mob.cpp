#include <algorithm>

#include "const.h"
#include "functions.h"
#include "mob.h"
#include "pikmin.h"
#include "vars.h"

mob::mob(float x, float y, float z, mob_type* t, sector* sec) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->sec = sec;
    
    type = t;
    
    to_delete = false;
    reached_destination = false;
    
    speed_x = speed_y = speed_z = 0;
    home_x = x; home_y = y;
    angle = intended_angle = 0;
    affected_by_gravity = true;
    
    health = t->max_health;
    team = MOB_TEAM_NONE;
    
    go_to_target = false;
    gtt_instant = false;
    target_x = x;
    target_y = y;
    target_rel_x = NULL;
    target_rel_y = NULL;
    
    focused_prey = NULL;
    timer = timer_interval = 0;
    script_wait = 0;
    script_wait_event = NULL;
    spawn_event_done = false;
    dead = false;
    state = MOB_STATE_IDLE;
    time_in_state = 0;
    
    following_party = NULL;
    was_thrown = false;
    uncallable_period = 0;
    party = NULL;
    
    carrier_info = NULL;
}

void mob::tick() {
    float delta_t_mult = (1.0f / game_fps);
    
    //Movement.
    bool was_airborne = z > sec->floors[0].z;
    x += delta_t_mult * speed_x;
    y += delta_t_mult * speed_y;
    z += delta_t_mult * speed_z;
    
    if(z <= sec->floors[0].z && was_airborne) {
        z = sec->floors[0].z;
        speed_x = 0;
        speed_y = 0;
        speed_z = 0;
        was_thrown = false;
    }
    
    //Gravity.
    if(z > sec->floors[0].z && affected_by_gravity) {
        speed_z += (1.0f / game_fps) * (GRAVITY_ADDER);
    }
    
    //Chasing a target.
    if(go_to_target && speed_z == 0) {
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
        }
    }
    
    //ToDo collisions
    
    //Other things.
    if(uncallable_period > 0) {
        uncallable_period -= (1.0 / game_fps);
        if(uncallable_period < 0) uncallable_period = 0;
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
        party->party_center_x += party_center_mx * delta_t_mult;
        party->party_center_y += party_center_my * delta_t_mult;
        
        size_t n_members = party->members.size();
        for(size_t m = 0; m < n_members; m++) {
            party->members[m]->face(atan2(y - party->members[m]->y, x - party->members[m]->x));
        }
    }
    
    time_in_state += 1.0 / game_fps;
    
    
    //Change the facing angle to the angle the mob wants to face.
    if(angle > M_PI)  angle -= M_PI * 2;
    if(angle < -M_PI) angle += M_PI * 2;
    if(intended_angle > M_PI)  intended_angle -= M_PI * 2;
    if(intended_angle < -M_PI) intended_angle += M_PI * 2;
    
    float angle_dif = intended_angle - angle;
    if(angle_dif > M_PI)  angle_dif -= M_PI * 2;
    if(angle_dif < -M_PI) angle_dif += M_PI * 2;
    
    angle += sign(angle_dif) * min(type->rotation_speed / game_fps, fabs(angle_dif));
    
    //Scripts.
    if(script_wait > 0) {
        script_wait -= 1.0 / game_fps;
        if(script_wait <= 0) {
            script_wait = 0;
            
            script_wait_event->run(this, script_wait_action); //Continue the waiting event.
        }
    }
    
    mob_event* ev_ptr = NULL;
    
    ev_ptr = get_mob_event(this, MOB_EVENT_NEAR_PREY);
    if(ev_ptr) {
        if(focused_prey) {
            if(!focused_prey_near) {
                if(dist(x, y, focused_prey->x, focused_prey->y) <= type->near_radius) {
                    focused_prey_near = true;
                    ev_ptr->run(this, 0);
                }
            }
        }
    }
    
    ev_ptr = get_mob_event(this, MOB_EVENT_SEE_PREY);
    if(ev_ptr) {
        if(focused_prey && focused_prey_near) {
            if(dist(x, y, focused_prey->x, focused_prey->y) > type->near_radius) {
                focused_prey_near = false;
                ev_ptr->run(this, 0);
            }
        }
        
        //Find a Pikmin.
        if(!focused_prey) {
            size_t n_pikmin = pikmin_list.size();
            for(size_t p = 0; p < n_pikmin; p++) {
                pikmin* pik_ptr = pikmin_list[p];
                
                if(dist(x, y, pik_ptr->x, pik_ptr->y) <= type->sight_radius) {
                    focused_prey = pik_ptr;
                    ev_ptr->run(this, 0);
                    break;
                }
            }
        }
        
        if(!focused_prey) {
            //Try the captains now.
            size_t n_leaders = leaders.size();
            for(size_t l = 0; l < n_leaders; l++) {
                leader* leader_ptr = leaders[l];
                
                if(dist(x, y, leader_ptr->x, leader_ptr->y) <= type->sight_radius) {
                    focused_prey = leader_ptr;
                    ev_ptr->run(this, 0);
                    break;
                }
            }
        }
    }
    
    ev_ptr = get_mob_event(this, MOB_EVENT_LOSE_PREY);
    if(ev_ptr) {
        //Lose the Pikmin in focus.
        if(focused_prey) {
            if(dist(x, y, focused_prey->x, focused_prey->y) > type->sight_radius) {
                focused_prey = NULL;
                ev_ptr->run(this, 0);
            }
        }
    }
    
    ev_ptr = get_mob_event(this, MOB_EVENT_TIMER);
    if(ev_ptr && timer_interval > 0) {
        if(timer > 0) {
            timer -= 1.0 / game_fps;
            if(timer <= 0) {
                timer = timer_interval;
                ev_ptr->run(this, 0);
            }
        }
    }
    
    ev_ptr = get_mob_event(this, MOB_EVENT_SPAWN);
    if(ev_ptr && !spawn_event_done) {
        spawn_event_done = true;
        ev_ptr->run(this, 0);
    }
    
    ev_ptr = get_mob_event(this, MOB_EVENT_REACH_HOME);
    if(ev_ptr) {
        if(reached_destination && target_code == MOB_TARGET_HOME) {
            target_code = MOB_TARGET_NONE;
            ev_ptr->run(this, 0);
        }
    }
    
    ev_ptr = get_mob_event(this, MOB_EVENT_DEATH);
    if(ev_ptr && !dead) {
        if(health <= 0) {
            dead = true;
            ev_ptr->run(this, 0);
        }
    }
    
    //Animation.
    bool finished_anim = anim.tick(1.0 / game_fps);
    
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

float mob::get_base_speed() {
    return this->type->move_speed;
}

void mob::set_state(unsigned char new_state) {
    state = new_state;
    time_in_state = 0;
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
