/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the area class and related functions.
 */

#ifndef AREA_INCLUDED
#define AREA_INCLUDED

#include <memory>

#include "sector.h"


//Types of areas that can be played.
enum AREA_TYPES {
    //A simple area with no goal.
    AREA_TYPE_SIMPLE,
    //An area that likely has a goal, constraints, and/or scoring.
    AREA_TYPE_MISSION,
};


//Possible goals in a mission.
enum MISSION_GOALS {
    //The player plays until they end from the pause menu.
    MISSION_GOAL_END_MANUALLY,
    //The player must collect certain treasures, or all of them.
    MISSION_GOAL_COLLECT_TREASURE,
    //The player must defeat certain enemies, or all of them.
    MISSION_GOAL_BATTLE_ENEMIES,
    //The player must survive for a certain amount of time.
    MISSION_GOAL_TIMED_SURVIVAL,
    //The player must get a leader or all of them to the exit point.
    MISSION_GOAL_GET_TO_EXIT,
    //The player must reach a certain number of total Pikmin.
    MISSION_GOAL_REACH_PIKMIN_AMOUNT,
};


//Possible ways of grading the player for a mission.
enum MISSION_GRADING_MODES {
    //Based on points in different criteria.
    MISSION_GRADING_POINTS,
    //Based on whether the player reached the goal or not.
    MISSION_GRADING_GOAL,
    //Based on whether the player played or not.
    MISSION_GRADING_PARTICIPATION,
};


//Possible ways to fail at a mission. This should be a bitmask.
enum MISSION_FAIL_CONDITIONS {
    //Ending from the pause menu.
    MISSION_FAIL_COND_PAUSE_MENU = 0x01,
    //Reaching a certain Pikmin amount. 0 = total extinction.
    MISSION_FAIL_COND_PIKMIN_AMOUNT = 0x02,
    //Losing a certain amount of Pikmin.
    MISSION_FAIL_COND_LOSE_PIKMIN = 0x04,
    //A leader takes damage.
    MISSION_FAIL_COND_TAKE_DAMAGE = 0x08,
    //Losing a certain amount of leaders.
    MISSION_FAIL_COND_LOSE_LEADERS = 0x10,
    //Killing a certain amount of enemies.
    MISSION_FAIL_COND_KILL_ENEMIES = 0x20,
    //Reaching the time limit.
    MISSION_FAIL_COND_TIME_LIMIT = 0x40,
};


//Possible types of mission medal.
enum MISSION_MEDALS {
    //None.
    MISSION_MEDAL_NONE,
    //Bronze.
    MISSION_MEDAL_BRONZE,
    //Silver.
    MISSION_MEDAL_SILVER,
    //Gold.
    MISSION_MEDAL_GOLD,
    //Platinum.
    MISSION_MEDAL_PLATINUM,
};


//Possible criteria for a mission's point scoring. This should be a bitmask.
enum MISSION_POINT_CRITERIA {
    //Points per Pikmin born.
    MISSION_POINT_CRITERIA_PIKMIN_BORN = 0x01,
    //Points per Pikmin death.
    MISSION_POINT_CRITERIA_PIKMIN_DEATH = 0x02,
    //Points per second left. Only for missions with a time limit.
    MISSION_POINT_CRITERIA_SEC_LEFT = 0x04,
    //Points per second passed.
    MISSION_POINT_CRITERIA_SEC_PASSED = 0x08,
    //Points per treasure point.
    MISSION_POINT_CRITERIA_TREASURE_POINTS = 0x10,
    //Points per enemy kill point.
    MISSION_POINT_CRITERIA_ENEMY_POINTS = 0x20,
};


namespace AREA {
extern const unsigned char DEF_DIFFICULTY;
extern const size_t DEF_DAY_TIME_START;
extern const float DEF_DAY_TIME_SPEED;
extern const int DEF_MISSION_MEDAL_BRONZE_REQ;
extern const int DEF_MISSION_MEDAL_SILVER_REQ;
extern const int DEF_MISSION_MEDAL_GOLD_REQ;
extern const int DEF_MISSION_MEDAL_PLATINUM_REQ;
extern const size_t DEF_MISSION_TIME_LIMIT;
};


struct mission_data {
    //Mission goal.
    MISSION_GOALS goal;
    //Does the mission goal require all relevant items, or just specific ones?
    bool goal_all_mobs;
    //If the mission goal requires specific items, their mob indexes go here.
    unordered_set<size_t> goal_mob_idxs;
    //Total amount of something required for the current mission goal.
    size_t goal_amount;
    //If the mission goal requires an amount, is it >= or <= ?
    bool goal_higher_than;
    //Mission exit region center coordinates.
    point goal_exit_center;
    //Mission exit region dimensions.
    point goal_exit_size;
    //Mission fail conditions bitmask. Use MISSION_FAIL_COND_*.
    uint8_t fail_conditions;
    //Amount for the "reach Pikmin amount" mission fail condition.
    size_t fail_pik_amount;
    //Is the mission "reach Pikmin amount" fail condition >= or <= ?
    bool fail_pik_higher_than;
    //Amount for the "lose Pikmin" mission fail condition.
    size_t fail_pik_killed;
    //Amount for the "lose leaders" mission fail condition.
    size_t fail_leaders_kod;
    //Amount for the "kill enemies" mission fail condition.
    size_t fail_enemies_killed;
    //Seconds amount for the "time limit" mission fail condition.
    size_t fail_time_limit;
    //Mission grading mode.
    MISSION_GRADING_MODES grading_mode;
    //Mission point multiplier for each Pikmin born.
    int points_per_pikmin_born;
    //Mission point multiplier for each Pikmin lost.
    int points_per_pikmin_death;
    //Mission point multiplier for each second left (only if time limit is on).
    int points_per_sec_left;
    //Mission point multiplier for each second passed.
    int points_per_sec_passed;
    //Mission point multiplier for each treasure point obtained.
    int points_per_treasure_point;
    //Mission point multiplier for each enemy point obtained.
    int points_per_enemy_point;
    //Bitmask for mission fail point loss criteria. Use MISSION_POINT_CRITERIA.
    uint8_t point_loss_data;
    //Starting number of points.
    int starting_points;
    //Bronze medal point requirement.
    int bronze_req;
    //Silver medal point requirement.
    int silver_req;
    //Gold medal point requirement.
    int gold_req;
    //Platinum medal point requirement.
    int platinum_req;
    
    mission_data();
};


/* ----------------------------------------------------------------------------
 * A structure that holds all of the
 * info about the current area, so that
 * the sectors know how to communicate with
 * the edges, the edges with the
 * vertexes, etc.
 */
struct area_data {
    //Type of area.
    AREA_TYPES type;
    //Name of the folder with this area's data.
    string folder_name;
    //Blockmap.
    blockmap bmap;
    //List of vertexes.
    vector<vertex*> vertexes;
    //List of edges.
    vector<edge*> edges;
    //List of sectors.
    vector<sector*> sectors;
    //List of mob generators.
    vector<mob_gen*> mob_generators;
    //List of path stops.
    vector<path_stop*> path_stops;
    //List of tree shadows.
    vector<tree_shadow*> tree_shadows;
    //Bitmap of the background.
    ALLEGRO_BITMAP* bg_bmp;
    //File name of the background bitmap.
    string bg_bmp_file_name;
    //Zoom the background by this much.
    float bg_bmp_zoom;
    //How far away the background is.
    float bg_dist;
    //Tint the background with this color.
    ALLEGRO_COLOR bg_color;
    //Name of the area. This is not the internal name.
    string name;
    //Area subtitle, if any.
    string subtitle;
    //Area description, if any.
    string description;
    //Area tags, separated by semicolon, if any.
    std::shared_ptr<ALLEGRO_BITMAP> thumbnail;
    //Thumbnail, if any.
    string tags;
    //Area difficulty, if applicable. Goes from 1 to 5.
    unsigned char difficulty;
    //Who made this area.
    string maker;
    //Optional version number.
    string version;
    //Any notes from the person who made it, for other makers to see.
    string notes;
    //Version of the engine this area was built in.
    string engine_version;
    //String representing the starting amounts of each spray.
    string spray_amounts;
    //Weather condition to use.
    weather weather_condition;
    //Name of the weather condition to use.
    string weather_name;
    //Area day time at the start of gameplay. This is in minutes.
    size_t day_time_start;
    //Area day time speed, in game-minutes per real-minutes.
    float day_time_speed;
    //Known geometry problems.
    geometry_problems problems;
    //Mission data.
    mission_data mission;
    
    area_data();
    void check_stability();
    void clone(area_data &other);
    void connect_edge_to_sector(edge* e_ptr, sector* s_ptr, size_t side);
    void connect_edge_to_vertex(edge* e_ptr, vertex* v_ptr, size_t endpoint);
    void connect_sector_edges(sector* s_ptr);
    void connect_vertex_edges(vertex* v_ptr);
    size_t find_edge_nr(const edge* e_ptr) const;
    size_t find_mob_gen_nr(const mob_gen* m_ptr) const;
    size_t find_sector_nr(const sector* s_ptr) const;
    size_t find_vertex_nr(const vertex* v_ptr) const;
    void fix_edge_nrs(edge* e_ptr);
    void fix_edge_pointers(edge* e_ptr);
    void fix_path_stop_nrs(path_stop* s_ptr);
    void fix_path_stop_pointers(path_stop* s_ptr);
    void fix_sector_nrs(sector* s_ptr);
    void fix_sector_pointers(sector* s_ptr);
    void fix_vertex_nrs(vertex* v_ptr);
    void fix_vertex_pointers(vertex* v_ptr);
    void generate_blockmap();
    void generate_edges_blockmap(vector<edge*> &edges);
    size_t get_nr_path_links();
    void load_thumbnail(const string &thumbnail_path);
    edge* new_edge();
    sector* new_sector();
    vertex* new_vertex();
    void remove_vertex(const size_t v_nr);
    void remove_vertex(const vertex* v_ptr);
    void remove_edge(const size_t e_nr);
    void remove_edge(const edge* e_ptr);
    void remove_sector(const size_t s_nr);
    void remove_sector(const sector* s_ptr);
    void clear();
};

void get_area_info_from_path(
    const string &requested_area_path,
    string* final_area_folder_name,
    AREA_TYPES* final_area_type
);
string get_base_area_folder_path(
    const AREA_TYPES type, const bool from_game_data
);

#endif //ifndef AREA_INCLUDED
