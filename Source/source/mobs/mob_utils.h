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
#include "../mob_types/onion_type.h"
#include "../mob_types/pellet_type.h"
#include "../mob_types/pikmin_type.h"
#include "../mob_types/pile_type.h"
#include "../mob_types/resource_type.h"
#include "../mob_types/scale_type.h"
#include "../mob_types/ship_type.h"
#include "../mob_types/tool_type.h"
#include "../mob_types/track_type.h"
#include "../mob_types/treasure_type.h"
#include "../sector.h"
#include "../utils/geometry_utils.h"


using std::size_t;
using std::vector;

class mob;

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
    //Is the object moving at the moment?
    bool is_moving;
    //When the object begins moving, the idea is to carry it to this mob.
    mob* intended_mob;
    //When the object begins moving, the idea is to carry it to this point.
    point intended_point;
    //When delivering to an Onion, this is the Pikmin type that will benefit.
    pikmin_type* intended_pik_type;
    //Is the Pikmin meant to return somewhere after carrying?
    bool must_return;
    //Location to return true.
    point return_point;
    //Distance from the return point to stop at.
    float return_dist;
    
    carry_info_struct(mob* m, const size_t destination);
    bool is_empty() const;
    bool is_full() const;
    float get_speed() const;
    void rotate_points(const float angle);
};


/* ----------------------------------------------------------------------------
 * Structure with information on what point the mob is chasing after.
 */
struct chase_info_struct {
    //If true, the mob is trying to go to a certain spot.
    bool is_chasing;
    //Chase after these coordinates, relative to the "origin" coordinates.
    point offset;
    //Pointer to the origin of the coordinates, or NULL for the world origin.
    point* orig_coords;
    //When chasing something in teleport mode, also change the z accordingly.
    float* teleport_z;
    //If true, teleport instantly.
    bool teleport;
    //If true, the mob can move in a direction it's not facing.
    bool free_move;
    //Distance from the target in which the mob is considered as being there.
    float target_dist;
    //Speed to move towards the target at.
    float speed;
    //If true, the mob successfully reached its intended destination.
    bool reached_destination;
    
    chase_info_struct();
};


/* ----------------------------------------------------------------------------
 * Structure with information about what mob or point that this
 * mob is circling around, if any.
 */
struct circling_info_struct {
    //Mob that this struct belongs to.
    mob* m;
    //Mob that it is circling.
    mob* circling_mob;
    //Point that it is circling, if it's not circling a mob.
    point circling_point;
    //Radius at which to circle around.
    float radius;
    //Is it circling clockwise?
    bool clockwise;
    //Speed at which to move.
    float speed;
    //Can the mob move freely, or only forward?
    bool can_free_move;
    //Angle of the circle to go to.
    float cur_angle;
    
    circling_info_struct(mob* m);
};


/* ----------------------------------------------------------------------------
 * Information on a mob that's being delivered to an Onion, ship, etc.
 */
struct delivery_info_struct {
    ALLEGRO_COLOR color;
    pikmin_type* intended_pik_type;
    delivery_info_struct();
};


/* ----------------------------------------------------------------------------
 * Information on a mob's group.
 * This includes a list of its members,
 * and the location and info of the spots in the
 * circle, when the members are following the mob.
 */
struct group_info_struct {

    struct group_spot {
        point pos; //Relative to the anchor.
        mob* mob_ptr;
        group_spot(const point &p = point(), mob* m = NULL) :
            pos(p), mob_ptr(m) {}
    };
    
    vector<mob*> members;
    vector<group_spot> spots;
    float radius;
    point anchor; //Position of element 0 of the group (frontmost member).
    ALLEGRO_TRANSFORM transform;
    subgroup_type* cur_standby_type;
    bool follow_mode;
    
    void init_spots(mob* affected_mob_ptr = NULL);
    void sort(subgroup_type* leading_type);
    void change_standby_type_if_needed();
    size_t get_amount_by_type(mob_type* type) const;
    point get_average_member_pos() const;
    point get_spot_offset(const size_t spot_index) const;
    void reassign_spots();
    bool set_next_cur_standby_type(const bool move_backwards);
    group_info_struct(mob* leader_ptr);
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
    //How should the held object rotate? Use HOLD_ROTATION_METHOD_*.
    unsigned char rotation_method;
    
    hold_info_struct();
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

/* ----------------------------------------------------------------------------
 * Lists of all mobs in the area.
 */
struct mob_lists {
    vector<mob*> all;
    vector<bouncer*> bouncers;
    vector<bridge*> bridges;
    vector<converter*> converters;
    vector<decoration*> decorations;
    vector<drop*> drops;
    vector<enemy*> enemies;
    vector<group_task*> group_tasks;
    vector<interactable*> interactables;
    vector<leader*> leaders;
    vector<onion*> onions;
    vector<pellet*> pellets;
    vector<pikmin*> pikmin_list;
    vector<pile*> piles;
    vector<resource*> resources;
    vector<scale*> scales;
    vector<ship*> ships;
    vector<tool*> tools;
    vector<track*> tracks;
    vector<treasure*> treasures;
};


/* ----------------------------------------------------------------------------
 * Lists of all mob types.
 */
struct mob_type_lists {
    map<string, bouncer_type*> bouncer;
    map<string, bridge_type*> bridge;
    map<string, converter_type*> converter;
    map<string, mob_type*> custom;
    map<string, decoration_type*> decoration;
    map<string, drop_type*> drop;
    map<string, enemy_type*> enemy;
    map<string, group_task_type*> group_task;
    map<string, interactable_type*> interactable;
    map<string, leader_type*> leader;
    map<string, onion_type*> onion;
    map<string, pellet_type*> pellet;
    map<string, pikmin_type*> pikmin;
    map<string, pile_type*> pile;
    map<string, resource_type*> resource;
    map<string, scale_type*> scale;
    map<string, ship_type*> ship;
    map<string, tool_type*> tool;
    map<string, track_type*> track;
    map<string, treasure_type*> treasure;
};


/* ----------------------------------------------------------------------------
 * Structure with information about this mob's parent, if any.
 */
struct parent_info_struct {
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
    
    parent_info_struct(mob* m);
};


/* ----------------------------------------------------------------------------
 * Structure with information on how to travel through the path graph that
 * the mob intends to travel.
 */
struct path_info_struct {
    //Mob that this struct belongs to.
    mob* m;
    //Target location.
    point target_point;
    //Path to take the mob to while being carried.
    vector<path_stop*> path;
    //Index of the current stop in the projected carrying path.
    size_t cur_path_stop_nr;
    //If true, it's best to go straight to the target point
    //instead of taking a path.
    bool go_straight;
    //For the chase from the final path stop to the target, use this
    //value in the target_distance parameter.
    float final_target_distance;
    //Is the way forward currently blocked?
    bool is_blocked;
    
    path_info_struct(mob* m, const point &target);
    bool check_blockage();
};


/* ----------------------------------------------------------------------------
 * Structure with information about the track mob that a mob is currently
 * riding. Includes things like current progress.
 */
struct track_info_struct {
    //Pointer to the track mob.
    mob* m;
    //List of checkpoints (body part indexes) to cross.
    vector<size_t> checkpoints;
    //Current checkpoint of the track. This is the last checkpoint crossed.
    size_t cur_cp_nr;
    //Progress within the current checkpoint. 0 means at the checkpoint.
    //1 means it's at the next checkpoint.
    float cur_cp_progress;
    //Speed to ride at, in ratio per second.
    float ride_speed;
    
    track_info_struct(
        mob* m, const vector<size_t> checkpoints, const float speed
    );
};


mob* create_mob(
    mob_category* category, const point &pos, mob_type* type,
    const float angle, const string &vars,
    std::function<void(mob*)> code_after_creation = nullptr
);
void delete_mob(mob* m, const bool complete_destruction = false);
string get_error_message_mob_info(mob* m);
size_t string_to_mob_target_type(const string &type_str);
size_t string_to_team_nr(const string &team_str);


#endif //ifndef MOB_UTILS_INCLUDED
