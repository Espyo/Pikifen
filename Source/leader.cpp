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
    auto_pluck_pikmin(nullptr),
    is_in_walking_anim(false) {
    
    team = MOB_TEAM_PLAYER_1; //TODO.
    invuln_period = timer(LEADER_INVULN_PERIOD);
    
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
        
        for(size_t m = 0; m < n_party_members; ++m) {
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
    for(size_t m = 0; m < n_party_members; ++m) {
    
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
        for(auto t = type_dismiss_angles.begin(); t != type_dismiss_angles.end(); ++t) {
            t->second = current_type_nr * (M_PI_2 / (n_types - 1));
            current_type_nr++;
        }
    }
    
    //Now, dismiss them.
    for(size_t m = 0; m < n_party_members; ++m) {
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
    cur_leader_ptr->set_animation(LEADER_ANIM_DISMISS);
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
    
    for(unsigned char d = 0; d < 6; ++d) whistle_dot_radius[d] = -1;
    whistle_fade_timer.start();
    whistle_fade_radius = 0;
    whistling = true;
    l_ptr->lea_type->sfx_whistle.play(0, false);
    l_ptr->set_animation(LEADER_ANIM_WHISTLING);
    l_ptr->script_timer.start(2.5f);
}

void leader::stop_whistle(mob* m, void* info1, void* info2) {
    if(!whistling) return;
    
    ((leader*) m)->lea_type->sfx_whistle.stop();
    
    whistle_fade_timer.start();
    whistle_fade_radius = whistle_radius;
    
    whistling = false;
    whistle_radius = 0;
    
}

void leader::join_group(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    
    add_to_party(cur_leader_ptr, l_ptr);
    size_t n_party_members = l_ptr->party->members.size();
    for(size_t m = 0; m < n_party_members; ++m) {
        mob* member = l_ptr->party->members[0];
        remove_from_party(member);
        add_to_party(cur_leader_ptr, member);
    }
}

void leader::focus(mob* m, void* info1, void* info2) {
    switch_to_leader((leader*) m);
    
    m->speed_x = 0;
    m->speed_y = 0;
    m->remove_target();
    ((leader*) m)->lea_type->sfx_name_call.play(0, false);
    
}

void leader::enter_idle(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_IDLE);
}

void leader::enter_active(mob* m, void* info1, void* info2) {
    ((leader*) m)->is_in_walking_anim = false;
    m->set_animation(LEADER_ANIM_IDLE);
}

void leader::unfocus(mob* m, void* info1, void* info2) {

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
    l_ptr->remove_target();
}

void leader::set_walk_anim(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(!l_ptr->is_in_walking_anim) {
        l_ptr->set_animation(LEADER_ANIM_WALK);
        l_ptr->is_in_walking_anim = true;
    }
}

void leader::set_stop_anim(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(l_ptr->is_in_walking_anim) {
        l_ptr->set_animation(LEADER_ANIM_IDLE);
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
    leader_ptr->set_animation(LEADER_ANIM_THROW);
}

void leader::release(mob* m, void* info1, void* info2) {
    cur_leader_ptr->holding_pikmin = NULL;
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
        
        for(size_t m = 0; m < n_party_members; ++m) {
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
    for(size_t m = 0; m < n_party_members; ++m) {
    
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
        for(auto t = type_angle_map.begin(); t != type_angle_map.end(); ++t) {
            t->second = current_type_nr * (M_PI_2 / (n_types - 1));
            current_type_nr++;
        }
    }
    
    //Now, dismiss them.
    for(size_t m = 0; m < n_party_members; ++m) {
        mob* member_ptr = l_ptr->party->members[0];
        remove_from_party(member_ptr);
        
        float angle = 0;
        
        if(typeid(*member_ptr) == typeid(pikmin)) {
            angle = base_angle + type_angle_map[((pikmin*) member_ptr)->pik_type] - M_PI_4 + M_PI;
        }
        
        float x = cur_leader_ptr->x + cos(angle) * DISMISS_DISTANCE;
        float y = cur_leader_ptr->y + sin(angle) * DISMISS_DISTANCE;
        
        member_ptr->fsm.run_event(MOB_EVENT_DISMISSED, (void*) &x, (void*) &y);
    }
    
    l_ptr->lea_type->sfx_dismiss.play(0, false);
    l_ptr->set_animation(LEADER_ANIM_DISMISS);
}

void leader::spray(mob* m, void* info1, void* info2) {
    m->remove_target();
    size_t spray_nr = *((size_t*) info1);
    
    if(spray_amounts[spray_nr] == 0) {
        m->fsm.set_state(LEADER_STATE_ACTIVE);
        return;
    }
    
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
    
    m->set_animation(LEADER_ANIM_SPRAYING);
}

void leader::lose_health(mob* m, void* info1, void* info2) {
    //TODO
    
    if(!m->invuln_period.ticked) return;
    m->invuln_period.start();
    
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    float damage = 0;
    float knockback = 0;
    float knockback_angle = 0;
    
    damage = calculate_damage(info->mob2, m, info->hi2, info->hi1);
    calculate_knockback(info->mob2, m, info->hi2, info->hi1, &knockback, &knockback_angle);
    
    m->health -= damage;
    apply_knockback(m, knockback, knockback_angle);
    
    //If info2 has a value, then this leader is inactive.
    if(knockback > 0 && damage == 0) {
        if(info2)
            m->fsm.set_state(LEADER_STATE_INACTIVE_KNOCKED_BACK);
        else
            m->fsm.set_state(LEADER_STATE_KNOCKED_BACK);
    } else {
        if(info2)
            m->fsm.set_state(LEADER_STATE_INACTIVE_PAIN);
        else
            m->fsm.set_state(LEADER_STATE_PAIN);
    }
}

void leader::inactive_lose_health(mob* m, void* info1, void* info2) {
    int a = 0;
    leader::lose_health(m, info1, &a);
    //We need to send the function a value so it knows
    //it's an inactive leader.
}

void leader::die(mob* m, void* info1, void* info2) {
    //TODO
}

void leader::inactive_die(mob* m, void* info1, void* info2) {
    //TODO
}

void leader::suffer_pain(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_PAIN);
    m->remove_target();
}

void leader::get_knocked_back(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_KNOCKED_DOWN);
}

void leader::fall_asleep(mob* m, void* info1, void* info2) {
    leader::dismiss(m, info1, info2);
    m->remove_target();
    
    m->carrier_info = new carrier_info_struct(
        m,
        3, //TODO
        false);
        
    m->set_animation(LEADER_ANIM_LIE);
}

void leader::start_waking_up(mob* m, void* info1, void* info2) {
    make_uncarriable(m);
    cur_leader_ptr->set_animation(LEADER_ANIM_GET_UP);
}

void leader::chase_leader(mob* m, void* info1, void* info2) {
    m->set_target(0, 0, &m->following_party->x, &m->following_party->y, false);
    m->set_animation(LEADER_ANIM_WALK);
    focus_mob(m, m->following_party);
}

void leader::stop_in_group(mob* m, void* info1, void* info2) {
    m->remove_target();
    m->set_animation(LEADER_ANIM_IDLE);
}

void leader::be_dismissed(mob* m, void* info1, void* info2) {
    m->remove_target();
    m->set_animation(LEADER_ANIM_IDLE);
}

void leader::go_pluck(mob* m, void* info1, void* info2) {
    leader* lea_ptr = (leader*) m;
    pikmin* pik_ptr = (pikmin*) info1;
    
    lea_ptr->auto_pluck_pikmin = pik_ptr;
    lea_ptr->set_target(
        pik_ptr->x, pik_ptr->y,
        NULL, NULL,
        false, nullptr, false,
        pik_ptr->type->radius + lea_ptr->type->radius
    );
    pik_ptr->pluck_reserved = true;
    
    //Now for the leaders in the party.
    for(size_t m = 0; m < lea_ptr->party->members.size(); ++m) {
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
    l_ptr->set_animation(LEADER_ANIM_PLUCK);
}

void leader::stop_pluck(mob* m, void* info1, void* info2) {
    leader* l_ptr = (leader*) m;
    if(l_ptr->auto_pluck_pikmin) {
        l_ptr->remove_target();
        l_ptr->auto_pluck_pikmin->pluck_reserved = false;
    }
    l_ptr->auto_pluck_pikmin = NULL;
    l_ptr->set_animation(LEADER_ANIM_IDLE);
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

void leader::be_grabbed_by_friend(mob* m, void* info1, void* info2) {
    m->set_animation(LEADER_ANIM_IDLE);
}

void leader::be_released(mob* m, void* info1, void* info2) {

}

void leader::be_thrown(mob* m, void* info1, void* info2) {

}

void leader::land(mob* m, void* info1, void* info2) {

}

void swap_pikmin(mob* new_pik) {
    leader* lea = cur_leader_ptr;
    if(lea->holding_pikmin) {
        lea->holding_pikmin->fsm.run_event(MOB_EVENT_RELEASED);
    }
    lea->holding_pikmin = new_pik;
    new_pik->fsm.run_event(MOB_EVENT_GRABBED_BY_FRIEND);
    
    sfx_switch_pikmin.play(0, false);
}

void switch_to_leader(leader* new_leader_ptr) {

    cur_leader_ptr->fsm.run_event(LEADER_EVENT_UNFOCUSED);
    
    size_t new_leader_nr = cur_leader_nr;
    for(size_t l = 0; l < leaders.size(); ++l) {
        if(leaders[l] == new_leader_ptr) {
            new_leader_nr = l;
            break;
        }
    }
    
    cur_leader_ptr = new_leader_ptr;
    cur_leader_nr = new_leader_nr;
    
    start_camera_pan(cur_leader_ptr->x, cur_leader_ptr->y);
    
}

void leader::draw() {
    mob::draw();
    
    frame* f_ptr = anim.get_frame();
    float draw_x, draw_y;
    float draw_w, draw_h;
    get_sprite_center(this, f_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, f_ptr, &draw_w, &draw_h);
    
    if(invuln_period.time_left > 0) {
        unsigned char anim_part = invuln_period.get_ratio_left() * LEADER_ZAP_ANIM_PARTS;
        float zap_x[4], zap_y[4];
        for(unsigned char p = 0; p < 4; ++p) {
            if(anim_part % 2 == 0) {
                zap_x[p] = draw_x + draw_w * (deterministic_random(anim_part * 3 + p) - 0.5);
                zap_y[p] = draw_y + draw_h * 0.5 * ((p % 2 == 0) ? 1 : -1);
            } else {
                zap_x[p] = draw_x + draw_w * 0.5 * ((p % 2 == 0) ? 1 : -1);
                zap_y[p] = draw_y + draw_h * (deterministic_random(anim_part * 3 + p) - 0.5);
            }
        }
        
        static const ALLEGRO_COLOR LEADER_ZAP_COLOR = al_map_rgba(128, 255, 255, 128);
        
        al_draw_line(
            zap_x[0], zap_y[0], zap_x[1], zap_y[1],
            LEADER_ZAP_COLOR,
            2.0f
        );
        al_draw_line(
            zap_x[1], zap_y[1], zap_x[2], zap_y[2],
            LEADER_ZAP_COLOR,
            2.0f
        );
        al_draw_line(
            zap_x[2], zap_y[2], zap_x[3], zap_y[3],
            LEADER_ZAP_COLOR,
            2.0f
        );
        
    }
}


/* ----------------------------------------------------------------------------
 * Signals the party members that the group move mode stopped.
 */
void leader::signal_group_move_end() {
    for(size_t m = 0; m < party->members.size(); ++m) {
        party->members[m]->fsm.run_event(MOB_EVENT_GROUP_MOVE_ENDED);
    }
}


/* ----------------------------------------------------------------------------
 * Signals the party members that the group move mode started.
 */
void leader::signal_group_move_start() {
    for(size_t m = 0; m < party->members.size(); ++m) {
        party->members[m]->fsm.run_event(MOB_EVENT_GROUP_MOVE_STARTED);
    }
}
