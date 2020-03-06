/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin class and Pikmin-related functions.
 */

#include <algorithm>

#include "pikmin.h"

#include "../drawing.h"
#include "../functions.h"
#include "../mob_fsms/pikmin_fsm.h"
#include "../utils/geometry_utils.h"
#include "../utils/string_utils.h"
#include "../vars.h"
#include "mob.h"

static const float PIKMIN_MISSED_ATTACK_DURATION = 1.5f;

/* ----------------------------------------------------------------------------
 * Creates a Pikmin mob.
 */
pikmin::pikmin(const point &pos, pikmin_type* type, const float angle) :
    mob(pos, type, angle),
    pik_type(type),
    carrying_mob(NULL),
    carrying_spot(0),
    missed_attack_ptr(nullptr),
    maturity(2),
    is_seed_or_sprout(false),
    pluck_reserved(false),
    latched(false),
    is_tool_primed_for_whistle(false),
    consecutive_dud_hits(0) {
    
    invuln_period = timer(PIKMIN_INVULN_PERIOD);
    team = MOB_TEAM_PLAYER_1; // TODO
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
 * Forces the Pikmin to start carrying the given mob.
 * This quickly runs over several steps in the usual FSM logic, just to
 * instantly get to the end result.
 * As such, be careful when using it.
 */
void pikmin::force_carry(mob* m) {
    pikmin_fsm::go_to_carriable_object(this, (void*) m, NULL);
    fsm.set_state(PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT);
    pikmin_fsm::reach_carriable_object(this, NULL, NULL);
    fsm.set_state(PIKMIN_STATE_CARRYING);
}


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
void pikmin::tick_class_specifics(const float delta_t) {
    //Carrying object.
    if(carrying_mob) {
        if(!carrying_mob->carry_info) {
            fsm.run_event(MOB_EVENT_FOCUSED_MOB_UNAVAILABLE);
        }
    }
    
    //Is it dead?
    if(health <= 0) {
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
void pikmin::draw_mob() {
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    //The Pikmin itself.
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(s_ptr, &eff, true, true);
    
    bool is_idle =
        fsm.cur_state->id == PIKMIN_STATE_IDLING ||
        fsm.cur_state->id == PIKMIN_STATE_IDLING_H ||
        fsm.cur_state->id == PIKMIN_STATE_SPROUT;
        
    if(is_idle) {
        eff.glow_color = al_map_rgb(255, 255, 255);
    }
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
    
    //Top.
    if(s_ptr->top_visible) {
        bitmap_effect_info top_eff = eff;
        ALLEGRO_BITMAP* top_bmp = pik_type->bmp_top[maturity];
        top_eff.translation = pos + rotate_point(s_ptr->top_pos, angle);
        top_eff.scale.x = s_ptr->top_size.x / al_get_bitmap_width(top_bmp);
        top_eff.scale.y = s_ptr->top_size.y / al_get_bitmap_height(top_bmp);
        top_eff.rotation += s_ptr->top_angle;
        top_eff.glow_color = map_gray(0);
        
        draw_bitmap_with_effects(top_bmp, top_eff);
    }
    
    //Idle glow.
    if(is_idle) {
        bitmap_effect_info idle_eff = eff;
        idle_eff.translation = pos;
        idle_eff.scale.x =
            (standard_pikmin_radius * 8) / al_get_bitmap_width(bmp_idle_glow);
        idle_eff.scale.y =
            (standard_pikmin_radius * 8) / al_get_bitmap_height(bmp_idle_glow);
        idle_eff.rotation = area_time_passed * IDLE_GLOW_SPIN_SPEED;
        idle_eff.tint_color = type->main_color;
        idle_eff.glow_color = map_gray(0);
        
        draw_bitmap_with_effects(bmp_idle_glow, idle_eff);
    }
    
    draw_status_effect_bmp(this, eff);
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
    
    increase_maturity(s->maturity_change_amount);
}


/* ----------------------------------------------------------------------------
 * Increases (or decreases) the Pikmin's maturity by the given amount.
 * This makes sure that the maturity doesn't overflow.
 */
void pikmin::increase_maturity(const int amount) {
    int new_maturity = maturity + amount;
    maturity = clamp(new_maturity, 0, N_MATURITIES - 1);
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 */
void pikmin::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    size_t maturity_var;
    bool sprout_var;
    
    if(svr.get("maturity", maturity_var)) {
        maturity = 0;
        increase_maturity(maturity_var);
    }
    if(svr.get("sprout", sprout_var)) {
        if(sprout_var) {
            fsm.first_state_override = PIKMIN_STATE_SPROUT;
        }
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
 * Handler for when there is no longer any status effect-induced panic.
 */
void pikmin::handle_panic_loss() {
    if(fsm.cur_state->id == PIKMIN_STATE_PANICKING) {
        fsm.set_state(PIKMIN_STATE_IDLING);
        pikmin_fsm::stand_still(this, NULL, NULL);
    }
}
