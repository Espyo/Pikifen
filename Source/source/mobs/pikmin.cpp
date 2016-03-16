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

#include "../drawing.h"
#include "../functions.h"
#include "mob.h"
#include "pikmin.h"
#include "../vars.h"

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
    return pik_type->move_speed + pik_type->move_speed * this->maturity * maturity_speed_mult;
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
        
        chase(final_px, final_py, NULL, NULL, true);
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


void pikmin::tick_class_specifics() {
    //Is it dead?
    if(dead) {
        to_delete = true;
        particles.push_back(
            particle(
                PARTICLE_TYPE_PIKMIN_SPIRIT, bmp_pikmin_spirit, x, y,
                0, -50, 0.5, 0, 2, pik_type->radius * 2, pik_type->main_color
            )
        );
        sfx_pikmin_dying.play(0.03, false);
    }
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
    
    bool is_idle = (fsm.cur_state->id == PIKMIN_STATE_IDLE || fsm.cur_state->id == PIKMIN_STATE_BURIED);
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
