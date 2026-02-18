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
    case MISSION_PRESET_GROW_PIKMIN: {
        //Grow Pikmin -- events.
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_PAUSE_MENU_END,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_TIME_LIMIT,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_LOSE_LEADERS,
            .param1 = 1,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        
        break;
        
    } case MISSION_PRESET_COLLECT_TREASURE: {
        //Collect Treasure -- events.
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_PAUSE_MENU_END,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_TIME_LIMIT,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_MOB_CHECKLIST,
            .param1 = 0,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_LOSE_LEADERS,
            .param1 = 1,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        
        break;
        
    } case MISSION_PRESET_BATTLE_ENEMIES: {
        //Battle Enemies -- events.
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_PAUSE_MENU_END,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_TIME_LIMIT,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_MOB_CHECKLIST,
            .param1 = 0,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_LOSE_LEADERS,
            .param1 = 1,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        
        break;
        
    } case MISSION_PRESET_DEFEAT_BOSSES: {
        //Defeat Bosses -- events.
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_PAUSE_MENU_END,
            .actionType = MISSION_ACTION_END_FAIL
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_TIME_LIMIT,
            .actionType = MISSION_ACTION_END_FAIL
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_MOB_CHECKLIST,
            .param1 = 0,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_LOSE_LEADERS,
            .param1 = 1,
            .actionType = MISSION_ACTION_END_FAIL
        }
        );
        
        break;
        
    } case MISSION_PRESET_COLLECT_EVERYTHING: {
        //Collect Everything -- events.
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_PAUSE_MENU_END,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_TIME_LIMIT,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_MOB_CHECKLIST,
            .param1 = 0,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        events.push_back(
        MissionEvent {
            .type = MISSION_EV_LOSE_LEADERS,
            .param1 = 1,
            .actionType = MISSION_ACTION_END_CLEAR
        }
        );
        
        break;
        
    } case MISSION_PRESET_CUSTOM: {
        break;
        
    }
    }
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
 * @brief Clears the variables.
 */
void MissionData::reset() {
    gradingMode = MISSION_GRADING_MODE_GOAL;
    startingPoints = 0;
    bronzeReq = MISSION::DEF_MEDAL_REQ_BRONZE;
    silverReq = MISSION::DEF_MEDAL_REQ_SILVER;
    goldReq = MISSION::DEF_MEDAL_REQ_GOLD;
    platinumReq = MISSION::DEF_MEDAL_REQ_PLATINUM;
    makerRecord = 0;
    makerRecordDate.clear();
    events.clear();
    hudItems.clear();
    hudItems.insert(hudItems.begin(), 4, MissionHudItem());
    scoreCriteria.clear();
}


/**
 * @brief Returns which medal the given score would give.
 *
 * @param score Score to check.
 * @return The medal, if any.
 */
MISSION_MEDAL MissionDataOld::getScoreMedal(int score) {
    if(score >= platinumReq) return MISSION_MEDAL_PLATINUM;
    if(score >= goldReq) return MISSION_MEDAL_GOLD;
    if(score >= silverReq) return MISSION_MEDAL_SILVER;
    if(score >= bronzeReq) return MISSION_MEDAL_BRONZE;
    return MISSION_MEDAL_NONE;
}


#pragma endregion
#pragma region Action types


/**
 * @brief Retrieves editor information about the mission action type.
 *
 * @return The information.
 */
MissionActionType::EditorInfo
MissionActionTypeEndClear::getEditorInfo() const {
    return
    MissionActionType::EditorInfo {
        .description =
        "Ends the mission as a clear. A medal can be awarded.",
    };
}


/**
 * @brief Returns the action's name.
 *
 * @return The name.
 */
string MissionActionTypeEndClear::getName() const {
    return "End mission, clear";
}


/**
 * @brief Runs the action.
 *
 * @param ev Mission event that triggered this action.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it was able to run.
 */
bool MissionActionTypeEndClear::run(
    MissionEvent* ev, GameplayState* gameplay
) const {
    return gameplay->endMission(true, ev->type == MISSION_EV_TIME_LIMIT, ev);
}


/**
 * @brief Retrieves editor information about the mission action type.
 *
 * @return The information.
 */
MissionActionType::EditorInfo
MissionActionTypeEndFail::getEditorInfo() const {
    return
    MissionActionType::EditorInfo {
        .description =
        "Ends the mission as a failure. No medal can be awarded.",
    };
}


/**
 * @brief Returns the action's name.
 *
 * @return The name.
 */
string MissionActionTypeEndFail::getName() const {
    return "End mission, failure";
}


/**
 * @brief Runs the action.
 *
 * @param ev Mission event that triggered this action.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it was able to run.
 */
bool MissionActionTypeEndFail::run(
    MissionEvent* ev, GameplayState* gameplay
) const {
    return gameplay->endMission(false, ev->type == MISSION_EV_TIME_LIMIT, ev);
}


/**
 * @brief Retrieves editor information about the mission action type.
 *
 * @return The information.
 */
MissionActionType::EditorInfo
MissionActionTypeScriptMessage::getEditorInfo() const {
    return
    MissionActionType::EditorInfo {
        .description =
        "Sends a message to the area's script.",
    };
}


/**
 * @brief Returns the action's name.
 *
 * @return The name.
 */
string MissionActionTypeScriptMessage::getName() const {
    return "Send script message";
}


/**
 * @brief Runs the action.
 *
 * @param ev Mission event that triggered this action.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it was able to run.
 */
bool MissionActionTypeScriptMessage::run(
    MissionEvent* ev, GameplayState* gameplay
) const {
    return true; //TODO
}


#pragma endregion
#pragma region Event types


/**
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypeLoseLeaders::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when the player loses the given number of leaders.",
        .param1Name =
        "Loss amount",
        .param1Description =
        "Number of leader losses to check for.",
        .param1IsIndex = false,
        .param1Default = 1,
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypeLoseLeaders::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEvType::HudInfo {
        .description =
        "Lose " + i2s(ev->param1) + " or more leaders.",
        .reason =
        "Lost " + amountStr((int) gameplay->leadersKod, "leader") + "!",
    };
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypeLoseLeaders::getName() const {
    return "Lose leaders";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypeLoseLeaders::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
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
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypeLoseLeaders::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->leadersKod >= ev->param1;
}


/**
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypeLosePikmin::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when the player loses the given number of Pikmin. "
        "Only Pikmin deaths count, not things like Candypop Buds.",
        .param1Name =
        "Loss amount",
        .param1Description =
        "Number of Pikmin losses to check for.",
        .param1IsIndex = false,
        .param1Default = 1,
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypeLosePikmin::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEvType::HudInfo {
        .description =
        "Lose " + i2s(ev->param1) + " or more Pikmin.",
        .reason =
        "Lost " + i2s(gameplay->pikminDeaths) + " Pikmin!",
    };
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypeLosePikmin::getName() const {
    return "Lose Pikmin";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypeLosePikmin::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
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
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypeLosePikmin::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->pikminDeaths >= ev->param1;
}


/**
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypeMobChecklist::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when the given mob checklist has been cleared. "
        "This happens when the required amount of mobs inside of "
        "that list has been collected or defeated.",
        .param1Name =
        "Mob checklist number",
        .param1Description =
        "Number of the mob checklist to check for.",
        .param1IsIndex = true,
        .param1Default = 0,
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypeMobChecklist::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    if(ev->param1 > gameplay->missionMobChecklists.size() - 1) {
        return {};
    }
    
    return
    MissionEvType::HudInfo {
        .description =
        "Clear the required things.",
        .reason =
        "Cleared " +
        i2s(gameplay->missionMobChecklists[ev->param1].requiredAmount) +
        " things!",
    };
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypeMobChecklist::getName() const {
    return "Clear mob checklist";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypeMobChecklist::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
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
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypeMobChecklist::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    if(ev->param1 > gameplay->missionMobChecklists.size() - 1) {
        return false;
    }
    
    size_t requiredAmount =
        gameplay->missionMobChecklists[ev->param1].requiredAmount;
    size_t remainingAmount =
        gameplay->missionMobChecklists[ev->param1].remaining.size();
    size_t startingAmount =
        gameplay->missionMobChecklists[ev->param1].startingAmount;
    size_t nrCleared =
        startingAmount - remainingAmount;
        
    return nrCleared >= requiredAmount;
}


/**
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypeLeadersInRegion::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when the given amount of leaders is inside "
        "the given region.",
        .param1Name =
        "Leader amount",
        .param1Description =
        "Number of leaders to check for.",
        .param1IsIndex = false,
        .param1Default = 1,
        .param2Name =
        "Region number",
        .param2Description =
        "Number of the region to check for.",
        .param2IsIndex = true,
        .param2Default = 0,
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypeLeadersInRegion::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEvType::HudInfo {
        .description = "Objects in the region.",
        .reason = "Got the objects to the region!",
    };
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypeLeadersInRegion::getName() const {
    return "Leaders in region";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypeLeadersInRegion::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    if(ev->param2 > gameplay->areaRegions.size() - 1) {
        return false;
    }
    Point avgPos;
    for(Leader* lPtr : gameplay->areaRegions[ev->param2].leadersInside) {
        if(lPtr) avgPos += lPtr->pos;
    }
    avgPos.x /= gameplay->areaRegions[ev->param2].leadersInside.size();
    avgPos.y /= gameplay->areaRegions[ev->param2].leadersInside.size();
    *outCamPos = avgPos;
    return true;
}


/**
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypeLeadersInRegion::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    if(ev->param2 > gameplay->areaRegions.size() - 1) {
        return false;
    }
    return
        gameplay->areaRegions[ev->param2].leadersInside.size() >=
        ev->param1;
}


/**
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypePauseEnd::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when the player ends the mission from the pause menu.",
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypePauseEnd::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEvType::HudInfo {
        .description = "End from the pause menu.",
        .reason = "Ended from pause menu!",
    };
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypePauseEnd::getName() const {
    return "Pause menu end";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypePauseEnd::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypePauseEnd::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    //The pause menu "end mission" logic is responsible for this one.
    return false;
}


/**
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypePikminOrFewer::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when the total Pikmin count reaches the given amount "
        "or fewer.",
        .param1Name =
        "Pikmin amount",
        .param1Description =
        "Amount of Pikmin to check for.",
        .param1IsIndex = false,
        .param1Default = 1,
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypePikminOrFewer::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEvType::HudInfo {
        .description =
        "Reach " + i2s(ev->param1) + " Pikmin or fewer.",
        .reason =
        "Reached " +
        i2s(gameplay->getAmountOfTotalPikmin()) + " Pikmin!",
    };
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypePikminOrFewer::getName() const {
    return "Pikmin or fewer";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypePikminOrFewer::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
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
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypePikminOrFewer::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->getAmountOfTotalPikmin() <= (long) ev->param1;
}


/**
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypePikminOrMore::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when the total Pikmin count reaches the given amount "
        "or more.",
        .param1Name =
        "Pikmin amount",
        .param1Description =
        "Amount of Pikmin to check for.",
        .param1IsIndex = false,
        .param1Default = 1,
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypePikminOrMore::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEvType::HudInfo {
        .description =
        "Reach " + i2s(ev->param1) + " Pikmin or more.",
        .reason =
        "Reached " +
        i2s(gameplay->getAmountOfTotalPikmin()) + " Pikmin!",
    };
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypePikminOrMore::getName() const {
    return "Pikmin or more";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypePikminOrMore::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
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
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypePikminOrMore::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->getAmountOfTotalPikmin() >= (long) ev->param1;
}


/**
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypeScriptTrigger::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when the area's script sends the given signal number.",
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypeScriptTrigger::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return {};
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypeScriptTrigger::getName() const {
    return "Script trigger";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypeScriptTrigger::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypeScriptTrigger::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return false; //TODO
}


/**
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypeTakeDamage::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when any leader takes any damage.",
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypeTakeDamage::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEvType::HudInfo {
        .description = "Take damage.",
        .reason = "Took damage!",
    };
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypeTakeDamage::getName() const {
    return "Take damage";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypeTakeDamage::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
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
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypeTakeDamage::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
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
 * @brief Retrieves editor information about the mission event type.
 *
 * @return The information.
 */
MissionEvType::EditorInfo MissionEvTypeTimeLimit::getEditorInfo() const {
    return
    MissionEvType::EditorInfo {
        .description =
        "Triggers when the mission's time limit is up.",
    };
}


/**
 * @brief Retrieves HUD information about the mission event type.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return The information.
 */
MissionEvType::HudInfo MissionEvTypeTimeLimit::getHudInfo(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    return
    MissionEvType::HudInfo {
        .description = "Reach the time limit.",
        .reason = "Time's up!",
    };
}


/**
 * @brief Returns the event's name.
 *
 * @return The name.
 */
string MissionEvTypeTimeLimit::getName() const {
    return "Time limit";
}


/**
 * @brief Returns where the camera should go to to zoom when the event happens.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @param outCamPos The final camera position is returned here.
 * @param outCamZoom The final camera zoom is returned here.
 * @return Whether the camera should zoom somewhere in the first place.
 */
bool MissionEvTypeTimeLimit::getZoomData(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
    Point* outCamPos, float* outCamZoom
) const {
    return false;
}


/**
 * @brief Checks if the event's conditions have been met.
 *
 * @param ev Event being processed.
 * @param mission Pointer to the mission data to get info from.
 * @param gameplay Pointer to the gameplay state to get info from.
 * @return Whether it is met.
 */
bool MissionEvTypeTimeLimit::isMet(
    MissionEvent* ev, MissionData* mission, GameplayState* gameplay
) const {
    if(mission->timeLimit == 0) return false;
    if(gameplay->afterHours) return false;
    if(gameplay->gameplayTimePassed < mission->timeLimit) return false;
    return true;
}


#pragma endregion
#pragma region Fail conditions


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
    MissionDataOld* mission
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
    if(gameplay->lastMobClearedPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastMobClearedPos;
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
    MissionDataOld* mission
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
    return (int) game.curAreaData->missionOld.failEnemiesDefeated;
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    return (int) game.curAreaData->missionOld.failLeadersKod;
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    return (int) game.curAreaData->missionOld.failPikKilled;
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    return (int) game.curAreaData->missionOld.failTimeLimit;
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    return (int) game.curAreaData->missionOld.failTooFewPikAmount;
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    return (int) game.curAreaData->missionOld.failTooManyPikAmount;
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


#pragma endregion
#pragma region Goals


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
    MissionDataOld* mission
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
    if(gameplay->lastMobClearedPos.x != LARGE_FLOAT) {
        *outCamPos = gameplay->lastMobClearedPos;
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    return (int) game.curAreaData->missionOld.goalAmount;
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
    MissionDataOld* mission
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
    MissionDataOld* mission
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
    return (int) game.curAreaData->missionOld.goalAmount;
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


#pragma endregion
#pragma region Mob checklists


/**
 * @brief Calculates the list of all applicable mob indexes,
 * from the mob generators.
 *
 * @return The list.
 */
vector<size_t> MissionMobChecklist::calculateList() const {
    if(type == MISSION_MOB_CHECKLIST_CUSTOM) {
        return mobIdxs;
    }
    
    vector<size_t> result;
    
    for(size_t g = 0; g < game.curAreaData->mobGenerators.size(); g++) {
        MobGen* gPtr = game.curAreaData->mobGenerators[g];
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
        case MISSION_MOB_CHECKLIST_CUSTOM: {
            break;
        } case MISSION_MOB_CHECKLIST_TREASURES: {
            toAdd = checkTreasure();
            break;
        } case MISSION_MOB_CHECKLIST_ENEMIES: {
            toAdd = checkEnemy();
            break;
        } case MISSION_MOB_CHECKLIST_TREASURES_ENEMIES: {
            toAdd = checkTreasure() || checkEnemy();
            break;
        } case MISSION_MOB_CHECKLIST_LEADERS: {
            toAdd = checkLeader();
            break;
        } case MISSION_MOB_CHECKLIST_PIKMIN: {
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
 * @brief Returns whether or not this record is a platinum medal.
 *
 * @param mission Mission data to get info from.
 * @return Whether it is platinum.
 */
bool MissionRecord::isPlatinum(const MissionDataOld& mission) {
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


#pragma endregion
#pragma region Score criteria


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeCollectionPts::getName() const {
    return "Collection points";
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
size_t MissionScoreCriterionTypeCollectionPts::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->treasurePointsObtained;
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeDefeatPts::getName() const {
    return "Defeat points";
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
    return gameplay->enemyPointsObtained;
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeMobChecklist::getName() const {
    return "Mob checklist mob";
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
size_t MissionScoreCriterionTypeMobChecklist::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    if(cri->param1 > gameplay->missionMobChecklists.size() - 1) {
        return 0;
    }
    
    MissionMobChecklistStatus* cPtr =
        &gameplay->missionMobChecklists[cri->param1];
    return cPtr->startingAmount - cPtr->remaining.size();
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
size_t MissionScoreCriterionTypePikmin::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->getAmountOfTotalPikmin();
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
size_t MissionScoreCriterionTypePikminBorn::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->pikminBorn;
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
size_t MissionScoreCriterionTypePikminDeaths::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return gameplay->pikminDeaths;
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
size_t MissionScoreCriterionTypeSecLeft::calculateAmount(
    MissionScoreCriterion* cri, MissionData* mission, GameplayState* gameplay
) const {
    return
        mission->timeLimit - floor(gameplay->gameplayTimePassed);
}


/**
 * @brief Returns the criterion's name.
 *
 * @return The name.
 */
string MissionScoreCriterionTypeSecPassed::getName() const {
    return "Seconds passed";
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
    return floor(gameplay->gameplayTimePassed);
}


#pragma endregion
#pragma region Score criteria (old)


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionEnemyPoints::getMultiplier(
    MissionDataOld* mission
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
    GameplayState* gameplay, MissionDataOld* mission
) const {
    return
        (int)
        gameplay->enemyPointsObtained *
        getMultiplier(mission);
}


/**
 * @brief Returns the mission score criterion's point multiplier.
 *
 * @param mission Mission data to get info from.
 * @return The multiplier.
 */
int MissionScoreCriterionPikminBorn::getMultiplier(
    MissionDataOld* mission
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
    GameplayState* gameplay, MissionDataOld* mission
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
    MissionDataOld* mission
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
    GameplayState* gameplay, MissionDataOld* mission
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
    MissionDataOld* mission
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
    GameplayState* gameplay, MissionDataOld* mission
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
    MissionDataOld* mission
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
    GameplayState* gameplay, MissionDataOld* mission
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
    MissionDataOld* mission
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
    GameplayState* gameplay, MissionDataOld* mission
) const {
    return
        (int)
        gameplay->treasurePointsObtained *
        getMultiplier(mission);
}
