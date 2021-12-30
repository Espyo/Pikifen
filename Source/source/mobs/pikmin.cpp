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
#include "../game.h"
#include "../mob_fsms/pikmin_fsm.h"
#include "../utils/geometry_utils.h"
#include "../utils/string_utils.h"
#include "mob.h"


//Height above the floor that a flying Pikmin prefers to stay at.
const float pikmin::FLIER_ABOVE_FLOOR_HEIGHT = 100.0f;
//How long to remember a missed incoming attack for.
const float pikmin::MISSED_ATTACK_DURATION = 1.5f;
//Chance of circling the opponent instead of striking, when grounded.
const float pikmin::CIRCLE_OPPONENT_CHANCE_GROUNDED = 0.2f;
//Chance of circling the opponent instead of latching, if it can latch.
const float pikmin::CIRCLE_OPPONENT_CHANCE_PRE_LATCH = 0.5f;

/* ----------------------------------------------------------------------------
 * Creates a Pikmin mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Pikmin type this mob belongs to.
 * angle:
 *   Starting angle.
 */
pikmin::pikmin(const point &pos, pikmin_type* type, const float angle) :
    mob(pos, type, angle),
    pik_type(type),
    carrying_mob(NULL),
    missed_attack_ptr(nullptr),
    was_last_hit_dud(false),
    consecutive_dud_hits(0),
    maturity(2),
    is_seed_or_sprout(false),
    is_grabbed_by_enemy(false),
    pluck_reserved(false),
    leader_to_return_to(nullptr),
    latched(false),
    is_tool_primed_for_whistle(false),
    must_follow_link_as_leader(false),
    temp_i(0) {
    
    invuln_period = timer(PIKMIN_INVULN_PERIOD);
    team = MOB_TEAM_PLAYER_1;
    subgroup_type_ptr =
        game.states.gameplay->subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, pik_type
        );
    near_reach = 0;
    far_reach = 2;
    
    missed_attack_timer =
        timer(
            MISSED_ATTACK_DURATION,
    [this] () { this->missed_attack_ptr = NULL; }
        );
        
    if(pik_type->can_fly) {
        can_move_in_midair = true;
    }
}


/* ----------------------------------------------------------------------------
 * Returns whether or not a Pikmin can receive a given status effect.
 * s:
 *   Status type to check.
 */
bool pikmin::can_receive_status(status_type* s) const {
    return s->affects & STATUS_AFFECTS_PIKMIN;
}


/* ----------------------------------------------------------------------------
 * Draws a Pikmin, including its leaf/bud/flower, idle glow, etc.
 */
void pikmin::draw_mob() {
    sprite* s_ptr = get_cur_sprite();
    if(!s_ptr) return;
    
    //The Pikmin itself.
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(s_ptr, &eff, true, true, true);
    
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
            (game.config.standard_pikmin_radius * 8) /
            al_get_bitmap_width(game.sys_assets.bmp_idle_glow);
        idle_eff.scale.y =
            (game.config.standard_pikmin_radius * 8) /
            al_get_bitmap_height(game.sys_assets.bmp_idle_glow);
        idle_eff.rotation =
            game.states.gameplay->area_time_passed * IDLE_GLOW_SPIN_SPEED;
        idle_eff.tint_color = type->main_color;
        idle_eff.glow_color = map_gray(0);
        
        draw_bitmap_with_effects(game.sys_assets.bmp_idle_glow, idle_eff);
    }
    
    draw_status_effect_bmp(this, eff);
}


/* ----------------------------------------------------------------------------
 * Forces the Pikmin to start carrying the given mob.
 * This quickly runs over several steps in the usual FSM logic, just to
 * instantly get to the end result.
 * As such, be careful when using it.
 * m:
 *   The mob to carry.
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
float pikmin::get_base_speed() const {
    float base = mob::get_base_speed();
    return base + (base * this->maturity * game.config.maturity_speed_mult);
}


/* ----------------------------------------------------------------------------
 * Returns its group spot information.
 * Basically, when it's in a leader's group, what point it should be following,
 * and within what distance.
 * final_spot:
 *   The final coordinates are returned here.
 * final_dist:
 *   The final distance to those coordinates is returned here.
 */
void pikmin::get_group_spot_info(
    point* final_spot, float* final_dist
) const {
    final_spot->x = 0.0f;
    final_spot->y = 0.0f;
    *final_dist = 0.0f;
    
    if(!following_group || !following_group->group) {
        return;
    }
    
    *final_spot =
        following_group->group->anchor +
        following_group->group->get_spot_offset(group_spot_index);
    *final_dist = 5.0f;
}


/* ----------------------------------------------------------------------------
 * Handles a status effect being applied.
 * sta_type:
 *   Status effect to handle.
 */
void pikmin::handle_status_effect_gain(status_type* sta_type) {
    mob::handle_status_effect_gain(sta_type);
    
    switch(sta_type->state_change_type) {
    case STATUS_STATE_CHANGE_FLAILING: {
        fsm.set_state(PIKMIN_STATE_FLAILING);
        break;
    }
    case STATUS_STATE_CHANGE_HELPLESS: {
        fsm.set_state(PIKMIN_STATE_HELPLESS);
        break;
    }
    case STATUS_STATE_CHANGE_PANIC: {
        fsm.set_state(PIKMIN_STATE_PANICKING);
        break;
    }
    default: {
        break;
    }
    }
    
    increase_maturity(sta_type->maturity_change_amount);
}


/* ----------------------------------------------------------------------------
 * Handles a status effect being removed.
 * sta_type:
 *   Status effect to handle.
 */
void pikmin::handle_status_effect_loss(status_type* sta_type) {
    bool still_has_flailing = false;
    bool still_has_helplessness = false;
    bool still_has_panic = false;
    for(size_t s = 0; s < statuses.size(); ++s) {
        if(statuses[s].type == sta_type) continue;
        
        switch(statuses[s].type->state_change_type) {
        case STATUS_STATE_CHANGE_FLAILING: {
            still_has_flailing = true;
            break;
        }
        case STATUS_STATE_CHANGE_HELPLESS: {
            still_has_helplessness = true;
            break;
        }
        case STATUS_STATE_CHANGE_PANIC: {
            still_has_panic = true;
            break;
        }
        default: {
            break;
        }
        }
    }
    
    if(
        sta_type->state_change_type == STATUS_STATE_CHANGE_FLAILING &&
        !still_has_flailing &&
        fsm.cur_state->id == PIKMIN_STATE_FLAILING
    ) {
        fsm.set_state(PIKMIN_STATE_IDLING);
        pikmin_fsm::stand_still(this, NULL, NULL);
        invuln_period.start();
    }
    
    if(
        sta_type->state_change_type == STATUS_STATE_CHANGE_HELPLESS &&
        !still_has_helplessness &&
        fsm.cur_state->id == PIKMIN_STATE_HELPLESS
    ) {
        fsm.set_state(PIKMIN_STATE_IDLING);
        pikmin_fsm::stand_still(this, NULL, NULL);
        invuln_period.start();
        
    } else if(
        sta_type->state_change_type == STATUS_STATE_CHANGE_PANIC &&
        !still_has_panic &&
        fsm.cur_state->id == PIKMIN_STATE_PANICKING
    ) {
        fsm.set_state(PIKMIN_STATE_IDLING);
        pikmin_fsm::stand_still(this, NULL, NULL);
        invuln_period.start();
        
    }
}


/* ----------------------------------------------------------------------------
 * Increases (or decreases) the Pikmin's maturity by the given amount.
 * This makes sure that the maturity doesn't overflow.
 * amount:
 *   Amount to increase by.
 */
void pikmin::increase_maturity(const int amount) {
    int new_maturity = maturity + amount;
    maturity = clamp(new_maturity, 0, N_MATURITIES - 1);
}


/* ----------------------------------------------------------------------------
 * Latches on to the specified mob.
 * m:
 *   Mob to latch on to.
 * h:
 *   Hitbox to latch on to.
 */
void pikmin::latch(mob* m, hitbox* h) {
    speed.x = speed.y = speed_z = 0;
    
    float h_offset_dist;
    float h_offset_angle;
    m->get_hitbox_hold_point(
        this, h, &h_offset_dist, &h_offset_angle
    );
    m->hold(
        this, h->body_part_index, h_offset_dist, h_offset_angle,
        true, true
    );
    
    latched = true;
}


/* ----------------------------------------------------------------------------
 * Checks if an incoming attack should miss, and returns the result.
 * If it was already decided that it missed in a previous frame, that's a
 * straight no. If not, it will roll with the hit rate to check.
 * If the attack is a miss, it also registers the miss, so that we can keep
 * memory of it for the next frames.
 * info:
 *   Info about the hitboxes involved.
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
 * Reads the provided script variables, if any, and does stuff with them.
 * svr:
 *   Script var reader to use.
 */
void pikmin::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    size_t maturity_var;
    bool sprout_var;
    bool follow_link_var;
    
    if(svr.get("maturity", maturity_var)) {
        maturity = 0;
        increase_maturity(maturity_var);
    }
    if(svr.get("sprout", sprout_var)) {
        if(sprout_var) {
            fsm.first_state_override = PIKMIN_STATE_SPROUT;
        }
    }
    if(svr.get("follow_link_as_leader", follow_link_var)) {
        if(follow_link_var) {
            must_follow_link_as_leader = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Starts the particle generator that leaves a trail behind a thrown Pikmin.
 */
void pikmin::start_throw_trail() {
    particle throw_p(
        PARTICLE_TYPE_CIRCLE, pos, z,
        radius, 0.6, PARTICLE_PRIORITY_LOW
    );
    throw_p.size_grow_speed = -5;
    throw_p.color = change_alpha(type->main_color, 128);
    particle_generator pg(THROW_PARTICLE_INTERVAL, throw_p, 1);
    pg.follow_mob = this;
    pg.id = MOB_PARTICLE_GENERATOR_THROW;
    particle_generators.push_back(pg);
}


/* ----------------------------------------------------------------------------
 * Ticks some logic specific to Pikmin.
 * delta_t:
 *   How many seconds to tick by.
 */
void pikmin::tick_class_specifics(const float delta_t) {
    //Carrying object.
    if(carrying_mob) {
        if(!carrying_mob->carry_info) {
            fsm.run_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE);
        }
    }
    
    //Is it dead?
    if(health <= 0 && !is_grabbed_by_enemy) {
        to_delete = true;
        
        pikmin_fsm::notify_leader_release(this, NULL, NULL);
        
        particle par(
            PARTICLE_TYPE_PIKMIN_SPIRIT, pos, LARGE_FLOAT,
            radius * 2, 2.0f
        );
        par.bitmap = game.sys_assets.bmp_pikmin_spirit;
        par.speed.x = randomf(-20, 20);
        par.speed.y = randomf(-70, -30);
        par.friction = 0.8;
        par.gravity = -0.2;
        par.color = pik_type->main_color;
        game.states.gameplay->particles.add(par);
        
        game.sys_assets.sfx_pikmin_dying.play(0.03, false);
        
        game.states.results->pikmin_deaths++;
    }
    
    //Tick the timer for the "missed" attack animation.
    missed_attack_timer.tick(delta_t);
    
    //Forcefully follow another mob as a leader.
    if(must_follow_link_as_leader) {
        if(!links.empty()) {
            fsm.run_event(MOB_EV_TOUCHED_ACTIVE_LEADER, (void*) (links[0]));
        }
        //Since this leader is likely an enemy, let's keep these Pikmin safe.
        is_huntable = false;
        is_hurtable = false;
        must_follow_link_as_leader = false;
    }
    
}


/* ----------------------------------------------------------------------------
 * Returns the sprout closest to a leader. Used when auto-plucking.
 * pos:
 *   Coordinates of the leader.
 * d:
 *   Variable to return the distance to. NULL for none.
 * ignore_reserved:
 *   If true, ignore any sprouts that are "reserved"
 *   (i.e. already chosen to be plucked by another leader).
 */
pikmin* get_closest_sprout(
    const point &pos, dist* d, const bool ignore_reserved
) {
    dist closest_distance = 0;
    pikmin* closest_pikmin = NULL;
    
    size_t n_pikmin = game.states.gameplay->mobs.pikmin_list.size();
    for(size_t p = 0; p < n_pikmin; ++p) {
        if(
            game.states.gameplay->mobs.pikmin_list[p]->fsm.cur_state->id !=
            PIKMIN_STATE_SPROUT
        ) {
            continue;
        }
        
        dist dis(pos, game.states.gameplay->mobs.pikmin_list[p]->pos);
        if(closest_pikmin == NULL || dis < closest_distance) {
        
            if(
                !(
                    ignore_reserved ||
                    game.states.gameplay->mobs.pikmin_list[p]->pluck_reserved
                )
            ) {
                closest_distance = dis;
                closest_pikmin = game.states.gameplay->mobs.pikmin_list[p];
            }
        }
    }
    
    if(d) *d = closest_distance;
    return closest_pikmin;
}
