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

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"
#include "../../util/string_utils.h"
#include "../mob_script/pikmin_fsm.h"
#include "mob_enums.h"
#include "mob.h"


namespace PIKMIN {

//Maximum amount of time for the random boredom animation delay.
const float BORED_ANIM_MAX_DELAY = 5.0f;

//Minimum amount of time for hte random boredom animation delay.
const float BORED_ANIM_MIN_DELAY = 1.0f;

//Chance of circling the opponent instead of striking, when grounded.
const float CIRCLE_OPPONENT_CHANCE_GROUNDED = 0.2f;

//Chance of circling the opponent instead of latching, if it can latch.
const float CIRCLE_OPPONENT_CHANCE_PRE_LATCH = 0.5f;

//Time until moving Pikmin timeout and stay in place, after being dismissed.
const float DISMISS_TIMEOUT = 4.0f;

//Height above the floor that a flying Pikmin prefers to stay at.
const float FLIER_ABOVE_FLOOR_HEIGHT = 55.0f;

//Timeout before a Pikmin gives up, when ordered to go to something.
const float GOTO_TIMEOUT = 5.0f;

//If the Pikmin is within this distance of the mob, it can ground attack.
const float GROUNDED_ATTACK_DIST = 5.0f;

//The idle glow spins these many radians per second.
const float IDLE_GLOW_SPIN_SPEED = TAU / 4;

//Invulnerability period after getting hit.
const float INVULN_PERIOD = 0.7f;

//How long to remember a missed incoming attack for.
const float MISSED_ATTACK_DURATION = 1.5f;

//Interval for when a Pikmin decides a new chase spot, when panicking.
const float PANIC_CHASE_INTERVAL = 0.2f;

//A plucked Pikmin is thrown behind the leader at this speed, horizontally.
const float THROW_HOR_SPEED = 80.0f;

//A plucked Pikmin is thrown behind the leader at this speed, vertically.
const float THROW_VER_SPEED = 900.0f;

}




/**
 * @brief Constructs a new Pikmin object.
 *
 * @param pos Starting coordinates.
 * @param type Pikmin type this mob belongs to.
 * @param angle Starting angle.
 */
pikmin::pikmin(const point &pos, pikmin_type* type, float angle) :
    mob(pos, type, angle),
    pik_type(type) {
    
    invuln_period = timer(PIKMIN::INVULN_PERIOD);
    team = MOB_TEAM_PLAYER_1;
    subgroup_type_ptr =
        game.states.gameplay->subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, pik_type
        );
    near_reach = 0;
    far_reach = 2;
    update_interaction_span();
    missed_attack_timer =
        timer(
            PIKMIN::MISSED_ATTACK_DURATION,
    [this] () { this->missed_attack_ptr = nullptr; }
        );
        
    if(pik_type->can_fly) {
        enable_flag(flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    }
}


/**
 * @brief Returns whether or not a Pikmin can receive a given status effect.
 *
 * @param s Status type to check.
 * @return Whether it can receive it.
 */
bool pikmin::can_receive_status(status_type* s) const {
    return has_flag(s->affects, STATUS_AFFECTS_FLAG_PIKMIN);
}


/**
 * @brief Draws a Pikmin, including its leaf/bud/flower, idle glow, etc.
 */
void pikmin::draw_mob() {
    sprite* cur_s_ptr;
    sprite* next_s_ptr;
    float interpolation_factor;
    get_sprite_data(&cur_s_ptr, &next_s_ptr, &interpolation_factor);
    if(!cur_s_ptr) return;
    
    //The Pikmin itself.
    bitmap_effect_t mob_eff;
    get_sprite_bitmap_effects(
        cur_s_ptr, next_s_ptr, interpolation_factor,
        &mob_eff,
        SPRITE_BMP_EFFECT_FLAG_STATUS |
        SPRITE_BMP_EFFECT_FLAG_SECTOR_BRIGHTNESS |
        SPRITE_BMP_EFFECT_FLAG_HEIGHT |
        SPRITE_BMP_EFFECT_DELIVERY
    );
    bitmap_effect_t pik_sprite_eff = mob_eff;
    get_sprite_bitmap_effects(
        cur_s_ptr, next_s_ptr, interpolation_factor,
        &pik_sprite_eff,
        SPRITE_BMP_EFFECT_FLAG_STANDARD
    );
    
    bool is_idle =
        fsm.cur_state->id == PIKMIN_STATE_IDLING ||
        fsm.cur_state->id == PIKMIN_STATE_IDLING_H ||
        fsm.cur_state->id == PIKMIN_STATE_SPROUT;
        
    if(is_idle) {
        mob_eff.glow_color = COLOR_WHITE;
    }
    
    draw_bitmap_with_effects(cur_s_ptr->bitmap, pik_sprite_eff);
    
    //Top.
    if(cur_s_ptr->top_visible) {
        point top_coords;
        float top_angle;
        point top_size;
        bitmap_effect_t top_eff = mob_eff;
        ALLEGRO_BITMAP* top_bmp = pik_type->bmp_top[maturity];
        get_sprite_basic_top_effects(
            cur_s_ptr, next_s_ptr, interpolation_factor,
            &top_coords, &top_angle, &top_size
        );
        //To get the height effect to work, we'll need to scale the translation
        //too, otherwise the top will detach from the Pikmin visually as
        //the Pikmin falls into a pit. The "right" scale is a bit of a guess
        //at this point in the code, but honestly, either X scale or Y scale
        //will work. In the off-chance they are different, using an average
        //will be more than enough.
        float avg_scale = (top_eff.scale.x + top_eff.scale.y) / 2.0f;
        point top_bmp_size = get_bitmap_dimensions(top_bmp);
        top_eff.translation +=
            pos + rotate_point(top_coords, angle) * avg_scale;
        top_eff.scale *= top_size / top_bmp_size;
        top_eff.rotation +=
            angle + top_angle;
        top_eff.glow_color =
            map_gray(0);
            
        draw_bitmap_with_effects(top_bmp, top_eff);
    }
    
    //Idle glow.
    if(is_idle) {
        bitmap_effect_t idle_eff = pik_sprite_eff;
        point glow_bmp_size =
            get_bitmap_dimensions(game.sys_content.bmp_idle_glow);
        idle_eff.translation = pos;
        idle_eff.scale =
            (game.config.standard_pikmin_radius * 8) / glow_bmp_size;
        idle_eff.rotation =
            game.states.gameplay->area_time_passed *
            PIKMIN::IDLE_GLOW_SPIN_SPEED;
        idle_eff.tint_color = type->main_color;
        idle_eff.glow_color = map_gray(0);
        
        draw_bitmap_with_effects(game.sys_content.bmp_idle_glow, idle_eff);
    }
    
    draw_status_effect_bmp(this, pik_sprite_eff);
}


/**
 * @brief Logic specific to Pikmin for when they finish dying.
 */
void pikmin::finish_dying_class_specifics() {
    //Essentials.
    to_delete = true;
    
    //Soul.
    particle par(
        pos, LARGE_FLOAT,
        radius * 2, 2.0f
    );
    par.bitmap = game.sys_content.bmp_pikmin_spirit;
    par.friction = 0.8;
    point base_speed = point(game.rng.f(-20, 20), game.rng.f(-70, -30));
    par.linear_speed = keyframe_interpolator<point>(base_speed);
    par.linear_speed.add(1, point(point(base_speed.x, base_speed.y - 20)));
    par.color.set_keyframe_value(0, change_alpha(pik_type->main_color, 0));
    par.color.add(0.1f, pik_type->main_color);
    par.color.add(1, change_alpha(pik_type->main_color, 0));
    game.states.gameplay->particles.add(par);
    
    //Sound. Create a positional sound source instead of a mob sound source,
    //since the Pikmin object is now practically deleted.
    size_t dying_sound_idx =
        pik_type->sound_data_idxs[PIKMIN_SOUND_DYING];
    if(dying_sound_idx != INVALID) {
        mob_type::sound_t* dying_sound =
            &type->sounds[dying_sound_idx];
        game.audio.create_pos_sound_source(
            dying_sound->sample,
            pos, false, dying_sound->config
        );
    }
}


/**
 * @brief Forces the Pikmin to start carrying the given mob.
 * This quickly runs over several steps in the usual FSM logic, just to
 * instantly get to the end result.
 * As such, be careful when using it.
 *
 * @param m The mob to carry.
 */
void pikmin::force_carry(mob* m) {
    fsm.set_state(PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT, (void*) m, nullptr);
    fsm.run_event(MOB_EV_REACHED_DESTINATION);
}


/**
 * @brief Returns a Pikmin's base speed, without status effects and the like.
 * This depends on the maturity.
 *
 * @return The base speed.
 */
float pikmin::get_base_speed() const {
    float base = mob::get_base_speed();
    return base + (base * this->maturity * game.config.maturity_speed_mult);
}


/**
 * @brief Returns its group spot information.
 * Basically, when it's in a leader's group, what point it should be following,
 * and within what distance.
 *
 * @param out_spot The final coordinates are returned here.
 * @param out_dist The final distance to those coordinates is returned here.
 */
void pikmin::get_group_spot_info(
    point* out_spot, float* out_dist
) const {
    out_spot->x = 0.0f;
    out_spot->y = 0.0f;
    *out_dist = 0.0f;
    
    if(!following_group || !following_group->group) {
        return;
    }
    
    *out_spot =
        following_group->group->anchor +
        following_group->group->get_spot_offset(group_spot_idx);
    *out_dist = 5.0f;
}


/**
 * @brief Handles a status effect being applied.
 *
 * @param sta_type Status effect to handle.
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
    
    if(carrying_mob) {
        carrying_mob->chase_info.max_speed =
            carrying_mob->carry_info->get_speed();
    }
}


/**
 * @brief Handles a status effect being removed.
 *
 * @param sta_type Status effect to handle.
 */
void pikmin::handle_status_effect_loss(status_type* sta_type) {
    bool still_has_flailing = false;
    bool still_has_helplessness = false;
    bool still_has_panic = false;
    for(size_t s = 0; s < statuses.size(); s++) {
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
        set_animation(PIKMIN_ANIM_SHAKING);
        in_shaking_animation = true;
        set_timer(0); //The boredom animation timeout.
        pikmin_fsm::stand_still(this, nullptr, nullptr);
        invuln_period.start();
    }
    
    if(
        sta_type->state_change_type == STATUS_STATE_CHANGE_HELPLESS &&
        !still_has_helplessness &&
        fsm.cur_state->id == PIKMIN_STATE_HELPLESS
    ) {
        fsm.set_state(PIKMIN_STATE_IDLING);
        pikmin_fsm::stand_still(this, nullptr, nullptr);
        invuln_period.start();
        
    } else if(
        sta_type->state_change_type == STATUS_STATE_CHANGE_PANIC &&
        !still_has_panic &&
        fsm.cur_state->id == PIKMIN_STATE_PANICKING
    ) {
        fsm.set_state(PIKMIN_STATE_IDLING);
        pikmin_fsm::stand_still(this, nullptr, nullptr);
        invuln_period.start();
        
    }
    
    if(carrying_mob) {
        carrying_mob->chase_info.max_speed =
            carrying_mob->carry_info->get_speed();
    }
}


/**
 * @brief Increases (or decreases) the Pikmin's maturity by the given amount.
 * This makes sure that the maturity doesn't overflow.
 *
 * @param amount Amount to increase by.
 * @return Whether it changed the maturity.
 */
bool pikmin::increase_maturity(int amount) {
    int old_maturity = maturity;
    int new_maturity = maturity + amount;
    maturity = clamp(new_maturity, 0, N_MATURITIES - 1);
    
    if(maturity > old_maturity) {
        game.statistics.pikmin_blooms++;
    }
    return maturity != old_maturity;
}


/**
 * @brief Latches on to the specified mob.
 *
 * @param m Mob to latch on to.
 * @param h Hitbox to latch on to.
 */
void pikmin::latch(mob* m, const hitbox* h) {
    speed.x = speed.y = speed_z = 0;
    
    float h_offset_dist;
    float h_offset_angle;
    float v_offset_dist;
    m->get_hitbox_hold_point(
        this, h, &h_offset_dist, &h_offset_angle, &v_offset_dist
    );
    m->hold(
        this, h->body_part_idx, h_offset_dist, h_offset_angle, v_offset_dist,
        true,
        HOLD_ROTATION_METHOD_NEVER //pikmin_fsm::prepare_to_attack handles it.
    );
    
    latched = true;
}


/**
 * @brief Checks if an incoming attack should miss, and returns the result.
 *
 * If it was already decided that it missed in a previous frame, that's a
 * straight no. If not, it will roll with the hit rate to check.
 * If the attack is a miss, it also registers the miss, so that we can keep
 * memory of it for the next frames.
 *
 * @param info Info about the hitboxes involved.
 * @return Whether it hit.
 */
bool pikmin::process_attack_miss(hitbox_interaction* info) {
    if(info->mob2->anim.cur_anim == missed_attack_ptr) {
        //In a previous frame, we had already considered this animation a miss.
        return false;
    }
    
    unsigned char hit_rate = info->mob2->anim.cur_anim->hit_rate;
    if(hit_rate == 0) return false;
    
    unsigned char hit_roll = game.rng.i(0, 100);
    if(hit_roll > hit_rate) {
        //This attack was randomly decided to be a miss.
        //Record this animation so it won't be considered a hit next frame.
        missed_attack_ptr = info->mob2->anim.cur_anim;
        missed_attack_timer.start();
        return false;
    }
    
    return true;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void pikmin::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    int maturity_var;
    bool sprout_var;
    bool follow_link_var;
    
    if(svr.get("maturity", maturity_var)) {
        maturity = clamp(maturity_var, 0, N_MATURITIES - 1);
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


/**
 * @brief Sets up stuff for the beginning of the Pikmin's death process.
 */
void pikmin::start_dying_class_specifics() {
    game.states.gameplay->pikmin_deaths++;
    game.states.gameplay->pikmin_deaths_per_type[pik_type]++;
    game.states.gameplay->last_pikmin_death_pos = pos;
    game.statistics.pikmin_deaths++;
    
    enable_flag(flags, MOB_FLAG_INTANGIBLE);
}


/**
 * @brief Starts the particle generator that leaves a trail behind
 * a thrown Pikmin.
 */
void pikmin::start_throw_trail() {
    particle_generator pg =
        standard_particle_gen_setup(
            game.sys_content_names.part_throw_trail, this
        );
    pg.follow_z_offset = 0.0f;
    adjust_keyframe_interpolator_values<float>(
        pg.base_particle.size,
    [ = ] (const float & f) { return f * radius; }
    );
    adjust_keyframe_interpolator_values<ALLEGRO_COLOR>(
        pg.base_particle.color,
    [ = ] (const ALLEGRO_COLOR & c) {
        ALLEGRO_COLOR new_c = c;
        new_c.r *= type->main_color.r;
        new_c.g *= type->main_color.g;
        new_c.b *= type->main_color.b;
        new_c.a *= type->main_color.a;
        return new_c;
    }
    );
    pg.id = MOB_PARTICLE_GENERATOR_ID_THROW;
    particle_generators.push_back(pg);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void pikmin::tick_class_specifics(float delta_t) {
    //Carrying object.
    if(carrying_mob) {
        if(!carrying_mob->carry_info) {
            fsm.run_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE);
        }
    }
    
    //Tick some timers.
    missed_attack_timer.tick(delta_t);
    bump_lock = std::max(bump_lock - delta_t, 0.0f);
    
    //Forcefully follow another mob as a leader.
    if(must_follow_link_as_leader) {
        if(!links.empty() && links[0]) {
            fsm.run_event(
                MOB_EV_TOUCHED_ACTIVE_LEADER,
                (void*) (links[0]),
                (void*) 1 //Be silent.
            );
        }
        //Since this leader is likely an enemy, let's keep these Pikmin safe.
        enable_flag(flags, MOB_FLAG_NON_HUNTABLE);
        enable_flag(flags, MOB_FLAG_NON_HURTABLE);
        must_follow_link_as_leader = false;
    }
    
}


/**
 * @brief Returns the sprout closest to a leader. Used when auto-plucking.
 *
 * @param pos Coordinates of the leader.
 * @param d Variable to return the distance to. nullptr for none.
 * @param ignore_reserved If true, ignore any sprouts that are "reserved"
 * (i.e. already chosen to be plucked by another leader).
 * @return The sprout.
 */
pikmin* get_closest_sprout(
    const point &pos, dist* d, bool ignore_reserved
) {
    dist closest_distance;
    pikmin* closest_pikmin = nullptr;
    
    size_t n_pikmin = game.states.gameplay->mobs.pikmin_list.size();
    for(size_t p = 0; p < n_pikmin; p++) {
        if(
            game.states.gameplay->mobs.pikmin_list[p]->fsm.cur_state->id !=
            PIKMIN_STATE_SPROUT
        ) {
            continue;
        }
        
        dist dis(pos, game.states.gameplay->mobs.pikmin_list[p]->pos);
        if(closest_pikmin == nullptr || dis < closest_distance) {
        
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
