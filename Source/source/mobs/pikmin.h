/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin class and Pikmin-related functions.
 */

#pragma once

class leader;

#include "../mob_types/pikmin_type.h"
#include "enemy.h"
#include "leader.h"
#include "onion.h"


namespace PIKMIN {
extern const float CIRCLE_OPPONENT_CHANCE_GROUNDED;
extern const float CIRCLE_OPPONENT_CHANCE_PRE_LATCH;
extern const float DISMISS_TIMEOUT;
extern const float FLIER_ABOVE_FLOOR_HEIGHT;
extern const float GOTO_TIMEOUT;
extern const float GROUNDED_ATTACK_DIST;
extern const float IDLE_GLOW_SPIN_SPEED;
extern const float INVULN_PERIOD;
extern const float MISSED_ATTACK_DURATION;
extern const float PANIC_CHASE_INTERVAL;
extern const float THROW_HOR_SPEED;
extern const float THROW_VER_SPEED;
}


/**
 * @brief The eponymous Pikmin.
 */
class pikmin : public mob {

public:

    //--- Members ---

    //What type of Pikmin it is.
    pikmin_type* pik_type = nullptr;
    
    //Mob that it is carrying.
    mob* carrying_mob = nullptr;

    //The Pikmin is considering this attack animation as having "missed".
    animation* missed_attack_ptr = nullptr;

    //The Pikmin will consider the miss for this long.
    timer missed_attack_timer;

    //Did the Pikmin's last attack cause zero damage?
    bool was_last_hit_dud = false;

    //How many hits in a row have done no damage.
    unsigned char consecutive_dud_hits = 0;
    
    //Maturity. 0: leaf. 1: bud. 2: flower.
    unsigned char maturity = 2;

    //Is this Pikmin currently a seed or a sprout?
    bool is_seed_or_sprout = false;

    //Is this Pikmin currently grabbed by an enemy?
    bool is_grabbed_by_enemy = false;

    //If true, someone's already coming to pluck this Pikmin.
    //This is to let other leaders know that they should pick a different one.
    bool pluck_reserved = false;

    //Leader it is meant to return to after what it is doing, if any.
    mob* leader_to_return_to = nullptr;

    //Is this Pikmin latched on to a mob?
    bool latched = false;

    //Is the Pikmin holding a tool and ready to drop it on whistle?
    bool is_tool_primed_for_whistle = false;

    //Does this Pikmin have to follow its linked mob as its leader?
    bool must_follow_link_as_leader = false;

    //Leader bump lock. Leaders close and timer running = timer resets.
    float bump_lock = 0.0f;
    
    //Temporary variable. Hacky, but effective. Only use within the same state!
    size_t temp_i = 0;
    

    //--- Function declarations ---
    
    pikmin(const point &pos, pikmin_type* type, const float angle);
    void force_carry(mob* m);
    bool process_attack_miss(hitbox_interaction* info);
    void increase_maturity(const int amount);
    void latch(mob* m, const hitbox* h);
    void start_throw_trail();
    bool can_receive_status(status_type* s) const override;
    void draw_mob() override;
    float get_base_speed() const override;
    void get_group_spot_info(
        point* out_spot, float* out_dist
    ) const override;
    void handle_status_effect_gain(status_type* s) override;
    void handle_status_effect_loss(status_type* s) override;
    void read_script_vars(const script_var_reader &svr) override;
    
protected:

    //--- Function declarations ---
    
    void tick_class_specifics(const float delta_t) override;
};


pikmin* get_closest_sprout(
    const point &pos, dist* d, const bool ignore_reserved
    
);
