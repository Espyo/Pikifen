/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
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
#include "../geometry_utils.h"
#include "mob.h"
#include "pikmin.h"
#include "pikmin_fsm.h"
#include "../vars.h"

static const float PIKMIN_MISSED_ATTACK_DURATION = 1.5f;

/* ----------------------------------------------------------------------------
 * Creates a Pikmin mob.
 */
pikmin::pikmin(
    const point &pos, pikmin_type* type,
    const float angle, const string &vars
) :
    mob(pos, type, angle, vars),
    pik_type(type),
    pluck_reserved(false),
    carrying_mob(NULL),
    carrying_spot(0),
    maturity(s2i(get_var_value(vars, "maturity", "2"))),
    missed_attack_ptr(nullptr),
    missed_attack_timer(
        PIKMIN_MISSED_ATTACK_DURATION,
        [this] () {
            this->missed_attack_ptr = NULL;
        }
    ),
    connected_hitbox_nr(INVALID),
    connected_hitbox_dist(0),
    connected_hitbox_angle(0) {
    
    invuln_period = timer(PIKMIN_INVULN_PERIOD);
    team = MOB_TEAM_PLAYER_1; // TODO
    if(s2b(get_var_value(vars, "sprout", "0"))) {
        fsm.set_state(PIKMIN_STATE_SPROUT);
    }
    subgroup_type_ptr =
        subgroup_types.get_type(SUBGROUP_TYPE_CATEGORY_PIKMIN, pik_type);
    near_reach = 0;
    far_reach = 2;
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
 * victim_hitbox: Hitbox of the victim.
 */
void pikmin::do_attack(mob* m, hitbox* victim_hitbox) {
    if(!m || !victim_hitbox) return;
    
    if(
        !is_resistant_to_hazards(
            this->pik_type->resistances, victim_hitbox->hazards
        )
    ) {
        //If the hitbox says it has a fire effect, and this
        //Pikmin is not immune to fire, don't let it be a wise-guy;
        //it cannot be able to attack the hitbox.
        return;
    }
    
    hitbox_touch_info info = hitbox_touch_info(this, victim_hitbox, NULL);
    focused_mob->fsm.run_event(MOB_EVENT_HITBOX_TOUCH_N_A, &info);
    
    sfx_attack.play(0.06, false, 0.4f);
    sfx_pikmin_attack.play(0.06, false, 0.8f);
    
    particle smack_p(
        PARTICLE_TYPE_SMACK, pos,
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
 * h_ptr: Hitbox of the other mob.
 * m:     The other mob.
 */
void pikmin::set_connected_hitbox_info(hitbox* h_ptr, mob* mob_ptr) {
    if(!h_ptr) return;
    
    point actual_h_pos = rotate_point(h_ptr->pos, mob_ptr->angle);
    actual_h_pos += mob_ptr->pos;
    
    point pos_dif = pos - actual_h_pos;
    coordinates_to_angle(
        pos_dif, &connected_hitbox_angle, &connected_hitbox_dist
    );
    
    //Relative to 0 degrees.
    connected_hitbox_angle -= mob_ptr->angle;
    //Distance in units to distance in percentage.
    connected_hitbox_dist /= h_ptr->radius;
    connected_hitbox_nr = h_ptr->body_part_index;
}


/* ----------------------------------------------------------------------------
 * Teleports the Pikmin to the hitbox it is connected to.
 */
void pikmin::teleport_to_connected_hitbox() {
    speed.x = speed.y = speed_z = 0;
    
    hitbox* h_ptr =
        focused_mob->get_hitbox(connected_hitbox_nr);
    if(h_ptr) {
        point actual_h_pos = rotate_point(h_ptr->pos, focused_mob->angle);
        actual_h_pos += focused_mob->pos;
        
        point final_pos =
            angle_to_coordinates(
                connected_hitbox_angle + focused_mob->angle,
                connected_hitbox_dist * h_ptr->radius
            );
        final_pos += actual_h_pos;
        
        chase(final_pos, NULL, true);
        
        face(get_angle(pos, focused_mob->pos));
        //Force the Z to be above the mob, so it'll always appear above it.
        z = focused_mob->z + 1;
        
    }
}


/* ----------------------------------------------------------------------------
 * Returns the sprout closest to a leader. Used when auto-plucking.
 * pos:             Coordinates of the leader.
 * d:               Variable to return the distance to. NULL for none.
 * ignore_reserved: If true, ignore any sprouts that are "reserved"
 *   (i.e. already chosen to be plucked by another leader).
 */
pikmin* get_closest_sprout(
    const point &pos, dist* d, const bool ignore_reserved
) {
    dist closest_distance = 0;
    pikmin* closest_pikmin = NULL;
    
    size_t n_pikmin = pikmin_list.size();
    for(size_t p = 0; p < n_pikmin; ++p) {
        if(pikmin_list[p]->fsm.cur_state->id != PIKMIN_STATE_SPROUT) continue;
        
        dist dis(pos, pikmin_list[p]->pos);
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
            PARTICLE_TYPE_PIKMIN_SPIRIT, pos,
            pik_type->radius * 2, 2.0f
        );
        par.bitmap = bmp_pikmin_spirit;
        par.speed.x = 0;
        par.speed.y = -50;
        par.friction = 0.5;
        par.gravity = 0;
        par.color = pik_type->main_color;
        particles.add(par);
        
        sfx_pikmin_dying.play(0.03, false);
    }
    
    //Tick the timer for the "missed" attack animation.
    missed_attack_timer.tick(delta_t);
    
}


/* ----------------------------------------------------------------------------
 * Draws a Pikmin, including its leaf/bud/flower, idle glow, etc.
 */
void pikmin::draw(sprite_effect_manager* effect_manager) {

    sprite* s_ptr = anim.get_cur_sprite();
    
    if(!s_ptr) return;
    
    point draw_pos = get_sprite_center(s_ptr);
    point draw_size = get_sprite_dimensions(s_ptr);
    
    bool is_idle =
        fsm.cur_state->id == PIKMIN_STATE_IDLING ||
        fsm.cur_state->id == PIKMIN_STATE_SPROUT;
        
    sprite_effect_manager effects;
    add_sector_brightness_sprite_effect(&effects);
    add_status_sprite_effects(&effects);
    
    if(is_idle) {
        sprite_effect idle_effect;
        sprite_effect_props idle_effect_props;
        idle_effect_props.glow_color = pik_type->main_color;
        idle_effect.add_keyframe(0, idle_effect_props);
        effects.add_effect(idle_effect);
    }
    
    draw_sprite_with_effects(
        s_ptr->bitmap,
        draw_pos, draw_size,
        angle, &effects
    );
    
    point size_mult = draw_size / s_ptr->game_size;
    
    if(s_ptr->top_visible) {
        point top_pos;
        top_pos = rotate_point(s_ptr->top_pos, angle);
        draw_sprite_with_effects(
            pik_type->bmp_top[maturity],
            pos + top_pos,
            s_ptr->top_size,
            s_ptr->top_angle + angle,
            &effects
        );
    }
    
    if(is_idle) {
        draw_sprite(
            bmp_idle_glow,
            pos,
            size_mult * 35,
            area_time_passed * IDLE_GLOW_SPIN_SPEED,
            type->main_color
        );
    }
    
    float status_bmp_scale;
    ALLEGRO_BITMAP* status_bmp = get_status_bitmap(&status_bmp_scale);
    if(status_bmp) {
        draw_sprite(
            status_bmp, pos,
            point(type->radius * 2 * status_bmp_scale, -1)
        );
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
    new_maturity = clamp(new_maturity, 0, 2);
    maturity = new_maturity;
}
