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
const size_t DEF_TIME_LIMIT = 60 * 5;

//Mission exit region minimum size.
const float EXIT_MIN_SIZE = 32.0f;

}


#pragma endregion
#pragma region Mission data


/**
 * @brief Applies the given preset.
 * Provided it's not a custom preset, this overwrites what was there before.
 *
 * @param newPreset Preset to apply.
 */
void MissionData::applyPreset(MISSION_PRESET newPreset) {
    preset = newPreset;
    
    if(newPreset == MISSION_PRESET_CUSTOM) return;
    
    reset();
    
    switch(newPreset) {
    case MISSION_PRESET_COLLECT_PIKMIN: {
        applyPresetCollectPikmin();
        break;
    } case MISSION_PRESET_COLLECT_TREASURE: {
        applyPresetCollectTreasure();
        break;
    } case MISSION_PRESET_BATTLE_ENEMIES: {
        applyPresetBattleEnemies();
        break;
    } case MISSION_PRESET_DEFEAT_BOSSES: {
        applyPresetDefeatBosses();
        break;
    } case MISSION_PRESET_COLLECT_EVERYTHING: {
        applyPresetCollectEverything();
        break;
    } case MISSION_PRESET_CUSTOM: {
        break;
    }
    }
}


/**
 * @brief Applies the Battle Enemies mission preset.
 */
void MissionData::applyPresetBattleEnemies() {
    //End conditions.
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_PAUSE_MENU,
        .clear = true,
        .zeroTimeForScore = true,
        .neutralMood = true,
        .reason = "Ended early from the pause menu!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_LESS,
        .metricType = MISSION_METRIC_SECS_LEFT,
        .matchAmount = 0,
        .clear = true,
        .neutralMood = true,
        .reason = "Time's up!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_MORE,
        .metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS,
        .idxParam = 0,
        .matchAmount = 0,
        .clear = true,
        .neutralMood = false,
        .reason = "Got all enemies!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_MORE,
        .metricType = MISSION_METRIC_LEADERS_LOST,
        .matchAmount = 1,
        .clear = true,
        .neutralMood = true,
        .reason = "Lost a leader!",
    }
    );
    
    //Mob groups.
    mobGroups.push_back(
    MissionMobGroup {
        .type = MISSION_MOB_GROUP_ENEMIES,
        .highlightOnRadar = true,
    }
    );
    
    //HUD items.
    hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
        MISSION_HUD_ITEM_DISPLAY_REM;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].text =
        "Enemies left";
    hudItems[MISSION_HUD_ITEM_ID_GOAL].metricType =
        MISSION_METRIC_MOB_GROUP_CLEARED_MOBS;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].idxParam = 0;
    
    hudItems[MISSION_HUD_ITEM_ID_SCORE].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_SCORE].displayType =
        MISSION_HUD_ITEM_DISPLAY_SCORE;
        
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].displayType =
        MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN;
        
    //Score criteria.
    scoreCriteria.push_back(
    MissionScoreCriterion {
        .metricType = MISSION_METRIC_DEFEAT_POINTS,
        .points = 1,
        .affectsHud = true,
    }
    );
    
    //Misc.
    medalAwardMode = MISSION_MEDAL_AWARD_MODE_POINTS;
    briefingObjective =
        "Defeat as many enemies as you can within the time limit!";
    briefingNotes = {
        "Defeat them all for a platinum medal!",
        "If a leader loses all their health, the mission will end early!"
    };
    startingPoints = 0;
}


/**
 * @brief Applies the Collect Everything mission preset.
 */
void MissionData::applyPresetCollectEverything() {
    //End conditions.
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_PAUSE_MENU,
        .clear = true,
        .zeroTimeForScore = true,
        .neutralMood = true,
        .reason = "Ended early from the pause menu!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_LESS,
        .metricType = MISSION_METRIC_SECS_LEFT,
        .matchAmount = 0,
        .clear = true,
        .neutralMood = true,
        .reason = "Time's up!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_MORE,
        .metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS,
        .idxParam = 0,
        .matchAmount = 0,
        .clear = true,
        .neutralMood = false,
        .reason = "Got everything!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_MORE,
        .metricType = MISSION_METRIC_LEADERS_LOST,
        .matchAmount = 1,
        .clear = true,
        .neutralMood = true,
        .reason = "Lost a leader!",
    }
    );
    
    //Mob groups.
    mobGroups.push_back(
    MissionMobGroup {
        .type = MISSION_MOB_GROUP_TREASURES_ENEMIES,
        .enemiesNeedCollection = true,
        .highlightOnRadar = true,
    }
    );
    
    //HUD items.
    hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
        MISSION_HUD_ITEM_DISPLAY_REM;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].text =
        "Things left";
    hudItems[MISSION_HUD_ITEM_ID_GOAL].metricType =
        MISSION_METRIC_MOB_GROUP_CLEARED_MOBS;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].idxParam = 0;
    
    hudItems[MISSION_HUD_ITEM_ID_SCORE].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_SCORE].displayType =
        MISSION_HUD_ITEM_DISPLAY_SCORE;
        
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].displayType =
        MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN;
        
    //Score criteria.
    scoreCriteria.push_back(
    MissionScoreCriterion {
        .metricType = MISSION_METRIC_COLLECTION_POINTS,
        .points = 1,
        .affectsHud = true,
    }
    );
    
    //Misc.
    medalAwardMode = MISSION_MEDAL_AWARD_MODE_POINTS;
    briefingObjective =
        "Collect as many treasures and enemies as you can within "
        "the time limit!";
    briefingNotes = {
        "Collect everything for a platinum medal!",
        "If a leader loses all their health, the mission will end early!"
    };
    startingPoints = 0;
}


/**
 * @brief Applies the Collect Pikmin mission preset.
 */
void MissionData::applyPresetCollectPikmin() {
    //End conditions.
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_PAUSE_MENU,
        .clear = true,
        .zeroTimeForScore = true,
        .neutralMood = true,
        .reason = "Ended early from the pause menu!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_LESS,
        .matchAmount = 0,
        .clear = true,
        .neutralMood = true,
        .reason = "Time's up!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_MORE,
        .metricType = MISSION_METRIC_LEADERS_LOST,
        .matchAmount = 1,
        .clear = true,
        .neutralMood = true,
        .reason = "Lost a leader!",
    }
    );
    
    //HUD items.
    hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
        MISSION_HUD_ITEM_DISPLAY_CUR;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].text =
        "Total Pikmin";
    hudItems[MISSION_HUD_ITEM_ID_GOAL].metricType =
        MISSION_METRIC_LIVING_PIKMIN;
        
    hudItems[MISSION_HUD_ITEM_ID_SCORE].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_SCORE].displayType =
        MISSION_HUD_ITEM_DISPLAY_SCORE;
        
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].displayType =
        MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN;
        
    //Score criteria.
    scoreCriteria.push_back(
    MissionScoreCriterion{
        .metricType = MISSION_METRIC_LIVING_PIKMIN,
        .points = 10,
        .affectsHud = true,
    }
    );
    
    //Misc.
    medalAwardMode = MISSION_MEDAL_AWARD_MODE_POINTS;
    briefingObjective =
        "Collect as many Pikmin as you can within the time limit!";
    briefingNotes = {
        "The more you have, the better your medal!",
        "If a leader loses all their health, the mission will end early!"
    };
    startingPoints = 0;
}


/**
 * @brief Applies the Collect Treasure mission preset.
 */
void MissionData::applyPresetCollectTreasure() {
    //End conditions.
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_PAUSE_MENU,
        .clear = true,
        .zeroTimeForScore = true,
        .neutralMood = true,
        .reason = "Ended early from the pause menu!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_LESS,
        .metricType = MISSION_METRIC_SECS_LEFT,
        .matchAmount = 0,
        .clear = true,
        .neutralMood = true,
        .reason = "Time's up!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_MORE,
        .metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS,
        .idxParam = 0,
        .matchAmount = 0,
        .clear = true,
        .neutralMood = false,
        .reason = "Got all treasures!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_MORE,
        .metricType = MISSION_METRIC_LEADERS_LOST,
        .matchAmount = 1,
        .clear = true,
        .neutralMood = true,
        .reason = "Lost a leader!",
    }
    );
    
    //Mob groups.
    mobGroups.push_back(
    MissionMobGroup {
        .type = MISSION_MOB_GROUP_TREASURES,
        .highlightOnRadar = true,
    }
    );
    
    //HUD items.
    hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
        MISSION_HUD_ITEM_DISPLAY_REM;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].text =
        "Treasures left";
    hudItems[MISSION_HUD_ITEM_ID_GOAL].metricType =
        MISSION_METRIC_MOB_GROUP_CLEARED_MOBS;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].idxParam = 0;
    
    hudItems[MISSION_HUD_ITEM_ID_SCORE].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_SCORE].displayType =
        MISSION_HUD_ITEM_DISPLAY_SCORE;
        
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].displayType =
        MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN;
        
    //Score criteria.
    scoreCriteria.push_back(
    MissionScoreCriterion {
        .metricType = MISSION_METRIC_COLLECTION_POINTS,
        .points = 1,
        .affectsHud = true,
    }
    );
    
    //Misc.
    medalAwardMode = MISSION_MEDAL_AWARD_MODE_POINTS;
    briefingObjective =
        "Collect as many treasures as you can within the time limit!";
    briefingNotes = {
        "Collect everything for a platinum medal!",
        "If a leader loses all their health, the mission will end early!"
    };
    startingPoints = 0;
}


/**
 * @brief Applies the Defeat Bosses mission preset.
 */
void MissionData::applyPresetDefeatBosses() {
    //End conditions.
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_PAUSE_MENU,
        .clear = false,
        .zeroTimeForScore = true,
        .neutralMood = false,
        .reason = "Ended early from the pause menu!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_LESS,
        .metricType = MISSION_METRIC_SECS_LEFT,
        .matchAmount = 0,
        .clear = false,
        .neutralMood = false,
        .reason = "Time's up!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_MORE,
        .metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS,
        .idxParam = 0,
        .matchAmount = 0,
        .clear = true,
        .neutralMood = false,
        .reason = "Defeated the boss!",
    }
    );
    endConds.push_back(
    MissionEndCond {
        .type = MISSION_END_COND_METRIC_OR_MORE,
        .metricType = MISSION_METRIC_LEADERS_LOST,
        .matchAmount = 1,
        .clear = false,
        .neutralMood = false,
        .reason = "Lost a leader!",
    }
    );
    
    //Mob groups.
    mobGroups.push_back(
    MissionMobGroup {
        .type = MISSION_MOB_GROUP_ENEMIES,
        .highlightOnRadar = true,
    }
    );
    
    //HUD items.
    hudItems[MISSION_HUD_ITEM_ID_GOAL].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_GOAL].displayType =
        MISSION_HUD_ITEM_DISPLAY_SCORE;
        
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].enabled =
        true;
    hudItems[MISSION_HUD_ITEM_ID_CLOCK].displayType =
        MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN;
        
    //Score criteria.
    scoreCriteria.push_back(
    MissionScoreCriterion {
        .metricType = MISSION_METRIC_SECS_LEFT,
        .points = 10,
        .affectsHud = true,
    }
    );
    
    //Misc.
    medalAwardMode = MISSION_MEDAL_AWARD_MODE_POINTS;
    briefingObjective =
        "Defeat the boss enemy within the time limit!";
    briefingNotes = {
        "The faster you are, the better your medal!",
        "If a leader loses all their health, you'll fail the mission!"
    };
    startingPoints = 0;
}


/**
 * @brief Returns what text to show in menus for the mission briefing's
 * objective.
 *
 * @return The text.
 */
string MissionData::getBriefingObjectiveText() const {
    return
        briefingObjective.empty() ?
        "This mission briefing doesn't contain any objective information!" :
        briefingObjective;
}


/**
 * @brief Returns what text to show in menus for each bullet point that explains
 * the mission medal award mode.
 *
 * @return The text.
 */
vector<string> MissionData::getMedalAwardBulletPoints() const {
    vector<string> result;
    
    switch(medalAwardMode) {
    case MISSION_MEDAL_AWARD_MODE_POINTS: {
        result.push_back("Platinum medal: " + i2s(platinumReq) + "+ points.");
        result.push_back("Gold medal: " + i2s(goldReq) + "+ points.");
        result.push_back("Silver medal: " + i2s(silverReq) + "+ points.");
        result.push_back("Bronze medal: " + i2s(bronzeReq) + "+ points.");
        if(!makerRecordDate.empty()) {
            result.push_back(
                "Maker's record: " + i2s(makerRecord) +
                " (" + makerRecordDate + ")"
            );
        }
        break;
    }
    case MISSION_MEDAL_AWARD_MODE_CLEAR: {
        result.push_back("Platinum medal: Clear the mission.");
        break;
    }
    case MISSION_MEDAL_AWARD_MODE_PARTICIPATION: {
        result.push_back("Platinum medal: Just play the mission.");
        break;
    }
    }
    
    return result;
}


/**
 * @brief Returns what text to show in menus for each bullet point that explains
 * the mission briefing's notes.
 *
 * @return The text.
 */
vector<string> MissionData::getNoteBulletPoints() const {
    vector<string> result;
    
    result.push_back(
        timeLimit == 0 ?
        "There is no time limit." :
        ("Time limit: " + timeToStr2(timeLimit))
    );
    
    result.insert(
        result.end(), briefingNotes.begin(), briefingNotes.end()
    );
    
    return result;
}


/**
 * @brief Returns which medal the given score would give.
 *
 * @param score Score to check.
 * @return The medal, if any.
 */
MISSION_MEDAL MissionData::getScoreMedal(int score) {
    if(score >= platinumReq) return MISSION_MEDAL_PLATINUM;
    if(score >= goldReq) return MISSION_MEDAL_GOLD;
    if(score >= silverReq) return MISSION_MEDAL_SILVER;
    if(score >= bronzeReq) return MISSION_MEDAL_BRONZE;
    return MISSION_MEDAL_NONE;
}


/**
 * @brief Returns whether ending early through the pause menu
 * results in a clear.
 *
 * @return Whether it is a clear.
 */
bool MissionData::isPauseMenuEndClear() const {
    for(size_t c = 0; c < game.curArea->mission.endConds.size(); c++) {
        if(
            game.curArea->mission.endConds[c].type ==
            MISSION_END_COND_PAUSE_MENU
        ) {
            return game.curArea->mission.endConds[c].clear;
        }
    }
    return false;
}


/**
 * @brief Clears the variables.
 */
void MissionData::reset() {
    medalAwardMode = MISSION_MEDAL_AWARD_MODE_CLEAR;
    startingPoints = 0;
    bronzeReq = MISSION::DEF_MEDAL_REQ_BRONZE;
    silverReq = MISSION::DEF_MEDAL_REQ_SILVER;
    goldReq = MISSION::DEF_MEDAL_REQ_GOLD;
    platinumReq = MISSION::DEF_MEDAL_REQ_PLATINUM;
    makerRecord = 0;
    makerRecordDate.clear();
    endConds.clear();
    mobGroups.clear();
    hudItems.clear();
    hudItems.insert(hudItems.begin(), 4, MissionHudItem());
    scoreCriteria.clear();
}


#pragma endregion
#pragma region End condition types


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::Info MissionEndCondTypePauseMenu::getInfo() const {
    return
    Info {
        .name =
        "Pause menu end",
        .description =
        "Triggers when the player ends the mission early from the pause menu.",
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypePauseMenu::getZoomData(
    MissionEndCond* cond, Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief Checks if the condition has been met.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEndCondTypePauseMenu::isMet(MissionEndCond* cond) const {
    //The pause menu "end mission" logic is responsible for this one.
    return false;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::Info MissionEndCondTypeMetricOrLess::getInfo() const {
    return
    Info {
        .name =
        "Metric is equal or less",
        .description =
        "Triggers when a mission metric's value reaches the specified "
        "amount or less.",
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeMetricOrLess::getZoomData(
    MissionEndCond* cond, Point* outCamPos, float* outCamZoom
) const {
    return
        game.missionMetricTypes[cond->metricType]->getZoomData(
            cond->idxParam, outCamPos, outCamZoom
        );
}


/**
 * @brief Checks if the condition has been met.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEndCondTypeMetricOrLess::isMet(MissionEndCond* cond) const {
    MissionMetricType* metricTypePtr = game.missionMetricTypes[cond->metricType];
    
    int amount =
        metricTypePtr->getAmount(cond->idxParam);
    int target =
        metricTypePtr->getTarget(cond->idxParam, cond->matchAmount);
        
    return amount <= target;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::Info MissionEndCondTypeMetricOrMore::getInfo() const {
    return
    Info {
        .name =
        "Metric is equal or more",
        .description =
        "Triggers when a mission metric's value reaches the specified "
        "amount or more.",
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeMetricOrMore::getZoomData(
    MissionEndCond* cond, Point* outCamPos, float* outCamZoom
) const {
    return
        game.missionMetricTypes[cond->metricType]->getZoomData(
            cond->idxParam, outCamPos, outCamZoom
        );
}


/**
 * @brief Checks if the condition has been met.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEndCondTypeMetricOrMore::isMet(MissionEndCond* cond) const {
    MissionMetricType* metricTypePtr = game.missionMetricTypes[cond->metricType];
    
    int amount =
        metricTypePtr->getAmount(cond->idxParam);
    int target =
        metricTypePtr->getTarget(cond->idxParam, cond->matchAmount);
        
    return amount >= target;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::Info MissionEndCondTypeTakeDamage::getInfo() const {
    return
    Info {
        .name =
        "Take damage",
        .description =
        "Triggers when a leader takes damage.",
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeTakeDamage::getZoomData(
    MissionEndCond* cond, Point* outCamPos, float* outCamZoom
) const {
    if(game.states.gameplay->lastHurtLeaderPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastHurtLeaderPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief Checks if the condition has been met.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEndCondTypeTakeDamage::isMet(MissionEndCond* cond) const {
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); l++) {
        if(
            game.states.gameplay->mobs.leaders[l]->health <
            game.states.gameplay->mobs.leaders[l]->maxHealth
        ) {
            return true;
        }
    }
    if(
        game.states.gameplay->mobs.leaders.size() <
        game.states.gameplay->startingNrOfLeaders
    ) {
        //If one of them vanished, they got forcefully KO'd, which...
        //really should count as taking damage.
        return true;
    }
    return false;
}


#pragma endregion
#pragma region Mission metric


/**
 * @brief Returns the amount remaining, if possible. It needs the total amount
 * for this, be it manual or automatic.
 *
 * @param idxParam Index parameter, if applicable.
 * @param manualTotal Manually-given total,
 * or INVALID to use the automatic one.
 * @return The amount.
 */
int MissionMetricType::getRemaining(
    size_t idxParam, size_t manualTotal
) const {
    return getTarget(idxParam, manualTotal) - getAmount(idxParam);
}


/**
 * @brief Returns the target amount, i.e. the match or total amount. This either
 * returns the manually-given amount or the automatic amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @param manualTarget Manually-given total,
 * or INVALID to use the automatic one.
 * @return The amount.
 */
int MissionMetricType::getTarget(
    size_t idxParam, size_t manualTarget
) const {
    if(manualTarget != INVALID) return manualTarget;
    return getAutoTarget(idxParam);
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeCollectionPoints::getAmount(size_t idxParam) const {
    return
        game.states.gameplay->treasurePointsObtained +
        game.states.gameplay->enemyCollectionPointsObtained;
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeCollectionPoints::getAutoTarget(size_t idxParam) const {
    return 0;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeCollectionPoints::getInfo() const {
    return
    Info {
        .name = "Object collection points",
        .hasAutoTarget = false
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeCollectionPoints::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    if(game.states.gameplay->lastCollectedTreasurePos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastCollectedTreasurePos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    if(game.states.gameplay->lastCollectedEnemyPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastCollectedEnemyPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeDefeatPoints::getAmount(size_t idxParam) const {
    return game.states.gameplay->enemyDefeatPointsObtained;
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeDefeatPoints::getAutoTarget(size_t idxParam) const {
    return 0;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeDefeatPoints::getInfo() const {
    return
    Info {
        .name = "Enemy defeat points",
        .hasAutoTarget = false
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeDefeatPoints::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    if(game.states.gameplay->lastMobClearedPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastMobClearedPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeLeadersInRegion::getAmount(size_t idxParam) const {
    if(idxParam >= game.curArea->regions.size()) {
        return 0;
    }
    return
        game.states.gameplay->areaRegions[idxParam].leadersInside.size();
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeLeadersInRegion::getAutoTarget(size_t idxParam) const {
    return 0;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeLeadersInRegion::getInfo() const {
    return
    Info {
        .name = "Leaders in region",
        .idxParamName = "Region number",
        .idxParamDescription = "Number of the area region.",
        .hasAutoTarget = false
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeLeadersInRegion::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    if(idxParam > game.states.gameplay->areaRegions.size() - 1) {
        return false;
    }
    Point avgPos;
    for(
        Leader* lPtr :
        game.states.gameplay->areaRegions[idxParam].leadersInside
    ) {
        if(lPtr) avgPos += lPtr->pos;
    }
    avgPos.x /=
        game.states.gameplay->areaRegions[idxParam].leadersInside.size();
    avgPos.y /=
        game.states.gameplay->areaRegions[idxParam].leadersInside.size();
    *outCamPos = avgPos;
    return true;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeLeadersLost::getAmount(size_t idxParam) const {
    return game.states.gameplay->leadersKod;
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeLeadersLost::getAutoTarget(size_t idxParam) const {
    return 0;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeLeadersLost::getInfo() const {
    return
    Info {
        .name = "Leaders lost",
        .hasAutoTarget = false
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeLeadersLost::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    if(game.states.gameplay->lastHurtLeaderPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastHurtLeaderPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeLivingPikmin::getAmount(size_t idxParam) const {
    return game.states.gameplay->getAmountOfTotalPikmin(nullptr, true);
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeLivingPikmin::getAutoTarget(size_t idxParam) const {
    return 0;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeLivingPikmin::getInfo() const {
    return
    Info {
        .name = "Living Pikmin",
        .hasAutoTarget = false
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeLivingPikmin::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    if(game.states.gameplay->lastPikminBornPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastPikminBornPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    if(game.states.gameplay->lastPikminDeathPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastPikminDeathPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeMobGroup::getAmount(size_t idxParam) const {
    if(idxParam >= game.states.gameplay->missionMobGroups.size()) {
        return 0;
    }
    return game.states.gameplay->missionMobGroups[idxParam].getNrCleared();
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeMobGroup::getAutoTarget(size_t idxParam) const {
    if(idxParam >= game.curArea->mission.mobGroups.size()) {
        return 0;
    }
    return game.curArea->mission.mobGroups[idxParam].calculateList().size();
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeMobGroup::getInfo() const {
    return
    Info {
        .name = "Mob group cleared mobs",
        .friendlyName = "Target objects cleared",
        .idxParamName = "Mob group number",
        .idxParamDescription = "Number of the mob group.",
        .hasAutoTarget = true
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeMobGroup::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    if(game.states.gameplay->lastMobClearedPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastMobClearedPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeMobGroupHealth::getAmount(size_t idxParam) const {
    if(idxParam >= game.states.gameplay->missionMobGroups.size()) {
        return 0;
    }
    return game.states.gameplay->missionMobGroups[idxParam].getCombinedHealth();
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeMobGroupHealth::getAutoTarget(size_t idxParam) const {
    if(idxParam >= game.curArea->mission.mobGroups.size()) {
        return 0;
    }
    return game.curArea->mission.mobGroups[idxParam].calculateTotalHealth();
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeMobGroupHealth::getInfo() const {
    return
    Info {
        .name = "Mob group health",
        .friendlyName = "Target object health",
        .idxParamName = "Mob group number",
        .idxParamDescription = "Number of the mob group.",
        .hasAutoTarget = true
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeMobGroupHealth::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    if(game.states.gameplay->lastMobClearedPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastMobClearedPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypePikminBorn::getAmount(size_t idxParam) const {
    return game.states.gameplay->pikminBorn;
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypePikminBorn::getAutoTarget(size_t idxParam) const {
    return 0;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypePikminBorn::getInfo() const {
    return
    Info {
        .name = "Pikmin born",
        .hasAutoTarget = false
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypePikminBorn::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    if(game.states.gameplay->lastPikminBornPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastPikminBornPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypePikminDeaths::getAmount(size_t idxParam) const {
    return game.states.gameplay->pikminDeaths;
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypePikminDeaths::getAutoTarget(size_t idxParam) const {
    return 0;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypePikminDeaths::getInfo() const {
    return
    Info {
        .name = "Pikmin deaths",
        .hasAutoTarget = false
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypePikminDeaths::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    if(game.states.gameplay->lastPikminDeathPos.x != LARGE_FLOAT) {
        *outCamPos = game.states.gameplay->lastPikminDeathPos;
        *outCamZoom = game.states.gameplay->zoomLevels[0];
        return true;
    }
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeScriptSlot::getAmount(size_t idxParam) const {
    //TODO;
    return 0;
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeScriptSlot::getAutoTarget(size_t idxParam) const {
    return 0;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeScriptSlot::getInfo() const {
    return
    Info {
        .name = "Script slot",
        .friendlyName = "Special metric",
        .idxParamName = "Slot number",
        .idxParamDescription = "Number of the script slot.",
        .hasAutoTarget = false
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeScriptSlot::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeSecsLeft::getAmount(size_t idxParam) const {
    if(game.curArea->mission.timeLimit == 0) return 0;
    int secsLeft =
        game.curArea->mission.timeLimit -
        std::floor(game.states.gameplay->areaTimePassed);
    return std::max(secsLeft, 0);
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeSecsLeft::getAutoTarget(size_t idxParam) const {
    return game.curArea->mission.timeLimit;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeSecsLeft::getInfo() const {
    return
    Info {
        .name = "Seconds left",
        .hasAutoTarget = true
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeSecsLeft::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief Returns the current amount.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount.
 */
int MissionMetricTypeSecsPassed::getAmount(size_t idxParam) const {
    return game.states.gameplay->areaTimePassed;
}


/**
 * @brief Returns the automatic target amount, if possible.
 *
 * @param idxParam Index parameter, if applicable.
 * @return The amount, or 0 if none.
 */
int MissionMetricTypeSecsPassed::getAutoTarget(size_t idxParam) const {
    return game.curArea->mission.timeLimit;
}


/**
 * @brief Returns static information about the type.
 *
 * @return The information.
 */
MissionMetricType::Info MissionMetricTypeSecsPassed::getInfo() const {
    return
    Info {
        .name = "Seconds passed",
        .hasAutoTarget = true
    };
}


/**
 * @brief Returns where the camera should go to to zoom
 * into something relevant to the metric.
 *
 * @param idxParam Index parameter, if applicable.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionMetricTypeSecsPassed::getZoomData(
    size_t idxParam, Point* outCamPos, float* outCamZoom
) const {
    return false;
}


#pragma endregion
#pragma region Mob groups


/**
 * @brief Calculates the list of all applicable mob indexes,
 * from the mob generators.
 *
 * @return The list.
 */
vector<size_t> MissionMobGroup::calculateList() const {
    if(type == MISSION_MOB_GROUP_CUSTOM) {
        return mobIdxs;
    }
    
    vector<size_t> result;
    
    for(size_t g = 0; g < game.curArea->mobGenerators.size(); g++) {
        MobGen* gPtr = game.curArea->mobGenerators[g];
        bool toAdd = false;
        
        const auto checkTreasure = [gPtr] () {
            if(gPtr->type->category->id == MOB_CATEGORY_TREASURES) {
                return true;
            } else if(gPtr->type->category->id == MOB_CATEGORY_RESOURCES) {
                ResourceType* resType = (ResourceType*) gPtr->type;
                if(
                    resType->deliveryResult ==
                    RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
                ) {
                    return true;
                }
            } else if(gPtr->type->category->id == MOB_CATEGORY_PILES) {
                PileType* pilType = (PileType*) gPtr->type;
                if(
                    pilType->contents->deliveryResult ==
                    RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
                ) {
                    return true;
                }
            }
            return false;
        };
        
        const auto checkEnemy = [gPtr] () {
            if(gPtr->type->category->id == MOB_CATEGORY_ENEMIES) {
                return true;
            }
            return false;
        };
        
        const auto checkLeader = [gPtr] () {
            if(gPtr->type->category->id == MOB_CATEGORY_LEADERS) {
                return true;
            }
            return false;
        };
        
        const auto checkPikmin = [gPtr] () {
            if(gPtr->type->category->id == MOB_CATEGORY_PIKMIN) {
                return true;
            }
            return false;
        };
        
        switch(type) {
        case MISSION_MOB_GROUP_CUSTOM: {
            break;
        } case MISSION_MOB_GROUP_TREASURES: {
            toAdd = checkTreasure();
            break;
        } case MISSION_MOB_GROUP_ENEMIES: {
            toAdd = checkEnemy();
            break;
        } case MISSION_MOB_GROUP_TREASURES_ENEMIES: {
            toAdd = checkTreasure() || checkEnemy();
            break;
        } case MISSION_MOB_GROUP_LEADERS: {
            toAdd = checkLeader();
            break;
        } case MISSION_MOB_GROUP_PIKMIN: {
            toAdd = checkPikmin();
            break;
        }
        }
        
        if(toAdd) {
            result.push_back(g);
        }
    }
    
    return result;
}


/**
 * @brief Calculates the total combined health of all applicable mob generators.
 *
 * @return The health.
 */
float MissionMobGroup::calculateTotalHealth() const {
    vector<size_t> idxs = calculateList();
    float total = 0.0f;
    for(size_t i = 0; i < idxs.size(); i++) {
        total += game.curArea->mobGenerators[idxs[i]]->type->maxHealth;
    }
    return total;
}


/**
 * @brief Clears the data inside.
 */
void MissionRecord::clear() {
    score = 0;
    date.clear();
}


/**
 * @brief Returns whether or not this record is a platinum medal.
 *
 * @param mission Mission data to get info from.
 * @return Whether it is platinum.
 */
bool MissionRecord::isPlatinum(const MissionData& mission) {
    if(date.empty()) return false;
    
    switch(mission.medalAwardMode) {
    case MISSION_MEDAL_AWARD_MODE_POINTS: {
        return score >= mission.platinumReq;
    } case MISSION_MEDAL_AWARD_MODE_CLEAR: {
        return true;
    } case MISSION_MEDAL_AWARD_MODE_PARTICIPATION: {
        return true;
    }
    }
    return false;
}


/**
 * @brief Loads a record from a data node.
 *
 * @param node The record's node.
 * @param ported If not nullptr, whether the record needed to be ported from
 * an older version's format is returned here.
 * @return Whether it succeeded.
 */
bool MissionRecord::loadFromDataNode(
    DataNode* node, bool* ported
) {
    if(ported) *ported = false;
    vector<string> parts = split(node->value, ";", true);
    
    if(parts.size() == 3) {
        //DEPRECATED by the medal-only system in 1.2.0.
        bool clear = parts[0] == "1";
        if(clear) {
            score = s2i(parts[1]);
            date = parts[2];
        }
        if(ported) *ported = true;
    } else if(parts.size() == 2) {
        score = s2i(parts[0]);
        date = parts[1];
    } else {
        return false;
    }
    
    return true;
}


/**
 * @brief Saves a record onto a data node.
 *
 * @param node The record's node.
 * @return Whether it succeeded.
 */
bool MissionRecord::saveToDataNode(DataNode* node) {
    node->value = i2s(score) + ";" + date;
    return true;
}


#pragma endregion
