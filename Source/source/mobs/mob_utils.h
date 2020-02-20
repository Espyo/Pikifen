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
#include "../sector.h"
#include "../utils/geometry_utils.h"

using namespace std;

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
    //When stuck, look out for these obstacles. They may fix the situation.
    unordered_set<mob*> obstacle_ptrs;
    //Are the Pikmin stuck with nowhere to go?
    bool is_stuck;
    //Is the object moving at the moment?
    bool is_moving;
    //When the object begins moving, the idea is to carry it to this mob.
    mob* intended_mob;
    //When the object begins moving, the idea is to carry it to this point.
    point intended_point;
    //Is the Pikmin meant to return somewhere after carrying?
    bool must_return;
    //Location to return true.
    point return_point;
    //Distance from the return point to stop at.
    float return_dist;
    
    carry_info_struct(mob* m, const size_t destination);
    bool is_empty();
    bool is_full();
    float get_speed();
    void rotate_points(const float angle);
    ~carry_info_struct();
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
    point get_average_member_pos();
    point get_spot_offset(const size_t spot_index);
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
    point get_final_pos(float* final_z);
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
    //List of all obstacles located somewhere in the path.
    unordered_set<mob*> obstacle_ptrs;
    //If true, it's best to go straight to the target point
    //instead of taking a path.
    bool go_straight;
    //For the chase from the final path stop to the target, use this
    //value in the target_distance parameter.
    float final_target_distance;
    
    path_info_struct(mob* m, const point &target);
};


/* ----------------------------------------------------------------------------
 * Structure with information about the track mob that a mob is currently
 * riding. Includes things like current progress.
 */
struct track_info_struct {
    //Pointer to the track mob.
    mob* m;
    //Current checkpoint of the track. This is the last checkpoint crossed.
    size_t cur_cp_nr;
    //Progress within the current checkpoint. 0 means at the checkpoint.
    //1 means it's at the next checkpoint.
    float cur_cp_progress;
    
    track_info_struct(mob* m);
};


mob* create_mob(
    mob_category* category, const point &pos, mob_type* type,
    const float angle, const string &vars,
    function<void(mob*)> code_after_creation = nullptr
);
void delete_mob(mob* m, const bool complete_destruction = false);
string get_error_message_mob_info(mob* m);
size_t string_to_mob_target_type(const string &type_str);
size_t string_to_team_nr(const string &team_str);


#endif //ifndef MOB_UTILS_INCLUDED
