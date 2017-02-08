/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob class and mob-related functions.
 */

#ifndef MOB_INCLUDED
#define MOB_INCLUDED

#include <map>
#include <vector>

#include <allegro5/allegro.h>

#include "../animation.h"
#include "../misc_structs.h"
#include "../mob_script.h"
#include "../particle.h"
#include "pikmin_type.h"
#include "../sector.h"
#include "../status.h"

using namespace std;

struct group_spot_info;
class mob_type;
class mob;

extern size_t next_mob_id;

//Accelerate the Z speed of mobs affected by gravity by this amount per second.
const float GRAVITY_ADDER = -1300.0f;
const float MOB_PUSH_EXTRA_AMOUNT = 50.0f;
const float MOB_KNOCKBACK_H_POWER = 130.0f;
const float MOB_KNOCKBACK_V_POWER = 200.0f;

enum MOB_TEAMS {
    //Can hurt/target anyone and be hurt/targeted by anyone, on any team.
    MOB_TEAM_NONE,
    MOB_TEAM_PLAYER_1,
    MOB_TEAM_PLAYER_2,
    MOB_TEAM_PLAYER_3,
    MOB_TEAM_PLAYER_4,
    MOB_TEAM_ENEMY_1,
    MOB_TEAM_ENEMY_2,
    //Can only be hurt by Pikmin.
    MOB_TEAM_OBSTACLE,
    //Cannot be hurt or targeted by anything.
    MOB_TEAM_DECORATION,
};

enum MOB_STATE_IDS {
    MOB_STATE_IDLE,
    MOB_STATE_BEING_CARRIED,
    MOB_STATE_BEING_DELIVERED, //Into an Onion.
    
};

enum MOB_PARTICLE_GENERATOR_IDS {
    MOB_PARTICLE_GENERATOR_NONE,
    //Custom particle generator issued by the script.
    MOB_PARTICLE_GENERATOR_SCRIPT,
    //Trail effect left behind by a throw.
    MOB_PARTICLE_GENERATOR_THROW,
    //Ring-shaped wave when going in water.
    MOB_PARTICLE_GENERATOR_WAVE_RING,
    
    //Specific status effects are numbered starting on this.
    //So make sure this is the last on the enum.
    MOB_PARTICLE_GENERATOR_STATUS,
};

enum DISABLED_STATE_FLAGS {
    //The Pikmin can be eaten by enemies.
    DISABLED_STATE_FLAG_INEDIBLE = 1,
};

enum PARTICLE_PRIORITIES {
    PARTICLE_PRIORITY_LOW,
    PARTICLE_PRIORITY_MEDIUM,
    PARTICLE_PRIORITY_HIGH,
};

enum CARRY_SPOT_STATES {
    CARRY_SPOT_FREE,
    CARRY_SPOT_RESERVED,
    CARRY_SPOT_USED,
};


/* ----------------------------------------------------------------------------
 * Information on a mob's group.
 * This includes a list of its members,
 * and the location and info of the spots in the
 * circle, when the members are following the mob.
 */
struct group_info {
    vector<mob*> members;
    group_spot_info* group_spots;
    float group_center_x, group_center_y;
    
    group_info(
        group_spot_info* ps, const float center_x, const float center_y
    ) {
        group_spots = ps;
        group_center_x = center_x;
        group_center_y = center_y;
    }
};


/* ----------------------------------------------------------------------------
 * Information on a carrying spot around a mob's perimeter.
 */
struct carrier_spot_struct {
    unsigned char state;
    //Relative coordinates of each spot.
    //They avoid calculating several sines and cosines over and over.
    float x;
    float y;
    mob* pik_ptr;
    carrier_spot_struct(const float x = 0, const float y = 0);
};


/* ----------------------------------------------------------------------------
 * Structure with information on how
 * the mob should be carried.
 */
struct carry_info_struct {
    mob* m;
    //If true, this is carried to the ship.
    //Otherwise, it's carried to an Onion.
    bool carry_to_ship;
    
    vector<carrier_spot_struct> spot_info;
    
    //This is to avoid going through the vector
    //only to find out the total strength.
    float cur_carrying_strength;
    //Likewise, this is to avoid going through the vector
    //only to find out the number. Note that this is the number
    //of spaces reserved. A Pikmin could be on its way to its spot,
    //not necessarily there already.
    size_t cur_n_carriers;
    float final_destination_x;
    float final_destination_y;
    //If the path has an obstacle, this is the pointer to it.
    //This not being NULL also means the last stop in the path is
    //the stop before the obstacle.
    mob* obstacle_ptr;
    //If true, it's best to go straight to the end point
    //instead of taking a path.
    bool go_straight;
    //Are the Pikmin stuck with nowhere to go?
    //0: no. 1: going to the alternative point, 2: going back to the start.
    unsigned char stuck_state;
    bool is_moving;
    
    carry_info_struct(mob* m, const bool carry_to_ship);
    bool is_full();
    float get_speed();
    ~carry_info_struct();
};


/* ----------------------------------------------------------------------------
 * A mob, short for "mobile object" or "map object",
 * or whatever tickles your fancy, is any instance of
 * an object in the game world. It can move, follow a point,
 * has health, and can be a variety of different sub-types,
 * like leader, Pikmin, enemy, Onion, etc.
 */
class mob {
protected:
    void tick_animation();
    void tick_brain();
    void tick_misc_logic();
    void tick_physics();
    void tick_script();
    virtual void tick_class_specifics();
    
public:
    mob(
        const float x, const float y, mob_type* type,
        const float angle, const string &vars
    );
    virtual ~mob(); //Needed so that typeid works.
    
    mob_type* type;
    
    animation_instance anim;
    
    //Flags.
    bool to_delete; //If true, this mob should be deleted.
    bool reached_destination;
    
    //Actual moving and other physics.
    //Coordinates. Z is height, the higher the value, the higher in the sky.
    float x, y, z;
    //Speed variables for physics only. Don't touch.
    float speed_x, speed_y, speed_z;
    //Starting coordinates; what the mob calls "home".
    float home_x, home_y;
    //Speed multiplies by this much each second. //TODO use this.
    float acceleration;
    //Speed moving forward. //TODO is this used?
    float speed;
    //0: Right. PI*0.5: Up. PI: Left. PI*1.5: Down.
    float angle;
    //Angle the mob wants to be facing.
    float intended_angle;
    //The highest ground below the entire mob.
    sector* ground_sector;
    //Sector that the mob's center is on.
    sector* center_sector;
    //Multiply the mob's gravity by this.
    float gravity_mult;
    //Amount it's being pushed by another mob.
    float push_amount;
    //Angle that another mob is pushing it to.
    float push_angle;
    //If it can be touched by other mobs.
    bool tangible;
    //If it should be hidden (no shadow, no health).
    bool hide;
    
    //Makes the mob face an angle, but it'll turn at its own pace.
    void face(const float new_angle);
    //Returns the final coordinates of the chasing target.
    void get_chase_target(float* x, float* y);
    //Returns the normal speed of this mob.
    //Subclasses are meant to override this.
    virtual float get_base_speed();
    
    //Target things.
    //If true, the mob is trying to go to a certain spot.
    bool chasing;
    //Chase after these coordinates, relative to the "origin" coordinates.
    float chase_offs_x, chase_offs_y;
    //Pointers to the origin of the coordinates, or NULL for the world origin.
    float* chase_orig_x, *chase_orig_y;
    //When chasing something in teleport mode, also change the z accordingly.
    float* chase_teleport_z;
    //If true, teleport instantly.
    bool chase_teleport;
    //If true, the mob can move in a direction it's not facing.
    bool chase_free_move;
    //Distance from the target in which the mob is considered as being there.
    float chase_target_dist;
    //Speed to move towards the target at.
    float chase_speed;
    vector<path_stop*> path;
    size_t cur_path_stop_nr;
    
    void chase(
        const float offs_x, const float offs_y,
        float* orig_x, float* orig_y,
        const bool teleport, float* teleport_z = NULL,
        const bool free_move = false, const float target_distance = 3,
        const float speed = -1
    );
    void stop_chasing();
    
    //Group things.
    //The current mob is following this mob's group.
    mob* following_group;
    //Is the mob airborne because it was thrown?
    bool was_thrown;
    //Info on the group this mob is a leader of.
    group_info* group;
    float group_spot_x;
    float group_spot_y;
    
    //Script.
    //Finite-state machine.
    mob_fsm fsm;
    //The mob it has focus on.
    mob* focused_mob;
    //The timer.
    timer script_timer;
    //Variables.
    map<string, string> vars;
    //Are we waiting to report the big damage event?
    bool big_damage_ev_queued;
    
    //Other properties.
    //Incremental ID. Used for minor things.
    size_t id;
    //Current health.
    float health;
    //During this period, the mob cannot be attacked.
    timer invuln_period;
    //Mob's team (who it can damage); use MOB_TEAM_*.
    unsigned char team;
    //Particle generators attached to it.
    vector<particle_generator> particle_generators;
    
    //Status effects currently inflicted on the mob.
    vector<status> statuses;
    //Hazard of the sector the mob is currently on.
    hazard* on_hazard;
    //Is the mob dead?
    bool dead;
    //List of hitboxes that will chomp Pikmin.
    vector<int> chomp_hitboxes;
    //Mobs it is chomping.
    vector<mob*> chomping_pikmin;
    //Max mobs it can chomp in the current attack.
    size_t chomp_max;
    
    //Carrying.
    //Structure holding information on how this mob should be carried.
    //If NULL, it cannot be carried.
    carry_info_struct* carry_info;
    void become_carriable(const bool to_ship);
    void become_uncarriable();
    
    void set_animation(const size_t nr, const bool pre_named = true);
    void set_health(const bool rel, const float amount);
    void set_timer(const float time);
    void set_var(const string &name, const string &value);
    
    void eat(size_t nr);
    void start_dying();
    void finish_dying();
    
    void apply_status_effect(status_type* s, const bool refill);
    void delete_old_status_effects();
    void remove_particle_generator(const size_t id);
    void add_status_sprite_effects(sprite_effect_manager* manager);
    ALLEGRO_BITMAP* get_status_bitmap(float* bmp_scale);
    //If the mob is currently "disabled", these flags specify behavior.
    unsigned char disabled_state_flags;
    virtual bool can_receive_status(status_type* s);
    virtual void receive_disable_from_status(const unsigned char flags);
    virtual void receive_flailing_from_status();
    virtual void receive_panic_from_status();
    virtual void lose_panic_from_status();
    virtual void change_maturity_amount_from_status(const int amount);
    
    void tick();
    virtual void draw(sprite_effect_manager* effect_manager = NULL);
    
    static void attack(
        mob* m1, mob* m2, const bool m1_is_pikmin, const float damage,
        const float angle, const float knockback,
        const float new_invuln_period, const float new_knockdown_period
    );
    void calculate_carrying_destination(mob* added, mob* removed);
    mob* carrying_target;
    
    //Drawing tools.
    void get_sprite_center(mob* m, sprite* s, float* x, float* y);
    void get_sprite_dimensions(
        mob* m, sprite* s, float* w, float* h, float* scale = NULL
    );
    void add_brightness_sprite_effect(sprite_effect_manager* manager);
    void add_delivery_sprite_effect(
        sprite_effect_manager* manager, const float delivery_time_ratio_left,
        const ALLEGRO_COLOR &onion_color
    );
    
};


void add_to_group(mob* group_leader, mob* new_member);
void apply_knockback(
    mob* m, const float knockback, const float knockback_angle
);
float calculate_damage(
    mob* attacker, mob* victim, hitbox* attacker_h,
    hitbox* victim_h
);
void calculate_knockback(
    mob* attacker, mob* victim, hitbox* attacker_h,
    hitbox* victim_h, float* knockback, float* angle
);
void cause_hitbox_damage(
    mob* attacker, mob* victim, hitbox* attacker_h,
    hitbox* victim_h, float* total_damage
);
void create_mob(mob* m);
void delete_mob(mob* m);
void focus_mob(mob* m1, mob* m2);
hitbox* get_closest_hitbox(
    const float x, const float y, mob* m, const size_t h_type = INVALID
);
hitbox* gui_hitbox(mob* m, const size_t nr);
bool is_resistant_to_hazards(
    vector<hazard*> &resistances, vector<hazard*> &hazards
);
void remove_from_group(mob* member);
bool should_attack(mob* m1, mob* m2);
void unfocus_mob(mob* m1);

#endif //ifndef MOB_INCLUDED
