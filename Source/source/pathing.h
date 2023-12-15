/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the paths, path-finding, and related functions.
 */

#ifndef PATHING_H_INCLUDED
#define PATHING_H_INCLUDED

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "hazard.h"
#include "utils/geometry_utils.h"

using std::map;
using std::string;
using std::unordered_set;
using std::vector;


class mob;
struct sector;
struct path_link;


//Types of path link.
enum PATH_LINK_TYPES {
    //Normal.
    PATH_LINK_TYPE_NORMAL,
    //One-way fall for normal mobs, two-way for airborne mobs.
    PATH_LINK_TYPE_LEDGE,
};


//Flags for path stops.
enum PATH_STOP_FLAGS {
    //Only usable by mob scripts that reference it.
    PATH_STOP_SCRIPT_ONLY = 0x01,
    //Only for mobs carrying nothing, or a 1-weight mob.
    PATH_STOP_LIGHT_LOAD_ONLY = 0x02,
    //Only for mobs that can fly.
    PATH_STOP_AIRBORNE_ONLY = 0x04,
};


//Flags that control how paths should be followed.
enum PATH_FOLLOW_FLAGS {
    //It's possible to continue from the last path if it wants.
    PATH_FOLLOW_FLAG_CAN_CONTINUE = 0x0001,
    //Ignore any obstacles in the path links.
    PATH_FOLLOW_FLAG_IGNORE_OBSTACLES = 0x0002,
    //At the end, constantly chase after the target mob (if any)'s position.
    PATH_FOLLOW_FLAG_FOLLOW_MOB = 0x0004,
    //Use the faked start point instead of the normal one.
    PATH_FOLLOW_FLAG_FAKED_START = 0x0008,
    //Use the faked end point instead of the normal one.
    PATH_FOLLOW_FLAG_FAKED_END = 0x0010,
    //The mob was told to use this path by a script.
    PATH_FOLLOW_FLAG_SCRIPT_USE = 0x0020,
    //The mob has light load.
    PATH_FOLLOW_FLAG_LIGHT_LOAD = 0x0040,
    //The mob can fly.
    PATH_FOLLOW_FLAG_AIRBORNE = 0x0080,
};


//Possible results for when a path is decided.
//Positive values mean the mob can go, negative values means it can't.
enum PATH_RESULTS {
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
enum PATH_BLOCK_REASONS {
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
extern const float MIN_STOP_RADIUS;
}


/* ----------------------------------------------------------------------------
 * Structure with settings about how a mob should follow a path.
 */
struct path_follow_settings {
    //Target location.
    point target_point;
    //If the target is a mob, this points to it.
    mob* target_mob;
    //For the final chase, from the last path stop to
    //the destination, use this for the target distance parameter.
    float final_target_distance;
    //Some flags. Use PATH_FOLLOW_FLAG_*.
    unsigned char flags;
    //Invulnerabilities of the mob/carriers.
    vector<hazard*> invulnerabilities;
    //If not empty, only follow path links with this label.
    string label;
    //Faked start point. Used to fake calculations.
    point faked_start;
    //Faked end point. Used to fake calculations.
    point faked_end;
    
    path_follow_settings();
};


/* ----------------------------------------------------------------------------
 * Stops are points that make up a path. In mathematics, this is a node
 * in the graph. In a real-world example, this is a bus stop.
 * Pikmin start carrying by going for the closest stop.
 * Then they move stop by stop, following the connections, until they
 * reach the final stop and go wherever they need.
 */
struct path_stop {
    //Coordinates.
    point pos;
    //Radius.
    float radius;
    //Flags. Use PATH_STOP_FLAGS.
    uint8_t flags;
    //Its label, if any.
    string label;
    //Links that go to other stops.
    vector<path_link*> links;
    //Sector it's on. Only applicable during gameplay. Cache for performance.
    sector* sector_ptr;
    
    path_stop(
        const point &pos = point(),
        const vector<path_link*> &links = vector<path_link*>()
    );
    ~path_stop();
    void clone(path_stop* destination) const;
    void add_link(path_stop* other_stop, const bool normal);
    path_link* get_link(const path_stop* other_stop) const;
    void remove_link(const path_link* link_ptr);
    void remove_link(const path_stop* other_stop);
    void calculate_dists();
    void calculate_dists_plus_neighbors();
};


/* ----------------------------------------------------------------------------
 * Info about a path link. A path stop can link to N other path stops,
 * and this structure holds information about a connection.
 */
struct path_link {
    //Pointer to the path stop at the start.
    path_stop* start_ptr;
    //Pointer to the path stop at the end.
    path_stop* end_ptr;
    //Index number of the path stop at the end.
    size_t end_nr;
    
    //Type. Used for special restrictions and behaviors.
    PATH_LINK_TYPES type;
    
    //Distance between the two stops.
    float distance;
    //Is the stop currently blocked by an obstacle? Cache for performance.
    bool blocked_by_obstacle;
    
    path_link(path_stop* start_ptr, path_stop* end_ptr, size_t end_nr);
    void calculate_dist(path_stop* start_ptr);
    void clone(path_link* destination) const;
    bool is_one_way() const;
};


/* ----------------------------------------------------------------------------
 * Manages the paths in the area. Particularly, this keeps an eye out on
 * what stops and links have any sort of obstacle in them that could deter
 * mobs. When these problems disappear, the manager is in charge of alerting
 * all mobs that were following paths, in order to get them recalculate
 * their paths if needed.
 * The reason we want them to recalculate regardless of whether the
 * obstacle affected them or not, is because this obstacle could've freed
 * a different route.
 */
struct path_manager {
    //Known obstructions.
    map<path_link*, unordered_set<mob*> > obstructions;
    //Stops known to have hazards.
    unordered_set<path_stop*> hazardous_stops;
    
    void handle_area_load();
    void handle_obstacle_add(mob* m);
    void handle_obstacle_remove(mob* m);
    void handle_sector_hazard_change(sector* sector_ptr);
    void clear();
};


bool can_take_path_stop(
    path_stop* stop_ptr, const path_follow_settings &settings,
    PATH_BLOCK_REASONS* reason = NULL
);
bool can_take_path_stop(
    path_stop* stop_ptr, const path_follow_settings &settings,
    sector* sector_ptr, PATH_BLOCK_REASONS* reason = NULL
);
bool can_traverse_path_link(
    path_link* link_ptr, const path_follow_settings &settings,
    PATH_BLOCK_REASONS* reason = NULL
);
void depth_first_search(
    vector<path_stop*> &nodes,
    unordered_set<path_stop*> &visited, path_stop* start
);
PATH_RESULTS dijkstra(
    vector<path_stop*> &final_path,
    path_stop* start_node, path_stop* end_node,
    const path_follow_settings &settings,
    float* total_dist
);
PATH_RESULTS get_path(
    const point &start, const point &end,
    const path_follow_settings &settings,
    vector<path_stop*> &full_path, float* total_dist,
    path_stop** start_stop, path_stop** end_stop
);
string path_block_reason_to_string(PATH_BLOCK_REASONS reason);
string path_result_to_string(PATH_RESULTS result);


#endif //ifndef PATHING_H_INCLUDED
