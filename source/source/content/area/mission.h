/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mission class and related functions.
 */

#pragma once

#include <array>
#include <string>
#include <unordered_set>

#include "../../core/const.h"
#include "../../util/enum_utils.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"
#include "../mob_category/mob_category.h"

using std::string;
using std::unordered_set;


class GameplayState;


#pragma region Constants


namespace MISSION {
extern const int DEF_MEDAL_REQ_BRONZE;
extern const int DEF_MEDAL_REQ_GOLD;
extern const int DEF_MEDAL_REQ_PLATINUM;
extern const int DEF_MEDAL_REQ_SILVER;
extern const size_t DEF_TIME_LIMIT;
extern const float EXIT_MIN_SIZE;
}


//Available presets for missions.
enum MISSION_PRESET {

    //Grow as many Pikmin as you can within the time limit.
    //Medal depends on how many you grew. Matches P1.
    MISSION_PRESET_GROW_PIKMIN,
    
    //Collect treasures within the time limit.
    //Medal depends on how many got collected, platinum for all. Matches P3.
    MISSION_PRESET_COLLECT_TREASURE,
    
    //Battle enemies within the time limit.
    //Medal depends on how many were defeated, platinum for all. Matches P3.
    MISSION_PRESET_BATTLE_ENEMIES,
    
    //Defeat bosses within the time limit.
    //Medal depends on time taken. Matches P3.
    MISSION_PRESET_DEFEAT_BOSSES,
    
    //Collect treasures and enemies within the time limit. Matches P4.
    MISSION_PRESET_COLLECT_EVERYTHING,
    
    //Custom rules.
    MISSION_PRESET_CUSTOM,
    
};


//Mission preset enum naming.
buildEnumNames(missionPresetNames, MISSION_PRESET)({
    { MISSION_PRESET_GROW_PIKMIN, "Grow Pikmin" },
    { MISSION_PRESET_COLLECT_TREASURE, "Collect Treasure" },
    { MISSION_PRESET_BATTLE_ENEMIES, "Battle Enemies" },
    { MISSION_PRESET_DEFEAT_BOSSES, "Defeat Bosses" },
    { MISSION_PRESET_COLLECT_EVERYTHING, "Collect Everything" },
    { MISSION_PRESET_CUSTOM, "Custom" },
});


//Possible events that can happen in missions.
enum MISSION_EV {

    //Mission was ended from the pause menu.
    MISSION_EV_PAUSE_MENU_END,
    
    //Specified mob checklist was cleared.
    MISSION_EV_MOB_CHECKLIST,
    
    //Time limit was reached.
    MISSION_EV_TIME_LIMIT,
    
    //Specified amount of leaders entered the specified in-area region.
    MISSION_EV_LEADERS_IN_REGION,
    
    //Pikmin count reached specified amount or more.
    MISSION_EV_PIKMIN_OR_MORE,
    
    //Pikmin count reached specified amount or fewer.
    MISSION_EV_PIKMIN_OR_FEWER,
    
    //Specified amount of Pikmin was lost.
    MISSION_EV_LOSE_PIKMIN,
    
    //Specified amount of leaders were lost.
    MISSION_EV_LOSE_LEADERS,
    
    //A leader took damage.
    MISSION_EV_TAKE_DAMAGE,
    
    //Triggered via scripting.
    MISSION_EV_SCRIPT,
    
};


//Possible actions to take when a mission event happens.
enum MISSION_ACTION {

    //End the mission as a clear. A medal can be awarded.
    MISSION_ACTION_END_CLEAR,
    
    //End the mission as a failure. A medal cannot be awarded.
    MISSION_ACTION_END_FAIL,
    
    //Send a message to the script.
    MISSION_ACTION_SEND_MESSAGE,
    
};


//Types of mission mob checklists.
enum MISSION_MOB_CHECKLIST {

    //Mobs from the given list.
    MISSION_MOB_CHECKLIST_CUSTOM,
    
    //Treasures and treasure-like objects.
    MISSION_MOB_CHECKLIST_TREASURES,
    
    //Enemies and enemy-like objects.
    MISSION_MOB_CHECKLIST_ENEMIES,
    
    //Treasures, treasure-like objects, enemies, and enemy-like objects.
    MISSION_MOB_CHECKLIST_TREASURES_ENEMIES,
    
    //Leader objects.
    MISSION_MOB_CHECKLIST_LEADERS,
    
    //Pikmin objects.
    MISSION_MOB_CHECKLIST_PIKMIN,
    
};


//Mission mob checklist type enum naming.
buildEnumNames(missionMobChecklistTypeNames, MISSION_MOB_CHECKLIST)({
    { MISSION_MOB_CHECKLIST_CUSTOM, "Custom" },
    { MISSION_MOB_CHECKLIST_TREASURES, "Treasures" },
    { MISSION_MOB_CHECKLIST_ENEMIES, "Enemies" },
    { MISSION_MOB_CHECKLIST_TREASURES_ENEMIES, "Treasures and enemies" },
    { MISSION_MOB_CHECKLIST_LEADERS, "Leaders" },
    { MISSION_MOB_CHECKLIST_PIKMIN, "Pikmin" },
});


//Scoring criterion types for score-based missions.
enum MISSION_SCORE_CRITERION {

    //Amount of mobs from a checklist cleared.
    MISSION_SCORE_CRITERION_MOB_CHECKLIST,
    
    //Total amount of Pikmin.
    MISSION_SCORE_CRITERION_PIKMIN,
    
    //Amount of Pikmin born.
    MISSION_SCORE_CRITERION_PIKMIN_BORN,
    
    //Amount of Pikmin deaths.
    MISSION_SCORE_CRITERION_PIKMIN_DEATHS,
    
    //Seconds left in the time limit.
    MISSION_SCORE_CRITERION_SEC_LEFT,
    
    //Seconds since the mission started.
    MISSION_SCORE_CRITERION_SEC_PASSED,
    
    //Treasure or enemy collection points collected.
    MISSION_SCORE_CRITERION_COLLECTION_PTS,
    
    //Enemy defeat points obtained.
    MISSION_SCORE_CRITERION_DEFEAT_PTS,
    
};


//Mission score criterion type enum naming.
buildEnumNames(missionScoreCriterionTypeNames, MISSION_SCORE_CRITERION)({
    { MISSION_SCORE_CRITERION_MOB_CHECKLIST, "Mob checklist item" },
    { MISSION_SCORE_CRITERION_PIKMIN, "Total Pikmin" },
    { MISSION_SCORE_CRITERION_PIKMIN_BORN, "Pikmin born" },
    { MISSION_SCORE_CRITERION_PIKMIN_DEATHS, "Pikmin deaths" },
    { MISSION_SCORE_CRITERION_SEC_LEFT, "Seconds left" },
    { MISSION_SCORE_CRITERION_SEC_PASSED, "Seconds passed" },
    { MISSION_SCORE_CRITERION_COLLECTION_PTS, "Mob collection points" },
    { MISSION_SCORE_CRITERION_DEFEAT_PTS, "Enemy defeat points" },
});


//Mission HUD item IDs and their typical purposes.
enum MISSION_HUD_ITEM_ID {

    //Item used mostly for something related to the goal.
    MISSION_HUD_ITEM_ID_GOAL,
    
    //Item used mostly for the score.
    MISSION_HUD_ITEM_ID_SCORE,
    
    //Item used mostly for the clock.
    MISSION_HUD_ITEM_ID_CLOCK,
    
    //Item used mostly for misc. things.
    MISSION_HUD_ITEM_ID_MISC,
    
};


//Mission HUD item ID enum naming.
buildEnumNames(missionHudItemIdNames, MISSION_HUD_ITEM_ID)({
    { MISSION_HUD_ITEM_ID_GOAL, "Goal" },
    { MISSION_HUD_ITEM_ID_SCORE, "Score" },
    { MISSION_HUD_ITEM_ID_CLOCK, "Clock" },
    { MISSION_HUD_ITEM_ID_MISC, "Misc." },
});


//Possible types of content to show in a mission HUD item.
enum MISSION_HUD_ITEM_CONTENT {

    //Custom text.
    MISSION_HUD_ITEM_CONTENT_TEXT,
    
    //The time limit in a clock, ticking down to 0.
    MISSION_HUD_ITEM_CONTENT_CLOCK_DOWN,
    
    //The time spent in a clock, ticking up.
    MISSION_HUD_ITEM_CONTENT_CLOCK_UP,
    
    //Current score.
    MISSION_HUD_ITEM_CONTENT_SCORE,
    
    //Current amount of something, out of a total.
    MISSION_HUD_ITEM_CONTENT_CUR_TOT,
    
    //Remaining amount of something, out of a total.
    MISSION_HUD_ITEM_CONTENT_REM_TOT,
    
    //Current amount of something.
    MISSION_HUD_ITEM_CONTENT_CUR_AMT,
    
    //Remaining amount of something.
    MISSION_HUD_ITEM_CONTENT_REM_AMT,
    
    //Total amount of something.
    MISSION_HUD_ITEM_CONTENT_TOT_AMT,
    
};


//Mission HUD item content type enum naming.
buildEnumNames(missionHudItemContentTypeNames, MISSION_HUD_ITEM_CONTENT)({
    { MISSION_HUD_ITEM_CONTENT_TEXT, "Custom text" },
    { MISSION_HUD_ITEM_CONTENT_CLOCK_DOWN, "Clock ticking down" },
    { MISSION_HUD_ITEM_CONTENT_CLOCK_UP, "Clock ticking up" },
    { MISSION_HUD_ITEM_CONTENT_SCORE, "Score" },
    { MISSION_HUD_ITEM_CONTENT_CUR_TOT, "Current amount / total" },
    { MISSION_HUD_ITEM_CONTENT_REM_TOT, "Remaining amount / total" },
    { MISSION_HUD_ITEM_CONTENT_CUR_AMT, "Current amount" },
    { MISSION_HUD_ITEM_CONTENT_REM_AMT, "Remaining amount" },
    { MISSION_HUD_ITEM_CONTENT_TOT_AMT, "Total amount" },
});


//Types of things a mission HUD item can show amounts of.
enum MISSION_HUD_ITEM_AMT {

    //Amounts in one or more mob checklist.
    MISSION_HUD_ITEM_AMT_MOB_CHECKLIST,
    
    //Amount of leaders inside one or more regions.
    MISSION_HUD_ITEM_AMT_LEADERS_IN_REGION,
    
    //Total amount of Pikmin.
    MISSION_HUD_ITEM_AMT_PIKMIN,
    
    //Total amount of leaders.
    MISSION_HUD_ITEM_AMT_LEADERS,
    
    //Amount of Pikmin deaths so far.
    MISSION_HUD_ITEM_AMT_PIKMIN_DEATHS,
    
    //Amount of leader KOs so far.
    MISSION_HUD_ITEM_AMT_LEADER_KOS,
    
};


//Mission HUD item amount type enum naming.
buildEnumNames(missionHudItemAmountTypeNames, MISSION_HUD_ITEM_AMT)({
    { MISSION_HUD_ITEM_AMT_MOB_CHECKLIST, "Mob checklist" },
    { MISSION_HUD_ITEM_AMT_LEADERS_IN_REGION, "Leaders in region" },
    { MISSION_HUD_ITEM_AMT_PIKMIN, "Pikmin count" },
    { MISSION_HUD_ITEM_AMT_LEADERS, "Leader count" },
    { MISSION_HUD_ITEM_AMT_PIKMIN_DEATHS, "Pikmin deaths" },
    { MISSION_HUD_ITEM_AMT_LEADER_KOS, "Leader KOs" },
});


//Possible types of mission medal.
enum MISSION_MEDAL {

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
enum MISSION_GRADING_MODE {

    //Based on points in different criteria.
    MISSION_GRADING_MODE_POINTS,
    
    //Based on whether the player reached the goal or not.
    MISSION_GRADING_MODE_GOAL,
    
    //Based on whether the player played or not.
    MISSION_GRADING_MODE_PARTICIPATION,
    
};


//DEPRECATED by MISSION_EV in 1.2.0.
//Possible goals in a mission.
enum MISSION_GOAL {

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
    
};


//DEPRECATED by MISSION_EV in 1.2.0.
//Possible ways to fail at a mission.
enum MISSION_FAIL_COND {

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
    
    //Defeating a certain amount of enemies.
    MISSION_FAIL_COND_DEFEAT_ENEMIES,
    
    //Ending from the pause menu.
    MISSION_FAIL_COND_PAUSE_MENU,
    
};


//DEPRECATED in 1.2.0 by MISSION_SCORE_CRITERION.
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
    
    //Points per enemy defeat point.
    MISSION_SCORE_CRITERIA_ENEMY_POINTS,
    
};


#pragma endregion
#pragma region General classes


/**
 * @brief Represents an event in the mission.
 */
struct MissionEvent {

    //--- Public members ---
    
    //Type of event.
    MISSION_EV type = MISSION_EV_PAUSE_MENU_END;
    
    //Index-related parameter, if applicable. Can be used for other things too.
    size_t indexParam = 0;
    
    //Amount-related parameter, if applicable. Can be used for other things too.
    size_t amountParam = 1;
    
    //Action to take when the event happens.
    MISSION_ACTION actionType = MISSION_ACTION_END_CLEAR;
    
    //Action script message to send, if applicable.
    string actionMessage;
    
    //Whether the time remaining becomes 0 for scoring purposes, if applicable.
    bool zeroTimeForScore = false;
    
};


/**
 * @brief A checklist of mobs that is relevant to a mission.
 */
struct MissionMobChecklist {

    //--- Public members ---
    
    //Type.
    MISSION_MOB_CHECKLIST type = MISSION_MOB_CHECKLIST_CUSTOM;
    
    //Amount, if any. 0 means all.
    size_t requiredAmount = 0;
    
    //For enemies, do they need to be collected, or is it enough for them
    //to be defeated?
    bool enemiesNeedCollection = false;
    
    //List of mob indexes, if applicable.
    vector<size_t> mobIdxs;
    
    
    //--- Public function declarations ---
    
    vector<size_t> calculateList() const;
    
};


/**
 * @brief One rule for how the score is determined.
 */
struct MissionScoreCriterion {

    //--- Public members ---
    
    //Type.
    MISSION_SCORE_CRITERION type = MISSION_SCORE_CRITERION_MOB_CHECKLIST;
    
    //Index-related parameter, if applicable. Can be used for other things too.
    size_t indexParam = 0;
    
    //Points received per every item in the criterion.
    int points = 1;
    
    //Whether it affects the HUD score, or only the final results.
    bool affectsHud = true;
    
};


/**
 * @brief Represents the sort of stuff that should be in a mission HUD item.
 */
struct MissionHudItem {

    //--- Public members ---
    
    //Whether it is enabled and visible.
    bool enabled = false;
    
    //Type of content to show.
    MISSION_HUD_ITEM_CONTENT contentType = MISSION_HUD_ITEM_CONTENT_TEXT;
    
    //Text to show, if applicable.
    string text;
    
    //Amount type, if applicable.
    MISSION_HUD_ITEM_AMT amountType = MISSION_HUD_ITEM_AMT_PIKMIN;
    
    //Fixed number for the total amount, if applicable.
    size_t totalAmount = 0;
    
    //List of mob checklists or regions to account for, if applicable.
    vector<size_t> idxsList;
    
};


/**
 * @brief Info about a given area's mission.
 */
struct MissionData {

    //--- Public members ---
    
    //Preset. Only really used for the editor's GUI.
    MISSION_PRESET preset = MISSION_PRESET_CUSTOM;
    
    //Mission events.
    vector<MissionEvent> events;
    
    //Mob checklists.
    vector<MissionMobChecklist> mobChecklists;
    
    //Mission grading mode.
    MISSION_GRADING_MODE gradingMode = MISSION_GRADING_MODE_GOAL;
    
    //Time limit in seconds, if any.
    size_t timeLimit = 0;
    
    //HUD items.
    vector<MissionHudItem> hudItems;
    
    //Scoring criteria.
    vector<MissionScoreCriterion> scoreCriteria;
    
    //Starting number of points.
    int startingPoints = 0;
    
    //Bronze medal point requirement.
    int bronzeReq = MISSION::DEF_MEDAL_REQ_BRONZE;
    
    //Silver medal point requirement.
    int silverReq = MISSION::DEF_MEDAL_REQ_SILVER;
    
    //Gold medal point requirement.
    int goldReq = MISSION::DEF_MEDAL_REQ_GOLD;
    
    //Platinum medal point requirement.
    int platinumReq = MISSION::DEF_MEDAL_REQ_PLATINUM;
    
    //The maker's record.
    int makerRecord = 0;
    
    //The date of the maker's record, or empty for no record.
    string makerRecordDate = "";
    
    
    //--- Public function declarations ---
    
    void applyPreset(MISSION_PRESET newPreset);
    MISSION_MEDAL getScoreMedal(int score);
    void reset();
    
};


//DEPRECATED by MissionData in 1.2.0.
/**
 * @brief Info about a given area's mission.
 */
struct MissionDataOld {

    //--- Public members ---
    
    //Mission goal.
    MISSION_GOAL goal = MISSION_GOAL_END_MANUALLY;
    
    //Does the mission goal require all relevant items, or just specific ones?
    bool goalAllMobs = true;
    
    //If the mission goal requires specific items, their mob indexes go here.
    unordered_set<size_t> goalMobIdxs;
    
    //Total amount of something required for the current mission goal.
    size_t goalAmount = 1;
    
    //Mission exit region center coordinates.
    Point goalExitCenter;
    
    //Mission exit region dimensions.
    Point goalExitSize = Point(MISSION::EXIT_MIN_SIZE);
    
    //Mission fail conditions bitmask. Use MISSION_FAIL_COND's indexes.
    Bitmask8 failConditions = 0;
    
    //Amount for the "reach too few Pikmin" mission fail condition.
    size_t failTooFewPikAmount = 0;
    
    //Amount for the "reach too many Pikmin" mission fail condition.
    size_t failTooManyPikAmount = 1;
    
    //Amount for the "lose Pikmin" mission fail condition.
    size_t failPikKilled = 1;
    
    //Amount for the "lose leaders" mission fail condition.
    size_t failLeadersKod = 1;
    
    //Amount for the "defeat enemies" mission fail condition.
    size_t failEnemiesDefeated = 1;
    
    //Seconds amount for the "time limit" mission fail condition.
    size_t failTimeLimit = MISSION::DEF_TIME_LIMIT;
    
    //Primary HUD element's fail condition. INVALID for none.
    size_t failHudPrimaryCond = INVALID;
    
    //Secondary HUD element's fail condition. INVALID for none.
    size_t failHudSecondaryCond = INVALID;
    
    //Mission grading mode.
    MISSION_GRADING_MODE gradingMode = MISSION_GRADING_MODE_GOAL;
    
    //Mission point multiplier for each Pikmin born.
    int pointsPerPikminBorn = 0;
    
    //Mission point multiplier for each Pikmin lost.
    int pointsPerPikminDeath = 0;
    
    //Mission point multiplier for each second left (only if time limit is on).
    int pointsPerSecLeft = 0;
    
    //Mission point multiplier for each second passed.
    int pointsPerSecPassed = 0;
    
    //Mission point multiplier for each treasure point obtained.
    int pointsPerTreasurePoint = 0;
    
    //Mission point multiplier for each enemy point obtained.
    int pointsPerEnemyPoint = 0;
    
    //If true, award points on enemy collection rather than on defeat.
    bool enemyPointsOnCollection = false;
    
    //Bitmask for mission fail point loss criteria. Use MISSION_SCORE_CRITERIA.
    Bitmask8 pointLossData = 0;
    
    //Bitmask for score HUD calculation criteria. Use MISSION_SCORE_CRITERIA.
    Bitmask8 pointHudData = 255;
    
    //Starting number of points.
    int startingPoints = 0;
    
    //Bronze medal point requirement.
    int bronzeReq = MISSION::DEF_MEDAL_REQ_BRONZE;
    
    //Silver medal point requirement.
    int silverReq = MISSION::DEF_MEDAL_REQ_SILVER;
    
    //Gold medal point requirement.
    int goldReq = MISSION::DEF_MEDAL_REQ_GOLD;
    
    //Platinum medal point requirement.
    int platinumReq = MISSION::DEF_MEDAL_REQ_PLATINUM;
    
    //The maker's record.
    int makerRecord = 0;
    
    //The date of the maker's record, or empty for no record.
    string makerRecordDate = "";
    
    
    //--- Public function declarations ---
    
    MISSION_MEDAL getScoreMedal(int score);
    
};


/**
 * @brief Info about a given mission's record.
 */
struct MissionRecord {

    //--- Public members ---
    
    //Has the mission's goal been cleared?
    bool clear = false;
    
    //Score obtained.
    int score = 0;
    
    //Date of the record.
    string date;
    
    
    //--- Public function declarations ---
    
    bool isPlatinum(const MissionDataOld& mission);
    
};


#pragma endregion
#pragma region Event types


/**
 * @brief Class interface for a mission event type.
 */
class MissionEvType {

public:

    //--- Public misc. definitions ---
    
    /**
     * @brief Information that's only useful for the editor.
     */
    struct EditorInfo {
    
        //--- Public members ---
        
        //Description of the event.
        string description;
        
        //The index-related parameter's name. Empty if not used.
        string indexParamName;
        
        //The index-related parameter's description. Empty if not used.
        string indexParamDescription;
        
        //The amount-related parameter's name. Empty if not used.
        string amountParamName;
        
        //The amount-related parameter's description. Empty if not used.
        string amountParamDescription;
        
    };
    
    
    /**
     * @brief Information that's only useful for the HUD.
     */
    struct HudInfo {
    
        //--- Public members ---
        
        //A description of the event. Empty if not used.
        string description;
        
        //The reason of what happened to trigger the event. Empty if not used.
        string reason;
        
    };
    
    
    //--- Public function declarations ---
    
    virtual ~MissionEvType() = default;
    virtual string getName() const = 0;
    virtual EditorInfo getEditorInfo() const = 0;
    virtual HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const = 0;
    virtual bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const = 0;
    virtual bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const = 0;
    
};


/**
 * @brief Class representing the "pause menu end" mission event.
 */
class MissionEvTypePauseEnd : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "mob clear" mission event.
 */
class MissionEvTypeMobChecklist : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "time limit" mission event.
 */
class MissionEvTypeTimeLimit : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "leaders in region" mission event.
 */
class MissionEvTypeLeadersInRegion : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin or more" mission event.
 */
class MissionEvTypePikminOrMore : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin or fewer" mission event.
 */
class MissionEvTypePikminOrFewer : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "lose Pikmin" mission event.
 */
class MissionEvTypeLosePikmin : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "lose leaders" mission event.
 */
class MissionEvTypeLoseLeaders : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "take damage" mission event.
 */
class MissionEvTypeTakeDamage : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "script" mission event.
 */
class MissionEvTypeScriptTrigger : public MissionEvType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEvent* ev, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


#pragma endregion
#pragma region Action types


/**
 * @brief Class interface for a mission action type.
 */
class MissionActionType {

public:

    //--- Public misc. definitions ---
    
    /**
     * @brief Information that's only useful for the editor.
     */
    struct EditorInfo {
    
        //--- Public members ---
        
        //Description of the event.
        string description;
        
    };
    
    
    //--- Public function declarations ---
    
    virtual ~MissionActionType() = default;
    virtual string getName() const = 0;
    virtual EditorInfo getEditorInfo() const = 0;
    virtual bool run(MissionEvent* ev, GameplayState* gameplay) const = 0;
    
};


/**
 * @brief Class representing the "mission end, clear" mission event.
 */
class MissionActionTypeEndClear : public MissionActionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    bool run(MissionEvent* ev, GameplayState* gameplay) const override;
    
};


/**
 * @brief Class representing the "mission end, fail" mission event.
 */
class MissionActionTypeEndFail : public MissionActionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    bool run(MissionEvent* ev, GameplayState* gameplay) const override;
    
};


/**
 * @brief Class representing the "send script message" mission event.
 */
class MissionActionTypeScriptMessage : public MissionActionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    bool run(MissionEvent* ev, GameplayState* gameplay) const override;
    
};


#pragma endregion
#pragma region Score criteria


/**
 * @brief Class interface for a mission score criterion type.
 */
class MissionScoreCriterionType {

public:

    //--- Public function declarations ---
    
    virtual ~MissionScoreCriterionType() = default;
    virtual string getName() const = 0;
    virtual size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const = 0;
    
};


/**
 * @brief Class representing the "mob checklist mob" mission score criterion.
 */
class MissionScoreCriterionTypeMobChecklist : public MissionScoreCriterionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin count" mission score criterion.
 */
class MissionScoreCriterionTypePikmin : public MissionScoreCriterionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin born" mission score criterion.
 */
class MissionScoreCriterionTypePikminBorn : public MissionScoreCriterionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin deaths" mission score criterion.
 */
class MissionScoreCriterionTypePikminDeaths : public MissionScoreCriterionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "seconds left" mission score criterion.
 */
class MissionScoreCriterionTypeSecLeft : public MissionScoreCriterionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "seconds passed" mission score criterion.
 */
class MissionScoreCriterionTypeSecPassed : public MissionScoreCriterionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "collection points" mission score criterion.
 */
class MissionScoreCriterionTypeCollectionPts :
    public MissionScoreCriterionType {
    
public:

    //--- Public function declarations ---
    
    string getName() const override;
    size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "defeat points" mission score criterion.
 */
class MissionScoreCriterionTypeDefeatPts :
    public MissionScoreCriterionType {
    
public:

    //--- Public function declarations ---
    
    string getName() const override;
    size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


#pragma endregion
#pragma region Fail conditions


/**
 * @brief Class interface for a mission fail condition.
 */
class MissionFail {

public:

    //--- Public function declarations ---
    
    virtual ~MissionFail() = default;
    virtual string getName() const = 0;
    virtual int getCurAmount(GameplayState* gameplay) const = 0;
    virtual int getReqAmount(GameplayState* gameplay) const = 0;
    virtual string getPlayerDescription(MissionDataOld* mission) const = 0;
    virtual string getStatus(
        int cur, int req, float percentage
    ) const = 0;
    virtual string getEndReason(MissionDataOld* mission) const = 0;
    virtual bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const = 0;
    virtual string getHudLabel(GameplayState* gameplay) const = 0;
    virtual bool hasHudContent() const = 0;
    virtual bool isMet(GameplayState* gameplay) const = 0;
    
};


/**
 * @brief Class representing the "defeat enemies" mission fail condition.
 */
class MissionFailDefeatEnemies : public MissionFail {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel(GameplayState* gameplay) const override;
    bool hasHudContent() const override;
    bool isMet(GameplayState* gameplay) const override;
    
};


/**
 * @brief Class representing the "lose leaders" mission fail condition.
 */
class MissionFailLoseLeaders : public MissionFail {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel(GameplayState* gameplay) const override;
    bool hasHudContent() const override;
    bool isMet(GameplayState* gameplay) const override;
    
};


/**
 * @brief Class representing the "lose Pikmin" mission fail condition.
 */
class MissionFailLosePikmin : public MissionFail {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel(GameplayState* gameplay) const override;
    bool hasHudContent() const override;
    bool isMet(GameplayState* gameplay) const override;
    
};


/**
 * @brief Class representing the "end from pause menu" mission fail condition.
 */
class MissionFailPauseMenu : public MissionFail {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel(GameplayState* gameplay) const override;
    bool hasHudContent() const override;
    bool isMet(GameplayState* gameplay) const override;
    
};


/**
 * @brief Class representing the "take damage" mission fail condition.
 */
class MissionFailTakeDamage : public MissionFail {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel(GameplayState* gameplay) const override;
    bool hasHudContent() const override;
    bool isMet(GameplayState* gameplay) const override;
    
};


/**
 * @brief Class representing the "time limit" mission fail condition.
 */
class MissionFailTimeLimit: public MissionFail {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel(GameplayState* gameplay) const override;
    bool hasHudContent() const override;
    bool isMet(GameplayState* gameplay) const override;
    
};


/**
 * @brief Class representing the "reach too few Pikmin" mission fail condition.
 */
class MissionFailTooFewPikmin : public MissionFail {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel(GameplayState* gameplay) const override;
    bool hasHudContent() const override;
    bool isMet(GameplayState* gameplay) const override;
    
};


/**
 * @brief Class representing the "reach too many Pikmin" mission fail condition.
 */
class MissionFailTooManyPikmin : public MissionFail {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel(GameplayState* gameplay) const override;
    bool hasHudContent() const override;
    bool isMet(GameplayState* gameplay) const override;
    
};


#pragma endregion
#pragma region Goals


/**
 * @brief Class interface for a mission goal.
 */
class MissionGoal {

public:

    //--- Public function declarations ---
    
    virtual ~MissionGoal() = default;
    virtual string getName() const = 0;
    virtual int getCurAmount(GameplayState* gameplay) const = 0;
    virtual int getReqAmount(GameplayState* gameplay) const = 0;
    virtual string getPlayerDescription(MissionDataOld* mission) const = 0;
    virtual string getStatus(
        int cur, int req, float percentage
    ) const = 0;
    virtual string getEndReason(MissionDataOld* mission) const = 0;
    virtual bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const = 0;
    virtual string getHudLabel() const = 0;
    virtual bool isMet(GameplayState* gameplay) const = 0;
    virtual bool isMobApplicable(MobType* type) const = 0;
    
};


/**
 * @brief Class representing the "battle enemies" mission goal.
 */
class MissionGoalBattleEnemies : public MissionGoal {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel() const override;
    bool isMet(GameplayState* gameplay) const override;
    bool isMobApplicable(MobType* type) const override;
    
};


/**
 * @brief Class representing the "collect treasures" mission goal.
 */
class MissionGoalCollectTreasures : public MissionGoal {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel() const override;
    bool isMet(GameplayState* gameplay) const override;
    bool isMobApplicable(MobType* type) const override;
    
};


/**
 * @brief Class representing the "end manually" mission goal.
 */
class MissionGoalEndManually : public MissionGoal {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel() const override;
    bool isMet(GameplayState* gameplay) const override;
    bool isMobApplicable(MobType* type) const override;
    
};


/**
 * @brief Class representing the "get to the exit" mission goal.
 */
class MissionGoalGetToExit : public MissionGoal {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel() const override;
    bool isMet(GameplayState* gameplay) const override;
    bool isMobApplicable(MobType* type) const override;
    
};


/**
 * @brief Class representing the "reach Pikmin amount" mission goal.
 */
class MissionGoalGrowPikmin : public MissionGoal {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel() const override;
    bool isMet(GameplayState* gameplay) const override;
    bool isMobApplicable(MobType* type) const override;
    
};


/**
 * @brief Class representing the "timed survival" mission goal.
 */
class MissionGoalTimedSurvival : public MissionGoal {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getCurAmount(GameplayState* gameplay) const override;
    int getReqAmount(GameplayState* gameplay) const override;
    string getPlayerDescription(MissionDataOld* mission) const override;
    string getStatus(
        int cur, int req, float percentage
    ) const override;
    string getEndReason(MissionDataOld* mission) const override;
    bool getEndZoomData(
        GameplayState* gameplay, Point* outCamPos, float* outCamZoom
    ) const override;
    string getHudLabel() const override;
    bool isMet(GameplayState* gameplay) const override;
    bool isMobApplicable(MobType* type) const override;
    
};


#pragma endregion
#pragma region Score criteria (old)


/**
 * @brief Class interface for a mission score criterion.
 */
class MissionScoreCriterionOld {

public:

    //--- Public function declarations ---
    
    virtual ~MissionScoreCriterionOld() = default;
    virtual string getName() const = 0;
    virtual int getMultiplier(MissionDataOld* mission) const = 0;
    virtual int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const = 0;
    
};


/**
 * @brief Class representing the "enemy points" mission score criterion.
 */
class MissionScoreCriterionEnemyPoints : public MissionScoreCriterionOld {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin born" mission score criterion.
 */
class MissionScoreCriterionPikminBorn : public MissionScoreCriterionOld {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin death" mission score criterion.
 */
class MissionScoreCriterionPikminDeath : public MissionScoreCriterionOld {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "seconds left" mission score criterion.
 */
class MissionScoreCriterionSecLeft : public MissionScoreCriterionOld {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "seconds passed" mission score criterion.
 */
class MissionScoreCriterionSecPassed : public MissionScoreCriterionOld {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "treasure points" mission score criterion.
 */
class MissionScoreCriterionTreasurePoints : public MissionScoreCriterionOld {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};
