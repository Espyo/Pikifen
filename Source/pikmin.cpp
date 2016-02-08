/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin class and Pikmin-related functions.
 */

#include "drawing.h"
#include "functions.h"
#include "mob.h"
#include "pikmin.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a Pikmin.
 */
pikmin::pikmin(const float x, const float y, pikmin_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    pik_type(type),
    hazard_timer(-1), //TODO the usual time
    attack_time(0),
    pluck_reserved(false),
    carrying_spot(0),
    maturity(s2i(get_var_value(vars, "maturity", "2"))),
    connected_hitbox_nr(0),
    connected_hitbox_dist(0),
    connected_hitbox_angle(0) {
    
    team = MOB_TEAM_PLAYER_1; // TODO
    if(s2b(get_var_value(vars, "buried", "0"))) {
        fsm.set_state(PIKMIN_STATE_BURIED);
        this->first_state_set = true;
    }
}


pikmin::~pikmin() { }


/* ----------------------------------------------------------------------------
 * Returns a Pikmin's base speed, without status effects and the like.
 * This depends on the maturity.
 */
float pikmin::get_base_speed() {
    return pik_type->move_speed + pik_type->move_speed * this->maturity * MATURITY_SPEED_MULT;
}


/* ----------------------------------------------------------------------------
 * Actually makes the Pikmin attack connect - the process that makes
 * the victim lose health, the sound, the sparks, etc.
 * victim_hitbox_i: Hitbox instance of the victim.
 */
void pikmin::do_attack(mob* m, hitbox_instance* victim_hitbox_i) {
    attack_time = pik_type->attack_interval;
    
    hitbox_touch_info info = hitbox_touch_info(this, victim_hitbox_i, NULL);
    focused_mob->fsm.run_event(MOB_EVENT_HITBOX_TOUCH_N_A, &info);
    
    sfx_attack.play(0.06, false, 0.4f);
    sfx_pikmin_attack.play(0.06, false, 0.8f);
    particles.push_back(
        particle(
            PARTICLE_TYPE_SMACK, bmp_smack,
            x, y,
            0, 0, 0, 0,
            SMACK_PARTICLE_DUR,
            64,
            al_map_rgb(255, 160, 128)
        )
    );
}


/* ----------------------------------------------------------------------------
 * Sets the info for when a Pikmin is connected to a hitbox
 * (e.g. latching on, being carried by a mouth, ...)
 * hi_ptr: Hitbox instance of the other mob.
 * m:      The other mob.
 */
void pikmin::set_connected_hitbox_info(hitbox_instance* hi_ptr, mob* mob_ptr) {
    if(!hi_ptr) return;
    
    float actual_hx, actual_hy;
    rotate_point(hi_ptr->x, hi_ptr->y, mob_ptr->angle, &actual_hx, &actual_hy);
    actual_hx += mob_ptr->x; actual_hy += mob_ptr->y;
    
    float x_dif = x - actual_hx;
    float y_dif = y - actual_hy;
    coordinates_to_angle(x_dif, y_dif, &connected_hitbox_angle, &connected_hitbox_dist);
    
    connected_hitbox_angle -= mob_ptr->angle; //Relative to 0 degrees.
    connected_hitbox_dist /= hi_ptr->radius;  //Distance in units to distance in percentage.
    connected_hitbox_nr = hi_ptr->hitbox_nr;
}


/* ----------------------------------------------------------------------------
 * Teleports the Pikmin to the hitbox it is connected to.
 */
void pikmin::teleport_to_connected_hitbox() {
    speed_x = speed_y = speed_z = 0;
    
    hitbox_instance* h_ptr = get_hitbox_instance(focused_mob, connected_hitbox_nr);
    if(h_ptr) {
        float actual_hx, actual_hy;
        rotate_point(h_ptr->x, h_ptr->y, focused_mob->angle, &actual_hx, &actual_hy);
        actual_hx += focused_mob->x; actual_hy += focused_mob->y;
        
        float final_px, final_py;
        angle_to_coordinates(
            connected_hitbox_angle + focused_mob->angle,
            connected_hitbox_dist * h_ptr->radius,
            &final_px, &final_py);
        final_px += actual_hx; final_py += actual_hy;
        
        set_target(final_px, final_py, NULL, NULL, true);
        face(atan2(focused_mob->y - y, focused_mob->x - x));
        if(attack_time == 0) attack_time = pik_type->attack_interval;
        
    }
}


/* ----------------------------------------------------------------------------
 * Returns the buried Pikmin closest to a leader. Used when auto-plucking.
 * x/y:             Coordinates of the leader.
 * d:               Variable to return the distance to. NULL for none.
 * ignore_reserved: If true, ignore any buried Pikmin that are "reserved"
   * (i.e. already chosen to be plucked by another leader).
 */
pikmin* get_closest_buried_pikmin(const float x, const float y, dist* d, const bool ignore_reserved) {
    dist closest_distance = 0;
    pikmin* closest_pikmin = NULL;
    
    size_t n_pikmin = pikmin_list.size();
    for(size_t p = 0; p < n_pikmin; ++p) {
        if(pikmin_list[p]->fsm.cur_state->id != PIKMIN_STATE_BURIED) continue;
        
        dist dis(x, y, pikmin_list[p]->x, pikmin_list[p]->y);
        if(closest_pikmin == NULL || dis < closest_distance) {
        
            if(!(ignore_reserved || pikmin_list[p]->pluck_reserved)) {
                closest_distance = dis;
                closest_pikmin = pikmin_list[p];
            }
        }
    }
    
    if(d) *d = closest_distance;
    return closest_pikmin;
}

void pikmin::become_buried(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_BURROWED);
}

void pikmin::begin_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    mob* lea = (mob*) info1;
    
    if(lea->following_party) {
        if(typeid(*lea->following_party) == typeid(leader)) {
            //If this leader is following another one, then the new Pikmin should be in the party of that top leader.
            lea = lea->following_party;
        }
    }
    
    pik->set_animation(PIKMIN_ANIM_PLUCKING);
    add_to_party(lea, pik);
}

void pikmin::end_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    pik->set_animation(PIKMIN_ANIM_IDLE);
    sfx_pikmin_plucked.play(0, false);
    sfx_pikmin_pluck.play(0, false);
}

void pikmin::be_grabbed_by_friend(mob* m, void* info1, void* info2) {
    sfx_pikmin_held.play(0, false);
    m->set_animation(PIKMIN_ANIM_IDLE);
}

void pikmin::be_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* mob_ptr = (mob*) info1;
    hitbox_instance* hi_ptr = (hitbox_instance*) info2;
    
    pik_ptr->set_connected_hitbox_info(hi_ptr, mob_ptr);
    
    pik_ptr->focused_mob = mob_ptr;
    
    sfx_pikmin_caught.play(0.2, 0);
    pik_ptr->set_animation(PIKMIN_ANIM_IDLE);
    remove_from_party(pik_ptr);
}

void pikmin::be_dismissed(mob* m, void* info1, void* info2) {
    m->set_target(
        *(float*) info1,
        *(float*) info2,
        NULL,
        NULL,
        false
    );
    sfx_pikmin_idle.play(0, false);
    
    m->set_animation(PIKMIN_ANIM_IDLE);
}

void pikmin::reach_dismiss_spot(mob* m, void* info1, void* info2) {
    m->remove_target();
    m->set_animation(PIKMIN_ANIM_IDLE);
}

void pikmin::become_idle(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_IDLE);
    unfocus_mob(m);
}

void pikmin::be_thrown(mob* m, void* info1, void* info2) {
    m->remove_target();
    sfx_pikmin_held.stop();
    sfx_pikmin_thrown.stop();
    sfx_pikmin_thrown.play(0, false);
    m->set_animation(PIKMIN_ANIM_THROWN);
}

void pikmin::be_released(mob* m, void* info1, void* info2) {

}

void pikmin::land(mob* m, void* info1, void* info2) {
    m->set_animation(PIKMIN_ANIM_IDLE);
    pikmin::stand_still(m, NULL, NULL);
}

void pikmin::stand_still(mob* m, void* info1, void* info2) {
    m->remove_target();
    m->speed_x = m->speed_y = 0;
}

void pikmin::go_to_task(mob* m, void* info1, void* info2) {

}

void pikmin::called(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    
    pik->attack_time = 0;
    add_to_party(cur_leader_ptr, pik);
    sfx_pikmin_called.play(0.03, false);
}

void pikmin::get_knocked_down(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    float knockback = 0;
    float knockback_angle = 0;
    
    calculate_knockback(info->mob2, m, info->hi2, info->hi1, &knockback, &knockback_angle);
    apply_knockback(m, knockback, knockback_angle);
    
    m->set_animation(PIKMIN_ANIM_LYING);
    
    remove_from_party(m);
}

void pikmin::go_to_opponent(mob* m, void* info1, void* info2) {
    focus_mob(m, (mob*) info1);
    m->remove_target();
    m->set_target(
        0, 0,
        &m->focused_mob->x, &m->focused_mob->y,
        false, nullptr, false,
        m->focused_mob->type->radius + m->type->radius
    );
    m->set_animation(PIKMIN_ANIM_WALK);
    remove_from_party(m);
}

void pikmin::rechase_opponent(mob* m, void* info1, void* info2) {
    if(
        m->focused_mob &&
        m->focused_mob->health > 0 &&
        dist(m->x, m->y, m->focused_mob->x, m->focused_mob->y) <=
        (m->type->radius + m->focused_mob->type->radius + m->type->near_radius)
    ) {
        return;
    }
    
    m->fsm.set_state(PIKMIN_STATE_IDLE);
}

void pikmin::go_to_carriable_object(mob* m, void* info1, void* info2) {
    mob* carriable_mob = (mob*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    focus_mob(pik_ptr, carriable_mob);
    pik_ptr->remove_target();
    
    size_t closest_spot = string::npos;
    dist closest_spot_dist;
    carrier_spot_struct* closest_spot_ptr;
    
    for(size_t s = 0; s < carriable_mob->type->max_carriers; ++s) {
        carrier_spot_struct* s_ptr = &carriable_mob->carry_info->spot_info[s];
        if(s_ptr->state != CARRY_SPOT_FREE) continue;
        
        dist d(
            pik_ptr->x,
            pik_ptr->y,
            carriable_mob->x + s_ptr->x,
            carriable_mob->y + s_ptr->y
        );
        if(closest_spot == string::npos || d < closest_spot_dist) {
            closest_spot = s;
            closest_spot_dist = d;
            closest_spot_ptr = s_ptr;
        }
    }
    
    pik_ptr->carrying_spot = closest_spot;
    closest_spot_ptr->state = CARRY_SPOT_RESERVED;
    closest_spot_ptr->pik_ptr = pik_ptr;
    
    pik_ptr->set_target(
        closest_spot_ptr->x,
        closest_spot_ptr->y,
        &carriable_mob->x,
        &carriable_mob->y,
        false, nullptr, false,
        pik_ptr->type->radius * 1.2
    );
    pik_ptr->set_animation(PIKMIN_ANIM_WALK);
    remove_from_party(pik_ptr);
}

void pikmin::reach_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* carriable_mob = pik_ptr->focused_mob;
    
    pik_ptr->set_animation(PIKMIN_ANIM_GRAB, true);
    
    pik_ptr->set_target(
        carriable_mob->carry_info->spot_info[pik_ptr->carrying_spot].x,
        carriable_mob->carry_info->spot_info[pik_ptr->carrying_spot].y,
        &carriable_mob->x,
        &carriable_mob->y,
        true, &carriable_mob->z
    );
    
    pik_ptr->face(atan2(carriable_mob->y - pik_ptr->y, carriable_mob->x - pik_ptr->x));
    
    //Let the carriable mob know that a new Pikmin has grabbed on.
    pik_ptr->focused_mob->fsm.run_event(
        MOB_EVENT_CARRIER_ADDED, (void*) pik_ptr
    );
}

void pikmin::forget_about_carrying(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(!p->focused_mob) return;
    p->focused_mob->fsm.run_event(MOB_EVENT_CARRIER_REMOVED, (void*) p);
    
    p->focused_mob = NULL;
    p->remove_target();
}

void pikmin::prepare_to_attack(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    p->set_animation(PIKMIN_ANIM_ATTACK);
    ((pikmin*) p)->attack_time = p->pik_type->attack_interval;
}

void pikmin::land_on_mob(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    
    mob* mob_ptr = info->mob2;
    hitbox_instance* hi_ptr = info->hi2;
    
    if(!hi_ptr || !hi_ptr->can_pikmin_latch) {
        //No good for latching on. Make it act like it landed on the ground.
        m->fsm.run_event(MOB_EVENT_LANDED);
        return;
    }
    
    pik_ptr->connected_hitbox_nr = hi_ptr->hitbox_nr;
    pik_ptr->speed_x = pik_ptr->speed_y = pik_ptr->speed_z = 0;
    
    pik_ptr->focused_mob = mob_ptr;
    pik_ptr->set_connected_hitbox_info(hi_ptr, mob_ptr);
    
    pik_ptr->was_thrown = false;
    
    pik_ptr->fsm.set_state(PIKMIN_STATE_ATTACKING_LATCHED);
    
}

void pikmin::lose_latched_mob(mob* m, void* info1, void* info2) {
    m->remove_target();
}

void pikmin::tick_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->focused_mob) return;
    
    pik_ptr->teleport_to_connected_hitbox();
}

void pikmin::tick_latched(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->focused_mob) return;
    
    pik_ptr->teleport_to_connected_hitbox();
    
    pik_ptr->attack_time -= delta_t;
    
    if(pik_ptr->attack_time <= 0) {
        pik_ptr->do_attack(pik_ptr->focused_mob, get_hitbox_instance(pik_ptr->focused_mob, pik_ptr->connected_hitbox_nr));
    }
}

void pikmin::tick_attacking_grounded(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    pik_ptr->attack_time -= delta_t;
    
    if(!pik_ptr->focused_mob || pik_ptr->focused_mob->dead) {
        return;
    }
    if(pik_ptr->attack_time <= 0) {
        if(!(
                    (pik_ptr->focused_mob->z > pik_ptr->z + pik_ptr->type->height) ||
                    (pik_ptr->focused_mob->z + pik_ptr->focused_mob->type->height < pik_ptr->z)
                )) {
            pik_ptr->do_attack(
                pik_ptr->focused_mob,
                get_hitbox_instance(pik_ptr->focused_mob, pik_ptr->connected_hitbox_nr)
            );
        }
        pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
    }
    
    pik_ptr->face(atan2(pik_ptr->focused_mob->y - pik_ptr->y, pik_ptr->focused_mob->x - pik_ptr->x));
}

void pikmin::work_on_task(mob* m, void* info1, void* info2) {

}

void pikmin::fall_down_pit(mob* m, void* info1, void* info2) {
    m->health = 0;
}

void pikmin::chase_leader(mob* m, void* info1, void* info2) {
    m->set_target(
        m->party_spot_x, m->party_spot_y,
        &m->following_party->party->party_center_x, &m->following_party->party->party_center_y,
        false
    );
    m->set_animation(PIKMIN_ANIM_WALK);
    focus_mob(m, m->following_party);
}

void pikmin::stop_being_idle(mob* m, void* info1, void* info2) {

}

void pikmin::stop_in_group(mob* m, void* info1, void* info2) {
    m->remove_target();
    m->set_animation(PIKMIN_ANIM_IDLE);
}

void pikmin::draw() {

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
    
    bool is_idle = (fsm.cur_state->id == PIKMIN_STATE_IDLE);
    if(is_idle) {
        int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
        al_get_separate_blender(&old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst);
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
        draw_sprite(
            f_ptr->bitmap,
            draw_x, draw_y,
            draw_w, draw_h,
            angle,
            map_gray(get_sprite_lighting(this))
        );
        al_set_separate_blender(old_op, old_src, old_dst, old_aop, old_asrc, old_adst);
    }
    
    float w_mult = draw_w / f_ptr->game_w;
    float h_mult = draw_h / f_ptr->game_h;
    
    if(f_ptr->top_visible) {
        float c = cos(angle), s = sin(angle);
        draw_sprite(
            pik_type->bmp_top[maturity],
            draw_x + c * f_ptr->top_x * w_mult + c * f_ptr->top_y * w_mult,
            draw_y - s * f_ptr->top_y * h_mult + s * f_ptr->top_x * h_mult,
            f_ptr->top_w * w_mult, f_ptr->top_h * h_mult,
            f_ptr->top_angle + angle,
            map_gray(get_sprite_lighting(this))
        );
    }
    
    if(is_idle) {
        draw_sprite(
            bmp_idle_glow,
            x, y,
            30 * w_mult, 30 * h_mult,
            idle_glow_angle,
            al_map_rgba_f(
                type->main_color.r * get_sprite_lighting(this) / 255,
                type->main_color.g * get_sprite_lighting(this) / 255,
                type->main_color.b * get_sprite_lighting(this) / 255,
                1
            )
        );
    }
    
    
}
