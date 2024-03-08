/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob utility classes and functions.
 */

#ifndef MOB_UTILS_INCLUDED
#define MOB_UTILS_INCLUDED

#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>

#include "../animation.h"
#include "../area/sector.h"
#include "../misc_structs.h"
#include "../mob_types/bouncer_type.h"
#include "../mob_types/bridge_type.h"
#include "../mob_types/converter_type.h"
#include "../mob_types/decoration_type.h"
#include "../mob_types/drop_type.h"
#include "../mob_types/enemy_type.h"
#include "../mob_types/group_task_type.h"
#include "../mob_types/interactable_type.h"
#include "../mob_types/leader_type.h"
#include "../mob_types/pellet_type.h"
#include "../mob_types/pikmin_type.h"
#include "../mob_types/pile_type.h"
#include "../mob_types/resource_type.h"
#include "../mob_types/scale_type.h"
#include "../mob_types/tool_type.h"
#include "../mob_types/track_type.h"
#include "../mob_types/treasure_type.h"
#include "../pathing.h"
#include "../utils/geometry_utils.h"
#include "mob_enums.h"


using std::size_t;
using std::vector;

class mob;

/**
 * @brief Info on a carrying spot around a mob's perimeter.
 */
struct carrier_spot_struct {

    //--- Members ---

    //State.
    CARRY_SPOT_STATES state = CARRY_SPOT_FREE;

    //Relative coordinates of each spot. Cache for performance.
    point pos;

    //Pikmin that is in this spot.
    mob* pik_ptr = nullptr;
    

    //--- Function declarations ---

    explicit carrier_spot_struct(const point &pos);

};


/**
 * @brief Info on how the mob should be carried.
 */
struct carry_info_struct {

    //--- Members ---

    //Mob that this struct belongs to.
    mob* m = nullptr;

    //Generic type of delivery destination.
    CARRY_DESTINATIONS destination = CARRY_DESTINATION_SHIP;

    //Information about each carrier spot.
    vector<carrier_spot_struct> spot_info;

    //Current carrying strength. Cache for performance.
    float cur_carrying_strength = 0.0f;

    //Number of carriers, including reserves. Cache for performance.
    size_t cur_n_carriers = 0;

    //Is the object moving at the moment?
    bool is_moving = false;

    //When the object begins moving, the idea is to carry it to this mob.
    mob* intended_mob = nullptr;

    //When the object begins moving, the idea is to carry it to this point.
    point intended_point;

    //When delivering to an Onion, this is the Pikmin type that will benefit.
    pikmin_type* intended_pik_type = nullptr;

    //True if a destination does exist, false otherwise.
    bool destination_exists = false;

    //Is the Pikmin meant to return somewhere after carrying?
    bool must_return = false;

    //Location to return to once they finish carrying.
    point return_point;

    //Distance from the return point to stop at.
    float return_dist = 0.0f;
    

    //--- Function declarations ---

    carry_info_struct(mob* m, const CARRY_DESTINATIONS destination);
    bool is_empty() const;
    bool is_full() const;
    vector<hazard*> get_carrier_invulnerabilities() const;
    bool can_fly() const;
    float get_speed() const;
    void rotate_points(const float angle);

};


/**
 * @brief Info on what point the mob is chasing after.
 */
struct chase_info_struct {

    //--- Members ---

    //Current chasing state.
    CHASE_STATES state = CHASE_STATE_STOPPED;

    //Flags that control how to chase. Use CHASE_FLAG_*.
    unsigned char flags = 0;
    
    //Chase after these coordinates, relative to the "origin" coordinates.
    point offset;

    //Same as above, but for the Z coordinate.
    float offset_z = 0.0f;

    //Pointer to the origin of the coordinates, or NULL for the world origin.
    point* orig_coords = nullptr;

    //Same as above, but for the Z coordinate.
    float* orig_z = nullptr;
    
    //Distance from the target in which the mob is considered as being there.
    float target_dist = 0.0f;

    //Acceleration to apply, in units per second per second.
    float acceleration = 0.0f;

    //Current speed to move towards the target at.
    float cur_speed = 0.0f;

    //Maximum speed.
    float max_speed = -1.0f;

};


/**
 * @brief Info about what mob or point that this mob is circling around,
 * if any.
 */
struct circling_info_struct {

    //--- Members ---

    //Mob that this struct belongs to.
    mob* m = nullptr;

    //Mob that it is circling.
    mob* circling_mob = nullptr;

    //Point that it is circling, if it's not circling a mob.
    point circling_point;

    //Radius at which to circle around.
    float radius = 0.0f;

    //Is it circling clockwise?
    bool clockwise = true;

    //Speed at which to move.
    float speed = 0.0f;

    //Can the mob move freely, or only forward?
    bool can_free_move = false;

    //Angle of the circle to go to.
    float cur_angle = 0.0f;
    

    //--- Function declarations ---

    explicit circling_info_struct(mob* m);

};


/**
 * @brief Info on a mob that's being delivered to an Onion, ship, etc.
 */
struct delivery_info_struct {

    //--- Members ---

    //Animation type.
    DELIVERY_ANIMATIONS anim_type = DELIVERY_ANIM_SUCK;

    //Ratio of time left in the animation.
    float anim_time_ratio_left = 1.0f;

    //Color to make the mob glow with.
    ALLEGRO_COLOR color;

    //Intended delivery Pikmin type, in the case of Onions.
    pikmin_type* intended_pik_type = nullptr;
    

    //--- Function declarations ---

    delivery_info_struct();

};


/**
 * @brief Info on a mob's group.
 *
 * This includes a list of its members,
 * and the location and info of the spots in the
 * circle, when the members are following the mob.
 */
struct group_info_struct {

    //--- Misc. declarations ---

    //Ways for Pikmin to follow the leader.
    enum MODES {

        //Follow the leader's back.
        MODE_FOLLOW_BACK,

        //Casually shuffle with the leader, if needed.
        MODE_SHUFFLE,
        
        //Swarming.
        MODE_SWARM,
        
    };
    
    /**
     * @brief A spot in the group.
     */
    struct group_spot {

        //--- Members ---

        //Position relative to the anchor.
        point pos;

        //Mob in this spot.
        mob* mob_ptr = nullptr;
        

        //--- Function declarations ---

        explicit group_spot(const point &p = point(), mob* m = NULL) :
            pos(p), mob_ptr(m) {}

    };
    

    //--- Members ---

    //All group members.
    vector<mob*> members;

    //Information about each spot.
    vector<group_spot> spots;

    //Radius of the group.
    float radius = 0.0f;

    //Absolute position of element 0 of the group (frontmost member).
    point anchor;

    //Angle from the leader to the anchor.
    float anchor_angle = TAU / 2.0f;

    //Transformation to apply to the group, like from swarming.
    ALLEGRO_TRANSFORM transform;

    //Currently selected standby type.
    subgroup_type* cur_standby_type = nullptr;

    //Mode of operation.
    MODES mode = MODE_SHUFFLE;
    

    //--- Function declarations ---

    explicit group_info_struct(mob* leader_ptr);
    void init_spots(mob* affected_mob_ptr = NULL);
    void sort(subgroup_type* leading_type);
    void change_standby_type_if_needed();
    size_t get_amount_by_type(const mob_type* type) const;
    point get_average_member_pos() const;
    vector<hazard*> get_group_invulnerabilities(
        mob* include_leader = NULL
    ) const;
    bool get_next_standby_type(
        const bool move_backwards, subgroup_type** new_type
    );
    point get_spot_offset(const size_t spot_index) const;
    void reassign_spots();
    bool change_standby_type(const bool move_backwards);
};


/**
 * @brief Info about how this mob is currently being held by
 * another, if it is.
 */
struct hold_info_struct {

    //--- Members ---

    //Points to the mob holding the current one, if any.
    mob* m = nullptr;

    //ID of the hitbox the mob is attached to.
    //If INVALID, it's attached to the mob center.
    size_t hitbox_nr = INVALID;

    //Ratio of distance from the hitbox/body center. 1 is the full radius.
    float offset_dist = 0.0f;

    //Angle the mob makes with the center of the hitbox/body.
    float offset_angle = 0.0f;

    //Ratio of distance from the hitbox/body's bottom. 1 is the very top.
    float vertical_dist = 0.0f;

    //Is the mob drawn above the holder?
    bool above_holder = false;

    //How should the held object rotate?
    HOLD_ROTATION_METHODS rotation_method = HOLD_ROTATION_METHOD_NEVER;
    

    //--- Function declarations ---

    void clear();
    point get_final_pos(float* final_z) const;

};


class bouncer;
class bridge;
class converter;
class decoration;
class drop;
class enemy;
class group_task;
class interactable;
class leader;
class onion;
class pellet;
class pikmin;
class pile;
class resource;
class scale;
class ship;
class tool;
class track;
class treasure;

class onion_type;
class ship_type;

/**
 * @brief Lists of all mobs in the area.
 */
struct mob_lists {

    //--- Members ---

    //All mobs in the area.
    vector<mob*> all;

    //Bouncers.
    vector<bouncer*> bouncers;

    //Bridges.
    vector<bridge*> bridges;

    //Converters.
    vector<converter*> converters;

    //Decorations.
    vector<decoration*> decorations;

    //Drops.
    vector<drop*> drops;

    //Enemies.
    vector<enemy*> enemies;

    //Group tasks.
    vector<group_task*> group_tasks;

    //Interactables.
    vector<interactable*> interactables;

    //Leaders.
    vector<leader*> leaders;

    //Onions.
    vector<onion*> onions;

    //Pellets.
    vector<pellet*> pellets;

    //Pikmin.
    vector<pikmin*> pikmin_list;

    //Piles.
    vector<pile*> piles;

    //Resources.
    vector<resource*> resources;

    //Scales.
    vector<scale*> scales;

    //Ships.
    vector<ship*> ships;

    //Tools.
    vector<tool*> tools;

    //Tracks.
    vector<track*> tracks;

    //Treasures.
    vector<treasure*> treasures;

};


/**
 * @brief Lists of all mob types.
 */
struct mob_type_lists {
    
    //--- Members ---

    //Bouncer types.
    map<string, bouncer_type*> bouncer;

    //Bridge types.
    map<string, bridge_type*> bridge;

    //Converter types.
    map<string, converter_type*> converter;

    //Custom mob types.
    map<string, mob_type*> custom;

    //Decoration types.
    map<string, decoration_type*> decoration;

    //Drop types.
    map<string, drop_type*> drop;

    //Enemy types.
    map<string, enemy_type*> enemy;

    //Group task types.
    map<string, group_task_type*> group_task;

    //Interactable types.
    map<string, interactable_type*> interactable;

    //Leader types.
    map<string, leader_type*> leader;

    //Onion types.
    map<string, onion_type*> onion;

    //Pellet types.
    map<string, pellet_type*> pellet;

    //Pikmin types.
    map<string, pikmin_type*> pikmin;

    //Pile types.
    map<string, pile_type*> pile;

    //Resource types.
    map<string, resource_type*> resource;

    //Scale types.
    map<string, scale_type*> scale;

    //Ship types.
    map<string, ship_type*> ship;

    //Tool types.
    map<string, tool_type*> tool;

    //Track types.
    map<string, track_type*> track;

    //Treasure types.
    map<string, treasure_type*> treasure;

};


/**
 * @brief Info about this mob's parent, if any.
 */
struct parent_info_struct {

    //--- Members ---

    //Mob serving as the parent.
    mob* m = nullptr;

    //Should the child handle damage?
    bool handle_damage = false;

    //Should the child relay damage to the parent?
    bool relay_damage = false;

    //Should the child handle status effects?
    bool handle_statuses = false;

    //Should the child relay status effects to the parent?
    bool relay_statuses = false;

    //Should the child handle script events?
    bool handle_events = false;

    //Should the child relay script events to the parent?
    bool relay_events = false;

    //Animation used for the limb connecting child and parent.
    animation_instance limb_anim;

    //Thickness of the limb.
    float limb_thickness = 32.0f;

    //Body part of the parent to link the limb to.
    size_t limb_parent_body_part = INVALID;

    //Offset from the parent body part to link the limb at.
    float limb_parent_offset = 0.0f;

    //Body part of the child to link the limb to.
    size_t limb_child_body_part = INVALID;

    //Offset from the child body part to link the limb at.
    float limb_child_offset = 0.0f;

    //Method by which the limb should be drawn.
    LIMB_DRAW_METHODS limb_draw_method = LIMB_DRAW_ABOVE_CHILD;
    

    //--- Function declarations ---

    explicit parent_info_struct(mob* m);

};


/**
 * @brief Info on how to travel through the path graph that
 * the mob currently intends to travel.
 */
struct path_info_struct {

    //--- Members ---

    //Mob that this struct belongs to.
    mob* m = nullptr;

    //Path to take the mob to while being carried.
    vector<path_stop*> path;

    //Index of the current stop in the projected carrying path.
    size_t cur_path_stop_nr = 0;

    //Result of the path calculation.
    PATH_RESULTS result = PATH_RESULT_NOT_CALCULATED;

    //Is the way forward currently blocked? If so, why?
    PATH_BLOCK_REASONS block_reason = PATH_BLOCK_REASON_NONE;

    //Settings about how the path should be followed.
    path_follow_settings settings;
    

    //--- Function declarations ---

    path_info_struct(
        mob* m,
        const path_follow_settings &settings
    );
    bool check_blockage(PATH_BLOCK_REASONS* reason = NULL);

};


/**
 * @brief Info that a mob type may have about how to nest Pikmin inside,
 * like an Onion or a ship.
 */
struct pikmin_nest_type_struct {

    //--- Members ---

    //Pikmin types it can manage.
    vector<pikmin_type*> pik_types;

    //Body parts that represent legs -- pairs of hole + foot.
    vector<string> leg_body_parts;

    //Speed at which Pikmin enter the nest.
    float pikmin_enter_speed = 0.7f;

    //Speed at which Pikmin exit the nest.
    float pikmin_exit_speed = 2.0f;
    

    //--- Function declarations ---

    void load_properties(data_node* file);

};


/**
 * @brief Info that a mob may have about how to nest Pikmin inside,
 * like an Onion or a ship.
 */
struct pikmin_nest_struct {

public:

    //--- Members ---

    //Pointer to the nest mob responsible.
    mob* m_ptr = nullptr;

    //Pointer to the type of nest.
    pikmin_nest_type_struct* nest_type = nullptr;

    //How many Pikmin are inside, per type, per maturity.
    vector<vector<size_t> > pikmin_inside;

    //How many Pikmin are queued up to be called out, of each type.
    vector<size_t> call_queue;

    //Which leader is calling the Pikmin over?
    leader* calling_leader = nullptr;

    //Time left until it can eject the next Pikmin in the call queue.
    float next_call_time = 0.0f;
    

    //--- Function declarations ---

    pikmin_nest_struct(mob* m_ptr, pikmin_nest_type_struct* type);
    bool call_pikmin(mob* m_ptr, const size_t type_idx);
    size_t get_amount_by_type(const pikmin_type* type);
    void read_script_vars(const script_var_reader &svr);
    void request_pikmin(
        const size_t type_idx, const size_t amount, leader* l_ptr
    );
    void store_pikmin(pikmin* p_ptr);
    void tick(const float delta_t);

};


/**
 * @brief Info about the track mob that a mob is currently
 * riding. Includes things like current progress.
 */
struct track_info_struct {
    
    //--- Members ---

    //Pointer to the track mob.
    mob* m = nullptr;

    //List of checkpoints (body part indexes) to cross.
    vector<size_t> checkpoints;

    //Current checkpoint of the track. This is the last checkpoint crossed.
    size_t cur_cp_nr = 0;

    //Progress within the current checkpoint. 0 means at the checkpoint.
    //1 means it's at the next checkpoint.
    float cur_cp_progress = 0.0f;

    //Speed to ride at, in ratio per second.
    float ride_speed = 0.0f;
    

    //--- Function declarations ---
    
    track_info_struct(
        mob* m, const vector<size_t> &checkpoints, const float speed
    );
    
};


float calculate_mob_max_span(
    const float radius, const float anim_max_span, const point &rectangular_dim
);
mob* create_mob(
    mob_category* category, const point &pos, mob_type* type,
    const float angle, const string &vars,
    std::function<void(mob*)> code_after_creation = nullptr,
    const size_t first_state_override = INVALID
);
void delete_mob(mob* m, const bool complete_destruction = false);
string get_error_message_mob_info(mob* m);
vector<hazard*> get_mob_type_list_invulnerabilities(
    const unordered_set<mob_type*> &types
);
mob_type::spawn_struct* get_spawn_info_from_child_info(
    mob_type* type, const mob_type::child_struct* child_info
);
MOB_TARGET_TYPES string_to_mob_target_type(const string &type_str);
MOB_TEAMS string_to_team_nr(const string &team_str);


#endif //ifndef MOB_UTILS_INCLUDED
