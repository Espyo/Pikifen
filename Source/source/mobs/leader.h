/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader class and leader-related functions.
 */

#ifndef LEADER_INCLUDED
#define LEADER_INCLUDED

#include <vector>

#include "../mob_types/leader_type.h"
#include "mob.h"


class pikmin;

using std::size_t;


/* ----------------------------------------------------------------------------
 * A leader controls Pikmin, and
 * is controlled by the player.
 */
class leader : public mob {
public:
    //What type of leader it is.
    leader_type* lea_type;
    
    //Is it active? i.e. being controlled by a player.
    bool active;
    //Is it currently auto-plucking?
    bool auto_plucking;
    //Pikmin it wants to pluck.
    pikmin* pluck_target;
    //Has the player asked for the auto-plucking to stop?
    bool queued_pluck_cancel;
    //Is the leader currently in the walking animation?
    bool is_in_walking_anim;
    //Time left before the leader can throw again.
    float throw_cooldown;
    //Whether or not a throw has been queued to be pulled off.
    bool throw_queued;
    //Is auto-throw mode on?
    bool auto_throwing;
    //Time left before the next auto-throw.
    float auto_throw_cooldown;
    //When the auto-throw cooldown restarts, set it to this value.
    float auto_throw_cooldown_duration;
    //Provided there's a throw, this is the mob to throw.
    mob* throwee;
    //Provided there's a throw, this is the angle.
    float throwee_angle;
    //Provided there's a throw, this is the max Z.
    float throwee_max_z;
    //Provided there's a throw, this is the horizontal speed.
    point throwee_speed;
    //Provided there's a throw, this is the vertical speed.
    float throwee_speed_z;
    //Provided there's a throw, this indicates whether it's low enough to reach.
    bool throwee_can_reach;
    //How much the health wheel is filled. Gradually moves to the target amount.
    float health_wheel_visible_ratio;
    
    //Returns whether or not a leader can throw.
    bool check_throw_ok() const;
    //Dismiss current group.
    void dismiss();
    //Order some Pikmin to get in the Onion.
    bool order_pikmin_to_onion(
        pikmin_type* type, pikmin_nest_struct* n_ptr, const size_t amount
    );
    //Queues up a throw.
    void queue_throw();
    //Signal to every group member that swarm mode started.
    void signal_swarm_start() const;
    //Signal to every group member that swarm mode ended.
    void signal_swarm_end() const;
    //Starts auto-throw mode.
    void start_auto_throwing();
    //Starts the trail behind a thrown leader.
    void start_throw_trail();
    //Start whistling.
    void start_whistling();
    //Stops the auto-throw mode.
    void stop_auto_throwing();
    //Stop whistling.
    void stop_whistling();
    //Change the current held Pikmin for another.
    void swap_held_pikmin(mob* new_pik);
    
    //Constructor.
    leader(const point &pos, leader_type* type, const float angle);
    
    //Can the mob currently receive the specified status effect?
    virtual bool can_receive_status(status_type* s) const;
    //Return the coords and distance of its spot in the group.
    virtual void get_group_spot_info(
        point* final_spot, float* final_dist
    ) const;
    //Mob drawing routine.
    virtual void draw_mob();
    
    static const float AUTO_THROW_COOLDOWN_MAX_DURATION;
    static const float AUTO_THROW_COOLDOWN_MIN_DURATION;
    static const float AUTO_THROW_COOLDOWN_SPEED;
    static const float THROW_COOLDOWN_DURATION;
    
protected:
    //Tick class-specific logic.
    virtual void tick_class_specifics(const float delta_t);
    
private:
    //Returns how many rows are needed for all members' dismissal.
    size_t get_dismiss_rows(const size_t n_members) const;
    //Update variables related to how the leader's throw would go.
    void update_throw_variables();
};


void change_to_next_leader(const bool forward, const bool force_success);
bool grab_closest_group_member();
bool process_total_leader_ko();

#endif //ifndef LEADER_INCLUDED
