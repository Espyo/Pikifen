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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
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
int MissionFailDefeatEnemies::getCurAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->enemyDefeats;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailDefeatEnemies::getEndReason(
    MissionData* mission
) const {
    return
        "Defeated " +
        amountStr((int) mission->failEnemiesDefeated, "enemy", "enemies") +
        "...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailDefeatEnemies::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastEnemyDefeatedPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastEnemyDefeatedPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
string MissionFailDefeatEnemies::getHudLabel(
    GameplayState* gameplay
) const {
    return "Enemies";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailDefeatEnemies::getName() const {
    return "Defeat enemies";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailDefeatEnemies::getPlayerDescription(
    MissionData* mission
) const {
    return
        "Defeat " +
        amountStr(
            (int) mission->failEnemiesDefeated, "enemy", "enemies"
        ) +
        " or more.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailDefeatEnemies::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) game.curAreaData->mission.failEnemiesDefeated;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailDefeatEnemies::getStatus(
    int cur, int req, float percentage
) const {
    return
        "You have defeated " +
        i2s(cur) + "/" + i2s(req) +
        " enemies. (" + i2s(percentage) + "%)";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailDefeatEnemies::hasHudContent() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailDefeatEnemies::isMet(
    GameplayState* gameplay
) const {
    return getCurAmount(gameplay) >= getReqAmount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailLoseLeaders::getCurAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->leadersKod;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailLoseLeaders::getEndReason(
    MissionData* mission
) const {
    return
        "Lost " +
        amountStr((int) mission->failLeadersKod, "leader") +
        "...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailLoseLeaders::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastHurtLeaderPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastHurtLeaderPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
string MissionFailLoseLeaders::getHudLabel(
    GameplayState* gameplay
) const {
    return "Leaders lost";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailLoseLeaders::getName() const {
    return "Lose leaders";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailLoseLeaders::getPlayerDescription(
    MissionData* mission
) const {
    return
        "Lose " +
        amountStr((int) mission->failLeadersKod, "leader") +
        " or more.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailLoseLeaders::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) game.curAreaData->mission.failLeadersKod;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailLoseLeaders::getStatus(
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
bool MissionFailLoseLeaders::hasHudContent() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailLoseLeaders::isMet(
    GameplayState* gameplay
) const {
    return getCurAmount(gameplay) >= getReqAmount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailLosePikmin::getCurAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->pikminDeaths;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailLosePikmin::getEndReason(
    MissionData* mission
) const {
    return
        "Lost " +
        i2s(mission->failPikKilled) +
        " Pikmin...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailLosePikmin::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastPikminDeathPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastPikminDeathPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
string MissionFailLosePikmin::getHudLabel(
    GameplayState* gameplay
) const {
    return "Pikmin lost";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailLosePikmin::getName() const {
    return "Lose Pikmin";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailLosePikmin::getPlayerDescription(
    MissionData* mission
) const {
    return
        "Lose " + i2s(mission->failPikKilled) + " Pikmin or more.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailLosePikmin::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) game.curAreaData->mission.failPikKilled;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailLosePikmin::getStatus(
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
bool MissionFailLosePikmin::hasHudContent() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailLosePikmin::isMet(
    GameplayState* gameplay
) const {
    return getCurAmount(gameplay) >= getReqAmount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailPauseMenu::getCurAmount(
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
string MissionFailPauseMenu::getEndReason(
    MissionData* mission
) const {
    return "Ended from pause menu...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailPauseMenu::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailPauseMenu::getHudLabel(
    GameplayState* gameplay
) const {
    return "";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailPauseMenu::getName() const {
    return "End from pause menu";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailPauseMenu::getPlayerDescription(
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
int MissionFailPauseMenu::getReqAmount(
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
string MissionFailPauseMenu::getStatus(
    int cur, int req, float percentage
) const {
    return "";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailPauseMenu::hasHudContent() const {
    return false;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailPauseMenu::isMet(
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
int MissionFailTakeDamage::getCurAmount(
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
string MissionFailTakeDamage::getEndReason(
    MissionData* mission
) const {
    return "A leader took damage...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailTakeDamage::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastHurtLeaderPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastHurtLeaderPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
string MissionFailTakeDamage::getHudLabel(
    GameplayState* gameplay
) const {
    return "";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailTakeDamage::getName() const {
    return "Take damage";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailTakeDamage::getPlayerDescription(
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
int MissionFailTakeDamage::getReqAmount(
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
string MissionFailTakeDamage::getStatus(
    int cur, int req, float percentage
) const {
    return "";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailTakeDamage::hasHudContent() const {
    return false;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailTakeDamage::isMet(
    GameplayState* gameplay
) const {
    for(size_t l = 0; l < gameplay->mobs.leaders.size(); l++) {
        if(
            gameplay->mobs.leaders[l]->health <
            gameplay->mobs.leaders[l]->maxHealth
        ) {
            return true;
        }
    }
    if(gameplay->mobs.leaders.size() < gameplay->startingNrOfLeaders) {
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
int MissionFailTimeLimit::getCurAmount(
    GameplayState* gameplay
) const {
    return gameplay->gameplayTimePassed;
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailTimeLimit::getEndReason(
    MissionData* mission
) const {
    return
        "Took " +
        timeToStr2(
            mission->failTimeLimit, "m", "s"
        ) +
        "...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailTimeLimit::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The label.
 */
string MissionFailTimeLimit::getHudLabel(
    GameplayState* gameplay
) const {
    return
        gameplay->afterHours ?
        "(After hours)" :
        "Time";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailTimeLimit::getName() const {
    return "Reach the time limit";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailTimeLimit::getPlayerDescription(
    MissionData* mission
) const {
    return
        "Run out of time. Time limit: " +
        timeToStr2(
            mission->failTimeLimit, "m", "s"
        ) + ".";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTimeLimit::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) game.curAreaData->mission.failTimeLimit;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailTimeLimit::getStatus(
    int cur, int req, float percentage
) const {
    return
        timeToStr2(cur, "m", "s") +
        " have passed so far. (" + i2s(percentage) + "%)";
}


/**
 * @brief Whether it has anything to show in the HUD.
 *
 * @return Whether it has content.
 */
bool MissionFailTimeLimit::hasHudContent() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailTimeLimit::isMet(
    GameplayState* gameplay
) const {
    if(gameplay->afterHours) return false;
    return getCurAmount(gameplay) >= getReqAmount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTooFewPikmin::getCurAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->getAmountOfTotalPikmin();
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailTooFewPikmin::getEndReason(
    MissionData* mission
) const {
    return
        "Reached <=" +
        i2s(mission->failTooFewPikAmount) +
        " Pikmin...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailTooFewPikmin::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastPikminDeathPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastPikminDeathPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
string MissionFailTooFewPikmin::getHudLabel(
    GameplayState* gameplay
) const {
    return "Pikmin";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailTooFewPikmin::getName() const {
    return "Reach too few Pikmin";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailTooFewPikmin::getPlayerDescription(
    MissionData* mission
) const {
    return
        "Reach " + i2s(mission->failTooFewPikAmount) + " Pikmin or fewer.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTooFewPikmin::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) game.curAreaData->mission.failTooFewPikAmount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailTooFewPikmin::getStatus(
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
bool MissionFailTooFewPikmin::hasHudContent() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailTooFewPikmin::isMet(
    GameplayState* gameplay
) const {
    return
        getCurAmount(gameplay) <=
        getReqAmount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTooManyPikmin::getCurAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->getAmountOfTotalPikmin();
}


/**
 * @brief Explains why the player lost, with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionFailTooManyPikmin::getEndReason(
    MissionData* mission
) const {
    return
        "Reached >=" +
        i2s(mission->failTooManyPikAmount) +
        " Pikmin...";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionFailTooManyPikmin::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastPikminBornPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastPikminBornPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
string MissionFailTooManyPikmin::getHudLabel(
    GameplayState* gameplay
) const {
    return "Pikmin";
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionFailTooManyPikmin::getName() const {
    return "Reach too many Pikmin";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionFailTooManyPikmin::getPlayerDescription(
    MissionData* mission
) const {
    return
        "Reach " + i2s(mission->failTooManyPikAmount) + " Pikmin or more.";
}


/**
 * @brief Returns the player's required amount for whatever the condition needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionFailTooManyPikmin::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) game.curAreaData->mission.failTooManyPikAmount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionFailTooManyPikmin::getStatus(
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
bool MissionFailTooManyPikmin::hasHudContent() const {
    return true;
}


/**
 * @brief Checks if its conditions have been met to end the mission as a fail.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionFailTooManyPikmin::isMet(
    GameplayState* gameplay
) const {
    return
        getCurAmount(gameplay) >=
        getReqAmount(gameplay);
}


/**
 * @brief Returns the player's current amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalBattleEnemies::getCurAmount(
    GameplayState* gameplay
) const {
    return
        (int) gameplay->missionRequiredMobAmount -
        (int) gameplay->missionRemainingMobIds.size();
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalBattleEnemies::getEndReason(
    MissionData* mission
) const {
    if(mission->goalAllMobs) {
        return
            "Defeated all enemies!";
    } else {
        return
            "Defeated the " +
            amountStr(
                (int) mission->goalMobIdxs.size(),
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
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalBattleEnemies::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastEnemyDefeatedPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastEnemyDefeatedPos;
        *outCamZoom = gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalBattleEnemies::getHudLabel() const {
    return "Enemies";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalBattleEnemies::getName() const {
    return "Battle enemies";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalBattleEnemies::getPlayerDescription(
    MissionData* mission
) const {
    if(mission->goalAllMobs) {
        return
            "Defeat all enemies.";
    } else {
        return
            "Defeat the specified enemies (" +
            i2s(mission->goalMobIdxs.size()) +
            ").";
    }
}


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalBattleEnemies::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->missionRequiredMobAmount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalBattleEnemies::getStatus(
    int cur, int req, float percentage
) const {
    return
        "You have defeated " + i2s(cur) + "/" + i2s(req) +
        " enemies. (" + i2s(percentage) + "%)";
}


/**
 * @brief Returns whether or not the mission goal's has been met.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionGoalBattleEnemies::isMet(
    GameplayState* gameplay
) const {
    return gameplay->missionRemainingMobIds.empty();
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalBattleEnemies::isMobApplicable(
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
int MissionGoalCollectTreasures::getCurAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->goalTreasuresCollected;
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalCollectTreasures::getEndReason(
    MissionData* mission
) const {
    if(mission->goalAllMobs) {
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
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalCollectTreasures::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastShipThatGotTreasurePos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastShipThatGotTreasurePos;
        *outCamZoom = gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalCollectTreasures::getHudLabel() const {
    return "Treasures";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalCollectTreasures::getName() const {
    return "Collect treasures";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalCollectTreasures::getPlayerDescription(
    MissionData* mission
) const {
    if(mission->goalAllMobs) {
        return
            "Collect all treasures.";
    } else {
        return
            "Collect the specified treasures (" +
            i2s(mission->goalMobIdxs.size()) +
            " sources).";
    }
}


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalCollectTreasures::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->goalTreasuresTotal;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalCollectTreasures::getStatus(
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
bool MissionGoalCollectTreasures::isMet(
    GameplayState* gameplay
) const {
    return
        gameplay->goalTreasuresCollected >=
        gameplay->goalTreasuresTotal;
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalCollectTreasures::isMobApplicable(
    MobType* type
) const {
    switch(type->category->id) {
    case MOB_CATEGORY_TREASURES: {
        return true;
        break;
    }
    case MOB_CATEGORY_RESOURCES: {
        ResourceType* resType = (ResourceType*) type;
        return
            resType->deliveryResult ==
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS;
        break;
    }
    case MOB_CATEGORY_PILES: {
        PileType* pilType = (PileType*) type;
        return
            pilType->contents->deliveryResult ==
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
int MissionGoalEndManually::getCurAmount(
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
string MissionGoalEndManually::getEndReason(
    MissionData* mission
) const {
    return "Ended successfully!";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalEndManually::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalEndManually::getHudLabel() const {
    return "";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalEndManually::getName() const {
    return "End whenever you want";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalEndManually::getPlayerDescription(
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
int MissionGoalEndManually::getReqAmount(
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
string MissionGoalEndManually::getStatus(
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
bool MissionGoalEndManually::isMet(
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
bool MissionGoalEndManually::isMobApplicable(
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
int MissionGoalGetToExit::getCurAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->curLeadersInMissionExit;
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalGetToExit::getEndReason(
    MissionData* mission
) const {
    return "Got to the exit!";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalGetToExit::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->missionRemainingMobIds.empty()) {
        return false;
    }
    Point avgPos;
    for(size_t leaderId : gameplay->missionRemainingMobIds) {
        Mob* leaderPtr = nullptr;
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            Mob* mPtr = game.states.gameplay->mobs.all[m];
            if(mPtr->id == leaderId) {
                leaderPtr = mPtr;
                break;
            }
        }
        if(leaderPtr) avgPos += leaderPtr->pos;
    }
    avgPos.x /= gameplay->missionRemainingMobIds.size();
    avgPos.y /= gameplay->missionRemainingMobIds.size();
    *outCamPos = avgPos;
    return true;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalGetToExit::getHudLabel() const {
    return "In exit";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalGetToExit::getName() const {
    return "Get to the exit";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalGetToExit::getPlayerDescription(
    MissionData* mission
) const {
    if(mission->goalAllMobs) {
        return
            "Get all leaders to the exit.";
    } else {
        return
            "Get the specified leaders (" +
            i2s(mission->goalMobIdxs.size()) +
            ") to the exit.";
    }
}


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalGetToExit::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->missionRequiredMobAmount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalGetToExit::getStatus(
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
bool MissionGoalGetToExit::isMet(
    GameplayState* gameplay
) const {
    return
        getCurAmount(gameplay) >=
        getReqAmount(gameplay);
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalGetToExit::isMobApplicable(
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
int MissionGoalGrowPikmin::getCurAmount(
    GameplayState* gameplay
) const {
    return (int) gameplay->getAmountOfTotalPikmin();
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalGrowPikmin::getEndReason(
    MissionData* mission
) const {
    return
        "Reached " +
        i2s(mission->goalAmount) +
        " Pikmin!";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalGrowPikmin::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastPikminBornPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastPikminBornPos;
        *outCamZoom = gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalGrowPikmin::getHudLabel() const {
    return "Pikmin";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalGrowPikmin::getName() const {
    return "Grow Pikmin";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalGrowPikmin::getPlayerDescription(
    MissionData* mission
) const {
    return "Reach a total of " + i2s(mission->goalAmount) + " Pikmin.";
}


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalGrowPikmin::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) game.curAreaData->mission.goalAmount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalGrowPikmin::getStatus(
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
bool MissionGoalGrowPikmin::isMet(
    GameplayState* gameplay
) const {
    return getCurAmount(gameplay) >= getReqAmount(gameplay);
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalGrowPikmin::isMobApplicable(
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
int MissionGoalTimedSurvival::getCurAmount(
    GameplayState* gameplay
) const {
    return gameplay->gameplayTimePassed;
}


/**
 * @brief Returns a celebration describing the player's victory,
 * with values fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The reason.
 */
string MissionGoalTimedSurvival::getEndReason(
    MissionData* mission
) const {
    return
        "Survived for " +
        timeToStr2(
            mission->goalAmount, "m", "s"
        ) +
        "!";
}


/**
 * @brief Returns where the camera should go to to zoom on the mission
 * end reason.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere.
 */
bool MissionGoalTimedSurvival::getEndZoomData(
    GameplayState* gameplay, Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief HUD label for the player's current amount.
 *
 * @return The label.
 */
string MissionGoalTimedSurvival::getHudLabel() const {
    return "Time";
}


/**
 * @brief Returns the goal's name.
 *
 * @return The name.
 */
string MissionGoalTimedSurvival::getName() const {
    return "Survive";
}


/**
 * @brief A description for the player, fed from the mission data.
 *
 * @param mission Mission data object to get info from.
 * @return The description.
 */
string MissionGoalTimedSurvival::getPlayerDescription(
    MissionData* mission
) const {
    return
        "Survive for " +
        timeToStr2(
            mission->goalAmount, "m", "s"
        ) + ".";
}


/**
 * @brief Returns the player's required amount for whatever the mission needs.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount.
 */
int MissionGoalTimedSurvival::getReqAmount(
    GameplayState* gameplay
) const {
    return (int) game.curAreaData->mission.goalAmount;
}


/**
 * @brief Status for the pause menu.
 *
 * @param cur Current amount.
 * @param req Required amount.
 * @param percentage Percentage cleared.
 * @return The status.
 */
string MissionGoalTimedSurvival::getStatus(
    int cur, int req, float percentage
) const {
    return
        "You have survived for " +
        timeToStr2(cur, "m", "s") +
        " so far. (" + i2s(percentage) + "%)";
}


/**
 * @brief Returns whether or not the mission goal's has been met.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionGoalTimedSurvival::isMet(
    GameplayState* gameplay
) const {
    return getCurAmount(gameplay) >= getReqAmount(gameplay);
}


/**
 * @brief Returns whether a given mob is applicable to this goal's
 * required mobs.
 *
 * @param type Type of the mob.
 * @return Whether it is applicable.
 */
bool MissionGoalTimedSurvival::isMobApplicable(
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
bool MissionRecord::isPlatinum(const MissionData& mission) {
    switch(mission.gradingMode) {
    case MISSION_GRADING_MODE_POINTS: {
        return score >= mission.platinumReq;
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
int MissionScoreCriterionEnemyPoints::getMultiplier(
    MissionData* mission
) const {
    return mission->pointsPerEnemyPoint;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionEnemyPoints::getName() const {
    return "Enemy points";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionEnemyPoints::getScore(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (int)
        gameplay->enemyPointsCollected *
        getMultiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionPikminBorn::getMultiplier(
    MissionData* mission
) const {
    return mission->pointsPerPikminBorn;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionPikminBorn::getName() const {
    return "Pikmin born";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionPikminBorn::getScore(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (int)
        gameplay->pikminBorn *
        getMultiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionPikminDeath::getMultiplier(
    MissionData* mission
) const {
    return mission->pointsPerPikminDeath;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionPikminDeath::getName() const {
    return "Pikmin deaths";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionPikminDeath::getScore(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (int)
        gameplay->pikminDeaths *
        getMultiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionSecLeft::getMultiplier(
    MissionData* mission
) const {
    if(
        hasFlag(
            mission->failConditions,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        return mission->pointsPerSecLeft;
    } else {
        return 0;
    }
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionSecLeft::getName() const {
    return "Seconds left";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionSecLeft::getScore(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (mission->failTimeLimit - floor(gameplay->gameplayTimePassed)) *
        getMultiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionSecPassed::getMultiplier(
    MissionData* mission
) const {
    return mission->pointsPerSecPassed;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionSecPassed::getName() const {
    return "Seconds passed";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionSecPassed::getScore(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        floor(gameplay->gameplayTimePassed) *
        getMultiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionTreasurePoints::getMultiplier(
    MissionData* mission
) const {
    return mission->pointsPerTreasurePoint;
}


/**
 * @brief Returns the mission score criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTreasurePoints::getName() const {
    return "Treasure points";
}


/**
 * @brief Returns the player's score for this criterion.
 *
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param mission Mission data to get info from.
 * @return The score.
 */
int MissionScoreCriterionTreasurePoints::getScore(
    GameplayState* gameplay, MissionData* mission
) const {
    return
        (int)
        gameplay->treasurePointsCollected *
        getMultiplier(mission);
}
