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
    case MISSION_PRESET_GROW_MANY_PIKMIN: {
        //Grow Many Pikmin -- end conditions.
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = true,
            .zeroTimeForScore = true,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_TIME_LIMIT,
            .clear = true,
            .neutralMood = true,
            .reason = "Time's up!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_LOSE_LEADERS,
            .amountParam = 1,
            .clear = true,
            .neutralMood = true,
            .reason = "Lost a leader!",
        }
        );
        
        break;
        
    } case MISSION_PRESET_COLLECT_TREASURE: {
        //Collect Treasure -- end conditions.
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = true,
            .zeroTimeForScore = true,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_TIME_LIMIT,
            .clear = true,
            .neutralMood = true,
            .reason = "Time's up!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_MOB_GROUP,
            .indexParam = 0,
            .clear = true,
            .neutralMood = false,
            .reason = "Got all treasures!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_LOSE_LEADERS,
            .amountParam = 1,
            .clear = true,
            .neutralMood = true,
            .reason = "Lost a leader!",
        }
        );
        
        break;
        
    } case MISSION_PRESET_BATTLE_ENEMIES: {
        //Battle Enemies -- end conditions.
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = true,
            .zeroTimeForScore = true,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_TIME_LIMIT,
            .clear = true,
            .neutralMood = true,
            .reason = "Time's up!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_MOB_GROUP,
            .indexParam = 0,
            .clear = true,
            .neutralMood = false,
            .reason = "Got all enemies!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_LOSE_LEADERS,
            .amountParam = 1,
            .clear = true,
            .neutralMood = true,
            .reason = "Lost a leader!",
        }
        );
        
        break;
        
    } case MISSION_PRESET_DEFEAT_BOSSES: {
        //Defeat Bosses -- end conditions.
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = false,
            .zeroTimeForScore = false,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_TIME_LIMIT,
            .clear = true,
            .neutralMood = false,
            .reason = "Time's up!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_MOB_GROUP,
            .indexParam = 0,
            .clear = false,
            .neutralMood = false,
            .reason = "Defeated the boss!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_LOSE_LEADERS,
            .amountParam = 1,
            .clear = true,
            .neutralMood = false,
            .reason = "Lost a leader!",
        }
        );
        
        break;
        
    } case MISSION_PRESET_COLLECT_EVERYTHING: {
        //Collect Everything -- end conditions.
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_PAUSE_MENU,
            .clear = true,
            .zeroTimeForScore = true,
            .neutralMood = false,
            .reason = "Ended early from the pause menu!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_TIME_LIMIT,
            .clear = true,
            .neutralMood = true,
            .reason = "Time's up!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_MOB_GROUP,
            .indexParam = 0,
            .clear = true,
            .neutralMood = false,
            .reason = "Got everything!",
        }
        );
        endConds.push_back(
        MissionEndCond {
            .type = MISSION_END_COND_LOSE_LEADERS,
            .amountParam = 1,
            .clear = true,
            .neutralMood = true,
            .reason = "Lost a leader!",
        }
        );
        
        break;
        
    } case MISSION_PRESET_CUSTOM: {
        break;
        
    }
    }
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
MissionEndCondType::EditorInfo
MissionEndCondTypeLeadersInRegion::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers when the given amount of leaders is inside "
        "the given region.",
        .indexParamName =
        "Region number",
        .indexParamDescription =
        "Number of the region to check for.",
        .amountParamName =
        "Leader amount",
        .amountParamDescription =
        "Number of leaders to check for.",
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypeLeadersInRegion::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEndCondType::HudInfo {
        .description = "Leaders in the region.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypeLeadersInRegion::getName() const {
    return "Leaders in region";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeLeadersInRegion::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    if(cond->indexParam > gameplay->areaRegions.size() - 1) {
        return false;
    }
    Point avgPos;
    for(Leader* lPtr : gameplay->areaRegions[cond->indexParam].leadersInside) {
        if(lPtr) avgPos += lPtr->pos;
    }
    avgPos.x /= gameplay->areaRegions[cond->indexParam].leadersInside.size();
    avgPos.y /= gameplay->areaRegions[cond->indexParam].leadersInside.size();
    *outCamPos = avgPos;
    return true;
}


/**
 * @brief Checks if the condition has been met.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEndCondTypeLeadersInRegion::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    if(cond->indexParam > gameplay->areaRegions.size() - 1) {
        return false;
    }
    return
        gameplay->areaRegions[cond->indexParam].leadersInside.size() >=
        cond->amountParam;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::EditorInfo
MissionEndCondTypeLoseLeaders::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers when the player loses the given number of leaders.",
        .amountParamName =
        "Loss amount",
        .amountParamDescription =
        "Number of leader losses to check for."
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypeLoseLeaders::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEndCondType::HudInfo {
        .description =
        "Lose " + i2s(cond->amountParam) + " or more leaders.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypeLoseLeaders::getName() const {
    return "Lose leaders";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeLoseLeaders::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastHurtLeaderPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastHurtLeaderPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
bool MissionEndCondTypeLoseLeaders::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->leadersKod >= cond->amountParam;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::EditorInfo
MissionEndCondTypeLosePikmin::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers when the player loses the given number of Pikmin. "
        "Only Pikmin deaths count, not things like Candypop Buds.",
        .amountParamName =
        "Loss amount",
        .amountParamDescription =
        "Number of Pikmin losses to check for."
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypeLosePikmin::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEndCondType::HudInfo {
        .description =
        "Lose " + i2s(cond->amountParam) + " or more Pikmin.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypeLosePikmin::getName() const {
    return "Lose Pikmin";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeLosePikmin::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastPikminDeathPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastPikminDeathPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
bool MissionEndCondTypeLosePikmin::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->pikminDeaths >= cond->amountParam;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::EditorInfo
MissionEndCondTypeMobGroup::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers when the required amount of mobs inside of the "
        "given mob group has been collected or defeated.",
        .indexParamName =
        "Mob group number",
        .indexParamDescription =
        "Number of the mob group to check for.",
        .amountParamName =
        "Required amount",
        .amountParamDescription =
        "The required number of mobs. 0 means all."
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypeMobGroup::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    if(cond->indexParam > gameplay->missionMobGroups.size() - 1) {
        return {};
    }
    
    return
    MissionEndCondType::HudInfo {
        .description =
        "Clear the required targets.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypeMobGroup::getName() const {
    return "Clear mob group";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeMobGroup::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastMobClearedPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastMobClearedPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
bool MissionEndCondTypeMobGroup::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    if(cond->indexParam > gameplay->missionMobGroups.size() - 1) {
        return false;
    }
    
    size_t requiredAmount =
        cond->amountParam == 0 ?
        gameplay->missionMobGroups[cond->indexParam].startingAmount :
        cond->amountParam;
        
    return
        gameplay->missionMobGroups[cond->indexParam].getNrCleared() >=
        requiredAmount;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::EditorInfo
MissionEndCondTypePauseMenu::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers when the player ends the mission early from the pause menu.",
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypePauseMenu::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEndCondType::HudInfo {
        .description =
        "End early from the pause menu.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypePauseMenu::getName() const {
    return "Pause menu end";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypePauseMenu::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
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
bool MissionEndCondTypePauseMenu::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    //The pause menu "end mission" logic is responsible for this one.
    return false;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::EditorInfo
MissionEndCondTypePikminOrFewer::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers when the total Pikmin count reaches the given amount "
        "or fewer.",
        .amountParamName =
        "Pikmin amount",
        .amountParamDescription =
        "Amount of Pikmin to check for.",
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypePikminOrFewer::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEndCondType::HudInfo {
        .description =
        "Reach " + i2s(cond->amountParam) + " Pikmin or fewer.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypePikminOrFewer::getName() const {
    return "Pikmin or fewer";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypePikminOrFewer::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastPikminDeathPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastPikminDeathPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
bool MissionEndCondTypePikminOrFewer::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->getAmountOfTotalPikmin() <= (long) cond->amountParam;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::EditorInfo
MissionEndCondTypePikminOrMore::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers when the total Pikmin count reaches the given amount "
        "or more.",
        .amountParamName =
        "Pikmin amount",
        .amountParamDescription =
        "Amount of Pikmin to check for.",
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypePikminOrMore::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEndCondType::HudInfo {
        .description =
        "Reach " + i2s(cond->amountParam) + " Pikmin or more.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypePikminOrMore::getName() const {
    return "Pikmin or more";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypePikminOrMore::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastPikminBornPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastPikminBornPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
bool MissionEndCondTypePikminOrMore::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->getAmountOfTotalPikmin() >= (long) cond->amountParam;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::EditorInfo
MissionEndCondTypeScript::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers exclusively when the area script calls it.",
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypeScript::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEndCondType::HudInfo {
        .description = "Script.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypeScript::getName() const {
    return "Script";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeScript::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
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
bool MissionEndCondTypeScript::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return false;
}


/**
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::EditorInfo
MissionEndCondTypeTakeDamage::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers when any leader takes any damage.",
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypeTakeDamage::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEndCondType::HudInfo {
        .description = "Take damage.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypeTakeDamage::getName() const {
    return "Take damage";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeTakeDamage::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    if(gameplay->lastHurtLeaderPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastHurtLeaderPos;
        *outCamZoom = gameplay->zoomLevels[0];
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
bool MissionEndCondTypeTakeDamage::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
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
 * @brief Retrieves editor information about the mission end condition type.
 *
 * @return The information.
 */
MissionEndCondType::EditorInfo
MissionEndCondTypeTimeLimit::getEditorInfo() const {
    return
    MissionEndCondType::EditorInfo {
        .description =
        "Triggers when the mission's time limit is up.",
    };
}


/**
 * @brief Retrieves HUD information about the mission end condition type.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEndCondType::HudInfo MissionEndCondTypeTimeLimit::getHudInfo(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEndCondType::HudInfo {
        .description = "Reach the time limit.",
    };
}


/**
 * @brief Returns the condition's name.
 *
 * @return The name.
 */
string MissionEndCondTypeTimeLimit::getName() const {
    return "Time limit";
}


/**
 * @brief Returns where the camera should go to to zoom
 * when the condition triggers.
 *
 * @param cond Condition being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEndCondTypeTimeLimit::getZoomData(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
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
bool MissionEndCondTypeTimeLimit::isMet(
    MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
) const {
    if(mission->timeLimit == 0) return false;
    if(gameplay->afterHours) return false;
    if(gameplay->gameplayTimePassed < mission->timeLimit) return false;
    return true;
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
#pragma region Score criteria


/**
 * @brief Calculates the amount relevant to this criterion so the final score
 * can be calculated.
 *
 * @param cri Criterion being process.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount of points.
 */
size_t MissionScoreCriterionTypeCollectionPts::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->treasurePointsObtained;
}


/**
 * @brief Returns the criterion's friendly name, for the player.
 * If empty, the standard name should be used.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeCollectionPts::getFriendlyName() const {
    return "";
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeCollectionPts::getName() const {
    return "Object collection points";
}


/**
 * @brief Calculates the amount relevant to this criterion so the final score
 * can be calculated.
 *
 * @param cri Criterion being process.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount of points.
 */
size_t MissionScoreCriterionTypeDefeatPts::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->enemyDefeatPointsObtained;
}


/**
 * @brief Returns the criterion's friendly name, for the player.
 * If empty, the standard name should be used.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeDefeatPts::getFriendlyName() const {
    return "";
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeDefeatPts::getName() const {
    return "Enemy defeat points";
}


/**
 * @brief Calculates the amount relevant to this criterion so the final score
 * can be calculated.
 *
 * @param cri Criterion being process.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount of points.
 */
size_t MissionScoreCriterionTypeMobGroup::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    if(cri->indexParam > gameplay->missionMobGroups.size() - 1) {
        return 0;
    }
    
    return gameplay->missionMobGroups[cri->indexParam].getNrCleared();
}


/**
 * @brief Returns the criterion's friendly name, for the player.
 * If empty, the standard name should be used.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeMobGroup::getFriendlyName() const {
    return "Target objects";
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeMobGroup::getName() const {
    return "Mob group target";
}


/**
 * @brief Calculates the amount relevant to this criterion so the final score
 * can be calculated.
 *
 * @param cri Criterion being process.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount of points.
 */
size_t MissionScoreCriterionTypePikmin::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->getAmountOfTotalPikmin();
}


/**
 * @brief Returns the criterion's friendly name, for the player.
 * If empty, the standard name should be used.
 *
 * @return The name.
 */
string MissionScoreCriterionTypePikmin::getFriendlyName() const {
    return "";
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypePikmin::getName() const {
    return "Pikmin total";
}


/**
 * @brief Calculates the amount relevant to this criterion so the final score
 * can be calculated.
 *
 * @param cri Criterion being process.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount of points.
 */
size_t MissionScoreCriterionTypePikminBorn::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->pikminBorn;
}


/**
 * @brief Returns the criterion's friendly name, for the player.
 * If empty, the standard name should be used.
 *
 * @return The name.
 */
string MissionScoreCriterionTypePikminBorn::getFriendlyName() const {
    return "";
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypePikminBorn::getName() const {
    return "Pikmin born";
}


/**
 * @brief Calculates the amount relevant to this criterion so the final score
 * can be calculated.
 *
 * @param cri Criterion being process.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount of points.
 */
size_t MissionScoreCriterionTypePikminDeaths::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->pikminDeaths;
}


/**
 * @brief Returns the criterion's friendly name, for the player.
 * If empty, the standard name should be used.
 *
 * @return The name.
 */
string MissionScoreCriterionTypePikminDeaths::getFriendlyName() const {
    return "";
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypePikminDeaths::getName() const {
    return "Pikmin deaths";
}


/**
 * @brief Calculates the amount relevant to this criterion so the final score
 * can be calculated.
 *
 * @param cri Criterion being process.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount of points.
 */
size_t MissionScoreCriterionTypeSecLeft::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return
        gameplay->missionConsiderZeroTime ?
        0 :
        mission->timeLimit - floor(gameplay->gameplayTimePassed);
}


/**
 * @brief Returns the criterion's friendly name, for the player.
 * If empty, the standard name should be used.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeSecLeft::getFriendlyName() const {
    return "";
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeSecLeft::getName() const {
    return "Seconds left";
}


/**
 * @brief Calculates the amount relevant to this criterion so the final score
 * can be calculated.
 *
 * @param cri Criterion being process.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The amount of points.
 */
size_t MissionScoreCriterionTypeSecPassed::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return
        gameplay->missionConsiderZeroTime ?
        0 :
        floor(gameplay->gameplayTimePassed);
}


/**
 * @brief Returns the criterion's friendly name, for the player.
 * If empty, the standard name should be used.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeSecPassed::getFriendlyName() const {
    return "";
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeSecPassed::getName() const {
    return "Seconds passed";
}


#pragma endregion
