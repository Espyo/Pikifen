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

#include <algorithm>

#include "../drawing.h"
#include "../functions.h"
#include "mob.h"
#include "pikmin.h"
#include "pikmin_fsm.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a Pikmin mob.
 */
pikmin::pikmin(
    const float x, const float y, pikmin_type* type,
    const float angle, const string &vars
) :
    mob(x, y, type, angle, vars),
    pik_type(type),
    attack_time(0),
    pluck_reserved(false),
    carrying_mob(NULL),
    carrying_spot(0),
    maturity(s2i(get_var_value(vars, "maturity", "2"))),
    connected_hitbox_nr(INVALID),
    connected_hitbox_dist(0),
    connected_hitbox_angle(0) {
    
    invuln_period = timer(PIKMIN_INVULN_PERIOD);
    team = MOB_TEAM_PLAYER_1; // TODO
    if(s2b(get_var_value(vars, "buried", "0"))) {
        fsm.set_state(PIKMIN_STATE_BURIED);
    }
}


pikmin::~pikmin() { }


/* ----------------------------------------------------------------------------
 * Returns a Pikmin's base speed, without status effects and the like.
 * This depends on the maturity.
 */
float pikmin::get_base_speed() {
    float base = mob::get_base_speed();
    return base + (base * this->maturity * maturity_speed_mult);
}


/* ----------------------------------------------------------------------------
 * Actually makes the Pikmin attack connect - the process that makes
 * the victim lose health, the sound, the sparks, etc.
 * victim_hitbox_i: Hitbox instance of the victim.
 */
void pikmin::do_attack(mob* m, hitbox_instance* victim_hitbox_i) {
    attack_time = pik_type->attack_interval;
    
    if(!m || !victim_hitbox_i) return;
    
    if(
        !is_resistant_to_hazards(
            this->pik_type->resistances, victim_hitbox_i->hazards
        )
    ) {
        //If the hitbox says it has a fire effect, and this
        //Pikmin is not immune to fire, don't let it be a wise-guy;
        //it cannot be able to attack the hitbox.
        return;
    }
    
    hitbox_touch_info info = hitbox_touch_info(this, victim_hitbox_i, NULL);
    focused_mob->fsm.run_event(MOB_EVENT_HITBOX_TOUCH_N_A, &info);
    
    sfx_attack.play(0.06, false, 0.4f);
    sfx_pikmin_attack.play(0.06, false, 0.8f);
    
    particle smack_p(
        PARTICLE_TYPE_SMACK, x, y,
        64, SMACK_PARTICLE_DUR, PARTICLE_PRIORITY_MEDIUM
    );
    smack_p.bitmap = bmp_smack;
    smack_p.color = al_map_rgb(255, 160, 128);
    smack_p.before_mobs = false;
    particles.add(smack_p);
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
    coordinates_to_angle(
        x_dif, y_dif, &connected_hitbox_angle, &connected_hitbox_dist
    );
    
    //Relative to 0 degrees.
    connected_hitbox_angle -= mob_ptr->angle;
    //Distance in units to distance in percentage.
    connected_hitbox_dist /= hi_ptr->radius;
    connected_hitbox_nr = hi_ptr->hitbox_nr;
}


/* ----------------------------------------------------------------------------
 * Teleports the Pikmin to the hitbox it is connected to.
 */
void pikmin::teleport_to_connected_hitbox() {
    speed_x = speed_y = speed_z = 0;
    
    hitbox_instance* h_ptr =
        get_hitbox_instance(focused_mob, connected_hitbox_nr);
    if(h_ptr) {
        float actual_hx, actual_hy;
        rotate_point(
            h_ptr->x, h_ptr->y, focused_mob->angle, &actual_hx, &actual_hy
        );
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
pikmin* get_closest_buried_pikmin(
    const float x, const float y, dist* d, const bool ignore_reserved
) {
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


/* ----------------------------------------------------------------------------
 * Ticks some logic specific to Pikmin.
 */
void pikmin::tick_class_specifics() {
    //Carrying object.
    if(carrying_mob) {
        if(!carrying_mob->carry_info) {
            fsm.run_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE);
        }
    }
    
    //Is it dead?
    if(dead) {
        to_delete = true;
        
        particle par(
            PARTICLE_TYPE_PIKMIN_SPIRIT, x, y,
            pik_type->radius * 2, 2.0f
        );
        par.bitmap = bmp_pikmin_spirit;
        par.speed_x = 0;
        par.speed_y = -50;
        par.friction = 0.5;
        par.gravity = 0;
        par.color = pik_type->main_color;
        particles.add(par);
        
        sfx_pikmin_dying.play(0.03, false);
    }
}


/* ----------------------------------------------------------------------------
 * Draws a Pikmin, including its leaf/bud/flower, idle glow, etc.
 */
void pikmin::draw() {

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
    
    draw_sprite(
        f_ptr->bitmap,
        draw_x, draw_y,
        draw_w, draw_h,
        angle,
        tint
    );
    
    bool is_idle =
        fsm.cur_state->id == PIKMIN_STATE_IDLING ||
        fsm.cur_state->id == PIKMIN_STATE_BURIED;
        
    if(is_idle) {
        int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
        al_get_separate_blender(
            &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
        );
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
        draw_sprite(
            f_ptr->bitmap,
            draw_x, draw_y,
            draw_w, draw_h,
            angle,
            map_gray(get_sprite_brightness(this))
        );
        al_set_separate_blender(
            old_op, old_src, old_dst, old_aop, old_asrc, old_adst
        );
    }
    
    float w_mult = draw_w / f_ptr->game_w;
    float h_mult = draw_h / f_ptr->game_h;
    
    if(f_ptr->top_visible) {
        float top_x = f_ptr->top_x;
        float top_y = f_ptr->top_y;
        rotate_point(f_ptr->top_x, f_ptr->top_y, angle, &top_x, &top_y);
        draw_sprite(
            pik_type->bmp_top[maturity],
            x + top_x, y + top_y,
            f_ptr->top_w, f_ptr->top_h,
            f_ptr->top_angle + angle,
            tint
        );
    }
    
    if(is_idle) {
        draw_sprite(
            bmp_idle_glow,
            x, y,
            30 * w_mult, 30 * h_mult,
            area_time_passed * IDLE_GLOW_SPIN_SPEED,
            al_map_rgba_f(
                type->main_color.r * get_sprite_brightness(this) / 255,
                type->main_color.g * get_sprite_brightness(this) / 255,
                type->main_color.b * get_sprite_brightness(this) / 255,
                1
            )
        );
    }
    
    float status_bmp_scale;
    ALLEGRO_BITMAP* status_bmp = get_status_bitmap(&status_bmp_scale);
    if(status_bmp) {
        draw_sprite(status_bmp, x, y, type->radius * 2 * status_bmp_scale, -1);
    }
    
}


/* ----------------------------------------------------------------------------
 * Returns whether or not a Pikmin can receive a given status effect.
 */
bool pikmin::can_receive_status(status_type* s) {
    return s->affects & STATUS_AFFECTS_PIKMIN;
}


/* ----------------------------------------------------------------------------
 * Handler for when a status effect causes "disabled".
 */
void pikmin::receive_disable_from_status(const unsigned char flags) {
    disabled_state_flags = flags;
    fsm.set_state(PIKMIN_STATE_DISABLED);
}


/* ----------------------------------------------------------------------------
 * Handler for when a status effect causes "flailing".
 */
void pikmin::receive_flailing_from_status() {
    fsm.set_state(PIKMIN_STATE_FLAILING);
}


/* ----------------------------------------------------------------------------
 * Handler for when a status effect causes "panic".
 */
void pikmin::receive_panic_from_status() {
    fsm.set_state(PIKMIN_STATE_PANICKING);
}


/* ----------------------------------------------------------------------------
 * Handler for when a status effect no longer causes "panic".
 */
void pikmin::lose_panic_from_status() {
    if(fsm.cur_state->id == PIKMIN_STATE_PANICKING) {
        fsm.set_state(PIKMIN_STATE_IDLING);
        pikmin_fsm::stand_still(this, NULL, NULL);
    }
}

/* ----------------------------------------------------------------------------
 * Handler for when a status effect changes maturity.
 * amount: Amount to increase or decrease.
 */
void pikmin::change_maturity_amount_from_status(const int amount) {
    int new_maturity = maturity + amount;
    new_maturity = max(0, new_maturity);
    new_maturity = min(new_maturity, 2);
    maturity = new_maturity;
}
