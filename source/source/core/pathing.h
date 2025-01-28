/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the paths, path-finding, and related functions.
 */

#pragma once

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "../content/other/hazard.h"
#include "../util/general_utils.h"
#include "../util/geometry_utils.h"

using std::map;
using std::string;
using std::unordered_set;
using std::vector;


class mob;
struct sector;
struct path_link;


//Types of path link.
enum PATH_LINK_TYPE {

    //Normal.
    PATH_LINK_TYPE_NORMAL,
    
    //One-way fall for normal mobs, two-way for airborne mobs.
    PATH_LINK_TYPE_LEDGE,
    
};


//Flags for path stops.
enum PATH_STOP_FLAG {

    //Only usable by mob scripts that reference it.
    PATH_STOP_FLAG_SCRIPT_ONLY = 1 << 0,
    
    //Only for mobs carrying nothing, or a 1-weight mob.
    PATH_STOP_FLAG_LIGHT_LOAD_ONLY = 1 << 1,
    
    //Only for mobs that can fly.
    PATH_STOP_FLAG_AIRBORNE_ONLY = 1 << 2,
    
};


//Flags that control how paths should be followed.
enum PATH_FOLLOW_FLAG {

    //It's possible to continue from the last path if it wants.
    PATH_FOLLOW_FLAG_CAN_CONTINUE = 1 << 0,
    
    //Ignore any obstacles in the path links.
    PATH_FOLLOW_FLAG_IGNORE_OBSTACLES = 1 << 1,
    
    //At the end, constantly chase after the target mob (if any)'s position.
    PATH_FOLLOW_FLAG_FOLLOW_MOB = 1 << 2,
    
    //Use the faked start point instead of the normal one.
    PATH_FOLLOW_FLAG_FAKED_START = 1 << 3,
    
    //Use the faked end point instead of the normal one.
    PATH_FOLLOW_FLAG_FAKED_END = 1 << 4,
    
    //The mob was told to use this path by a script.
    PATH_FOLLOW_FLAG_SCRIPT_USE = 1 << 5,
    
    //The mob has light load.
    PATH_FOLLOW_FLAG_LIGHT_LOAD = 1 << 6,
    
    //The mob can fly.
    PATH_FOLLOW_FLAG_AIRBORNE = 1 << 7,
    
};


//Possible results for when a path is decided.
//Positive values mean the mob can go, negative values means it can't.
enum PATH_RESULT {

    //An open path exists, and is to be followed normally.
    PATH_RESULT_NORMAL_PATH = 1,
    
    //A path exists, but is blocked by obstacles.
    PATH_RESULT_PATH_WITH_OBSTACLES = 2,
    
    //The shortest path passes through one stop only.
    PATH_RESULT_PATH_WITH_SINGLE_STOP = 3,
    
    //The shortest path is to go directly to the end point.
    PATH_RESULT_DIRECT = 4,
    
    //Area has no stops, so go directly to the end point.
    PATH_RESULT_DIRECT_NO_STOPS = 5,
    
    //The end stop cannot be reached from the start stop. No path.
    PATH_RESULT_END_STOP_UNREACHABLE = -1,
    
    //There is nowhere to go because the destination was never set.
    PATH_RESULT_NO_DESTINATION = -2,
    
    //Something went wrong. No path.
    PATH_RESULT_ERROR = -3,
    
    //A path has not been calculated yet.
    PATH_RESULT_NOT_CALCULATED = -4,
    
};


//Possible reasons for the path ahead to be blocked.
enum PATH_BLOCK_REASON {

    //Not blocked.
    PATH_BLOCK_REASON_NONE,
    
    //There's simply no valid path.
    PATH_BLOCK_REASON_NO_PATH,
    
    //There's an obstacle object in the way.
    PATH_BLOCK_REASON_OBSTACLE,
    
    //The link requires the path to be from a script, but it isn't.
    PATH_BLOCK_REASON_NOT_IN_SCRIPT,
    
    //The link requires a light load, but the object isn't travelling light.
    PATH_BLOCK_REASON_NOT_LIGHT_LOAD,
    
    //The link requires an airborne mob, but the object isn't.
    PATH_BLOCK_REASON_NOT_AIRBORNE,
    
    //The link is through a ledge the mob can't climb up.
    PATH_BLOCK_REASON_UP_LEDGE,
    
    //The link has a label that the object doesn't want.
    PATH_BLOCK_REASON_NOT_RIGHT_LABEL,
    
    //The next path stop is in the void.
    PATH_BLOCK_REASON_STOP_IN_VOID,
    
    //The next path stop is in a sector with hazards the mob is vulnerable to.
    PATH_BLOCK_REASON_HAZARDOUS_STOP,
    
};


namespace PATHS {
extern const float DEF_CHASE_TARGET_DISTANCE;
extern const float MIN_STOP_RADIUS;
}


/**
 * @brief Settings about how a mob should follow a path.
 */
struct path_follow_settings {

    //--- Members ---
    
    //Target location.
    point target_point;
    
    //If the target is a mob, this points to it.
    mob* target_mob = nullptr;
    
    //For the final chase, from the last path stop to
    //the destination, use this for the target distance parameter.
    float final_target_distance = PATHS::DEF_CHASE_TARGET_DISTANCE;
    
    //Some flags. Use PATH_FOLLOW_FLAG_*.
    bitmask_8_t flags = 0;
    
    //Invulnerabilities of the mob/carriers.
    vector<hazard*> invulnerabilities;
    
    //If not empty, only follow path links with this label.
    string label;
    
    //Faked start point. Used to fake calculations.
    point faked_start;
    
    //Faked end point. Used to fake calculations.
    point faked_end;
    
};


/**
 * @brief Stops are points that make up a path.
 *
 * In mathematics, this is a node
 * in the graph. In a real-world example, this is a bus stop.
 * Pikmin start carrying by going for the closest stop.
 * Then they move stop by stop, following the connections, until they
 * reach the final stop and go wherever they need.
 */
struct path_stop {

    //--- Members ---
    
    //Coordinates.
    point pos;
    
    //Radius.
    float radius = PATHS::MIN_STOP_RADIUS;
    
    //Flags. Use PATH_STOP_FLAG.
    bitmask_8_t flags = 0;
    
    //Its label, if any.
    string label;
    
    //Links that go to other stops.
    vector<path_link*> links;
    
    //Sector it's on. Only applicable during gameplay. Cache for performance.
    sector* sector_ptr = nullptr;
    
    
    //--- Function declarations ---
    
    explicit path_stop(
        const point &pos = point(),
        const vector<path_link*> &links = vector<path_link*>()
    );
    ~path_stop();
    void clone(path_stop* destination) const;
    void add_link(path_stop* other_stop, bool normal);
    path_link* get_link(const path_stop* other_stop) const;
    void remove_link(const path_link* link_ptr);
    void remove_link(const path_stop* other_stop);
    void calculate_dists();
    void calculate_dists_plus_neighbors();
    
};


/**
 * @brief Info about a path link. A path stop can link to N other
 * path stops, and this structure holds information about a connection.
 */
struct path_link {

    //--- Members ---
    
    //Pointer to the path stop at the start.
    path_stop* start_ptr = nullptr;
    
    //Pointer to the path stop at the end.
    path_stop* end_ptr = nullptr;
    
    //Index number of the path stop at the end.
    size_t end_idx = 0;
    
    //Type. Used for special restrictions and behaviors.
    PATH_LINK_TYPE type = PATH_LINK_TYPE_NORMAL;
    
    //Distance between the two stops.
    float distance = 0.0f;
    
    //Is the stop currently blocked by an obstacle? Cache for performance.
    bool blocked_by_obstacle = false;
    
    
    //--- Function declarations ---
    
    path_link(path_stop* start_ptr, path_stop* end_ptr, size_t end_idx);
    void calculate_dist(const path_stop* start_ptr);
    void clone(path_link* destination) const;
    bool is_one_way() const;
    
};


/**
 * @brief Manages the paths in the area.
 *
 * Particularly, this keeps an eye out on what stops and links
 * have any sort of obstacle in them that could deter
 * mobs. When these problems disappear, the manager is in charge of alerting
 * all mobs that were following paths, in order to get them recalculate
 * their paths if needed.
 * The reason we want them to recalculate regardless of whether the
 * obstacle affected them or not, is because this obstacle could've freed
 * a different route.
 */
struct path_manager {

    //--- Members ---
    
    //Known obstructions.
    map<path_link*, unordered_set<mob*> > obstructions;
    
    //Stops known to have hazards.
    unordered_set<path_stop*> hazardous_stops;
    
    
    //--- Function declarations ---
    
    void handle_area_load();
    void handle_obstacle_add(mob* m);
    void handle_obstacle_remove(mob* m);
    void handle_sector_hazard_change(sector* sector_ptr);
    void clear();
    
};


bool can_take_path_stop(
    path_stop* stop_ptr, const path_follow_settings &settings,
    PATH_BLOCK_REASON* out_reason = nullptr
);
bool can_take_path_stop(
    const path_stop* stop_ptr, const path_follow_settings &settings,
    sector* sector_ptr, PATH_BLOCK_REASON* out_reason = nullptr
);
bool can_traverse_path_link(
    path_link* link_ptr, const path_follow_settings &settings,
    PATH_BLOCK_REASON* out_reason = nullptr
);
void depth_first_search(
    vector<path_stop*> &nodes,
    unordered_set<path_stop*> &visited, path_stop* start
);
PATH_RESULT a_star(
    vector<path_stop*> &out_path,
    path_stop* start_node, path_stop* end_node,
    const path_follow_settings &settings,
    float* out_total_dist
);
PATH_RESULT get_path(
    const point &start, const point &end,
    const path_follow_settings &settings,
    vector<path_stop*> &full_path, float* out_total_dist,
    path_stop** out_start_stop, path_stop** out_end_stop
);
string path_block_reason_to_string(PATH_BLOCK_REASON reason);
string path_result_to_string(PATH_RESULT result);
