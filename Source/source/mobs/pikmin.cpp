/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin class and Pikmin-related functions.
 */

#include <algorithm>

#include "../drawing.h"
#include "../functions.h"
#include "../utils/geometry_utils.h"
#include "mob.h"
#include "pikmin.h"
#include "pikmin_fsm.h"
#include "../utils/string_utils.h"
#include "../vars.h"

static const float PIKMIN_MISSED_ATTACK_DURATION = 1.5f;

/* ----------------------------------------------------------------------------
 * Creates a Pikmin mob.
 */
pikmin::pikmin(
    const point &pos, pikmin_type* type,
    const float angle, const string &vars, mob* parent
) :
    mob(pos, type, angle, vars, parent),
    pik_type(type),
    carrying_mob(NULL),
    carrying_spot(0),
    missed_attack_ptr(nullptr),
    maturity(s2i(get_var_value(vars, "maturity", "2"))),
    is_seed_or_sprout(false),
    pluck_reserved(false) {
    
    invuln_period = timer(PIKMIN_INVULN_PERIOD);
    team = MOB_TEAM_PLAYER_1; // TODO
    if(s2b(get_var_value(vars, "sprout", "0"))) {
        fsm.set_state(PIKMIN_STATE_SPROUT);
    }
    subgroup_type_ptr =
        subgroup_types.get_type(SUBGROUP_TYPE_CATEGORY_PIKMIN, pik_type);
    near_reach = 0;
    far_reach = 2;
    
    missed_attack_timer =
        timer(
            PIKMIN_MISSED_ATTACK_DURATION,
    [this] () { this->missed_attack_ptr = NULL; }
        );
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
    if(health <= 0) {
        dead = true;
    }
    if(dead) {
        to_delete = true;
        
        pikmin_fsm::notify_leader_release(this, NULL, NULL);
        
        particle par(
            PARTICLE_TYPE_PIKMIN_SPIRIT, pos, LARGE_FLOAT,
            pik_type->radius * 2, 2.0f
        );
        par.bitmap = bmp_pikmin_spirit;
        par.speed.x = randomf(-20, 20);
        par.speed.y = randomf(-70, -30);
        par.friction = 0.8;
        par.gravity = -0.2;
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
void pikmin::draw_mob(bitmap_effect_manager* effect_manager) {

    sprite* s_ptr = anim.get_cur_sprite();
    
    if(!s_ptr) return;
    
    point draw_pos = get_sprite_center(s_ptr);
    point draw_size = get_sprite_dimensions(s_ptr);
    
    bool is_idle =
        fsm.cur_state->id == PIKMIN_STATE_IDLING ||
        fsm.cur_state->id == PIKMIN_STATE_SPROUT;
        
    bitmap_effect_manager effects;
    add_sector_brightness_bitmap_effect(&effects);
    add_status_bitmap_effects(&effects);
    
    if(is_idle) {
        bitmap_effect idle_effect;
        bitmap_effect_props idle_effect_props;
        idle_effect_props.glow_color = al_map_rgb(255, 255, 255);
        idle_effect.add_keyframe(0, idle_effect_props);
        effects.add_effect(idle_effect);
    }
    
    draw_bitmap_with_effects(
        s_ptr->bitmap,
        draw_pos, draw_size,
        angle + s_ptr->angle, &effects
    );
    
    if(s_ptr->top_visible) {
        point top_pos;
        top_pos = rotate_point(s_ptr->top_pos, angle);
        draw_bitmap_with_effects(
            pik_type->bmp_top[maturity],
            pos + top_pos,
            s_ptr->top_size,
            angle + s_ptr->top_angle,
            &effects
        );
    }
    
    if(is_idle) {
        draw_bitmap(
            bmp_idle_glow,
            pos,
            point(standard_pikmin_radius * 8, standard_pikmin_radius * 8),
            area_time_passed * IDLE_GLOW_SPIN_SPEED,
            type->main_color
        );
    }
    
    float status_bmp_scale;
    ALLEGRO_BITMAP* status_bmp = get_status_bitmap(&status_bmp_scale);
    if(status_bmp) {
        draw_bitmap(
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
 * Handles a status effect being applied.
 */
void pikmin::handle_status_effect(status_type* s) {
    if(s->causes_disable) {
        disabled_state_flags =
            s->disabled_state_inedible ? DISABLED_STATE_FLAG_INEDIBLE : 0;
        fsm.set_state(PIKMIN_STATE_DISABLED);
    } else if(s->causes_panic) {
        fsm.set_state(PIKMIN_STATE_PANICKING);
    } else if(s->causes_flailing) {
        fsm.set_state(PIKMIN_STATE_FLAILING);
    }
    
    if(s->maturity_change_amount != 0) {
        int new_maturity = maturity + s->maturity_change_amount;
        new_maturity = clamp(new_maturity, 0, 2);
        maturity = new_maturity;
    }
}


/* ----------------------------------------------------------------------------
 * Checks if the attack should miss, and returns the result.
 * If it was already decided that it missed in a previous frame, that's a
 * straight no. If not, it will roll with the hit rate to check.
 * If the attack is a miss, it also registers the miss, so that we can keep
 * memory of it for the next frames.
 */
bool pikmin::process_attack_miss(hitbox_interaction* info) {
    if(info->mob2->anim.cur_anim == missed_attack_ptr) {
        //In a previous frame, we had already considered this animation a miss.
        return false;
    }
    
    unsigned char hit_rate = info->mob2->anim.cur_anim->hit_rate;
    if(hit_rate == 0) return false;
    
    unsigned char hit_roll = randomi(0, 100);
    if(hit_roll > hit_rate) {
        //This attack was randomly decided to be a miss.
        //Record this animation so it won't be considered a hit next frame.
        missed_attack_ptr = info->mob2->anim.cur_anim;
        missed_attack_timer.start();
        return false;
    }
    
    return true;
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
