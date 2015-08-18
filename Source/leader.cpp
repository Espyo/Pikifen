/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader class and leader-related functions.
 */

#include "const.h"
#include "functions.h"
#include "leader.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a leader.
 */
leader::leader(const float x, const float y, leader_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    lea_type(type),
    holding_pikmin(nullptr),
    auto_pluck_mode(false),
    auto_pluck_pikmin(nullptr),
    pluck_time(-1),
    is_in_walking_anim(false) {
    
    team = MOB_TEAM_PLAYER_1; //TODO.
    
    party_spot_info* ps = new party_spot_info(max_pikmin_in_field, 12);
    party = new party_info(ps, x, y);
}


/* ----------------------------------------------------------------------------
 * Makes the current leader dismiss their party.
 * The party is then organized in groups, by type,
 * and is dismissed close to the leader.
 */
void dismiss() {
    float base_angle; //They are dismissed towards this angle. This is then offset a bit depending on the Pikmin type, so they spread out.
    
    //TODO what if there are a lot of Pikmin types?
    size_t n_party_members = cur_leader_ptr->party->members.size();
    if(n_party_members == 0) return;
    
    //First, calculate what direction the party should be dismissed to.
    if(group_move_intensity > 0) {
        //If the leader's moving the group, they should be dismissed towards the cursor.
        base_angle = group_move_angle + M_PI;
    } else {
        float min_x = 0, min_y = 0, max_x = 0, max_y = 0; //Leftmost member coordinate, rightmost, etc.
        
        for(size_t m = 0; m < n_party_members; m++) {
            mob* member_ptr = cur_leader_ptr->party->members[m];
            
            if(member_ptr->x < min_x || m == 0) min_x = member_ptr->x;
            if(member_ptr->x > max_x || m == 0) max_x = member_ptr->x;
            if(member_ptr->y < min_y || m == 0) min_y = member_ptr->y;
            if(member_ptr->y > max_y || m == 0) max_y = member_ptr->y;
        }
        
        base_angle =
            atan2(
                ((min_y + max_y) / 2) - cur_leader_ptr->y,
                ((min_x + max_x) / 2) - cur_leader_ptr->x
            ) + M_PI;
    }
    
    //Then, calculate how many Pikmin types there are in the party.
    map<pikmin_type*, float> type_dismiss_angles;
    for(size_t m = 0; m < n_party_members; m++) {
    
        if(typeid(*cur_leader_ptr->party->members[m]) == typeid(pikmin)) {
            pikmin* pikmin_ptr = dynamic_cast<pikmin*>(cur_leader_ptr->party->members[m]);
            
            type_dismiss_angles[pikmin_ptr->pik_type] = 0;
        }
    }
    
    //For each type, calculate the angle;
    size_t n_types = type_dismiss_angles.size();
    if(n_types == 1) {
        //Small hack. If there's only one Pikmin type, dismiss them directly towards the base angle.
        type_dismiss_angles.begin()->second = M_PI_4;
    } else {
        unsigned current_type_nr = 0;
        for(auto t = type_dismiss_angles.begin(); t != type_dismiss_angles.end(); t++) {
            t->second = current_type_nr * (M_PI_2 / (n_types - 1));
            current_type_nr++;
        }
    }
    
    //Now, dismiss them.
    for(size_t m = 0; m < n_party_members; m++) {
        mob* member_ptr = cur_leader_ptr->party->members[0];
        remove_from_party(member_ptr);
        
        float angle = 0;
        
        if(typeid(*member_ptr) == typeid(pikmin)) {
            pikmin* pikmin_ptr = dynamic_cast<pikmin*>(member_ptr);
            
            angle = base_angle + type_dismiss_angles[pikmin_ptr->pik_type] - M_PI_4 + M_PI;
            pikmin_ptr->fsm.run_event(MOB_EVENT_DISMISSED, (void*) &angle);
            
        }
    }
    
    sfx_pikmin_idle.play(0, false);
    cur_leader_ptr->lea_type->sfx_dismiss.play(0, false);
    cur_leader_ptr->anim.change(LEADER_ANIM_DISMISS, true, false, false);
}


/* ----------------------------------------------------------------------------
 * Returns the distance between a leader and the center of its group.
 */
float get_leader_to_group_center_dist(mob* l) {
    return
        (l->party->party_spots->current_wheel + 1) *
        l->party->party_spots->spot_radius +
        (l->party->party_spots->current_wheel + 1) *
        PARTY_SPOT_INTERVAL;
}


void leader::whistle(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    
    l_ptr->lea_type->sfx_whistle.play(0, false);
    
    for(unsigned char d = 0; d < 6; d++) whistle_dot_radius[d] = -1;
    whistle_fade_time = 0;
    whistle_fade_radius = 0;
    whistling = true;
    l_ptr->lea_type->sfx_whistle.play(0, false);
    l_ptr->anim.change(LEADER_ANIM_WHISTLING, true, true, false);
}

void leader::stop_whistle(mob* m, void* info1, void* info2) {
    if(!whistling) return;
    
    ((leader*) m)->lea_type->sfx_whistle.stop();
    
    whistle_fade_time = WHISTLE_FADE_TIME;
    whistle_fade_radius = whistle_radius;
    
    whistling = false;
    whistle_radius = 0;
    
}

void leader::join_group(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    
    add_to_party(cur_leader_ptr, l_ptr);
    size_t n_party_members = l_ptr->party->members.size();
    for(size_t m = 0; m < n_party_members; m++) {
        mob* member = l_ptr->party->members[0];
        remove_from_party(member);
        add_to_party(cur_leader_ptr, member);
    }
}

void leader::focus(mob* m, void* info1, void* info2) {
    //TODO
    m->speed_x = 0;
    m->speed_y = 0;
    m->remove_target(true);
    ((leader*) m)->lea_type->sfx_name_call.play(0, false);
}

void leader::enter_idle(mob* m, void* info1, void* info2) {
    m->anim.change(LEADER_ANIM_IDLE, true, false, false);
}

void leader::enter_active(mob* m, void* info1, void* info2) {
    ((leader*) m)->is_in_walking_anim = false;
    m->anim.change(LEADER_ANIM_IDLE, true, false, false);
}

void leader::unfocus(mob* m, void* info1, void* info2) {
    //TODO
    
}

void leader::move(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    movement_struct* mov = (movement_struct*) info1;
    l_ptr->set_target(
        l_ptr->x + mov->get_x() * l_ptr->type->move_speed,
        l_ptr->y + mov->get_y() * l_ptr->type->move_speed,
        NULL, NULL, false, NULL, true
    );
}

void leader::stop(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    l_ptr->remove_target(true);
}

void leader::set_walk_anim(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(!l_ptr->is_in_walking_anim) {
        l_ptr->anim.change(LEADER_ANIM_WALK, true, false, false);
        l_ptr->is_in_walking_anim = true;
    }
}

void leader::set_stop_anim(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(l_ptr->is_in_walking_anim) {
        l_ptr->anim.change(LEADER_ANIM_IDLE, true, false, false);
        l_ptr->is_in_walking_anim = false;
    }
}

void leader::grab_mob(mob* m, void* info1, void* info2) {
    ((leader*) m)->holding_pikmin = (mob*) info1;
    
}

void leader::do_throw(mob* m, void* info1, void* info2) {
    leader* leader_ptr = (leader*) m;
    mob* holding_ptr = leader_ptr->holding_pikmin;
    
    holding_ptr->fsm.run_event(MOB_EVENT_THROWN);
    
    holding_ptr->x = leader_ptr->x;
    holding_ptr->y = leader_ptr->y;
    holding_ptr->z = leader_ptr->z;
    
    float angle, d;
    coordinates_to_angle(cursor_x - leader_ptr->x, cursor_y - leader_ptr->y, &angle, &d);
    
    float throw_height_mult = 1.0;
    if(typeid(*holding_ptr) == typeid(pikmin)) {
        throw_height_mult = ((pikmin*) holding_ptr)->pik_type->throw_height_mult;
    }
    
    //This results in a 1.3 second throw, just like in Pikmin 2. Regular Pikmin are thrown about 288.88 units high.
    holding_ptr->speed_x =
        cos(angle) * d * THROW_DISTANCE_MULTIPLIER * (1.0 / (THROW_STRENGTH_MULTIPLIER * throw_height_mult));
    holding_ptr->speed_y =
        sin(angle) * d * THROW_DISTANCE_MULTIPLIER * (1.0 / (THROW_STRENGTH_MULTIPLIER * throw_height_mult));
    holding_ptr->speed_z =
        -(GRAVITY_ADDER) * (THROW_STRENGTH_MULTIPLIER * throw_height_mult);
        
    holding_ptr->angle = angle;
    holding_ptr->face(angle);
    
    holding_ptr->was_thrown = true;
    
    remove_from_party(holding_ptr);
    leader_ptr->holding_pikmin = NULL;
    
    sfx_throw.stop();
    sfx_throw.play(0, false);
    leader_ptr->anim.change(LEADER_ANIM_THROW, true, false, false);
}

void leader::release(mob* m, void* info1, void* info2) {

}

void leader::get_hurt(mob* m, void* info1, void* info2) {
    //TODO
    m->health -= 2;
}

void leader::die(mob* m, void* info1, void* info2) {
    //TODO
}

void leader::dismiss(mob* m, void* info1, void* info2) {
    //TODO
    
    leader* l_ptr = (leader*) m;
    
    float base_angle; //They are dismissed towards this angle. This is then offset a bit depending on the Pikmin type, so they spread out.
    
    //TODO what if there are a lot of Pikmin types?
    size_t n_party_members = l_ptr->party->members.size();
    if(n_party_members == 0) return;
    
    //First, calculate what direction the party should be dismissed to.
    if(group_move_intensity > 0) {
        //If the leader's moving the group, they should be dismissed towards the cursor.
        base_angle = group_move_angle + M_PI;
    } else {
        float min_x = 0, min_y = 0, max_x = 0, max_y = 0; //Leftmost member coordinate, rightmost, etc.
        
        for(size_t m = 0; m < n_party_members; m++) {
            mob* member_ptr = l_ptr->party->members[m];
            
            if(member_ptr->x < min_x || m == 0) min_x = member_ptr->x;
            if(member_ptr->x > max_x || m == 0) max_x = member_ptr->x;
            if(member_ptr->y < min_y || m == 0) min_y = member_ptr->y;
            if(member_ptr->y > max_y || m == 0) max_y = member_ptr->y;
        }
        
        base_angle =
            atan2(
                ((min_y + max_y) / 2) - l_ptr->y,
                ((min_x + max_x) / 2) - l_ptr->x
            ) + M_PI;
    }
    
    //Then, calculate how many Pikmin types there are in the party.
    map<pikmin_type*, float> type_angle_map;
    for(size_t m = 0; m < n_party_members; m++) {
    
        if(typeid(*l_ptr->party->members[m]) == typeid(pikmin)) {
            pikmin* pikmin_ptr = dynamic_cast<pikmin*>(l_ptr->party->members[m]);
            
            type_angle_map[pikmin_ptr->pik_type] = 0;
        }
    }
    
    //For each type, calculate the angle;
    size_t n_types = type_angle_map.size();
    if(n_types == 1) {
        //Small hack. If there's only one Pikmin type, dismiss them directly towards the base angle.
        type_angle_map.begin()->second = M_PI_4;
        
    } else {
        unsigned current_type_nr = 0;
        for(auto t = type_angle_map.begin(); t != type_angle_map.end(); t++) {
            t->second = current_type_nr * (M_PI_2 / (n_types - 1));
            current_type_nr++;
        }
    }
    
    //Now, dismiss them.
    for(size_t m = 0; m < n_party_members; m++) {
        mob* member_ptr = l_ptr->party->members[0];
        remove_from_party(member_ptr);
        
        float angle = 0;
        
        if(typeid(*member_ptr) == typeid(pikmin)) {
            angle = base_angle + type_angle_map[((pikmin*) member_ptr)->pik_type] - M_PI_4 + M_PI;
        }
        
        member_ptr->fsm.run_event(MOB_EVENT_DISMISSED, (void*) &angle);
    }
    
    sfx_pikmin_idle.play(0, false); //TODO move to Pikmin FSM.
    l_ptr->lea_type->sfx_dismiss.play(0, false);
    l_ptr->anim.change(LEADER_ANIM_DISMISS, true, false, false);
}

void leader::spray(mob* m, void* info1, void* info2) {
    //TODO
    size_t spray_nr = *((size_t*) info1);
    
    if(spray_amounts[spray_nr] == 0) return;
    
    float shoot_angle = cursor_angle + ((spray_types[spray_nr].burpable) ? M_PI : 0);
    
    random_particle_spray(
        PARTICLE_TYPE_BITMAP,
        bmp_smoke,
        m->x + cos(shoot_angle) * m->type->radius,
        m->y + sin(shoot_angle) * m->type->radius,
        shoot_angle,
        spray_types[spray_nr].main_color
    );
    
    spray_amounts[spray_nr]--;
    
    m->anim.change(LEADER_ANIM_DISMISS, true, false, false); //TODO have one specifically for spraying.
    m->fsm.set_state(LEADER_STATE_SPRAYING);
}

void leader::pain(mob* m, void* info1, void* info2) {
    //TODO
}

void leader::get_knocked_back(mob* m, void* info1, void* info2) {
    //TODO
}

void leader::sleep(mob* m, void* info1, void* info2) {
    leader::dismiss(m, info1, info2);
    
    m->carrier_info = new carrier_info_struct(
        m,
        3, //TODO
        false);
        
    m->anim.change(LEADER_ANIM_LIE, true, false, false);
}

void leader::chase_leader(mob* m, void* info1, void* info2) {
    m->set_target(0, 0, &m->following_party->x, &m->following_party->y, false);
    m->anim.change(LEADER_ANIM_WALK, true, false, false);
    focus_mob(m, m->following_party);
}

void leader::stop_in_group(mob* m, void* info1, void* info2) {
    m->remove_target(true);
    m->anim.change(LEADER_ANIM_IDLE, true, false, false);
}

void leader::be_dismissed(mob* m, void* info1, void* info2) {
    m->remove_target(true);
    m->anim.change(LEADER_ANIM_IDLE, true, false, false);
}

void leader::go_pluck(mob* m, void* info1, void* info2) {
    leader* lea_ptr = (leader*) m;
    pikmin* pik_ptr = (pikmin*) info1;
    
    lea_ptr->auto_pluck_pikmin = pik_ptr;
    lea_ptr->pluck_time = -1;
    lea_ptr->set_target(
        pik_ptr->x, pik_ptr->y,
        NULL, NULL,
        false, nullptr, false,
        pik_ptr->type->radius + lea_ptr->type->radius
    );
    pik_ptr->pluck_reserved = true;
    
    //Now for the leaders in the party.
    for(size_t m = 0; m < lea_ptr->party->members.size(); m++) {
        mob* member_ptr = lea_ptr->party->members[m];
        if(typeid(*member_ptr) == typeid(leader)) {
            member_ptr->fsm.run_event(LEADER_EVENT_INACTIVE_SEARCH_SEED);
        }
    }
}

void leader::start_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    l_ptr->auto_pluck_pikmin->fsm.run_event(MOB_EVENT_PLUCKED, (void*) l_ptr);
    l_ptr->auto_pluck_pikmin = nullptr;
    l_ptr->anim.change(LEADER_ANIM_PLUCK, true, false, false);
}

void leader::stop_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(l_ptr->auto_pluck_pikmin) {
        l_ptr->remove_target(true);
        l_ptr->auto_pluck_pikmin->pluck_reserved = false;
    }
    l_ptr->auto_pluck_mode = false;
    l_ptr->auto_pluck_pikmin = NULL;
    l_ptr->anim.change(LEADER_ANIM_IDLE, true, false, false);
}

void leader::search_seed(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    dist d;
    pikmin* new_pikmin = get_closest_buried_pikmin(l_ptr->x, l_ptr->y, &d, false);
    
    //If info1 is not void, that means this is an inactive leader.
    if(info1) {
        l_ptr->fsm.set_state(LEADER_STATE_IN_GROUP_CHASING);
    } else {
        l_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    }
    
    if(new_pikmin && d <= AUTO_PLUCK_MAX_RADIUS) {
        l_ptr->fsm.run_event(LEADER_EVENT_GO_PLUCK, (void*) new_pikmin);
    }
}

void leader::inactive_search_seed(mob* m, void* info1, void* info2) {
    int a = 0; //Dummy value.
    leader::search_seed(m, &a, NULL);
}