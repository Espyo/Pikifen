/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mission class and related functions.
 */

#include "mission.h"

#include "functions.h"
#include "game.h"
#include "game_states/area_editor/editor.h"
#include "utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Initializes a mission data struct.
 */
mission_data::mission_data() :
    goal(MISSION_GOAL_END_MANUALLY),
    goal_all_mobs(true),
    goal_amount(1),
    goal_exit_size(
        AREA_EDITOR::MISSION_EXIT_MIN_SIZE, AREA_EDITOR::MISSION_EXIT_MIN_SIZE
    ),
    fail_conditions(0),
    fail_too_few_pik_amount(0),
    fail_too_many_pik_amount(1),
    fail_pik_killed(1),
    fail_leaders_kod(1),
    fail_enemies_killed(1),
    fail_time_limit(AREA::DEF_MISSION_TIME_LIMIT),
    fail_hud_primary_cond(INVALID),
    fail_hud_secondary_cond(INVALID),
    grading_mode(MISSION_GRADING_GOAL),
    points_per_pikmin_born(0),
    points_per_pikmin_death(0),
    points_per_sec_left(0),
    points_per_sec_passed(0),
    points_per_treasure_point(0),
    points_per_enemy_point(0),
    point_loss_data(0),
    point_hud_data(255),
    starting_points(0),
    bronze_req(AREA::DEF_MISSION_MEDAL_BRONZE_REQ),
    silver_req(AREA::DEF_MISSION_MEDAL_SILVER_REQ),
    gold_req(AREA::DEF_MISSION_MEDAL_GOLD_REQ),
    platinum_req(AREA::DEF_MISSION_MEDAL_PLATINUM_REQ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_kill_enemies::get_cur_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->enemy_deaths;
}


/* ----------------------------------------------------------------------------
 * Explains why the player lost, with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_kill_enemies::get_end_reason(
    mission_data* mission
) const {
    return
        "Killed " +
        nr_and_plural(mission->fail_enemies_killed, "enemy", "enemies") +
        "...";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_fail_kill_enemies::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->last_enemy_killed_pos.x != LARGE_FLOAT) {
        *final_cam_pos = gameplay->last_enemy_killed_pos;
        *final_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
string mission_fail_kill_enemies::get_hud_label(
    gameplay_state* gameplay
) const {
    return "Enemies";
}


/* ----------------------------------------------------------------------------
 * Returns the condition's name.
 */
string mission_fail_kill_enemies::get_name() const {
    return "Kill enemies";
}


/* ----------------------------------------------------------------------------
 * A description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_kill_enemies::get_player_description(
    mission_data* mission
) const {
    return
        "Kill " +
        nr_and_plural(
            mission->fail_enemies_killed, "enemy", "enemies"
        ) +
        " or more.";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_kill_enemies::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) game.cur_area_data.mission.fail_enemies_killed;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_fail_kill_enemies::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have killed " +
        i2s(cur) + "/" + i2s(req) +
        " enemies. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Whether it has anything to show in the HUD.
 */
bool mission_fail_kill_enemies::has_hud_content() const {
    return true;
}


/* ----------------------------------------------------------------------------
 * Checks if its conditions have been met to end the mission as a fail.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_fail_kill_enemies::is_met(
    gameplay_state* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_lose_leaders::get_cur_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->leaders_kod;
}


/* ----------------------------------------------------------------------------
 * Explains why the player lost, with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_lose_leaders::get_end_reason(
    mission_data* mission
) const {
    return
        "Lost " +
        nr_and_plural(mission->fail_leaders_kod, "leader") +
        "...";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_fail_lose_leaders::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->last_hurt_leader_pos.x != LARGE_FLOAT) {
        *final_cam_pos = gameplay->last_hurt_leader_pos;
        *final_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
string mission_fail_lose_leaders::get_hud_label(
    gameplay_state* gameplay
) const {
    return "Leaders lost";
}


/* ----------------------------------------------------------------------------
 * Returns the condition's name.
 */
string mission_fail_lose_leaders::get_name() const {
    return "Lose leaders";
}


/* ----------------------------------------------------------------------------
 * A description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_lose_leaders::get_player_description(
    mission_data* mission
) const {
    return
        "Lose " +
        nr_and_plural(mission->fail_leaders_kod, "leader") +
        " or more.";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_lose_leaders::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) game.cur_area_data.mission.fail_leaders_kod;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_fail_lose_leaders::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have lost " +
        i2s(cur) + "/" + i2s(req) +
        " leaders. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Whether it has anything to show in the HUD.
 */
bool mission_fail_lose_leaders::has_hud_content() const {
    return true;
}


/* ----------------------------------------------------------------------------
 * Checks if its conditions have been met to end the mission as a fail.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_fail_lose_leaders::is_met(
    gameplay_state* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_lose_pikmin::get_cur_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->pikmin_deaths;
}


/* ----------------------------------------------------------------------------
 * Explains why the player lost, with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_lose_pikmin::get_end_reason(
    mission_data* mission
) const {
    return
        "Lost " +
        i2s(mission->fail_pik_killed) +
        " Pikmin...";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_fail_lose_pikmin::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->last_pikmin_death_pos.x != LARGE_FLOAT) {
        *final_cam_pos = gameplay->last_pikmin_death_pos;
        *final_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
string mission_fail_lose_pikmin::get_hud_label(
    gameplay_state* gameplay
) const {
    return "Pikmin lost";
}


/* ----------------------------------------------------------------------------
 * Returns the condition's name.
 */
string mission_fail_lose_pikmin::get_name() const {
    return "Lose Pikmin";
}


/* ----------------------------------------------------------------------------
 * A description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_lose_pikmin::get_player_description(
    mission_data* mission
) const {
    return
        "Lose " + i2s(mission->fail_pik_killed) + " Pikmin or more.";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_lose_pikmin::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) game.cur_area_data.mission.fail_pik_killed;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_fail_lose_pikmin::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have lost " +
        i2s(cur) + "/" + i2s(req) +
        " Pikmin. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Whether it has anything to show in the HUD.
 */
bool mission_fail_lose_pikmin::has_hud_content() const {
    return true;
}


/* ----------------------------------------------------------------------------
 * Checks if its conditions have been met to end the mission as a fail.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_fail_lose_pikmin::is_met(
    gameplay_state* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_pause_menu::get_cur_amount(
    gameplay_state* gameplay
) const {
    return 0;
}


/* ----------------------------------------------------------------------------
 * Explains why the player lost, with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_pause_menu::get_end_reason(
    mission_data* mission
) const {
    return "Ended from pause menu...";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_fail_pause_menu::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
string mission_fail_pause_menu::get_hud_label(
    gameplay_state* gameplay
) const {
    return "";
}


/* ----------------------------------------------------------------------------
 * Returns the condition's name.
 */
string mission_fail_pause_menu::get_name() const {
    return "End from pause menu";
}


/* ----------------------------------------------------------------------------
 * A description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_pause_menu::get_player_description(
    mission_data* mission
) const {
    return "End from the pause menu.";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_pause_menu::get_req_amount(
    gameplay_state* gameplay
) const {
    return 0;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_fail_pause_menu::get_status(
    const int cur, const int req, const float percentage
) const {
    return "";
}


/* ----------------------------------------------------------------------------
 * Whether it has anything to show in the HUD.
 */
bool mission_fail_pause_menu::has_hud_content() const {
    return false;
}


/* ----------------------------------------------------------------------------
 * Checks if its conditions have been met to end the mission as a fail.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_fail_pause_menu::is_met(
    gameplay_state* gameplay
) const {
    //The pause menu "end mission" logic is responsible for this one.
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_take_damage::get_cur_amount(
    gameplay_state* gameplay
) const {
    return 0;
}


/* ----------------------------------------------------------------------------
 * Explains why the player lost, with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_take_damage::get_end_reason(
    mission_data* mission
) const {
    return "A leader took damage...";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_fail_take_damage::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->last_hurt_leader_pos.x != LARGE_FLOAT) {
        *final_cam_pos = gameplay->last_hurt_leader_pos;
        *final_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
string mission_fail_take_damage::get_hud_label(
    gameplay_state* gameplay
) const {
    return "";
}


/* ----------------------------------------------------------------------------
 * Returns the condition's name.
 */
string mission_fail_take_damage::get_name() const {
    return "Take damage";
}


/* ----------------------------------------------------------------------------
 * A description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_take_damage::get_player_description(
    mission_data* mission
) const {
    return "A leader takes damage.";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_take_damage::get_req_amount(
    gameplay_state* gameplay
) const {
    return 0;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_fail_take_damage::get_status(
    const int cur, const int req, const float percentage
) const {
    return "";
}


/* ----------------------------------------------------------------------------
 * Whether it has anything to show in the HUD.
 */
bool mission_fail_take_damage::has_hud_content() const {
    return false;
}


/* ----------------------------------------------------------------------------
 * Checks if its conditions have been met to end the mission as a fail.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_fail_take_damage::is_met(
    gameplay_state* gameplay
) const {
    for(size_t l = 0; l < gameplay->mobs.leaders.size(); ++l) {
        if(
            gameplay->mobs.leaders[l]->health <
            gameplay->mobs.leaders[l]->max_health
        ) {
            return true;
        }
    }
    if(gameplay->mobs.leaders.size() < gameplay->starting_nr_of_leaders) {
        //If one of them vanished, they got forcefully KO'd, which...
        //really should count as taking damage.
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_time_limit::get_cur_amount(
    gameplay_state* gameplay
) const {
    return gameplay->gameplay_time_passed;
}


/* ----------------------------------------------------------------------------
 * Explains why the player lost, with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_time_limit::get_end_reason(
    mission_data* mission
) const {
    return
        "Took " +
        time_to_str2(
            mission->fail_time_limit, "m", "s"
        ) +
        "...";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_fail_time_limit::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
string mission_fail_time_limit::get_hud_label(
    gameplay_state* gameplay
) const {
    return
        gameplay->after_hours ?
        "(After hours)" :
        "Time";
}


/* ----------------------------------------------------------------------------
 * Returns the condition's name.
 */
string mission_fail_time_limit::get_name() const {
    return "Reach the time limit";
}


/* ----------------------------------------------------------------------------
 * A description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_time_limit::get_player_description(
    mission_data* mission
) const {
    return
        "Run out of time. Time limit: " +
        time_to_str2(
            mission->fail_time_limit, "m", "s"
        ) + ".";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_time_limit::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) game.cur_area_data.mission.fail_time_limit;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_fail_time_limit::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        time_to_str2(cur, "m", "s") +
        " have passed so far. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Whether it has anything to show in the HUD.
 */
bool mission_fail_time_limit::has_hud_content() const {
    return true;
}


/* ----------------------------------------------------------------------------
 * Checks if its conditions have been met to end the mission as a fail.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_fail_time_limit::is_met(
    gameplay_state* gameplay
) const {
    if(gameplay->after_hours) return false;
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_too_few_pikmin::get_cur_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->get_amount_of_total_pikmin();
}


/* ----------------------------------------------------------------------------
 * Explains why the player lost, with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_too_few_pikmin::get_end_reason(
    mission_data* mission
) const {
    return
        "Reached <=" +
        i2s(mission->fail_too_few_pik_amount) +
        " Pikmin...";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_fail_too_few_pikmin::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->last_pikmin_death_pos.x != LARGE_FLOAT) {
        *final_cam_pos = gameplay->last_pikmin_death_pos;
        *final_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
string mission_fail_too_few_pikmin::get_hud_label(
    gameplay_state* gameplay
) const {
    return "Pikmin";
}


/* ----------------------------------------------------------------------------
 * Returns the condition's name.
 */
string mission_fail_too_few_pikmin::get_name() const {
    return "Reach too few Pikmin";
}


/* ----------------------------------------------------------------------------
 * A description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_too_few_pikmin::get_player_description(
    mission_data* mission
) const {
    return
        "Reach " + i2s(mission->fail_too_few_pik_amount) + " Pikmin or fewer.";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_too_few_pikmin::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) game.cur_area_data.mission.fail_too_few_pik_amount;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_fail_too_few_pikmin::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have " +
        i2s(cur) + "/" + i2s(req) +
        " Pikmin.";
}


/* ----------------------------------------------------------------------------
 * Whether it has anything to show in the HUD.
 */
bool mission_fail_too_few_pikmin::has_hud_content() const {
    return true;
}


/* ----------------------------------------------------------------------------
 * Checks if its conditions have been met to end the mission as a fail.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_fail_too_few_pikmin::is_met(
    gameplay_state* gameplay
) const {
    return
        get_cur_amount(gameplay) <=
        get_req_amount(gameplay);
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_too_many_pikmin::get_cur_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->get_amount_of_total_pikmin();
}


/* ----------------------------------------------------------------------------
 * Explains why the player lost, with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_too_many_pikmin::get_end_reason(
    mission_data* mission
) const {
    return
        "Reached >=" +
        i2s(mission->fail_too_many_pik_amount) +
        " Pikmin...";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_fail_too_many_pikmin::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->last_pikmin_born_pos.x != LARGE_FLOAT) {
        *final_cam_pos = gameplay->last_pikmin_born_pos;
        *final_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
string mission_fail_too_many_pikmin::get_hud_label(
    gameplay_state* gameplay
) const {
    return "Pikmin";
}


/* ----------------------------------------------------------------------------
 * Returns the condition's name.
 */
string mission_fail_too_many_pikmin::get_name() const {
    return "Reach too many Pikmin";
}


/* ----------------------------------------------------------------------------
 * A description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_fail_too_many_pikmin::get_player_description(
    mission_data* mission
) const {
    return
        "Reach " + i2s(mission->fail_too_many_pik_amount) + " Pikmin or more.";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the condition needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_fail_too_many_pikmin::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) game.cur_area_data.mission.fail_too_many_pik_amount;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_fail_too_many_pikmin::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have " +
        i2s(cur) + "/" + i2s(req) +
        " Pikmin. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Whether it has anything to show in the HUD.
 */
bool mission_fail_too_many_pikmin::has_hud_content() const {
    return true;
}


/* ----------------------------------------------------------------------------
 * Checks if its conditions have been met to end the mission as a fail.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_fail_too_many_pikmin::is_met(
    gameplay_state* gameplay
) const {
    return
        get_cur_amount(gameplay) >=
        get_req_amount(gameplay);
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_battle_enemies::get_cur_amount(
    gameplay_state* gameplay
) const {
    return
        (int) gameplay->mission_required_mob_amount -
        (int) gameplay->mission_remaining_mob_ids.size();
}


/* ----------------------------------------------------------------------------
 * Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_battle_enemies::get_end_reason(
    mission_data* mission
) const {
    if(mission->goal_all_mobs) {
        return
            "Defeated all enemies!";
    } else {
        return
            "Defeated the " +
            nr_and_plural(
                mission->goal_mob_idxs.size(),
                "enemy",
                "enemies"
            ) +
            "!";
    }
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_goal_battle_enemies::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->last_enemy_killed_pos.x != LARGE_FLOAT) {
        *final_cam_pos = gameplay->last_enemy_killed_pos;
        *final_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 */
string mission_goal_battle_enemies::get_hud_label() const {
    return "Enemies";
}


/* ----------------------------------------------------------------------------
 * Returns the goal's name.
 */
string mission_goal_battle_enemies::get_name() const {
    return "Battle enemies";
}


/* ----------------------------------------------------------------------------
 * Returns a description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_battle_enemies::get_player_description(
    mission_data* mission
) const {
    if(mission->goal_all_mobs) {
        return
            "Defeat all enemies.";
    } else {
        return
            "Defeat the specified enemies (" +
            i2s(mission->goal_mob_idxs.size()) +
            ").";
    }
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_battle_enemies::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->mission_required_mob_amount;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_goal_battle_enemies::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have killed " + i2s(cur) + "/" + i2s(req) +
        " enemies. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Returns whether or not the mission goal's has been met.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_goal_battle_enemies::is_met(
    gameplay_state* gameplay
) const {
    return gameplay->mission_remaining_mob_ids.empty();
}


/* ----------------------------------------------------------------------------
 * Returns whether a given mob is applicable to this goal's required mobs.
 * type:
 *   Type of the mob.
 */
bool mission_goal_battle_enemies::is_mob_applicable(
    mob_type* type
) const {
    return type->category->id == MOB_CATEGORY_ENEMIES;
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_collect_treasures::get_cur_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->goal_treasures_collected;
}


/* ----------------------------------------------------------------------------
 * Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_collect_treasures::get_end_reason(
    mission_data* mission
) const {
    if(mission->goal_all_mobs) {
        return
            "Collected all treasures!";
    } else {
        return
            "Collected the treasures!";
    }
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_goal_collect_treasures::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->last_ship_that_got_treasure_pos.x != LARGE_FLOAT) {
        *final_cam_pos = gameplay->last_ship_that_got_treasure_pos;
        *final_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 */
string mission_goal_collect_treasures::get_hud_label() const {
    return "Treasures";
}


/* ----------------------------------------------------------------------------
 * Returns the goal's name.
 */
string mission_goal_collect_treasures::get_name() const {
    return "Collect treasures";
}


/* ----------------------------------------------------------------------------
 * Returns a description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_collect_treasures::get_player_description(
    mission_data* mission
) const {
    if(mission->goal_all_mobs) {
        return
            "Collect all treasures.";
    } else {
        return
            "Collect the specified treasures (" +
            i2s(mission->goal_mob_idxs.size()) +
            " sources).";
    }
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_collect_treasures::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->goal_treasures_total;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_goal_collect_treasures::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have collected " + i2s(cur) + "/" + i2s(req) +
        " treasures. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Returns whether or not the mission goal's has been met.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_goal_collect_treasures::is_met(
    gameplay_state* gameplay
) const {
    return
        gameplay->goal_treasures_collected >=
        gameplay->goal_treasures_total;
}


/* ----------------------------------------------------------------------------
 * Returns whether a given mob is applicable to this goal's required mobs.
 * type:
 *   Type of the mob.
 */
bool mission_goal_collect_treasures::is_mob_applicable(
    mob_type* type
) const {
    switch(type->category->id) {
    case MOB_CATEGORY_TREASURES: {
        return true;
        break;
    }
    case MOB_CATEGORY_RESOURCES: {
        resource_type* res_type = (resource_type*) type;
        return
            res_type->delivery_result ==
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS;
        break;
    }
    case MOB_CATEGORY_PILES: {
        pile_type* pil_type = (pile_type*) type;
        return
            pil_type->contents->delivery_result ==
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS;
        break;
    }
    default: {
        return false;
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_end_manually::get_cur_amount(
    gameplay_state* gameplay
) const {
    return 0;
}


/* ----------------------------------------------------------------------------
 * Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_end_manually::get_end_reason(
    mission_data* mission
) const {
    return "Ended successfully!";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_goal_end_manually::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 */
string mission_goal_end_manually::get_hud_label() const {
    return "";
}


/* ----------------------------------------------------------------------------
 * Returns the goal's name.
 */
string mission_goal_end_manually::get_name() const {
    return "End whenever you want";
}


/* ----------------------------------------------------------------------------
 * Returns a description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_end_manually::get_player_description(
    mission_data* mission
) const {
    return "End from the pause menu whenever you want.";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_end_manually::get_req_amount(
    gameplay_state* gameplay
) const {
    return 0;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_goal_end_manually::get_status(
    const int cur, const int req, const float percentage
) const {
    return "";
}


/* ----------------------------------------------------------------------------
 * Returns whether or not the mission goal's has been met.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_goal_end_manually::is_met(
    gameplay_state* gameplay
) const {
    //The pause menu "end mission" logic is responsible for this one.
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns whether a given mob is applicable to this goal's required mobs.
 * type:
 *   Type of the mob.
 */
bool mission_goal_end_manually::is_mob_applicable(
    mob_type* type
) const {
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_get_to_exit::get_cur_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->cur_leaders_in_mission_exit;
}


/* ----------------------------------------------------------------------------
 * Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_get_to_exit::get_end_reason(
    mission_data* mission
) const {
    return "Got to the exit!";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_goal_get_to_exit::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->mission_remaining_mob_ids.empty()) {
        return false;
    }
    point avg_pos;
    for(size_t leader_id : gameplay->mission_remaining_mob_ids) {
        mob* leader_ptr = NULL;
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
            mob* m_ptr = game.states.gameplay->mobs.all[m];
            if(m_ptr->id == leader_id) {
                leader_ptr = m_ptr;
                break;
            }
        }
        avg_pos += leader_ptr->pos;
    }
    avg_pos.x /= gameplay->mission_remaining_mob_ids.size();
    avg_pos.y /= gameplay->mission_remaining_mob_ids.size();
    *final_cam_pos = avg_pos;
    return true;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 */
string mission_goal_get_to_exit::get_hud_label() const {
    return "In exit";
}


/* ----------------------------------------------------------------------------
 * Returns the goal's name.
 */
string mission_goal_get_to_exit::get_name() const {
    return "Get to the exit";
}


/* ----------------------------------------------------------------------------
 * Returns a description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_get_to_exit::get_player_description(
    mission_data* mission
) const {
    if(mission->goal_all_mobs) {
        return
            "Get all leaders to the exit.";
    } else {
        return
            "Get the specified leaders (" +
            i2s(mission->goal_mob_idxs.size()) +
            ") to the exit.";
    }
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_get_to_exit::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->mission_required_mob_amount;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_goal_get_to_exit::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have " + i2s(cur) + "/" + i2s(req) +
        " leaders in the exit. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Returns whether or not the mission goal's has been met.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_goal_get_to_exit::is_met(
    gameplay_state* gameplay
) const {
    return
        get_cur_amount(gameplay) >=
        get_req_amount(gameplay);
}


/* ----------------------------------------------------------------------------
 * Returns whether a given mob is applicable to this goal's required mobs.
 * type:
 *   Type of the mob.
 */
bool mission_goal_get_to_exit::is_mob_applicable(
    mob_type* type
) const {
    return type->category->id == MOB_CATEGORY_LEADERS;
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_grow_pikmin::get_cur_amount(
    gameplay_state* gameplay
) const {
    return (int) gameplay->get_amount_of_total_pikmin();
}


/* ----------------------------------------------------------------------------
 * Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_grow_pikmin::get_end_reason(
    mission_data* mission
) const {
    return
        "Reached " +
        i2s(mission->goal_amount) +
        " Pikmin!";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_goal_grow_pikmin::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    if(gameplay->last_pikmin_born_pos.x != LARGE_FLOAT) {
        *final_cam_pos = gameplay->last_pikmin_born_pos;
        *final_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 */
string mission_goal_grow_pikmin::get_hud_label() const {
    return "Pikmin";
}


/* ----------------------------------------------------------------------------
 * Returns the goal's name.
 */
string mission_goal_grow_pikmin::get_name() const {
    return "Grow Pikmin";
}


/* ----------------------------------------------------------------------------
 * Returns a description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_grow_pikmin::get_player_description(
    mission_data* mission
) const {
    return "Reach a total of " + i2s(mission->goal_amount) + " Pikmin.";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_grow_pikmin::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) game.cur_area_data.mission.goal_amount;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_goal_grow_pikmin::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have " + i2s(cur) + "/" + i2s(req) +
        " Pikmin. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Returns whether or not the mission goal's has been met.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_goal_grow_pikmin::is_met(
    gameplay_state* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/* ----------------------------------------------------------------------------
 * Returns whether a given mob is applicable to this goal's required mobs.
 * type:
 *   Type of the mob.
 */
bool mission_goal_grow_pikmin::is_mob_applicable(
    mob_type* type
) const {
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns the player's current amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_timed_survival::get_cur_amount(
    gameplay_state* gameplay
) const {
    return gameplay->gameplay_time_passed;
}


/* ----------------------------------------------------------------------------
 * Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_timed_survival::get_end_reason(
    mission_data* mission
) const {
    return
        "Survived for " +
        time_to_str2(
            mission->goal_amount, "m", "s"
        ) +
        "!";
}


/* ----------------------------------------------------------------------------
 * Returns where the camera should go to to zoom on the mission end reason.
 * Returns true if the camera should zoom somewhere, false if there's
 * nothing to do.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * final_cam_pos:
 *   The final camera position is returned here.
 * final_cam_zoom:
 *   The final camera zoom is returned here.
 */
bool mission_goal_timed_survival::get_end_zoom_data(
    gameplay_state* gameplay, point* final_cam_pos, float* final_cam_zoom
) const {
    return false;
}


/* ----------------------------------------------------------------------------
 * HUD label for the player's current amount.
 */
string mission_goal_timed_survival::get_hud_label() const {
    return "Time";
}


/* ----------------------------------------------------------------------------
 * Returns the goal's name.
 */
string mission_goal_timed_survival::get_name() const {
    return "Survive";
}


/* ----------------------------------------------------------------------------
 * Returns a description for the player, fed from the mission data.
 * mission:
 *   Mission data object to get info from.
 */
string mission_goal_timed_survival::get_player_description(
    mission_data* mission
) const {
    return
        "Survive for " +
        time_to_str2(
            mission->goal_amount, "m", "s"
        ) + ".";
}


/* ----------------------------------------------------------------------------
 * Returns the player's required amount for whatever the mission needs.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
int mission_goal_timed_survival::get_req_amount(
    gameplay_state* gameplay
) const {
    return (int) game.cur_area_data.mission.goal_amount;
}


/* ----------------------------------------------------------------------------
 * Status for the pause menu.
 * cur:
 *   Current amount.
 * req:
 *   Required amount.
 * percentage:
 *   Percentage cleared.
 */
string mission_goal_timed_survival::get_status(
    const int cur, const int req, const float percentage
) const {
    return
        "You have survived for " +
        time_to_str2(cur, "m", "s") +
        " so far. (" + i2s(percentage) + "%)";
}


/* ----------------------------------------------------------------------------
 * Returns whether or not the mission goal's has been met.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 */
bool mission_goal_timed_survival::is_met(
    gameplay_state* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/* ----------------------------------------------------------------------------
 * Returns whether a given mob is applicable to this goal's required mobs.
 * type:
 *   Type of the mob.
 */
bool mission_goal_timed_survival::is_mob_applicable(
    mob_type* type
) const {
    return false;
}


/* ----------------------------------------------------------------------------
 * Constructs a new mission record.
 */
mission_record::mission_record() :
    clear(false),
    score(0) {
    
}


/* ----------------------------------------------------------------------------
 * Returns whether or not this record is a platinum medal.
 * mission:
 *   Mission data to get info from.
 */
bool mission_record::is_platinum(const mission_data &mission) {
    switch(mission.grading_mode) {
    case MISSION_GRADING_POINTS: {
        return score >= mission.platinum_req;
    } case MISSION_GRADING_GOAL: {
        return clear;
    } case MISSION_GRADING_PARTICIPATION: {
        return !date.empty();
    }
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's point multiplier.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_enemy_points::get_multiplier(
    mission_data* mission
) const {
    return mission->points_per_enemy_point;
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's name.
 */
string mission_score_criterion_enemy_points::get_name() const {
    return "Enemy points";
}


/* ----------------------------------------------------------------------------
 * Returns the player's score for this criterion.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_enemy_points::get_score(
    gameplay_state* gameplay, mission_data* mission
) const {
    return
        (int)
        gameplay->enemy_points_collected *
        get_multiplier(mission);
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's point multiplier.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_pikmin_born::get_multiplier(
    mission_data* mission
) const {
    return mission->points_per_pikmin_born;
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's name.
 */
string mission_score_criterion_pikmin_born::get_name() const {
    return "Pikmin born";
}


/* ----------------------------------------------------------------------------
 * Returns the player's score for this criterion.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_pikmin_born::get_score(
    gameplay_state* gameplay, mission_data* mission
) const {
    return
        (int)
        gameplay->pikmin_born *
        get_multiplier(mission);
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's point multiplier.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_pikmin_death::get_multiplier(
    mission_data* mission
) const {
    return mission->points_per_pikmin_death;
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's name.
 */
string mission_score_criterion_pikmin_death::get_name() const {
    return "Pikmin deaths";
}


/* ----------------------------------------------------------------------------
 * Returns the player's score for this criterion.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_pikmin_death::get_score(
    gameplay_state* gameplay, mission_data* mission
) const {
    return
        (int)
        gameplay->pikmin_deaths *
        get_multiplier(mission);
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's point multiplier.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_sec_left::get_multiplier(
    mission_data* mission
) const {
    if(
        has_flag(
            mission->fail_conditions,
            get_index_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        return mission->points_per_sec_left;
    } else {
        return 0;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's name.
 */
string mission_score_criterion_sec_left::get_name() const {
    return "Seconds left";
}


/* ----------------------------------------------------------------------------
 * Returns the player's score for this criterion.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_sec_left::get_score(
    gameplay_state* gameplay, mission_data* mission
) const {
    return
        (mission->fail_time_limit - floor(gameplay->gameplay_time_passed)) *
        get_multiplier(mission);
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's point multiplier.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_sec_passed::get_multiplier(
    mission_data* mission
) const {
    return mission->points_per_sec_passed;
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's name.
 */
string mission_score_criterion_sec_passed::get_name() const {
    return "Seconds passed";
}


/* ----------------------------------------------------------------------------
 * Returns the player's score for this criterion.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_sec_passed::get_score(
    gameplay_state* gameplay, mission_data* mission
) const {
    return
        floor(gameplay->gameplay_time_passed) *
        get_multiplier(mission);
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's point multiplier.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_treasure_points::get_multiplier(
    mission_data* mission
) const {
    return mission->points_per_treasure_point;
}


/* ----------------------------------------------------------------------------
 * Returns the mission score criterion's name.
 */
string mission_score_criterion_treasure_points::get_name() const {
    return "Treasure points";
}


/* ----------------------------------------------------------------------------
 * Returns the player's score for this criterion.
 * gameplay:
 *   Pointer to the gameplay state to get info from.
 * mission:
 *   Mission data to get info from.
 */
int mission_score_criterion_treasure_points::get_score(
    gameplay_state* gameplay, mission_data* mission
) const {
    return
        (int)
        gameplay->treasure_points_collected *
        get_multiplier(mission);
}
