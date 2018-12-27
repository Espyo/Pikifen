/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
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
#include "../mob_types/pikmin_type.h"
#include "../sector.h"
#include "../status.h"

using namespace std;

class mob_type;
class mob;

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
    //Can only be hurt, cannot hurt.
    MOB_TEAM_BOTTOM,
    //Cannot be hurt or targeted by anything.
    MOB_TEAM_PROP,
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
    //The Pikmin cannot be eaten by enemies.
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

enum CARRY_DESTINATIONS {
    CARRY_DESTINATION_SHIP,
    CARRY_DESTINATION_ONION,
    CARRY_DESTINATION_LINKED_MOB,
};


/* ----------------------------------------------------------------------------
 * Information about a spot in a group.
 */
struct group_spot {
    point pos; //Relative to the anchor.
    mob* mob_ptr;
    group_spot(const point &p = point(), mob* m = NULL) :
        pos(p), mob_ptr(m) {}
};


/* ----------------------------------------------------------------------------
 * Information on a mob's group.
 * This includes a list of its members,
 * and the location and info of the spots in the
 * circle, when the members are following the mob.
 */
struct group_info {
    vector<mob*> members;
    vector<group_spot> spots;
    float radius;
    point anchor; //Position of element 0 of the group (frontmost member).
    ALLEGRO_TRANSFORM transform;
    subgroup_type* cur_standby_type;
    bool follow_mode;
    
    void init_spots(mob* affected_mob_ptr = NULL);
    void sort(subgroup_type* leading_type);
    point get_average_member_pos();
    point get_spot_offset(const size_t spot_index);
    void reassign_spots();
    bool set_next_cur_standby_type(const bool move_backwards);
    group_info(mob* leader_ptr);
};


/* ----------------------------------------------------------------------------
 * Information on a carrying spot around a mob's perimeter.
 */
struct carrier_spot_struct {
    unsigned char state;
    //Relative coordinates of each spot.
    //They avoid calculating several sines and cosines over and over.
    point pos;
    mob* pik_ptr;
    carrier_spot_struct(const point &pos);
};


/* ----------------------------------------------------------------------------
 * Structure with information on how
 * the mob should be carried.
 */
struct carry_info_struct {
    //Mob that this struct belongs to.
    mob* m;
    //Generic type of delivery destination. Use CARRY_DESTINATION_*.
    size_t destination;
    
    vector<carrier_spot_struct> spot_info;
    
    //This is to avoid going through the vector
    //only to find out the total strength.
    float cur_carrying_strength;
    //Likewise, this is to avoid going through the vector
    //only to find out the number. Note that this is the number
    //of spaces reserved. A Pikmin could be on its way to its spot,
    //not necessarily there already.
    size_t cur_n_carriers;
    //Are the Pikmin stuck with nowhere to go?
    //0: no. 1: going to the alternative point, 2: going back to the start.
    unsigned char stuck_state;
    //Is the object moving at the moment?
    bool is_moving;
    //When the object begins moving, the idea is to carry it to this mob.
    mob* intended_mob;
    //When the object begins moving, the idea is to carry it to this point.
    point intended_point;
    
    carry_info_struct(mob* m, const size_t destination);
    bool is_empty();
    bool is_full();
    float get_speed();
    void rotate_points(const float angle);
    ~carry_info_struct();
};


/* ----------------------------------------------------------------------------
 * Structure with information on how to travel through the path graph that
 * the mob intends to travel.
 */
struct path_info_struct {
    //Mob that this struct belongs to.
    mob* m;
    //Final target, if it is a mob.
    mob* target_mob;
    //Target location.
    point target_point;
    //Path to take the mob to while being carried.
    vector<path_stop*> path;
    //Index of the current stop in the projected carrying path.
    size_t cur_path_stop_nr;
    //If there is no clear path, this points to all obstacles found.
    unordered_set<mob*> obstacle_ptrs;
    //If true, it's best to go straight to the target point
    //instead of taking a path.
    bool go_straight;
    
    path_info_struct(mob* m, const point &target);
};


/* ----------------------------------------------------------------------------
 * Structure with information about how this mob is currently being held by
 * another, if it is.
 */
struct hold_info_struct {
    //Points to the mob holding the current one, if any.
    mob* m;
    //ID of the hitbox the mob is attached to.
    //If INVALID, it's attached to the mob center.
    size_t hitbox_nr;
    //Ratio of distance from the hitbox/body center. 1 is the full radius.
    float offset_dist;
    //Angle the mob makes with the center of the hitbox/body.
    float offset_angle;
    //Is the mob drawn above the holder?
    bool above_holder;
    
    hold_info_struct();
    void clear();
    point get_final_pos(float* final_z);
};


/* ----------------------------------------------------------------------------
 * Structure with information about this mob's parent, if any.
 */
struct parent_mob_info {
    mob* m;
    bool handle_damage;
    bool relay_damage;
    bool handle_statuses;
    bool relay_statuses;
    bool handle_events;
    bool relay_events;
    
    //Limbs are visible connective textures between both mobs.
    animation_instance limb_anim;
    float limb_thickness;
    size_t limb_parent_body_part;
    float limb_parent_offset;
    size_t limb_child_body_part;
    float limb_child_offset;
    unsigned char limb_draw_method;
    
    parent_mob_info(mob* m);
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
    //Information about the path it is following, if any.
    path_info_struct* path_info;
    
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
    //Mobs it is chomping.
    vector<mob*> chomping_pikmin;
    //Max mobs it can chomp in the current attack.
    size_t chomp_max;
    //If the mob is currently "disabled", these flags specify behavior.
    unsigned char disabled_state_flags;
    //If this mob is a sub-mob, this points to the parent mob.
    parent_mob_info* parent;
    
    
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
    bool should_attack(mob* m);
    bool is_resistant_to_hazards(vector<hazard*> &hazards);
    void swallow_chomped_pikmin(size_t nr);
    void start_height_effect();
    void stop_height_effect();
    void release_chomped_pikmin();
    void send_message(mob* receiver, string &msg);
    mob* spawn(mob_type::spawn_struct* info);
    void start_dying();
    void finish_dying();
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
    void follow_path(const point &target, const bool can_continue = true);
    void stop_following_path();
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
    
    //Drawing tools.
    point get_sprite_center(sprite* s);
    point get_sprite_dimensions(sprite* s, float* scale = NULL);
    void add_sector_brightness_bitmap_effect(bitmap_effect_manager* manager);
    void add_delivery_bitmap_effect(
        bitmap_effect_manager* manager, const float delivery_time_ratio_left,
        const ALLEGRO_COLOR &onion_color
    );
    
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


void calculate_knockback(
    mob* attacker, mob* victim, hitbox* attacker_h,
    hitbox* victim_h, float* knockback, float* angle
);
void cause_hitbox_damage(
    mob* attacker, mob* victim, hitbox* attacker_h,
    hitbox* victim_h, float* total_damage
);
mob* create_mob(
    mob_category* category, const point &pos, mob_type* type,
    const float angle, const string &vars,
    function<void(mob*)> code_after_creation = nullptr
);
void delete_mob(mob* m, const bool complete_destruction = false);
size_t string_to_team_nr(const string &team_str);

#endif //ifndef MOB_INCLUDED
