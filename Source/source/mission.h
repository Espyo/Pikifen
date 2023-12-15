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

#include "mob_categories/mob_category.h"
#include "utils/geometry_utils.h"

using std::string;
using std::unordered_set;


class gameplay_state;


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
    //The player must grow enough Pikmin to reach a certain total.
    MISSION_GOAL_GROW_PIKMIN,
    //Anyone other than the team reaches their goal
    //This team is inserted into first place, and the other team goes in second,
    // unless this one is eliminated
    MISSION_GOAL_ANYONE_WINS,
    //The team must eliminate all teams on the list
    MISSION_GOAL_ELIMINATE_LIST,
    //The team must cause this amount of teams to fail
    MISSION_GOAL_ELIMINATE_AMOUNT,
    //A mob sends out a success action aimed at this team
//    MISSION_GOAL_SCRIPTED,
};


//Possible ways to fail at a mission.
enum MISSION_FAIL_CONDITIONS {
    //Reaching the time limit.
    MISSION_FAIL_COND_TIME_LIMIT,
    //Reaching a certain Pikmin amount or fewer. 0 = total extinction.
    MISSION_FAIL_COND_TOO_FEW_PIKMIN,
    //Reaching a certain Pikmin amount or more.
    MISSION_FAIL_COND_TOO_MANY_PIKMIN,
    //Losing a certain amount of Pikmin.
    MISSION_FAIL_COND_LOSE_PIKMIN,
    //A leader takes damage.
    MISSION_FAIL_COND_TAKE_DAMAGE,
    //Losing a certain amount of leaders.
    MISSION_FAIL_COND_LOSE_LEADERS,
    //Killing a certain amount of enemies.
    MISSION_FAIL_COND_KILL_ENEMIES,
    //Ending from the pause menu.
    MISSION_FAIL_COND_PAUSE_MENU,
    //The goal condition is no longer reachable
    MISSION_FAIL_COND_MISSION_IMPOSSIBLE,
    //Anyone other than the team reaches their goal
    MISSION_FAIL_COND_AMOUNT_WINS,
    //Everybody else reaches their goal
    MISSION_FAIL_COND_LIST_ALL_WIN,
    //Everybody else reaches their goal
    //A mob sends out a failure action aimed at this team
//    MISSION_FAIL_COND_SCRIPTED,
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


//Possible ways of grading the player for a mission.
enum MISSION_GRADING_MODES {
    //Based on points in different criteria.
    MISSION_GRADING_POINTS,
    //Based on whether the player reached the goal or not.
    MISSION_GRADING_GOAL,
    //Based on whether the player played or not.
    MISSION_GRADING_PARTICIPATION,
};


//Possible criteria for a mission's point scoring.
enum MISSION_SCORE_CRITERIA {
    //Points per Pikmin born.
    MISSION_SCORE_CRITERIA_PIKMIN_BORN,
    //Points per Pikmin death.
    MISSION_SCORE_CRITERIA_PIKMIN_DEATH,
    //Points per second left. Only for missions with a time limit.
    MISSION_SCORE_CRITERIA_SEC_LEFT,
    //Points per second passed.
    MISSION_SCORE_CRITERIA_SEC_PASSED,
    //Points per treasure point.
    MISSION_SCORE_CRITERIA_TREASURE_POINTS,
    //Points per enemy kill point.
    MISSION_SCORE_CRITERIA_ENEMY_POINTS,
};

enum MISSION_RANKING_ORDER {
    // based on Completion order
    MISSION_RANKING_ORDER_COMPLETION_TIME,
    // based on who reached the goal first
    MISSION_RANKING_ORDER_FIRST_PAST_THE_POST,
    // based on who failed and who won
    MISSION_RANKING_ORDER_WINNERS_AND_LOSERS,
    // based on The medal they obtained
    MISSION_RANKING_ORDER_MEDAL,
    // based on the amount of points they have
    MISSION_RANKING_ORDER_POINTS,
};
struct mission_team_data {
    //Mission goal.
    MISSION_GOALS goal;
    //Does the mission goal require all relevant items, or just specific ones?
    bool goal_all_mobs;
    //If the mission goal requires specific items, their mob indexes go here.
    unordered_set<size_t> goal_mob_idxs;
    //Total amount of something required for the current mission goal.
    size_t goal_amount;
    //Mission exit region center coordinates.
    point goal_exit_center;
    //Mission exit region dimensions.
    point goal_exit_size;
    //Mission fail conditions bitmask. Use MISSION_FAIL_COND_*'s indexes.
    int fail_conditions;
    //Amount for the "reach too few Pikmin" mission fail condition.
    size_t fail_too_few_pik_amount;
    //Amount for the "reach too many Pikmin" mission fail condition.
    size_t fail_too_many_pik_amount;
    //Amount for the "lose Pikmin" mission fail condition.
    size_t fail_pik_killed;
    //Amount for the "lose leaders" mission fail condition.
    size_t fail_leaders_kod;
    //Amount for the "kill enemies" mission fail condition.
    size_t fail_enemies_killed;
    //Seconds amount for the "time limit" mission fail condition.
    size_t fail_time_limit;
    //Primary HUD element's fail condition. INVALID for none.
    size_t fail_hud_primary_cond;
    //Secondary HUD element's fail condition. INVALID for none.
    size_t fail_hud_secondary_cond;
    //If the mission goal requires specific items, their mob indexes go here.
    unordered_set<size_t> fail_if_win_idxs;
    //Total amount of something required for the current mission goal.
    size_t fail_if_win_amount;
    mission_team_data();
};
/* ----------------------------------------------------------------------------
 * Holds information about a given area's mission.
 */
struct mission_data {
    mission_team_data team_data[4UL];
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
    //Bitmask for mission fail point loss criteria. Use MISSION_SCORE_CRITERIA.
    uint8_t point_loss_data;
    //Bitmask for score HUD calculation criteria. Use MISSION_SCORE_CRITERIA.
    uint8_t point_hud_data;
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
    //Mission Ranking mode, for ranking who won a versus Missions
    MISSION_RANKING_ORDER ranking_order;
    mission_data();
};


/* ----------------------------------------------------------------------------
 * Holds information about a given mission's record.
 */
struct mission_record {
    //Has the mission's goal been cleared?
    bool clear;
    //Score obtained.
    int score;
    //Date of the record.
    string date;
    
    mission_record();
    bool is_platinum(const mission_data &mission);
};


/* ----------------------------------------------------------------------------
 * Class interface for a mission fail condition.
 */
class mission_fail {
public:
    //The condition's name.
    virtual string get_name() const = 0;
    //Returns the player's current amount for whatever the condition needs.
    virtual int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const = 0;
    //Returns the player's required amount for whatever the condition needs.
    virtual int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const = 0;
    //A description for the player, fed from the mission data.
    virtual string get_player_description(mission_data* mission,
    const int team_nr=0) const = 0;
    //Status for the pause menu.
    virtual string get_status(
        const int cur, const int req, const float percentage,
        const int team_nr=0
    ) const = 0;
    //Explains why the player lost, with values fed from the mission data.
    virtual string get_end_reason(mission_data* mission,
    const int team_nr=0) const = 0;
    //Returns where the camera should go to to zoom on the mission end reason.
    virtual bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,
        const int team_nr=0
    ) const = 0;
    //HUD label for the player's current amount.
    virtual string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const = 0;
    //Whether it has anything to show in the HUD.
    virtual bool has_hud_content() const = 0;
    //Checks if its conditions have been met to end the mission as a fail.
    virtual bool is_met(gameplay_state* gameplay,const int team_nr=0) const = 0;
};


/* ----------------------------------------------------------------------------
 * Class representing the "kill enemies" mission fail condition.
 */
class mission_fail_kill_enemies : public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "lose leaders" mission fail condition.
 */
class mission_fail_lose_leaders : public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "lose Pikmin" mission fail condition.
 */
class mission_fail_lose_pikmin : public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "end from pause menu" mission fail condition.
 */
class mission_fail_pause_menu : public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "take damage" mission fail condition.
 */
class mission_fail_take_damage : public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "time limit" mission fail condition.
 */
class mission_fail_time_limit: public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "reach too few Pikmin" mission fail condition.
 */
class mission_fail_too_few_pikmin : public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "reach too many Pikmin" mission fail condition.
 */
class mission_fail_too_many_pikmin : public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};

/* ----------------------------------------------------------------------------
 * Class representing the "eliminate everyone else" mission fail.
 */
class mission_fail_win_list : public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "eliminate anyone" mission fail.
 */
class mission_fail_win_amount : public mission_fail {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label(gameplay_state* gameplay,const int team_nr=0) const override;
    bool has_hud_content() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
};
/* ----------------------------------------------------------------------------
 * Class interface for a mission goal.
 */
class mission_goal {
public:
    //The goal's name.
    virtual string get_name() const = 0;
    //Returns the player's current amount for whatever the goal needs.
    virtual int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const = 0;
    //Returns the player's required amount for whatever the goal needs.
    virtual int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const = 0;
    //A description for the player, fed from the mission data.
    virtual string get_player_description(mission_data* mission,const int team_nr=0) const = 0;
    //Status for the pause menu.
    virtual string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const = 0;
    //Celebrates the player's victory with values fed from the mission data.
    virtual string get_end_reason(mission_data* mission,const int team_nr=0) const = 0;
    //Returns where the camera should go to to zoom on the mission end reason.
    virtual bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const = 0;
    //HUD label for the player's current amount.
    virtual string get_hud_label() const = 0;
    //Checks if its conditions have been met to end the mission as a clear.
    virtual bool is_met(gameplay_state* gameplay,const int team_nr=0) const = 0;
    //Returns whether a given mob is applicable to this goal's required mobs.
    virtual bool is_mob_applicable(mob_type* type,const int team_nr=0) const = 0;
};


/* ----------------------------------------------------------------------------
 * Class representing the "battle enemies" mission goal.
 */
class mission_goal_battle_enemies : public mission_goal {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "collect treasures" mission goal.
 */
class mission_goal_collect_treasures : public mission_goal {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "end manually" mission goal.
 */
class mission_goal_end_manually : public mission_goal {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "get to the exit" mission goal.
 */
class mission_goal_get_to_exit : public mission_goal {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "reach Pikmin amount" mission goal.
 */
class mission_goal_grow_pikmin : public mission_goal {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "timed survival" mission goal.
 */
class mission_goal_timed_survival : public mission_goal {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "anyone else wins" mission goal.
 */
class mission_goal_anyone_wins : public mission_goal {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "eliminate everyone else" mission goal.
 */
class mission_goal_eliminate_list : public mission_goal {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "eliminate anyone" mission goal.
 */
class mission_goal_eliminate_amount : public mission_goal {
public:
    string get_name() const override;
    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
    string get_player_description(mission_data* mission,const int team_nr=0) const override;
    string get_status(
        const int cur, const int req, const float percentage,const int team_nr=0
    ) const override;
    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
    bool get_end_zoom_data(
        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
    ) const override;
    string get_hud_label() const override;
    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
};




/* ----------------------------------------------------------------------------
 * Class representing the "scripted" mission goal.
 */
//class mission_goal_scripted : public mission_goal {
//public:
//    string get_name() const override;
//    int get_cur_amount(gameplay_state* gameplay,const int team_nr=0) const override;
//    int get_req_amount(gameplay_state* gameplay,const int team_nr=0) const override;
//    string get_player_description(mission_data* mission,const int team_nr=0) const override;
//    string get_status(
//        const int cur, const int req, const float percentage,const int team_nr=0
//    ) const override;
//    string get_end_reason(mission_data* mission,const int team_nr=0) const override;
//    bool get_end_zoom_data(
//        gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom,const int team_nr=0
//    ) const override;
//    string get_hud_label() const override;
//    bool is_met(gameplay_state* gameplay,const int team_nr=0) const override;
//    bool is_mob_applicable(mob_type* type,const int team_nr=0) const override;
//};


/* ----------------------------------------------------------------------------
 * Class interface for a mission score criterion.
 */
class mission_score_criterion {
public:
    //The criterion's name.
    virtual string get_name() const = 0;
    //The criterion's point multiplier.
    virtual int get_multiplier(mission_data* mission,const int team_nr=0) const = 0;
    //Returns the player's score for this criterion.
    virtual int get_score(
        gameplay_state* gameplay, mission_data* mission,const int team_nr=0
    ) const = 0;
};


/* ----------------------------------------------------------------------------
 * Class representing the "enemy points" mission score criterion.
 */
class mission_score_criterion_enemy_points : public mission_score_criterion {
public:
    string get_name() const override;
    int get_multiplier(mission_data* mission,const int team_nr=0) const override;
    int get_score(
        gameplay_state* gameplay, mission_data* mission,const int team_nr=0
    ) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "Pikmin born" mission score criterion.
 */
class mission_score_criterion_pikmin_born : public mission_score_criterion {
public:
    string get_name() const override;
    int get_multiplier(mission_data* mission,const int team_nr=0) const override;
    int get_score(
        gameplay_state* gameplay, mission_data* mission,const int team_nr=0
    ) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "Pikmin death" mission score criterion.
 */
class mission_score_criterion_pikmin_death : public mission_score_criterion {
public:
    string get_name() const override;
    int get_multiplier(mission_data* mission,const int team_nr=0) const override;
    int get_score(
        gameplay_state* gameplay, mission_data* mission,const int team_nr=0
    ) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "seconds left" mission score criterion.
 */
class mission_score_criterion_sec_left : public mission_score_criterion {
public:
    string get_name() const override;
    int get_multiplier(mission_data* mission,const int team_nr=0) const override;
    int get_score(
        gameplay_state* gameplay, mission_data* mission,const int team_nr=0
    ) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "seconds passed" mission score criterion.
 */
class mission_score_criterion_sec_passed : public mission_score_criterion {
public:
    string get_name() const override;
    int get_multiplier(mission_data* mission,const int team_nr=0) const override;
    int get_score(
        gameplay_state* gameplay, mission_data* mission,const int team_nr=0
    ) const override;
};


/* ----------------------------------------------------------------------------
 * Class representing the "treasure points" mission score criterion.
 */
class mission_score_criterion_treasure_points : public mission_score_criterion {
public:
    string get_name() const override;
    int get_multiplier(mission_data* mission,const int team_nr=0) const override;
    int get_score(
        gameplay_state* gameplay, mission_data* mission,const int team_nr=0
    ) const override;
};


#endif //ifndef MISSION_INCLUDED
