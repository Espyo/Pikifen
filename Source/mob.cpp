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
    rotation_speed = M_PI * 2; //ToDo should this be here, in order to give the rotation speed a default value?
    angle = intended_angle = 0;
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
            move_point(x, y, final_target_x, final_target_y, move_speed, 0.001, &speed_x, &speed_y, &new_angle, &reached_destination);
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
            this->move_speed,
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
    
    
    //Change the facing angle to the angle the mob wants to face.
    if(angle > M_PI)  angle -= M_PI * 2;
    if(angle < -M_PI) angle += M_PI * 2;
    if(intended_angle > M_PI)  intended_angle -= M_PI * 2;
    if(intended_angle < -M_PI) intended_angle += M_PI * 2;
    
    float angle_dif = intended_angle - angle;
    if(angle_dif > M_PI)  angle_dif -= M_PI * 2;
    if(angle_dif < -M_PI) angle_dif += M_PI * 2;
    
    angle += sign(angle_dif) * min(rotation_speed / game_fps, fabs(angle_dif));
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

void mob::face(float new_angle) {
    intended_angle = new_angle;
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
