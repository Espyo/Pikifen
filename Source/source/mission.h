/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mission class and related functions.
 */

#ifndef MISSION_INCLUDED
#define MISSION_INCLUDED

#include <string>
#include <unordered_set>

#include "game_states/gameplay/gameplay.h"
#include "mob_categories/mob_category.h"
#include "utils/geometry_utils.h"

using std::string;
using std::unordered_set;


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


/* ----------------------------------------------------------------------------
 * Holds information about a given area's mission.
 */
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
    string get_fail_description(const uint8_t id) const;
};


/* ----------------------------------------------------------------------------
 * Class interface for a mission goal.
 */
class mission_goal {
public:
    //Numeric ID. Uses MISSION_GOALS.
    virtual MISSION_GOALS get_id() const = 0;
    //The goal's name.
    virtual string get_name() const = 0;
    //A description for the player, fed from the mission data.
    virtual string get_player_description(mission_data* mission) const = 0;
    //Status for the pause menu.
    virtual string get_status(
        const int cur, const int req, const float percentage
    ) const = 0;
    //Celebrates the player's victory with values fed from the mission data.
    virtual string get_congratulation(mission_data* mission) const = 0;
    //Relevant mob category, if any.
    virtual MOB_CATEGORIES get_relevant_mob_cat() const = 0;
    //Returns the player's current amount for whatever is needed.
    virtual int get_cur_amount(gameplay_state* gameplay) const = 0;
    //Returns the player's required amount for whatever is needed.
    virtual int get_req_amount(gameplay_state* gameplay) const = 0;
    //HUD label for the player's current amount.
    virtual string get_hud_label() const = 0;
    //Returns where the camera should go to to zoom on the mission end reason.
    virtual bool get_mission_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
    ) const = 0;
    virtual bool is_met(gameplay_state* gameplay) const = 0;
};


/* ----------------------------------------------------------------------------
 * Class representing the "battle enemies" mission goal.
 */
class mission_goal_battle_enemies : public mission_goal {
public:
    MISSION_GOALS get_id() const override;
    string get_name() const override;
    string get_player_description(mission_data* mission) const override;
    string get_status(
        const int cur, const int req, const float percentage
    ) const override;
    string get_congratulation(mission_data* mission) const override;
    MOB_CATEGORIES get_relevant_mob_cat() const override;
    int get_cur_amount(gameplay_state* gameplay) const override;
    int get_req_amount(gameplay_state* gameplay) const override;
    string get_hud_label() const override;
    bool get_mission_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
    ) const override;
    bool is_met(gameplay_state* gameplay) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "collect treasures" mission goal.
 */
class mission_goal_collect_treasures : public mission_goal {
public:
    MISSION_GOALS get_id() const override;
    string get_name() const override;
    string get_player_description(mission_data* mission) const override;
    string get_status(
        const int cur, const int req, const float percentage
    ) const override;
    string get_congratulation(mission_data* mission) const override;
    MOB_CATEGORIES get_relevant_mob_cat() const override;
    int get_cur_amount(gameplay_state* gameplay) const override;
    int get_req_amount(gameplay_state* gameplay) const override;
    string get_hud_label() const override;
    bool get_mission_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
    ) const override;
    bool is_met(gameplay_state* gameplay) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "end manually" mission goal.
 */
class mission_goal_end_manually : public mission_goal {
public:
    MISSION_GOALS get_id() const override;
    string get_name() const override;
    string get_player_description(mission_data* mission) const override;
    string get_status(
        const int cur, const int req, const float percentage
    ) const override;
    string get_congratulation(mission_data* mission) const override;
    MOB_CATEGORIES get_relevant_mob_cat() const override;
    int get_cur_amount(gameplay_state* gameplay) const override;
    int get_req_amount(gameplay_state* gameplay) const override;
    string get_hud_label() const override;
    bool get_mission_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
    ) const override;
    bool is_met(gameplay_state* gameplay) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "get to the exit" mission goal.
 */
class mission_goal_get_to_exit : public mission_goal {
public:
    MISSION_GOALS get_id() const override;
    string get_name() const override;
    string get_player_description(mission_data* mission) const override;
    string get_status(
        const int cur, const int req, const float percentage
    ) const override;
    string get_congratulation(mission_data* mission) const override;
    MOB_CATEGORIES get_relevant_mob_cat() const override;
    int get_cur_amount(gameplay_state* gameplay) const override;
    int get_req_amount(gameplay_state* gameplay) const override;
    string get_hud_label() const override;
    bool get_mission_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
    ) const override;
    bool is_met(gameplay_state* gameplay) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "reach Pikmin amount" mission goal.
 */
class mission_goal_reach_pikmin_amount : public mission_goal {
public:
    MISSION_GOALS get_id() const override;
    string get_name() const override;
    string get_player_description(mission_data* mission) const override;
    string get_status(
        const int cur, const int req, const float percentage
    ) const override;
    string get_congratulation(mission_data* mission) const override;
    MOB_CATEGORIES get_relevant_mob_cat() const override;
    int get_cur_amount(gameplay_state* gameplay) const override;
    int get_req_amount(gameplay_state* gameplay) const override;
    string get_hud_label() const override;
    bool get_mission_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
    ) const override;
    bool is_met(gameplay_state* gameplay) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "timed survival" mission goal.
 */
class mission_goal_timed_survival : public mission_goal {
public:
    MISSION_GOALS get_id() const override;
    string get_name() const override;
    string get_player_description(mission_data* mission) const override;
    string get_status(
        const int cur, const int req, const float percentage
    ) const override;
    string get_congratulation(mission_data* mission) const override;
    MOB_CATEGORIES get_relevant_mob_cat() const override;
    int get_cur_amount(gameplay_state* gameplay) const override;
    int get_req_amount(gameplay_state* gameplay) const override;
    string get_hud_label() const override;
    bool get_mission_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
    ) const override;
    bool is_met(gameplay_state* gameplay) const override;
};


#endif //ifndef MISSION_INCLUDED
