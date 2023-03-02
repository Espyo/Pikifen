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
    //Only useable by mob scripts that reference it.
    PATH_LINK_TYPE_SCRIPT_ONLY,
    //Only for mobs carrying nothing, or a 1-weight mob.
    PATH_LINK_TYPE_LIGHT_LOAD_ONLY,
    //Only for objects that can fly.
    PATH_LINK_TYPE_AIRBORNE_ONLY,
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
    //Links that go to other stops.
    vector<path_link*> links;
    //Sector it's on. Only applicable during gameplay. Cache for performance.
    sector* sector_ptr;
    
    path_stop(
        const point &pos = point(),
        const vector<path_link*> &links = vector<path_link*>()
    );
    ~path_stop();
    void add_link(path_stop* other_stop, const bool normal);
    path_link* get_link(path_stop* other_stop) const;
    void remove_link(path_link* link_ptr);
    void remove_link(path_stop* other_stop);
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
    //Its label, if any.
    string label;
    
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


bool can_traverse_path_link(
    path_link* link_ptr, const path_follow_settings &settings
);
void depth_first_search(
    vector<path_stop*> &nodes,
    unordered_set<path_stop*> &visited, path_stop* start
);
vector<path_stop*> dijkstra(
    path_stop* start_node, path_stop* end_node,
    const path_follow_settings &settings,
    float* total_dist
);
vector<path_stop*> get_path(
    const point &start, const point &end,
    const path_follow_settings &settings,
    bool* go_straight, float* get_dist,
    path_stop** start_stop, path_stop** end_stop
);
mob* get_path_link_obstacle(path_stop* s1, path_stop* s2);


#endif //ifndef PATHING_H_INCLUDED
