#include "const.h"
#include "functions.h"
#include "mob.h"
#include "vars.h"

mob::mob(float x, float y, float z, float move_speed, sector* sec) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->sec = sec;
    
    to_delete = false;
    reached_destination = false;
    
    speed_x = speed_y = speed_z = 0;
    this->move_speed = move_speed;
    angle = 0;
    size = 1;
    
    go_to_target = false;
    gtt_instant = false;
    target_x = x;
    target_y = y;
    target_rel_x = NULL;
    target_rel_y = NULL;
    
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
    if(speed_z != 0) {
        speed_z += (1.0f / game_fps) * (GRAVITY_ADDER);
    }
    
    //Automated movement.
    if(go_to_target && speed_z == 0) {
        float final_target_x = target_x, final_target_y = target_y;
        if(target_rel_x) final_target_x += *target_rel_x;
        if(target_rel_y) final_target_y += *target_rel_y;
        
        if(gtt_instant) {
        
            x = final_target_x;
            y = final_target_y;
            speed_x = speed_y = 0;
            
        } else if(x != final_target_x || y != final_target_y) {
        
            move_point(x, y, final_target_x, final_target_y, move_speed, 0.001, &speed_x, &speed_y, &angle, &reached_destination);
            
            /*float dx = final_target_x - x, dy = final_target_y - y;
            if(fabs(dx) < 0.001) {
                dx = 0;
                x = final_target_x;
            }
            if(fabs(dy) < 0.001) {
                y = final_target_y;
                dy = 0;
            }
            float dist = sqrt(dx * dx + dy * dy);
            
            if(dist > 0) {
                float move_amount = min(dist * game_fps / 2, move_speed);
            
                dx *= move_amount / dist;
                dy *= move_amount / dist;
            
                speed_x = dx;
                speed_y = dy;
                angle = atan2(dy, dx);
            } else {
                reached_destination = true;
            }*/
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
            this->move_speed,
            ((party->party_spots->current_wheel + 1) * party->party_spots->spot_radius) + (party->party_spots->current_wheel + 1) * PARTY_SPOT_INTERVAL,
            &party_center_mx, &party_center_my, NULL, NULL
        );
        party->party_center_x += party_center_mx * delta_t_mult;
        party->party_center_y += party_center_my * delta_t_mult;
        
        size_t n_members = party->members.size();
        for(size_t m = 0; m < n_members; m++) {
            party->members[m]->angle = atan2(y - party->members[m]->y, x - party->members[m]->x);
        }
    }
}

void mob::set_target(float target_x, float target_y, float* target_rel_x, float* target_rel_y, bool instant) {
    this->target_x = target_x; this->target_y = target_y;
    this->target_rel_x = target_rel_x; this->target_rel_y = target_rel_y;
    this->gtt_instant = instant;
    
    go_to_target = true;
    reached_destination = false;
}

void mob::remove_target(bool stop) {
    go_to_target = false;
    reached_destination = false;
    
    if(stop) {
        speed_x = 0;
        speed_y = 0;
    }
}

mob::~mob() {}

carrier_info_struct::carrier_info_struct(mob* m, unsigned int max_carriers, bool carry_to_ship) {
    this->max_carriers = max_carriers;
    this->carry_to_ship = carry_to_ship;
    
    current_n_carriers = 0;
    decided_type = NULL;
    
    for(size_t c = 0; c < max_carriers; c++) {
        carrier_spots.push_back(NULL);
        float angle = (M_PI * 2) / max_carriers * c;
        carrier_spots_x.push_back(cos(angle) * m->size * 0.5);
        carrier_spots_y.push_back(sin(angle) * m->size * 0.5);
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
