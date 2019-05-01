/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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
#include "../mob_types/pikmin_type.h"
#include "../particle.h"
#include "../sector.h"
#include "../status.h"
#include "mob_utils.h"

using namespace std;

class mob_type;

extern size_t next_mob_id;

//Accelerate the Z speed of mobs affected by gravity by this amount per second.
const float GRAVITY_ADDER = -2600.0f;
const float MOB_KNOCKBACK_H_POWER = 64.0f;
const float MOB_KNOCKBACK_V_POWER = 800.0f;
const float MOB_PUSH_EXTRA_AMOUNT = 50.0f;
//When a leader throws a Pikmin, multiply the horizontal distance by 1/this.
const float THROW_DISTANCE_MULTIPLIER = 0.49f;
//When a leader throws a Pikmin, multiply the strength by this.
const float THROW_STRENGTH_MULTIPLIER = 0.457f;

enum DISABLED_STATE_FLAGS {
    //The Pikmin cannot be eaten by enemies.
    DISABLED_STATE_FLAG_INEDIBLE = 1,
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

enum MOB_TEAMS {
    //Can hurt everything, cannot be hurt.
    MOB_TEAM_TOP,
    //Can hurt/target anyone and be hurt/targeted by anyone, on any team.
    MOB_TEAM_NEUTRAL,
    MOB_TEAM_PLAYER_1,
    MOB_TEAM_PLAYER_2,
    MOB_TEAM_PLAYER_3,
    MOB_TEAM_PLAYER_4,
    MOB_TEAM_ENEMY_1,
    MOB_TEAM_ENEMY_2,
    MOB_TEAM_ENEMY_3,
    //Can only be hurt by Pikmin.
    MOB_TEAM_OBSTACLE,
    //Can be hurt by anything, but nothing WANTS to hurt it.
    MOB_TEAM_TOOL,
    //Can only be hurt, cannot hurt.
    MOB_TEAM_BOTTOM,
    //Cannot be hurt or targeted by anything.
    MOB_TEAM_PROP,
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
        const point &pos, mob_type* type, const float angle
    );
    virtual ~mob();
    
    //Basic information.
    //The type of mob -- Olimar, Red Bulborb, etc.
    mob_type* type;
    //Schedule this mob to be deleted from memory at the end of the frame.
    bool to_delete;
    
    //Script and animation.
    //Current animation instance.
    animation_instance anim;
    //Finite-state machine.
    mob_fsm fsm;
    //The script-controlled timer.
    timer script_timer;
    //Variables.
    map<string, string> vars;
    //The mob it has focus on.
    mob* focused_mob;
    //Mobs that it just hit. Used to stop hitboxes from hitting every frame.
    vector<pair<float, mob*> > hit_opponents;
    //How much damage did it take since the last time the itch event triggered?
    float itch_damage;
    //How much time has passed the last time the itch event triggered?
    float itch_time;
    //Index of the reach to use for "X in reach" events.
    size_t far_reach;
    //Index or the reach to use for "focused mob out of reach" events.
    size_t near_reach;
    //Mobs it is linked to.
    vector<mob*> links;
    
    //Movement and other physics.
    //Coordinates.
    point pos;
    //Z coordinate. This is height; the higher the value, the higher in the sky.
    float z;
    //Speed at which it's moving in X/Y...
    point speed;
    //...and Z.
    float speed_z;
    //Current facing angle. 0: Right. PI*0.5: Up. PI: Left. PI*1.5: Down.
    float angle;
    //Angle the mob wants to be facing.
    float intended_turn_angle;
    //Variable that holds the position the mob wants to be facing.
    point* intended_turn_pos;
    //Due to framerate imperfections, thrown Pikmin/leaders can reach higher
    //than intended. z_cap forces a cap. FLT_MAX = no cap.
    float z_cap;
    //Starting coordinates; what the mob calls "home".
    point home;
    //The highest ground below the entire mob.
    sector* ground_sector;
    //Sector that the mob's center is on.
    sector* center_sector;
    //Mob this mob is standing on top of, if any.
    mob* standing_on_mob;
    //Multiply the mob's gravity by this.
    float gravity_mult;
    //How much it's being pushed by another mob.
    float push_amount;
    //Angle that another mob is pushing it to.
    float push_angle;
    //Is it currently in a state where it cannot be pushed?
    bool unpushable;
    //Can it be touched by other mobs?
    bool tangible;
    //Is the mob airborne because it was thrown?
    bool was_thrown;
    
    //Target things.
    //If true, the mob is trying to go to a certain spot.
    bool chasing;
    //Chase after these coordinates, relative to the "origin" coordinates.
    point chase_offset;
    //Pointer to the origin of the coordinates, or NULL for the world origin.
    point* chase_orig_coords;
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
    //If true, the mob successfully reached its intended destination.
    bool reached_destination;
    //Information about the path it is following, if any.
    path_info_struct* path_info;
    //Information about the mob/point it's circling, if any.
    circling_info_struct* circling_info;
    
    //Group things.
    //The current mob is following this mob's group.
    mob* following_group;
    //The current subgroup type.
    subgroup_type* subgroup_type_ptr;
    //Info on the group this mob is a leader of.
    group_info* group;
    //Index of this mob's spot in the leader's group spots.
    size_t group_spot_index;
    
    //Carrying.
    //Structure holding information on how this mob should be carried.
    //If NULL, it cannot be carried.
    carry_info_struct* carry_info;
    
    //If it's being held by another mob, the information is kept here.
    hold_info_struct holder;
    //List of mobs it is holding.
    vector<mob*> holding;
    
    //Other properties.
    //Incremental ID. Used for minor things.
    size_t id;
    //Current health.
    float health;
    //During this period, the mob cannot be attacked.
    timer invuln_period;
    //Mob's team (who it can damage); use MOB_TEAM_*.
    unsigned char team;
    //If it should be hidden (no shadow, no health).
    bool hide;
    //Is invisible due to a status effect. Cache for performance.
    bool has_invisibility_status;
    //If not LARGE_FLOAT, compare the Z with this to shrink/grow the sprite.
    float height_effect_pivot;
    //Particle generators attached to it.
    vector<particle_generator> particle_generators;
    //Status effects currently inflicted on the mob.
    vector<status> statuses;
    //Hazard of the sector the mob is currently on.
    hazard* on_hazard;
    //Is it completely dead? Health = 0 isn't necessarily dead; could be dying.
    bool dead;
    //List of body parts that will chomp Pikmin.
    vector<int> chomp_body_parts;
    //List of mobs currently in its mouth, i.e., chomped.
    vector<mob*> chomping_mobs;
    //Max number of mobs it can chomp in the current attack.
    size_t chomp_max;
    //If the mob is currently "disabled", these flags specify behavior.
    unsigned char disabled_state_flags;
    //If this mob is a sub-mob, this points to the parent mob.
    parent_mob_info* parent;
    //How long it's been alive for.
    float time_alive;
    //Cached value of the angle's cosine.
    float angle_cos;
    //Cached value of the angle's sine.
    float angle_sin;
    
    
    void tick();
    void draw(bitmap_effect_manager* effect_manager = NULL);
    void draw_limb(bitmap_effect_manager* effect_manager = NULL);
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    
    void set_animation(
        const size_t nr,
        const bool pre_named = true, const bool auto_start = true
    );
    void set_health(const bool add, const bool ratio, const float amount);
    void set_timer(const float time);
    void set_var(const string &name, const string &value);
    
    void become_carriable(const size_t destination);
    void become_uncarriable();
    
    bool attack(mob* victim, hitbox* attack_h, hitbox* victim_h, float* damage);
    void add_to_group(mob* new_member);
    void apply_knockback(const float knockback, const float knockback_angle);
    bool calculate_carrying_destination(
        mob* added, mob* removed, mob** target_mob, point* target_point
    );
    void cause_spike_damage(mob* victim, const bool is_ingestion);
    void chomp(mob* m, hitbox* hitbox_info);
    void get_hitbox_hold_point(
        mob* mob_to_hold, hitbox* h_ptr, float* offset_dist, float* offset_angle
    );
    size_t get_latched_pikmin_amount();
    float get_latched_pikmin_weight();
    bool is_off_camera();
    void focus_on_mob(mob* m);
    void unfocus_from_mob();
    void leave_group();
    void hold(
        mob* m, const size_t hitbox_nr,
        const float offset_dist, const float offset_angle,
        const bool above_holder
    );
    void release(mob* m);
    bool can_damage(mob* m);
    bool wants_to_attack(mob* m);
    float get_hazard_vulnerability(hazard* h_ptr);
    bool is_resistant_to_hazards(vector<hazard*> &hazards);
    void swallow_chomped_pikmin(size_t nr);
    void start_height_effect();
    void stop_height_effect();
    void release_chomped_pikmin();
    void send_message(mob* receiver, string &msg);
    mob* spawn(mob_type::spawn_struct* info, mob_type* type_ptr = NULL);
    void start_dying();
    void finish_dying();
    void process_focused_mob();
    void respawn();
    hitbox* get_hitbox(const size_t nr);
    hitbox* get_closest_hitbox(
        const point &p, const size_t h_type = INVALID, dist* d = NULL
    );
    
    void chase(
        const point &offset, point* orig_coords,
        const bool teleport, float* teleport_z = NULL,
        const bool free_move = false, const float target_distance = 3,
        const float speed = -1
    );
    void stop_chasing();
    void stop_turning();
    bool follow_path(
        const point &target, const bool can_continue = true,
        const float speed = -1.0f, const float final_target_distance = 3
    );
    void stop_following_path();
    void circle_around(
        mob* m, const point &p, const float radius, const bool clockwise,
        const float speed, const bool can_free_move
    );
    void stop_circling();
    void face(const float new_angle, point* new_pos);
    point get_chase_target();
    virtual float get_base_speed();
    
    void arachnorb_head_turn_logic();
    void arachnorb_plan_logic(const unsigned char goal);
    void arachnorb_foot_move_logic();
    
    void apply_status_effect(
        status_type* s, const bool refill, const bool given_by_parent
    );
    void delete_old_status_effects();
    void remove_particle_generator(const size_t id);
    void add_status_bitmap_effects(bitmap_effect_manager* manager);
    ALLEGRO_BITMAP* get_status_bitmap(float* bmp_scale);
    virtual bool can_receive_status(status_type* s);
    virtual void handle_status_effect(status_type* s);
    //TODO Replace lose_panic_from_status() with handle_lose_status()?
    virtual void lose_panic_from_status();
    virtual void read_script_vars(const string &vars);
    virtual void start_dying_class_specific();
    
    //Drawing tools.
    point get_sprite_center(sprite* s);
    point get_sprite_dimensions(sprite* s, float* scale = NULL);
    void add_sector_brightness_bitmap_effect(bitmap_effect_manager* manager);
    void add_delivery_bitmap_effect(
        bitmap_effect_manager* manager, const float delivery_time_ratio_left,
        const ALLEGRO_COLOR &onion_color
    );
    
    string print_state_history();
    
};


/* See mob_type_with_anim_groups.
 */
class mob_with_anim_groups {
public:
    size_t cur_base_anim_nr;
    size_t get_animation_nr_from_base_and_group(
        const size_t base_anim_nr, const size_t group_nr,
        const size_t base_anim_total
    );
    
    mob_with_anim_groups();
};


#endif //ifndef MOB_INCLUDED
