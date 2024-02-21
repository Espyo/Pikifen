/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin class and Pikmin-related functions.
 */

#ifndef PIKMIN_INCLUDED
#define PIKMIN_INCLUDED

class leader;

#include "../mob_types/pikmin_type.h"
#include "enemy.h"
#include "leader.h"
#include "onion.h"


namespace PIKMIN {
extern const float DEF_KNOCKED_DOWN_DURATION;
extern const float DEF_KNOCKED_DOWN_WHISTLE_BONUS;
extern const float DISMISS_TIMEOUT;
extern const float GOTO_TIMEOUT;
extern const float GROUNDED_ATTACK_DIST;
extern const float IDLE_GLOW_SPIN_SPEED;
extern const float INVULN_PERIOD;
extern const float PANIC_CHASE_INTERVAL;
extern const float THROW_HOR_SPEED;
extern const float THROW_VER_SPEED;
}


/* ----------------------------------------------------------------------------
 * The eponymous Pikmin.
 */
class pikmin : public mob {
public:
    //What type of Pikmin it is.
    pikmin_type* pik_type;
    
    //Mob that it is carrying.
    mob* carrying_mob;
    //The Pikmin is considering this attack animation as having "missed".
    animation* missed_attack_ptr;
    //The Pikmin will consider the miss for this long.
    timer missed_attack_timer;
    //Did the Pikmin's last attack cause zero damage?
    bool was_last_hit_dud;
    //How many hits in a row have done no damage.
    unsigned char consecutive_dud_hits;
    
    //Maturity. 0: leaf. 1: bud. 2: flower.
    unsigned char maturity;
    //Is this Pikmin currently a seed or a sprout?
    bool is_seed_or_sprout;
    //Is this Pikmin currently grabbed by an enemy?
    bool is_grabbed_by_enemy;
    //If true, someone's already coming to pluck this Pikmin.
    //This is to let other leaders know that they should pick a different one.
    bool pluck_reserved;
    //Leader it is meant to return to after what it is doing, if any.
    mob* leader_to_return_to;
    //Is this Pikmin latched on to a mob?
    bool latched;
    //Is the Pikmin holding a tool and ready to drop it on whistle?
    bool is_tool_primed_for_whistle;
    //Does this Pikmin have to follow its linked mob as its leader?
    bool must_follow_link_as_leader;
    //Leader bump lock. Leaders close and timer running = timer resets.
    float bump_lock;
    
    //Temporary variable. Hacky, but effective. Only use within the same state!
    size_t temp_i;
    
    //Forces the Pikmin to carry a mob.
    void force_carry(mob* m);
    //Checks and processes an attack's miss.
    bool process_attack_miss(hitbox_interaction* info);
    //Increases the Pikmin's maturity by an amount.
    void increase_maturity(const int amount);
    //Latches on to the specified mob.
    void latch(mob* m, hitbox* h);
    //Starts the trail behind a thrown Pikmin.
    void start_throw_trail();
    
    //Constructor.
    pikmin(const point &pos, pikmin_type* type, const float angle);
    
    //Can the mob currently receive the specified status effect?
    bool can_receive_status(status_type* s) const override;
    //Mob drawing routine.
    void draw_mob() override;
    //Get the base movement speed.
    float get_base_speed() const override;
    //Return the coords and distance of its spot in the group.
    void get_group_spot_info(
        point* final_spot, float* final_dist
    ) const override;
    //Handler for a status effect being applied.
    void handle_status_effect_gain(status_type* s) override;
    //Handler for a status effect being removed.
    void handle_status_effect_loss(status_type* s) override;
    //Read script variables from the area data.
    void read_script_vars(const script_var_reader &svr) override;
    
    static const float CIRCLE_OPPONENT_CHANCE_GROUNDED;
    static const float CIRCLE_OPPONENT_CHANCE_PRE_LATCH;
    static const float FLIER_ABOVE_FLOOR_HEIGHT;
    static const float MISSED_ATTACK_DURATION;
    
protected:
    //Tick class-specific logic.
    void tick_class_specifics(const float delta_t) override;
};


pikmin* get_closest_sprout(
    const point &pos, dist* d, const bool ignore_reserved
);


#endif //ifndef PIKMIN_INCLUDED
