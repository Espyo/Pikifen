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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../game_state/area_editor/editor.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


namespace MISSION {

//Default mission bronze medal point requirement.
const int DEF_MEDAL_REQ_BRONZE = 1000;

//Default mission gold medal point requirement.
const int DEF_MEDAL_REQ_GOLD = 3000;

//Default mission platinum medal point requirement.
const int DEF_MEDAL_REQ_PLATINUM = 4000;

//Default mission silver medal point requirement.
const int DEF_MEDAL_REQ_SILVER = 2000;

//Default mission time limit duration, in seconds.
const size_t DEF_TIME_LIMIT = 60;

//Mission exit region minimum size.
const float EXIT_MIN_SIZE = 32.0f;

}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailKillEnemies::get_cur_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->enemy_deaths;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailKillEnemies::get_end_reason(
    MissionData* mission
) const {
    return
        "Killed " +
        amount_str((int) mission->fail_enemies_killed, "enemy", "enemies") +
        "...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailKillEnemies::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->last_enemy_killed_pos.x != LARGE_FLOAT) {
        *out_cam_pos = gameplay->last_enemy_killed_pos;
        *out_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailKillEnemies::get_hud_label(
    GameplayState* gameplay
) const {
    return "Enemies";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailKillEnemies::get_name() const {
    return "Kill enemies";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailKillEnemies::get_player_description(
    MissionData* mission
) const {
    return
        "Kill " +
        amount_str(
            (int) mission->fail_enemies_killed, "enemy", "enemies"
        ) +
        " or more.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailKillEnemies::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) game.cur_area_data->mission.fail_enemies_killed;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailKillEnemies::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have killed " +
        i2s(cur) + "/" + i2s(req) +
        " enemies. (" + i2s(percentage) + "%)";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailKillEnemies::has_hud_content() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailKillEnemies::is_met(
    GameplayState* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailLoseLeaders::get_cur_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->leaders_kod;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailLoseLeaders::get_end_reason(
    MissionData* mission
) const {
    return
        "Lost " +
        amount_str((int) mission->fail_leaders_kod, "leader") +
        "...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailLoseLeaders::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->last_hurt_leader_pos.x != LARGE_FLOAT) {
        *out_cam_pos = gameplay->last_hurt_leader_pos;
        *out_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailLoseLeaders::get_hud_label(
    GameplayState* gameplay
) const {
    return "Leaders lost";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailLoseLeaders::get_name() const {
    return "Lose leaders";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailLoseLeaders::get_player_description(
    MissionData* mission
) const {
    return
        "Lose " +
        amount_str((int) mission->fail_leaders_kod, "leader") +
        " or more.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailLoseLeaders::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) game.cur_area_data->mission.fail_leaders_kod;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailLoseLeaders::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have lost " +
        i2s(cur) + "/" + i2s(req) +
        " leaders. (" + i2s(percentage) + "%)";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailLoseLeaders::has_hud_content() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailLoseLeaders::is_met(
    GameplayState* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailLosePikmin::get_cur_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->pikmin_deaths;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailLosePikmin::get_end_reason(
    MissionData* mission
) const {
    return
        "Lost " +
        i2s(mission->fail_pik_killed) +
        " Pikmin...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailLosePikmin::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->last_pikmin_death_pos.x != LARGE_FLOAT) {
        *out_cam_pos = gameplay->last_pikmin_death_pos;
        *out_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailLosePikmin::get_hud_label(
    GameplayState* gameplay
) const {
    return "Pikmin lost";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailLosePikmin::get_name() const {
    return "Lose Pikmin";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailLosePikmin::get_player_description(
    MissionData* mission
) const {
    return
        "Lose " + i2s(mission->fail_pik_killed) + " Pikmin or more.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailLosePikmin::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) game.cur_area_data->mission.fail_pik_killed;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailLosePikmin::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have lost " +
        i2s(cur) + "/" + i2s(req) +
        " Pikmin. (" + i2s(percentage) + "%)";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailLosePikmin::has_hud_content() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailLosePikmin::is_met(
    GameplayState* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailPauseMenu::get_cur_amount(
    GameplayState* gameplay
) const {
    return 0;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailPauseMenu::get_end_reason(
    MissionData* mission
) const {
    return "Ended from pause menu...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailPauseMenu::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailPauseMenu::get_hud_label(
    GameplayState* gameplay
) const {
    return "";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailPauseMenu::get_name() const {
    return "End from pause menu";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailPauseMenu::get_player_description(
    MissionData* mission
) const {
    return "End from the pause menu.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailPauseMenu::get_req_amount(
    GameplayState* gameplay
) const {
    return 0;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailPauseMenu::get_status(
    int cur, int req, float percentage
) const {
    return "";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailPauseMenu::has_hud_content() const {
    return false;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailPauseMenu::is_met(
    GameplayState* gameplay
) const {
    //The pause menu "end mission" logic is responsible for this one.
    return false;
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTakeDamage::get_cur_amount(
    GameplayState* gameplay
) const {
    return 0;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailTakeDamage::get_end_reason(
    MissionData* mission
) const {
    return "A leader took damage...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailTakeDamage::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->last_hurt_leader_pos.x != LARGE_FLOAT) {
        *out_cam_pos = gameplay->last_hurt_leader_pos;
        *out_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailTakeDamage::get_hud_label(
    GameplayState* gameplay
) const {
    return "";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailTakeDamage::get_name() const {
    return "Take damage";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailTakeDamage::get_player_description(
    MissionData* mission
) const {
    return "A leader takes damage.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTakeDamage::get_req_amount(
    GameplayState* gameplay
) const {
    return 0;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailTakeDamage::get_status(
    int cur, int req, float percentage
) const {
    return "";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailTakeDamage::has_hud_content() const {
    return false;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailTakeDamage::is_met(
    GameplayState* gameplay
) const {
    for(size_t l = 0; l < gameplay->mobs.leaders.size(); l++) {
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


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTimeLimit::get_cur_amount(
    GameplayState* gameplay
) const {
    return gameplay->gameplay_time_passed;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailTimeLimit::get_end_reason(
    MissionData* mission
) const {
    return
        "Took " +
        time_to_str2(
            mission->fail_time_limit, "m", "s"
        ) +
        "...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailTimeLimit::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailTimeLimit::get_hud_label(
    GameplayState* gameplay
) const {
    return
        gameplay->after_hours ?
        "(After hours)" :
        "Time";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailTimeLimit::get_name() const {
    return "Reach the time limit";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailTimeLimit::get_player_description(
    MissionData* mission
) const {
    return
        "Run out of time. Time limit: " +
        time_to_str2(
            mission->fail_time_limit, "m", "s"
        ) + ".";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTimeLimit::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) game.cur_area_data->mission.fail_time_limit;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailTimeLimit::get_status(
    int cur, int req, float percentage
) const {
    return
        time_to_str2(cur, "m", "s") +
        " have passed so far. (" + i2s(percentage) + "%)";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailTimeLimit::has_hud_content() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailTimeLimit::is_met(
    GameplayState* gameplay
) const {
    if(gameplay->after_hours) return false;
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTooFewPikmin::get_cur_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->get_amount_of_total_pikmin();
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailTooFewPikmin::get_end_reason(
    MissionData* mission
) const {
    return
        "Reached <=" +
        i2s(mission->fail_too_few_pik_amount) +
        " Pikmin...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailTooFewPikmin::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->last_pikmin_death_pos.x != LARGE_FLOAT) {
        *out_cam_pos = gameplay->last_pikmin_death_pos;
        *out_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailTooFewPikmin::get_hud_label(
    GameplayState* gameplay
) const {
    return "Pikmin";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailTooFewPikmin::get_name() const {
    return "Reach too few Pikmin";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailTooFewPikmin::get_player_description(
    MissionData* mission
) const {
    return
        "Reach " + i2s(mission->fail_too_few_pik_amount) + " Pikmin or fewer.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTooFewPikmin::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) game.cur_area_data->mission.fail_too_few_pik_amount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailTooFewPikmin::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have " +
        i2s(cur) + "/" + i2s(req) +
        " Pikmin.";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailTooFewPikmin::has_hud_content() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailTooFewPikmin::is_met(
    GameplayState* gameplay
) const {
    return
        get_cur_amount(gameplay) <=
        get_req_amount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTooManyPikmin::get_cur_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->get_amount_of_total_pikmin();
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailTooManyPikmin::get_end_reason(
    MissionData* mission
) const {
    return
        "Reached >=" +
        i2s(mission->fail_too_many_pik_amount) +
        " Pikmin...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailTooManyPikmin::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->last_pikmin_born_pos.x != LARGE_FLOAT) {
        *out_cam_pos = gameplay->last_pikmin_born_pos;
        *out_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailTooManyPikmin::get_hud_label(
    GameplayState* gameplay
) const {
    return "Pikmin";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailTooManyPikmin::get_name() const {
    return "Reach too many Pikmin";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailTooManyPikmin::get_player_description(
    MissionData* mission
) const {
    return
        "Reach " + i2s(mission->fail_too_many_pik_amount) + " Pikmin or more.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTooManyPikmin::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) game.cur_area_data->mission.fail_too_many_pik_amount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailTooManyPikmin::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have " +
        i2s(cur) + "/" + i2s(req) +
        " Pikmin. (" + i2s(percentage) + "%)";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailTooManyPikmin::has_hud_content() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailTooManyPikmin::is_met(
    GameplayState* gameplay
) const {
    return
        get_cur_amount(gameplay) >=
        get_req_amount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalBattleEnemies::get_cur_amount(
    GameplayState* gameplay
) const {
    return
        (int) gameplay->mission_required_mob_amount -
        (int) gameplay->mission_remaining_mob_ids.size();
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalBattleEnemies::get_end_reason(
    MissionData* mission
) const {
    if(mission->goal_all_mobs) {
        return
            "Defeated all enemies!";
    } else {
        return
            "Defeated the " +
            amount_str(
                (int) mission->goal_mob_idxs.size(),
                "enemy",
                "enemies"
            ) +
            "!";
    }
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalBattleEnemies::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->last_enemy_killed_pos.x != LARGE_FLOAT) {
        *out_cam_pos = gameplay->last_enemy_killed_pos;
        *out_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalBattleEnemies::get_hud_label() const {
    return "Enemies";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalBattleEnemies::get_name() const {
    return "Battle enemies";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalBattleEnemies::get_player_description(
    MissionData* mission
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


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalBattleEnemies::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->mission_required_mob_amount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalBattleEnemies::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have killed " + i2s(cur) + "/" + i2s(req) +
        " enemies. (" + i2s(percentage) + "%)";
}


/**
 * @brief Returns whether or not the mission goal's has been met.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionGoalBattleEnemies::is_met(
    GameplayState* gameplay
) const {
    return gameplay->mission_remaining_mob_ids.empty();
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalBattleEnemies::is_mob_applicable(
    MobType* type
) const {
    return type->category->id == MOB_CATEGORY_ENEMIES;
}


/**
 * @brief Returns the player's current amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalCollectTreasures::get_cur_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->goal_treasures_collected;
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalCollectTreasures::get_end_reason(
    MissionData* mission
) const {
    if(mission->goal_all_mobs) {
        return
            "Collected all treasures!";
    } else {
        return
            "Collected the treasures!";
    }
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalCollectTreasures::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->last_ship_that_got_treasure_pos.x != LARGE_FLOAT) {
        *out_cam_pos = gameplay->last_ship_that_got_treasure_pos;
        *out_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalCollectTreasures::get_hud_label() const {
    return "Treasures";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalCollectTreasures::get_name() const {
    return "Collect treasures";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalCollectTreasures::get_player_description(
    MissionData* mission
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


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalCollectTreasures::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->goal_treasures_total;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalCollectTreasures::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have collected " + i2s(cur) + "/" + i2s(req) +
        " treasures. (" + i2s(percentage) + "%)";
}


/**
 * @brief Returns whether or not the mission goal's has been met.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionGoalCollectTreasures::is_met(
    GameplayState* gameplay
) const {
    return
        gameplay->goal_treasures_collected >=
        gameplay->goal_treasures_total;
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalCollectTreasures::is_mob_applicable(
    MobType* type
) const {
    switch(type->category->id) {
    case MOB_CATEGORY_TREASURES: {
        return true;
        break;
    }
    case MOB_CATEGORY_RESOURCES: {
        ResourceType* res_type = (ResourceType*) type;
        return
            res_type->delivery_result ==
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS;
        break;
    }
    case MOB_CATEGORY_PILES: {
        PileType* pil_type = (PileType*) type;
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


/**
 * @brief Returns the player's current amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalEndManually::get_cur_amount(
    GameplayState* gameplay
) const {
    return 0;
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalEndManually::get_end_reason(
    MissionData* mission
) const {
    return "Ended successfully!";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalEndManually::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalEndManually::get_hud_label() const {
    return "";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalEndManually::get_name() const {
    return "End whenever you want";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalEndManually::get_player_description(
    MissionData* mission
) const {
    return "End from the pause menu whenever you want.";
}


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalEndManually::get_req_amount(
    GameplayState* gameplay
) const {
    return 0;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalEndManually::get_status(
    int cur, int req, float percentage
) const {
    return "";
}


/**
 * @brief Returns whether or not the mission goal's has been met.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionGoalEndManually::is_met(
    GameplayState* gameplay
) const {
    //The pause menu "end mission" logic is responsible for this one.
    return false;
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalEndManually::is_mob_applicable(
    MobType* type
) const {
    return false;
}


/**
 * @brief Returns the player's current amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalGetToExit::get_cur_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->cur_leaders_in_mission_exit;
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalGetToExit::get_end_reason(
    MissionData* mission
) const {
    return "Got to the exit!";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalGetToExit::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->mission_remaining_mob_ids.empty()) {
        return false;
    }
    Point avg_pos;
    for(size_t leader_id : gameplay->mission_remaining_mob_ids) {
        Mob* leader_ptr = nullptr;
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            Mob* m_ptr = game.states.gameplay->mobs.all[m];
            if(m_ptr->id == leader_id) {
                leader_ptr = m_ptr;
                break;
            }
        }
        if(leader_ptr) avg_pos += leader_ptr->pos;
    }
    avg_pos.x /= gameplay->mission_remaining_mob_ids.size();
    avg_pos.y /= gameplay->mission_remaining_mob_ids.size();
    *out_cam_pos = avg_pos;
    return true;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalGetToExit::get_hud_label() const {
    return "In exit";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalGetToExit::get_name() const {
    return "Get to the exit";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalGetToExit::get_player_description(
    MissionData* mission
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


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalGetToExit::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->mission_required_mob_amount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalGetToExit::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have " + i2s(cur) + "/" + i2s(req) +
        " leaders in the exit. (" + i2s(percentage) + "%)";
}


/**
 * @brief Returns whether or not the mission goal's has been met.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionGoalGetToExit::is_met(
    GameplayState* gameplay
) const {
    return
        get_cur_amount(gameplay) >=
        get_req_amount(gameplay);
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalGetToExit::is_mob_applicable(
    MobType* type
) const {
    return type->category->id == MOB_CATEGORY_LEADERS;
}


/**
 * @brief Returns the player's current amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalGrowPikmin::get_cur_amount(
    GameplayState* gameplay
) const {
    return (int) gameplay->get_amount_of_total_pikmin();
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalGrowPikmin::get_end_reason(
    MissionData* mission
) const {
    return
        "Reached " +
        i2s(mission->goal_amount) +
        " Pikmin!";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalGrowPikmin::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    if(gameplay->last_pikmin_born_pos.x != LARGE_FLOAT) {
        *out_cam_pos = gameplay->last_pikmin_born_pos;
        *out_cam_zoom = game.config.zoom_max_level;
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalGrowPikmin::get_hud_label() const {
    return "Pikmin";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalGrowPikmin::get_name() const {
    return "Grow Pikmin";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalGrowPikmin::get_player_description(
    MissionData* mission
) const {
    return "Reach a total of " + i2s(mission->goal_amount) + " Pikmin.";
}


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalGrowPikmin::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) game.cur_area_data->mission.goal_amount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalGrowPikmin::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have " + i2s(cur) + "/" + i2s(req) +
        " Pikmin. (" + i2s(percentage) + "%)";
}


/**
 * @brief Returns whether or not the mission goal's has been met.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionGoalGrowPikmin::is_met(
    GameplayState* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalGrowPikmin::is_mob_applicable(
    MobType* type
) const {
    return false;
}


/**
 * @brief Returns the player's current amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalTimedSurvival::get_cur_amount(
    GameplayState* gameplay
) const {
    return gameplay->gameplay_time_passed;
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalTimedSurvival::get_end_reason(
    MissionData* mission
) const {
    return
        "Survived for " +
        time_to_str2(
            mission->goal_amount, "m", "s"
        ) +
        "!";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param out_cam_pos The final camera position is returned here.
 * @param out_cam_zoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalTimedSurvival::get_end_zoom_data(
    GameplayState* gameplay, Point* out_cam_pos, float* out_cam_zoom
) const {
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalTimedSurvival::get_hud_label() const {
    return "Time";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalTimedSurvival::get_name() const {
    return "Survive";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalTimedSurvival::get_player_description(
    MissionData* mission
) const {
    return
        "Survive for " +
        time_to_str2(
            mission->goal_amount, "m", "s"
        ) + ".";
}


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalTimedSurvival::get_req_amount(
    GameplayState* gameplay
) const {
    return (int) game.cur_area_data->mission.goal_amount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalTimedSurvival::get_status(
    int cur, int req, float percentage
) const {
    return
        "You have survived for " +
        time_to_str2(cur, "m", "s") +
        " so far. (" + i2s(percentage) + "%)";
}


/**
 * @brief Returns whether or not the mission goal's has been met.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionGoalTimedSurvival::is_met(
    GameplayState* gameplay
) const {
    return get_cur_amount(gameplay) >= get_req_amount(gameplay);
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalTimedSurvival::is_mob_applicable(
    MobType* type
) const {
    return false;
}


/**
 * @brief Returns whether or not this record is a platinum medal.
 *
 * @param mission Mission data to get info from.
 * @return Whether it is platinum.
 */
bool MissionRecord::is_platinum(const MissionData &mission) {
    switch(mission.grading_mode) {
    case MISSION_GRADING_MODE_POINTS: {
        return score >= mission.platinum_req;
    } case MISSION_GRADING_MODE_GOAL: {
        return clear;
    } case MISSION_GRADING_MODE_PARTICIPATION: {
        return !date.empty();
    }
    }
    return false;
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionEnemyPoints::get_multiplier(
    MissionData* mission
) const {
    return mission->points_per_enemy_point;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionEnemyPoints::get_name() const {
    return "Enemy points";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionEnemyPoints::get_score(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (int)
        gameplay->enemy_points_collected *
        get_multiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionPikminBorn::get_multiplier(
    MissionData* mission
) const {
    return mission->points_per_pikmin_born;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionPikminBorn::get_name() const {
    return "Pikmin born";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionPikminBorn::get_score(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (int)
        gameplay->pikmin_born *
        get_multiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionPikminDeath::get_multiplier(
    MissionData* mission
) const {
    return mission->points_per_pikmin_death;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionPikminDeath::get_name() const {
    return "Pikmin deaths";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionPikminDeath::get_score(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (int)
        gameplay->pikmin_deaths *
        get_multiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionSecLeft::get_multiplier(
    MissionData* mission
) const {
    if(
        has_flag(
            mission->fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        return mission->points_per_sec_left;
    } else {
        return 0;
    }
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionSecLeft::get_name() const {
    return "Seconds left";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionSecLeft::get_score(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (mission->fail_time_limit - floor(gameplay->gameplay_time_passed)) *
        get_multiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionSecPassed::get_multiplier(
    MissionData* mission
) const {
    return mission->points_per_sec_passed;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionSecPassed::get_name() const {
    return "Seconds passed";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionSecPassed::get_score(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        floor(gameplay->gameplay_time_passed) *
        get_multiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionTreasurePoints::get_multiplier(
    MissionData* mission
) const {
    return mission->points_per_treasure_point;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTreasurePoints::get_name() const {
    return "Treasure points";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionTreasurePoints::get_score(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (int)
        gameplay->treasure_points_collected *
        get_multiplier(mission);
}
